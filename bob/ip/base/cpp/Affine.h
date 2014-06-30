/**
 * @author Manuel Günther <manuel.guenther@idiap.ch>
 * @date Thu Jun 26 09:33:10 CEST 2014
 *
 * This file defines functions and classes for affine image transformations
 *
 * Copyright (C) Idiap Research Institute, Martigny, Switzerland
 */

#ifndef BOB_IP_BASE_AFFINE_H
#define BOB_IP_BASE_AFFINE_H

#include <boost/shared_ptr.hpp>
#include <bob/core/assert.h>
#include <bob/core/check.h>


namespace bob { namespace ip { namespace base {

    /** Implementation of the bi-linear interpolation of a source to a target image. */

    template <typename T, bool mask>
    void transform(
        const blitz::Array<T,2>& source,
        const blitz::Array<bool,2>& source_mask,
        const blitz::TinyVector<double,2>& source_center,
        blitz::Array<double,2>& target,
        blitz::Array<bool,2>& target_mask,
        const blitz::TinyVector<double,2>& target_center,
        const blitz::TinyVector<double,2>& scaling_factor,
        const double& rotation_angle
     ){
      // This is the fastest version of the function that I can imagine...
      // It handles two different coordinate systems: original image and new image

      // transformation center in original image
      const double original_center_y = source_center[0],
                   original_center_x = source_center[1];
      // transformation center in new image:
      const double new_center_y = target_center[0],
                   new_center_x = target_center[1];

      // With these positions, we can define a mapping from the new image to the original image
      const double sin_angle = -sin(rotation_angle * M_PI / 180.),
                   cos_angle = cos(rotation_angle * M_PI / 180.);

      // we compute the distance in the source image, when going 1 pixel in the new image
      const double col_dy = -sin_angle / scaling_factor[0],
                   col_dx = cos_angle / scaling_factor[1];
      const double row_dy = cos_angle / scaling_factor[0],
                   row_dx = sin_angle / scaling_factor[1];


      // Now, we iterate through the target image, and compute pixel positions in the source.
      // For this purpose, get the (0,0) position of the target image in source image coordinates:
      double origin_y = original_center_y - (new_center_y * cos_angle - new_center_x * sin_angle) / scaling_factor[0];
      double origin_x = original_center_x - (new_center_x * cos_angle + new_center_y * sin_angle) / scaling_factor[1];
      // WARNING: I am not sure, if this is correct, or if we rather need to do something like:
      //double origin_y = original_center_y - (new_center_y * cos_angle / scaling_factor[0] - new_center_x * sin_angle / scaling_factor[1]);
      //double origin_x = original_center_x - (new_center_x * cos_angle / scaling_factor[1] + new_center_y * sin_angle / scaling_factor[0]);
      // Note: as long a single scale is used, or scaling is done without rotation, it should be the same.
      //   (at least, the tests pass with both ways)

      // some helpers for the interpolation
      int ox, oy;
      double mx, my;
      int h = source.extent(0)-1;
      int w = source.extent(1)-1;

      int size_y = target.extent(0), size_x = target.extent(1);

      // Ok, so let's do it.
      for (int y = 0; y < size_y; ++y){
        // set the source image point to first point in row
        double source_x = origin_x, source_y = origin_y;
        // iterate over the row
        for (int x = 0; x < size_x; ++x){

          // We are at the desired pixel in the new image. Interpolate the old image's pixels:
          double& res = target(y,x) = 0.;

          // split each source x and y in integral and decimal digits
          ox = std::floor(source_x);
          oy = std::floor(source_y);
          mx = source_x - ox;
          my = source_y - oy;

          // add the four values bi-linearly interpolated
          if (mask){
            bool& new_mask = target_mask(y,x) = false;
            // upper left
            if (ox >= 0 && oy >= 0 && ox <= w && oy <= h && source_mask(oy,ox)){
              res += (1.-mx) * (1.-my) * source(oy,ox);
              new_mask = true;
            }
            // upper right
            if (ox >= -1 && oy >= 0 && ox < w && oy <= h && source_mask(oy,ox+1)){
              res += mx * (1.-my) * source(oy,ox+1);
              new_mask = true;
            }
            // lower left
            if (ox >= 0 && oy >= -1 && ox <= w && oy < h && source_mask(oy+1,ox)){
              res += (1.-mx) * my * source(oy+1,ox);
              new_mask = true;
            }
            // lower right
            if (ox >= -1 && oy >= -1 && ox < w && oy < h && source_mask(oy+1,ox+1)){
              res += mx * my * source(oy+1,ox+1);
              new_mask = true;
            }
          } else {
            // upper left
            if (ox >= 0 && oy >= 0 && ox <= w && oy <= h)
              res += (1.-mx) * (1.-my) * source(oy,ox);

            // upper right
            if (ox >= -1 && oy >= 0 && ox < w && oy <= h)
              res += mx * (1.-my) * source(oy,ox+1);

            // lower left
            if (ox >= 0 && oy >= -1 && ox <= w && oy < h)
              res += (1.-mx) * my * source(oy+1,ox);

            // lower right
            if (ox >= -1 && oy >= -1 && ox < w && oy < h)
              res += mx * my * source(oy+1,ox+1);
          }

          // done with this pixel...
          // go to the next source pixel in the row
          source_y += col_dy;
          source_x += col_dx;
        }
        // at the end of the row, we shift the origin to the next line
        origin_y += row_dy;
        origin_x += row_dx;
      }
      // done!
    }


    /**
      * @brief Function which extracts a rectangle of maximal area from a
      *   2D mask of booleans (i.e. a 2D blitz array).
      * @warning The function assumes that the true values on the mask form
      *   a convex area.
      * @param mask The 2D input blitz array mask.
      * @result A blitz::TinyVector which contains in the following order:
      *   0/ The y-coordinate of the top left corner
      *   1/ The x-coordinate of the top left corner
      *   2/ The height of the rectangle
      *   3/ The width of the rectangle
      */
    const blitz::TinyVector<int,4> maxRectInMask(const blitz::Array<bool,2>& mask);



/************************************************************************
**************  Scaling functionality  **********************************
************************************************************************/


    /** helper function to compute the scale required by bob.ip.base.GeomNorm for the given image shapes */
    static inline blitz::TinyVector<double,2> _get_scale_factor(const blitz::TinyVector<int,2>& src_shape, const blitz::TinyVector<int,2>& dst_shape){
      double y_scale = (dst_shape[0]-1.) / (src_shape[0]-1.);
      double x_scale = (dst_shape[1]-1.) / (src_shape[1]-1.);
      return blitz::TinyVector<double,2>(y_scale, x_scale);
    }

    /**
     * @brief Function which rescales a 2D blitz::array/image of a given type.
     *   The first dimension is the height (y-axis), whereas the second
     *   one is the width (x-axis).
     * @param src The input blitz array
     * @param dst The output blitz array. The new array is resized according
     *   to the dimensions of this dst array.
     */
    template <typename T>
    void scale(const blitz::Array<T,2>& src, blitz::Array<double,2>& dst){
      blitz::TinyVector<double,2> offset(0,0);
      blitz::Array<bool,2> src_mask, dst_mask;
      // .. apply scale with (0,0) as offset and 0 as rotation angle
      transform<T,false>(src, src_mask, offset, dst, dst_mask, offset, _get_scale_factor(src.shape(), dst.shape()), 0.);
    }

    /**
     * @brief Function which rescales a 2D blitz::array/image of a given type.
     *   The first dimension is the height (y-axis), whereas the second
     *   one is the width (x-axis).
     * @param src The input blitz array
     * @param src_mask The input blitz boolean mask array
     * @param dst The output blitz array. The new array is resized according
     *   to the dimensions of this dst array.
     * @param dst_mask The output blitz boolean mask array
     */
    template <typename T>
    void scale(const blitz::Array<T,2>& src, const blitz::Array<bool,2>& src_mask, blitz::Array<double,2>& dst, blitz::Array<bool,2>& dst_mask){
      blitz::TinyVector<double,2> offset(0,0);
      // .. apply scale with (0,0) as offset and 0 as rotation angle
      transform<T,true>(src, src_mask, offset, dst, dst_mask, offset, _get_scale_factor(src.shape(), dst.shape()), 0.);
    }

    /**
     * @brief Function which rescales a 3D blitz::array/image of a given type.
     *   The first dimension is the number of color plane, the second is the
     * height (y-axis), whereas the third one is the width (x-axis).
     * @param src The input blitz array
     * @param dst The output blitz array. The new array is resized according
     *   to the dimensions of this dst array.
     */
    template <typename T>
    void scale(const blitz::Array<T,3>& src, blitz::Array<double,3>& dst)
    {
      // Check number of planes
      bob::core::array::assertSameDimensionLength(src.extent(0), dst.extent(0));
      for (int p = 0; p < dst.extent(0); ++p){
        const blitz::Array<T,2> src_slice = src(p, blitz::Range::all(), blitz::Range::all());
        blitz::Array<double,2> dst_slice =dst(p, blitz::Range::all(), blitz::Range::all());
        // Process one plane
        scale(src_slice, dst_slice);
      }
    }

    template <typename T>
    void scale(const blitz::Array<T,3>& src, const blitz::Array<bool,3>& src_mask, blitz::Array<double,3>& dst, blitz::Array<bool,3>& dst_mask)
    {
      // Check number of planes
      bob::core::array::assertSameDimensionLength(src.extent(0), dst.extent(0));
      bob::core::array::assertSameDimensionLength(src.extent(0), src_mask.extent(0));
      bob::core::array::assertSameDimensionLength(src_mask.extent(0), dst_mask.extent(0));
      for (int p = 0; p < dst.extent(0); ++p){
        const blitz::Array<T,2> src_slice = src(p, blitz::Range::all(), blitz::Range::all());
        const blitz::Array<bool,2> src_mask_slice = src_mask(p, blitz::Range::all(), blitz::Range::all());
        blitz::Array<double,2> dst_slice = dst(p, blitz::Range::all(), blitz::Range::all());
        blitz::Array<bool,2> dst_mask_slice = dst_mask(p, blitz::Range::all(), blitz::Range::all());
        // Process one plane
        scale(src_slice, src_mask_slice, dst_slice, dst_mask_slice);
      }
    }

    /**
     * @brief Function which returns the shape of an output blitz::array
     *   when rescaling an input image with the given scale factor.
     * @param src The input blitz array shape
     * @param scale_factor The scaling factor to apply
     * @return A blitz::TinyVector containing the shape of the rescaled image
     */
    template <int D>
    blitz::TinyVector<int, D> getScaledShape(const blitz::TinyVector<int, D> src_shape, const double scale_factor){
      blitz::TinyVector<int, D> dst_shape = src_shape;
      dst_shape(D-2) = floor(dst_shape(D-2) * scale_factor + 0.5);
      dst_shape(D-1) = floor(dst_shape(D-1) * scale_factor + 0.5);
      return dst_shape;
    }


/************************************************************************
**************  Rotating functionality  *********************************
************************************************************************/

    /**
     * @brief Function which rotates a 2D blitz::array/image of a given type with the given angle in degrees.
     *   The first dimension is the height (y-axis), whereas the second
     *   one is the width (x-axis).
     * @param src The input blitz array
     * @param dst The output blitz array
     * @param rotation_angle The angle in degrees to rotate the image with
     */
    template <typename T>
    void rotate(const blitz::Array<T,2>& src, blitz::Array<double,2>& dst, const double rotation_angle){
      // rotation offset is the center of the image
      blitz::TinyVector<double,2> src_offset((src.extent(0)-1.)/2.,(src.extent(1)-1.)/2.);
      blitz::TinyVector<double,2> dst_offset((dst.extent(0)-1.)/2.,(dst.extent(1)-1.)/2.);
      blitz::Array<bool,2> src_mask, dst_mask;
      // .. apply scale with (0,0) as offset and 0 as rotation angle
      transform<T,false>(src, src_mask, src_offset, dst, dst_mask, dst_offset, blitz::TinyVector<double,2>(1., 1.), rotation_angle);
    }

    /**
     * @brief Function which rotates a 2D blitz::array/image of a given type with the given angle in degrees.
     *   The first dimension is the height (y-axis), whereas the second
     *   one is the width (x-axis).
     * @param src The input blitz array
     * @param src_mask The input blitz boolean mask array
     * @param dst The output blitz array
     * @param dst_mask The output blitz boolean mask array
     * @param rotation_angle The angle in degrees to rotate the image with
     */
    template <typename T>
    void rotate(const blitz::Array<T,2>& src, const blitz::Array<bool,2>& src_mask, blitz::Array<double,2>& dst, blitz::Array<bool,2>& dst_mask, const double rotation_angle){
      // rotation offset is the center of the image
      blitz::TinyVector<double,2> src_offset((src.extent(0)-1.)/2.,(src.extent(1)-1.)/2.);
      blitz::TinyVector<double,2> dst_offset((dst.extent(0)-1.)/2.,(dst.extent(1)-1.)/2.);
      // .. apply scale with (0,0) as offset and 0 as rotation angle
      transform<T,true>(src, src_mask, src_offset, dst, dst_mask, dst_offset, blitz::TinyVector<double,2>(1., 1.), rotation_angle);
    }

    /**
     * @brief Function which rotates a 3D blitz::array/image of a given type.
     *   The first dimension is the number of color plane, the second is the
     * height (y-axis), whereas the third one is the width (x-axis).
     * @param src The input blitz array
     * @param dst The output blitz array
     * @param rotation_angle The angle in degrees to rotate the image with
     */
    template <typename T>
    void rotate(const blitz::Array<T,3>& src, blitz::Array<double,3>& dst, const double rotation_angle)
    {
      // Check number of planes
      bob::core::array::assertSameDimensionLength(src.extent(0), dst.extent(0));
      for (int p = 0; p < dst.extent(0); ++p){
        const blitz::Array<T,2> src_slice = src(p, blitz::Range::all(), blitz::Range::all());
        blitz::Array<double,2> dst_slice = dst(p, blitz::Range::all(), blitz::Range::all());
        // Process one plane
        rotate(src_slice, dst_slice, rotation_angle);
      }
    }

    template <typename T>
    void rotate(const blitz::Array<T,3>& src, const blitz::Array<bool,3>& src_mask, blitz::Array<double,3>& dst, blitz::Array<bool,3>& dst_mask, const double rotation_angle)
    {
      // Check number of planes
      bob::core::array::assertSameDimensionLength(src.extent(0), dst.extent(0));
      bob::core::array::assertSameDimensionLength(src.extent(0), src_mask.extent(0));
      bob::core::array::assertSameDimensionLength(src_mask.extent(0), dst_mask.extent(0));
      for (int p = 0; p < dst.extent(0); ++p){
        const blitz::Array<T,2> src_slice = src(p, blitz::Range::all(), blitz::Range::all());
        const blitz::Array<bool,2> src_mask_slice = src_mask(p, blitz::Range::all(), blitz::Range::all());
        blitz::Array<double,2> dst_slice = dst(p, blitz::Range::all(), blitz::Range::all());
        blitz::Array<bool,2> dst_mask_slice = dst_mask(p, blitz::Range::all(), blitz::Range::all());
        // Process one plane
        rotate(src_slice, src_mask_slice, dst_slice, dst_mask_slice, rotation_angle);
      }
    }

    /**
     * @brief Function which returns the shape of an output blitz::array
     *   when rotating an input image with the given rotation angle.
     * @param src_shape The input blitz array shape
     * @param rotation_angle The angle in degrees to rotate the image with
     * @return A blitz::TinyVector containing the shape of the rotated image
     */
    template <int D>
    blitz::TinyVector<int, D> getRotatedShape(const blitz::TinyVector<int, D> src_shape, const double rotation_angle){
      blitz::TinyVector<int, D> dst_shape = src_shape;
      // compute rotation shape
      double rad_angle = rotation_angle * M_PI / 180.;
      const double absCos = std::abs(cos(rad_angle));
      const double absSin = std::abs(sin(rad_angle));
      dst_shape(D-2) = floor(src_shape[D-2] * absCos + src_shape[D-1] * absSin + 0.5);
      dst_shape(D-1) = floor(src_shape[D-1] * absCos + src_shape[D-2] * absSin + 0.5);
      return dst_shape;
    }

} } } // namespaces

#endif // BOB_IP_BASE_AFFINE_H


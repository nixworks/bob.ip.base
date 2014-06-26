/**
 * @author Manuel Guenther <manuel.guenthr@idiap.ch>
 * @date Tue Jun 24 14:03:17 CEST 2014
 *
 * @brief Binds the GeomNorm class to python
 *
 * Copyright (C) 2011-2014 Idiap Research Institute, Martigny, Switzerland
 */

#include "main.h"

/******************************************************************/
/************ Constructor Section *********************************/
/******************************************************************/

static auto GeomNorm_doc = bob::extension::ClassDoc(
  BOB_EXT_MODULE_PREFIX ".GeomNorm",
  "Objects of this class, after configuration, can perform a geometric normalization of images",
  "The geometric normalization is a combination of rotation, scaling and cropping an image."
).add_constructor(
  bob::extension::FunctionDoc(
    "__init__",
    "Constructs a GeomNorm object with the given scale, angle, size of the new image and transformation offset in the new image",
    ".. todo:: Add a more detailed documentation of GeomNorm, i.e., by adding some equations",
    true
  )
  .add_prototype("rotation_angle, scaling_factor, crop_size, crop_offset", "")
  .add_prototype("other", "")
  .add_parameter("rotation_angle", "float", "The rotation angle **in degrees** that sould be applied")
  .add_parameter("scaling_factor", "float", "The scale factor to apply")
  .add_parameter("crop_size", "(int, int)", "The resolution of the processed images")
  .add_parameter("crop_offset", "(float, float)", "The transformation offset in the processed images")
  .add_parameter("other", ":py:class:`GeomNorm`", "Another GeomNorm object to copy")
);


static int PyBobIpBaseGeomNorm_init(PyBobIpBaseGeomNormObject* self, PyObject* args, PyObject* kwargs) {
  TRY

  char* kwlist1[] = {c("rotation_angle"), c("scaling_factor"), c("crop_size"), c("crop_offset"), NULL};
  char* kwlist2[] = {c("other"), NULL};

  // get the number of command line arguments
  Py_ssize_t nargs = (args?PyTuple_Size(args):0) + (kwargs?PyDict_Size(kwargs):0);

  if (!nargs){
    // at least one argument is required
    GeomNorm_doc.print_usage();
    PyErr_Format(PyExc_TypeError, "`%s' constructor requires at least one parameter", Py_TYPE(self)->tp_name);
    return -1;
  } // nargs == 0

  if (nargs == 1){
    // copy constructor
    PyBobIpBaseGeomNormObject* geomNorm;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!", kwlist2, &PyBobIpBaseGeomNormType, &geomNorm)){
      GeomNorm_doc.print_usage();
      return -1;
    }
    self->cxx.reset(new bob::ip::base::GeomNorm(*geomNorm->cxx));
    return 0;
  } // nargs == 1

  double scale, angle;
  blitz::TinyVector<int,2> size;
  blitz::TinyVector<double,2> offset;
  // more than one parameter; check the second one
  if (!(PyArg_ParseTupleAndKeywords(args, kwargs, "dd(ii)(dd)", kwlist1, &scale, &angle, &size[0], &size[1], &offset[0], &offset[1]))){
    GeomNorm_doc.print_usage();
    return -1;
  }
  self->cxx.reset(new bob::ip::base::GeomNorm(scale, angle, size, offset));
  return 0;

  CATCH("cannot create GeomNorm object", -1)
}

static void PyBobIpBaseGeomNorm_delete(PyBobIpBaseGeomNormObject* self) {
  Py_TYPE(self)->tp_free((PyObject*)self);
}

int PyBobIpBaseGeomNorm_Check(PyObject* o) {
  return PyObject_IsInstance(o, reinterpret_cast<PyObject*>(&PyBobIpBaseGeomNormType));
}

static PyObject* PyBobIpBaseGeomNorm_RichCompare(PyBobIpBaseGeomNormObject* self, PyObject* other, int op) {
  TRY

  if (!PyBobIpBaseGeomNorm_Check(other)) {
    PyErr_Format(PyExc_TypeError, "cannot compare `%s' with `%s'", Py_TYPE(self)->tp_name, Py_TYPE(other)->tp_name);
    return 0;
  }
  auto other_ = reinterpret_cast<PyBobIpBaseGeomNormObject*>(other);
  switch (op) {
    case Py_EQ:
      if (*self->cxx==*other_->cxx) Py_RETURN_TRUE; else Py_RETURN_FALSE;
    case Py_NE:
      if (*self->cxx==*other_->cxx) Py_RETURN_FALSE; else Py_RETURN_TRUE;
    default:
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
  }
  CATCH("cannot compare GeomNorm objects", 0)
}


/******************************************************************/
/************ Variables Section ***********************************/
/******************************************************************/

static auto angle = bob::extension::VariableDoc(
  "rotation_angle",
  "float",
  "The rotation angle, with read and write access"
);
PyObject* PyBobIpBaseGeomNorm_getAngle(PyBobIpBaseGeomNormObject* self, void*){
  TRY
  return Py_BuildValue("d", self->cxx->getRotationAngle());
  CATCH("rotation_angle could not be read", 0)
}
int PyBobIpBaseGeomNorm_setAngle(PyBobIpBaseGeomNormObject* self, PyObject* value, void*){
  TRY
  double d = PyFloat_AsDouble(value);
  if (PyErr_Occurred()) return -1;
  self->cxx->setRotationAngle(d);
  return 0;
  CATCH("rotation_angle could not be set", -1)
}

static auto scale = bob::extension::VariableDoc(
  "scaling_factor",
  "float",
  "The scale factor, with read and write access"
);
PyObject* PyBobIpBaseGeomNorm_getScale(PyBobIpBaseGeomNormObject* self, void*){
  TRY
  return Py_BuildValue("d", self->cxx->getScalingFactor());
  CATCH("scaling_factor could not be read", 0)
}
int PyBobIpBaseGeomNorm_setScale(PyBobIpBaseGeomNormObject* self, PyObject* value, void*){
  TRY
  double d = PyFloat_AsDouble(value);
  if (PyErr_Occurred()) return -1;
  self->cxx->setScalingFactor(d);
  return 0;
  CATCH("scaling_factor could not be set", -1)
}

static auto cropSize = bob::extension::VariableDoc(
  "crop_size",
  "(int, int)",
  "The size of the processed image, with read and write access"
);
PyObject* PyBobIpBaseGeomNorm_getCropSize(PyBobIpBaseGeomNormObject* self, void*){
  TRY
  auto r = self->cxx->getCropSize();
  return Py_BuildValue("(ii)", r[0], r[1]);
  CATCH("crop_size could not be read", 0)
}
int PyBobIpBaseGeomNorm_setCropSize(PyBobIpBaseGeomNormObject* self, PyObject* value, void*){
  TRY
  blitz::TinyVector<double,2> r;
  if (!PyArg_ParseTuple(value, "dd", &r[0], &r[1])){
    PyErr_Format(PyExc_RuntimeError, "%s %s expects a tuple of two floats", Py_TYPE(self)->tp_name, cropSize.name());
    return -1;
  }
  self->cxx->setCropSize(r);
  return 0;
  CATCH("crop_size could not be set", -1)
}

static auto cropOffset = bob::extension::VariableDoc(
  "crop_offset",
  "(float, float)",
  "The size of the processed image, with read and write access"
);
PyObject* PyBobIpBaseGeomNorm_getCropOffset(PyBobIpBaseGeomNormObject* self, void*){
  TRY
  auto r = self->cxx->getCropOffset();
  return Py_BuildValue("(dd)", r[0], r[1]);
  CATCH("crop_offset could not be read", 0)
}
int PyBobIpBaseGeomNorm_setCropOffset(PyBobIpBaseGeomNormObject* self, PyObject* value, void*){
  TRY
  blitz::TinyVector<double,2> r;
  if (!PyArg_ParseTuple(value, "dd", &r[0], &r[1])){
    PyErr_Format(PyExc_RuntimeError, "%s %s expects a tuple of two floats", Py_TYPE(self)->tp_name, cropOffset.name());
    return -1;
  }
  self->cxx->setCropOffset(r);
  return 0;
  CATCH("crop_offset could not be set", -1)
}

static PyGetSetDef PyBobIpBaseGeomNorm_getseters[] = {
    {
      angle.name(),
      (getter)PyBobIpBaseGeomNorm_getAngle,
      (setter)PyBobIpBaseGeomNorm_setAngle,
      angle.doc(),
      0
    },
    {
      scale.name(),
      (getter)PyBobIpBaseGeomNorm_getScale,
      (setter)PyBobIpBaseGeomNorm_setScale,
      scale.doc(),
      0
    },
    {
      cropSize.name(),
      (getter)PyBobIpBaseGeomNorm_getCropSize,
      (setter)PyBobIpBaseGeomNorm_setCropSize,
      cropSize.doc(),
      0
    },
    {
      cropOffset.name(),
      (getter)PyBobIpBaseGeomNorm_getCropOffset,
      (setter)PyBobIpBaseGeomNorm_setCropOffset,
      cropOffset.doc(),
      0
    },
    {0}  /* Sentinel */
};


/******************************************************************/
/************ Functions Section ***********************************/
/******************************************************************/

static auto process = bob::extension::FunctionDoc(
  "process",
  "This function processes the image or position",
  ".. todo:: Add more detailed information\n\n"
  ".. note:: The :py:func:`__call__` function is an alias for this method.",
  true
)
.add_prototype("input, output, center")
.add_prototype("input, input_mask, output, output_mask, center")
.add_prototype("position, center", "transformed")
.add_parameter("input", "array_like (2D or 3D)", "The input image to which GeomNorm should be applied")
.add_parameter("output", "array_like (2D or 3D, float)", "The output image, which must be of size :py:attr:`cropped_size`")
.add_parameter("center", "(float, float)", "The transformation center in the given image; this will be placed to :py:attr:`crop_offset` in the output image")
.add_parameter("input_mask", "array_like (bool, 2D or 3D)", "An input mask of valid pixels before geometric normalization, must be of same size as ``input``")
.add_parameter("output_mask", "array_like (bool, 2D or 3D)", "The output mask of valid pixels after geometric normalization, must be of same size as ``output``")
.add_parameter("position", "(float, float)", "A position in input image space that will be transformed to output image space (might be outside of the crop area)")
.add_return("transformed", "uint16", "The resulting GeomNorm code at the given position in the image")
;

template <typename T>
static PyObject* process_inner(PyBobIpBaseGeomNormObject* self, PyBlitzArrayObject* input, PyBlitzArrayObject* input_mask, PyBlitzArrayObject* output, PyBlitzArrayObject* output_mask, const blitz::TinyVector<double,2>& offset){
  if (input_mask && output_mask){
    self->cxx->process(*PyBlitzArrayCxx_AsBlitz<T,2>(input), *PyBlitzArrayCxx_AsBlitz<bool,2>(input_mask), *PyBlitzArrayCxx_AsBlitz<double,2>(output), *PyBlitzArrayCxx_AsBlitz<bool,2>(output_mask), offset);
  } else {
    self->cxx->process(*PyBlitzArrayCxx_AsBlitz<T,2>(input), *PyBlitzArrayCxx_AsBlitz<double,2>(output), offset);
  }
  Py_RETURN_NONE;
}

static PyObject* PyBobIpBaseGeomNorm_process(PyBobIpBaseGeomNormObject* self, PyObject* args, PyObject* kwargs) {
  TRY
  static char* kwlist1[] = {c("input"), c("output"), c("center"), 0};
  static char* kwlist2[] = {c("input"), c("input_mask"), c("output"), c("output_mask"), c("center"), 0};
  static char* kwlist3[] = {c("position"), c("center"), 0};

  // get the number of command line arguments
  Py_ssize_t nargs = (args?PyTuple_Size(args):0) + (kwargs?PyDict_Size(kwargs):0);

  PyBlitzArrayObject* input = 0,* input_mask = 0,* output = 0,* output_mask = 0;
  blitz::TinyVector<double,2> center(0,0), position(0,0);

  switch (nargs){
    case 2:{
      // with position
      if (!PyArg_ParseTupleAndKeywords(args, kwargs, "(dd)(dd)", kwlist3, &position[0], &position[1], &center[0], &center[1])){
        process.print_usage();
        return 0;
      }
      auto transformed = self->cxx->process(position, center);
      // return here
      return Py_BuildValue("(dd)", transformed[0], transformed[1]);
    }
    case 3:{
      // with input and output array
      if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&O&(dd)", kwlist1, &PyBlitzArray_Converter, &input, &PyBlitzArray_OutputConverter, &output, &center[0], &center[1])){
        process.print_usage();
        return 0;
      }
      break;
    }
    case 5:{
      // with mask
      if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&O&O&O&(dd)", kwlist2, &PyBlitzArray_Converter, &input, &PyBlitzArray_Converter, &input_mask, &PyBlitzArray_OutputConverter, &output, &PyBlitzArray_OutputConverter, &output_mask, &center[0], &center[1])){
        process.print_usage();
        return 0;
      }
      break;
    }
    default:{
      process.print_usage();
      PyErr_Format(PyExc_TypeError, "`%s' process called with wrong number of parameters", Py_TYPE(self)->tp_name);
      return 0;
    }
  } // switch

  auto input_ = make_safe(input), output_ = make_safe(output);
  auto input_mask_ = make_xsafe(input_mask), output_mask_ = make_xsafe(output_mask);

  // perform checks on input and output image
  if (input->ndim != 2 && input->ndim != 3){
    PyErr_Format(PyExc_TypeError, "`%s' only processes 2D or 3D arrays", Py_TYPE(self)->tp_name);
    process.print_usage();
    return 0;
  }
  if (output->ndim != input->ndim){
    PyErr_Format(PyExc_TypeError, "`%s' processes only input and output arrays with the same number of dimensions", Py_TYPE(self)->tp_name);
    process.print_usage();
    return 0;
  }
  if (output->type_num != NPY_FLOAT64){
    PyErr_Format(PyExc_TypeError, "`%s' processes only output arrays of type float", Py_TYPE(self)->tp_name);
    process.print_usage();
    return 0;
  }

  if (input_mask && output_mask){
    if (input_mask->ndim != input->ndim || output_mask->ndim != output->ndim){
      PyErr_Format(PyExc_TypeError, "`%s' masks must have the same shape as the input matrix", Py_TYPE(self)->tp_name);
      process.print_usage();
      return 0;
    }
    if (input_mask->type_num != NPY_BOOL || output_mask->type_num != NPY_BOOL){
      PyErr_Format(PyExc_TypeError, "`%s' masks must be of boolean type", Py_TYPE(self)->tp_name);
      process.print_usage();
      return 0;
    }
  }

  // finally, process the data
  switch (input->type_num){
    case NPY_UINT8:   return process_inner<uint8_t>(self, input, input_mask, output, output_mask, center);
    case NPY_UINT16:  return process_inner<uint16_t>(self, input, input_mask, output, output_mask, center);
    case NPY_FLOAT64: return process_inner<double>(self, input, input_mask, output, output_mask, center);
    default:
      PyErr_Format(PyExc_TypeError, "`%s' input array of type %s are currently not supported", Py_TYPE(self)->tp_name, PyBlitzArray_TypenumAsString(input->type_num));
      process.print_usage();
      return 0;
  }

  CATCH("cannot process image", 0)
}

static PyMethodDef PyBobIpBaseGeomNorm_methods[] = {
  {
    process.name(),
    (PyCFunction)PyBobIpBaseGeomNorm_process,
    METH_VARARGS|METH_KEYWORDS,
    process.doc()
  },
  {0} /* Sentinel */
};


/******************************************************************/
/************ Module Section **************************************/
/******************************************************************/

// Define the GeomNorm type struct; will be initialized later
PyTypeObject PyBobIpBaseGeomNormType = {
  PyVarObject_HEAD_INIT(0,0)
  0
};

bool init_BobIpBaseGeomNorm(PyObject* module)
{
  // initialize the Gabor wavelet type struct
  PyBobIpBaseGeomNormType.tp_name = GeomNorm_doc.name();
  PyBobIpBaseGeomNormType.tp_basicsize = sizeof(PyBobIpBaseGeomNormObject);
  PyBobIpBaseGeomNormType.tp_flags = Py_TPFLAGS_DEFAULT;
  PyBobIpBaseGeomNormType.tp_doc = GeomNorm_doc.doc();

  // set the functions
  PyBobIpBaseGeomNormType.tp_new = PyType_GenericNew;
  PyBobIpBaseGeomNormType.tp_init = reinterpret_cast<initproc>(PyBobIpBaseGeomNorm_init);
  PyBobIpBaseGeomNormType.tp_dealloc = reinterpret_cast<destructor>(PyBobIpBaseGeomNorm_delete);
  PyBobIpBaseGeomNormType.tp_richcompare = reinterpret_cast<richcmpfunc>(PyBobIpBaseGeomNorm_RichCompare);
  PyBobIpBaseGeomNormType.tp_methods = PyBobIpBaseGeomNorm_methods;
  PyBobIpBaseGeomNormType.tp_getset = PyBobIpBaseGeomNorm_getseters;
  PyBobIpBaseGeomNormType.tp_call = reinterpret_cast<ternaryfunc>(PyBobIpBaseGeomNorm_process);

  // check that everything is fine
  if (PyType_Ready(&PyBobIpBaseGeomNormType) < 0)
    return false;

  // add the type to the module
  Py_INCREF(&PyBobIpBaseGeomNormType);
  return PyModule_AddObject(module, "GeomNorm", (PyObject*)&PyBobIpBaseGeomNormType) >= 0;
}

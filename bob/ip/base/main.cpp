/**
 * @author Laurent El Shafey <Laurent.El-Shafey@idiap.ch>
 * @date Mon Apr 14 20:45:21 CEST 2014
 *
 * @brief Bindings to bob::ip routines
 */

#ifdef NO_IMPORT_ARRAY
#undef NO_IMPORT_ARRAY
#endif
#include "main.h"

static PyMethodDef module_methods[] = {
  {
    s_scale.name(),
    (PyCFunction)PyBobIpBase_scale,
    METH_VARARGS|METH_KEYWORDS,
    s_scale.doc()
  },
  {
    s_getScaledOutputShape.name(),
    (PyCFunction)PyBobIpBase_getScaledOutputShape,
    METH_VARARGS|METH_KEYWORDS,
    s_getScaledOutputShape.doc()
  },
  {
    s_rotate.name(),
    (PyCFunction)PyBobIpBase_rotate,
    METH_VARARGS|METH_KEYWORDS,
    s_rotate.doc()
  },
  {
    s_getRotatedOutputShape.name(),
    (PyCFunction)PyBobIpBase_getRotatedOutputShape,
    METH_VARARGS|METH_KEYWORDS,
    s_getRotatedOutputShape.doc()
  },
  {
    s_maxRectInMask.name(),
    (PyCFunction)PyBobIpBase_maxRectInMask,
    METH_VARARGS|METH_KEYWORDS,
    s_maxRectInMask.doc()
  },
  {
    s_extrapolateMask.name(),
    (PyCFunction)PyBobIpBase_extrapolateMask,
    METH_VARARGS|METH_KEYWORDS,
    s_extrapolateMask.doc()
  },
  {
    s_block.name(),
    (PyCFunction)PyBobIpBase_block,
    METH_VARARGS|METH_KEYWORDS,
    s_block.doc()
  },
  {
    s_blockOutputShape.name(),
    (PyCFunction)PyBobIpBase_blockOutputShape,
    METH_VARARGS|METH_KEYWORDS,
    s_blockOutputShape.doc()
  },
  {
    s_lbphs.name(),
    (PyCFunction)PyBobIpBase_lbphs,
    METH_VARARGS|METH_KEYWORDS,
    s_lbphs.doc()
  },
  {
    s_lbphsOutputShape.name(),
    (PyCFunction)PyBobIpBase_lbphsOutputShape,
    METH_VARARGS|METH_KEYWORDS,
    s_lbphsOutputShape.doc()
  },
  {
    s_integral.name(),
    (PyCFunction)PyBobIpBase_integral,
    METH_VARARGS|METH_KEYWORDS,
    s_integral.doc()
  },
  {
    s_histogram.name(),
    (PyCFunction)PyBobIpBase_histogram,
    METH_VARARGS|METH_KEYWORDS,
    s_histogram.doc()
  },
  {
    s_histogramEqualization.name(),
    (PyCFunction)PyBobIpBase_histogramEqualization,
    METH_VARARGS|METH_KEYWORDS,
    s_histogramEqualization.doc()
  },
  {
    s_zigzag.name(),
    (PyCFunction)PyBobIpBase_zigzag,
    METH_VARARGS|METH_KEYWORDS,
    s_zigzag.doc()
  },
  {0}  // Sentinel
};


PyDoc_STRVAR(module_docstr, "Bob Image Processing Base Routines");

#if PY_VERSION_HEX >= 0x03000000
static PyModuleDef module_definition = {
  PyModuleDef_HEAD_INIT,
  BOB_EXT_MODULE_NAME,
  module_docstr,
  -1,
  module_methods,
  0, 0, 0, 0
};
#endif

static PyObject* create_module (void) {

# if PY_VERSION_HEX >= 0x03000000
  PyObject* module = PyModule_Create(&module_definition);
# else
  PyObject* module = Py_InitModule3(BOB_EXT_MODULE_NAME, module_methods, module_docstr);
# endif
  if (!module) return 0;
  auto module_ = make_safe(module); ///< protects against early returns

  if (PyModule_AddStringConstant(module, "__version__", BOB_EXT_MODULE_VERSION) < 0) return 0;
  if (!init_BobIpBaseGeomNorm(module)) return 0;
  if (!init_BobIpBaseFaceEyesNorm(module)) return 0;
  if (!init_BobIpBaseLBP(module)) return 0;
  if (!init_BobIpBaseLBPTop(module)) return 0;
  if (!init_BobIpBaseDCTFeatures(module)) return 0;
  if (!init_BobIpBaseTanTriggs(module)) return 0;

  /* imports bob.blitz C-API + dependencies */
  if (import_bob_blitz() < 0) return 0;
  if (import_bob_io_base() < 0) return 0;
  if (import_bob_sp() < 0) return 0;

  Py_INCREF(module);
  return module;

}

PyMODINIT_FUNC BOB_EXT_ENTRY_NAME (void) {
# if PY_VERSION_HEX >= 0x03000000
  return
# endif
    create_module();
}

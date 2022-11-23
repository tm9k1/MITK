if(MITK_WRAP_PYTHON_ENABLED)
  mitkFunctionCheckCompilerFlags("-Wno-cpp" CMAKE_CXX_FLAGS)

  mitk_create_module(PythonService
    DEPENDS PRIVATE MitkPython
    PACKAGE_DEPENDS PRIVATE Python3|NumPy
    AUTOLOAD_WITH MitkPython
  )

  if(TARGET ${MODULE_TARGET})
    configure_file(PythonPath.h.in "${CMAKE_CURRENT_BINARY_DIR}/PythonPath.h" @ONLY)
  endif()
endif()

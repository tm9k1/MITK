if(MITK_WRAP_PYTHON_ENABLED)
  mitk_create_module(
    DEPENDS MitkCore
    PACKAGE_DEPENDS PUBLIC Python3|Python
  )

  if(TARGET ${MODULE_TARGET} AND BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()

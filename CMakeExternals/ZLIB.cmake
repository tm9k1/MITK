set(proj ZLIB)
set(proj_DEPENDENCIES "")

if(MITK_USE_${proj})
  set(${proj}_DEPENDS ${proj})

  if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
    message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to non-existing directory!")
  endif()

  find_package(ZLIB QUIET)

  if(NOT DEFINED ${proj}_DIR AND NOT ZLIB_FOUND)
    ExternalProject_Add(${proj}
      GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
      GIT_TAG d54e3769be0c522015b784eca2af258b1c026107 # 2.2.1 (2024-07-02)
      CMAKE_ARGS ${ep_common_args}
        -DZLIB_COMPAT:BOOL=ON
        -DZLIB_ENABLE_TESTS:BOOL=OFF
      CMAKE_CACHE_ARGS ${ep_common_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(${proj}_DIR "${ep_prefix}/lib/cmake/ZLIB")
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

set(proj msgpack-cxx)
set(proj_DEPENDENCIES Boost)

if(MITK_USE_${proj})
  set(${proj}_DEPENDS ${proj})

  if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
    message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to non-existing directory!")
  endif()

  if(NOT DEFINED ${proj}_DIR)
    ExternalProject_Add(${proj}
      GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
      GIT_TAG 44c0f705c9a60217d7e07de844fb13ce4c1c1e6e # cpp-6.1.1 (2024-04-02)
      CMAKE_ARGS ${ep_common_args}
      CMAKE_CACHE_ARGS ${ep_common_cache_args}
        "-DBoost_DIR:PATH=${Boost_DIR}"
        -DMSGPACK_BUILD_DOCS:BOOL=OFF
        -DMSGPACK_CXX17:BOOL=ON
      CMAKE_CACHE_DEFAULT_ARGS ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(${proj}_DIR "${ep_prefix}/lib/cmake/${proj}")
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

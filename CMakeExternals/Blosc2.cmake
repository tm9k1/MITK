set(proj Blosc2)
set(proj_DEPENDENCIES lz4 ZLIB)

if(MITK_USE_${proj})
  set(${proj}_DEPENDS ${proj})

  if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
    message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to non-existing directory!")
  endif()

  if(NOT DEFINED ${proj}_DIR)
    ExternalProject_Add(${proj}
      GIT_REPOSITORY https://github.com/Blosc/c-blosc2.git
      GIT_TAG 7424ecfb6ccabfbf865f34c9019559efeb8a39e0 # v2.15.0 (2024-06-20)
      CMAKE_ARGS ${ep_common_args}
      CMAKE_CACHE_ARGS ${ep_common_cache_args}
        -DBUILD_STATIC:BOOL=OFF
        -DBUILD_TESTS:BOOL=OFF
        -DBUILD_FUZZERS:BOOL=OFF
        -DBUILD_BENCHMARKS:BOOL=OFF
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_PLUGINS:BOOL=OFF
        -DPREFER_EXTERNAL_LZ4:BOOL=ON
        "-DLZ4_DIR:PATH=${lz4_DIR}"
        -DPREFER_EXTERNAL_ZLIB:BOOL=ON
        "-DZLIB_DIR:PATH=${ZLIB_DIR}"
      CMAKE_CACHE_DEFAULT_ARGS ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    if(WIN32)
      set(${proj}_DIR "${ep_prefix}/cmake")
    else()
      set(${proj}_DIR "${ep_prefix}/lib/cmake/Blosc2")
    endif()
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

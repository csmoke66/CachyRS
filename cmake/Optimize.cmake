# Shared compiler/linker settings for CachyRS and CachyRS-Private.
# An unset CMAKE_BUILD_TYPE compiles with no -O flags; default to Release.

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CACHYRS_ENABLE_LTO ON CACHE BOOL "Full link-time optimization")
set(CACHYRS_NATIVE ON CACHE BOOL "Optimize for this CPU (-march=native)")

# Shared objects default to allowing undefined imports, which then fail
# silently when rs2client/dlopen can't resolve them. Fail the link instead.
function(cachyrs_no_undefined target)
  if(NOT TARGET ${target})
    message(WARNING "cachyrs_no_undefined: no target named ${target}")
    return()
  endif()

  get_target_property(_cachyrs_type ${target} TYPE)
  if(NOT _cachyrs_type STREQUAL "SHARED_LIBRARY" AND NOT _cachyrs_type STREQUAL "MODULE_LIBRARY")
    return()
  endif()

  target_link_options(${target} PRIVATE "LINKER:--no-undefined")
endfunction()

function(cachyrs_warnings target)
  if(NOT TARGET ${target})
    message(WARNING "cachyrs_warnings: no target named ${target}")
    return()
  endif()

  target_compile_options(${target} PRIVATE
    -Wall
    -Wextra
    -Wshadow
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wdelete-incomplete
  )
endfunction()

function(cachyrs_optimize target)
  if(NOT TARGET ${target})
    message(WARNING "cachyrs_optimize: no target named ${target}")
    return()
  endif()

  cachyrs_warnings(${target})
  target_compile_options(${target} PRIVATE
    $<$<NOT:$<CONFIG:Debug>>:-O3 -ffunction-sections -fdata-sections -fomit-frame-pointer -fvisibility-inlines-hidden>
  )
  if(CACHYRS_NATIVE)
    target_compile_options(${target} PRIVATE
      $<$<NOT:$<CONFIG:Debug>>:-march=native -mtune=native>
    )
  endif()
  target_link_options(${target} PRIVATE
    $<$<NOT:$<CONFIG:Debug>>:-Wl,--gc-sections>
  )
  if(CACHYRS_ENABLE_LTO)
    set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
  endif()
  cachyrs_no_undefined(${target})
endfunction()

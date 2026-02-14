
include(CMakeParseArguments)

function(add_swift_library library)
  set(options)
  set(single_value_options MODULE_NAME;MODULE_LINK_NAME;MODULE_PATH;MODULE_CACHE_PATH;OUTPUT;TARGET)
  set(multiple_value_options SOURCES;SWIFT_FLAGS;CFLAGS;DEPENDS)

  cmake_parse_arguments(ASL "${options}" "${single_value_options}" "${multiple_value_options}" ${ARGN})

  set(flags ${CMAKE_SWIFT_FLAGS})

  list(APPEND flags -emit-library)

  if(ASL_TARGET)
    list(APPEND FLAGS -target;${ASL_TARGET})
  endif()
  if(ASL_MODULE_NAME)
    list(APPEND flags -module-name;${ASL_MODULE_NAME})
  endif()
  if(ASL_MODULE_LINK_NAME)
    list(APPEND flags -module-link-name;${ASL_MODULE_LINK_NAME})
  endif()
  if(ASL_MODULE_PATH)
    list(APPEND flags -emit-module-path;${ASL_MODULE_PATH})
  endif()
  if(ASL_MODULE_CACHE_PATH)
    list(APPEND flags -module-cache-path;${ASL_MODULE_CACHE_PATH})
  endif()
  if(ASL_SWIFT_FLAGS)
    foreach(flag ${ASL_SWIFT_FLAGS})
      list(APPEND flags ${flag})
    endforeach()
  endif()
  if(ASL_CFLAGS)
    foreach(flag ${ASL_CFLAGS})
      list(APPEND flags -Xcc;${flag})
    endforeach()
  endif()

  # FIXME: We shouldn't /have/ to build things in a single process.
  # <rdar://problem/15972329>
  list(APPEND flags -force-single-frontend-invocation)

  set(sources)
  foreach(source ${ASL_SOURCES})
    get_filename_component(location ${source} PATH)
    if(IS_ABSOLUTE ${location})
      list(APPEND sources ${source})
    else()
      list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/${source})
    endif()
  endforeach()

  get_filename_component(module_directory ${ASL_MODULE_PATH} DIRECTORY)

  add_custom_command(OUTPUT
                       ${ASL_OUTPUT}
                       ${ASL_MODULE_PATH}
                       ${module_directory}/${ASL_MODULE_NAME}.swiftdoc
                     DEPENDS
                       ${ASL_SOURCES}
                       ${CMAKE_SWIFT_COMPILER}
                       ${ASL_DEPENDS}
                     COMMAND
                       ${CMAKE_COMMAND} -E make_directory ${module_directory}
                     COMMAND
                       ${CMAKE_SWIFT_COMPILER} ${flags} -c ${sources} -o ${ASL_OUTPUT})
  add_custom_target(${library}
                    DEPENDS
                       ${ASL_OUTPUT}
                       ${ASL_MODULE_PATH}
                       ${module_directory}/${ASL_MODULE_NAME}.swiftdoc)
endfunction()

# Returns the current achitecture name in a variable
#
# Usage:
#   get_swift_host_arch(result_var_name)
#
# If the current architecture is supported by Swift, sets ${result_var_name}
# with the sanitized host architecture name derived from CMAKE_SYSTEM_PROCESSOR.
function(get_swift_host_arch result_var_name)
  if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    set("${result_var_name}" "x86_64" PARENT_SCOPE)
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
    set("${result_var_name}" "aarch64" PARENT_SCOPE)
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "ppc64")
    set("${result_var_name}" "powerpc64" PARENT_SCOPE)
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "ppc64le")
    set("${result_var_name}" "powerpc64le" PARENT_SCOPE)
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "s390x")
    set("${result_var_name}" "s390x" PARENT_SCOPE)
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv6l")
    set("${result_var_name}" "armv6" PARENT_SCOPE)
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv7l")
    set("${result_var_name}" "armv7" PARENT_SCOPE)
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
    set("${result_var_name}" "x86_64" PARENT_SCOPE)
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "IA64")
    set("${result_var_name}" "itanium" PARENT_SCOPE)
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86")
    set("${result_var_name}" "i686" PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Unrecognized architecture on host system: ${CMAKE_SYSTEM_PROCESSOR}")
  endif()
endfunction()

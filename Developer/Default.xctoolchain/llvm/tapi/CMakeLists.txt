set(TAPI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TAPI_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(TAPI_FULL_VERSION "2.0.0" CACHE STRING "Specify tapi version.")
message(STATUS "TAPI version: ${TAPI_FULL_VERSION}")

string(REGEX REPLACE "([0-9]+)\\.[0-9]+(\\.[0-9]+)*" "\\1" TAPI_VERSION_MAJOR
  ${TAPI_FULL_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9]+)(\\.[0-9]+)*" "\\1" TAPI_VERSION_MINOR
  ${TAPI_FULL_VERSION})

if(${TAPI_FULL_VERSION} MATCHES "[0-9]+\\.[0-9]+(\\.[0-9]+)+")
  string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)(\\.[0-9]+)*" "\\1" TAPI_VERSION_PATCH
         ${TAPI_FULL_VERSION})
else()
  set(TAPI_VERSION_PATCH 0)
endif()
set(TAPI_VERSION "${TAPI_VERSION_MAJOR}.${TAPI_VERSION_MINOR}.${TAPI_VERSION_PATCH}")

set(TAPI_REPOSITORY_STRING "" CACHE STRING
  "Vendor-specific text for showing the repository the source is taken from.")

if(TAPI_REPOSITORY_STRING)
  add_definitions(-DTAPI_REPOSITORY_STRING="${TAPI_REPOSITORY_STRING}")
endif()

set(TAPI_VENDOR ${PACKAGE_VENDOR} CACHE STRING
  "Vendor-specific text for showing with version information.")

if (TAPI_VENDOR)
  add_definitions( -DTAPI_VENDOR="${TAPI_VENDOR} ")
endif()

if(DEFINED ENV{RC_SUPPORTED_ARCHS})
  string(REPLACE " " ";" TAPI_SUPPORTED_ARCHS $ENV{RC_SUPPORTED_ARCHS})
elseif(NOT DEFINED TAPI_SUPPORTED_ARCHS)
  set(TAPI_SUPPORTED_ARCHS i386 x86_64 x86_64h armv4t armv6 armv5 armv7 armv7s armv7k armv6m armv7m armv7em arm64)
endif()
message(STATUS "Supported Architectures: ${TAPI_SUPPORTED_ARCHS}")

set(KNOWN_ARCHS i386 x86_64 x86_64h armv4t armv6 armv5 armv7 armv7s armv7k armv6m armv7m armv7em arm64)

set (CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/include/tapi/Core/ArchitectureConfig.h)
file(WRITE ${CONFIG_FILE} "#ifndef TAPI_CORE_ARCHITECTURE_CONFIG_H\n")
file(APPEND ${CONFIG_FILE} "#define TAPI_CORE_ARCHITECTURE_CONFIG_H\n\n")

foreach(arch ${TAPI_SUPPORTED_ARCHS})
  if(arch IN_LIST KNOWN_ARCHS)
    string(TOUPPER ${arch} ARCH)
    file(APPEND ${CONFIG_FILE} "#define SUPPORT_ARCH_${ARCH} 1\n")
  else()
    message(FATAL_ERROR "unknown architecture: ${arch}")
  endif()
endforeach(arch)

file(APPEND ${CONFIG_FILE} "\n#endif // TAPI_CORE_ARCHITECTURE_CONFIG_H\n")

# Configure the Version.inc file.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/tapi/Version.inc.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/tapi/Version.inc)

# Check if the linker support -no_inits.
llvm_check_linker_flag(CXX "-Wl,-no_inits" LINKER_SUPPORTS_NO_INITS)

# MARZIPAN RENAME: We might need to update this. This will also require an
# update of the linker on the CI bots.
# Check if the linker has iosmac support.
llvm_check_linker_flag(CXX "-Wl,-iosmac_version_min,13.0" LINKER_SUPPORTS_IOSMAC)
# MARZIPAN RENAME

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
  include_directories(${CMAKE_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS)
else()
  # Check if i386 is supported.
  SET(CMAKE_OLD_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
  SET(CMAKE_OLD_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
  SET(CMAKE_REQUIRED_FLAGS "-arch i386")
  SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-arch_errors_fatal -Wl,-framework,Foundation")
  check_c_compiler_flag("" SDK_SUPPORTS_I386)
  SET(CMAKE_REQUIRED_FLAGS ${CMAKE_OLD_REQUIRED_FLAGS})
  SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_OLD_LINKER_FLAGS})
endif()

macro(add_tapi_library name)
  cmake_parse_arguments(ARG
    "SHARED"
    ""
    "ADDITIONAL_HEADERS"
    ${ARGN})
  set(srcs)
  if(MSVC_IDE OR XCODE)
    # Add public headers
    file(RELATIVE_PATH lib_path
      ${TAPI_SOURCE_DIR}/lib/
      ${CMAKE_CURRENT_SOURCE_DIR}
    )
    if(NOT lib_path MATCHES "^[.][.]")
      file( GLOB_RECURSE headers
        ${TAPI_SOURCE_DIR}/include/tapi/${lib_path}/*.h
        ${CLANG_SOURCE_DIR}/include/tapi/${lib_path}/*.def
      )
      set_source_files_properties(${headers} PROPERTIES HEADER_FILE_ONLY ON)

      file( GLOB_RECURSE tds
        ${TAPI_SOURCE_DIR}/include/tapi/${lib_path}/*.td
      )
      source_group("TableGen descriptions" FILES ${tds})
      set_source_files_properties(${tds}} PROPERTIES HEADER_FILE_ONLY ON)

      if(headers OR tds)
        set(srcs ${headers} ${tds})
      endif()
    endif()
  endif(MSVC_IDE OR XCODE)
  if(srcs OR ARG_ADDITIONAL_HEADERS)
    set(srcs
      ADDITIONAL_HEADERS
      ${srcs}
      ${ARG_ADDITIONAL_HEADERS} # It may contain unparsed unknown args.
      )
  endif()
  if(ARG_SHARED)
    set(ARG_ENABLE_SHARED SHARED)
  endif()
  llvm_add_library(${name} ${ARG_ENABLE_SHARED} ${ARG_UNPARSED_ARGUMENTS} ${srcs})

  if(TARGET ${name})
    target_link_libraries(${name} INTERFACE ${LLVM_COMMON_LIBS})

    if (NOT LLVM_INSTALL_TOOLCHAIN_ONLY)
      if(${name} IN_LIST LLVM_DISTRIBUTION_COMPONENTS OR
          NOT LLVM_DISTRIBUTION_COMPONENTS)
        set(export_to_tapitargets EXPORT TapiTargets)
      endif()

      install(TARGETS ${name}
        COMPONENT ${name}
        ${export_to_tapitargets}
        LIBRARY DESTINATION lib${LLVM_LIBDIR_SUFFIX}
        ARCHIVE DESTINATION lib${LLVM_LIBDIR_SUFFIX}
        RUNTIME DESTINATION bin)

      if (${ARG_SHARED} AND NOT CMAKE_CONFIGURATION_TYPES)
        add_llvm_install_targets(install-${name}
                                 DEPENDS ${name}
                                 COMPONENT ${name})
      endif()
    endif()

    set_property(GLOBAL APPEND PROPERTY TAPI_EXPORTS ${name})
  else()
    # Add empty "phony" target
    add_custom_target(${name})
  endif()

  set_target_properties(${name} PROPERTIES FOLDER "Tapi libraries")
endmacro(add_tapi_library)

macro(add_tapi_executable name)
  add_llvm_executable(${name} ${ARGN})
  set_target_properties(${name} PROPERTIES FOLDER "Tapi executables")
  if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_options(${name} PRIVATE "-fuse-ld=lld")
  else()
    target_link_options(${name} PRIVATE "-Wl,-no_exported_symbols")
  endif()
endmacro(add_tapi_executable)

function(tapi_clang_tablegen)
  cmake_parse_arguments(CTG "" "SOURCE;TARGET" "" ${ARGN})

  if( NOT CTG_SOURCE )
    message(FATAL_ERROR "SOURCE source-file required by clang_tablegen")
  endif()

  set( LLVM_TARGET_DEFINITIONS ${CTG_SOURCE} )
  tablegen(CLANG ${CTG_UNPARSED_ARGUMENTS})

  if(CTG_TARGET)
    add_public_tablegen_target(${CTG_TARGET})
    set_target_properties( ${CTG_TARGET} PROPERTIES FOLDER "Tapi tablegenning")
    set_property(GLOBAL APPEND PROPERTY TAPI_TABLEGEN_TARGETS ${CTG_TARGET})
  endif()
endfunction(tapi_clang_tablegen)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/include
  ${CMAKE_CURRENT_BINARY_DIR}/tools/clang/include)

option(TAPI_INCLUDE_TESTS "Generate build targets for the TAPI unit tests."
       ${LLVM_INCLUDE_TESTS})
option(TAPI_INCLUDE_DOCS "Generate build targets for the TAPI docs."
      ${LLVM_INCLUDE_DOCS})

# Include must go first.
add_subdirectory(include)
# All targets below may depend on all tablegen'd files.
get_property(TAPI_TABLEGEN_TARGETS GLOBAL PROPERTY TAPI_TABLEGEN_TARGETS)
list(APPEND LLVM_COMMON_DEPENDS ${TAPI_TABLEGEN_TARGETS})
add_subdirectory(lib)
add_subdirectory(tools)
if( TAPI_INCLUDE_DOCS )
  add_subdirectory(docs)
endif()
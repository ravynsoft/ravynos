add_library(host_ld_libparsers STATIC)
target_sources(host_ld_libparsers PRIVATE
    archive_file.cpp
    generic_dylib_file.cpp
    lto_file.cpp
    macho_dylib_file.cpp
    macho_relocatable_file.cpp
    opaque_section_file.cpp
    textstub_dylib_file.cpp
)
# This gets built before xnu headers, so we have to do some trickery. Also we
# want a tool built for the host OS but targeting ravynOS, which means complicated
# header management.
file(MAKE_DIRECTORY ${TOOLCHAIN}/usr/include/mach)
file(COPY ${CMAKE_SOURCE_DIR}/Developer/cctools/include/foreign/mach/mach_host.h
    DESTINATION ${TOOLCHAIN}/usr/include/mach/)
file(TOUCH ${TOOLCHAIN}/usr/include/CrashReporterClient.h)
add_dependencies(host_ld_libparsers host_clang)
target_compile_definitions(host_ld_libparsers PRIVATE 
    __DARWIN_UNIX03
    _DARWIN_C_SOURCE
    io_master_t=mach_port_t
)
target_include_directories(host_ld_libparsers PRIVATE
    ..
    ../../abstraction
    ../../../../include
    ../../../../include/foreign
    ../../../../../llvm-project/tapi/include
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${CMAKE_SOURCE_DIR}/Libraries/libplatform/private
    ${CMAKE_SOURCE_DIR}/Libraries/dyld/include
    ${CMAKE_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
    ${CMAKE_SOURCE_DIR}/Kernel/xnu/osfmk
    ${TOOLCHAIN}/usr/include
)
set_property(TARGET host_ld_libparsers PROPERTY CXX_STANDARD 17)
suppress_all_warnings(host_ld_libparsers)
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    target_compile_options(host_ld_libparsers PRIVATE -w -fblocks)
endif()

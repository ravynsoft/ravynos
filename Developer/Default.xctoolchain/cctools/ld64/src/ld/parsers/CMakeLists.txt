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
add_dependencies(host_ld_libparsers host_clang)
target_compile_definitions(host_ld_libparsers PRIVATE 
    __DARWIN_UNIX03 __APPLE_CC__
    _DARWIN_C_SOURCE
    io_master_t=mach_port_t
)
target_include_directories(host_ld_libparsers PRIVATE
    ..
    ../../abstraction
    ${DEVEL}/Default.xctoolchain/llvm/tapi/include
    ${ROOT_BINARY_DIR}/Developer/Default.xctoolchain
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
set_property(TARGET host_ld_libparsers PROPERTY CXX_STANDARD 17)
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    target_compile_options(host_ld_libparsers PRIVATE
        -stdlib=libc++ -w -fblocks
    )
endif()

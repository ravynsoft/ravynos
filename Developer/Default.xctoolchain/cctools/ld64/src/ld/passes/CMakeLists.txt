add_library(host_ld_libpasses STATIC)
target_sources(host_ld_libpasses PRIVATE
    branch_island.cpp
    branch_shim.cpp
    compact_unwind.cpp
    dtrace_dof.cpp
    dylibs.cpp
    got.cpp
    huge.cpp
    objc.cpp
    objc_constants.cpp
    order.cpp
    tlvp.cpp
    stubs/stubs.cpp
    bitcode_bundle.cpp
    code_dedup.cpp
    thread_starts.cpp
    inits.cpp
)

add_dependencies(host_ld_libpasses host_clang)
target_compile_definitions(host_ld_libpasses PRIVATE __DARWIN_UNIX03 _DARWIN_C_SOURCE)
target_include_directories(host_ld_libpasses PRIVATE
    ${TOOLCHAIN}/usr/include
    ${ROOT_SOURCE_DIR}/Libraries/libmalloc/include
    ../../abstraction
    ../../../../include
    ../../../../include/foreign
    ..
    ../..
    ../parsers
    ../../..
    ../../../../../llvm/tapi/include
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
)
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    target_compile_options(host_ld_libpasses PRIVATE
        -stdlib=libc++ -fblocks
    )
endif()
set_property(TARGET host_ld_libpasses PROPERTY CXX_STANDARD 17)

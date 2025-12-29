set(CMAKE_INSTALL_PREFIX "")

add_library(libmalloc)
set_property(TARGET libmalloc PROPERTY OUTPUT_NAME "system_malloc")

file(GLOB_RECURSE srcfiles LIST_DIRECTORIES false *.c)
file(GLOB_RECURSE interface LIST_DIRECTORIES false include/*.h)
file(GLOB_RECURSE privhdrs LIST_DIRECTORIES false private/*.h internal/*.h)

target_sources(libmalloc PRIVATE ${srcfiles})
target_sources(libmalloc INTERFACE ${interface})
target_sources(libmalloc PRIVATE ${privhdrs})

target_include_directories(libmalloc PRIVATE
    ${RAVYN_SDKROOT}/usr/include
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/PrivateHeaders
    ../libplatform/private ../libplatform/include ../libpthread
    ../../Kernel/xnu/libsyscall/wrappers
    ../../Kernel/xnu/libsyscall/mach
    ../../Kernel/xnu/libsyscall
    ../dyld/include
    internal include private src
)

target_compile_definitions(libmalloc PRIVATE
    TARGET_OS_BRIDGE=0 DARWINTEST=1
)

target_compile_options(libmalloc PRIVATE
    --sysroot=${RAVYN_SDKROOT}
    -mmacosx-version-min=${CMAKE_MACOSX_MIN_VERSION}
)

add_dependencies(libmalloc ravynOS_sdk)

# add_custom_command(TARGET libfirehose_kernel
#     POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory
#         ${CMAKE_BINARY_DIR}/sysroot/usr/local/lib/kernel
#     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libfirehose_kernel>
#         ${CMAKE_BINARY_DIR}/sysroot/usr/local/lib/kernel
# )
set(CMAKE_INSTALL_PREFIX "")

add_library(libpthread)
set_property(TARGET libpthread PROPERTY OUTPUT_NAME "system_pthread")

file(GLOB_RECURSE srcfiles LIST_DIRECTORIES false src/*.c src/*.s)
file(GLOB_RECURSE interface LIST_DIRECTORIES false include/*.h)
file(GLOB_RECURSE privhdrs LIST_DIRECTORIES false private/*.h internal/*.h)

target_sources(libpthread PRIVATE ${srcfiles})
target_sources(libpthread INTERFACE ${interface})
target_sources(libpthread PRIVATE ${privhdrs})

target_include_directories(libpthread PRIVATE
    . private private/pthread src/resolver
    ../libplatform/include ../libplatform/private
    ../libmalloc/include
    ../../Kernel/xnu/libsyscall/mach
    ../../Kernel/xnu/libsyscall
    ${RAVYN_SDKROOT}/usr/include
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Headers
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/PrivateHeaders
)

# target_compile_definitions(libpthread PRIVATE

# )

target_compile_options(libpthread PRIVATE
    --sysroot=${RAVYN_SDKROOT}
    -mmacosx-version-min=${CMAKE_MACOSX_MIN_VERSION}
)

add_dependencies(libpthread ravynOS_sdk libplatform)

# add_custom_command(TARGET libfirehose_kernel
#     POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory
#         ${CMAKE_BINARY_DIR}/sysroot/usr/local/lib/kernel
#     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libfirehose_kernel>
#         ${CMAKE_BINARY_DIR}/sysroot/usr/local/lib/kernel
# )
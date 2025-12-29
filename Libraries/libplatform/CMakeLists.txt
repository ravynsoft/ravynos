set(CMAKE_INSTALL_PREFIX "")

add_library(libplatform)
set_property(TARGET libplatform PROPERTY OUTPUT_NAME "system_platform")

file(GLOB_RECURSE srcfiles LIST_DIRECTORIES false src/*.c src/*.s)
file(GLOB_RECURSE interface LIST_DIRECTORIES false include/*.h)
file(GLOB_RECURSE privhdrs LIST_DIRECTORIES false private/*.h internal/*.h)

target_sources(libplatform PRIVATE ${srcfiles})
target_sources(libplatform INTERFACE ${interface})
target_sources(libplatform PRIVATE ${privhdrs})

target_include_directories(libplatform PRIVATE
    internal include private src
    ${RAVYN_SDKROOT}/usr/include
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/PrivateHeaders
)

# target_compile_definitions(libplatform PRIVATE

# )

target_compile_options(libplatform PRIVATE
    --sysroot=${RAVYN_SDKROOT}
    -mmacosx-version-min=${CMAKE_MACOSX_MIN_VERSION}
)

add_dependencies(libplatform ravynOS_sdk)

# add_custom_command(TARGET libfirehose_kernel
#     POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory
#         ${CMAKE_BINARY_DIR}/sysroot/usr/local/lib/kernel
#     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libfirehose_kernel>
#         ${CMAKE_BINARY_DIR}/sysroot/usr/local/lib/kernel
# )
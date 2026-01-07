cmake_policy(SET CMP0114 NEW)

set(CMAKE_INSTALL_PREFIX "")

add_darwin_static_library(libfirehose_kernel)
set_property(TARGET libfirehose_kernel PROPERTY OUTPUT_NAME "firehose_kernel")
target_sources(libfirehose_kernel PRIVATE
    ${ROOT_SOURCE_DIR}/Libraries/libdispatch/src/firehose/firehose_buffer.c
)

target_compile_definitions(libfirehose_kernel PRIVATE
    KERNEL=1 DISPATCH_USE_DTRACE=0 OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1
    OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1 OS_ATOMIC_CONFIG_STARVATION_FREE_ONLY=0)

target_compile_options(libfirehose_kernel PRIVATE -mkernel -Wno-packed
    --sysroot=${RAVYN_SDKROOT}
    -I${CMAKE_SOURCE_DIR}/xnu/osfmk
    -I${CMAKE_SOURCE_DIR}/xnu/libkern
    -I${CMAKE_SOURCE_DIR}/xnu/libkern/firehose
    -I${CMAKE_SOURCE_DIR}/xnu/bsd
    -Wno-error=implicit-function-declaration
    -Wno-implicit-function-declaration
    -Wno-sign-conversion 
)
add_dependencies(libfirehose_kernel xnu_headers)

add_custom_command(TARGET libfirehose_kernel
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${RAVYN_SDKROOT}/usr/local/lib/kernel
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libfirehose_kernel>
        ${RAVYN_SDKROOT}/usr/local/lib/kernel
)


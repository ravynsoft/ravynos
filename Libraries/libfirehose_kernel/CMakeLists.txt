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

file(MAKE_DIRECTORY ${ROOT_BINARY_DIR}/sysroot/usr/include/mach/)
file(COPY ${CMAKE_SOURCE_DIR}/xnu/osfmk/mach/vm_statistics.h 
    DESTINATION ${ROOT_BINARY_DIR}/sysroot/usr/include/mach/)
target_compile_options(libfirehose_kernel PRIVATE -mkernel -Wno-packed
    --sysroot=${RAVYN_SDKROOT}
    -I${RAVYN_SDKROOT}/usr/include
    -I${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Headers
    -I${ROOT_SOURCE_DIR}/Libraries/libdispatch
    -I${ROOT_SOURCE_DIR}/Libraries/libdispatch/src/firehose
    -I${ROOT_SOURCE_DIR}/Libraries/libplatform/private/os
    -I${ROOT_SOURCE_DIR}/Libraries/libpthread
    -I${CMAKE_SOURCE_DIR}/xnu/osfmk
    -I${CMAKE_SOURCE_DIR}/xnu/osfmk/libsa
    -I${CMAKE_SOURCE_DIR}/xnu/libkern
    -I${CMAKE_SOURCE_DIR}/xnu/libkern/firehose
    -I${CMAKE_SOURCE_DIR}/xnu/bsd
    -Wno-error=implicit-function-declaration
    -Wno-implicit-function-declaration
    -Wno-sign-conversion 
    -mmacosx-version-min=${CMAKE_MACOSX_MIN_VERSION}
)
add_dependencies(libfirehose_kernel xnu_headers)
target_link_libraries(libfirehose_kernel PRIVATE AvailabilityHeaders xnu_private_headers)
target_link_libraries(libfirehose_kernel PRIVATE xnu_kernel_headers xnu_kernel_private_headers)
target_link_libraries(libfirehose_kernel PRIVATE libplatform_headers libplatform_private_headers)
target_link_libraries(libfirehose_kernel PRIVATE pthread_common_headers pthread_common_private_headers)

add_custom_command(TARGET libfirehose_kernel
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${ROOT_BINARY_DIR}/sysroot/usr/local/lib/kernel
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libfirehose_kernel>
        ${ROOT_BINARY_DIR}/sysroot/usr/local/lib/kernel
)
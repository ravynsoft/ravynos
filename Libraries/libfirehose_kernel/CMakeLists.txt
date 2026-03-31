cmake_policy(SET CMP0114 NEW)

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_C_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})
set(CMAKE_C_USING_LINKER_APPLE ${LD})
set(CMAKE_CXX_USING_LINKER_APPLE ${LD})
set(CMAKE_ASM_USING_LINKER_APPLE ${LD})
set(CMAKE_LINK_DEPENDS_USE_LINKER FALSE)

add_library(libfirehose_kernel STATIC)
set_target_properties(libfirehose_kernel PROPERTIES
    OUTPUT_NAME "firehose_kernel"
    PREFIX "lib"
)
target_sources(libfirehose_kernel PRIVATE
    ../Libsystem/libdispatch/src/firehose/firehose_buffer.c
    hack.s
)

target_compile_definitions(libfirehose_kernel PRIVATE
    KERNEL=1 DISPATCH_USE_DTRACE=0 OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1
    OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1 OS_ATOMIC_CONFIG_STARVATION_FREE_ONLY=0
)

target_compile_options(libfirehose_kernel PRIVATE 
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT}
    -mmacosx-version-min=10.15 -mkernel -Wno-packed
    -Wno-error=implicit-function-declaration
    -Wno-implicit-function-declaration
    -Wno-sign-conversion 
)

target_include_directories(libfirehose_kernel PRIVATE
    ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern/firehose
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ../Libsystem/private ${SDK_SOURCE_DIR}/usr/include
    ${RAVYN_SDKROOT}/usr/local/include
    ${RAVYN_SDKROOT}/usr/local/include/kernel
)

add_dependencies(libfirehose_kernel xnu_headers libplatform libkernel)
add_dependencies(xnu libfirehose_kernel compiler_rt)

add_custom_command(TARGET libfirehose_kernel
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${RAVYN_SDKROOT}/usr/local/lib/kernel
    COMMAND
        ${TOOLS}/libtool -static -o ${RAVYN_SDKROOT}/usr/local/lib/kernel/libfirehose_kernel.a
	-no_warning_for_no_symbols $<TARGET_FILE:libfirehose_kernel>
        -L${RAVYN_SDKROOT}/usr/local/lib/system -lplatform
        -L${RAVYN_SDKROOT}/usr/local/lib/kernel -lkernel
        -L${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib/system -lc++ -lc++abi
        ${CMAKE_CURRENT_BINARY_DIR}/../llvm_target/compiler-rt/lib/ravynos/libclang_rt.builtins-${CpuArch}.a
        ${CMAKE_CURRENT_BINARY_DIR}/../llvm_target/compiler-rt/lib/ravynos/libclang_rt.profile-${CpuArch}.a
)

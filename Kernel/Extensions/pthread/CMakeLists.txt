add_kext_bundle(pthread.kext INFO_PLIST pthread-Info.plist KERNEL_PRIVATE
    BUNDLE_IDENTIFIER com.ravynos.kext.pthread
    BUNDLE_VERSION ${PROD_VERSION}
    MAIN_FUNCTION pthread_start ANTIMAIN_FUNCTION pthread_stop)
set_property(TARGET pthread.kext PROPERTY OUTPUT_NAME pthread)

target_sources(pthread.kext PRIVATE
    kern_init.c
    kern_support.c
    kern_synch.c
)

target_compile_definitions(pthread.kext PRIVATE XNU_KERNEL_PRIVATE
    MACH_KERNEL_PRIVATE ABSOLUTETIME_SCALAR_TYPE NEEDS_SCHED_CALL_T
    __PTHREAD_EXPOSE_INTERNALS__ __STDC_HOSTED__=0)
target_compile_options(pthread.kext PRIVATE -Wno-sign-conversion
    -Wno-int-conversion -Wno-implicit-function-declaration
    -Wno-incompatible-library-redeclaration -Wno-undef-prefix
    -I${RAVYN_SDKROOT}/usr/include
    -I${RAVYN_SDKROOT}/usr/local/include/kernel
    -I${ROOT_BINARY_DIR}/Kernel/xnu/EXTERNAL_HEADERS
    -I${ROOT_BINARY_DIR}/Libraries/libpthread
    -I${ROOT_BINARY_DIR}/Libraries/libpthread/private
    -I${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers
    -I${ROOT_BINARY_DIR}/Kernel/xnu/bsd
)
add_dependencies(pthread.kext xnu libkmod)

install(TARGETS pthread.kext DESTINATION System/Library/Extensions COMPONENT BaseSystem)

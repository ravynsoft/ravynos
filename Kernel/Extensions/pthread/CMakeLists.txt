add_kext_bundle(pthread.kext INFO_PLIST Info.plist KERNEL_PRIVATE
    BUNDLE_IDENTIFIER com.apple.kec.pthread
    BUNDLE_VERSION ${PROD_VERSION}
    MAIN_FUNCTION pthread_start ANTIMAIN_FUNCTION pthread_stop)

target_sources(pthread.kext PRIVATE
    kern_init.c
    kern_support.c
    kern_synch.c
)

target_include_directories(pthread.kext PRIVATE
    kern .
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/private
    ${RAVYN_SDKROOT}/usr/local/include
    ${RAVYN_SDKROOT}/usr/local/include/kernel
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${ROOT_BINARY_DIR}/${CpuArch}
)

target_compile_options(pthread.kext PRIVATE
    --sysroot=${RAVYN_SDKROOT}
    -Wno-sign-conversion
    -Wno-int-conversion -Wno-implicit-function-declaration
    -Wno-incompatible-library-redeclaration -Wno-undef-prefix
)
target_compile_definitions(pthread.kext PRIVATE XNU_KERNEL_PRIVATE
    MACH_KERNEL_PRIVATE ABSOLUTETIME_SCALAR_TYPE NEEDS_SCHED_CALL_T
    __PTHREAD_EXPOSE_INTERNALS__ __STDC_HOSTED__=0
    _LIBC_NO_FEATURE_VERIFICATION)

target_link_options(pthread.kext PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN}
    -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib
    -L${RAVYN_SDKROOT}/usr/local/lib)

add_dependencies(pthread.kext xnu libkmod)

install(TARGETS pthread.kext DESTINATION ${SYSROOT_DIR}/System/Library/Extensions COMPONENT BaseSystem)

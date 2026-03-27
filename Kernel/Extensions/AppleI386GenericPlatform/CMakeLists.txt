add_kext_bundle(AppleI386GenericPlatform INFO_PLIST Info.plist KERNEL_PRIVATE
    BUNDLE_IDENTIFIER com.ravynos.kext.AppleI386GenericPlatform BUNDLE_VERSION ${PROD_VERSION})

target_sources(AppleI386GenericPlatform PRIVATE
    AppleI386CPU.cpp
    AppleI386PlatformExpert.cpp
)

target_include_directories(AppleI386GenericPlatform PRIVATE
    ${RAVYN_SDKROOT}/usr/local/include
    ${RAVYN_SDKROOT}/usr/local/include/kernel
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${ROOT_BINARY_DIR}/${CpuArch}
)

target_link_options(AppleI386GenericPlatform PRIVATE
    -mmacos-version-min=10.15
    -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib
    -L${RAVYN_SDKROOT}/usr/local/lib)

install(TARGETS AppleI386GenericPlatform DESTINATION ${SYSROOT_DIR}/System/Library/Extensions COMPONENT BaseSystem)

add_dependencies(AppleI386GenericPlatform kextBuild)


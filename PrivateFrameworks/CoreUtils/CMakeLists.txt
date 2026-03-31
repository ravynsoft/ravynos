add_framework_bundle(CoreUtils
    PRIVATE_FRAMEWORK
    VERSION A
    INFO_PLIST Info.plist
    HEADERS CoreUtils.h DebugServices.h CommonServices.h
)

target_include_directories(CoreUtils PRIVATE ..)
target_sources(CoreUtils PRIVATE CoreUtils.c DebugServices.c)
target_compile_options(CoreUtils PRIVATE -isysroot${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN})

add_dependencies(dns_services CoreUtils)
add_dependencies(liblog_mdnsresponder CoreUtils)


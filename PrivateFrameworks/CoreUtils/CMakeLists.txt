add_framework_bundle(CoreUtils VERSION A INFO_PLIST Info.plist)

target_include_directories(CoreUtils PRIVATE ..)
target_sources(CoreUtils PRIVATE CoreUtils.c DebugServices.c)
target_compile_options(CoreUtils PRIVATE -isysroot${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN})

add_custom_command(TARGET CoreUtils POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/System/Library/PrivateFrameworks 
    COMMAND cp -R ${CMAKE_CURRENT_BINARY_DIR}/CoreUtils.framework ${RAVYN_SDKROOT}/System/Library/PrivateFrameworks/
)

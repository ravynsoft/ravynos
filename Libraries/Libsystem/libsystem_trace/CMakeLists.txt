add_library(libsystem_trace SHARED log.c)
set_target_properties(libsystem_trace PROPERTIES SUFFIX ".dylib" PREFIX "libsystem_"
    OUTPUT_NAME "trace" NO_SONAME true)

target_compile_options(libsystem_trace PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN} --sysroot=${RAVYN_SDKROOT}
    -fblocks -DOS_OBJECT_USE_OBJC)

target_include_directories(libsystem_trace PRIVATE
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers
    ${RAVYN_SDKROOT}/usr/local/include/kernel
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern/libkern
)

target_link_options(libsystem_trace PRIVATE
    --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}
    -Wl,-install_name,/usr/lib/system/libsystem_trace.dylib
    -Wl,-current_version,1147.0.3 -Wl,-compatibility_version,1
    -Wl,-all_load -umbrella System -lobjc)

add_custom_command(TARGET libsystem_trace POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_trace> ${RAVYN_SDKROOT}/usr/lib/system/)

add_dependencies(libsystem_trace libsystem_darwin objc)

add_library(libsystem_m SHARED math.c)
set_target_properties(libsystem_m PROPERTIES SUFFIX ".dylib" PREFIX "libsystem_"
    OUTPUT_NAME "m" NO_SONAME true)

target_compile_options(libsystem_m PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN} --sysroot=${RAVYN_SDKROOT})

target_link_options(libsystem_m PRIVATE
    --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}
    -Wl,-install_name,/usr/lib/system/libsystem_m.dylib
    -Wl,-current_version,3178 -Wl,-compatibility_version,1
    -Wl,-all_load -umbrella System -nostdlib)

add_custom_command(TARGET libsystem_m POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_m> ${RAVYN_SDKROOT}/usr/lib/system/)

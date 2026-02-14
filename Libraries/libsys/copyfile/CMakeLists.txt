add_library(copyfile SHARED
	./copyfile.c
	./xattr_flags.c
)
set_target_properties(copyfile PROPERTIES OUTPUT_NAME "copyfile" PREFIX "lib" SUFFIX ".dylib" NO_SONAME True)

target_compile_options(copyfile PRIVATE
    -isysroot${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN})

target_compile_definitions(copyfile PRIVATE
    "VERSION_INFO_PREFIX=__attribute__((visibility(\"hidden\")))"
     NO_QUARANTINE # until quarantine.h implemented
)

target_include_directories(copyfile PRIVATE
    ../liblaunch
    ${RAVYN_SDKROOT}/usr/include
    ${RAVYN_SDKROOT}/usr/local/include/kernel)

target_link_options(copyfile PRIVATE
    -umbrella System -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib
    -Wl,-install_name,/usr/lib/system/libcopyfile.dylib -Wl,-compatibility_version,1
    -mmacos-version-min=${MACOS_VERSION_MIN} -lsystem_info -lsystem_asl)

add_custom_command(TARGET copyfile POST_BUILD
    COMMAND env PLATFORM=MacOSX DSTROOT=${RAVYN_SDKROOT}
        sh ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/install-files.sh)

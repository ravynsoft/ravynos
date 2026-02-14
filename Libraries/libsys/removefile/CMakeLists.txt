add_library(removefile SHARED
	./removefile_random.c
	./removefile_rename_unlink.c
	./removefile_sunlink.c
	./removefile_tree_walker.c
	./removefile.c
)
set_target_properties(removefile PROPERTIES OUTPUT_NAME "removefile" PREFIX "lib" SUFFIX ".dylib" NO_SONAME True)

target_compile_options(removefile PRIVATE
    --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN})

target_compile_definitions(removefile PRIVATE
    "VERSION_INFO_PREFIX=__attribute__((visibility(\"hidden\")))"
     NO_QUARANTINE # until quarantine.h implemented
)

target_include_directories(removefile PRIVATE ${RAVYN_SDKROOT}/usr/local/include/kernel)

target_link_options(removefile PRIVATE
    -umbrella System -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib
    -Wl,-install_name,/usr/lib/system/libremovefile.dylib -Wl,-compatbility_version,1
    -Wl,-current_version,48  -mmacos-version-min=${MACOS_VERSION_MIN})

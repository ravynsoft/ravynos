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
    -L${RAVYN_SDKROOT}/usr/lib/system  -mmacos-version-min=${MACOS_VERSION_MIN}
    -Wl,-install_name,/usr/lib/system/libremovefile.dylib -Wl,-compatibility_version,1
    -Wl,-current_version,48 -lsystem_kernel -lsystem_c -ldyld -lsystem_malloc)


add_custom_command(TARGET removefile POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    COMMAND env PLATFORM=MacOSX DSTROOT=${RAVYN_SDKROOT}
        sh ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/install_files.sh
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:removefile> ${RAVYN_SDKROOT}/usr/lib/system/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:removefile> ${SYSROOT_DIR}/usr/lib/system/
)

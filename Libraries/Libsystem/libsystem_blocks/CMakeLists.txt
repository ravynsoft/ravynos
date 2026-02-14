add_library(libsystem_blocks SHARED data.c runtime.c)
set_target_properties(libsystem_blocks PROPERTIES NO_SONAME true
    PREFIX "lib" OUTPUT_NAME "system_blocks" SUFFIX ".dylib")

target_compile_options(libsystem_blocks PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT}
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
    -fno-stack-protector -fno-stack-check -fno-builtin
    -momit-leaf-frame-pointer
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

target_include_directories(libsystem_blocks PRIVATE
    . ../private ${ROOT_BINARY_DIR}/${CpuArch}
)

target_link_options(libsystem_blocks PRIVATE
    --sysroot=${RAVYN_SDKROOT} -Wl,-flat_namespace -Wl,-undefined,warning
    -Wl,-all_load -Wl,-umbrella,System -L${RAVYN_SDKROOT}/usr/lib
    -Wl,-install_name,/usr/lib/system/libsystem_blocks.dylib
    -Wl,-search_paths_first -mmacosx-version-min=10.15
    -Wl,-current_version,74 -Wl,-compatibility_version,1.0.0
)

add_custom_command(TARGET libsystem_blocks POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_blocks>
		${RAVYN_SDKROOT}/usr/lib/system/
)

add_dependencies(libsystem_blocks libsystem_platform)

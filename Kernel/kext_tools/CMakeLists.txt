add_executable(kextcache
	./FILES/kextcache_main.c
	./staging.m
	./FILES/update_boot.c
	./syspolicy.m
	./compression.c
	./bootcaches.c
	./driverkit.m
	./FILES/mkext1_file.c
	./safecalls.c
	./rosp_staging.m
	./kext_tools_util.c
	./signposts.m
	./fork_program.c
	./kernelcache.c
	./security.c
)

target_include_directories(kextcache PRIVATE
    ${ROOT_SOURCE_DIR}/Frameworks
    ${ROOT_SOURCE_DIR}/Frameworks/Foundation/Headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../IOKitUser/kext.subproj
    ${RAVYN_SDKROOT}/usr/local/include/kernel
    ${RAVYN_SDKROOT}/usr/local/include
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers)

target_compile_options(kextcache PRIVATE
    --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}
    -F${RAVYN_SDKROOT}/System/Library/Frameworks
    -F${RAVYN_SDKROOT}/System/Library/PrivateFrameworks
)

target_link_options(kextcache PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN}
    -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib
    -L${RAVYN_SDKROOT}/usr/local/lib)

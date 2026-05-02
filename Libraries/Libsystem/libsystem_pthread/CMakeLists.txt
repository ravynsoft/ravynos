add_library(libsystem_pthread SHARED)
set_target_properties(libsystem_pthread PROPERTIES
    OUTPUT_NAME "pthread" SUFFIX ".dylib" PREFIX "libsystem_" NO_SONAME true)

set(libpthread_sources
    pthread.c pthread_asm.s pthread_atfork.c pthread_cancelable.c
    pthread_cond.c pthread_cwd.c pthread_dependency.c pthread_mutex.c
    pthread_rwlock.c pthread_tsd.c qos.c resolver/resolver.c
    variants/pthread_cancelable_cancel.c variants/pthread_cancelable_legacy.c
    variants/pthread_cond_legacy.c variants/pthread_mutex_legacy.c
    variants/pthread_rwlock_legacy.c
)
set(libpthread_incs
    ../../../Kernel/xnu/bsd
    ${CMAKE_CURRENT_BINARY_DIR}/../libsystem_kernel/mig_hdr/include
	${CMAKE_CURRENT_BINARY_DIR}/../libsystem_kernel/mig_hdr/local/include
    . ./private ../private resolver ../libsystem_c/darwin
    ${RAVYN_SDKROOT}/usr/local/include/pthread
    ${ROOT_BINARY_DIR}/${CpuArch}
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ../../../Kernel/Extensions/pthread
    ../../../Kernel/xnu/libsyscall/wrappers/spawn
)
set(libpthread_defs
    DISPATCH_USE_DTRACE=0 __LIBC__ __DARWIN_UNIX03 __DARWIN_64_BIT_INO_T
    __DARWIN_NON_CANCELABLE __DARWIN_VERS_1050 _FORTIFY_SOURCE=0
    __PTHREAD_BUILDING_PTHREAD__ __PTHREAD_EXPOSE_INTERNALS__
    OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1
)
set(libpthread_opts
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT}
    -Wno-implicit-function-declaration -Wno-nullability-completeness
    -Wno-int-conversion -Wno-implicit-int -Wno-incompatible-sysroot
    -Wno-expansion-to-defined -fno-stack-protector -fno-stack-check -fno-builtin
    -momit-leaf-frame-pointer -DPRIVATE
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

target_sources(libsystem_pthread PRIVATE ${libpthread_sources})
target_include_directories(libsystem_pthread PRIVATE ${libpthread_incs})
target_compile_definitions(libsystem_pthread PRIVATE ${libpthread_defs})
target_compile_options(libsystem_pthread PRIVATE ${libpthread_opts})

target_link_options(libsystem_pthread PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT}
    -mmacos-version-min=${MACOS_VERSION_MIN} 
    -Wl,-install_name,/usr/lib/system/libsystem_pthread.dylib
    -Wl,-current_version,416.0.6 -Wl,-compatibility_version,1.0.0
    -rdynamic -nodefaultlibs -Wl,-search_paths_first
    -all_load -L${RAVYN_SDKROOT}/usr/lib/system -umbrella System
    -ldyld -lsystem_platform -lsystem_c -lsystem_kernel -lcompiler_rt
    -Wl,-dirty_data_list,${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/pthread.dirty
    -Wl,-alias_list,${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/pthread.aliases
)

add_custom_command(TARGET libsystem_pthread POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_pthread>
		${RAVYN_SDKROOT}/usr/lib/system/
)

add_library(libpthread_dyld STATIC)
set_target_properties(libpthread_dyld PROPERTIES OUTPUT_NAME "pthread" PREFIX "lib")
target_sources(libpthread_dyld PRIVATE ${libpthread_sources})
target_include_directories(libpthread_dyld PRIVATE ${libpthread_incs})
target_compile_definitions(libpthread_dyld PRIVATE ${libpthread_defs}
    VARIANT_STATIC VARIANT_DYLD)
target_compile_options(libpthread_dyld PRIVATE ${libpthread_opts})

add_custom_command(TARGET libpthread_dyld POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libpthread_dyld>
		${RAVYN_SDKROOT}/usr/local/lib/dyld/
)


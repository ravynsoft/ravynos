add_library(libsystem_pthread SHARED)
set_target_properties(libsystem_pthread PROPERTIES
    OUTPUT_NAME "system_pthread" SUFFIX ".dylib" PREFIX "lib" NO_SONAME true)

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
    -mmacos-version-min=10.15 -flat_namespace -undefined warning
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

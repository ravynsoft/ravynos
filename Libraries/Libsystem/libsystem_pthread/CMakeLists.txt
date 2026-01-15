add_library(libsystem_pthread SHARED)
set_target_properties(libsystem_pthread PROPERTIES
    OUTPUT_NAME "system_pthread" SUFFIX ".dylib" PREFIX "lib" NO_SONAME true)

target_sources(libsystem_pthread PRIVATE
    pthread.c pthread_asm.s pthread_atfork.c pthread_cancelable.c
    pthread_cond.c pthread_cwd.c pthread_dependency.c pthread_mutex.c
    pthread_rwlock.c pthread_tsd.c qos.c resolver/resolver.c
    variants/pthread_cancelable_cancel.c variants/pthread_cancelable_legacy.c
    variants/pthread_cond_legacy.c variants/pthread_mutex_legacy.c
    variants/pthread_rwlock_legacy.c
)

target_include_directories(libsystem_pthread PRIVATE
    ${SDK_SOURCE_DIR}/usr/include
    ${SDK_SOURCE_DIR}/usr/local/include/pthread
    ${SDK_SOURCE_DIR}/usr/local/include
    ../private/pthread ../private .. resolver ../libsystem_c/darwin
    ../../../Kernel/xnu/libsyscall/mach
    ../../../Kernel/xnu/libsyscall/wrappers/spawn
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${SDK_SOURCE_DIR}/usr/include/pthread
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers
    ../../../Kernel/Extensions/pthread
)

target_compile_definitions(libsystem_pthread PRIVATE
    DISPATCH_USE_DTRACE=0 __LIBC__ __DARWIN_UNIX03 __DARWIN_64_BIT_INO_T
    __DARWIN_NON_CANCELABLE __DARWIN_VERS_1050 _FORTIFY_SOURCE=0
    __PTHREAD_BUILDING_PTHREAD__ __PTHREAD_EXPOSE_INTERNALS__
    memory_order_dependency=os_atomic_memory_order_dependency
    memory_order_acquire=os_atomic_memory_order_acquire
    memory_order_release=os_atomic_memory_order_release
    memory_order_relaxed=os_atomic_memory_order_relaxed
    OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1
)

target_compile_options(libsystem_pthread PRIVATE
    --target=${TARGET_TRIPLE} -w
    --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT}
    -isystem ${RAVYN_SDKROOT}/usr/include
    -isystem ${RAVYN_SDKROOT}/usr/local/include
    -iframework ${RAVYN_SDKROOT}/System/Library/Frameworks
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
    -fno-stack-protector -fno-stack-check -fno-builtin
    -momit-leaf-frame-pointer
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

target_link_options(libsystem_pthread PRIVATE
    -mmacos-version-min=10.15 -v -Wl,-install_name,/usr/lib/system/libsystem_pthread.dylib
    -rdynamic -nodefaultlibs -Wl,-search_paths_first
    -all_load -nostdlib -L${RAVYN_SDKROOT}/usr/lib/system -umbrella System
    #-lsystem_kernel -ldyld -lcompiler_rt
    -lsystem_platform
    -Wl,-dirty_data_list,xcodescripts/pthread.dirty
    -Wl,-alias_list,xcodescripts/pthread.aliases 
)

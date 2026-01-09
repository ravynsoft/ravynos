add_library(libsystem_pthread OBJECT)
set_property(TARGET libsystem_pthread PROPERTY OUTPUT_NAME "system_pthread")
set_property(TARGET libsystem_pthread PROPERTY SUFFIX ".dylib")
set_property(TARGET libsystem_pthread PROPERTY PREFIX "lib")

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
    ../private/pthread ../private .. resolver ../libsystem_c/darwin
    ../../../Kernel/xnu/libsyscall/mach
    ../../../Kernel/xnu/libsyscall/wrappers/spawn
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${SDK_SOURCE_DIR}/usr/include/pthread
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers
    ../../../Kernel/Extensions/pthread
)

target_compile_definitions(libsystem_pthread PRIVATE
    DISPATCH_USE_DTRACE=0 __PTHREAD_EXPOSE_INTERNALS__=1
    memory_order_dependency=os_atomic_memory_order_dependency
    memory_order_acquire=os_atomic_memory_order_acquire
    memory_order_release=os_atomic_memory_order_release
    memory_order_relaxed=os_atomic_memory_order_relaxed
    OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1
)

target_compile_options(libsystem_pthread PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -w
    --target=${TARGET_TRIPLE}
    -iframework ${RAVYN_SDKROOT}/System/Library/Frameworks
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
)

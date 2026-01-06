add_library(libsystem_platform STATIC)
set_property(TARGET libsystem_platform PROPERTY OUTPUT_NAME "system_platform")
set_property(TARGET libsystem_platform PROPERTY SUFFIX ".dylib")
set_property(TARGET libsystem_platform PROPERTY PREFIX "lib")

target_sources(libsystem_platform PRIVATE
    simple/asl.c
    simple/getenv.c
    string/generic/memmove.c
    string/generic/strcpy.c
    string/generic/strlen.c
    string/generic/memccpy.c
    string/generic/ffsll.c
    string/generic/strcmp.c
    string/generic/memchr.c
    string/generic/strchr.c
    string/generic/bzero.c
    string/generic/flsll.c
    string/generic/memcmp.c
    string/generic/memset_pattern.c
    string/generic/strncpy.c
    string/generic/strnlen.c
    string/generic/strlcat.c
    string/generic/strlcpy.c
    string/generic/strstr.c
    string/generic/strncmp.c
    force_libplatform_to_build.c
    os/lock.c
    os/alloc_once.c
    os/atomic.c
    os/semaphore.c
    ucontext/x86_64/_ctx_start.s
    ucontext/x86_64/_setcontext.s
    ucontext/x86_64/getcontext.s
    ucontext/generic/setcontext.c
    ucontext/generic/makecontext.c
    ucontext/generic/swapcontext.c
    ucontext/generic/getmcontext.c
    setjmp/x86_64/_setjmp.s
    setjmp/x86_64/setjmp.s
    setjmp/x86_64/_sigtramp.s
    setjmp/generic/setjmperr.c
    setjmp/generic/sigaction.c
    setjmp/generic/sigtramp.c
    init.c
    cachecontrol/x86_64/cache.s
    cachecontrol/generic/cache.c
    atomics/common/MKGetTimeBaseInfo.c
    atomics/x86_64/OSAtomic.s
    atomics/x86_64/pfz.s
    atomics/init.c
    introspection/introspection.c
)

target_include_directories(libsystem_platform PRIVATE
    ../private internal os/resolver ${SDK_SOURCE_DIR}/usr/include
)

target_compile_definitions(libsystem_platform PRIVATE
    DISPATCH_USE_DTRACE=0
    # memory_order_dependency=os_atomic_memory_order_dependency
    # memory_order_acquire=os_atomic_memory_order_acquire
    # memory_order_release=os_atomic_memory_order_release
    # memory_order_relaxed=os_atomic_memory_order_relaxed
    # OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1
    # OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1 
    # OS_ATOMIC_CONFIG_STARVATION_FREE_ONLY=0
    # __DARWIN_LITTLE_ENDIAN=1234 __DARWIN_BIG_ENDIAN=4321
)

target_compile_options(libsystem_platform PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -w
    --target=${TARGET_TRIPLE}
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
)

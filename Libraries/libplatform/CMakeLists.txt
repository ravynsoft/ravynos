set(CMAKE_INSTALL_PREFIX "")

add_library(libplatform)
set_property(TARGET libplatform PROPERTY OUTPUT_NAME "system_platform")

target_sources(libplatform PRIVATE
    src/simple/asl.c
    src/simple/getenv.c
    src/string/generic/memmove.c
    src/string/generic/strcpy.c
    src/string/generic/strlen.c
    src/string/generic/memccpy.c
    src/string/generic/ffsll.c
    src/string/generic/strcmp.c
    src/string/generic/memchr.c
    src/string/generic/strchr.c
    src/string/generic/bzero.c
    src/string/generic/flsll.c
    src/string/generic/memcmp.c
    src/string/generic/memset_pattern.c
    src/string/generic/strncpy.c
    src/string/generic/strnlen.c
    src/string/generic/strlcat.c
    src/string/generic/strlcpy.c
    src/string/generic/strstr.c
    src/string/generic/strncmp.c
    src/force_libplatform_to_build.c
    src/os/lock.c
    src/os/alloc_once.c
    src/os/atomic.c
    src/os/semaphore.c
    src/ucontext/x86_64/_ctx_start.s
    src/ucontext/x86_64/_setcontext.s
    src/ucontext/x86_64/getcontext.s
    src/ucontext/generic/setcontext.c
    src/ucontext/generic/makecontext.c
    src/ucontext/generic/swapcontext.c
    src/ucontext/generic/getmcontext.c
    src/setjmp/x86_64/_setjmp.s
    src/setjmp/x86_64/setjmp.s
    src/setjmp/x86_64/_sigtramp.s
    src/setjmp/generic/setjmperr.c
    src/setjmp/generic/sigaction.c
    src/setjmp/generic/sigtramp.c
    src/init.c
    src/cachecontrol/x86_64/cache.s
    src/cachecontrol/generic/cache.c
    src/atomics/common/MKGetTimeBaseInfo.c
    src/atomics/x86_64/OSAtomic.s
    src/atomics/x86_64/pfz.s
    src/atomics/init.c
    src/introspection/introspection.c
)

set(headers
    include/ucontext.h
    include/setjmp.h
    include/libkern/OSAtomic.h
    include/libkern/OSAtomicDeprecated.h
    include/libkern/OSAtomicQueue.h
    include/libkern/OSSpinLockDeprecated.h
    include/libkern/OSCacheControl.h
    include/os/lock.h
    include/os/base.h
)
target_sources(libplatform INTERFACE ${headers})

target_sources(libplatform PRIVATE
    private/_simple.h
    private/libkern/OSAtomic.h
    private/os/internal/internal_shared.h
    private/os/internal/crashlog.h
    private/os/internal/atomic.h
    private/os/semaphore_private.h
    private/os/lock.h
    private/os/base_private.h
    private/os/alloc_once_impl.h
    private/os/once_private.h
    private/os/lock_private.h
    private/platform/compat.h
    private/platform/introspection_private.h
    private/platform/string.h
    internal/os/internal_asm.h
    internal/os/yield.h
    internal/os/internal.h
)

file(CREATE_LINK ${ROOT_SOURCE_DIR}/Libraries/Libsystem/alloc_once_private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/private/os/alloc_once_private.h)

target_include_directories(libplatform PRIVATE
    internal include private src src/os/resolver
    ../libmalloc/include
    ../libpthread/private
    ../../Kernel/xnu/libsyscall
    ../../Kernel/xnu/libsyscall/wrappers
    ../../Kernel/xnu/libsyscall/mach
    ${RAVYN_SDKROOT}/usr/include
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/PrivateHeaders
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Headers
)

target_compile_definitions(libplatform PRIVATE
    DISPATCH_USE_DTRACE=0
    memory_order_dependency=os_atomic_memory_order_dependency
    memory_order_acquire=os_atomic_memory_order_acquire
    memory_order_release=os_atomic_memory_order_release
    memory_order_relaxed=os_atomic_memory_order_relaxed
    OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1
    OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1 
    OS_ATOMIC_CONFIG_STARVATION_FREE_ONLY=0
    __DARWIN_LITTLE_ENDIAN=1234 __DARWIN_BIG_ENDIAN=4321
)

target_compile_options(libplatform PRIVATE
    --sysroot=${RAVYN_SDKROOT}
    -mmacosx-version-min=${CMAKE_MACOSX_MIN_VERSION}
    -Wno-incompatible-library-redeclaration
    -Wno-implicit-function-declaration
)

add_dependencies(libplatform xnu_headers)

add_custom_command(TARGET libplatform
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${headers} ${SYSROOT_DIR}/usr/include
    COMMAND ${CMAKE_COMMAND} -E copy ${headers} ${RAVYN_SDKROOT}/usr/include
    COMMAND ${CMAKE_COMMAND} -E copy ${headers}
        ${SYSROOT_DIR}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${headers}
        ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/
)

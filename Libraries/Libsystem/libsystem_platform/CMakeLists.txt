add_library(libsystem_platform SHARED)
set_target_properties(libsystem_platform PROPERTIES
    OUTPUT_NAME "system_platform" SUFFIX ".dylib" PREFIX "lib" NO_SONAME true)

target_sources(libsystem_platform PRIVATE
    simple/asl.c
    simple/getenv.c
    simple/string_io.c
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
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ../libsystem_c/include
    ../private internal
    ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk
    os/resolver
    ${CMAKE_CURRENT_BINARY_DIR}/../libsystem_kernel/mig_hdr/include
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers
)

target_compile_definitions(libsystem_platform PRIVATE
    DISPATCH_USE_DTRACE=0 voucher_mach_msg_state_t=mach_port_t;
    OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1
    OS_ATOMIC_CONFIG_STARVATION_FREE_ONLY=0 OS_UNFAIR_LOCK_INLINE=0
    OSATOMIC_USE_INLINED=0 OSATOMIC_DEPRECATED=0 OSSPINLOCK_USE_INLINED=0
)

target_compile_options(libsystem_platform PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT} -w
    -Wno-implicit-function-declaration -Wno-int-conversion -Wno-implicit-int
    -fno-stack-protector -fdollars-in-identifiers -fno-common -fverbose-asm
    -momit-leaf-frame-pointer -DPRIVATE
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

target_link_options(libsystem_platform PRIVATE
    -mmacos-version-min=10.15 -v -Wl,-install_name,/usr/lib/system/libsystem_platform.dylib
    -rdynamic -nodefaultlibs -Wl,-search_paths_first
    -all_load -nostdlib -L${RAVYN_SDKROOT}/usr/lib/system -umbrella System -lsystem_kernel
    #-lcompiler-rt -ldyld
    -Wl,-alias_list,${CMAKE_CURRENT_SOURCE_DIR}/xcodeconfig/libplatform.aliases
)
SRCS = \
        simple/asl.c \
        simple/getenv.c \
        simple/string_io.c \
        string/generic/memmove.c \
        string/generic/strcpy.c \
        string/generic/strlen.c \
        string/generic/memccpy.c \
        string/generic/ffsll.c \
        string/generic/strcmp.c \
        string/generic/memchr.c \
        string/generic/strchr.c \
        string/generic/bzero.c \
        string/generic/flsll.c \
        string/generic/memcmp.c \
        string/generic/memset_pattern.c \
        string/generic/strncpy.c \
        string/generic/strnlen.c \
        string/generic/strlcat.c \
        string/generic/strlcpy.c \
        string/generic/strstr.c \
        string/generic/strncmp.c \
        force_libplatform_to_build.c \
        os/alloc_once.c \
        os/lock.c \
        os/semaphore.c \
        ucontext/x86_64/_ctx_start.s \
        ucontext/x86_64/_setcontext.s \
        ucontext/x86_64/getcontext.s \
        ucontext/generic/setcontext.c \
        ucontext/generic/makecontext.c \
        ucontext/generic/swapcontext.c \
        ucontext/generic/getmcontext.c \
        setjmp/x86_64/_setjmp.s \
        setjmp/x86_64/setjmp.s \
        setjmp/x86_64/_sigtramp.s \
        setjmp/generic/setjmperr.c \
        setjmp/generic/sigaction.c \
        setjmp/generic/sigtramp.c \
        init.c \
        cachecontrol/x86_64/cache.s \
        cachecontrol/generic/cache.c \
        atomics/common/MKGetTimeBaseInfo.c \
        atomics/x86_64/OSAtomic.s \
        atomics/x86_64/pfz.s \
        atomics/init.c \
        introspection/introspection.c

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -w -Wno-implicit-function-declaration -Wno-int-conversion \
        -Wno-implicit-int -fno-stack-protector -fdollars-in-identifiers \
        -fno-common -fverbose-asm -momit-leaf-frame-pointer -DPRIVATE \
        -DDISPATCH_USE_DTRACE=0 -D_FORTIFY_SOURCE=0 -DOS_UNFAIR_LOCK_INLINE=0 \
        -DOSATOMIC_USE_INLINED=0 -DOSATOMIC_DEPRECATED=0 ${SEED_DEFS} \
        -DOSSPINLOCK_USE_INLINED=0 -I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
        -I${.CURDIR}/../private -I${.CURDIR}/internal -I${.CURDIR}/os/resolver \
        -I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers \
        -I${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders \
        ${CFLAGS.${.TARGET:T}}


PACKAGE=	clibs
SHLIBDIR?= /lib

LIBADD=	c sys
.if defined(COMPAT_libcompat)
# XXX: work around gcc -m32 + bfd ld path issue for DT_NEEDED symbols.
# https://sourceware.org/bugzilla/show_bug.cgi?id=31395
LDFLAGS+=	-Wl,--rpath=/usr/lib${COMPAT_libcompat}
.endif

.include <src.opts.mk>
MK_SSP=	no
# SSP forced off already implies FORTIFY_SOURCE=0, but we must make sure that
# one cannot turn it back on.
FORTIFY_SOURCE=	0

LIB=thr
SHLIB_MAJOR= 3

NO_WTHREAD_SAFETY=1
NO_WCAST_ALIGN.gcc=1    # for gcc 4.2
CFLAGS+=-fPIC
CFLAGS+=-DPTHREAD_KERNEL
CFLAGS+=-I${SRCTOP}/lib/libc/include
CFLAGS+=-I${SRCTOP}/lib/libc/${MACHINE_CPUARCH}
CFLAGS+=-I${.CURDIR}/thread
CFLAGS+=-I${.CURDIR}/arch/${MACHINE_CPUARCH}/include
CFLAGS+=-I${.CURDIR}/sys
CFLAGS+=-I${SRCTOP}/libexec/rtld-elf
CFLAGS+=-I${SRCTOP}/libexec/rtld-elf/${MACHINE_CPUARCH}
CFLAGS+=-I${SRCTOP}/lib/libthread_db

CFLAGS.thr_stack.c+=	-Wno-cast-align
CFLAGS.rtld_malloc.c+=	-Wno-cast-align
# Disable compiler builtins so that the compiler does not optimize away rtld's
# attempts to force early PLT resolution for certain functions that may be
# replaced by builtins.
CFLAGS.thr_rtld.c+=	-fno-builtin
CFLAGS.thr_symbols.c+=	-Wno-missing-variable-declarations
.if ${MK_ASAN} != "no"
# False-positive ASAN error claiming the local "struct sigaction act;" is
# overflowed by handle_signal() reading from the ucontext_t argument. This
# could be caused by ASAN not treating this function as a signal handler.
CFLAGS.thr_sig.c+=	-fno-sanitize=address
.endif

.if ${MACHINE_CPUARCH} == "arm"
NO_THREAD_UNWIND_STACK= yes
.endif

.if ${MACHINE_CPUARCH} == "arm"
NO_THREAD_UNWIND_STACK= yes
.endif

.ifndef NO_THREAD_UNWIND_STACK
CFLAGS+=-fexceptions
CFLAGS+=-D_PTHREAD_FORCED_UNWIND
.endif

LDFLAGS+=-Wl,-znodelete
LDFLAGS+=-Wl,-zinitfirst
LDFLAGS+=-Wl,--auxiliary,libsys.so.7

VERSION_DEF=${SRCTOP}/lib/libc/Versions.def
SYMBOL_MAPS=${.CURDIR}/pthread.map

MAN=	libthr.3

.if ${MK_PTHREADS_ASSERTIONS} != "no"
# enable extra internal consistency checks
CFLAGS+=-D_PTHREADS_INVARIANTS
.endif

PRECIOUSLIB=

.PATH: ${.CURDIR}/arch/${MACHINE_CPUARCH}/${MACHINE_CPUARCH}
.PATH: ${SRCTOP}/libexec/rtld-elf

.if exists(${.CURDIR}/arch/${MACHINE_CPUARCH}/Makefile.inc)
.include "${.CURDIR}/arch/${MACHINE_CPUARCH}/Makefile.inc"
.endif
.include "${.CURDIR}/sys/Makefile.inc"
.include "${.CURDIR}/thread/Makefile.inc"
SRCS+= rtld_malloc.c

LIBSYS_SRCTOP=	${.CURDIR:H}/libsys
.if exists(${LIBSYS_SRCTOP}/${MACHINE_CPUARCH}/_umtx_op_err.S)
.PATH: ${LIBSYS_SRCTOP}/${MACHINE_CPUARCH}
.else
.PATH: ${LIBSYS_SRCTOP}
.endif
OBJS+=	_umtx_op_err.o

.if ${MK_INSTALLLIB} != "no"
SYMLINKS+=lib${LIB}.a ${LIBDIR}/libpthread.a
.endif
.if !defined(NO_PIC)
SYMLINKS+=lib${LIB}.so ${LIBDIR}/libpthread.so
.endif

HAS_TESTS=
SUBDIR.${MK_TESTS}+= tests

.include <bsd.lib.mk>

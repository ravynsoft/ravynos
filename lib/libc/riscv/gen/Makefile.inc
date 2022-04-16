# $FreeBSD$

SRCS+=	_ctx_start.S \
	fabs.S \
	flt_rounds.c \
	fpgetmask.c \
	fpsetmask.c \
	_get_tp.c \
	infinity.c \
	ldexp.c \
	makecontext.c \
	_setjmp.S \
	setjmp.S \
	sigsetjmp.S \
	trivial-getcontextx.c

CFLAGS._get_tp.c+= ${RTLD_HDRS}

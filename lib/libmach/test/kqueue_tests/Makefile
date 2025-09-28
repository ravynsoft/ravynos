
PROG= kqueue_tests
SRCS= kqueue_tests.c

#MK_MAN=no
CFLAGS+= -g
CFLAGS+= -I. -I../../../../include -D__APPLE__ -O0
LDADD+= -lmach -pthread

.include <bsd.prog.mk>

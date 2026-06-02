.PATH: ${.CURDIR:H}
MK_WERROR = no
LIB = vCancelable
SRCS = compat-43/FreeBSD/creat.c \
	compat-43/sigcompat.c \
	gen/FreeBSD/lockf.c \
	gen/nanosleep.c \
	gen/FreeBSD/pause.c \
	gen/FreeBSD/sleep.c \
	gen/FreeBSD/termios.c \
	gen/FreeBSD/usleep.c \
	gen/FreeBSD/wait.c \
	gen/FreeBSD/waitpid.c \
	net/FreeBSD/recv.c \
	net/FreeBSD/send.c \
	stdlib/FreeBSD/system.c

CFLAGS += -DBUILDING_VARIANT -DVARIANT_CANCELABLE \
	-U__DARWIN_NON_CANCELABLE -D__DARWIN_NON_CANCELABLE=0 \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd \
	-I${.CURDIR:H}/fbsdcompat \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${.CURDIR:H}/regex/TRE -I${.CURDIR:H}/regex/FreeBSD \
	-I${.CURDIR:H}/locale/FreeBSD -I${.CURDIR:H}/stdtime/FreeBSD \
	-I${.CURDIR:H}/../private -I${SDK_SOURCE_DIR}/usr/include \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-include${.CURDIR:H}/fbsdcompat/_fbsd_compat_.h \
	${CFLAGS.${.TARGET:T}}

CFLAGS.creat.o = -DLIBC_ALIAS_CREAT
CFLAGS.sigcompat.o = -DLIBC_ALIAS_SIGPAUSE
CFLAGS.lockf.o = -DLIBC_ALIAS_LOCKF
CFLAGS.pause.o = -DLIBC_ALIAS_PAUSE
CFLAGS.sleep.o = -DLIBC_ALIAS_SLEEP
CFLAGS.termios.o = -DLIBC_ALIAS_TCDRAIN
CFLAGS.usleep.o = -DLIBC_ALIAS_USLEEP
CFLAGS.wait.o = -DLIBC_ALIAS_WAIT
CFLAGS.waitpid.o = -DLIBC_ALIAS_WAITPID
CFLAGS.recv.o = -DLIBC_ALIAS_RECV
CFLAGS.send.o = -DLIBC_ALIAS_SEND
CFLAGS.system.o = -DLIBC_ALIAS_SYSTEM

.include <bsd.lib.mk>


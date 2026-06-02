.PATH: ${.CURDIR:H}
MK_WERROR = no
LIB = vLegacy
SRCS = compat-43/FreeBSD/creat.c \
	compat-43/setregid.c \
	compat-43/setreuid.c \
	compat-43/sigcompat.c \
	compat-43/FreeBSD/killpg.c \
	gdtoa/FreeBSD/gdtoa-strtof.c \
	gdtoa/FreeBSD/gdtoa-strtod.c \
	gdtoa/FreeBSD/gdtoa-strtodg.c \
	gen/FreeBSD/clock.c \
	gen/confstr.c \
	gen/crypt.c \
	gen/FreeBSD/fnmatch.c \
	gen/FreeBSD/lockf.c \
	gen/nanosleep.c \
	gen/nftw.c \
	gen/FreeBSD/nice.c \
	gen/FreeBSD/pause.c \
	gen/FreeBSD/popen.c \
	gen/FreeBSD/setmode.c \
	gen/FreeBSD/sleep.c \
	gen/FreeBSD/termios.c \
	gen/FreeBSD/timezone.c \
	gen/FreeBSD/ttyname.c \
	gen/FreeBSD/usleep.c \
	gen/FreeBSD/wait.c \
	gen/FreeBSD/waitpid.c \
	locale/FreeBSD/wcsftime.c \
	net/FreeBSD/recv.c \
	net/FreeBSD/send.c \
	regex/TRE/lib/regcomp.c \
	stdio/FreeBSD/fdopen.c \
	stdio/FreeBSD/fopen.c \
	stdio/FreeBSD/fputs.c \
	stdio/FreeBSD/freopen.c \
	stdio/FreeBSD/fwrite.c \
	stdio/FreeBSD/tempnam.c \
	stdlib/FreeBSD/getopt.c \
	stdlib/FreeBSD/putenv.c \
	stdlib/FreeBSD/realpath.c \
	stdlib/FreeBSD/setenv.c \
	stdlib/FreeBSD/system.c \
	stdtime/FreeBSD/localtime.c \
	stdtime/FreeBSD/strftime.c \
	stdtime/FreeBSD/strptime.c \
	sys/msgctl.c \
	sys/semctl.c \
	sys/shmctl.c

CFLAGS += -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0 -U__DARWIN_64_BIT_INO_T \
	-D__DARWIN_64_BIT_INO_T=0 -DBUILDING_VARIANT -DVARIANT_LEGACY \
	-DHAVE_MBSTATE_T -D__isthreaded \
	-I${.CURDIR:H}/include -I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${.CURDIR:H}/../private -I${.CURDIR:H}/fbsdcompat \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${.CURDIR:H}/gdtoa -I${.CURDIR:H}/../private/pthread \
	-I${.CURDIR:H}/regex/TRE -I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-include${.CURDIR:H}/fbsdcompat/_fbsd_compat_.h \
	${CFLAGS.${.TARGET:T}}

#BEFORE include ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
#AFTER ../private fbsdcompat ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach

CFLAGS.creat.o = -DLIBC_ALIAS_CREAT -include${.CURDIR:H}/fbsdcompat/sys/cdefs.h
CFLAGS.setregid.o = -DLIBC_ALIAS_SETREGID -include${.CURDIR:H}/include/sys/cdefs.h
CFLAGS.setreuid.o = -DLIBC_ALIAS_SETREUID -include${.CURDIR:H}/include/sys/cdefs.h
CFLAGS.sigcompat.o = -DLIBC_ALIAS_SIGPAUSE -include${.CURDIR:H}/include/sys/cdefs.h
CFLAGS.confstr.o = -DLIBC_ALIAS_CONFSTR -include${.CURDIR:H}/include/sys/cdefs.h
CFLAGS.crypt.o = -DLIBC_ALIAS_ENCRYPT -DLIBC_ALIAS_SETKEY
CFLAGS.clock.o = -DLIBC_ALIAS_CLOCK -include${.CURDIR:H}/include/sys/cdefs.h
CFLAGS.fnmatch.o = -DLIBC_ALIAS_FNMATCH -include${.CURDIR:H}/include/sys/cdefs.h
CFLAGS.lockf.o = -DLIBC_ALIAS_LOCKF -include${.CURDIR:H}/include/sys/cdefs.h
CFLAGS.nice.o = -DLIBC_ALIAS_NICE
CFLAGS.pause.o = -DLIBC_ALIAS_PAUSE -include${.CURDIR:H}/fbsdcompat/sys/cdefs.h
CFLAGS.popen.o = -DLIBC_ALIAS_POPEN -I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn
CFLAGS.setmode.o = -DLIBC_ALIAS_SETMODE
CFLAGS.sleep.o = -DLIBC_ALIAS_SLEEP -include${.CURDIR:H}/fbsdcompat/sys/cdefs.h
CFLAGS.termios.o = -DLIBC_ALIAS_TCDRAIN -include${.CURDIR:H}/fbsdcompat/sys/cdefs.h
CFLAGS.ttyname.o = -DLIBC_ALIAS_TTYNAME_R
CFLAGS.usleep.o = -DLIBC_ALIAS_USLEEP
CFLAGS.wait.o = -DLIBC_ALIAS_WAIT -include${.CURDIR:H}/fbsdcompat/sys/cdefs.h
CFLAGS.waitpid.o = -DLIBC_ALIAS_WAITPID
CFLAGS.nanosleep.o = -DLIBC_ALIAS_NANOSLEEP -include${.CURDIR:H}/include/sys/cdefs.h
CFLAGS.nftw.o = -DLIBC_ALIAS_FTW -DLIBC_ALIAS_NFTW
CFLAGS.wcsftime.o = -DLIBC_ALIAS_WCSFTIME -DLIBC_ALIAS_WCSFTIME_L
CFLAGS.recv.o = -DLIBC_ALIAS_RECV
CFLAGS.send.o = -DLIBC_ALIAS_SEND
CFLAGS.fdopen.o = -DLIBC_ALIAS_FDOPEN
CFLAGS.fopen.o = -DLIBC_ALIAS_FOPEN
CFLAGS.fputs.o = -DLIBC_ALIAS_FPUTS
CFLAGS.freopen.o = -DLIBC_ALIAS_FREOPEN
CFLAGS.fwrite.o = -DLIBC_ALIAS_FWRITE
CFLAGS.tempnam.o = -DLIBC_ALIAS_TEMPNAM -include${.CURDIR:H}/fbsdcompat/sys/cdefs.h
CFLAGS.getopt.o = -DLIBC_ALIAS_GETOPT
CFLAGS.putenv.o = -DLIBC_ALIAS_PUTENV
CFLAGS.realpath.o = -DLIBC_ALIAS_REALPATH
CFLAGS.setenv.o = -DLIBC_ALIAS_REALPATH
CFLAGS.system.o = -DLIBC_ALIAS_SYSTEM -include${.CURDIR:H}/fbsdcompat/sys/cdefs.h -I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn
CFLAGS.localtime.o = -DLIBC_ALIAS_MKTIME
CFLAGS.strptime.o = -DLIBC_ALIAS_STRPTIME -DLIBC_ALIAS_STRPTIME_L
CFLAGS.msgctl.o = -DLIBC_ALIAS_MSGCTL -DKERNEL -D_LIBC_NO_FEATURE_VERIFICATION \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk
CFLAGS.semctl.o = -DLIBC_ALIAS_SEMCTL -DKERNEL -D_LIBC_NO_FEATURE_VERIFICATION \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk
CFLAGS.shmctl.o = -DLIBC_ALIAS_SHMCTL -DKERNEL -D_LIBC_NO_FEATURE_VERIFICATION \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk

.include <bsd.lib.mk>


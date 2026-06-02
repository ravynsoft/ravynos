.PATH: ${.CURDIR:H}
LIB = Base
SRCS = \
	emulated/lchflags.c \
	emulated/lchmod.c \
	emulated/lutimes.c \
	emulated/statvfs.c \
	gen/backtrace.c \
	gen/clock_gettime.c \
	gen/confstr.c \
	gen/crypt.c \
	gen/devname.c \
	gen/disklabel.c \
	gen/errlst.c \
	gen/filesec.c \
	gen/fts.c \
	gen/get_compat.c \
	gen/getloadavg.c \
	gen/getttyent.c \
	gen/getusershell.c \
	gen/getvfsbyname.c \
	darwin/atexit_receipt.c \
	gen/nftw.c \
	gen/setlogin.c \
	gen/sigsetops.c \
	gen/strtofflags.c \
	gen/thread_stack_pcs.c \
	gen/uname.c \
	gen/utmpx-darwin.c \
	os/debug_private.c \
	gmon/gmon.c \
	locale/isctype.c \
	string/bcopy.c \
	locale/iswctype.c \
	locale/xlocale.c \
	posix1e/acl.c \
	posix1e/acl_entry.c \
	posix1e/acl_file.c \
	posix1e/acl_flag.c \
	posix1e/acl_perm.c \
	posix1e/acl_translate.c \
	stdtime/getdate.c \
	stdtime/timezone_unix03.c \
	stdio/xprintf_all_in_one.c \
	stdio/xprintf_comp.c \
	stdio/xprintf_domain.c \
	stdio/xprintf_exec.c \
	util/fparseln.c \
	util/login_tty.c \
	util/logwtmp.c \
	util/mkpath_np.c \
	gen/raise.c \
	util/opendev.c \
	util/pty.c \
	uuid/uuidsrc/clear.c \
	uuid/uuidsrc/compare.c \
	uuid/uuidsrc/copy.c \
	uuid/uuidsrc/gen_uuid.c \
	uuid/uuidsrc/isnull.c \
	uuid/uuidsrc/pack.c \
	uuid/uuidsrc/parse.c \
	uuid/uuidsrc/unpack.c \
	uuid/uuidsrc/unparse.c \
	emulated/tcgetsid.c \
	sys/_libc_init.c \
	sys/_libc_fork_child.c \
	sys/chmodx_np.c \
	sys/crt_externs.c \
	sys/fork.c \
	sys/gettimeofday.c \
	sys/openx_np.c \
	sys/posix_spawn.c \
	sys/settimeofday.c \
	sys/sigaction.c \
	sys/statx_np.c \
	sys/umaskx_np.c \
	locale/frune.c \
	locale/mbrune.c \
	locale/runedepreciated.c \
	locale/setinvalidrune.c \
	util/login.c \
	util/logout.c \
	gen/nlist.c \
	sys/OSThermalNotification.c \
	string/stpcpy.c \
	string/stpncpy.c \
	string/strcat.c \
	string/strncat.c \
	gen/sync_volume_np.c \
	gen/dirfd.c \
	os/assumes.c \
	stubs.c

SLF = /System/Library/Frameworks
CFLAGS += -DHAVE_STDINT_H -DPRIVATE \
	-I${.CURDIR:H}/include \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
	-I${.CURDIR:H}/fbsdcompat \
	-I${.CURDIR:H}/gdtoa \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${.CURDIR:H}/../private \
	-I${.CURDIR:H}/../libsystem_info/membership.subproj \
	-I${.CURDIR:H}/../libsystem_info/lookup.subproj \
	-I${.CURDIR:H}/../libsystem_notify \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${ROOT_SOURCE_DIR}/Libraries/Libsystem/private \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${SDK_SOURCE_DIR}/usr/include \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS \
	-Wimplicit-fallthrough -Wno-unknown-pragmas -Wno-assume \
	-Wformat-nonliteral -Wno-vla-extension -Wundef-prefix=SUPPORT_ \
	-include${.CURDIR:H}/fbsdcompat/_fbsd_compat_.h \
	${CFLAGS.${.TARGET:T}}

CFLAGS.bcopy.o = -momit-leaf-frame-pointer
CFLAGS.xprintf_all_in_one.o = -I${.CURDIR:H}/stdio/FreeBSD
CFLAGS.xprintf_comp.o = -I${.CURDIR:H}/stdio/FreeBSD
CFLAGS.xprintf_domain.o = -I${.CURDIR:H}/stdio/FreeBSD
CFLAGS.xprintf_exec.o = -I${.CURDIR:H}/stdio/FreeBSD
CFLAGS.fparseln.o = -D__FBSDID=__RCSID
CFLAGS.clear.o = -include${.CURDIR:H}/uuid/uuid-config.h
CFLAGS.compare.o = -include${.CURDIR:H}/uuid/uuid-config.h
CFLAGS.copy.o = -include${.CURDIR:H}/uuid/uuid-config.h
CFLAGS.gen_uuid.o = -include${.CURDIR:H}/uuid/uuid-config.h
CFLAGS.isnull.o = -include${.CURDIR:H}/uuid/uuid-config.h
CFLAGS.pack.o = -include${.CURDIR:H}/uuid/uuid-config.h
CFLAGS.parse.o = -include${.CURDIR:H}/uuid/uuid-config.h
CFLAGS.unpack.o = -include${.CURDIR:H}/uuid/uuid-config.h
CFLAGS.unparse.o = -include${.CURDIR:H}/uuid/uuid-config.h
CFLAGS.frune.o = -D__FBSDID=__RCSID
CFLAGS.mbrune.o = -D__FBSDID=__RCSID
CFLAGS.runedepreciated.o = -D__FBSDID=__RCSID
CFLAGS.setinvalidrune.o = -D__FBSDID=__RCSID
CFLAGS._libc_init.o = -I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders
CFLAGS.gettimeofday.o = -I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders
CFLAGS.assumes.o = -I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders

.include <bsd.lib.mk>

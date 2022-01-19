.include <bsd.init.mk>

LIB=	mach


A= ${.CURDIR}/${MACHINE_CPUARCH}
H= ${.CURDIR}/../../include/mach
DEFS=${.CURDIR}/../../sys/compat/mach/defs


.PATH: ${.CURDIR}/mach
.PATH: ${A}
.PATH: ${A}/sys

.for _dir in ${.CURDIR}/../.. ${.CURDIR}/../../sys ${.CURDIR}/../../.. /sys /usr/src/sys
.if !defined(SYSDIR) && exists(${_dir}/kern/)
SYSDIR=	${_dir}
.endif
.endfor
.if !defined(SYSDIR) || !exists(${SYSDIR}/kern/)
.error "can't find kernel source tree"
.endif

CFLAGS+= -I${.CURDIR}/../../include -I. -I${A} -I${.CURDIR} -I./include
CFLAGS+= -I${SYSDIR} -I${.CURDIR}/../../apsl/include -I${MACHINE_INCLUDES}
CFLAGS+= -fPIC -DPRIVATE -D__MigTypeCheck=1 -DLIBSYSCALL_INTERFACE=1
DEBUG_FLAGS= -g -O0
mach_port.h mach_portUser.c: ${DEFS}/mach_port.defs
	mig ${CFLAGS} -server /dev/null ${DEFS}/mach_port.defs
	cp mach_port.h ${H}/mach_port.h
task.h taskUser.c: ${DEFS}/task.defs
	mig ${CFLAGS} -server /dev/null ${DEFS}/task.defs
	cp task.h ${H}/task.h
host_priv.h host_privUser.c: ${DEFS}/host_priv.defs
	mig ${CFLAGS} -server /dev/null ${DEFS}/host_priv.defs
	cp host_priv.h ${H}/host_priv.h
mach_host.h mach_hostUser.c: ${DEFS}/mach_host.defs
	mig ${CFLAGS} -server /dev/null ${DEFS}/mach_host.defs
	cp mach_host.h ${H}/mach_host.h
vm_map.h vm_mapUser.c: ${DEFS}/vm_map.defs
	mig ${CFLAGS} -server /dev/null ${DEFS}/vm_map.defs
	cp vm_map.h ${H}/vm_map.h
mach_vm.h mach_vmUser.c: ${DEFS}/mach_vm.defs
	mig ${CFLAGS} -server /dev/null ${DEFS}/mach_vm.defs
	cp mach_vm.h ${H}/mach_vm.h
clock.h clockUser.c: ${DEFS}/clock.defs
	mig ${CFLAGS} -server /dev/null ${DEFS}/clock.defs
	cp clock.h ${H}/clock.h
SRCS=					\
	clockUser.c			\
	mach_portUser.c		\
	taskUser.c			\
	host_privUser.c		\
	mach_hostUser.c		\
	vm_mapUser.c		\
	mach_vmUser.c		\
	error_codes.c		\
	mach_error_string.c \
	mach_init.c 		\
	mach_msg.c			\
	mach_misc.c			\
	mig_allocate.c 		\
	mig_deallocate.c 	\
	mig_reply_setup.c	\
	mig_strncpy.c 		\
	mig_support.c

CLEANFILES+= machine *~ mach_port.h task.h host_priv.h mach_host.h
CLEANFILES+= cscope.* vm_map.h *User.c

.include <bsd.lib.mk>

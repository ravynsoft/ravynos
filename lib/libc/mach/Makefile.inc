#	@(#)Makefile.inc	8.6 (Berkeley) 5/4/95

# mach sources
A= ${LIBC_SRCTOP}/${MACHINE_CPUARCH}
H= ${LIBC_SRCTOP}/../../include/mach
DEFS=${LIBC_SRCTOP}/../../sys/compat/mach/defs

.PATH: ${LIBC_SRCTOP}/mach
.PATH: ${A}
.PATH: ${A}/sys

CFLAGS+= -I${LIBC_SRCTOP}/../../include -I${LIBC_SRCTOP}/mach -I${A}
CFLAGS+= -I${LIBC_SRCTOP}/../../sys
CFLAGS+= -fPIC -DPRIVATE -D__MigTypeCheck=1 -DLIBSYSCALL_INTERFACE=1
#DEBUG_FLAGS= -g -O0
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

SRCS+=	clockUser.c			\
	mach_portUser.c		\
	taskUser.c			\
	host_privUser.c		\
	mach_hostUser.c		\
	vm_mapUser.c		\
	mach_vmUser.c		\
	error_codes.c \
	mach_error_string.c \
	mach_init.c \
	mach_misc.c \
	mach_msg.c \
	mig_allocate.c \
	mig_deallocate.c \
	mig_reply_setup.c \
	mig_strncpy.c \
	mig_support.c

SYM_MAPS+=${LIBC_SRCTOP}/mach/Symbol.map
CLEANFILES+= machine *~ mach_port.h task.h host_priv.h mach_host.h
CLEANFILES+= cscope.* vm_map.h *User.c

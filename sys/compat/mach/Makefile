# Makefile for syscall tables
#
# $FreeBSD$



all:
	@echo "make serverdefs only"

SYSPATH=../../sys/mach

.PATH: defs
.PATH: ${SYSPATH}

HEADERNAMES= host_priv_server.h mach_host_server.h mach_port_server.h mach_vm_server.h \
	task_server.h vm_map_server.h clock_server.c

SERVERHEADERS= ${HEADERNAMES:S//${SYSPATH}\/&/1}

serverdefs: ${SERVERHEADERS}	host_priv_server.c mach_host_server.c mach_port_server.c \
	mach_vm_server.c task_server.c vm_map_server.c clock_server.c



MFLAGS= -DKERNEL -I../.. -DKERNEL_SERVER

# XXX
MFLAGS+= -D__LP64__


host_priv_server.h host_priv_server.c: defs/host_priv.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server host_priv_server.c defs/host_priv.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server /dev/null \
	-sheader ${SYSPATH}/host_priv_server.h defs/host_priv.defs
${SYSPATH}/mach_host_server.h mach_host_server.c: defs/mach_host.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server mach_host_server.c defs/mach_host.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server /dev/null \
	-sheader ${SYSPATH}/mach_host_server.h defs/mach_host.defs
${SYSPATH}/mach_port_server.h mach_port_server.c: defs/mach_port.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server mach_port_server.c defs/mach_port.defs 
	mig ${MFLAGS} -header /dev/null -user /dev/null -server /dev/null \
	-sheader ${SYSPATH}/mach_port_server.h defs/mach_port.defs
${SYSPATH}/mach_vm_server.h mach_vm_server.c: defs/mach_vm.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server mach_vm_server.c defs/mach_vm.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server /dev/null \
	-sheader ${SYSPATH}/mach_vm_server.h defs/mach_vm.defs
${SYSPATH}/task_server.h task_server.c: defs/task.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server task_server.c defs/task.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server /dev/null \
	-sheader ${SYSPATH}/task_server.h defs/task.defs
${SYSPATH}/vm_map_server.h vm_map_server.c: defs/vm_map.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server vm_map_server.c defs/vm_map.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server /dev/null \
	-sheader ${SYSPATH}/vm_map_server.h defs/vm_map.defs
${SYSPATH}/clock_server.h clock_server.c: defs/clock.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server clock_server.c defs/clock.defs
	mig ${MFLAGS} -header /dev/null -user /dev/null -server /dev/null \
	-sheader ${SYSPATH}/clock_server.h defs/clock.defs

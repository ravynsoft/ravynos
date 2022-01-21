# $FreeBSD$

NO_OBJ=
INCS=	boolean.h \
	clock.h \
	clock_types.defs \
	clock_types.h \
	error.h \
	exception.h \
	exception_types.h \
	host_info.h \
	host_notify.h \
	host_priv.h \
	host_special_ports.h \
	ipc_common.h \
	ipc_types.h \
	kern_return.h \
	mach.h \
	mach_error.h \
	mach_host.h \
	mach_init.h \
	mach_interface.h \
	mach_port.h \
	mach_time.h \
	mach_traps.h \
	mach_types.defs \
	mach_types.h \
	mach_vm.h \
	machine.h \
	memory_object.h \
	memory_object_types.h \
	message.h \
	mig.h \
	mig_errors.h \
	ndr.h \
	notify.h \
	policy.h \
	port.h \
	processor_info.h \
	prof_types.h \
	rpc.h \
	std_types.defs \
	std_types.h \
	task.h \
	task_info.h \
	task_policy.h \
	task_special_ports.h \
	thread_info.h \
	thread_special_ports.h \
	thread_status.h \
	time_value.h \
	vm_attributes.h \
	vm_behavior.h \
	vm_inherit.h \
	vm_map.h \
	vm_prot.h \
	vm_region.h \
	vm_statistics.h \
	vm_sync.h \
	vm_types.h
INCSDIR=${INCLUDEDIR}/mach
SUBDIR=	device i386

includes:
	${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 0755 ${DESTDIR}${INCSDIR}
.for h in ${INCS}
	${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 0444 ${.CURDIR}/$h ${DESTDIR}${INCSDIR}
.endfor


distprep:
	mkdir -pv ${DISTDIR}/base/${INCSDIR}

.include <bsd.prog.mk>

distribute: distprep

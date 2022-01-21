#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <mach/mach.h>
#include <mach/boolean.h>
#include <mach/mach_traps.h>
#include <mach/mach_init.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <mach/mach_port.h>
#include <mach/mach_vm.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define HAVE_MACH
#include <pthread.h>

kern_return_t _kernelrpc_mach_port_allocate_trap(
	ipc_space_t task,
	mach_port_right_t right,
	mach_port_name_t *name
);
kern_return_t _kernelrpc_mach_port_deallocate_trap(
	ipc_space_t task,
	mach_port_name_t name
);

kern_return_t _kernelrpc_mach_port_insert_right_trap(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_t poly,
	mach_msg_type_name_t polyPoly
);
kern_return_t _kernelrpc_mach_port_extract_right_trap(
	ipc_space_t task,
	mach_port_name_t name,
	mach_msg_type_name_t msgt_name,
	mach_port_t *poly,
	mach_msg_type_name_t *polyPoly
);

kern_return_t _kernelrpc_mach_vm_allocate_trap(
       mach_vm_map_t target,
       mach_vm_address_t *address,
       mach_vm_size_t size,
       int flags
	);
kern_return_t _kernelrpc_mach_vm_deallocate_trap(
       mach_vm_map_t target,
       vm_address_t address,
       mach_vm_size_t size
);
kern_return_t _kernelrpc_mach_vm_map_trap(
	mach_vm_map_t target,
	vm_address_t *address,
	mach_vm_offset_t mask,
	mach_vm_size_t size,
	int flags,
	vm_prot_t cur_protection
	);

mach_port_t host_self_trap(void);
mach_port_t thread_self_trap(void);


kern_return_t vm_allocate(mach_port_name_t target, vm_address_t *addr, vm_size_t size, int flags);
kern_return_t vm_deallocate(mach_port_name_t target, vm_address_t addr, vm_size_t size);
mach_port_t pthread_mach_thread_np(uintptr_t self);


kern_return_t
mach_port_allocate(mach_port_name_t target, mach_port_right_t right,
				   mach_port_name_t *name)
{

	return (_kernelrpc_mach_port_allocate_trap(target, right, name));
}

kern_return_t
mach_port_deallocate(mach_port_name_t target, mach_port_name_t name)
{

	return (_kernelrpc_mach_port_deallocate_trap(target, name));
}

kern_return_t
mach_port_extract_right(mach_port_name_t target __unused,
						mach_port_name_t name __unused,
						mach_msg_type_name_t msgt_name __unused,
						mach_port_name_t *poly __unused,
						mach_msg_type_name_t *polyPoly __unused)
{

	/* no syscall for this - needs to go through the normal RPC glue */
	return (ENOSYS);
}

kern_return_t
mach_port_insert_right(mach_port_name_t target, mach_port_name_t name,
					   mach_port_name_t poly, mach_msg_type_name_t polyPoly)
{

	return (_kernelrpc_mach_port_insert_right_trap(target, name, poly, polyPoly));
}

kern_return_t
mach_port_mod_refs(mach_port_name_t target, mach_port_name_t name,
					   mach_port_right_t right, mach_port_delta_t delta)
{

	return (_kernelrpc_mach_port_mod_refs_trap(target, name, right, delta));
}

mach_port_t
mach_host_self(void)
{

	return (host_self_trap());
}

kern_return_t
mach_vm_allocate(mach_port_name_t target, vm_address_t *addr, vm_size_t size, int flags)
{

	return (_kernelrpc_mach_vm_allocate_trap(target, addr, size, flags));
}

kern_return_t
vm_allocate(mach_port_name_t target, vm_address_t *addr, vm_size_t size, int flags)
{

	return (_kernelrpc_mach_vm_allocate_trap(target, addr, size, flags));
}

kern_return_t
mach_vm_deallocate(mach_port_name_t target, vm_address_t addr, vm_size_t size)
{

	return (_kernelrpc_mach_vm_deallocate_trap(target, addr, size));
}

kern_return_t
vm_deallocate(mach_port_name_t target, vm_address_t addr, vm_size_t size)
{

	return (_kernelrpc_mach_vm_deallocate_trap(target, addr, size));
}


kern_return_t
mach_vm_map(mach_vm_map_t target, mach_vm_address_t *address, mach_vm_offset_t mask,
			mach_vm_size_t size, int flags, mem_entry_name_port_t object __unused,
			memory_object_offset_t offset __unused, boolean_t copy __unused,
			vm_prot_t cur_protection, vm_prot_t max_protection __unused,
			vm_inherit_t inheritance __unused)
{

	return (_kernelrpc_mach_vm_map_trap(target, address, size, mask, flags, cur_protection));
}

kern_return_t
mach_port_move_member(mach_port_name_t target, mach_port_name_t member, mach_port_name_t after)
{

	return (_kernelrpc_mach_port_move_member_trap(target, member, after));
}

kern_return_t
mach_port_insert_member(mach_port_name_t target, mach_port_name_t name, mach_port_name_t pset)
{

	return (_kernelrpc_mach_port_insert_member_trap(target, name, pset));
}


kern_return_t
mach_port_extract_member(mach_port_name_t target, mach_port_name_t name, mach_port_name_t pset)
{

	return (_kernelrpc_mach_port_extract_member_trap(target, name, pset));
}

#include <time.h>
#include <mach/mach_time.h>
uint64_t
mach_absolute_time(void)
{
	struct timespec tp;

	if (clock_gettime(CLOCK_REALTIME_FAST, &tp))
		return (0);

	return (tp.tv_sec*NSEC_PER_SEC + tp.tv_nsec);
}

mach_port_t
pthread_mach_thread_np(uintptr_t self)
{

	if ((pthread_t)self != pthread_self()) {
		printf("invalid pthread_mach_thread_np usage");
		abort();
	}

	return (thread_self_trap());
}

#ifndef _MACH_MACH_TRAPS_H_
#define _MACH_MACH_TRAPS_H_
#include <sys/mach/mach_traps.h>

kern_return_t mach_port_allocate(
	ipc_space_t task,
	mach_port_right_t right,
	mach_port_name_t *name
);

kern_return_t mach_port_mod_refs(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_right_t right,
	mach_port_delta_t delta
);

kern_return_t mach_port_insert_right(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_t poly,
	mach_msg_type_name_t polyPoly
);

kern_return_t mach_port_extract_right(
	ipc_space_t task,
	mach_port_name_t name,
	mach_msg_type_name_t msgt_name,
	mach_port_t *poly,
	mach_msg_type_name_t *polyPoly
);

kern_return_t mach_port_deallocate(	
	ipc_space_t task,
	mach_port_name_t name
);

kern_return_t mach_port_move_member(
	mach_port_name_t target,
	mach_port_name_t member,
	mach_port_name_t after
	);

kern_return_t mach_port_insert_member(
	mach_port_name_t target,
	mach_port_name_t member,
	mach_port_name_t after
	);

kern_return_t mach_port_extract_member(
	mach_port_name_t target,
	mach_port_name_t member,
	mach_port_name_t after
);

kern_return_t mach_vm_allocate(
       mach_vm_map_t target,
       mach_vm_address_t *address,
       mach_vm_size_t size,
       int flags
	);

kern_return_t mach_vm_deallocate(	
       mach_vm_map_t target,
       mach_vm_address_t address,
       mach_vm_size_t size
	);

kern_return_t mach_vm_map(mach_vm_map_t target, mach_vm_address_t *address, mach_vm_offset_t mask,
			mach_vm_size_t size, int flags, mem_entry_name_port_t object __unused,
			memory_object_offset_t offset __unused, boolean_t copy __unused,
			vm_prot_t cur_protection, vm_prot_t max_protection __unused,
			vm_inherit_t inheritance __unused);

mach_port_t mach_reply_port(void);
mach_port_t mach_reply_port(void);
#endif

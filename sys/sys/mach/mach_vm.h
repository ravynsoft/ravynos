#ifndef MACH_VM_H_
#define MACH_VM_H_
#include <vm/vm.h>
#include <vm/vm_object.h>

struct vm_map_links {
	struct vm_map_entry	*prev;		/* previous entry */
	struct vm_map_entry	*next;		/* next entry */
	vm_offset_t		start;		/* start address */
	vm_offset_t		end;		/* end address */
};

/*
 *	Type:		struct vm_map_header
 *
 *	Description:
 *		Header for a vm_map and a vm_map_copy.
 */
struct vm_map_header {
	struct vm_map_links	links;		/* first, last, min, max */
	int			nentries;	/* Number of entries */
	boolean_t		entries_pageable;
						/* are map entries pageable? */
};


struct vm_map_copy {
	int			type;
#define VM_MAP_COPY_ENTRY_LIST		1
#define VM_MAP_COPY_OBJECT		2
#define VM_MAP_COPY_OBJECT_PREALLOC 3
#define VM_MAP_COPY_KERNEL_BUFFER	4
	vm_offset_t		offset;
	vm_size_t		size;
	union {
	    struct vm_map_header	hdr;	/* ENTRY_LIST */
		vm_object_t		object; /* OBJECT */
	    struct {				/* KERNEL_BUFFER */
		vm_offset_t		kdata;
		vm_size_t		kalloc_size;  /* size of this copy_t */
	    } c_k;
	} c_u;
};

#define cpy_hdr			c_u.hdr

#define cpy_object		c_u.object
#define	cpy_index		c_u.c_o.index

#define cpy_kdata		c_u.c_k.kdata
#define cpy_kalloc_size		c_u.c_k.kalloc_size

#define	VM_MAP_COPY_NULL	((vm_map_copy_t) 0)
#define	VM_MAP_NULL		((vm_map_t) 0)

#define vm_map_copy_to_entry(copy)		\
		((struct vm_map_entry *) &(copy)->cpy_hdr.links)
#define vm_map_copy_first_entry(copy)		\
		((copy)->cpy_hdr.links.next)
#define vm_map_copy_last_entry(copy)		\
		((copy)->cpy_hdr.links.prev)

typedef struct vm_map_copy *vm_map_copy_t;

/* Discard a copy without using it */
extern void		vm_map_copy_discard(
				vm_map_copy_t		copy);

/* Overwrite existing memory with a copy */
extern kern_return_t	vm_map_copy_overwrite(
				vm_map_t                dst_map,
				vm_map_address_t        dst_addr,
				vm_map_copy_t           copy,
				boolean_t               interruptible);

/* Place a copy into a map */
extern kern_return_t	vm_map_copyout(
				vm_map_t		dst_map,
				vm_map_address_t	*dst_addr,	/* OUT */
				vm_map_copy_t		copy);

extern kern_return_t	vm_map_copyin(
				vm_map_t			src_map,
				vm_map_address_t	src_addr,
				vm_map_size_t		len,
				boolean_t			src_destroy,
				vm_map_copy_t		*copy_result);	/* OUT */

extern kern_return_t	vm_map_copyin_common(
				vm_map_t		src_map,
				vm_map_address_t	src_addr,
				vm_map_size_t		len,
				boolean_t		src_destroy,
				boolean_t		src_volatile,
				vm_map_copy_t		*copy_result,	/* OUT */
				boolean_t		use_maxprot);

extern kern_return_t	vm_map_copy_extract(
	vm_map_t		src_map,
	vm_map_address_t	src_addr,
	vm_map_size_t		len,
	vm_map_copy_t		*copy_result,	/* OUT */
	vm_prot_t		*cur_prot,	/* OUT */
	vm_prot_t		*max_prot);


int mach_vm_allocate(vm_map_t map, vm_offset_t *addr, size_t _size, int flags);
int mach_vm_deallocate(vm_map_t target __unused, mach_vm_address_t addr, mach_vm_size_t len);

int
mach_vm_map(vm_map_t map, mach_vm_address_t *address, mach_vm_size_t _size,
			mach_vm_offset_t _mask, int _flags, mem_entry_name_port_t object __unused,
			memory_object_offset_t offset __unused, boolean_t copy __unused,
			vm_prot_t cur_protection, vm_prot_t max_protection, vm_inherit_t inh);

#endif /* MACH_VM_H_ */

/*-
 * Copyright (c) 2014-2015, Matthew Macy <mmacy@nextbsd.org>
 * Copyright (c) 2021-2022 Zoe Knox <zoe@pixin.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Neither the name of Matthew Macy nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/mman.h>
#include <sys/malloc.h>
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/exec.h>
#include <sys/sysproto.h>
#include <sys/uio.h>
#include <sys/vnode.h>

#include <sys/ktrace.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_extern.h>
#include <vm/vm_kern.h>

#include <sys/mach/mach_types.h>
#include <sys/mach/mach_vm.h>
#include <sys/mach/ipc/ipc_kmsg.h>
#include <sys/mach/thread.h>
#include <sys/mach/mach_vm_server.h>
#include <sys/mach/vm_map_server.h>
#include <sys/mach/host_priv_server.h>


extern vm_size_t	msg_ool_size_small;

int mach_vm_map_page_query(vm_map_t target_map, vm_offset_t offset, integer_t *disposition, integer_t *ref_count);
int mach_vm_mapped_pages_info(vm_map_t task, page_address_array_t *pages, mach_msg_type_number_t *pagesCnt);
int mach_vm_wire_32(
	host_priv_t host_priv,
	vm_map_t task,
	vm_address_t address,
	vm_size_t size,
	vm_prot_t desired_access
	);


#ifdef notused
static kern_return_t
vm_map_copyout_internal(
	register vm_map_t	dst_map,
	vm_offset_t		*dst_addr,	/* OUT */
	register vm_map_copy_t	copy,
	boolean_t consume_on_success,
	vm_prot_t cur_protection,
	vm_prot_t max_protection,
	vm_inherit_t inheritance);


#ifdef INVARIANTS
static int
first_free_is_valid(vm_map_t map)
{
	return TRUE;
}
#endif
#endif

int
mach_vm_map(vm_map_t map, mach_vm_address_t *address, mach_vm_size_t _size,
			mach_vm_offset_t _mask, int _flags, mem_entry_name_port_t object __unused,
			memory_object_offset_t offset __unused, boolean_t copy __unused,
			vm_prot_t cur_protection, vm_prot_t max_protection, vm_inherit_t inh)
{
	vm_offset_t addr = 0;
	size_t size;
	int docow, error, find_space;

	/* XXX Darwin fails on mapping a page at address 0 */
	if ((_flags & VM_FLAGS_ANYWHERE) == 0 && *address == 0)
		return (ENOMEM);

	size = round_page(_size);
	docow = error = 0;

	if (!(_mask & (_mask + 1)) && _mask != 0)
		_mask++;

	find_space = _mask ? VMFS_ALIGNED_SPACE(ffs(_mask)) : VMFS_ANY_SPACE;
	//flags = MAP_ANON;
	if ((_flags & VM_FLAGS_ANYWHERE) == 0) {
	//	flags |= MAP_FIXED;
		addr = trunc_page(*address);
	} else
		addr = 0;

	switch(inh) {
	case VM_INHERIT_SHARE:
	//	flags |= MAP_INHERIT_SHARE;
		break;
	case VM_INHERIT_COPY:
	//	flags |= MAP_COPY_ON_WRITE;
		docow = MAP_COPY_ON_WRITE;
		break;
	case VM_INHERIT_NONE:
		break;
	case VM_INHERIT_DONATE_COPY:
	default:
		uprintf("mach_vm_map: unsupported inheritance flag %d\n", inh);
		break;
	}

	if (vm_map_find(map, NULL, 0, &addr, size, 0, find_space,
	    cur_protection, max_protection, docow) != KERN_SUCCESS) {
		error = ENOMEM;
		goto done;
	}

	*address = addr;
done:
	return (error);
}

int
vm_allocate(vm_map_t map, vm_offset_t *addr, size_t _size, int flags)
{

	return (mach_vm_allocate(map, addr, _size, flags));
}

int
mach_vm_allocate(vm_map_t map, vm_offset_t *addr, size_t _size, int flags)
{
	size_t size = round_page(_size);
	vm_offset_t start, daddr;
	vm_prot_t prot, protmax;
	int err;

	prot = VM_PROT_READ|VM_PROT_WRITE;
	protmax = VM_PROT_ALL;
	KASSERT(addr != NULL, ("invalid addr passed"));
#if defined(INVARIANTS) && defined(__LP64__)
	start = 1UL<<32;
#else
	start = 0;
#endif

	daddr = trunc_page(*addr);
	vm_map_lock(map);
	if ((flags & VM_FLAGS_ANYWHERE) &&
		((daddr = vm_map_findspace(map, start, size)) == (vm_map_max(map)-size+1) )) {
	  err = ENOMEM;
	  goto error;
	} else {
		err = EINVAL;
		/* Address range must be all in user VM space. */
		if (daddr < vm_map_min(map) ||
		    daddr + size > vm_map_max(map))
		  goto error;
		if (daddr + size < daddr)
		  goto error;
	}
	if (vm_map_insert(map, NULL, 0, daddr, daddr + size, prot, protmax, 0)) {
	  err = EFAULT;
	  goto error;
	}
	vm_map_unlock(map);
	*addr = daddr;
	return (0);
 error:
	vm_map_unlock(map);
	return (err);
}

int
vm_deallocate(vm_map_t map, vm_offset_t addr, size_t len)
{

	return (mach_vm_deallocate(map, addr, len));
}

int
mach_vm_deallocate(vm_map_t target __unused, mach_vm_address_t addr, mach_vm_size_t len)
{
	struct munmap_args cup;

	cup.addr = (void *)addr;
	cup.len = len;
	return (sys_munmap(curthread, &cup));
}

int
mach_vm_protect(vm_map_t target_task __unused, vm_offset_t addr, size_t len,
				boolean_t setmax __unused, vm_prot_t prot)
{
	struct mprotect_args cup;

	cup.addr = (void *)addr;
	cup.len = len;
	cup.prot = prot;

	return (sys_mprotect(curthread, &cup));
}


int
mach_vm_inherit(vm_map_t target_task, mach_vm_address_t address, mach_vm_size_t size,
				vm_inherit_t new_inheritance)
{
	struct minherit_args cup;
	int error;

	cup.addr = (void *)address;
	cup.len = size;
	cup.inherit = new_inheritance;

	if ((error = sys_minherit(curthread, &cup)) != 0)
		return (KERN_FAILURE);

	return (0);
}

int
mach_vm_map_page_query(
	vm_map_t target_map,
	vm_offset_t offset,
	integer_t *disposition,
	integer_t *ref_count
)
{
	return (KERN_NOT_SUPPORTED);
}

int
_mach_make_memory_entry(
	vm_map_t target_task,
	memory_object_size_t *size,
	memory_object_offset_t offset,
	vm_prot_t permission,
	mem_entry_name_port_t *object_handle,
	mem_entry_name_port_t parent_handle
)
{	

	return (KERN_NOT_SUPPORTED);
}

int
mach_make_memory_entry(vm_map_t target_task, vm_size_t *size,
					   vm_offset_t offset, vm_prot_t permission,
					   mem_entry_name_port_t *object_handle,
					   mem_entry_name_port_t parent_handle)
{	

	return (KERN_NOT_SUPPORTED);
}


int
mach_make_memory_entry_64(vm_map_t target_task, memory_object_size_t *size,
						  memory_object_offset_t offset, vm_prot_t permission,
						  mach_port_t *object_handle,
						  mem_entry_name_port_t parent_handle)
{	

	return (KERN_NOT_SUPPORTED);
}


int
mach_vm_msync(vm_map_t target_task __unused, mach_vm_address_t addr, mach_vm_size_t size,
		vm_sync_t flags)
{
	struct msync_args cup;
	int error;

	cup.addr = (void *)addr;
	cup.len = size;
	cup.flags = 0;
	if (flags & VM_SYNC_ASYNCHRONOUS)
		cup.flags |= MS_ASYNC;
	if (flags & VM_SYNC_SYNCHRONOUS)
		cup.flags |= MS_SYNC;
	if (flags & VM_SYNC_INVALIDATE)
		cup.flags |= MS_INVALIDATE;
	error = sys_msync(curthread, &cup);
	
	if (error)
		return (KERN_FAILURE);
	return (0);
}

/* XXX Do it for remote task */
int
mach_vm_copy(vm_map_t target_task, mach_vm_address_t src, mach_vm_size_t size,
		mach_vm_address_t dst)
		
{
	char *tmpbuf;
	int error;

#ifdef DEBUG_MACH_VM
	printf("mach_vm_copy: src = 0x%08lx, size = 0x%08lx, addr = 0x%08lx\n",
	    (long)req->req_src, (long)req->req_size, (long)req->req_addr);
#endif
	if ((src & (PAGE_SIZE - 1)) ||
	    (dst & (PAGE_SIZE - 1)) ||
	    (size & (PAGE_SIZE - 1)))
		return (KERN_INVALID_ARGUMENT);


	tmpbuf = malloc(PAGE_SIZE, M_TEMP, M_WAITOK);

	/* Is there an easy way of dealing with that efficiently? */
	do {
		if ((error = copyin((void*)src, tmpbuf, PAGE_SIZE)) != 0)
			goto out;

		if ((error = copyout(tmpbuf, (void *)dst, PAGE_SIZE)) != 0)
			goto out;

		src += PAGE_SIZE;
		dst += PAGE_SIZE;
		size -= PAGE_SIZE;
	} while (size > 0);

	free(tmpbuf, M_TEMP);
	return (0);

out:
	free(tmpbuf, M_TEMP);
	return (KERN_FAILURE);
}

int
mach_vm_read(vm_map_t map, mach_vm_address_t addr, mach_vm_size_t size,
			 vm_offset_t *data, mach_msg_type_number_t *dataCnt)
{
	caddr_t tbuf;
	vm_offset_t dstaddr;
	int error;

	size = round_page(size);

	if ((error = mach_vm_allocate(map, &dstaddr, size, 0)))
		return (KERN_NO_SPACE);
	/*
	 * Copy the data from the target process to the current process
	 * This is reasonable for small chunk of data, but we should
	 * remap COW for areas bigger than a page.
	 */
	tbuf = malloc(size, M_MACH_TMP, M_WAITOK);
#ifdef notyet
	if ((error = copyin_vm_map(map, (caddr_t)addr, tbuf, size)) != 0) {
		printf("copyin_proc error = %d, addr = %lx, size = %zx\n", error, addr, size);
		free(tbuf, M_MACH_TMP);
		return (KERN_PROTECTION_FAILURE);
	}
#endif
	if ((error = copyout(tbuf, (void *)dstaddr, size)) != 0) {
		printf("copyout error = %d\n", error);
		free(tbuf, M_MACH_TMP);
		return (KERN_PROTECTION_FAILURE);
	}

	free(tbuf, M_MACH_TMP);
	return (0);
}


/*
 * mach_vm_write -
 * Overwrite the specified address range with the data provided
 * (from the current map).
 */
kern_return_t
mach_vm_write(
	vm_map_t			map,
	mach_vm_address_t		address,
	vm_offset_t			data,
	mach_msg_type_number_t	size __unused)
{
	if (map == VM_MAP_NULL)
		return KERN_INVALID_ARGUMENT;

	return vm_map_copy_overwrite(map, (vm_map_address_t)address,
		(vm_map_copy_t) data, FALSE /* interruptible XXX */);
}

int
mach_vm_machine_attribute(vm_map_t target_task, mach_vm_address_t addr, mach_vm_size_t size,
						  vm_machine_attribute_t attribute, vm_machine_attribute_val_t *valuep)
{
	int error = 0;
	vm_machine_attribute_val_t value;

	
	if ((error = copyin(valuep, &value, sizeof(value))))
		return (KERN_PROTECTION_FAILURE);

	switch (attribute) {
	case MATTR_CACHE:
		switch(value) {
		case MATTR_VAL_CACHE_FLUSH:
		case MATTR_VAL_DCACHE_FLUSH:
		case MATTR_VAL_ICACHE_FLUSH:
		case MATTR_VAL_CACHE_SYNC:
#ifdef notyet
			error = cpu_mach_vm_machine_attribute(target_task, addr, size, &value);
#endif
			break;
		default:
#ifdef DEBUG_MACH
			printf("unimplemented value %d\n", value);
#endif
			error = EINVAL;
			break;
		}
		break;

	case MATTR_MIGRATE:
	case MATTR_REPLICATE:
	default:
#ifdef DEBUG_MACH
		printf("unimplemented attribute %d\n", attribute);
#endif
		error = EINVAL;
		break;
	}


	if (error)
		return (KERN_FAILURE);

	return (copyout(&value, valuep, sizeof(value)));
}

/*
 *	vm_map_entry_dispose:	[ internal use only ]
 *
 *	Inverse of vm_map_entry_create.
 */
#define	vm_map_entry_dispose(map, entry)		\
MACRO_BEGIN						\
	assert((entry) != (map)->first_free &&		\
	       (entry) != (map)->hint);			\
	_vm_map_entry_dispose(&(map)->hdr, (entry));	\
MACRO_END

#define	vm_map_copy_entry_dispose(map, entry) \
	_vm_map_entry_dispose(&(copy)->cpy_hdr, (entry))

#ifdef notused
static uma_zone_t vm_map_entry_zone;
static uma_zone_t vm_map_kentry_zone;

static void
_vm_map_entry_dispose(
	register struct vm_map_header	*map_header,
	register vm_map_entry_t		entry)
{
	uma_zone_t		zone;

	if (map_header->entries_pageable)
	    zone = vm_map_entry_zone;
	else
	    zone = vm_map_kentry_zone;

	uma_zfree(zone, entry);
}
#endif

/*
 *	Routine: vm_map_copyin_internal [internal use only]
 *
 *	Description:
 *		Copy in data to a kernel buffer from space in the
 *		source map. The original space may be optionally
 *		deallocated.
 *
 *		If successful, returns a new copy object.
 */
static kern_return_t
vm_map_copyin_internal(
	vm_map_t	src_map,
	vm_map_offset_t	src_addr,
	vm_map_size_t	len,
	boolean_t	src_destroy,
	vm_map_copy_t	*copy_result)
{
	kern_return_t kr;
	vm_map_copy_t copy;
	vm_size_t size;

	if ((vm_size_t) len != len) {
		/* "len" is too big and doesn't fit in a "vm_size_t" */
		return KERN_RESOURCE_SHORTAGE;
	}
	size =  (vm_size_t) (sizeof(struct vm_map_copy) + len);
	copy = (vm_map_copy_t) malloc(size, M_MACH_VM, M_NOWAIT|M_ZERO);
	if (copy == VM_MAP_COPY_NULL) {
		kr = KERN_RESOURCE_SHORTAGE;
		goto fail;
	}
	copy->type = VM_MAP_COPY_KERNEL_BUFFER;
	copy->size = len;
	copy->offset = 0;
	copy->cpy_kalloc_size = size;
	copy->cpy_kdata = (vm_offset_t) (copy + 1);

	if (src_map == kernel_map) {
		memcpy((void *)copy->cpy_kdata, (void *)src_addr, len);
		free((void *)src_addr, M_MACH_VM);
	} else {

		kr = copyinmap(src_map, src_addr, copy->cpy_kdata, (vm_size_t) len);
		if (kr != KERN_SUCCESS) {
			free(copy, M_MACH_VM);
			return kr;
		}
		if (src_destroy) {
			(void) vm_map_remove(
				src_map,
				trunc_page(src_addr),
				round_page(src_addr + len));
		}
	}
	*copy_result = copy;
	return KERN_SUCCESS;
fail:
	if (src_map == kernel_map)
		free((void *)src_addr, M_MACH_VM);
	return (kr);
}

/*
 *	Routine:	vm_map_copyout_kernel_buffer
 *
 *	Description:
 *		Copy out data from a kernel buffer into space in the
 *		destination map. The space may be otpionally dynamically
 *		allocated.
 *
 *		If successful, consumes the copy object.
 *		Otherwise, the caller is responsible for it.
 */
static kern_return_t
vm_map_copyout_kernel_buffer(
	vm_map_t	map,
	vm_offset_t	*addr,	/* IN/OUT */
	vm_map_copy_t	copy,
	boolean_t	overwrite)
{
	kern_return_t kr = KERN_SUCCESS;

	if (!overwrite) {

		/*
		 * Allocate space in the target map for the data
		 */
		*addr = 0;
		kr = mach_vm_allocate(map, addr, round_page(copy->size), VM_FLAGS_ANYWHERE);
		if (kr != KERN_SUCCESS)
			return(kr);
	}

	/*
	 * Copyout the data from the kernel buffer to the target map.
	 */
	if (current_map() == map) {
		/*
		 * If the target map is the current map, just do
		 * the copy.
		 */
		if (copyout((char *)copy->cpy_kdata, (char *)*addr,
				copy->size)) {
			kr = KERN_INVALID_ADDRESS;
		}
	}
	else {
		panic("unexpected map %p", map);
#if 0
		vm_map_t oldmap;

		/*
		 * If the target map is another map, assume the
		 * target's address space identity for the duration
		 * of the copy.
		 */
		vm_map_reference(map);
		oldmap = vm_map_switch(map);

		if (copyout((char *)copy->cpy_kdata, (char *)*addr,
				copy->size)) {
			kr = KERN_INVALID_ADDRESS;
		}

		(void) vm_map_switch(oldmap);
		vm_map_deallocate(map);
#endif
		kr = KERN_NOT_SUPPORTED;
	}

	free(copy, M_MACH_VM);

	return(kr);
}



/*
 *	vm_map_copyin_object:
 *
 *	Create a copy object from an object.
 *	Our caller donates an object reference.
 */

static kern_return_t
vm_map_copyin_object(
	vm_object_t	object,
	vm_offset_t	offset,		/* offset of region in object */
	vm_size_t	size,		/* size of region in object */
	vm_map_copy_t	*copy_result)	/* OUT */
{
	vm_map_copy_t	copy;		/* Resulting copy */

	/*
	 *	We drop the object into a special copy object
	 *	that contains the object directly.
	 */

	copy = (vm_map_copy_t) malloc(sizeof(vm_map_copy_t), M_MACH_VM, M_WAITOK|M_ZERO);
	copy->type = VM_MAP_COPY_OBJECT;
	copy->cpy_object = object;

	copy->offset = offset;
	copy->size = size;

	*copy_result = copy;
	return(KERN_SUCCESS);
}

kern_return_t	vm_map_copyin(
				vm_map_t			src_map,
				vm_map_address_t	src_addr,
				vm_map_size_t		len,
				boolean_t			src_destroy,
				vm_map_copy_t		*copy_result)
{
	vm_map_entry_t	tmp_entry;	/* Result of last map lookup --
					 * in multi-level lookup, this
					 * entry contains the actual
					 * vm_object/offset.
					 */
	vm_map_entry_t	prev_entry, next_entry;
	vm_object_t object;
	vm_offset_t prev_end;

	vm_offset_t	src_start;	/* Start of current entry --
					 * where copy is taking place now
					 */
	vm_offset_t src_end;
	vm_offset_t offset;

	/*
	 * three possibilities:
	 * - len is less than  MSG_OOL_SIZE_SMALL:
	 *   allocate memory and copyin with header
	 * - len is greater but src_map is the special copy map:
	 *   allocate just a header and point at src_addr
	 * - len is greater:
	 *     lookup entry
	 *     while entry does not cover full range && entry changed:
	 *       simplify entry
	 *     if entry edges extend outside of range:
	 *       clip
	 *     if no object:
	 *       add object
	 *     reference object
	 *     if src_destroy is not true:
	 *       mark entry needs COW && NEEDS_COPY
	 *       pmap_protect range
	 *       create shadow object
	 *     else:
	 *       split object
	 *       delete entry from source map
	 *       if data is not aligned copy edge pages and insert
	 *         into source map
	 *     create map copy object w/ object
	 */


	if (len == 0) {
		*copy_result = VM_MAP_COPY_NULL;
		return(KERN_SUCCESS);
	}

	src_start = trunc_page(src_addr);
	src_end = round_page(src_addr + len);
	if (src_end < src_addr)
		return KERN_INVALID_ADDRESS;

	if (len < msg_ool_size_small || src_map == kernel_map)
		return vm_map_copyin_internal(src_map, src_addr, len,
										   src_destroy, copy_result);

	/*
	 *	Find the beginning of the region.
	 */
	vm_map_lock(src_map);

	if (!vm_map_lookup_entry(src_map, src_start, &tmp_entry)) {
		vm_map_unlock(src_map);
		return KERN_INVALID_ADDRESS;
	}

	prev_end = 0;
	while (prev_end != tmp_entry->end  && tmp_entry->end < src_end) {
		prev_end = tmp_entry->end;
		prev_entry = vm_map_entry_pred(tmp_entry);
		next_entry = vm_map_entry_succ(tmp_entry);
		vm_map_try_merge_entries(src_map, prev_entry, tmp_entry);
		vm_map_try_merge_entries(src_map, tmp_entry, next_entry);
	}
	/* only handle single map entry for now */
	if (tmp_entry->end < src_end) {
		vm_map_unlock(src_map);
		return KERN_NOT_SUPPORTED;
	}

	object = tmp_entry->object.vm_object;

	if (src_destroy) {
		/*
		 * The original code seems to be keeping the vm_object and pages
		 * to create the copy object, but removing the object from src_map
		 * by setting vm_object to NULL before vm_map_delete(). This smells
		 * like a hack and causes a kernel page fault in 12.x.
		 *
		 * Instead, let's create a new entry with new_object and insert
		 * it into src_map at new_addr. This seems to work and follows
		 * the spirit of the original, destroying the memory at src_addr
		 * while keeping a copy. Hopefully this is not a leak.
		 *     -- mszoek
		 */
		offset = 0;
		vm_map_clip_start(src_map, tmp_entry, src_start);
		vm_map_clip_end(src_map, tmp_entry, src_end);
		VM_OBJECT_WLOCK(object);
		vm_object_reference_locked(object);
		vm_object_split(tmp_entry);
		/* this is now 'new_object' from vm_object_split */
		object = tmp_entry->object.vm_object;
		vm_object_reference_locked(object);
		VM_OBJECT_WUNLOCK(object);
		unsigned long size = src_end - src_start;
		vm_offset_t new_addr = vm_map_findspace(src_map, 0, size);
		vm_object_reference(object);
		vm_map_insert(src_map, object, offset, new_addr, 
					new_addr + size, VM_PROT_READ|VM_PROT_WRITE,
					VM_PROT_READ|VM_PROT_WRITE, 0);
		vm_map_unlock(src_map);
		vm_map_remove(src_map, src_start, src_end);
	} else {
		offset = tmp_entry->offset;
		vm_map_unlock(src_map);
		vm_object_reference(object);
		vm_map_protect(src_map, src_start, src_end, tmp_entry->protection & ~VM_PROT_WRITE, 0, VM_MAP_PROTECT_SET_PROT);
		tmp_entry->eflags |= MAP_ENTRY_NEEDS_COPY | MAP_ENTRY_COW;
	}

	vm_map_copyin_object(object, offset, src_end - src_start, copy_result);
	/* neither mach nor osx does anything to prevent information leakage
	 * in unaligned sends
	 */
	return KERN_SUCCESS;
}



/*
 *	Routine:	vm_map_copyout
 *
 *	Description:
 *		Copy out a copy chain ("copy") into newly-allocated
 *		space in the destination map.
 *
 *		If successful, consumes the copy object.
 *		Otherwise, the caller is responsible for it.
 */


kern_return_t
vm_map_copyout(
	vm_map_t		dst_map,
	vm_map_address_t	*dst_addr,	/* OUT */
	vm_map_copy_t		copy)
{
	int cow;
	vm_object_t object;

	*dst_addr = 0;
/*
	 *	Check for special kernel buffer allocated
	 *	by new_ipc_kmsg_copyin.
	 */
	if (copy->type == VM_MAP_COPY_KERNEL_BUFFER) {
		return (vm_map_copyout_kernel_buffer(dst_map, dst_addr,
						    copy, FALSE));
	}
	if (copy->type == VM_MAP_COPY_OBJECT) {
		*dst_addr = 0;
		object = copy->cpy_object;
		vm_object_reference(copy->cpy_object);
		cow = object->ref_count > 1 ? MAP_COPY_ON_WRITE : 0;
		return vm_map_find(dst_map, copy->cpy_object, copy->offset,
		    dst_addr, round_page(copy->size), 0, VMFS_OPTIMAL_SPACE,
		    VM_PROT_READ|VM_PROT_WRITE, VM_PROT_READ|VM_PROT_WRITE,
		    cow);
	}
	return KERN_NOT_SUPPORTED;
#if 0
	return vm_map_copyout_internal(dst_map, dst_addr, copy,
				       TRUE, /* consume_on_success */
				       VM_PROT_DEFAULT,
				       VM_PROT_ALL,
				       VM_INHERIT_DEFAULT);
#endif
}

#if 0
static kern_return_t
vm_map_copyout_internal(
	register vm_map_t	dst_map,
	vm_offset_t		*dst_addr,	/* OUT */
	register vm_map_copy_t	copy,
	boolean_t consume_on_success,
	vm_prot_t cur_protection,
	vm_prot_t max_protection,
	vm_inherit_t inheritance)
{
	vm_size_t	size;
	vm_size_t	adjustment;
	vm_offset_t	start;
	vm_offset_t	vm_copy_start;
	vm_map_entry_t	last;
	vm_map_entry_t	entry;

	/*
	 *	Check for null copy object.
	 */

	if (copy == VM_MAP_COPY_NULL) {
		*dst_addr = 0;
		return(KERN_SUCCESS);
	}

	/*
	 *	Check for special copy object, created
	 *	by vm_map_copyin_object.
	 */

	if (copy->type == VM_MAP_COPY_OBJECT) {
		vm_object_t object = copy->cpy_object;
		kern_return_t kr;
		vm_size_t offset;

		offset = trunc_page(copy->offset);
		size = round_page(copy->size + copy->offset - offset);
		*dst_addr = 0;
		kr = vm_map_enter(dst_map, dst_addr, size,
				  (vm_offset_t) 0, TRUE,
				  object, offset, FALSE,
				  VM_PROT_DEFAULT, VM_PROT_ALL,
				  VM_INHERIT_DEFAULT);
		if (kr != KERN_SUCCESS)
			return(kr);
		/* Account for non-pagealigned copy object */
		*dst_addr += copy->offset - offset;
		uma_zfree(vm_map_copy_zone, copy);
		return(KERN_SUCCESS);
	}

	/*
	 *	Check for special kernel buffer allocated
	 *	by new_ipc_kmsg_copyin.
	 */

	if (copy->type == VM_MAP_COPY_KERNEL_BUFFER) {
		return(vm_map_copyout_kernel_buffer(dst_map, dst_addr, 
						    copy, FALSE));
	}

	/*
	 *	Find space for the data
	 */

	vm_copy_start = trunc_page(copy->offset);
	size =	round_page(copy->offset + copy->size) - vm_copy_start;

 StartAgain: ;

	vm_map_lock(dst_map);
	assert(first_free_is_valid(dst_map));
	start = ((last = dst_map->first_free) == vm_map_to_entry(dst_map)) ?
		vm_map_min(dst_map) : last->vme_end;

	while (TRUE) {
		vm_map_entry_t	next = last->vme_next;
		vm_offset_t	end = start + size;

		if ((end > dst_map->max_offset) || (end < start)) {
			if (dst_map->wait_for_space) {
				if (size <= (dst_map->max_offset - dst_map->min_offset)) {
					assert_wait((event_t) dst_map, TRUE);
					vm_map_unlock(dst_map);
					thread_block((void (*)(void))0);
					goto StartAgain;
				}
			}
			vm_map_unlock(dst_map);
			return(KERN_NO_SPACE);
		}

		if ((next == vm_map_to_entry(dst_map)) ||
		    (next->vme_start >= end))
			break;

		last = next;
		start = last->vme_end;
	}

	/*
	 *	Since we're going to just drop the map
	 *	entries from the copy into the destination
	 *	map, they must come from the same pool.
	 */

	if (copy->cpy_hdr.entries_pageable != dst_map->hdr.entries_pageable) {
	    /*
	     * Mismatches occur when dealing with the default
	     * pager.
	     */
	    zone_t		old_zone;
	    vm_map_entry_t	next, new;

	    /*
	     * Find the zone that the copies were allocated from
	     */
	    old_zone = (copy->cpy_hdr.entries_pageable)
			? vm_map_entry_zone
			: vm_map_kentry_zone;
	    entry = vm_map_copy_first_entry(copy);

	    /*
	     * Reinitialize the copy so that vm_map_copy_entry_link
	     * will work.
	     */
	    copy->cpy_hdr.nentries = 0;
	    copy->cpy_hdr.entries_pageable = dst_map->hdr.entries_pageable;
	    vm_map_copy_first_entry(copy) =
	     vm_map_copy_last_entry(copy) =
		vm_map_copy_to_entry(copy);

	    /*
	     * Copy each entry.
	     */
	    while (entry != vm_map_copy_to_entry(copy)) {
		new = vm_map_copy_entry_create(copy);
		vm_map_entry_copy_full(new, entry);
		vm_map_copy_entry_link(copy,
				vm_map_copy_last_entry(copy),
				new);
		next = entry->vme_next;
		uma_zfree(old_zone, entry);
		entry = next;
	    }
	}

	/*
	 *	Adjust the addresses in the copy chain, and
	 *	reset the region attributes.
	 */

	adjustment = start - vm_copy_start;
	for (entry = vm_map_copy_first_entry(copy);
	     entry != vm_map_copy_to_entry(copy);
	     entry = entry->vme_next) {
		entry->vme_start += adjustment;
		entry->vme_end += adjustment;

		entry->inheritance = VM_INHERIT_DEFAULT;
		entry->protection = VM_PROT_DEFAULT;
		entry->max_protection = VM_PROT_ALL;
		entry->behavior = VM_BEHAVIOR_DEFAULT;

		/*
		 * If the entry is now wired,
		 * map the pages into the destination map.
		 */
		if (entry->wired_count != 0) {
		    register vm_offset_t va;
		    vm_offset_t		 offset;
		    register vm_object_t object;

		    object = entry->object.vm_object;
		    offset = entry->offset;
		    va = entry->vme_start;

		    pmap_pageable(dst_map->pmap,
				  entry->vme_start,
				  entry->vme_end,
				  TRUE);

		    while (va < entry->vme_end) {
			register vm_page_t	m;

			/*
			 * Look up the page in the object.
			 * Assert that the page will be found in the
			 * top object:
			 * either
			 *	the object was newly created by
			 *	vm_object_copy_slowly, and has
			 *	copies of all of the pages from
			 *	the source object
			 * or
			 *	the object was moved from the old
			 *	map entry; because the old map
			 *	entry was wired, all of the pages
			 *	were in the top-level object.
			 *	(XXX not true if we wire pages for
			 *	 reading)
			 */
			vm_object_lock(object);
			vm_object_paging_begin(object);

			m = vm_page_lookup(object, offset);
			if (m == VM_PAGE_NULL || m->wire_count == 0 ||
			    m->absent)
			    panic("vm_map_copyout: wiring 0x%x", m);

			m->busy = TRUE;
			vm_object_unlock(object);

			PMAP_ENTER(dst_map->pmap, va, m,
				   entry->protection, TRUE);

			vm_object_lock(object);
			PAGE_WAKEUP_DONE(m);
			/* the page is wired, so we don't have to activate */
			vm_object_paging_end(object);
			vm_object_unlock(object);

			offset += PAGE_SIZE;
			va += PAGE_SIZE;
		    }
		}
		else if (size <= vm_map_aggressive_enter_max) {

			register vm_offset_t	va;
			vm_offset_t		offset;
			register vm_object_t	object;
			vm_prot_t		prot;

			object = entry->object.vm_object;
			if (object != VM_OBJECT_NULL) {

				offset = entry->offset;
				va = entry->vme_start;
				while (va < entry->vme_end) {
					register vm_page_t	m;
				    
					/*
					 * Look up the page in the object.
					 * Assert that the page will be found
					 * in the top object if at all...
					 */
					vm_object_lock(object);
					vm_object_paging_begin(object);

					if (((m = vm_page_lookup(object,
								 offset))
					     != VM_PAGE_NULL) &&
					    !m->busy && !m->fictitious &&
					    !m->absent && !m->error) {
						m->busy = TRUE;
						vm_object_unlock(object);

						/* honor cow obligations */
						prot = entry->protection;
						if (entry->needs_copy)
							prot &= ~VM_PROT_WRITE;

						PMAP_ENTER(dst_map->pmap, va, 
							   m, prot, FALSE);

						vm_object_lock(object);
						vm_page_lock_queues();
						if (!m->active && !m->inactive)
							vm_page_activate(m);
						vm_page_unlock_queues();
						PAGE_WAKEUP_DONE(m);
					}
					vm_object_paging_end(object);
					vm_object_unlock(object);

					offset += PAGE_SIZE;
					va += PAGE_SIZE;
				}
			}
		}
	}

	/*
	 *	Correct the page alignment for the result
	 */

	*dst_addr = start + (copy->offset - vm_copy_start);

	/*
	 *	Update the hints and the map size
	 */

	SAVE_HINT(dst_map, vm_map_copy_last_entry(copy));

	dst_map->size += size;

	/*
	 *	Link in the copy
	 */

	vm_map_copy_insert(dst_map, last, copy);

	vm_map_unlock(dst_map);

	/*
	 * XXX	If wiring_required, call vm_map_pageable
	 */

	return(KERN_SUCCESS);
}
#endif
/*
 *	Routine:	vm_map_copy_discard
 *
 *	Description:
 *		Dispose of a map copy object (returned by
 *		vm_map_copyin).
 */
void
vm_map_copy_discard(
	vm_map_copy_t	copy)
{

	if (copy == VM_MAP_COPY_NULL)
		return;

	if (copy->type == VM_MAP_COPY_OBJECT)
		vm_object_deallocate(copy->cpy_object);
	free(copy, M_MACH_VM);
}

kern_return_t
vm_map_copy_overwrite(vm_map_t                dst_map,
					  vm_map_address_t        dst_addr,
					  vm_map_copy_t           copy,
					  boolean_t               interruptible)
{
	return (KERN_NOT_SUPPORTED);
}

int
mach_vm_behavior_set(vm_map_t target_task, mach_vm_address_t address, mach_vm_size_t size, vm_behavior_t new_behavior)
{

	return (KERN_NOT_SUPPORTED);
}

int
mach_vm_page_info(vm_map_t target_task, mach_vm_address_t address,
				  vm_page_info_flavor_t flavor, vm_page_info_t info,
				  mach_msg_type_number_t *infoCnt)
{

		return (KERN_NOT_SUPPORTED);
}

int
mach_vm_page_query(
	vm_map_t target_map,
	mach_vm_offset_t offset,
	integer_t *disposition,
	integer_t *ref_count
)
{

		return (KERN_NOT_SUPPORTED);
}

int
mach_vm_purgable_control(
	vm_map_t target_task,
	mach_vm_address_t address,
	vm_purgable_t control,
	int *state
	)
{

	return (KERN_NOT_SUPPORTED);
}
	
int
mach_vm_read_list(
	vm_map_t target_task,
	mach_vm_read_entry_t data_list,
	natural_t count
	)
{
	return (KERN_NOT_SUPPORTED);
}


int
vm_read_list(
	vm_map_t target_task,
	vm_read_entry_t data_list,
	natural_t count
	)
{
	return (KERN_NOT_SUPPORTED);
}

int
mach_vm_read_overwrite(
	vm_map_t target_task,
	mach_vm_address_t address,
	mach_vm_size_t size,
	mach_vm_address_t data,
	mach_vm_size_t *outsize
)
{
	return (KERN_NOT_SUPPORTED);
}

int
mach_vm_region(
	vm_map_t target_task,
	mach_vm_address_t *address,
	mach_vm_size_t *size,
	vm_region_flavor_t flavor,
	vm_region_info_t info,
	mach_msg_type_number_t *infoCnt,
	mach_port_t *object_name
	)
{
	/*
	 * MACH_VM_REGION_BASIC_INFO is the only
	 * supported flavor in Darwin.
	 */

	return (KERN_NOT_SUPPORTED);
}

int
mach_vm_region_info(
	vm_map_t task,
	vm_address_t address,
	vm_info_region_t *region,
	vm_info_object_array_t *objects,
	mach_msg_type_number_t *objectsCnt
)
{
	return (KERN_NOT_SUPPORTED);
}

int
mach_vm_region_info_64(
	vm_map_t task,
	vm_address_t address,
	vm_info_region_64_t *region,
	vm_info_object_array_t *objects,
	mach_msg_type_number_t *objectsCnt
)
{
	return (KERN_NOT_SUPPORTED);
}

int
mach_vm_region_recurse(
	vm_map_t target_task,
	mach_vm_address_t *address,
	mach_vm_size_t *size,
	natural_t *nesting_depth,
	vm_region_recurse_info_t info,
	mach_msg_type_number_t *infoCnt
	)
{
	return (KERN_NOT_SUPPORTED);
}

int
mach_vm_mapped_pages_info(
	vm_map_t task,
	page_address_array_t *pages,
	mach_msg_type_number_t *pagesCnt
)
{
	return (KERN_NOT_SUPPORTED);
}

	
int
mach_vm_remap(
	vm_map_t target_task,
	mach_vm_address_t *target_address,
	mach_vm_size_t size,
	mach_vm_offset_t mask,
	int flags,
	vm_map_t src_task,
	mach_vm_address_t src_address,
	boolean_t copy,
	vm_prot_t *cur_protection,
	vm_prot_t *max_protection,
	vm_inherit_t inheritance
	)
{
	return (KERN_NOT_SUPPORTED);
}

int
mach_vm_wire(
	host_priv_t host_priv,
	vm_map_t task,
	mach_vm_address_t address,
	mach_vm_size_t size,
	vm_prot_t desired_access
)
{
	return (KERN_NOT_SUPPORTED);
}

int
mach_vm_wire_32(
	host_priv_t host_priv,
	vm_map_t task,
	vm_address_t address,
	vm_size_t size,
	vm_prot_t desired_access
)
{
	return (KERN_NOT_SUPPORTED);
}

int
task_wire(vm_map_t target_task, boolean_t must_wire)
{

	return (KERN_NOT_SUPPORTED);
}

int
vm_allocate_cpm
(
	host_priv_t host_priv,
	vm_map_t task,
	vm_address_t *address,
	vm_size_t size,
	int flags
)
{

	return (KERN_NOT_SUPPORTED);
}

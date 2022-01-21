#ifndef	_vm_map_server_
#define	_vm_map_server_

/* Module vm_map */

#if !defined(__i386__) && !defined(__amd64__)
#pragma GCC diagnostic ignored "-Wredundant-decls"
#endif

#include <sys/cdefs.h>
#include <sys/types.h>
#ifdef _KERNEL
#include <sys/mach/ndr.h>
#include <sys/mach/kern_return.h>
#include <sys/mach/notify.h>
#include <sys/mach/mach_types.h>
#include <sys/mach/message.h>
#include <sys/mach/mig_errors.h>
#else /* !_KERNEL */
#include <string.h>
#include <mach/ndr.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/notify.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <mach/mig_errors.h>
#endif /*_KERNEL */

#ifdef AUTOTEST
#ifndef FUNCTION_PTR_T
#define FUNCTION_PTR_T
typedef void (*function_ptr_t)(mach_port_t, char *, mach_msg_type_number_t);
typedef struct {
        char            *name;
        function_ptr_t  function;
} function_table_entry;
typedef function_table_entry   *function_table_t;
#endif /* FUNCTION_PTR_T */
#endif /* AUTOTEST */

#ifndef	vm_map_MSG_COUNT
#define	vm_map_MSG_COUNT	31
#endif	/* vm_map_MSG_COUNT */

#include <sys/mach/std_types.h>
#include <sys/mach/mig.h>
#include <sys/mach/ipc_sync.h>
#include <sys/mach/ipc/ipc_voucher.h>
#include <sys/mach/ipc_host.h>
#include <sys/mach/ipc_tt.h>
#include <sys/mach/ipc_mig.h>
#include <sys/mach/mig.h>
#include <sys/mach/mach_types.h>
#include <sys/mach_debug/mach_debug_types.h>
#include <sys/mach/vm_types.h>

#ifdef __BeforeMigServerHeader
__BeforeMigServerHeader
#endif /* __BeforeMigServerHeader */


/* Routine vm_region */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_region
#if	defined(LINTLIBRARY)
    (target_task, address, size, flavor, info, infoCnt, object_name)
	vm_map_t target_task;
	vm_address_t *address;
	vm_size_t *size;
	vm_region_flavor_t flavor;
	vm_region_info_t info;
	mach_msg_type_number_t *infoCnt;
	mach_port_t *object_name;
{ return vm_region(target_task, address, size, flavor, info, infoCnt, object_name); }
#else
(
	vm_map_t target_task,
	vm_address_t *address,
	vm_size_t *size,
	vm_region_flavor_t flavor,
	vm_region_info_t info,
	mach_msg_type_number_t *infoCnt,
	mach_port_t *object_name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_allocate */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_allocate
#if	defined(LINTLIBRARY)
    (target_task, address, size, flags)
	vm_map_t target_task;
	vm_address_t *address;
	vm_size_t size;
	int flags;
{ return vm_allocate(target_task, address, size, flags); }
#else
(
	vm_map_t target_task,
	vm_address_t *address,
	vm_size_t size,
	int flags
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_deallocate */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_deallocate
#if	defined(LINTLIBRARY)
    (target_task, address, size)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
{ return vm_deallocate(target_task, address, size); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_protect */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_protect
#if	defined(LINTLIBRARY)
    (target_task, address, size, set_maximum, new_protection)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	boolean_t set_maximum;
	vm_prot_t new_protection;
{ return vm_protect(target_task, address, size, set_maximum, new_protection); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	boolean_t set_maximum,
	vm_prot_t new_protection
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_inherit */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_inherit
#if	defined(LINTLIBRARY)
    (target_task, address, size, new_inheritance)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	vm_inherit_t new_inheritance;
{ return vm_inherit(target_task, address, size, new_inheritance); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	vm_inherit_t new_inheritance
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_read */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_read
#if	defined(LINTLIBRARY)
    (target_task, address, size, data, dataCnt)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	vm_offset_t *data;
	mach_msg_type_number_t *dataCnt;
{ return vm_read(target_task, address, size, data, dataCnt); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	vm_offset_t *data,
	mach_msg_type_number_t *dataCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_read_list */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_read_list
#if	defined(LINTLIBRARY)
    (target_task, data_list, count)
	vm_map_t target_task;
	vm_read_entry_t data_list;
	natural_t count;
{ return vm_read_list(target_task, data_list, count); }
#else
(
	vm_map_t target_task,
	vm_read_entry_t data_list,
	natural_t count
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_write */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_write
#if	defined(LINTLIBRARY)
    (target_task, address, data, dataCnt)
	vm_map_t target_task;
	vm_address_t address;
	vm_offset_t data;
	mach_msg_type_number_t dataCnt;
{ return vm_write(target_task, address, data, dataCnt); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_offset_t data,
	mach_msg_type_number_t dataCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_copy */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_copy
#if	defined(LINTLIBRARY)
    (target_task, source_address, size, dest_address)
	vm_map_t target_task;
	vm_address_t source_address;
	vm_size_t size;
	vm_address_t dest_address;
{ return vm_copy(target_task, source_address, size, dest_address); }
#else
(
	vm_map_t target_task,
	vm_address_t source_address,
	vm_size_t size,
	vm_address_t dest_address
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_read_overwrite */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_read_overwrite
#if	defined(LINTLIBRARY)
    (target_task, address, size, data, outsize)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	vm_address_t data;
	vm_size_t *outsize;
{ return vm_read_overwrite(target_task, address, size, data, outsize); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	vm_address_t data,
	vm_size_t *outsize
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_msync */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_msync
#if	defined(LINTLIBRARY)
    (target_task, address, size, sync_flags)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	vm_sync_t sync_flags;
{ return vm_msync(target_task, address, size, sync_flags); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	vm_sync_t sync_flags
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_behavior_set */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_behavior_set
#if	defined(LINTLIBRARY)
    (target_task, address, size, new_behavior)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	vm_behavior_t new_behavior;
{ return vm_behavior_set(target_task, address, size, new_behavior); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	vm_behavior_t new_behavior
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_machine_attribute */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_machine_attribute
#if	defined(LINTLIBRARY)
    (target_task, address, size, attribute, value)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	vm_machine_attribute_t attribute;
	vm_machine_attribute_val_t *value;
{ return vm_machine_attribute(target_task, address, size, attribute, value); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	vm_machine_attribute_t attribute,
	vm_machine_attribute_val_t *value
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_remap */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_remap
#if	defined(LINTLIBRARY)
    (target_task, target_address, size, mask, flags, src_task, src_address, copy, cur_protection, max_protection, inheritance)
	vm_map_t target_task;
	vm_address_t *target_address;
	vm_size_t size;
	vm_address_t mask;
	int flags;
	vm_map_t src_task;
	vm_address_t src_address;
	boolean_t copy;
	vm_prot_t *cur_protection;
	vm_prot_t *max_protection;
	vm_inherit_t inheritance;
{ return vm_remap(target_task, target_address, size, mask, flags, src_task, src_address, copy, cur_protection, max_protection, inheritance); }
#else
(
	vm_map_t target_task,
	vm_address_t *target_address,
	vm_size_t size,
	vm_address_t mask,
	int flags,
	vm_map_t src_task,
	vm_address_t src_address,
	boolean_t copy,
	vm_prot_t *cur_protection,
	vm_prot_t *max_protection,
	vm_inherit_t inheritance
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_wire */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_wire
#if	defined(LINTLIBRARY)
    (target_task, must_wire)
	vm_map_t target_task;
	boolean_t must_wire;
{ return task_wire(target_task, must_wire); }
#else
(
	vm_map_t target_task,
	boolean_t must_wire
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_make_memory_entry */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_make_memory_entry
#if	defined(LINTLIBRARY)
    (target_task, size, offset, permission, object_handle, parent_entry)
	vm_map_t target_task;
	vm_size_t *size;
	vm_offset_t offset;
	vm_prot_t permission;
	mem_entry_name_port_t *object_handle;
	mem_entry_name_port_t parent_entry;
{ return mach_make_memory_entry(target_task, size, offset, permission, object_handle, parent_entry); }
#else
(
	vm_map_t target_task,
	vm_size_t *size,
	vm_offset_t offset,
	vm_prot_t permission,
	mem_entry_name_port_t *object_handle,
	mem_entry_name_port_t parent_entry
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_map_page_query */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_map_page_query
#if	defined(LINTLIBRARY)
    (target_map, offset, disposition, ref_count)
	vm_map_t target_map;
	vm_offset_t offset;
	integer_t *disposition;
	integer_t *ref_count;
{ return vm_map_page_query(target_map, offset, disposition, ref_count); }
#else
(
	vm_map_t target_map,
	vm_offset_t offset,
	integer_t *disposition,
	integer_t *ref_count
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_region_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_region_info
#if	defined(LINTLIBRARY)
    (task, address, region, objects, objectsCnt)
	vm_map_t task;
	vm_address_t address;
	vm_info_region_t *region;
	vm_info_object_array_t *objects;
	mach_msg_type_number_t *objectsCnt;
{ return mach_vm_region_info(task, address, region, objects, objectsCnt); }
#else
(
	vm_map_t task,
	vm_address_t address,
	vm_info_region_t *region,
	vm_info_object_array_t *objects,
	mach_msg_type_number_t *objectsCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_mapped_pages_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_mapped_pages_info
#if	defined(LINTLIBRARY)
    (task, pages, pagesCnt)
	vm_map_t task;
	page_address_array_t *pages;
	mach_msg_type_number_t *pagesCnt;
{ return vm_mapped_pages_info(task, pages, pagesCnt); }
#else
(
	vm_map_t task,
	page_address_array_t *pages,
	mach_msg_type_number_t *pagesCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_region_recurse */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_region_recurse
#if	defined(LINTLIBRARY)
    (target_task, address, size, nesting_depth, info, infoCnt)
	vm_map_t target_task;
	vm_address_t *address;
	vm_size_t *size;
	natural_t *nesting_depth;
	vm_region_recurse_info_t info;
	mach_msg_type_number_t *infoCnt;
{ return vm_region_recurse(target_task, address, size, nesting_depth, info, infoCnt); }
#else
(
	vm_map_t target_task,
	vm_address_t *address,
	vm_size_t *size,
	natural_t *nesting_depth,
	vm_region_recurse_info_t info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_region_recurse_64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_region_recurse_64
#if	defined(LINTLIBRARY)
    (target_task, address, size, nesting_depth, info, infoCnt)
	vm_map_t target_task;
	vm_address_t *address;
	vm_size_t *size;
	natural_t *nesting_depth;
	vm_region_recurse_info_t info;
	mach_msg_type_number_t *infoCnt;
{ return vm_region_recurse_64(target_task, address, size, nesting_depth, info, infoCnt); }
#else
(
	vm_map_t target_task,
	vm_address_t *address,
	vm_size_t *size,
	natural_t *nesting_depth,
	vm_region_recurse_info_t info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_region_info_64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_region_info_64
#if	defined(LINTLIBRARY)
    (task, address, region, objects, objectsCnt)
	vm_map_t task;
	vm_address_t address;
	vm_info_region_64_t *region;
	vm_info_object_array_t *objects;
	mach_msg_type_number_t *objectsCnt;
{ return mach_vm_region_info_64(task, address, region, objects, objectsCnt); }
#else
(
	vm_map_t task,
	vm_address_t address,
	vm_info_region_64_t *region,
	vm_info_object_array_t *objects,
	mach_msg_type_number_t *objectsCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_region_64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_region_64
#if	defined(LINTLIBRARY)
    (target_task, address, size, flavor, info, infoCnt, object_name)
	vm_map_t target_task;
	vm_address_t *address;
	vm_size_t *size;
	vm_region_flavor_t flavor;
	vm_region_info_t info;
	mach_msg_type_number_t *infoCnt;
	mach_port_t *object_name;
{ return vm_region_64(target_task, address, size, flavor, info, infoCnt, object_name); }
#else
(
	vm_map_t target_task,
	vm_address_t *address,
	vm_size_t *size,
	vm_region_flavor_t flavor,
	vm_region_info_t info,
	mach_msg_type_number_t *infoCnt,
	mach_port_t *object_name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_make_memory_entry_64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_make_memory_entry_64
#if	defined(LINTLIBRARY)
    (target_task, size, offset, permission, object_handle, parent_entry)
	vm_map_t target_task;
	memory_object_size_t *size;
	memory_object_offset_t offset;
	vm_prot_t permission;
	mach_port_t *object_handle;
	mem_entry_name_port_t parent_entry;
{ return mach_make_memory_entry_64(target_task, size, offset, permission, object_handle, parent_entry); }
#else
(
	vm_map_t target_task,
	memory_object_size_t *size,
	memory_object_offset_t offset,
	vm_prot_t permission,
	mach_port_t *object_handle,
	mem_entry_name_port_t parent_entry
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_purgable_control */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_purgable_control
#if	defined(LINTLIBRARY)
    (target_task, address, control, state)
	vm_map_t target_task;
	vm_address_t address;
	vm_purgable_t control;
	int *state;
{ return vm_purgable_control(target_task, address, control, state); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_purgable_t control,
	int *state
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_region */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_region
#if	defined(LINTLIBRARY)
    (target_task, address, size, flavor, info, infoCnt, object_name)
	vm_map_t target_task;
	vm_address_t *address;
	vm_size_t *size;
	vm_region_flavor_t flavor;
	vm_region_info_t info;
	mach_msg_type_number_t *infoCnt;
	mach_port_t *object_name;
{ return vm_region(target_task, address, size, flavor, info, infoCnt, object_name); }
#else
(
	vm_map_t target_task,
	vm_address_t *address,
	vm_size_t *size,
	vm_region_flavor_t flavor,
	vm_region_info_t info,
	mach_msg_type_number_t *infoCnt,
	mach_port_t *object_name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_allocate */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_allocate
#if	defined(LINTLIBRARY)
    (target_task, address, size, flags)
	vm_map_t target_task;
	vm_address_t *address;
	vm_size_t size;
	int flags;
{ return vm_allocate(target_task, address, size, flags); }
#else
(
	vm_map_t target_task,
	vm_address_t *address,
	vm_size_t size,
	int flags
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_deallocate */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_deallocate
#if	defined(LINTLIBRARY)
    (target_task, address, size)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
{ return vm_deallocate(target_task, address, size); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_protect */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_protect
#if	defined(LINTLIBRARY)
    (target_task, address, size, set_maximum, new_protection)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	boolean_t set_maximum;
	vm_prot_t new_protection;
{ return vm_protect(target_task, address, size, set_maximum, new_protection); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	boolean_t set_maximum,
	vm_prot_t new_protection
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_inherit */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_inherit
#if	defined(LINTLIBRARY)
    (target_task, address, size, new_inheritance)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	vm_inherit_t new_inheritance;
{ return vm_inherit(target_task, address, size, new_inheritance); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	vm_inherit_t new_inheritance
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_read */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_read
#if	defined(LINTLIBRARY)
    (target_task, address, size, data, dataCnt)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	vm_offset_t *data;
	mach_msg_type_number_t *dataCnt;
{ return vm_read(target_task, address, size, data, dataCnt); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	vm_offset_t *data,
	mach_msg_type_number_t *dataCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_read_list */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_read_list
#if	defined(LINTLIBRARY)
    (target_task, data_list, count)
	vm_map_t target_task;
	vm_read_entry_t data_list;
	natural_t count;
{ return vm_read_list(target_task, data_list, count); }
#else
(
	vm_map_t target_task,
	vm_read_entry_t data_list,
	natural_t count
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_write */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_write
#if	defined(LINTLIBRARY)
    (target_task, address, data, dataCnt)
	vm_map_t target_task;
	vm_address_t address;
	vm_offset_t data;
	mach_msg_type_number_t dataCnt;
{ return vm_write(target_task, address, data, dataCnt); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_offset_t data,
	mach_msg_type_number_t dataCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_copy */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_copy
#if	defined(LINTLIBRARY)
    (target_task, source_address, size, dest_address)
	vm_map_t target_task;
	vm_address_t source_address;
	vm_size_t size;
	vm_address_t dest_address;
{ return vm_copy(target_task, source_address, size, dest_address); }
#else
(
	vm_map_t target_task,
	vm_address_t source_address,
	vm_size_t size,
	vm_address_t dest_address
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_read_overwrite */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_read_overwrite
#if	defined(LINTLIBRARY)
    (target_task, address, size, data, outsize)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	vm_address_t data;
	vm_size_t *outsize;
{ return vm_read_overwrite(target_task, address, size, data, outsize); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	vm_address_t data,
	vm_size_t *outsize
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_msync */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_msync
#if	defined(LINTLIBRARY)
    (target_task, address, size, sync_flags)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	vm_sync_t sync_flags;
{ return vm_msync(target_task, address, size, sync_flags); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	vm_sync_t sync_flags
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_behavior_set */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_behavior_set
#if	defined(LINTLIBRARY)
    (target_task, address, size, new_behavior)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	vm_behavior_t new_behavior;
{ return vm_behavior_set(target_task, address, size, new_behavior); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	vm_behavior_t new_behavior
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_machine_attribute */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_machine_attribute
#if	defined(LINTLIBRARY)
    (target_task, address, size, attribute, value)
	vm_map_t target_task;
	vm_address_t address;
	vm_size_t size;
	vm_machine_attribute_t attribute;
	vm_machine_attribute_val_t *value;
{ return vm_machine_attribute(target_task, address, size, attribute, value); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_size_t size,
	vm_machine_attribute_t attribute,
	vm_machine_attribute_val_t *value
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_remap */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_remap
#if	defined(LINTLIBRARY)
    (target_task, target_address, size, mask, flags, src_task, src_address, copy, cur_protection, max_protection, inheritance)
	vm_map_t target_task;
	vm_address_t *target_address;
	vm_size_t size;
	vm_address_t mask;
	int flags;
	vm_map_t src_task;
	vm_address_t src_address;
	boolean_t copy;
	vm_prot_t *cur_protection;
	vm_prot_t *max_protection;
	vm_inherit_t inheritance;
{ return vm_remap(target_task, target_address, size, mask, flags, src_task, src_address, copy, cur_protection, max_protection, inheritance); }
#else
(
	vm_map_t target_task,
	vm_address_t *target_address,
	vm_size_t size,
	vm_address_t mask,
	int flags,
	vm_map_t src_task,
	vm_address_t src_address,
	boolean_t copy,
	vm_prot_t *cur_protection,
	vm_prot_t *max_protection,
	vm_inherit_t inheritance
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_wire */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_wire
#if	defined(LINTLIBRARY)
    (target_task, must_wire)
	vm_map_t target_task;
	boolean_t must_wire;
{ return task_wire(target_task, must_wire); }
#else
(
	vm_map_t target_task,
	boolean_t must_wire
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_make_memory_entry */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_make_memory_entry
#if	defined(LINTLIBRARY)
    (target_task, size, offset, permission, object_handle, parent_entry)
	vm_map_t target_task;
	vm_size_t *size;
	vm_offset_t offset;
	vm_prot_t permission;
	mem_entry_name_port_t *object_handle;
	mem_entry_name_port_t parent_entry;
{ return mach_make_memory_entry(target_task, size, offset, permission, object_handle, parent_entry); }
#else
(
	vm_map_t target_task,
	vm_size_t *size,
	vm_offset_t offset,
	vm_prot_t permission,
	mem_entry_name_port_t *object_handle,
	mem_entry_name_port_t parent_entry
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_map_page_query */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_map_page_query
#if	defined(LINTLIBRARY)
    (target_map, offset, disposition, ref_count)
	vm_map_t target_map;
	vm_offset_t offset;
	integer_t *disposition;
	integer_t *ref_count;
{ return vm_map_page_query(target_map, offset, disposition, ref_count); }
#else
(
	vm_map_t target_map,
	vm_offset_t offset,
	integer_t *disposition,
	integer_t *ref_count
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_region_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_region_info
#if	defined(LINTLIBRARY)
    (task, address, region, objects, objectsCnt)
	vm_map_t task;
	vm_address_t address;
	vm_info_region_t *region;
	vm_info_object_array_t *objects;
	mach_msg_type_number_t *objectsCnt;
{ return mach_vm_region_info(task, address, region, objects, objectsCnt); }
#else
(
	vm_map_t task,
	vm_address_t address,
	vm_info_region_t *region,
	vm_info_object_array_t *objects,
	mach_msg_type_number_t *objectsCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_mapped_pages_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_mapped_pages_info
#if	defined(LINTLIBRARY)
    (task, pages, pagesCnt)
	vm_map_t task;
	page_address_array_t *pages;
	mach_msg_type_number_t *pagesCnt;
{ return vm_mapped_pages_info(task, pages, pagesCnt); }
#else
(
	vm_map_t task,
	page_address_array_t *pages,
	mach_msg_type_number_t *pagesCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_region_recurse */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_region_recurse
#if	defined(LINTLIBRARY)
    (target_task, address, size, nesting_depth, info, infoCnt)
	vm_map_t target_task;
	vm_address_t *address;
	vm_size_t *size;
	natural_t *nesting_depth;
	vm_region_recurse_info_t info;
	mach_msg_type_number_t *infoCnt;
{ return vm_region_recurse(target_task, address, size, nesting_depth, info, infoCnt); }
#else
(
	vm_map_t target_task,
	vm_address_t *address,
	vm_size_t *size,
	natural_t *nesting_depth,
	vm_region_recurse_info_t info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_region_recurse_64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_region_recurse_64
#if	defined(LINTLIBRARY)
    (target_task, address, size, nesting_depth, info, infoCnt)
	vm_map_t target_task;
	vm_address_t *address;
	vm_size_t *size;
	natural_t *nesting_depth;
	vm_region_recurse_info_t info;
	mach_msg_type_number_t *infoCnt;
{ return vm_region_recurse_64(target_task, address, size, nesting_depth, info, infoCnt); }
#else
(
	vm_map_t target_task,
	vm_address_t *address,
	vm_size_t *size,
	natural_t *nesting_depth,
	vm_region_recurse_info_t info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_region_info_64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_region_info_64
#if	defined(LINTLIBRARY)
    (task, address, region, objects, objectsCnt)
	vm_map_t task;
	vm_address_t address;
	vm_info_region_64_t *region;
	vm_info_object_array_t *objects;
	mach_msg_type_number_t *objectsCnt;
{ return mach_vm_region_info_64(task, address, region, objects, objectsCnt); }
#else
(
	vm_map_t task,
	vm_address_t address,
	vm_info_region_64_t *region,
	vm_info_object_array_t *objects,
	mach_msg_type_number_t *objectsCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_region_64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_region_64
#if	defined(LINTLIBRARY)
    (target_task, address, size, flavor, info, infoCnt, object_name)
	vm_map_t target_task;
	vm_address_t *address;
	vm_size_t *size;
	vm_region_flavor_t flavor;
	vm_region_info_t info;
	mach_msg_type_number_t *infoCnt;
	mach_port_t *object_name;
{ return vm_region_64(target_task, address, size, flavor, info, infoCnt, object_name); }
#else
(
	vm_map_t target_task,
	vm_address_t *address,
	vm_size_t *size,
	vm_region_flavor_t flavor,
	vm_region_info_t info,
	mach_msg_type_number_t *infoCnt,
	mach_port_t *object_name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_make_memory_entry_64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_make_memory_entry_64
#if	defined(LINTLIBRARY)
    (target_task, size, offset, permission, object_handle, parent_entry)
	vm_map_t target_task;
	memory_object_size_t *size;
	memory_object_offset_t offset;
	vm_prot_t permission;
	mach_port_t *object_handle;
	mem_entry_name_port_t parent_entry;
{ return mach_make_memory_entry_64(target_task, size, offset, permission, object_handle, parent_entry); }
#else
(
	vm_map_t target_task,
	memory_object_size_t *size,
	memory_object_offset_t offset,
	vm_prot_t permission,
	mach_port_t *object_handle,
	mem_entry_name_port_t parent_entry
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_purgable_control */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_purgable_control
#if	defined(LINTLIBRARY)
    (target_task, address, control, state)
	vm_map_t target_task;
	vm_address_t address;
	vm_purgable_t control;
	int *state;
{ return vm_purgable_control(target_task, address, control, state); }
#else
(
	vm_map_t target_task,
	vm_address_t address,
	vm_purgable_t control,
	int *state
);
#endif	/* defined(LINTLIBRARY) */

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
boolean_t vm_map_server(
		mach_msg_header_t *InHeadP,
		mach_msg_header_t *OutHeadP);

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
mig_routine_t vm_map_server_routine(
		mach_msg_header_t *InHeadP);


/* Description of this subsystem, for use in direct RPC */
extern const struct vm_map_subsystem {
	mig_server_routine_t	server;	/* Server routine */
	mach_msg_id_t	start;	/* Min routine number */
	mach_msg_id_t	end;	/* Max routine number + 1 */
	unsigned int	maxsize;	/* Max msg size */
	vm_address_t	reserved;	/* Reserved */
	struct routine_descriptor	/*Array of routine descriptors */
		routine[31];
} vm_map_subsystem;

/* typedefs for all requests */

#ifndef __Request__vm_map_subsystem__defined
#define __Request__vm_map_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_region_flavor_t flavor;
		mach_msg_type_number_t infoCnt;
	} __Request__vm_region_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		int flags;
	} __Request__vm_allocate_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
	} __Request__vm_deallocate_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		boolean_t set_maximum;
		vm_prot_t new_protection;
	} __Request__vm_protect_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		vm_inherit_t new_inheritance;
	} __Request__vm_inherit_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
	} __Request__vm_read_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_read_entry_t data_list;
		natural_t count;
	} __Request__vm_read_list_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t data;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		mach_msg_type_number_t dataCnt;
	} __Request__vm_write_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t source_address;
		vm_size_t size;
		vm_address_t dest_address;
	} __Request__vm_copy_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		vm_address_t data;
	} __Request__vm_read_overwrite_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		vm_sync_t sync_flags;
	} __Request__vm_msync_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		vm_behavior_t new_behavior;
	} __Request__vm_behavior_set_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		vm_machine_attribute_t attribute;
		vm_machine_attribute_val_t value;
	} __Request__vm_machine_attribute_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t src_task;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t target_address;
		vm_size_t size;
		vm_address_t mask;
		int flags;
		vm_address_t src_address;
		boolean_t copy;
		vm_inherit_t inheritance;
	} __Request__vm_remap_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t must_wire;
	} __Request__task_wire_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t parent_entry;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_size_t size;
		vm_offset_t offset;
		vm_prot_t permission;
	} __Request__mach_make_memory_entry_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_offset_t offset;
	} __Request__vm_map_page_query_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
	} __Request__mach_vm_region_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
	} __Request__vm_mapped_pages_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		natural_t nesting_depth;
		mach_msg_type_number_t infoCnt;
	} __Request__vm_region_recurse_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		natural_t nesting_depth;
		mach_msg_type_number_t infoCnt;
	} __Request__vm_region_recurse_64_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
	} __Request__mach_vm_region_info_64_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_region_flavor_t flavor;
		mach_msg_type_number_t infoCnt;
	} __Request__vm_region_64_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t parent_entry;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		memory_object_size_t size;
		memory_object_offset_t offset;
		vm_prot_t permission;
	} __Request__mach_make_memory_entry_64_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_purgable_t control;
		int state;
	} __Request__vm_purgable_control_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__vm_map_subsystem__defined */


/* union of all requests */

#ifndef __RequestUnion__vm_map_subsystem__defined
#define __RequestUnion__vm_map_subsystem__defined
union __RequestUnion__vm_map_subsystem {
	__Request__vm_region_t Request_vm_region;
	__Request__vm_allocate_t Request_vm_allocate;
	__Request__vm_deallocate_t Request_vm_deallocate;
	__Request__vm_protect_t Request_vm_protect;
	__Request__vm_inherit_t Request_vm_inherit;
	__Request__vm_read_t Request_vm_read;
	__Request__vm_read_list_t Request_vm_read_list;
	__Request__vm_write_t Request_vm_write;
	__Request__vm_copy_t Request_vm_copy;
	__Request__vm_read_overwrite_t Request_vm_read_overwrite;
	__Request__vm_msync_t Request_vm_msync;
	__Request__vm_behavior_set_t Request_vm_behavior_set;
	__Request__vm_machine_attribute_t Request_vm_machine_attribute;
	__Request__vm_remap_t Request_vm_remap;
	__Request__task_wire_t Request_task_wire;
	__Request__mach_make_memory_entry_t Request_mach_make_memory_entry;
	__Request__vm_map_page_query_t Request_vm_map_page_query;
	__Request__mach_vm_region_info_t Request_mach_vm_region_info;
	__Request__vm_mapped_pages_info_t Request_vm_mapped_pages_info;
	__Request__vm_region_recurse_t Request_vm_region_recurse;
	__Request__vm_region_recurse_64_t Request_vm_region_recurse_64;
	__Request__mach_vm_region_info_64_t Request_mach_vm_region_info_64;
	__Request__vm_region_64_t Request_vm_region_64;
	__Request__mach_make_memory_entry_64_t Request_mach_make_memory_entry_64;
	__Request__vm_purgable_control_t Request_vm_purgable_control;
};
#endif /* __RequestUnion__vm_map_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__vm_map_subsystem__defined
#define __Reply__vm_map_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t object_name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		mach_msg_type_number_t infoCnt;
		int info[10];
	} __Reply__vm_region_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		vm_address_t address;
	} __Reply__vm_allocate_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__vm_deallocate_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__vm_protect_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__vm_inherit_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t data;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t dataCnt;
	} __Reply__vm_read_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		vm_read_entry_t data_list;
	} __Reply__vm_read_list_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__vm_write_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__vm_copy_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		vm_size_t outsize;
	} __Reply__vm_read_overwrite_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__vm_msync_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__vm_behavior_set_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		vm_machine_attribute_val_t value;
	} __Reply__vm_machine_attribute_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		vm_address_t target_address;
		vm_prot_t cur_protection;
		vm_prot_t max_protection;
	} __Reply__vm_remap_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_wire_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t object_handle;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_size_t size;
	} __Reply__mach_make_memory_entry_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		integer_t disposition;
		integer_t ref_count;
	} __Reply__vm_map_page_query_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t objects;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_info_region_t region;
		mach_msg_type_number_t objectsCnt;
	} __Reply__mach_vm_region_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t pages;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t pagesCnt;
	} __Reply__vm_mapped_pages_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		vm_address_t address;
		vm_size_t size;
		natural_t nesting_depth;
		mach_msg_type_number_t infoCnt;
		int info[19];
	} __Reply__vm_region_recurse_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		vm_address_t address;
		vm_size_t size;
		natural_t nesting_depth;
		mach_msg_type_number_t infoCnt;
		int info[19];
	} __Reply__vm_region_recurse_64_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t objects;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_info_region_64_t region;
		mach_msg_type_number_t objectsCnt;
	} __Reply__mach_vm_region_info_64_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t object_name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		mach_msg_type_number_t infoCnt;
		int info[10];
	} __Reply__vm_region_64_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t object_handle;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		memory_object_size_t size;
	} __Reply__mach_make_memory_entry_64_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		int state;
	} __Reply__vm_purgable_control_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__vm_map_subsystem__defined */


/* union of all replies */

#ifndef __ReplyUnion__vm_map_subsystem__defined
#define __ReplyUnion__vm_map_subsystem__defined
union __ReplyUnion__vm_map_subsystem {
	__Reply__vm_region_t Reply_vm_region;
	__Reply__vm_allocate_t Reply_vm_allocate;
	__Reply__vm_deallocate_t Reply_vm_deallocate;
	__Reply__vm_protect_t Reply_vm_protect;
	__Reply__vm_inherit_t Reply_vm_inherit;
	__Reply__vm_read_t Reply_vm_read;
	__Reply__vm_read_list_t Reply_vm_read_list;
	__Reply__vm_write_t Reply_vm_write;
	__Reply__vm_copy_t Reply_vm_copy;
	__Reply__vm_read_overwrite_t Reply_vm_read_overwrite;
	__Reply__vm_msync_t Reply_vm_msync;
	__Reply__vm_behavior_set_t Reply_vm_behavior_set;
	__Reply__vm_machine_attribute_t Reply_vm_machine_attribute;
	__Reply__vm_remap_t Reply_vm_remap;
	__Reply__task_wire_t Reply_task_wire;
	__Reply__mach_make_memory_entry_t Reply_mach_make_memory_entry;
	__Reply__vm_map_page_query_t Reply_vm_map_page_query;
	__Reply__mach_vm_region_info_t Reply_mach_vm_region_info;
	__Reply__vm_mapped_pages_info_t Reply_vm_mapped_pages_info;
	__Reply__vm_region_recurse_t Reply_vm_region_recurse;
	__Reply__vm_region_recurse_64_t Reply_vm_region_recurse_64;
	__Reply__mach_vm_region_info_64_t Reply_mach_vm_region_info_64;
	__Reply__vm_region_64_t Reply_vm_region_64;
	__Reply__mach_make_memory_entry_64_t Reply_mach_make_memory_entry_64;
	__Reply__vm_purgable_control_t Reply_vm_purgable_control;
};
#endif /* __RequestUnion__vm_map_subsystem__defined */

#ifndef subsystem_to_name_map_vm_map
#define subsystem_to_name_map_vm_map \
    { "vm_region", 3800 },\
    { "vm_allocate", 3801 },\
    { "vm_deallocate", 3802 },\
    { "vm_protect", 3803 },\
    { "vm_inherit", 3804 },\
    { "vm_read", 3805 },\
    { "vm_read_list", 3806 },\
    { "vm_write", 3807 },\
    { "vm_copy", 3808 },\
    { "vm_read_overwrite", 3809 },\
    { "vm_msync", 3810 },\
    { "vm_behavior_set", 3811 },\
    { "vm_machine_attribute", 3813 },\
    { "vm_remap", 3814 },\
    { "task_wire", 3815 },\
    { "mach_make_memory_entry", 3816 },\
    { "vm_map_page_query", 3817 },\
    { "mach_vm_region_info", 3818 },\
    { "vm_mapped_pages_info", 3819 },\
    { "vm_region_recurse", 3821 },\
    { "vm_region_recurse_64", 3822 },\
    { "mach_vm_region_info_64", 3823 },\
    { "vm_region_64", 3824 },\
    { "mach_make_memory_entry_64", 3825 },\
    { "vm_purgable_control", 3830 }
#endif

#ifdef __AfterMigServerHeader
__AfterMigServerHeader
#endif /* __AfterMigServerHeader */

#endif	 /* _vm_map_server_ */

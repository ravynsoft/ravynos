#ifndef	_mach_vm_user_
#define	_mach_vm_user_

/* Module mach_vm */

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

#ifndef	mach_vm_MSG_COUNT
#define	mach_vm_MSG_COUNT	20
#endif	/* mach_vm_MSG_COUNT */

#include <sys/mach/std_types.h>
#include <sys/mach/mig.h>
#include <sys/mach/thread_status.h>
#include <sys/mach/mig.h>
#include <sys/mach/mach_types.h>
#include <sys/mach_debug/mach_debug_types.h>

#ifdef __BeforeMigUserHeader
__BeforeMigUserHeader
#endif /* __BeforeMigUserHeader */

#include <sys/cdefs.h>
__BEGIN_DECLS


/* Routine mach_vm_protect */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_protect
#if	defined(LINTLIBRARY)
    (target_task, address, size, set_maximum, new_protection)
	mach_vm_map_t target_task;
	mach_vm_address_t address;
	mach_vm_size_t size;
	boolean_t set_maximum;
	vm_prot_t new_protection;
{ return mach_vm_protect(target_task, address, size, set_maximum, new_protection); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t address,
	mach_vm_size_t size,
	boolean_t set_maximum,
	vm_prot_t new_protection
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_inherit */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_inherit
#if	defined(LINTLIBRARY)
    (target_task, address, size, new_inheritance)
	mach_vm_map_t target_task;
	mach_vm_address_t address;
	mach_vm_size_t size;
	vm_inherit_t new_inheritance;
{ return mach_vm_inherit(target_task, address, size, new_inheritance); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t address,
	mach_vm_size_t size,
	vm_inherit_t new_inheritance
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_read */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_read
#if	defined(LINTLIBRARY)
    (target_task, address, size, data, dataCnt)
	mach_vm_map_t target_task;
	mach_vm_address_t address;
	mach_vm_size_t size;
	vm_offset_t *data;
	mach_msg_type_number_t *dataCnt;
{ return mach_vm_read(target_task, address, size, data, dataCnt); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t address,
	mach_vm_size_t size,
	vm_offset_t *data,
	mach_msg_type_number_t *dataCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_read_list */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_read_list
#if	defined(LINTLIBRARY)
    (target_task, data_list, count)
	mach_vm_map_t target_task;
	mach_vm_read_entry_t data_list;
	natural_t count;
{ return mach_vm_read_list(target_task, data_list, count); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_read_entry_t data_list,
	natural_t count
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_write */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_write
#if	defined(LINTLIBRARY)
    (target_task, address, data, dataCnt)
	mach_vm_map_t target_task;
	mach_vm_address_t address;
	vm_offset_t data;
	mach_msg_type_number_t dataCnt;
{ return mach_vm_write(target_task, address, data, dataCnt); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t address,
	vm_offset_t data,
	mach_msg_type_number_t dataCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_copy */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_copy
#if	defined(LINTLIBRARY)
    (target_task, source_address, size, dest_address)
	mach_vm_map_t target_task;
	mach_vm_address_t source_address;
	mach_vm_size_t size;
	mach_vm_address_t dest_address;
{ return mach_vm_copy(target_task, source_address, size, dest_address); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t source_address,
	mach_vm_size_t size,
	mach_vm_address_t dest_address
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_read_overwrite */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_read_overwrite
#if	defined(LINTLIBRARY)
    (target_task, address, size, data, outsize)
	mach_vm_map_t target_task;
	mach_vm_address_t address;
	mach_vm_size_t size;
	mach_vm_address_t data;
	mach_vm_size_t *outsize;
{ return mach_vm_read_overwrite(target_task, address, size, data, outsize); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t address,
	mach_vm_size_t size,
	mach_vm_address_t data,
	mach_vm_size_t *outsize
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_msync */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_msync
#if	defined(LINTLIBRARY)
    (target_task, address, size, sync_flags)
	mach_vm_map_t target_task;
	mach_vm_address_t address;
	mach_vm_size_t size;
	vm_sync_t sync_flags;
{ return mach_vm_msync(target_task, address, size, sync_flags); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t address,
	mach_vm_size_t size,
	vm_sync_t sync_flags
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_behavior_set */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_behavior_set
#if	defined(LINTLIBRARY)
    (target_task, address, size, new_behavior)
	mach_vm_map_t target_task;
	mach_vm_address_t address;
	mach_vm_size_t size;
	vm_behavior_t new_behavior;
{ return mach_vm_behavior_set(target_task, address, size, new_behavior); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t address,
	mach_vm_size_t size,
	vm_behavior_t new_behavior
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_machine_attribute */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_machine_attribute
#if	defined(LINTLIBRARY)
    (target_task, address, size, attribute, value)
	mach_vm_map_t target_task;
	mach_vm_address_t address;
	mach_vm_size_t size;
	vm_machine_attribute_t attribute;
	vm_machine_attribute_val_t *value;
{ return mach_vm_machine_attribute(target_task, address, size, attribute, value); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t address,
	mach_vm_size_t size,
	vm_machine_attribute_t attribute,
	vm_machine_attribute_val_t *value
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_remap */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_remap
#if	defined(LINTLIBRARY)
    (target_task, target_address, size, mask, flags, src_task, src_address, copy, cur_protection, max_protection, inheritance)
	mach_vm_map_t target_task;
	mach_vm_address_t *target_address;
	mach_vm_size_t size;
	mach_vm_offset_t mask;
	int flags;
	mach_vm_map_t src_task;
	mach_vm_address_t src_address;
	boolean_t copy;
	vm_prot_t *cur_protection;
	vm_prot_t *max_protection;
	vm_inherit_t inheritance;
{ return mach_vm_remap(target_task, target_address, size, mask, flags, src_task, src_address, copy, cur_protection, max_protection, inheritance); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t *target_address,
	mach_vm_size_t size,
	mach_vm_offset_t mask,
	int flags,
	mach_vm_map_t src_task,
	mach_vm_address_t src_address,
	boolean_t copy,
	vm_prot_t *cur_protection,
	vm_prot_t *max_protection,
	vm_inherit_t inheritance
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_page_query */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_page_query
#if	defined(LINTLIBRARY)
    (target_map, offset, disposition, ref_count)
	mach_vm_map_t target_map;
	mach_vm_offset_t offset;
	integer_t *disposition;
	integer_t *ref_count;
{ return mach_vm_page_query(target_map, offset, disposition, ref_count); }
#else
(
	mach_vm_map_t target_map,
	mach_vm_offset_t offset,
	integer_t *disposition,
	integer_t *ref_count
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_region_recurse */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_region_recurse
#if	defined(LINTLIBRARY)
    (target_task, address, size, nesting_depth, info, infoCnt)
	mach_vm_map_t target_task;
	mach_vm_address_t *address;
	mach_vm_size_t *size;
	natural_t *nesting_depth;
	vm_region_recurse_info_t info;
	mach_msg_type_number_t *infoCnt;
{ return mach_vm_region_recurse(target_task, address, size, nesting_depth, info, infoCnt); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t *address,
	mach_vm_size_t *size,
	natural_t *nesting_depth,
	vm_region_recurse_info_t info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_region */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_region
#if	defined(LINTLIBRARY)
    (target_task, address, size, flavor, info, infoCnt, object_name)
	mach_vm_map_t target_task;
	mach_vm_address_t *address;
	mach_vm_size_t *size;
	vm_region_flavor_t flavor;
	vm_region_info_t info;
	mach_msg_type_number_t *infoCnt;
	mach_port_t *object_name;
{ return mach_vm_region(target_task, address, size, flavor, info, infoCnt, object_name); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t *address,
	mach_vm_size_t *size,
	vm_region_flavor_t flavor,
	vm_region_info_t info,
	mach_msg_type_number_t *infoCnt,
	mach_port_t *object_name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _mach_make_memory_entry */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _mach_make_memory_entry
#if	defined(LINTLIBRARY)
    (target_task, size, offset, permission, object_handle, parent_handle)
	mach_vm_map_t target_task;
	memory_object_size_t *size;
	memory_object_offset_t offset;
	vm_prot_t permission;
	mem_entry_name_port_t *object_handle;
	mem_entry_name_port_t parent_handle;
{ return _mach_make_memory_entry(target_task, size, offset, permission, object_handle, parent_handle); }
#else
(
	mach_vm_map_t target_task,
	memory_object_size_t *size,
	memory_object_offset_t offset,
	vm_prot_t permission,
	mem_entry_name_port_t *object_handle,
	mem_entry_name_port_t parent_handle
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_purgable_control */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_purgable_control
#if	defined(LINTLIBRARY)
    (target_task, address, control, state)
	mach_vm_map_t target_task;
	mach_vm_address_t address;
	vm_purgable_t control;
	int *state;
{ return mach_vm_purgable_control(target_task, address, control, state); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t address,
	vm_purgable_t control,
	int *state
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_page_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_page_info
#if	defined(LINTLIBRARY)
    (target_task, address, flavor, info, infoCnt)
	mach_vm_map_t target_task;
	mach_vm_address_t address;
	vm_page_info_flavor_t flavor;
	vm_page_info_t info;
	mach_msg_type_number_t *infoCnt;
{ return mach_vm_page_info(target_task, address, flavor, info, infoCnt); }
#else
(
	mach_vm_map_t target_task,
	mach_vm_address_t address,
	vm_page_info_flavor_t flavor,
	vm_page_info_t info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

__END_DECLS

/********************** Caution **************************/
/* The following data types should be used to calculate  */
/* maximum message sizes only. The actual message may be */
/* smaller, and the position of the arguments within the */
/* message layout may vary from what is presented here.  */
/* For example, if any of the arguments are variable-    */
/* sized, and less than the maximum is sent, the data    */
/* will be packed tight in the actual message to reduce  */
/* the presence of holes.                                */
/********************** Caution **************************/

/* typedefs for all requests */

#ifndef __Request__mach_vm_subsystem__defined
#define __Request__mach_vm_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_vm_address_t address;
		mach_vm_size_t size;
		boolean_t set_maximum;
		vm_prot_t new_protection;
	} __Request__mach_vm_protect_t;
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
		mach_vm_address_t address;
		mach_vm_size_t size;
		vm_inherit_t new_inheritance;
	} __Request__mach_vm_inherit_t;
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
		mach_vm_address_t address;
		mach_vm_size_t size;
	} __Request__mach_vm_read_t;
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
		mach_vm_read_entry_t data_list;
		natural_t count;
	} __Request__mach_vm_read_list_t;
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
		mach_vm_address_t address;
		mach_msg_type_number_t dataCnt;
	} __Request__mach_vm_write_t;
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
		mach_vm_address_t source_address;
		mach_vm_size_t size;
		mach_vm_address_t dest_address;
	} __Request__mach_vm_copy_t;
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
		mach_vm_address_t address;
		mach_vm_size_t size;
		mach_vm_address_t data;
	} __Request__mach_vm_read_overwrite_t;
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
		mach_vm_address_t address;
		mach_vm_size_t size;
		vm_sync_t sync_flags;
	} __Request__mach_vm_msync_t;
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
		mach_vm_address_t address;
		mach_vm_size_t size;
		vm_behavior_t new_behavior;
	} __Request__mach_vm_behavior_set_t;
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
		mach_vm_address_t address;
		mach_vm_size_t size;
		vm_machine_attribute_t attribute;
		vm_machine_attribute_val_t value;
	} __Request__mach_vm_machine_attribute_t;
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
		mach_vm_address_t target_address;
		mach_vm_size_t size;
		mach_vm_offset_t mask;
		int flags;
		mach_vm_address_t src_address;
		boolean_t copy;
		vm_inherit_t inheritance;
	} __Request__mach_vm_remap_t;
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
		mach_vm_offset_t offset;
	} __Request__mach_vm_page_query_t;
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
		mach_vm_address_t address;
		natural_t nesting_depth;
		mach_msg_type_number_t infoCnt;
	} __Request__mach_vm_region_recurse_t;
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
		mach_vm_address_t address;
		vm_region_flavor_t flavor;
		mach_msg_type_number_t infoCnt;
	} __Request__mach_vm_region_t;
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
		mach_msg_port_descriptor_t parent_handle;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		memory_object_size_t size;
		memory_object_offset_t offset;
		vm_prot_t permission;
	} __Request___mach_make_memory_entry_t;
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
		mach_vm_address_t address;
		vm_purgable_t control;
		int state;
	} __Request__mach_vm_purgable_control_t;
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
		mach_vm_address_t address;
		vm_page_info_flavor_t flavor;
		mach_msg_type_number_t infoCnt;
	} __Request__mach_vm_page_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__mach_vm_subsystem__defined */

/* union of all requests */

#ifndef __RequestUnion__mach_vm_subsystem__defined
#define __RequestUnion__mach_vm_subsystem__defined
union __RequestUnion__mach_vm_subsystem {
	__Request__mach_vm_protect_t Request_mach_vm_protect;
	__Request__mach_vm_inherit_t Request_mach_vm_inherit;
	__Request__mach_vm_read_t Request_mach_vm_read;
	__Request__mach_vm_read_list_t Request_mach_vm_read_list;
	__Request__mach_vm_write_t Request_mach_vm_write;
	__Request__mach_vm_copy_t Request_mach_vm_copy;
	__Request__mach_vm_read_overwrite_t Request_mach_vm_read_overwrite;
	__Request__mach_vm_msync_t Request_mach_vm_msync;
	__Request__mach_vm_behavior_set_t Request_mach_vm_behavior_set;
	__Request__mach_vm_machine_attribute_t Request_mach_vm_machine_attribute;
	__Request__mach_vm_remap_t Request_mach_vm_remap;
	__Request__mach_vm_page_query_t Request_mach_vm_page_query;
	__Request__mach_vm_region_recurse_t Request_mach_vm_region_recurse;
	__Request__mach_vm_region_t Request_mach_vm_region;
	__Request___mach_make_memory_entry_t Request__mach_make_memory_entry;
	__Request__mach_vm_purgable_control_t Request_mach_vm_purgable_control;
	__Request__mach_vm_page_info_t Request_mach_vm_page_info;
};
#endif /* !__RequestUnion__mach_vm_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__mach_vm_subsystem__defined
#define __Reply__mach_vm_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__mach_vm_protect_t;
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
	} __Reply__mach_vm_inherit_t;
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
	} __Reply__mach_vm_read_t;
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
		mach_vm_read_entry_t data_list;
	} __Reply__mach_vm_read_list_t;
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
	} __Reply__mach_vm_write_t;
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
	} __Reply__mach_vm_copy_t;
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
		mach_vm_size_t outsize;
	} __Reply__mach_vm_read_overwrite_t;
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
	} __Reply__mach_vm_msync_t;
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
	} __Reply__mach_vm_behavior_set_t;
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
	} __Reply__mach_vm_machine_attribute_t;
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
		mach_vm_address_t target_address;
		vm_prot_t cur_protection;
		vm_prot_t max_protection;
	} __Reply__mach_vm_remap_t;
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
	} __Reply__mach_vm_page_query_t;
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
		mach_vm_address_t address;
		mach_vm_size_t size;
		natural_t nesting_depth;
		mach_msg_type_number_t infoCnt;
		int info[19];
	} __Reply__mach_vm_region_recurse_t;
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
		mach_vm_address_t address;
		mach_vm_size_t size;
		mach_msg_type_number_t infoCnt;
		int info[10];
	} __Reply__mach_vm_region_t;
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
	} __Reply___mach_make_memory_entry_t;
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
	} __Reply__mach_vm_purgable_control_t;
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
		mach_msg_type_number_t infoCnt;
		int info[32];
	} __Reply__mach_vm_page_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__mach_vm_subsystem__defined */

/* union of all replies */

#ifndef __ReplyUnion__mach_vm_subsystem__defined
#define __ReplyUnion__mach_vm_subsystem__defined
union __ReplyUnion__mach_vm_subsystem {
	__Reply__mach_vm_protect_t Reply_mach_vm_protect;
	__Reply__mach_vm_inherit_t Reply_mach_vm_inherit;
	__Reply__mach_vm_read_t Reply_mach_vm_read;
	__Reply__mach_vm_read_list_t Reply_mach_vm_read_list;
	__Reply__mach_vm_write_t Reply_mach_vm_write;
	__Reply__mach_vm_copy_t Reply_mach_vm_copy;
	__Reply__mach_vm_read_overwrite_t Reply_mach_vm_read_overwrite;
	__Reply__mach_vm_msync_t Reply_mach_vm_msync;
	__Reply__mach_vm_behavior_set_t Reply_mach_vm_behavior_set;
	__Reply__mach_vm_machine_attribute_t Reply_mach_vm_machine_attribute;
	__Reply__mach_vm_remap_t Reply_mach_vm_remap;
	__Reply__mach_vm_page_query_t Reply_mach_vm_page_query;
	__Reply__mach_vm_region_recurse_t Reply_mach_vm_region_recurse;
	__Reply__mach_vm_region_t Reply_mach_vm_region;
	__Reply___mach_make_memory_entry_t Reply__mach_make_memory_entry;
	__Reply__mach_vm_purgable_control_t Reply_mach_vm_purgable_control;
	__Reply__mach_vm_page_info_t Reply_mach_vm_page_info;
};
#endif /* !__RequestUnion__mach_vm_subsystem__defined */

#ifndef subsystem_to_name_map_mach_vm
#define subsystem_to_name_map_mach_vm \
    { "mach_vm_protect", 4802 },\
    { "mach_vm_inherit", 4803 },\
    { "mach_vm_read", 4804 },\
    { "mach_vm_read_list", 4805 },\
    { "mach_vm_write", 4806 },\
    { "mach_vm_copy", 4807 },\
    { "mach_vm_read_overwrite", 4808 },\
    { "mach_vm_msync", 4809 },\
    { "mach_vm_behavior_set", 4810 },\
    { "mach_vm_machine_attribute", 4812 },\
    { "mach_vm_remap", 4813 },\
    { "mach_vm_page_query", 4814 },\
    { "mach_vm_region_recurse", 4815 },\
    { "mach_vm_region", 4816 },\
    { "_mach_make_memory_entry", 4817 },\
    { "mach_vm_purgable_control", 4818 },\
    { "mach_vm_page_info", 4819 }
#endif

#ifdef __AfterMigUserHeader
__AfterMigUserHeader
#endif /* __AfterMigUserHeader */

#endif	 /* _mach_vm_user_ */

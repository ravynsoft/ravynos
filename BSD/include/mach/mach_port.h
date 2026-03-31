#ifndef	_mach_port_user_
#define	_mach_port_user_

/* Module mach_port */

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

#ifndef	mach_port_MSG_COUNT
#define	mach_port_MSG_COUNT	36
#endif	/* mach_port_MSG_COUNT */

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


/* Routine mach_port_names */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_names
#if	defined(LINTLIBRARY)
    (task, names, namesCnt, types, typesCnt)
	ipc_space_t task;
	mach_port_name_array_t *names;
	mach_msg_type_number_t *namesCnt;
	mach_port_type_array_t *types;
	mach_msg_type_number_t *typesCnt;
{ return mach_port_names(task, names, namesCnt, types, typesCnt); }
#else
(
	ipc_space_t task,
	mach_port_name_array_t *names,
	mach_msg_type_number_t *namesCnt,
	mach_port_type_array_t *types,
	mach_msg_type_number_t *typesCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_type */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_type
#if	defined(LINTLIBRARY)
    (task, name, ptype)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_type_t *ptype;
{ return mach_port_type(task, name, ptype); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_type_t *ptype
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_rename */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_rename
#if	defined(LINTLIBRARY)
    (task, old_name, new_name)
	ipc_space_t task;
	mach_port_name_t old_name;
	mach_port_name_t new_name;
{ return mach_port_rename(task, old_name, new_name); }
#else
(
	ipc_space_t task,
	mach_port_name_t old_name,
	mach_port_name_t new_name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_allocate_name */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_allocate_name
#if	defined(LINTLIBRARY)
    (task, right, name)
	ipc_space_t task;
	mach_port_right_t right;
	mach_port_name_t name;
{ return mach_port_allocate_name(task, right, name); }
#else
(
	ipc_space_t task,
	mach_port_right_t right,
	mach_port_name_t name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_destroy */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_destroy
#if	defined(LINTLIBRARY)
    (task, name)
	ipc_space_t task;
	mach_port_name_t name;
{ return mach_port_destroy(task, name); }
#else
(
	ipc_space_t task,
	mach_port_name_t name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_get_refs */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_get_refs
#if	defined(LINTLIBRARY)
    (task, name, right, refs)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_right_t right;
	mach_port_urefs_t *refs;
{ return mach_port_get_refs(task, name, right, refs); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_right_t right,
	mach_port_urefs_t *refs
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_peek */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_peek
#if	defined(LINTLIBRARY)
    (task, name, trailer_type, request_seqnop, msg_sizep, msg_idp, trailer_infop, trailer_infopCnt)
	ipc_space_t task;
	mach_port_name_t name;
	mach_msg_trailer_type_t trailer_type;
	mach_port_seqno_t *request_seqnop;
	mach_msg_size_t *msg_sizep;
	mach_msg_id_t *msg_idp;
	mach_msg_trailer_info_t trailer_infop;
	mach_msg_type_number_t *trailer_infopCnt;
{ return mach_port_peek(task, name, trailer_type, request_seqnop, msg_sizep, msg_idp, trailer_infop, trailer_infopCnt); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_msg_trailer_type_t trailer_type,
	mach_port_seqno_t *request_seqnop,
	mach_msg_size_t *msg_sizep,
	mach_msg_id_t *msg_idp,
	mach_msg_trailer_info_t trailer_infop,
	mach_msg_type_number_t *trailer_infopCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_set_mscount */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_set_mscount
#if	defined(LINTLIBRARY)
    (task, name, mscount)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_mscount_t mscount;
{ return mach_port_set_mscount(task, name, mscount); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_mscount_t mscount
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_get_set_status */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_get_set_status
#if	defined(LINTLIBRARY)
    (task, name, members, membersCnt)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_name_array_t *members;
	mach_msg_type_number_t *membersCnt;
{ return mach_port_get_set_status(task, name, members, membersCnt); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_name_array_t *members,
	mach_msg_type_number_t *membersCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_request_notification */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_request_notification
#if	defined(LINTLIBRARY)
    (task, name, msgid, sync, notify, notifyPoly, previous)
	ipc_space_t task;
	mach_port_name_t name;
	mach_msg_id_t msgid;
	mach_port_mscount_t sync;
	mach_port_t notify;
	mach_msg_type_name_t notifyPoly;
	mach_port_t *previous;
{ return mach_port_request_notification(task, name, msgid, sync, notify, notifyPoly, previous); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_msg_id_t msgid,
	mach_port_mscount_t sync,
	mach_port_t notify,
	mach_msg_type_name_t notifyPoly,
	mach_port_t *previous
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_set_seqno */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_set_seqno
#if	defined(LINTLIBRARY)
    (task, name, seqno)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_seqno_t seqno;
{ return mach_port_set_seqno(task, name, seqno); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_seqno_t seqno
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_get_attributes */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_get_attributes
#if	defined(LINTLIBRARY)
    (task, name, flavor, port_info_out, port_info_outCnt)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_flavor_t flavor;
	mach_port_info_t port_info_out;
	mach_msg_type_number_t *port_info_outCnt;
{ return mach_port_get_attributes(task, name, flavor, port_info_out, port_info_outCnt); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_flavor_t flavor,
	mach_port_info_t port_info_out,
	mach_msg_type_number_t *port_info_outCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_set_attributes */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_set_attributes
#if	defined(LINTLIBRARY)
    (task, name, flavor, port_info, port_infoCnt)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_flavor_t flavor;
	mach_port_info_t port_info;
	mach_msg_type_number_t port_infoCnt;
{ return mach_port_set_attributes(task, name, flavor, port_info, port_infoCnt); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_flavor_t flavor,
	mach_port_info_t port_info,
	mach_msg_type_number_t port_infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_allocate_qos */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_allocate_qos
#if	defined(LINTLIBRARY)
    (task, right, qos, name)
	ipc_space_t task;
	mach_port_right_t right;
	mach_port_qos_t *qos;
	mach_port_name_t *name;
{ return mach_port_allocate_qos(task, right, qos, name); }
#else
(
	ipc_space_t task,
	mach_port_right_t right,
	mach_port_qos_t *qos,
	mach_port_name_t *name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_allocate_full */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_allocate_full
#if	defined(LINTLIBRARY)
    (task, right, proto, qos, name)
	ipc_space_t task;
	mach_port_right_t right;
	mach_port_t proto;
	mach_port_qos_t *qos;
	mach_port_name_t *name;
{ return mach_port_allocate_full(task, right, proto, qos, name); }
#else
(
	ipc_space_t task,
	mach_port_right_t right,
	mach_port_t proto,
	mach_port_qos_t *qos,
	mach_port_name_t *name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_set_port_space */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_port_space
#if	defined(LINTLIBRARY)
    (task, table_entries)
	ipc_space_t task;
	int table_entries;
{ return task_set_port_space(task, table_entries); }
#else
(
	ipc_space_t task,
	int table_entries
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_get_srights */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_get_srights
#if	defined(LINTLIBRARY)
    (task, name, srights)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_rights_t *srights;
{ return mach_port_get_srights(task, name, srights); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_rights_t *srights
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_space_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_space_info
#if	defined(LINTLIBRARY)
    (task, space_info, table_info, table_infoCnt, tree_info, tree_infoCnt)
	ipc_space_t task;
	ipc_info_space_t *space_info;
	ipc_info_name_array_t *table_info;
	mach_msg_type_number_t *table_infoCnt;
	ipc_info_tree_name_array_t *tree_info;
	mach_msg_type_number_t *tree_infoCnt;
{ return mach_port_space_info(task, space_info, table_info, table_infoCnt, tree_info, tree_infoCnt); }
#else
(
	ipc_space_t task,
	ipc_info_space_t *space_info,
	ipc_info_name_array_t *table_info,
	mach_msg_type_number_t *table_infoCnt,
	ipc_info_tree_name_array_t *tree_info,
	mach_msg_type_number_t *tree_infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_dnrequest_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_dnrequest_info
#if	defined(LINTLIBRARY)
    (task, name, dnr_total, dnr_used)
	ipc_space_t task;
	mach_port_name_t name;
	unsigned *dnr_total;
	unsigned *dnr_used;
{ return mach_port_dnrequest_info(task, name, dnr_total, dnr_used); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	unsigned *dnr_total,
	unsigned *dnr_used
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_get_context */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_get_context
#if	defined(LINTLIBRARY)
    (task, name, context)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_context_t *context;
{ return mach_port_get_context(task, name, context); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_context_t *context
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_set_context */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_set_context
#if	defined(LINTLIBRARY)
    (task, name, context)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_context_t context;
{ return mach_port_set_context(task, name, context); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_context_t context
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_kobject */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_kobject
#if	defined(LINTLIBRARY)
    (task, name, object_type, object_addr)
	ipc_space_t task;
	mach_port_name_t name;
	natural_t *object_type;
	mach_vm_address_t *object_addr;
{ return mach_port_kobject(task, name, object_type, object_addr); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	natural_t *object_type,
	mach_vm_address_t *object_addr
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_construct */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_construct
#if	defined(LINTLIBRARY)
    (task, options, context, name)
	ipc_space_t task;
	mach_port_options_ptr_t options;
	mach_port_context_t context;
	mach_port_name_t *name;
{ return mach_port_construct(task, options, context, name); }
#else
(
	ipc_space_t task,
	mach_port_options_ptr_t options,
	mach_port_context_t context,
	mach_port_name_t *name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_destruct */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_destruct
#if	defined(LINTLIBRARY)
    (task, name, srdelta, guard)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_delta_t srdelta;
	mach_port_context_t guard;
{ return mach_port_destruct(task, name, srdelta, guard); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_delta_t srdelta,
	mach_port_context_t guard
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_guard */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_guard
#if	defined(LINTLIBRARY)
    (task, name, guard, strict)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_context_t guard;
	boolean_t strict;
{ return mach_port_guard(task, name, guard, strict); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_context_t guard,
	boolean_t strict
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_unguard */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_unguard
#if	defined(LINTLIBRARY)
    (task, name, guard)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_context_t guard;
{ return mach_port_unguard(task, name, guard); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_context_t guard
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_space_basic_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_space_basic_info
#if	defined(LINTLIBRARY)
    (task, basic_info)
	ipc_space_t task;
	ipc_info_space_basic_t *basic_info;
{ return mach_port_space_basic_info(task, basic_info); }
#else
(
	ipc_space_t task,
	ipc_info_space_basic_t *basic_info
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

#ifndef __Request__mach_port_subsystem__defined
#define __Request__mach_port_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
	} __Request__mach_port_names_t;
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
		mach_port_name_t name;
	} __Request__mach_port_type_t;
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
		mach_port_name_t old_name;
		mach_port_name_t new_name;
	} __Request__mach_port_rename_t;
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
		mach_port_right_t right;
		mach_port_name_t name;
	} __Request__mach_port_allocate_name_t;
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
		mach_port_name_t name;
	} __Request__mach_port_destroy_t;
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
		mach_port_name_t name;
		mach_port_right_t right;
	} __Request__mach_port_get_refs_t;
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
		mach_port_name_t name;
		mach_msg_trailer_type_t trailer_type;
		mach_port_seqno_t request_seqnop;
		mach_msg_type_number_t trailer_infopCnt;
	} __Request__mach_port_peek_t;
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
		mach_port_name_t name;
		mach_port_mscount_t mscount;
	} __Request__mach_port_set_mscount_t;
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
		mach_port_name_t name;
	} __Request__mach_port_get_set_status_t;
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
		mach_msg_port_descriptor_t notify;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_port_name_t name;
		mach_msg_id_t msgid;
		mach_port_mscount_t sync;
	} __Request__mach_port_request_notification_t;
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
		mach_port_name_t name;
		mach_port_seqno_t seqno;
	} __Request__mach_port_set_seqno_t;
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
		mach_port_name_t name;
		mach_port_flavor_t flavor;
		mach_msg_type_number_t port_info_outCnt;
	} __Request__mach_port_get_attributes_t;
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
		mach_port_name_t name;
		mach_port_flavor_t flavor;
		mach_msg_type_number_t port_infoCnt;
		integer_t port_info[17];
	} __Request__mach_port_set_attributes_t;
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
		mach_port_right_t right;
		mach_port_qos_t qos;
	} __Request__mach_port_allocate_qos_t;
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
		mach_msg_port_descriptor_t proto;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_port_right_t right;
		mach_port_qos_t qos;
		mach_port_name_t name;
	} __Request__mach_port_allocate_full_t;
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
		int table_entries;
	} __Request__task_set_port_space_t;
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
		mach_port_name_t name;
	} __Request__mach_port_get_srights_t;
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
	} __Request__mach_port_space_info_t;
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
		mach_port_name_t name;
	} __Request__mach_port_dnrequest_info_t;
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
		mach_port_name_t name;
	} __Request__mach_port_get_context_t;
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
		mach_port_name_t name;
		mach_port_context_t context;
	} __Request__mach_port_set_context_t;
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
		mach_port_name_t name;
	} __Request__mach_port_kobject_t;
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
		mach_msg_ool_descriptor_t options;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_port_context_t context;
	} __Request__mach_port_construct_t;
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
		mach_port_name_t name;
		mach_port_delta_t srdelta;
		mach_port_context_t guard;
	} __Request__mach_port_destruct_t;
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
		mach_port_name_t name;
		mach_port_context_t guard;
		boolean_t strict;
	} __Request__mach_port_guard_t;
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
		mach_port_name_t name;
		mach_port_context_t guard;
	} __Request__mach_port_unguard_t;
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
	} __Request__mach_port_space_basic_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__mach_port_subsystem__defined */

/* union of all requests */

#ifndef __RequestUnion__mach_port_subsystem__defined
#define __RequestUnion__mach_port_subsystem__defined
union __RequestUnion__mach_port_subsystem {
	__Request__mach_port_names_t Request_mach_port_names;
	__Request__mach_port_type_t Request_mach_port_type;
	__Request__mach_port_rename_t Request_mach_port_rename;
	__Request__mach_port_allocate_name_t Request_mach_port_allocate_name;
	__Request__mach_port_destroy_t Request_mach_port_destroy;
	__Request__mach_port_get_refs_t Request_mach_port_get_refs;
	__Request__mach_port_peek_t Request_mach_port_peek;
	__Request__mach_port_set_mscount_t Request_mach_port_set_mscount;
	__Request__mach_port_get_set_status_t Request_mach_port_get_set_status;
	__Request__mach_port_request_notification_t Request_mach_port_request_notification;
	__Request__mach_port_set_seqno_t Request_mach_port_set_seqno;
	__Request__mach_port_get_attributes_t Request_mach_port_get_attributes;
	__Request__mach_port_set_attributes_t Request_mach_port_set_attributes;
	__Request__mach_port_allocate_qos_t Request_mach_port_allocate_qos;
	__Request__mach_port_allocate_full_t Request_mach_port_allocate_full;
	__Request__task_set_port_space_t Request_task_set_port_space;
	__Request__mach_port_get_srights_t Request_mach_port_get_srights;
	__Request__mach_port_space_info_t Request_mach_port_space_info;
	__Request__mach_port_dnrequest_info_t Request_mach_port_dnrequest_info;
	__Request__mach_port_get_context_t Request_mach_port_get_context;
	__Request__mach_port_set_context_t Request_mach_port_set_context;
	__Request__mach_port_kobject_t Request_mach_port_kobject;
	__Request__mach_port_construct_t Request_mach_port_construct;
	__Request__mach_port_destruct_t Request_mach_port_destruct;
	__Request__mach_port_guard_t Request_mach_port_guard;
	__Request__mach_port_unguard_t Request_mach_port_unguard;
	__Request__mach_port_space_basic_info_t Request_mach_port_space_basic_info;
};
#endif /* !__RequestUnion__mach_port_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__mach_port_subsystem__defined
#define __Reply__mach_port_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t names;
		mach_msg_ool_descriptor_t types;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t namesCnt;
		mach_msg_type_number_t typesCnt;
	} __Reply__mach_port_names_t;
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
		mach_port_type_t ptype;
	} __Reply__mach_port_type_t;
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
	} __Reply__mach_port_rename_t;
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
	} __Reply__mach_port_allocate_name_t;
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
	} __Reply__mach_port_destroy_t;
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
		mach_port_urefs_t refs;
	} __Reply__mach_port_get_refs_t;
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
		mach_port_seqno_t request_seqnop;
		mach_msg_size_t msg_sizep;
		mach_msg_id_t msg_idp;
		mach_msg_type_number_t trailer_infopCnt;
		char trailer_infop[68];
	} __Reply__mach_port_peek_t;
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
	} __Reply__mach_port_set_mscount_t;
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
		mach_msg_ool_descriptor_t members;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t membersCnt;
	} __Reply__mach_port_get_set_status_t;
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
		mach_msg_port_descriptor_t previous;
		/* end of the kernel processed data */
	} __Reply__mach_port_request_notification_t;
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
	} __Reply__mach_port_set_seqno_t;
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
		mach_msg_type_number_t port_info_outCnt;
		integer_t port_info_out[17];
	} __Reply__mach_port_get_attributes_t;
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
	} __Reply__mach_port_set_attributes_t;
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
		mach_port_qos_t qos;
		mach_port_name_t name;
	} __Reply__mach_port_allocate_qos_t;
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
		mach_port_qos_t qos;
		mach_port_name_t name;
	} __Reply__mach_port_allocate_full_t;
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
	} __Reply__task_set_port_space_t;
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
		mach_port_rights_t srights;
	} __Reply__mach_port_get_srights_t;
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
		mach_msg_ool_descriptor_t table_info;
		mach_msg_ool_descriptor_t tree_info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		ipc_info_space_t space_info;
		mach_msg_type_number_t table_infoCnt;
		mach_msg_type_number_t tree_infoCnt;
	} __Reply__mach_port_space_info_t;
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
		unsigned dnr_total;
		unsigned dnr_used;
	} __Reply__mach_port_dnrequest_info_t;
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
		mach_port_context_t context;
	} __Reply__mach_port_get_context_t;
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
	} __Reply__mach_port_set_context_t;
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
		natural_t object_type;
		mach_vm_address_t object_addr;
	} __Reply__mach_port_kobject_t;
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
		mach_port_name_t name;
	} __Reply__mach_port_construct_t;
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
	} __Reply__mach_port_destruct_t;
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
	} __Reply__mach_port_guard_t;
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
	} __Reply__mach_port_unguard_t;
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
		ipc_info_space_basic_t basic_info;
	} __Reply__mach_port_space_basic_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__mach_port_subsystem__defined */

/* union of all replies */

#ifndef __ReplyUnion__mach_port_subsystem__defined
#define __ReplyUnion__mach_port_subsystem__defined
union __ReplyUnion__mach_port_subsystem {
	__Reply__mach_port_names_t Reply_mach_port_names;
	__Reply__mach_port_type_t Reply_mach_port_type;
	__Reply__mach_port_rename_t Reply_mach_port_rename;
	__Reply__mach_port_allocate_name_t Reply_mach_port_allocate_name;
	__Reply__mach_port_destroy_t Reply_mach_port_destroy;
	__Reply__mach_port_get_refs_t Reply_mach_port_get_refs;
	__Reply__mach_port_peek_t Reply_mach_port_peek;
	__Reply__mach_port_set_mscount_t Reply_mach_port_set_mscount;
	__Reply__mach_port_get_set_status_t Reply_mach_port_get_set_status;
	__Reply__mach_port_request_notification_t Reply_mach_port_request_notification;
	__Reply__mach_port_set_seqno_t Reply_mach_port_set_seqno;
	__Reply__mach_port_get_attributes_t Reply_mach_port_get_attributes;
	__Reply__mach_port_set_attributes_t Reply_mach_port_set_attributes;
	__Reply__mach_port_allocate_qos_t Reply_mach_port_allocate_qos;
	__Reply__mach_port_allocate_full_t Reply_mach_port_allocate_full;
	__Reply__task_set_port_space_t Reply_task_set_port_space;
	__Reply__mach_port_get_srights_t Reply_mach_port_get_srights;
	__Reply__mach_port_space_info_t Reply_mach_port_space_info;
	__Reply__mach_port_dnrequest_info_t Reply_mach_port_dnrequest_info;
	__Reply__mach_port_get_context_t Reply_mach_port_get_context;
	__Reply__mach_port_set_context_t Reply_mach_port_set_context;
	__Reply__mach_port_kobject_t Reply_mach_port_kobject;
	__Reply__mach_port_construct_t Reply_mach_port_construct;
	__Reply__mach_port_destruct_t Reply_mach_port_destruct;
	__Reply__mach_port_guard_t Reply_mach_port_guard;
	__Reply__mach_port_unguard_t Reply_mach_port_unguard;
	__Reply__mach_port_space_basic_info_t Reply_mach_port_space_basic_info;
};
#endif /* !__RequestUnion__mach_port_subsystem__defined */

#ifndef subsystem_to_name_map_mach_port
#define subsystem_to_name_map_mach_port \
    { "mach_port_names", 3200 },\
    { "mach_port_type", 3201 },\
    { "mach_port_rename", 3202 },\
    { "mach_port_allocate_name", 3203 },\
    { "mach_port_destroy", 3210 },\
    { "mach_port_get_refs", 3211 },\
    { "mach_port_peek", 3212 },\
    { "mach_port_set_mscount", 3213 },\
    { "mach_port_get_set_status", 3214 },\
    { "mach_port_request_notification", 3215 },\
    { "mach_port_set_seqno", 3216 },\
    { "mach_port_get_attributes", 3217 },\
    { "mach_port_set_attributes", 3218 },\
    { "mach_port_allocate_qos", 3219 },\
    { "mach_port_allocate_full", 3220 },\
    { "task_set_port_space", 3221 },\
    { "mach_port_get_srights", 3222 },\
    { "mach_port_space_info", 3223 },\
    { "mach_port_dnrequest_info", 3224 },\
    { "mach_port_get_context", 3228 },\
    { "mach_port_set_context", 3229 },\
    { "mach_port_kobject", 3230 },\
    { "mach_port_construct", 3231 },\
    { "mach_port_destruct", 3232 },\
    { "mach_port_guard", 3233 },\
    { "mach_port_unguard", 3234 },\
    { "mach_port_space_basic_info", 3235 }
#endif

#ifdef __AfterMigUserHeader
__AfterMigUserHeader
#endif /* __AfterMigUserHeader */

#endif	 /* _mach_port_user_ */

/*
 * IDENTIFICATION:
 * stub generated Thu Jun 11 18:17:45 2015
 * with a MiG generated Thu Jun 11 16:16:11 PDT 2015 by kmacy@serenity
 * OPTIONS: 
 *	KernelServer
 */

/* Module mach_port */

#define	__MIG_check__Request__mach_port_subsystem__ 1

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

#ifndef	mig_internal
#define	mig_internal	static __inline__
#endif	/* mig_internal */

#ifndef	mig_external
#define mig_external
#endif	/* mig_external */

#if	!defined(__MigTypeCheck) && defined(TypeCheck)
#define	__MigTypeCheck		TypeCheck	/* Legacy setting */
#endif	/* !defined(__MigTypeCheck) */

#if	!defined(__MigKernelSpecificCode) && defined(_MIG_KERNEL_SPECIFIC_CODE_)
#define	__MigKernelSpecificCode	_MIG_KERNEL_SPECIFIC_CODE_	/* Legacy setting */
#endif	/* !defined(__MigKernelSpecificCode) */

#ifndef	LimitCheck
#define	LimitCheck 0
#endif	/* LimitCheck */

#ifndef	min
#define	min(a,b)  ( ((a) < (b))? (a): (b) )
#endif	/* min */

#if !defined(_WALIGN_)
#define _WALIGN_(x) (((x) + 7) & ~7)
#endif /* !defined(_WALIGN_) */

#if !defined(_WALIGNSZ_)
#define _WALIGNSZ_(x) _WALIGN_(sizeof(x))
#endif /* !defined(_WALIGNSZ_) */

#ifndef	UseStaticTemplates
#define	UseStaticTemplates	1
#endif	/* UseStaticTemplates */

#define _WALIGN_(x) (((x) + 7) & ~7)
#define _WALIGNSZ_(x) _WALIGN_(sizeof(x))
#ifndef	__DeclareRcvRpc
#define	__DeclareRcvRpc(_NUM_, _NAME_)
#endif	/* __DeclareRcvRpc */

#ifndef	__BeforeRcvRpc
#define	__BeforeRcvRpc(_NUM_, _NAME_)
#endif	/* __BeforeRcvRpc */

#ifndef	__AfterRcvRpc
#define	__AfterRcvRpc(_NUM_, _NAME_)
#endif	/* __AfterRcvRpc */

#ifndef	__DeclareRcvSimple
#define	__DeclareRcvSimple(_NUM_, _NAME_)
#endif	/* __DeclareRcvSimple */

#ifndef	__BeforeRcvSimple
#define	__BeforeRcvSimple(_NUM_, _NAME_)
#endif	/* __BeforeRcvSimple */

#ifndef	__AfterRcvSimple
#define	__AfterRcvSimple(_NUM_, _NAME_)
#endif	/* __AfterRcvSimple */

#define novalue void
#if	__MigKernelSpecificCode
#define msgh_request_port	msgh_remote_port
#define MACH_MSGH_BITS_REQUEST(bits)	MACH_MSGH_BITS_REMOTE(bits)
#define msgh_reply_port		msgh_local_port
#define MACH_MSGH_BITS_REPLY(bits)	MACH_MSGH_BITS_LOCAL(bits)
#else
#define msgh_request_port	msgh_local_port
#define MACH_MSGH_BITS_REQUEST(bits)	MACH_MSGH_BITS_LOCAL(bits)
#define msgh_reply_port		msgh_remote_port
#define MACH_MSGH_BITS_REPLY(bits)	MACH_MSGH_BITS_REMOTE(bits)
#endif /* __MigKernelSpecificCode */

#define MIG_RETURN_ERROR(X, code)	{\
				((mig_reply_error_t *)X)->RetCode = code;\
				((mig_reply_error_t *)X)->NDR = NDR_record;\
				return;\
				}

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
		mach_port_right_t right;
	} __Request__mach_port_allocate_t;
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
	} __Request__mach_port_deallocate_t;
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
		mach_msg_port_descriptor_t poly;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_port_name_t name;
	} __Request__mach_port_insert_right_t;
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
		mach_msg_type_name_t msgt_name;
	} __Request__mach_port_extract_right_t;
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
		mach_port_delta_t delta;
	} __Request__mach_port_mod_refs_t;
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
		mach_port_name_t member;
		mach_port_name_t after;
	} __Request__mach_port_move_member_t;
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
		mach_port_name_t pset;
	} __Request__mach_port_insert_member_t;
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
		mach_port_name_t pset;
	} __Request__mach_port_extract_member_t;
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
		mach_vm_address_t context;
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
		uint64_t context;
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
		uint64_t guard;
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
		uint64_t guard;
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
		uint64_t guard;
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
		mach_port_name_t name;
	} __Reply__mach_port_allocate_t;
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
	} __Reply__mach_port_deallocate_t;
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
	} __Reply__mach_port_insert_right_t;
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
		mach_msg_port_descriptor_t poly;
		/* end of the kernel processed data */
	} __Reply__mach_port_extract_right_t;
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
	} __Reply__mach_port_mod_refs_t;
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
	} __Reply__mach_port_move_member_t;
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
	} __Reply__mach_port_insert_member_t;
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
	} __Reply__mach_port_extract_member_t;
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
		mach_vm_address_t context;
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
	__Reply__mach_port_allocate_t Reply_mach_port_allocate;
	__Reply__mach_port_deallocate_t Reply_mach_port_deallocate;
	__Reply__mach_port_insert_right_t Reply_mach_port_insert_right;
	__Reply__mach_port_extract_right_t Reply_mach_port_extract_right;
	__Reply__mach_port_mod_refs_t Reply_mach_port_mod_refs;
	__Reply__mach_port_move_member_t Reply_mach_port_move_member;
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
	__Reply__mach_port_insert_member_t Reply_mach_port_insert_member;
	__Reply__mach_port_extract_member_t Reply_mach_port_extract_member;
	__Reply__mach_port_get_context_t Reply_mach_port_get_context;
	__Reply__mach_port_set_context_t Reply_mach_port_set_context;
	__Reply__mach_port_kobject_t Reply_mach_port_kobject;
	__Reply__mach_port_construct_t Reply_mach_port_construct;
	__Reply__mach_port_destruct_t Reply_mach_port_destruct;
	__Reply__mach_port_guard_t Reply_mach_port_guard;
	__Reply__mach_port_unguard_t Reply_mach_port_unguard;
	__Reply__mach_port_space_basic_info_t Reply_mach_port_space_basic_info;
};
#endif /* __RequestUnion__mach_port_subsystem__defined */
/* Forward Declarations */


mig_internal novalue _Xmach_port_names
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_type
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_rename
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_allocate_name
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_allocate
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_deallocate
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_insert_right
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_extract_right
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_mod_refs
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_move_member
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_destroy
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_get_refs
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_peek
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_set_mscount
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_get_set_status
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_request_notification
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_set_seqno
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_get_attributes
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_set_attributes
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_allocate_qos
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_allocate_full
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_set_port_space
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_get_srights
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_space_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_dnrequest_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_insert_member
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_extract_member
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_get_context
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_set_context
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_kobject
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_construct
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_destruct
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_guard
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_unguard
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_port_space_basic_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);


#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_names_t__defined)
#define __MIG_check__Request__mach_port_names_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_names_t(__attribute__((__unused__)) __Request__mach_port_names_t *In0P)
{

	typedef __Request__mach_port_names_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_names_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_names */
mig_internal novalue _Xmach_port_names
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_names_t __Request;
	typedef __Reply__mach_port_names_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_names_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_names_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t namesTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t typesTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t namesTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t typesTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	ipc_space_t task;

	__DeclareRcvRpc(3200, "mach_port_names")
	__BeforeRcvRpc(3200, "mach_port_names")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_names_t__defined)
	check_result = __MIG_check__Request__mach_port_names_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_names_t__defined) */

#if	UseStaticTemplates
	OutP->names = namesTemplate;
#else	/* UseStaticTemplates */
	OutP->names.deallocate =  FALSE;
	OutP->names.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->names.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


#if	UseStaticTemplates
	OutP->types = typesTemplate;
#else	/* UseStaticTemplates */
	OutP->types.deallocate =  FALSE;
	OutP->types.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->types.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->namesCnt = 0;

	OutP->typesCnt = 0;

	RetCode = mach_port_names(task, (mach_port_name_array_t *)&(OutP->names.address), &OutP->namesCnt, (mach_port_type_array_t *)&(OutP->types.address), &OutP->typesCnt);
	space_deallocate(task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->names.size = OutP->namesCnt * 4;

	OutP->types.size = OutP->typesCnt * 4;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 2;
	__AfterRcvRpc(3200, "mach_port_names")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_type_t__defined)
#define __MIG_check__Request__mach_port_type_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_type_t(__attribute__((__unused__)) __Request__mach_port_type_t *In0P)
{

	typedef __Request__mach_port_type_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_type_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_type */
mig_internal novalue _Xmach_port_type
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_type_t __Request;
	typedef __Reply__mach_port_type_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_type_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_type_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3201, "mach_port_type")
	__BeforeRcvRpc(3201, "mach_port_type")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_type_t__defined)
	check_result = __MIG_check__Request__mach_port_type_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_type_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_type(task, In0P->name, &OutP->ptype);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3201, "mach_port_type")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_rename_t__defined)
#define __MIG_check__Request__mach_port_rename_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_rename_t(__attribute__((__unused__)) __Request__mach_port_rename_t *In0P)
{

	typedef __Request__mach_port_rename_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_rename_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_rename */
mig_internal novalue _Xmach_port_rename
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_rename_t __Request;
	typedef __Reply__mach_port_rename_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_rename_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_rename_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3202, "mach_port_rename")
	__BeforeRcvRpc(3202, "mach_port_rename")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_rename_t__defined)
	check_result = __MIG_check__Request__mach_port_rename_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_rename_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_rename(task, In0P->old_name, In0P->new_name);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3202, "mach_port_rename")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_allocate_name_t__defined)
#define __MIG_check__Request__mach_port_allocate_name_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_allocate_name_t(__attribute__((__unused__)) __Request__mach_port_allocate_name_t *In0P)
{

	typedef __Request__mach_port_allocate_name_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_allocate_name_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_allocate_name */
mig_internal novalue _Xmach_port_allocate_name
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_allocate_name_t __Request;
	typedef __Reply__mach_port_allocate_name_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_allocate_name_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_allocate_name_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3203, "mach_port_allocate_name")
	__BeforeRcvRpc(3203, "mach_port_allocate_name")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_allocate_name_t__defined)
	check_result = __MIG_check__Request__mach_port_allocate_name_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_allocate_name_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_allocate_name(task, In0P->right, In0P->name);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3203, "mach_port_allocate_name")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_allocate_t__defined)
#define __MIG_check__Request__mach_port_allocate_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_allocate_t(__attribute__((__unused__)) __Request__mach_port_allocate_t *In0P)
{

	typedef __Request__mach_port_allocate_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_allocate_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_port_allocate */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_allocate
#if	defined(LINTLIBRARY)
    (task, right, name)
	ipc_space_t task;
	mach_port_right_t right;
	mach_port_name_t *name;
{ return mach_port_allocate(task, right, name); }
#else
(
	ipc_space_t task,
	mach_port_right_t right,
	mach_port_name_t *name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_allocate */
mig_internal novalue _Xmach_port_allocate
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_allocate_t __Request;
	typedef __Reply__mach_port_allocate_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_allocate_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_allocate_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3204, "mach_port_allocate")
	__BeforeRcvRpc(3204, "mach_port_allocate")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_allocate_t__defined)
	check_result = __MIG_check__Request__mach_port_allocate_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_allocate_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_allocate(task, In0P->right, &OutP->name);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3204, "mach_port_allocate")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_deallocate_t__defined)
#define __MIG_check__Request__mach_port_deallocate_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_deallocate_t(__attribute__((__unused__)) __Request__mach_port_deallocate_t *In0P)
{

	typedef __Request__mach_port_deallocate_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_deallocate_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_port_deallocate */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_deallocate
#if	defined(LINTLIBRARY)
    (task, name)
	ipc_space_t task;
	mach_port_name_t name;
{ return mach_port_deallocate(task, name); }
#else
(
	ipc_space_t task,
	mach_port_name_t name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_deallocate */
mig_internal novalue _Xmach_port_deallocate
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_deallocate_t __Request;
	typedef __Reply__mach_port_deallocate_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_deallocate_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_deallocate_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3205, "mach_port_deallocate")
	__BeforeRcvRpc(3205, "mach_port_deallocate")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_deallocate_t__defined)
	check_result = __MIG_check__Request__mach_port_deallocate_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_deallocate_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_deallocate(task, In0P->name);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3205, "mach_port_deallocate")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_insert_right_t__defined)
#define __MIG_check__Request__mach_port_insert_right_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_insert_right_t(__attribute__((__unused__)) __Request__mach_port_insert_right_t *In0P)
{

	typedef __Request__mach_port_insert_right_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->poly.type != MACH_MSG_PORT_DESCRIPTOR)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_insert_right_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_port_insert_right */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_insert_right
#if	defined(LINTLIBRARY)
    (task, name, poly, polyPoly)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_t poly;
	mach_msg_type_name_t polyPoly;
{ return mach_port_insert_right(task, name, poly, polyPoly); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_t poly,
	mach_msg_type_name_t polyPoly
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_insert_right */
mig_internal novalue _Xmach_port_insert_right
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t poly;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_port_name_t name;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_insert_right_t __Request;
	typedef __Reply__mach_port_insert_right_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_insert_right_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_insert_right_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3206, "mach_port_insert_right")
	__BeforeRcvRpc(3206, "mach_port_insert_right")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_insert_right_t__defined)
	check_result = __MIG_check__Request__mach_port_insert_right_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_insert_right_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_insert_right(task, In0P->name, In0P->poly.name, In0P->poly.disposition);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3206, "mach_port_insert_right")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_extract_right_t__defined)
#define __MIG_check__Request__mach_port_extract_right_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_extract_right_t(__attribute__((__unused__)) __Request__mach_port_extract_right_t *In0P)
{

	typedef __Request__mach_port_extract_right_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_extract_right_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_port_extract_right */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_extract_right
#if	defined(LINTLIBRARY)
    (task, name, msgt_name, poly, polyPoly)
	ipc_space_t task;
	mach_port_name_t name;
	mach_msg_type_name_t msgt_name;
	mach_port_t *poly;
	mach_msg_type_name_t *polyPoly;
{ return mach_port_extract_right(task, name, msgt_name, poly, polyPoly); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_msg_type_name_t msgt_name,
	mach_port_t *poly,
	mach_msg_type_name_t *polyPoly
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_extract_right */
mig_internal novalue _Xmach_port_extract_right
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_type_name_t msgt_name;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_extract_right_t __Request;
	typedef __Reply__mach_port_extract_right_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_extract_right_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_extract_right_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t polyTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = ((mach_msg_type_name_t) -1),
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t polyTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = ((mach_msg_type_name_t) -1),
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	ipc_space_t task;
	mach_msg_type_name_t polyPoly;

	__DeclareRcvRpc(3207, "mach_port_extract_right")
	__BeforeRcvRpc(3207, "mach_port_extract_right")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_extract_right_t__defined)
	check_result = __MIG_check__Request__mach_port_extract_right_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_extract_right_t__defined) */

#if	UseStaticTemplates
	OutP->poly = polyTemplate;
#else	/* UseStaticTemplates */
	OutP->poly.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	task = convert_port_to_space(In0P->Head.msgh_request_port);

	RetCode = mach_port_extract_right(task, In0P->name, In0P->msgt_name, &OutP->poly.name, &polyPoly);
	space_deallocate(task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->poly.disposition = polyPoly;

#if	__MigKernelSpecificCode
	if (polyPoly == MACH_MSG_TYPE_PORT_RECEIVE)
	  if (IP_VALID((ipc_port_t) In0P->Head.msgh_reply_port) &&
	    IP_VALID((ipc_port_t) OutP->poly.name) &&
	    ipc_port_check_circularity((ipc_port_t) OutP->poly.name, (ipc_port_t) In0P->Head.msgh_reply_port))
		OutP->Head.msgh_bits |= MACH_MSGH_BITS_CIRCULAR;
#endif /* __MigKernelSpecificCode */

	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3207, "mach_port_extract_right")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_mod_refs_t__defined)
#define __MIG_check__Request__mach_port_mod_refs_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_mod_refs_t(__attribute__((__unused__)) __Request__mach_port_mod_refs_t *In0P)
{

	typedef __Request__mach_port_mod_refs_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_mod_refs_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_port_mod_refs */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_mod_refs
#if	defined(LINTLIBRARY)
    (task, name, right, delta)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_right_t right;
	mach_port_delta_t delta;
{ return mach_port_mod_refs(task, name, right, delta); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_right_t right,
	mach_port_delta_t delta
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_mod_refs */
mig_internal novalue _Xmach_port_mod_refs
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_port_delta_t delta;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_mod_refs_t __Request;
	typedef __Reply__mach_port_mod_refs_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_mod_refs_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_mod_refs_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3208, "mach_port_mod_refs")
	__BeforeRcvRpc(3208, "mach_port_mod_refs")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_mod_refs_t__defined)
	check_result = __MIG_check__Request__mach_port_mod_refs_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_mod_refs_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_mod_refs(task, In0P->name, In0P->right, In0P->delta);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3208, "mach_port_mod_refs")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_move_member_t__defined)
#define __MIG_check__Request__mach_port_move_member_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_move_member_t(__attribute__((__unused__)) __Request__mach_port_move_member_t *In0P)
{

	typedef __Request__mach_port_move_member_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_move_member_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_port_move_member */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_move_member
#if	defined(LINTLIBRARY)
    (task, member, after)
	ipc_space_t task;
	mach_port_name_t member;
	mach_port_name_t after;
{ return mach_port_move_member(task, member, after); }
#else
(
	ipc_space_t task,
	mach_port_name_t member,
	mach_port_name_t after
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_move_member */
mig_internal novalue _Xmach_port_move_member
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_port_name_t member;
		mach_port_name_t after;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_move_member_t __Request;
	typedef __Reply__mach_port_move_member_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_move_member_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_move_member_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3209, "mach_port_move_member")
	__BeforeRcvRpc(3209, "mach_port_move_member")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_move_member_t__defined)
	check_result = __MIG_check__Request__mach_port_move_member_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_move_member_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_move_member(task, In0P->member, In0P->after);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3209, "mach_port_move_member")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_destroy_t__defined)
#define __MIG_check__Request__mach_port_destroy_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_destroy_t(__attribute__((__unused__)) __Request__mach_port_destroy_t *In0P)
{

	typedef __Request__mach_port_destroy_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_destroy_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_destroy */
mig_internal novalue _Xmach_port_destroy
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_destroy_t __Request;
	typedef __Reply__mach_port_destroy_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_destroy_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_destroy_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3210, "mach_port_destroy")
	__BeforeRcvRpc(3210, "mach_port_destroy")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_destroy_t__defined)
	check_result = __MIG_check__Request__mach_port_destroy_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_destroy_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_destroy(task, In0P->name);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3210, "mach_port_destroy")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_get_refs_t__defined)
#define __MIG_check__Request__mach_port_get_refs_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_get_refs_t(__attribute__((__unused__)) __Request__mach_port_get_refs_t *In0P)
{

	typedef __Request__mach_port_get_refs_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_get_refs_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_get_refs */
mig_internal novalue _Xmach_port_get_refs
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_get_refs_t __Request;
	typedef __Reply__mach_port_get_refs_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_get_refs_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_get_refs_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3211, "mach_port_get_refs")
	__BeforeRcvRpc(3211, "mach_port_get_refs")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_get_refs_t__defined)
	check_result = __MIG_check__Request__mach_port_get_refs_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_get_refs_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_get_refs(task, In0P->name, In0P->right, &OutP->refs);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3211, "mach_port_get_refs")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_peek_t__defined)
#define __MIG_check__Request__mach_port_peek_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_peek_t(__attribute__((__unused__)) __Request__mach_port_peek_t *In0P)
{

	typedef __Request__mach_port_peek_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_peek_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_peek */
mig_internal novalue _Xmach_port_peek
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_peek_t __Request;
	typedef __Reply__mach_port_peek_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_peek_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_peek_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3212, "mach_port_peek")
	__BeforeRcvRpc(3212, "mach_port_peek")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_peek_t__defined)
	check_result = __MIG_check__Request__mach_port_peek_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_peek_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->trailer_infopCnt = 68;
	if (In0P->trailer_infopCnt < OutP->trailer_infopCnt)
		OutP->trailer_infopCnt = In0P->trailer_infopCnt;

	OutP->RetCode = mach_port_peek(task, In0P->name, In0P->trailer_type, &In0P->request_seqnop, &OutP->msg_sizep, &OutP->msg_idp, OutP->trailer_infop, &OutP->trailer_infopCnt);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->request_seqnop = In0P->request_seqnop;
	OutP->Head.msgh_size = (sizeof(Reply) - 68) + (_WALIGN_((OutP->trailer_infopCnt + 3) & ~3));

	__AfterRcvRpc(3212, "mach_port_peek")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_set_mscount_t__defined)
#define __MIG_check__Request__mach_port_set_mscount_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_set_mscount_t(__attribute__((__unused__)) __Request__mach_port_set_mscount_t *In0P)
{

	typedef __Request__mach_port_set_mscount_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_set_mscount_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_set_mscount */
mig_internal novalue _Xmach_port_set_mscount
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_set_mscount_t __Request;
	typedef __Reply__mach_port_set_mscount_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_set_mscount_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_set_mscount_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3213, "mach_port_set_mscount")
	__BeforeRcvRpc(3213, "mach_port_set_mscount")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_set_mscount_t__defined)
	check_result = __MIG_check__Request__mach_port_set_mscount_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_set_mscount_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_set_mscount(task, In0P->name, In0P->mscount);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3213, "mach_port_set_mscount")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_get_set_status_t__defined)
#define __MIG_check__Request__mach_port_get_set_status_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_get_set_status_t(__attribute__((__unused__)) __Request__mach_port_get_set_status_t *In0P)
{

	typedef __Request__mach_port_get_set_status_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_get_set_status_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_get_set_status */
mig_internal novalue _Xmach_port_get_set_status
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_get_set_status_t __Request;
	typedef __Reply__mach_port_get_set_status_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_get_set_status_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_get_set_status_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t membersTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t membersTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	ipc_space_t task;

	__DeclareRcvRpc(3214, "mach_port_get_set_status")
	__BeforeRcvRpc(3214, "mach_port_get_set_status")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_get_set_status_t__defined)
	check_result = __MIG_check__Request__mach_port_get_set_status_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_get_set_status_t__defined) */

#if	UseStaticTemplates
	OutP->members = membersTemplate;
#else	/* UseStaticTemplates */
	OutP->members.deallocate =  FALSE;
	OutP->members.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->members.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->membersCnt = 0;

	RetCode = mach_port_get_set_status(task, In0P->name, (mach_port_name_array_t *)&(OutP->members.address), &OutP->membersCnt);
	space_deallocate(task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->members.size = OutP->membersCnt * 4;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3214, "mach_port_get_set_status")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_request_notification_t__defined)
#define __MIG_check__Request__mach_port_request_notification_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_request_notification_t(__attribute__((__unused__)) __Request__mach_port_request_notification_t *In0P)
{

	typedef __Request__mach_port_request_notification_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->notify.type != MACH_MSG_PORT_DESCRIPTOR || (
	    In0P->notify.disposition != MACH_MSG_TYPE_MOVE_SEND_ONCE &&  
	    In0P->notify.disposition != MACH_MSG_TYPE_MAKE_SEND_ONCE))
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_request_notification_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_port_request_notification */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_request_notification
#if	defined(LINTLIBRARY)
    (task, name, msgid, sync, notify, previous)
	ipc_space_t task;
	mach_port_name_t name;
	mach_msg_id_t msgid;
	mach_port_mscount_t sync;
	mach_port_t notify;
	mach_port_t *previous;
{ return mach_port_request_notification(task, name, msgid, sync, notify, previous); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_msg_id_t msgid,
	mach_port_mscount_t sync,
	mach_port_t notify,
	mach_port_t *previous
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_request_notification */
mig_internal novalue _Xmach_port_request_notification
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_request_notification_t __Request;
	typedef __Reply__mach_port_request_notification_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_request_notification_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_request_notification_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t previousTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 18,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t previousTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 18,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	ipc_space_t task;

	__DeclareRcvRpc(3215, "mach_port_request_notification")
	__BeforeRcvRpc(3215, "mach_port_request_notification")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_request_notification_t__defined)
	check_result = __MIG_check__Request__mach_port_request_notification_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_request_notification_t__defined) */

#if	UseStaticTemplates
	OutP->previous = previousTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->previous.disposition = 18;
#else
	OutP->previous.disposition = 18;
#endif /* __MigKernelSpecificCode */
	OutP->previous.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	task = convert_port_to_space(In0P->Head.msgh_request_port);

	RetCode = mach_port_request_notification(task, In0P->name, In0P->msgid, In0P->sync, In0P->notify.name, &OutP->previous.name);
	space_deallocate(task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3215, "mach_port_request_notification")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_set_seqno_t__defined)
#define __MIG_check__Request__mach_port_set_seqno_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_set_seqno_t(__attribute__((__unused__)) __Request__mach_port_set_seqno_t *In0P)
{

	typedef __Request__mach_port_set_seqno_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_set_seqno_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_set_seqno */
mig_internal novalue _Xmach_port_set_seqno
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_set_seqno_t __Request;
	typedef __Reply__mach_port_set_seqno_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_set_seqno_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_set_seqno_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3216, "mach_port_set_seqno")
	__BeforeRcvRpc(3216, "mach_port_set_seqno")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_set_seqno_t__defined)
	check_result = __MIG_check__Request__mach_port_set_seqno_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_set_seqno_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_set_seqno(task, In0P->name, In0P->seqno);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3216, "mach_port_set_seqno")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_get_attributes_t__defined)
#define __MIG_check__Request__mach_port_get_attributes_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_get_attributes_t(__attribute__((__unused__)) __Request__mach_port_get_attributes_t *In0P)
{

	typedef __Request__mach_port_get_attributes_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_get_attributes_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_get_attributes */
mig_internal novalue _Xmach_port_get_attributes
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_get_attributes_t __Request;
	typedef __Reply__mach_port_get_attributes_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_get_attributes_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_get_attributes_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3217, "mach_port_get_attributes")
	__BeforeRcvRpc(3217, "mach_port_get_attributes")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_get_attributes_t__defined)
	check_result = __MIG_check__Request__mach_port_get_attributes_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_get_attributes_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->port_info_outCnt = 17;
	if (In0P->port_info_outCnt < OutP->port_info_outCnt)
		OutP->port_info_outCnt = In0P->port_info_outCnt;

	OutP->RetCode = mach_port_get_attributes(task, In0P->name, In0P->flavor, OutP->port_info_out, &OutP->port_info_outCnt);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;

	OutP->Head.msgh_size = (sizeof(Reply) - 68) + (_WALIGN_((4 * OutP->port_info_outCnt)));

	__AfterRcvRpc(3217, "mach_port_get_attributes")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_set_attributes_t__defined)
#define __MIG_check__Request__mach_port_set_attributes_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_set_attributes_t(__attribute__((__unused__)) __Request__mach_port_set_attributes_t *In0P)
{

	typedef __Request__mach_port_set_attributes_t __Request;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	msgh_size = In0P->Head.msgh_size;
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (msgh_size < (sizeof(__Request) - 68)) ||  (msgh_size > (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if defined(__NDR_convert__int_rep__Request__mach_port_set_attributes_t__port_infoCnt__defined)
	if (In0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Request__mach_port_set_attributes_t__port_infoCnt(&In0P->port_infoCnt, In0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Request__mach_port_set_attributes_t__port_infoCnt__defined */
#if	__MigTypeCheck
	if ( In0P->port_infoCnt > 17 )
		return MIG_BAD_ARGUMENTS;
	if (((msgh_size - (sizeof(__Request) - 68)) / 4 < In0P->port_infoCnt) ||
	    (msgh_size != (sizeof(__Request) - 68) + _WALIGN_(4 * In0P->port_infoCnt)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_set_attributes_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_set_attributes */
mig_internal novalue _Xmach_port_set_attributes
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_set_attributes_t __Request;
	typedef __Reply__mach_port_set_attributes_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_set_attributes_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_set_attributes_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3218, "mach_port_set_attributes")
	__BeforeRcvRpc(3218, "mach_port_set_attributes")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_set_attributes_t__defined)
	check_result = __MIG_check__Request__mach_port_set_attributes_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_set_attributes_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_set_attributes(task, In0P->name, In0P->flavor, In0P->port_info, In0P->port_infoCnt);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3218, "mach_port_set_attributes")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_allocate_qos_t__defined)
#define __MIG_check__Request__mach_port_allocate_qos_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_allocate_qos_t(__attribute__((__unused__)) __Request__mach_port_allocate_qos_t *In0P)
{

	typedef __Request__mach_port_allocate_qos_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_allocate_qos_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_allocate_qos */
mig_internal novalue _Xmach_port_allocate_qos
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_allocate_qos_t __Request;
	typedef __Reply__mach_port_allocate_qos_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_allocate_qos_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_allocate_qos_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3219, "mach_port_allocate_qos")
	__BeforeRcvRpc(3219, "mach_port_allocate_qos")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_allocate_qos_t__defined)
	check_result = __MIG_check__Request__mach_port_allocate_qos_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_allocate_qos_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_allocate_qos(task, In0P->right, &In0P->qos, &OutP->name);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->qos = In0P->qos;

	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3219, "mach_port_allocate_qos")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_allocate_full_t__defined)
#define __MIG_check__Request__mach_port_allocate_full_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_allocate_full_t(__attribute__((__unused__)) __Request__mach_port_allocate_full_t *In0P)
{

	typedef __Request__mach_port_allocate_full_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->proto.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->proto.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_allocate_full_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_allocate_full */
mig_internal novalue _Xmach_port_allocate_full
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_allocate_full_t __Request;
	typedef __Reply__mach_port_allocate_full_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_allocate_full_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_allocate_full_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3220, "mach_port_allocate_full")
	__BeforeRcvRpc(3220, "mach_port_allocate_full")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_allocate_full_t__defined)
	check_result = __MIG_check__Request__mach_port_allocate_full_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_allocate_full_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_allocate_full(task, In0P->right, In0P->proto.name, &In0P->qos, &In0P->name);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->qos = In0P->qos;

	OutP->name = In0P->name;

	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3220, "mach_port_allocate_full")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__task_set_port_space_t__defined)
#define __MIG_check__Request__task_set_port_space_t__defined

mig_internal kern_return_t __MIG_check__Request__task_set_port_space_t(__attribute__((__unused__)) __Request__task_set_port_space_t *In0P)
{

	typedef __Request__task_set_port_space_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_set_port_space_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine task_set_port_space */
mig_internal novalue _Xtask_set_port_space
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_set_port_space_t __Request;
	typedef __Reply__task_set_port_space_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_set_port_space_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_set_port_space_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3221, "task_set_port_space")
	__BeforeRcvRpc(3221, "task_set_port_space")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_set_port_space_t__defined)
	check_result = __MIG_check__Request__task_set_port_space_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_set_port_space_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = task_set_port_space(task, In0P->table_entries);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3221, "task_set_port_space")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_get_srights_t__defined)
#define __MIG_check__Request__mach_port_get_srights_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_get_srights_t(__attribute__((__unused__)) __Request__mach_port_get_srights_t *In0P)
{

	typedef __Request__mach_port_get_srights_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_get_srights_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_get_srights */
mig_internal novalue _Xmach_port_get_srights
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_get_srights_t __Request;
	typedef __Reply__mach_port_get_srights_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_get_srights_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_get_srights_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3222, "mach_port_get_srights")
	__BeforeRcvRpc(3222, "mach_port_get_srights")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_get_srights_t__defined)
	check_result = __MIG_check__Request__mach_port_get_srights_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_get_srights_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_get_srights(task, In0P->name, &OutP->srights);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3222, "mach_port_get_srights")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_space_info_t__defined)
#define __MIG_check__Request__mach_port_space_info_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_space_info_t(__attribute__((__unused__)) __Request__mach_port_space_info_t *In0P)
{

	typedef __Request__mach_port_space_info_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_space_info_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_space_info */
mig_internal novalue _Xmach_port_space_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_space_info_t __Request;
	typedef __Reply__mach_port_space_info_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_space_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_space_info_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t table_infoTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t tree_infoTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t table_infoTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t tree_infoTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	ipc_space_t task;

	__DeclareRcvRpc(3223, "mach_port_space_info")
	__BeforeRcvRpc(3223, "mach_port_space_info")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_space_info_t__defined)
	check_result = __MIG_check__Request__mach_port_space_info_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_space_info_t__defined) */

#if	UseStaticTemplates
	OutP->table_info = table_infoTemplate;
#else	/* UseStaticTemplates */
	OutP->table_info.deallocate =  FALSE;
	OutP->table_info.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->table_info.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


#if	UseStaticTemplates
	OutP->tree_info = tree_infoTemplate;
#else	/* UseStaticTemplates */
	OutP->tree_info.deallocate =  FALSE;
	OutP->tree_info.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->tree_info.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->table_infoCnt = 0;

	OutP->tree_infoCnt = 0;

	RetCode = mach_port_space_info(task, &OutP->space_info, (ipc_info_name_array_t *)&(OutP->table_info.address), &OutP->table_infoCnt, (ipc_info_tree_name_array_t *)&(OutP->tree_info.address), &OutP->tree_infoCnt);
	space_deallocate(task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->table_info.size = OutP->table_infoCnt * 28;

	OutP->tree_info.size = OutP->tree_infoCnt * 36;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 2;
	__AfterRcvRpc(3223, "mach_port_space_info")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_dnrequest_info_t__defined)
#define __MIG_check__Request__mach_port_dnrequest_info_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_dnrequest_info_t(__attribute__((__unused__)) __Request__mach_port_dnrequest_info_t *In0P)
{

	typedef __Request__mach_port_dnrequest_info_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_dnrequest_info_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_dnrequest_info */
mig_internal novalue _Xmach_port_dnrequest_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_dnrequest_info_t __Request;
	typedef __Reply__mach_port_dnrequest_info_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_dnrequest_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_dnrequest_info_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3224, "mach_port_dnrequest_info")
	__BeforeRcvRpc(3224, "mach_port_dnrequest_info")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_dnrequest_info_t__defined)
	check_result = __MIG_check__Request__mach_port_dnrequest_info_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_dnrequest_info_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_dnrequest_info(task, In0P->name, &OutP->dnr_total, &OutP->dnr_used);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3224, "mach_port_dnrequest_info")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_insert_member_t__defined)
#define __MIG_check__Request__mach_port_insert_member_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_insert_member_t(__attribute__((__unused__)) __Request__mach_port_insert_member_t *In0P)
{

	typedef __Request__mach_port_insert_member_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_insert_member_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_port_insert_member */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_insert_member
#if	defined(LINTLIBRARY)
    (task, name, pset)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_name_t pset;
{ return mach_port_insert_member(task, name, pset); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_name_t pset
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_insert_member */
mig_internal novalue _Xmach_port_insert_member
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_port_name_t pset;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_insert_member_t __Request;
	typedef __Reply__mach_port_insert_member_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_insert_member_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_insert_member_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3226, "mach_port_insert_member")
	__BeforeRcvRpc(3226, "mach_port_insert_member")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_insert_member_t__defined)
	check_result = __MIG_check__Request__mach_port_insert_member_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_insert_member_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_insert_member(task, In0P->name, In0P->pset);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3226, "mach_port_insert_member")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_extract_member_t__defined)
#define __MIG_check__Request__mach_port_extract_member_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_extract_member_t(__attribute__((__unused__)) __Request__mach_port_extract_member_t *In0P)
{

	typedef __Request__mach_port_extract_member_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_extract_member_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_port_extract_member */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_port_extract_member
#if	defined(LINTLIBRARY)
    (task, name, pset)
	ipc_space_t task;
	mach_port_name_t name;
	mach_port_name_t pset;
{ return mach_port_extract_member(task, name, pset); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_name_t pset
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_extract_member */
mig_internal novalue _Xmach_port_extract_member
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_port_name_t pset;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_extract_member_t __Request;
	typedef __Reply__mach_port_extract_member_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_extract_member_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_extract_member_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3227, "mach_port_extract_member")
	__BeforeRcvRpc(3227, "mach_port_extract_member")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_extract_member_t__defined)
	check_result = __MIG_check__Request__mach_port_extract_member_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_extract_member_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_extract_member(task, In0P->name, In0P->pset);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3227, "mach_port_extract_member")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_get_context_t__defined)
#define __MIG_check__Request__mach_port_get_context_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_get_context_t(__attribute__((__unused__)) __Request__mach_port_get_context_t *In0P)
{

	typedef __Request__mach_port_get_context_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_get_context_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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
	mach_vm_address_t *context;
{ return mach_port_get_context(task, name, context); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_vm_address_t *context
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_get_context */
mig_internal novalue _Xmach_port_get_context
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_get_context_t __Request;
	typedef __Reply__mach_port_get_context_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_get_context_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_get_context_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3228, "mach_port_get_context")
	__BeforeRcvRpc(3228, "mach_port_get_context")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_get_context_t__defined)
	check_result = __MIG_check__Request__mach_port_get_context_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_get_context_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_get_context(task, In0P->name, &OutP->context);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3228, "mach_port_get_context")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_set_context_t__defined)
#define __MIG_check__Request__mach_port_set_context_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_set_context_t(__attribute__((__unused__)) __Request__mach_port_set_context_t *In0P)
{

	typedef __Request__mach_port_set_context_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_set_context_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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
	mach_vm_address_t context;
{ return mach_port_set_context(task, name, context); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_vm_address_t context
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_set_context */
mig_internal novalue _Xmach_port_set_context
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_vm_address_t context;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_set_context_t __Request;
	typedef __Reply__mach_port_set_context_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_set_context_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_set_context_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3229, "mach_port_set_context")
	__BeforeRcvRpc(3229, "mach_port_set_context")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_set_context_t__defined)
	check_result = __MIG_check__Request__mach_port_set_context_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_set_context_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_set_context(task, In0P->name, In0P->context);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3229, "mach_port_set_context")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_kobject_t__defined)
#define __MIG_check__Request__mach_port_kobject_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_kobject_t(__attribute__((__unused__)) __Request__mach_port_kobject_t *In0P)
{

	typedef __Request__mach_port_kobject_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_kobject_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_kobject */
mig_internal novalue _Xmach_port_kobject
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_kobject_t __Request;
	typedef __Reply__mach_port_kobject_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_kobject_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_kobject_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3230, "mach_port_kobject")
	__BeforeRcvRpc(3230, "mach_port_kobject")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_kobject_t__defined)
	check_result = __MIG_check__Request__mach_port_kobject_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_kobject_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_kobject(task, In0P->name, &OutP->object_type, &OutP->object_addr);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3230, "mach_port_kobject")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_construct_t__defined)
#define __MIG_check__Request__mach_port_construct_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_construct_t(__attribute__((__unused__)) __Request__mach_port_construct_t *In0P)
{

	typedef __Request__mach_port_construct_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->options.type != MACH_MSG_OOL_DESCRIPTOR ||
	    In0P->options.size != 24)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

#if __MigTypeCheck
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_construct_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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
	uint64_t context;
	mach_port_name_t *name;
{ return mach_port_construct(task, options, context, name); }
#else
(
	ipc_space_t task,
	mach_port_options_ptr_t options,
	uint64_t context,
	mach_port_name_t *name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_construct */
mig_internal novalue _Xmach_port_construct
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		uint64_t context;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_construct_t __Request;
	typedef __Reply__mach_port_construct_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_construct_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_construct_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3231, "mach_port_construct")
	__BeforeRcvRpc(3231, "mach_port_construct")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_construct_t__defined)
	check_result = __MIG_check__Request__mach_port_construct_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_construct_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_construct(task, (mach_port_options_ptr_t)(In0P->options.address), In0P->context, &OutP->name);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3231, "mach_port_construct")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_destruct_t__defined)
#define __MIG_check__Request__mach_port_destruct_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_destruct_t(__attribute__((__unused__)) __Request__mach_port_destruct_t *In0P)
{

	typedef __Request__mach_port_destruct_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_destruct_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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
	uint64_t guard;
{ return mach_port_destruct(task, name, srdelta, guard); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_delta_t srdelta,
	uint64_t guard
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_destruct */
mig_internal novalue _Xmach_port_destruct
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		uint64_t guard;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_destruct_t __Request;
	typedef __Reply__mach_port_destruct_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_destruct_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_destruct_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3232, "mach_port_destruct")
	__BeforeRcvRpc(3232, "mach_port_destruct")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_destruct_t__defined)
	check_result = __MIG_check__Request__mach_port_destruct_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_destruct_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_destruct(task, In0P->name, In0P->srdelta, In0P->guard);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3232, "mach_port_destruct")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_guard_t__defined)
#define __MIG_check__Request__mach_port_guard_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_guard_t(__attribute__((__unused__)) __Request__mach_port_guard_t *In0P)
{

	typedef __Request__mach_port_guard_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_guard_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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
	uint64_t guard;
	boolean_t strict;
{ return mach_port_guard(task, name, guard, strict); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	uint64_t guard,
	boolean_t strict
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_guard */
mig_internal novalue _Xmach_port_guard
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		uint64_t guard;
		boolean_t strict;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_guard_t __Request;
	typedef __Reply__mach_port_guard_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_guard_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_guard_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3233, "mach_port_guard")
	__BeforeRcvRpc(3233, "mach_port_guard")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_guard_t__defined)
	check_result = __MIG_check__Request__mach_port_guard_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_guard_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_guard(task, In0P->name, In0P->guard, In0P->strict);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3233, "mach_port_guard")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_unguard_t__defined)
#define __MIG_check__Request__mach_port_unguard_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_unguard_t(__attribute__((__unused__)) __Request__mach_port_unguard_t *In0P)
{

	typedef __Request__mach_port_unguard_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_unguard_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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
	uint64_t guard;
{ return mach_port_unguard(task, name, guard); }
#else
(
	ipc_space_t task,
	mach_port_name_t name,
	uint64_t guard
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_port_unguard */
mig_internal novalue _Xmach_port_unguard
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

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
		uint64_t guard;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_unguard_t __Request;
	typedef __Reply__mach_port_unguard_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_unguard_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_unguard_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3234, "mach_port_unguard")
	__BeforeRcvRpc(3234, "mach_port_unguard")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_unguard_t__defined)
	check_result = __MIG_check__Request__mach_port_unguard_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_unguard_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_unguard(task, In0P->name, In0P->guard);
	space_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3234, "mach_port_unguard")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_port_subsystem__
#if !defined(__MIG_check__Request__mach_port_space_basic_info_t__defined)
#define __MIG_check__Request__mach_port_space_basic_info_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_port_space_basic_info_t(__attribute__((__unused__)) __Request__mach_port_space_basic_info_t *In0P)
{

	typedef __Request__mach_port_space_basic_info_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_port_space_basic_info_t__defined) */
#endif /* __MIG_check__Request__mach_port_subsystem__ */
#endif /* ( __MigTypeCheck ) */


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

/* Routine mach_port_space_basic_info */
mig_internal novalue _Xmach_port_space_basic_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_port_space_basic_info_t __Request;
	typedef __Reply__mach_port_space_basic_info_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_port_space_basic_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_port_space_basic_info_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	ipc_space_t task;

	__DeclareRcvRpc(3235, "mach_port_space_basic_info")
	__BeforeRcvRpc(3235, "mach_port_space_basic_info")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_port_space_basic_info_t__defined)
	check_result = __MIG_check__Request__mach_port_space_basic_info_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_port_space_basic_info_t__defined) */

	task = convert_port_to_space(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_port_space_basic_info(task, &OutP->basic_info);
	space_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3235, "mach_port_space_basic_info")
}


#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
boolean_t mach_port_server(
		mach_msg_header_t *InHeadP,
		mach_msg_header_t *OutHeadP);

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
mig_routine_t mach_port_server_routine(
		mach_msg_header_t *InHeadP);


/* Description of this subsystem, for use in direct RPC */
extern const struct mach_port_subsystem mach_port_subsystem;
const struct mach_port_subsystem {
	mig_server_routine_t 	server;	/* Server routine */
	mach_msg_id_t	start;	/* Min routine number */
	mach_msg_id_t	end;	/* Max routine number + 1 */
	unsigned int	maxsize;	/* Max msg size */
	vm_address_t	reserved;	/* Reserved */
	struct routine_descriptor	/*Array of routine descriptors */
		routine[36];
} mach_port_subsystem = {
	mach_port_server_routine,
	3200,
	3236,
	(mach_msg_size_t)sizeof(union __ReplyUnion__mach_port_subsystem),
	(vm_address_t)0,
	{
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_names, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_names_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_type, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_type_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_rename, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_rename_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_allocate_name, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_allocate_name_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_allocate, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_allocate_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_deallocate, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_deallocate_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_insert_right, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_insert_right_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_extract_right, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_extract_right_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_mod_refs, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_mod_refs_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_move_member, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_move_member_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_destroy, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_destroy_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_get_refs, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_get_refs_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_peek, 8, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_peek_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_set_mscount, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_set_mscount_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_get_set_status, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_get_set_status_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_request_notification, 6, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_request_notification_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_set_seqno, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_set_seqno_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_get_attributes, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_get_attributes_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_set_attributes, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_set_attributes_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_allocate_qos, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_allocate_qos_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_allocate_full, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_allocate_full_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_set_port_space, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_set_port_space_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_get_srights, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_get_srights_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_space_info, 6, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_space_info_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_dnrequest_info, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_dnrequest_info_t) },
		{0, 0, 0, 0, 0, 0},
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_insert_member, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_insert_member_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_extract_member, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_extract_member_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_get_context, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_get_context_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_set_context, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_set_context_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_kobject, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_kobject_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_construct, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_construct_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_destruct, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_destruct_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_guard, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_guard_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_unguard, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_unguard_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_port_space_basic_info, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_port_space_basic_info_t) },
	}
};

mig_external boolean_t mach_port_server
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	register mig_routine_t routine;

	OutHeadP->msgh_bits = MACH_MSGH_BITS(MACH_MSGH_BITS_REPLY(InHeadP->msgh_bits), 0);
	OutHeadP->msgh_remote_port = InHeadP->msgh_reply_port;
	/* Minimal size: routine() will update it if different */
	OutHeadP->msgh_size = (mach_msg_size_t)sizeof(mig_reply_error_t);
	OutHeadP->msgh_local_port = MACH_PORT_NULL;
	OutHeadP->msgh_id = InHeadP->msgh_id + 100;

	if ((InHeadP->msgh_id > 3235) || (InHeadP->msgh_id < 3200) ||
	    ((routine = mach_port_subsystem.routine[InHeadP->msgh_id - 3200].stub_routine) == 0)) {
		((mig_reply_error_t *)OutHeadP)->NDR = NDR_record;
		((mig_reply_error_t *)OutHeadP)->RetCode = MIG_BAD_ID;
		return FALSE;
	}
	(*routine) (InHeadP, OutHeadP);
	return TRUE;
}

mig_external mig_routine_t mach_port_server_routine
	(mach_msg_header_t *InHeadP)
{
	register int msgh_id;

	msgh_id = InHeadP->msgh_id - 3200;

	if ((msgh_id > 35) || (msgh_id < 0))
		return 0;

	return mach_port_subsystem.routine[msgh_id].stub_routine;
}

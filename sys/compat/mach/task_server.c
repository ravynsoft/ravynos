/*
 * IDENTIFICATION:
 * stub generated Thu Jun 11 18:17:45 2015
 * with a MiG generated Thu Jun 11 16:16:11 PDT 2015 by kmacy@serenity
 * OPTIONS: 
 *	KernelServer
 */

/* Module task */

#define	__MIG_check__Request__task_subsystem__ 1

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
#include <sys/mach/task.h>

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

#ifndef __Request__task_subsystem__defined
#define __Request__task_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_ports_descriptor_t ledgers;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t ledgersCnt;
		boolean_t inherit_memory;
	} __Request__task_create_t;
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
	} __Request__task_terminate_t;
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
	} __Request__task_threads_t;
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
		mach_msg_ool_ports_descriptor_t init_port_set;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t init_port_setCnt;
	} __Request__mach_ports_register_t;
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
	} __Request__mach_ports_lookup_t;
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
		task_flavor_t flavor;
		mach_msg_type_number_t task_info_outCnt;
	} __Request__task_info_t;
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
		task_flavor_t flavor;
		mach_msg_type_number_t task_info_inCnt;
		integer_t task_info_in[52];
	} __Request__task_set_info_t;
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
	} __Request__task_suspend_t;
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
	} __Request__task_resume_t;
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
		int which_port;
	} __Request__task_get_special_port_t;
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
		mach_msg_port_descriptor_t special_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		int which_port;
	} __Request__task_set_special_port_t;
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
	} __Request__thread_create_from_user_t;
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
		thread_state_flavor_t flavor;
		mach_msg_type_number_t new_stateCnt;
		natural_t new_state[32];
	} __Request__thread_create_running_from_user_t;
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
		mach_msg_port_descriptor_t new_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		exception_mask_t exception_mask;
		exception_behavior_t behavior;
		thread_state_flavor_t new_flavor;
	} __Request__task_set_exception_ports_t;
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
		exception_mask_t exception_mask;
	} __Request__task_get_exception_ports_t;
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
		mach_msg_port_descriptor_t new_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		exception_mask_t exception_mask;
		exception_behavior_t behavior;
		thread_state_flavor_t new_flavor;
	} __Request__task_swap_exception_ports_t;
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
		int policy;
		int value;
	} __Request__semaphore_create_t;
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
		mach_msg_port_descriptor_t semaphore;
		/* end of the kernel processed data */
	} __Request__semaphore_destroy_t;
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
		task_policy_flavor_t flavor;
		mach_msg_type_number_t policy_infoCnt;
		integer_t policy_info[16];
	} __Request__task_policy_set_t;
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
		task_policy_flavor_t flavor;
		mach_msg_type_number_t policy_infoCnt;
		boolean_t get_default;
	} __Request__task_policy_get_t;
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
		policy_t policy;
		mach_msg_type_number_t baseCnt;
		integer_t base[5];
		boolean_t set_limit;
		boolean_t change;
	} __Request__task_policy_t;
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
		vm_address_t basepc;
		vm_address_t boundspc;
	} __Request__task_set_ras_pc_t;
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
	} __Request__task_zone_info_t;
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
		mach_msg_port_descriptor_t new_set;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t assign_threads;
	} __Request__task_assign_t;
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
		boolean_t assign_threads;
	} __Request__task_assign_default_t;
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
	} __Request__task_get_assignment_t;
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
		mach_msg_port_descriptor_t pset;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		policy_t policy;
		mach_msg_type_number_t baseCnt;
		integer_t base[5];
		mach_msg_type_number_t limitCnt;
		integer_t limit[1];
		boolean_t change;
	} __Request__task_set_policy_t;
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
		thread_state_flavor_t flavor;
		mach_msg_type_number_t old_stateCnt;
	} __Request__task_get_state_t;
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
		thread_state_flavor_t flavor;
		mach_msg_type_number_t new_stateCnt;
		natural_t new_state[32];
	} __Request__task_set_state_t;
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
		int new_limit;
	} __Request__task_set_phys_footprint_limit_t;
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
	} __Request__task_suspend2_t;
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
	} __Request__task_resume2_t;
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
	} __Request__task_purgable_info_t;
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
		mach_voucher_selector_t which;
	} __Request__task_get_mach_voucher_t;
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
		mach_msg_port_descriptor_t voucher;
		/* end of the kernel processed data */
	} __Request__task_set_mach_voucher_t;
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
		mach_msg_port_descriptor_t new_voucher;
		mach_msg_port_descriptor_t old_voucher;
		/* end of the kernel processed data */
	} __Request__task_swap_mach_voucher_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__task_subsystem__defined */

/* typedefs for all replies */

#ifndef __Reply__task_subsystem__defined
#define __Reply__task_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t child_task;
		/* end of the kernel processed data */
	} __Reply__task_create_t;
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
	} __Reply__task_terminate_t;
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
		mach_msg_ool_ports_descriptor_t act_list;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t act_listCnt;
	} __Reply__task_threads_t;
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
	} __Reply__mach_ports_register_t;
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
		mach_msg_ool_ports_descriptor_t init_port_set;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t init_port_setCnt;
	} __Reply__mach_ports_lookup_t;
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
		mach_msg_type_number_t task_info_outCnt;
		integer_t task_info_out[52];
	} __Reply__task_info_t;
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
	} __Reply__task_set_info_t;
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
	} __Reply__task_suspend_t;
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
	} __Reply__task_resume_t;
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
		mach_msg_port_descriptor_t special_port;
		/* end of the kernel processed data */
	} __Reply__task_get_special_port_t;
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
	} __Reply__task_set_special_port_t;
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
		mach_msg_port_descriptor_t child_act;
		/* end of the kernel processed data */
	} __Reply__thread_create_from_user_t;
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
		mach_msg_port_descriptor_t child_act;
		/* end of the kernel processed data */
	} __Reply__thread_create_running_from_user_t;
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
	} __Reply__task_set_exception_ports_t;
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
		mach_msg_port_descriptor_t old_handlers[32];
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t masksCnt;
		exception_mask_t masks[32];
		exception_behavior_t old_behaviors[32];
		thread_state_flavor_t old_flavors[32];
	} __Reply__task_get_exception_ports_t;
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
		mach_msg_port_descriptor_t old_handlerss[32];
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t masksCnt;
		exception_mask_t masks[32];
		exception_behavior_t old_behaviors[32];
		thread_state_flavor_t old_flavors[32];
	} __Reply__task_swap_exception_ports_t;
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
		mach_msg_port_descriptor_t semaphore;
		/* end of the kernel processed data */
	} __Reply__semaphore_create_t;
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
	} __Reply__semaphore_destroy_t;
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
	} __Reply__task_policy_set_t;
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
		mach_msg_type_number_t policy_infoCnt;
		integer_t policy_info[16];
		boolean_t get_default;
	} __Reply__task_policy_get_t;
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
	} __Reply__task_policy_t;
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
	} __Reply__task_set_ras_pc_t;
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
		mach_msg_ool_descriptor_t names;
		mach_msg_ool_descriptor_t info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t namesCnt;
		mach_msg_type_number_t infoCnt;
	} __Reply__task_zone_info_t;
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
	} __Reply__task_assign_t;
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
	} __Reply__task_assign_default_t;
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
		mach_msg_port_descriptor_t assigned_set;
		/* end of the kernel processed data */
	} __Reply__task_get_assignment_t;
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
	} __Reply__task_set_policy_t;
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
		mach_msg_type_number_t old_stateCnt;
		natural_t old_state[32];
	} __Reply__task_get_state_t;
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
	} __Reply__task_set_state_t;
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
		int old_limit;
	} __Reply__task_set_phys_footprint_limit_t;
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
		mach_msg_port_descriptor_t suspend_token;
		/* end of the kernel processed data */
	} __Reply__task_suspend2_t;
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
	} __Reply__task_resume2_t;
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
		task_purgable_info_t stats;
	} __Reply__task_purgable_info_t;
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
		mach_msg_port_descriptor_t voucher;
		/* end of the kernel processed data */
	} __Reply__task_get_mach_voucher_t;
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
	} __Reply__task_set_mach_voucher_t;
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
		mach_msg_port_descriptor_t old_voucher;
		/* end of the kernel processed data */
	} __Reply__task_swap_mach_voucher_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__task_subsystem__defined */


/* union of all replies */

#ifndef __ReplyUnion__task_subsystem__defined
#define __ReplyUnion__task_subsystem__defined
union __ReplyUnion__task_subsystem {
	__Reply__task_create_t Reply_task_create;
	__Reply__task_terminate_t Reply_task_terminate;
	__Reply__task_threads_t Reply_task_threads;
	__Reply__mach_ports_register_t Reply_mach_ports_register;
	__Reply__mach_ports_lookup_t Reply_mach_ports_lookup;
	__Reply__task_info_t Reply_task_info;
	__Reply__task_set_info_t Reply_task_set_info;
	__Reply__task_suspend_t Reply_task_suspend;
	__Reply__task_resume_t Reply_task_resume;
	__Reply__task_get_special_port_t Reply_task_get_special_port;
	__Reply__task_set_special_port_t Reply_task_set_special_port;
	__Reply__thread_create_from_user_t Reply_thread_create_from_user;
	__Reply__thread_create_running_from_user_t Reply_thread_create_running_from_user;
	__Reply__task_set_exception_ports_t Reply_task_set_exception_ports;
	__Reply__task_get_exception_ports_t Reply_task_get_exception_ports;
	__Reply__task_swap_exception_ports_t Reply_task_swap_exception_ports;
	__Reply__semaphore_create_t Reply_semaphore_create;
	__Reply__semaphore_destroy_t Reply_semaphore_destroy;
	__Reply__task_policy_set_t Reply_task_policy_set;
	__Reply__task_policy_get_t Reply_task_policy_get;
	__Reply__task_policy_t Reply_task_policy;
	__Reply__task_set_ras_pc_t Reply_task_set_ras_pc;
	__Reply__task_zone_info_t Reply_task_zone_info;
	__Reply__task_assign_t Reply_task_assign;
	__Reply__task_assign_default_t Reply_task_assign_default;
	__Reply__task_get_assignment_t Reply_task_get_assignment;
	__Reply__task_set_policy_t Reply_task_set_policy;
	__Reply__task_get_state_t Reply_task_get_state;
	__Reply__task_set_state_t Reply_task_set_state;
	__Reply__task_set_phys_footprint_limit_t Reply_task_set_phys_footprint_limit;
	__Reply__task_suspend2_t Reply_task_suspend2;
	__Reply__task_resume2_t Reply_task_resume2;
	__Reply__task_purgable_info_t Reply_task_purgable_info;
	__Reply__task_get_mach_voucher_t Reply_task_get_mach_voucher;
	__Reply__task_set_mach_voucher_t Reply_task_set_mach_voucher;
	__Reply__task_swap_mach_voucher_t Reply_task_swap_mach_voucher;
};
#endif /* __RequestUnion__task_subsystem__defined */
/* Forward Declarations */


mig_internal novalue _Xtask_create
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_terminate
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_threads
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_ports_register
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_ports_lookup
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_set_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_suspend
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_resume
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_get_special_port
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_set_special_port
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xthread_create_from_user
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xthread_create_running_from_user
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_set_exception_ports
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_get_exception_ports
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_swap_exception_ports
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xsemaphore_create
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xsemaphore_destroy
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_policy_set
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_policy_get
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_policy
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_set_ras_pc
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_zone_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_assign
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_assign_default
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_get_assignment
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_set_policy
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_get_state
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_set_state
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_set_phys_footprint_limit
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_suspend2
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_resume2
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_purgable_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_get_mach_voucher
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_set_mach_voucher
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xtask_swap_mach_voucher
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);


#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_create_t__defined)
#define __MIG_check__Request__task_create_t__defined

mig_internal kern_return_t __MIG_check__Request__task_create_t(__attribute__((__unused__)) __Request__task_create_t *In0P)
{

	typedef __Request__task_create_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->ledgers.type != MACH_MSG_OOL_PORTS_DESCRIPTOR ||
	    In0P->ledgers.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_create_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_create */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_create
#if	defined(LINTLIBRARY)
    (target_task, ledgers, ledgersCnt, inherit_memory, child_task)
	task_t target_task;
	ledger_array_t ledgers;
	mach_msg_type_number_t ledgersCnt;
	boolean_t inherit_memory;
	task_t *child_task;
{ return task_create(target_task, ledgers, ledgersCnt, inherit_memory, child_task); }
#else
(
	task_t target_task,
	ledger_array_t ledgers,
	mach_msg_type_number_t ledgersCnt,
	boolean_t inherit_memory,
	task_t *child_task
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_create */
mig_internal novalue _Xtask_create
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_ports_descriptor_t ledgers;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t ledgersCnt;
		boolean_t inherit_memory;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_create_t __Request;
	typedef __Reply__task_create_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_create_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_create_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t child_taskTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t child_taskTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t target_task;
	task_t child_task;

	__DeclareRcvRpc(3400, "task_create")
	__BeforeRcvRpc(3400, "task_create")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_create_t__defined)
	check_result = __MIG_check__Request__task_create_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_create_t__defined) */

#if	UseStaticTemplates
	OutP->child_task = child_taskTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->child_task.disposition = 17;
#else
	OutP->child_task.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->child_task.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	target_task = convert_port_to_task(In0P->Head.msgh_request_port);

	RetCode = task_create(target_task, (ledger_array_t)(In0P->ledgers.address), In0P->ledgersCnt, In0P->inherit_memory, &child_task);
	task_deallocate(target_task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->child_task.name = (mach_port_t)convert_task_to_port(child_task);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3400, "task_create")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_terminate_t__defined)
#define __MIG_check__Request__task_terminate_t__defined

mig_internal kern_return_t __MIG_check__Request__task_terminate_t(__attribute__((__unused__)) __Request__task_terminate_t *In0P)
{

	typedef __Request__task_terminate_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_terminate_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_terminate */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_terminate
#if	defined(LINTLIBRARY)
    (target_task)
	task_t target_task;
{ return task_terminate(target_task); }
#else
(
	task_t target_task
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_terminate */
mig_internal novalue _Xtask_terminate
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
	typedef __Request__task_terminate_t __Request;
	typedef __Reply__task_terminate_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_terminate_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_terminate_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t target_task;

	__DeclareRcvRpc(3401, "task_terminate")
	__BeforeRcvRpc(3401, "task_terminate")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_terminate_t__defined)
	check_result = __MIG_check__Request__task_terminate_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_terminate_t__defined) */

	target_task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_terminate(target_task);
	task_deallocate(target_task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3401, "task_terminate")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_threads_t__defined)
#define __MIG_check__Request__task_threads_t__defined

mig_internal kern_return_t __MIG_check__Request__task_threads_t(__attribute__((__unused__)) __Request__task_threads_t *In0P)
{

	typedef __Request__task_threads_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_threads_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_threads */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_threads
#if	defined(LINTLIBRARY)
    (target_task, act_list, act_listCnt)
	task_t target_task;
	thread_act_array_t *act_list;
	mach_msg_type_number_t *act_listCnt;
{ return task_threads(target_task, act_list, act_listCnt); }
#else
(
	task_t target_task,
	thread_act_array_t *act_list,
	mach_msg_type_number_t *act_listCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_threads */
mig_internal novalue _Xtask_threads
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
	typedef __Request__task_threads_t __Request;
	typedef __Reply__task_threads_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_threads_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_threads_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_ool_ports_descriptor_t act_listTemplate = {
		.address = (void *)0,
		.count = 0,
		.deallocate = FALSE,
		/* copy is meaningful only in overwrite mode */
		.copy = MACH_MSG_PHYSICAL_COPY,
		.disposition = 17,
		.type = MACH_MSG_OOL_PORTS_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_ool_ports_descriptor_t act_listTemplate = {
		.address = (void *)0,
		.count = 0,
		.deallocate = FALSE,
		/* copy is meaningful only in overwrite mode */
		.copy = MACH_MSG_PHYSICAL_COPY,
		.disposition = 19,
		.type = MACH_MSG_OOL_PORTS_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t target_task;

	__DeclareRcvRpc(3402, "task_threads")
	__BeforeRcvRpc(3402, "task_threads")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_threads_t__defined)
	check_result = __MIG_check__Request__task_threads_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_threads_t__defined) */

#if	UseStaticTemplates
	OutP->act_list = act_listTemplate;
#else	/* UseStaticTemplates */
#if	__MigKernelSpecificCode
	OutP->act_list.disposition = 17;
#else
	OutP->act_list.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->act_list.deallocate =  FALSE;
	OutP->act_list.type = MACH_MSG_OOL_PORTS_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	target_task = convert_port_to_task(In0P->Head.msgh_request_port);

	RetCode = task_threads(target_task, (thread_act_array_t *)&(OutP->act_list.address), &OutP->act_listCnt);
	task_deallocate(target_task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->act_list.count = OutP->act_listCnt;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3402, "task_threads")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__mach_ports_register_t__defined)
#define __MIG_check__Request__mach_ports_register_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_ports_register_t(__attribute__((__unused__)) __Request__mach_ports_register_t *In0P)
{

	typedef __Request__mach_ports_register_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->init_port_set.type != MACH_MSG_OOL_PORTS_DESCRIPTOR ||
	    In0P->init_port_set.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_ports_register_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_ports_register */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_ports_register
#if	defined(LINTLIBRARY)
    (target_task, init_port_set, init_port_setCnt)
	task_t target_task;
	mach_port_array_t init_port_set;
	mach_msg_type_number_t init_port_setCnt;
{ return mach_ports_register(target_task, init_port_set, init_port_setCnt); }
#else
(
	task_t target_task,
	mach_port_array_t init_port_set,
	mach_msg_type_number_t init_port_setCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_ports_register */
mig_internal novalue _Xmach_ports_register
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_ports_descriptor_t init_port_set;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t init_port_setCnt;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_ports_register_t __Request;
	typedef __Reply__mach_ports_register_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_ports_register_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_ports_register_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t target_task;

	__DeclareRcvRpc(3403, "mach_ports_register")
	__BeforeRcvRpc(3403, "mach_ports_register")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_ports_register_t__defined)
	check_result = __MIG_check__Request__mach_ports_register_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_ports_register_t__defined) */

	target_task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = mach_ports_register(target_task, (mach_port_array_t)(In0P->init_port_set.address), In0P->init_port_setCnt);
	task_deallocate(target_task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3403, "mach_ports_register")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__mach_ports_lookup_t__defined)
#define __MIG_check__Request__mach_ports_lookup_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_ports_lookup_t(__attribute__((__unused__)) __Request__mach_ports_lookup_t *In0P)
{

	typedef __Request__mach_ports_lookup_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_ports_lookup_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_ports_lookup */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_ports_lookup
#if	defined(LINTLIBRARY)
    (target_task, init_port_set, init_port_setCnt)
	task_t target_task;
	mach_port_array_t *init_port_set;
	mach_msg_type_number_t *init_port_setCnt;
{ return mach_ports_lookup(target_task, init_port_set, init_port_setCnt); }
#else
(
	task_t target_task,
	mach_port_array_t *init_port_set,
	mach_msg_type_number_t *init_port_setCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_ports_lookup */
mig_internal novalue _Xmach_ports_lookup
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
	typedef __Request__mach_ports_lookup_t __Request;
	typedef __Reply__mach_ports_lookup_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_ports_lookup_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_ports_lookup_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_ool_ports_descriptor_t init_port_setTemplate = {
		.address = (void *)0,
		.count = 0,
		.deallocate = FALSE,
		/* copy is meaningful only in overwrite mode */
		.copy = MACH_MSG_PHYSICAL_COPY,
		.disposition = 17,
		.type = MACH_MSG_OOL_PORTS_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_ool_ports_descriptor_t init_port_setTemplate = {
		.address = (void *)0,
		.count = 0,
		.deallocate = FALSE,
		/* copy is meaningful only in overwrite mode */
		.copy = MACH_MSG_PHYSICAL_COPY,
		.disposition = 19,
		.type = MACH_MSG_OOL_PORTS_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t target_task;

	__DeclareRcvRpc(3404, "mach_ports_lookup")
	__BeforeRcvRpc(3404, "mach_ports_lookup")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_ports_lookup_t__defined)
	check_result = __MIG_check__Request__mach_ports_lookup_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_ports_lookup_t__defined) */

#if	UseStaticTemplates
	OutP->init_port_set = init_port_setTemplate;
#else	/* UseStaticTemplates */
#if	__MigKernelSpecificCode
	OutP->init_port_set.disposition = 17;
#else
	OutP->init_port_set.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->init_port_set.deallocate =  FALSE;
	OutP->init_port_set.type = MACH_MSG_OOL_PORTS_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	target_task = convert_port_to_task(In0P->Head.msgh_request_port);

	RetCode = mach_ports_lookup(target_task, (mach_port_array_t *)&(OutP->init_port_set.address), &OutP->init_port_setCnt);
	task_deallocate(target_task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->init_port_set.count = OutP->init_port_setCnt;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3404, "mach_ports_lookup")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_info_t__defined)
#define __MIG_check__Request__task_info_t__defined

mig_internal kern_return_t __MIG_check__Request__task_info_t(__attribute__((__unused__)) __Request__task_info_t *In0P)
{

	typedef __Request__task_info_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_info_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_info
#if	defined(LINTLIBRARY)
    (target_task, flavor, task_info_out, task_info_outCnt)
	task_name_t target_task;
	task_flavor_t flavor;
	task_info_t task_info_out;
	mach_msg_type_number_t *task_info_outCnt;
{ return task_info(target_task, flavor, task_info_out, task_info_outCnt); }
#else
(
	task_name_t target_task,
	task_flavor_t flavor,
	task_info_t task_info_out,
	mach_msg_type_number_t *task_info_outCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_info */
mig_internal novalue _Xtask_info
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
		task_flavor_t flavor;
		mach_msg_type_number_t task_info_outCnt;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_info_t __Request;
	typedef __Reply__task_info_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_info_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_name_t target_task;

	__DeclareRcvRpc(3405, "task_info")
	__BeforeRcvRpc(3405, "task_info")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_info_t__defined)
	check_result = __MIG_check__Request__task_info_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_info_t__defined) */

	target_task = convert_port_to_task_name(In0P->Head.msgh_request_port);

	OutP->task_info_outCnt = 52;
	if (In0P->task_info_outCnt < OutP->task_info_outCnt)
		OutP->task_info_outCnt = In0P->task_info_outCnt;

	OutP->RetCode = task_info(target_task, In0P->flavor, OutP->task_info_out, &OutP->task_info_outCnt);
	task_name_deallocate(target_task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;

	OutP->Head.msgh_size = (sizeof(Reply) - 208) + (_WALIGN_((4 * OutP->task_info_outCnt)));

	__AfterRcvRpc(3405, "task_info")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_set_info_t__defined)
#define __MIG_check__Request__task_set_info_t__defined

mig_internal kern_return_t __MIG_check__Request__task_set_info_t(__attribute__((__unused__)) __Request__task_set_info_t *In0P)
{

	typedef __Request__task_set_info_t __Request;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	msgh_size = In0P->Head.msgh_size;
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (msgh_size < (sizeof(__Request) - 208)) ||  (msgh_size > (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if defined(__NDR_convert__int_rep__Request__task_set_info_t__task_info_inCnt__defined)
	if (In0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Request__task_set_info_t__task_info_inCnt(&In0P->task_info_inCnt, In0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Request__task_set_info_t__task_info_inCnt__defined */
#if	__MigTypeCheck
	if ( In0P->task_info_inCnt > 52 )
		return MIG_BAD_ARGUMENTS;
	if (((msgh_size - (sizeof(__Request) - 208)) / 4 < In0P->task_info_inCnt) ||
	    (msgh_size != (sizeof(__Request) - 208) + _WALIGN_(4 * In0P->task_info_inCnt)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_set_info_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_set_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_info
#if	defined(LINTLIBRARY)
    (target_task, flavor, task_info_in, task_info_inCnt)
	task_t target_task;
	task_flavor_t flavor;
	task_info_t task_info_in;
	mach_msg_type_number_t task_info_inCnt;
{ return task_set_info(target_task, flavor, task_info_in, task_info_inCnt); }
#else
(
	task_t target_task,
	task_flavor_t flavor,
	task_info_t task_info_in,
	mach_msg_type_number_t task_info_inCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_set_info */
mig_internal novalue _Xtask_set_info
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
		task_flavor_t flavor;
		mach_msg_type_number_t task_info_inCnt;
		integer_t task_info_in[52];
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_set_info_t __Request;
	typedef __Reply__task_set_info_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_set_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_set_info_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t target_task;

	__DeclareRcvRpc(3406, "task_set_info")
	__BeforeRcvRpc(3406, "task_set_info")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_set_info_t__defined)
	check_result = __MIG_check__Request__task_set_info_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_set_info_t__defined) */

	target_task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_set_info(target_task, In0P->flavor, In0P->task_info_in, In0P->task_info_inCnt);
	task_deallocate(target_task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3406, "task_set_info")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_suspend_t__defined)
#define __MIG_check__Request__task_suspend_t__defined

mig_internal kern_return_t __MIG_check__Request__task_suspend_t(__attribute__((__unused__)) __Request__task_suspend_t *In0P)
{

	typedef __Request__task_suspend_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_suspend_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_suspend */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_suspend
#if	defined(LINTLIBRARY)
    (target_task)
	task_t target_task;
{ return task_suspend(target_task); }
#else
(
	task_t target_task
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_suspend */
mig_internal novalue _Xtask_suspend
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
	typedef __Request__task_suspend_t __Request;
	typedef __Reply__task_suspend_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_suspend_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_suspend_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t target_task;

	__DeclareRcvRpc(3407, "task_suspend")
	__BeforeRcvRpc(3407, "task_suspend")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_suspend_t__defined)
	check_result = __MIG_check__Request__task_suspend_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_suspend_t__defined) */

	target_task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_suspend(target_task);
	task_deallocate(target_task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3407, "task_suspend")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_resume_t__defined)
#define __MIG_check__Request__task_resume_t__defined

mig_internal kern_return_t __MIG_check__Request__task_resume_t(__attribute__((__unused__)) __Request__task_resume_t *In0P)
{

	typedef __Request__task_resume_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_resume_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_resume */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_resume
#if	defined(LINTLIBRARY)
    (target_task)
	task_t target_task;
{ return task_resume(target_task); }
#else
(
	task_t target_task
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_resume */
mig_internal novalue _Xtask_resume
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
	typedef __Request__task_resume_t __Request;
	typedef __Reply__task_resume_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_resume_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_resume_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t target_task;

	__DeclareRcvRpc(3408, "task_resume")
	__BeforeRcvRpc(3408, "task_resume")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_resume_t__defined)
	check_result = __MIG_check__Request__task_resume_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_resume_t__defined) */

	target_task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_resume(target_task);
	task_deallocate(target_task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3408, "task_resume")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_get_special_port_t__defined)
#define __MIG_check__Request__task_get_special_port_t__defined

mig_internal kern_return_t __MIG_check__Request__task_get_special_port_t(__attribute__((__unused__)) __Request__task_get_special_port_t *In0P)
{

	typedef __Request__task_get_special_port_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_get_special_port_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_get_special_port */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_get_special_port
#if	defined(LINTLIBRARY)
    (task, which_port, special_port)
	task_t task;
	int which_port;
	mach_port_t *special_port;
{ return task_get_special_port(task, which_port, special_port); }
#else
(
	task_t task,
	int which_port,
	mach_port_t *special_port
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_get_special_port */
mig_internal novalue _Xtask_get_special_port
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
		int which_port;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_get_special_port_t __Request;
	typedef __Reply__task_get_special_port_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_get_special_port_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_get_special_port_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t special_portTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t special_portTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t task;

	__DeclareRcvRpc(3409, "task_get_special_port")
	__BeforeRcvRpc(3409, "task_get_special_port")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_get_special_port_t__defined)
	check_result = __MIG_check__Request__task_get_special_port_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_get_special_port_t__defined) */

#if	UseStaticTemplates
	OutP->special_port = special_portTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->special_port.disposition = 17;
#else
	OutP->special_port.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->special_port.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	task = convert_port_to_task(In0P->Head.msgh_request_port);

	RetCode = task_get_special_port(task, In0P->which_port, &OutP->special_port.name);
	task_deallocate(task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3409, "task_get_special_port")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_set_special_port_t__defined)
#define __MIG_check__Request__task_set_special_port_t__defined

mig_internal kern_return_t __MIG_check__Request__task_set_special_port_t(__attribute__((__unused__)) __Request__task_set_special_port_t *In0P)
{

	typedef __Request__task_set_special_port_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->special_port.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->special_port.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_set_special_port_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_set_special_port */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_special_port
#if	defined(LINTLIBRARY)
    (task, which_port, special_port)
	task_t task;
	int which_port;
	mach_port_t special_port;
{ return task_set_special_port(task, which_port, special_port); }
#else
(
	task_t task,
	int which_port,
	mach_port_t special_port
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_set_special_port */
mig_internal novalue _Xtask_set_special_port
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t special_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		int which_port;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_set_special_port_t __Request;
	typedef __Reply__task_set_special_port_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_set_special_port_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_set_special_port_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;

	__DeclareRcvRpc(3410, "task_set_special_port")
	__BeforeRcvRpc(3410, "task_set_special_port")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_set_special_port_t__defined)
	check_result = __MIG_check__Request__task_set_special_port_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_set_special_port_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_set_special_port(task, In0P->which_port, In0P->special_port.name);
	task_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3410, "task_set_special_port")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__thread_create_from_user_t__defined)
#define __MIG_check__Request__thread_create_from_user_t__defined

mig_internal kern_return_t __MIG_check__Request__thread_create_from_user_t(__attribute__((__unused__)) __Request__thread_create_from_user_t *In0P)
{

	typedef __Request__thread_create_from_user_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__thread_create_from_user_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine thread_create_from_user */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t thread_create_from_user
#if	defined(LINTLIBRARY)
    (parent_task, child_act)
	task_t parent_task;
	thread_act_t *child_act;
{ return thread_create_from_user(parent_task, child_act); }
#else
(
	task_t parent_task,
	thread_act_t *child_act
);
#endif	/* defined(LINTLIBRARY) */

/* Routine thread_create_from_user */
mig_internal novalue _Xthread_create_from_user
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
	typedef __Request__thread_create_from_user_t __Request;
	typedef __Reply__thread_create_from_user_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__thread_create_from_user_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__thread_create_from_user_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t child_actTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t child_actTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t parent_task;
	thread_act_t child_act;

	__DeclareRcvRpc(3411, "thread_create_from_user")
	__BeforeRcvRpc(3411, "thread_create_from_user")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__thread_create_from_user_t__defined)
	check_result = __MIG_check__Request__thread_create_from_user_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__thread_create_from_user_t__defined) */

#if	UseStaticTemplates
	OutP->child_act = child_actTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->child_act.disposition = 17;
#else
	OutP->child_act.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->child_act.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	parent_task = convert_port_to_task(In0P->Head.msgh_request_port);

	RetCode = thread_create_from_user(parent_task, &child_act);
	task_deallocate(parent_task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->child_act.name = (mach_port_t)convert_thread_to_port(child_act);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3411, "thread_create_from_user")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__thread_create_running_from_user_t__defined)
#define __MIG_check__Request__thread_create_running_from_user_t__defined

mig_internal kern_return_t __MIG_check__Request__thread_create_running_from_user_t(__attribute__((__unused__)) __Request__thread_create_running_from_user_t *In0P)
{

	typedef __Request__thread_create_running_from_user_t __Request;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	msgh_size = In0P->Head.msgh_size;
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (msgh_size < (sizeof(__Request) - 128)) ||  (msgh_size > (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if defined(__NDR_convert__int_rep__Request__thread_create_running_from_user_t__new_stateCnt__defined)
	if (In0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Request__thread_create_running_from_user_t__new_stateCnt(&In0P->new_stateCnt, In0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Request__thread_create_running_from_user_t__new_stateCnt__defined */
#if	__MigTypeCheck
	if ( In0P->new_stateCnt > 32 )
		return MIG_BAD_ARGUMENTS;
	if (((msgh_size - (sizeof(__Request) - 128)) / 4 < In0P->new_stateCnt) ||
	    (msgh_size != (sizeof(__Request) - 128) + _WALIGN_(4 * In0P->new_stateCnt)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__thread_create_running_from_user_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine thread_create_running_from_user */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t thread_create_running_from_user
#if	defined(LINTLIBRARY)
    (parent_task, flavor, new_state, new_stateCnt, child_act)
	task_t parent_task;
	thread_state_flavor_t flavor;
	thread_state_t new_state;
	mach_msg_type_number_t new_stateCnt;
	thread_act_t *child_act;
{ return thread_create_running_from_user(parent_task, flavor, new_state, new_stateCnt, child_act); }
#else
(
	task_t parent_task,
	thread_state_flavor_t flavor,
	thread_state_t new_state,
	mach_msg_type_number_t new_stateCnt,
	thread_act_t *child_act
);
#endif	/* defined(LINTLIBRARY) */

/* Routine thread_create_running_from_user */
mig_internal novalue _Xthread_create_running_from_user
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
		thread_state_flavor_t flavor;
		mach_msg_type_number_t new_stateCnt;
		natural_t new_state[32];
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__thread_create_running_from_user_t __Request;
	typedef __Reply__thread_create_running_from_user_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__thread_create_running_from_user_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__thread_create_running_from_user_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t child_actTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t child_actTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t parent_task;
	thread_act_t child_act;

	__DeclareRcvRpc(3412, "thread_create_running_from_user")
	__BeforeRcvRpc(3412, "thread_create_running_from_user")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__thread_create_running_from_user_t__defined)
	check_result = __MIG_check__Request__thread_create_running_from_user_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__thread_create_running_from_user_t__defined) */

#if	UseStaticTemplates
	OutP->child_act = child_actTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->child_act.disposition = 17;
#else
	OutP->child_act.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->child_act.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	parent_task = convert_port_to_task(In0P->Head.msgh_request_port);

	RetCode = thread_create_running_from_user(parent_task, In0P->flavor, In0P->new_state, In0P->new_stateCnt, &child_act);
	task_deallocate(parent_task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->child_act.name = (mach_port_t)convert_thread_to_port(child_act);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3412, "thread_create_running_from_user")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_set_exception_ports_t__defined)
#define __MIG_check__Request__task_set_exception_ports_t__defined

mig_internal kern_return_t __MIG_check__Request__task_set_exception_ports_t(__attribute__((__unused__)) __Request__task_set_exception_ports_t *In0P)
{

	typedef __Request__task_set_exception_ports_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->new_port.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->new_port.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_set_exception_ports_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_set_exception_ports */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_exception_ports
#if	defined(LINTLIBRARY)
    (task, exception_mask, new_port, behavior, new_flavor)
	task_t task;
	exception_mask_t exception_mask;
	mach_port_t new_port;
	exception_behavior_t behavior;
	thread_state_flavor_t new_flavor;
{ return task_set_exception_ports(task, exception_mask, new_port, behavior, new_flavor); }
#else
(
	task_t task,
	exception_mask_t exception_mask,
	mach_port_t new_port,
	exception_behavior_t behavior,
	thread_state_flavor_t new_flavor
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_set_exception_ports */
mig_internal novalue _Xtask_set_exception_ports
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		exception_mask_t exception_mask;
		exception_behavior_t behavior;
		thread_state_flavor_t new_flavor;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_set_exception_ports_t __Request;
	typedef __Reply__task_set_exception_ports_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_set_exception_ports_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_set_exception_ports_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;

	__DeclareRcvRpc(3413, "task_set_exception_ports")
	__BeforeRcvRpc(3413, "task_set_exception_ports")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_set_exception_ports_t__defined)
	check_result = __MIG_check__Request__task_set_exception_ports_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_set_exception_ports_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_set_exception_ports(task, In0P->exception_mask, In0P->new_port.name, In0P->behavior, In0P->new_flavor);
	task_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3413, "task_set_exception_ports")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_get_exception_ports_t__defined)
#define __MIG_check__Request__task_get_exception_ports_t__defined

mig_internal kern_return_t __MIG_check__Request__task_get_exception_ports_t(__attribute__((__unused__)) __Request__task_get_exception_ports_t *In0P)
{

	typedef __Request__task_get_exception_ports_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_get_exception_ports_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_get_exception_ports */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_get_exception_ports
#if	defined(LINTLIBRARY)
    (task, exception_mask, masks, masksCnt, old_handlers, old_behaviors, old_flavors)
	task_t task;
	exception_mask_t exception_mask;
	exception_mask_array_t masks;
	mach_msg_type_number_t *masksCnt;
	exception_handler_array_t old_handlers;
	exception_behavior_array_t old_behaviors;
	exception_flavor_array_t old_flavors;
{ return task_get_exception_ports(task, exception_mask, masks, masksCnt, old_handlers, old_behaviors, old_flavors); }
#else
(
	task_t task,
	exception_mask_t exception_mask,
	exception_mask_array_t masks,
	mach_msg_type_number_t *masksCnt,
	exception_handler_array_t old_handlers,
	exception_behavior_array_t old_behaviors,
	exception_flavor_array_t old_flavors
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_get_exception_ports */
mig_internal novalue _Xtask_get_exception_ports
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
		exception_mask_t exception_mask;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_get_exception_ports_t __Request;
	typedef __Reply__task_get_exception_ports_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
	unsigned int msgh_size;
	unsigned int msgh_size_delta;

#ifdef	__MIG_check__Request__task_get_exception_ports_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_get_exception_ports_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t old_handlersTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t old_handlersTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t task;
	mach_msg_type_number_t masksCnt;
	exception_handler_t old_handlers[32];
	exception_behavior_t old_behaviors[32];
	thread_state_flavor_t old_flavors[32];

	__DeclareRcvRpc(3414, "task_get_exception_ports")
	__BeforeRcvRpc(3414, "task_get_exception_ports")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_get_exception_ports_t__defined)
	check_result = __MIG_check__Request__task_get_exception_ports_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_get_exception_ports_t__defined) */

	{
	    register	mach_msg_port_descriptor_t	*ptr;
	    register int	i;

	    ptr = &OutP->old_handlers[0];
	    for (i = 0; i < 32; ptr++, i++) {
#if	UseStaticTemplates
		*ptr = old_handlersTemplate;
#else	/* UseStaticTemplates */
		ptr->name = MACH_PORT_NULL;
#if __MigKernelSpecificCode
		ptr->disposition = 17;
#else
		ptr->disposition = 19;
#endif /* __MigKernelSpecificCode */
		ptr->type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */
	    }
	}


	task = convert_port_to_task(In0P->Head.msgh_request_port);

	masksCnt = 32;

	RetCode = task_get_exception_ports(task, In0P->exception_mask, OutP->masks, &masksCnt, old_handlers, old_behaviors, old_flavors);
	task_deallocate(task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	{
	    register	mach_msg_port_descriptor_t	*ptr;
	    register int	i, j;

	    ptr = &OutP->old_handlers[0];
	    j = min(32, masksCnt);
	    for (i = 0; i < j; ptr++, i++) {
		ptr->name = old_handlers[i];
	    }
	}


	OutP->NDR = NDR_record;

	OutP->masksCnt = masksCnt;
	msgh_size_delta = _WALIGN_((4 * masksCnt));
	msgh_size = (sizeof(Reply) - 384) + msgh_size_delta;
	OutP = (Reply *) ((pointer_t) OutP + msgh_size_delta - 128);
	(void)memcpy((char *) OutP->old_behaviors, (const char *) old_behaviors, 4 * masksCnt);
	msgh_size_delta = _WALIGN_((4 * masksCnt));
	msgh_size += msgh_size_delta;
	OutP = (Reply *) ((pointer_t) OutP + msgh_size_delta - 128);
	(void)memcpy((char *) OutP->old_flavors, (const char *) old_flavors, 4 * masksCnt);
	msgh_size += _WALIGN_((4 * masksCnt));

	OutP = (Reply *) OutHeadP;
	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = msgh_size;
	OutP->msgh_body.msgh_descriptor_count = 32;
	__AfterRcvRpc(3414, "task_get_exception_ports")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_swap_exception_ports_t__defined)
#define __MIG_check__Request__task_swap_exception_ports_t__defined

mig_internal kern_return_t __MIG_check__Request__task_swap_exception_ports_t(__attribute__((__unused__)) __Request__task_swap_exception_ports_t *In0P)
{

	typedef __Request__task_swap_exception_ports_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->new_port.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->new_port.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_swap_exception_ports_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_swap_exception_ports */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_swap_exception_ports
#if	defined(LINTLIBRARY)
    (task, exception_mask, new_port, behavior, new_flavor, masks, masksCnt, old_handlerss, old_behaviors, old_flavors)
	task_t task;
	exception_mask_t exception_mask;
	mach_port_t new_port;
	exception_behavior_t behavior;
	thread_state_flavor_t new_flavor;
	exception_mask_array_t masks;
	mach_msg_type_number_t *masksCnt;
	exception_handler_array_t old_handlerss;
	exception_behavior_array_t old_behaviors;
	exception_flavor_array_t old_flavors;
{ return task_swap_exception_ports(task, exception_mask, new_port, behavior, new_flavor, masks, masksCnt, old_handlerss, old_behaviors, old_flavors); }
#else
(
	task_t task,
	exception_mask_t exception_mask,
	mach_port_t new_port,
	exception_behavior_t behavior,
	thread_state_flavor_t new_flavor,
	exception_mask_array_t masks,
	mach_msg_type_number_t *masksCnt,
	exception_handler_array_t old_handlerss,
	exception_behavior_array_t old_behaviors,
	exception_flavor_array_t old_flavors
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_swap_exception_ports */
mig_internal novalue _Xtask_swap_exception_ports
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		exception_mask_t exception_mask;
		exception_behavior_t behavior;
		thread_state_flavor_t new_flavor;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_swap_exception_ports_t __Request;
	typedef __Reply__task_swap_exception_ports_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
	unsigned int msgh_size;
	unsigned int msgh_size_delta;

#ifdef	__MIG_check__Request__task_swap_exception_ports_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_swap_exception_ports_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t old_handlerssTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t old_handlerssTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t task;
	mach_msg_type_number_t masksCnt;
	exception_handler_t old_handlerss[32];
	exception_behavior_t old_behaviors[32];
	thread_state_flavor_t old_flavors[32];

	__DeclareRcvRpc(3415, "task_swap_exception_ports")
	__BeforeRcvRpc(3415, "task_swap_exception_ports")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_swap_exception_ports_t__defined)
	check_result = __MIG_check__Request__task_swap_exception_ports_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_swap_exception_ports_t__defined) */

	{
	    register	mach_msg_port_descriptor_t	*ptr;
	    register int	i;

	    ptr = &OutP->old_handlerss[0];
	    for (i = 0; i < 32; ptr++, i++) {
#if	UseStaticTemplates
		*ptr = old_handlerssTemplate;
#else	/* UseStaticTemplates */
		ptr->name = MACH_PORT_NULL;
#if __MigKernelSpecificCode
		ptr->disposition = 17;
#else
		ptr->disposition = 19;
#endif /* __MigKernelSpecificCode */
		ptr->type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */
	    }
	}


	task = convert_port_to_task(In0P->Head.msgh_request_port);

	masksCnt = 32;

	RetCode = task_swap_exception_ports(task, In0P->exception_mask, In0P->new_port.name, In0P->behavior, In0P->new_flavor, OutP->masks, &masksCnt, old_handlerss, old_behaviors, old_flavors);
	task_deallocate(task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	{
	    register	mach_msg_port_descriptor_t	*ptr;
	    register int	i, j;

	    ptr = &OutP->old_handlerss[0];
	    j = min(32, masksCnt);
	    for (i = 0; i < j; ptr++, i++) {
		ptr->name = old_handlerss[i];
	    }
	}


	OutP->NDR = NDR_record;

	OutP->masksCnt = masksCnt;
	msgh_size_delta = _WALIGN_((4 * masksCnt));
	msgh_size = (sizeof(Reply) - 384) + msgh_size_delta;
	OutP = (Reply *) ((pointer_t) OutP + msgh_size_delta - 128);
	(void)memcpy((char *) OutP->old_behaviors, (const char *) old_behaviors, 4 * masksCnt);
	msgh_size_delta = _WALIGN_((4 * masksCnt));
	msgh_size += msgh_size_delta;
	OutP = (Reply *) ((pointer_t) OutP + msgh_size_delta - 128);
	(void)memcpy((char *) OutP->old_flavors, (const char *) old_flavors, 4 * masksCnt);
	msgh_size += _WALIGN_((4 * masksCnt));

	OutP = (Reply *) OutHeadP;
	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = msgh_size;
	OutP->msgh_body.msgh_descriptor_count = 32;
	__AfterRcvRpc(3415, "task_swap_exception_ports")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__semaphore_create_t__defined)
#define __MIG_check__Request__semaphore_create_t__defined

mig_internal kern_return_t __MIG_check__Request__semaphore_create_t(__attribute__((__unused__)) __Request__semaphore_create_t *In0P)
{

	typedef __Request__semaphore_create_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__semaphore_create_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine semaphore_create */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t semaphore_create
#if	defined(LINTLIBRARY)
    (task, semaphore, policy, value)
	task_t task;
	semaphore_t *semaphore;
	int policy;
	int value;
{ return semaphore_create(task, semaphore, policy, value); }
#else
(
	task_t task,
	semaphore_t *semaphore,
	int policy,
	int value
);
#endif	/* defined(LINTLIBRARY) */

/* Routine semaphore_create */
mig_internal novalue _Xsemaphore_create
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
		int policy;
		int value;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__semaphore_create_t __Request;
	typedef __Reply__semaphore_create_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__semaphore_create_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__semaphore_create_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t semaphoreTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t semaphoreTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t task;
	semaphore_t semaphore;

	__DeclareRcvRpc(3418, "semaphore_create")
	__BeforeRcvRpc(3418, "semaphore_create")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__semaphore_create_t__defined)
	check_result = __MIG_check__Request__semaphore_create_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__semaphore_create_t__defined) */

#if	UseStaticTemplates
	OutP->semaphore = semaphoreTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->semaphore.disposition = 17;
#else
	OutP->semaphore.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->semaphore.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	task = convert_port_to_task(In0P->Head.msgh_request_port);

	RetCode = semaphore_create(task, &semaphore, In0P->policy, In0P->value);
	task_deallocate(task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->semaphore.name = (mach_port_t)convert_semaphore_to_port(semaphore);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3418, "semaphore_create")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__semaphore_destroy_t__defined)
#define __MIG_check__Request__semaphore_destroy_t__defined

mig_internal kern_return_t __MIG_check__Request__semaphore_destroy_t(__attribute__((__unused__)) __Request__semaphore_destroy_t *In0P)
{

	typedef __Request__semaphore_destroy_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->semaphore.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->semaphore.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__semaphore_destroy_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine semaphore_destroy */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t semaphore_destroy
#if	defined(LINTLIBRARY)
    (task, semaphore)
	task_t task;
	semaphore_t semaphore;
{ return semaphore_destroy(task, semaphore); }
#else
(
	task_t task,
	semaphore_t semaphore
);
#endif	/* defined(LINTLIBRARY) */

/* Routine semaphore_destroy */
mig_internal novalue _Xsemaphore_destroy
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t semaphore;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__semaphore_destroy_t __Request;
	typedef __Reply__semaphore_destroy_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__semaphore_destroy_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__semaphore_destroy_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;
	semaphore_t semaphore;

	__DeclareRcvRpc(3419, "semaphore_destroy")
	__BeforeRcvRpc(3419, "semaphore_destroy")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__semaphore_destroy_t__defined)
	check_result = __MIG_check__Request__semaphore_destroy_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__semaphore_destroy_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	semaphore = convert_port_to_semaphore(In0P->semaphore.name);

	OutP->RetCode = semaphore_destroy(task, semaphore);
	semaphore_dereference(semaphore);
	task_deallocate(task);
#if	__MigKernelSpecificCode
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}

	if (IP_VALID((ipc_port_t)In0P->semaphore.name))
		ipc_port_release_send((ipc_port_t)In0P->semaphore.name);
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3419, "semaphore_destroy")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_policy_set_t__defined)
#define __MIG_check__Request__task_policy_set_t__defined

mig_internal kern_return_t __MIG_check__Request__task_policy_set_t(__attribute__((__unused__)) __Request__task_policy_set_t *In0P)
{

	typedef __Request__task_policy_set_t __Request;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	msgh_size = In0P->Head.msgh_size;
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (msgh_size < (sizeof(__Request) - 64)) ||  (msgh_size > (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if defined(__NDR_convert__int_rep__Request__task_policy_set_t__policy_infoCnt__defined)
	if (In0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Request__task_policy_set_t__policy_infoCnt(&In0P->policy_infoCnt, In0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Request__task_policy_set_t__policy_infoCnt__defined */
#if	__MigTypeCheck
	if ( In0P->policy_infoCnt > 16 )
		return MIG_BAD_ARGUMENTS;
	if (((msgh_size - (sizeof(__Request) - 64)) / 4 < In0P->policy_infoCnt) ||
	    (msgh_size != (sizeof(__Request) - 64) + _WALIGN_(4 * In0P->policy_infoCnt)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_policy_set_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_policy_set */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_policy_set
#if	defined(LINTLIBRARY)
    (task, flavor, policy_info, policy_infoCnt)
	task_t task;
	task_policy_flavor_t flavor;
	task_policy_t policy_info;
	mach_msg_type_number_t policy_infoCnt;
{ return task_policy_set(task, flavor, policy_info, policy_infoCnt); }
#else
(
	task_t task,
	task_policy_flavor_t flavor,
	task_policy_t policy_info,
	mach_msg_type_number_t policy_infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_policy_set */
mig_internal novalue _Xtask_policy_set
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
		task_policy_flavor_t flavor;
		mach_msg_type_number_t policy_infoCnt;
		integer_t policy_info[16];
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_policy_set_t __Request;
	typedef __Reply__task_policy_set_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_policy_set_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_policy_set_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;

	__DeclareRcvRpc(3420, "task_policy_set")
	__BeforeRcvRpc(3420, "task_policy_set")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_policy_set_t__defined)
	check_result = __MIG_check__Request__task_policy_set_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_policy_set_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_policy_set(task, In0P->flavor, In0P->policy_info, In0P->policy_infoCnt);
	task_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3420, "task_policy_set")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_policy_get_t__defined)
#define __MIG_check__Request__task_policy_get_t__defined

mig_internal kern_return_t __MIG_check__Request__task_policy_get_t(__attribute__((__unused__)) __Request__task_policy_get_t *In0P)
{

	typedef __Request__task_policy_get_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_policy_get_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_policy_get */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_policy_get
#if	defined(LINTLIBRARY)
    (task, flavor, policy_info, policy_infoCnt, get_default)
	task_t task;
	task_policy_flavor_t flavor;
	task_policy_t policy_info;
	mach_msg_type_number_t *policy_infoCnt;
	boolean_t *get_default;
{ return task_policy_get(task, flavor, policy_info, policy_infoCnt, get_default); }
#else
(
	task_t task,
	task_policy_flavor_t flavor,
	task_policy_t policy_info,
	mach_msg_type_number_t *policy_infoCnt,
	boolean_t *get_default
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_policy_get */
mig_internal novalue _Xtask_policy_get
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
		task_policy_flavor_t flavor;
		mach_msg_type_number_t policy_infoCnt;
		boolean_t get_default;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_policy_get_t __Request;
	typedef __Reply__task_policy_get_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
	unsigned int msgh_size_delta;

#ifdef	__MIG_check__Request__task_policy_get_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_policy_get_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;

	__DeclareRcvRpc(3421, "task_policy_get")
	__BeforeRcvRpc(3421, "task_policy_get")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_policy_get_t__defined)
	check_result = __MIG_check__Request__task_policy_get_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_policy_get_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->policy_infoCnt = 16;
	if (In0P->policy_infoCnt < OutP->policy_infoCnt)
		OutP->policy_infoCnt = In0P->policy_infoCnt;

	OutP->RetCode = task_policy_get(task, In0P->flavor, OutP->policy_info, &OutP->policy_infoCnt, &In0P->get_default);
	task_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;

	msgh_size_delta = _WALIGN_((4 * OutP->policy_infoCnt));
	OutP->Head.msgh_size = (sizeof(Reply) - 64) + msgh_size_delta;
	OutP = (Reply *) ((pointer_t) OutP + msgh_size_delta - 64);

	OutP->get_default = In0P->get_default;

	OutP = (Reply *) OutHeadP;
	__AfterRcvRpc(3421, "task_policy_get")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_policy_t__defined)
#define __MIG_check__Request__task_policy_t__defined

mig_internal kern_return_t __MIG_check__Request__task_policy_t(__attribute__((__unused__)) __Request__task_policy_t *In0P, __attribute__((__unused__)) __Request__task_policy_t **In1PP)
{

	typedef __Request__task_policy_t __Request;
	__Request *In1P;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	unsigned int msgh_size_delta;

#if	__MigTypeCheck
	msgh_size = In0P->Head.msgh_size;
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (msgh_size < (sizeof(__Request) - 20)) ||  (msgh_size > (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if defined(__NDR_convert__int_rep__Request__task_policy_t__baseCnt__defined)
	if (In0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Request__task_policy_t__baseCnt(&In0P->baseCnt, In0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Request__task_policy_t__baseCnt__defined */
	msgh_size_delta = _WALIGN_(4 * In0P->baseCnt);
#if	__MigTypeCheck
	if ( In0P->baseCnt > 5 )
		return MIG_BAD_ARGUMENTS;
	if (((msgh_size - (sizeof(__Request) - 20)) / 4 < In0P->baseCnt) ||
	    (msgh_size != (sizeof(__Request) - 20) + _WALIGN_(4 * In0P->baseCnt)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	*In1PP = In1P = (__Request *) ((pointer_t) In0P + msgh_size_delta - 20);

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_policy_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_policy */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_policy
#if	defined(LINTLIBRARY)
    (task, policy, base, baseCnt, set_limit, change)
	task_t task;
	policy_t policy;
	policy_base_t base;
	mach_msg_type_number_t baseCnt;
	boolean_t set_limit;
	boolean_t change;
{ return task_policy(task, policy, base, baseCnt, set_limit, change); }
#else
(
	task_t task,
	policy_t policy,
	policy_base_t base,
	mach_msg_type_number_t baseCnt,
	boolean_t set_limit,
	boolean_t change
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_policy */
mig_internal novalue _Xtask_policy
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
		policy_t policy;
		mach_msg_type_number_t baseCnt;
		integer_t base[5];
		boolean_t set_limit;
		boolean_t change;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_policy_t __Request;
	typedef __Reply__task_policy_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Request *In1P = NULL;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_policy_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_policy_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;

	__DeclareRcvRpc(3423, "task_policy")
	__BeforeRcvRpc(3423, "task_policy")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_policy_t__defined)
	check_result = __MIG_check__Request__task_policy_t((__Request *)In0P, (__Request **)&In1P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_policy_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_policy(task, In0P->policy, In0P->base, In0P->baseCnt, In1P->set_limit, In1P->change);
	task_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3423, "task_policy")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_set_ras_pc_t__defined)
#define __MIG_check__Request__task_set_ras_pc_t__defined

mig_internal kern_return_t __MIG_check__Request__task_set_ras_pc_t(__attribute__((__unused__)) __Request__task_set_ras_pc_t *In0P)
{

	typedef __Request__task_set_ras_pc_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_set_ras_pc_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_set_ras_pc */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_ras_pc
#if	defined(LINTLIBRARY)
    (target_task, basepc, boundspc)
	task_t target_task;
	vm_address_t basepc;
	vm_address_t boundspc;
{ return task_set_ras_pc(target_task, basepc, boundspc); }
#else
(
	task_t target_task,
	vm_address_t basepc,
	vm_address_t boundspc
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_set_ras_pc */
mig_internal novalue _Xtask_set_ras_pc
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
		vm_address_t basepc;
		vm_address_t boundspc;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_set_ras_pc_t __Request;
	typedef __Reply__task_set_ras_pc_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_set_ras_pc_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_set_ras_pc_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t target_task;

	__DeclareRcvRpc(3427, "task_set_ras_pc")
	__BeforeRcvRpc(3427, "task_set_ras_pc")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_set_ras_pc_t__defined)
	check_result = __MIG_check__Request__task_set_ras_pc_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_set_ras_pc_t__defined) */

	target_task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_set_ras_pc(target_task, In0P->basepc, In0P->boundspc);
	task_deallocate(target_task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3427, "task_set_ras_pc")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_zone_info_t__defined)
#define __MIG_check__Request__task_zone_info_t__defined

mig_internal kern_return_t __MIG_check__Request__task_zone_info_t(__attribute__((__unused__)) __Request__task_zone_info_t *In0P)
{

	typedef __Request__task_zone_info_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_zone_info_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_zone_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_zone_info
#if	defined(LINTLIBRARY)
    (target_task, names, namesCnt, info, infoCnt)
	task_t target_task;
	mach_zone_name_array_t *names;
	mach_msg_type_number_t *namesCnt;
	task_zone_info_array_t *info;
	mach_msg_type_number_t *infoCnt;
{ return task_zone_info(target_task, names, namesCnt, info, infoCnt); }
#else
(
	task_t target_task,
	mach_zone_name_array_t *names,
	mach_msg_type_number_t *namesCnt,
	task_zone_info_array_t *info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_zone_info */
mig_internal novalue _Xtask_zone_info
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
	typedef __Request__task_zone_info_t __Request;
	typedef __Reply__task_zone_info_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_zone_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_zone_info_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t namesTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t infoTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t namesTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t infoTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t target_task;

	__DeclareRcvRpc(3428, "task_zone_info")
	__BeforeRcvRpc(3428, "task_zone_info")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_zone_info_t__defined)
	check_result = __MIG_check__Request__task_zone_info_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_zone_info_t__defined) */

#if	UseStaticTemplates
	OutP->names = namesTemplate;
#else	/* UseStaticTemplates */
	OutP->names.deallocate =  TRUE;
	OutP->names.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->names.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


#if	UseStaticTemplates
	OutP->info = infoTemplate;
#else	/* UseStaticTemplates */
	OutP->info.deallocate =  TRUE;
	OutP->info.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->info.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	target_task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->namesCnt = 0;

	OutP->infoCnt = 0;

	RetCode = task_zone_info(target_task, (mach_zone_name_array_t *)&(OutP->names.address), &OutP->namesCnt, (task_zone_info_array_t *)&(OutP->info.address), &OutP->infoCnt);
	task_deallocate(target_task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->names.size = OutP->namesCnt * 80;

	OutP->info.size = OutP->infoCnt * 88;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 2;
	__AfterRcvRpc(3428, "task_zone_info")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_assign_t__defined)
#define __MIG_check__Request__task_assign_t__defined

mig_internal kern_return_t __MIG_check__Request__task_assign_t(__attribute__((__unused__)) __Request__task_assign_t *In0P)
{

	typedef __Request__task_assign_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->new_set.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->new_set.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_assign_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_assign */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_assign
#if	defined(LINTLIBRARY)
    (task, new_set, assign_threads)
	task_t task;
	processor_set_t new_set;
	boolean_t assign_threads;
{ return task_assign(task, new_set, assign_threads); }
#else
(
	task_t task,
	processor_set_t new_set,
	boolean_t assign_threads
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_assign */
mig_internal novalue _Xtask_assign
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_set;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t assign_threads;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_assign_t __Request;
	typedef __Reply__task_assign_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_assign_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_assign_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;
	processor_set_t new_set;

	__DeclareRcvRpc(3429, "task_assign")
	__BeforeRcvRpc(3429, "task_assign")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_assign_t__defined)
	check_result = __MIG_check__Request__task_assign_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_assign_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	new_set = convert_port_to_pset(In0P->new_set.name);

	OutP->RetCode = task_assign(task, new_set, In0P->assign_threads);
	pset_deallocate(new_set);
	task_deallocate(task);
#if	__MigKernelSpecificCode
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}

	if (IP_VALID((ipc_port_t)In0P->new_set.name))
		ipc_port_release_send((ipc_port_t)In0P->new_set.name);
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3429, "task_assign")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_assign_default_t__defined)
#define __MIG_check__Request__task_assign_default_t__defined

mig_internal kern_return_t __MIG_check__Request__task_assign_default_t(__attribute__((__unused__)) __Request__task_assign_default_t *In0P)
{

	typedef __Request__task_assign_default_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_assign_default_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_assign_default */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_assign_default
#if	defined(LINTLIBRARY)
    (task, assign_threads)
	task_t task;
	boolean_t assign_threads;
{ return task_assign_default(task, assign_threads); }
#else
(
	task_t task,
	boolean_t assign_threads
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_assign_default */
mig_internal novalue _Xtask_assign_default
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
		boolean_t assign_threads;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_assign_default_t __Request;
	typedef __Reply__task_assign_default_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_assign_default_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_assign_default_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;

	__DeclareRcvRpc(3430, "task_assign_default")
	__BeforeRcvRpc(3430, "task_assign_default")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_assign_default_t__defined)
	check_result = __MIG_check__Request__task_assign_default_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_assign_default_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_assign_default(task, In0P->assign_threads);
	task_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3430, "task_assign_default")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_get_assignment_t__defined)
#define __MIG_check__Request__task_get_assignment_t__defined

mig_internal kern_return_t __MIG_check__Request__task_get_assignment_t(__attribute__((__unused__)) __Request__task_get_assignment_t *In0P)
{

	typedef __Request__task_get_assignment_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_get_assignment_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_get_assignment */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_get_assignment
#if	defined(LINTLIBRARY)
    (task, assigned_set)
	task_t task;
	processor_set_name_t *assigned_set;
{ return task_get_assignment(task, assigned_set); }
#else
(
	task_t task,
	processor_set_name_t *assigned_set
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_get_assignment */
mig_internal novalue _Xtask_get_assignment
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
	typedef __Request__task_get_assignment_t __Request;
	typedef __Reply__task_get_assignment_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_get_assignment_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_get_assignment_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t assigned_setTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t assigned_setTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t task;
	processor_set_name_t assigned_set;

	__DeclareRcvRpc(3431, "task_get_assignment")
	__BeforeRcvRpc(3431, "task_get_assignment")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_get_assignment_t__defined)
	check_result = __MIG_check__Request__task_get_assignment_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_get_assignment_t__defined) */

#if	UseStaticTemplates
	OutP->assigned_set = assigned_setTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->assigned_set.disposition = 17;
#else
	OutP->assigned_set.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->assigned_set.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	task = convert_port_to_task(In0P->Head.msgh_request_port);

	RetCode = task_get_assignment(task, &assigned_set);
	task_deallocate(task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->assigned_set.name = (mach_port_t)convert_pset_name_to_port(assigned_set);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3431, "task_get_assignment")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_set_policy_t__defined)
#define __MIG_check__Request__task_set_policy_t__defined

mig_internal kern_return_t __MIG_check__Request__task_set_policy_t(__attribute__((__unused__)) __Request__task_set_policy_t *In0P, __attribute__((__unused__)) __Request__task_set_policy_t **In1PP, __attribute__((__unused__)) __Request__task_set_policy_t **In2PP)
{

	typedef __Request__task_set_policy_t __Request;
	__Request *In1P;
	__Request *In2P;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	unsigned int msgh_size_delta;

#if	__MigTypeCheck
	msgh_size = In0P->Head.msgh_size;
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (msgh_size < (sizeof(__Request) - 24)) ||  (msgh_size > (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->pset.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->pset.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

#if defined(__NDR_convert__int_rep__Request__task_set_policy_t__baseCnt__defined)
	if (In0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Request__task_set_policy_t__baseCnt(&In0P->baseCnt, In0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Request__task_set_policy_t__baseCnt__defined */
	msgh_size_delta = _WALIGN_(4 * In0P->baseCnt);
#if	__MigTypeCheck
	if ( In0P->baseCnt > 5 )
		return MIG_BAD_ARGUMENTS;
	if (((msgh_size - (sizeof(__Request) - 24)) / 4 < In0P->baseCnt) ||
	    (msgh_size < (sizeof(__Request) - 24) + _WALIGN_(4 * In0P->baseCnt)))
		return MIG_BAD_ARGUMENTS;
	msgh_size -= msgh_size_delta;
#endif	/* __MigTypeCheck */

	*In1PP = In1P = (__Request *) ((pointer_t) In0P + msgh_size_delta - 20);

#if defined(__NDR_convert__int_rep__Request__task_set_policy_t__limitCnt__defined)
	if (In0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Request__task_set_policy_t__limitCnt(&In1P->limitCnt, In1P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Request__task_set_policy_t__limitCnt__defined */
	msgh_size_delta = _WALIGN_(4 * In1P->limitCnt);
#if	__MigTypeCheck
	if ( In1P->limitCnt > 1 )
		return MIG_BAD_ARGUMENTS;
	if (((msgh_size - (sizeof(__Request) - 24)) / 4 < In1P->limitCnt) ||
	    (msgh_size != (sizeof(__Request) - 24) + _WALIGN_(4 * In1P->limitCnt)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	*In2PP = In2P = (__Request *) ((pointer_t) In1P + msgh_size_delta - 4);

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_set_policy_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_set_policy */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_policy
#if	defined(LINTLIBRARY)
    (task, pset, policy, base, baseCnt, limit, limitCnt, change)
	task_t task;
	processor_set_t pset;
	policy_t policy;
	policy_base_t base;
	mach_msg_type_number_t baseCnt;
	policy_limit_t limit;
	mach_msg_type_number_t limitCnt;
	boolean_t change;
{ return task_set_policy(task, pset, policy, base, baseCnt, limit, limitCnt, change); }
#else
(
	task_t task,
	processor_set_t pset,
	policy_t policy,
	policy_base_t base,
	mach_msg_type_number_t baseCnt,
	policy_limit_t limit,
	mach_msg_type_number_t limitCnt,
	boolean_t change
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_set_policy */
mig_internal novalue _Xtask_set_policy
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t pset;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		policy_t policy;
		mach_msg_type_number_t baseCnt;
		integer_t base[5];
		mach_msg_type_number_t limitCnt;
		integer_t limit[1];
		boolean_t change;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_set_policy_t __Request;
	typedef __Reply__task_set_policy_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Request *In1P = NULL;
	Request *In2P = NULL;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_set_policy_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_set_policy_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;
	processor_set_t pset;

	__DeclareRcvRpc(3432, "task_set_policy")
	__BeforeRcvRpc(3432, "task_set_policy")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_set_policy_t__defined)
	check_result = __MIG_check__Request__task_set_policy_t((__Request *)In0P, (__Request **)&In1P, (__Request **)&In2P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_set_policy_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	pset = convert_port_to_pset(In0P->pset.name);

	OutP->RetCode = task_set_policy(task, pset, In0P->policy, In0P->base, In0P->baseCnt, In1P->limit, In1P->limitCnt, In2P->change);
	pset_deallocate(pset);
	task_deallocate(task);
#if	__MigKernelSpecificCode
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}

	if (IP_VALID((ipc_port_t)In0P->pset.name))
		ipc_port_release_send((ipc_port_t)In0P->pset.name);
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3432, "task_set_policy")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_get_state_t__defined)
#define __MIG_check__Request__task_get_state_t__defined

mig_internal kern_return_t __MIG_check__Request__task_get_state_t(__attribute__((__unused__)) __Request__task_get_state_t *In0P)
{

	typedef __Request__task_get_state_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_get_state_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_get_state */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_get_state
#if	defined(LINTLIBRARY)
    (task, flavor, old_state, old_stateCnt)
	task_t task;
	thread_state_flavor_t flavor;
	thread_state_t old_state;
	mach_msg_type_number_t *old_stateCnt;
{ return task_get_state(task, flavor, old_state, old_stateCnt); }
#else
(
	task_t task,
	thread_state_flavor_t flavor,
	thread_state_t old_state,
	mach_msg_type_number_t *old_stateCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_get_state */
mig_internal novalue _Xtask_get_state
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
		thread_state_flavor_t flavor;
		mach_msg_type_number_t old_stateCnt;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_get_state_t __Request;
	typedef __Reply__task_get_state_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_get_state_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_get_state_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;

	__DeclareRcvRpc(3433, "task_get_state")
	__BeforeRcvRpc(3433, "task_get_state")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_get_state_t__defined)
	check_result = __MIG_check__Request__task_get_state_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_get_state_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->old_stateCnt = 32;
	if (In0P->old_stateCnt < OutP->old_stateCnt)
		OutP->old_stateCnt = In0P->old_stateCnt;

	OutP->RetCode = task_get_state(task, In0P->flavor, OutP->old_state, &OutP->old_stateCnt);
	task_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;

	OutP->Head.msgh_size = (sizeof(Reply) - 128) + (_WALIGN_((4 * OutP->old_stateCnt)));

	__AfterRcvRpc(3433, "task_get_state")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_set_state_t__defined)
#define __MIG_check__Request__task_set_state_t__defined

mig_internal kern_return_t __MIG_check__Request__task_set_state_t(__attribute__((__unused__)) __Request__task_set_state_t *In0P)
{

	typedef __Request__task_set_state_t __Request;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	msgh_size = In0P->Head.msgh_size;
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (msgh_size < (sizeof(__Request) - 128)) ||  (msgh_size > (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if defined(__NDR_convert__int_rep__Request__task_set_state_t__new_stateCnt__defined)
	if (In0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Request__task_set_state_t__new_stateCnt(&In0P->new_stateCnt, In0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Request__task_set_state_t__new_stateCnt__defined */
#if	__MigTypeCheck
	if ( In0P->new_stateCnt > 32 )
		return MIG_BAD_ARGUMENTS;
	if (((msgh_size - (sizeof(__Request) - 128)) / 4 < In0P->new_stateCnt) ||
	    (msgh_size != (sizeof(__Request) - 128) + _WALIGN_(4 * In0P->new_stateCnt)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_set_state_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_set_state */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_state
#if	defined(LINTLIBRARY)
    (task, flavor, new_state, new_stateCnt)
	task_t task;
	thread_state_flavor_t flavor;
	thread_state_t new_state;
	mach_msg_type_number_t new_stateCnt;
{ return task_set_state(task, flavor, new_state, new_stateCnt); }
#else
(
	task_t task,
	thread_state_flavor_t flavor,
	thread_state_t new_state,
	mach_msg_type_number_t new_stateCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_set_state */
mig_internal novalue _Xtask_set_state
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
		thread_state_flavor_t flavor;
		mach_msg_type_number_t new_stateCnt;
		natural_t new_state[32];
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_set_state_t __Request;
	typedef __Reply__task_set_state_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_set_state_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_set_state_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;

	__DeclareRcvRpc(3434, "task_set_state")
	__BeforeRcvRpc(3434, "task_set_state")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_set_state_t__defined)
	check_result = __MIG_check__Request__task_set_state_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_set_state_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_set_state(task, In0P->flavor, In0P->new_state, In0P->new_stateCnt);
	task_deallocate(task);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3434, "task_set_state")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_set_phys_footprint_limit_t__defined)
#define __MIG_check__Request__task_set_phys_footprint_limit_t__defined

mig_internal kern_return_t __MIG_check__Request__task_set_phys_footprint_limit_t(__attribute__((__unused__)) __Request__task_set_phys_footprint_limit_t *In0P)
{

	typedef __Request__task_set_phys_footprint_limit_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_set_phys_footprint_limit_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_set_phys_footprint_limit */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_phys_footprint_limit
#if	defined(LINTLIBRARY)
    (task, new_limit, old_limit)
	task_t task;
	int new_limit;
	int *old_limit;
{ return task_set_phys_footprint_limit(task, new_limit, old_limit); }
#else
(
	task_t task,
	int new_limit,
	int *old_limit
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_set_phys_footprint_limit */
mig_internal novalue _Xtask_set_phys_footprint_limit
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
		int new_limit;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_set_phys_footprint_limit_t __Request;
	typedef __Reply__task_set_phys_footprint_limit_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_set_phys_footprint_limit_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_set_phys_footprint_limit_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;

	__DeclareRcvRpc(3435, "task_set_phys_footprint_limit")
	__BeforeRcvRpc(3435, "task_set_phys_footprint_limit")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_set_phys_footprint_limit_t__defined)
	check_result = __MIG_check__Request__task_set_phys_footprint_limit_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_set_phys_footprint_limit_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_set_phys_footprint_limit(task, In0P->new_limit, &OutP->old_limit);
	task_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3435, "task_set_phys_footprint_limit")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_suspend2_t__defined)
#define __MIG_check__Request__task_suspend2_t__defined

mig_internal kern_return_t __MIG_check__Request__task_suspend2_t(__attribute__((__unused__)) __Request__task_suspend2_t *In0P)
{

	typedef __Request__task_suspend2_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_suspend2_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_suspend2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_suspend2
#if	defined(LINTLIBRARY)
    (target_task, suspend_token)
	task_t target_task;
	task_suspension_token_t *suspend_token;
{ return task_suspend2(target_task, suspend_token); }
#else
(
	task_t target_task,
	task_suspension_token_t *suspend_token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_suspend2 */
mig_internal novalue _Xtask_suspend2
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
	typedef __Request__task_suspend2_t __Request;
	typedef __Reply__task_suspend2_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_suspend2_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_suspend2_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t suspend_tokenTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 18,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t suspend_tokenTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 18,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t target_task;
	task_suspension_token_t suspend_token;

	__DeclareRcvRpc(3436, "task_suspend2")
	__BeforeRcvRpc(3436, "task_suspend2")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_suspend2_t__defined)
	check_result = __MIG_check__Request__task_suspend2_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_suspend2_t__defined) */

#if	UseStaticTemplates
	OutP->suspend_token = suspend_tokenTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->suspend_token.disposition = 18;
#else
	OutP->suspend_token.disposition = 18;
#endif /* __MigKernelSpecificCode */
	OutP->suspend_token.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	target_task = convert_port_to_task(In0P->Head.msgh_request_port);

	RetCode = task_suspend2(target_task, &suspend_token);
	task_deallocate(target_task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->suspend_token.name = (mach_port_t)convert_task_suspension_token_to_port(suspend_token);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3436, "task_suspend2")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_resume2_t__defined)
#define __MIG_check__Request__task_resume2_t__defined

mig_internal kern_return_t __MIG_check__Request__task_resume2_t(__attribute__((__unused__)) __Request__task_resume2_t *In0P)
{

	typedef __Request__task_resume2_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_resume2_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_resume2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_resume2
#if	defined(LINTLIBRARY)
    (suspend_token)
	task_suspension_token_t suspend_token;
{ return task_resume2(suspend_token); }
#else
(
	task_suspension_token_t suspend_token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_resume2 */
mig_internal novalue _Xtask_resume2
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
	typedef __Request__task_resume2_t __Request;
	typedef __Reply__task_resume2_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_resume2_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_resume2_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(3437, "task_resume2")
	__BeforeRcvRpc(3437, "task_resume2")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_resume2_t__defined)
	check_result = __MIG_check__Request__task_resume2_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_resume2_t__defined) */

	OutP->RetCode = task_resume2(convert_port_to_task_suspension_token(In0P->Head.msgh_request_port));
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3437, "task_resume2")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_purgable_info_t__defined)
#define __MIG_check__Request__task_purgable_info_t__defined

mig_internal kern_return_t __MIG_check__Request__task_purgable_info_t(__attribute__((__unused__)) __Request__task_purgable_info_t *In0P)
{

	typedef __Request__task_purgable_info_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_purgable_info_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_purgable_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_purgable_info
#if	defined(LINTLIBRARY)
    (task, stats)
	task_t task;
	task_purgable_info_t *stats;
{ return task_purgable_info(task, stats); }
#else
(
	task_t task,
	task_purgable_info_t *stats
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_purgable_info */
mig_internal novalue _Xtask_purgable_info
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
	typedef __Request__task_purgable_info_t __Request;
	typedef __Reply__task_purgable_info_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_purgable_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_purgable_info_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;

	__DeclareRcvRpc(3438, "task_purgable_info")
	__BeforeRcvRpc(3438, "task_purgable_info")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_purgable_info_t__defined)
	check_result = __MIG_check__Request__task_purgable_info_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_purgable_info_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	OutP->RetCode = task_purgable_info(task, &OutP->stats);
	task_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(3438, "task_purgable_info")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_get_mach_voucher_t__defined)
#define __MIG_check__Request__task_get_mach_voucher_t__defined

mig_internal kern_return_t __MIG_check__Request__task_get_mach_voucher_t(__attribute__((__unused__)) __Request__task_get_mach_voucher_t *In0P)
{

	typedef __Request__task_get_mach_voucher_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_get_mach_voucher_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_get_mach_voucher */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_get_mach_voucher
#if	defined(LINTLIBRARY)
    (task, which, voucher)
	task_t task;
	mach_voucher_selector_t which;
	ipc_voucher_t *voucher;
{ return task_get_mach_voucher(task, which, voucher); }
#else
(
	task_t task,
	mach_voucher_selector_t which,
	ipc_voucher_t *voucher
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_get_mach_voucher */
mig_internal novalue _Xtask_get_mach_voucher
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
		mach_voucher_selector_t which;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_get_mach_voucher_t __Request;
	typedef __Reply__task_get_mach_voucher_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_get_mach_voucher_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_get_mach_voucher_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t voucherTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t voucherTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t task;
	ipc_voucher_t voucher;

	__DeclareRcvRpc(3439, "task_get_mach_voucher")
	__BeforeRcvRpc(3439, "task_get_mach_voucher")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_get_mach_voucher_t__defined)
	check_result = __MIG_check__Request__task_get_mach_voucher_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_get_mach_voucher_t__defined) */

#if	UseStaticTemplates
	OutP->voucher = voucherTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->voucher.disposition = 17;
#else
	OutP->voucher.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->voucher.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	task = convert_port_to_task(In0P->Head.msgh_request_port);

	RetCode = task_get_mach_voucher(task, In0P->which, &voucher);
	task_deallocate(task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->voucher.name = (mach_port_t)convert_voucher_to_port(voucher);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3439, "task_get_mach_voucher")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_set_mach_voucher_t__defined)
#define __MIG_check__Request__task_set_mach_voucher_t__defined

mig_internal kern_return_t __MIG_check__Request__task_set_mach_voucher_t(__attribute__((__unused__)) __Request__task_set_mach_voucher_t *In0P)
{

	typedef __Request__task_set_mach_voucher_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->voucher.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->voucher.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_set_mach_voucher_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_set_mach_voucher */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_mach_voucher
#if	defined(LINTLIBRARY)
    (task, voucher)
	task_t task;
	ipc_voucher_t voucher;
{ return task_set_mach_voucher(task, voucher); }
#else
(
	task_t task,
	ipc_voucher_t voucher
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_set_mach_voucher */
mig_internal novalue _Xtask_set_mach_voucher
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t voucher;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_set_mach_voucher_t __Request;
	typedef __Reply__task_set_mach_voucher_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_set_mach_voucher_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_set_mach_voucher_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	task_t task;
	ipc_voucher_t voucher;

	__DeclareRcvRpc(3440, "task_set_mach_voucher")
	__BeforeRcvRpc(3440, "task_set_mach_voucher")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_set_mach_voucher_t__defined)
	check_result = __MIG_check__Request__task_set_mach_voucher_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_set_mach_voucher_t__defined) */

	task = convert_port_to_task(In0P->Head.msgh_request_port);

	voucher = convert_port_to_voucher(In0P->voucher.name);

	OutP->RetCode = task_set_mach_voucher(task, voucher);
	ipc_voucher_release(voucher);
	task_deallocate(task);
#if	__MigKernelSpecificCode
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}

	if (IP_VALID((ipc_port_t)In0P->voucher.name))
		ipc_port_release_send((ipc_port_t)In0P->voucher.name);
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(3440, "task_set_mach_voucher")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__task_subsystem__
#if !defined(__MIG_check__Request__task_swap_mach_voucher_t__defined)
#define __MIG_check__Request__task_swap_mach_voucher_t__defined

mig_internal kern_return_t __MIG_check__Request__task_swap_mach_voucher_t(__attribute__((__unused__)) __Request__task_swap_mach_voucher_t *In0P)
{

	typedef __Request__task_swap_mach_voucher_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 2) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->new_voucher.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->new_voucher.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->old_voucher.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->old_voucher.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__task_swap_mach_voucher_t__defined) */
#endif /* __MIG_check__Request__task_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine task_swap_mach_voucher */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_swap_mach_voucher
#if	defined(LINTLIBRARY)
    (task, new_voucher, old_voucher)
	task_t task;
	ipc_voucher_t new_voucher;
	ipc_voucher_t *old_voucher;
{ return task_swap_mach_voucher(task, new_voucher, old_voucher); }
#else
(
	task_t task,
	ipc_voucher_t new_voucher,
	ipc_voucher_t *old_voucher
);
#endif	/* defined(LINTLIBRARY) */

/* Routine task_swap_mach_voucher */
mig_internal novalue _Xtask_swap_mach_voucher
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_voucher;
		mach_msg_port_descriptor_t old_voucher;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__task_swap_mach_voucher_t __Request;
	typedef __Reply__task_swap_mach_voucher_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__task_swap_mach_voucher_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__task_swap_mach_voucher_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t old_voucherTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t old_voucherTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	task_t task;
	ipc_voucher_t new_voucher;
	ipc_voucher_t old_voucher;

	__DeclareRcvRpc(3441, "task_swap_mach_voucher")
	__BeforeRcvRpc(3441, "task_swap_mach_voucher")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__task_swap_mach_voucher_t__defined)
	check_result = __MIG_check__Request__task_swap_mach_voucher_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__task_swap_mach_voucher_t__defined) */

#if	UseStaticTemplates
	OutP->old_voucher = old_voucherTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->old_voucher.disposition = 17;
#else
	OutP->old_voucher.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->old_voucher.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	task = convert_port_to_task(In0P->Head.msgh_request_port);

	new_voucher = convert_port_to_voucher(In0P->new_voucher.name);

	old_voucher = convert_port_to_voucher(In0P->old_voucher.name);

	RetCode = task_swap_mach_voucher(task, new_voucher, &old_voucher);
	ipc_voucher_release(new_voucher);
	task_deallocate(task);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode

	if (IP_VALID((ipc_port_t)In0P->old_voucher.name))
		ipc_port_release_send((ipc_port_t)In0P->old_voucher.name);

	if (IP_VALID((ipc_port_t)In0P->new_voucher.name))
		ipc_port_release_send((ipc_port_t)In0P->new_voucher.name);
#endif /* __MigKernelSpecificCode */
	OutP->old_voucher.name = (mach_port_t)convert_voucher_to_port(old_voucher);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(3441, "task_swap_mach_voucher")
}


#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
boolean_t task_server(
		mach_msg_header_t *InHeadP,
		mach_msg_header_t *OutHeadP);

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
mig_routine_t task_server_routine(
		mach_msg_header_t *InHeadP);


/* Description of this subsystem, for use in direct RPC */
extern const struct task_subsystem task_subsystem;
const struct task_subsystem {
	mig_server_routine_t 	server;	/* Server routine */
	mach_msg_id_t	start;	/* Min routine number */
	mach_msg_id_t	end;	/* Max routine number + 1 */
	unsigned int	maxsize;	/* Max msg size */
	vm_address_t	reserved;	/* Reserved */
	struct routine_descriptor	/*Array of routine descriptors */
		routine[42];
} task_subsystem = {
	task_server_routine,
	3400,
	3442,
	(mach_msg_size_t)sizeof(union __ReplyUnion__task_subsystem),
	(vm_address_t)0,
	{
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_create, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_create_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_terminate, 1, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_terminate_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_threads, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_threads_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_ports_register, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_ports_register_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_ports_lookup, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_ports_lookup_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_info, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_info_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_set_info, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_set_info_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_suspend, 1, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_suspend_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_resume, 1, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_resume_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_get_special_port, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_get_special_port_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_set_special_port, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_set_special_port_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xthread_create_from_user, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__thread_create_from_user_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xthread_create_running_from_user, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__thread_create_running_from_user_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_set_exception_ports, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_set_exception_ports_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_get_exception_ports, 7, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_get_exception_ports_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_swap_exception_ports, 10, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_swap_exception_ports_t) },
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xsemaphore_create, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__semaphore_create_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xsemaphore_destroy, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__semaphore_destroy_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_policy_set, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_policy_set_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_policy_get, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_policy_get_t) },
		{0, 0, 0, 0, 0, 0},
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_policy, 6, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_policy_t) },
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_set_ras_pc, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_set_ras_pc_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_zone_info, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_zone_info_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_assign, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_assign_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_assign_default, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_assign_default_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_get_assignment, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_get_assignment_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_set_policy, 8, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_set_policy_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_get_state, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_get_state_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_set_state, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_set_state_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_set_phys_footprint_limit, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_set_phys_footprint_limit_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_suspend2, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_suspend2_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_resume2, 1, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_resume2_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_purgable_info, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_purgable_info_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_get_mach_voucher, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_get_mach_voucher_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_set_mach_voucher, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_set_mach_voucher_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xtask_swap_mach_voucher, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__task_swap_mach_voucher_t) },
	}
};

mig_external boolean_t task_server
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

	if ((InHeadP->msgh_id > 3441) || (InHeadP->msgh_id < 3400) ||
	    ((routine = task_subsystem.routine[InHeadP->msgh_id - 3400].stub_routine) == 0)) {
		((mig_reply_error_t *)OutHeadP)->NDR = NDR_record;
		((mig_reply_error_t *)OutHeadP)->RetCode = MIG_BAD_ID;
		return FALSE;
	}
	(*routine) (InHeadP, OutHeadP);
	return TRUE;
}

mig_external mig_routine_t task_server_routine
	(mach_msg_header_t *InHeadP)
{
	register int msgh_id;

	msgh_id = InHeadP->msgh_id - 3400;

	if ((msgh_id > 41) || (msgh_id < 0))
		return 0;

	return task_subsystem.routine[msgh_id].stub_routine;
}

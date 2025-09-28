/*
 * IDENTIFICATION:
 * stub generated Thu Jun 11 18:17:44 2015
 * with a MiG generated Thu Jun 11 16:16:11 PDT 2015 by kmacy@serenity
 * OPTIONS: 
 *	KernelServer
 */

/* Module host_priv */

#define	__MIG_check__Request__host_priv_subsystem__ 1

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

#ifndef __Request__host_priv_subsystem__defined
#define __Request__host_priv_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
	} __Request__host_get_boot_info_t;
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
		int options;
	} __Request__host_reboot_t;
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
		host_flavor_t flavor;
		mach_msg_type_number_t host_info_outCnt;
	} __Request__host_priv_statistics_t;
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
		mach_msg_port_descriptor_t default_manager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		memory_object_cluster_size_t cluster_size;
	} __Request__host_default_memory_manager_t;
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
		mach_msg_port_descriptor_t task;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		vm_prot_t desired_access;
	} __Request__vm_wire_t;
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
		mach_msg_port_descriptor_t thread;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t wired;
	} __Request__thread_wire_t;
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
		mach_msg_port_descriptor_t task;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		int flags;
	} __Request__vm_allocate_cpm_t;
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
	} __Request__host_processors_t;
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
		clock_id_t clock_id;
	} __Request__host_get_clock_control_t;
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
		int node;
		int which;
	} __Request__host_get_special_port_t;
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
		mach_msg_port_descriptor_t port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		int which;
	} __Request__host_set_special_port_t;
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
	} __Request__host_set_exception_ports_t;
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
	} __Request__host_get_exception_ports_t;
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
	} __Request__host_swap_exception_ports_t;
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
		mach_msg_port_descriptor_t task;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_vm_address_t address;
		mach_vm_size_t size;
		vm_prot_t desired_access;
	} __Request__mach_vm_wire_t;
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
	} __Request__host_processor_sets_t;
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
		mach_msg_port_descriptor_t set_name;
		/* end of the kernel processed data */
	} __Request__host_processor_set_priv_t;
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
		mach_msg_port_descriptor_t control_port;
		/* end of the kernel processed data */
	} __Request__set_dp_control_port_t;
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
	} __Request__get_dp_control_port_t;
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
		mach_msg_port_descriptor_t server;
		/* end of the kernel processed data */
	} __Request__host_set_UNDServer_t;
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
	} __Request__host_get_UNDServer_t;
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
		mach_msg_ool_descriptor_t request_data;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		uint32_t user_log_flags;
		mach_msg_type_number_t request_dataCnt;
	} __Request__kext_request_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__host_priv_subsystem__defined */

/* typedefs for all replies */

#ifndef __Reply__host_priv_subsystem__defined
#define __Reply__host_priv_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t boot_infoOffset; /* MiG doesn't use it */
		mach_msg_type_number_t boot_infoCnt;
		char boot_info[4096];
	} __Reply__host_get_boot_info_t;
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
	} __Reply__host_reboot_t;
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
		mach_msg_type_number_t host_info_outCnt;
		integer_t host_info_out[68];
	} __Reply__host_priv_statistics_t;
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
		mach_msg_port_descriptor_t default_manager;
		/* end of the kernel processed data */
	} __Reply__host_default_memory_manager_t;
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
	} __Reply__vm_wire_t;
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
	} __Reply__thread_wire_t;
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
	} __Reply__vm_allocate_cpm_t;
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
		mach_msg_ool_ports_descriptor_t out_processor_list;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t out_processor_listCnt;
	} __Reply__host_processors_t;
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
		mach_msg_port_descriptor_t clock_ctrl;
		/* end of the kernel processed data */
	} __Reply__host_get_clock_control_t;
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
		mach_msg_port_descriptor_t port;
		/* end of the kernel processed data */
	} __Reply__host_get_special_port_t;
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
	} __Reply__host_set_special_port_t;
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
	} __Reply__host_set_exception_ports_t;
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
	} __Reply__host_get_exception_ports_t;
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
	} __Reply__host_swap_exception_ports_t;
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
	} __Reply__mach_vm_wire_t;
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
		mach_msg_ool_ports_descriptor_t processor_sets;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t processor_setsCnt;
	} __Reply__host_processor_sets_t;
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
		mach_msg_port_descriptor_t set;
		/* end of the kernel processed data */
	} __Reply__host_processor_set_priv_t;
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
	} __Reply__set_dp_control_port_t;
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
		mach_msg_port_descriptor_t contorl_port;
		/* end of the kernel processed data */
	} __Reply__get_dp_control_port_t;
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
	} __Reply__host_set_UNDServer_t;
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
		mach_msg_port_descriptor_t server;
		/* end of the kernel processed data */
	} __Reply__host_get_UNDServer_t;
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
		mach_msg_ool_descriptor_t response_data;
		mach_msg_ool_descriptor_t log_data;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t response_dataCnt;
		mach_msg_type_number_t log_dataCnt;
		kern_return_t op_result;
	} __Reply__kext_request_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__host_priv_subsystem__defined */


/* union of all replies */

#ifndef __ReplyUnion__host_priv_subsystem__defined
#define __ReplyUnion__host_priv_subsystem__defined
union __ReplyUnion__host_priv_subsystem {
	__Reply__host_get_boot_info_t Reply_host_get_boot_info;
	__Reply__host_reboot_t Reply_host_reboot;
	__Reply__host_priv_statistics_t Reply_host_priv_statistics;
	__Reply__host_default_memory_manager_t Reply_host_default_memory_manager;
	__Reply__vm_wire_t Reply_vm_wire;
	__Reply__thread_wire_t Reply_thread_wire;
	__Reply__vm_allocate_cpm_t Reply_vm_allocate_cpm;
	__Reply__host_processors_t Reply_host_processors;
	__Reply__host_get_clock_control_t Reply_host_get_clock_control;
	__Reply__host_get_special_port_t Reply_host_get_special_port;
	__Reply__host_set_special_port_t Reply_host_set_special_port;
	__Reply__host_set_exception_ports_t Reply_host_set_exception_ports;
	__Reply__host_get_exception_ports_t Reply_host_get_exception_ports;
	__Reply__host_swap_exception_ports_t Reply_host_swap_exception_ports;
	__Reply__mach_vm_wire_t Reply_mach_vm_wire;
	__Reply__host_processor_sets_t Reply_host_processor_sets;
	__Reply__host_processor_set_priv_t Reply_host_processor_set_priv;
	__Reply__set_dp_control_port_t Reply_set_dp_control_port;
	__Reply__get_dp_control_port_t Reply_get_dp_control_port;
	__Reply__host_set_UNDServer_t Reply_host_set_UNDServer;
	__Reply__host_get_UNDServer_t Reply_host_get_UNDServer;
	__Reply__kext_request_t Reply_kext_request;
};
#endif /* __RequestUnion__host_priv_subsystem__defined */
/* Forward Declarations */


mig_internal novalue _Xhost_get_boot_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_reboot
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_priv_statistics
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_default_memory_manager
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xvm_wire
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xthread_wire
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xvm_allocate_cpm
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_processors
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_get_clock_control
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_get_special_port
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_set_special_port
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_set_exception_ports
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_get_exception_ports
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_swap_exception_ports
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_vm_wire
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_processor_sets
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_processor_set_priv
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xset_dp_control_port
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xget_dp_control_port
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_set_UNDServer
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_get_UNDServer
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xkext_request
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);


#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_get_boot_info_t__defined)
#define __MIG_check__Request__host_get_boot_info_t__defined

mig_internal kern_return_t __MIG_check__Request__host_get_boot_info_t(__attribute__((__unused__)) __Request__host_get_boot_info_t *In0P)
{

	typedef __Request__host_get_boot_info_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_get_boot_info_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_get_boot_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_get_boot_info
#if	defined(LINTLIBRARY)
    (host_priv, boot_info)
	host_priv_t host_priv;
	kernel_boot_info_t boot_info;
{ return host_get_boot_info(host_priv, boot_info); }
#else
(
	host_priv_t host_priv,
	kernel_boot_info_t boot_info
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_get_boot_info */
mig_internal novalue _Xhost_get_boot_info
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
	typedef __Request__host_get_boot_info_t __Request;
	typedef __Reply__host_get_boot_info_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_get_boot_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_get_boot_info_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(400, "host_get_boot_info")
	__BeforeRcvRpc(400, "host_get_boot_info")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_get_boot_info_t__defined)
	check_result = __MIG_check__Request__host_get_boot_info_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_get_boot_info_t__defined) */

	OutP->RetCode = host_get_boot_info(convert_port_to_host_priv(In0P->Head.msgh_request_port), OutP->boot_info);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;

#ifdef __LP64__
	{
		size_t strLength = strlen(OutP->boot_info) + 1;
		if (strLength > 0xffffffff)
			MIG_RETURN_ERROR(OutP, MIG_BAD_ARGUMENTS);
		OutP->boot_infoCnt = (mach_msg_type_number_t) strLength;
	}
#else
	OutP->boot_infoCnt = (mach_msg_type_number_t) strlen(OutP->boot_info) + 1;
#endif /* __LP64__ */
	OutP->Head.msgh_size = (sizeof(Reply) - 4096) + (_WALIGN_((OutP->boot_infoCnt + 3) & ~3));

	__AfterRcvRpc(400, "host_get_boot_info")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_reboot_t__defined)
#define __MIG_check__Request__host_reboot_t__defined

mig_internal kern_return_t __MIG_check__Request__host_reboot_t(__attribute__((__unused__)) __Request__host_reboot_t *In0P)
{

	typedef __Request__host_reboot_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_reboot_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_reboot */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_reboot
#if	defined(LINTLIBRARY)
    (host_priv, options)
	host_priv_t host_priv;
	int options;
{ return host_reboot(host_priv, options); }
#else
(
	host_priv_t host_priv,
	int options
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_reboot */
mig_internal novalue _Xhost_reboot
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
		int options;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_reboot_t __Request;
	typedef __Reply__host_reboot_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_reboot_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_reboot_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(401, "host_reboot")
	__BeforeRcvRpc(401, "host_reboot")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_reboot_t__defined)
	check_result = __MIG_check__Request__host_reboot_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_reboot_t__defined) */

	OutP->RetCode = host_reboot(convert_port_to_host_priv(In0P->Head.msgh_request_port), In0P->options);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(401, "host_reboot")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_priv_statistics_t__defined)
#define __MIG_check__Request__host_priv_statistics_t__defined

mig_internal kern_return_t __MIG_check__Request__host_priv_statistics_t(__attribute__((__unused__)) __Request__host_priv_statistics_t *In0P)
{

	typedef __Request__host_priv_statistics_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_priv_statistics_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_priv_statistics */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_priv_statistics
#if	defined(LINTLIBRARY)
    (host_priv, flavor, host_info_out, host_info_outCnt)
	host_priv_t host_priv;
	host_flavor_t flavor;
	host_info_t host_info_out;
	mach_msg_type_number_t *host_info_outCnt;
{ return host_priv_statistics(host_priv, flavor, host_info_out, host_info_outCnt); }
#else
(
	host_priv_t host_priv,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_priv_statistics */
mig_internal novalue _Xhost_priv_statistics
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
		host_flavor_t flavor;
		mach_msg_type_number_t host_info_outCnt;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_priv_statistics_t __Request;
	typedef __Reply__host_priv_statistics_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_priv_statistics_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_priv_statistics_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(402, "host_priv_statistics")
	__BeforeRcvRpc(402, "host_priv_statistics")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_priv_statistics_t__defined)
	check_result = __MIG_check__Request__host_priv_statistics_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_priv_statistics_t__defined) */

	OutP->host_info_outCnt = 68;
	if (In0P->host_info_outCnt < OutP->host_info_outCnt)
		OutP->host_info_outCnt = In0P->host_info_outCnt;

	OutP->RetCode = host_priv_statistics(convert_port_to_host_priv(In0P->Head.msgh_request_port), In0P->flavor, OutP->host_info_out, &OutP->host_info_outCnt);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;

	OutP->Head.msgh_size = (sizeof(Reply) - 272) + (_WALIGN_((4 * OutP->host_info_outCnt)));

	__AfterRcvRpc(402, "host_priv_statistics")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_default_memory_manager_t__defined)
#define __MIG_check__Request__host_default_memory_manager_t__defined

mig_internal kern_return_t __MIG_check__Request__host_default_memory_manager_t(__attribute__((__unused__)) __Request__host_default_memory_manager_t *In0P)
{

	typedef __Request__host_default_memory_manager_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->default_manager.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->default_manager.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_default_memory_manager_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_default_memory_manager */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_default_memory_manager
#if	defined(LINTLIBRARY)
    (host_priv, default_manager, cluster_size)
	host_priv_t host_priv;
	memory_object_default_t *default_manager;
	memory_object_cluster_size_t cluster_size;
{ return host_default_memory_manager(host_priv, default_manager, cluster_size); }
#else
(
	host_priv_t host_priv,
	memory_object_default_t *default_manager,
	memory_object_cluster_size_t cluster_size
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_default_memory_manager */
mig_internal novalue _Xhost_default_memory_manager
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t default_manager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		memory_object_cluster_size_t cluster_size;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_default_memory_manager_t __Request;
	typedef __Reply__host_default_memory_manager_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_default_memory_manager_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_default_memory_manager_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t default_managerTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t default_managerTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 20,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	__DeclareRcvRpc(403, "host_default_memory_manager")
	__BeforeRcvRpc(403, "host_default_memory_manager")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_default_memory_manager_t__defined)
	check_result = __MIG_check__Request__host_default_memory_manager_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_default_memory_manager_t__defined) */

#if	UseStaticTemplates
	OutP->default_manager = default_managerTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->default_manager.disposition = 17;
#else
	OutP->default_manager.disposition = 20;
#endif /* __MigKernelSpecificCode */
	OutP->default_manager.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = host_default_memory_manager(convert_port_to_host_priv(In0P->Head.msgh_request_port), &In0P->default_manager.name, In0P->cluster_size);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->default_manager.name = In0P->default_manager.name;

	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(403, "host_default_memory_manager")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__vm_wire_t__defined)
#define __MIG_check__Request__vm_wire_t__defined

mig_internal kern_return_t __MIG_check__Request__vm_wire_t(__attribute__((__unused__)) __Request__vm_wire_t *In0P)
{

	typedef __Request__vm_wire_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->task.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->task.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__vm_wire_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine vm_wire */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_wire
#if	defined(LINTLIBRARY)
    (host_priv, task, address, size, desired_access)
	host_priv_t host_priv;
	vm_map_t task;
	vm_address_t address;
	vm_size_t size;
	vm_prot_t desired_access;
{ return vm_wire(host_priv, task, address, size, desired_access); }
#else
(
	host_priv_t host_priv,
	vm_map_t task,
	vm_address_t address,
	vm_size_t size,
	vm_prot_t desired_access
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_wire */
mig_internal novalue _Xvm_wire
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t task;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		vm_prot_t desired_access;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__vm_wire_t __Request;
	typedef __Reply__vm_wire_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__vm_wire_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__vm_wire_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	vm_map_t task;

	__DeclareRcvRpc(404, "vm_wire")
	__BeforeRcvRpc(404, "vm_wire")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__vm_wire_t__defined)
	check_result = __MIG_check__Request__vm_wire_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__vm_wire_t__defined) */

	task = convert_port_to_map(In0P->task.name);

	OutP->RetCode = vm_wire(convert_port_to_host_priv(In0P->Head.msgh_request_port), task, In0P->address, In0P->size, In0P->desired_access);
	vm_map_deallocate(task);
#if	__MigKernelSpecificCode
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}

	if (IP_VALID((ipc_port_t)In0P->task.name))
		ipc_port_release_send((ipc_port_t)In0P->task.name);
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(404, "vm_wire")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__thread_wire_t__defined)
#define __MIG_check__Request__thread_wire_t__defined

mig_internal kern_return_t __MIG_check__Request__thread_wire_t(__attribute__((__unused__)) __Request__thread_wire_t *In0P)
{

	typedef __Request__thread_wire_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->thread.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->thread.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__thread_wire_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine thread_wire */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t thread_wire
#if	defined(LINTLIBRARY)
    (host_priv, thread, wired)
	host_priv_t host_priv;
	thread_act_t thread;
	boolean_t wired;
{ return thread_wire(host_priv, thread, wired); }
#else
(
	host_priv_t host_priv,
	thread_act_t thread,
	boolean_t wired
);
#endif	/* defined(LINTLIBRARY) */

/* Routine thread_wire */
mig_internal novalue _Xthread_wire
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t thread;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t wired;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__thread_wire_t __Request;
	typedef __Reply__thread_wire_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__thread_wire_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__thread_wire_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	thread_act_t thread;

	__DeclareRcvRpc(405, "thread_wire")
	__BeforeRcvRpc(405, "thread_wire")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__thread_wire_t__defined)
	check_result = __MIG_check__Request__thread_wire_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__thread_wire_t__defined) */

	thread = convert_port_to_thread(In0P->thread.name);

	OutP->RetCode = thread_wire(convert_port_to_host_priv(In0P->Head.msgh_request_port), thread, In0P->wired);
	thread_deallocate(thread);
#if	__MigKernelSpecificCode
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}

	if (IP_VALID((ipc_port_t)In0P->thread.name))
		ipc_port_release_send((ipc_port_t)In0P->thread.name);
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(405, "thread_wire")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__vm_allocate_cpm_t__defined)
#define __MIG_check__Request__vm_allocate_cpm_t__defined

mig_internal kern_return_t __MIG_check__Request__vm_allocate_cpm_t(__attribute__((__unused__)) __Request__vm_allocate_cpm_t *In0P)
{

	typedef __Request__vm_allocate_cpm_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->task.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->task.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__vm_allocate_cpm_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine vm_allocate_cpm */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t vm_allocate_cpm
#if	defined(LINTLIBRARY)
    (host_priv, task, address, size, flags)
	host_priv_t host_priv;
	vm_map_t task;
	vm_address_t *address;
	vm_size_t size;
	int flags;
{ return vm_allocate_cpm(host_priv, task, address, size, flags); }
#else
(
	host_priv_t host_priv,
	vm_map_t task,
	vm_address_t *address,
	vm_size_t size,
	int flags
);
#endif	/* defined(LINTLIBRARY) */

/* Routine vm_allocate_cpm */
mig_internal novalue _Xvm_allocate_cpm
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t task;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		vm_address_t address;
		vm_size_t size;
		int flags;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__vm_allocate_cpm_t __Request;
	typedef __Reply__vm_allocate_cpm_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__vm_allocate_cpm_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__vm_allocate_cpm_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	vm_map_t task;

	__DeclareRcvRpc(406, "vm_allocate_cpm")
	__BeforeRcvRpc(406, "vm_allocate_cpm")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__vm_allocate_cpm_t__defined)
	check_result = __MIG_check__Request__vm_allocate_cpm_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__vm_allocate_cpm_t__defined) */

	task = convert_port_to_map(In0P->task.name);

	OutP->RetCode = vm_allocate_cpm(convert_port_to_host_priv(In0P->Head.msgh_request_port), task, &In0P->address, In0P->size, In0P->flags);
	vm_map_deallocate(task);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode

	if (IP_VALID((ipc_port_t)In0P->task.name))
		ipc_port_release_send((ipc_port_t)In0P->task.name);
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->address = In0P->address;

	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(406, "vm_allocate_cpm")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_processors_t__defined)
#define __MIG_check__Request__host_processors_t__defined

mig_internal kern_return_t __MIG_check__Request__host_processors_t(__attribute__((__unused__)) __Request__host_processors_t *In0P)
{

	typedef __Request__host_processors_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_processors_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_processors */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_processors
#if	defined(LINTLIBRARY)
    (host_priv, out_processor_list, out_processor_listCnt)
	host_priv_t host_priv;
	processor_array_t *out_processor_list;
	mach_msg_type_number_t *out_processor_listCnt;
{ return host_processors(host_priv, out_processor_list, out_processor_listCnt); }
#else
(
	host_priv_t host_priv,
	processor_array_t *out_processor_list,
	mach_msg_type_number_t *out_processor_listCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_processors */
mig_internal novalue _Xhost_processors
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
	typedef __Request__host_processors_t __Request;
	typedef __Reply__host_processors_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_processors_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_processors_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_ool_ports_descriptor_t out_processor_listTemplate = {
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
	const static mach_msg_ool_ports_descriptor_t out_processor_listTemplate = {
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
	__DeclareRcvRpc(407, "host_processors")
	__BeforeRcvRpc(407, "host_processors")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_processors_t__defined)
	check_result = __MIG_check__Request__host_processors_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_processors_t__defined) */

#if	UseStaticTemplates
	OutP->out_processor_list = out_processor_listTemplate;
#else	/* UseStaticTemplates */
#if	__MigKernelSpecificCode
	OutP->out_processor_list.disposition = 17;
#else
	OutP->out_processor_list.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->out_processor_list.deallocate =  FALSE;
	OutP->out_processor_list.type = MACH_MSG_OOL_PORTS_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = host_processors(convert_port_to_host_priv(In0P->Head.msgh_request_port), (processor_array_t *)&(OutP->out_processor_list.address), &OutP->out_processor_listCnt);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->out_processor_list.count = OutP->out_processor_listCnt;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(407, "host_processors")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_get_clock_control_t__defined)
#define __MIG_check__Request__host_get_clock_control_t__defined

mig_internal kern_return_t __MIG_check__Request__host_get_clock_control_t(__attribute__((__unused__)) __Request__host_get_clock_control_t *In0P)
{

	typedef __Request__host_get_clock_control_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_get_clock_control_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_get_clock_control */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_get_clock_control
#if	defined(LINTLIBRARY)
    (host_priv, clock_id, clock_ctrl)
	host_priv_t host_priv;
	clock_id_t clock_id;
	clock_ctrl_t *clock_ctrl;
{ return host_get_clock_control(host_priv, clock_id, clock_ctrl); }
#else
(
	host_priv_t host_priv,
	clock_id_t clock_id,
	clock_ctrl_t *clock_ctrl
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_get_clock_control */
mig_internal novalue _Xhost_get_clock_control
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
		clock_id_t clock_id;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_get_clock_control_t __Request;
	typedef __Reply__host_get_clock_control_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_get_clock_control_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_get_clock_control_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t clock_ctrlTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t clock_ctrlTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	clock_ctrl_t clock_ctrl;

	__DeclareRcvRpc(408, "host_get_clock_control")
	__BeforeRcvRpc(408, "host_get_clock_control")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_get_clock_control_t__defined)
	check_result = __MIG_check__Request__host_get_clock_control_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_get_clock_control_t__defined) */

#if	UseStaticTemplates
	OutP->clock_ctrl = clock_ctrlTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->clock_ctrl.disposition = 17;
#else
	OutP->clock_ctrl.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->clock_ctrl.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = host_get_clock_control(convert_port_to_host_priv(In0P->Head.msgh_request_port), In0P->clock_id, &clock_ctrl);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->clock_ctrl.name = (mach_port_t)convert_clock_ctrl_to_port(clock_ctrl);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(408, "host_get_clock_control")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_get_special_port_t__defined)
#define __MIG_check__Request__host_get_special_port_t__defined

mig_internal kern_return_t __MIG_check__Request__host_get_special_port_t(__attribute__((__unused__)) __Request__host_get_special_port_t *In0P)
{

	typedef __Request__host_get_special_port_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_get_special_port_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_get_special_port */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_get_special_port
#if	defined(LINTLIBRARY)
    (host_priv, node, which, port)
	host_priv_t host_priv;
	int node;
	int which;
	mach_port_t *port;
{ return host_get_special_port(host_priv, node, which, port); }
#else
(
	host_priv_t host_priv,
	int node,
	int which,
	mach_port_t *port
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_get_special_port */
mig_internal novalue _Xhost_get_special_port
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
		int node;
		int which;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_get_special_port_t __Request;
	typedef __Reply__host_get_special_port_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_get_special_port_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_get_special_port_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t portTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t portTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	__DeclareRcvRpc(412, "host_get_special_port")
	__BeforeRcvRpc(412, "host_get_special_port")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_get_special_port_t__defined)
	check_result = __MIG_check__Request__host_get_special_port_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_get_special_port_t__defined) */

#if	UseStaticTemplates
	OutP->port = portTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->port.disposition = 17;
#else
	OutP->port.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->port.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = host_get_special_port(convert_port_to_host_priv(In0P->Head.msgh_request_port), In0P->node, In0P->which, &OutP->port.name);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(412, "host_get_special_port")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_set_special_port_t__defined)
#define __MIG_check__Request__host_set_special_port_t__defined

mig_internal kern_return_t __MIG_check__Request__host_set_special_port_t(__attribute__((__unused__)) __Request__host_set_special_port_t *In0P)
{

	typedef __Request__host_set_special_port_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->port.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->port.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_set_special_port_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_set_special_port */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_set_special_port
#if	defined(LINTLIBRARY)
    (host_priv, which, port)
	host_priv_t host_priv;
	int which;
	mach_port_t port;
{ return host_set_special_port(host_priv, which, port); }
#else
(
	host_priv_t host_priv,
	int which,
	mach_port_t port
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_set_special_port */
mig_internal novalue _Xhost_set_special_port
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		int which;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_set_special_port_t __Request;
	typedef __Reply__host_set_special_port_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_set_special_port_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_set_special_port_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(413, "host_set_special_port")
	__BeforeRcvRpc(413, "host_set_special_port")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_set_special_port_t__defined)
	check_result = __MIG_check__Request__host_set_special_port_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_set_special_port_t__defined) */

	OutP->RetCode = host_set_special_port(convert_port_to_host_priv(In0P->Head.msgh_request_port), In0P->which, In0P->port.name);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(413, "host_set_special_port")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_set_exception_ports_t__defined)
#define __MIG_check__Request__host_set_exception_ports_t__defined

mig_internal kern_return_t __MIG_check__Request__host_set_exception_ports_t(__attribute__((__unused__)) __Request__host_set_exception_ports_t *In0P)
{

	typedef __Request__host_set_exception_ports_t __Request;
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
#endif /* !defined(__MIG_check__Request__host_set_exception_ports_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_set_exception_ports */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_set_exception_ports
#if	defined(LINTLIBRARY)
    (host_priv, exception_mask, new_port, behavior, new_flavor)
	host_priv_t host_priv;
	exception_mask_t exception_mask;
	mach_port_t new_port;
	exception_behavior_t behavior;
	thread_state_flavor_t new_flavor;
{ return host_set_exception_ports(host_priv, exception_mask, new_port, behavior, new_flavor); }
#else
(
	host_priv_t host_priv,
	exception_mask_t exception_mask,
	mach_port_t new_port,
	exception_behavior_t behavior,
	thread_state_flavor_t new_flavor
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_set_exception_ports */
mig_internal novalue _Xhost_set_exception_ports
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
	typedef __Request__host_set_exception_ports_t __Request;
	typedef __Reply__host_set_exception_ports_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_set_exception_ports_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_set_exception_ports_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(414, "host_set_exception_ports")
	__BeforeRcvRpc(414, "host_set_exception_ports")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_set_exception_ports_t__defined)
	check_result = __MIG_check__Request__host_set_exception_ports_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_set_exception_ports_t__defined) */

	OutP->RetCode = host_set_exception_ports(convert_port_to_host_priv(In0P->Head.msgh_request_port), In0P->exception_mask, In0P->new_port.name, In0P->behavior, In0P->new_flavor);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(414, "host_set_exception_ports")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_get_exception_ports_t__defined)
#define __MIG_check__Request__host_get_exception_ports_t__defined

mig_internal kern_return_t __MIG_check__Request__host_get_exception_ports_t(__attribute__((__unused__)) __Request__host_get_exception_ports_t *In0P)
{

	typedef __Request__host_get_exception_ports_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_get_exception_ports_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_get_exception_ports */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_get_exception_ports
#if	defined(LINTLIBRARY)
    (host_priv, exception_mask, masks, masksCnt, old_handlers, old_behaviors, old_flavors)
	host_priv_t host_priv;
	exception_mask_t exception_mask;
	exception_mask_array_t masks;
	mach_msg_type_number_t *masksCnt;
	exception_handler_array_t old_handlers;
	exception_behavior_array_t old_behaviors;
	exception_flavor_array_t old_flavors;
{ return host_get_exception_ports(host_priv, exception_mask, masks, masksCnt, old_handlers, old_behaviors, old_flavors); }
#else
(
	host_priv_t host_priv,
	exception_mask_t exception_mask,
	exception_mask_array_t masks,
	mach_msg_type_number_t *masksCnt,
	exception_handler_array_t old_handlers,
	exception_behavior_array_t old_behaviors,
	exception_flavor_array_t old_flavors
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_get_exception_ports */
mig_internal novalue _Xhost_get_exception_ports
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
	typedef __Request__host_get_exception_ports_t __Request;
	typedef __Reply__host_get_exception_ports_t Reply;

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

#ifdef	__MIG_check__Request__host_get_exception_ports_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_get_exception_ports_t__defined */

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
	mach_msg_type_number_t masksCnt;
	exception_handler_t old_handlers[32];
	exception_behavior_t old_behaviors[32];
	thread_state_flavor_t old_flavors[32];

	__DeclareRcvRpc(415, "host_get_exception_ports")
	__BeforeRcvRpc(415, "host_get_exception_ports")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_get_exception_ports_t__defined)
	check_result = __MIG_check__Request__host_get_exception_ports_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_get_exception_ports_t__defined) */

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


	masksCnt = 32;

	RetCode = host_get_exception_ports(convert_port_to_host_priv(In0P->Head.msgh_request_port), In0P->exception_mask, OutP->masks, &masksCnt, old_handlers, old_behaviors, old_flavors);
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
	__AfterRcvRpc(415, "host_get_exception_ports")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_swap_exception_ports_t__defined)
#define __MIG_check__Request__host_swap_exception_ports_t__defined

mig_internal kern_return_t __MIG_check__Request__host_swap_exception_ports_t(__attribute__((__unused__)) __Request__host_swap_exception_ports_t *In0P)
{

	typedef __Request__host_swap_exception_ports_t __Request;
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
#endif /* !defined(__MIG_check__Request__host_swap_exception_ports_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_swap_exception_ports */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_swap_exception_ports
#if	defined(LINTLIBRARY)
    (host_priv, exception_mask, new_port, behavior, new_flavor, masks, masksCnt, old_handlerss, old_behaviors, old_flavors)
	host_priv_t host_priv;
	exception_mask_t exception_mask;
	mach_port_t new_port;
	exception_behavior_t behavior;
	thread_state_flavor_t new_flavor;
	exception_mask_array_t masks;
	mach_msg_type_number_t *masksCnt;
	exception_handler_array_t old_handlerss;
	exception_behavior_array_t old_behaviors;
	exception_flavor_array_t old_flavors;
{ return host_swap_exception_ports(host_priv, exception_mask, new_port, behavior, new_flavor, masks, masksCnt, old_handlerss, old_behaviors, old_flavors); }
#else
(
	host_priv_t host_priv,
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

/* Routine host_swap_exception_ports */
mig_internal novalue _Xhost_swap_exception_ports
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
	typedef __Request__host_swap_exception_ports_t __Request;
	typedef __Reply__host_swap_exception_ports_t Reply;

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

#ifdef	__MIG_check__Request__host_swap_exception_ports_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_swap_exception_ports_t__defined */

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
	mach_msg_type_number_t masksCnt;
	exception_handler_t old_handlerss[32];
	exception_behavior_t old_behaviors[32];
	thread_state_flavor_t old_flavors[32];

	__DeclareRcvRpc(416, "host_swap_exception_ports")
	__BeforeRcvRpc(416, "host_swap_exception_ports")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_swap_exception_ports_t__defined)
	check_result = __MIG_check__Request__host_swap_exception_ports_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_swap_exception_ports_t__defined) */

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


	masksCnt = 32;

	RetCode = host_swap_exception_ports(convert_port_to_host_priv(In0P->Head.msgh_request_port), In0P->exception_mask, In0P->new_port.name, In0P->behavior, In0P->new_flavor, OutP->masks, &masksCnt, old_handlerss, old_behaviors, old_flavors);
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
	__AfterRcvRpc(416, "host_swap_exception_ports")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__mach_vm_wire_t__defined)
#define __MIG_check__Request__mach_vm_wire_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_vm_wire_t(__attribute__((__unused__)) __Request__mach_vm_wire_t *In0P)
{

	typedef __Request__mach_vm_wire_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->task.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->task.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_vm_wire_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_vm_wire */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_vm_wire
#if	defined(LINTLIBRARY)
    (host_priv, task, address, size, desired_access)
	host_priv_t host_priv;
	vm_map_t task;
	mach_vm_address_t address;
	mach_vm_size_t size;
	vm_prot_t desired_access;
{ return mach_vm_wire(host_priv, task, address, size, desired_access); }
#else
(
	host_priv_t host_priv,
	vm_map_t task,
	mach_vm_address_t address,
	mach_vm_size_t size,
	vm_prot_t desired_access
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_vm_wire */
mig_internal novalue _Xmach_vm_wire
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t task;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_vm_address_t address;
		mach_vm_size_t size;
		vm_prot_t desired_access;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_vm_wire_t __Request;
	typedef __Reply__mach_vm_wire_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_vm_wire_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_vm_wire_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	vm_map_t task;

	__DeclareRcvRpc(418, "mach_vm_wire")
	__BeforeRcvRpc(418, "mach_vm_wire")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_vm_wire_t__defined)
	check_result = __MIG_check__Request__mach_vm_wire_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_vm_wire_t__defined) */

	task = convert_port_to_map(In0P->task.name);

	OutP->RetCode = mach_vm_wire(convert_port_to_host_priv(In0P->Head.msgh_request_port), task, In0P->address, In0P->size, In0P->desired_access);
	vm_map_deallocate(task);
#if	__MigKernelSpecificCode
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}

	if (IP_VALID((ipc_port_t)In0P->task.name))
		ipc_port_release_send((ipc_port_t)In0P->task.name);
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(418, "mach_vm_wire")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_processor_sets_t__defined)
#define __MIG_check__Request__host_processor_sets_t__defined

mig_internal kern_return_t __MIG_check__Request__host_processor_sets_t(__attribute__((__unused__)) __Request__host_processor_sets_t *In0P)
{

	typedef __Request__host_processor_sets_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_processor_sets_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_processor_sets */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_processor_sets
#if	defined(LINTLIBRARY)
    (host_priv, processor_sets, processor_setsCnt)
	host_priv_t host_priv;
	processor_set_name_array_t *processor_sets;
	mach_msg_type_number_t *processor_setsCnt;
{ return host_processor_sets(host_priv, processor_sets, processor_setsCnt); }
#else
(
	host_priv_t host_priv,
	processor_set_name_array_t *processor_sets,
	mach_msg_type_number_t *processor_setsCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_processor_sets */
mig_internal novalue _Xhost_processor_sets
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
	typedef __Request__host_processor_sets_t __Request;
	typedef __Reply__host_processor_sets_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_processor_sets_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_processor_sets_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_ool_ports_descriptor_t processor_setsTemplate = {
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
	const static mach_msg_ool_ports_descriptor_t processor_setsTemplate = {
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
	__DeclareRcvRpc(419, "host_processor_sets")
	__BeforeRcvRpc(419, "host_processor_sets")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_processor_sets_t__defined)
	check_result = __MIG_check__Request__host_processor_sets_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_processor_sets_t__defined) */

#if	UseStaticTemplates
	OutP->processor_sets = processor_setsTemplate;
#else	/* UseStaticTemplates */
#if	__MigKernelSpecificCode
	OutP->processor_sets.disposition = 17;
#else
	OutP->processor_sets.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->processor_sets.deallocate =  FALSE;
	OutP->processor_sets.type = MACH_MSG_OOL_PORTS_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = host_processor_sets(convert_port_to_host_priv(In0P->Head.msgh_request_port), (processor_set_name_array_t *)&(OutP->processor_sets.address), &OutP->processor_setsCnt);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->processor_sets.count = OutP->processor_setsCnt;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(419, "host_processor_sets")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_processor_set_priv_t__defined)
#define __MIG_check__Request__host_processor_set_priv_t__defined

mig_internal kern_return_t __MIG_check__Request__host_processor_set_priv_t(__attribute__((__unused__)) __Request__host_processor_set_priv_t *In0P)
{

	typedef __Request__host_processor_set_priv_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->set_name.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->set_name.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_processor_set_priv_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_processor_set_priv */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_processor_set_priv
#if	defined(LINTLIBRARY)
    (host_priv, set_name, set)
	host_priv_t host_priv;
	processor_set_name_t set_name;
	processor_set_t *set;
{ return host_processor_set_priv(host_priv, set_name, set); }
#else
(
	host_priv_t host_priv,
	processor_set_name_t set_name,
	processor_set_t *set
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_processor_set_priv */
mig_internal novalue _Xhost_processor_set_priv
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t set_name;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_processor_set_priv_t __Request;
	typedef __Reply__host_processor_set_priv_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_processor_set_priv_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_processor_set_priv_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t setTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t setTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	processor_set_name_t set_name;
	processor_set_t set;

	__DeclareRcvRpc(420, "host_processor_set_priv")
	__BeforeRcvRpc(420, "host_processor_set_priv")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_processor_set_priv_t__defined)
	check_result = __MIG_check__Request__host_processor_set_priv_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_processor_set_priv_t__defined) */

#if	UseStaticTemplates
	OutP->set = setTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->set.disposition = 17;
#else
	OutP->set.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->set.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	set_name = convert_port_to_pset_name(In0P->set_name.name);

	RetCode = host_processor_set_priv(convert_port_to_host_priv(In0P->Head.msgh_request_port), set_name, &set);
	pset_deallocate(set_name);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode

	if (IP_VALID((ipc_port_t)In0P->set_name.name))
		ipc_port_release_send((ipc_port_t)In0P->set_name.name);
#endif /* __MigKernelSpecificCode */
	OutP->set.name = (mach_port_t)convert_pset_to_port(set);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(420, "host_processor_set_priv")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__set_dp_control_port_t__defined)
#define __MIG_check__Request__set_dp_control_port_t__defined

mig_internal kern_return_t __MIG_check__Request__set_dp_control_port_t(__attribute__((__unused__)) __Request__set_dp_control_port_t *In0P)
{

	typedef __Request__set_dp_control_port_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->control_port.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->control_port.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__set_dp_control_port_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine set_dp_control_port */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t set_dp_control_port
#if	defined(LINTLIBRARY)
    (host, control_port)
	host_priv_t host;
	mach_port_t control_port;
{ return set_dp_control_port(host, control_port); }
#else
(
	host_priv_t host,
	mach_port_t control_port
);
#endif	/* defined(LINTLIBRARY) */

/* Routine set_dp_control_port */
mig_internal novalue _Xset_dp_control_port
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t control_port;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__set_dp_control_port_t __Request;
	typedef __Reply__set_dp_control_port_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__set_dp_control_port_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__set_dp_control_port_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(421, "set_dp_control_port")
	__BeforeRcvRpc(421, "set_dp_control_port")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__set_dp_control_port_t__defined)
	check_result = __MIG_check__Request__set_dp_control_port_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__set_dp_control_port_t__defined) */

	OutP->RetCode = set_dp_control_port(convert_port_to_host_priv(In0P->Head.msgh_request_port), In0P->control_port.name);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(421, "set_dp_control_port")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__get_dp_control_port_t__defined)
#define __MIG_check__Request__get_dp_control_port_t__defined

mig_internal kern_return_t __MIG_check__Request__get_dp_control_port_t(__attribute__((__unused__)) __Request__get_dp_control_port_t *In0P)
{

	typedef __Request__get_dp_control_port_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__get_dp_control_port_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine get_dp_control_port */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t get_dp_control_port
#if	defined(LINTLIBRARY)
    (host, contorl_port)
	host_priv_t host;
	mach_port_t *contorl_port;
{ return get_dp_control_port(host, contorl_port); }
#else
(
	host_priv_t host,
	mach_port_t *contorl_port
);
#endif	/* defined(LINTLIBRARY) */

/* Routine get_dp_control_port */
mig_internal novalue _Xget_dp_control_port
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
	typedef __Request__get_dp_control_port_t __Request;
	typedef __Reply__get_dp_control_port_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__get_dp_control_port_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__get_dp_control_port_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t contorl_portTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t contorl_portTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	__DeclareRcvRpc(422, "get_dp_control_port")
	__BeforeRcvRpc(422, "get_dp_control_port")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__get_dp_control_port_t__defined)
	check_result = __MIG_check__Request__get_dp_control_port_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__get_dp_control_port_t__defined) */

#if	UseStaticTemplates
	OutP->contorl_port = contorl_portTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->contorl_port.disposition = 17;
#else
	OutP->contorl_port.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->contorl_port.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = get_dp_control_port(convert_port_to_host_priv(In0P->Head.msgh_request_port), &OutP->contorl_port.name);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(422, "get_dp_control_port")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_set_UNDServer_t__defined)
#define __MIG_check__Request__host_set_UNDServer_t__defined

mig_internal kern_return_t __MIG_check__Request__host_set_UNDServer_t(__attribute__((__unused__)) __Request__host_set_UNDServer_t *In0P)
{

	typedef __Request__host_set_UNDServer_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->server.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->server.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_set_UNDServer_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_set_UNDServer */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_set_UNDServer
#if	defined(LINTLIBRARY)
    (host, server)
	host_priv_t host;
	UNDServerRef server;
{ return host_set_UNDServer(host, server); }
#else
(
	host_priv_t host,
	UNDServerRef server
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_set_UNDServer */
mig_internal novalue _Xhost_set_UNDServer
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t server;
		/* end of the kernel processed data */
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_set_UNDServer_t __Request;
	typedef __Reply__host_set_UNDServer_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_set_UNDServer_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_set_UNDServer_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(423, "host_set_UNDServer")
	__BeforeRcvRpc(423, "host_set_UNDServer")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_set_UNDServer_t__defined)
	check_result = __MIG_check__Request__host_set_UNDServer_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_set_UNDServer_t__defined) */

	OutP->RetCode = host_set_UNDServer(convert_port_to_host_priv(In0P->Head.msgh_request_port), In0P->server.name);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(423, "host_set_UNDServer")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__host_get_UNDServer_t__defined)
#define __MIG_check__Request__host_get_UNDServer_t__defined

mig_internal kern_return_t __MIG_check__Request__host_get_UNDServer_t(__attribute__((__unused__)) __Request__host_get_UNDServer_t *In0P)
{

	typedef __Request__host_get_UNDServer_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_get_UNDServer_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_get_UNDServer */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_get_UNDServer
#if	defined(LINTLIBRARY)
    (host, server)
	host_priv_t host;
	UNDServerRef *server;
{ return host_get_UNDServer(host, server); }
#else
(
	host_priv_t host,
	UNDServerRef *server
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_get_UNDServer */
mig_internal novalue _Xhost_get_UNDServer
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
	typedef __Request__host_get_UNDServer_t __Request;
	typedef __Reply__host_get_UNDServer_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_get_UNDServer_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_get_UNDServer_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t serverTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t serverTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	__DeclareRcvRpc(424, "host_get_UNDServer")
	__BeforeRcvRpc(424, "host_get_UNDServer")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_get_UNDServer_t__defined)
	check_result = __MIG_check__Request__host_get_UNDServer_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_get_UNDServer_t__defined) */

#if	UseStaticTemplates
	OutP->server = serverTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->server.disposition = 17;
#else
	OutP->server.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->server.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = host_get_UNDServer(convert_port_to_host_priv(In0P->Head.msgh_request_port), &OutP->server.name);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(424, "host_get_UNDServer")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__host_priv_subsystem__
#if !defined(__MIG_check__Request__kext_request_t__defined)
#define __MIG_check__Request__kext_request_t__defined

mig_internal kern_return_t __MIG_check__Request__kext_request_t(__attribute__((__unused__)) __Request__kext_request_t *In0P)
{

	typedef __Request__kext_request_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->request_data.type != MACH_MSG_OOL_DESCRIPTOR)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

#if __MigTypeCheck
	if (In0P->request_data.size != In0P->request_dataCnt)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__kext_request_t__defined) */
#endif /* __MIG_check__Request__host_priv_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine kext_request */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t kext_request
#if	defined(LINTLIBRARY)
    (host_priv, user_log_flags, request_data, request_dataCnt, response_data, response_dataCnt, log_data, log_dataCnt, op_result)
	host_priv_t host_priv;
	uint32_t user_log_flags;
	vm_offset_t request_data;
	mach_msg_type_number_t request_dataCnt;
	vm_offset_t *response_data;
	mach_msg_type_number_t *response_dataCnt;
	vm_offset_t *log_data;
	mach_msg_type_number_t *log_dataCnt;
	kern_return_t *op_result;
{ return kext_request(host_priv, user_log_flags, request_data, request_dataCnt, response_data, response_dataCnt, log_data, log_dataCnt, op_result); }
#else
(
	host_priv_t host_priv,
	uint32_t user_log_flags,
	vm_offset_t request_data,
	mach_msg_type_number_t request_dataCnt,
	vm_offset_t *response_data,
	mach_msg_type_number_t *response_dataCnt,
	vm_offset_t *log_data,
	mach_msg_type_number_t *log_dataCnt,
	kern_return_t *op_result
);
#endif	/* defined(LINTLIBRARY) */

/* Routine kext_request */
mig_internal novalue _Xkext_request
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t request_data;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		uint32_t user_log_flags;
		mach_msg_type_number_t request_dataCnt;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__kext_request_t __Request;
	typedef __Reply__kext_request_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__kext_request_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__kext_request_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t response_dataTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t log_dataTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t response_dataTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t log_dataTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	__DeclareRcvRpc(425, "kext_request")
	__BeforeRcvRpc(425, "kext_request")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__kext_request_t__defined)
	check_result = __MIG_check__Request__kext_request_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__kext_request_t__defined) */

#if	UseStaticTemplates
	OutP->response_data = response_dataTemplate;
#else	/* UseStaticTemplates */
	OutP->response_data.deallocate =  FALSE;
	OutP->response_data.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->response_data.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


#if	UseStaticTemplates
	OutP->log_data = log_dataTemplate;
#else	/* UseStaticTemplates */
	OutP->log_data.deallocate =  FALSE;
	OutP->log_data.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->log_data.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = kext_request(convert_port_to_host_priv(In0P->Head.msgh_request_port), In0P->user_log_flags, (vm_offset_t)(In0P->request_data.address), In0P->request_dataCnt, (vm_offset_t *)&(OutP->response_data.address), &OutP->response_dataCnt, (vm_offset_t *)&(OutP->log_data.address), &OutP->log_dataCnt, &OutP->op_result);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->response_data.size = OutP->response_dataCnt;

	OutP->log_data.size = OutP->log_dataCnt;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 2;
	__AfterRcvRpc(425, "kext_request")
}


#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
boolean_t host_priv_server(
		mach_msg_header_t *InHeadP,
		mach_msg_header_t *OutHeadP);

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
mig_routine_t host_priv_server_routine(
		mach_msg_header_t *InHeadP);


/* Description of this subsystem, for use in direct RPC */
extern const struct host_priv_subsystem host_priv_subsystem;
const struct host_priv_subsystem {
	mig_server_routine_t 	server;	/* Server routine */
	mach_msg_id_t	start;	/* Min routine number */
	mach_msg_id_t	end;	/* Max routine number + 1 */
	unsigned int	maxsize;	/* Max msg size */
	vm_address_t	reserved;	/* Reserved */
	struct routine_descriptor	/*Array of routine descriptors */
		routine[26];
} host_priv_subsystem = {
	host_priv_server_routine,
	400,
	426,
	(mach_msg_size_t)sizeof(union __ReplyUnion__host_priv_subsystem),
	(vm_address_t)0,
	{
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_get_boot_info, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_get_boot_info_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_reboot, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_reboot_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_priv_statistics, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_priv_statistics_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_default_memory_manager, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_default_memory_manager_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xvm_wire, 7, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__vm_wire_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xthread_wire, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__thread_wire_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xvm_allocate_cpm, 6, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__vm_allocate_cpm_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_processors, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_processors_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_get_clock_control, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_get_clock_control_t) },
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_get_special_port, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_get_special_port_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_set_special_port, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_set_special_port_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_set_exception_ports, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_set_exception_ports_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_get_exception_ports, 7, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_get_exception_ports_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_swap_exception_ports, 10, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_swap_exception_ports_t) },
		{0, 0, 0, 0, 0, 0},
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_vm_wire, 7, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_vm_wire_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_processor_sets, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_processor_sets_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_processor_set_priv, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_processor_set_priv_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xset_dp_control_port, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__set_dp_control_port_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xget_dp_control_port, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__get_dp_control_port_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_set_UNDServer, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_set_UNDServer_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_get_UNDServer, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_get_UNDServer_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xkext_request, 9, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__kext_request_t) },
	}
};

mig_external boolean_t host_priv_server
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

	if ((InHeadP->msgh_id > 425) || (InHeadP->msgh_id < 400) ||
	    ((routine = host_priv_subsystem.routine[InHeadP->msgh_id - 400].stub_routine) == 0)) {
		((mig_reply_error_t *)OutHeadP)->NDR = NDR_record;
		((mig_reply_error_t *)OutHeadP)->RetCode = MIG_BAD_ID;
		return FALSE;
	}
	(*routine) (InHeadP, OutHeadP);
	return TRUE;
}

mig_external mig_routine_t host_priv_server_routine
	(mach_msg_header_t *InHeadP)
{
	register int msgh_id;

	msgh_id = InHeadP->msgh_id - 400;

	if ((msgh_id > 25) || (msgh_id < 0))
		return 0;

	return host_priv_subsystem.routine[msgh_id].stub_routine;
}

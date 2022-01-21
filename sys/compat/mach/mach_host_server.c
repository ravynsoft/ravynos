/*
 * IDENTIFICATION:
 * stub generated Thu Jun 11 18:17:44 2015
 * with a MiG generated Thu Jun 11 16:16:11 PDT 2015 by kmacy@serenity
 * OPTIONS: 
 *	KernelServer
 */

/* Module mach_host */

#define	__MIG_check__Request__mach_host_subsystem__ 1

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

#ifndef __Request__mach_host_subsystem__defined
#define __Request__mach_host_subsystem__defined

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
	} __Request__host_info_t;
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
	} __Request__host_kernel_version_t;
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
	} __Request__host_page_size_t;
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
		mach_msg_port_descriptor_t pager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t internal;
		vm_size_t size;
		vm_prot_t permission;
	} __Request__mach_memory_object_memory_entry_t;
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
		processor_flavor_t flavor;
	} __Request__host_processor_info_t;
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
	} __Request__host_get_clock_service_t;
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
	} __Request__host_virtual_physical_table_info_t;
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
	} __Request__processor_set_default_t;
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
	} __Request__processor_set_create_t;
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
		mach_msg_port_descriptor_t pager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t internal;
		memory_object_size_t size;
		vm_prot_t permission;
	} __Request__mach_memory_object_memory_entry_64_t;
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
	} __Request__host_statistics_t;
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
		mach_msg_port_descriptor_t notify_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		host_flavor_t notify_type;
	} __Request__host_request_notification_t;
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
		mach_msg_type_number_t host_info64_outCnt;
	} __Request__host_statistics64_t;
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
	} __Request__mach_zone_info_t;
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
		mach_msg_type_number_t recipesCnt;
		uint8_t recipes[5120];
	} __Request__host_create_mach_voucher_t;
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
		mach_msg_port_descriptor_t attr_manager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_voucher_attr_value_handle_t default_value;
	} __Request__host_register_mach_voucher_attr_manager_t;
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
		mach_msg_port_descriptor_t attr_manager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_voucher_attr_value_handle_t default_value;
		mach_voucher_attr_key_t key;
	} __Request__host_register_well_known_mach_voucher_attr_manager_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__mach_host_subsystem__defined */

/* typedefs for all replies */

#ifndef __Reply__mach_host_subsystem__defined
#define __Reply__mach_host_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t host_info_outCnt;
		integer_t host_info_out[68];
	} __Reply__host_info_t;
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
		mach_msg_type_number_t kernel_versionOffset; /* MiG doesn't use it */
		mach_msg_type_number_t kernel_versionCnt;
		char kernel_version[512];
	} __Reply__host_kernel_version_t;
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
		vm_size_t out_page_size;
	} __Reply__host_page_size_t;
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
		mach_msg_port_descriptor_t entry_handle;
		/* end of the kernel processed data */
	} __Reply__mach_memory_object_memory_entry_t;
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
		mach_msg_ool_descriptor_t out_processor_info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		natural_t out_processor_count;
		mach_msg_type_number_t out_processor_infoCnt;
	} __Reply__host_processor_info_t;
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
		mach_msg_port_descriptor_t clock_serv;
		/* end of the kernel processed data */
	} __Reply__host_get_clock_service_t;
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
		mach_msg_ool_descriptor_t info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t infoCnt;
	} __Reply__host_virtual_physical_table_info_t;
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
		mach_msg_port_descriptor_t default_set;
		/* end of the kernel processed data */
	} __Reply__processor_set_default_t;
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
		mach_msg_port_descriptor_t new_name;
		/* end of the kernel processed data */
	} __Reply__processor_set_create_t;
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
		mach_msg_port_descriptor_t entry_handle;
		/* end of the kernel processed data */
	} __Reply__mach_memory_object_memory_entry_64_t;
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
	} __Reply__host_statistics_t;
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
	} __Reply__host_request_notification_t;
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
		mach_msg_type_number_t host_info64_outCnt;
		integer_t host_info64_out[256];
	} __Reply__host_statistics64_t;
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
	} __Reply__mach_zone_info_t;
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
	} __Reply__host_create_mach_voucher_t;
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
		mach_msg_port_descriptor_t new_attr_control;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_voucher_attr_key_t new_key;
	} __Reply__host_register_mach_voucher_attr_manager_t;
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
		mach_msg_port_descriptor_t new_attr_control;
		/* end of the kernel processed data */
	} __Reply__host_register_well_known_mach_voucher_attr_manager_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__mach_host_subsystem__defined */


/* union of all replies */

#ifndef __ReplyUnion__mach_host_subsystem__defined
#define __ReplyUnion__mach_host_subsystem__defined
union __ReplyUnion__mach_host_subsystem {
	__Reply__host_info_t Reply_host_info;
	__Reply__host_kernel_version_t Reply_host_kernel_version;
	__Reply__host_page_size_t Reply_host_page_size;
	__Reply__mach_memory_object_memory_entry_t Reply_mach_memory_object_memory_entry;
	__Reply__host_processor_info_t Reply_host_processor_info;
	__Reply__host_get_clock_service_t Reply_host_get_clock_service;
	__Reply__host_virtual_physical_table_info_t Reply_host_virtual_physical_table_info;
	__Reply__processor_set_default_t Reply_processor_set_default;
	__Reply__processor_set_create_t Reply_processor_set_create;
	__Reply__mach_memory_object_memory_entry_64_t Reply_mach_memory_object_memory_entry_64;
	__Reply__host_statistics_t Reply_host_statistics;
	__Reply__host_request_notification_t Reply_host_request_notification;
	__Reply__host_statistics64_t Reply_host_statistics64;
	__Reply__mach_zone_info_t Reply_mach_zone_info;
	__Reply__host_create_mach_voucher_t Reply_host_create_mach_voucher;
	__Reply__host_register_mach_voucher_attr_manager_t Reply_host_register_mach_voucher_attr_manager;
	__Reply__host_register_well_known_mach_voucher_attr_manager_t Reply_host_register_well_known_mach_voucher_attr_manager;
};
#endif /* __RequestUnion__mach_host_subsystem__defined */
/* Forward Declarations */


mig_internal novalue _Xhost_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_kernel_version
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_page_size
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_memory_object_memory_entry
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_processor_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_get_clock_service
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_virtual_physical_table_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xprocessor_set_default
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xprocessor_set_create
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_memory_object_memory_entry_64
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_statistics
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_request_notification
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_statistics64
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xmach_zone_info
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_create_mach_voucher
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_register_mach_voucher_attr_manager
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xhost_register_well_known_mach_voucher_attr_manager
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);


#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__host_info_t__defined)
#define __MIG_check__Request__host_info_t__defined

mig_internal kern_return_t __MIG_check__Request__host_info_t(__attribute__((__unused__)) __Request__host_info_t *In0P)
{

	typedef __Request__host_info_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_info_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_info
#if	defined(LINTLIBRARY)
    (host, flavor, host_info_out, host_info_outCnt)
	host_t host;
	host_flavor_t flavor;
	host_info_t host_info_out;
	mach_msg_type_number_t *host_info_outCnt;
{ return host_info(host, flavor, host_info_out, host_info_outCnt); }
#else
(
	host_t host,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_info */
mig_internal novalue _Xhost_info
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
	typedef __Request__host_info_t __Request;
	typedef __Reply__host_info_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_info_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(200, "host_info")
	__BeforeRcvRpc(200, "host_info")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_info_t__defined)
	check_result = __MIG_check__Request__host_info_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_info_t__defined) */

	OutP->host_info_outCnt = 68;
	if (In0P->host_info_outCnt < OutP->host_info_outCnt)
		OutP->host_info_outCnt = In0P->host_info_outCnt;

	OutP->RetCode = host_info(convert_port_to_host(In0P->Head.msgh_request_port), In0P->flavor, OutP->host_info_out, &OutP->host_info_outCnt);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;

	OutP->Head.msgh_size = (sizeof(Reply) - 272) + (_WALIGN_((4 * OutP->host_info_outCnt)));

	__AfterRcvRpc(200, "host_info")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__host_kernel_version_t__defined)
#define __MIG_check__Request__host_kernel_version_t__defined

mig_internal kern_return_t __MIG_check__Request__host_kernel_version_t(__attribute__((__unused__)) __Request__host_kernel_version_t *In0P)
{

	typedef __Request__host_kernel_version_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_kernel_version_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_kernel_version */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_kernel_version
#if	defined(LINTLIBRARY)
    (host, kernel_version)
	host_t host;
	kernel_version_t kernel_version;
{ return host_kernel_version(host, kernel_version); }
#else
(
	host_t host,
	kernel_version_t kernel_version
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_kernel_version */
mig_internal novalue _Xhost_kernel_version
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
	typedef __Request__host_kernel_version_t __Request;
	typedef __Reply__host_kernel_version_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_kernel_version_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_kernel_version_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(201, "host_kernel_version")
	__BeforeRcvRpc(201, "host_kernel_version")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_kernel_version_t__defined)
	check_result = __MIG_check__Request__host_kernel_version_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_kernel_version_t__defined) */

	OutP->RetCode = host_kernel_version(convert_port_to_host(In0P->Head.msgh_request_port), OutP->kernel_version);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;

#ifdef __LP64__
	{
		size_t strLength = strlen(OutP->kernel_version) + 1;
		if (strLength > 0xffffffff)
			MIG_RETURN_ERROR(OutP, MIG_BAD_ARGUMENTS);
		OutP->kernel_versionCnt = (mach_msg_type_number_t) strLength;
	}
#else
	OutP->kernel_versionCnt = (mach_msg_type_number_t) strlen(OutP->kernel_version) + 1;
#endif /* __LP64__ */
	OutP->Head.msgh_size = (sizeof(Reply) - 512) + (_WALIGN_((OutP->kernel_versionCnt + 3) & ~3));

	__AfterRcvRpc(201, "host_kernel_version")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__host_page_size_t__defined)
#define __MIG_check__Request__host_page_size_t__defined

mig_internal kern_return_t __MIG_check__Request__host_page_size_t(__attribute__((__unused__)) __Request__host_page_size_t *In0P)
{

	typedef __Request__host_page_size_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_page_size_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_page_size */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_page_size
#if	defined(LINTLIBRARY)
    (host, out_page_size)
	host_t host;
	vm_size_t *out_page_size;
{ return host_page_size(host, out_page_size); }
#else
(
	host_t host,
	vm_size_t *out_page_size
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_page_size */
mig_internal novalue _Xhost_page_size
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
	typedef __Request__host_page_size_t __Request;
	typedef __Reply__host_page_size_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_page_size_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_page_size_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(202, "host_page_size")
	__BeforeRcvRpc(202, "host_page_size")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_page_size_t__defined)
	check_result = __MIG_check__Request__host_page_size_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_page_size_t__defined) */

	OutP->RetCode = host_page_size(convert_port_to_host(In0P->Head.msgh_request_port), &OutP->out_page_size);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(202, "host_page_size")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__mach_memory_object_memory_entry_t__defined)
#define __MIG_check__Request__mach_memory_object_memory_entry_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_memory_object_memory_entry_t(__attribute__((__unused__)) __Request__mach_memory_object_memory_entry_t *In0P)
{

	typedef __Request__mach_memory_object_memory_entry_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->pager.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->pager.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_memory_object_memory_entry_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_memory_object_memory_entry */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_memory_object_memory_entry
#if	defined(LINTLIBRARY)
    (host, internal, size, permission, pager, entry_handle)
	host_t host;
	boolean_t internal;
	vm_size_t size;
	vm_prot_t permission;
	memory_object_t pager;
	mach_port_t *entry_handle;
{ return mach_memory_object_memory_entry(host, internal, size, permission, pager, entry_handle); }
#else
(
	host_t host,
	boolean_t internal,
	vm_size_t size,
	vm_prot_t permission,
	memory_object_t pager,
	mach_port_t *entry_handle
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_memory_object_memory_entry */
mig_internal novalue _Xmach_memory_object_memory_entry
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t pager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t internal;
		vm_size_t size;
		vm_prot_t permission;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_memory_object_memory_entry_t __Request;
	typedef __Reply__mach_memory_object_memory_entry_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_memory_object_memory_entry_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_memory_object_memory_entry_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t entry_handleTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t entry_handleTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	__DeclareRcvRpc(203, "mach_memory_object_memory_entry")
	__BeforeRcvRpc(203, "mach_memory_object_memory_entry")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_memory_object_memory_entry_t__defined)
	check_result = __MIG_check__Request__mach_memory_object_memory_entry_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_memory_object_memory_entry_t__defined) */

#if	UseStaticTemplates
	OutP->entry_handle = entry_handleTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->entry_handle.disposition = 17;
#else
	OutP->entry_handle.disposition = 17;
#endif /* __MigKernelSpecificCode */
	OutP->entry_handle.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = mach_memory_object_memory_entry(convert_port_to_host(In0P->Head.msgh_request_port), In0P->internal, In0P->size, In0P->permission, In0P->pager.name, &OutP->entry_handle.name);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(203, "mach_memory_object_memory_entry")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__host_processor_info_t__defined)
#define __MIG_check__Request__host_processor_info_t__defined

mig_internal kern_return_t __MIG_check__Request__host_processor_info_t(__attribute__((__unused__)) __Request__host_processor_info_t *In0P)
{

	typedef __Request__host_processor_info_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_processor_info_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_processor_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_processor_info
#if	defined(LINTLIBRARY)
    (host, flavor, out_processor_count, out_processor_info, out_processor_infoCnt)
	host_t host;
	processor_flavor_t flavor;
	natural_t *out_processor_count;
	processor_info_array_t *out_processor_info;
	mach_msg_type_number_t *out_processor_infoCnt;
{ return host_processor_info(host, flavor, out_processor_count, out_processor_info, out_processor_infoCnt); }
#else
(
	host_t host,
	processor_flavor_t flavor,
	natural_t *out_processor_count,
	processor_info_array_t *out_processor_info,
	mach_msg_type_number_t *out_processor_infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_processor_info */
mig_internal novalue _Xhost_processor_info
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
		processor_flavor_t flavor;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_processor_info_t __Request;
	typedef __Reply__host_processor_info_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_processor_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_processor_info_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t out_processor_infoTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t out_processor_infoTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = FALSE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	__DeclareRcvRpc(204, "host_processor_info")
	__BeforeRcvRpc(204, "host_processor_info")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_processor_info_t__defined)
	check_result = __MIG_check__Request__host_processor_info_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_processor_info_t__defined) */

#if	UseStaticTemplates
	OutP->out_processor_info = out_processor_infoTemplate;
#else	/* UseStaticTemplates */
	OutP->out_processor_info.deallocate =  FALSE;
	OutP->out_processor_info.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->out_processor_info.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = host_processor_info(convert_port_to_host(In0P->Head.msgh_request_port), In0P->flavor, &OutP->out_processor_count, (processor_info_array_t *)&(OutP->out_processor_info.address), &OutP->out_processor_infoCnt);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->out_processor_info.size = OutP->out_processor_infoCnt * 4;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(204, "host_processor_info")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__host_get_clock_service_t__defined)
#define __MIG_check__Request__host_get_clock_service_t__defined

mig_internal kern_return_t __MIG_check__Request__host_get_clock_service_t(__attribute__((__unused__)) __Request__host_get_clock_service_t *In0P)
{

	typedef __Request__host_get_clock_service_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_get_clock_service_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_get_clock_service */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_get_clock_service
#if	defined(LINTLIBRARY)
    (host, clock_id, clock_serv)
	host_t host;
	clock_id_t clock_id;
	clock_serv_t *clock_serv;
{ return host_get_clock_service(host, clock_id, clock_serv); }
#else
(
	host_t host,
	clock_id_t clock_id,
	clock_serv_t *clock_serv
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_get_clock_service */
mig_internal novalue _Xhost_get_clock_service
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
	typedef __Request__host_get_clock_service_t __Request;
	typedef __Reply__host_get_clock_service_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_get_clock_service_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_get_clock_service_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t clock_servTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t clock_servTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	clock_serv_t clock_serv;

	__DeclareRcvRpc(206, "host_get_clock_service")
	__BeforeRcvRpc(206, "host_get_clock_service")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_get_clock_service_t__defined)
	check_result = __MIG_check__Request__host_get_clock_service_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_get_clock_service_t__defined) */

#if	UseStaticTemplates
	OutP->clock_serv = clock_servTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->clock_serv.disposition = 17;
#else
	OutP->clock_serv.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->clock_serv.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = host_get_clock_service(convert_port_to_host(In0P->Head.msgh_request_port), In0P->clock_id, &clock_serv);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->clock_serv.name = (mach_port_t)convert_clock_to_port(clock_serv);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(206, "host_get_clock_service")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__host_virtual_physical_table_info_t__defined)
#define __MIG_check__Request__host_virtual_physical_table_info_t__defined

mig_internal kern_return_t __MIG_check__Request__host_virtual_physical_table_info_t(__attribute__((__unused__)) __Request__host_virtual_physical_table_info_t *In0P)
{

	typedef __Request__host_virtual_physical_table_info_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_virtual_physical_table_info_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_virtual_physical_table_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_virtual_physical_table_info
#if	defined(LINTLIBRARY)
    (host, info, infoCnt)
	host_t host;
	hash_info_bucket_array_t *info;
	mach_msg_type_number_t *infoCnt;
{ return host_virtual_physical_table_info(host, info, infoCnt); }
#else
(
	host_t host,
	hash_info_bucket_array_t *info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_virtual_physical_table_info */
mig_internal novalue _Xhost_virtual_physical_table_info
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
	typedef __Request__host_virtual_physical_table_info_t __Request;
	typedef __Reply__host_virtual_physical_table_info_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_virtual_physical_table_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_virtual_physical_table_info_t__defined */

#if	__MigKernelSpecificCode
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
	__DeclareRcvRpc(209, "host_virtual_physical_table_info")
	__BeforeRcvRpc(209, "host_virtual_physical_table_info")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_virtual_physical_table_info_t__defined)
	check_result = __MIG_check__Request__host_virtual_physical_table_info_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_virtual_physical_table_info_t__defined) */

#if	UseStaticTemplates
	OutP->info = infoTemplate;
#else	/* UseStaticTemplates */
	OutP->info.deallocate =  TRUE;
	OutP->info.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->info.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	OutP->infoCnt = 0;

	RetCode = host_virtual_physical_table_info(convert_port_to_host(In0P->Head.msgh_request_port), (hash_info_bucket_array_t *)&(OutP->info.address), &OutP->infoCnt);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->info.size = OutP->infoCnt * 4;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(209, "host_virtual_physical_table_info")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__processor_set_default_t__defined)
#define __MIG_check__Request__processor_set_default_t__defined

mig_internal kern_return_t __MIG_check__Request__processor_set_default_t(__attribute__((__unused__)) __Request__processor_set_default_t *In0P)
{

	typedef __Request__processor_set_default_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__processor_set_default_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine processor_set_default */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t processor_set_default
#if	defined(LINTLIBRARY)
    (host, default_set)
	host_t host;
	processor_set_name_t *default_set;
{ return processor_set_default(host, default_set); }
#else
(
	host_t host,
	processor_set_name_t *default_set
);
#endif	/* defined(LINTLIBRARY) */

/* Routine processor_set_default */
mig_internal novalue _Xprocessor_set_default
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
	typedef __Request__processor_set_default_t __Request;
	typedef __Reply__processor_set_default_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__processor_set_default_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__processor_set_default_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t default_setTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t default_setTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	processor_set_name_t default_set;

	__DeclareRcvRpc(213, "processor_set_default")
	__BeforeRcvRpc(213, "processor_set_default")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__processor_set_default_t__defined)
	check_result = __MIG_check__Request__processor_set_default_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__processor_set_default_t__defined) */

#if	UseStaticTemplates
	OutP->default_set = default_setTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->default_set.disposition = 17;
#else
	OutP->default_set.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->default_set.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = processor_set_default(convert_port_to_host(In0P->Head.msgh_request_port), &default_set);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->default_set.name = (mach_port_t)convert_pset_name_to_port(default_set);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(213, "processor_set_default")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__processor_set_create_t__defined)
#define __MIG_check__Request__processor_set_create_t__defined

mig_internal kern_return_t __MIG_check__Request__processor_set_create_t(__attribute__((__unused__)) __Request__processor_set_create_t *In0P)
{

	typedef __Request__processor_set_create_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__processor_set_create_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine processor_set_create */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t processor_set_create
#if	defined(LINTLIBRARY)
    (host, new_set, new_name)
	host_t host;
	processor_set_t *new_set;
	processor_set_name_t *new_name;
{ return processor_set_create(host, new_set, new_name); }
#else
(
	host_t host,
	processor_set_t *new_set,
	processor_set_name_t *new_name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine processor_set_create */
mig_internal novalue _Xprocessor_set_create
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
	typedef __Request__processor_set_create_t __Request;
	typedef __Reply__processor_set_create_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__processor_set_create_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__processor_set_create_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t new_setTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t new_nameTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t new_setTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t new_nameTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	processor_set_t new_set;
	processor_set_name_t new_name;

	__DeclareRcvRpc(214, "processor_set_create")
	__BeforeRcvRpc(214, "processor_set_create")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__processor_set_create_t__defined)
	check_result = __MIG_check__Request__processor_set_create_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__processor_set_create_t__defined) */

#if	UseStaticTemplates
	OutP->new_set = new_setTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->new_set.disposition = 17;
#else
	OutP->new_set.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->new_set.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


#if	UseStaticTemplates
	OutP->new_name = new_nameTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->new_name.disposition = 17;
#else
	OutP->new_name.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->new_name.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = processor_set_create(convert_port_to_host(In0P->Head.msgh_request_port), &new_set, &new_name);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->new_set.name = (mach_port_t)convert_pset_to_port(new_set);

	OutP->new_name.name = (mach_port_t)convert_pset_name_to_port(new_name);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 2;
	__AfterRcvRpc(214, "processor_set_create")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__mach_memory_object_memory_entry_64_t__defined)
#define __MIG_check__Request__mach_memory_object_memory_entry_64_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_memory_object_memory_entry_64_t(__attribute__((__unused__)) __Request__mach_memory_object_memory_entry_64_t *In0P)
{

	typedef __Request__mach_memory_object_memory_entry_64_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->pager.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->pager.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_memory_object_memory_entry_64_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_memory_object_memory_entry_64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_memory_object_memory_entry_64
#if	defined(LINTLIBRARY)
    (host, internal, size, permission, pager, entry_handle)
	host_t host;
	boolean_t internal;
	memory_object_size_t size;
	vm_prot_t permission;
	memory_object_t pager;
	mach_port_t *entry_handle;
{ return mach_memory_object_memory_entry_64(host, internal, size, permission, pager, entry_handle); }
#else
(
	host_t host,
	boolean_t internal,
	memory_object_size_t size,
	vm_prot_t permission,
	memory_object_t pager,
	mach_port_t *entry_handle
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_memory_object_memory_entry_64 */
mig_internal novalue _Xmach_memory_object_memory_entry_64
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t pager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t internal;
		memory_object_size_t size;
		vm_prot_t permission;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__mach_memory_object_memory_entry_64_t __Request;
	typedef __Reply__mach_memory_object_memory_entry_64_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_memory_object_memory_entry_64_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_memory_object_memory_entry_64_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t entry_handleTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t entry_handleTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	__DeclareRcvRpc(215, "mach_memory_object_memory_entry_64")
	__BeforeRcvRpc(215, "mach_memory_object_memory_entry_64")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_memory_object_memory_entry_64_t__defined)
	check_result = __MIG_check__Request__mach_memory_object_memory_entry_64_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_memory_object_memory_entry_64_t__defined) */

#if	UseStaticTemplates
	OutP->entry_handle = entry_handleTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->entry_handle.disposition = 17;
#else
	OutP->entry_handle.disposition = 17;
#endif /* __MigKernelSpecificCode */
	OutP->entry_handle.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = mach_memory_object_memory_entry_64(convert_port_to_host(In0P->Head.msgh_request_port), In0P->internal, In0P->size, In0P->permission, In0P->pager.name, &OutP->entry_handle.name);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(215, "mach_memory_object_memory_entry_64")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__host_statistics_t__defined)
#define __MIG_check__Request__host_statistics_t__defined

mig_internal kern_return_t __MIG_check__Request__host_statistics_t(__attribute__((__unused__)) __Request__host_statistics_t *In0P)
{

	typedef __Request__host_statistics_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_statistics_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_statistics */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_statistics
#if	defined(LINTLIBRARY)
    (host_priv, flavor, host_info_out, host_info_outCnt)
	host_t host_priv;
	host_flavor_t flavor;
	host_info_t host_info_out;
	mach_msg_type_number_t *host_info_outCnt;
{ return host_statistics(host_priv, flavor, host_info_out, host_info_outCnt); }
#else
(
	host_t host_priv,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_statistics */
mig_internal novalue _Xhost_statistics
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
	typedef __Request__host_statistics_t __Request;
	typedef __Reply__host_statistics_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_statistics_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_statistics_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(216, "host_statistics")
	__BeforeRcvRpc(216, "host_statistics")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_statistics_t__defined)
	check_result = __MIG_check__Request__host_statistics_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_statistics_t__defined) */

	OutP->host_info_outCnt = 68;
	if (In0P->host_info_outCnt < OutP->host_info_outCnt)
		OutP->host_info_outCnt = In0P->host_info_outCnt;

	OutP->RetCode = host_statistics(convert_port_to_host(In0P->Head.msgh_request_port), In0P->flavor, OutP->host_info_out, &OutP->host_info_outCnt);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;

	OutP->Head.msgh_size = (sizeof(Reply) - 272) + (_WALIGN_((4 * OutP->host_info_outCnt)));

	__AfterRcvRpc(216, "host_statistics")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__host_request_notification_t__defined)
#define __MIG_check__Request__host_request_notification_t__defined

mig_internal kern_return_t __MIG_check__Request__host_request_notification_t(__attribute__((__unused__)) __Request__host_request_notification_t *In0P)
{

	typedef __Request__host_request_notification_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->notify_port.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->notify_port.disposition != 18)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_request_notification_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_request_notification */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_request_notification
#if	defined(LINTLIBRARY)
    (host, notify_type, notify_port)
	host_t host;
	host_flavor_t notify_type;
	mach_port_t notify_port;
{ return host_request_notification(host, notify_type, notify_port); }
#else
(
	host_t host,
	host_flavor_t notify_type,
	mach_port_t notify_port
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_request_notification */
mig_internal novalue _Xhost_request_notification
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t notify_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		host_flavor_t notify_type;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_request_notification_t __Request;
	typedef __Reply__host_request_notification_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_request_notification_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_request_notification_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(217, "host_request_notification")
	__BeforeRcvRpc(217, "host_request_notification")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_request_notification_t__defined)
	check_result = __MIG_check__Request__host_request_notification_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_request_notification_t__defined) */

	OutP->RetCode = host_request_notification(convert_port_to_host(In0P->Head.msgh_request_port), In0P->notify_type, In0P->notify_port.name);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(217, "host_request_notification")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__host_statistics64_t__defined)
#define __MIG_check__Request__host_statistics64_t__defined

mig_internal kern_return_t __MIG_check__Request__host_statistics64_t(__attribute__((__unused__)) __Request__host_statistics64_t *In0P)
{

	typedef __Request__host_statistics64_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_statistics64_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_statistics64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_statistics64
#if	defined(LINTLIBRARY)
    (host_priv, flavor, host_info64_out, host_info64_outCnt)
	host_t host_priv;
	host_flavor_t flavor;
	host_info64_t host_info64_out;
	mach_msg_type_number_t *host_info64_outCnt;
{ return host_statistics64(host_priv, flavor, host_info64_out, host_info64_outCnt); }
#else
(
	host_t host_priv,
	host_flavor_t flavor,
	host_info64_t host_info64_out,
	mach_msg_type_number_t *host_info64_outCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_statistics64 */
mig_internal novalue _Xhost_statistics64
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
		mach_msg_type_number_t host_info64_outCnt;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_statistics64_t __Request;
	typedef __Reply__host_statistics64_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_statistics64_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_statistics64_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(219, "host_statistics64")
	__BeforeRcvRpc(219, "host_statistics64")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_statistics64_t__defined)
	check_result = __MIG_check__Request__host_statistics64_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_statistics64_t__defined) */

	OutP->host_info64_outCnt = 256;
	if (In0P->host_info64_outCnt < OutP->host_info64_outCnt)
		OutP->host_info64_outCnt = In0P->host_info64_outCnt;

	OutP->RetCode = host_statistics64(convert_port_to_host(In0P->Head.msgh_request_port), In0P->flavor, OutP->host_info64_out, &OutP->host_info64_outCnt);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;

	OutP->Head.msgh_size = (sizeof(Reply) - 1024) + (_WALIGN_((4 * OutP->host_info64_outCnt)));

	__AfterRcvRpc(219, "host_statistics64")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__mach_zone_info_t__defined)
#define __MIG_check__Request__mach_zone_info_t__defined

mig_internal kern_return_t __MIG_check__Request__mach_zone_info_t(__attribute__((__unused__)) __Request__mach_zone_info_t *In0P)
{

	typedef __Request__mach_zone_info_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__mach_zone_info_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine mach_zone_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_zone_info
#if	defined(LINTLIBRARY)
    (host, names, namesCnt, info, infoCnt)
	host_priv_t host;
	mach_zone_name_array_t *names;
	mach_msg_type_number_t *namesCnt;
	mach_zone_info_array_t *info;
	mach_msg_type_number_t *infoCnt;
{ return mach_zone_info(host, names, namesCnt, info, infoCnt); }
#else
(
	host_priv_t host,
	mach_zone_name_array_t *names,
	mach_msg_type_number_t *namesCnt,
	mach_zone_info_array_t *info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_zone_info */
mig_internal novalue _Xmach_zone_info
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
	typedef __Request__mach_zone_info_t __Request;
	typedef __Reply__mach_zone_info_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__mach_zone_info_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__mach_zone_info_t__defined */

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
	__DeclareRcvRpc(220, "mach_zone_info")
	__BeforeRcvRpc(220, "mach_zone_info")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__mach_zone_info_t__defined)
	check_result = __MIG_check__Request__mach_zone_info_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__mach_zone_info_t__defined) */

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


	OutP->namesCnt = 0;

	OutP->infoCnt = 0;

	RetCode = mach_zone_info(convert_port_to_host_priv(In0P->Head.msgh_request_port), (mach_zone_name_array_t *)&(OutP->names.address), &OutP->namesCnt, (mach_zone_info_array_t *)&(OutP->info.address), &OutP->infoCnt);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->names.size = OutP->namesCnt * 80;

	OutP->info.size = OutP->infoCnt * 64;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 2;
	__AfterRcvRpc(220, "mach_zone_info")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__host_create_mach_voucher_t__defined)
#define __MIG_check__Request__host_create_mach_voucher_t__defined

mig_internal kern_return_t __MIG_check__Request__host_create_mach_voucher_t(__attribute__((__unused__)) __Request__host_create_mach_voucher_t *In0P)
{

	typedef __Request__host_create_mach_voucher_t __Request;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	msgh_size = In0P->Head.msgh_size;
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (msgh_size < (sizeof(__Request) - 5120)) ||  (msgh_size > (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if defined(__NDR_convert__int_rep__Request__host_create_mach_voucher_t__recipesCnt__defined)
	if (In0P->NDR.int_rep != NDR_record.int_rep)
		__NDR_convert__int_rep__Request__host_create_mach_voucher_t__recipesCnt(&In0P->recipesCnt, In0P->NDR.int_rep);
#endif	/* __NDR_convert__int_rep__Request__host_create_mach_voucher_t__recipesCnt__defined */
#if	__MigTypeCheck
	if ( In0P->recipesCnt > 5120 )
		return MIG_BAD_ARGUMENTS;
	if (((msgh_size - (sizeof(__Request) - 5120)) < In0P->recipesCnt) ||
	    (msgh_size != (sizeof(__Request) - 5120) + _WALIGN_(In0P->recipesCnt)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_create_mach_voucher_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_create_mach_voucher */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_create_mach_voucher
#if	defined(LINTLIBRARY)
    (host, recipes, recipesCnt, voucher)
	host_t host;
	mach_voucher_attr_raw_recipe_array_t recipes;
	mach_msg_type_number_t recipesCnt;
	ipc_voucher_t *voucher;
{ return host_create_mach_voucher(host, recipes, recipesCnt, voucher); }
#else
(
	host_t host,
	mach_voucher_attr_raw_recipe_array_t recipes,
	mach_msg_type_number_t recipesCnt,
	ipc_voucher_t *voucher
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_create_mach_voucher */
mig_internal novalue _Xhost_create_mach_voucher
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
		mach_msg_type_number_t recipesCnt;
		uint8_t recipes[5120];
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_create_mach_voucher_t __Request;
	typedef __Reply__host_create_mach_voucher_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_create_mach_voucher_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_create_mach_voucher_t__defined */

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
	ipc_voucher_t voucher;

	__DeclareRcvRpc(222, "host_create_mach_voucher")
	__BeforeRcvRpc(222, "host_create_mach_voucher")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_create_mach_voucher_t__defined)
	check_result = __MIG_check__Request__host_create_mach_voucher_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_create_mach_voucher_t__defined) */

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


	RetCode = host_create_mach_voucher(convert_port_to_host(In0P->Head.msgh_request_port), In0P->recipes, In0P->recipesCnt, &voucher);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->voucher.name = (mach_port_t)convert_voucher_to_port(voucher);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(222, "host_create_mach_voucher")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__host_register_mach_voucher_attr_manager_t__defined)
#define __MIG_check__Request__host_register_mach_voucher_attr_manager_t__defined

mig_internal kern_return_t __MIG_check__Request__host_register_mach_voucher_attr_manager_t(__attribute__((__unused__)) __Request__host_register_mach_voucher_attr_manager_t *In0P)
{

	typedef __Request__host_register_mach_voucher_attr_manager_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->attr_manager.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->attr_manager.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_register_mach_voucher_attr_manager_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_register_mach_voucher_attr_manager */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_register_mach_voucher_attr_manager
#if	defined(LINTLIBRARY)
    (host, attr_manager, default_value, new_key, new_attr_control)
	host_t host;
	mach_voucher_attr_manager_t attr_manager;
	mach_voucher_attr_value_handle_t default_value;
	mach_voucher_attr_key_t *new_key;
	ipc_voucher_attr_control_t *new_attr_control;
{ return host_register_mach_voucher_attr_manager(host, attr_manager, default_value, new_key, new_attr_control); }
#else
(
	host_t host,
	mach_voucher_attr_manager_t attr_manager,
	mach_voucher_attr_value_handle_t default_value,
	mach_voucher_attr_key_t *new_key,
	ipc_voucher_attr_control_t *new_attr_control
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_register_mach_voucher_attr_manager */
mig_internal novalue _Xhost_register_mach_voucher_attr_manager
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t attr_manager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_voucher_attr_value_handle_t default_value;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_register_mach_voucher_attr_manager_t __Request;
	typedef __Reply__host_register_mach_voucher_attr_manager_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_register_mach_voucher_attr_manager_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_register_mach_voucher_attr_manager_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t new_attr_controlTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t new_attr_controlTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	ipc_voucher_attr_control_t new_attr_control;

	__DeclareRcvRpc(223, "host_register_mach_voucher_attr_manager")
	__BeforeRcvRpc(223, "host_register_mach_voucher_attr_manager")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_register_mach_voucher_attr_manager_t__defined)
	check_result = __MIG_check__Request__host_register_mach_voucher_attr_manager_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_register_mach_voucher_attr_manager_t__defined) */

#if	UseStaticTemplates
	OutP->new_attr_control = new_attr_controlTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->new_attr_control.disposition = 17;
#else
	OutP->new_attr_control.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->new_attr_control.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = host_register_mach_voucher_attr_manager(convert_port_to_host(In0P->Head.msgh_request_port), In0P->attr_manager.name, In0P->default_value, &OutP->new_key, &new_attr_control);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->new_attr_control.name = (mach_port_t)convert_voucher_attr_control_to_port(new_attr_control);


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(223, "host_register_mach_voucher_attr_manager")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__mach_host_subsystem__
#if !defined(__MIG_check__Request__host_register_well_known_mach_voucher_attr_manager_t__defined)
#define __MIG_check__Request__host_register_well_known_mach_voucher_attr_manager_t__defined

mig_internal kern_return_t __MIG_check__Request__host_register_well_known_mach_voucher_attr_manager_t(__attribute__((__unused__)) __Request__host_register_well_known_mach_voucher_attr_manager_t *In0P)
{

	typedef __Request__host_register_well_known_mach_voucher_attr_manager_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->attr_manager.type != MACH_MSG_PORT_DESCRIPTOR ||
	    In0P->attr_manager.disposition != 17)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__host_register_well_known_mach_voucher_attr_manager_t__defined) */
#endif /* __MIG_check__Request__mach_host_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine host_register_well_known_mach_voucher_attr_manager */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_register_well_known_mach_voucher_attr_manager
#if	defined(LINTLIBRARY)
    (host, attr_manager, default_value, key, new_attr_control)
	host_t host;
	mach_voucher_attr_manager_t attr_manager;
	mach_voucher_attr_value_handle_t default_value;
	mach_voucher_attr_key_t key;
	ipc_voucher_attr_control_t *new_attr_control;
{ return host_register_well_known_mach_voucher_attr_manager(host, attr_manager, default_value, key, new_attr_control); }
#else
(
	host_t host,
	mach_voucher_attr_manager_t attr_manager,
	mach_voucher_attr_value_handle_t default_value,
	mach_voucher_attr_key_t key,
	ipc_voucher_attr_control_t *new_attr_control
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_register_well_known_mach_voucher_attr_manager */
mig_internal novalue _Xhost_register_well_known_mach_voucher_attr_manager
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t attr_manager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_voucher_attr_value_handle_t default_value;
		mach_voucher_attr_key_t key;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__host_register_well_known_mach_voucher_attr_manager_t __Request;
	typedef __Reply__host_register_well_known_mach_voucher_attr_manager_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__host_register_well_known_mach_voucher_attr_manager_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__host_register_well_known_mach_voucher_attr_manager_t__defined */

#if	__MigKernelSpecificCode
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t new_attr_controlTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#else
#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t new_attr_controlTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 19,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#endif /* __MigKernelSpecificCode */
	kern_return_t RetCode;
	ipc_voucher_attr_control_t new_attr_control;

	__DeclareRcvRpc(224, "host_register_well_known_mach_voucher_attr_manager")
	__BeforeRcvRpc(224, "host_register_well_known_mach_voucher_attr_manager")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__host_register_well_known_mach_voucher_attr_manager_t__defined)
	check_result = __MIG_check__Request__host_register_well_known_mach_voucher_attr_manager_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__host_register_well_known_mach_voucher_attr_manager_t__defined) */

#if	UseStaticTemplates
	OutP->new_attr_control = new_attr_controlTemplate;
#else	/* UseStaticTemplates */
#if __MigKernelSpecificCode
	OutP->new_attr_control.disposition = 17;
#else
	OutP->new_attr_control.disposition = 19;
#endif /* __MigKernelSpecificCode */
	OutP->new_attr_control.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = host_register_well_known_mach_voucher_attr_manager(convert_port_to_host(In0P->Head.msgh_request_port), In0P->attr_manager.name, In0P->default_value, In0P->key, &new_attr_control);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */
	OutP->new_attr_control.name = (mach_port_t)convert_voucher_attr_control_to_port(new_attr_control);


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(224, "host_register_well_known_mach_voucher_attr_manager")
}


#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
boolean_t mach_host_server(
		mach_msg_header_t *InHeadP,
		mach_msg_header_t *OutHeadP);

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
mig_routine_t mach_host_server_routine(
		mach_msg_header_t *InHeadP);


/* Description of this subsystem, for use in direct RPC */
extern const struct mach_host_subsystem mach_host_subsystem;
const struct mach_host_subsystem {
	mig_server_routine_t 	server;	/* Server routine */
	mach_msg_id_t	start;	/* Min routine number */
	mach_msg_id_t	end;	/* Max routine number + 1 */
	unsigned int	maxsize;	/* Max msg size */
	vm_address_t	reserved;	/* Reserved */
	struct routine_descriptor	/*Array of routine descriptors */
		routine[25];
} mach_host_subsystem = {
	mach_host_server_routine,
	200,
	225,
	(mach_msg_size_t)sizeof(union __ReplyUnion__mach_host_subsystem),
	(vm_address_t)0,
	{
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_info, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_info_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_kernel_version, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_kernel_version_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_page_size, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_page_size_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_memory_object_memory_entry, 7, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_memory_object_memory_entry_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_processor_info, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_processor_info_t) },
		{0, 0, 0, 0, 0, 0},
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_get_clock_service, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_get_clock_service_t) },
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_virtual_physical_table_info, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_virtual_physical_table_info_t) },
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xprocessor_set_default, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__processor_set_default_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xprocessor_set_create, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__processor_set_create_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_memory_object_memory_entry_64, 7, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_memory_object_memory_entry_64_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_statistics, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_statistics_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_request_notification, 3, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_request_notification_t) },
		{0, 0, 0, 0, 0, 0},
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_statistics64, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_statistics64_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xmach_zone_info, 5, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__mach_zone_info_t) },
		{0, 0, 0, 0, 0, 0},
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_create_mach_voucher, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_create_mach_voucher_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_register_mach_voucher_attr_manager, 6, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_register_mach_voucher_attr_manager_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xhost_register_well_known_mach_voucher_attr_manager, 6, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__host_register_well_known_mach_voucher_attr_manager_t) },
	}
};

mig_external boolean_t mach_host_server
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

	if ((InHeadP->msgh_id > 224) || (InHeadP->msgh_id < 200) ||
	    ((routine = mach_host_subsystem.routine[InHeadP->msgh_id - 200].stub_routine) == 0)) {
		((mig_reply_error_t *)OutHeadP)->NDR = NDR_record;
		((mig_reply_error_t *)OutHeadP)->RetCode = MIG_BAD_ID;
		return FALSE;
	}
	(*routine) (InHeadP, OutHeadP);
	return TRUE;
}

mig_external mig_routine_t mach_host_server_routine
	(mach_msg_header_t *InHeadP)
{
	register int msgh_id;

	msgh_id = InHeadP->msgh_id - 200;

	if ((msgh_id > 24) || (msgh_id < 0))
		return 0;

	return mach_host_subsystem.routine[msgh_id].stub_routine;
}

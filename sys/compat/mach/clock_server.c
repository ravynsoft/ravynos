/*
 * IDENTIFICATION:
 * stub generated Thu Jun 11 18:17:44 2015
 * with a MiG generated Thu Jun 11 16:16:11 PDT 2015 by kmacy@serenity
 * OPTIONS: 
 *	KernelServer
 */

/* Module clock */

#define	__MIG_check__Request__clock_subsystem__ 1

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

#ifndef __Request__clock_subsystem__defined
#define __Request__clock_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
	} __Request__clock_get_time_t;
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
		clock_flavor_t flavor;
		mach_msg_type_number_t clock_attrCnt;
	} __Request__clock_get_attributes_t;
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
		mach_msg_port_descriptor_t alarm_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		alarm_type_t alarm_type;
		mach_timespec_t alarm_time;
	} __Request__clock_alarm_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__clock_subsystem__defined */

/* typedefs for all replies */

#ifndef __Reply__clock_subsystem__defined
#define __Reply__clock_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_timespec_t cur_time;
	} __Reply__clock_get_time_t;
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
		mach_msg_type_number_t clock_attrCnt;
		int clock_attr[1];
	} __Reply__clock_get_attributes_t;
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
	} __Reply__clock_alarm_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__clock_subsystem__defined */


/* union of all replies */

#ifndef __ReplyUnion__clock_subsystem__defined
#define __ReplyUnion__clock_subsystem__defined
union __ReplyUnion__clock_subsystem {
	__Reply__clock_get_time_t Reply_clock_get_time;
	__Reply__clock_get_attributes_t Reply_clock_get_attributes;
	__Reply__clock_alarm_t Reply_clock_alarm;
};
#endif /* __RequestUnion__clock_subsystem__defined */
/* Forward Declarations */


mig_internal novalue _Xclock_get_time
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xclock_get_attributes
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _Xclock_alarm
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);


#if ( __MigTypeCheck )
#if __MIG_check__Request__clock_subsystem__
#if !defined(__MIG_check__Request__clock_get_time_t__defined)
#define __MIG_check__Request__clock_get_time_t__defined

mig_internal kern_return_t __MIG_check__Request__clock_get_time_t(__attribute__((__unused__)) __Request__clock_get_time_t *In0P)
{

	typedef __Request__clock_get_time_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__clock_get_time_t__defined) */
#endif /* __MIG_check__Request__clock_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine clock_get_time */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t clock_get_time
#if	defined(LINTLIBRARY)
    (clock_serv, cur_time)
	clock_serv_t clock_serv;
	mach_timespec_t *cur_time;
{ return clock_get_time(clock_serv, cur_time); }
#else
(
	clock_serv_t clock_serv,
	mach_timespec_t *cur_time
);
#endif	/* defined(LINTLIBRARY) */

/* Routine clock_get_time */
mig_internal novalue _Xclock_get_time
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
	typedef __Request__clock_get_time_t __Request;
	typedef __Reply__clock_get_time_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__clock_get_time_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__clock_get_time_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(1000, "clock_get_time")
	__BeforeRcvRpc(1000, "clock_get_time")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__clock_get_time_t__defined)
	check_result = __MIG_check__Request__clock_get_time_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__clock_get_time_t__defined) */

	OutP->RetCode = clock_get_time(convert_port_to_clock(In0P->Head.msgh_request_port), &OutP->cur_time);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(1000, "clock_get_time")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__clock_subsystem__
#if !defined(__MIG_check__Request__clock_get_attributes_t__defined)
#define __MIG_check__Request__clock_get_attributes_t__defined

mig_internal kern_return_t __MIG_check__Request__clock_get_attributes_t(__attribute__((__unused__)) __Request__clock_get_attributes_t *In0P)
{

	typedef __Request__clock_get_attributes_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__clock_get_attributes_t__defined) */
#endif /* __MIG_check__Request__clock_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine clock_get_attributes */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t clock_get_attributes
#if	defined(LINTLIBRARY)
    (clock_serv, flavor, clock_attr, clock_attrCnt)
	clock_serv_t clock_serv;
	clock_flavor_t flavor;
	clock_attr_t clock_attr;
	mach_msg_type_number_t *clock_attrCnt;
{ return clock_get_attributes(clock_serv, flavor, clock_attr, clock_attrCnt); }
#else
(
	clock_serv_t clock_serv,
	clock_flavor_t flavor,
	clock_attr_t clock_attr,
	mach_msg_type_number_t *clock_attrCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine clock_get_attributes */
mig_internal novalue _Xclock_get_attributes
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
		clock_flavor_t flavor;
		mach_msg_type_number_t clock_attrCnt;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__clock_get_attributes_t __Request;
	typedef __Reply__clock_get_attributes_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__clock_get_attributes_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__clock_get_attributes_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(1001, "clock_get_attributes")
	__BeforeRcvRpc(1001, "clock_get_attributes")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__clock_get_attributes_t__defined)
	check_result = __MIG_check__Request__clock_get_attributes_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__clock_get_attributes_t__defined) */

	OutP->clock_attrCnt = 1;
	if (In0P->clock_attrCnt < OutP->clock_attrCnt)
		OutP->clock_attrCnt = In0P->clock_attrCnt;

	OutP->RetCode = clock_get_attributes(convert_port_to_clock(In0P->Head.msgh_request_port), In0P->flavor, OutP->clock_attr, &OutP->clock_attrCnt);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;

	OutP->Head.msgh_size = (sizeof(Reply) - 4) + (_WALIGN_((4 * OutP->clock_attrCnt)));

	__AfterRcvRpc(1001, "clock_get_attributes")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__clock_subsystem__
#if !defined(__MIG_check__Request__clock_alarm_t__defined)
#define __MIG_check__Request__clock_alarm_t__defined

mig_internal kern_return_t __MIG_check__Request__clock_alarm_t(__attribute__((__unused__)) __Request__clock_alarm_t *In0P)
{

	typedef __Request__clock_alarm_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->alarm_port.type != MACH_MSG_PORT_DESCRIPTOR)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request__clock_alarm_t__defined) */
#endif /* __MIG_check__Request__clock_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine clock_alarm */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t clock_alarm
#if	defined(LINTLIBRARY)
    (clock_serv, alarm_type, alarm_time, alarm_port, alarm_portPoly)
	clock_serv_t clock_serv;
	alarm_type_t alarm_type;
	mach_timespec_t alarm_time;
	clock_reply_t alarm_port;
	mach_msg_type_name_t alarm_portPoly;
{ return clock_alarm(clock_serv, alarm_type, alarm_time, alarm_port, alarm_portPoly); }
#else
(
	clock_serv_t clock_serv,
	alarm_type_t alarm_type,
	mach_timespec_t alarm_time,
	clock_reply_t alarm_port,
	mach_msg_type_name_t alarm_portPoly
);
#endif	/* defined(LINTLIBRARY) */

/* Routine clock_alarm */
mig_internal novalue _Xclock_alarm
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t alarm_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		alarm_type_t alarm_type;
		mach_timespec_t alarm_time;
		mach_msg_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request__clock_alarm_t __Request;
	typedef __Reply__clock_alarm_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
#ifdef	__MIG_check__Request__clock_alarm_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request__clock_alarm_t__defined */

#if	__MigKernelSpecificCode
#else
#endif /* __MigKernelSpecificCode */
	__DeclareRcvRpc(1002, "clock_alarm")
	__BeforeRcvRpc(1002, "clock_alarm")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request__clock_alarm_t__defined)
	check_result = __MIG_check__Request__clock_alarm_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request__clock_alarm_t__defined) */

	OutP->RetCode = clock_alarm(convert_port_to_clock(In0P->Head.msgh_request_port), In0P->alarm_type, In0P->alarm_time, In0P->alarm_port.name, In0P->alarm_port.disposition);
#if	__MigKernelSpecificCode
#endif /* __MigKernelSpecificCode */

	OutP->NDR = NDR_record;


	__AfterRcvRpc(1002, "clock_alarm")
}


#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
boolean_t clock_server(
		mach_msg_header_t *InHeadP,
		mach_msg_header_t *OutHeadP);

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
mig_routine_t clock_server_routine(
		mach_msg_header_t *InHeadP);


/* Description of this subsystem, for use in direct RPC */
extern const struct clock_subsystem clock_subsystem;
const struct clock_subsystem {
	mig_server_routine_t 	server;	/* Server routine */
	mach_msg_id_t	start;	/* Min routine number */
	mach_msg_id_t	end;	/* Max routine number + 1 */
	unsigned int	maxsize;	/* Max msg size */
	vm_address_t	reserved;	/* Reserved */
	struct routine_descriptor	/*Array of routine descriptors */
		routine[3];
} clock_subsystem = {
	clock_server_routine,
	1000,
	1003,
	(mach_msg_size_t)sizeof(union __ReplyUnion__clock_subsystem),
	(vm_address_t)0,
	{
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xclock_get_time, 2, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__clock_get_time_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xclock_get_attributes, 4, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__clock_get_attributes_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _Xclock_alarm, 6, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__clock_alarm_t) },
	}
};

mig_external boolean_t clock_server
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

	if ((InHeadP->msgh_id > 1002) || (InHeadP->msgh_id < 1000) ||
	    ((routine = clock_subsystem.routine[InHeadP->msgh_id - 1000].stub_routine) == 0)) {
		((mig_reply_error_t *)OutHeadP)->NDR = NDR_record;
		((mig_reply_error_t *)OutHeadP)->RetCode = MIG_BAD_ID;
		return FALSE;
	}
	(*routine) (InHeadP, OutHeadP);
	return TRUE;
}

mig_external mig_routine_t clock_server_routine
	(mach_msg_header_t *InHeadP)
{
	register int msgh_id;

	msgh_id = InHeadP->msgh_id - 1000;

	if ((msgh_id > 2) || (msgh_id < 0))
		return 0;

	return clock_subsystem.routine[msgh_id].stub_routine;
}

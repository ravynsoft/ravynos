/*
 * IDENTIFICATION:
 * stub generated Fri Nov 26 16:30:31 2021
 * with a MiG generated Fri Nov 26 11:12:33 EST 2021 by zoe@yuki
 * OPTIONS: 
 */
#define	__MIG_check__Reply__asl_ipc_subsystem__ 1

#include "asl_ipc.h"
/* LINTLIBRARY */


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
#ifndef	__MachMsgErrorWithTimeout
#define	__MachMsgErrorWithTimeout(_R_) { \
	switch (_R_) { \
	case MACH_SEND_INVALID_REPLY: \
	case MACH_RCV_INVALID_NAME: \
	case MACH_RCV_IN_SET: \
	case MACH_RCV_PORT_DIED: \
	case MACH_RCV_PORT_CHANGED: \
	case MACH_SEND_INVALID_MEMORY: \
	case MACH_SEND_INVALID_RIGHT: \
	case MACH_SEND_INVALID_TYPE: \
	case MACH_SEND_MSG_TOO_SMALL: \
	case MACH_SEND_INVALID_RT_OOL_SIZE: \
	case MACH_RCV_TIMED_OUT: \
		mig_dealloc_reply_port(InP->Head.msgh_reply_port); \
	} \
}
#endif	/* __MachMsgErrorWithTimeout */

#ifndef	__MachMsgErrorWithoutTimeout
#define	__MachMsgErrorWithoutTimeout(_R_) { \
	switch (_R_) { \
	case MACH_SEND_INVALID_REPLY: \
	case MACH_RCV_INVALID_NAME: \
	case MACH_RCV_IN_SET: \
	case MACH_RCV_PORT_DIED: \
	case MACH_RCV_PORT_CHANGED: \
	case MACH_SEND_INVALID_MEMORY: \
	case MACH_SEND_INVALID_RIGHT: \
	case MACH_SEND_INVALID_TYPE: \
	case MACH_SEND_MSG_TOO_SMALL: \
	case MACH_SEND_INVALID_RT_OOL_SIZE: \
		mig_dealloc_reply_port(InP->Head.msgh_reply_port); \
	} \
}
#endif	/* __MachMsgErrorWithoutTimeout */

#ifndef	__DeclareSendRpc
#define	__DeclareSendRpc(_NUM_, _NAME_)
#endif	/* __DeclareSendRpc */

#ifndef	__BeforeSendRpc
#define	__BeforeSendRpc(_NUM_, _NAME_)
#endif	/* __BeforeSendRpc */

#ifndef	__AfterSendRpc
#define	__AfterSendRpc(_NUM_, _NAME_)
#endif	/* __AfterSendRpc */

#ifndef	__DeclareSendSimple
#define	__DeclareSendSimple(_NUM_, _NAME_)
#endif	/* __DeclareSendSimple */

#ifndef	__BeforeSendSimple
#define	__BeforeSendSimple(_NUM_, _NAME_)
#endif	/* __BeforeSendSimple */

#ifndef	__AfterSendSimple
#define	__AfterSendSimple(_NUM_, _NAME_)
#endif	/* __AfterSendSimple */

#define msgh_request_port	msgh_remote_port
#define msgh_reply_port		msgh_local_port



#if ( __MigTypeCheck )
#if __MIG_check__Reply__asl_ipc_subsystem__
#if !defined(__MIG_check__Reply___asl_server_query_t__defined)
#define __MIG_check__Reply___asl_server_query_t__defined

mig_internal kern_return_t __MIG_check__Reply___asl_server_query_t(__Reply___asl_server_query_t *Out0P)
{

	typedef __Reply___asl_server_query_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 214) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->reply.type != MACH_MSG_OOL_DESCRIPTOR) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply___asl_server_query_t__defined) */
#endif /* __MIG_check__Reply__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine _asl_server_query */
mig_external kern_return_t _asl_server_query
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	uint64_t startid,
	int count,
	int flags,
	caddr_t *reply,
	mach_msg_type_number_t *replyCnt,
	uint64_t *lastid,
	int *status,
	security_token_t *token
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t request;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t requestCnt;
		uint64_t startid;
		int count;
		int flags;
	} Request;
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
		mach_msg_ool_descriptor_t reply;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t replyCnt;
		uint64_t lastid;
		int status;
		mach_msg_max_trailer_t trailer;
	} Reply;
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
		mach_msg_ool_descriptor_t reply;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t replyCnt;
		uint64_t lastid;
		int status;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;
	mach_msg_max_trailer_t *TrailerP;
#if	__MigTypeCheck
	unsigned int trailer_size;
#endif	/* __MigTypeCheck */

#ifdef	__MIG_check__Reply___asl_server_query_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply___asl_server_query_t__defined */

	__DeclareSendRpc(114, "_asl_server_query")

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t requestTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	InP->msgh_body.msgh_descriptor_count = 1;
#if	UseStaticTemplates
	InP->request = requestTemplate;
	InP->request.address = (void *)(request);
	InP->request.size = requestCnt;
#else	/* UseStaticTemplates */
	InP->request.address = (void *)(request);
	InP->request.size = requestCnt;
	InP->request.deallocate =  TRUE;
	InP->request.copy = MACH_MSG_VIRTUAL_COPY;
	InP->request.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */

	InP->NDR = NDR_record;

	InP->requestCnt = requestCnt;

	InP->startid = startid;

	InP->count = count;

	InP->flags = flags;

	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = server;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 114;
	
/* BEGIN VOUCHER CODE */

#ifdef USING_VOUCHERS
	if (voucher_mach_msg_set != NULL) {
		voucher_mach_msg_set(&InP->Head);
	}
#endif // USING_VOUCHERS
	
/* END VOUCHER CODE */

	__BeforeSendRpc(114, "_asl_server_query")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0)|MACH_MSG_OPTION_NONE|MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_SENDER), sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(114, "_asl_server_query")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply___asl_server_query_t__defined)
	check_result = __MIG_check__Reply___asl_server_query_t((__Reply___asl_server_query_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply___asl_server_query_t__defined) */

	TrailerP = (mach_msg_max_trailer_t *)((vm_offset_t)Out0P +
		round_msg(Out0P->Head.msgh_size));
	if (TrailerP->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0)
		{ return MIG_TRAILER_ERROR ; }
#if	__MigTypeCheck
	trailer_size = TrailerP->msgh_trailer_size -
		(mach_msg_size_t)(sizeof(mach_msg_trailer_type_t) - sizeof(mach_msg_trailer_size_t));
#endif	/* __MigTypeCheck */
#if	__MigTypeCheck
	if (trailer_size < (mach_msg_size_t)sizeof(security_token_t))
		{ return MIG_TRAILER_ERROR ; }
	trailer_size -= (mach_msg_size_t)sizeof(security_token_t);
#endif	/* __MigTypeCheck */

	*reply = (caddr_t)(Out0P->reply.address);
	*replyCnt = Out0P->replyCnt;

	*lastid = Out0P->lastid;

	*status = Out0P->status;

	*token = TrailerP->msgh_sender;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck )
#if __MIG_check__Reply__asl_ipc_subsystem__
#if !defined(__MIG_check__Reply___asl_server_query_timeout_t__defined)
#define __MIG_check__Reply___asl_server_query_timeout_t__defined

mig_internal kern_return_t __MIG_check__Reply___asl_server_query_timeout_t(__Reply___asl_server_query_timeout_t *Out0P)
{

	typedef __Reply___asl_server_query_timeout_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 215) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->reply.type != MACH_MSG_OOL_DESCRIPTOR) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply___asl_server_query_timeout_t__defined) */
#endif /* __MIG_check__Reply__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine _asl_server_query_timeout */
mig_external kern_return_t _asl_server_query_timeout
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	uint64_t startid,
	int count,
	natural_t timeout,
	int flags,
	caddr_t *reply,
	mach_msg_type_number_t *replyCnt,
	uint64_t *lastid,
	int *status
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t request;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t requestCnt;
		uint64_t startid;
		int count;
		int flags;
	} Request;
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
		mach_msg_ool_descriptor_t reply;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t replyCnt;
		uint64_t lastid;
		int status;
		mach_msg_trailer_t trailer;
	} Reply;
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
		mach_msg_ool_descriptor_t reply;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t replyCnt;
		uint64_t lastid;
		int status;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply___asl_server_query_timeout_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply___asl_server_query_timeout_t__defined */

	__DeclareSendRpc(115, "_asl_server_query_timeout")

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t requestTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	InP->msgh_body.msgh_descriptor_count = 1;
#if	UseStaticTemplates
	InP->request = requestTemplate;
	InP->request.address = (void *)(request);
	InP->request.size = requestCnt;
#else	/* UseStaticTemplates */
	InP->request.address = (void *)(request);
	InP->request.size = requestCnt;
	InP->request.deallocate =  TRUE;
	InP->request.copy = MACH_MSG_VIRTUAL_COPY;
	InP->request.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */

	InP->NDR = NDR_record;

	InP->requestCnt = requestCnt;

	InP->startid = startid;

	InP->count = count;

	InP->flags = flags;

	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = server;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 115;
	
/* BEGIN VOUCHER CODE */

#ifdef USING_VOUCHERS
	if (voucher_mach_msg_set != NULL) {
		voucher_mach_msg_set(&InP->Head);
	}
#endif // USING_VOUCHERS
	
/* END VOUCHER CODE */

	__BeforeSendRpc(115, "_asl_server_query_timeout")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_SEND_TIMEOUT|MACH_RCV_TIMEOUT|MACH_MSG_OPTION_NONE, sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, timeout, MACH_PORT_NULL);
	__AfterSendRpc(115, "_asl_server_query_timeout")

	if (msg_result == MACH_SEND_TIMED_OUT) {
		if((vm_offset_t) InP->request.address != (vm_offset_t) request)
			mig_deallocate((vm_offset_t) InP->request.address, (vm_size_t) InP->request.size);
	}

	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply___asl_server_query_timeout_t__defined)
	check_result = __MIG_check__Reply___asl_server_query_timeout_t((__Reply___asl_server_query_timeout_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply___asl_server_query_timeout_t__defined) */

	*reply = (caddr_t)(Out0P->reply.address);
	*replyCnt = Out0P->replyCnt;

	*lastid = Out0P->lastid;

	*status = Out0P->status;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck )
#if __MIG_check__Reply__asl_ipc_subsystem__
#if !defined(__MIG_check__Reply___asl_server_prune_t__defined)
#define __MIG_check__Reply___asl_server_prune_t__defined

mig_internal kern_return_t __MIG_check__Reply___asl_server_prune_t(__Reply___asl_server_prune_t *Out0P)
{

	typedef __Reply___asl_server_prune_t __Reply;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 216) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    ((msgh_size != sizeof(__Reply)) &&
	     (msgh_size != sizeof(mig_reply_error_t) ||
	      Out0P->RetCode == KERN_SUCCESS)))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (Out0P->RetCode != KERN_SUCCESS) {
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply___asl_server_prune_t__defined) */
#endif /* __MIG_check__Reply__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine _asl_server_prune */
mig_external kern_return_t _asl_server_prune
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	int *status,
	security_token_t *token
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t request;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t requestCnt;
	} Request;
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
		int status;
		mach_msg_max_trailer_t trailer;
	} Reply;
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
		int status;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;
	mach_msg_max_trailer_t *TrailerP;
#if	__MigTypeCheck
	unsigned int trailer_size;
#endif	/* __MigTypeCheck */

#ifdef	__MIG_check__Reply___asl_server_prune_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply___asl_server_prune_t__defined */

	__DeclareSendRpc(116, "_asl_server_prune")

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t requestTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	InP->msgh_body.msgh_descriptor_count = 1;
#if	UseStaticTemplates
	InP->request = requestTemplate;
	InP->request.address = (void *)(request);
	InP->request.size = requestCnt;
#else	/* UseStaticTemplates */
	InP->request.address = (void *)(request);
	InP->request.size = requestCnt;
	InP->request.deallocate =  TRUE;
	InP->request.copy = MACH_MSG_VIRTUAL_COPY;
	InP->request.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */

	InP->NDR = NDR_record;

	InP->requestCnt = requestCnt;

	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = server;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 116;
	
/* BEGIN VOUCHER CODE */

#ifdef USING_VOUCHERS
	if (voucher_mach_msg_set != NULL) {
		voucher_mach_msg_set(&InP->Head);
	}
#endif // USING_VOUCHERS
	
/* END VOUCHER CODE */

	__BeforeSendRpc(116, "_asl_server_prune")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0)|MACH_MSG_OPTION_NONE|MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_SENDER), sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(116, "_asl_server_prune")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply___asl_server_prune_t__defined)
	check_result = __MIG_check__Reply___asl_server_prune_t((__Reply___asl_server_prune_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply___asl_server_prune_t__defined) */

	TrailerP = (mach_msg_max_trailer_t *)((vm_offset_t)Out0P +
		round_msg(Out0P->Head.msgh_size));
	if (TrailerP->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0)
		{ return MIG_TRAILER_ERROR ; }
#if	__MigTypeCheck
	trailer_size = TrailerP->msgh_trailer_size -
		(mach_msg_size_t)(sizeof(mach_msg_trailer_type_t) - sizeof(mach_msg_trailer_size_t));
#endif	/* __MigTypeCheck */
#if	__MigTypeCheck
	if (trailer_size < (mach_msg_size_t)sizeof(security_token_t))
		{ return MIG_TRAILER_ERROR ; }
	trailer_size -= (mach_msg_size_t)sizeof(security_token_t);
#endif	/* __MigTypeCheck */

	*status = Out0P->status;

	*token = TrailerP->msgh_sender;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck )
#if __MIG_check__Reply__asl_ipc_subsystem__
#if !defined(__MIG_check__Reply___asl_server_create_aux_link_t__defined)
#define __MIG_check__Reply___asl_server_create_aux_link_t__defined

mig_internal kern_return_t __MIG_check__Reply___asl_server_create_aux_link_t(__Reply___asl_server_create_aux_link_t *Out0P)
{

	typedef __Reply___asl_server_create_aux_link_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 217) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 2 ||
	    msgh_size != sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->fileport.type != MACH_MSG_PORT_DESCRIPTOR ||
	    Out0P->fileport.disposition != 17) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (Out0P->url.type != MACH_MSG_OOL_DESCRIPTOR) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply___asl_server_create_aux_link_t__defined) */
#endif /* __MIG_check__Reply__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine _asl_server_create_aux_link */
mig_external kern_return_t _asl_server_create_aux_link
(
	mach_port_t server,
	caddr_t message,
	mach_msg_type_number_t messageCnt,
	mach_port_t *fileport,
	caddr_t *url,
	mach_msg_type_number_t *urlCnt,
	int *status
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t message;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t messageCnt;
	} Request;
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
		mach_msg_port_descriptor_t fileport;
		mach_msg_ool_descriptor_t url;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t urlCnt;
		int status;
		mach_msg_trailer_t trailer;
	} Reply;
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
		mach_msg_port_descriptor_t fileport;
		mach_msg_ool_descriptor_t url;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t urlCnt;
		int status;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply___asl_server_create_aux_link_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply___asl_server_create_aux_link_t__defined */

	__DeclareSendRpc(117, "_asl_server_create_aux_link")

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t messageTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	InP->msgh_body.msgh_descriptor_count = 1;
#if	UseStaticTemplates
	InP->message = messageTemplate;
	InP->message.address = (void *)(message);
	InP->message.size = messageCnt;
#else	/* UseStaticTemplates */
	InP->message.address = (void *)(message);
	InP->message.size = messageCnt;
	InP->message.deallocate =  TRUE;
	InP->message.copy = MACH_MSG_VIRTUAL_COPY;
	InP->message.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */

	InP->NDR = NDR_record;

	InP->messageCnt = messageCnt;

	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = server;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 117;
	
/* BEGIN VOUCHER CODE */

#ifdef USING_VOUCHERS
	if (voucher_mach_msg_set != NULL) {
		voucher_mach_msg_set(&InP->Head);
	}
#endif // USING_VOUCHERS
	
/* END VOUCHER CODE */

	__BeforeSendRpc(117, "_asl_server_create_aux_link")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(117, "_asl_server_create_aux_link")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply___asl_server_create_aux_link_t__defined)
	check_result = __MIG_check__Reply___asl_server_create_aux_link_t((__Reply___asl_server_create_aux_link_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply___asl_server_create_aux_link_t__defined) */

	*fileport = Out0P->fileport.name;
	*url = (caddr_t)(Out0P->url.address);
	*urlCnt = Out0P->urlCnt;

	*status = Out0P->status;

	return KERN_SUCCESS;
}

/* SimpleRoutine _asl_server_message */
mig_external kern_return_t _asl_server_message
(
	mach_port_t server,
	caddr_t message,
	mach_msg_type_number_t messageCnt
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t message;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t messageCnt;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
	} Mess;

	Request *InP = &Mess.In;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply___asl_server_message_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply___asl_server_message_t__defined */

	__DeclareSendSimple(118, "_asl_server_message")

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t messageTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	InP->msgh_body.msgh_descriptor_count = 1;
#if	UseStaticTemplates
	InP->message = messageTemplate;
	InP->message.address = (void *)(message);
	InP->message.size = messageCnt;
#else	/* UseStaticTemplates */
	InP->message.address = (void *)(message);
	InP->message.size = messageCnt;
	InP->message.deallocate =  TRUE;
	InP->message.copy = MACH_MSG_VIRTUAL_COPY;
	InP->message.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */

	InP->NDR = NDR_record;

	InP->messageCnt = messageCnt;

	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, 0);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = server;
	InP->Head.msgh_reply_port = MACH_PORT_NULL;
	InP->Head.msgh_id = 118;
	
/* BEGIN VOUCHER CODE */

#ifdef USING_VOUCHERS
	if (voucher_mach_msg_set != NULL) {
		voucher_mach_msg_set(&InP->Head);
	}
#endif // USING_VOUCHERS
	
/* END VOUCHER CODE */

	__BeforeSendSimple(118, "_asl_server_message")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_MSG_OPTION_NONE, sizeof(Request), 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendSimple(118, "_asl_server_message")
	return msg_result;
}

/* SimpleRoutine _asl_server_register_direct_watch */
mig_external kern_return_t _asl_server_register_direct_watch
(
	mach_port_t server,
	int port
)
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
		int port;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
	} Mess;

	Request *InP = &Mess.In;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply___asl_server_register_direct_watch_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply___asl_server_register_direct_watch_t__defined */

	__DeclareSendSimple(119, "_asl_server_register_direct_watch")

	InP->msgh_body.msgh_descriptor_count = 0;
	InP->NDR = NDR_record;

	InP->port = port;

	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, 0);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = server;
	InP->Head.msgh_reply_port = MACH_PORT_NULL;
	InP->Head.msgh_id = 119;
	
/* BEGIN VOUCHER CODE */

#ifdef USING_VOUCHERS
	if (voucher_mach_msg_set != NULL) {
		voucher_mach_msg_set(&InP->Head);
	}
#endif // USING_VOUCHERS
	
/* END VOUCHER CODE */

	__BeforeSendSimple(119, "_asl_server_register_direct_watch")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_MSG_OPTION_NONE, sizeof(Request), 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendSimple(119, "_asl_server_register_direct_watch")
	return msg_result;
}

/* SimpleRoutine _asl_server_cancel_direct_watch */
mig_external kern_return_t _asl_server_cancel_direct_watch
(
	mach_port_t server,
	int port
)
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
		int port;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
	} Mess;

	Request *InP = &Mess.In;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply___asl_server_cancel_direct_watch_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply___asl_server_cancel_direct_watch_t__defined */

	__DeclareSendSimple(120, "_asl_server_cancel_direct_watch")

	InP->msgh_body.msgh_descriptor_count = 0;
	InP->NDR = NDR_record;

	InP->port = port;

	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, 0);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = server;
	InP->Head.msgh_reply_port = MACH_PORT_NULL;
	InP->Head.msgh_id = 120;
	
/* BEGIN VOUCHER CODE */

#ifdef USING_VOUCHERS
	if (voucher_mach_msg_set != NULL) {
		voucher_mach_msg_set(&InP->Head);
	}
#endif // USING_VOUCHERS
	
/* END VOUCHER CODE */

	__BeforeSendSimple(120, "_asl_server_cancel_direct_watch")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_MSG_OPTION_NONE, sizeof(Request), 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendSimple(120, "_asl_server_cancel_direct_watch")
	return msg_result;
}

#if ( __MigTypeCheck )
#if __MIG_check__Reply__asl_ipc_subsystem__
#if !defined(__MIG_check__Reply___asl_server_query_2_t__defined)
#define __MIG_check__Reply___asl_server_query_2_t__defined

mig_internal kern_return_t __MIG_check__Reply___asl_server_query_2_t(__Reply___asl_server_query_2_t *Out0P)
{

	typedef __Reply___asl_server_query_2_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 221) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->reply.type != MACH_MSG_OOL_DESCRIPTOR) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply___asl_server_query_2_t__defined) */
#endif /* __MIG_check__Reply__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine _asl_server_query_2 */
mig_external kern_return_t _asl_server_query_2
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	uint64_t startid,
	int count,
	int flags,
	caddr_t *reply,
	mach_msg_type_number_t *replyCnt,
	uint64_t *lastid,
	int *status
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t request;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t requestCnt;
		uint64_t startid;
		int count;
		int flags;
	} Request;
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
		mach_msg_ool_descriptor_t reply;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t replyCnt;
		uint64_t lastid;
		int status;
		mach_msg_trailer_t trailer;
	} Reply;
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
		mach_msg_ool_descriptor_t reply;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t replyCnt;
		uint64_t lastid;
		int status;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply___asl_server_query_2_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply___asl_server_query_2_t__defined */

	__DeclareSendRpc(121, "_asl_server_query_2")

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t requestTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	InP->msgh_body.msgh_descriptor_count = 1;
#if	UseStaticTemplates
	InP->request = requestTemplate;
	InP->request.address = (void *)(request);
	InP->request.size = requestCnt;
#else	/* UseStaticTemplates */
	InP->request.address = (void *)(request);
	InP->request.size = requestCnt;
	InP->request.deallocate =  TRUE;
	InP->request.copy = MACH_MSG_VIRTUAL_COPY;
	InP->request.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */

	InP->NDR = NDR_record;

	InP->requestCnt = requestCnt;

	InP->startid = startid;

	InP->count = count;

	InP->flags = flags;

	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = server;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 121;
	
/* BEGIN VOUCHER CODE */

#ifdef USING_VOUCHERS
	if (voucher_mach_msg_set != NULL) {
		voucher_mach_msg_set(&InP->Head);
	}
#endif // USING_VOUCHERS
	
/* END VOUCHER CODE */

	__BeforeSendRpc(121, "_asl_server_query_2")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(121, "_asl_server_query_2")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply___asl_server_query_2_t__defined)
	check_result = __MIG_check__Reply___asl_server_query_2_t((__Reply___asl_server_query_2_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply___asl_server_query_2_t__defined) */

	*reply = (caddr_t)(Out0P->reply.address);
	*replyCnt = Out0P->replyCnt;

	*lastid = Out0P->lastid;

	*status = Out0P->status;

	return KERN_SUCCESS;
}

#if ( __MigTypeCheck )
#if __MIG_check__Reply__asl_ipc_subsystem__
#if !defined(__MIG_check__Reply___asl_server_match_t__defined)
#define __MIG_check__Reply___asl_server_match_t__defined

mig_internal kern_return_t __MIG_check__Reply___asl_server_match_t(__Reply___asl_server_match_t *Out0P)
{

	typedef __Reply___asl_server_match_t __Reply;
	boolean_t msgh_simple;
#if	__MigTypeCheck
	unsigned int msgh_size;
#endif	/* __MigTypeCheck */
	if (Out0P->Head.msgh_id != 222) {
	    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)
		{ return MIG_SERVER_DIED; }
	    else
		{ return MIG_REPLY_MISMATCH; }
	}

	msgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);
#if	__MigTypeCheck
	msgh_size = Out0P->Head.msgh_size;

	if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != 1 ||
	    msgh_size != sizeof(__Reply)) &&
	    (!msgh_simple || msgh_size != sizeof(mig_reply_error_t) ||
	    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))
		{ return MIG_TYPE_ERROR ; }
#endif	/* __MigTypeCheck */

	if (msgh_simple) {
		return ((mig_reply_error_t *)Out0P)->RetCode;
	}

#if	__MigTypeCheck
	if (Out0P->reply.type != MACH_MSG_OOL_DESCRIPTOR) {
		return MIG_TYPE_ERROR;
	}
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Reply___asl_server_match_t__defined) */
#endif /* __MIG_check__Reply__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine _asl_server_match */
mig_external kern_return_t _asl_server_match
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	uint64_t startid,
	uint64_t count,
	uint32_t duration,
	int direction,
	caddr_t *reply,
	mach_msg_type_number_t *replyCnt,
	uint64_t *lastid,
	int *status
)
{

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t request;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t requestCnt;
		uint64_t startid;
		uint64_t count;
		uint32_t duration;
		int direction;
	} Request;
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
		mach_msg_ool_descriptor_t reply;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t replyCnt;
		uint64_t lastid;
		int status;
		mach_msg_trailer_t trailer;
	} Reply;
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
		mach_msg_ool_descriptor_t reply;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t replyCnt;
		uint64_t lastid;
		int status;
	} __Reply;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	union {
		Request In;
		Reply Out;
	} Mess;

	Request *InP = &Mess.In;
	Reply *Out0P = &Mess.Out;

	mach_msg_return_t msg_result;

#ifdef	__MIG_check__Reply___asl_server_match_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Reply___asl_server_match_t__defined */

	__DeclareSendRpc(122, "_asl_server_match")

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t requestTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	InP->msgh_body.msgh_descriptor_count = 1;
#if	UseStaticTemplates
	InP->request = requestTemplate;
	InP->request.address = (void *)(request);
	InP->request.size = requestCnt;
#else	/* UseStaticTemplates */
	InP->request.address = (void *)(request);
	InP->request.size = requestCnt;
	InP->request.deallocate =  TRUE;
	InP->request.copy = MACH_MSG_VIRTUAL_COPY;
	InP->request.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */

	InP->NDR = NDR_record;

	InP->requestCnt = requestCnt;

	InP->startid = startid;

	InP->count = count;

	InP->duration = duration;

	InP->direction = direction;

	InP->Head.msgh_bits = MACH_MSGH_BITS_COMPLEX|
		MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	/* msgh_size passed as argument */
	InP->Head.msgh_request_port = server;
	InP->Head.msgh_reply_port = mig_get_reply_port();
	InP->Head.msgh_id = 122;
	
/* BEGIN VOUCHER CODE */

#ifdef USING_VOUCHERS
	if (voucher_mach_msg_set != NULL) {
		voucher_mach_msg_set(&InP->Head);
	}
#endif // USING_VOUCHERS
	
/* END VOUCHER CODE */

	__BeforeSendRpc(122, "_asl_server_match")
	msg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_MSG_OPTION_NONE, sizeof(Request), (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	__AfterSendRpc(122, "_asl_server_match")
	if (msg_result != MACH_MSG_SUCCESS) {
		__MachMsgErrorWithoutTimeout(msg_result);
		{ return msg_result; }
	}


#if	defined(__MIG_check__Reply___asl_server_match_t__defined)
	check_result = __MIG_check__Reply___asl_server_match_t((__Reply___asl_server_match_t *)Out0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ return check_result; }
#endif	/* defined(__MIG_check__Reply___asl_server_match_t__defined) */

	*reply = (caddr_t)(Out0P->reply.address);
	*replyCnt = Out0P->replyCnt;

	*lastid = Out0P->lastid;

	*status = Out0P->status;

	return KERN_SUCCESS;
}

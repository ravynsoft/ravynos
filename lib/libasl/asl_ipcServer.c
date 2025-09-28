/*
 * IDENTIFICATION:
 * stub generated Fri Nov 26 16:30:31 2021
 * with a MiG generated Fri Nov 26 11:12:33 EST 2021 by zoe@yuki
 * OPTIONS: 
 */

/* Module asl_ipc */

#define	__MIG_check__Request__asl_ipc_subsystem__ 1

#include "asl_ipc.h"

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
#define msgh_request_port	msgh_local_port
#define MACH_MSGH_BITS_REQUEST(bits)	MACH_MSGH_BITS_LOCAL(bits)
#define msgh_reply_port		msgh_remote_port
#define MACH_MSGH_BITS_REPLY(bits)	MACH_MSGH_BITS_REMOTE(bits)

#define MIG_RETURN_ERROR(X, code)	{\
				((mig_reply_error_t *)X)->RetCode = code;\
				((mig_reply_error_t *)X)->NDR = NDR_record;\
				return;\
				}

/* Forward Declarations */


mig_internal novalue _X_asl_server_query
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _X_asl_server_query_timeout
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _X_asl_server_prune
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _X_asl_server_create_aux_link
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _X_asl_server_message
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _X_asl_server_register_direct_watch
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _X_asl_server_cancel_direct_watch
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _X_asl_server_query_2
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

mig_internal novalue _X_asl_server_match
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);


#if ( __MigTypeCheck )
#if __MIG_check__Request__asl_ipc_subsystem__
#if !defined(__MIG_check__Request___asl_server_query_t__defined)
#define __MIG_check__Request___asl_server_query_t__defined

mig_internal kern_return_t __MIG_check__Request___asl_server_query_t(__attribute__((__unused__)) __Request___asl_server_query_t *In0P)
{

	typedef __Request___asl_server_query_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->request.type != MACH_MSG_OOL_DESCRIPTOR)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

#if __MigTypeCheck
	if (In0P->request.size != In0P->requestCnt)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request___asl_server_query_t__defined) */
#endif /* __MIG_check__Request__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine _asl_server_query */
mig_internal novalue _X_asl_server_query
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
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
		mach_msg_max_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request___asl_server_query_t __Request;
	typedef __Reply___asl_server_query_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
	mach_msg_max_trailer_t *TrailerP;
#if	__MigTypeCheck
	unsigned int trailer_size;
#endif	/* __MigTypeCheck */
#ifdef	__MIG_check__Request___asl_server_query_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request___asl_server_query_t__defined */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t replyTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	kern_return_t RetCode;
	__DeclareRcvRpc(114, "_asl_server_query")
	__BeforeRcvRpc(114, "_asl_server_query")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request___asl_server_query_t__defined)
	check_result = __MIG_check__Request___asl_server_query_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request___asl_server_query_t__defined) */

	TrailerP = (mach_msg_max_trailer_t *)((vm_offset_t)In0P +
		round_msg(In0P->Head.msgh_size));
	if (TrailerP->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0)
		{ MIG_RETURN_ERROR(In0P, MIG_TRAILER_ERROR); }
#if	__MigTypeCheck
	trailer_size = TrailerP->msgh_trailer_size -
		(mach_msg_size_t)(sizeof(mach_msg_trailer_type_t) - sizeof(mach_msg_trailer_size_t));
#endif	/* __MigTypeCheck */
#if	__MigTypeCheck
	if (trailer_size < (mach_msg_size_t)sizeof(security_token_t))
		{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }
	trailer_size -= (mach_msg_size_t)sizeof(security_token_t);
#endif	/* __MigTypeCheck */
#if	UseStaticTemplates
	OutP->reply = replyTemplate;
#else	/* UseStaticTemplates */
	OutP->reply.deallocate =  TRUE;
	OutP->reply.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->reply.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = __asl_server_query(In0P->Head.msgh_request_port, (caddr_t)(In0P->request.address), In0P->requestCnt, In0P->startid, In0P->count, In0P->flags, (caddr_t *)&(OutP->reply.address), &OutP->replyCnt, &OutP->lastid, &OutP->status, &TrailerP->msgh_sender);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
	OutP->reply.size = OutP->replyCnt;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(114, "_asl_server_query")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__asl_ipc_subsystem__
#if !defined(__MIG_check__Request___asl_server_query_timeout_t__defined)
#define __MIG_check__Request___asl_server_query_timeout_t__defined

mig_internal kern_return_t __MIG_check__Request___asl_server_query_timeout_t(__attribute__((__unused__)) __Request___asl_server_query_timeout_t *In0P)
{

	typedef __Request___asl_server_query_timeout_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->request.type != MACH_MSG_OOL_DESCRIPTOR)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

#if __MigTypeCheck
	if (In0P->request.size != In0P->requestCnt)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request___asl_server_query_timeout_t__defined) */
#endif /* __MIG_check__Request__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine _asl_server_query_timeout */
mig_internal novalue _X_asl_server_query_timeout
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
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
		mach_msg_max_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request___asl_server_query_timeout_t __Request;
	typedef __Reply___asl_server_query_timeout_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
	mach_msg_max_trailer_t *TrailerP;
#if	__MigTypeCheck
	unsigned int trailer_size;
#endif	/* __MigTypeCheck */
#ifdef	__MIG_check__Request___asl_server_query_timeout_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request___asl_server_query_timeout_t__defined */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t replyTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	kern_return_t RetCode;
	__DeclareRcvRpc(115, "_asl_server_query_timeout")
	__BeforeRcvRpc(115, "_asl_server_query_timeout")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request___asl_server_query_timeout_t__defined)
	check_result = __MIG_check__Request___asl_server_query_timeout_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request___asl_server_query_timeout_t__defined) */

	TrailerP = (mach_msg_max_trailer_t *)((vm_offset_t)In0P +
		round_msg(In0P->Head.msgh_size));
	if (TrailerP->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0)
		{ MIG_RETURN_ERROR(In0P, MIG_TRAILER_ERROR); }
#if	__MigTypeCheck
	trailer_size = TrailerP->msgh_trailer_size -
		(mach_msg_size_t)(sizeof(mach_msg_trailer_type_t) - sizeof(mach_msg_trailer_size_t));
#endif	/* __MigTypeCheck */
#if	__MigTypeCheck
	if (trailer_size < (mach_msg_size_t)sizeof(audit_token_t))
		{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }
	trailer_size -= (mach_msg_size_t)sizeof(audit_token_t);
#endif	/* __MigTypeCheck */
#if	UseStaticTemplates
	OutP->reply = replyTemplate;
#else	/* UseStaticTemplates */
	OutP->reply.deallocate =  TRUE;
	OutP->reply.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->reply.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = __asl_server_query_timeout(In0P->Head.msgh_request_port, (caddr_t)(In0P->request.address), In0P->requestCnt, In0P->startid, In0P->count, In0P->flags, (caddr_t *)&(OutP->reply.address), &OutP->replyCnt, &OutP->lastid, &OutP->status, TrailerP->msgh_audit);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
	OutP->reply.size = OutP->replyCnt;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(115, "_asl_server_query_timeout")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__asl_ipc_subsystem__
#if !defined(__MIG_check__Request___asl_server_prune_t__defined)
#define __MIG_check__Request___asl_server_prune_t__defined

mig_internal kern_return_t __MIG_check__Request___asl_server_prune_t(__attribute__((__unused__)) __Request___asl_server_prune_t *In0P)
{

	typedef __Request___asl_server_prune_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->request.type != MACH_MSG_OOL_DESCRIPTOR)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

#if __MigTypeCheck
	if (In0P->request.size != In0P->requestCnt)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request___asl_server_prune_t__defined) */
#endif /* __MIG_check__Request__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine _asl_server_prune */
mig_internal novalue _X_asl_server_prune
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
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
		mach_msg_max_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request___asl_server_prune_t __Request;
	typedef __Reply___asl_server_prune_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
	mach_msg_max_trailer_t *TrailerP;
#if	__MigTypeCheck
	unsigned int trailer_size;
#endif	/* __MigTypeCheck */
#ifdef	__MIG_check__Request___asl_server_prune_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request___asl_server_prune_t__defined */

	__DeclareRcvRpc(116, "_asl_server_prune")
	__BeforeRcvRpc(116, "_asl_server_prune")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request___asl_server_prune_t__defined)
	check_result = __MIG_check__Request___asl_server_prune_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request___asl_server_prune_t__defined) */

	TrailerP = (mach_msg_max_trailer_t *)((vm_offset_t)In0P +
		round_msg(In0P->Head.msgh_size));
	if (TrailerP->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0)
		{ MIG_RETURN_ERROR(In0P, MIG_TRAILER_ERROR); }
#if	__MigTypeCheck
	trailer_size = TrailerP->msgh_trailer_size -
		(mach_msg_size_t)(sizeof(mach_msg_trailer_type_t) - sizeof(mach_msg_trailer_size_t));
#endif	/* __MigTypeCheck */
#if	__MigTypeCheck
	if (trailer_size < (mach_msg_size_t)sizeof(security_token_t))
		{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }
	trailer_size -= (mach_msg_size_t)sizeof(security_token_t);
#endif	/* __MigTypeCheck */
	OutP->RetCode = __asl_server_prune(In0P->Head.msgh_request_port, (caddr_t)(In0P->request.address), In0P->requestCnt, &OutP->status, &TrailerP->msgh_sender);
	if (OutP->RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, OutP->RetCode);
	}

	OutP->NDR = NDR_record;


	OutP->Head.msgh_size = (sizeof(Reply));
	__AfterRcvRpc(116, "_asl_server_prune")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__asl_ipc_subsystem__
#if !defined(__MIG_check__Request___asl_server_create_aux_link_t__defined)
#define __MIG_check__Request___asl_server_create_aux_link_t__defined

mig_internal kern_return_t __MIG_check__Request___asl_server_create_aux_link_t(__attribute__((__unused__)) __Request___asl_server_create_aux_link_t *In0P)
{

	typedef __Request___asl_server_create_aux_link_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->message.type != MACH_MSG_OOL_DESCRIPTOR)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

#if __MigTypeCheck
	if (In0P->message.size != In0P->messageCnt)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request___asl_server_create_aux_link_t__defined) */
#endif /* __MIG_check__Request__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine _asl_server_create_aux_link */
mig_internal novalue _X_asl_server_create_aux_link
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
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
		mach_msg_max_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request___asl_server_create_aux_link_t __Request;
	typedef __Reply___asl_server_create_aux_link_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
	mach_msg_max_trailer_t *TrailerP;
#if	__MigTypeCheck
	unsigned int trailer_size;
#endif	/* __MigTypeCheck */
#ifdef	__MIG_check__Request___asl_server_create_aux_link_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request___asl_server_create_aux_link_t__defined */

#if	UseStaticTemplates
	const static mach_msg_port_descriptor_t fileportTemplate = {
		.name = MACH_PORT_NULL,
		.disposition = 17,
		.type = MACH_MSG_PORT_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t urlTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	kern_return_t RetCode;
	__DeclareRcvRpc(117, "_asl_server_create_aux_link")
	__BeforeRcvRpc(117, "_asl_server_create_aux_link")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request___asl_server_create_aux_link_t__defined)
	check_result = __MIG_check__Request___asl_server_create_aux_link_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request___asl_server_create_aux_link_t__defined) */

	TrailerP = (mach_msg_max_trailer_t *)((vm_offset_t)In0P +
		round_msg(In0P->Head.msgh_size));
	if (TrailerP->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0)
		{ MIG_RETURN_ERROR(In0P, MIG_TRAILER_ERROR); }
#if	__MigTypeCheck
	trailer_size = TrailerP->msgh_trailer_size -
		(mach_msg_size_t)(sizeof(mach_msg_trailer_type_t) - sizeof(mach_msg_trailer_size_t));
#endif	/* __MigTypeCheck */
#if	__MigTypeCheck
	if (trailer_size < (mach_msg_size_t)sizeof(audit_token_t))
		{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }
	trailer_size -= (mach_msg_size_t)sizeof(audit_token_t);
#endif	/* __MigTypeCheck */
#if	UseStaticTemplates
	OutP->fileport = fileportTemplate;
#else	/* UseStaticTemplates */
	OutP->fileport.disposition = 17;
	OutP->fileport.type = MACH_MSG_PORT_DESCRIPTOR;
#endif	/* UseStaticTemplates */


#if	UseStaticTemplates
	OutP->url = urlTemplate;
#else	/* UseStaticTemplates */
	OutP->url.deallocate =  TRUE;
	OutP->url.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->url.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = __asl_server_create_aux_link(In0P->Head.msgh_request_port, (caddr_t)(In0P->message.address), In0P->messageCnt, &OutP->fileport.name, (caddr_t *)&(OutP->url.address), &OutP->urlCnt, &OutP->status, TrailerP->msgh_audit);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
	OutP->url.size = OutP->urlCnt;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 2;
	__AfterRcvRpc(117, "_asl_server_create_aux_link")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__asl_ipc_subsystem__
#if !defined(__MIG_check__Request___asl_server_message_t__defined)
#define __MIG_check__Request___asl_server_message_t__defined

mig_internal kern_return_t __MIG_check__Request___asl_server_message_t(__attribute__((__unused__)) __Request___asl_server_message_t *In0P)
{

	typedef __Request___asl_server_message_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->message.type != MACH_MSG_OOL_DESCRIPTOR)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

#if __MigTypeCheck
	if (In0P->message.size != In0P->messageCnt)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request___asl_server_message_t__defined) */
#endif /* __MIG_check__Request__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* SimpleRoutine _asl_server_message */
mig_internal novalue _X_asl_server_message
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
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
		mach_msg_max_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request___asl_server_message_t __Request;
	typedef __Reply___asl_server_message_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
	mach_msg_max_trailer_t *TrailerP;
#if	__MigTypeCheck
	unsigned int trailer_size;
#endif	/* __MigTypeCheck */
#ifdef	__MIG_check__Request___asl_server_message_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request___asl_server_message_t__defined */

	__DeclareRcvSimple(118, "_asl_server_message")
	__BeforeRcvSimple(118, "_asl_server_message")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request___asl_server_message_t__defined)
	check_result = __MIG_check__Request___asl_server_message_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request___asl_server_message_t__defined) */

	TrailerP = (mach_msg_max_trailer_t *)((vm_offset_t)In0P +
		round_msg(In0P->Head.msgh_size));
	if (TrailerP->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0)
		{ MIG_RETURN_ERROR(In0P, MIG_TRAILER_ERROR); }
#if	__MigTypeCheck
	trailer_size = TrailerP->msgh_trailer_size -
		(mach_msg_size_t)(sizeof(mach_msg_trailer_type_t) - sizeof(mach_msg_trailer_size_t));
#endif	/* __MigTypeCheck */
#if	__MigTypeCheck
	if (trailer_size < (mach_msg_size_t)sizeof(audit_token_t))
		{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }
	trailer_size -= (mach_msg_size_t)sizeof(audit_token_t);
#endif	/* __MigTypeCheck */
	OutP->RetCode = __asl_server_message(In0P->Head.msgh_request_port, (caddr_t)(In0P->message.address), In0P->messageCnt, TrailerP->msgh_audit);
	__AfterRcvSimple(118, "_asl_server_message")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__asl_ipc_subsystem__
#if !defined(__MIG_check__Request___asl_server_register_direct_watch_t__defined)
#define __MIG_check__Request___asl_server_register_direct_watch_t__defined

mig_internal kern_return_t __MIG_check__Request___asl_server_register_direct_watch_t(__attribute__((__unused__)) __Request___asl_server_register_direct_watch_t *In0P)
{

	typedef __Request___asl_server_register_direct_watch_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request___asl_server_register_direct_watch_t__defined) */
#endif /* __MIG_check__Request__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* SimpleRoutine _asl_server_register_direct_watch */
mig_internal novalue _X_asl_server_register_direct_watch
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
		int port;
		mach_msg_max_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request___asl_server_register_direct_watch_t __Request;
	typedef __Reply___asl_server_register_direct_watch_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
	mach_msg_max_trailer_t *TrailerP;
#if	__MigTypeCheck
	unsigned int trailer_size;
#endif	/* __MigTypeCheck */
#ifdef	__MIG_check__Request___asl_server_register_direct_watch_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request___asl_server_register_direct_watch_t__defined */

	__DeclareRcvSimple(119, "_asl_server_register_direct_watch")
	__BeforeRcvSimple(119, "_asl_server_register_direct_watch")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request___asl_server_register_direct_watch_t__defined)
	check_result = __MIG_check__Request___asl_server_register_direct_watch_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request___asl_server_register_direct_watch_t__defined) */

	TrailerP = (mach_msg_max_trailer_t *)((vm_offset_t)In0P +
		round_msg(In0P->Head.msgh_size));
	if (TrailerP->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0)
		{ MIG_RETURN_ERROR(In0P, MIG_TRAILER_ERROR); }
#if	__MigTypeCheck
	trailer_size = TrailerP->msgh_trailer_size -
		(mach_msg_size_t)(sizeof(mach_msg_trailer_type_t) - sizeof(mach_msg_trailer_size_t));
#endif	/* __MigTypeCheck */
#if	__MigTypeCheck
	if (trailer_size < (mach_msg_size_t)sizeof(audit_token_t))
		{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }
	trailer_size -= (mach_msg_size_t)sizeof(audit_token_t);
#endif	/* __MigTypeCheck */
	OutP->RetCode = __asl_server_register_direct_watch(In0P->Head.msgh_request_port, In0P->port, TrailerP->msgh_audit);
	__AfterRcvSimple(119, "_asl_server_register_direct_watch")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__asl_ipc_subsystem__
#if !defined(__MIG_check__Request___asl_server_cancel_direct_watch_t__defined)
#define __MIG_check__Request___asl_server_cancel_direct_watch_t__defined

mig_internal kern_return_t __MIG_check__Request___asl_server_cancel_direct_watch_t(__attribute__((__unused__)) __Request___asl_server_cancel_direct_watch_t *In0P)
{

	typedef __Request___asl_server_cancel_direct_watch_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 0) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request___asl_server_cancel_direct_watch_t__defined) */
#endif /* __MIG_check__Request__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* SimpleRoutine _asl_server_cancel_direct_watch */
mig_internal novalue _X_asl_server_cancel_direct_watch
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
		int port;
		mach_msg_max_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request___asl_server_cancel_direct_watch_t __Request;
	typedef __Reply___asl_server_cancel_direct_watch_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
	mach_msg_max_trailer_t *TrailerP;
#if	__MigTypeCheck
	unsigned int trailer_size;
#endif	/* __MigTypeCheck */
#ifdef	__MIG_check__Request___asl_server_cancel_direct_watch_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request___asl_server_cancel_direct_watch_t__defined */

	__DeclareRcvSimple(120, "_asl_server_cancel_direct_watch")
	__BeforeRcvSimple(120, "_asl_server_cancel_direct_watch")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request___asl_server_cancel_direct_watch_t__defined)
	check_result = __MIG_check__Request___asl_server_cancel_direct_watch_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request___asl_server_cancel_direct_watch_t__defined) */

	TrailerP = (mach_msg_max_trailer_t *)((vm_offset_t)In0P +
		round_msg(In0P->Head.msgh_size));
	if (TrailerP->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0)
		{ MIG_RETURN_ERROR(In0P, MIG_TRAILER_ERROR); }
#if	__MigTypeCheck
	trailer_size = TrailerP->msgh_trailer_size -
		(mach_msg_size_t)(sizeof(mach_msg_trailer_type_t) - sizeof(mach_msg_trailer_size_t));
#endif	/* __MigTypeCheck */
#if	__MigTypeCheck
	if (trailer_size < (mach_msg_size_t)sizeof(audit_token_t))
		{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }
	trailer_size -= (mach_msg_size_t)sizeof(audit_token_t);
#endif	/* __MigTypeCheck */
	OutP->RetCode = __asl_server_cancel_direct_watch(In0P->Head.msgh_request_port, In0P->port, TrailerP->msgh_audit);
	__AfterRcvSimple(120, "_asl_server_cancel_direct_watch")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__asl_ipc_subsystem__
#if !defined(__MIG_check__Request___asl_server_query_2_t__defined)
#define __MIG_check__Request___asl_server_query_2_t__defined

mig_internal kern_return_t __MIG_check__Request___asl_server_query_2_t(__attribute__((__unused__)) __Request___asl_server_query_2_t *In0P)
{

	typedef __Request___asl_server_query_2_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->request.type != MACH_MSG_OOL_DESCRIPTOR)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

#if __MigTypeCheck
	if (In0P->request.size != In0P->requestCnt)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request___asl_server_query_2_t__defined) */
#endif /* __MIG_check__Request__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine _asl_server_query_2 */
mig_internal novalue _X_asl_server_query_2
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
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
		mach_msg_max_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request___asl_server_query_2_t __Request;
	typedef __Reply___asl_server_query_2_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
	mach_msg_max_trailer_t *TrailerP;
#if	__MigTypeCheck
	unsigned int trailer_size;
#endif	/* __MigTypeCheck */
#ifdef	__MIG_check__Request___asl_server_query_2_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request___asl_server_query_2_t__defined */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t replyTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	kern_return_t RetCode;
	__DeclareRcvRpc(121, "_asl_server_query_2")
	__BeforeRcvRpc(121, "_asl_server_query_2")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request___asl_server_query_2_t__defined)
	check_result = __MIG_check__Request___asl_server_query_2_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request___asl_server_query_2_t__defined) */

	TrailerP = (mach_msg_max_trailer_t *)((vm_offset_t)In0P +
		round_msg(In0P->Head.msgh_size));
	if (TrailerP->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0)
		{ MIG_RETURN_ERROR(In0P, MIG_TRAILER_ERROR); }
#if	__MigTypeCheck
	trailer_size = TrailerP->msgh_trailer_size -
		(mach_msg_size_t)(sizeof(mach_msg_trailer_type_t) - sizeof(mach_msg_trailer_size_t));
#endif	/* __MigTypeCheck */
#if	__MigTypeCheck
	if (trailer_size < (mach_msg_size_t)sizeof(audit_token_t))
		{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }
	trailer_size -= (mach_msg_size_t)sizeof(audit_token_t);
#endif	/* __MigTypeCheck */
#if	UseStaticTemplates
	OutP->reply = replyTemplate;
#else	/* UseStaticTemplates */
	OutP->reply.deallocate =  TRUE;
	OutP->reply.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->reply.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = __asl_server_query_2(In0P->Head.msgh_request_port, (caddr_t)(In0P->request.address), In0P->requestCnt, In0P->startid, In0P->count, In0P->flags, (caddr_t *)&(OutP->reply.address), &OutP->replyCnt, &OutP->lastid, &OutP->status, TrailerP->msgh_audit);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
	OutP->reply.size = OutP->replyCnt;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(121, "_asl_server_query_2")
}

#if ( __MigTypeCheck )
#if __MIG_check__Request__asl_ipc_subsystem__
#if !defined(__MIG_check__Request___asl_server_match_t__defined)
#define __MIG_check__Request___asl_server_match_t__defined

mig_internal kern_return_t __MIG_check__Request___asl_server_match_t(__attribute__((__unused__)) __Request___asl_server_match_t *In0P)
{

	typedef __Request___asl_server_match_t __Request;
#if	__MigTypeCheck
	if (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
	    (In0P->msgh_body.msgh_descriptor_count != 1) ||
	    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))
		return MIG_BAD_ARGUMENTS;
#endif	/* __MigTypeCheck */

#if	__MigTypeCheck
	if (In0P->request.type != MACH_MSG_OOL_DESCRIPTOR)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

#if __MigTypeCheck
	if (In0P->request.size != In0P->requestCnt)
		return MIG_TYPE_ERROR;
#endif	/* __MigTypeCheck */

	return MACH_MSG_SUCCESS;
}
#endif /* !defined(__MIG_check__Request___asl_server_match_t__defined) */
#endif /* __MIG_check__Request__asl_ipc_subsystem__ */
#endif /* ( __MigTypeCheck ) */


/* Routine _asl_server_match */
mig_internal novalue _X_asl_server_match
	(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)
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
		mach_msg_max_trailer_t trailer;
	} Request;
#ifdef  __MigPackStructs
#pragma pack()
#endif
	typedef __Request___asl_server_match_t __Request;
	typedef __Reply___asl_server_match_t Reply;

	/*
	 * typedef struct {
	 * 	mach_msg_header_t Head;
	 * 	NDR_record_t NDR;
	 * 	kern_return_t RetCode;
	 * } mig_reply_error_t;
	 */

	Request *In0P = (Request *) InHeadP;
	Reply *OutP = (Reply *) OutHeadP;
	mach_msg_max_trailer_t *TrailerP;
#if	__MigTypeCheck
	unsigned int trailer_size;
#endif	/* __MigTypeCheck */
#ifdef	__MIG_check__Request___asl_server_match_t__defined
	kern_return_t check_result;
#endif	/* __MIG_check__Request___asl_server_match_t__defined */

#if	UseStaticTemplates
	const static mach_msg_ool_descriptor_t replyTemplate = {
		.address = (void *)0,
		.size = 0,
		.deallocate = TRUE,
		.copy = MACH_MSG_VIRTUAL_COPY,
		.type = MACH_MSG_OOL_DESCRIPTOR,
	};
#endif	/* UseStaticTemplates */

	kern_return_t RetCode;
	__DeclareRcvRpc(122, "_asl_server_match")
	__BeforeRcvRpc(122, "_asl_server_match")
/* RetCArg=0x0 rtSimpleRequest=0 */

#if	defined(__MIG_check__Request___asl_server_match_t__defined)
	check_result = __MIG_check__Request___asl_server_match_t((__Request *)In0P);
	if (check_result != MACH_MSG_SUCCESS)
		{ MIG_RETURN_ERROR(OutP, check_result); }
#endif	/* defined(__MIG_check__Request___asl_server_match_t__defined) */

	TrailerP = (mach_msg_max_trailer_t *)((vm_offset_t)In0P +
		round_msg(In0P->Head.msgh_size));
	if (TrailerP->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0)
		{ MIG_RETURN_ERROR(In0P, MIG_TRAILER_ERROR); }
#if	__MigTypeCheck
	trailer_size = TrailerP->msgh_trailer_size -
		(mach_msg_size_t)(sizeof(mach_msg_trailer_type_t) - sizeof(mach_msg_trailer_size_t));
#endif	/* __MigTypeCheck */
#if	__MigTypeCheck
	if (trailer_size < (mach_msg_size_t)sizeof(audit_token_t))
		{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }
	trailer_size -= (mach_msg_size_t)sizeof(audit_token_t);
#endif	/* __MigTypeCheck */
#if	UseStaticTemplates
	OutP->reply = replyTemplate;
#else	/* UseStaticTemplates */
	OutP->reply.deallocate =  TRUE;
	OutP->reply.copy = MACH_MSG_VIRTUAL_COPY;
	OutP->reply.type = MACH_MSG_OOL_DESCRIPTOR;
#endif	/* UseStaticTemplates */


	RetCode = __asl_server_match(In0P->Head.msgh_request_port, (caddr_t)(In0P->request.address), In0P->requestCnt, In0P->startid, In0P->count, In0P->duration, In0P->direction, (caddr_t *)&(OutP->reply.address), &OutP->replyCnt, &OutP->lastid, &OutP->status, TrailerP->msgh_audit);
	if (RetCode != KERN_SUCCESS) {
		MIG_RETURN_ERROR(OutP, RetCode);
	}
	OutP->reply.size = OutP->replyCnt;


	OutP->NDR = NDR_record;


	OutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	OutP->Head.msgh_size = (sizeof(Reply));
	OutP->msgh_body.msgh_descriptor_count = 1;
	__AfterRcvRpc(122, "_asl_server_match")
}



/* Description of this subsystem, for use in direct RPC */
const struct _asl_ipc_subsystem _asl_ipc_subsystem = {
	asl_ipc_server_routine,
	114,
	123,
	(mach_msg_size_t)sizeof(union __ReplyUnion___asl_ipc_subsystem),
	(vm_address_t)0,
	{
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _X_asl_server_query, 13, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply___asl_server_query_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _X_asl_server_query_timeout, 19, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply___asl_server_query_timeout_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _X_asl_server_prune, 6, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply___asl_server_prune_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _X_asl_server_create_aux_link, 15, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply___asl_server_create_aux_link_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _X_asl_server_message, 11, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply___asl_server_message_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _X_asl_server_register_direct_watch, 10, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply___asl_server_register_direct_watch_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _X_asl_server_cancel_direct_watch, 10, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply___asl_server_cancel_direct_watch_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _X_asl_server_query_2, 19, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply___asl_server_query_2_t) },
          { (mig_impl_routine_t) 0,
            (mig_stub_routine_t) _X_asl_server_match, 21, 0, (routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply___asl_server_match_t) },
	}
};

mig_external boolean_t asl_ipc_server
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

	if ((InHeadP->msgh_id > 122) || (InHeadP->msgh_id < 114) ||
	    ((routine = _asl_ipc_subsystem.routine[InHeadP->msgh_id - 114].stub_routine) == 0)) {
		((mig_reply_error_t *)OutHeadP)->NDR = NDR_record;
		((mig_reply_error_t *)OutHeadP)->RetCode = MIG_BAD_ID;
		return FALSE;
	}
	(*routine) (InHeadP, OutHeadP);
	return TRUE;
}

mig_external mig_routine_t asl_ipc_server_routine
	(mach_msg_header_t *InHeadP)
{
	register int msgh_id;

	msgh_id = InHeadP->msgh_id - 114;

	if ((msgh_id > 8) || (msgh_id < 0))
		return 0;

	return _asl_ipc_subsystem.routine[msgh_id].stub_routine;
}

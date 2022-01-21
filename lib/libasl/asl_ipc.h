#ifndef	_asl_ipc_server_
#define	_asl_ipc_server_

/* Module asl_ipc */

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

#ifndef	asl_ipc_MSG_COUNT
#define	asl_ipc_MSG_COUNT	9
#endif	/* asl_ipc_MSG_COUNT */

#include <sys/mach/std_types.h>
#include <sys/mach/mig.h>
#include <sys/mach/thread_status.h>
#include <sys/mach/mig.h>
#include <sys/mach/mach_types.h>
#include <sys/types.h>

#ifdef __BeforeMigServerHeader
__BeforeMigServerHeader
#endif /* __BeforeMigServerHeader */


/* Routine _asl_server_query */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_query
#if	defined(LINTLIBRARY)
    (server, request, requestCnt, startid, count, flags, reply, replyCnt, lastid, status, token)
	mach_port_t server;
	caddr_t request;
	mach_msg_type_number_t requestCnt;
	uint64_t startid;
	int count;
	int flags;
	caddr_t *reply;
	mach_msg_type_number_t *replyCnt;
	uint64_t *lastid;
	int *status;
	security_token_t *token;
{ return __asl_server_query(server, request, requestCnt, startid, count, flags, reply, replyCnt, lastid, status, token); }
#else
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
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _asl_server_query_timeout */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_query_timeout
#if	defined(LINTLIBRARY)
    (server, request, requestCnt, startid, count, flags, reply, replyCnt, lastid, status, token)
	mach_port_t server;
	caddr_t request;
	mach_msg_type_number_t requestCnt;
	uint64_t startid;
	int count;
	int flags;
	caddr_t *reply;
	mach_msg_type_number_t *replyCnt;
	uint64_t *lastid;
	int *status;
	audit_token_t token;
{ return __asl_server_query_timeout(server, request, requestCnt, startid, count, flags, reply, replyCnt, lastid, status, token); }
#else
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
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _asl_server_prune */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_prune
#if	defined(LINTLIBRARY)
    (server, request, requestCnt, status, token)
	mach_port_t server;
	caddr_t request;
	mach_msg_type_number_t requestCnt;
	int *status;
	security_token_t *token;
{ return __asl_server_prune(server, request, requestCnt, status, token); }
#else
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	int *status,
	security_token_t *token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _asl_server_create_aux_link */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_create_aux_link
#if	defined(LINTLIBRARY)
    (server, message, messageCnt, fileport, url, urlCnt, status, token)
	mach_port_t server;
	caddr_t message;
	mach_msg_type_number_t messageCnt;
	mach_port_t *fileport;
	caddr_t *url;
	mach_msg_type_number_t *urlCnt;
	int *status;
	audit_token_t token;
{ return __asl_server_create_aux_link(server, message, messageCnt, fileport, url, urlCnt, status, token); }
#else
(
	mach_port_t server,
	caddr_t message,
	mach_msg_type_number_t messageCnt,
	mach_port_t *fileport,
	caddr_t *url,
	mach_msg_type_number_t *urlCnt,
	int *status,
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _asl_server_message */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_message
#if	defined(LINTLIBRARY)
    (server, message, messageCnt, token)
	mach_port_t server;
	caddr_t message;
	mach_msg_type_number_t messageCnt;
	audit_token_t token;
{ return __asl_server_message(server, message, messageCnt, token); }
#else
(
	mach_port_t server,
	caddr_t message,
	mach_msg_type_number_t messageCnt,
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _asl_server_register_direct_watch */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_register_direct_watch
#if	defined(LINTLIBRARY)
    (server, port, token)
	mach_port_t server;
	int port;
	audit_token_t token;
{ return __asl_server_register_direct_watch(server, port, token); }
#else
(
	mach_port_t server,
	int port,
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _asl_server_cancel_direct_watch */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_cancel_direct_watch
#if	defined(LINTLIBRARY)
    (server, port, token)
	mach_port_t server;
	int port;
	audit_token_t token;
{ return __asl_server_cancel_direct_watch(server, port, token); }
#else
(
	mach_port_t server,
	int port,
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _asl_server_query_2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_query_2
#if	defined(LINTLIBRARY)
    (server, request, requestCnt, startid, count, flags, reply, replyCnt, lastid, status, token)
	mach_port_t server;
	caddr_t request;
	mach_msg_type_number_t requestCnt;
	uint64_t startid;
	int count;
	int flags;
	caddr_t *reply;
	mach_msg_type_number_t *replyCnt;
	uint64_t *lastid;
	int *status;
	audit_token_t token;
{ return __asl_server_query_2(server, request, requestCnt, startid, count, flags, reply, replyCnt, lastid, status, token); }
#else
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
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _asl_server_match */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_match
#if	defined(LINTLIBRARY)
    (server, request, requestCnt, startid, count, duration, direction, reply, replyCnt, lastid, status, token)
	mach_port_t server;
	caddr_t request;
	mach_msg_type_number_t requestCnt;
	uint64_t startid;
	uint64_t count;
	uint32_t duration;
	int direction;
	caddr_t *reply;
	mach_msg_type_number_t *replyCnt;
	uint64_t *lastid;
	int *status;
	audit_token_t token;
{ return __asl_server_match(server, request, requestCnt, startid, count, duration, direction, reply, replyCnt, lastid, status, token); }
#else
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
	int *status,
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _asl_server_query */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_query
#if	defined(LINTLIBRARY)
    (server, request, requestCnt, startid, count, flags, reply, replyCnt, lastid, status, token)
	mach_port_t server;
	caddr_t request;
	mach_msg_type_number_t requestCnt;
	uint64_t startid;
	int count;
	int flags;
	caddr_t *reply;
	mach_msg_type_number_t *replyCnt;
	uint64_t *lastid;
	int *status;
	security_token_t *token;
{ return __asl_server_query(server, request, requestCnt, startid, count, flags, reply, replyCnt, lastid, status, token); }
#else
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
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _asl_server_query_timeout */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_query_timeout
#if	defined(LINTLIBRARY)
    (server, request, requestCnt, startid, count, flags, reply, replyCnt, lastid, status, token)
	mach_port_t server;
	caddr_t request;
	mach_msg_type_number_t requestCnt;
	uint64_t startid;
	int count;
	int flags;
	caddr_t *reply;
	mach_msg_type_number_t *replyCnt;
	uint64_t *lastid;
	int *status;
	audit_token_t token;
{ return __asl_server_query_timeout(server, request, requestCnt, startid, count, flags, reply, replyCnt, lastid, status, token); }
#else
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
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _asl_server_prune */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_prune
#if	defined(LINTLIBRARY)
    (server, request, requestCnt, status, token)
	mach_port_t server;
	caddr_t request;
	mach_msg_type_number_t requestCnt;
	int *status;
	security_token_t *token;
{ return __asl_server_prune(server, request, requestCnt, status, token); }
#else
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	int *status,
	security_token_t *token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _asl_server_create_aux_link */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_create_aux_link
#if	defined(LINTLIBRARY)
    (server, message, messageCnt, fileport, url, urlCnt, status, token)
	mach_port_t server;
	caddr_t message;
	mach_msg_type_number_t messageCnt;
	mach_port_t *fileport;
	caddr_t *url;
	mach_msg_type_number_t *urlCnt;
	int *status;
	audit_token_t token;
{ return __asl_server_create_aux_link(server, message, messageCnt, fileport, url, urlCnt, status, token); }
#else
(
	mach_port_t server,
	caddr_t message,
	mach_msg_type_number_t messageCnt,
	mach_port_t *fileport,
	caddr_t *url,
	mach_msg_type_number_t *urlCnt,
	int *status,
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _asl_server_message */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_message
#if	defined(LINTLIBRARY)
    (server, message, messageCnt, token)
	mach_port_t server;
	caddr_t message;
	mach_msg_type_number_t messageCnt;
	audit_token_t token;
{ return __asl_server_message(server, message, messageCnt, token); }
#else
(
	mach_port_t server,
	caddr_t message,
	mach_msg_type_number_t messageCnt,
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _asl_server_register_direct_watch */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_register_direct_watch
#if	defined(LINTLIBRARY)
    (server, port, token)
	mach_port_t server;
	int port;
	audit_token_t token;
{ return __asl_server_register_direct_watch(server, port, token); }
#else
(
	mach_port_t server,
	int port,
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _asl_server_cancel_direct_watch */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_cancel_direct_watch
#if	defined(LINTLIBRARY)
    (server, port, token)
	mach_port_t server;
	int port;
	audit_token_t token;
{ return __asl_server_cancel_direct_watch(server, port, token); }
#else
(
	mach_port_t server,
	int port,
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _asl_server_query_2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_query_2
#if	defined(LINTLIBRARY)
    (server, request, requestCnt, startid, count, flags, reply, replyCnt, lastid, status, token)
	mach_port_t server;
	caddr_t request;
	mach_msg_type_number_t requestCnt;
	uint64_t startid;
	int count;
	int flags;
	caddr_t *reply;
	mach_msg_type_number_t *replyCnt;
	uint64_t *lastid;
	int *status;
	audit_token_t token;
{ return __asl_server_query_2(server, request, requestCnt, startid, count, flags, reply, replyCnt, lastid, status, token); }
#else
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
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _asl_server_match */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t __asl_server_match
#if	defined(LINTLIBRARY)
    (server, request, requestCnt, startid, count, duration, direction, reply, replyCnt, lastid, status, token)
	mach_port_t server;
	caddr_t request;
	mach_msg_type_number_t requestCnt;
	uint64_t startid;
	uint64_t count;
	uint32_t duration;
	int direction;
	caddr_t *reply;
	mach_msg_type_number_t *replyCnt;
	uint64_t *lastid;
	int *status;
	audit_token_t token;
{ return __asl_server_match(server, request, requestCnt, startid, count, duration, direction, reply, replyCnt, lastid, status, token); }
#else
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
	int *status,
	audit_token_t token
);
#endif	/* defined(LINTLIBRARY) */

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
boolean_t asl_ipc_server(
		mach_msg_header_t *InHeadP,
		mach_msg_header_t *OutHeadP);

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
mig_routine_t asl_ipc_server_routine(
		mach_msg_header_t *InHeadP);


/* Description of this subsystem, for use in direct RPC */
extern const struct _asl_ipc_subsystem {
	mig_server_routine_t	server;	/* Server routine */
	mach_msg_id_t	start;	/* Min routine number */
	mach_msg_id_t	end;	/* Max routine number + 1 */
	unsigned int	maxsize;	/* Max msg size */
	vm_address_t	reserved;	/* Reserved */
	struct routine_descriptor	/*Array of routine descriptors */
		routine[9];
} _asl_ipc_subsystem;

/* typedefs for all requests */

#ifndef __Request__asl_ipc_subsystem__defined
#define __Request__asl_ipc_subsystem__defined

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
	} __Request___asl_server_query_t;
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
		mach_msg_ool_descriptor_t request;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t requestCnt;
		uint64_t startid;
		int count;
		int flags;
	} __Request___asl_server_query_timeout_t;
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
		mach_msg_ool_descriptor_t request;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t requestCnt;
	} __Request___asl_server_prune_t;
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
		mach_msg_ool_descriptor_t message;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t messageCnt;
	} __Request___asl_server_create_aux_link_t;
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
		mach_msg_ool_descriptor_t message;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t messageCnt;
	} __Request___asl_server_message_t;
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
		int port;
	} __Request___asl_server_register_direct_watch_t;
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
		int port;
	} __Request___asl_server_cancel_direct_watch_t;
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
		mach_msg_ool_descriptor_t request;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t requestCnt;
		uint64_t startid;
		int count;
		int flags;
	} __Request___asl_server_query_2_t;
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
		mach_msg_ool_descriptor_t request;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t requestCnt;
		uint64_t startid;
		uint64_t count;
		uint32_t duration;
		int direction;
	} __Request___asl_server_match_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__asl_ipc_subsystem__defined */


/* union of all requests */

#ifndef __RequestUnion___asl_ipc_subsystem__defined
#define __RequestUnion___asl_ipc_subsystem__defined
union __RequestUnion___asl_ipc_subsystem {
	__Request___asl_server_query_t Request__asl_server_query;
	__Request___asl_server_query_timeout_t Request__asl_server_query_timeout;
	__Request___asl_server_prune_t Request__asl_server_prune;
	__Request___asl_server_create_aux_link_t Request__asl_server_create_aux_link;
	__Request___asl_server_message_t Request__asl_server_message;
	__Request___asl_server_register_direct_watch_t Request__asl_server_register_direct_watch;
	__Request___asl_server_cancel_direct_watch_t Request__asl_server_cancel_direct_watch;
	__Request___asl_server_query_2_t Request__asl_server_query_2;
	__Request___asl_server_match_t Request__asl_server_match;
};
#endif /* __RequestUnion___asl_ipc_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__asl_ipc_subsystem__defined
#define __Reply__asl_ipc_subsystem__defined

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
	} __Reply___asl_server_query_t;
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
	} __Reply___asl_server_query_timeout_t;
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
	} __Reply___asl_server_prune_t;
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
	} __Reply___asl_server_create_aux_link_t;
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
	} __Reply___asl_server_message_t;
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
	} __Reply___asl_server_register_direct_watch_t;
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
	} __Reply___asl_server_cancel_direct_watch_t;
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
	} __Reply___asl_server_query_2_t;
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
	} __Reply___asl_server_match_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__asl_ipc_subsystem__defined */


/* union of all replies */

#ifndef __ReplyUnion___asl_ipc_subsystem__defined
#define __ReplyUnion___asl_ipc_subsystem__defined
union __ReplyUnion___asl_ipc_subsystem {
	__Reply___asl_server_query_t Reply__asl_server_query;
	__Reply___asl_server_query_timeout_t Reply__asl_server_query_timeout;
	__Reply___asl_server_prune_t Reply__asl_server_prune;
	__Reply___asl_server_create_aux_link_t Reply__asl_server_create_aux_link;
	__Reply___asl_server_message_t Reply__asl_server_message;
	__Reply___asl_server_register_direct_watch_t Reply__asl_server_register_direct_watch;
	__Reply___asl_server_cancel_direct_watch_t Reply__asl_server_cancel_direct_watch;
	__Reply___asl_server_query_2_t Reply__asl_server_query_2;
	__Reply___asl_server_match_t Reply__asl_server_match;
};
#endif /* __RequestUnion___asl_ipc_subsystem__defined */

#ifndef subsystem_to_name_map_asl_ipc
#define subsystem_to_name_map_asl_ipc \
    { "_asl_server_query", 114 },\
    { "_asl_server_query_timeout", 115 },\
    { "_asl_server_prune", 116 },\
    { "_asl_server_create_aux_link", 117 },\
    { "_asl_server_message", 118 },\
    { "_asl_server_register_direct_watch", 119 },\
    { "_asl_server_cancel_direct_watch", 120 },\
    { "_asl_server_query_2", 121 },\
    { "_asl_server_match", 122 }
#endif

#ifdef __AfterMigServerHeader
__AfterMigServerHeader
#endif /* __AfterMigServerHeader */

#endif	 /* _asl_ipc_server_ */

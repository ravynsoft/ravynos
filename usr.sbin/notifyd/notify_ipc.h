#ifndef	_notify_ipc_user_
#define	_notify_ipc_user_

/* Module notify_ipc */

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

#ifndef	notify_ipc_MSG_COUNT
#define	notify_ipc_MSG_COUNT	38
#endif	/* notify_ipc_MSG_COUNT */

#include <sys/mach/std_types.h>
#include <sys/mach/mig.h>
#include <sys/mach/mig.h>
#include <sys/mach/mach_types.h>
#include <sys/types.h>
#include "notify_ipc_types.h"

#ifdef __BeforeMigUserHeader
__BeforeMigUserHeader
#endif /* __BeforeMigUserHeader */

#include <sys/cdefs.h>
__BEGIN_DECLS


/* Routine _notify_server_post */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_post
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int *status;
{ return _notify_server_post(server, name, nameCnt, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_register_plain */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_register_plain
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, token, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int *token;
	int *status;
{ return _notify_server_register_plain(server, name, nameCnt, token, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int *token,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_register_check */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_register_check
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, size, slot, token, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int *size;
	int *slot;
	int *token;
	int *status;
{ return _notify_server_register_check(server, name, nameCnt, size, slot, token, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int *size,
	int *slot,
	int *token,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_register_signal */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_register_signal
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, sig, token, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int sig;
	int *token;
	int *status;
{ return _notify_server_register_signal(server, name, nameCnt, sig, token, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int sig,
	int *token,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_register_file_descriptor */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_register_file_descriptor
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, fileport, ntoken, token, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	mach_port_t fileport;
	int ntoken;
	int *token;
	int *status;
{ return _notify_server_register_file_descriptor(server, name, nameCnt, fileport, ntoken, token, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	mach_port_t fileport,
	int ntoken,
	int *token,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_register_mach_port */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_register_mach_port
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, port, ntoken, token, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	mach_port_t port;
	int ntoken;
	int *token;
	int *status;
{ return _notify_server_register_mach_port(server, name, nameCnt, port, ntoken, token, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	mach_port_t port,
	int ntoken,
	int *token,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_set_owner */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_set_owner
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, user, group, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int user;
	int group;
	int *status;
{ return _notify_server_set_owner(server, name, nameCnt, user, group, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int user,
	int group,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_get_owner */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_get_owner
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, user, group, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int *user;
	int *group;
	int *status;
{ return _notify_server_get_owner(server, name, nameCnt, user, group, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int *user,
	int *group,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_set_access */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_set_access
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, mode, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int mode;
	int *status;
{ return _notify_server_set_access(server, name, nameCnt, mode, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int mode,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_get_access */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_get_access
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, mode, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int *mode;
	int *status;
{ return _notify_server_get_access(server, name, nameCnt, mode, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int *mode,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_release_name */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_release_name
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int *status;
{ return _notify_server_release_name(server, name, nameCnt, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_cancel */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_cancel
#if	defined(LINTLIBRARY)
    (server, token, status)
	mach_port_t server;
	int token;
	int *status;
{ return _notify_server_cancel(server, token, status); }
#else
(
	mach_port_t server,
	int token,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_check */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_check
#if	defined(LINTLIBRARY)
    (server, token, check, status)
	mach_port_t server;
	int token;
	int *check;
	int *status;
{ return _notify_server_check(server, token, check, status); }
#else
(
	mach_port_t server,
	int token,
	int *check,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_get_state */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_get_state
#if	defined(LINTLIBRARY)
    (server, token, state, status)
	mach_port_t server;
	int token;
	uint64_t *state;
	int *status;
{ return _notify_server_get_state(server, token, state, status); }
#else
(
	mach_port_t server,
	int token,
	uint64_t *state,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_set_state */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_set_state
#if	defined(LINTLIBRARY)
    (server, token, state, status)
	mach_port_t server;
	int token;
	uint64_t state;
	int *status;
{ return _notify_server_set_state(server, token, state, status); }
#else
(
	mach_port_t server,
	int token,
	uint64_t state,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_monitor_file */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_monitor_file
#if	defined(LINTLIBRARY)
    (server, token, path, pathCnt, flags, status)
	mach_port_t server;
	int token;
	caddr_t path;
	mach_msg_type_number_t pathCnt;
	int flags;
	int *status;
{ return _notify_server_monitor_file(server, token, path, pathCnt, flags, status); }
#else
(
	mach_port_t server,
	int token,
	caddr_t path,
	mach_msg_type_number_t pathCnt,
	int flags,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_suspend */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_suspend
#if	defined(LINTLIBRARY)
    (server, token, status)
	mach_port_t server;
	int token;
	int *status;
{ return _notify_server_suspend(server, token, status); }
#else
(
	mach_port_t server,
	int token,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_resume */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_resume
#if	defined(LINTLIBRARY)
    (server, token, status)
	mach_port_t server;
	int token;
	int *status;
{ return _notify_server_resume(server, token, status); }
#else
(
	mach_port_t server,
	int token,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_suspend_pid */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_suspend_pid
#if	defined(LINTLIBRARY)
    (server, pid, status)
	mach_port_t server;
	int pid;
	int *status;
{ return _notify_server_suspend_pid(server, pid, status); }
#else
(
	mach_port_t server,
	int pid,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_resume_pid */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_resume_pid
#if	defined(LINTLIBRARY)
    (server, pid, status)
	mach_port_t server;
	int pid;
	int *status;
{ return _notify_server_resume_pid(server, pid, status); }
#else
(
	mach_port_t server,
	int pid,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _notify_server_simple_post */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_simple_post
#if	defined(LINTLIBRARY)
    (server, name, nameCnt)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
{ return _notify_server_simple_post(server, name, nameCnt); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_post_2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_post_2
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, name_id, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	uint64_t *name_id;
	int *status;
{ return _notify_server_post_2(server, name, nameCnt, name_id, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	uint64_t *name_id,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _notify_server_post_3 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_post_3
#if	defined(LINTLIBRARY)
    (server, name_id)
	mach_port_t server;
	uint64_t name_id;
{ return _notify_server_post_3(server, name_id); }
#else
(
	mach_port_t server,
	uint64_t name_id
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _notify_server_post_4 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_post_4
#if	defined(LINTLIBRARY)
    (server, name, nameCnt)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
{ return _notify_server_post_4(server, name, nameCnt); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _notify_server_register_plain_2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_register_plain_2
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, token)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int token;
{ return _notify_server_register_plain_2(server, name, nameCnt, token); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_register_check_2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_register_check_2
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, token, size, slot, name_id, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int token;
	int *size;
	int *slot;
	uint64_t *name_id;
	int *status;
{ return _notify_server_register_check_2(server, name, nameCnt, token, size, slot, name_id, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int token,
	int *size,
	int *slot,
	uint64_t *name_id,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _notify_server_register_signal_2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_register_signal_2
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, token, sig)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int token;
	int sig;
{ return _notify_server_register_signal_2(server, name, nameCnt, token, sig); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int token,
	int sig
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _notify_server_register_file_descriptor_2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_register_file_descriptor_2
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, token, fileport)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int token;
	mach_port_t fileport;
{ return _notify_server_register_file_descriptor_2(server, name, nameCnt, token, fileport); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int token,
	mach_port_t fileport
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _notify_server_register_mach_port_2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_register_mach_port_2
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, token, port)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int token;
	mach_port_t port;
{ return _notify_server_register_mach_port_2(server, name, nameCnt, token, port); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int token,
	mach_port_t port
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _notify_server_cancel_2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_cancel_2
#if	defined(LINTLIBRARY)
    (server, token)
	mach_port_t server;
	int token;
{ return _notify_server_cancel_2(server, token); }
#else
(
	mach_port_t server,
	int token
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_get_state_2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_get_state_2
#if	defined(LINTLIBRARY)
    (server, name_id, state, status)
	mach_port_t server;
	uint64_t name_id;
	uint64_t *state;
	int *status;
{ return _notify_server_get_state_2(server, name_id, state, status); }
#else
(
	mach_port_t server,
	uint64_t name_id,
	uint64_t *state,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_get_state_3 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_get_state_3
#if	defined(LINTLIBRARY)
    (server, token, state, nid, status)
	mach_port_t server;
	int token;
	uint64_t *state;
	uint64_t *nid;
	int *status;
{ return _notify_server_get_state_3(server, token, state, nid, status); }
#else
(
	mach_port_t server,
	int token,
	uint64_t *state,
	uint64_t *nid,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _notify_server_set_state_2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_set_state_2
#if	defined(LINTLIBRARY)
    (server, name_id, state)
	mach_port_t server;
	uint64_t name_id;
	uint64_t state;
{ return _notify_server_set_state_2(server, name_id, state); }
#else
(
	mach_port_t server,
	uint64_t name_id,
	uint64_t state
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_set_state_3 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_set_state_3
#if	defined(LINTLIBRARY)
    (server, token, state, nid, status)
	mach_port_t server;
	int token;
	uint64_t state;
	uint64_t *nid;
	int *status;
{ return _notify_server_set_state_3(server, token, state, nid, status); }
#else
(
	mach_port_t server,
	int token,
	uint64_t state,
	uint64_t *nid,
	int *status
);
#endif	/* defined(LINTLIBRARY) */

/* SimpleRoutine _notify_server_monitor_file_2 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_monitor_file_2
#if	defined(LINTLIBRARY)
    (server, token, path, pathCnt, flags)
	mach_port_t server;
	int token;
	caddr_t path;
	mach_msg_type_number_t pathCnt;
	int flags;
{ return _notify_server_monitor_file_2(server, token, path, pathCnt, flags); }
#else
(
	mach_port_t server,
	int token,
	caddr_t path,
	mach_msg_type_number_t pathCnt,
	int flags
);
#endif	/* defined(LINTLIBRARY) */

/* Routine _notify_server_regenerate */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _notify_server_regenerate
#if	defined(LINTLIBRARY)
    (server, name, nameCnt, token, reg_type, port, sig, prev_slot, prev_state, prev_time, path, pathCnt, path_flags, new_slot, new_name_id, status)
	mach_port_t server;
	caddr_t name;
	mach_msg_type_number_t nameCnt;
	int token;
	uint32_t reg_type;
	mach_port_t port;
	int sig;
	int prev_slot;
	uint64_t prev_state;
	uint64_t prev_time;
	caddr_t path;
	mach_msg_type_number_t pathCnt;
	int path_flags;
	int *new_slot;
	uint64_t *new_name_id;
	int *status;
{ return _notify_server_regenerate(server, name, nameCnt, token, reg_type, port, sig, prev_slot, prev_state, prev_time, path, pathCnt, path_flags, new_slot, new_name_id, status); }
#else
(
	mach_port_t server,
	caddr_t name,
	mach_msg_type_number_t nameCnt,
	int token,
	uint32_t reg_type,
	mach_port_t port,
	int sig,
	int prev_slot,
	uint64_t prev_state,
	uint64_t prev_time,
	caddr_t path,
	mach_msg_type_number_t pathCnt,
	int path_flags,
	int *new_slot,
	uint64_t *new_name_id,
	int *status
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

#ifndef __Request__notify_ipc_subsystem__defined
#define __Request__notify_ipc_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
	} __Request___notify_server_post_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
	} __Request___notify_server_register_plain_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
	} __Request___notify_server_register_check_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
		int sig;
	} __Request___notify_server_register_signal_t;
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
		mach_msg_ool_descriptor_t name;
		mach_msg_port_descriptor_t fileport;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
		int ntoken;
	} __Request___notify_server_register_file_descriptor_t;
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
		mach_msg_ool_descriptor_t name;
		mach_msg_port_descriptor_t port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
		int ntoken;
	} __Request___notify_server_register_mach_port_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
		int user;
		int group;
	} __Request___notify_server_set_owner_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
	} __Request___notify_server_get_owner_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
		int mode;
	} __Request___notify_server_set_access_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
	} __Request___notify_server_get_access_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
	} __Request___notify_server_release_name_t;
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
		int token;
	} __Request___notify_server_cancel_t;
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
		int token;
	} __Request___notify_server_check_t;
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
		int token;
	} __Request___notify_server_get_state_t;
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
		int token;
		uint64_t state;
	} __Request___notify_server_set_state_t;
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
		mach_msg_ool_descriptor_t path;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		int token;
		mach_msg_type_number_t pathCnt;
		int flags;
	} __Request___notify_server_monitor_file_t;
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
		int token;
	} __Request___notify_server_suspend_t;
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
		int token;
	} __Request___notify_server_resume_t;
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
		int pid;
	} __Request___notify_server_suspend_pid_t;
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
		int pid;
	} __Request___notify_server_resume_pid_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
	} __Request___notify_server_simple_post_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
	} __Request___notify_server_post_2_t;
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
		uint64_t name_id;
	} __Request___notify_server_post_3_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
	} __Request___notify_server_post_4_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
		int token;
	} __Request___notify_server_register_plain_2_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
		int token;
	} __Request___notify_server_register_check_2_t;
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
		mach_msg_ool_descriptor_t name;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
		int token;
		int sig;
	} __Request___notify_server_register_signal_2_t;
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
		mach_msg_ool_descriptor_t name;
		mach_msg_port_descriptor_t fileport;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
		int token;
	} __Request___notify_server_register_file_descriptor_2_t;
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
		mach_msg_ool_descriptor_t name;
		mach_msg_port_descriptor_t port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
		int token;
	} __Request___notify_server_register_mach_port_2_t;
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
		int token;
	} __Request___notify_server_cancel_2_t;
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
		uint64_t name_id;
	} __Request___notify_server_get_state_2_t;
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
		int token;
	} __Request___notify_server_get_state_3_t;
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
		uint64_t name_id;
		uint64_t state;
	} __Request___notify_server_set_state_2_t;
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
		int token;
		uint64_t state;
	} __Request___notify_server_set_state_3_t;
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
		mach_msg_ool_descriptor_t path;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		int token;
		mach_msg_type_number_t pathCnt;
		int flags;
	} __Request___notify_server_monitor_file_2_t;
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
		mach_msg_ool_descriptor_t name;
		mach_msg_port_descriptor_t port;
		mach_msg_ool_descriptor_t path;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t nameCnt;
		int token;
		uint32_t reg_type;
		int sig;
		int prev_slot;
		uint64_t prev_state;
		uint64_t prev_time;
		mach_msg_type_number_t pathCnt;
		int path_flags;
	} __Request___notify_server_regenerate_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__notify_ipc_subsystem__defined */

/* union of all requests */

#ifndef __RequestUnion__notify_ipc_subsystem__defined
#define __RequestUnion__notify_ipc_subsystem__defined
union __RequestUnion__notify_ipc_subsystem {
	__Request___notify_server_post_t Request__notify_server_post;
	__Request___notify_server_register_plain_t Request__notify_server_register_plain;
	__Request___notify_server_register_check_t Request__notify_server_register_check;
	__Request___notify_server_register_signal_t Request__notify_server_register_signal;
	__Request___notify_server_register_file_descriptor_t Request__notify_server_register_file_descriptor;
	__Request___notify_server_register_mach_port_t Request__notify_server_register_mach_port;
	__Request___notify_server_set_owner_t Request__notify_server_set_owner;
	__Request___notify_server_get_owner_t Request__notify_server_get_owner;
	__Request___notify_server_set_access_t Request__notify_server_set_access;
	__Request___notify_server_get_access_t Request__notify_server_get_access;
	__Request___notify_server_release_name_t Request__notify_server_release_name;
	__Request___notify_server_cancel_t Request__notify_server_cancel;
	__Request___notify_server_check_t Request__notify_server_check;
	__Request___notify_server_get_state_t Request__notify_server_get_state;
	__Request___notify_server_set_state_t Request__notify_server_set_state;
	__Request___notify_server_monitor_file_t Request__notify_server_monitor_file;
	__Request___notify_server_suspend_t Request__notify_server_suspend;
	__Request___notify_server_resume_t Request__notify_server_resume;
	__Request___notify_server_suspend_pid_t Request__notify_server_suspend_pid;
	__Request___notify_server_resume_pid_t Request__notify_server_resume_pid;
	__Request___notify_server_simple_post_t Request__notify_server_simple_post;
	__Request___notify_server_post_2_t Request__notify_server_post_2;
	__Request___notify_server_post_3_t Request__notify_server_post_3;
	__Request___notify_server_post_4_t Request__notify_server_post_4;
	__Request___notify_server_register_plain_2_t Request__notify_server_register_plain_2;
	__Request___notify_server_register_check_2_t Request__notify_server_register_check_2;
	__Request___notify_server_register_signal_2_t Request__notify_server_register_signal_2;
	__Request___notify_server_register_file_descriptor_2_t Request__notify_server_register_file_descriptor_2;
	__Request___notify_server_register_mach_port_2_t Request__notify_server_register_mach_port_2;
	__Request___notify_server_cancel_2_t Request__notify_server_cancel_2;
	__Request___notify_server_get_state_2_t Request__notify_server_get_state_2;
	__Request___notify_server_get_state_3_t Request__notify_server_get_state_3;
	__Request___notify_server_set_state_2_t Request__notify_server_set_state_2;
	__Request___notify_server_set_state_3_t Request__notify_server_set_state_3;
	__Request___notify_server_monitor_file_2_t Request__notify_server_monitor_file_2;
	__Request___notify_server_regenerate_t Request__notify_server_regenerate;
};
#endif /* !__RequestUnion__notify_ipc_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__notify_ipc_subsystem__defined
#define __Reply__notify_ipc_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		int status;
	} __Reply___notify_server_post_t;
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
		int token;
		int status;
	} __Reply___notify_server_register_plain_t;
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
		int size;
		int slot;
		int token;
		int status;
	} __Reply___notify_server_register_check_t;
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
		int token;
		int status;
	} __Reply___notify_server_register_signal_t;
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
		int token;
		int status;
	} __Reply___notify_server_register_file_descriptor_t;
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
		int token;
		int status;
	} __Reply___notify_server_register_mach_port_t;
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
	} __Reply___notify_server_set_owner_t;
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
		int user;
		int group;
		int status;
	} __Reply___notify_server_get_owner_t;
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
	} __Reply___notify_server_set_access_t;
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
		int mode;
		int status;
	} __Reply___notify_server_get_access_t;
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
	} __Reply___notify_server_release_name_t;
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
	} __Reply___notify_server_cancel_t;
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
		int check;
		int status;
	} __Reply___notify_server_check_t;
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
		uint64_t state;
		int status;
	} __Reply___notify_server_get_state_t;
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
	} __Reply___notify_server_set_state_t;
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
	} __Reply___notify_server_monitor_file_t;
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
	} __Reply___notify_server_suspend_t;
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
	} __Reply___notify_server_resume_t;
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
	} __Reply___notify_server_suspend_pid_t;
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
	} __Reply___notify_server_resume_pid_t;
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
	} __Reply___notify_server_simple_post_t;
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
		uint64_t name_id;
		int status;
	} __Reply___notify_server_post_2_t;
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
	} __Reply___notify_server_post_3_t;
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
	} __Reply___notify_server_post_4_t;
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
	} __Reply___notify_server_register_plain_2_t;
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
		int size;
		int slot;
		uint64_t name_id;
		int status;
	} __Reply___notify_server_register_check_2_t;
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
	} __Reply___notify_server_register_signal_2_t;
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
	} __Reply___notify_server_register_file_descriptor_2_t;
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
	} __Reply___notify_server_register_mach_port_2_t;
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
	} __Reply___notify_server_cancel_2_t;
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
		uint64_t state;
		int status;
	} __Reply___notify_server_get_state_2_t;
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
		uint64_t state;
		uint64_t nid;
		int status;
	} __Reply___notify_server_get_state_3_t;
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
	} __Reply___notify_server_set_state_2_t;
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
		uint64_t nid;
		int status;
	} __Reply___notify_server_set_state_3_t;
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
	} __Reply___notify_server_monitor_file_2_t;
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
		int new_slot;
		uint64_t new_name_id;
		int status;
	} __Reply___notify_server_regenerate_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__notify_ipc_subsystem__defined */

/* union of all replies */

#ifndef __ReplyUnion__notify_ipc_subsystem__defined
#define __ReplyUnion__notify_ipc_subsystem__defined
union __ReplyUnion__notify_ipc_subsystem {
	__Reply___notify_server_post_t Reply__notify_server_post;
	__Reply___notify_server_register_plain_t Reply__notify_server_register_plain;
	__Reply___notify_server_register_check_t Reply__notify_server_register_check;
	__Reply___notify_server_register_signal_t Reply__notify_server_register_signal;
	__Reply___notify_server_register_file_descriptor_t Reply__notify_server_register_file_descriptor;
	__Reply___notify_server_register_mach_port_t Reply__notify_server_register_mach_port;
	__Reply___notify_server_set_owner_t Reply__notify_server_set_owner;
	__Reply___notify_server_get_owner_t Reply__notify_server_get_owner;
	__Reply___notify_server_set_access_t Reply__notify_server_set_access;
	__Reply___notify_server_get_access_t Reply__notify_server_get_access;
	__Reply___notify_server_release_name_t Reply__notify_server_release_name;
	__Reply___notify_server_cancel_t Reply__notify_server_cancel;
	__Reply___notify_server_check_t Reply__notify_server_check;
	__Reply___notify_server_get_state_t Reply__notify_server_get_state;
	__Reply___notify_server_set_state_t Reply__notify_server_set_state;
	__Reply___notify_server_monitor_file_t Reply__notify_server_monitor_file;
	__Reply___notify_server_suspend_t Reply__notify_server_suspend;
	__Reply___notify_server_resume_t Reply__notify_server_resume;
	__Reply___notify_server_suspend_pid_t Reply__notify_server_suspend_pid;
	__Reply___notify_server_resume_pid_t Reply__notify_server_resume_pid;
	__Reply___notify_server_simple_post_t Reply__notify_server_simple_post;
	__Reply___notify_server_post_2_t Reply__notify_server_post_2;
	__Reply___notify_server_post_3_t Reply__notify_server_post_3;
	__Reply___notify_server_post_4_t Reply__notify_server_post_4;
	__Reply___notify_server_register_plain_2_t Reply__notify_server_register_plain_2;
	__Reply___notify_server_register_check_2_t Reply__notify_server_register_check_2;
	__Reply___notify_server_register_signal_2_t Reply__notify_server_register_signal_2;
	__Reply___notify_server_register_file_descriptor_2_t Reply__notify_server_register_file_descriptor_2;
	__Reply___notify_server_register_mach_port_2_t Reply__notify_server_register_mach_port_2;
	__Reply___notify_server_cancel_2_t Reply__notify_server_cancel_2;
	__Reply___notify_server_get_state_2_t Reply__notify_server_get_state_2;
	__Reply___notify_server_get_state_3_t Reply__notify_server_get_state_3;
	__Reply___notify_server_set_state_2_t Reply__notify_server_set_state_2;
	__Reply___notify_server_set_state_3_t Reply__notify_server_set_state_3;
	__Reply___notify_server_monitor_file_2_t Reply__notify_server_monitor_file_2;
	__Reply___notify_server_regenerate_t Reply__notify_server_regenerate;
};
#endif /* !__RequestUnion__notify_ipc_subsystem__defined */

#ifndef subsystem_to_name_map_notify_ipc
#define subsystem_to_name_map_notify_ipc \
    { "_notify_server_post", 78945668 },\
    { "_notify_server_register_plain", 78945669 },\
    { "_notify_server_register_check", 78945670 },\
    { "_notify_server_register_signal", 78945671 },\
    { "_notify_server_register_file_descriptor", 78945672 },\
    { "_notify_server_register_mach_port", 78945673 },\
    { "_notify_server_set_owner", 78945674 },\
    { "_notify_server_get_owner", 78945675 },\
    { "_notify_server_set_access", 78945676 },\
    { "_notify_server_get_access", 78945677 },\
    { "_notify_server_release_name", 78945678 },\
    { "_notify_server_cancel", 78945679 },\
    { "_notify_server_check", 78945680 },\
    { "_notify_server_get_state", 78945681 },\
    { "_notify_server_set_state", 78945682 },\
    { "_notify_server_monitor_file", 78945685 },\
    { "_notify_server_suspend", 78945686 },\
    { "_notify_server_resume", 78945687 },\
    { "_notify_server_suspend_pid", 78945688 },\
    { "_notify_server_resume_pid", 78945689 },\
    { "_notify_server_simple_post", 78945690 },\
    { "_notify_server_post_2", 78945691 },\
    { "_notify_server_post_3", 78945692 },\
    { "_notify_server_post_4", 78945693 },\
    { "_notify_server_register_plain_2", 78945694 },\
    { "_notify_server_register_check_2", 78945695 },\
    { "_notify_server_register_signal_2", 78945696 },\
    { "_notify_server_register_file_descriptor_2", 78945697 },\
    { "_notify_server_register_mach_port_2", 78945698 },\
    { "_notify_server_cancel_2", 78945699 },\
    { "_notify_server_get_state_2", 78945700 },\
    { "_notify_server_get_state_3", 78945701 },\
    { "_notify_server_set_state_2", 78945702 },\
    { "_notify_server_set_state_3", 78945703 },\
    { "_notify_server_monitor_file_2", 78945704 },\
    { "_notify_server_regenerate", 78945705 }
#endif

#ifdef __AfterMigUserHeader
__AfterMigUserHeader
#endif /* __AfterMigUserHeader */

#endif	 /* _notify_ipc_user_ */

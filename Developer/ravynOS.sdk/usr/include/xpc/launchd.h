#ifndef XPC_LAUNCHD_H_
#define XPC_LAUNCHD_H_
#include <xpc/xpc.h>

#define EXNOERROR	0
#define EXNOMEM		1
#define EXINVAL		2
#define EXSRCH      3
#define EXMAX		EXSRCH

const char *xpc_strerror(int error);
xpc_object_t xpc_copy_entitlement_for_token(const char *, audit_token_t *);
int xpc_pipe_routine_reply(xpc_object_t);
int xpc_pipe_try_receive(mach_port_t, xpc_object_t *, mach_port_t *,
    boolean_t (*)(mach_msg_header_t *, mach_msg_header_t *), mach_msg_size_t, int);
kern_return_t xpc_call_wakeup(mach_port_t, int);
void xpc_dictionary_get_audit_token(xpc_object_t, audit_token_t *);
void xpc_dictionary_set_mach_recv(xpc_object_t, const char *, mach_port_t);
void xpc_dictionary_set_mach_send(xpc_object_t, const char *, mach_port_t);
mach_port_t xpc_dictionary_copy_mach_send(xpc_object_t, const char *);
xpc_object_t xpc_copy_entitlements_for_pid(pid_t);
xpc_object_t ld2xpc(launch_data_t);

#endif

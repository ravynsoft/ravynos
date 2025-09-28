#ifndef	_host_priv_server_
#define	_host_priv_server_

/* Module host_priv */

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

#ifndef	host_priv_MSG_COUNT
#define	host_priv_MSG_COUNT	26
#endif	/* host_priv_MSG_COUNT */

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

#ifdef __BeforeMigServerHeader
__BeforeMigServerHeader
#endif /* __BeforeMigServerHeader */


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
extern const struct host_priv_subsystem {
	mig_server_routine_t	server;	/* Server routine */
	mach_msg_id_t	start;	/* Min routine number */
	mach_msg_id_t	end;	/* Max routine number + 1 */
	unsigned int	maxsize;	/* Max msg size */
	vm_address_t	reserved;	/* Reserved */
	struct routine_descriptor	/*Array of routine descriptors */
		routine[26];
} host_priv_subsystem;

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


/* union of all requests */

#ifndef __RequestUnion__host_priv_subsystem__defined
#define __RequestUnion__host_priv_subsystem__defined
union __RequestUnion__host_priv_subsystem {
	__Request__host_get_boot_info_t Request_host_get_boot_info;
	__Request__host_reboot_t Request_host_reboot;
	__Request__host_priv_statistics_t Request_host_priv_statistics;
	__Request__host_default_memory_manager_t Request_host_default_memory_manager;
	__Request__vm_wire_t Request_vm_wire;
	__Request__thread_wire_t Request_thread_wire;
	__Request__vm_allocate_cpm_t Request_vm_allocate_cpm;
	__Request__host_processors_t Request_host_processors;
	__Request__host_get_clock_control_t Request_host_get_clock_control;
	__Request__host_get_special_port_t Request_host_get_special_port;
	__Request__host_set_special_port_t Request_host_set_special_port;
	__Request__host_set_exception_ports_t Request_host_set_exception_ports;
	__Request__host_get_exception_ports_t Request_host_get_exception_ports;
	__Request__host_swap_exception_ports_t Request_host_swap_exception_ports;
	__Request__mach_vm_wire_t Request_mach_vm_wire;
	__Request__host_processor_sets_t Request_host_processor_sets;
	__Request__host_processor_set_priv_t Request_host_processor_set_priv;
	__Request__set_dp_control_port_t Request_set_dp_control_port;
	__Request__get_dp_control_port_t Request_get_dp_control_port;
	__Request__host_set_UNDServer_t Request_host_set_UNDServer;
	__Request__host_get_UNDServer_t Request_host_get_UNDServer;
	__Request__kext_request_t Request_kext_request;
};
#endif /* __RequestUnion__host_priv_subsystem__defined */
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

#ifndef subsystem_to_name_map_host_priv
#define subsystem_to_name_map_host_priv \
    { "host_get_boot_info", 400 },\
    { "host_reboot", 401 },\
    { "host_priv_statistics", 402 },\
    { "host_default_memory_manager", 403 },\
    { "vm_wire", 404 },\
    { "thread_wire", 405 },\
    { "vm_allocate_cpm", 406 },\
    { "host_processors", 407 },\
    { "host_get_clock_control", 408 },\
    { "host_get_special_port", 412 },\
    { "host_set_special_port", 413 },\
    { "host_set_exception_ports", 414 },\
    { "host_get_exception_ports", 415 },\
    { "host_swap_exception_ports", 416 },\
    { "mach_vm_wire", 418 },\
    { "host_processor_sets", 419 },\
    { "host_processor_set_priv", 420 },\
    { "set_dp_control_port", 421 },\
    { "get_dp_control_port", 422 },\
    { "host_set_UNDServer", 423 },\
    { "host_get_UNDServer", 424 },\
    { "kext_request", 425 }
#endif

#ifdef __AfterMigServerHeader
__AfterMigServerHeader
#endif /* __AfterMigServerHeader */

#endif	 /* _host_priv_server_ */

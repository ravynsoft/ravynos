#ifndef	_task_user_
#define	_task_user_

/* Module task */

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

#ifndef	task_MSG_COUNT
#define	task_MSG_COUNT	42
#endif	/* task_MSG_COUNT */

#include <sys/mach/std_types.h>
#include <sys/mach/mig.h>
#include <sys/mach/thread_status.h>
#include <sys/mach/mig.h>
#include <sys/mach/mach_types.h>
#include <sys/mach_debug/mach_debug_types.h>

#ifdef __BeforeMigUserHeader
__BeforeMigUserHeader
#endif /* __BeforeMigUserHeader */

#include <sys/cdefs.h>
__BEGIN_DECLS


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

/* Routine thread_create */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t thread_create
#if	defined(LINTLIBRARY)
    (parent_task, child_act)
	task_t parent_task;
	thread_act_t *child_act;
{ return thread_create(parent_task, child_act); }
#else
(
	task_t parent_task,
	thread_act_t *child_act
);
#endif	/* defined(LINTLIBRARY) */

/* Routine thread_create_running */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t thread_create_running
#if	defined(LINTLIBRARY)
    (parent_task, flavor, new_state, new_stateCnt, child_act)
	task_t parent_task;
	thread_state_flavor_t flavor;
	thread_state_t new_state;
	mach_msg_type_number_t new_stateCnt;
	thread_act_t *child_act;
{ return thread_create_running(parent_task, flavor, new_state, new_stateCnt, child_act); }
#else
(
	task_t parent_task,
	thread_state_flavor_t flavor,
	thread_state_t new_state,
	mach_msg_type_number_t new_stateCnt,
	thread_act_t *child_act
);
#endif	/* defined(LINTLIBRARY) */

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

/* Routine task_sample */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_sample
#if	defined(LINTLIBRARY)
    (task, reply)
	task_t task;
	mach_port_t reply;
{ return task_sample(task, reply); }
#else
(
	task_t task,
	mach_port_t reply
);
#endif	/* defined(LINTLIBRARY) */

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
	} __Request__thread_create_t;
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
	} __Request__thread_create_running_t;
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
		mach_msg_port_descriptor_t reply;
		/* end of the kernel processed data */
	} __Request__task_sample_t;
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

/* union of all requests */

#ifndef __RequestUnion__task_subsystem__defined
#define __RequestUnion__task_subsystem__defined
union __RequestUnion__task_subsystem {
	__Request__task_create_t Request_task_create;
	__Request__task_terminate_t Request_task_terminate;
	__Request__task_threads_t Request_task_threads;
	__Request__mach_ports_register_t Request_mach_ports_register;
	__Request__mach_ports_lookup_t Request_mach_ports_lookup;
	__Request__task_info_t Request_task_info;
	__Request__task_set_info_t Request_task_set_info;
	__Request__task_suspend_t Request_task_suspend;
	__Request__task_resume_t Request_task_resume;
	__Request__task_get_special_port_t Request_task_get_special_port;
	__Request__task_set_special_port_t Request_task_set_special_port;
	__Request__thread_create_t Request_thread_create;
	__Request__thread_create_running_t Request_thread_create_running;
	__Request__task_set_exception_ports_t Request_task_set_exception_ports;
	__Request__task_get_exception_ports_t Request_task_get_exception_ports;
	__Request__task_swap_exception_ports_t Request_task_swap_exception_ports;
	__Request__semaphore_create_t Request_semaphore_create;
	__Request__semaphore_destroy_t Request_semaphore_destroy;
	__Request__task_policy_set_t Request_task_policy_set;
	__Request__task_policy_get_t Request_task_policy_get;
	__Request__task_sample_t Request_task_sample;
	__Request__task_policy_t Request_task_policy;
	__Request__task_set_ras_pc_t Request_task_set_ras_pc;
	__Request__task_zone_info_t Request_task_zone_info;
	__Request__task_assign_t Request_task_assign;
	__Request__task_assign_default_t Request_task_assign_default;
	__Request__task_get_assignment_t Request_task_get_assignment;
	__Request__task_set_policy_t Request_task_set_policy;
	__Request__task_get_state_t Request_task_get_state;
	__Request__task_set_state_t Request_task_set_state;
	__Request__task_set_phys_footprint_limit_t Request_task_set_phys_footprint_limit;
	__Request__task_suspend2_t Request_task_suspend2;
	__Request__task_resume2_t Request_task_resume2;
	__Request__task_purgable_info_t Request_task_purgable_info;
	__Request__task_get_mach_voucher_t Request_task_get_mach_voucher;
	__Request__task_set_mach_voucher_t Request_task_set_mach_voucher;
	__Request__task_swap_mach_voucher_t Request_task_swap_mach_voucher;
};
#endif /* !__RequestUnion__task_subsystem__defined */
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
	} __Reply__thread_create_t;
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
	} __Reply__thread_create_running_t;
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
	} __Reply__task_sample_t;
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
	__Reply__thread_create_t Reply_thread_create;
	__Reply__thread_create_running_t Reply_thread_create_running;
	__Reply__task_set_exception_ports_t Reply_task_set_exception_ports;
	__Reply__task_get_exception_ports_t Reply_task_get_exception_ports;
	__Reply__task_swap_exception_ports_t Reply_task_swap_exception_ports;
	__Reply__semaphore_create_t Reply_semaphore_create;
	__Reply__semaphore_destroy_t Reply_semaphore_destroy;
	__Reply__task_policy_set_t Reply_task_policy_set;
	__Reply__task_policy_get_t Reply_task_policy_get;
	__Reply__task_sample_t Reply_task_sample;
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
#endif /* !__RequestUnion__task_subsystem__defined */

#ifndef subsystem_to_name_map_task
#define subsystem_to_name_map_task \
    { "task_create", 3400 },\
    { "task_terminate", 3401 },\
    { "task_threads", 3402 },\
    { "mach_ports_register", 3403 },\
    { "mach_ports_lookup", 3404 },\
    { "task_info", 3405 },\
    { "task_set_info", 3406 },\
    { "task_suspend", 3407 },\
    { "task_resume", 3408 },\
    { "task_get_special_port", 3409 },\
    { "task_set_special_port", 3410 },\
    { "thread_create", 3411 },\
    { "thread_create_running", 3412 },\
    { "task_set_exception_ports", 3413 },\
    { "task_get_exception_ports", 3414 },\
    { "task_swap_exception_ports", 3415 },\
    { "semaphore_create", 3418 },\
    { "semaphore_destroy", 3419 },\
    { "task_policy_set", 3420 },\
    { "task_policy_get", 3421 },\
    { "task_sample", 3422 },\
    { "task_policy", 3423 },\
    { "task_set_ras_pc", 3427 },\
    { "task_zone_info", 3428 },\
    { "task_assign", 3429 },\
    { "task_assign_default", 3430 },\
    { "task_get_assignment", 3431 },\
    { "task_set_policy", 3432 },\
    { "task_get_state", 3433 },\
    { "task_set_state", 3434 },\
    { "task_set_phys_footprint_limit", 3435 },\
    { "task_suspend2", 3436 },\
    { "task_resume2", 3437 },\
    { "task_purgable_info", 3438 },\
    { "task_get_mach_voucher", 3439 },\
    { "task_set_mach_voucher", 3440 },\
    { "task_swap_mach_voucher", 3441 }
#endif

#ifdef __AfterMigUserHeader
__AfterMigUserHeader
#endif /* __AfterMigUserHeader */

#endif	 /* _task_user_ */

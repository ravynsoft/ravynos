#ifndef SYS_MACH_THREAD_H_
#define SYS_MACH_THREAD_H_

#undef vm_map_remove
#include <sys/proc.h>
#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>

#include <sys/mach/task.h>



/*
 * thread_t->at_safe_point values
 */
#define NOT_AT_SAFE_POINT                0
#define SAFE_EXCEPTION_RETURN           -1
#define SAFE_BOOTSTRAP_RETURN           -2
#define SAFE_EXTERNAL_RECEIVE           -3
#define SAFE_THR_DEPRESS                -4
#define SAFE_SUSPENDED                  -5
#define SAFE_MISCELLANEOUS              -6
#define SAFE_INTERNAL_RECEIVE           -7



#define disable_preemption() critical_enter()
#define enable_preemption() critical_exit()


/* XXX move me */
struct thread_shuttle {

	/* IPC data structures */
	struct thread_shuttle *ith_next, *ith_prev, *ith_pool_next;

	struct ipc_kmsg_queue ith_messages;

	struct mtx ith_lock_data;
	struct mtx *ith_block_lock_data;
	mach_port_t     ith_mig_reply;  /* reply port for mig */
	struct ipc_port *ith_rpc_reply; /* reply port for kernel RPCs */
	uint32_t timeout;
	uint32_t sleep_stamp;

		/* Various bits of stashed state */
	union {
		struct {
			mach_msg_return_t	state;		/* receive state */
			mach_port_seqno_t	seqno;		/* seqno of recvd message */
		  	ipc_object_t		object;		/* object received on */
		  	mach_vm_address_t	msg_addr;	/* receive buffer pointer */
			mach_msg_size_t		msize;		/* max size for recvd msg */
		  	mach_msg_option_t	option;		/* options for receive */
		  	mach_msg_size_t		slist_size;	/* scatter list size */
			mach_port_name_t	receiver_name;	/* the receive port name */
			struct ipc_kmsg		*kmsg;		/* received message */
			mach_msg_body_t *scatter_list;
			mach_msg_size_t scatter_list_size;
		} receive;
		char *other;		/* catch-all for other state */
	} saved;
	kern_return_t	wait_result;	/* outcome of wait -
					   may be examined by this thread
					   WITHOUT locking */
	struct callout *timer;		/* timer for thread */
	struct thread *ith_td;

	/* Special ports attached to this activation */
	struct ipc_port *ith_self;	/* not a right, doesn't hold ref */
	struct ipc_port *ith_sself;	/* a send right */
	struct exception_action exc_actions[EXC_TYPES_COUNT];
	int		ref_count;	/* number of references to me */
	int ith_active;


#define	ith_wait_result		wait_result

#define	ith_option		saved.receive.option
#define ith_scatter_list	saved.receive.scatter_list
#define ith_scatter_list_size	saved.receive.scatter_list_size
#define ith_state		saved.receive.state
#define ith_object		saved.receive.object
#define ith_msg_addr			saved.receive.msg_addr
#define ith_msize		saved.receive.msize
#define ith_receiver_name	saved.receive.receiver_name
#define ith_kmsg		saved.receive.kmsg
#define ith_seqno		saved.receive.seqno

#define ith_other		saved.other

#if 0


	/*
	 * Beginning of thread_shuttle proper
	 */
	queue_chain_t	links;		/* current run queue links */
	run_queue_t	runq;		/* run queue p is on SEE BELOW */
	int		whichq;		/* which queue level p is on */
/*
 *	NOTE:	The runq field in the thread structure has an unusual
 *	locking protocol.  If its value is RUN_QUEUE_NULL, then it is
 *	locked by the thread_lock, but if its value is something else
 *	(i.e. a run_queue) then it is locked by that run_queue's lock.
 */

	/* Thread bookkeeping */
	queue_chain_t	pset_threads;	/* list of all shuttles in proc set */

	/* Self-preservation */
	decl_simple_lock_data(,lock)	/* scheduling lock (thread_lock()) */
	decl_simple_lock_data(,w_lock)  /* covers wake_active (wake_lock())*/
	decl_mutex_data(,rpc_lock)	/* RPC lock (rpc_lock()) */
	int		ref_count;	/* number of references to me */
	
	vm_offset_t     kernel_stack;   /* accurate only if the thread is 
					   not swapped and not executing */

	vm_offset_t	stack_privilege;/* reserved kernel stack */

	/* Blocking information */
	int		reason;		/* why we blocked */
	event_t		wait_event;	/* event we are waiting on */
	kern_return_t	wait_result;	/* outcome of wait -
					   may be examined by this thread
					   WITHOUT locking */
	queue_chain_t	wait_link;	/* event's wait queue link */
	boolean_t	wake_active;	/* Someone is waiting for this
					   thread to become suspended */
	int		state;		/* Thread state: */

/*
 *	Thread states [bits or'ed]
 */
#define TH_WAIT			0x01	/* thread is queued for waiting */
#define TH_SUSP			0x02	/* thread has been asked to stop */
#define TH_RUN			0x04	/* thread is running or on runq */
#define TH_UNINT		0x08	/* thread is waiting uninteruptibly */
#define	TH_HALTED		0x10	/* thread is halted at clean point ? */

#define TH_ABORT		0x20	/* abort interruptible waits */
#define TH_SWAPPED_OUT		0x40	/* thread is swapped out */

#define TH_IDLE			0x80	/* thread is an idle thread */

#define	TH_SCHED_STATE	(TH_WAIT|TH_SUSP|TH_RUN|TH_UNINT)

/* These two flags will never be seen and might well be removed */
#define	TH_STACK_HANDOFF	0x0100	/* thread has no kernel stack */
#define	TH_STACK_COMING_IN	0x0200	/* thread is waiting for kernel stack */
#define	TH_STACK_STATE	(TH_STACK_HANDOFF | TH_STACK_COMING_IN)

	int		preempt;	/* Thread preemption status */
#define	TH_PREEMPTABLE		0	/* Thread is preemptable */
#define	TH_NOT_PREEMPTABLE	1	/* Thread is not preemptable */
#define	TH_PREEMPTED		2	/* Thread has been preempted */

#if	ETAP_EVENT_MONITOR
	int		etap_reason;	/* real reason why we blocked */
	boolean_t	etap_trace;	/* ETAP trace status */
#endif	/* ETAP_EVENT_MONITOR */


	/* Stack handoff information */
	void		(*continuation)(/* start here next time runnable */
				void);

	/* Scheduling information */
	int		priority;	/* thread's priority */
	int		max_priority;	/* maximum priority */
	int		sched_pri;	/* scheduled (computed) priority */
	int		sched_data;	/* for use by policy */
	int		policy;		/* scheduling policy */
	int		depress_priority; /* depressed from this priority */
	unsigned int	cpu_usage;	/* exp. decaying cpu usage [%cpu] */
	unsigned int	sched_usage;	/* load-weighted cpu usage [sched] */
	unsigned int	sched_stamp;	/* last time priority was updated */
	unsigned int	sleep_stamp;	/* last time in TH_WAIT state */
	unsigned int	sched_change_stamp;
					/* last time priority or policy was
					   explicitly changed (not the same
					   units as sched_stamp!) */
	int		unconsumed_quantum;	/* leftover quantum (RR/FIFO) */

	/* VM global variables */
	boolean_t	vm_privilege;	/* can use reserved memory? */
	vm_offset_t	recover;	/* page fault recovery (copyin/out) */

	/* IPC data structures */
	struct thread_shuttle *ith_next, *ith_prev;

	struct ipc_kmsg_queue ith_messages;

	decl_mutex_data(,ith_lock_data)
	mach_port_t	ith_mig_reply;	/* reply port for mig */
	struct ipc_port	*ith_rpc_reply;	/* reply port for kernel RPCs */

	/* Various bits of stashed state */
	union {
		struct {
			mach_msg_option_t option;
			mach_msg_body_t *scatter_list;
			mach_msg_size_t scatter_list_size;
		} receive;
		char *other;		/* catch-all for other state */
	} saved;

	/* Timing data structures */
	timer_data_t	user_timer;	/* user mode timer */
	timer_data_t	system_timer;	/* system mode timer */
	timer_data_t	depressed_timer;/* depressed priority timer */
	timer_save_data_t user_timer_save;  /* saved user timer value */
	timer_save_data_t system_timer_save;  /* saved sys timer val. */
	unsigned int	cpu_delta;	/* cpu usage since last update */
	unsigned int	sched_delta;	/* weighted cpu usage since update */

	/* Time-outs */
	timer_elt_data_t timer;		/* timer for thread */
	timer_elt_data_t depress_timer;	/* timer for priority depression */

	/* Ast/Halt data structures */
	boolean_t	active;		/* how alive is the thread */

	/* Processor data structures */
	processor_set_t	processor_set;	/* assigned processor set */
#if	NCPUS > 1
	processor_t	bound_processor;	/* bound to processor ?*/
#endif	/* NCPUS > 1 */
#if	MACH_HOST
	boolean_t	may_assign;	/* may assignment change? */
	boolean_t	assign_active;	/* someone waiting for may_assign */
#endif	/* MACH_HOST */

#if	XKMACHKERNEL
	int		xk_type;
#endif	/* XKMACHKERNEL */

#if	NCPUS > 1
	processor_t	last_processor; /* processor this last ran on */
#if	MACH_LOCK_MON
	unsigned	lock_stack;	/* number of locks held */
#endif  /* MACH_LOCK_MON */o
#endif	/* NCPUS > 1 */

	int		at_safe_point;	/* thread_abort_safely allowed */

#if	MACH_LDEBUG
	/*
	 *	Debugging:  track acquired mutexes and locks.
	 *	Because a thread can block while holding such
	 *	synchronizers, we think of the thread as
	 *	"owning" them.
	 */
#define	MUTEX_STACK_DEPTH	20
#define	LOCK_STACK_DEPTH	20
	mutex_t		*mutex_stack[MUTEX_STACK_DEPTH];
	lock_t		*lock_stack[LOCK_STACK_DEPTH];
	unsigned int	mutex_stack_index;
	unsigned int	lock_stack_index;
	unsigned	mutex_count;	/* XXX to be deleted XXX */
	boolean_t	kthread;	/* thread is a kernel thread */
#endif	/* MACH_LDEBUG */

#if	LINUX_DEV
	/*
	 * State for Linux device drivers
	 */
	void            *linux_data;    /* used by linuxdev/mach/glue/block.c */
#endif	/* LINUX_DEV */

	/*
	 * End of thread_shuttle proper
	 */

	/*
	 * Migration and thread_activation linkage information
	 */
	struct thread_activation *top_act; /* "current" thr_act */

} Thread_Shuttle;


/* typedef of thread_t is in kern/kern_types.h */
typedef struct thread_shuttle	*thread_shuttle_t;
#define THREAD_NULL		((thread_t) 0)
#define THREAD_SHUTTLE_NULL	((thread_shuttle_t)0)


/*
 * thread_t->at_safe_point values
 */
#define NOT_AT_SAFE_POINT		 0
#define SAFE_EXCEPTION_RETURN		-1
#define SAFE_BOOTSTRAP_RETURN		-2
#define SAFE_EXTERNAL_RECEIVE		-3
#define SAFE_THR_DEPRESS		-4
#define SAFE_SUSPENDED			-5
#define SAFE_MISCELLANEOUS		-6
#define SAFE_INTERNAL_RECEIVE		-7

extern thread_act_t active_kloaded[NCPUS];	/* "" kernel-loaded acts */
extern vm_offset_t active_stacks[NCPUS];	/* active kernel stacks */
extern vm_offset_t kernel_stack[NCPUS];

#ifndef MACHINE_STACK_STASH
/*
 * MD Macro to fill up global stack state,
 * keeping the MD structure sizes + games private
 */
#define MACHINE_STACK_STASH(stack)					\
MACRO_BEGIN								\
	mp_disable_preemption();					\
	active_stacks[cpu_number()] = (stack),				\
	    kernel_stack[cpu_number()] = (stack) + KERNEL_STACK_SIZE;	\
	mp_enable_preemption();						\
MACRO_END
#endif	/* MACHINE_STACK_STASH */

/*
 *	Kernel-only routines
 */

/* Initialize thread module */
extern void		thread_init(void);

/* Take reference on thread (make sure it doesn't go away) */
extern void		thread_reference(
				thread_t	th);

/* Release reference on thread */
extern void		thread_deallocate(
				thread_t	th);

/* Set priority of calling thread */
extern void		thread_set_own_priority(
				int		priority);
/* Reset thread's priority */
extern kern_return_t thread_priority(
				thread_act_t	thr_act,
				int		priority,
				boolean_t	set_max);

/* Reset thread's max priority */
extern kern_return_t	thread_max_priority(
				thread_act_t	thr_act,
				processor_set_t	pset,
				int		max_priority);

/* Reset thread's max priority while holding RPC locks */
extern kern_return_t	thread_max_priority_locked(
				thread_t	thread,
				processor_set_t	pset,
				int		max_priority);

/* Set a thread's priority while holding RPC locks */
extern kern_return_t	thread_priority_locked(
				thread_t	thread,
				int		priority,
				boolean_t	set_max);

/* Start a thread at specified routine */
#define thread_start(thread, start) (thread)->continuation = (start)

/* Create Kernel mode thread */
extern thread_t		kernel_thread(
				task_t	task,
				void	(*start)(void),
				void	*arg);

/* Reaps threads waiting to be destroyed */
extern void		reaper_thread(void);

#if	MACH_HOST
/* Preclude thread processor set assignement */
extern void		thread_freeze(
				thread_t	th);

/* Assign thread to a processor set */
extern void		thread_doassign(
				thread_t		thread,
				processor_set_t		new_pset,
				boolean_t		release_freeze);

/* Allow thread processor set assignement */
extern void		thread_unfreeze(
				thread_t	th);

#endif	/* MACH_HOST */

/* Insure thread always has a kernel stack */
extern void		stack_privilege(
				thread_t	th);

extern void		consider_thread_collect(void);

/*
 *	Arguments to specify aggressiveness to thread halt.
 *	Can't have MUST_HALT and SAFELY at the same time.
 */
#define	THREAD_HALT_NORMAL	0
#define	THREAD_HALT_MUST_HALT	1	/* no deadlock checks */
#define	THREAD_HALT_SAFELY	2	/* result must be restartable */

/*
 *	Macro-defined routines
 */

#define	thread_lock_init(th)	simple_lock_init(&(th)->lock, ETAP_THREAD_LOCK)
#define thread_lock(th)		simple_lock(&(th)->lock)
#define thread_unlock(th)	simple_unlock(&(th)->lock)

#define thread_should_halt(thread)	\
	(!(thread)->top_act || \
	!(thread)->top_act->active || \
	(thread)->top_act->ast & (AST_HALT|AST_TERMINATE))

/*
 * We consider a thread not preempt
 */
 /*
	 * Beginning of thread_shuttle proper
	 */
	queue_chain_t	links;		/* current run queue links */
	run_queue_t	runq;		/* run queue p is on SEE BELOW */
	int		whichq;		/* which queue level p is on */
/*
 *	NOTE:	The runq field in the thread structure has an unusual
 *	locking protocol.  If its value is RUN_QUEUE_NULL, then it is
 *	locked by the thread_lock, but if its value is something else
 *	(i.e. a run_queue) then it is locked by that run_queue's lock.
 */

	/* Thread bookkeeping */
	queue_chain_t	pset_threads;	/* list of all shuttles in proc set */

	/* Self-preservation */
	decl_simple_lock_data(,lock)	/* scheduling lock (thread_lock()) */
	decl_simple_lock_data(,w_lock)  /* covers wake_active (wake_lock())*/
	decl_mutex_data(,rpc_lock)	/* RPC lock (rpc_lock()) */
	
	vm_offset_t     kernel_stack;   /* accurate only if the thread is 
					   not swapped and not executing */

	vm_offset_t	stack_privilege;/* reserved kernel stack */

	/* Blocking information */
	int		reason;		/* why we blocked */
	event_t		wait_event;	/* event we are waiting on */
	kern_return_t	wait_result;	/* outcome of wait -
					   may be examined by this thread
					   WITHOUT locking */
	queue_chain_t	wait_link;	/* event's wait queue link */
	boolean_t	wake_active;	/* Someone is waiting for this
					   thread to become suspended */
	int		state;		/* Thread state: */

/*
 *	Thread states [bits or'ed]
 */
#define TH_WAIT			0x01	/* thread is queued for waiting */
#define TH_SUSP			0x02	/* thread has been asked to stop */
#define TH_RUN			0x04	/* thread is running or on runq */
#define TH_UNINT		0x08	/* thread is waiting uninteruptibly */
#define	TH_HALTED		0x10	/* thread is halted at clean point ? */

#define TH_ABORT		0x20	/* abort interruptible waits */
#define TH_SWAPPED_OUT		0x40	/* thread is swapped out */

#define TH_IDLE			0x80	/* thread is an idle thread */

#define	TH_SCHED_STATE	(TH_WAIT|TH_SUSP|TH_RUN|TH_UNINT)

/* These two flags will never be seen and might well be removed */
#define	TH_STACK_HANDOFF	0x0100	/* thread has no kernel stack */
#define	TH_STACK_COMING_IN	0x0200	/* thread is waiting for kernel stack */
#define	TH_STACK_STATE	(TH_STACK_HANDOFF | TH_STACK_COMING_IN)

	int		preempt;	/* Thread preemption status */
#define	TH_PREEMPTABLE		0	/* Thread is preemptable */
#define	TH_NOT_PREEMPTABLE	1	/* Thread is not preemptable */
#define	TH_PREEMPTED		2	/* Thread has been preempted */

#if	ETAP_EVENT_MONITOR
	int		etap_reason;	/* real reason why we blocked */
	boolean_t	etap_trace;	/* ETAP trace status */
#endif	/* ETAP_EVENT_MONITOR */


	/* Stack handoff information */
	void		(*continuation)(/* start here next time runnable */
				void);

	/* Scheduling information */
	int		priority;	/* thread's priority */
	int		max_priority;	/* maximum priority */
	int		sched_pri;	/* scheduled (computed) priority */
	int		sched_data;	/* for use by policy */
	int		policy;		/* scheduling policy */
	int		depress_priority; /* depressed from this priority */
	unsigned int	cpu_usage;	/* exp. decaying cpu usage [%cpu] */
	unsigned int	sched_usage;	/* load-weighted cpu usage [sched] */
	unsigned int	sched_stamp;	/* last time priority was updated */
	unsigned int	sleep_stamp;	/* last time in TH_WAIT state */
	unsigned int	sched_change_stamp;
					/* last time priority or policy was
					   explicitly changed (not the same
					   units as sched_stamp!) */
	int		unconsumed_quantum;	/* leftover quantum (RR/FIFO) */

	/* VM global variables */
	boolean_t	vm_privilege;	/* can use reserved memory? */
	vm_offset_t	recover;	/* page fault recovery (copyin/out) */

	/* IPC data structures */
	struct thread_shuttle *ith_next, *ith_prev;

	struct ipc_kmsg_queue ith_messages;

	decl_mutex_data(,ith_lock_data)
	mach_port_t	ith_mig_reply;	/* reply port for mig */
	struct ipc_port	*ith_rpc_reply;	/* reply port for kernel RPCs */

	/* Various bits of stashed state */
	union {
		struct {
			mach_msg_option_t option;
			mach_msg_body_t *scatter_list;
			mach_msg_size_t scatter_list_size;
		} receive;
		char *other;		/* catch-all for other state */
	} saved;

	/* Timing data structures */
	timer_data_t	user_timer;	/* user mode timer */
	timer_data_t	system_timer;	/* system mode timer */
	timer_data_t	depressed_timer;/* depressed priority timer */
	timer_save_data_t user_timer_save;  /* saved user timer value */
	timer_save_data_t system_timer_save;  /* saved sys timer val. */
	unsigned int	cpu_delta;	/* cpu usage since last update */
	unsigned int	sched_delta;	/* weighted cpu usage since update */

	/* Time-outs */
	timer_elt_data_t timer;		/* timer for thread */
	timer_elt_data_t depress_timer;	/* timer for priority depression */

	/* Ast/Halt data structures */
	boolean_t	active;		/* how alive is the thread */

	/* Processor data structures */
	processor_set_t	processor_set;	/* assigned processor set */
#if	NCPUS > 1
	processor_t	bound_processor;	/* bound to processor ?*/
#endif	/* NCPUS > 1 */
#if	MACH_HOST
	boolean_t	may_assign;	/* may assignment change? */
	boolean_t	assign_active;	/* someone waiting for may_assign */
#endif	/* MACH_HOST */

#if	XKMACHKERNEL
	int		xk_type;
#endif	/* XKMACHKERNEL */

#if	NCPUS > 1
	processor_t	last_processor; /* processor this last ran on */
#if	MACH_LOCK_MON
	unsigned	lock_stack;	/* number of locks held */
#endif  /* MACH_LOCK_MON */
#endif	/* NCPUS > 1 */

	int		at_safe_point;	/* thread_abort_safely allowed */

#if	MACH_LDEBUG
	/*
	 *	Debugging:  track acquired mutexes and locks.
	 *	Because a thread can block while holding such
	 *	synchronizers, we think of the thread as
	 *	"owning" them.
	 */
#define	MUTEX_STACK_DEPTH	20
#define	LOCK_STACK_DEPTH	20
	mutex_t		*mutex_stack[MUTEX_STACK_DEPTH];
	lock_t		*lock_stack[LOCK_STACK_DEPTH];
	unsigned int	mutex_stack_index;
	unsigned int	lock_stack_index;
	unsigned	mutex_count;	/* XXX to be deleted XXX */
	boolean_t	kthread;	/* thread is a kernel thread */
#endif	/* MACH_LDEBUG */

#if	LINUX_DEV
	/*
	 * State for Linux device drivers
	 */
	void            *linux_data;    /* used by linuxdev/mach/glue/block.c */
#endif	/* LINUX_DEV */

	/*
	 * End of thread_shuttle proper
	 */

	/*
	 * Migration and thread_activation linkage information
	 */
	struct thread_activation *top_act; /* "current" thr_act */


#endif


};

#define	ith_wait_result		wait_result

#define	ith_option		saved.receive.option
#define ith_scatter_list	saved.receive.scatter_list
#define ith_scatter_list_size	saved.receive.scatter_list_size
#define ith_task		ith_td->td_proc->p_machdata
#define ith_map			ith_td->td_proc->p_vmspace->vm_map
#define ith_other		saved.other

#define thread_map(thread) (&(thread)->ith_td->td_proc->p_vmspace->vm_map)

static __inline thread_t
current_thread(void)
{

	return (curthread->td_machdata);
}


static __inline task_t
current_task(void)
{

	return (curthread->td_proc->p_machdata);
}

#define current_space() current_task()->itk_space

#define current_act() current_thread()

static __inline vm_map_t
current_map(void)
{

	return (&curthread->td_proc->p_vmspace->vm_map);
}

#define act_deallocate(act) thread_deallocate(act)
#define act_lock(act)  mtx_lock(&(act)->ith_lock_data)
#define act_lock_try(act) mtx_trylock(&(act)->ith_lock_data)
#define act_unlock(act) mtx_unlock(&act->ith_lock_data)
#define act_locked_act_reference(act) ((act)->ref_count++)


/* wakeup a thread */
extern void thread_go(thread_t);
extern void thread_will_wait_with_timeout(thread_t, int);
extern void thread_will_wait(thread_t);
extern void thread_block(void);
#if 0
extern void assert_wait(event_t, boolean_t);
#else
#define assert_wait(a, b)
#endif
/*
 *	Possible results of assert_wait - returned in
 *	current_thread()->wait_result.
 */
#define	THREAD_WAITING		-1
#define THREAD_AWAKENED		0		/* normal wakeup */
#define THREAD_TIMED_OUT	1		/* timeout expired */
#define THREAD_INTERRUPTED	2		/* interrupted by clear_wait */
#define THREAD_RESTART		3		/* restart operation entirely */
#define	THREAD_NOT_WAITING		10


#endif

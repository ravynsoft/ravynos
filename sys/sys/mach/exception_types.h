#ifndef _MACH_EXCEPTION_TYPES_H_
#define _MACH_EXCEPTION_TYPES_H_

#if defined(__i386__) || defined(__amd64__) || defined(__arm64__) || defined(__aarch64__)
#define EXC_TYPES_COUNT 13
#else
#define EXC_TYPES_COUNT 2
#endif

#define	EXC_MASK_MACHINE	0

/*
 *      Machine-independent exception definitions.
 */

#define EXC_BAD_ACCESS          1       /* Could not access memory */
                /* Code contains kern_return_t describing error. */
                /* Subcode contains bad memory address. */

#define EXC_BAD_INSTRUCTION     2       /* Instruction failed */
                /* Illegal or undefined instruction or operand */

#define EXC_ARITHMETIC          3       /* Arithmetic exception */
                /* Exact nature of exception is in code field */

#define EXC_EMULATION           4       /* Emulation instruction */
                /* Emulation support instruction encountered */
                /* Details in code and subcode fields   */

#define EXC_SOFTWARE            5       /* Software generated exception */
                /* Exact exception is in code field. */
                /* Codes 0 - 0xFFFF reserved to hardware */
                /* Codes 0x10000 - 0x1FFFF reserved for OS emulation (Unix) */

#define EXC_BREAKPOINT          6       /* Trace, breakpoint, etc. */
                /* Details in code field. */

#define EXC_SYSCALL             7       /* System calls. */

#define EXC_MACH_SYSCALL        8       /* Mach system calls. */

#define EXC_RPC_ALERT           9       /* RPC alert */
 
#define EXC_CRASH               10      /* Abnormal process exit */

#define EXC_RESOURCE            11      /* Hit resource consumption limit */
                /* Exact resource is in code field. */

#define EXC_GUARD               12      /* Violated guarded resource protections */

#define EXC_MAX EXC_GUARD

#define EXC_MASK_BAD_ACCESS             (1 << EXC_BAD_ACCESS)
#define EXC_MASK_BAD_INSTRUCTION        (1 << EXC_BAD_INSTRUCTION)
#define EXC_MASK_ARITHMETIC             (1 << EXC_ARITHMETIC)
#define EXC_MASK_EMULATION              (1 << EXC_EMULATION)
#define EXC_MASK_SOFTWARE               (1 << EXC_SOFTWARE)
#define EXC_MASK_BREAKPOINT             (1 << EXC_BREAKPOINT)
#define EXC_MASK_SYSCALL                (1 << EXC_SYSCALL)
#define EXC_MASK_MACH_SYSCALL           (1 << EXC_MACH_SYSCALL)
#define EXC_MASK_RPC_ALERT              (1 << EXC_RPC_ALERT)
#define EXC_MASK_CRASH                  (1 << EXC_CRASH)
#define EXC_MASK_RESOURCE               (1 << EXC_RESOURCE)
#define EXC_MASK_GUARD                  (1 << EXC_GUARD)

#define EXC_MASK_ALL    (EXC_MASK_BAD_ACCESS |                  \
                         EXC_MASK_BAD_INSTRUCTION |             \
                         EXC_MASK_ARITHMETIC |                  \
                         EXC_MASK_EMULATION |                   \
                         EXC_MASK_SOFTWARE |                    \
                         EXC_MASK_BREAKPOINT |                  \
                         EXC_MASK_SYSCALL |                     \
                         EXC_MASK_MACH_SYSCALL |                \
                         EXC_MASK_RPC_ALERT |                   \
                         EXC_MASK_RESOURCE |                    \
                         EXC_MASK_GUARD |                       \
                         EXC_MASK_MACHINE)




#define EXCEPTION_DEFAULT			1
#define EXCEPTION_STATE				2
#define EXCEPTION_STATE_IDENTITY	3

#define MACH_EXCEPTION_CODES            0x80000000
/*      Send 64-bit code and subcode in the exception header */

#define FIRST_EXCEPTION		1	/* ZERO is illegal */



#include <sys/mach/port.h>
#include <sys/mach/thread_status.h>
#include <sys/mach/vm_types.h>
/*
 * Exported types
 */

typedef int                             exception_type_t;
typedef int		                        exception_data_type_t;
typedef int64_t                         mach_exception_data_type_t;
typedef int                             exception_behavior_t;
typedef exception_data_type_t           *exception_data_t;
typedef mach_exception_data_type_t      *mach_exception_data_t;
typedef unsigned int                    exception_mask_t;
typedef exception_mask_t                *exception_mask_array_t;
typedef exception_behavior_t            *exception_behavior_array_t;
typedef thread_state_flavor_t           *exception_flavor_array_t;
typedef mach_port_t			exception_port_t;
typedef mach_port_t                     *exception_port_array_t;
typedef mach_exception_data_type_t      mach_exception_code_t;
typedef mach_exception_data_type_t      mach_exception_subcode_t;



struct exception_action{
	struct ipc_port		*port;		/* exception port */
	thread_state_flavor_t	flavor;		/* state flavor to send */
	exception_behavior_t	behavior;	/* exception type to raise */
};

#endif

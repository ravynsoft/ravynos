/*
 * Copyright (c) 2003-2019 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*-
 * Copyright (c) 1999,2000,2001 Jonathan Lemon <jlemon@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$FreeBSD: src/sys/sys/event.h,v 1.5.2.5 2001/12/14 19:21:22 jlemon Exp $
 */

#ifndef _SYS_EVENT_H_
#define _SYS_EVENT_H_

#include <machine/types.h>
#include <sys/cdefs.h>
#include <stdint.h>

/*
 * Filter types
 */
#define EVFILT_READ             (-1)
#define EVFILT_WRITE            (-2)
#define EVFILT_AIO              (-3)    /* attached to aio requests */
#define EVFILT_VNODE            (-4)    /* attached to vnodes */
#define EVFILT_PROC             (-5)    /* attached to struct proc */
#define EVFILT_SIGNAL           (-6)    /* attached to struct proc */
#define EVFILT_TIMER            (-7)    /* timers */
#define EVFILT_MACHPORT         (-8)    /* Mach portsets */
#define EVFILT_FS               (-9)    /* Filesystem events */
#define EVFILT_USER             (-10)   /* User events */
#define EVFILT_UNUSED_11        (-11)   /* (-11) unused */
#define EVFILT_VM               (-12)   /* Virtual memory events */
#define EVFILT_SOCK             (-13)   /* Socket events */
#define EVFILT_MEMORYSTATUS     (-14)   /* Memorystatus events */
#define EVFILT_EXCEPT           (-15)   /* Exception events */
#define EVFILT_WORKLOOP         (-17)   /* Workloop events */

#define EVFILT_SYSCOUNT         17
#define EVFILT_THREADMARKER     EVFILT_SYSCOUNT /* Internal use only */

#pragma pack(4)

struct kevent {
	uintptr_t       ident;  /* identifier for this event */
	int16_t         filter; /* filter for event */
	uint16_t        flags;  /* general flags */
	uint32_t        fflags; /* filter-specific flags */
	intptr_t        data;   /* filter-specific data */
	void            *udata; /* opaque user data identifier */
};


#pragma pack()

struct kevent64_s {
	uint64_t        ident;          /* identifier for this event */
	int16_t         filter;         /* filter for event */
	uint16_t        flags;          /* general flags */
	uint32_t        fflags;         /* filter-specific flags */
	int64_t         data;           /* filter-specific data */
	uint64_t        udata;          /* opaque user data identifier */
	uint64_t        ext[2];         /* filter-specific extensions */
};

struct kevent_qos_s {
	uint64_t        ident;          /* identifier for this event */
	int16_t         filter;         /* filter for event */
	uint16_t        flags;          /* general flags */
	int32_t         qos;            /* quality of service */
	uint64_t        udata;          /* opaque user data identifier */
	uint32_t        fflags;         /* filter-specific flags */
	uint32_t        xflags;         /* extra filter-specific flags */
	int64_t         data;           /* filter-specific data */
	uint64_t        ext[4];         /* filter-specific extensions */
};

/*
 * Type definition for names/ids of dynamically allocated kqueues.
 */
typedef uint64_t kqueue_id_t;

#define EV_SET(kevp, a, b, c, d, e, f) do {     \
	struct kevent *__kevp__ = (kevp);       \
	__kevp__->ident = (a);                  \
	__kevp__->filter = (b);                 \
	__kevp__->flags = (c);                  \
	__kevp__->fflags = (d);                 \
	__kevp__->data = (e);                   \
	__kevp__->udata = (f);                  \
} while(0)

#define EV_SET64(kevp, a, b, c, d, e, f, g, h) do {     \
	struct kevent64_s *__kevp__ = (kevp);           \
	__kevp__->ident = (a);                          \
	__kevp__->filter = (b);                         \
	__kevp__->flags = (c);                          \
	__kevp__->fflags = (d);                         \
	__kevp__->data = (e);                           \
	__kevp__->udata = (f);                          \
	__kevp__->ext[0] = (g);                         \
	__kevp__->ext[1] = (h);                         \
} while(0)


/* kevent system call flags */
#define KEVENT_FLAG_NONE                         0x000000       /* no flag value */
#define KEVENT_FLAG_IMMEDIATE                    0x000001       /* immediate timeout */
#define KEVENT_FLAG_ERROR_EVENTS                 0x000002       /* output events only include change errors */


/*
 * Rather than provide an EV_SET_QOS macro for kevent_qos_t structure
 * initialization, we encourage use of named field initialization support
 * instead.
 */

// was  KEVENT_FLAG_STACK_EVENTS                 0x000004
#define KEVENT_FLAG_STACK_DATA                   0x000008   /* output data allocated as stack (grows down) */
//      KEVENT_FLAG_POLL                         0x000010
#define KEVENT_FLAG_WORKQ                        0x000020   /* interact with the default workq kq */
//      KEVENT_FLAG_LEGACY32                     0x000040
//      KEVENT_FLAG_LEGACY64                     0x000080
//      KEVENT_FLAG_PROC64                       0x000100
#define KEVENT_FLAG_WORKQ_MANAGER                0x000200   /* obsolete */
#define KEVENT_FLAG_WORKLOOP                     0x000400   /* interact with the specified workloop kq */
#define KEVENT_FLAG_PARKING                      0x000800   /* workq thread is parking */
//      KEVENT_FLAG_KERNEL                       0x001000
//      KEVENT_FLAG_DYNAMIC_KQUEUE               0x002000
//      KEVENT_FLAG_NEEDS_END_PROCESSING         0x004000
#define KEVENT_FLAG_WORKLOOP_SERVICER_ATTACH     0x008000   /* obsolete */
#define KEVENT_FLAG_WORKLOOP_SERVICER_DETACH     0x010000   /* obsolete */
#define KEVENT_FLAG_DYNAMIC_KQ_MUST_EXIST        0x020000   /* kq lookup by id must exist */
#define KEVENT_FLAG_DYNAMIC_KQ_MUST_NOT_EXIST    0x040000   /* kq lookup by id must not exist */
#define KEVENT_FLAG_WORKLOOP_NO_WQ_THREAD        0x080000   /* obsolete */


#define EV_SET_QOS 0


/* actions */
#define EV_ADD              0x0001      /* add event to kq (implies enable) */
#define EV_DELETE           0x0002      /* delete event from kq */
#define EV_ENABLE           0x0004      /* enable event */
#define EV_DISABLE          0x0008      /* disable event (not reported) */

/* flags */
#define EV_ONESHOT          0x0010      /* only report one occurrence */
#define EV_CLEAR            0x0020      /* clear event state after reporting */
#define EV_RECEIPT          0x0040      /* force immediate event output */
                                        /* ... with or without EV_ERROR */
                                        /* ... use KEVENT_FLAG_ERROR_EVENTS */
                                        /*     on syscalls supporting flags */

#define EV_DISPATCH         0x0080      /* disable event after reporting */
#define EV_UDATA_SPECIFIC   0x0100      /* unique kevent per udata value */

#define EV_DISPATCH2        (EV_DISPATCH | EV_UDATA_SPECIFIC)
/* ... in combination with EV_DELETE */
/* will defer delete until udata-specific */
/* event enabled. EINPROGRESS will be */
/* returned to indicate the deferral */

#define EV_VANISHED         0x0200      /* report that source has vanished  */
                                        /* ... only valid with EV_DISPATCH2 */

#define EV_SYSFLAGS         0xF000      /* reserved by system */
#define EV_FLAG0            0x1000      /* filter-specific flag */
#define EV_FLAG1            0x2000      /* filter-specific flag */

/* returned values */
#define EV_EOF              0x8000      /* EOF detected */
#define EV_ERROR            0x4000      /* error, data contains errno */

/*
 * Filter specific flags for EVFILT_READ
 *
 * The default behavior for EVFILT_READ is to make the "read" determination
 * relative to the current file descriptor read pointer.
 *
 * The EV_POLL flag indicates the determination should be made via poll(2)
 * semantics. These semantics dictate always returning true for regular files,
 * regardless of the amount of unread data in the file.
 *
 * On input, EV_OOBAND specifies that filter should actively return in the
 * presence of OOB on the descriptor. It implies that filter will return
 * if there is OOB data available to read OR when any other condition
 * for the read are met (for example number of bytes regular data becomes >=
 * low-watermark).
 * If EV_OOBAND is not set on input, it implies that the filter should not actively
 * return for out of band data on the descriptor. The filter will then only return
 * when some other condition for read is met (ex: when number of regular data bytes
 * >=low-watermark OR when socket can't receive more data (SS_CANTRCVMORE)).
 *
 * On output, EV_OOBAND indicates the presence of OOB data on the descriptor.
 * If it was not specified as an input parameter, then the data count is the
 * number of bytes before the current OOB marker, else data count is the number
 * of bytes beyond OOB marker.
 */
#define EV_POLL         EV_FLAG0
#define EV_OOBAND       EV_FLAG1

/*
 * data/hint fflags for EVFILT_USER, shared with userspace
 */

/*
 * On input, NOTE_TRIGGER causes the event to be triggered for output.
 */
#define NOTE_TRIGGER    0x01000000

/*
 * On input, the top two bits of fflags specifies how the lower twenty four
 * bits should be applied to the stored value of fflags.
 *
 * On output, the top two bits will always be set to NOTE_FFNOP and the
 * remaining twenty four bits will contain the stored fflags value.
 */
#define NOTE_FFNOP      0x00000000              /* ignore input fflags */
#define NOTE_FFAND      0x40000000              /* and fflags */
#define NOTE_FFOR       0x80000000              /* or fflags */
#define NOTE_FFCOPY     0xc0000000              /* copy fflags */
#define NOTE_FFCTRLMASK 0xc0000000              /* mask for operations */
#define NOTE_FFLAGSMASK 0x00ffffff

/*
 * data/hint fflags for EVFILT_WORKLOOP, shared with userspace
 *
 * The ident for thread requests should be the dynamic ID of the workloop
 * The ident for each sync waiter must be unique to that waiter [for this workloop]
 *
 *
 * Commands:
 *
 * @const NOTE_WL_THREAD_REQUEST [in/out]
 * The kevent represents asynchronous userspace work and its associated QoS.
 * There can only be a single knote with this flag set per workloop.
 *
 * @const NOTE_WL_SYNC_WAIT [in/out]
 * This bit is set when the caller is waiting to become the owner of a workloop.
 * If the NOTE_WL_SYNC_WAKE bit is already set then the caller is not blocked,
 * else it blocks until it is set.
 *
 * The QoS field of the knote is used to push on other owners or servicers.
 *
 * @const NOTE_WL_SYNC_WAKE [in/out]
 * Marks the waiter knote as being eligible to become an owner
 * This bit can only be set once, trying it again will fail with EALREADY.
 *
 * @const NOTE_WL_SYNC_IPC [in/out]
 * The knote is a sync IPC redirected turnstile push.
 *
 * Flags/Modifiers:
 *
 * @const NOTE_WL_UPDATE_QOS [in] (only NOTE_WL_THREAD_REQUEST)
 * For successful updates (EV_ADD only), learn the new userspace async QoS from
 * the kevent qos field.
 *
 * @const NOTE_WL_END_OWNERSHIP [in]
 * If the update is successful (including deletions) or returns ESTALE, and
 * the caller thread or the "suspended" thread is currently owning the workloop,
 * then ownership is forgotten.
 *
 * @const NOTE_WL_DISCOVER_OWNER [in]
 * If the update is successful (including deletions), learn the owner identity
 * from the loaded value during debounce. This requires an address to have been
 * filled in the EV_EXTIDX_WL_ADDR ext field, but doesn't require a mask to have
 * been set in the EV_EXTIDX_WL_MASK.
 *
 * @const NOTE_WL_IGNORE_ESTALE [in]
 * If the operation would fail with ESTALE, mask the error and pretend the
 * update was successful. However the operation itself didn't happen, meaning
 * that:
 * - attaching a new knote will not happen
 * - dropping an existing knote will not happen
 * - NOTE_WL_UPDATE_QOS or NOTE_WL_DISCOVER_OWNER will have no effect
 *
 * This modifier doesn't affect NOTE_WL_END_OWNERSHIP.
 */
#define NOTE_WL_THREAD_REQUEST   0x00000001
#define NOTE_WL_SYNC_WAIT        0x00000004
#define NOTE_WL_SYNC_WAKE        0x00000008
#define NOTE_WL_SYNC_IPC         0x80000000
#define NOTE_WL_COMMANDS_MASK    0x8000000f /* Mask of all the [in] commands above */

#define NOTE_WL_UPDATE_QOS       0x00000010
#define NOTE_WL_END_OWNERSHIP    0x00000020
#define NOTE_WL_DISCOVER_OWNER   0x00000080
#define NOTE_WL_IGNORE_ESTALE    0x00000100
#define NOTE_WL_UPDATES_MASK     0x000001f0 /* Mask of all the [in] updates above */

#define NOTE_WL_UPDATE_OWNER     0 /* ... compatibility define ... */

/*
 * EVFILT_WORKLOOP ext[] array indexes/meanings.
 */
#define EV_EXTIDX_WL_LANE        0         /* lane identifier  [in: sync waiter]
	                                    *                  [out: thread request]     */
#define EV_EXTIDX_WL_ADDR        1         /* debounce address [in: NULL==no debounce]   */
#define EV_EXTIDX_WL_MASK        2         /* debounce mask    [in]                      */
#define EV_EXTIDX_WL_VALUE       3         /* debounce value   [in: not current->ESTALE]
	                                    *                  [out: new/debounce value] */


/*
 * data/hint fflags for EVFILT_{READ|WRITE}, shared with userspace
 *
 * The default behavior for EVFILT_READ is to make the determination
 * realtive to the current file descriptor read pointer.
 */
#define NOTE_LOWAT      0x00000001              /* low water mark */

/* data/hint flags for EVFILT_EXCEPT, shared with userspace */
#define NOTE_OOB        0x00000002              /* OOB data */

/*
 * data/hint fflags for EVFILT_VNODE, shared with userspace
 */
#define NOTE_DELETE     0x00000001              /* vnode was removed */
#define NOTE_WRITE      0x00000002              /* data contents changed */
#define NOTE_EXTEND     0x00000004              /* size increased */
#define NOTE_ATTRIB     0x00000008              /* attributes changed */
#define NOTE_LINK       0x00000010              /* link count changed */
#define NOTE_RENAME     0x00000020              /* vnode was renamed */
#define NOTE_REVOKE     0x00000040              /* vnode access was revoked */
#define NOTE_NONE       0x00000080              /* No specific vnode event: to test for EVFILT_READ activation*/
#define NOTE_FUNLOCK    0x00000100              /* vnode was unlocked by flock(2) */

/*
 * data/hint fflags for EVFILT_PROC, shared with userspace
 *
 * Please note that EVFILT_PROC and EVFILT_SIGNAL share the same knote list
 * that hangs off the proc structure. They also both play games with the hint
 * passed to KNOTE(). If NOTE_SIGNAL is passed as a hint, then the lower bits
 * of the hint contain the signal. IF NOTE_FORK is passed, then the lower bits
 * contain the PID of the child (but the pid does not get passed through in
 * the actual kevent).
 */
enum {
	eNoteReapDeprecated __deprecated_enum_msg("This kqueue(2) EVFILT_PROC flag is deprecated") = 0x10000000
};

#define NOTE_EXIT               0x80000000      /* process exited */
#define NOTE_FORK               0x40000000      /* process forked */
#define NOTE_EXEC               0x20000000      /* process exec'd */
#define NOTE_REAP               ((unsigned int)eNoteReapDeprecated /* 0x10000000 */ )   /* process reaped */
#define NOTE_SIGNAL             0x08000000      /* shared with EVFILT_SIGNAL */
#define NOTE_EXITSTATUS         0x04000000      /* exit status to be returned, valid for child process only */
#define NOTE_EXIT_DETAIL        0x02000000      /* provide details on reasons for exit */

#define NOTE_PDATAMASK  0x000fffff              /* mask for signal & exit status */
#define NOTE_PCTRLMASK  (~NOTE_PDATAMASK)

/*
 * If NOTE_EXITSTATUS is present, provide additional info about exiting process.
 */
enum {
	eNoteExitReparentedDeprecated __deprecated_enum_msg("This kqueue(2) EVFILT_PROC flag is no longer sent") = 0x00080000
};
#define NOTE_EXIT_REPARENTED    ((unsigned int)eNoteExitReparentedDeprecated)   /* exited while reparented */

/*
 * If NOTE_EXIT_DETAIL is present, these bits indicate specific reasons for exiting.
 */
#define NOTE_EXIT_DETAIL_MASK           0x00070000
#define NOTE_EXIT_DECRYPTFAIL           0x00010000
#define NOTE_EXIT_MEMORY                0x00020000
#define NOTE_EXIT_CSERROR               0x00040000


/*
 * If NOTE_EXIT_MEMORY is present, these bits indicate specific jetsam condition.
 */
#define NOTE_EXIT_MEMORY_DETAIL_MASK    0xfe000000
#define NOTE_EXIT_MEMORY_VMPAGESHORTAGE 0x80000000      /* jetsam condition: lowest jetsam priority proc killed due to vm page shortage */
#define NOTE_EXIT_MEMORY_VMTHRASHING    0x40000000      /* jetsam condition: lowest jetsam priority proc killed due to vm thrashing */
#define NOTE_EXIT_MEMORY_HIWAT          0x20000000      /* jetsam condition: process reached its high water mark */
#define NOTE_EXIT_MEMORY_PID            0x10000000      /* jetsam condition: special pid kill requested */
#define NOTE_EXIT_MEMORY_IDLE           0x08000000      /* jetsam condition: idle process cleaned up */
#define NOTE_EXIT_MEMORY_VNODE          0X04000000      /* jetsam condition: virtual node kill */
#define NOTE_EXIT_MEMORY_FCTHRASHING    0x02000000      /* jetsam condition: lowest jetsam priority proc killed due to filecache thrashing */


/*
 * data/hint fflags for EVFILT_VM, shared with userspace.
 */
#define NOTE_VM_PRESSURE                        0x80000000              /* will react on memory pressure */
#define NOTE_VM_PRESSURE_TERMINATE              0x40000000              /* will quit on memory pressure, possibly after cleaning up dirty state */
#define NOTE_VM_PRESSURE_SUDDEN_TERMINATE       0x20000000              /* will quit immediately on memory pressure */
#define NOTE_VM_ERROR                           0x10000000              /* there was an error */


/*
 * data/hint fflags for EVFILT_MEMORYSTATUS, shared with userspace.
 */
#define NOTE_MEMORYSTATUS_PRESSURE_NORMAL       0x00000001      /* system memory pressure has returned to normal */
#define NOTE_MEMORYSTATUS_PRESSURE_WARN         0x00000002      /* system memory pressure has changed to the warning state */
#define NOTE_MEMORYSTATUS_PRESSURE_CRITICAL     0x00000004      /* system memory pressure has changed to the critical state */
#define NOTE_MEMORYSTATUS_LOW_SWAP              0x00000008      /* system is in a low-swap state */
#define NOTE_MEMORYSTATUS_PROC_LIMIT_WARN       0x00000010      /* process memory limit has hit a warning state */
#define NOTE_MEMORYSTATUS_PROC_LIMIT_CRITICAL   0x00000020      /* process memory limit has hit a critical state - soft limit */
#define NOTE_MEMORYSTATUS_MSL_STATUS   0xf0000000      /* bits used to request change to process MSL status */


typedef enum vm_pressure_level {
	kVMPressureNormal   = 0,
	kVMPressureWarning  = 1,
	kVMPressureUrgent   = 2,
	kVMPressureCritical = 3,
	kVMPressureJetsam   = 4,  /* jetsam approaching FG bands */
} vm_pressure_level_t;


/*
 * data/hint fflags for EVFILT_TIMER, shared with userspace.
 * The default is a (repeating) interval timer with the data
 * specifying the timeout interval in milliseconds.
 *
 * All timeouts are implicitly EV_CLEAR events.
 */
#define NOTE_SECONDS    0x00000001              /* data is seconds         */
#define NOTE_USECONDS   0x00000002              /* data is microseconds    */
#define NOTE_NSECONDS   0x00000004              /* data is nanoseconds     */
#define NOTE_ABSOLUTE   0x00000008              /* absolute timeout        */
/* ... implicit EV_ONESHOT, timeout uses the gettimeofday epoch */
#define NOTE_LEEWAY             0x00000010              /* ext[1] holds leeway for power aware timers */
#define NOTE_CRITICAL   0x00000020              /* system does minimal timer coalescing */
#define NOTE_BACKGROUND 0x00000040              /* system does maximum timer coalescing */
#define NOTE_MACH_CONTINUOUS_TIME       0x00000080
/*
 * NOTE_MACH_CONTINUOUS_TIME:
 * with NOTE_ABSOLUTE: causes the timer to continue to tick across sleep,
 *      still uses gettimeofday epoch
 * with NOTE_MACHTIME and NOTE_ABSOLUTE: uses mach continuous time epoch
 * without NOTE_ABSOLUTE (interval timer mode): continues to tick across sleep
 */
#define NOTE_MACHTIME   0x00000100              /* data is mach absolute time units */
/* timeout uses the mach absolute time epoch */

/*
 * data/hint fflags for EVFILT_SOCK, shared with userspace.
 *
 */
#define NOTE_CONNRESET          0x00000001 /* Received RST */
#define NOTE_READCLOSED         0x00000002 /* Read side is shutdown */
#define NOTE_WRITECLOSED        0x00000004 /* Write side is shutdown */
#define NOTE_TIMEOUT            0x00000008 /* timeout: rexmt, keep-alive or persist */
#define NOTE_NOSRCADDR          0x00000010 /* source address not available */
#define NOTE_IFDENIED           0x00000020 /* interface denied connection */
#define NOTE_SUSPEND            0x00000040 /* output queue suspended */
#define NOTE_RESUME             0x00000080 /* output queue resumed */
#define NOTE_KEEPALIVE          0x00000100 /* TCP Keepalive received */
#define NOTE_ADAPTIVE_WTIMO     0x00000200 /* TCP adaptive write timeout */
#define NOTE_ADAPTIVE_RTIMO     0x00000400 /* TCP adaptive read timeout */
#define NOTE_CONNECTED          0x00000800 /* socket is connected */
#define NOTE_DISCONNECTED       0x00001000 /* socket is disconnected */
#define NOTE_CONNINFO_UPDATED   0x00002000 /* connection info was updated */
#define NOTE_NOTIFY_ACK         0x00004000 /* notify acknowledgement */

#define EVFILT_SOCK_LEVEL_TRIGGER_MASK \
	        (NOTE_READCLOSED | NOTE_WRITECLOSED | NOTE_SUSPEND | NOTE_RESUME | \
	         NOTE_CONNECTED | NOTE_DISCONNECTED)

#define EVFILT_SOCK_ALL_MASK \
	        (NOTE_CONNRESET | NOTE_READCLOSED | NOTE_WRITECLOSED | NOTE_TIMEOUT | \
	        NOTE_NOSRCADDR | NOTE_IFDENIED | NOTE_SUSPEND | NOTE_RESUME | \
	        NOTE_KEEPALIVE | NOTE_ADAPTIVE_WTIMO | NOTE_ADAPTIVE_RTIMO | \
	        NOTE_CONNECTED | NOTE_DISCONNECTED | NOTE_CONNINFO_UPDATED | \
	        NOTE_NOTIFY_ACK)


/*
 * data/hint fflags for EVFILT_MACHPORT, shared with userspace.
 *
 * Only portsets are supported at this time.
 *
 * The fflags field can optionally contain the MACH_RCV_MSG, MACH_RCV_LARGE,
 * and related trailer receive options as defined in <mach/message.h>.
 * The presence of these flags directs the kevent64() call to attempt to receive
 * the message during kevent delivery, rather than just indicate that a message exists.
 * On setup, The ext[0] field contains the receive buffer pointer and ext[1] contains
 * the receive buffer length.  Upon event delivery, the actual received message size
 * is returned in ext[1].  As with mach_msg(), the buffer must be large enough to
 * receive the message and the requested (or default) message trailers.  In addition,
 * the fflags field contains the return code normally returned by mach_msg().
 *
 * If MACH_RCV_MSG is specified, and the ext[1] field specifies a zero length, the
 * system call argument specifying an ouput area (kevent_qos) will be consulted. If
 * the system call specified an output data area, the user-space address
 * of the received message is carved from that provided output data area (if enough
 * space remains there). The address and length of each received message is
 * returned in the ext[0] and ext[1] fields (respectively) of the corresponding kevent.
 *
 * IF_MACH_RCV_VOUCHER_CONTENT is specified, the contents of the message voucher is
 * extracted (as specified in the xflags field) and stored in ext[2] up to ext[3]
 * length.  If the input length is zero, and the system call provided a data area,
 * the space for the voucher content is carved from the provided space and its
 * address and length is returned in ext[2] and ext[3] respectively.
 *
 * If no message receipt options were provided in the fflags field on setup, no
 * message is received by this call. Instead, on output, the data field simply
 * contains the name of the actual port detected with a message waiting.
 */

/*
 * DEPRECATED!!!!!!!!!
 * NOTE_TRACK, NOTE_TRACKERR, and NOTE_CHILD are no longer supported as of 10.5
 */
/* additional flags for EVFILT_PROC */
#define NOTE_TRACK      0x00000001              /* follow across forks */
#define NOTE_TRACKERR   0x00000002              /* could not track child */
#define NOTE_CHILD      0x00000004              /* am a child process */



/* Temporay solution for BootX to use inode.h till kqueue moves to vfs layer */
#include <sys/queue.h>
struct knote;
SLIST_HEAD(klist, knote);


#include <sys/types.h>

struct timespec;

__BEGIN_DECLS
int     kqueue(void);
int     kevent(int kq,
    const struct kevent *changelist, int nchanges,
    struct kevent *eventlist, int nevents,
    const struct timespec *timeout);
int     kevent64(int kq,
    const struct kevent64_s *changelist, int nchanges,
    struct kevent64_s *eventlist, int nevents,
    unsigned int flags,
    const struct timespec *timeout);

int     kevent_qos(int kq,
    const struct kevent_qos_s *changelist, int nchanges,
    struct kevent_qos_s *eventlist, int nevents,
    void *data_out, size_t *data_available,
    unsigned int flags);

int     kevent_id(kqueue_id_t id,
    const struct kevent_qos_s *changelist, int nchanges,
    struct kevent_qos_s *eventlist, int nevents,
    void *data_out, size_t *data_available,
    unsigned int flags);

__END_DECLS




/* Flags for pending events notified by kernel via return-to-kernel ast */
#define R2K_WORKLOOP_PENDING_EVENTS             0x1
#define R2K_WORKQ_PENDING_EVENTS                0x2


#endif /* !_SYS_EVENT_H_ */

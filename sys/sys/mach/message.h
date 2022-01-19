/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * MkLinux
 */
/* CMU_HIST */
/*
 * Revision 2.11.2.2  92/03/03  16:22:15  jeffreyh
 * 	Changes from TRUNK
 * 	[92/02/26  12:10:31  jeffreyh]
 * 
 * Revision 2.12  92/01/15  13:44:29  rpd
 * 	Changed MACH_IPC_COMPAT conditionals to default to not present.
 *
 * Revision 2.11.2.1  92/01/03  16:36:23  jsb
 * 	Added MACH_MSGH_BITS_MIGRATED (NORMA_IPC internal).
 * 	[91/12/25  16:42:44  jsb]
 * 
 * Revision 2.11  91/10/09  16:12:14  af
 * 	Revision 2.10.1.1  91/10/05  15:44:57  rpd
 *  	Added check for __STDC__ to get function prototypes.
 *  	Removed MACH_MSGH_KIND_NOTIFICATION compatibility definition.
 * 
 * Revision 2.10  91/08/28  11:15:27  jsb
 * 	Replaced msgh_kind with msgh_seqno.
 * 	[91/08/09            rpd]
 * 
 * Revision 2.9  91/08/03  18:19:05  jsb
 * 	Added MACH_MSGH_BITS_COMPLEX_{PORTS,DATA}.
 * 	[91/07/04  12:38:13  jsb]
 * 
 * Revision 2.8  91/05/14  16:56:21  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/02/05  17:34:12  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:19:32  mrt]
 * 
 * Revision 2.6  90/06/19  22:59:59  rpd
 * 	Changed mach_msg_timeout_t to unsigned.
 * 	[90/06/06            rpd]
 * 
 * Revision 2.5  90/06/02  14:59:06  rpd
 * 	Revised the comments.
 * 	[90/05/13            rpd]
 * 	Converted to new IPC.
 * 	[90/03/26  22:36:07  rpd]
 * 
 *
 * Condensed history:
 *	Named unused bits in message structures (rpd).
 *	Added MSG_TYPE_POLYMORPHIC (rpd).
 *	Put ownership rights under MACH_IPC_XXXHACK (rpd).
 * 	Removed some unused defines (rpd).
 *	Made MSG_TYPE_PORT_NAME a separate type (rpd).
 *	Added SEND_SWITCH (mwyoung).
 *	Added SEND_MSG_SIZE_CHANGE (mwyoung).
 *	Added msg_size_t, msg_timeout_t (mwyoung).
 *	Added MSG_TYPE_INTERNAL_MEMORY (mwyoung).
 *	Use unsigned ints/shorts (avie).
 *	Added SEND_INTERRUPT (mwyoung).
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */
/*
 *	File:	mach/message.h
 *
 *	Mach IPC message and primitive function definitions.
 */

#ifndef	_MACH_MESSAGE_H_
#define _MACH_MESSAGE_H_

#define MACH_ASSERT 0
#define MACH_KDB 0
#define NCPUS (SMP ? 32 : 1)
#define NORMA_VM 0

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <vm/vm.h>
#include <sys/mach/kern_return.h>
#include <sys/mach/port.h>


/*
 *  The timeout mechanism uses mach_msg_timeout_t values,
 *  passed by value.  The timeout units are milliseconds.
 *  It is controlled with the MACH_SEND_TIMEOUT
 *  and MACH_RCV_TIMEOUT options.
 */

typedef natural_t mach_msg_timeout_t;

typedef char *mach_msg_trailer_info_t;

/*
 *  The value to be used when there is no timeout.
 *  (No MACH_SEND_TIMEOUT/MACH_RCV_TIMEOUT option.)
 */

#define MACH_MSG_TIMEOUT_NONE		((mach_msg_timeout_t) 0)

/*
 *  The kernel uses MACH_MSGH_BITS_COMPLEX as a hint.  It it isn't on, it
 *  assumes the body of the message doesn't contain port rights or OOL
 *  data.  The field is set in received messages.  A user task must
 *  use caution in interpreting the body of a message if the bit isn't
 *  on, because the mach_msg_type's in the body might "lie" about the
 *  contents.  If the bit isn't on, but the mach_msg_types
 *  in the body specify rights or OOL data, the behavior is undefined.
 *  (Ie, an error may or may not be produced.)
 *
 *  The value of MACH_MSGH_BITS_REMOTE determines the interpretation
 *  of the msgh_remote_port field.  It is handled like a msgt_name.
 *
 *  The value of MACH_MSGH_BITS_LOCAL determines the interpretation
 *  of the msgh_local_port field.  It is handled like a msgt_name.
 *
 *  MACH_MSGH_BITS() combines two MACH_MSG_TYPE_* values, for the remote
 *  and local fields, into a single value suitable for msgh_bits.
 *
 *  MACH_MSGH_BITS_CIRCULAR should be zero; is is used internally.
 *
 *  MACH_MSGH_BITS_RTALLOC indicates that rtalloc/rtfree should be used
 *  instead of kalloc/kfree for the kmsg and associated data buffers.
 *
 *  The unused bits should be zero and are reserved for the kernel
 *  or for future interface expansion.
 */

#define MACH_MSGH_BITS_ZERO		0x00000000

#define MACH_MSGH_BITS_REMOTE_MASK	0x0000001f
#define MACH_MSGH_BITS_LOCAL_MASK	0x00001f00
#define MACH_MSGH_BITS_VOUCHER_MASK	0x001f0000

#define	MACH_MSGH_BITS_PORTS_MASK		\
		(MACH_MSGH_BITS_REMOTE_MASK |	\
		 MACH_MSGH_BITS_LOCAL_MASK |	\
		 MACH_MSGH_BITS_VOUCHER_MASK)

#define MACH_MSGH_BITS_COMPLEX		0x80000000U
#define	MACH_MSGH_BITS_CIRCULAR		0x10000000	/* internal use only */

#define MACH_MSGH_BITS(remote, local)				\
	((remote) | ((local) << 8))
#define	MACH_MSGH_BITS_SET_PORTS(remote, local, voucher)	\
	(((remote) & MACH_MSGH_BITS_REMOTE_MASK) | 		\
	 (((local) << 8) & MACH_MSGH_BITS_LOCAL_MASK) | 	\
	 (((voucher) << 16) & MACH_MSGH_BITS_VOUCHER_MASK))
#define MACH_MSGH_BITS_SET(remote, local, voucher, other)	\
	(MACH_MSGH_BITS_SET_PORTS((remote), (local), (voucher)) \
	 | ((other) &~ MACH_MSGH_BITS_PORTS_MASK))

#define	MACH_MSGH_BITS_REMOTE(bits)				\
		((bits) & MACH_MSGH_BITS_REMOTE_MASK)
#define	MACH_MSGH_BITS_LOCAL(bits)				\
		(((bits) & MACH_MSGH_BITS_LOCAL_MASK) >> 8)
#define	MACH_MSGH_BITS_PORTS(bits)				\
		((bits) & MACH_MSGH_BITS_PORTS_MASK)
#define	MACH_MSGH_BITS_OTHER(bits)				\
		((bits) &~ MACH_MSGH_BITS_PORTS_MASK)

/*
 *  Every message starts with a message header.
 *  Following the message header are zero or more pairs of
 *  type descriptors (mach_msg_type_t/mach_msg_type_long_t) and
 *  data values.  The size of the message must be specified in bytes,
 *  and includes the message header, type descriptors, inline
 *  data, and inline pointer for out-of-line data.
 *
 *  The msgh_remote_port field specifies the destination of the message.
 *  It must specify a valid send or send-once right for a port.
 *
 *  The msgh_local_port field specifies a "reply port".  Normally,
 *  This field carries a send-once right that the receiver will use
 *  to reply to the message.  It may carry the values MACH_PORT_NULL,
 *  MACH_PORT_DEAD, a send-once right, or a send right.
 *
 *  The msgh_seqno field carries a sequence number associated with the
 *  received-from port.  A port's sequence number is incremented every
 *  time a message is received from it.  In sent messages, the field's
 *  value is ignored.
 *
 *  The msgh_id field is uninterpreted by the message primitives.
 *  It normally carries information specifying the format
 *  or meaning of the message.
 */

typedef unsigned int mach_msg_bits_t;
typedef	natural_t mach_msg_size_t;
typedef integer_t mach_msg_id_t;


#define MACH_MSG_SIZE_NULL (mach_msg_size_t *) 0

typedef uint8_t mach_msg_type_name_t;

#define MACH_MSG_TYPE_MOVE_RECEIVE	16	/* Must hold receive rights */
#define MACH_MSG_TYPE_MOVE_SEND		17	/* Must hold send rights */
#define MACH_MSG_TYPE_MOVE_SEND_ONCE	18	/* Must hold sendonce rights */
#define MACH_MSG_TYPE_COPY_SEND		19	/* Must hold send rights */
#define MACH_MSG_TYPE_MAKE_SEND		20	/* Must hold receive rights */
#define MACH_MSG_TYPE_MAKE_SEND_ONCE	21	/* Must hold receive rights */

#define MACH_MSG_TYPE_DISPOSE_RECEIVE	24	/* must hold receive right */
#define MACH_MSG_TYPE_DISPOSE_SEND	25	/* must hold send right(s) */
#define MACH_MSG_TYPE_DISPOSE_SEND_ONCE 26	/* must hold sendonce right */

typedef unsigned int mach_msg_copy_options_t;

#define MACH_MSG_PHYSICAL_COPY		0
#define MACH_MSG_VIRTUAL_COPY   	1
#define MACH_MSG_ALLOCATE		2
#define MACH_MSG_OVERWRITE		3
#ifdef  _KERNEL
#define MACH_MSG_KALLOC_COPY_T		4
#define MACH_MSG_PAGE_LIST_COPY_T	5
#endif  /* MACH_KERNEL */


typedef unsigned int mach_msg_descriptor_type_t;

#define MACH_MSG_PORT_DESCRIPTOR 		0
#define MACH_MSG_OOL_DESCRIPTOR  		1
#define MACH_MSG_OOL_PORTS_DESCRIPTOR 		2
#define MACH_MSG_OOL_VOLATILE_DESCRIPTOR  	3


#pragma pack(4)

typedef struct
{
  natural_t         pad1;
  mach_msg_size_t		pad2;
  unsigned int			pad3 : 24;
  mach_msg_descriptor_type_t	type : 8;
} mach_msg_type_descriptor_t;

typedef struct
{
	mach_port_t			name;
#if !(defined(_KERNEL) && defined(__LP64__))
	mach_msg_size_t		pad1;
	unsigned int			pad2 : 16;
#else
	mach_msg_size_t		pad1 : 8;
	unsigned int			pad2 : 8;
#endif
	mach_msg_type_name_t		disposition : 8;
	mach_msg_descriptor_type_t	type : 8;
#if defined(_KERNEL)
  uint32_t      pad_end;
#endif
} mach_msg_port_descriptor_t;

typedef struct
{
  uint32_t			address;
  mach_msg_size_t       	size;
  boolean_t     		deallocate: 8;
  mach_msg_copy_options_t       copy: 8;
  unsigned int     		pad1: 8;
  mach_msg_descriptor_type_t    type: 8;
} mach_msg_ool_descriptor32_t;

typedef struct
{
  uint64_t			address;
  boolean_t     		deallocate: 8;
  mach_msg_copy_options_t       copy: 8;
  unsigned int     		pad1: 8;
  mach_msg_descriptor_type_t    type: 8;
  mach_msg_size_t       	size;
} mach_msg_ool_descriptor64_t;

typedef struct
{
  void* address;
#ifndef __LP64__
  mach_msg_size_t       	size;
#endif
  boolean_t     		deallocate: 8;
  mach_msg_copy_options_t       copy: 8;
  unsigned int     		pad1: 8;
  mach_msg_descriptor_type_t    type: 8;
#ifdef __LP64__
  mach_msg_size_t       	size;
#endif
#if defined(KERNEL) && !defined(__LP64__)
  uint32_t          pad_end;
#endif
} mach_msg_ool_descriptor_t;

typedef struct
{
  uint32_t			address;
  mach_msg_size_t		count;
  boolean_t     		deallocate: 8;
  mach_msg_copy_options_t       copy: 8;
  mach_msg_type_name_t		disposition : 8;
  mach_msg_descriptor_type_t	type : 8;
} mach_msg_ool_ports_descriptor32_t;

typedef struct
{
  uint64_t			address;
  boolean_t     		deallocate: 8;
  mach_msg_copy_options_t       copy: 8;
  mach_msg_type_name_t		disposition : 8;
  mach_msg_descriptor_type_t	type : 8;
  mach_msg_size_t		count;
} mach_msg_ool_ports_descriptor64_t;

typedef struct
{
  void*				address;
  boolean_t     		deallocate: 8;
  mach_msg_copy_options_t       copy: 8;
  mach_msg_type_name_t		disposition : 8;
  mach_msg_descriptor_type_t  type : 8;
  mach_msg_size_t   count;
} mach_msg_ool_ports_descriptor_t;

typedef union
{
  mach_msg_port_descriptor_t		port;
  mach_msg_ool_descriptor_t		out_of_line;
  mach_msg_ool_ports_descriptor_t	ool_ports;
  mach_msg_type_descriptor_t		type;
} mach_msg_descriptor_t;

typedef struct
{
        mach_msg_size_t msgh_descriptor_count;
} mach_msg_body_t;

#define MACH_MSG_BODY_NULL (mach_msg_body_t *) 0
#define MACH_MSG_DESCRIPTOR_NULL (mach_msg_descriptor_t *) 0

typedef	struct
{
  mach_msg_bits_t	msgh_bits;
  mach_msg_size_t	msgh_size;
  mach_port_t		msgh_remote_port;
  mach_port_t		msgh_local_port;
  mach_port_name_t 	msgh_voucher_port;
  mach_msg_id_t		msgh_id;
} mach_msg_header_t;

#define	msgh_reserved		msgh_voucher_port
#define MACH_MSG_NULL (mach_msg_header_t *) 0

typedef struct
{
        mach_msg_header_t       header;
        mach_msg_body_t         body;
} mach_msg_base_t;

typedef	unsigned int mach_msg_trailer_type_t;

#define	MACH_MSG_TRAILER_FORMAT_0	0

typedef	unsigned int mach_msg_trailer_size_t;

typedef struct 
{
  mach_msg_trailer_type_t	msgh_trailer_type;
  mach_msg_trailer_size_t	msgh_trailer_size;
} mach_msg_trailer_t;

typedef struct
{
  mach_msg_trailer_type_t       msgh_trailer_type;
  mach_msg_trailer_size_t       msgh_trailer_size;
  mach_port_seqno_t             msgh_seqno;
} mach_msg_seqno_trailer_t;

typedef struct
{
  unsigned int			val[2];
} security_token_t;

typedef struct 
{
  mach_msg_trailer_type_t	msgh_trailer_type;
  mach_msg_trailer_size_t	msgh_trailer_size;
  mach_port_seqno_t		msgh_seqno;
  security_token_t		msgh_sender;
} mach_msg_security_trailer_t;

/*
 * The audit token is an opaque token which identifies
 * Mach tasks and senders of Mach messages as subjects
 * to the BSM audit system.  Only the appropriate BSM
 * library routines should be used to interpret the
 * contents of the audit token as the representation
 * of the subject identity within the token may change
 * over time.
 */
typedef struct
{
  unsigned int                  val[8];
} audit_token_t;

typedef struct 
{
  mach_msg_trailer_type_t       msgh_trailer_type;
  mach_msg_trailer_size_t       msgh_trailer_size;
  mach_port_seqno_t             msgh_seqno;
  security_token_t              msgh_sender;
  audit_token_t                 msgh_audit;
} mach_msg_audit_trailer_t;

typedef struct 
{
  mach_msg_trailer_type_t       msgh_trailer_type;
  mach_msg_trailer_size_t       msgh_trailer_size;
  mach_port_seqno_t             msgh_seqno;
  security_token_t              msgh_sender;
  audit_token_t                 msgh_audit;
  mach_port_context_t           msgh_context;
} mach_msg_context_trailer_t;

typedef struct
{
  mach_port_name_t sender;
} msg_labels_t;

/* 
   Trailer type to pass MAC policy label info as a mach message trailer.
*/

typedef struct
{
  mach_msg_trailer_type_t       msgh_trailer_type;
  mach_msg_trailer_size_t       msgh_trailer_size;
  mach_port_seqno_t             msgh_seqno;
  security_token_t              msgh_sender;
  audit_token_t                 msgh_audit;
  mach_port_context_t           msgh_context;
  int                           msgh_ad;
  msg_labels_t                  msgh_labels;
} mach_msg_mac_trailer_t;


#define MACH_MSG_TRAILER_MINIMUM_SIZE  sizeof(mach_msg_trailer_t)

/*
 * These values can change from release to release - but clearly
 * code cannot request additional trailer elements one was not
 * compiled to understand.  Therefore, it is safe to use this
 * constant when the same module specified the receive options.
 * Otherwise, you run the risk that the options requested by
 * another module may exceed the local modules notion of
 * MAX_TRAILER_SIZE.
 */

typedef mach_msg_mac_trailer_t mach_msg_max_trailer_t;
#define MAX_TRAILER_SIZE ((mach_msg_size_t)sizeof(mach_msg_max_trailer_t))

typedef mach_msg_security_trailer_t mach_msg_format_0_trailer_t;

#define MACH_MSG_TRAILER_FORMAT_0_SIZE sizeof(mach_msg_format_0_trailer_t)
#define MACH_MSG_TRAILER_MINIMUM_SIZE  sizeof(mach_msg_trailer_t)
#define   KERNEL_SECURITY_TOKEN_VALUE  { {0, 1} }
extern security_token_t KERNEL_SECURITY_TOKEN;

#define   KERNEL_AUDIT_TOKEN_VALUE  { {0, 0, 0, 0, 0, 0, 0, 0} }
extern audit_token_t KERNEL_AUDIT_TOKEN;

typedef	integer_t mach_msg_options_t;

typedef struct
{
  mach_msg_header_t	header;
} mach_msg_empty_send_t;

typedef struct
{
  mach_msg_header_t	header;
  mach_msg_trailer_t	trailer;
} mach_msg_empty_rcv_t;

typedef union
{
  mach_msg_empty_send_t	send;
  mach_msg_empty_rcv_t	rcv;
} mach_msg_empty_t;

#pragma pack()

/* utility to round the message size - will become machine dependent */
#define round_msg(x)	(((mach_msg_size_t)(x) + sizeof (natural_t) - 1) & \
				~(sizeof (natural_t) - 1))

/*
 *  There is no fixed upper bound to the size of Mach messages.
 */

#define	MACH_MSG_SIZE_MAX	((mach_msg_size_t) ~0)

/*
 *  Compatibility definitions, for code written
 *  when there was a msgh_kind instead of msgh_seqno.
 */

#define MACH_MSGH_KIND_NORMAL		0x00000000
#if	0
/* code using this is likely to break, so better not to have it defined */
#define MACH_MSGH_KIND_NOTIFICATION	0x00000001
#endif
#define	msgh_kind			msgh_seqno
#define mach_msg_kind_t			mach_port_seqno_t

/*
 *  The msgt_number field specifies the number of data elements.
 *  The msgt_size field specifies the size of each data element, in bits.
 *  The msgt_name field specifies the type of each data element.
 *  If msgt_inline is TRUE, the data follows the type descriptor
 *  in the body of the message.  If msgt_inline is FALSE, then a pointer
 *  to the data should follow the type descriptor, and the data is
 *  sent out-of-line.  In this case, if msgt_deallocate is TRUE,
 *  then the out-of-line data is moved (instead of copied) into the message.
 *  If msgt_longform is TRUE, then the type descriptor is actually
 *  a mach_msg_type_long_t.
 *
 *  The actual amount of inline data following the descriptor must
 *  a multiple of the word size.  For out-of-line data, this is a
 *  pointer.  For inline data, the supplied data size (calculated
 *  from msgt_number/msgt_size) is rounded up.  This guarantees
 *  that type descriptors always fall on word boundaries.
 *
 *  For port rights, msgt_size must be 8*sizeof(mach_port_t).
 *  If the data is inline, msgt_deallocate should be FALSE.
 *  The msgt_unused bit should be zero.
 *  The msgt_name, msgt_size, msgt_number fields in
 *  a mach_msg_type_long_t should be zero.
 */

typedef natural_t mach_msg_type_size_t;
typedef natural_t mach_msg_type_number_t;

/*
 *  Values received/carried in messages.  Tells the receiver what
 *  sort of port right he now has.
 *
 *  MACH_MSG_TYPE_PORT_NAME is used to transfer a port name
 *  which should remain uninterpreted by the kernel.  (Port rights
 *  are not transferred, just the port name.)
 */

#define MACH_MSG_TYPE_PORT_NONE		0

#define MACH_MSG_TYPE_PORT_NAME		15
#define MACH_MSG_TYPE_PORT_RECEIVE	MACH_MSG_TYPE_MOVE_RECEIVE
#define MACH_MSG_TYPE_PORT_SEND		MACH_MSG_TYPE_MOVE_SEND
#define MACH_MSG_TYPE_PORT_SEND_ONCE	MACH_MSG_TYPE_MOVE_SEND_ONCE

#define MACH_MSG_TYPE_LAST		22		/* Last assigned */

/*
 *  A dummy value.  Mostly used to indicate that the actual value
 *  will be filled in later, dynamically.
 */

#define MACH_MSG_TYPE_POLYMORPHIC	((mach_msg_type_name_t) -1)

/*
 *	Is a given item a port type?
 */

#define MACH_MSG_TYPE_PORT_ANY(x)			\
	(((x) >= MACH_MSG_TYPE_MOVE_RECEIVE) &&		\
	 ((x) <= MACH_MSG_TYPE_MAKE_SEND_ONCE))

#define	MACH_MSG_TYPE_PORT_ANY_SEND(x)			\
	(((x) >= MACH_MSG_TYPE_MOVE_SEND) &&		\
	 ((x) <= MACH_MSG_TYPE_MAKE_SEND_ONCE))

#define	MACH_MSG_TYPE_PORT_ANY_RIGHT(x)			\
	(((x) >= MACH_MSG_TYPE_MOVE_RECEIVE) &&		\
	 ((x) <= MACH_MSG_TYPE_MOVE_SEND_ONCE))

typedef integer_t mach_msg_option_t;

#define MACH_MSG_OPTION_NONE	0x00000000

#define	MACH_SEND_MSG		0x00000001
#define	MACH_RCV_MSG		0x00000002
#define MACH_RCV_LARGE          0x00000004      /* report large message sizes */
#define MACH_RCV_LARGE_IDENTITY 0x00000008      /* identify source of large messages */


#define MACH_SEND_TIMEOUT	0x00000010
#define MACH_SEND_INTERRUPT	0x00000040	/* libmach implements */
#define MACH_SEND_NOTIFY	0x00000080
#define MACH_SEND_ALWAYS	0x00010000	/* internal use only */
#define MACH_SEND_TRAILER	0x00020000



#define MACH_RCV_TIMEOUT	0x00000100
#define MACH_RCV_NOTIFY		0x00000200
#define MACH_RCV_INTERRUPT	0x00000400	/* libmach implements */
#define MACH_RCV_VOUCHER	0x00000800	/* willing to receive voucher port */
#define MACH_RCV_OVERWRITE	0x00001000

/* 
 * NOTE: a 0x00------ RCV mask implies to ask for
 * a MACH_MSG_TRAILER_FORMAT_0 with 0 Elements, 
 * which is equivalent to a mach_msg_trailer_t.
 */
#define MACH_RCV_TRAILER_NULL   0
#define MACH_RCV_TRAILER_SEQNO  1
#define MACH_RCV_TRAILER_SENDER 2
#define MACH_RCV_TRAILER_AUDIT  3
#define MACH_RCV_TRAILER_CTX    4
#define MACH_RCV_TRAILER_AV     7
#define MACH_RCV_TRAILER_LABELS 8


#define MACH_RCV_TRAILER_TYPE(x)     (((x) & 0xf) << 28) 
#define MACH_RCV_TRAILER_ELEMENTS(x) (((x) & 0xf) << 24)  
#define MACH_RCV_TRAILER_MASK 	     ((0xff << 24))


#define GET_RCV_ELEMENTS(y) (((y) >> 24) & 0xf)

#ifdef _KERNEL
/* The options that the kernel honors when passed from user space */
#define MACH_SEND_USER		 (MACH_SEND_MSG | \
				  MACH_SEND_TIMEOUT | MACH_SEND_NOTIFY | \
				  MACH_SEND_TRAILER)

#define MACH_RCV_USER		 (MACH_RCV_MSG | \
				  MACH_RCV_TIMEOUT | MACH_RCV_OVERWRITE | \
				  MACH_RCV_LARGE | MACH_RCV_LARGE_IDENTITY | \
				  MACH_RCV_TRAILER_MASK)

#define MACH_MSG_OPTION_USER	 (MACH_SEND_USER | MACH_RCV_USER)
#endif

/* 
 * XXXMAC: note that in the case of MACH_RCV_TRAILER_LABELS, 
 * we just fall through to mach_msg_max_trailer_t.
 * This is correct behavior since mach_msg_max_trailer_t is defined as
 * mac_msg_mac_trailer_t which is used for the LABELS trailer.
 * It also makes things work properly if MACH_RCV_TRAILER_LABELS is ORed 
 * with one of the other options.
 */

#define REQUESTED_TRAILER_SIZE_NATIVE(y)                        \
        ((mach_msg_trailer_size_t)                              \
         ((GET_RCV_ELEMENTS(y) == MACH_RCV_TRAILER_NULL) ?      \
          sizeof(mach_msg_trailer_t) :                          \
          ((GET_RCV_ELEMENTS(y) == MACH_RCV_TRAILER_SEQNO) ?    \
           sizeof(mach_msg_seqno_trailer_t) :                   \
          ((GET_RCV_ELEMENTS(y) == MACH_RCV_TRAILER_SENDER) ?   \
           sizeof(mach_msg_security_trailer_t) :                \
           ((GET_RCV_ELEMENTS(y) == MACH_RCV_TRAILER_AUDIT) ?   \
            sizeof(mach_msg_audit_trailer_t) :                  \
            ((GET_RCV_ELEMENTS(y) == MACH_RCV_TRAILER_CTX) ?    \
             sizeof(mach_msg_context_trailer_t) :               \
             ((GET_RCV_ELEMENTS(y) == MACH_RCV_TRAILER_AV) ?    \
              sizeof(mach_msg_mac_trailer_t) :                  \
             sizeof(mach_msg_max_trailer_t))))))))

#define REQUESTED_TRAILER_SIZE(y) REQUESTED_TRAILER_SIZE_NATIVE(y)

/*
 *  Much code assumes that mach_msg_return_t == kern_return_t.
 *  This definition is useful for descriptive purposes.
 *
 *  See <mach/error.h> for the format of error codes.
 *  IPC errors are system 4.  Send errors are subsystem 0;
 *  receive errors are subsystem 1.  The code field is always non-zero.
 *  The high bits of the code field communicate extra information
 *  for some error codes.  MACH_MSG_MASK masks off these special bits.
 */

typedef kern_return_t mach_msg_return_t;

#define MACH_MSG_SUCCESS		0x00000000


#define	MACH_MSG_MASK			0x00003e00
		/* All special error code bits defined below. */
#define	MACH_MSG_IPC_SPACE		0x00002000
		/* No room in IPC name space for another capability name. */
#define	MACH_MSG_VM_SPACE		0x00001000
		/* No room in VM address space for out-of-line memory. */
#define	MACH_MSG_IPC_KERNEL		0x00000800
		/* Kernel resource shortage handling an IPC capability. */
#define	MACH_MSG_VM_KERNEL		0x00000400
		/* Kernel resource shortage handling out-of-line memory. */
#define MACH_MSG_INVALID_RT_DESCRIPTOR	0x00000200
		/* Descriptor has an option incompatible with RT
		   behavior */

#define MACH_SEND_IN_PROGRESS		0x10000001
		/* Thread is waiting to send.  (Internal use only.) */
#define MACH_SEND_INVALID_DATA		0x10000002
		/* Bogus in-line data. */
#define MACH_SEND_INVALID_DEST		0x10000003
		/* Bogus destination port. */
#define MACH_SEND_TIMED_OUT		0x10000004
		/* Message not sent before timeout expired. */
#define MACH_SEND_INVALID_VOUCHER	0x10000005
		/* Bogus voucher port. */
#define MACH_SEND_INTERRUPTED		0x10000007
		/* Software interrupt. */
#define MACH_SEND_MSG_TOO_SMALL		0x10000008
		/* Data doesn't contain a complete message. */
#define MACH_SEND_INVALID_REPLY		0x10000009
		/* Bogus reply port. */
#define MACH_SEND_INVALID_RIGHT		0x1000000a
		/* Bogus port rights in the message body. */
#define MACH_SEND_INVALID_NOTIFY	0x1000000b
		/* Bogus notify port argument. */
#define MACH_SEND_INVALID_MEMORY	0x1000000c
		/* Invalid out-of-line memory pointer. */
#define MACH_SEND_NO_BUFFER		0x1000000d
		/* No message buffer is available. */
#define MACH_SEND_TOO_LARGE		0x1000000e
		/* Send is too large for port */
#define MACH_SEND_INVALID_TYPE		0x1000000f
		/* Invalid msg-type specification. */
#define MACH_SEND_INVALID_HEADER	0x10000010
		/* A field in the header had a bad value. */
#define MACH_SEND_INVALID_TRAILER	0x10000011
		/* The trailer to be sent does not match kernel format. */

#define MACH_SEND_INVALID_RT_OOL_SIZE	0x10000015
		/* The OOL buffer size is too large for RT behavior */

#define MACH_RCV_IN_PROGRESS		0x10004001
		/* Thread is waiting for receive.  (Internal use only.) */
#define MACH_RCV_INVALID_NAME		0x10004002
		/* Bogus name for receive port/port-set. */
#define MACH_RCV_TIMED_OUT		0x10004003
		/* Didn't get a message within the timeout value. */
#define MACH_RCV_TOO_LARGE		0x10004004
		/* Message buffer is not large enough for inline data. */
#define MACH_RCV_INTERRUPTED		0x10004005
		/* Software interrupt. */
#define MACH_RCV_PORT_CHANGED		0x10004006
		/* Port moved into a set during the receive. */
#define MACH_RCV_INVALID_NOTIFY		0x10004007
		/* Bogus notify port argument. */
#define MACH_RCV_INVALID_DATA		0x10004008
		/* Bogus message buffer for inline data. */
#define MACH_RCV_PORT_DIED		0x10004009
		/* Port/set was sent away/died during receive. */
#define	MACH_RCV_IN_SET			0x1000400a
		/* Port is a member of a port set. */
#define	MACH_RCV_HEADER_ERROR		0x1000400b
		/* Error receiving message header.  See special bits. */
#define	MACH_RCV_BODY_ERROR		0x1000400c
		/* Error receiving message body.  See special bits. */
#define	MACH_RCV_INVALID_TYPE		0x1000400d
		/* Invalid msg-type specification in scatter list. */
#define	MACH_RCV_SCATTER_SMALL		0x1000400e
		/* Out-of-line overwrite region is not large enough */
#define MACH_RCV_INVALID_TRAILER	0x1000400f
		/* trailer type or number of trailer elements not supported */
#define MACH_RCV_IN_PROGRESS_TIMED      0x10004011
                /* Waiting for receive with timeout. (Internal use only.) */

extern mach_msg_return_t	mach_msg_overwrite_trap(
					mach_msg_header_t *msg,
					mach_msg_option_t option,
					mach_msg_size_t send_size,
					mach_msg_size_t rcv_size,
					mach_port_name_t rcv_name,
					mach_msg_timeout_t timeout,
					mach_port_name_t notify,
					mach_msg_header_t *rcv_msg,
					mach_msg_size_t rcv_limit);

extern mach_msg_return_t	mach_msg_overwrite(
					mach_msg_header_t *msg,
					mach_msg_option_t option,
					mach_msg_size_t send_size,
					mach_msg_size_t rcv_size,
					mach_port_name_t rcv_name,
					mach_msg_timeout_t timeout,
					mach_port_name_t notify,
					mach_msg_header_t *rcv_msg,
					mach_msg_size_t rcv_limit);

extern mach_msg_return_t	mach_msg_trap(
					mach_msg_header_t *msg,
					mach_msg_option_t option,
					mach_msg_size_t send_size,
					mach_msg_size_t rcv_size,
					mach_port_name_t rcv_name,
					mach_msg_timeout_t timeout,
					mach_port_name_t notify);

extern mach_msg_return_t	mach_msg(
					mach_msg_header_t *msg,
					mach_msg_option_t option,
					mach_msg_size_t send_size,
					mach_msg_size_t rcv_size,
					mach_port_name_t rcv_name,
					mach_msg_timeout_t timeout,
					mach_port_name_t notify);


#if defined(_KERNEL) && defined(MACH_INTERNAL)
struct thread_shuttle;

extern mach_msg_return_t	mach_msg_receive_results(
					struct thread_shuttle *thread);
#endif

#endif	/* _MACH_MESSAGE_H_ */

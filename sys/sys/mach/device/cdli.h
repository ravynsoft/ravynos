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
/*
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _SYS_CDLI_H
#define _SYS_CDLI_H

#include <mach/mach_types.h>
#include <types.h>
#include <kern/spl.h>
#include <kern/misc_protos.h>
#include <kern/kalloc.h>
#include <device/cdli_err.h>
#include <device/mbuf.h>
#include <device/ndd.h>

struct cdli_inits
{
	int	(*init_rtn)(void);
};
extern struct cdli_inits cdli_inits[];

struct ns_user {
	void			(*isr)(struct ndd *,
				       struct mbuf *,
				       caddr_t, caddr_t); /* protocol input function */
	caddr_t			isr_data;/* arg to isr			*/
        struct ifqueue 		*protoq; /* input queue, may be NULL	*/
	u_long			pkt_format; /* specifies hdr presentation */
	u_long			netisr;	 /* isr number for schednetisr	*/
	struct ifnet		*ifp;    /* ifnet ptr for socket users	*/
};
typedef struct ns_user ns_user_t;

/* values for pkt_format */
#define	NS_PROTO		0x0001
#define NS_PROTO_SNAP		0x0002
#define	NS_INCLUDE_LLC		0x0004
#define	NS_INCLUDE_MAC		0x0008
#define	NS_HANDLE_NON_UI	0x0100

struct nd_dmxstats {
	u_long		nd_nofilter;	/* packets dropped due to no user */
	u_long		nd_nobufs;	/* packets dropped due to no buffers */
	u_long		nd_bcast;	/* # of broadcast packets received */
	u_long		nd_mcast;	/* # of multicast packets received */
};
typedef struct nd_dmxstats nd_dmxstats_t;

struct ns_statuser {
	void		(*isr)(caddr_t, caddr_t); /* status input function	*/
	caddr_t		isr_data;	/* arg to isr			*/
};
typedef struct ns_statuser ns_statuser_t;

struct ns_demuxer {
	struct ns_demuxer *nd_next;/* link em together			   */
	u_short	nd_inuse;	   /* any users?			   */
	u_short	nd_use_nsdmx;	   /* boolean- true => common dmx services */
	int	(*nd_add_filter)(struct ndd *,
				 caddr_t,
				 int,
				 struct ns_user *); /* add func for receive filter (ie sap) */
	int	(*nd_del_filter)(struct ndd *,
				 caddr_t,
				 int); /* delete func for removing filter */
	int	(*nd_add_status)(struct ndd *,
				 caddr_t,
				 int,
				 struct ns_statuser *);	/* add func for receive filter (ie sap) */
	int	(*nd_del_status)(struct ndd *,
				 caddr_t,
				 int);	/* delete func for removing filter	   */
	void	(*nd_receive)(struct ndd *, struct mbuf *); /* parser/demuxer for input packets     */
	void	(*nd_status)(struct ndd *, int); /* asynchronous status handler          */
	void	(*nd_response)(struct ndd *, struct mbuf *, int); /* XID and TEST response function       */
	struct nd_dmxstats nd_dmxstats;	/* common demuxer statistics       */
	u_int	nd_speclen;	   /* length of demuxer specific stats     */
	caddr_t	nd_specstats;	   /* ptr to demuxer spcecific stats       */
	u_long	nd_type;	   /* NDD type of this demuxer		   */
};
typedef struct ns_demuxer ns_demuxer_t;


#define	NS_8022_LLC_DSAP	(0x00000001)	/* filter on DSAP only 	*/
#define	NS_8022_LLC_DSAP_SNAP	(0x00000002)	/* filter on DSAP & SNAP*/
#define	NS_TAP			(0x00000003)	/* network tap		*/
#define	NS_ETHERTYPE		(0x00000004)	/* ethertype only 	*/

struct ns_8022 {
	u_int		filtertype;
	u_char		dsap;		/* DSAP				*/
	u_char		orgcode[3];	/* SNAP organization code	*/
	u_short		ethertype;	/* SNAP ethertype		*/
};
typedef struct ns_8022 ns_8022_t;

#define	NS_STATUS_MASK		(0x00000001)	/* status mask filtering*/

struct ns_com_status {
	u_int		filtertype;	/* type of filter		*/
	u_int 		mask;		/* status code mask value 	*/
	struct com_status_user	*sid;	/* returned user status id	*/
};
typedef struct ns_com_status ns_com_status_t;

#define ND_CONFIG_VERSION_1	0x01

typedef struct nd_config {
	int	version;
	int	errcode;
	int	ndd_type;	/* type of demuxer. see sys/ndd.h */
} nd_config_t;

#define CFG_INIT	1
#define CFG_TERM	2

#include <device/cdli_proto.h>	/* Placement for ns_* and nd_* structs */

#if CPUS > 1
#define CDLI_LOCK_INIT(x)	simple_lock_init(x, ETAP_IO_CDLI)
#define CDLI_LOCK(x)		simple_lock(x)
#define CDLI_UNLOCK(x)		simple_unlock(x)
#define CDLI_IS_LOCKED(l)	((l).lock_data)
#else
#define CDLI_LOCK_INIT(x)
#define CDLI_LOCK(x)
#define CDLI_UNLOCK(x)
#define CDLI_IS_LOCKED(l)	1
#endif

#define	IF_DEQUEUE_M(q, m)	if_dequeue_m((q), &(m))
#endif /* _SYS_CDLI_H */

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
#ifndef _KERN_CDLI_PROTO_H_
#define _KERN_CDLI_PROTO_H_

#include <device/if_ether.h>
#include <device/mbuf.h>
#include <device/ndd.h>
#include <device/nd_lan.h>
#include <device/cdli_err.h>

/* Prototypes */

/* From cdli.c */
int ns_init(void);
struct ns_demuxer *ns_get_demuxer(u_short);
int ns_add_demux(u_long, struct ns_demuxer *);
int ns_del_demux(u_int);
int ns_add_filter(struct ndd *, caddr_t, int, struct ns_user *);
int ns_del_filter(struct ndd *, caddr_t, int);
int ns_add_status(struct ndd *, caddr_t, int, struct ns_statuser *);
int ns_del_status(struct ndd *, caddr_t, int);
int ns_attach(struct ndd *);
int ns_detach(struct ndd *);
struct ndd *ns_locate_wo_lock(char *);
struct ndd *ns_locate(char *);
int ns_alloc(char *, struct ndd **);
void ns_free(struct ndd *);
void ether_input(struct ifnet *, struct ether_header *, struct mbuf *);
void if_dequeue_m(struct ifqueue *, struct mbuf **);

/* From eth_demux.c */
int nd_eth_config(char *, int);
int eth_config(int, struct nd_config);
int eth_add_filter(struct ndd *, struct ns_8022 *, int,
	       struct ns_user *);
int eth_del_filter(struct ndd *, struct ns_8022 *, int);
int eth_add_status(struct ndd *, struct ns_com_status *,
		   int, struct ns_statuser *);
int eth_del_status(struct ndd *, struct ns_com_status *, int);
struct eth_user	*eth_get_user(u_short, struct ndd *);
int eth_add_ethertype(struct ndd *, struct ns_8022 *, struct ns_user *);
int eth_del_ethertype(struct ndd *, struct ns_8022 *);
int eth_add_tap(struct ndd *, struct ns_user *);
int eth_del_tap(struct ndd *);
void eth_receive(struct ndd *, caddr_t);
int eth_std_receive(struct ndd *, caddr_t, struct ether_header *);
void eth_response(struct ndd *, caddr_t, int);
int eth_dmx_init(struct ndd *);
int eth_add_demuxer(int);

/* From nd_lan.c */
int dmx_init(struct ndd *);
void dmx_term(struct ndd *);
void dmx_8022_receive(struct ndd *, struct mbuf *, int);
void dmx_non_ui(struct ndd *, struct mbuf *, int);
int dmx_8022_add_filter(struct ndd *, struct ns_8022 *, struct ns_user *);
int dmx_8022_del_filter(struct ndd *, struct ns_8022 *);
int dmx_add_status(struct ndd *, struct ns_com_status *, struct ns_statuser *);
int dmx_del_status(struct ndd *, struct ns_com_status *);
void dmx_status(struct ndd *, struct ndd_statblk *);
struct ns_8022_user *dmx_8022_get_user(struct ns_8022 *, struct ndd *);
int dmx_init(struct ndd *);
void dmx_term(struct ndd *);
int nd_config_proto(u_short, struct config_proto *);
int nd_config_sap(u_short, u_short, struct config_proto *, u_short *);
int nd_find_af(u_short, u_short);

/* From devices files */
int cdli_ns8390init(void);

#endif

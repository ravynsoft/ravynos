#ifndef _sw_socket_h
#define _sw_socket_h

/*
 * Copyright 2003, 2004 Porchdog Software. All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without modification,
 *	are permitted provided that the following conditions are met:
 *
 *		1. Redistributions of source code must retain the above copyright notice,
 *		   this list of conditions and the following disclaimer.
 *		2. Redistributions in binary form must reproduce the above copyright notice,
 *		   this list of conditions and the following disclaimer in the documentation
 *		   and/or other materials provided with the distribution.
 *
 *	THIS SOFTWARE IS PROVIDED BY PORCHDOG SOFTWARE ``AS IS'' AND ANY
 *	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *	IN NO EVENT SHALL THE HOWL PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 *	OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *	OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *	OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	The views and conclusions contained in the software and documentation are those
 *	of the authors and should not be interpreted as representing official policies,
 *	either expressed or implied, of Porchdog Software.
 */

#include <salt/platform.h>
#include <salt/address.h>


#ifdef __cplusplus
extern "C"
{
#endif


struct											_sw_socket;
typedef struct _sw_socket				*	sw_socket;
struct											_sw_socket_options;
typedef struct _sw_socket_options	*	sw_socket_options;
typedef sw_uint16							sw_port;



sw_result HOWL_API
sw_tcp_socket_init(
				sw_socket	*	self);


sw_result HOWL_API
sw_tcp_socket_init_with_desc(
				sw_socket	*	self,
				sw_sockdesc_t	desc);


sw_result HOWL_API
sw_udp_socket_init(
				sw_socket	*	self);


sw_result HOWL_API
sw_multicast_socket_init(
				sw_socket	*	self);


sw_result HOWL_API
sw_socket_fina(
				sw_socket	self);


sw_result HOWL_API
sw_socket_bind(
				sw_socket			self,
				sw_ipv4_address	address,
				sw_port			port);


sw_result HOWL_API
sw_socket_join_multicast_group(
				sw_socket			self,
				sw_ipv4_address	local_address,
				sw_ipv4_address	multicast_address,
				sw_uint32			ttl);


sw_result HOWL_API
sw_socket_leave_multicast_group(
				sw_socket	self);


sw_result HOWL_API
sw_socket_listen(
				sw_socket	self,
				int			qsize);


sw_result HOWL_API
sw_socket_connect(
				sw_socket			self,
				sw_ipv4_address	address,
				sw_port			port);


sw_result HOWL_API
sw_socket_accept(
				sw_socket		self,
				sw_socket	*	socket);


sw_result HOWL_API
sw_socket_send(
				sw_socket		self,
				sw_octets		buffer,
				sw_size_t		len,
				sw_size_t	*	bytesWritten);


sw_result HOWL_API
sw_socket_sendto(
				sw_socket			self,
				sw_octets			buffer,
				sw_size_t			len,
				sw_size_t		*	bytesWritten,
				sw_ipv4_address	to,
				sw_port			port);


sw_result HOWL_API
sw_socket_recv(
				sw_socket		self,
				sw_octets		buffer,
				sw_size_t		max,
				sw_size_t	*	len);


sw_result HOWL_API
sw_socket_recvfrom(
				sw_socket				self,
				sw_octets				buffer,
				sw_size_t				max,
				sw_size_t			*	len,
				sw_ipv4_address	*	from,
				sw_port			*	port,
				sw_ipv4_address	*	dest,
				sw_uint32				*	interface_index);


sw_result HOWL_API
sw_socket_set_blocking_mode(
				sw_socket	self,
				sw_bool	blocking_mode);


sw_result HOWL_API
sw_socket_set_options(
				sw_socket				self,
				sw_socket_options		options);


sw_ipv4_address HOWL_API
sw_socket_ipv4_address(
				sw_socket	self);


sw_port HOWL_API
sw_socket_port(
				sw_socket	self);


sw_sockdesc_t HOWL_API
sw_socket_desc(
				sw_socket	self);


sw_result HOWL_API
sw_socket_close(
				sw_socket	self);


sw_result HOWL_API
sw_socket_options_init(
				sw_socket_options	*	self);


sw_result HOWL_API
sw_socket_options_fina(
				sw_socket_options	self);


sw_result HOWL_API
sw_socket_options_set_debug(
				sw_socket_options		self,
				sw_bool					val);


sw_result HOWL_API
sw_socket_options_set_nodelay(
				sw_socket_options		self,
				sw_bool					val);


sw_result HOWL_API
sw_socket_options_set_dontroute(
				sw_socket_options		self,
				sw_bool					val);


sw_result HOWL_API
sw_socket_options_set_keepalive(
				sw_socket_options		self,
				sw_bool					val);


sw_result HOWL_API
sw_socket_options_set_linger(
				sw_socket_options		self,
				sw_bool					onoff,
				sw_uint32					linger);


sw_result HOWL_API
sw_socket_options_set_reuseaddr(
				sw_socket_options		self,
				sw_bool					val);


sw_result HOWL_API
sw_socket_options_set_rcvbuf(
				sw_socket_options		self,
				sw_uint32					val);


sw_result HOWL_API
sw_socket_options_set_sndbuf(
				sw_socket_options		self,
				sw_uint32					val);


int
sw_socket_error_code(void);


#define	SW_E_SOCKET_BASE		0x80000200
#define	SW_E_SOCKET				(SW_E_SOCKET_BASE) + 1
#define	SW_E_BIND				(SW_E_SOCKET_BASE) + 2
#define	SW_E_GETSOCKNAME		(SW_E_SOCKET_BASE) + 3
#define	SW_E_ADD_MEMBERSHIP	(SW_E_SOCKET_BASE) + 4
#define	SW_E_MULTICAST_TTL	(SW_E_SOCKET_BASE) + 5
#define	SW_E_NOCONNECTION		(SW_E_SOCKET_BASE) + 6
#define	SW_E_INPROGRESS		(SW_E_SOCKET_BASE) + 7


#ifdef __cplusplus
}
#endif


#endif

#ifndef _sw_salt_h
#define _sw_salt_h

/*
 * Copyright 2003, 2004 Porchdog Software, Inc. All rights reserved.
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
 *	either expressed or implied, of Porchdog Software, Inc.
 */

#include <salt/platform.h>
#include <salt/time.h>


#ifdef __cplusplus
extern "C"
{
#endif


typedef enum _sw_socket_event
{
	SW_SOCKET_READ		=	(1 << 0),
	SW_SOCKET_WRITE	=	(1 << 1),
	SW_SOCKET_OOB		=	(1 << 2)
} sw_socket_event;


struct							_sw_salt;
typedef struct _sw_salt	*	sw_salt;
struct							_sw_socket;
struct							_sw_timer;
struct							_sw_network_interface;
struct							_sw_signal;

typedef sw_opaque				sw_socket_handler;
typedef sw_result
(HOWL_API *sw_socket_handler_func)(
									sw_socket_handler		handler,
									sw_salt					salt,
									struct _sw_socket	*	socket,
									sw_socket_event		events,
									sw_opaque				extra);


typedef sw_opaque				sw_timer_handler;
typedef sw_result
(HOWL_API *sw_timer_handler_func)(
									sw_timer_handler		handler,
									sw_salt					salt,
									struct _sw_timer	*	timer,
									sw_time					timeout,
									sw_opaque				extra);

typedef sw_opaque				sw_network_interface_handler;
typedef sw_result
(HOWL_API *sw_network_interface_handler_func)(
									sw_network_interface_handler		handler,
									sw_salt									salt,
									struct _sw_network_interface	*	netif,
									sw_opaque								extra);

typedef sw_opaque				sw_signal_handler;
typedef sw_result
(HOWL_API *sw_signal_handler_func)(
									sw_signal_handler		handler,
									sw_salt					salt,
									struct _sw_signal	*	signal,
									sw_opaque				extra);


sw_result HOWL_API
sw_salt_init(
				sw_salt		*	self,
				int				argc,
				char			**	argv);


sw_result HOWL_API
sw_salt_fina(
				sw_salt	self);


sw_result HOWL_API
sw_salt_register_socket(
				sw_salt						self,
				struct _sw_socket		*	socket,
				sw_socket_event			events,
				sw_socket_handler			handler,
				sw_socket_handler_func	func,
				sw_opaque					extra);


sw_result HOWL_API
sw_salt_unregister_socket(
				sw_salt						self,
				struct _sw_socket		*	socket);


sw_result HOWL_API
sw_salt_register_timer(
				sw_salt						self,
				struct _sw_timer		*	timer,
				sw_time						timeout,
				sw_timer_handler			handler,
				sw_timer_handler_func	func,
				sw_opaque					extra);


sw_result HOWL_API
sw_salt_unregister_timer(
				sw_salt						self,
				struct _sw_timer		*	timer);


sw_result HOWL_API
sw_salt_register_network_interface(
				sw_salt										self,
				struct _sw_network_interface		*	netif,
				sw_network_interface_handler			handler,
				sw_network_interface_handler_func	func,
				sw_opaque									extra);


sw_result HOWL_API
sw_salt_unregister_network_interface_handler(
				sw_salt						self);


sw_result HOWL_API
sw_salt_register_signal(
				sw_salt						self,
				struct _sw_signal	*		signal,
				sw_signal_handler			handler,
				sw_signal_handler_func	func,
				sw_opaque					extra);


sw_result HOWL_API
sw_salt_unregister_signal(
				sw_salt						self,
				struct _sw_signal	*		signal);


sw_result HOWL_API
sw_salt_lock(
				sw_salt	self);


sw_result HOWL_API
sw_salt_unlock(
				sw_salt	self);


sw_result HOWL_API
sw_salt_step(
				sw_salt		self,
				sw_uint32	*	msec);


sw_result HOWL_API
sw_salt_run(
				sw_salt	self);


sw_result HOWL_API
sw_salt_stop_run(
				sw_salt	self);


#define SW_FALSE		0
#define SW_TRUE		1
#define SW_OKAY		0


/*
 * error codes
 */
#define	SW_E_CORE_BASE					0x80000000
#define	SW_E_UNKNOWN					(SW_E_CORE_BASE) + 1
#define	SW_E_INIT						(SW_E_CORE_BASE) + 2
#define	SW_E_MEM							(SW_E_CORE_BASE) + 3
#define	SW_E_EOF							(SW_E_CORE_BASE) + 4
#define	SW_E_NO_IMPL					(SW_E_CORE_BASE) + 5
#define	SW_E_FILE_LOCKED				(SW_E_CORE_BASE) + 6
#define	SW_E_PROTOCOL_NOT_FOUND		(SW_E_CORE_BASE) + 7


#ifdef __cplusplus
}
#endif


#endif

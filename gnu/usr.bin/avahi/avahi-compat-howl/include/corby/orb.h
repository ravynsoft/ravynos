#ifndef _corby_orb_h
#define _corby_orb_h

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

#include <salt/salt.h>
#include <salt/address.h>
#include <salt/socket.h>
#include <corby/corby.h>


#ifdef __cplusplus
extern "C"
{
#endif


struct									_sw_corby_orb;
typedef struct _sw_corby_orb	*	sw_corby_orb;
typedef sw_opaque						sw_corby_orb_observer;
typedef sw_opaque						sw_corby_servant;
struct									_sw_corby_object;
struct									_sw_corby_channel;
struct									_sw_corby_message;
struct									_sw_corby_buffer;

typedef sw_result
(HOWL_API *sw_corby_orb_accept_channel_func)(
								sw_corby_orb						orb,
								struct _sw_corby_channel	*	channel);


typedef struct _sw_corby_orb_delegate
{
	sw_opaque								m_delegate;
	sw_corby_orb_accept_channel_func	m_accept_channel_func;
	sw_opaque								m_extra;
} * sw_corby_orb_delegate;


typedef struct _sw_corby_orb_config
{
	sw_string	m_name;
	sw_uint32	m_tag;
	sw_string	m_host;
	sw_port	m_port;
	sw_string	m_options;
} sw_corby_orb_config;


typedef sw_result
(HOWL_API *sw_corby_servant_cb)(
				sw_corby_servant					servant,
				sw_salt								salt,
				sw_corby_orb						orb,
				struct _sw_corby_channel	*	channel,
				struct _sw_corby_message	*	message,
				struct _sw_corby_buffer		*	buffer,
				sw_const_string					op,
				sw_uint32							op_len,
				sw_uint32							request_id,
				sw_uint8							endian);


typedef sw_result
(HOWL_API *sw_corby_orb_observer_func)(
				sw_corby_orb_observer			handler,
				sw_salt								salt,
				sw_corby_orb						orb,
				struct _sw_corby_channel	*	channel,
				sw_opaque_t							extra);


sw_result HOWL_API
sw_corby_orb_init(
				sw_corby_orb						*	self,
				sw_salt									salt,
				const sw_corby_orb_config		*	config,
            sw_corby_orb_observer				observer,
            sw_corby_orb_observer_func			func,
            sw_opaque_t								extra);


sw_result HOWL_API
sw_corby_orb_fina(
				sw_corby_orb	self);


sw_result HOWL_API
sw_corby_orb_register_servant(
				sw_corby_orb					self,
				sw_corby_servant				servant,
            sw_corby_servant_cb			cb,
				sw_const_string				oid,
				struct _sw_corby_object	**	object,
				sw_const_string				protocol_name);


sw_result HOWL_API
sw_corby_orb_unregister_servant(
				sw_corby_orb		self,
				sw_const_string	oid);


sw_result HOWL_API
sw_corby_orb_register_bidirectional_object(
				sw_corby_orb					self,
				struct _sw_corby_object	*	object);


sw_result HOWL_API
sw_corby_orb_register_channel(
				sw_corby_orb						self,
				struct _sw_corby_channel	*	channel);


sw_corby_orb_delegate HOWL_API
sw_corby_orb_get_delegate(
				sw_corby_orb						self);


sw_result HOWL_API
sw_corby_orb_set_delegate(
				sw_corby_orb						self,
				sw_corby_orb_delegate			delegate);


sw_result HOWL_API
sw_corby_orb_set_observer(
				sw_corby_orb						self,
				sw_corby_orb_observer			observer,
				sw_corby_orb_observer_func		func,
				sw_opaque_t							extra);


sw_result HOWL_API
sw_corby_orb_protocol_to_address(
				sw_corby_orb		self,
				sw_const_string	tag,
				sw_string			addr,
				sw_port		*	port);


sw_result HOWL_API
sw_corby_orb_protocol_to_url(
				sw_corby_orb		self,
				sw_const_string	tag,
				sw_const_string	name,
				sw_string			url,
				sw_size_t			url_len);


sw_result HOWL_API
sw_corby_orb_read_channel(
				sw_corby_orb						self,
				struct _sw_corby_channel	*	channel);


sw_result HOWL_API
sw_corby_orb_dispatch_message(
				sw_corby_orb						self,
				struct _sw_corby_channel	*	channel,
				struct _sw_corby_message	*	message,
				struct _sw_corby_buffer		*	buffer,
				sw_uint8							endian);


#ifdef __cplusplus
}
#endif


#endif

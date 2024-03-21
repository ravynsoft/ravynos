#ifndef _sw_corby_channel_h
#define _sw_corby_channel_h

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
#include <salt/socket.h>
#include <corby/corby.h>
#include <corby/buffer.h>


#ifdef __cplusplus
extern "C"
{
#endif

struct												_sw_corby_channel;
typedef struct _sw_corby_channel			*	sw_corby_channel;
struct												_sw_corby_message;
struct												_sw_corby_profile;
typedef struct _sw_corby_profile			*	sw_corby_profile;
typedef struct _sw_corby_profile const	*	sw_const_corby_profile;


typedef enum _sw_corby_reply_status
{
   SW_CORBY_NO_EXCEPTION		=	0,
   SW_CORBY_SYSTEM_EXCEPTION	=	1,
   SW_CORBY_USER_EXCEPTION		=	2,
   SW_CORBY_LOCATION_FORWARD	=	3
} sw_corby_reply_status;


typedef sw_result
(HOWL_API *sw_corby_channel_will_send_func)(
						sw_corby_channel			channel,
						sw_octets					bytes,
						sw_size_t					len,
						sw_opaque_t					extra);


typedef sw_result
(HOWL_API *sw_corby_channel_did_read_func)(
						sw_corby_channel			channel,
						sw_octets					bytes,
						sw_size_t					len,
						sw_opaque_t					extra);


typedef void
(HOWL_API *sw_corby_channel_cleanup_func)(
						sw_corby_channel			channel);


typedef struct _sw_corby_channel_delegate
{
	sw_opaque_t								m_delegate;
	sw_corby_channel_will_send_func	m_will_send_func;
	sw_corby_channel_did_read_func	m_did_read_func;
	sw_corby_channel_cleanup_func		m_cleanup_func;
	sw_opaque_t								m_extra;
} * sw_corby_channel_delegate;


sw_result HOWL_API
sw_corby_channel_start_request(
							sw_corby_channel				self,
							sw_const_corby_profile		profile,
							struct _sw_corby_buffer	**	buffer,
							sw_const_string				op,
							sw_uint32						oplen,
							sw_bool						reply_expected);


sw_result HOWL_API
sw_corby_channel_start_reply(
							sw_corby_channel				self,
							struct _sw_corby_buffer	**	buffer,
							sw_uint32						request_id,
							sw_corby_reply_status		status);


sw_result HOWL_API
sw_corby_channel_send(
							sw_corby_channel					self,
							struct _sw_corby_buffer		*	buffer,
							sw_corby_buffer_observer		observer,
							sw_corby_buffer_written_func	func,
							sw_opaque_t							extra);


sw_result HOWL_API
sw_corby_channel_recv(
							sw_corby_channel					self,
							sw_salt							*	salt,
							struct _sw_corby_message	**	message,
							sw_uint32						*	request_id,
							sw_string						*	op,
							sw_uint32						*	op_len,
							struct _sw_corby_buffer		**	buffer,
							sw_uint8						*	endian,
							sw_bool							block);


sw_result HOWL_API
sw_corby_channel_last_recv_from(
							sw_corby_channel					self,
							sw_ipv4_address				*	from,
							sw_port						*	from_port);


sw_result HOWL_API
sw_corby_channel_ff(
							sw_corby_channel					self,
							struct _sw_corby_buffer		*	buffer);


sw_socket HOWL_API
sw_corby_channel_socket(
							sw_corby_channel				self);


sw_result HOWL_API
sw_corby_channel_retain(
							sw_corby_channel				self);


sw_result HOWL_API
sw_corby_channel_set_delegate(
							sw_corby_channel				self,
							sw_corby_channel_delegate	delegate);


sw_corby_channel_delegate HOWL_API
sw_corby_channel_get_delegate(
							sw_corby_channel				self);


void HOWL_API
sw_corby_channel_set_app_data(
							sw_corby_channel				self,
							sw_opaque						app_data);


sw_opaque HOWL_API
sw_corby_channel_get_app_data(
							sw_corby_channel				self);


sw_result HOWL_API
sw_corby_channel_fina(
							sw_corby_channel				self);


#ifdef __cplusplus
}
#endif


#endif

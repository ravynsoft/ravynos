#ifndef _corby_object_h
#define _corby_object_h

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
#include <corby/message.h>
#include <corby/channel.h>
#include <corby/buffer.h>
#include <corby/corby.h>

#if defined(__cplusplus)
extern "C"
{
#endif

struct										_sw_corby_orb;
struct										_sw_corby_object;
typedef struct _sw_corby_object	*	sw_corby_object;
typedef sw_opaque							sw_corby_object_send_completion_handler;
typedef void
(HOWL_API *sw_corby_object_send_completion_func)(
												sw_corby_object	object,
												sw_corby_buffer	buffer,
												sw_result			result);



sw_result HOWL_API
sw_corby_object_init_from_url(
							sw_corby_object			*	self,
							struct _sw_corby_orb		*	orb,
							sw_const_string				url,
							sw_socket_options				options,
							sw_uint32							bufsize);


sw_result HOWL_API
sw_corby_object_fina(
							sw_corby_object		self);


sw_result HOWL_API
sw_corby_object_start_request(
							sw_corby_object 		self,
							sw_const_string		op,
							sw_uint32					op_len,
							sw_bool					reply_expected,
							sw_corby_buffer	*	buffer);


sw_result HOWL_API
sw_corby_object_send(
							sw_corby_object					self,
							sw_corby_buffer					buffer,
							sw_corby_buffer_observer		observer,
							sw_corby_buffer_written_func	func,
							sw_opaque							extra);


sw_result HOWL_API
sw_corby_object_recv(
							sw_corby_object		self,
							sw_corby_message	*	message,
                     sw_corby_buffer	*	buffer,
                     sw_uint8				*	endian,
                     sw_bool					block);


sw_result HOWL_API
sw_corby_object_channel(
							sw_corby_object		self,
							sw_corby_channel	*	channel);


sw_result HOWL_API
sw_corby_object_set_channel(
							sw_corby_object		self,
							sw_corby_channel		channel);


#if defined(__cplusplus)
}
#endif


#endif

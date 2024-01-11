#ifndef _sw_corby_buffer_h
#define _sw_corby_buffer_h

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
#include <corby/corby.h>


#ifdef __cplusplus
extern "C"
{
#endif


struct										_sw_corby_buffer;
typedef struct _sw_corby_buffer	*	sw_corby_buffer;
struct										_sw_corby_object;
typedef sw_opaque							sw_corby_buffer_delegate;
typedef sw_opaque							sw_corby_buffer_observer;


typedef enum _sw_corby_buffer_pad
{
	SW_CORBY_BUFFER_PAD_NONE,
	SW_CORBY_BUFFER_PAD_ALIGN_2,
	SW_CORBY_BUFFER_PAD_ALIGN_4,
	SW_CORBY_BUFFER_PAD_ALIGN_8,
	SW_CORBY_BUFFER_PAD_ALIGN_16,
	SW_CORBY_BUFFER_PAD_ALIGN_32
} sw_corby_buffer_pad;


typedef sw_result
(HOWL_API* sw_corby_buffer_written_func)(
								sw_corby_buffer_observer	observer,
								sw_corby_buffer				buffer,
								sw_result						result,
								sw_size_t						bytesWritten,
								sw_opaque_t						extra);


typedef sw_result
(HOWL_API* sw_corby_buffer_overflow_func)(
								sw_corby_buffer_delegate	delegate,
								sw_corby_buffer				buffer,
								sw_uint8						octet,
								sw_uint8					**	base,
								sw_uint8					**	bptr,
								sw_uint8					**	eptr,
								sw_uint8					**	end,
								sw_opaque_t						extra);


typedef sw_result
(HOWL_API* sw_corby_buffer_underflow_func)(
								sw_corby_buffer_delegate	delegate,
								sw_corby_buffer				buffer,
								sw_uint8					*	octet,
								sw_uint8					**	base,
								sw_uint8					**	bptr,
								sw_uint8					**	eptr,
								sw_uint8					**	end,
								sw_opaque_t						extra);


sw_result HOWL_API
sw_corby_buffer_init(
				sw_corby_buffer	*	self);


sw_result HOWL_API
sw_corby_buffer_init_with_size(
				sw_corby_buffer	*	self,
				sw_size_t				size);


sw_result HOWL_API
sw_corby_buffer_init_with_delegate(
				sw_corby_buffer					*	self,
				sw_corby_buffer_delegate			delegate,
				sw_corby_buffer_overflow_func		overflow,
				sw_corby_buffer_underflow_func	underflow,
				sw_opaque_t								extra);


sw_result HOWL_API
sw_corby_buffer_init_with_size_and_delegate(
				sw_corby_buffer					*	self,
				sw_size_t								size,
				sw_corby_buffer_delegate			delegate,
				sw_corby_buffer_overflow_func		overflow,
				sw_corby_buffer_underflow_func	underflow,
				sw_opaque_t								extra);


sw_result HOWL_API
sw_corby_buffer_fina(
				sw_corby_buffer	self);


void HOWL_API
sw_corby_buffer_reset(
				sw_corby_buffer	self);


sw_result HOWL_API
sw_corby_buffer_set_octets(
				sw_corby_buffer	self,
				sw_octets			octets,
				sw_size_t			size);


sw_octets HOWL_API
sw_corby_buffer_octets(
				sw_corby_buffer	self);


sw_size_t HOWL_API
sw_corby_buffer_bytes_used(
				sw_corby_buffer	self);


sw_size_t HOWL_API
sw_corby_buffer_size(
				sw_corby_buffer	self);


sw_result HOWL_API
sw_corby_buffer_put_int8(
				sw_corby_buffer	self,
				sw_int8			val);


sw_result HOWL_API
sw_corby_buffer_put_uint8(
				sw_corby_buffer	self,
				sw_uint8			val);


sw_result HOWL_API
sw_corby_buffer_put_int16(
				sw_corby_buffer	self,
				sw_int16			val);


sw_result HOWL_API
sw_corby_buffer_put_uint16(
				sw_corby_buffer	self,
				sw_uint16			val);


sw_result HOWL_API
sw_corby_buffer_put_int32(
				sw_corby_buffer	self,
				sw_int32			val);


sw_result HOWL_API
sw_corby_buffer_put_uint32(
				sw_corby_buffer	self,
				sw_uint32			val);


sw_result HOWL_API
sw_corby_buffer_put_octets(
				sw_corby_buffer	self,
				sw_const_octets	val,
				sw_size_t			size);


sw_result HOWL_API
sw_corby_buffer_put_sized_octets(
				sw_corby_buffer	self,
				sw_const_octets	val,
				sw_uint32			len);


sw_result HOWL_API
sw_corby_buffer_put_cstring(
				sw_corby_buffer	self,
				sw_const_string	val);


sw_result HOWL_API
sw_corby_buffer_put_object(
				sw_corby_buffer						self,
				const struct _sw_corby_object	*	object);


sw_result HOWL_API
sw_corby_buffer_put_pad(
				sw_corby_buffer						self,
				sw_corby_buffer_pad					pad);


sw_result HOWL_API
sw_corby_buffer_get_int8(
				sw_corby_buffer	self,
				sw_int8		*	val);


sw_result HOWL_API
sw_corby_buffer_get_uint8(
				sw_corby_buffer	self,
				sw_uint8		*	val);


sw_result HOWL_API
sw_corby_buffer_get_int16(
				sw_corby_buffer	self,
				sw_int16		*	val,
            sw_uint8			endian);


sw_result HOWL_API
sw_corby_buffer_get_uint16(
				sw_corby_buffer	self,
				sw_uint16		*	val,
            sw_uint8			endian);


sw_result HOWL_API
sw_corby_buffer_get_int32(
				sw_corby_buffer	self,
				sw_int32		*	val,
            sw_uint8			endian);


sw_result HOWL_API
sw_corby_buffer_get_uint32(
				sw_corby_buffer	self,
				sw_uint32		*	val,
            sw_uint8			endian);


sw_result HOWL_API
sw_corby_buffer_get_octets(
				sw_corby_buffer	self,
				sw_octets			octets,
				sw_size_t			size);


sw_result HOWL_API
sw_corby_buffer_allocate_and_get_sized_octets(
				sw_corby_buffer	self,
				sw_octets		*	val,
				sw_uint32		*	size,
            sw_uint8			endian);


sw_result HOWL_API
sw_corby_buffer_get_zerocopy_sized_octets(
				sw_corby_buffer	self,
				sw_octets		*	val,
				sw_uint32		*	size,
				sw_uint8			endian);


sw_result HOWL_API
sw_corby_buffer_get_sized_octets(
				sw_corby_buffer	self,
				sw_octets			val,
				sw_uint32		*	len,
            sw_uint8			endian);


sw_result HOWL_API
sw_corby_buffer_allocate_and_get_cstring(
				sw_corby_buffer	self,
				sw_string		*	val,
				sw_uint32		*	len,
				sw_uint8			endian);


sw_result HOWL_API
sw_corby_buffer_get_zerocopy_cstring(
				sw_corby_buffer	self,
				sw_string		*	val,
				sw_uint32		*	len,
				sw_uint8			endian);


sw_result HOWL_API
sw_corby_buffer_get_cstring(
				sw_corby_buffer	self,
				sw_string			val,
				sw_uint32		*	len,
				sw_uint8			endian);


sw_result HOWL_API
sw_corby_buffer_get_object(
				sw_corby_buffer				self,
				struct _sw_corby_object	**	object,
				sw_uint8						endian);


#ifdef __cplusplus
}
#endif


#endif

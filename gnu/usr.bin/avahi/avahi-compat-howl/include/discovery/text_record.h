#ifndef _discovery_text_record_h
#define _discovery_text_record_h

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

#if defined(__cplusplus)
extern "C"
{
#endif

#define SW_TEXT_RECORD_MAX_LEN 255


struct															_sw_text_record;
typedef struct _sw_text_record						*	sw_text_record;
struct															_sw_text_record_iterator;
typedef struct _sw_text_record_iterator			*	sw_text_record_iterator;
struct															_sw_text_record_string_iterator;
typedef struct _sw_text_record_string_iterator	*	sw_text_record_string_iterator;


/*
 * Text record APIs
 */
sw_result HOWL_API
sw_text_record_init(
				sw_text_record	*	self);


sw_result HOWL_API
sw_text_record_fina(
				sw_text_record		self);


sw_result HOWL_API
sw_text_record_add_string(
				sw_text_record		self,
				sw_const_string	string);


sw_result HOWL_API
sw_text_record_add_key_and_string_value(
				sw_text_record		self,
				sw_const_string	key,
				sw_const_string	val);


sw_result HOWL_API
sw_text_record_add_key_and_binary_value(
				sw_text_record		self,
				sw_const_string	key,
				sw_octets			val,
				sw_uint32				len);


sw_octets HOWL_API
sw_text_record_bytes(
				sw_text_record		self);


sw_uint32 HOWL_API
sw_text_record_len(
				sw_text_record		self);


/*
 * APIs for iterating through raw text records
 */
sw_result HOWL_API
sw_text_record_iterator_init(
				sw_text_record_iterator	*	self,
				sw_octets						text_record,
				sw_uint32						text_record_len);


sw_result HOWL_API
sw_text_record_iterator_fina(
				sw_text_record_iterator		self);


sw_result HOWL_API
sw_text_record_iterator_next(
				sw_text_record_iterator		self,
				char								key[SW_TEXT_RECORD_MAX_LEN],
				sw_uint8							val[SW_TEXT_RECORD_MAX_LEN],
				sw_uint32					*	val_len);


/*
 * APIs for iterating through stringified text records
 */
sw_result HOWL_API
sw_text_record_string_iterator_init(
				sw_text_record_string_iterator	*	self,
				sw_const_string							text_record_string);


sw_result HOWL_API
sw_text_record_string_iterator_fina(
				sw_text_record_string_iterator	self);


sw_result HOWL_API
sw_text_record_string_iterator_next(
				sw_text_record_string_iterator	self,
				char										key[SW_TEXT_RECORD_MAX_LEN],
				char										val[SW_TEXT_RECORD_MAX_LEN]);


#if defined(__cplusplus)
}
#endif


#endif

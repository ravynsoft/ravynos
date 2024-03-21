#ifndef _salt_address_h
#define _salt_address_h

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


typedef struct _sw_ipv4_address
{
	sw_uint32	m_addr;
} sw_ipv4_address;

typedef sw_uint32 sw_saddr;


sw_ipv4_address HOWL_API
sw_ipv4_address_any(void);


sw_ipv4_address HOWL_API
sw_ipv4_address_loopback(void);


sw_result HOWL_API
sw_ipv4_address_init(
				sw_ipv4_address	*	self);


sw_result HOWL_API
sw_ipv4_address_init_from_saddr(
				sw_ipv4_address	*	self,
				sw_saddr			saddr);


sw_result HOWL_API
sw_ipv4_address_init_from_name(
				sw_ipv4_address		*	self,
				sw_const_string	name);


sw_result HOWL_API
sw_ipv4_address_init_from_address(
				sw_ipv4_address		*	self,
				sw_ipv4_address			addr);


sw_result HOWL_API
sw_ipv4_address_init_from_this_host(
				sw_ipv4_address		*	self);


sw_result HOWL_API
sw_ipv4_address_fina(
				sw_ipv4_address	self);


sw_bool HOWL_API
sw_ipv4_address_is_any(
				sw_ipv4_address	self);


sw_saddr HOWL_API
sw_ipv4_address_saddr(
				sw_ipv4_address	self);


sw_string HOWL_API
sw_ipv4_address_name(
				sw_ipv4_address	self,
				sw_string			name,
				sw_uint32				len);


sw_result HOWL_API
sw_ipv4_address_decompose(
				sw_ipv4_address	self,
				sw_uint8			*	a1,
				sw_uint8			*	a2,
				sw_uint8			*	a3,
				sw_uint8			*	a4);


sw_bool HOWL_API
sw_ipv4_address_equals(
				sw_ipv4_address	self,
				sw_ipv4_address	addr);


#if defined(__cplusplus)
}
#endif


#endif

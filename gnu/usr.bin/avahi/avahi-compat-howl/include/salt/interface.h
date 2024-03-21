#ifndef _sw_interface_h
#define _sw_interface_h

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

#include <salt/address.h>

struct											_sw_network_interface;
typedef struct _sw_network_interface * sw_network_interface;


typedef enum _sw_network_interface_state
{
	SW_NETWORK_INTERFACE_RUNNING		= 0x1,
} sw_network_interface_state;


typedef struct _sw_mac_address
{
	sw_uint8	m_id[6];
} sw_mac_address;


sw_result HOWL_API
sw_network_interfaces(
				sw_uint32					*	count,
				sw_network_interface	**	netifs);


sw_result HOWL_API
sw_network_interfaces_fina(
				sw_uint32						count,
				sw_network_interface	*	netifs);


sw_result HOWL_API
sw_network_interface_fina(
				sw_network_interface		netif);


sw_result  HOWL_API
sw_network_interface_by_name(
				sw_string					name,
				sw_network_interface	*	netif);


sw_result HOWL_API
sw_network_interface_name(
				sw_network_interface		netif,
				sw_string					name,
				sw_uint32						len);


sw_result HOWL_API
sw_network_interface_mac_address(
				sw_network_interface		netif,
				sw_mac_address			*	addr);


sw_result HOWL_API
sw_network_interface_ipv4_address(
				sw_network_interface		netif,
				sw_ipv4_address		*	addr);


sw_result HOWL_API
sw_network_interface_set_ipv4_address(
				sw_network_interface		netif,
				sw_ipv4_address			addr);


sw_result HOWL_API
sw_network_interface_index(
				sw_network_interface		netif,
				sw_uint32					*	index);


sw_result HOWL_API
sw_network_interface_linked(
				sw_network_interface		netif,
				sw_bool					*	linked);


sw_result HOWL_API
sw_network_interface_up(
				sw_network_interface		netif);


#endif

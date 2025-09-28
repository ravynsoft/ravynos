#ifndef _rendezvous_rendezvous_h
#define _rendezvous_rendezvous_h

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

#include <discovery/discovery.h>

#define _sw_rendezvous							_sw_discovery
#define sw_rendezvous							sw_discovery
#define sw_rendezvous_publish_domain_id	sw_discovery_publish_domain_id
#define sw_rendezvous_publish_host_id		sw_discovery_publish_host_id
#define sw_rendezvous_publish_id				sw_discovery_publish_id
#define sw_rendezvous_browse_id				sw_discovery_browse_id
#define sw_rendezvous_resolve_id				sw_discovery_resolve_id
#define sw_rendezvous_publish_status		sw_discovery_publish_status
#define SW_RENDEZVOUS_PUBLISH_STARTED		SW_DISCOVERY_PUBLISH_STARTED
#define SW_RENDEZVOUS_PUBLISH_STOPPED		SW_DISCOVERY_PUBLISH_STOPPED
#define SW_RENDEZVOUS_PUBLISH_NAME_COLLISION	SW_DISCOVERY_PUBLISH_NAME_COLLISION
#define SW_RENDEZVOUS_PUBLISH_INVALID		SW_DISCOVERY_PUBLISH_INVALID
#define sw_rendezvous_browse_status			sw_discovery_browse_status
#define SW_RENDEZVOUS_BROWSE_INVALID		SW_DISCOVERY_BROWSE_INVALID
#define SW_RENDEZVOUS_BROWSE_RELEASE		SW_DISCOVERY_BROWSE_RELEASE
#define SW_RENDEZVOUS_BROWSE_ADD_DOMAIN	SW_DISCOVERY_BROWSE_ADD_DOMAIN
#define SW_RENDEZVOUS_BROWSE_ADD_DEFAULT_DOMAIN	SW_DISCOVERY_BROWSE_ADD_DEFAULT_DOMAIN
#define SW_RENDEZVOUS_BROWSE_REMOVE_DOMAIN	SW_DISCOVERY_BROWSE_REMOVE_DOMAIN
#define SW_RENDEZVOUS_BROWSE_ADD_SERVICE		SW_DISCOVERY_BROWSE_ADD_SERVICE
#define SW_RENDEZVOUS_BROWSE_REMOVE_SERVICE	SW_DISCOVERY_BROWSE_REMOVE_SERVICE
#define SW_RENDEZVOUS_BROWSE_RESOLVED			SW_DISCOVERY_BROWSE_RESOLVED
#define sw_rendezvous_publish_domain_handler	sw_discovery_publish_domain_handler
#define sw_rendezvous_publish_domain_reply	sw_discovery_publish_domain_reply
#define sw_rendezvous_publish_host_handler	sw_discovery_publish_host_handler
#define sw_rendezvous_publish_host_reply		sw_discovery_publish_host_reply
#define sw_rendezvous_publish_handler			sw_discovery_publish_handler
#define sw_rendezvous_publish_reply				sw_discovery_publish_reply
#define sw_rendezvous_browse_handler			sw_discovery_browse_handler
#define sw_rendezvous_browse_reply				sw_discovery_browse_reply
#define sw_rendezvous_resolve_handler			sw_discovery_resolve_handler
#define sw_rendezvous_resolve_reply				sw_discovery_resolve_reply
#define sw_rendezvous_init							sw_discovery_init
#define sw_rendezvous_fina							sw_discovery_fina
#define sw_rendezvous_publish_domain			sw_discovery_publish_domain
#define sw_rendezvous_stop_publish_domain		sw_discovery_stop_publish_domain
#define sw_rendezvous_publish_host				sw_discovery_publish_host
#define sw_rendezvous_stop_publish_host		sw_discovery_stop_publish_host
#define sw_rendezvous_publish						sw_discovery_publish
#define sw_rendezvous_publish_update			sw_discovery_publish_update
#define sw_rendezvous_stop_publish				sw_discovery_stop_publish
#define sw_rendezvous_browse_domains			sw_discovery_browse_domains
#define sw_rendezvous_stop_browse_domains		sw_discovery_stop_browse_domains
#define sw_rendezvous_browse_services			sw_discovery_browse
#define sw_rendezvous_stop_browse_services	sw_discovery_stop_browse
#define sw_rendezvous_resolve						sw_discovery_resolve
#define sw_rendezvous_stop_resolve				sw_discovery_stop_resolve
#define sw_rendezvous_run							sw_discovery_run
#define sw_rendezvous_stop_run					sw_discovery_stop_run
#define sw_rendezvous_socket						sw_discovery_socket
#define sw_rendezvous_read_socket				sw_discovery_read_socket
#define sw_rendezvous_salt							sw_discovery_salt
#define SW_RENDEZVOUS_E_BASE						SW_DISCOVERY_E_BASE
#define SW_RENDEZVOUS_E_UNKNOWN					SW_DISCOVERY_E_UNKNOWN
#define SW_RENDEZVOUS_E_NO_SUCH_NAME			SW_DISCOVERY_E_NO_SUCH_NAME
#define SW_RENDEZVOUS_E_NO_MEM					SW_DISCOVERY_E_NO_MEM
#define SW_RENDEZVOUS_E_BAD_PARAM				SW_DISCOVERY_E_BAD_PARAM
#define SW_RENDEZVOUS_E_BAD_REFERENCE			SW_DISCOVERY_E_BAD_REFERENCE
#define SW_RENDEZVOUS_E_BAD_STATE				SW_DISCOVERY_E_BAD_STATE
#define SW_RENDEZVOUS_E_BAD_FLAGS				SW_DISCOVERY_E_BAD_FLAGS
#define SW_RENDEZVOUS_E_NOT_SUPPORTED			SW_DISCOVERY_E_NOT_SUPPORTED
#define SW_RENDEZVOUS_E_NOT_INITIALIZED		SW_DISCOVERY_E_NOT_INITIALIZED
#define SW_RENDEZVOUS_E_NO_CACHE					SW_DISCOVERY_E_NO_CACHE
#define SW_RENDEZVOUS_E_ALREADY_REGISTERED	SW_DISCOVERY_E_ALREADY_REGISTERED
#define SW_RENDEZVOUS_E_NAME_CONFLICT			SW_DISCOVERY_E_NAME_CONFLICT
#define SW_RENDEZVOUS_E_INVALID					SW_DISCOVERY_E_INVALID


#endif

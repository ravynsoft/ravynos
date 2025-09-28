#ifndef _discovery_discovery_h
#define _discovery_discovery_h

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


#if defined(__cplusplus)
extern "C"
{
#endif


struct									_sw_discovery;
typedef struct _sw_discovery	*	sw_discovery;


/*
 * keeps track of different discovery operations
 */
typedef sw_uint32						sw_discovery_oid;


/*
 * For backwards compatibility
 */
#define sw_discovery_publish_host_id	sw_discovery_oid
#define sw_discovery_publish_id			sw_discovery_oid
#define sw_discovery_browse_id			sw_discovery_oid
#define sw_discovery_resolve_id			sw_discovery_oid


/*
 * how to connect to server
 */
typedef enum _sw_discovery_init_flags
{
	SW_DISCOVERY_USE_SHARED_SERVICE		=	0x1,
	SW_DISCOVERY_USE_PRIVATE_SERVICE		=	0x2,
	SW_DISCOVERY_SKIP_VERSION_CHECK		=	0x4
} sw_discovery_init_flags;


/*
 * status for asynchronous registration call
 */
typedef enum _sw_discovery_publish_status
{
	SW_DISCOVERY_PUBLISH_STARTED,
	SW_DISCOVERY_PUBLISH_STOPPED,
	SW_DISCOVERY_PUBLISH_NAME_COLLISION,
	SW_DISCOVERY_PUBLISH_INVALID
} sw_discovery_publish_status;


typedef enum _sw_discovery_browse_status
{
	SW_DISCOVERY_BROWSE_INVALID,
	SW_DISCOVERY_BROWSE_RELEASE,
	SW_DISCOVERY_BROWSE_ADD_DOMAIN,
	SW_DISCOVERY_BROWSE_ADD_DEFAULT_DOMAIN,
	SW_DISCOVERY_BROWSE_REMOVE_DOMAIN,
	SW_DISCOVERY_BROWSE_ADD_SERVICE,
	SW_DISCOVERY_BROWSE_REMOVE_SERVICE,
	SW_DISCOVERY_BROWSE_RESOLVED
} sw_discovery_browse_status;


typedef enum _sw_discovery_query_record_status
{
	SW_DISCOVERY_QUERY_RECORD_ADD			=	0x1
} sw_discovery_query_record_status;


typedef sw_result
(HOWL_API *sw_discovery_publish_reply)(
							sw_discovery						session,
							sw_discovery_oid					oid,
							sw_discovery_publish_status	status,
							sw_opaque							extra);

typedef sw_result
(HOWL_API *sw_discovery_browse_reply)(
							sw_discovery						session,
							sw_discovery_oid					oid,
							sw_discovery_browse_status		status,
							sw_uint32							interface_index,
							sw_const_string					name,
							sw_const_string					type,
							sw_const_string					domain,
							sw_opaque							extra);

typedef sw_result
(HOWL_API *sw_discovery_resolve_reply)(
							sw_discovery						session,
							sw_discovery_oid					oid,
							sw_uint32							interface_index,
							sw_const_string					name,
							sw_const_string					type,
							sw_const_string					domain,
							sw_ipv4_address					address,
							sw_port								port,
							sw_octets							text_record,
							sw_uint32							text_record_len,
							sw_opaque							extra);


typedef sw_result
(HOWL_API *sw_discovery_query_record_reply)(
							sw_discovery								session,
							sw_discovery_oid							oid,
							sw_discovery_query_record_status		status,
							sw_uint32									interface_index,
							sw_const_string							fullname,
							sw_uint16									rrtype,
							sw_uint16									rrclass,
							sw_uint16									rrdatalen,
							sw_const_octets							rrdata,
							sw_uint32									ttl,
							sw_opaque									extra);


/*
 * API for publishing/browsing/resolving services
 */
sw_result HOWL_API
sw_discovery_init(
					sw_discovery			*	self);


sw_result HOWL_API
sw_discovery_init_with_flags(
					sw_discovery			*	self,
					sw_discovery_init_flags	flags);


sw_result HOWL_API
sw_discovery_fina(
					sw_discovery		self);


sw_result HOWL_API
sw_discovery_publish_host(
					sw_discovery						self,
					sw_uint32							interface_index,
					sw_const_string					name,
					sw_const_string					domain,
					sw_ipv4_address					address,
					sw_discovery_publish_reply		reply,
					sw_opaque							extra,
					sw_discovery_oid				*	oid);


sw_result HOWL_API
sw_discovery_publish(
					sw_discovery						self,
					sw_uint32							interface_index,
					sw_const_string					name,
					sw_const_string					type,
					sw_const_string					domain,
					sw_const_string					host,
					sw_port								port,
					sw_octets							text_record,
					sw_uint32							text_record_len,
					sw_discovery_publish_reply		reply,
					sw_opaque							extra,
					sw_discovery_oid				*	oid);


sw_result HOWL_API
sw_discovery_publish_update(
					sw_discovery						self,
					sw_discovery_oid					oid,
					sw_octets							text_record,
					sw_uint32							text_record_len);



/*
 * API for browsing domains
 */
sw_result HOWL_API
sw_discovery_browse_domains(
					sw_discovery						self,
					sw_uint32							interface_index,
					sw_discovery_browse_reply		reply,
					sw_opaque							extra,
					sw_discovery_oid				*	oid);



/*
 * API for browsing services
 */
sw_result HOWL_API
sw_discovery_browse(
					sw_discovery						self,
					sw_uint32							interface_index,
					sw_const_string					type,
					sw_const_string					domain,
					sw_discovery_browse_reply		reply,
					sw_opaque							extra,
					sw_discovery_oid				*	oid);


/*
 * API for resolving services
 */
sw_result HOWL_API
sw_discovery_resolve(
					sw_discovery						self,
					sw_uint32							interface_index,
					sw_const_string					name,
					sw_const_string					type,
					sw_const_string					domain,
					sw_discovery_resolve_reply		reply,
					sw_opaque							extra,
					sw_discovery_oid				*	oid);


sw_result HOWL_API
sw_discovery_query_record(
					sw_discovery								self,
					sw_uint32									interface_index,
					sw_uint32									flags,
					sw_const_string							fullname,
					sw_uint16									rrtype,
					sw_uint16									rrclass,
					sw_discovery_query_record_reply		reply,
					sw_opaque									extra,
					sw_discovery_oid						*	oid);


sw_result HOWL_API
sw_discovery_cancel(
					sw_discovery		self,
					sw_discovery_oid	oid);



/* ----------------------------------------------------------
 *
 * Event Processing APIs
 *
 * ----------------------------------------------------------
 */


sw_result HOWL_API
sw_discovery_run(
					sw_discovery						self);


sw_result HOWL_API
sw_discovery_stop_run(
					sw_discovery						self);


int HOWL_API
sw_discovery_socket(
					sw_discovery						self);


sw_result HOWL_API
sw_discovery_read_socket(
					sw_discovery						self);


sw_result HOWL_API
sw_discovery_salt(
					sw_discovery	self,
					sw_salt		*	salt);


/*
 * Error codes
 */
#define SW_DISCOVERY_E_BASE					900
#define SW_DISCOVERY_E_UNKNOWN				(SW_DISCOVERY_E_BASE + 2)
#define SW_DISCOVERY_E_NO_SUCH_NAME			(SW_DISCOVERY_E_BASE + 3)
#define SW_DISCOVERY_E_NO_MEM					(SW_DISCOVERY_E_BASE + 4)
#define SW_DISCOVERY_E_BAD_PARAM				(SW_DISCOVERY_E_BASE + 5)
#define SW_DISCOVERY_E_BAD_REFERENCE		(SW_DISCOVERY_E_BASE + 6)
#define SW_DISCOVERY_E_BAD_STATE				(SW_DISCOVERY_E_BASE + 7)
#define SW_DISCOVERY_E_BAD_FLAGS				(SW_DISCOVERY_E_BASE + 8)
#define SW_DISCOVERY_E_NOT_SUPPORTED		(SW_DISCOVERY_E_BASE + 9)
#define SW_DISCOVERY_E_NOT_INITIALIZED		(SW_DISCOVERY_E_BASE + 10)
#define SW_DISCOVERY_E_NO_CACHE				(SW_DISCOVERY_E_BASE + 11)
#define SW_DISCOVERY_E_ALREADY_REGISTERED	(SW_DISCOVERY_E_BASE + 12)
#define SW_DISCOVERY_E_NAME_CONFLICT		(SW_DISCOVERY_E_BASE + 13)
#define SW_DISCOVERY_E_INVALID				(SW_DISCOVERY_E_BASE + 14)


#if defined(__cplusplus)
}
#endif


#endif

#ifndef _salt_time_h
#define _salt_time_h

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

#include <salt/platform.h>


#ifdef __cplusplus
extern "C"
{
#endif


struct								_sw_timer;
typedef struct _sw_timer	*	sw_timer;


sw_result HOWL_API
sw_timer_init(
		sw_timer	*	self);


sw_result HOWL_API
sw_timer_fina(
		sw_timer	self);


typedef struct _sw_time
{
	sw_uint32	m_secs;
	sw_uint32 m_usecs;
} sw_time;


sw_result HOWL_API
sw_time_init(
		sw_time	*	self);


sw_result HOWL_API
sw_time_init_now(
		sw_time	*	self);


sw_result HOWL_API
sw_time_fina(
		sw_time	self);


sw_time HOWL_API
sw_time_add(
		sw_time	self,
		sw_time	y);


sw_time HOWL_API
sw_time_sub(
		sw_time	self,
		sw_time	y);


sw_int32 HOWL_API
sw_time_cmp(
		sw_time	self,
		sw_time	y);


#ifdef __cplusplus
}
#endif


#endif

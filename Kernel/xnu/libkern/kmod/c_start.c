/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/* This file has been modified to support C++ initialization for the ravynOS
 * project in March 2026. This notice is in support of Clause 2.2 of the
 * License.
 */

#include <mach/mach_types.h>
#include <libkern/OSKextLib.h>

// These global symbols will be defined by CreateInfo script's info.c file.
extern kmod_start_func_t *_realmain;
extern kmod_info_t KMOD_INFO_NAME;

extern void kprintf(const char *fmt, ...);

/*********************************************************************
*********************************************************************/
__private_extern__ kern_return_t
_start(kmod_info_t *ki, void *data)
{
	if (_realmain) {
		return (*_realmain)(ki, data);
	} else {
		return KERN_SUCCESS;
	}
}

/*********************************************************************
*********************************************************************/
__private_extern__ const char *
OSKextGetCurrentIdentifier(void)
{
	return KMOD_INFO_NAME.name;
}

/*********************************************************************
*********************************************************************/
__private_extern__ const char *
OSKextGetCurrentVersionString(void)
{
	return KMOD_INFO_NAME.version;
}

/*********************************************************************
*********************************************************************/
__private_extern__ OSKextLoadTag
OSKextGetCurrentLoadTag(void)
{
	return (OSKextLoadTag)KMOD_INFO_NAME.id;
}

__private_extern__ void
__cxa_atexit(void)
{
	return;
}

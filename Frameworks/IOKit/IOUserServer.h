/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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


#ifndef _IOUSERSERVER_H
#define _IOUSERSERVER_H

#include <IOKit/IORPC.h>

#define kIOUserClassKey        "IOUserClass"
#define kIOUserServerClassKey  "IOUserServer"
#define kIOUserServerNameKey   "IOUserServerName"
#define kIOUserServerTagKey    "IOUserServerTag"
// the expected cdhash value of the userspace driver executable
#define kIOUserServerCDHashKey "IOUserServerCDHash"

#if DRIVERKIT_PRIVATE

enum{
	kIOKitUserServerClientType  = 0x99000003,
};

enum{
	kIOUserServerMethodRegisterClass = 0x0001000,
	kIOUserServerMethodStart         = 0x0001001,
	kIOUserServerMethodRegister      = 0x0001002,
};


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

class OSObject;

#define OSObject_Instantiate_ID       0x0000000100000001ULL

enum {
	kOSObjectRPCRemote = 0x00000001,
	kOSObjectRPCKernel = 0x00000002,
};

struct OSObject_Instantiate_Msg_Content {
	IORPCMessage __hdr;
	OSObjectRef  __object;
};

struct OSObject_Instantiate_Rpl_Content {
	IORPCMessage  __hdr;
	kern_return_t __result;
	uint32_t      __pad;
	uint64_t      flags;
	char          classname[64];
	uint64_t      methods[0];
};

#pragma pack(4)
struct OSObject_Instantiate_Msg {
	IORPCMessageMach mach;
	mach_msg_port_descriptor_t __object__descriptor;
	OSObject_Instantiate_Msg_Content content;
};
struct OSObject_Instantiate_Rpl {
	IORPCMessageMach mach;
	OSObject_Instantiate_Rpl_Content content;
};
#pragma pack()

typedef uint64_t IOTrapMessageBuffer[256];

#endif /* DRIVERKIT_PRIVATE */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#endif /* _IOUSERSERVER_H */

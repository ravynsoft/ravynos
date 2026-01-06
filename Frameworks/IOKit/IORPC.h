/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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


#ifndef _IORPC_H
#define _IORPC_H

#include <stdint.h>


#include <mach/message.h>



typedef uint64_t OSObjectRef;

enum {
	kIORPCVersion190615       = (mach_msg_id_t) 0x4da2b68c,
	kIORPCVersion190615Reply  = (mach_msg_id_t) 0x4da2b68d,

#if DRIVERKIT_PRIVATE
	kIORPCVersion190501       = (mach_msg_id_t) 0xfe316a7a,
	kIORPCVersion190501Reply  = (mach_msg_id_t) 0xfe316a7b,

	kIORPCVersionCurrent      = kIORPCVersion190615,
	kIORPCVersionCurrentReply = kIORPCVersion190615Reply
#endif /* DRIVERKIT_PRIVATE */
};

enum{
	kIORPCMessageRemote     = 0x00000001,
	kIORPCMessageLocalHost  = 0x00000002,
	kIORPCMessageKernel     = 0x00000004,
	kIORPCMessageOneway     = 0x00000008,
	kIORPCMessageObjectRefs = 0x00000010,
	kIORPCMessageOnqueue    = 0x00000020,
	kIORPCMessageError      = 0x00000040,
	kIORPCMessageSimpleReply = 0x00000080,
};

enum{
	kIORPCMessageIDKernel   = (1ULL << 63),
};

struct IORPCMessageMach {
	mach_msg_header_t          msgh;
	mach_msg_body_t            msgh_body;
	mach_msg_port_descriptor_t objects[0];
};
typedef struct IORPCMessageMach IORPCMessageMach;

struct IORPCMessage {
	uint64_t         msgid;
	uint64_t         flags;
	uint64_t         objectRefs;
	OSObjectRef      objects[0];
};
typedef struct IORPCMessage IORPCMessage;

extern "C" IORPCMessage *
IORPCMessageFromMach(IORPCMessageMach * msg, bool reply);

struct IORPCMessageErrorReturnContent {
	IORPCMessage  hdr;
	kern_return_t result;
	uint32_t      pad;
};

#pragma pack(4)
struct IORPCMessageErrorReturn {
	IORPCMessageMach mach;
	IORPCMessageErrorReturnContent content;
};
#pragma pack()


class OSMetaClassBase;
struct IORPC;
typedef kern_return_t (*OSDispatchMethod)(OSMetaClassBase * self, const IORPC rpc);

struct IORPC {
	IORPCMessageMach * message;
	IORPCMessageMach * reply;
	uint32_t           sendSize;
	uint32_t           replySize;
};
typedef struct IORPC IORPC;

enum {
	kOSClassCanRemote   = 0x00000001,
};

struct OSClassDescription {
	uint32_t    descriptionSize;

	char        name[96];
	char        superName[96];

	uint32_t    methodOptionsSize;
	uint32_t    methodOptionsOffset;
	uint32_t    metaMethodOptionsSize;
	uint32_t    metaMethodOptionsOffset;
	uint32_t    queueNamesSize;
	uint32_t    queueNamesOffset;
	uint32_t    methodNamesSize;
	uint32_t    methodNamesOffset;
	uint32_t    metaMethodNamesSize;
	uint32_t    metaMethodNamesOffset;

	uint64_t    flags;

	uint64_t    resv1[8];

	uint64_t    methodOptions[0];
	uint64_t    metaMethodOptions[0];

	char        dispatchNames[0];
	char        methodNames[0];
	char        metaMethodNames[0];
};

#endif /* _IORPC_H */

/*
 * Copyright (c) 2008, 2012 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * FILE: kextmanager_types.h
 * AUTH: I/O Kit Team (Copyright Apple Computer, 2002, 2006-7)
 * DATE: June 2002, September 2006, August 2007
 * DESC: typedefs for the kextmanager_mig.defs's MiG-generated code 
 *
 */

#ifndef __KEXT_TYPES_H__
#define __KEXT_TYPES_H__

#include <mach/mach_types.h>        // allows to compile standalone
#include <mach/kmod.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <uuid/uuid.h>              // uuid_t

#define KEXTD_SERVER_NAME       "com.apple.KernelExtensionServer"
#define PROPERTYKEY_LEN         128

typedef int kext_result_t;
typedef char mountpoint_t[MNAMELEN];
typedef char property_key_t[PROPERTYKEY_LEN];
typedef char kext_bundle_id_t[KMOD_MAX_NAME];
typedef char posix_path_t[MAXPATHLEN];

// nowadays this is binary plist data but we keep the name for now
typedef char * xmlDataOut_t;
typedef char * xmlDataIn_t;

#endif /* __KEXT_TYPES_H__ */

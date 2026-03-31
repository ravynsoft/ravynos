/*
 * Copyright (c) 1999-2001, 2005-2006 Apple Inc.  All Rights Reserved.
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
 *	objc-load.h
 *	Copyright 1988-1996, NeXT Software, Inc.
 */

#ifndef _OBJC_LOAD_H_
#define _OBJC_LOAD_H_

#include <objc/objc-class.h>

#include <mach-o/loader.h>

/* dynamically loading Mach-O object files that contain Objective-C code */

OBJC_EXPORT long objc_loadModules (
	char * _Nullable modlist[_Nullable], 
	void * _Nullable errStream,
	void (* _Nullable class_callback) (Class _Nullable, Category _Nullable),
	/*headerType*/ struct mach_header * _Nullable * _Nullable hdr_addr,
	char * _Nullable debug_file
) OBJC2_UNAVAILABLE;

OBJC_EXPORT int objc_loadModule (
	char * _Nullable moduleName, 
	void	(* _Nullable class_callback) (Class _Nullable, Category _Nullable),
	int * _Nullable errorCode
) OBJC2_UNAVAILABLE;
OBJC_EXPORT long objc_unloadModules(
	void * _Nullable errorStream,				/* input (optional) */
	void (* _Nullable unloadCallback)(Class _Nullable, Category _Nullable)		/* input (optional) */
) OBJC2_UNAVAILABLE;

#endif /* _OBJC_LOAD_H_ */

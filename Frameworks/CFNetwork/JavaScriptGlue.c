/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
 *  JavaScriptGlue.c
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on Thu Oct 16 2003.
 *  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 */

#include <stdio.h>
#include <JavaScriptGlue/JavaScriptGlue.h>
#include "CFNetworkInternal.h"
#include <mach-o/dyld.h>

void JSLockInterpreter(void);
void JSUnlockInterpreter(void);

#ifndef DYNAMICALLY_LOAD_JAVASCRIPT
#define DYNAMICALLY_LOAD_JAVASCRIPT 1
#endif

#if DYNAMICALLY_LOAD_JAVASCRIPT

struct _JavaScriptCallBacks {
	void (*JSRelease_proc)(JSTypeRef);
	void (*JSLockInterpreter_proc)(void);
	void (*JSUnlockInterpreter_proc)(void);
	JSObjectRef (*JSObjectCallFunction_proc)(JSObjectRef, JSObjectRef, CFArrayRef);
	CFMutableArrayRef (*JSCreateJSArrayFromCFArray_proc)(CFArrayRef);
	JSRunRef (*JSRunCreate_proc)(CFStringRef, JSFlags);
	JSObjectRef (*JSRunCopyGlobalObject_proc)(JSRunRef);
	JSObjectRef (*JSRunEvaluate_proc)(JSRunRef);
	bool (*JSRunCheckSyntax_proc)(JSRunRef);
	JSObjectRef (*JSObjectCreate_proc)(void*, JSObjectCallBacksPtr);
	void (*JSObjectSetProperty_proc)(JSObjectRef, CFStringRef, JSObjectRef);
	JSObjectRef (*JSObjectCreateWithCFType_proc)(CFTypeRef);
	CFTypeRef (*JSObjectCopyCFValue_proc)(JSObjectRef);
	JSObjectRef (*JSObjectCopyProperty_proc)(JSObjectRef, CFStringRef);
};

static const char kJavaScriptLibraryPath[] = "/System/Library/PrivateFrameworks/JavaScriptGlue.framework/Versions/A/JavaScriptGlue";

static const void* JavaScriptLibrary = NULL;
static CFSpinLock_t JavaScriptLibraryLock = 0;
static struct _JavaScriptCallBacks* JavaScriptCallBacks = NULL;

static const void* returns_ref(void) { return NULL; }
static bool returns_bool(void) { return 0; }
static void returns(void) { return; }


#define GET_DYNAMIC_SYMBOL(sym, rettype, arglist, alt) \
	if (!JavaScriptLibrary) { \
		__CFSpinLock(&JavaScriptLibraryLock); \
		JavaScriptLibrary = __CFNetworkLoadFramework(kJavaScriptLibraryPath); \
		if (!JavaScriptCallBacks) { \
			JavaScriptCallBacks = (struct _JavaScriptCallBacks*)calloc(1, sizeof(JavaScriptCallBacks[0])); \
		}	\
		__CFSpinUnlock(&JavaScriptLibraryLock); \
	} \
	if (!JavaScriptCallBacks->sym##_proc) { \
		JavaScriptCallBacks->sym##_proc = (rettype(*)arglist)NSAddressOfSymbol(NSLookupSymbolInImage(JavaScriptLibrary, "_"#sym, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND)); \
		if (!JavaScriptCallBacks->sym##_proc) JavaScriptCallBacks->sym##_proc = (rettype(*)arglist)alt; \
	} \


void
JSRelease(JSTypeRef ref) {

    GET_DYNAMIC_SYMBOL(JSRelease, void, (JSTypeRef), returns);
    
    return JavaScriptCallBacks->JSRelease_proc(ref);
}

void
JSLockInterpreter(void) {

    GET_DYNAMIC_SYMBOL(JSLockInterpreter, void, (void), returns);
    
    JavaScriptCallBacks->JSLockInterpreter_proc();
}

void
JSUnlockInterpreter(void) {

    GET_DYNAMIC_SYMBOL(JSUnlockInterpreter, void, (void), returns);
    
    JavaScriptCallBacks->JSUnlockInterpreter_proc();
}

JSObjectRef
JSObjectCallFunction(JSObjectRef ref, JSObjectRef thisObj, CFArrayRef args) {

    GET_DYNAMIC_SYMBOL(JSObjectCallFunction, JSObjectRef, (JSObjectRef, JSObjectRef, CFArrayRef), returns_ref);
    
    return JavaScriptCallBacks->JSObjectCallFunction_proc(ref, thisObj, args);
}

CFMutableArrayRef 
JSCreateJSArrayFromCFArray(CFArrayRef array) {
    GET_DYNAMIC_SYMBOL(JSCreateJSArrayFromCFArray, CFMutableArrayRef, (CFArrayRef), returns_ref);
    
    return JavaScriptCallBacks->JSCreateJSArrayFromCFArray_proc(array);
}



JSRunRef
JSRunCreate(CFStringRef jsSource, JSFlags inFlags) {

    GET_DYNAMIC_SYMBOL(JSRunCreate, JSRunRef, (CFStringRef, JSFlags), returns_ref);
    
    return JavaScriptCallBacks->JSRunCreate_proc(jsSource, inFlags);
}


JSObjectRef
JSRunCopyGlobalObject(JSRunRef ref) {

    GET_DYNAMIC_SYMBOL(JSRunCopyGlobalObject, JSObjectRef, (JSRunRef), returns_ref);
    
    return JavaScriptCallBacks->JSRunCopyGlobalObject_proc(ref);
}


JSObjectRef
JSRunEvaluate(JSRunRef ref) {

    GET_DYNAMIC_SYMBOL(JSRunEvaluate, JSObjectRef, (JSRunRef), returns_ref);
    
    return JavaScriptCallBacks->JSRunEvaluate_proc(ref);
}


bool
JSRunCheckSyntax(JSRunRef ref) {

    GET_DYNAMIC_SYMBOL(JSRunCheckSyntax, bool, (JSRunRef), returns_bool);
    
    return JavaScriptCallBacks->JSRunCheckSyntax_proc(ref);
}


JSObjectRef
JSObjectCreate(void* data, JSObjectCallBacksPtr callBacks) {

    GET_DYNAMIC_SYMBOL(JSObjectCreate, JSObjectRef, (void*, JSObjectCallBacksPtr), returns_ref);
    
    return JavaScriptCallBacks->JSObjectCreate_proc(data, callBacks);
}


void
JSObjectSetProperty(JSObjectRef ref, CFStringRef propertyName, JSObjectRef value) {

    GET_DYNAMIC_SYMBOL(JSObjectSetProperty, void, (JSObjectRef, CFStringRef, JSObjectRef), returns);
    
    return JavaScriptCallBacks->JSObjectSetProperty_proc(ref, propertyName, value);
}


JSObjectRef
JSObjectCreateWithCFType(CFTypeRef inRef) {

    GET_DYNAMIC_SYMBOL(JSObjectCreateWithCFType, JSObjectRef, (CFTypeRef), returns_ref);
    
    return JavaScriptCallBacks->JSObjectCreateWithCFType_proc(inRef);
}


CFTypeRef
JSObjectCopyCFValue(JSObjectRef ref) {

    GET_DYNAMIC_SYMBOL(JSObjectCopyCFValue, CFTypeRef, (JSObjectRef), returns_ref);
    
    return JavaScriptCallBacks->JSObjectCopyCFValue_proc(ref);
}


JSObjectRef
JSObjectCopyProperty(JSObjectRef ref, CFStringRef propertyName) {

    GET_DYNAMIC_SYMBOL(JSObjectCopyProperty, JSObjectRef, (JSObjectRef, CFStringRef), returns_ref);

    return JavaScriptCallBacks->JSObjectCopyProperty_proc(ref, propertyName);
}


#undef GET_DYNAMIC_SYMBOL
#endif	/* DYNAMICALLY_LOAD_JAVASCRIPT */


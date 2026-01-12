/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFBundle_BinaryTypes.h
	Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFBUNDLE_BINARYTYPES__)
#define __COREFOUNDATION_CFBUNDLE_BINARYTYPES__ 1

CF_EXTERN_C_BEGIN

#if DEPLOYMENT_TARGET_MACOSX
#define BINARY_SUPPORT_DYLD 1
#define BINARY_SUPPORT_DLFCN 1
#define USE_DYLD_PRIV 1
#elif DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#define BINARY_SUPPORT_DYLD 1
#define BINARY_SUPPORT_DLFCN 1
#define USE_DYLD_PRIV 1
#elif DEPLOYMENT_TARGET_WINDOWS
#define BINARY_SUPPORT_DLL 1
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif


typedef enum {
    __CFBundleUnknownBinary,
    __CFBundleCFMBinary,
    __CFBundleDYLDExecutableBinary,
    __CFBundleDYLDBundleBinary,
    __CFBundleDYLDFrameworkBinary,
    __CFBundleDLLBinary,
    __CFBundleUnreadableBinary,
    __CFBundleNoBinary,
    __CFBundleELFBinary
} __CFPBinaryType;

/* Intended for eventual public consumption */
typedef enum {
    kCFBundleOtherExecutableType = 0,
    kCFBundleMachOExecutableType,
    kCFBundlePEFExecutableType,
    kCFBundleELFExecutableType,
    kCFBundleDLLExecutableType
} CFBundleExecutableType;

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFBUNDLE_BINARYTYPES__ */


/*	CFBundle_BinaryTypes.h
	Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFBUNDLE_BINARYTYPES__)
#define __COREFOUNDATION_CFBUNDLE_BINARYTYPES__ 1

CF_EXTERN_C_BEGIN

#if TARGET_OS_OSX
#define BINARY_SUPPORT_DYLD 1
#define BINARY_SUPPORT_DLFCN 1
#if DEPLOYMENT_RUNTIME_SWIFT
#define USE_DYLD_PRIV 0
#else
#define USE_DYLD_PRIV 1
#endif
#elif TARGET_OS_IPHONE
#define BINARY_SUPPORT_DYLD 1
#define BINARY_SUPPORT_DLFCN 1
#define USE_DYLD_PRIV 1
#elif TARGET_OS_WIN32
#define BINARY_SUPPORT_DLL 1
#elif TARGET_OS_LINUX || TARGET_OS_BSD
#define BINARY_SUPPORT_DLFCN 1
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


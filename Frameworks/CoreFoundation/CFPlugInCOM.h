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

/*	CFPlugInCOM.h
	Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFPLUGINCOM__)
#define __COREFOUNDATION_CFPLUGINCOM__ 1

#include <CoreFoundation/CFPlugIn.h>

CF_EXTERN_C_BEGIN

/* ================= IUnknown definition (C struct) ================= */

/* All interface structs must have an IUnknownStruct at the beginning. */
/* The _reserved field is part of the Microsoft COM binary standard on Macintosh. */
/* You can declare new C struct interfaces by defining a new struct that includes "IUNKNOWN_C_GUTS;" before the first field of the struct. */

typedef SInt32 HRESULT;
typedef UInt32 ULONG;
typedef void *LPVOID;
typedef CFUUIDBytes REFIID;

#define SUCCEEDED(Status) ((HRESULT)(Status) >= 0)
#define FAILED(Status) ((HRESULT)(Status)<0)

/* Macros for more detailed HRESULT analysis */
#define IS_ERROR(Status) ((unsigned long)(Status) >> 31 == SEVERITY_ERROR)
#define HRESULT_CODE(hr) ((hr) & 0xFFFF)
#define HRESULT_FACILITY(hr) (((hr) >> 16) & 0x1fff)
#define HRESULT_SEVERITY(hr) (((hr) >> 31) & 0x1)
#define SEVERITY_SUCCESS 0
#define SEVERITY_ERROR 1

/* Creating an HRESULT from its component pieces */
#define MAKE_HRESULT(sev,fac,code) ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )

/* Pre-defined success HRESULTS */
#define S_OK ((HRESULT)0x00000000L)
#define S_FALSE ((HRESULT)0x00000001L)

/* Common error HRESULTS */
#define E_UNEXPECTED ((HRESULT)0x8000FFFFL)
#define E_NOTIMPL ((HRESULT)0x80000001L)
#define E_OUTOFMEMORY ((HRESULT)0x80000002L)
#define E_INVALIDARG ((HRESULT)0x80000003L)
#define E_NOINTERFACE ((HRESULT)0x80000004L)
#define E_POINTER ((HRESULT)0x80000005L)
#define E_HANDLE ((HRESULT)0x80000006L)
#define E_ABORT ((HRESULT)0x80000007L)
#define E_FAIL ((HRESULT)0x80000008L)
#define E_ACCESSDENIED ((HRESULT)0x80000009L)

/* This macro should be used when defining all interface functions (as it is for the IUnknown functions below). */
#define STDMETHODCALLTYPE

/* The __RPC_FAR macro is for COM source compatibility only. This macro is used a lot in COM interface definitions.  If your CFPlugIn interfaces need to be COM interfaces as well, you can use this macro to get better source compatibility.  It is not used in the IUnknown definition below, because when doing COM, you will be using the Microsoft supplied IUnknown interface anyway. */
#define __RPC_FAR

/* The IUnknown interface */
#define IUnknownUUID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46)

#define IUNKNOWN_C_GUTS \
    void *_reserved; \
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(void *thisPointer, REFIID iid, LPVOID *ppv); \
    ULONG (STDMETHODCALLTYPE *AddRef)(void *thisPointer); \
    ULONG (STDMETHODCALLTYPE *Release)(void *thisPointer)
    
typedef struct IUnknownVTbl {
    IUNKNOWN_C_GUTS;
} IUnknownVTbl;

CF_EXTERN_C_END


/* C++ specific stuff */
#if defined(__cplusplus)
/* ================= IUnknown definition (C++ class) ================= */

/* This is a definition of IUnknown as a pure abstract virtual C++ class.  This class will work only with compilers that can produce COM-compatible object layouts for C++ classes.  egcs can not do this.  MetroWerks can do this (if you subclass from __comobject) */

class IUnknown
#if defined(__MWERKS__) && TARGET_OS_WIN32
 : __comobject
#endif
{
    public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) = 0;
    virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
    virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};

#endif /* __cplusplus */

#endif /* ! __COREFOUNDATION_CFPLUGINCOM__ */


/*	CFPlugInCOM.h
	Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFPLUGINCOM__)
#define __COREFOUNDATION_CFPLUGINCOM__ 1

#include <CoreFoundation/CFPlugIn.h>

CF_EXTERN_C_BEGIN

/* ================= IUnknown definition (C struct) ================= */

/* All interface structs must have an IUnknownStruct at the beginning. */
/* The _reserved field is part of the Microsoft COM binary standard on Macintosh. */
/* You can declare new C struct interfaces by defining a new struct that includes "IUNKNOWN_C_GUTS;" before the first field of the struct. */

#if TARGET_OS_WIN32
#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
typedef SInt32 HRESULT;
typedef UInt32 ULONG;
typedef void *LPVOID;
typedef CFUUIDBytes REFIID;
#endif

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

#if __clang__
#define __PRAGMA_PUSH_NO_NULLABILITY_COMPLETENESS_WARNINGS _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wnullability-completeness\"")
#define __PRAGMA_POP_NO_NULLABILITY_COMPLETENESS_WARNINGS _Pragma("clang diagnostic pop")
#else
#define __PRAGMA_PUSH_NO_NULLABILITY_COMPLETENESS_WARNINGS
#define __PRAGMA_POP_NO_NULLABILITY_COMPLETENESS_WARNINGS
#endif

#define IUNKNOWN_C_GUTS \
    __PRAGMA_PUSH_NO_NULLABILITY_COMPLETENESS_WARNINGS \
    void *_reserved; \
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(void *thisPointer, REFIID iid, LPVOID *ppv); \
    ULONG (STDMETHODCALLTYPE *AddRef)(void *thisPointer); \
    ULONG (STDMETHODCALLTYPE *Release)(void *thisPointer) \
    __PRAGMA_POP_NO_NULLABILITY_COMPLETENESS_WARNINGS
    
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


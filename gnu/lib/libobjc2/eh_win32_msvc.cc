#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>

#include "objc/runtime.h"
#include "visibility.h"

#include <windows.h>
#define RtlAddGrowableFunctionTable ClangIsConfusedByTypedefReturnTypes
#include <rtlsupportapi.h>


#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if !__has_builtin(__builtin_unreachable)
#define __builtin_unreachable abort
#endif

struct _MSVC_TypeDescriptor
{
	const void* pVFTable;
	void* spare;
	char name[0];
};

struct _MSVC_CatchableType
{
	unsigned int flags;
	unsigned long type;
	int mdisp;
	int pdisp;
	int vdisp;
	int size;
	unsigned long copyFunction;
};

struct _MSVC_CatchableTypeArray
{
	int count;
	unsigned long types[0];
};

struct _MSVC_ThrowInfo
{
	unsigned int attributes;
	unsigned long pfnUnwind;
	unsigned long pfnForwardCompat;
	unsigned long pCatchableTypeArray;
};

#if defined(_WIN64)
#define IMAGE_RELATIVE(ptr, base) (static_cast<unsigned long>((ptr ? ((uintptr_t)ptr - (uintptr_t)base) : (uintptr_t)nullptr)))
#else
#define IMAGE_RELATIVE(ptr, base) reinterpret_cast<unsigned long>((ptr))
#endif

extern "C" void __stdcall _CxxThrowException(void*, _MSVC_ThrowInfo*);

namespace
{

static std::string mangleObjcObject()
{
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8
	return ".PEAUobjc_object@@";
#else
	return ".PAUobjc_object@@";
#endif
}

static std::string mangleStructNamed(const char* className)
{
	// 32-bit:
	//  .PAUxxx@@ = ?? struct xxx * `RTTI Type Descriptor'
	// 64-bit:
	//  .PEAUxxx@@ = ?? struct xxx * __ptr64 `RTTI Type Descriptor'
	//return
	auto r =
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8
		std::string(".PEAU") +
#else
		std::string(".PAU") +
#endif
		className + "@@";
	return r;
}

void fillCatchableType(_MSVC_CatchableType* exceptType)
{
	exceptType->flags = 1;
	exceptType->mdisp = 0;
	exceptType->pdisp = -1;
	exceptType->vdisp = 0;
	exceptType->size = sizeof(id);
	exceptType->copyFunction = 0;
}

} // <anonymous-namespace>

struct X {};
OBJC_PUBLIC extern "C" void objc_exception_rethrow(void* exc);

OBJC_PUBLIC extern "C" void objc_exception_throw(id object)
{
	// Base used for image-relative addresses.
	char x;
	// This is the base vtable for all RTTI entries
	static const void* typeid_vtable = *(void**)&typeid(void *);

	SEL rethrow_sel = sel_registerName("rethrow");
	if ((nil != object) &&
		(class_respondsToSelector(object_getClass(object), rethrow_sel)))
	{
		IMP rethrow = objc_msg_lookup(object, rethrow_sel);
		rethrow(object, rethrow_sel);
		// Should not be reached!  If it is, then the rethrow method actually
		// didn't, so we throw it normally.
	}

	SEL processException_sel = sel_registerName("_processException");
	if ((nil != object) &&
		(class_respondsToSelector(object_getClass(object), processException_sel)))
	{
		IMP processException = objc_msg_lookup(object, processException_sel);
		processException(object, processException_sel);
	}

	// The 'id' base type will be taking up a spot in the list:
	size_t typeCount = 1;

	// Get count of all types in exception
	for (Class cls = object_getClass(object); cls != Nil; cls = class_getSuperclass(cls), ++typeCount)
		;

	// Unfortunately we can't put this in a real function since the alloca has to be in this stack frame:
#define CREATE_TYPE_DESCRIPTOR(desc, symName) \
	desc = reinterpret_cast<_MSVC_TypeDescriptor*>(alloca(sizeof(_MSVC_TypeDescriptor) + symName.size() + 1 /* null terminator */)); \
	desc->pVFTable = typeid_vtable; \
	desc->spare = nullptr; \
	strcpy_s(desc->name, symName.size() + 1, symName.c_str());

	auto exceptTypes =
		(_MSVC_CatchableTypeArray*)_alloca(sizeof(_MSVC_CatchableTypeArray) + sizeof(_MSVC_CatchableType*) * typeCount);
	exceptTypes->count = typeCount;

	// Add exception type and all base types to throw information
	size_t curTypeIndex = 0;
	for (Class cls = object_getClass(object); cls != Nil; cls = class_getSuperclass(cls))
	{
		auto exceptType = (_MSVC_CatchableType*)_alloca(sizeof(_MSVC_CatchableType));
		fillCatchableType(exceptType);

		auto mangledName = mangleStructNamed(class_getName(cls));
		_MSVC_TypeDescriptor *ty;
		CREATE_TYPE_DESCRIPTOR(ty, mangledName);
		exceptType->type = IMAGE_RELATIVE(ty, &x);
		exceptTypes->types[curTypeIndex++] = IMAGE_RELATIVE(exceptType, &x);
	}

	// Add id (struct objc_object*)
	auto exceptType = (_MSVC_CatchableType*)_alloca(sizeof(_MSVC_CatchableType));
	fillCatchableType(exceptType);
	auto idName = mangleObjcObject();
	_MSVC_TypeDescriptor *ty;
	CREATE_TYPE_DESCRIPTOR(ty, idName);
	exceptType->type = IMAGE_RELATIVE(ty, &x);
	exceptTypes->types[curTypeIndex++] = IMAGE_RELATIVE(exceptType, &x);

	_MSVC_ThrowInfo ti = {
		0, // attributes
		0, // pfnUnwind
		0, // pfnForwardCompat
		IMAGE_RELATIVE(exceptTypes, &x) // pCatchableTypeArray
	};
#	define EH_EXCEPTION_NUMBER ('msc' | 0xE0000000)
#	define EH_MAGIC_NUMBER1 0x19930520
#	define EXCEPTION_NONCONTINUABLE 0x1
	EXCEPTION_RECORD  exception;
	exception.ExceptionCode = EH_EXCEPTION_NUMBER;
	exception.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
	exception.ExceptionRecord = nullptr;
	exception.ExceptionAddress = nullptr;
	// The fourth parameter is the base address of the image (for us, this stack 
	// frame), but we only use image-relative 32-bit addresses on 64-bit
	// platforms.  On 32-bit platforms, we use 32-bit absolute addresses.
	exception.NumberParameters = sizeof(void*) == 4 ? 3 : 4;
	exception.ExceptionInformation[0] = EH_MAGIC_NUMBER1;
	exception.ExceptionInformation[1] = reinterpret_cast<ULONG_PTR>(&object);
	exception.ExceptionInformation[2] = reinterpret_cast<ULONG_PTR>(&ti);
	exception.ExceptionInformation[3] = reinterpret_cast<ULONG_PTR>(&x);

#ifdef _WIN64
 	RtlRaiseException(&exception);
#else
	RaiseException(exception.ExceptionCode,
		exception.ExceptionFlags,
		exception.NumberParameters,
		exception.ExceptionInformation);
#endif
	__builtin_unreachable();
}


OBJC_PUBLIC extern "C" void objc_exception_rethrow(void* exc)
{
	_CxxThrowException(nullptr, nullptr);
	__builtin_unreachable();
}


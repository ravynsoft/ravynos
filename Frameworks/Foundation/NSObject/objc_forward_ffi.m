/*
 Parts of this come from PyObjC, http://pyobjc.sourceforge.net/
 Copyright 2002, 2003 - Bill Bumgarner, Ronald Oussoren, Steve Majewski, Lele Gaifax, et.al.
 Copyright 2008 Johannes Fortmann

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */


#ifdef HAVE_LIBFFI

#import "objc_forward_ffi.h"
#import "objc_size_alignment.h"

#import <objc/objc-class.h>
#import <objc/message.h>
#include <ctype.h>
#ifdef DARWIN
#import <ffi/ffi.h>
#else
#include <ffi.h>
#endif
#import <Foundation/NSMethodSignature.h>
#import "ObjCHashTable.h"
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#import <Foundation/NSInvocation.h>
#import <Foundation/NSRaiseException.h>

extern void* _NSClosureAlloc(size_t);
extern void _NSClosureProtect(void*, size_t);

#pragma mark Converting @encode to ffi type descriptors

static inline OBJCHashTable *ffi_type_table(void) {
	static OBJCHashTable *ffiTypeTable=NULL;

	if(ffiTypeTable==NULL)
		ffiTypeTable=OBJCCreateHashTable(50);

	return ffiTypeTable;
}

static inline ffi_type* ffi_try_find_type(const char* argtype)
{
	return (ffi_type*)OBJCHashValueForKey(ffi_type_table(), argtype);
}

static inline void ffi_insert_type(const char* argtype, ffi_type* type)
{
	OBJCHashInsertValueForKey(ffi_type_table(), argtype, type);
}

static size_t
num_struct_fields(const char* argtype)
{
	size_t res = 0;

	if (*argtype != _C_STRUCT_B) [NSException raise:NSInternalInconsistencyException format:@"exception while encoding type"];
	while (*argtype != _C_STRUCT_E && *argtype != '=') argtype++;
	if (*argtype == _C_STRUCT_E) return 0;

	argtype++;
	while (*argtype != _C_STRUCT_E) {
		argtype = objc_skip_type_specifier(argtype,YES);
		if (argtype == NULL) [NSException raise:NSInternalInconsistencyException format:@"exception while encoding type"];
		res ++;
	}
	return res;
}




static ffi_type* signature_to_ffi_type(const char* argtype);

static ffi_type*
array_to_ffi_type(const char* argtype)
{
	ffi_type* type=ffi_try_find_type(argtype);
	if(type)
		return type;

	/* We don't have a type description yet, dynamicly
	 * create it.
	 */
	size_t field_count = atoi(argtype+1);
	size_t i;

	type = NSZoneMalloc(NULL, sizeof(*type));

	type->size=objc_sizeof_type(argtype);
	type->alignment=objc_alignof_type(argtype);

	/* Libffi doesn't really know about arrays as part of larger
	 * data-structres (e.g. struct foo { int field[3]; };). We fake it
	 * by treating the nested array as a struct. These seems to work
	 * fine on MacOS X.
	 */
	type->type = FFI_TYPE_STRUCT;
	type->elements = NSZoneMalloc(NULL, (1+field_count) * sizeof(ffi_type*));

	while (isdigit(*++argtype));
	type->elements[0] = signature_to_ffi_type(argtype);
	for (i = 1; i < field_count; i++) {
		type->elements[i] = type->elements[0];
	}
	type->elements[field_count] = 0;

	ffi_insert_type(argtype, type);
	return type;
}

static ffi_type*
struct_to_ffi_type(const char* argtype)
{
	ffi_type* type=ffi_try_find_type(argtype);
	if(type)
		return type;
	const char* curtype;


	/* We don't have a type description yet, dynamicly
	 * create it.
	 */
	size_t field_count = num_struct_fields(argtype);

	type = NSZoneMalloc(NULL, sizeof(*type));

	type->size = objc_sizeof_type(argtype);
	type->alignment = objc_alignof_type(argtype);

	type->type = FFI_TYPE_STRUCT;
	type->elements = NSZoneMalloc(NULL, (1+field_count) * sizeof(ffi_type*));

	field_count = 0;
	curtype = argtype+1;
	while (*curtype != _C_STRUCT_E && *curtype != '=') curtype++;
	if (*curtype == '=') {
		curtype ++;
		while (*curtype != _C_STRUCT_E) {
			type->elements[field_count] =
			signature_to_ffi_type(curtype);
			field_count++;
			curtype = objc_skip_type_specifier(curtype,YES);
		}
	}
	type->elements[field_count] = NULL;

	ffi_insert_type(argtype, type);

	return type;
}


static ffi_type*
signature_to_ffi_return_type(const char* argtype)
{
#ifdef __ppc__
	static const char long_type[] = { _C_LNG, 0 };
	static const char ulong_type[] = { _C_ULNG, 0 };

	switch (*argtype) {
		case _C_CHR: case _C_SHT: case _C_UNICHAR:
			return signature_to_ffi_type(long_type);
		case _C_UCHR: case _C_USHT: //case _C_UNICHAR:
			return signature_to_ffi_type(ulong_type);
#ifdef _C_BOOL
		case _C_BOOL: return signature_to_ffi_type(long_type);
#endif
		case _C_NSBOOL:
			return signature_to_ffi_type(long_type);
		default:
			return signature_to_ffi_type(argtype);
	}
#else
	return signature_to_ffi_type(argtype);
#endif
}



static ffi_type*
signature_to_ffi_type(const char* argtype)
{
	switch (*argtype) {
		case _C_VOID: return &ffi_type_void;
		case _C_ID: return &ffi_type_pointer;
		case _C_CLASS: return &ffi_type_pointer;
		case _C_SEL: return &ffi_type_pointer;
		case _C_CHR: return &ffi_type_schar;
#ifdef _C_BOOL
		case _C_BOOL:
			/* sizeof(bool) == 4 on PPC32, and 1 on all others */
#if defined(__ppc__) && !defined(__LP__)
			return &ffi_type_sint;
#else
			return &ffi_type_schar;
#endif

#endif
		case _C_UCHR: return &ffi_type_uchar;
		case _C_SHT: return &ffi_type_sshort;
		case _C_USHT: return &ffi_type_ushort;
		case _C_INT: return &ffi_type_sint;
		case _C_UINT: return &ffi_type_uint;

			/* The next to defintions are incorrect, but the correct definitions
			 * don't work (e.g. give testsuite failures).
			 */
#ifdef __LP64__
		case _C_LNG: return &ffi_type_sint64;  /* ffi_type_slong */
		case _C_ULNG: return &ffi_type_uint64;  /* ffi_type_ulong */
#else
		case _C_LNG: return &ffi_type_sint;  /* ffi_type_slong */
		case _C_ULNG: return &ffi_type_uint;  /* ffi_type_ulong */
#endif
		case _C_LNGLNG: return &ffi_type_sint64;
		case _C_ULNG_LNG: return &ffi_type_uint64;
		case _C_FLT: return &ffi_type_float;
		case _C_DBL: return &ffi_type_double;
		case _C_CHARPTR: return &ffi_type_pointer;
		case _C_PTR: return &ffi_type_pointer;
		case _C_ARY_B:
			return array_to_ffi_type(argtype);
		case _C_IN: case _C_OUT: case _C_INOUT: case _C_CONST:
			return signature_to_ffi_type(argtype+1);
		case _C_STRUCT_B:
			return struct_to_ffi_type(argtype);
		case _C_UNDEF:
			return &ffi_type_pointer;
		default:
			NSLog(@"Type '%c' not supported", *argtype);
			return NULL;
	}
}


/*
 * arg_signature_to_ffi_type: Make the ffi_type for the call to the method IMP.
 */

#ifdef __ppc__
#define arg_signature_to_ffi_type signature_to_ffi_type

#else
static inline ffi_type*
arg_signature_to_ffi_type(const char* argtype)
{
	/* NOTE: This is the minimal change to pass the unittests, it is not
	 * based on analysis of the calling conventions.
	 */
	switch (*argtype) {
		case _C_CHR: return &ffi_type_sint;
		case _C_UCHR: return &ffi_type_uint;
		case _C_SHT: return &ffi_type_sint;
		case _C_USHT: return &ffi_type_uint;
		default: return signature_to_ffi_type(argtype);
	}
}
#endif


#pragma mark -
#pragma mark Implementation of closures and NSInvocation -invoke

static void
invocation_closure(ffi_cif* cif, void* result, void** args, void* userdata)
{
	NSMethodSignature *sig=(id)userdata;
	NSInvocation *inv=[NSInvocation invocationWithMethodSignature:sig];
	NSInteger i, numArgs=[sig numberOfArguments];
	for(i=0; i<numArgs; i++)
	{
		[inv setArgument:args[i] atIndex:i];
	}

	id receiver=*(id*)args[0];

	[receiver forwardInvocation:inv];

	[inv getReturnValue:result];
}

@interface NSMethodSignature (FFIClosure)
-(void*)_callingInfo;
-(void*)_closure;
@end

@implementation NSMethodSignature (FFIClosure)
-(void)_deallocateClosure
{
	if(_closureInfo && ((ffi_cif*)_closureInfo)->arg_types)
		NSZoneFree(NULL, ((ffi_cif*)_closureInfo)->arg_types);
	if(_closureInfo)
		NSZoneFree(NULL, _closureInfo);
	if(_closure)
		NSZoneFree(NULL, _closure);
}

-(void*)_callingInfo
{
	@synchronized(self)
	{
		if(!_closureInfo)
		{
			NSInteger i, numArgs=[self numberOfArguments];
			ffi_type** arg_type=NSZoneCalloc(NULL, sizeof(ffi_type*),numArgs);

			ffi_type* ret_type=signature_to_ffi_return_type([self methodReturnType]);

			for(i=0; i<numArgs; i++)
			{
				arg_type[i]=signature_to_ffi_type([self getArgumentTypeAtIndex:i]);
         }

			_closureInfo=NSZoneCalloc(NULL, sizeof(ffi_cif), 1);
			ffi_prep_cif((ffi_cif*)_closureInfo, FFI_DEFAULT_ABI,(unsigned)numArgs, ret_type, arg_type);
		}
	}
	NSAssert(_closureInfo, nil);
	return _closureInfo;
}

-(void*)_closure
{
	@synchronized([NSMethodSignature class])
	{
		if(!_closure)
		{
         _closure=_NSClosureAlloc(sizeof(ffi_closure));
			ffi_prep_closure((ffi_closure*)_closure, (ffi_cif*)[self _callingInfo], invocation_closure, self);
         _NSClosureProtect(_closure, sizeof(ffi_closure));
		}
	}
	NSAssert(_closure, nil);

	return _closure;
}

-(const char*)_realTypes
{
	return _typesCString;
}
@end

@implementation NSInvocation (FFICalling)


- (void)_ffiInvokeWithTarget:target
{
    NSInteger i, numArgs = [_signature numberOfArguments];
    void *arguments[numArgs + 8];
    ffi_cif *cif = [_signature _callingInfo];
    NSAssert(numArgs >= 2, @"invocation must have target and selector");
    const char *type = [_signature _realTypes];
    type = objc_skip_type_specifier(type, YES);
    for (i = 0; i < numArgs; i++) {
        arguments[i] = _argumentFrame + _argumentOffsets[i];
        type = objc_skip_type_specifier(type, YES);
    }

#if defined(GCC_RUNTIME_3) || defined(APPLE_RUNTIME_4)
    IMP imp = class_getMethodImplementation(object_getClass(target), [self selector]);
#else
    IMP imp = objc_msg_lookup(target, [self selector]);
#endif

    ffi_call(cif, FFI_FN(imp), _returnValue, arguments);
}


@end

id _objc_throwDoesNotRecognizeException(id object, SEL selector)
{
	Class       class=object->isa;
   NSRaiseException(NSInvalidArgumentException,
                    object,
                    selector,
                    @"Unrecognized selector sent to %p. Break on _objc_throwDoesNotRecognizeException to catch.", object);
   return nil;
}

IMP objc_forward_ffi(id object, SEL selector)
{
	NSMethodSignature *sig=[object methodSignatureForSelector:selector];

	if(sig)
		return (IMP)[sig _closure];
	return (IMP)_objc_throwDoesNotRecognizeException;
}

#endif // HAVE_LIBFFI

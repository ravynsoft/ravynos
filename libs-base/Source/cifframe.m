/** cifframe.m - Wrapper/Objective-C interface for ffi function interface

   Copyright (C) 1999-2015 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Dec 1999, rewritten Apr 2002

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
   */

#import "common.h"

#if !defined (__GNU_LIBOBJC__)
#  include <objc/encoding.h>
#endif

#ifdef HAVE_MALLOC_H
#if !defined(__OpenBSD__)
#include <malloc.h>
#endif
#endif

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include "cifframe.h"
#import "Foundation/NSException.h"
#import "Foundation/NSData.h"
#import "GSInvocation.h"
#import "GSPrivate.h"

/* ffi defines types in a very odd way that doesn't map to the
   normal objective-c type (see ffi.h). Here we make up for that */
#if GS_SIZEOF_SHORT == 2
#define gsffi_type_ushort ffi_type_uint16
#define gsffi_type_sshort ffi_type_sint16
#elif GS_SIZEOF_SHORT == 4
#define gsffi_type_ushort ffi_type_uint32
#define gsffi_type_sshort ffi_type_sint32
#else
#error FFI Sizeof SHORT case not handled
#endif

#if GS_SIZEOF_INT == 2
#define gsffi_type_uint ffi_type_uint16
#define gsffi_type_sint ffi_type_sint16
#elif GS_SIZEOF_INT == 4
#define gsffi_type_uint ffi_type_uint32
#define gsffi_type_sint ffi_type_sint32
#elif GS_SIZEOF_INT == 8
#define gsffi_type_uint ffi_type_uint64
#define gsffi_type_sint ffi_type_sint64
#else
#error FFI Sizeof INT case not handled
#endif

#if GS_SIZEOF_LONG == 2
#define gsffi_type_ulong ffi_type_uint16
#define gsffi_type_slong ffi_type_sint16
#elif GS_SIZEOF_LONG == 4
#define gsffi_type_ulong ffi_type_uint32
#define gsffi_type_slong ffi_type_sint32
#elif GS_SIZEOF_LONG == 8
#define gsffi_type_ulong ffi_type_uint64
#define gsffi_type_slong ffi_type_sint64
#else
#error FFI Sizeof LONG case not handled
#endif

#ifdef	_C_LNG_LNG
#if GS_SIZEOF_LONG_LONG == 8
#define gsffi_type_ulong_long ffi_type_uint64
#define gsffi_type_slong_long ffi_type_sint64
#else
#error FFI Sizeof LONG LONG case not handled
#endif
#endif

ffi_type *cifframe_type(const char *typePtr, const char **advance);

/* Best guess at the space needed for a structure, since we don't know
   for sure until it's calculated in ffi_prep_cif, which is too late */
int
cifframe_guess_struct_size(ffi_type *stype)
{
  int      i, size;
  unsigned align = __alignof(double);

  if (stype->elements == NULL)
    return stype->size;

  size = 0;
  i = 0;
  while (stype->elements[i])
    {
      if (stype->elements[i]->elements)
	size += cifframe_guess_struct_size(stype->elements[i]);
      else
	size += stype->elements[i]->size;

      if (size % align != 0)
	{
	  size += (align - size % align);
	}
      i++;
    }
  return size;
}


NSMutableData *
cifframe_from_signature (NSMethodSignature *info)
{
  unsigned      size = sizeof(cifframe_t);
  unsigned      align = __alignof(double);
  unsigned      type_offset = 0;
  unsigned      offset = 0;
  NSMutableData	*result;
  void          *buf;
  int           i;
  int		numargs = [info numberOfArguments];
  ffi_type      *rtype;
  ffi_type      *arg_types[numargs];
  cifframe_t    *cframe;

  /* FIXME: in cifframe_type, return values/arguments that are structures
     have custom ffi_types with are allocated separately. We should allocate
     them in our cifframe so we don't leak memory. Or maybe we could
     cache structure types? */
  rtype = cifframe_type([info methodReturnType], NULL);
  for (i = 0; i < numargs; i++)
    {
      arg_types[i] = cifframe_type([info getArgumentTypeAtIndex: i], NULL);
    }

  if (numargs > 0)
    {
      if (size % align != 0)
        {
          size += align - (size % align);
        }
      type_offset = size;
      /* Make room to copy the arg_types */
      size += sizeof(ffi_type *) * numargs;
      if (size % align != 0)
        {
          size += align - (size % align);
        }
      offset = size;
      size += numargs * sizeof(void*);
      if (size % align != 0)
        {
          size += (align - (size % align));
        }
      for (i = 0; i < numargs; i++)
        {
	  if (arg_types[i]->elements)
	    size += cifframe_guess_struct_size(arg_types[i]);
	  else
	    size += arg_types[i]->size;

          if (size % align != 0)
            {
              size += (align - size % align);
            }
        }
    }

  result = [NSMutableData dataWithCapacity: size];
  [result setLength: size];
  cframe = buf = [result mutableBytes];

  if (cframe)
    {
      cframe->nargs = numargs;
      cframe->arg_types = buf + type_offset;
      memcpy(cframe->arg_types, arg_types, sizeof(ffi_type *) * numargs);
      cframe->values = buf + offset;

      if (ffi_prep_cif (&cframe->cif, FFI_DEFAULT_ABI, numargs,
	rtype, cframe->arg_types) != FFI_OK)
	{
	  cframe = NULL;
	  result = NULL;
	}
      else
	{
	  /* Set values locations. This must be done after ffi_prep_cif so
	     that any structure sizes get calculated first. */
	  offset += numargs * sizeof(void*);
	  if (offset % align != 0)
	    {
	      offset += align - (offset % align);
	    }
	  for (i = 0; i < numargs; i++)
	    {
	      cframe->values[i] = buf + offset;

	      offset += arg_types[i]->size;

	      if (offset % align != 0)
		{
		  offset += (align - offset % align);
		}
	    }
	}
    }
  return result;
}

void
cifframe_set_arg(cifframe_t *cframe, int index, void *buffer, int size)
{
  if (index < 0 || index >= cframe->nargs)
     return;
  memcpy(cframe->values[index], buffer, size);
}

void
cifframe_get_arg(cifframe_t *cframe, int index, void *buffer, int size)
{
  if (index < 0 || index >= cframe->nargs)
     return;
  memcpy(buffer, cframe->values[index], size);
}

void *
cifframe_arg_addr(cifframe_t *cframe, int index)
{
  if (index < 0 || index >= cframe->nargs)
     return NULL;
  return cframe->values[index];
}

/*
 * Get the ffi_type for this type
 */
ffi_type *
cifframe_type(const char *typePtr, const char **advance)
{
  static ffi_type	stypeNSPoint = { 0 };
  static ffi_type	stypeNSRange = { 0 };
  static ffi_type	stypeNSRect = { 0 };
  static ffi_type	stypeNSSize = { 0 };
  const char *type;
  ffi_type *ftype = 0;

  typePtr = objc_skip_type_qualifiers (typePtr);
  type = typePtr;

  /*
   *	Scan for size and alignment information.
   */
  switch (*typePtr++)
    {
    case _C_ID: ftype = &ffi_type_pointer;
      break;
    case _C_CLASS: ftype = &ffi_type_pointer;
      break;
    case _C_SEL: ftype = &ffi_type_pointer;
      break;
    case _C_CHR: ftype = &ffi_type_schar;
      break;
    case _C_UCHR: ftype = &ffi_type_uchar;
      break;
    case _C_SHT: ftype = &gsffi_type_sshort;
      break;
    case _C_USHT: ftype = &gsffi_type_ushort;
      break;
    case _C_INT: ftype = &gsffi_type_sint;
      break;
    case _C_UINT: ftype = &gsffi_type_uint;
      break;
    case _C_LNG: ftype = &gsffi_type_slong;
      break;
    case _C_ULNG: ftype = &gsffi_type_ulong;
      break;
#ifdef	_C_LNG_LNG
    case _C_LNG_LNG: ftype = &gsffi_type_slong_long;
      break;
    case _C_ULNG_LNG: ftype = &gsffi_type_ulong_long;
      break;
#endif
    case _C_FLT: ftype = &ffi_type_float;
      break;
    case _C_DBL: ftype = &ffi_type_double;
      break;
    case _C_PTR:
      ftype = &ffi_type_pointer;
      if (*typePtr == '?')
	{
	  typePtr++;
	}
      else
	{
	  const char *adv;
	  cifframe_type(typePtr, &adv);
	  typePtr = adv;
	}
      break;

    case _C_ATOM:
    case _C_CHARPTR:
      ftype = &ffi_type_pointer;
      break;

    case _C_ARY_B:
      {
	const char *adv;
	ftype = &ffi_type_pointer;

	while (isdigit(*typePtr))
	  {
	    typePtr++;
	  }
	cifframe_type(typePtr, &adv);
	typePtr = adv;
	typePtr++;	/* Skip end-of-array	*/
      }
      break;

    case _C_STRUCT_B:
      {
	int types, maxtypes, size;
	ffi_type *local;
	const char *adv;
	unsigned   align = __alignof(double);

	/* Standard structures can be handled using cached type information.
	   Since the switch statement has already skipped the _C_STRUCT_B
	   character, we must use typePtr-1 below to successfully match the
	   type encoding with one of the standard type encodings. The same
	   holds for skipping past the whole structure type's encoding with
	   objc_skip_typespec.
	 */
	if (GSSelectorTypesMatch(typePtr - 1, @encode(NSRange)))
	  {
	    ftype = &stypeNSRange;
	    if (ftype->type == 0)
	      {
                static ffi_type	*elems[3];

		if (*@encode(NSUInteger) == _C_ULNG)
		  {
		    elems[0] = &gsffi_type_ulong;
		  }
#ifdef	_C_LNG_LNG
		else if (*@encode(NSUInteger) == _C_ULNG_LNG)
		  {
		    elems[0] = &gsffi_type_ulong_long;
		  }
#endif
		else
		  {
		    elems[0] = &gsffi_type_uint;
		  }
		elems[1] = elems[0];
		elems[2] = 0;
		ftype->elements = elems;
		ftype->type = FFI_TYPE_STRUCT;
	      }
	    typePtr = objc_skip_typespec (typePtr - 1);
	    break;
	  }
	else if (GSSelectorTypesMatch(typePtr - 1, @encode(NSPoint)))
	  {
	    ftype = &stypeNSPoint;
	    if (ftype->type == 0)
	      {
                static ffi_type	*elems[3];

		if (*@encode(CGFloat) == _C_DBL)
		  {
		    elems[0] = &ffi_type_double;
		  }
		else
		  {
		    elems[0] = &ffi_type_float;
		  }
		elems[1] = elems[0];
		elems[2] = 0;
		ftype->elements = elems;
		ftype->type = FFI_TYPE_STRUCT;
	      }
	    typePtr = objc_skip_typespec (typePtr - 1);
	    break;
	  }
	else if (GSSelectorTypesMatch(typePtr - 1, @encode(NSSize)))
	  {
	    ftype = &stypeNSSize;
	    if (ftype->type == 0)
	      {
                static ffi_type	*elems[3];

		if (*@encode(CGFloat) == _C_DBL)
		  {
		    elems[0] = &ffi_type_double;
		  }
		else
		  {
		    elems[0] = &ffi_type_float;
		  }
		elems[1] = elems[0];
		elems[2] = 0;
		ftype->elements = elems;
		ftype->type = FFI_TYPE_STRUCT;
	      }
	    typePtr = objc_skip_typespec (typePtr - 1);
	    break;
	  }
	else if (GSSelectorTypesMatch(typePtr - 1, @encode(NSRect)))
	  {
	    ftype = &stypeNSRect;
	    if (ftype->type == 0)
	      {
                static ffi_type	*elems[3];

		/* An NSRect is an NSPoint and an NSSize, but those
	 	 * two structures are actually identical.
		 */
		elems[0] = cifframe_type(@encode(NSSize), NULL);
		elems[1] = cifframe_type(@encode(NSPoint), NULL);
		elems[2] = 0;
		ftype->elements = elems;
		ftype->type = FFI_TYPE_STRUCT;
	      }
	    typePtr = objc_skip_typespec (typePtr - 1);
	    break;
	  }

	/*
	 *	Skip "<name>=" stuff.
	 */
	while (*typePtr != _C_STRUCT_E)
	  {
	    if (*typePtr++ == '=')
	      {
		break;
	      }
	  }

	types = 0;
	maxtypes = 4;
	size = sizeof(ffi_type);
	if (size % align != 0)
	  {
	    size += (align - (size % align));
	  }
	ftype = malloc(size + (maxtypes+1)*sizeof(ffi_type));
	ftype->size = 0;
	ftype->alignment = 0;
	ftype->type = FFI_TYPE_STRUCT;
	ftype->elements = (void*)ftype + size;
	/*
	 *	Continue accumulating structure size.
	 */
	while (*typePtr != _C_STRUCT_E)
	  {
	    local = cifframe_type(typePtr, &adv);
	    typePtr = adv;
	    NSCAssert(typePtr, @"End of signature while parsing");
	    ftype->elements[types++] = local;
	    if (types >= maxtypes)
	      {
		maxtypes *=2;
		ftype = realloc(ftype,
                  size + (maxtypes+1)*sizeof(ffi_type));
	        ftype->elements = (void*)ftype + size;
	      }
	  }
	ftype->elements[types] = NULL;
	typePtr++;	/* Skip end-of-struct	*/
      }
      break;

    case _C_UNION_B:
      {
	const char *adv;
	int	max_align = 0;

	/*
	 *	Skip "<name>=" stuff.
	 */
	while (*typePtr != _C_UNION_E)
	  {
	    if (*typePtr++ == '=')
	      {
		break;
	      }
	  }
	ftype = NULL;
	while (*typePtr != _C_UNION_E)
	  {
	    ffi_type *local;
	    int align = objc_alignof_type(typePtr);
	    local = cifframe_type(typePtr, &adv);
	    typePtr = adv;
	    NSCAssert(typePtr, @"End of signature while parsing");
	    if (align > max_align)
	      {
		if (ftype && ftype->type == FFI_TYPE_STRUCT
		  && ftype != &stypeNSPoint
		  && ftype != &stypeNSRange
		  && ftype != &stypeNSRect
		  && ftype != &stypeNSSize)
		  {
		    free(ftype);
		  }
		ftype = local;
		max_align = align;
	      }
	  }
	typePtr++;	/* Skip end-of-union	*/
      }
      break;

    case _C_VOID: ftype = &ffi_type_void;
      break;
#if __GNUC__ > 2 && defined(_C_BOOL)
    case _C_BOOL: ftype = &ffi_type_uchar;
      break;
#endif
    default:
      ftype = &ffi_type_void;
      NSCAssert(0, @"Unknown type in sig");
    }

  /* Skip past any offset information, if there is any */
  if (*type != _C_PTR || *type == '?')
    {
      if (*typePtr == '+')
	typePtr++;
      if (*typePtr == '-')
	typePtr++;
      while (isdigit(*typePtr))
	typePtr++;
    }
  if (advance)
    *advance = typePtr;

  return ftype;
}

GSCodeBuffer*
cifframe_closure (NSMethodSignature *sig, void (*cb)())
{
  NSMutableData		*frame;
  cifframe_t            *cframe;
  ffi_closure           *cclosure;
  void			*executable;
  GSCodeBuffer          *memory;

  /* Construct the frame (stored in an NSMutableDate object) and sety it
   * in a new closure.
   */
  frame = cifframe_from_signature(sig);
  cframe = [frame mutableBytes];
  memory = [GSCodeBuffer memoryWithSize: sizeof(ffi_closure)];
  [memory setFrame: frame];
  cclosure = [memory buffer];
  executable = [memory executable];
  if (cframe == NULL || cclosure == NULL)
    {
      [NSException raise: NSMallocException format: @"Allocating closure"];
    }
#if	HAVE_FFI_PREP_CLOSURE_LOC
  if (ffi_prep_closure_loc(cclosure, &(cframe->cif),
    cb, frame, executable) != FFI_OK)
    {
      [NSException raise: NSGenericException format: @"Preping closure"];
    }
#else
  executable = (void*)cclosure;
  if (ffi_prep_closure(cclosure, &(cframe->cif),
    cb, frame) != FFI_OK)
    {
      [NSException raise: NSGenericException format: @"Preping closure"];
    }
#endif
  [memory protect];
  return memory;
}

/*-------------------------------------------------------------------------*/
/* Functions for handling sending and receiving messages accross a
   connection
*/

/* Some return types actually get coded differently. We need to convert
   back to the expected return type */
BOOL
cifframe_decode_arg (const char *type, void* buffer)
{
  type = objc_skip_type_qualifiers (type);
  switch (*type)
    {
      case _C_CHR:
	*(signed char*)buffer = (signed char)(*((ffi_sarg *)buffer));
        break;
      case _C_UCHR:
	*(unsigned char*)buffer = (unsigned char)(*((ffi_arg *)buffer));
	break;
      case _C_SHT:
	*(signed short*)buffer = (signed short)(*((ffi_sarg *)buffer));
	break;
      case _C_USHT:
	*(unsigned short*)buffer = (unsigned short)(*((ffi_arg *)buffer));
	break;
      case _C_INT:
	*(signed int*)buffer = (signed int)(*((ffi_sarg *)buffer));
	break;
      case _C_UINT:
	*(unsigned int*)buffer = (unsigned int)(*((ffi_arg *)buffer));
	break;
      case _C_LNG:
        if (sizeof(signed long) < sizeof(ffi_sarg))
          *(signed long*)buffer = (signed long)(*((ffi_sarg *)buffer));
	break;
      case _C_ULNG:
        if (sizeof(unsigned long) < sizeof(ffi_arg))
          *(unsigned long*)buffer = (unsigned long)(*((ffi_arg *)buffer));
	break;
      default:
        return NO;
    }
  return YES;
}

BOOL
cifframe_encode_arg (const char *type, void* buffer)
{
  type = objc_skip_type_qualifiers (type);
  switch (*type)
    {
      case _C_CHR:
        *(ffi_sarg *)buffer = (ffi_sarg)(*((signed char *)buffer));
        break;
      case _C_UCHR:
        *(ffi_arg *)buffer = (ffi_arg)(*((unsigned char *)buffer));
        break;
      case _C_SHT:
        *(ffi_sarg *)buffer = (ffi_sarg)(*((signed short *)buffer));
        break;
      case _C_USHT:
        *(ffi_arg *)buffer = (ffi_arg)(*((unsigned short *)buffer));
        break;
      case _C_INT:
        *(ffi_sarg *)buffer = (ffi_sarg)(*((signed int *)buffer));
        break;
      case _C_UINT:
        *(ffi_arg *)buffer = (ffi_arg)(*((unsigned int *)buffer));
        break;
      case _C_LNG:
        if (sizeof(signed long) < sizeof(ffi_sarg))
          *(ffi_sarg *)buffer = (ffi_sarg)(*((signed long *)buffer));
        break;
      case _C_ULNG:
        if (sizeof(unsigned long) < sizeof(ffi_arg))
          *(ffi_arg *)buffer = (ffi_arg)(*((unsigned int *)buffer));
        break;
      default:
        return NO;
    }
  return YES;
}


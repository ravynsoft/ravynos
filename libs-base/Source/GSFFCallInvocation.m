/** Implementation of GSFFCallInvocation for GNUStep
   Copyright (C) 2000 Free Software Foundation, Inc.

   Written: Adam Fedor <fedor@gnu.org>
   Date: Nov 2000

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
#import "Foundation/NSException.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSDistantObject.h"
#import "GSInvocation.h"
#import <avcall.h>
#import <callback.h>
#import "callframe.h"

#if !defined (__GNU_LIBOBJC__)
#  include <objc/encoding.h>
#endif

#include <pthread.h>

#import "GSInvocation.h"

#ifndef GS_STATIC_INLINE
#define GS_STATIC_INLINE static inline
#endif


typedef struct _NSInvocation_t {
  @defs(NSInvocation)
} NSInvocation_t;


/*
  Wim Oudshoorn (3 aug 2001)

  This information is used by GSInvocationCallback
  actually the last three arguments are only used
  when type == __VAstruct, but the code becomes
  more difficult when you try to optimize for
  it.  And what are 15 * 4 * 3 = 180 bytes anyway.
*/
typedef struct _vacallReturnTypeInfo_t
{
  enum __VAtype type;
  unsigned structSize;
  unsigned structAlign;
  unsigned structSplit;
} vacallReturnTypeInfo;

/*
   Create the map table for the forwarding functions
 */
#define GSI_MAP_KTYPES GSUNION_PTR
#define GSI_MAP_VTYPES GSUNION_PTR

/*
  Wim Oudshoorn (6 aug 2001)

  Hash function for the mapping
  return_type --> callback functions

  Rationale for the magic constants.
  ----------------------------------
  We want to avoid hash colissions.
  So we encode the hash value as follows:

  +------+--------+----------+-------+
  | Size |alignmnt|splittable| vaType|
  | 24bit| 3bit   | 1bit     | 4bit  |
  +------+--------+----------+-------+
 */

GS_STATIC_INLINE unsigned int
ReturnTypeHash (vacallReturnTypeInfo *ret_type)
{
  return ret_type->type
    ^ ret_type->structSplit << 4
    ^ ret_type->structAlign << 5
    ^ ret_type->structSize << 8;
}

/*
  Wim Oudshoorn (6 aug 2001)

  Comparison function, used by the hash
  table.  I tried to order the comparison
  so that the earlier comparisons
  fail more often than later comparisons.
 */
GS_STATIC_INLINE BOOL
ReturnTypeEqualsReturnType (vacallReturnTypeInfo *a, vacallReturnTypeInfo *b)
{
  return (a->structSize == b->structSize)
    && (a->structAlign == b->structAlign)
    && (a->structSplit == b->structSplit)
    && (a->type == b->type);
}

#define GSI_MAP_HASH(M, X)        ReturnTypeHash (X.ptr)
#define GSI_MAP_EQUAL(M, X,Y)     ReturnTypeEqualsReturnType (X.ptr, Y.ptr)
#define GSI_MAP_RETAIN_KEY(M, X)
#define GSI_MAP_RETAIN_VAL(M, X)
#define GSI_MAP_RELEASE_KEY(M, X)
#define GSI_MAP_RELEASE_VAL(M, X)
#define	GSI_MAP_NOCLEAN	1

#include "GNUstepBase/GSIMap.h"

/* This determines the number of precomputed
   callback data entries.  The list is indexed
   by __VAtype and only usefull for non-struct
   types.  Therefore it is of no use increasing
   the size of this table.  Except if the
   callback module of ffcall changes
*/
#define STATIC_CALLBACK_LIST_SIZE 15

/* Callback functions for forwarding methods */

static void         *ff_callback [STATIC_CALLBACK_LIST_SIZE];
static GSIMapTable_t ff_callback_map;

/* Lock that protects the ff_callback_map */

static pthread_mutex_t ff_callback_map_lock = PTHREAD_MUTEX_INITIALIZER;

/* Static pre-computed return type info */

static vacallReturnTypeInfo returnTypeInfo [STATIC_CALLBACK_LIST_SIZE];

/* Function that implements the actual forwarding */
static void
GSInvocationCallback(void *callback_data, va_alist args);

/* Count the number of subtypes in a structure
 */
static const char *gs_subtypes(const char *type, int *result)
{
  int	count = 0;

  if (*type == _C_STRUCT_B)
    {
      type++;
      while (*type != _C_STRUCT_E && *type++ != '='); /* skip "<name>=" */
      while (*type != '\0' && *type != _C_STRUCT_E)
        {
	  count++;
	  if (*type == _C_STRUCT_B)
	    {
	      /* count a nested structure as a single type.
	       */
	      type = gs_subtypes (type, 0);
	    }
	  else
	    {
	      type = objc_skip_typespec (type);
	    }
	}
      if (*type == _C_STRUCT_E)
        {
	  type++;	/* step past end of struct */
	}
    }
  if (result != 0)
    {
      *result = count;
    }
  return type;
}

/* return the index'th subtype
 */
static __attribute__((unused))
const char *gs_subtype(const char *type, int index)
{
  int	count = 0;

  if (*type != _C_STRUCT_B)
    {
      return "";
    }
  type++;
  while (*type != _C_STRUCT_E && *type++ != '='); /* skip "<name>=" */
  while (*type != '\0' && *type != _C_STRUCT_E)
    {
      if (count++ == index)
	{
	  return type;
	}
      if (*type == _C_STRUCT_B)
	{
	  /* count and skip a nested structure as a single type.
	   */
	  type = gs_subtypes (type, 0);
	}
      else
	{
	  type = objc_skip_typespec (type);
	}
    }
  if (*type == _C_STRUCT_E)
    {
      type++;	/* step past end of struct */
    }
  return type;
}

/*
 * Recursively calculate the offset using the offset of the previous
 * sub-type
 */
static int
gs_offset(const char *type, int index)
{
  int offset;
  const char *subtype;

  if (index == 0)
    return 0;
  subtype = type;
  while (*subtype != _C_STRUCT_E && *subtype++ != '='); /* skip "<name>=" */

  offset = (gs_offset(type, index-1) + objc_sizeof_type(&subtype[index-1])
    + objc_alignof_type(&subtype[index]) - 1)
    & -(long)objc_alignof_type(&subtype[index]);
  return offset;
}

/* Determines if the structure type can be returned entirely in registers.
   See the avcall or vacall man pages for more info. FIXME: I'm betting
   this won't work if a structure contains another structure */
static int
gs_splittable (const char *type)
{
  int i, numtypes;
  const char *subtype;
  int  result;

  subtype = type;
  while (*subtype != _C_STRUCT_E && *subtype++ != '='); /* skip "<name>=" */
  numtypes = 0;
  while (*subtype != _C_STRUCT_E)
    {
      numtypes++;
      subtype = objc_skip_typespec (subtype);
    }
  subtype = type;
  while (*subtype != _C_STRUCT_E && *subtype++ != '='); /* skip "<name>=" */

  result = 1;
  for (i = 0; i < numtypes; i++)
    {
      result = result
	&& (gs_offset(type, i)/sizeof(__avword)
	    == (gs_offset(type, i)+objc_sizeof_type(&subtype[i])-1)
	       / sizeof(__avword));
    }
  //printf("Splittable for %s is %d\n", type, result);
  return result;
}



/*
 * If we are using the GNU ObjC runtime we could
 * simplify this function quite a lot because this
 * function is already present in the ObjC runtime.
 * However, it is not part of the public API, so
 * we work around it.
 */

GS_STATIC_INLINE GSMethod
gs_method_for_receiver_and_selector (id receiver, SEL sel)
{
  if (receiver)
    {
      return GSGetMethod((GSObjCIsInstance(receiver)
			  ? object_getClass(receiver) : (Class)receiver),
			 sel,
			 GSObjCIsInstance(receiver),
			 YES);
    }

  return METHOD_NULL;
}


/*
 * Selectors are not unique, and not all selectors have
 * type information.  This method tries to find the
 * best equivalent selector with type information.
 *
 * the conversion sel -> name -> sel
 * is not what we want.  However
 * I can not see a way to dispose of the
 * name, except if we can access the
 * internal data structures of the runtime.
 *
 * If we can access the private data structures
 * we can also check for incompatible
 * return types between all equivalent selectors.
 */

GS_STATIC_INLINE SEL
gs_find_best_typed_sel (SEL sel)
{
  if (!sel_get_type (sel))
    {
      const char *name = sel_getName(sel);

      if (name)
	{
	  SEL tmp_sel = sel_get_any_typed_uid (name);
	  if (sel_get_type (tmp_sel))
	    return tmp_sel;
	}
    }
  return sel;
}

/*
 * Take the receiver into account for finding the best
 * selector.  That is, we look if the receiver
 * implements the selector and the implementation
 * selector has type info.  If both conditions
 * are satisfied, return this selector.
 *
 * In all other cases fallback
 * to gs_find_best_typed_sel ().
 */
GS_STATIC_INLINE SEL
gs_find_by_receiver_best_typed_sel (id receiver, SEL sel)
{
  if (sel_get_type (sel))
    return sel;

  if (receiver)
    {
      GSMethod method;

      method = gs_method_for_receiver_and_selector (receiver, sel);
      /* CHECKME:  Can we assume that:
	 (a) method_name is a selector (compare libobjc header files)
	 (b) this selector IS really typed?
	 At the moment I assume (a) but not (b)
         not assuming (b) is the reason for
         calling gs_find_best_typed_sel () even
         if we have an implementation.
      */
      if (method)
	sel = method->method_name;
    }
  return gs_find_best_typed_sel (sel);
}

/*
  Convert objc selector type to a vacallReturnTypeInfo.
  Only passes the first part.  Is used for determining
  the return type for the vacall macros.
*/
static void
gs_sel_type_to_callback_type (const char *sel_type,
  vacallReturnTypeInfo *vatype)
{
  switch (*sel_type)
    {
      case _C_ID:
      case _C_CLASS:
      case _C_SEL:
      case _C_PTR:
      case _C_CHARPTR:
	vatype->type = __VAvoidp;
	break;
      case _C_CHR:
	vatype->type = __VAchar;
	break;
      case _C_UCHR:
	vatype->type = __VAuchar;
	break;
      case _C_SHT:
	vatype->type = __VAshort;
	break;
      case _C_USHT:
	vatype->type = __VAushort;
	break;
      case _C_INT:
	vatype->type = __VAint;
	break;
      case _C_UINT:
	vatype->type = __VAuint;
	break;
      case _C_LNG:
	vatype->type = __VAlong;
	break;
      case _C_ULNG:
	vatype->type = __VAulong;
	break;
      case _C_LNG_LNG:
	vatype->type = __VAlonglong;
	break;
      case _C_ULNG_LNG:
	vatype->type = __VAulonglong;
	break;
      case _C_FLT:
	vatype->type = __VAfloat;
	break;
      case _C_DBL:
	vatype->type = __VAdouble;
	break;
#if __GNUC__ > 2 && defined(_C_BOOL)
      case _C_BOOL:
	vatype->type = __VAuchar;
	break;
#endif
      case _C_STRUCT_B:
	vatype->structSize = objc_sizeof_type (sel_type);
	if (vatype->structSize > sizeof (long)
	  && vatype->structSize <= 2 * sizeof (long))
	  vatype->structSplit = gs_splittable (sel_type);
	vatype->structAlign = objc_alignof_type (sel_type);
	vatype->type = __VAstruct;
	break;
      case _C_VOID:
	vatype->type = __VAvoid;
	break;
      default:
	NSCAssert1 (0, @"GSFFCallInvocation: Return Type '%s' not implemented",
	  sel_type);
	break;
    }
}


@implementation GSFFCallInvocation

static IMP gs_objc_msg_forward (SEL sel)
{
  const char		*sel_type;
  vacallReturnTypeInfo	returnInfo;
  void			*forwarding_callback;

  /*
   * 1. determine return type.  The compiler should have provided us with
   * a typed selector if possible, if not we have to assume an id return.
   */
  sel_type = sel_get_type (sel);

  if (!sel_type)
    {
      sel_type = "@";	// Default to id return type
    }

  sel_type = objc_skip_type_qualifiers (sel_type);
  gs_sel_type_to_callback_type (sel_type, &returnInfo);

  /*
   * 2. Check if we have already a callback
   */
  if ((returnInfo.type < STATIC_CALLBACK_LIST_SIZE)
    && (returnInfo.type != __VAstruct))
    {
      // 2.a Do we have a statically precomputed callback
      forwarding_callback = ff_callback [returnInfo.type];
    }
  else
    {
      // 2.b Or do we have it already in our hash table
      GSIMapNode node;

      // Lock
      pthread_mutex_lock (&ff_callback_map_lock);

      node = GSIMapNodeForKey (&ff_callback_map,
	(GSIMapKey) ((void *) &returnInfo));

      if (node)
	{
	  // 2.b.1 YES, we have it in our cache
	  forwarding_callback =  node->value.ptr;
	}
      else
	{
	  // 2.b.2 NO, we do not have it.
	  vacallReturnTypeInfo *ret_info;

	  ret_info = malloc(sizeof (vacallReturnTypeInfo));
	  *ret_info = returnInfo;
	
	  forwarding_callback
	    = alloc_callback (&GSInvocationCallback, ret_info);

	  GSIMapAddPairNoRetain (&ff_callback_map,
	    (GSIMapKey) (void *) ret_info,
	    (GSIMapVal) forwarding_callback);
	}
      // Unlock
      pthread_mutex_unlock (&ff_callback_map_lock);
    }
  return forwarding_callback;
}

+ (void) load
{
  int index;

  for (index = 0; index < STATIC_CALLBACK_LIST_SIZE; ++index)
    {
      returnTypeInfo[index].type = index;
      ff_callback[index] = alloc_callback (&GSInvocationCallback,
	&returnTypeInfo [index]);
    }

  GSIMapInitWithZoneAndCapacity (&ff_callback_map, NSDefaultMallocZone(), 9);

  __objc_msg_forward = gs_objc_msg_forward;
}

- (id) initWithArgframe: (arglist_t)frame selector: (SEL)aSelector
{
  /* We should never get here */
  [self dealloc];
  self = nil;
  [NSException raise: NSInternalInconsistencyException
	      format: @"Runtime incorrectly configured to pass argframes"];
  return nil;
}

/*
 *	This is the designated initialiser.
 */
- (id) initWithMethodSignature: (NSMethodSignature*)aSignature
{
  if (aSignature == nil)
    {
      DESTROY(self);
      return nil;
    }
  _sig = RETAIN(aSignature);
  _numArgs = [aSignature numberOfArguments];
  _info = (void*)[aSignature methodInfo];
  _cframe = callframe_from_signature(_sig, &_retval);
  return self;
}

/*
 * This is implemented as a function so it can be used by other
 * routines (like the DO forwarding)
 */
void
GSFFCallInvokeWithTargetAndImp(NSInvocation *_inv, id anObject, IMP imp)
{
  unsigned int		i;
  av_alist		alist;
  NSInvocation_t	*inv = (NSInvocation_t*)_inv;
  NSArgumentInfo	*info = (NSArgumentInfo*)inv->_info;
  void			*retval = inv->_retval;

  /* Do an av call starting with the return type */
#undef CASE_TYPE
#define CASE_TYPE(_T, _V, _F)				\
	case _T:					\
	  _F(alist, imp, retval);	       		\
          break;

  switch (*info[0].type)
    {
      case _C_ID:
	av_start_ptr(alist, imp, id, retval);
	break;

      case _C_CLASS:
	av_start_ptr(alist, imp, Class, retval);
	break;

      case _C_SEL:
	av_start_ptr(alist, imp, SEL, retval);
	break;

      case _C_PTR:
	av_start_ptr(alist, imp, void *, retval);
	break;

      case _C_CHARPTR:
	av_start_ptr(alist, imp, char *, retval);
	break;
	
      CASE_TYPE(_C_CHR,  char, av_start_char)
      CASE_TYPE(_C_UCHR, unsigned char, av_start_uchar)
      CASE_TYPE(_C_SHT,  short, av_start_short)
      CASE_TYPE(_C_USHT, unsigned short, av_start_ushort)
      CASE_TYPE(_C_INT,  int, av_start_int)
      CASE_TYPE(_C_UINT, unsigned int, av_start_uint)
      CASE_TYPE(_C_LNG,  long, av_start_long)
      CASE_TYPE(_C_ULNG, unsigned long, av_start_ulong)
      CASE_TYPE(_C_LNG_LNG,  long long, av_start_longlong)
      CASE_TYPE(_C_ULNG_LNG, unsigned long long, av_start_ulonglong)
      CASE_TYPE(_C_FLT,  float, av_start_float)
      CASE_TYPE(_C_DBL,  double, av_start_double)
#if __GNUC__ > 2 && defined(_C_BOOL)
      CASE_TYPE(_C_BOOL, _Bool, av_start_uchar)
#endif

      case _C_STRUCT_B:
	{
	  int split = 0;

	  if (info[0].size > sizeof(long)
	    && info[0].size <= 2*sizeof(long))
	    {
	      split = gs_splittable(info[0].type);
	    }
	  _av_start_struct(alist, imp,
	    info[0].size, split, retval);
	  break;
	}

      case _C_VOID:
	av_start_void(alist, imp);
	break;

      default:
	NSCAssert1(0, @"GSFFCallInvocation: Return Type '%s' not implemented",
	  info[0].type);
	break;
    }

  /* Set target and selector */
  av_ptr(alist, id, anObject);
  av_ptr(alist, SEL, inv->_selector);

  /* Set the rest of the arguments */
  for (i = 2; i < inv->_numArgs; i++)
    {
      const char	*type = info[i+1].type;
      unsigned		size = info[i+1].size;
      void              *datum;

      datum = callframe_arg_addr((callframe_t *)inv->_cframe, i);

#undef CASE_TYPE
#define CASE_TYPE(_T, _V, _F)				\
	case _T:					\
	  {						\
	    _V c;          				\
            memcpy(&c, datum, size);                    \
            _F(alist, c);                               \
	    break;					\
	  }

      switch (*type)
	{
	  case _C_ID:
	    {
	      id obj;
	      memcpy(&obj, datum, size);
	      av_ptr(alist, id, obj);
	      break;
	    }

	  case _C_CLASS:
	    {
	      Class obj;
	      memcpy(&obj, datum, size);
	      av_ptr(alist, Class, obj);
	      break;
	    }

	  case _C_SEL:
	    {
	      SEL sel;
	      memcpy(&sel, datum, size);
	      av_ptr(alist, SEL, sel);
	      break;
	    }

	  case _C_PTR:
	    {
	      void *ptr;
	      memcpy(&ptr, datum, size);
	      av_ptr(alist, void *, ptr);
	      break;
	    }

	  case _C_CHARPTR:
	    {
	      char *ptr;
	      memcpy(&ptr, datum, size);
	      av_ptr(alist, char *, ptr);
	      break;
	    }
	
          CASE_TYPE(_C_CHR,  char, av_char)
          CASE_TYPE(_C_UCHR, unsigned char, av_uchar)
          CASE_TYPE(_C_SHT,  short, av_short)
          CASE_TYPE(_C_USHT, unsigned short, av_ushort)
          CASE_TYPE(_C_INT,  int, av_int)
          CASE_TYPE(_C_UINT, unsigned int, av_uint)
          CASE_TYPE(_C_LNG,  long, av_long)
          CASE_TYPE(_C_ULNG, unsigned long, av_ulong)
          CASE_TYPE(_C_LNG_LNG,  long long, av_longlong)
          CASE_TYPE(_C_ULNG_LNG, unsigned long long, av_ulonglong)
          CASE_TYPE(_C_FLT,  float, av_float)
          CASE_TYPE(_C_DBL,  double, av_double)
#if __GNUC__ > 2 && defined(_C_BOOL)
          CASE_TYPE(_C_BOOL, _Bool, av_uchar)
#endif
	
	  case _C_STRUCT_B:
	    _av_struct(alist, size,
	      info[i+1].align, datum);
	    break;

	  default:
	    NSCAssert1(0, @"GSFFCallInvocation: Type '%s' not implemented",
	      type);
	    break;
	}
    }

  /* Do it */
  av_call(alist);
}

- (void) invokeWithTarget: (id)anObject
{
  id		old_target;
  IMP		imp;

  CLEAR_RETURN_VALUE_IF_OBJECT;
  _validReturn = NO;

  /*
   *	A message to a nil object returns nil.
   */
  if (anObject == nil)
    {
      memset(_retval, '\0', _inf[0].size);	/* Clear return value */
      if (*_inf[0].type != _C_VOID)
        {
          _validReturn = YES;
        }
      return;
    }


  NSAssert(_selector != 0, @"you must set the selector before invoking");

  /*
   *	Temporarily set new target and copy it (and the selector) into the
   *	_cframe.
   */
  old_target = RETAIN(_target);
  [self setTarget: anObject];

  callframe_set_arg((callframe_t *)_cframe, 0, &_target, _inf[1].size);
  callframe_set_arg((callframe_t *)_cframe, 1, &_selector, _inf[2].size);

  if (_sendToSuper == YES)
    {
      Super	s;

      s.self = _target;
      if (GSObjCIsInstance(_target))
	s.class = class_getSuperclass(object_getClass(_target));
      else
	s.class = class_getSuperclass((Class)_target);
      imp = objc_msg_lookup_super(&s, _selector);
    }
  else
    {
      GSMethod method;
      method = GSGetMethod((GSObjCIsInstance(_target)
                            ? (id)object_getClass(_target)
                            : (id)_target),
                           _selector,
                           GSObjCIsInstance(_target),
                           YES);
      imp = method_get_imp(method);
      /*
       * If fast lookup failed, we may be forwarding or something ...
       */
      if (imp == 0)
	{
	  imp = objc_msg_lookup(_target, _selector);
	}
    }

  [self setTarget: old_target];
  RELEASE(old_target);

  GSFFCallInvokeWithTargetAndImp(self, anObject, imp);

  RETAIN_RETURN_VALUE;
  _validReturn = YES;
}

- (void*) returnFrame: (arglist_t)argFrame
{
  return _retval;
}
@end

/*
 * Return YES if the selector contains protocol qualifiers.
 */
static BOOL
gs_protocol_selector(const char *types)
{
  if (types == 0)
    {
      return NO;
    }
  while (*types != '\0')
    {
      if (*types == '+' || *types == '-')
	{
	  types++;
	}
      while (isdigit(*types))
	{
	  types++;
	}
      while (*types == _C_CONST || *types == _C_GCINVISIBLE)
	{
	  types++;
	}
      if (*types == _C_IN
	|| *types == _C_INOUT
	|| *types == _C_OUT
	|| *types == _C_BYCOPY
	|| *types == _C_BYREF
	|| *types == _C_ONEWAY)
	{
	  return YES;
	}
      if (*types == '\0')
	{
	  return NO;
	}
      types = objc_skip_typespec(types);
    }
  return NO;
}

/*
 * Wim Oudshoorn (6 aug 2001)
 *
 * The function that performs the actual forwarding
 * `callback_data' contains the information needed
 * in order pop off the receiver and selector from
 * the va_list `args'
 *
 * TODO:
 * Add a check that the return type the selector
 * expects matches the the `callback_data'
 * information.
 */

static void
GSInvocationCallback (void *callback_data, va_alist args)
{
  id			obj;
  SEL			selector;
  int			i;
  int			num_args;
  void			*retval;
  vacallReturnTypeInfo	*typeinfo;
  NSArgumentInfo	*info;
  GSFFCallInvocation	*invocation;
  NSMethodSignature	*sig;
  GSMethod               fwdInvMethod;

  typeinfo = (vacallReturnTypeInfo *) callback_data;

  if (typeinfo->type != __VAstruct)
    {
      __va_start (args, typeinfo->type);
    }
  else
    {
      _va_start_struct (args, typeinfo->structSize,
	typeinfo->structAlign, typeinfo->structSplit);
    }

  obj      = va_arg_ptr(args, id);
  selector = va_arg_ptr(args, SEL);

  fwdInvMethod = gs_method_for_receiver_and_selector
    (obj, @selector (forwardInvocation:));

  if (!fwdInvMethod)
    {
      [NSException raise: NSInvalidArgumentException
		   format: @"GSFFCallInvocation: Class '%s(%s)'"
		           @" does not respond"
		           @" to forwardInvocation: for '%s'",
		   GSClassNameFromObject(obj),
		   GSObjCIsInstance(obj) ? "instance" : "class",
		   selector ? sel_getName(selector) : "(null)"];
    }

  sig = nil;
  if (gs_protocol_selector(sel_get_type(selector)) == YES)
    {
      /*
       * We already have protocol information locally, so we don't need
       * to get it from the remote system.
       */
      sig = [NSMethodSignature signatureWithObjCTypes: sel_get_type(selector)];
    }
  if (sig == nil)
    {
      sig = [obj methodSignatureForSelector: selector];
    }

  /*
   * If we got a method signature from the receiving object,
   * ensure that the selector we are using matches the types.
   */
  if (sig != nil)
    {
      const char	*receiverTypes = [sig methodType];
      const char	*runtimeTypes = sel_get_type (selector);

      if (runtimeTypes == 0
        || NO == GSSelectorTypesMatch(receiverTypes, runtimeTypes))
	{
	  const char	*runtimeName = sel_getName(selector);

	  runtimeTypes = GSTypesFromSelector(selector);
	  if (selector == 0)
	    {
	      selector = GSSelectorFromNameAndTypes(runtimeName, receiverTypes);
	    }
	  if (runtimeTypes != 0)
	    {
	      /*
	       * FIXME ... if we have a typed selector, it probably came
	       * from the compiler, and the types of the proxied method
	       * MUST match those that the compiler supplied on the stack
	       * and the type it expects to retrieve from the stack.
	       * We should therefore discriminate between signatures where
	       * type qalifiers and sizes differ, and those where the
	       * actual types differ.
	       */
	      NSDebugFLog(@"Changed type signature '%s' to '%s' for '%s'",
		runtimeTypes, receiverTypes, runtimeName);
	    }
	}
    }

  if (sig == nil)
    {
      selector = gs_find_best_typed_sel (selector);

      if (sel_get_type (selector) != 0)
    {
      sig = [NSMethodSignature signatureWithObjCTypes: sel_get_type(selector)];
    }
    }

  if (sig == nil)
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Can not determine type information for %s[%s %s]",
                   GSObjCIsInstance(obj) ? "-" : "+",
	 GSClassNameFromObject(obj),
	 selector ? sel_getName(selector) : "(null)"];
    }

  invocation = [[GSFFCallInvocation alloc] initWithMethodSignature: sig];
  AUTORELEASE(invocation);
  [invocation setTarget: obj];
  [invocation setSelector: selector];

  /* Set the rest of the arguments */
  num_args = [sig numberOfArguments];
  info = [sig methodInfo];
  for (i = 2; i < num_args; i++)
    {
      const char	*type = info[i+1].type;
      unsigned		size = info[i+1].size;

#undef CASE_TYPE
#define CASE_TYPE(_T, _V, _F)				\
	  case _T:					\
	    {						\
	      _V c = _F(args);				\
	      [invocation setArgument: &c atIndex: i];	\
	      break;					\
	    }

      switch (*type)
	{
	  case _C_ID:
	    {
	      id obj = va_arg_ptr (args, id);
	      [invocation setArgument: &obj atIndex: i];
	      break;
	    }

	  case _C_CLASS:
	    {
	      Class obj = va_arg_ptr (args, Class);
	      [invocation setArgument: &obj atIndex: i];
	      break;
	    }

	  case _C_SEL:
	    {
	      SEL sel = va_arg_ptr (args, SEL);
	      [invocation setArgument: &sel atIndex: i];
	      break;
	    }

	  case _C_PTR:
	    {
	      void *ptr = va_arg_ptr (args, void *);
	      [invocation setArgument: &ptr atIndex: i];
	      break;
	    }

	  case _C_CHARPTR:
	    {
	      char *ptr = va_arg_ptr (args, char *);
	      [invocation setArgument: &ptr atIndex: i];
	      break;
	    }
	
	  CASE_TYPE(_C_CHR,  char, va_arg_char)
	  CASE_TYPE(_C_UCHR, unsigned char, va_arg_uchar)
	  CASE_TYPE(_C_SHT,  short, va_arg_short)
	  CASE_TYPE(_C_USHT, unsigned short, va_arg_ushort)
	  CASE_TYPE(_C_INT,  int, va_arg_int)
	  CASE_TYPE(_C_UINT, unsigned int, va_arg_uint)
	  CASE_TYPE(_C_LNG,  long, va_arg_long)
	  CASE_TYPE(_C_ULNG, unsigned long, va_arg_ulong)
	  CASE_TYPE(_C_LNG_LNG,  long long, va_arg_longlong)
	  CASE_TYPE(_C_ULNG_LNG, unsigned long long, va_arg_ulonglong)
	  CASE_TYPE(_C_FLT,  float, va_arg_float)
	  CASE_TYPE(_C_DBL,  double, va_arg_double)
#if __GNUC__ > 2 && defined(_C_BOOL)
	  CASE_TYPE(_C_BOOL,  _Bool, va_arg_uchar)
#endif
	
	  case _C_STRUCT_B:
	    {
	      /* Here we actually get a ptr to the struct */
	      void *ptr = _va_arg_struct(args, size, info[i+1].align);
	      [invocation setArgument: ptr atIndex: i];
	      break;
	    }

	  default:
	    NSCAssert1(0, @"GSFFCallInvocation: Type '%s' not implemented",
	      type);
	    break;
	}
    }

  /*
   * Now do it.
   * The next line is equivalent to
   *
   *   [obj forwardInvocation: invocation];
   *
   * but we have already the GSMethod for forwardInvocation
   * so the line below is somewhat faster. */
  fwdInvMethod->method_imp (obj, fwdInvMethod->method_name, invocation);

  /* Return the proper type */
  retval = [invocation returnFrame: NULL];

#undef CASE_TYPE
#define CASE_TYPE(_T, _V, _F)				\
      case _T:					\
	if (typeinfo->type == __VAvoidp) \
	    va_return_ptr(args, void *, *(void **)retval); \
	else \
	_F(args, *(_V *)retval);       		\
	break;

  switch (*info[0].type)
    {
      case _C_ID:
      case _C_CLASS:
      case _C_SEL:
      case _C_PTR:
      case _C_CHARPTR:
	va_return_ptr(args, void *, *(void **)retval);
	break;
	
      CASE_TYPE(_C_CHR,  char, va_return_char)
      CASE_TYPE(_C_UCHR, unsigned char, va_return_uchar)
      CASE_TYPE(_C_SHT,  short, va_return_short)
      CASE_TYPE(_C_USHT, unsigned short, va_return_ushort)
      CASE_TYPE(_C_INT,  int, va_return_int)
      CASE_TYPE(_C_UINT, unsigned int, va_return_uint)
      CASE_TYPE(_C_LNG,  long, va_return_long)
      CASE_TYPE(_C_ULNG, unsigned long, va_return_ulong)
      CASE_TYPE(_C_LNG_LNG,  long long, va_return_longlong)
      CASE_TYPE(_C_ULNG_LNG, unsigned long long, va_return_ulonglong)
      CASE_TYPE(_C_FLT,  float, va_return_float)
      CASE_TYPE(_C_DBL,  double, va_return_double)
#if __GNUC__ > 2 && defined(_C_BOOL)
      CASE_TYPE(_C_BOOL, _Bool, va_return_uchar)
#endif

      case _C_STRUCT_B:
	_va_return_struct(args, info[0].size, info[0].align, retval);
	break;

      case _C_VOID:
	/* FIXME ... evil hack ... where the compiler did not know
	 * selector types, if may have had to assume a method returning
	 * an id, but the actual method may have returned void ...
	 * we check for that case here, and use the fact that in the case
	 * of a void return value, passing retval back as a voipd will
	 * look like the method actually returned nil.
	 */
	if (typeinfo->type == __VAvoidp)
	  {
	    va_return_ptr(args, void *, *(void **)retval);
	  }
	else
	  {
	    va_return_void(args);
	  }
	break;

      default:
	NSCAssert1(0, @"GSFFCallInvocation: Return Type '%s' not implemented",
	  info[0].type);
	break;
    }
}

@implementation NSInvocation (DistantCoding)

/* An internal method used to help NSConnections code invocations
   to send over the wire */
- (BOOL) encodeWithDistantCoder: (NSCoder*)coder passPointers: (BOOL)passp
{
  unsigned int	i;
  BOOL		out_parameters = NO;
  const char	*type = [_sig methodType];

  [coder encodeValueOfObjCType: @encode(char*) at: &type];

  for (i = 0; i < _numArgs; i++)
    {
      int		flags = _inf[i+1].qual;
      const char	*type = _inf[i+1].type;
      void		*datum;

      if (i == 0)
	{
	  datum = &_target;
	}
      else if (i == 1)
	{
	  datum = &_selector;
	}
      else
	{
	  datum = callframe_arg_addr((callframe_t *)_cframe, i);
	}

      /*
       * Decide how, (or whether or not), to encode the argument
       * depending on its FLAGS and TYPE.  Only the first two cases
       * involve parameters that may potentially be passed by
       * reference, and thus only the first two may change the value
       * of OUT_PARAMETERS.
       */

      switch (*type)
	{
	  case _C_ID:
	    if (flags & _F_BYCOPY)
	      {
		[coder encodeBycopyObject: *(id*)datum];
	      }
#ifdef	_F_BYREF
	    else if (flags & _F_BYREF)
	      {
		[coder encodeByrefObject: *(id*)datum];
	      }
#endif
	    else
	      {
		[coder encodeObject: *(id*)datum];
	      }
	    break;
	  case _C_CHARPTR:
	    /*
	     * Handle a (char*) argument.
	     * If the char* is qualified as an OUT parameter, or if it
	     * not explicitly qualified as an IN parameter, then we will
	     * have to get this char* again after the method is run,
	     * because the method may have changed it.  Set
	     * OUT_PARAMETERS accordingly.
	     */
	    if ((flags & _F_OUT) || !(flags & _F_IN))
	      {
		out_parameters = YES;
	      }
	    /*
	     * If the char* is qualified as an IN parameter, or not
	     * explicity qualified as an OUT parameter, then encode
	     * it.
	     */
	    if ((flags & _F_IN) || !(flags & _F_OUT))
	      {
		[coder encodeValueOfObjCType: type at: datum];
	      }
	    break;

	  case _C_PTR:
	    /*
	     * If the pointer's value is qualified as an OUT parameter,
	     * or if it not explicitly qualified as an IN parameter,
	     * then we will have to get the value pointed to again after
	     * the method is run, because the method may have changed
	     * it.  Set OUT_PARAMETERS accordingly.
	     */
	    if ((flags & _F_OUT) || !(flags & _F_IN))
	      {
		out_parameters = YES;
	      }
	    if (passp)
	      {
		if ((flags & _F_IN) || !(flags & _F_OUT))
		  {
		    [coder encodeValueOfObjCType: type at: datum];
		  }
	      }
	    else
	      {
		/*
		 * Handle an argument that is a pointer to a non-char.  But
		 * (void*) and (anything**) is not allowed.
		 * The argument is a pointer to something; increment TYPE
		 * so we can see what it is a pointer to.
		 */
		type++;
		/*
		 * If the pointer's value is qualified as an IN parameter,
		 * or not explicity qualified as an OUT parameter, then
		 * encode it.
		 */
		if ((flags & _F_IN) || !(flags & _F_OUT))
		  {
		    [coder encodeValueOfObjCType: type at: *(void**)datum];
		  }
	      }
	    break;

	  case _C_STRUCT_B:
	  case _C_UNION_B:
	  case _C_ARY_B:
	    /*
	     * Handle struct and array arguments.
	     * Whether DATUM points to the data, or points to a pointer
	     * that points to the data, depends on the value of
	     * CALLFRAME_STRUCT_BYREF.  Do the right thing
	     * so that ENCODER gets a pointer to directly to the data.
	     */
	    [coder encodeValueOfObjCType: type at: datum];
	    break;

	  default:
	    /* Handle arguments of all other types. */
	    [coder encodeValueOfObjCType: type at: datum];
	}
    }

  /*
   * Return a BOOL indicating whether or not there are parameters that
   * were passed by reference; we will need to get those values again
   * after the method has finished executing because the execution of
   * the method may have changed them.
   */
  return out_parameters;
}

@end

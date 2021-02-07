/** Implementation of NSInvocation for GNUStep
   Copyright (C) 1998,2003 Free Software Foundation, Inc.

   Author:     Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: August 1998
   Based on code by: Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>

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

   <title>NSInvocation class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#define	EXPOSE_NSInvocation_IVARS	1
#import "Foundation/NSException.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSData.h"
#import "Foundation/NSInvocation.h"
#import "Foundation/NSZone.h"
#import "GSInvocation.h"
#import "GSPrivate.h"

#if defined(USE_LIBFFI)
#include "cifframe.h"
#elif defined(USE_FFCALL)
#include "callframe.h"
#endif

#if     defined(HAVE_SYS_MMAN_H)
#include <sys/mman.h>
#endif

#if     defined(HAVE_MMAP)
#  if   !defined(MAP_ANONYMOUS)
#    if defined(MAP_ANON)
#      define MAP_ANONYMOUS   MAP_ANON
#    else
#      undef  HAVE_MMAP
#    endif
#  endif
#endif

@implementation GSCodeBuffer

+ (GSCodeBuffer*) memoryWithSize: (NSUInteger)_size
{
  return [[[self alloc] initWithSize: _size] autorelease];
}

- (void*) buffer
{
  return buffer;
}

- (void) dealloc
{
  DESTROY(frame);
  if (size > 0)
    {
#if	defined(HAVE_FFI_PREP_CLOSURE_LOC)
      ffi_closure_free(buffer);
#else
#if     defined(HAVE_MMAP)
      munmap(buffer, size);
#else
#if     !defined(_WIN32) && defined(HAVE_MPROTECT)
      if (mprotect(buffer, NSPageSize(), PROT_READ|PROT_WRITE) == -1)
	{
	  NSLog(@"Failed to protect memory as writable: %@", [NSError _last]);
	}
#endif
      NSDeallocateMemoryPages(buffer, NSPageSize());
#endif
#endif
      buffer = 0;
      executable = 0;
      size = 0;
    }
  [super dealloc];
}

- (void*) executable
{
  return executable;
}

- (id) initWithSize: (NSUInteger)_size
{
  NSAssert(_size > 0, @"Tried to allocate zero length buffer.");
  NSAssert(_size <= NSPageSize(), @"Tried to allocate more than one page.");
#if	defined(HAVE_FFI_PREP_CLOSURE_LOC)
  buffer = ffi_closure_alloc(_size, &executable);
  if (0 == buffer)
    {
      executable = 0;
    }  
  else
    {
      size = _size;
    }
#else
#if     defined(HAVE_MMAP)
#if     defined(HAVE_MPROTECT)
  /* We have mprotect, so we create memory as writable and change it to
   * executable later (writable and executable may not be possible at
   * the same time).
   */
  buffer = mmap (NULL, _size, PROT_READ|PROT_WRITE,
    MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#else
  /* We do not have mprotect, so we have to try to create writable and
   * executable memory.
   */
  buffer = mmap (NULL, _size, PROT_READ|PROT_WRITE|PROT_EXEC,
    MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#endif  /* HAVE_MPROTECT */
  if (buffer == (void*)-1) buffer = (void*)0;
#else
  buffer = NSAllocateMemoryPages(NSPageSize());
#endif  /* HAVE_MMAP */

  if (buffer == (void*)0)
    {
      NSLog(@"Failed to map %"PRIuPTR
	" bytes for execute: %@", _size, [NSError _last]);
      buffer = 0;
      executable = 0;
      [self dealloc];
      self = nil;
    }
  else
    {
      executable = buffer;
      size = _size;
    }
#endif	/* USE_LIBFFI */
  return self;
}

/* Ensure that the protection on the buffer is such that it will execute
 * on any architecture.
 */
- (void) protect
{
#if	!defined(HAVE_FFI_PREP_CLOSURE_LOC)
#if	defined(_WIN32)
  DWORD old;
  if (VirtualProtect(buffer, size, PAGE_EXECUTE, &old) == 0)
    {
      NSLog(@"Failed to protect memory as executable: %@", [NSError _last]);
    }
#elif     defined(HAVE_MPROTECT)
  if (mprotect(buffer, NSPageSize(), PROT_READ|PROT_EXEC) == -1)
    {
      NSLog(@"Failed to protect memory as executable: %@", [NSError _last]);
    }
#endif
#endif
}

- (void) setFrame: (id)aFrame
{
  ASSIGN(frame, aFrame);
}
@end

static Class   NSInvocation_abstract_class;
static Class   NSInvocation_concrete_class;




GS_ROOT_CLASS
@interface GSInvocationProxy
{
@public
  Class		isa;
  id		target;
  NSInvocation	*invocation;
}
+ (id) _newWithTarget: (id)t;
- (NSInvocation*) _invocation;
- (void) forwardInvocation: (NSInvocation*)anInvocation;
- (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector;
@end
@interface GSMessageProxy : GSInvocationProxy
@end

#define	_inf	((NSArgumentInfo*)_info)

/**
 * <p>The <code>NSInvocation</code> class implements a mechanism of constructing
 * messages (as <code>NSInvocation</code> instances), sending these to other
 * objects, and handling the returned values.
 * </p>
 * <p>An <code>NSInvocation</code> object may contain a target object to which a
 * message can be sent, or may send the message to an arbitrary object.<br />
 * Each message consists of a selector for that method and an argument
 * list.  Once the message has been sent, the invocation will contain
 * a return value whose contents may be copied out of it.
 * </p>
 * <p>The target, selector, and arguments of an instance be constructed
 * dynamically, providing a great deal of power/flexibility.
 * </p>
 * <p>The sending of the message to the target object (using the -invoke
 * or -invokeWithTarget: method) can be done at any time, but a standard
 * use of this is by the [NSObject-forwardInvocation:] method which is
 * called whenever a method is not implemented by the class of the
 * object to which it was sent.
 * </p>
 * <p>Related to the class are two convenience macros ... NS_MESSAGE()
 * and NS_INVOCATION() ... to allow easy construction of invocations
 * with all the arguments set up.
 * </p>
 */
@implementation NSInvocation

#ifdef USE_LIBFFI
static inline void
_get_arg(NSInvocation *inv, int index, void *buffer)
{
  cifframe_get_arg((cifframe_t *)inv->_cframe, index, buffer,
		   ((NSArgumentInfo*)inv->_info)[index+1].size);
}

static inline void
_set_arg(NSInvocation *inv, int index, void *buffer)
{
  cifframe_set_arg((cifframe_t *)inv->_cframe, index, buffer,
		   ((NSArgumentInfo*)inv->_info)[index+1].size);
}

static inline void *
_arg_addr(NSInvocation *inv, int index)
{
  return cifframe_arg_addr((cifframe_t *)inv->_cframe, index);
}

#elif defined(USE_FFCALL)
static inline void
_get_arg(NSInvocation *inv, int index, void *buffer)
{
  callframe_get_arg((callframe_t *)inv->_cframe, index, buffer,
		    ((NSArgumentInfo*)inv->_info)[index+1].size);
}

static inline void
_set_arg(NSInvocation *inv, int index, void *buffer)
{
  callframe_set_arg((callframe_t *)inv->_cframe, index, buffer,
		    ((NSArgumentInfo*)inv->_info)[index+1].size);
}

static inline void *
_arg_addr(NSInvocation *inv, int index)
{
  return callframe_arg_addr((callframe_t *)inv->_cframe, index);
}

#else

static inline void
_get_arg(NSInvocation *inv, int index, void *buffer)
{
}

static inline void
_set_arg(NSInvocation *inv, int index, void *buffer)
{
}

static inline void *
_arg_addr(NSInvocation *inv, int index)
{
  return 0;
}

#endif

+ (id) allocWithZone: (NSZone*)aZone
{
  if (self == NSInvocation_abstract_class)
    {
      return NSAllocateObject(NSInvocation_concrete_class, 0, aZone);
    }
  else
    {
      return NSAllocateObject(self, 0, aZone);
    }
}

+ (void) initialize
{
  if (self == [NSInvocation class])
    {
      NSInvocation_abstract_class = self;
#if defined(USE_LIBFFI)
      NSInvocation_concrete_class = [GSFFIInvocation class];
#elif defined(USE_FFCALL)
      NSInvocation_concrete_class = [GSFFCallInvocation class];
#else
      NSInvocation_concrete_class = [GSDummyInvocation class];
#endif
    }
}

/**
 * Returns an invocation instance which can be used to send messages to
 * a target object using the described signature.<br />
 * You must set the target and selector (using -setTarget: and -setSelector:)
 * before you attempt to use the invocation.<br />
 * Raises an NSInvalidArgumentException if the signature is nil.
 */
+ (NSInvocation*) invocationWithMethodSignature: (NSMethodSignature*)_signature
{
  return AUTORELEASE([[NSInvocation_concrete_class alloc]
    initWithMethodSignature: _signature]);
}

- (void) dealloc
{
  if (_targetRetained)
    {
      _targetRetained = NO;
      RELEASE(_target);
    }
  if (_argsRetained)
    {
      _argsRetained = NO;
      if (_cframe && _sig)
	{
	  unsigned int	i;

	  for (i = 3; i <= _numArgs; i++)
	    {
	      if (*_inf[i].type == _C_CHARPTR)
		{
		  char	*str = 0;

		  _get_arg(self, i-1, &str);
		  if (str != 0)
		    {
		      NSZoneFree(NSDefaultMallocZone(), str);
		    }
		}
	      else if (*_inf[i].type == _C_ID)
		{
		  id	obj = nil;

		  _get_arg(self, i-1, &obj);
		  RELEASE(obj);
		}
	    }
	}
    }


  CLEAR_RETURN_VALUE_IF_OBJECT;

#if	defined(USE_LIBFFI)
  if (_cframe)
    {
      /* If we get here then we are not using GC, so the _frame instance
       * variable points to a mutable data object containing _cframe and
       * we can release it.
       */
      [((GSFFIInvocation*)self)->_frame release];
    }
#elif defined(USE_FFCALL)
  if (_cframe)
    {
      NSZoneFree(NSDefaultMallocZone(), _cframe);
    }
#endif
  if (_retptr)
    {
      NSZoneFree(NSDefaultMallocZone(), _retptr);
    }
  RELEASE(_sig);
  [super dealloc];
}

/**
 * Copies the argument identified by index into the memory location specified
 * by the buffer argument.<br />
 * An index of zero is the target object, an index of one is the selector,
 * so the actual method arguments start at index 2.
 */
- (void) getArgument: (void*)buffer
	     atIndex: (NSInteger)index
{
  if ((NSUInteger)index >= _numArgs)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"bad invocation argument index"];
    }
  if (index == 0)
    {
      *(id*)buffer = _target;
    }
  else if (index == 1)
    {
      *(SEL*)buffer = _selector;
    }
  else
    {
      _get_arg(self, index, buffer);
    }		
}

/**
 * Copies the invocations return value to the location pointed to by buffer
 * if a return value has been set (see the -setReturnValue: method).<br />
 * If there isn't a return value then this method raises an exception.
 */
- (void) getReturnValue: (void*)buffer
{
  if (_validReturn == NO)
    {
      [NSException raise: NSGenericException
		  format: @"getReturnValue with no value set"];
    }
  if (*_inf[0].type != _C_VOID)
    {
      memcpy(buffer, _retval, _inf[0].size);
    }
}

/**
 * Returns the selector of the invocation (the argument at index 1)
 */
- (SEL) selector
{
  return _selector;
}

/**
 * Sets the argument identified by index from the memory location specified
 * by the buffer argument.<br />
 * Using an index of 0 is equivalent to calling -setTarget: and using an
 * argument of 1 is equivalent to -setSelector:<br />
 * Proper arguments start at index 2.<br />
 * NB. Unlike -setTarget: and -setSelector: the value of buffer must be
 * <em>a pointer to</em> the argument to be set in the invocation.<br />
 * If -retainArguments was called, then any object argument set in the
 * receiver is retained by it.
 */
- (void) setArgument: (void*)buffer
	     atIndex: (NSInteger)index
{
  if ((NSUInteger)index >= _numArgs)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"bad invocation argument index"];
    }
  if (index == 0)
    {
      [self setTarget: *(id*)buffer];
    }
  else if (index == 1)
    {
      [self setSelector: *(SEL*)buffer];
    }
  else
    {
      int		i = index+1;	/* Allow for return type in '_inf' */
      const char	*type = _inf[i].type;

      if (_argsRetained && (*type == _C_ID || *type == _C_CHARPTR))
	{
	  if (*type == _C_ID)
	    {
	      id	old;

	      _get_arg(self, index, &old);
	      _set_arg(self, index, buffer);
	      IF_NO_GC(RETAIN(*(id*)buffer));
	      if (old != nil)
		{
		  RELEASE(old);
		}
	    }
	  else
	    {
	      char	*oldstr;
	      char	*newstr = *(char**)buffer;

	      _get_arg(self, index, &oldstr);
	      if (newstr == 0)
		{
		  _set_arg(self, index, buffer);
		}
	      else
		{
		  int	len;
		  char	*tmp;

		  len = strlen(newstr);
		  tmp = NSZoneMalloc(NSDefaultMallocZone(), len + 1);
		  strncpy(tmp, newstr, len);
		  tmp[len] = '\0';
		  _set_arg(self, index, tmp);
		}
	      if (oldstr != 0)
		{
		  NSZoneFree(NSDefaultMallocZone(), oldstr);
		}
	    }
	}
      else
	{
	  _set_arg(self, index, buffer);
	}
    }		
}

/**
 * Sets the return value of the invocation to the item that buffer points to.
 */
- (void) setReturnValue: (void*)buffer
{
  const char	*type;

  type = _inf[0].type;

  CLEAR_RETURN_VALUE_IF_OBJECT;

  if (*type != _C_VOID)
    {
      memcpy(_retval, buffer, _inf[0].size);
    }

  RETAIN_RETURN_VALUE;
  _validReturn = YES;
}

/**
 * Sets the selector for the invocation.
 */
- (void) setSelector: (SEL)aSelector
{
  _selector = aSelector;
}

/**
 * Sets the target object for the invocation.<br />
 * If -retainArguments was called, then the target is retained.
 */
- (void) setTarget: (id)anObject
{
  if (_targetRetained)
    {
      ASSIGN(_target, anObject);
    }
  else
    {
      _target = anObject;
    }
}

/**
 * Returns the target object of the invocation.
 */
- (id) target
{
  return _target;
}

/**
 * Returns a flag to indicate whether object arguments of the invocation
 * (including its target) are retained by the invocation.
 */
- (BOOL) argumentsRetained
{
  return _argsRetained;
}

/**
 * Instructs the invocation to retain its object arguments (including the
 * target). The default is not to retain them.
 */
- (void) retainArguments
{
  [self retainArgumentsIncludingTarget: YES];
}

/**
 * Returns YES if target has been retained yet, NO otherwise.
 */
- (BOOL) targetRetained
{
  return _targetRetained;
}

/**
 * Similar to -[NSInvocation retainArguments], but allows the sender to
 * explicitly control whether the target is retained as well. Retaining
 * the target is sometimes not desirable (such as in NSUndoManager), as
 * retain loops could result.
 */
- (void) retainArgumentsIncludingTarget: (BOOL)retainTargetFlag
{
  if (_argsRetained == NO)
    {
      unsigned int	i;

      _argsRetained = YES;
      if (_cframe == 0)
	{
	  return;
	}
      for (i = 3; i <= _numArgs; i++)
	{
	  if (*_inf[i].type == _C_ID)
	    {
              id        old;

	      _get_arg(self, i-1, &old);
	      if (old != nil)
		{
		  IF_NO_GC(RETAIN(old));
		}
            }
	  else if (*_inf[i].type == _C_CHARPTR)
	    {
	      char      *str;

	      _get_arg(self, i-1, &str);
	      if (str != 0)
	        {
		  char  *tmp;
		  int	len;

		  len = strlen(str);
		  tmp = NSZoneMalloc(NSDefaultMallocZone(), len + 1);
		  strncpy(tmp, str, len);
		  tmp[len] = '\0';
		  _set_arg(self, i-1, &tmp);
		}
	    }
	}
    }

  if (retainTargetFlag && _targetRetained == NO)
    {
      _targetRetained = YES;

      IF_NO_GC(RETAIN(_target));
    }
}

/**
 * Sends the message encapsulated in the invocation to its target.
 */
- (void) invoke
{
  [self invokeWithTarget: _target];
}

/**
 * Sends the message encapsulated in the invocation to anObject.
 */
- (void) invokeWithTarget: (id)anObject
{
  [self subclassResponsibility: _cmd];
}

/**
 * Returns the method signature of the invocation.
 */
- (NSMethodSignature*) methodSignature
{
  return _sig;
}

- (NSString*) description
{
  /*
   *	Don't use -[NSString stringWithFormat:] method because it can cause
   *	infinite recursion.
   */
  char buffer[1024];

  snprintf (buffer, 1024, "<%s %p selector: %s target: %s>", \
    GSClassNameFromObject(self), \
    self, \
    _selector ? sel_getName(_selector) : "nil", \
    _target ? class_getName([_target class]) : "nil" \
   );

  return [NSString stringWithUTF8String: buffer];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  const char	*types = [_sig methodType];
  unsigned int	i;

  [aCoder encodeValueOfObjCType: @encode(char*)
			     at: &types];

  [aCoder encodeObject: _target];

  [aCoder encodeValueOfObjCType: _inf[2].type
			     at: &_selector];

  for (i = 3; i <= _numArgs; i++)
    {
      const char	*type = _inf[i].type;
      void		*datum;

      datum = _arg_addr(self, i-1);

      if (*type == _C_ID)
	{
	  [aCoder encodeObject: *(id*)datum];
	}
      else
	{
	  [aCoder encodeValueOfObjCType: type at: datum];
	}
    }
  if (*_inf[0].type != _C_VOID)
    {
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_validReturn];
      if (_validReturn)
	{
	  [aCoder encodeValueOfObjCType: _inf[0].type at: _retval];
	}
    }
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  NSMethodSignature	*newSig;
  const char		*types;
  void			*datum;
  unsigned int		i;

  [aCoder decodeValueOfObjCType: @encode(char*) at: &types];
  newSig = [NSMethodSignature signatureWithObjCTypes: types];
  NSZoneFree(NSDefaultMallocZone(), (void*)types);

  DESTROY(self);
  self = RETAIN([NSInvocation invocationWithMethodSignature: newSig]);

  [aCoder decodeValueOfObjCType: @encode(id) at: &_target];

  [aCoder decodeValueOfObjCType: @encode(SEL) at: &_selector];

  for (i = 3; i <= _numArgs; i++)
    {
      datum = _arg_addr(self, i-1);
      [aCoder decodeValueOfObjCType: _inf[i].type at: datum];
    }
  _argsRetained = YES;
  if (*_inf[0].type != _C_VOID)
    {
      [aCoder decodeValueOfObjCType: @encode(BOOL) at: &_validReturn];
      if (_validReturn)
        {
          [aCoder decodeValueOfObjCType: _inf[0].type at: _retval];
        }
    }
  return self;
}

@end

/**
 * Provides some minor extensions and some utility methods to aid
 * integration of <code>NSInvocation</code> with the Objective-C runtime.
 */
@implementation NSInvocation (GNUstep)

- (BOOL) sendsToSuper
{
  return _sendToSuper;
}

- (void) setSendsToSuper: (BOOL)flag
{
  _sendToSuper = flag;
}
@end

/**
 * These methods are for internal use only ... not public API<br />
 * They are used by the NS_INVOCATION() and NS_MESSAGE() macros to help
 * create invocations.
 */
@implementation NSInvocation (MacroSetup)

/** <init /><override-subclass />
 * Initialised an invocation instance which can be used to send messages to
 * a target object using aSignature.<br />
 * You must set the target and selector (using -setTarget: and -setSelector:)
 * before you attempt to use the invocation.<br />
 * Raises an NSInvalidArgumentException if aSignature is nil.
 */
- (id) initWithMethodSignature: (NSMethodSignature*)aSignature
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/**
 * Internal use.
 */
+ (id) _newProxyForInvocation: (id)target
{
  return (id)[GSInvocationProxy _newWithTarget: target];
}
+ (id) _newProxyForMessage: (id)target
{
  return (id)[GSMessageProxy _newWithTarget: target];
}
+ (NSInvocation*) _returnInvocationAndDestroyProxy: (id)proxy
{
  NSInvocation  *inv = [proxy _invocation];
  NSDeallocateObject(proxy);
  return inv;
}
@end

@implementation NSInvocation (BackwardCompatibility)

- (void) invokeWithObject: (id)obj
{
  [self invokeWithTarget: (id)obj];
}

@end

#if !defined(USE_FFCALL) && !defined(USE_LIBFFI)
#warning Using dummy NSInvocation implementation.  It is strongly recommended that you use libffi.
@implementation GSDummyInvocation

/*
 *	This is the de_signated initialiser.
 */
- (id) initWithMethodSignature: (NSMethodSignature*)aSignature
{
  return self;
}

- (void) invokeWithTarget: (id)anObject
{
  CLEAR_RETURN_VALUE_IF_OBJECT;
  _validReturn = NO;
  /*
   *	A message to a nil object returns nil.
   */
  if (anObject == nil)
    {
      _validReturn = YES;
      return;
    }
}

@end
#endif

@implementation	GSInvocationProxy
+ (id) _newWithTarget: (id)t
{
  GSInvocationProxy	*o;
  o = (GSInvocationProxy*) NSAllocateObject(self, 0, NSDefaultMallocZone());
  o->target = RETAIN(t);
  return o;
}
- (NSInvocation*) _invocation
{
  return invocation;
}
- (void) forwardInvocation: (NSInvocation*)anInvocation
{
  invocation = anInvocation;
}
- (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector
{
  return [target methodSignatureForSelector: aSelector];
}
@end

@implementation	GSMessageProxy
- (NSInvocation*) _invocation
{
  [invocation setTarget: target];
  return invocation;
}
@end


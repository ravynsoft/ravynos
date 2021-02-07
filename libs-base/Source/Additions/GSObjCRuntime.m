/** Implementation of ObjC runtime additions for GNUStep
   Copyright (C) 1995-2010 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Date: Aug 1995
   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: Nov 2002
   Written by:  Manuel Guesdon <mguesdon@orange-concept.com>
   Date: Nov 2002

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

   <title>GSObjCRuntime function and macro reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#ifndef NeXT_Foundation_LIBRARY
#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSInvocation.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSMethodSignature.h"
#import "Foundation/NSNull.h"
#import "Foundation/NSSet.h"
#import "Foundation/NSValue.h"
#endif
#import "GNUstepBase/GSObjCRuntime.h"
#import "GNUstepBase/NSObject+GNUstepBase.h"

#import "../GSPrivate.h"

#include <objc/Protocol.h>

#include <stdio.h>
#include <ctype.h>

#ifndef NeXT_RUNTIME
#include <pthread.h>
#endif
#ifdef __GNUSTEP_RUNTIME__
extern struct objc_slot	*objc_get_slot(Class, SEL);
#endif

#ifdef NeXT_Foundation_LIBRARY
@interface NSObject (MissingFromMacOSX)
+ (IMP) methodForSelector: (SEL)aSelector;
@end
#endif

#define BDBGPrintf(format, args...) \
  do { if (behavior_debug) { fprintf(stderr, (format) , ## args); } } while (0)

Class
GSObjCClass(id obj)
{
  return object_getClass(obj);
}
Class GSObjCSuper(Class cls)
{
  return class_getSuperclass(cls);
}
BOOL
GSObjCIsInstance(id obj)
{
  Class	c = object_getClass(obj);

  if (c != Nil && class_isMetaClass(c) == NO)
    return YES;
  else
    return NO;
}
BOOL
GSObjCIsClass(Class cls)
{
  if (class_isMetaClass(object_getClass(cls)))
    return YES; 
  else
    return NO;
}
BOOL
GSObjCIsKindOf(Class cls, Class other)
{
  while (cls != Nil)
    {
      if (cls == other)
	{
	  return YES;
	}
      cls = class_getSuperclass(cls);
    }
  return NO;
}
Class
GSClassFromName(const char *name)
{
  return objc_lookUpClass(name);
}
const char *
GSNameFromClass(Class cls)
{
  return class_getName(cls);
}
const char *
GSClassNameFromObject(id obj)
{
  return class_getName(object_getClass(obj));
}
const char *
GSNameFromSelector(SEL sel)
{
  return sel_getName(sel);
}
SEL
GSSelectorFromName(const char *name)
{
  return sel_getUid(name);
}

#if defined (__GNU_LIBOBJC__) && (__GNU_LIBOBJC__ < 20110608)
/* Don't use sel_registerTypedName() ... it's broken when first introduced
 * into gcc (fails to correctly check for multiple registrations with same
 * types but different layout info).
 * Later versions of the runtime should be OK though.
 * Hack - need to provide these function declarations
 * for gcc 4.6 libobjc. They're called below, and they're declared 
 * in objc-api.h, but we're using runtime.h, so objc-api.h can't be imported.
 */
SEL sel_get_any_typed_uid(const char *name);
SEL sel_get_typed_uid(const char *name, const char*);
SEL sel_register_name(const char *name);
SEL sel_register_typed_name(const char *name, const char*type);
#endif

SEL
GSSelectorFromNameAndTypes(const char *name, const char *types)
{
#if NeXT_RUNTIME
  return sel_getUid(name);
#elif defined (__GNU_LIBOBJC__) && (__GNU_LIBOBJC__ >= 20110608)
  return sel_registerTypedName(name, types);
#elif defined (__GNUSTEP_RUNTIME__)
  return sel_registerTypedName_np(name, types);
#else
extern SEL sel_get_any_typed_uid(const char*);
extern SEL sel_get_typed_uid(const char*, const char*);
extern SEL sel_register_name(const char*);
extern SEL sel_register_typed_name(const char*, const char*);

  if (name == 0)
    {
      return 0;
    }
  else
    {
      SEL s;

      if (types == 0)
	{
	  s = sel_get_any_typed_uid(name);
	}
      else
	{
	  s = sel_get_typed_uid(name, types);
	}
      if (s == 0)
	{
	  if (types == 0)
	    {
	      s = sel_register_name(name);
	    }
	  else
	    {
	      s = sel_register_typed_name(name, types);
	    }
	}
      return s;
    }
#endif
}

const char *
GSTypesFromSelector(SEL sel)
{
#if NeXT_RUNTIME
  return 0;
#elif defined (__GNU_LIBOBJC__)
  return sel_getTypeEncoding(sel);
#elif defined (__GNUSTEP_RUNTIME__)
  return sel_getType_np(sel);
#else
  if (sel == 0)
    {
      return 0;
    }
  else
    {
      return sel_get_type(sel);
    }
#endif
}

void
GSFlushMethodCacheForClass (Class cls)
{
  return;
}
int
GSObjCVersion(Class cls)
{
  return class_getVersion(cls);
}


/**
 * This function is used to locate information about the instance
 * variable of obj called name.  It returns YES if the variable
 * was found, NO otherwise.  If it returns YES, then the values
 * pointed to by type, size, and offset will be set (except where
 * they are null pointers).
 */
BOOL
GSObjCFindVariable(id obj, const char *name,
  const char **type, unsigned int *size, int *offset)
{
  Class		class = object_getClass(obj);
  Ivar		ivar = class_getInstanceVariable(class, name);

  if (ivar == 0)
    {
      return NO;
    }
  else
    {
      const char	*enc = ivar_getTypeEncoding(ivar);

      if (type != 0)
	{
	  *type = enc;
	}
      if (size != 0)
	{
	  NSUInteger	s;
	  NSUInteger	a;

	  NSGetSizeAndAlignment(enc, &s, &a);
	  *size = s;
	}
      if (offset != 0)
	{
	  *offset = ivar_getOffset(ivar);
	}
      return YES;
    }
}

/**
 * This method returns an array listing the names of all the
 * instance methods available to obj, whether they
 * belong to the class of obj or one of its superclasses.<br />
 * If obj is a class, this returns the class methods.<br />
 * Returns nil if obj is nil.
 */
NSArray *
GSObjCMethodNames(id obj, BOOL recurse)
{
  NSMutableSet	*set;
  NSArray	*array;
  Class		 class;

  if (obj == nil)
    {
      return nil;
    }
  /*
   * Add names to a set so methods declared in superclasses
   * and then overridden do not appear more than once.
   */
  set = [[NSMutableSet alloc] initWithCapacity: 32];

  class = object_getClass(obj);

  while (class != Nil)
    {
      unsigned	count;
      Method	*meth = class_copyMethodList(class, &count);

      while (count-- > 0)
	{
	  NSString	*name;

	  name = [[NSString alloc] initWithFormat: @"%s",
	    sel_getName(method_getName(meth[count]))];
	  [set addObject: name];
	  [name release];
	}
      if (meth != NULL)
	{
          free(meth);
        }
      if (NO == recurse)
	{
	  break;
	}
      class = class_getSuperclass(class);
    }

  array = [set allObjects];
  RELEASE(set);
  return array;
}

/**
 * This method returns an array listing the names of all the
 * instance variables present in the instance obj, whether they
 * belong to the class of obj or one of its superclasses.<br />
 * Returns nil if obj is nil.
 */
NSArray *
GSObjCVariableNames(id obj, BOOL recurse)
{
  NSMutableSet	*set;
  NSArray	*array;
  Class		 class;

  if (obj == nil)
    {
      return nil;
    }
  /*
   * Add names to a set so methods declared in superclasses
   * and then overridden do not appear more than once.
   */
  set = [[NSMutableSet alloc] initWithCapacity: 32];

  class = object_getClass(obj);

  while (class != Nil)
    {
      unsigned	count;
      Ivar	*ivar = class_copyIvarList(class, &count);

      while (count-- > 0)
	{
	  NSString	*name;

	  name = [[NSString alloc] initWithFormat: @"%s",
	    ivar_getName(ivar[count])];
	  [set addObject: name];
	  [name release];
	}
      if (ivar != NULL)
	{
          free(ivar);
	}
      if (NO == recurse)
	{
	  break;
	}
      class = class_getSuperclass(class);
    }

  array = [set allObjects];
  RELEASE(set);
  return array;
}

/**
 * Gets the value from an instance variable in obj<br />
 * This function performs no checking ... you should use it only where
 * you are providing information from a call to GSObjCFindVariable()
 * and you know that the data area provided is the correct size.
 */
void
GSObjCGetVariable(id obj, int offset, unsigned int size, void *data)
{
  memcpy(data, ((void*)obj) + offset, size);
}

/**
 * Sets the value in an instance variable in obj<br />
 * This function performs no checking ... you should use it only where
 * you are providing information from a call to GSObjCFindVariable()
 * and you know that the data area provided is the correct size.
 */
void
GSObjCSetVariable(id obj, int offset, unsigned int size, const void *data)
{
  memcpy(((void*)obj) + offset, data, size);
}

GS_EXPORT unsigned int
GSClassList(Class *buffer, unsigned int max, BOOL clearCache)
{
  int num;

  if (buffer != NULL)
    {
      memset(buffer, 0, sizeof(Class) * (max + 1));
    }

  num = objc_getClassList(buffer, max);
  num = (num < 0) ? 0 : num;
  return num;
}

/** references:
http://www.macdevcenter.com/pub/a/mac/2002/05/31/runtime_parttwo.html?page=1
http://developer.apple.com/documentation/Cocoa/Conceptual/ObjectiveC/9objc_runtime_reference/chapter_5_section_1.html
http://developer.apple.com/documentation/Cocoa/Conceptual/ObjectiveC/9objc_runtime_reference/chapter_5_section_21.html
ObjcRuntimeUtilities.m by Nicola Pero
**/

/**
 * <p>Create a Class structure for use by the ObjectiveC runtime and return
 * an NSValue object pointing to it.  The class will not be added to the
 * runtime (you must do that later using the GSObjCAddClasses() function).
 * </p>
 * <p>The iVars dictionary lists the instance variable names and their types.
 * </p>
 */
NSValue *
GSObjCMakeClass(NSString *name, NSString *superName, NSDictionary *iVars)
{
  Class		newClass;
  Class		classSuperClass;
  const char	*classNameCString;

  NSCAssert(name, @"no name");
  NSCAssert(superName, @"no superName");

  classSuperClass = NSClassFromString(superName);

  NSCAssert1(classSuperClass, @"No class named %@",superName);
  NSCAssert1(!NSClassFromString(name), @"A class %@ already exists", name);

  classNameCString = [name UTF8String];
  newClass = objc_allocateClassPair(classSuperClass, classNameCString, 0);
  if ([iVars count] > 0)
    {
      NSEnumerator	*enumerator = [iVars keyEnumerator];
      NSString		*key;

      while ((key = [enumerator nextObject]) != nil)
        {
          const	char	*iVarName = [key UTF8String];
          const char	*iVarType = [[iVars objectForKey: key] UTF8String];
	  uint8_t	iVarAlign = 0;
	  size_t	iVarSize;
	  NSUInteger	s;
	  NSUInteger	a;

	  NSGetSizeAndAlignment(iVarType, &s, &a);
	  // Convert size to number of bitshifts needed for alignment.
	  iVarSize = 1;
	  while (iVarSize < s)
	    {
	      iVarSize <<= 1;
	      iVarAlign++;
	    }
	  // Record actual size
	  iVarSize = s;
	  if (NO
	    == class_addIvar(newClass, iVarName, iVarSize, iVarAlign, iVarType))
	    {
	      NSLog(@"Error adding ivar '%s' of type '%s'", iVarName, iVarType);
	    }
	}
    }

  return [NSValue valueWithPointer: newClass];
}

/**
 * The classes argument is an array of NSValue objects containing pointers
 * to classes previously created by the GSObjCMakeClass() function.
 */
void
GSObjCAddClasses(NSArray *classes)
{
  NSUInteger	numClasses = [classes count];
  NSUInteger	i;

  for (i = 0; i < numClasses; i++)
    {
      objc_registerClassPair((Class)[[classes objectAtIndex: i] pointerValue]);
    }
}



static BOOL behavior_debug = NO;

BOOL
GSObjCBehaviorDebug(int setget)
{
  BOOL	old = behavior_debug;

  if (setget == 1)
    {
      behavior_debug = YES;
    }
  else if (setget == 0)
    {
      behavior_debug = NO;
    }
  return old;
}

void
GSObjCAddMethods(Class cls, Method *list, BOOL replace)
{
  unsigned int	index = 0;
  char		c;
  Method	m;

  if (cls == 0 || list == 0)
    {
      return;
    }
  c = class_isMetaClass(cls) ? '+' : '-';

  while ((m = list[index++]) != NULL)
    {
      SEL		n = method_getName(m);
      IMP		i = method_getImplementation(m);
      const char	*t = method_getTypeEncoding(m);

      /* This will override a superclass method but will not replace a
       * method which already exists in the class itsself.
       */
      if (YES == class_addMethod(cls, n, i, t))
	{
          BDBGPrintf("    added %c%s\n", c, sel_getName(n));
	}
      else if (YES == replace)
	{
	  /* If we want to replace an existing implemetation ...
	   */
	  method_setImplementation(class_getInstanceMethod(cls, n), i);
          BDBGPrintf("    replaced %c%s\n", c, sel_getName(n));
	} 
      else
	{
          BDBGPrintf("    skipped %c%s\n", c, sel_getName(n));
	}
    }
}

GSMethod
GSGetMethod(Class cls, SEL sel,
  BOOL searchInstanceMethods,
  BOOL searchSuperClasses)
{
  if (cls == 0 || sel == 0)
    {
      return 0;
    }

  if (searchSuperClasses == NO)
    {
      unsigned int	count;
      Method		method = NULL;
      Method		*methods;

      if (searchInstanceMethods == NO)
	{
	  methods = class_copyMethodList(object_getClass(cls), &count);
	}
      else
	{
	  methods = class_copyMethodList(cls, &count);
	}
      if (methods != NULL)
	{
	  unsigned int	index = 0;

	  while ((method = methods[index++]) != NULL)
	    {
	      if (sel_isEqual(sel, method_getName(method)))
		{
		  break;
		}
	    }
	  free(methods);
	}
      return method;
    }
  else
    {
      if (searchInstanceMethods == NO)
	{
	  return class_getClassMethod(cls, sel);
	}
      else
	{
	  return class_getInstanceMethod(cls, sel);
	}
    }
}


GS_EXPORT const char *
GSSkipTypeQualifierAndLayoutInfo(const char *types)
{
  while (*types == '+'
    || *types == '-'
    || *types == _C_CONST
    || *types == _C_IN
    || *types == _C_INOUT
    || *types == _C_OUT
    || *types == _C_BYCOPY
    || *types == _C_BYREF
    || *types == _C_ONEWAY
    || *types == _C_GCINVISIBLE
    || isdigit ((unsigned char) *types))
    {
      types++;
    }

  return types;
}

/* See header for documentation. */
GS_EXPORT BOOL
GSSelectorTypesMatch(const char *types1, const char *types2)
{
  if (! types1 || ! types2)
    {
      return NO;        // Nul pointers never match
    }
  if (types1 == types2)
    {
      return YES;
    }
  while (*types1 && *types2)
    {
      types1 = GSSkipTypeQualifierAndLayoutInfo (types1);
      types2 = GSSkipTypeQualifierAndLayoutInfo (types2);

      /* Reached the end of the selector.  */
      if (! *types1 && ! *types2)
        {
          return YES;
        }

      /* Ignore structure name yet compare layout.  */
      if (*types1 == '{' && *types2 == '{')
	{
	  while (*types1 != '=' && *types1 != '}')
            {
              types1++;
            }
	  while (*types2 != '=' && *types2 != '}')
            {
              types2++;
            }
	}

      if (*types1 != *types2)
        {
          return NO;
        }
      types1++;
      types2++;
    }

  types1 = GSSkipTypeQualifierAndLayoutInfo (types1);
  types2 = GSSkipTypeQualifierAndLayoutInfo (types2);

  return (! *types1 && ! *types2) ? YES : NO;
}

/* See header for documentation. */
GSIVar
GSCGetInstanceVariableDefinition(Class cls, const char *name)
{
  return class_getInstanceVariable(cls, name);
}

GSIVar
GSObjCGetInstanceVariableDefinition(Class cls, NSString *name)
{
  return class_getInstanceVariable(cls, [name UTF8String]);
}


static inline unsigned int
gs_string_hash(const char *s)
{
  unsigned int val = 0;
  while (*s != 0)
    {
      val = (val << 5) + val + *s++;
    }
  return val;
}

#define GSI_MAP_HAS_VALUE 1
#define GSI_MAP_RETAIN_KEY(M, X)
#define GSI_MAP_RETAIN_VAL(M, X)
#define GSI_MAP_RELEASE_KEY(M, X)
#define GSI_MAP_RELEASE_VAL(M, X)
#define GSI_MAP_HASH(M, X)    (gs_string_hash(X.ptr))
#define GSI_MAP_EQUAL(M, X,Y) (strcmp(X.ptr, Y.ptr) == 0)
#define GSI_MAP_NOCLEAN 1

#define GSI_MAP_KTYPES GSUNION_PTR
#define GSI_MAP_VTYPES GSUNION_PTR

#include "GNUstepBase/GSIMap.h"
#include <pthread.h>

static GSIMapTable_t protocol_by_name;
static BOOL protocol_by_name_init = NO;
static pthread_mutex_t protocol_by_name_lock = PTHREAD_MUTEX_INITIALIZER;

/* Not sure about the semantics of inlining
   functions with static variables.  */
static void
gs_init_protocol_lock(void)
{
  pthread_mutex_lock(&protocol_by_name_lock);
  if (protocol_by_name_init == NO)
  	{
	  GSIMapInitWithZoneAndCapacity (&protocol_by_name,
					 NSDefaultMallocZone(),
					 128);
	  protocol_by_name_init = YES;
	}
  pthread_mutex_unlock(&protocol_by_name_lock);
}

void
GSRegisterProtocol(Protocol *proto)
{
  if (protocol_by_name_init == NO)
    {
      gs_init_protocol_lock();
    }

  if (proto != nil)
    {
      GSIMapNode node;

      pthread_mutex_lock(&protocol_by_name_lock);
      node = GSIMapNodeForKey(&protocol_by_name,
	(GSIMapKey)protocol_getName(proto));
      if (node == 0)
	{
	  GSIMapAddPairNoRetain(&protocol_by_name,
	    (GSIMapKey)(void*)protocol_getName(proto),
	    (GSIMapVal)(void*)proto);
	}
      pthread_mutex_unlock(&protocol_by_name_lock);
    }
}

Protocol *
GSProtocolFromName(const char *name)
{
  GSIMapNode node;
  Protocol *p;

  if (protocol_by_name_init == NO)
    {
      gs_init_protocol_lock();
    }

  node = GSIMapNodeForKey(&protocol_by_name, (GSIMapKey) name);
  if (node)
    {
      p = node->value.ptr;
    }
  else
    {
      pthread_mutex_lock(&protocol_by_name_lock);
      node = GSIMapNodeForKey(&protocol_by_name, (GSIMapKey) name);

      if (node)
	{
	  p = node->value.ptr;
	}
      else
	{
	  p = objc_getProtocol(name);
	  if (p)
	    {
	      /* Use the protocol's name to save us from allocating
		 a copy of the parameter 'name'.  */
	      GSIMapAddPairNoRetain(&protocol_by_name,
	        (GSIMapKey)(void*)protocol_getName(p),
		(GSIMapVal)(void*)p);
	    }
	}
      pthread_mutex_unlock(&protocol_by_name_lock);

    }

  return p;
}

struct objc_method_description
GSProtocolGetMethodDescriptionRecursive(Protocol *aProtocol, SEL aSel, BOOL isRequired, BOOL isInstance)
{
  struct objc_method_description desc;

  desc = protocol_getMethodDescription(aProtocol, aSel, isRequired, isInstance);
  if (desc.name == NULL && desc.types == NULL)
    {
      Protocol **list;
      unsigned int count;

      list = protocol_copyProtocolList(aProtocol, &count);
      if (list != NULL)
        {
          unsigned int i;

          for (i = 0; i < count; i++)
            {
              desc = GSProtocolGetMethodDescriptionRecursive(list[i],
                aSel, isRequired, isInstance);
              if (desc.name != NULL || desc.types != NULL)
                {
                  break;
                }
            }
          free(list);
        }
    }

  return desc;
}

void
GSObjCAddClassBehavior(Class receiver, Class behavior)
{
  unsigned int	count;
  Method	*methods;
  Class behavior_super_class = class_getSuperclass(behavior);

  if (YES == class_isMetaClass(receiver))
    {
      fprintf(stderr, "Trying to add behavior (%s) to meta class (%s)\n",
	class_getName(behavior), class_getName(receiver));
      abort();
    }
  if (YES == class_isMetaClass(behavior))
    {
      fprintf(stderr, "Trying to add meta class as behavior (%s) to (%s)\n",
	class_getName(behavior), class_getName(receiver));
      abort();
    }
  if (class_getInstanceSize(receiver) < class_getInstanceSize(behavior))
    {
      const char *b = class_getName(behavior);
      const char *r = class_getName(receiver);

#ifdef NeXT_Foundation_LIBRARY
      fprintf(stderr, "Trying to add behavior (%s) with instance "
	"size larger than class (%s)\n", b, r);
      abort();
#else
      /* As a special case we allow adding GSString/GSCString to the
       * constant string class ... since we know the base library
       * takes care not to access non-existent instance variables.
       */
      if ((strcmp(b, "GSCString") && strcmp(b, "GSString"))
        || (strcmp(r, "NSConstantString") && strcmp(r, "NXConstantString")))
	{
	  fprintf(stderr, "Trying to add behavior (%s) with instance "
	    "size larger than class (%s)\n", b, r);
          abort();
	}
#endif
    }

  BDBGPrintf("Adding behavior to class %s\n", class_getName(receiver));

  /* Add instance methods */
  methods = class_copyMethodList(behavior, &count);
  BDBGPrintf("  instance methods from %s %u\n", class_getName(behavior), count);
  if (methods == NULL)
    {
      BDBGPrintf("    none.\n");
    }
  else
    {
      GSObjCAddMethods (receiver, methods, NO);
      free(methods);
    }

  /* Add class methods */
  methods = class_copyMethodList(object_getClass(behavior), &count);
  BDBGPrintf("  class methods from %s %u\n", class_getName(behavior), count);
  if (methods == NULL)
    {
      BDBGPrintf("    none.\n");
    }
  else
    {
      GSObjCAddMethods (object_getClass(receiver), methods, NO);
      free(methods);
    }

  /* Add behavior's superclass, if not already there. */
  if (!GSObjCIsKindOf(receiver, behavior_super_class))
    {
      GSObjCAddClassBehavior (receiver, behavior_super_class);
    }
  GSFlushMethodCacheForClass (receiver);
}

void
GSObjCAddClassOverride(Class receiver, Class override)
{
  unsigned int	count;
  Method	*methods;

  if (YES == class_isMetaClass(receiver))
    {
      fprintf(stderr, "Trying to add override (%s) to meta class (%s)\n",
	class_getName(override), class_getName(receiver));
      abort();
    }
  if (YES == class_isMetaClass(override))
    {
      fprintf(stderr, "Trying to add meta class as override (%s) to (%s)\n",
	class_getName(override), class_getName(receiver));
      abort();
    }
  if (class_getInstanceSize(receiver) < class_getInstanceSize(override))
    {
      fprintf(stderr, "Trying to add override (%s) with instance "
	"size larger than class (%s)\n",
	class_getName(override), class_getName(receiver));
      abort();
    }

  BDBGPrintf("Adding override to class %s\n", class_getName(receiver));

  /* Add instance methods */
  methods = class_copyMethodList(override, &count);
  BDBGPrintf("  instance methods from %s %u\n", class_getName(override), count);
  if (methods == NULL)
    {
      BDBGPrintf("    none.\n");
    }
  else
    {
      GSObjCAddMethods (receiver, methods, YES);
      free(methods);
    }

  /* Add class methods */
  methods = class_copyMethodList(object_getClass(override), &count);
  BDBGPrintf("  class methods from %s %u\n", class_getName(override), count);
  if (methods == NULL)
    {
      BDBGPrintf("    none.\n");
    }
  else
    {
      GSObjCAddMethods (object_getClass(receiver), methods, YES);
      free(methods);
    }
  GSFlushMethodCacheForClass (receiver);
}




#ifndef NeXT_Foundation_LIBRARY
#import	"Foundation/NSValue.h"
#import	"Foundation/NSKeyValueCoding.h"
#endif


/**
 * This is used internally by the key-value coding methods, to get a
 * value from an object either via an accessor method (if sel is
 * supplied), or via direct access (if type, size, and offset are
 * supplied).<br />
 * Automatic conversion between NSNumber and C scalar types is performed.<br />
 * If type is null and can't be determined from the selector, the
 * [NSObject-handleQueryWithUnboundKey:] method is called to try
 * to get a value.
 */
id
GSObjCGetVal(NSObject *self, const char *key, SEL sel,
	       const char *type, unsigned size, int offset)
{
  NSMethodSignature	*sig = nil;

  if (sel != 0)
    {
      sig = [self methodSignatureForSelector: sel];
      if ([sig numberOfArguments] != 2)
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"key-value get method has wrong number of args"];
	}
      type = [sig methodReturnType];
    }
  if (type == NULL)
    {
      return [self valueForUndefinedKey: [NSString stringWithUTF8String: key]];
    }
  else
    {
      id	val = nil;

      switch (*type)
	{
	  case _C_ID:
	  case _C_CLASS:
	    {
	      id	v;

	      if (sel == 0)
		{
		  v = *(id *)((char *)self + offset);
		}
	      else
		{
		  id	(*imp)(id, SEL) =
		    (id (*)(id, SEL))[self methodForSelector: sel];

		  v = (*imp)(self, sel);
		}
	      val = v;
	    }
	    break;

	  case _C_CHR:
	    {
	      signed char	v;

	      if (sel == 0)
		{
		  v = *(char *)((char *)self + offset);
		}
	      else
		{
		  signed char	(*imp)(id, SEL) =
		    (signed char (*)(id, SEL))[self methodForSelector: sel];

		  v = (*imp)(self, sel);
		}
	      val = [NSNumber numberWithChar: v];
	    }
	    break;

	  case _C_UCHR:
	    {
	      unsigned char	v;

	      if (sel == 0)
		{
		  v = *(unsigned char *)((char *)self + offset);
		}
	      else
		{
		  unsigned char	(*imp)(id, SEL) =
		    (unsigned char (*)(id, SEL))[self methodForSelector:
		    sel];

		  v = (*imp)(self, sel);
		}
	      val = [NSNumber numberWithUnsignedChar: v];
	    }
	    break;

#if __GNUC__ > 2 && defined(_C_BOOL)
          case _C_BOOL:
            {
              _Bool     v;
 
              if (sel == 0)
                {
                  v = *(_Bool *)((char *)self + offset);
                }
              else
                {
                  _Bool (*imp)(id, SEL) =
                    (_Bool (*)(id, SEL))[self methodForSelector: sel];
 
                  v = (*imp)(self, sel);
                }
              val = [NSNumber numberWithBool: (BOOL)v];
            }
            break;
#endif

	  case _C_SHT:
	    {
	      short	v;

	      if (sel == 0)
		{
		  v = *(short *)((char *)self + offset);
		}
	      else
		{
		  short	(*imp)(id, SEL) =
		    (short (*)(id, SEL))[self methodForSelector: sel];

		  v = (*imp)(self, sel);
		}
	      val = [NSNumber numberWithShort: v];
	    }
	    break;

	  case _C_USHT:
	    {
	      unsigned short	v;

	      if (sel == 0)
		{
		  v = *(unsigned short *)((char *)self + offset);
		}
	      else
		{
		  unsigned short	(*imp)(id, SEL) =
		    (unsigned short (*)(id, SEL))[self methodForSelector:
		    sel];

		  v = (*imp)(self, sel);
		}
	      val = [NSNumber numberWithUnsignedShort: v];
	    }
	    break;

	  case _C_INT:
	    {
	      int	v;

	      if (sel == 0)
		{
		  v = *(int *)((char *)self + offset);
		}
	      else
		{
		  int	(*imp)(id, SEL) =
		    (int (*)(id, SEL))[self methodForSelector: sel];

		  v = (*imp)(self, sel);
		}
	      val = [NSNumber numberWithInt: v];
	    }
	    break;

	  case _C_UINT:
	    {
	      unsigned int	v;

	      if (sel == 0)
		{
		  v = *(unsigned int *)((char *)self + offset);
		}
	      else
		{
		  unsigned int	(*imp)(id, SEL) =
		    (unsigned int (*)(id, SEL))[self methodForSelector:
		    sel];

		  v = (*imp)(self, sel);
		}
	      val = [NSNumber numberWithUnsignedInt: v];
	    }
	    break;

	  case _C_LNG:
	    {
	      long	v;

	      if (sel == 0)
		{
		  v = *(long *)((char *)self + offset);
		}
	      else
		{
		  long	(*imp)(id, SEL) =
		    (long (*)(id, SEL))[self methodForSelector: sel];

		  v = (*imp)(self, sel);
		}
	      val = [NSNumber numberWithLong: v];
	    }
	    break;

	  case _C_ULNG:
	    {
	      unsigned long	v;

	      if (sel == 0)
		{
		  v = *(unsigned long *)((char *)self + offset);
		}
	      else
		{
		  unsigned long	(*imp)(id, SEL) =
		    (unsigned long (*)(id, SEL))[self methodForSelector:
		    sel];

		  v = (*imp)(self, sel);
		}
	      val = [NSNumber numberWithUnsignedLong: v];
	    }
	    break;

#ifdef	_C_LNG_LNG
	  case _C_LNG_LNG:
	    {
	      long long	v;

	      if (sel == 0)
		{
		  v = *(long long *)((char *)self + offset);
		}
	      else
		{
		   long long	(*imp)(id, SEL) =
		    (long long (*)(id, SEL))[self methodForSelector: sel];

		  v = (*imp)(self, sel);
		}
	      val = [NSNumber numberWithLongLong: v];
	    }
	    break;
#endif

#ifdef	_C_ULNG_LNG
	  case _C_ULNG_LNG:
	    {
	      unsigned long long	v;

	      if (sel == 0)
		{
		  v = *(unsigned long long *)((char *)self + offset);
		}
	      else
		{
		  unsigned long long	(*imp)(id, SEL) =
		    (unsigned long long (*)(id, SEL))[self
		    methodForSelector: sel];

		  v = (*imp)(self, sel);
		}
	      val = [NSNumber numberWithUnsignedLongLong: v];
	    }
	    break;
#endif

	  case _C_FLT:
	    {
	      float	v;

	      if (sel == 0)
		{
		  v = *(float *)((char *)self + offset);
		}
	      else
		{
		  float	(*imp)(id, SEL) =
		    (float (*)(id, SEL))[self methodForSelector: sel];

		  v = (*imp)(self, sel);
		}
	      val = [NSNumber numberWithFloat: v];
	    }
	    break;

	  case _C_DBL:
	    {
	      double	v;

	      if (sel == 0)
		{
		  v = *(double *)((char *)self + offset);
		}
	      else
		{
		  double	(*imp)(id, SEL) =
		    (double (*)(id, SEL))[self methodForSelector: sel];

		  v = (*imp)(self, sel);
		}
	      val = [NSNumber numberWithDouble: v];
	    }
	    break;

	  case _C_VOID:
            {
              void        (*imp)(id, SEL) =
                (void (*)(id, SEL))[self methodForSelector: sel];

              (*imp)(self, sel);
            }
            val = nil;
            break;

          case _C_STRUCT_B:
            if (GSSelectorTypesMatch(@encode(NSPoint), type))
              {
                NSPoint	v;

                if (sel == 0)
                  {
                    memcpy((char*)&v, ((char *)self + offset), sizeof(v));
                  }
                else
                  {
                    NSPoint	(*imp)(id, SEL) =
                      (NSPoint (*)(id, SEL))[self methodForSelector: sel];

                    v = (*imp)(self, sel);
                  }
                val = [NSValue valueWithPoint: v];
              }
            else if (GSSelectorTypesMatch(@encode(NSRange), type))
              {
                NSRange	v;

                if (sel == 0)
                  {
                    memcpy((char*)&v, ((char *)self + offset), sizeof(v));
                  }
                else
                  {
                    NSRange	(*imp)(id, SEL) =
                      (NSRange (*)(id, SEL))[self methodForSelector: sel];

                    v = (*imp)(self, sel);
                  }
                val = [NSValue valueWithRange: v];
              }
            else if (GSSelectorTypesMatch(@encode(NSRect), type))
              {
                NSRect	v;

                if (sel == 0)
                  {
                    memcpy((char*)&v, ((char *)self + offset), sizeof(v));
                  }
                else
                  {
                    NSRect	(*imp)(id, SEL) =
                      (NSRect (*)(id, SEL))[self methodForSelector: sel];

                    v = (*imp)(self, sel);
                  }
                val = [NSValue valueWithRect: v];
              }
            else if (GSSelectorTypesMatch(@encode(NSSize), type))
              {
                NSSize	v;

                if (sel == 0)
                  {
                    memcpy((char*)&v, ((char *)self + offset), sizeof(v));
                  }
                else
                  {
                    NSSize	(*imp)(id, SEL) =
                      (NSSize (*)(id, SEL))[self methodForSelector: sel];

                    v = (*imp)(self, sel);
                  }
                val = [NSValue valueWithSize: v];
              }
            else
              {
                if (sel == 0)
                  {
		    return [NSValue valueWithBytes: ((char *)self + offset)
					  objCType: type];
                  }
                else
                  {
		    NSInvocation	*inv;
		    size_t		retSize;

		    inv = [NSInvocation invocationWithMethodSignature: sig];
		    [inv setSelector: sel];
		    [inv invokeWithTarget: self];
		    retSize = [sig methodReturnLength];
		    {
		      char ret[retSize];

		      [inv getReturnValue: ret];
		      return [NSValue valueWithBytes: ret objCType: type];
		    }
                  }
              }
            break;

	  default:
#ifdef __GNUSTEP_RUNTIME__
	    {
	      Class		cls;
	      struct objc_slot	*type_slot;
	      SEL		typed;
	      struct objc_slot	*slot;

	      cls = [self class];
	      type_slot = objc_get_slot(cls, @selector(retain));
	      typed = GSSelectorFromNameAndTypes(sel_getName(sel), NULL);
	      slot = objc_get_slot(cls, typed);
	      if (strcmp(slot->types, type_slot->types) == 0)
		{
		  return slot->method(self, typed);
		}
	    }
#endif
	    val = [self valueForUndefinedKey:
	      [NSString stringWithUTF8String: key]];
	}
      return val;
    }
}

/**
 * Calls GSObjCGetVal()
 */
id
GSObjCGetValue(NSObject *self, NSString *key, SEL sel,
	       const char *type, unsigned size, int offset)
{
  return GSObjCGetVal(self, [key UTF8String], sel, type, size, offset);
}

/**
 * This is used internally by the key-value coding methods, to set a
 * value in an object either via an accessor method (if sel is
 * supplied), or via direct access (if type, size, and offset are
 * supplied).<br />
 * Automatic conversion between NSNumber and C scalar types is performed.<br />
 * If type is null and can't be determined from the selector, the
 * [NSObject-handleTakeValue:forUnboundKey:] method is called to try
 * to set a value.
 */
void
GSObjCSetVal(NSObject *self, const char *key, id val, SEL sel,
  const char *type, unsigned size, int offset)
{
  static NSNull		*null = nil;
  NSMethodSignature	*sig = nil;

  if (null == nil)
    {
      null = [NSNull new];
    }
  if (sel != 0)
    {
      sig = [self methodSignatureForSelector: sel];
      if ([sig numberOfArguments] != 3)
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"key-value set method has wrong number of args"];
	}
      type = [sig getArgumentTypeAtIndex: 2];
    }
  if (type == NULL)
    {
      [self setValue: val forUndefinedKey:
	[NSString stringWithUTF8String: key]];
    }
  else if ((val == nil || val == null) && *type != _C_ID && *type != _C_CLASS)
    {
      [self setNilValueForKey: [NSString stringWithUTF8String: key]];
    }
  else
    {
      switch (*type)
	{
	  case _C_ID:
	  case _C_CLASS:
	    {
	      id	v = val;

	      if (sel == 0)
		{
		  id *ptr = (id *)((char *)self + offset);

		  ASSIGN(*ptr, v);
		}
	      else
		{
		  void	(*imp)(id, SEL, id) =
		    (void (*)(id, SEL, id))[self methodForSelector: sel];

		  (*imp)(self, sel, val);
		}
	    }
	    break;

	  case _C_CHR:
	    {
	      char	v = [val charValue];

	      if (sel == 0)
		{
		  char *ptr = (char *)((char *)self + offset);

		  *ptr = v;
		}
	      else
		{
		  void	(*imp)(id, SEL, char) =
		    (void (*)(id, SEL, char))[self methodForSelector: sel];

		  (*imp)(self, sel, v);
		}
	    }
	    break;

	  case _C_UCHR:
	    {
	      unsigned char	v = [val unsignedCharValue];

	      if (sel == 0)
		{
		  unsigned char *ptr = (unsigned char*)((char *)self + offset);

		  *ptr = v;
		}
	      else
		{
		  void	(*imp)(id, SEL, unsigned char) =
		    (void (*)(id, SEL, unsigned char))[self methodForSelector:
		    sel];

		  (*imp)(self, sel, v);
		}
	    }
	    break;

#if __GNUC__ > 2 && defined(_C_BOOL)
          case _C_BOOL:
            {
              _Bool     v = (_Bool)[val boolValue];
 
              if (sel == 0)
                {
                  _Bool *ptr = (_Bool*)((char *)self + offset);
 
                  *ptr = v;
                }
              else
                {
                  void  (*imp)(id, SEL, _Bool) =
                    (void (*)(id, SEL, _Bool))[self methodForSelector: sel];
 
                  (*imp)(self, sel, v);
                }
            }
            break;
#endif

	  case _C_SHT:
	    {
	      short	v = [val shortValue];

	      if (sel == 0)
		{
		  short *ptr = (short*)((char *)self + offset);

		  *ptr = v;
		}
	      else
		{
		  void	(*imp)(id, SEL, short) =
		    (void (*)(id, SEL, short))[self methodForSelector: sel];

		  (*imp)(self, sel, v);
		}
	    }
	    break;

	  case _C_USHT:
	    {
	      unsigned short	v = [val unsignedShortValue];

	      if (sel == 0)
		{
		  unsigned short *ptr;

		  ptr = (unsigned short*)((char *)self + offset);
		  *ptr = v;
		}
	      else
		{
		  void	(*imp)(id, SEL, unsigned short) =
		    (void (*)(id, SEL, unsigned short))[self methodForSelector:
		    sel];

		  (*imp)(self, sel, v);
		}
	    }
	    break;

	  case _C_INT:
	    {
	      int	v = [val intValue];

	      if (sel == 0)
		{
		  int *ptr = (int*)((char *)self + offset);

		  *ptr = v;
		}
	      else
		{
		  void	(*imp)(id, SEL, int) =
		    (void (*)(id, SEL, int))[self methodForSelector: sel];

		  (*imp)(self, sel, v);
		}
	    }
	    break;

	  case _C_UINT:
	    {
	      unsigned int	v = [val unsignedIntValue];

	      if (sel == 0)
		{
		  unsigned int *ptr = (unsigned int*)((char *)self + offset);

		  *ptr = v;
		}
	      else
		{
		  void	(*imp)(id, SEL, unsigned int) =
		    (void (*)(id, SEL, unsigned int))[self methodForSelector:
		    sel];

		  (*imp)(self, sel, v);
		}
	    }
	    break;

	  case _C_LNG:
	    {
	      long	v = [val longValue];

	      if (sel == 0)
		{
		  long *ptr = (long*)((char *)self + offset);

		  *ptr = v;
		}
	      else
		{
		  void	(*imp)(id, SEL, long) =
		    (void (*)(id, SEL, long))[self methodForSelector: sel];

		  (*imp)(self, sel, v);
		}
	    }
	    break;

	  case _C_ULNG:
	    {
	      unsigned long	v = [val unsignedLongValue];

	      if (sel == 0)
		{
		  unsigned long *ptr = (unsigned long*)((char *)self + offset);

		  *ptr = v;
		}
	      else
		{
		  void	(*imp)(id, SEL, unsigned long) =
		    (void (*)(id, SEL, unsigned long))[self methodForSelector:
		    sel];

		  (*imp)(self, sel, v);
		}
	    }
	    break;

#ifdef	_C_LNG_LNG
	  case _C_LNG_LNG:
	    {
	      long long	v = [val longLongValue];

	      if (sel == 0)
		{
		  long long *ptr = (long long*)((char *)self + offset);

		  *ptr = v;
		}
	      else
		{
		  void	(*imp)(id, SEL, long long) =
		    (void (*)(id, SEL, long long))[self methodForSelector: sel];

		  (*imp)(self, sel, v);
		}
	    }
	    break;
#endif

#ifdef	_C_ULNG_LNG
	  case _C_ULNG_LNG:
	    {
	      unsigned long long	v = [val unsignedLongLongValue];

	      if (sel == 0)
		{
		  unsigned long long *ptr = (unsigned long long*)((char*)self +
								  offset);

		  *ptr = v;
		}
	      else
		{
		  void	(*imp)(id, SEL, unsigned long long) =
		    (void (*)(id, SEL, unsigned long long))[self
		    methodForSelector: sel];

		  (*imp)(self, sel, v);
		}
	    }
	    break;
#endif

	  case _C_FLT:
	    {
	      float	v = [val floatValue];

	      if (sel == 0)
		{
		  float *ptr = (float*)((char *)self + offset);

		  *ptr = v;
		}
	      else
		{
		  void	(*imp)(id, SEL, float) =
		    (void (*)(id, SEL, float))[self methodForSelector: sel];

		  (*imp)(self, sel, v);
		}
	    }
	    break;

	  case _C_DBL:
	    {
	      double	v = [val doubleValue];

	      if (sel == 0)
		{
		  double *ptr = (double*)((char *)self + offset);

		  *ptr = v;
		}
	      else
		{
		  void	(*imp)(id, SEL, double) =
		    (void (*)(id, SEL, double))[self methodForSelector: sel];

		  (*imp)(self, sel, v);
		}
	    }
	    break;

          case _C_STRUCT_B:
            if (GSSelectorTypesMatch(@encode(NSPoint), type))
              {
                NSPoint	v = [val pointValue];

                if (sel == 0)
                  {
                    NSPoint *ptr = (NSPoint*)((char *)self + offset);

                    *ptr = v;
                  }
                else
                  {
                    void	(*imp)(id, SEL, NSPoint) =
                      (void (*)(id, SEL, NSPoint))[self methodForSelector: sel];

                    (*imp)(self, sel, v);
                  }
              }
            else if (GSSelectorTypesMatch(@encode(NSRange), type))
              {
                NSRange	v = [val rangeValue];

                if (sel == 0)
                  {
                    NSRange *ptr = (NSRange*)((char *)self + offset);

                    *ptr = v;
                  }
                else
                  {
                    void	(*imp)(id, SEL, NSRange) =
                      (void (*)(id, SEL, NSRange))[self methodForSelector: sel];

                    (*imp)(self, sel, v);
                  }
              }
            else if (GSSelectorTypesMatch(@encode(NSRect), type))
              {
                NSRect	v = [val rectValue];

                if (sel == 0)
                  {
                    NSRect *ptr = (NSRect*)((char *)self + offset);

                    *ptr = v;
                  }
                else
                  {
                    void	(*imp)(id, SEL, NSRect) =
                      (void (*)(id, SEL, NSRect))[self methodForSelector: sel];

                    (*imp)(self, sel, v);
                  }
              }
            else if (GSSelectorTypesMatch(@encode(NSSize), type))
              {
                NSSize	v = [val sizeValue];

                if (sel == 0)
                  {
                    NSSize *ptr = (NSSize*)((char *)self + offset);

                    *ptr = v;
                  }
                else
                  {
                    void	(*imp)(id, SEL, NSSize) =
                      (void (*)(id, SEL, NSSize))[self methodForSelector: sel];

                    (*imp)(self, sel, v);
                  }
              }
            else
              {
		NSUInteger	size;

		NSGetSizeAndAlignment(type, &size, 0);
                if (sel == 0)
                  {
		    [val getValue: ((char *)self + offset)];
		  }
		else
		  {
		    NSInvocation	*inv;
		    char		buf[size];

		    [val getValue: buf];
		    inv = [NSInvocation invocationWithMethodSignature: sig];
		    [inv setSelector: sel];
		    [inv setArgument: buf atIndex: 2];
		    [inv invokeWithTarget: self];
		  }
              }
            break;

	  default:
            [self setValue: val forUndefinedKey:
	      [NSString stringWithUTF8String: key]];
	}
    }
}

/**
 * Calls GSObjCSetVal()
 */
void
GSObjCSetValue(NSObject *self, NSString *key, id val, SEL sel,
	       const char *type, unsigned size, int offset)
{
  GSObjCSetVal(self, [key UTF8String], val, sel, type, size, offset);
}


/** Returns an autoreleased array of subclasses of Class cls, including
 *  subclasses of subclasses. */
NSArray *GSObjCAllSubclassesOfClass(Class cls)
{
  if (!cls)
    {
      return nil;
    }
  else
    {
      NSMutableArray	*result;
      Class		*classes;
      int 		numClasses;
      int		i;

      numClasses = objc_getClassList(NULL, 0);
      classes = NSZoneMalloc(NSDefaultMallocZone(), numClasses*sizeof(Class));
      objc_getClassList(classes, numClasses);
      result = [NSMutableArray array];
      for (i = 0; i < numClasses; i++)
	{
	  Class	c = classes[i];

	  if (YES == GSObjCIsKindOf(c, cls) && cls != c)
	    {
	      [result addObject: c];
	    }
	}
      NSZoneFree(NSDefaultMallocZone(), classes);
      return result;
    }
}

/** Returns an autoreleased array containing subclasses directly descendent of
 *  Class cls. */
NSArray *GSObjCDirectSubclassesOfClass(Class cls)
{
  if (!cls)
    {
      return nil;
    }
  else
    {
      NSMutableArray	*result;
      Class		*classes;
      int 		numClasses;
      int		i;

      numClasses = objc_getClassList(NULL, 0);
      classes = NSZoneMalloc(NSDefaultMallocZone(), numClasses*sizeof(Class));
      objc_getClassList(classes, numClasses);
      result = [NSMutableArray array];
      for (i = 0; i < numClasses; i++)
	{
	  Class	c = classes[i];

	  if (class_getSuperclass(c) == cls)
	    {
	      [result addObject: c];
	    }
	}
      NSZoneFree(NSDefaultMallocZone(), classes);
      return result;
    }
}

@interface 	GSAutoreleasedMemory : NSObject
@end
@implementation	GSAutoreleasedMemory
@end

void *
GSAutoreleasedBuffer(unsigned size)
{
#ifdef ALIGN
#undef ALIGN
#endif
#define ALIGN __alignof__(double)

  static Class	buffer_class = 0;
  static Class	autorelease_class;
  static SEL	autorelease_sel;
  static id	(*autorelease_imp)(Class, SEL, id);
  static int	instance_size;
  static int	offset;
  NSObject	*o;

  if (buffer_class == 0)
    {
      buffer_class = [GSAutoreleasedMemory class];
      instance_size = class_getInstanceSize(buffer_class);
      offset = instance_size % ALIGN;
      autorelease_class = [NSAutoreleasePool class];
      autorelease_sel = @selector(addObject:);
      autorelease_imp = (id (*)(Class, SEL, id))
        [autorelease_class methodForSelector: autorelease_sel];
    }
  o = (NSObject*)NSAllocateObject(buffer_class,
    size + offset, NSDefaultMallocZone());
  (*autorelease_imp)(autorelease_class, autorelease_sel, o);
  return ((void*)o) + instance_size + offset;
}



/*
 * Deprecated function.
 */
const char *
GSLastErrorStr(long error_id)
{
  return [[[NSError _last] localizedDescription] cString];
}



BOOL
GSPrintf (FILE *fptr, NSString* format, ...)
{
  static Class                  stringClass = 0;
  static NSStringEncoding       enc;
  va_list       		ap;
  NSAutoreleasePool		*arp = [NSAutoreleasePool new];
  NSString      		*message;
  NSData        		*data;
  BOOL          		ok = NO;

  if (stringClass == 0)
    {
      stringClass = [NSString class];
      enc = [stringClass defaultCStringEncoding];
    }
  message = [stringClass allocWithZone: NSDefaultMallocZone()];
  va_start (ap, format);
  message = [message initWithFormat: format locale: nil arguments: ap];
  va_end (ap);
  data = [message dataUsingEncoding: enc];
  if (data == nil)
    {
      data = [message dataUsingEncoding: NSUTF8StringEncoding];
    }
  [message release];

  if (data != nil)
    {
      unsigned int      length = [data length];

      if (length == 0 || fwrite([data bytes], 1, length, fptr) == length)
        {
          ok = YES;
        }
    }
  [arp drain];
  return ok;
}

#if     defined(GNUSTEP_BASE_LIBRARY)

# ifndef	NDEBUG
#   define	AADD(c, o) GSDebugAllocationAdd(c, o)
#   define	AREM(c, o) GSDebugAllocationRemove(c, o)
# else
#   define	AADD(c, o) 
#   define	AREM(c, o) 
# endif
#else
# define	AADD(c, o) 
# define	AREM(c, o) 
#endif	/* defined(GNUSTEP_BASE_LIBRARY) */

void
GSClassSwizzle(id instance, Class newClass)
{
  Class	oldClass = object_getClass(instance);

  /* Only set if the old and new class differ
   */
  if (oldClass != newClass)
    {
      /* NB.  The call to object_setClass() may not work (eg for a libobjc2
       * 'small object', in which case the class is unchanged and we need
       * to allow for that.
       */
      AREM(oldClass, instance);
      object_setClass(instance, newClass);
      newClass = object_getClass(instance);
      AADD(newClass, instance);
    }
}

void
GSObjCPrint(void *base, void *item)
{
  FILE	*fptr = stdout;
  Class	c;
  id	o;

  if (NULL == base)
    {
      fprintf(fptr, "null\n");
      return;
    }
  if (GSObjCIsClass((Class)base))
    {
      o = nil;
      c = (Class)base;
      if (NULL == item)
	{
	  fprintf(fptr, "%p is class %s {\n", base, GSNameFromClass(c));
	}
    }
  else if (GSObjCIsInstance((id)base))
    {
      o = (id)base;
      c = GSObjCClass(o);
      if (NULL == item)
	{
	  fprintf(fptr, "%p is instance of class %s (%p) {\n",
	    base, GSNameFromClass(c), c);
	}
    }
  else
    {
      fprintf(fptr, "%p is not a class or instance\n", base);
      return;
    }

  while (c != Nil)
    {
      unsigned	count;
      Ivar	*ivars = class_copyIvarList(c, &count);

      while (count-- > 0)
	{
	  Ivar		ivar = ivars[count];
	  const char	*name = ivar_getName(ivar);
	  const char	*type = ivar_getTypeEncoding(ivar);
	  ptrdiff_t	offset = ivar_getOffset(ivar);
	  const char	*t;

	  if (NULL == item)
	    {
	      fprintf(fptr, "  (%ld) %s", (long)offset, name);
	    }
	  else if (strcmp(item, name) == 0)
	    {
	      continue;	// not a match
	    }
	  else
	    {
	      fprintf(fptr, "(%ld) %s", (long)offset, name);
	    }

	  if (nil == o)
	    {
	      /* We have no instance ... display offset to ivar
	       */
	      fprintf(fptr, "\n");
	      continue;
	    }

	  fprintf(fptr, " = ");

	  t = GSSkipTypeQualifierAndLayoutInfo(type);
	  switch (*t)
	    {
	      case _C_ID:
		{
		  id	v = *(id *)((char *)o + offset);

		  if (nil == v)
		    {
		      fprintf(fptr, "nil\n");
		    }
		  else
		    {
		      fprintf(fptr, "%s instance %p\n",
			GSNameFromClass(GSObjCClass(v)), v);
		    }
		}
		break;

	      case _C_CLASS:
		{
		  Class	v = *(Class *)((char *)o + offset);

		  if (Nil == v)
		    {
		      fprintf(fptr, "Nil\n");
		    }
		  else
		    {
		      fprintf(fptr, "%s class %p\n", GSNameFromClass(v), v);
		    }
		}
		break;

	      case _C_CHR:
		{
		  signed char	v = *(char *)((char *)o + offset);

		  fprintf(fptr, "%c %d\n", v, (int)v);
		}
		break;

	      case _C_UCHR:
		{
		  unsigned char	v = *(unsigned char *)((char *)o + offset);

		  fprintf(fptr, "%c %u\n", v, (unsigned)v);
		}
		break;

#if __GNUC__ > 2 && defined(_C_BOOL)
	      case _C_BOOL:
		{
		  _Bool     v = *(_Bool *)((char *)o + offset);

		  fprintf(fptr, "%s %u\n", (v ? "YES" : "NO"), (unsigned)v);
		}
		break;
#endif

	      case _C_SHT:
		{
		  short	v = *(short *)((char *)o + offset);

		  fprintf(fptr, "%hd\n", v);
		}
		break;

	      case _C_USHT:
		{
		  unsigned short	v;

		  v = *(unsigned short *)((char *)o + offset);
		  fprintf(fptr, "%hu\n", v);
		}
		break;

	      case _C_INT:
		{
		  int	v = *(int *)((char *)o + offset);

		  fprintf(fptr, "%d\n", v);
		}
		break;

	      case _C_UINT:
		{
		  unsigned int	v = *(unsigned int *)((char *)o + offset);

		  fprintf(fptr, "%u\n", v);
		}
		break;

	      case _C_LNG:
		{
		  long	v = *(long *)((char *)o + offset);

		  fprintf(fptr, "%ld\n", v);
		}
		break;

	      case _C_ULNG:
		{
		  unsigned long	v = *(unsigned long *)((char *)o + offset);

		  fprintf(fptr, "%lu\n", v);
		}
		break;

#ifdef	_C_LNG_LNG
	      case _C_LNG_LNG:
		{
		  long long	v = *(long long *)((char *)o + offset);

		  fprintf(fptr, "%lld\n", v);
		}
		break;
#endif

#ifdef	_C_ULNG_LNG
	      case _C_ULNG_LNG:
		{
		  unsigned long long	v;

		  v = *(unsigned long long *)((char *)o + offset);
		  fprintf(fptr, "%llu\n", v);
		}
		break;
#endif

	      case _C_FLT:
		{
		  float	v = *(float *)((char *)o + offset);

		  fprintf(fptr, "%g\n", v);
		}
		break;

	      case _C_DBL:
		{
		  double	v = *(double *)((char *)o + offset);

		  fprintf(fptr, "%g\n", v);
		}
		break;

	      case _C_VOID:
		{
		  fprintf(fptr, "void ???\n");
		}
		break;

	      case _C_STRUCT_B:
		{
		  fprintf(fptr, "struct not supported\n");
		}
		break;

	      default:
		{
		  fprintf(fptr, "type %s not supported\n", type);
		}
		break;
	    }
	}
      if (ivars != NULL)
	{
          free(ivars);
	}
      c = class_getSuperclass(c);
    }
  if (NULL == item)
    {
      fprintf(fptr, "}\n");
    }
}


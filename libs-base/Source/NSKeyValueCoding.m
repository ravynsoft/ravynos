/** Implementation of KeyValueCoding for GNUStep
   Copyright (C) 2000,2002 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>

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

   <title>NSKeyValueCoding informal protocol reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSKeyValueCoding.h"
#import "Foundation/NSMethodSignature.h"
#import "Foundation/NSNull.h"
#import "Foundation/NSSet.h"
#import "Foundation/NSValue.h"

/* For the NSKeyValueMutableArray and NSKeyValueMutableSet classes
 */
#include "NSKeyValueMutableArray.m"
#include "NSKeyValueMutableSet.m"

/* For backward compatibility NSUndefinedKeyException is actually the same
 * as the older NSUnknownKeyException
 */
NSString* const NSUnknownKeyException = @"NSUnknownKeyException";
NSString* const NSUndefinedKeyException = @"NSUnknownKeyException";


/* this should move into autoconf once it's accepted */
#define WANT_DEPRECATED_KVC_COMPAT 1

#ifdef WANT_DEPRECATED_KVC_COMPAT

static IMP      takePath = 0;
static IMP      takeValue = 0;
static IMP      takePathKVO = 0;
static IMP      takeValueKVO = 0;

static inline void setupCompat()
{
  if (takePath == 0)
    {
      Class  c = NSClassFromString(@"GSKVOBase");

      takePathKVO = [c instanceMethodForSelector:
        @selector(takeValue:forKeyPath:)];
      takePath = [NSObject instanceMethodForSelector:
        @selector(takeValue:forKeyPath:)];
      takeValueKVO = [c instanceMethodForSelector:
        @selector(takeValue:forKey:)];
      takeValue = [NSObject instanceMethodForSelector:
        @selector(takeValue:forKey:)];
    }
}

#endif

static void
SetValueForKey(NSObject *self, id anObject, const char *key, unsigned size)
{
  SEL		sel = 0;
  const char	*type = 0;
  int		off = 0;

  if (size > 0)
    {
      const char	*name;
      char		buf[size + 6];
      char		lo;
      char		hi;

      memcpy(buf, "_set", 4);
      memcpy(&buf[4], key, size);
      lo = buf[4];
      hi = islower(lo) ? toupper(lo) : lo;
      buf[4] = hi;
      buf[size + 4] = ':';
      buf[size + 5] = '\0';

      name = &buf[1];	// setKey:
      type = NULL;
      sel = sel_getUid(name);
      if (sel == 0 || [self respondsToSelector: sel] == NO)
	{
	  name = buf;	// _setKey:
	  sel = sel_getUid(name);
	  if (sel == 0 || [self respondsToSelector: sel] == NO)
	    {
	      sel = 0;
	      if ([[self class] accessInstanceVariablesDirectly] == YES)
		{
		  buf[size + 4] = '\0';
		  buf[3] = '_';
		  buf[4] = lo;
		  name = &buf[3];	// _key
		  if (GSObjCFindVariable(self, name, &type, &size, &off) == NO)
		    {
		      buf[4] = hi;
		      buf[3] = 's';
		      buf[2] = 'i';
		      buf[1] = '_';
		      name = &buf[1];	// _isKey
		      if (GSObjCFindVariable(self,
			name, &type, &size, &off) == NO)
			{
			  buf[4] = lo;
			  name = &buf[4];	// key
			  if (GSObjCFindVariable(self,
			    name, &type, &size, &off) == NO)
			    {
			      buf[4] = hi;
			      buf[3] = 's';
			      buf[2] = 'i';
			      name = &buf[2];	// isKey
			      GSObjCFindVariable(self,
				name, &type, &size, &off);
			    }
			}
		    }
		}
	    }
	  else
	    {
	      GSOnceFLog(@"Key-value access using _setKey: is deprecated:");
	    }
	}
    }
  GSObjCSetVal(self, key, anObject, sel, type, size, off);
}

static id ValueForKey(NSObject *self, const char *key, unsigned size)
{
  SEL		sel = 0;
  int		off = 0;
  const char	*type = NULL;

  if (size > 0)
    {
      const char	*name;
      char		buf[size + 5];
      char		lo;
      char		hi;

      memcpy(buf, "_get", 4);
      memcpy(&buf[4], key, size);
      buf[size + 4] = '\0';
      lo = buf[4];
      hi = islower(lo) ? toupper(lo) : lo;
      buf[4] = hi;

      name = &buf[1];	// getKey
      sel = sel_getUid(name);
      if (sel == 0 || [self respondsToSelector: sel] == NO)
	{
	  buf[4] = lo;
	  name = &buf[4];	// key
	  sel = sel_getUid(name);
	  if (sel == 0 || [self respondsToSelector: sel] == NO)
	    {
              buf[4] = hi;
              buf[3] = 's';
              buf[2] = 'i';
              name = &buf[2];	// isKey
              sel = sel_getUid(name);
              if (sel == 0 || [self respondsToSelector: sel] == NO)
                {
                  sel = 0;
                }
	    }
	}

      if (sel == 0 && [[self class] accessInstanceVariablesDirectly] == YES)
	{
	  buf[4] = hi;
	  name = buf;	// _getKey
	  sel = sel_getUid(name);
	  if (sel == 0 || [self respondsToSelector: sel] == NO)
	    {
	      buf[4] = lo;
	      buf[3] = '_';
	      name = &buf[3];	// _key
	      sel = sel_getUid(name);
	      if (sel == 0 || [self respondsToSelector: sel] == NO)
		{
		  sel = 0;
		}
	    }
	  if (sel == 0)
	    {
	      if (GSObjCFindVariable(self, name, &type, &size, &off) == NO)
		{
                  buf[4] = hi;
                  buf[3] = 's';
                  buf[2] = 'i';
                  buf[1] = '_';
                  name = &buf[1];	// _isKey
		  if (!GSObjCFindVariable(self, name, &type, &size, &off))
                    {
                       buf[4] = lo;
                       name = &buf[4];		// key
		       if (!GSObjCFindVariable(self, name, &type, &size, &off))
                         {
                            buf[4] = hi;
                            buf[3] = 's';
                            buf[2] = 'i';
                            name = &buf[2];	// isKey
                            GSObjCFindVariable(self, name, &type, &size, &off);
                         }
                    }
		}
	    }
	}
    }
  return GSObjCGetVal(self, key, sel, type, size, off);
}


@implementation NSObject(KeyValueCoding)

+ (BOOL) accessInstanceVariablesDirectly
{
  return YES;
}


- (NSDictionary*) dictionaryWithValuesForKeys: (NSArray*)keys
{
  NSMutableDictionary	*dictionary;
  NSEnumerator		*enumerator;
  id			key;
#ifdef WANT_DEPRECATED_KVC_COMPAT
  static IMP	o = 0;

  /* Backward compatibility hack */
  if (o == 0)
    {
      o = [NSObject instanceMethodForSelector:
	@selector(valuesForKeys:)];
    }
  if ([self methodForSelector: @selector(valuesForKeys:)] != o)
    {
      return [self valuesForKeys: keys];
    }
#endif

  dictionary = [NSMutableDictionary dictionaryWithCapacity: [keys count]];
  enumerator = [keys objectEnumerator];
  while ((key = [enumerator nextObject]) != nil)
    {
      id	value = [self valueForKey: key];

      if (value == nil)
	{
	  value = [NSNull null];
	}
      [dictionary setObject: value forKey: key];
    }
  return dictionary;
}

- (NSMutableSet*) mutableSetValueForKey: (NSString*)aKey
{
  return [NSKeyValueMutableSet setForKey: aKey ofObject: self];
}

- (NSMutableSet*) mutableSetValueForKeyPath: (NSString*)aKey
{
  NSRange       r = [aKey rangeOfString: @"." options: NSLiteralSearch];

  if (r.length == 0)
    {
      return [self mutableSetValueForKey: aKey];
    }
  else
    {
      NSString	*key = [aKey substringToIndex: r.location];
      NSString	*path = [aKey substringFromIndex: NSMaxRange(r)];

      return [[self valueForKey: key] mutableSetValueForKeyPath: path];
    }
}

- (NSMutableArray*) mutableArrayValueForKey: (NSString*)aKey
{
  return [NSKeyValueMutableArray arrayForKey: aKey ofObject: self];
}

- (NSMutableArray*) mutableArrayValueForKeyPath: (NSString*)aKey
{
  NSRange       r = [aKey rangeOfString: @"." options: NSLiteralSearch];

  if (r.length == 0)
    {
      return [self mutableArrayValueForKey: aKey];
    }
  else
    {
      NSString	*key = [aKey substringToIndex: r.location];
      NSString	*path = [aKey substringFromIndex: NSMaxRange(r)];

      return [[self valueForKey: key] mutableArrayValueForKeyPath: path];
    }
}

- (void) setNilValueForKey: (NSString*)aKey
{
#ifdef WANT_DEPRECATED_KVC_COMPAT
  static IMP	o = 0;

  /* Backward compatibility hack */
  if (o == 0)
    {
      o = [NSObject instanceMethodForSelector:
	@selector(unableToSetNilForKey:)];
    }
  if ([self methodForSelector: @selector(unableToSetNilForKey:)] != o)
    {
      [self unableToSetNilForKey: aKey];
      return;
    }
#endif
  [NSException raise: NSInvalidArgumentException
    format: @"%@ -- %@ 0x%"PRIxPTR": Given nil value to set for key \"%@\"",
    NSStringFromSelector(_cmd), NSStringFromClass([self class]),
    (NSUInteger)self, aKey];
}


- (void) setValue: (id)anObject forKey: (NSString*)aKey
{
  unsigned	size = [aKey length] * 8;
  char		key[size + 1];
#ifdef WANT_DEPRECATED_KVC_COMPAT
  IMP   	o = [self methodForSelector: @selector(takeValue:forKey:)];

  setupCompat();
  if (o != takeValue && o != takeValueKVO)
    {
      (*o)(self, @selector(takeValue:forKey:), anObject, aKey);
      return;
    }
#endif

  [aKey getCString: key
	 maxLength: size + 1
	  encoding: NSUTF8StringEncoding];
  size = strlen(key);
  SetValueForKey(self, anObject, key, size);
}


- (void) setValue: (id)anObject forKeyPath: (NSString*)aKey
{
  NSRange       r = [aKey rangeOfString: @"." options: NSLiteralSearch];
#ifdef WANT_DEPRECATED_KVC_COMPAT
  IMP	        o = [self methodForSelector: @selector(takeValue:forKeyPath:)];

  setupCompat();
  if (o != takePath && o != takePathKVO)
    {
      (*o)(self, @selector(takeValue:forKeyPath:), anObject, aKey);
      return;
    }
#endif

  if (r.length == 0)
    {
      [self setValue: anObject forKey: aKey];
    }
  else
    {
      NSString	*key = [aKey substringToIndex: r.location];
      NSString	*path = [aKey substringFromIndex: NSMaxRange(r)];
      
      [[self valueForKey: key] setValue: anObject forKeyPath: path];
    }
}


- (void) setValue: (id)anObject forUndefinedKey: (NSString*)aKey
{
  NSDictionary	*dict;
  NSException	*exp; 
#ifdef WANT_DEPRECATED_KVC_COMPAT
  static IMP	o = 0;

  /* Backward compatibility hack */
  if (o == 0)
    {
      o = [NSObject instanceMethodForSelector:
	@selector(handleTakeValue:forUnboundKey:)];
    }
  if ([self methodForSelector: @selector(handleTakeValue:forUnboundKey:)] != o)
    {
      [self handleTakeValue: anObject forUnboundKey: aKey];
      return;
    }
#endif

  dict = [NSDictionary dictionaryWithObjectsAndKeys:
    (anObject ? (id)anObject : (id)@"(nil)"), @"NSTargetObjectUserInfoKey",
    (aKey ? (id)aKey : (id)@"(nil)"), @"NSUnknownUserInfoKey",
    nil];
  exp = [NSException exceptionWithName: NSUndefinedKeyException
				reason: @"Unable to set value for undefined key"
			      userInfo: dict];
  [exp raise];
}


- (void) setValuesForKeysWithDictionary: (NSDictionary*)aDictionary
{
  NSEnumerator	*enumerator;
  NSString	*key;
#ifdef WANT_DEPRECATED_KVC_COMPAT
  static IMP	o = 0;

  /* Backward compatibility hack */
  if (o == 0)
    {
      o = [NSObject instanceMethodForSelector:
	@selector(takeValuesFromDictionary:)];
    }
  if ([self methodForSelector: @selector(takeValuesFromDictionary:)] != o)
    {
      [self takeValuesFromDictionary: aDictionary];
      return;
    }
#endif

  enumerator = [aDictionary keyEnumerator];
  while ((key = [enumerator nextObject]) != nil)
    {
      [self setValue: [aDictionary objectForKey: key] forKey: key];
    }
}


- (BOOL) validateValue: (id*)aValue
                forKey: (NSString*)aKey
                 error: (NSError**)anError
{
  unsigned	size;

  if (aValue == 0 || (size = [aKey length] * 8) == 0)
    {
      [NSException raise: NSInvalidArgumentException format: @"nil argument"];
    }
  else
    {
      char		name[size + 16];
      SEL		sel;
      BOOL		(*imp)(id,SEL,id*,id*);

      memcpy(name, "validate", 8);
      [aKey getCString: &name[8]
	     maxLength: size + 1
	      encoding: NSUTF8StringEncoding];
      size = strlen(&name[8]);
      memcpy(&name[size + 8], ":error:", 7);
      name[size + 15] = '\0';
      if (islower(name[8]))
	{
	  name[8] = toupper(name[8]);
	}
      sel = sel_getUid(name);
      if (sel != 0 && [self respondsToSelector: sel] == YES)
	{
	  imp = (BOOL (*)(id,SEL,id*,id*))[self methodForSelector: sel];
	  return (*imp)(self, sel, aValue, anError);
	}
    }
  return YES;
}

- (BOOL) validateValue: (id*)aValue
            forKeyPath: (NSString*)aKey
                 error: (NSError**)anError
{
  NSRange       r = [aKey rangeOfString: @"." options: NSLiteralSearch];

  if (r.length == 0)
    {
      return [self validateValue: aValue forKey: aKey error: anError];
    }
  else
    {
      NSString	*key = [aKey substringToIndex: r.location];
      NSString	*path = [aKey substringFromIndex: NSMaxRange(r)];

      return [[self valueForKey: key] validateValue: aValue
                                         forKeyPath: path
                                              error: anError];
    }
}


- (id) valueForKey: (NSString*)aKey
{
  unsigned	size = [aKey length] * 8;
  char		key[size + 1];

  [aKey getCString: key
	 maxLength: size + 1
	  encoding: NSUTF8StringEncoding];
  size = strlen(key);
  return ValueForKey(self, key, size);
}


- (id) valueForKeyPath: (NSString*)aKey
{
  NSRange       r = [aKey rangeOfString: @"." options: NSLiteralSearch];

  if (r.length == 0)
    {
      return [self valueForKey: aKey];
    }
  else
    {
      NSString	*key = [aKey substringToIndex: r.location];
      NSString	*path = [aKey substringFromIndex: NSMaxRange(r)];

      return [[self valueForKey: key] valueForKeyPath: path];
    }
}


- (id) valueForUndefinedKey: (NSString*)aKey
{
  NSDictionary	*dict;
  NSException	*exp;
  NSString      *reason;
#ifdef WANT_DEPRECATED_KVC_COMPAT
  static IMP	o = 0;

  /* Backward compatibility hack */
  if (o == 0)
    {
      o = [NSObject instanceMethodForSelector:
	@selector(handleQueryWithUnboundKey:)];
    }
  if ([self methodForSelector: @selector(handleQueryWithUnboundKey:)] != o)
    {
      return [self handleQueryWithUnboundKey: aKey];
    }
#endif
  dict = [NSDictionary dictionaryWithObjectsAndKeys:
    self, @"NSTargetObjectUserInfoKey",
    (aKey ? (id)aKey : (id)@"(nil)"), @"NSUnknownUserInfoKey",
    nil];
  reason = [NSString stringWithFormat:
    @"Unable to find value for key \"%@\" of object %@ (%@)",
    aKey, self, [self class]];
  exp = [NSException exceptionWithName: NSUndefinedKeyException
				reason: reason
			      userInfo: dict];

  [exp raise];
  return nil;
}


#ifdef WANT_DEPRECATED_KVC_COMPAT

+ (BOOL) useStoredAccessor
{
  return YES;
}

- (id) storedValueForKey: (NSString*)aKey
{
  unsigned	size;

  if ([[self class] useStoredAccessor] == NO)
    {
      return [self valueForKey: aKey];
    }

  size = [aKey length] * 8;
  if (size > 0)
    {
      SEL		sel = 0;
      const char	*type = NULL;
      int		off = 0;
      const char	*name;
      char		key[size + 1];
      char		buf[size + 5];
      char		lo;
      char		hi;

      memcpy(buf, "_get", 4);
      [aKey getCString: key
	     maxLength: size + 1
	      encoding: NSUTF8StringEncoding];
      size = strlen(key);
      memcpy(&buf[4], key, size);
      buf[size + 4] = '\0';
      lo = buf[4];
      hi = islower(lo) ? toupper(lo) : lo;
      buf[4] = hi;

      name = buf;	// _getKey
      sel = sel_getUid(name);
      if (sel == 0 || [self respondsToSelector: sel] == NO)
	{
	  buf[3] = '_';
	  buf[4] = lo;
	  name = &buf[3]; // _key
	  sel = sel_getUid(name);
	  if (sel == 0 || [self respondsToSelector: sel] == NO)
	    {
	      sel = 0;
	    }
	}
      if (sel == 0)
	{
	  if ([[self class] accessInstanceVariablesDirectly] == YES)
	    {
	      // _key
	      if (GSObjCFindVariable(self, name, &type, &size, &off) == NO)
		{
		  name = &buf[4]; // key
		  GSObjCFindVariable(self, name, &type, &size, &off);
		}
	    }
	  if (type == NULL)
	    {
	      buf[3] = 't';
	      buf[4] = hi;
	      name = &buf[1]; // getKey
	      sel = sel_getUid(name);
	      if (sel == 0 || [self respondsToSelector: sel] == NO)
		{
		  buf[4] = lo;
		  name = &buf[4];	// key
		  sel = sel_getUid(name);
		  if (sel == 0 || [self respondsToSelector: sel] == NO)
		    {
		      sel = 0;
		    }
		}
	    }
	}
      if (sel != 0 || type != NULL)
	{
	  return GSObjCGetVal(self, key, sel, type, size, off);
	}
    }
  [self handleTakeValue: nil forUnboundKey: aKey];
  return nil;
}


- (void) takeStoredValue: (id)anObject forKey: (NSString*)aKey
{
  unsigned	size;

  if ([[self class] useStoredAccessor] == NO)
    {
      [self takeValue: anObject forKey: aKey];
      return;
    }

  size = [aKey length] * 8;
  if (size > 0)
    {
      SEL		sel;
      const char	*type;
      int		off;
      const char	*name;
      char		key[size + 1];
      char		buf[size + 6];
      char		lo;
      char		hi;

      memcpy(buf, "_set", 4);
      [aKey getCString: key
	     maxLength: size + 1
	      encoding: NSUTF8StringEncoding];
      size = strlen(key);
      memcpy(&buf[4], key, size);
      buf[size + 4] = '\0';
      lo = buf[4];
      hi = islower(lo) ? toupper(lo) : lo;
      buf[4] = hi;
      buf[size + 4] = ':';
      buf[size + 5] = '\0';

      name = buf;	// _setKey:
      type = NULL;
      off = 0;
      sel = sel_getUid(name);
      if (sel == 0 || [self respondsToSelector: sel] == NO)
	{
	  sel = 0;
	  if ([[self class] accessInstanceVariablesDirectly] == YES)
	    {
	      buf[size + 4] = '\0';
	      buf[4] = lo;
	      buf[3] = '_';
	      name = &buf[3];		// _key
	      if (GSObjCFindVariable(self, name, &type, &size, &off) == NO)
		{
		  name = &buf[4];	// key
		  GSObjCFindVariable(self, name, &type, &size, &off);
		}
	    }
	  if (type == NULL)
	    {
	      buf[size + 4] = ':';
	      buf[4] = hi;
	      buf[3] = 't';
	      name = &buf[1];		// setKey:
	      sel = sel_getUid(name);
	      if (sel == 0 || [self respondsToSelector: sel] == NO)
		{
		  sel = 0;
		}
	    }
	}
      if (sel != 0 || type != NULL)
	{
	  GSObjCSetVal(self, key, anObject, sel, type, size, off);
	  return;
	}
    }
  [self handleTakeValue: anObject forUnboundKey: aKey];
}


- (void) takeStoredValuesFromDictionary: (NSDictionary*)aDictionary
{
  NSEnumerator	*enumerator = [aDictionary keyEnumerator];
  NSNull	*null = [NSNull null];
  NSString	*key;

  while ((key = [enumerator nextObject]) != nil)
    {
      id obj = [aDictionary objectForKey: key];

      if (obj == null)
	{
	  obj = nil;
	}
      [self takeStoredValue: obj forKey: key];
    }
}

- (id) handleQueryWithUnboundKey: (NSString*)aKey
{
  NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
    self, @"NSTargetObjectUserInfoKey",
    (aKey ? (id)aKey : (id)@"(nil)"), @"NSUnknownUserInfoKey",
    nil];
  NSException *exp = [NSException exceptionWithName: NSUndefinedKeyException
				  reason: @"Unable to find value for key"
				  userInfo: dict];

  GSOnceMLog(@"This method is deprecated, use -valueForUndefinedKey:");
  [exp raise];
  return nil;
}


- (void) handleTakeValue: (id)anObject forUnboundKey: (NSString*)aKey
{
  NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
    (anObject ? (id)anObject : (id)@"(nil)"), @"NSTargetObjectUserInfoKey",
    (aKey ? (id)aKey : (id)@"(nil)"), @"NSUnknownUserInfoKey",
    nil];
  NSException *exp = [NSException exceptionWithName: NSUndefinedKeyException
				  reason: @"Unable to set value for key"
				  userInfo: dict];
  GSOnceMLog(@"This method is deprecated, use -setValue:forUndefinedKey:");
  [exp raise];
}


- (void) takeValue: (id)anObject forKey: (NSString*)aKey
{
  SEL		sel = 0;
  const char	*type = 0;
  int		off = 0;
  unsigned	size = [aKey length] * 8;
  char		key[size + 1];

  GSOnceMLog(@"This method is deprecated, use -setValue:forKey:");
  [aKey getCString: key
	 maxLength: size + 1
	  encoding: NSUTF8StringEncoding];
  size = strlen(key);
  if (size > 0)
    {
      const char	*name;
      char		buf[size + 6];
      char		lo;
      char		hi;

      memcpy(buf, "_set", 4);
      memcpy(&buf[4], key, size);
      lo = buf[4];
      hi = islower(lo) ? toupper(lo) : lo;
      buf[4] = hi;
      buf[size + 4] = ':';
      buf[size + 5] = '\0';

      name = &buf[1];	// setKey:
      type = NULL;
      sel = sel_getUid(name);
      if (sel == 0 || [self respondsToSelector: sel] == NO)
	{
	  name = buf;	// _setKey:
	  sel = sel_getUid(name);
	  if (sel == 0 || [self respondsToSelector: sel] == NO)
	    {
	      sel = 0;
	      if ([[self class] accessInstanceVariablesDirectly] == YES)
		{
		  buf[size + 4] = '\0';
		  buf[3] = '_';
		  buf[4] = lo;
		  name = &buf[4];	// key
		  if (GSObjCFindVariable(self, name, &type, &size, &off) == NO)
		    {
		      name = &buf[3];	// _key
		      GSObjCFindVariable(self, name, &type, &size, &off);
		    }
		}
	    }
	}
    }
  GSObjCSetVal(self, key, anObject, sel, type, size, off);
}


- (void) takeValue: (id)anObject forKeyPath: (NSString*)aKey
{
  NSRange	r = [aKey rangeOfString: @"." options: NSLiteralSearch];

  GSOnceMLog(@"This method is deprecated, use -setValue:forKeyPath:");
  if (r.length == 0)
    {
      [self takeValue: anObject forKey: aKey];
    }
  else
    {
      NSString	*key = [aKey substringToIndex: r.location];
      NSString	*path = [aKey substringFromIndex: NSMaxRange(r)];

      [[self valueForKey: key] takeValue: anObject forKeyPath: path];
    }
}


- (void) takeValuesFromDictionary: (NSDictionary*)aDictionary
{
  NSEnumerator	*enumerator = [aDictionary keyEnumerator];
  NSNull	*null = [NSNull null];
  NSString	*key;

  GSOnceMLog(@"This method is deprecated, use -setValuesForKeysWithDictionary:");
  while ((key = [enumerator nextObject]) != nil)
    {
      id obj = [aDictionary objectForKey: key];

      if (obj == null)
	{
	  obj = nil;
	}
      [self takeValue: obj forKey: key];
    }
}


- (void) unableToSetNilForKey: (NSString*)aKey
{
  GSOnceMLog(@"This method is deprecated, use -setNilValueForKey:");
  [NSException raise: NSInvalidArgumentException format:
    @"%@ -- %@ 0x%"PRIxPTR": Given nil value to set for key \"%@\"",
    NSStringFromSelector(_cmd), NSStringFromClass([self class]),
    (NSUInteger)self, aKey];
}


- (NSDictionary*) valuesForKeys: (NSArray*)keys
{
  NSMutableDictionary	*dict;
  NSNull		*null = [NSNull null];
  unsigned		count = [keys count];
  unsigned		pos;

  GSOnceMLog(@"This method is deprecated, use -dictionaryWithValuesForKeys:");
  dict = [NSMutableDictionary dictionaryWithCapacity: count];
  for (pos = 0; pos < count; pos++)
    {
      NSString	*key = [keys objectAtIndex: pos];
      id 	val = [self valueForKey: key];

      if (val == nil)
	{
	  val = null;
	}
      [dict setObject: val forKey: key];
    }
  return AUTORELEASE([dict copy]);
}

#endif

@end


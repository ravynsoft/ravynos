/* 
   GSTextStorage.m

   Implementation of concrete subclass of a string class with attributes

   Copyright (C) 1999 Free Software Foundation, Inc.

   Based on code by: ANOQ of the sun <anoq@vip.cybercity.dk>
   Written by: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: July 1999
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

/* Warning -	[-initWithString:attributes:] is the designated initialiser,
 *		but it doesn't provide any way to perform the function of the
 *		[-initWithAttributedString:] initialiser.
 *		In order to work youd this, the string argument of the
 *		designated initialiser has been overloaded such that it
 *		is expected to accept an NSAttributedString here instead of
 *		a string.  If you create an NSAttributedString subclass, you
 *		must make sure that your implementation of the initialiser
 *		copes with either an NSString or an NSAttributedString.
 *		If it receives an NSAttributedString, it should ignore the
 *		attributes argument and use the values from the string.
 */

#import <Foundation/NSAttributedString.h>
#import <Foundation/NSException.h>
#import <Foundation/NSRange.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSZone.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSProxy.h>
#import <Foundation/NSInvocation.h>
#import <Foundation/NSNotification.h>
#import "AppKit/NSTextStorage.h"
#import "GSTextStorage.h"

#define		SANITY_CHECKS	0

static BOOL     adding;

/* When caching attributes we make a shallow copy of the dictionary cached,
 * so that it is immutable and safe to cache.
 * However, we have a potential problem if the objects within the attributes
 * dictionary are themselves mutable, and something mutates them while they
 * are in the cache.  In this case we could items added while different and
 * then mutated to have the same contents, so we would not know which of
 * the equal dictionaries to remove.
 * The solution is to require dictionaries to be identical for removal.
 */
static inline BOOL
cacheEqual(id A, id B)
{
  if (YES == adding)
    return [A isEqualToDictionary: B];
  else
    return A == B;
}

#define	GSI_MAP_RETAIN_KEY(M, X)	
#define	GSI_MAP_RELEASE_KEY(M, X)	
#define	GSI_MAP_RETAIN_VAL(M, X)	
#define	GSI_MAP_RELEASE_VAL(M, X)	
#define	GSI_MAP_EQUAL(M, X,Y)	cacheEqual((X).obj, (Y).obj)
#define GSI_MAP_KTYPES	GSUNION_OBJ
#define GSI_MAP_VTYPES	GSUNION_NSINT
#define	GSI_MAP_NOCLEAN	1
#include <GNUstepBase/GSIMap.h>

static NSDictionary	*blank;
static NSLock		*attrLock = nil;
static GSIMapTable_t	attrMap;
static SEL		lockSel;
static SEL		unlockSel;
static IMP		lockImp;
static IMP		unlockImp;

#define	ALOCK()	if (attrLock != nil) (*lockImp)(attrLock, lockSel)
#define	AUNLOCK() if (attrLock != nil) (*unlockImp)(attrLock, unlockSel)

@interface GSTextStorageProxy : NSProxy
{
  NSString	*string;
}
- (id) _initWithString: (NSString*)s;
@end

@implementation	GSTextStorageProxy

static Class NSObjectClass = nil;
static Class NSStringClass = nil;

+ (void) initialize
{
  NSObjectClass = [NSObject class];
  NSStringClass = [NSString class];
}

- (Class) class
{
  return NSStringClass;
}

- (void) dealloc
{
  [string release];
  [super dealloc];
}

- (void) forwardInvocation: (NSInvocation*)anInvocation
{
  SEL	aSel = [anInvocation selector];

  if (YES == [NSStringClass instancesRespondToSelector: aSel])
    {
      [anInvocation invokeWithTarget: string];
    }
  else
    {
      [NSException raise: NSGenericException
	          format: @"NSString(instance) does not recognize %s",
	aSel ? GSNameFromSelector(aSel) : "(null)"];
    }
}

- (NSUInteger) hash
{
  return [string hash];
}

- (id) _initWithString: (NSString*)s
{
  string = [s retain];
  return self;
}

- (BOOL) isEqual: (id)other
{
  return [string isEqual: other];
}

- (BOOL) isMemberOfClass: (Class)c
{
  return (c == NSStringClass) ? YES : NO;
}

- (BOOL) isKindOfClass: (Class)c
{
  return (c == NSStringClass || c == NSObjectClass) ? YES : NO;
}

- (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector
{
  NSMethodSignature	*sig;

  if (YES == [NSStringClass instancesRespondToSelector: aSelector])
    {
      sig = [string methodSignatureForSelector: aSelector];
    }
  else
    {
      sig = [super methodSignatureForSelector: aSelector];
    }
  return sig;
}

- (BOOL) respondsToSelector: (SEL)aSelector
{
  if (YES == [NSStringClass instancesRespondToSelector: aSelector])
    {
      return YES;
    }
  return [super respondsToSelector: aSelector];
}

@end

/*
 * Add a dictionary to the cache - if it was not already there, return
 * the copy added to the cache, if it was, count it and return retained
 * object that was there.
 */
static NSDictionary*
cacheAttributes(NSDictionary *attrs)
{
  GSIMapNode	node;

  ALOCK();
  adding = YES;
  node = GSIMapNodeForKey(&attrMap, (GSIMapKey)((id)attrs));
  if (node == 0)
    {
      /*
       * Shallow copy of dictionary, without copying objects ... results
       * in an immutable dictionary that can safely be cached.
       */
      attrs = [[NSDictionary alloc] initWithDictionary: attrs copyItems: NO];
      GSIMapAddPair(&attrMap,
        (GSIMapKey)((id)attrs), (GSIMapVal)(NSUInteger)1);
    }
  else
    {
      node->value.nsu++;
      attrs = RETAIN(node->key.obj);
    }
  AUNLOCK();
  return attrs;
}

static void
unCacheAttributes(NSDictionary *attrs)
{
  GSIMapBucket       bucket;

  ALOCK();
  adding = NO;
  bucket = GSIMapBucketForKey(&attrMap, (GSIMapKey)((id)attrs));
  if (bucket != 0)
    {
      GSIMapNode     node;

      node = GSIMapNodeForKeyInBucket(&attrMap,
        bucket, (GSIMapKey)((id)attrs));
      if (node != 0)
	{
	  if (--node->value.nsu == 0)
	    {
	      GSIMapRemoveNodeFromMap(&attrMap, bucket, node);
	      GSIMapFreeNode(&attrMap, node);
	    }
	}
    }
  AUNLOCK();
}



@interface	GSTextInfo : NSObject
{
@public
  unsigned	loc;
  NSDictionary	*attrs;
}

+ (GSTextInfo*) newWithZone: (NSZone*)z value: (NSDictionary*)a at: (unsigned)l;

@end

@implementation	GSTextInfo

/*
 * Called to record attributes at a particular location - the given attributes
 * dictionary must have been produced by 'cacheAttributes()' so that it is
 * already copied/retained and this method doesn't need to do it.
 */
+ (GSTextInfo*) newWithZone: (NSZone*)z value: (NSDictionary*)a at: (unsigned)l;
{
  GSTextInfo	*info = (GSTextInfo*)NSAllocateObject(self, 0, z);

  info->loc = l;
  info->attrs = a;
  return info;
}

- (Class) classForPortCoder
{
  return [self class];
}

- (void) dealloc
{
  [self finalize];
  [super dealloc];
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"Attributes at %u are - %@",
    loc, attrs];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding] == NO)
    {
      [aCoder encodeValueOfObjCType: @encode(unsigned) at: &loc];
      [aCoder encodeValueOfObjCType: @encode(id) at: &attrs];
    }
}

- (void) finalize
{
  unCacheAttributes(attrs);
  DESTROY(attrs);
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding] == NO)
    {
      NSDictionary	*a;
      [aCoder decodeValueOfObjCType: @encode(unsigned) at: &loc];
      a = [aCoder decodeObject];
      attrs = cacheAttributes(a);
    }
  return self;
}

- (id) replacementObjectForPortCoder: (NSPortCoder*)aCoder
{
  return self;
}

@end



static Class	infCls = 0;

static SEL infSel;
static SEL addSel;
static SEL cntSel;
static SEL insSel;
static SEL oatSel;
static SEL remSel;

static IMP	infImp;
static void	(*addImp)();
static unsigned (*cntImp)();
static void	(*insImp)();
static IMP	oatImp;
static void	(*remImp)();

#define	NEWINFO(Z,O,L)	((*infImp)(infCls, infSel, (Z), (O), (L)))
#define	ADDOBJECT(O)	((*addImp)(_infoArray, addSel, (O)))
#define	INSOBJECT(O,I)	((*insImp)(_infoArray, insSel, (O), (I)))
#define	OBJECTAT(I)	((*oatImp)(_infoArray, oatSel, (I)))
#define	REMOVEAT(I)	((*remImp)(_infoArray, remSel, (I)))

static inline NSDictionary *attrDict(GSTextInfo* info)
{
  return info->attrs;
}


static void _setup()
{
  if (infCls == 0)
    {
      NSMutableArray	*a;
      NSDictionary	*d;

      GSIMapInitWithZoneAndCapacity(&attrMap, NSDefaultMallocZone(), 32);

      infSel = @selector(newWithZone:value:at:);
      addSel = @selector(addObject:);
      cntSel = @selector(count);
      insSel = @selector(insertObject:atIndex:);
      oatSel = @selector(objectAtIndex:);
      remSel = @selector(removeObjectAtIndex:);

      infCls = [GSTextInfo class];
      infImp = [infCls methodForSelector: infSel];

      a = [NSMutableArray allocWithZone: NSDefaultMallocZone()];
      a = [a initWithCapacity: 1];
      addImp = (void (*)())[a methodForSelector: addSel];
      cntImp = (unsigned (*)())[a methodForSelector: cntSel];
      insImp = (void (*)())[a methodForSelector: insSel];
      oatImp = [a methodForSelector: oatSel];
      remImp = (void (*)())[a methodForSelector: remSel];
      RELEASE(a);

      d = [NSDictionary new];
      blank = cacheAttributes(d);
      RELEASE(d);
    }
}

static void
_setAttributesFrom(
  NSAttributedString *attributedString,
  NSRange aRange,
  NSMutableArray *_infoArray)
{
  NSZone	*z = [_infoArray zone];
  NSRange	range;
  NSDictionary	*attr;
  GSTextInfo	*info;
  unsigned	loc;

  /*
   * remove any old attributes of the string.
   */
  [_infoArray removeAllObjects];

  if (aRange.length <= 0)
    {
      attr = blank;
      range = aRange; /* Set to satisfy the loop condition below. */
    }
  else
    {
      attr = [attributedString attributesAtIndex: aRange.location
				  effectiveRange: &range];
    }
  attr = cacheAttributes(attr);
  info = NEWINFO(z, attr, 0);
  ADDOBJECT(info);
  RELEASE(info);

  while ((loc = NSMaxRange(range)) < NSMaxRange(aRange))
    {
      attr = [attributedString attributesAtIndex: loc
				  effectiveRange: &range];
      attr = cacheAttributes(attr);
      info = NEWINFO(z, attr, loc - aRange.location);
      ADDOBJECT(info);
      RELEASE(info);
    }
}

inline static NSDictionary*
_attributesAtIndexEffectiveRange(
  unsigned int index,
  NSRange *aRange,
  unsigned int tmpLength,
  NSMutableArray *_infoArray,
  unsigned int *foundIndex)
{
  unsigned	low, high, used, cnt, nextLoc;
  GSTextInfo	*found = nil;

  used = (*cntImp)(_infoArray, cntSel);
  NSCAssert(used > 0, NSInternalInconsistencyException);
  high = used - 1;

  if (index >= tmpLength)
    {
      if (index == tmpLength)
	{
	  found = OBJECTAT(high);
	  if (foundIndex != 0)
	    {
	      *foundIndex = high;
	    }
	  if (aRange != 0)
	    {
	      aRange->location = found->loc;
	      aRange->length = tmpLength - found->loc;
	    }
	  return attrDict(found);
	}
      [NSException raise: NSRangeException
		  format: @"index is out of range in function "
			  @"_attributesAtIndexEffectiveRange()"];
    }
  
  /*
   * Binary search for efficiency in huge attributed strings
   */
  low = 0;
  while (low <= high)
    {
      cnt = (low + high) / 2;
      found = OBJECTAT(cnt);
      if (found->loc > index)
	{
	  high = cnt - 1;
	}
      else
	{
	  if (cnt >= used - 1)
	    {
	      nextLoc = tmpLength;
	    }
	  else
	    {
	      GSTextInfo	*inf = OBJECTAT(cnt + 1);

	      nextLoc = inf->loc;
	    }
	  if (found->loc == index || index < nextLoc)
	    {
	      //Found
	      if (aRange != 0)
		{
		  aRange->location = found->loc;
		  aRange->length = nextLoc - found->loc;
		}
	      if (foundIndex != 0)
		{
		  *foundIndex = cnt;
		}
	      return attrDict(found);
	    }
	  else
	    {
	      low = cnt + 1;
	    }
	}
    }
  NSCAssert(NO,@"Error in binary search algorithm");
  return nil;
}

@implementation GSTextStorage

#if	SANITY_CHECKS

#define	SANITY()	[self sanity]
#else
#define	SANITY()	
#endif

/* We always compile in this method so that it is available from
 * regression test cases.  */
- (void) _sanity
{
  GSTextInfo	*info;
  unsigned	i;
  unsigned	l = 0;
  unsigned	len = [_textChars length];
  unsigned	c = (*cntImp)(_infoArray, cntSel);

  NSAssert(c > 0, NSInternalInconsistencyException);
  info = OBJECTAT(0);
  NSAssert(info->loc == 0, NSInternalInconsistencyException);
  for (i = 1; i < c; i++)
    {
      info = OBJECTAT(i);
      NSAssert(info->loc > l, NSInternalInconsistencyException);
      NSAssert(info->loc < len, NSInternalInconsistencyException);
      l = info->loc;
    }
}

/*
 * If we are multi-threaded, we must guard access to the uniquing set.
 */
+ (void) _becomeThreaded: (id)notification
{
  attrLock = [NSLock new];
  lockSel = @selector(lock);
  unlockSel = @selector(unlock);
  lockImp = [attrLock methodForSelector: lockSel];
  unlockImp = [attrLock methodForSelector: unlockSel];
}

+ (void) initialize
{
  _setup();

  if ([NSThread isMultiThreaded])
    {
      [self _becomeThreaded: nil];
    }
  else
    {
      [[NSNotificationCenter defaultCenter]
	addObserver: self
	   selector: @selector(_becomeThreaded:)
	       name: NSWillBecomeMultiThreadedNotification
	     object: nil];
    }
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  self = [super initWithCoder: aCoder];
  if([aCoder allowsKeyedCoding] == NO)
    {
      if ([aCoder versionForClassName: @"GSTextStorage"] != (NSInteger)NSNotFound)
        {
          NSLog(@"Warning - decoding archive containing obsolete %@ object - please delete/replace this archive", NSStringFromClass([self class]));
          [aCoder decodeValueOfObjCType: @encode(id) at: &_textChars];
          [aCoder decodeValueOfObjCType: @encode(id) at: &_infoArray];
        }
    }
  return self;
}

- (id) initWithString: (NSString*)aString
	   attributes: (NSDictionary*)attributes
{
  NSZone *z = [self zone];

  self = [super initWithString: aString attributes: attributes];
  _infoArray = [[NSMutableArray allocWithZone: z] initWithCapacity: 1];
  if (aString != nil && [aString isKindOfClass: [NSAttributedString class]])
    {
      NSAttributedString *as = (NSAttributedString*)aString;

      aString = [as string];
      _setAttributesFrom(as, NSMakeRange(0, [aString length]), _infoArray);
    }
  else
    {
      GSTextInfo *info;

      if (attributes == nil)
        {
          attributes = blank;
        }
      attributes = cacheAttributes(attributes);
      info = NEWINFO(z, attributes, 0);
      ADDOBJECT(info);
      RELEASE(info);
    }
  if (aString == nil)
    _textChars = [[NSMutableString allocWithZone: z] init];
  else
    _textChars = [aString mutableCopyWithZone: z];
  return self;
}

- (NSString*) string
{
  /* NB. This method is SUPPOSED to return a proxy to the mutable string!
   * This is a performance feature documented ifor OSX.
   */
  if (_textProxy == nil)
    {
      _textProxy = [[_textChars immutableProxy] retain];
    }
  return _textProxy;
}

- (NSDictionary*) attributesAtIndex: (NSUInteger)index
		     effectiveRange: (NSRange*)aRange
{
  unsigned dummy;

  return _attributesAtIndexEffectiveRange(
    index, aRange, [_textChars length], _infoArray, &dummy);
}

/*
 *	Primitive method! Sets attributes and values for a given range of
 *	characters, replacing any previous attributes and values for that
 *	range.
 *
 *	Sets the attributes for the characters in aRange to attributes.
 *	These new attributes replace any attributes previously associated
 *	with the characters in aRange. Raises an NSRangeException if any
 *	part of aRange lies beyond the end of the receiver's characters.
 *	See also: - addAtributes: range: , - removeAttributes: range:
 */
- (void) setAttributes: (NSDictionary*)attributes
		 range: (NSRange)range
{
  unsigned	tmpLength;
  unsigned      arrayIndex = 0;
  unsigned      arraySize;
  NSRange	effectiveRange = NSMakeRange(0, NSNotFound);
  NSRange	originalRange = range;
  unsigned	afterRangeLoc, beginRangeLoc;
  NSDictionary	*attrs;
  NSZone	*z = [self zone];
  GSTextInfo	*info;

  if (range.length == 0)
    {
      NSWarnMLog(@"Attempt to set attribute for zero-length range");
      return;
    }
  if (attributes == nil)
    {
      attributes = blank;
    }
  attributes = cacheAttributes(attributes);
SANITY();
  tmpLength = [_textChars length];
  GS_RANGE_CHECK(range, tmpLength);
  arraySize = (*cntImp)(_infoArray, cntSel);
  beginRangeLoc = range.location;
  afterRangeLoc = NSMaxRange(range);
  if (afterRangeLoc < tmpLength)
    {
      /*
       * Locate the first range that extends beyond our range.
       */
      attrs = _attributesAtIndexEffectiveRange(
	afterRangeLoc, &effectiveRange, tmpLength, _infoArray, &arrayIndex);
      if (attrs == attributes)
	{
	  /*
	   * The located range has the same attributes as us - so we can
	   * extend our range to include it.
	   */
	  if (effectiveRange.location < beginRangeLoc)
	    {
	      range.length += beginRangeLoc - effectiveRange.location;
	      range.location = effectiveRange.location;
	      beginRangeLoc = range.location;
	    }
	  if (NSMaxRange(effectiveRange) > afterRangeLoc)
	    {
	      range.length = NSMaxRange(effectiveRange) - range.location;
	    }
	}
      else if (effectiveRange.location > beginRangeLoc)
	{
	  /*
	   * The located range also starts at or after our range.
	   */
	  info = OBJECTAT(arrayIndex);
	  info->loc = afterRangeLoc;
	  arrayIndex--;
	}
      else if (NSMaxRange(effectiveRange) > afterRangeLoc)
	{
	  /*
	   * The located range starts before our range.
	   * Create a subrange to go from our end to the end of the old range.
	   */
	  info = NEWINFO(z, cacheAttributes(attrs), afterRangeLoc);
	  arrayIndex++;
	  INSOBJECT(info, arrayIndex);
	  RELEASE(info);
	  arrayIndex--;
	}
    }
  else
    {
      arrayIndex = arraySize - 1;
    }
  
  /*
   * Remove any ranges completely within ours
   */
  while (arrayIndex > 0)
    {
      info = OBJECTAT(arrayIndex-1);
      if (info->loc < beginRangeLoc)
	break;
      REMOVEAT(arrayIndex);
      arrayIndex--;
    }

  /*
   * Use the location/attribute info in the current slot if possible,
   * otherwise, add a new slot and use that.
   */
  info = OBJECTAT(arrayIndex);
  if (info->loc >= beginRangeLoc)
    {
      info->loc = beginRangeLoc;
      if (info->attrs == attributes)
	{
	  unCacheAttributes(attributes);
	  RELEASE(attributes);
	}
      else
	{
	  unCacheAttributes(info->attrs);
	  RELEASE(info->attrs);
	  info->attrs = attributes;
	}
    }
  else if (info->attrs == attributes)
    {
      unCacheAttributes(attributes);
      RELEASE(attributes);
    }
  else
    {
      arrayIndex++;
      info = NEWINFO(z, attributes, beginRangeLoc);
      INSOBJECT(info, arrayIndex);
      RELEASE(info);
    }
  
SANITY();
  [self edited: NSTextStorageEditedAttributes
	 range: originalRange
changeInLength: 0];
}

- (void) replaceCharactersInRange: (NSRange)range
		       withString: (NSString*)aString
{
  unsigned	tmpLength;
  unsigned      arrayIndex = 0;
  unsigned      arraySize;
  NSRange	effectiveRange = NSMakeRange(0, NSNotFound);
  GSTextInfo	*info;
  int		moveLocations;
  unsigned	start;

SANITY();
  if (aString == nil)
    {
      aString = @"";
    }
  tmpLength = [_textChars length];
  GS_RANGE_CHECK(range, tmpLength);
  if (range.location == tmpLength)
    {
      /*
       * Special case - replacing a zero length string at the end
       * simply appends the new string and attributes are inherited.
       */
      [_textChars appendString: aString];
      goto finish;
    }

  arraySize = (*cntImp)(_infoArray, cntSel);
  if (arraySize == 1)
    {
      /*
       * Special case - if the string has only one set of attributes
       * then the replacement characters will get them too.
       */
      [_textChars replaceCharactersInRange: range withString: aString];
      goto finish;
    }

  /*
   * Get the attributes to associate with our replacement string.
   * Should be those of the first character replaced.
   * If the range replaced is empty, we use the attributes of the
   * previous character (if possible).
   */
  if (range.length == 0 && range.location > 0)
    start = range.location - 1;
  else
    start = range.location;
  _attributesAtIndexEffectiveRange(start, &effectiveRange,
                                   tmpLength, _infoArray, &arrayIndex);

  moveLocations = [aString length] - range.length;

  arrayIndex++;
  if (NSMaxRange(effectiveRange) < NSMaxRange(range))
    {
      /*
       * Remove all range info for ranges enclosed within the one
       * we are replacing.  Adjust the start point of a range that
       * extends beyond ours.
       */
      info = OBJECTAT(arrayIndex);
      if (info->loc < NSMaxRange(range))
	{
	  unsigned int	next = arrayIndex + 1;

	  while (next < arraySize)
	    {
	      GSTextInfo	*n = OBJECTAT(next);
	      if (n->loc <= NSMaxRange(range))
		{
		  REMOVEAT(arrayIndex);
		  arraySize--;
		  info = n;
		}
	      else
		{
		  break;
		}
	    }
	}
      if (NSMaxRange(range) < [_textChars length])
	{
	  info->loc = NSMaxRange(range);
	}
      else
	{
	  REMOVEAT(arrayIndex);
	  arraySize--;
	}
    }

  /*
   * If we are replacing a range with a zero length string and the
   * range we are using matches the range replaced, then we must
   * remove it from the array to avoid getting a zero length range.
   */
  if ((moveLocations + range.length) == 0)
    {
      _attributesAtIndexEffectiveRange(start, &effectiveRange,
                                       tmpLength, _infoArray, &arrayIndex);
      arrayIndex++;

      if (effectiveRange.location == range.location
	&& effectiveRange.length == range.length)
	{
	  arrayIndex--;
	  if (arrayIndex != 0 || arraySize > 1)
	    {
	      REMOVEAT(arrayIndex);
	      arraySize--;
	    }
	  else
	    {
	      NSDictionary	*d = blank;

	      info = OBJECTAT(0);
	      unCacheAttributes(info->attrs);
	      DESTROY(info->attrs);
	      d = cacheAttributes(d);
	      info->attrs = d;
	      info->loc = NSMaxRange(range);
	    }
	}
    }

  /*
   * Now adjust the positions of the ranges following the one we are using.
   */
  while (arrayIndex < arraySize)
    {
      info = OBJECTAT(arrayIndex);
      info->loc += moveLocations;
      arrayIndex++;
    }
  [_textChars replaceCharactersInRange: range withString: aString];

finish:
SANITY();
  [self edited: NSTextStorageEditedCharacters
         range: range
changeInLength: [aString length] - range.length];
}

- (void) dealloc
{
  RELEASE(_textChars);
  RELEASE(_infoArray);
  RELEASE(_textProxy);
  [super dealloc];
}

// The superclass implementation is correct but too slow
- (NSUInteger) length
{
  return [_textChars length];
}
@end

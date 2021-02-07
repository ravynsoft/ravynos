/** <title>NSTextStorage</title>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: 1999
  
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

#import <Foundation/NSNotification.h>
#import <Foundation/NSException.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSPortCoder.h>
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSTextStorage.h"
#import "AppKit/NSColor.h"
#import "GNUstepGUI/GSLayoutManager.h"
#import "GSTextStorage.h"

@implementation NSTextStorage

static Class abstract;
static Class concrete;

static NSNotificationCenter *nc = nil;

+ (void) initialize
{
  if (self == [NSTextStorage class])
    {
      abstract = self;
      concrete = [GSTextStorage class];
      nc = [NSNotificationCenter defaultCenter];
    }
}

+ (id) allocWithZone: (NSZone*)zone
{
  if (self == abstract)
    return NSAllocateObject(concrete, 0, zone);
  else
    return NSAllocateObject(self, 0, zone);
}

- (void) dealloc
{
  [self setDelegate: nil];
  [_layoutManagers makeObjectsPerformSelector: @selector(setTextStorage:)
				   withObject: nil];
  RELEASE (_layoutManagers);
  [super dealloc];
}

/*
 *	The designated intialiser
 */
- (id) initWithString: (NSString*)aString
           attributes: (NSDictionary*)attributes
{
  _layoutManagers = [[NSMutableArray alloc] initWithCapacity: 2];
  return self;
}

/*
 * Return a string
 */

- (NSString*) string
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/*
 *	Managing GSLayoutManagers
 */
- (void) addLayoutManager: (GSLayoutManager*)obj
{
  if ([_layoutManagers indexOfObjectIdenticalTo: obj] == NSNotFound)
    {
      [_layoutManagers addObject: obj];
      [obj setTextStorage: self];
    }
}

- (void) removeLayoutManager: (GSLayoutManager*)obj
{
  /* If the receiver belongs to a text network that is owned by its text view
     it could get deallocated by the call to -setTextStorage:. To prevent
     crashes we retain the receiver until the end of this method. */
  RETAIN(self);
  [obj setTextStorage: nil];
  [_layoutManagers removeObjectIdenticalTo: obj];
  RELEASE(self);
}

- (NSArray*) layoutManagers
{
  return _layoutManagers;
}

- (void) beginEditing
{
  _editCount++;
}

- (void) endEditing
{
  if (_editCount == 0)
    {
      [NSException raise: NSGenericException
		   format: @"endEditing without corresponding beginEditing"];
    }
  if (--_editCount == 0)
    {
      [self processEditing];
    }
}

/*
 *	If there are no outstanding beginEditing calls, this method calls
 *	processEditing to cause post-editing stuff to happen. This method
 *	has to be called by the primitives after changes are made.
 *	The range argument to edited:... is the range in the original string
 *	(before the edit).
 */
- (void) edited: (unsigned)mask range: (NSRange)old changeInLength: (int)delta
{

  NSDebugLLog(@"NSText", @"edited:range:changeInLength: called");

  /*
   * Extend edited range to encompass the latest edit.
   */
  if (_editedMask == 0)
    {
      _editedRange = old;		// First edit.
    }
  else
    {
      _editedRange = NSUnionRange (_editedRange, old);
    }

  /*
   * Add in any new flags for this edit.
   */
  _editedMask |= mask;

  /*
   * If the number of characters has been increased or decreased -
   * adjust the delta accordingly.
   */
  if ((mask & NSTextStorageEditedCharacters) && delta)
    {
      if (delta < 0)
	{
	  NSAssert (old.length >= (unsigned)-delta, NSInvalidArgumentException);
	}
      _editedRange.length += delta; 
      _editedDelta += delta;
    }

  if (_editCount == 0)
    [self processEditing];
}

/*
 *	This is called from edited:range:changeInLength: or endEditing.
 *	This method sends out NSTextStorageWillProcessEditing, then fixes
 *	the attributes, then sends out NSTextStorageDidProcessEditing,
 *	and finally notifies the layout managers of change with the
 *	textStorage:edited:range:changeInLength:invalidatedRange: method.
 */
- (void) processEditing
{
  NSRange	r;
  int original_delta;
  unsigned int i;
  unsigned length;

  NSDebugLLog(@"NSText", @"processEditing called in NSTextStorage.");

  /*
   * The _editCount gets decreased later again, so that changes by the
   * delegate or by ourselves when we fix attributes dont trigger a
   * new processEditing */
  _editCount++;
  [nc postNotificationName: NSTextStorageWillProcessEditingNotification
		    object: self];

  /* Very important: we save the current _editedRange */
  r = _editedRange;
  original_delta = _editedDelta;
  length = [self length];
  // Multiple adds at the end might give a too long result
  if (NSMaxRange(r) > length)
    {
      r.length = length - r.location;
    }
  
  /* The following call will potentially fix attributes.  These changes 
     are done through NSTextStorage methods, which records the changes 
     by calling edited:range:changeInLength: - which modifies editedRange.
     
     As a consequence, if any attribute has been fixed, r !=
     editedRange after this call.  This is why we saved r in the first
     place. */
  [self invalidateAttributesInRange: r];

  [nc postNotificationName: NSTextStorageDidProcessEditingNotification
                    object: self];
  _editCount--;

  /*
  The attribute fixes might have added or removed characters. We must make
  sure that range and delta we give to the layout managers is valid.
  */
  if (original_delta != _editedDelta)
    {
      if (_editedDelta - original_delta > 0)
	{
	  r.length += _editedDelta - original_delta;
	}
      else
	{
	  if ((unsigned)(original_delta - _editedDelta) > r.length)
	    {
	      r.length = 0;
	      if (r.location > [self length])
		r.location = [self length];
	    }
	  else
	    {
	      r.length += _editedDelta - original_delta;
	    }
	}
    }

  /*
   * Calls textStorage:edited:range:changeInLength:invalidatedRange: for
   * every layoutManager.
   */

  for (i = 0; i < [_layoutManagers count]; i++)
    {
      GSLayoutManager *lManager = [_layoutManagers objectAtIndex: i];

      [lManager textStorage: self  edited: _editedMask  range: r
		changeInLength: _editedDelta  invalidatedRange: _editedRange];
    }

  /*
   * edited values reset to be used again in the next pass.
   */

  _editedRange = NSMakeRange (0, 0);
  _editedDelta = 0;
  _editedMask = 0;
}

/*
 *	These methods return information about the editing status.
 *	Especially useful when there are outstanding beginEditing calls or
 *	during processEditing... editedRange.location will be NSNotFound if
 *	nothing has been edited.
 */       
- (unsigned) editedMask
{
  return _editedMask;
}

- (NSRange) editedRange
{
  return _editedRange;
}

- (int) changeInLength
{
  return _editedDelta;
}

/**
 * Set the delegate (adds it as an observer for text storage notifications)
 * and removes any old value (removes it as an observer).<br />
 * The delegate is <em>not</em> retained.
 */
- (void) setDelegate: (id)delegate
{
  if (_delegate != nil)
    {
      [nc removeObserver: _delegate  name: nil  object: self];
    }
  _delegate = delegate;

#define SET_DELEGATE_NOTIFICATION(notif_name) \
  if ([_delegate respondsToSelector: @selector(textStorage##notif_name:)]) \
    [nc addObserver: _delegate \
	   selector: @selector(textStorage##notif_name:) \
	       name: NSTextStorage##notif_name##Notification object: self]

  SET_DELEGATE_NOTIFICATION(DidProcessEditing);
  SET_DELEGATE_NOTIFICATION(WillProcessEditing);
}

/**
 * Returns the value most recently set usiong the -setDelegate: method.
 */
- (id) delegate
{
  return _delegate;
}

- (void) ensureAttributesAreFixedInRange: (NSRange)range
{
  // Do nothing as the default is not lazy fixing, so all is done already
}

- (BOOL) fixesAttributesLazily
{
  return NO;
}

- (void) invalidateAttributesInRange: (NSRange)range
{
  [self fixAttributesInRange: range];
}

- (Class) classForCoder
{
  return abstract;
}

- (id) replacementObjectForPortCoder: (NSPortCoder*)aCoder
{
  return self;
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if ([aDecoder allowsKeyedCoding])
    {
      id delegate = [aDecoder decodeObjectForKey: @"NSDelegate"];

      [self setDelegate: delegate];
    }
  else
    {
    }
     
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: [self delegate] forKey: @"NSDelegate"];
    }
  else
    {
    }
}

@end

@implementation NSTextStorage (Scripting)

- (NSFont*) font
{
  return [self attribute: NSFontAttributeName atIndex: 0 effectiveRange: NULL];
}

- (void) setFont: (NSFont*)font
{
  if (font != nil)
    {
      [self addAttribute: NSFontAttributeName
		   value: font
		   range: NSMakeRange(0, [self length])];
    }
}


/*
 * The text storage contents as an array of attribute runs.
 */
- (NSArray *)attributeRuns
{
  // Return nothing for now
  return [NSArray array];
}

/*
 * The text storage contents as an array of paragraphs.
 */
- (NSArray *)paragraphs
{
  NSArray *array = [[self string] componentsSeparatedByCharactersInSet:
			   [NSCharacterSet newlineCharacterSet]];
  NSMutableArray *result = [NSMutableArray array];
  NSEnumerator *en = [array objectEnumerator];
  NSString *obj = nil;

  while((obj = [en nextObject]) != nil)
    {
      NSTextStorage *s = AUTORELEASE([[NSTextStorage alloc] initWithString: obj]);
      [result addObject: s];
    }

  return [NSArray arrayWithArray: result]; // make immutable
}

/*
 * The text storage contents as an array of words.
 */
- (NSArray *)words
{
  NSArray *array = [[self string] componentsSeparatedByCharactersInSet:
			   [NSCharacterSet whitespaceCharacterSet]];
  NSMutableArray *result = [NSMutableArray array];
  NSEnumerator *en = [array objectEnumerator];
  NSString *obj = nil;

  while((obj = [en nextObject]) != nil)
    {
      NSTextStorage *s = AUTORELEASE([[NSTextStorage alloc] initWithString: obj]);
      [result addObject: s];
    }

  return [NSArray arrayWithArray: result]; // make immutable
}

/*
 * The text storage contents as an array of characters.
 */
- (NSArray *)characters
{
  NSMutableArray *array = [NSMutableArray array];
  NSUInteger len = [self length];
  NSUInteger i = 0;

  for(i = 0; i < len; i++)
    {
      NSRange r = NSMakeRange(i,1);
      NSString *c = [[self string] substringWithRange: r];
      NSTextStorage *s = AUTORELEASE([[NSTextStorage alloc] initWithString: c]);
      [array addObject: s];
    }

  return [NSArray arrayWithArray: array]; // make immutable
}

/*
 * The font color used when drawing text.
 */
- (NSColor *)foregroundColor
{
  NSRange r = NSMakeRange(0, [self length]);
  NSDictionary *d = [self fontAttributesInRange: r];
  NSColor *c = (NSColor *)[d objectForKey: NSForegroundColorAttributeName];
  return c;
}

@end

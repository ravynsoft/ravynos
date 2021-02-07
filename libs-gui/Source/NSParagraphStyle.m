/** <title>NSParagraphStyle</title>

   <abstract>NSParagraphStyle and NSMutableParagraphStyle hold paragraph style 
     information NSTextTab holds information about a single tab stop</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date March 1999
   
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

#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import "AppKit/NSParagraphStyle.h"

@implementation NSTextTab

- (id) initWithType: (NSTextTabType)type location: (CGFloat)loc
{
  if ((self = [super init]))
    {
      _tabStopType = type;
      _location = loc;
      switch (type)
        {
          default:
          case NSLeftTabStopType:	
            _alignment = NSLeftTextAlignment;
            break;
          case NSRightTabStopType:
            _alignment = NSRightTextAlignment;
            break;
          case NSCenterTabStopType:
            _alignment = NSCenterTextAlignment;
            break;
          case NSDecimalTabStopType:
            _alignment = NSRightTextAlignment;
            break;
        }
    }
  return self;
}

- (id) initWithTextAlignment: (NSTextAlignment)align 
                    location: (CGFloat)loc 
                     options: (NSDictionary *)options
{
  NSTextTabType type;

  switch (align)
    {
      default:
      case NSLeftTextAlignment:	
	type = NSLeftTabStopType; 
	break;
      case NSRightTextAlignment:
	if ([options objectForKey: NSTabColumnTerminatorsAttributeName] != nil)
	  {
	    type = NSDecimalTabStopType;
	  }
	else
	  {
	    type = NSRightTabStopType;
	  }
	break;
      case NSCenterTextAlignment:
	type = NSCenterTabStopType;
	break;
      case NSJustifiedTextAlignment:
	type = NSLeftTabStopType;
	break;
      case NSNaturalTextAlignment:
	// FIXME: Get from language user setting
	type = YES ? NSLeftTabStopType : NSRightTabStopType;
	break;
    }

  if ((self = [self initWithType: type location: loc]))
    {
      _alignment = align;
      ASSIGN(_options, options);
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_options);
  [super dealloc];
}

- (id) copyWithZone: (NSZone*)aZone
{
  NSTextTab *copy;

  if (NSShouldRetainWithZone(self, aZone) == YES)
    return RETAIN(self);

  copy = (NSTextTab *)NSCopyObject(self, 0, aZone);
  copy->_options = [_options copyWithZone: aZone];
  return copy; 
}

- (NSComparisonResult) compare: (id)anObject
{
  float	loc;

  if (anObject == self)
    return NSOrderedSame;
  if (anObject == nil || ([anObject isKindOfClass: object_getClass(self)] == NO))
    return NSOrderedAscending;
  loc = ((NSTextTab*)anObject)->_location;
  if (_location < loc)
    return NSOrderedAscending;
  else if (_location > loc)
    return NSOrderedDescending;
  else
    return NSOrderedSame;
}

- (NSUInteger) hash
{
  NSUInteger val = (NSUInteger)_location;

  val ^= (NSUInteger)_tabStopType;
  return val;
}

- (BOOL) isEqual: (id)anObject
{
  if (anObject == self)
    return YES;
  if ([anObject isKindOfClass: object_getClass(self)] == NO)
    return NO;
  else if (((NSTextTab*)anObject)->_tabStopType != _tabStopType)
    return NO;
  else if (((NSTextTab*)anObject)->_location != _location)
    return NO;
  return YES;
}

- (CGFloat) location
{
  return _location;
}

- (NSTextTabType) tabStopType
{
  return _tabStopType;
}
- (NSTextAlignment) alignment
{
  return _alignment;
}

- (NSDictionary *) options
{
  return _options;
}

- (id) initWithCoder: (NSCoder *)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      _location = [aCoder decodeFloatForKey: @"NSLocation"];
    }
  else
    {
      // FIXME
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeFloat: _location forKey: @"NSLocation"];
    }
  else
    {
      // FIXME
    }
}

@end



@implementation NSParagraphStyle

static NSParagraphStyle	*defaultStyle = nil;

+ (NSParagraphStyle*) defaultParagraphStyle
{
  if (defaultStyle == nil)
    {
      NSParagraphStyle	*style = [[self alloc] init];
      /*
      int		i;

      for (i = 0; i < 12; i++)
        {
          NSTextTab	*tab;

          tab = [[NSTextTab alloc] initWithType: NSLeftTabStopType
                                   location: (i + 1) * 28.0];
          [style->_tabStops addObject: tab];
          RELEASE(tab);
        }
      */
      defaultStyle = style;
    }
  return defaultStyle;
}

+ (void) initialize
{
  if (self == [NSParagraphStyle class])
    {
      /* Set the class version to 2, as the writing direction is now 
	 stored in the encoding */
      [self setVersion: 3];
    }
}

+ (NSWritingDirection) defaultWritingDirectionForLanguage: (NSString*) language
{
  static NSArray *rightToLeft;
  NSWritingDirection writingDirection;
  NSString *langCode = nil;

  /* If language is 5/6 characters long with underscore in the middle,
     treat it as ISO language-region format. */
  if ([language length] == 5 && [language characterAtIndex: 2] == '_')
      langCode = [language substringToIndex: 2];
  else if ([language length] == 6 && [language characterAtIndex: 3] == '_')
      langCode = [language substringToIndex: 3];
  /* Else if it's just two or three chars long, treat as ISO 639 code. */
  else if ([language length] == 2 || [language length] == 3)
    langCode = language;
  
  if (!rightToLeft)
    // Holds languages whose current scripts are written right to left.
    rightToLeft = [[NSArray alloc] initWithObjects: @"ar", @"ara", @"arc",
				   @"chi", @"fa", @"fas", @"he", @"heb", @"iw",
				   @"ji", @"kas", @"ks", @"ku", @"kur", @"pa",
				   @"pan", @"per" @"ps", @"pus", @"sd", @"snd",
				   @"syr", @"tk", @"tmh", @"tuk", @"ug",
				   @"uig", @"ur," @"urd", @"yi", @"yid", @"zh",
				   @"zho", nil];
  if ([rightToLeft containsObject: langCode] == YES)
    writingDirection = NSWritingDirectionRightToLeft;
  else // If it's not RTL, assume LTR.
    writingDirection = NSWritingDirectionLeftToRight;
  
  return writingDirection;  
}

- (void) dealloc
{
  if (self == defaultStyle)
    {
      NSLog(@"Argh - attempt to dealloc the default paragraph style!");
      return;
    }
  RELEASE(_tabStops);
  RELEASE(_textBlocks);
  RELEASE(_textLists);
  [super dealloc];
}

- (id) init
{
  if ((self = [super init]))
    {
      _alignment = NSNaturalTextAlignment;
      //_firstLineHeadIndent = 0.0;
      //_headIndent = 0.0;
      _lineBreakMode = NSLineBreakByWordWrapping;
      //_lineSpacing = 0.0;
      //_maximumLineHeight = 0.0;
      //_minimumLineHeight = 0.0;
      //_paragraphSpacing = 0.0;
      //_tailIndent = 0.0;
      _baseDirection = NSWritingDirectionNaturalDirection;
      _tabStops = [[NSMutableArray allocWithZone: [self zone]] 
                      initWithCapacity: 12];
    }
  return self;
}

- (NSString *)description
{
  return [NSString stringWithFormat: 
                     @"%@ Alignment: %ld LineSpacing: %f ParagraphSpacing: %f LineBreakMode: %ld", 
                   [super description], (long)_alignment, (float)_lineSpacing,
                   (float)_paragraphSpacing, (long)_lineBreakMode];
}

/*
 *      "Leading": distance between the bottom of one line fragment and top
 *      of next (applied between lines in the same container).
 *      Can't be negative. This value is included in the line fragment
 *      heights in layout manager.
 */
- (CGFloat) lineSpacing
{
  return _lineSpacing;
}

/*
 *      Distance between the bottom of this paragraph and top of next.
 */
- (CGFloat) paragraphSpacing
{
  return _paragraphSpacing;
}

- (NSTextAlignment) alignment
{
  return _alignment;
}

/*
 *      The following values are relative to the appropriate margin
 *      (depending on the paragraph direction)
 */

/*
 *      Distance from margin to front edge of paragraph
 */
- (CGFloat) headIndent
{
  return _headIndent;
}

/*
 *      Distance from margin to back edge of paragraph; if negative or 0,
 *      from other margin
 */
- (CGFloat) tailIndent
{
  return _tailIndent;
}

/*
 *      Distance from margin to edge appropriate for text direction
 */
- (CGFloat) firstLineHeadIndent
{
  return _firstLineHeadIndent;
}

/*
 *      Distance from margin to tab stops
 */
- (NSArray *) tabStops
{
  return AUTORELEASE ([_tabStops copyWithZone: NSDefaultMallocZone ()]);
}

/*
 *      Line height is the distance from bottom of descenders to to
 *      of ascenders; basically the line fragment height. Does not include
 *      lineSpacing (which is added after this computation).
 */
- (CGFloat) minimumLineHeight
{
  return _minimumLineHeight;
}

/*
 *      0 implies no maximum.
 */
- (CGFloat) maximumLineHeight
{
  return _maximumLineHeight;
} 

- (NSLineBreakMode) lineBreakMode
{
  return _lineBreakMode;
}

- (NSWritingDirection) baseWritingDirection
{
  return _baseDirection;
}

- (CGFloat) defaultTabInterval
{
  return _defaultTabInterval;
}

- (CGFloat) lineHeightMultiple
{
  return _lineHeightMultiple;
}

- (CGFloat) paragraphSpacingBefore
{
  return _paragraphSpacingBefore;
}

- (NSInteger) headerLevel
{
  return _headerLevel;
}

- (float) hyphenationFactor
{
  return _hyphenationFactor;
}

- (NSArray *) textBlocks
{
  return _textBlocks;
}

- (NSArray *) textLists
{
  return _textLists;
}

- (float) tighteningFactorForTruncation
{
  return _tighteningFactorForTruncation;
}

- (id) copyWithZone: (NSZone*)aZone
{
  if (NSShouldRetainWithZone (self, aZone) == YES)
    return RETAIN (self);
  else
    {
      NSParagraphStyle	*c;

      c = (NSParagraphStyle*)NSCopyObject (self, 0, aZone);
      c->_textBlocks = [_textBlocks mutableCopyWithZone: aZone];
      c->_textLists = [_textLists mutableCopyWithZone: aZone];
      return c;
    }
}

- (id) mutableCopyWithZone: (NSZone*)aZone
{
  NSMutableParagraphStyle	*c;

  c = [[NSMutableParagraphStyle allocWithZone: aZone] init];
  [c setParagraphStyle: self];
  return c;
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      _firstLineHeadIndent = [aCoder decodeFloatForKey: @"NSFirstLineHeadIndent"];
      _headIndent = [aCoder decodeFloatForKey: @"NSHeadIndent"];
      _paragraphSpacing = [aCoder decodeFloatForKey: @"NSParagraphSpacingBefore"];
      ASSIGN(_tabStops, [aCoder decodeObjectForKey: @"NSTabStops"]);
      ASSIGN(_textLists, [aCoder decodeObjectForKey: @"NSTextLists"]);
      _baseDirection = [aCoder decodeIntForKey: @"NSWritingDirection"];
    }
  else
    {
      NSUInteger count;
      
      [aCoder decodeValueOfObjCType: @encode(NSInteger) at: &_alignment];
      [aCoder decodeValueOfObjCType: @encode(NSInteger) at: &_lineBreakMode];
      [aCoder decodeValueOfObjCType: @encode(float) at: &_firstLineHeadIndent];
      [aCoder decodeValueOfObjCType: @encode(float) at: &_headIndent];
      [aCoder decodeValueOfObjCType: @encode(float) at: &_lineSpacing];
      [aCoder decodeValueOfObjCType: @encode(float) at: &_maximumLineHeight];
      [aCoder decodeValueOfObjCType: @encode(float) at: &_minimumLineHeight];
      [aCoder decodeValueOfObjCType: @encode(float) at: &_paragraphSpacing];
      [aCoder decodeValueOfObjCType: @encode(float) at: &_tailIndent];
      
      /*
       *	Tab stops don't conform to NSCoding - so we do it the long way.
       */
      [aCoder decodeValueOfObjCType: @encode(NSUInteger) at: &count];
      _tabStops = [[NSMutableArray alloc] initWithCapacity: count];
      if (count > 0)
        {
          float locations[count];
          NSTextTabType types[count];
          NSUInteger i;
          
          [aCoder decodeArrayOfObjCType: @encode(float)
                  count: count
                  at: locations];
          if ([aCoder versionForClassName: @"NSParagraphStyle"] >= 3)
            {
              [aCoder decodeArrayOfObjCType: @encode(NSInteger)
                  count: count
                  at: types];
	    }
	  else
            {
              [aCoder decodeArrayOfObjCType: @encode(int)
                  count: count
                  at: types];
	    }
          for (i = 0; i < count; i++)
            {
              NSTextTab	*tab;
              
              tab = [[NSTextTab alloc] initWithType: types[i] 
                                       location: locations[i]];
              [_tabStops addObject: tab];
              RELEASE(tab);
            }
        }
      
      if ([aCoder versionForClassName: @"NSParagraphStyle"] >= 2)
        {
          [aCoder decodeValueOfObjCType: @encode(NSInteger) at: &_baseDirection];
        }
    }

  return self;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeFloat: _firstLineHeadIndent forKey: @"NSFirstLineHeadIndent"];
      [aCoder encodeFloat: _headIndent forKey: @"NSHeadIndent"];
      [aCoder encodeFloat: _paragraphSpacing forKey: @"NSParagraphSpacingBefore"];
      [aCoder encodeObject: _tabStops forKey: @"NSTabStops"];
      [aCoder encodeObject: _textLists forKey: @"NSTextLists"];
      [aCoder encodeInt: _baseDirection forKey: @"NSWritingDirection"];
    }
  else
    {
      NSUInteger count;
      
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_alignment];
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_lineBreakMode];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_firstLineHeadIndent];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_headIndent];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_lineSpacing];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_maximumLineHeight];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_minimumLineHeight];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_paragraphSpacing];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_tailIndent];
      
      /*
       *	Tab stops don't conform to NSCoding - so we do it the long way.
       */
      count = [_tabStops count];
      [aCoder encodeValueOfObjCType: @encode(NSUInteger) at: &count];
      if (count > 0)
        {
          float locations[count];
          NSTextTabType types[count];
          NSUInteger i;
          
          for (i = 0; i < count; i++)
            {
              NSTextTab	*tab = [_tabStops objectAtIndex: i];
              
              locations[i] = [tab location]; 
              types[i] = [tab tabStopType]; 
            }
          [aCoder encodeArrayOfObjCType: @encode(float)
                  count: count
                  at: locations];
          [aCoder encodeArrayOfObjCType: @encode(NSInteger)
                  count: count
                  at: types];
        }
      
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_baseDirection];
    }
}

- (BOOL) isEqual: (id)aother
{
  NSParagraphStyle *other = aother;
  if (other == self)
    return YES;
  if ([other isKindOfClass: [NSParagraphStyle class]] == NO)
    return NO;

#define C(x) if (x != other->x) return NO
  C(_lineSpacing);
  C(_paragraphSpacing);
  C(_headIndent);
  C(_tailIndent);
  C(_firstLineHeadIndent);
  C(_minimumLineHeight);
  C(_maximumLineHeight);
  C(_alignment);
  C(_lineBreakMode);
  C(_paragraphSpacingBefore);
  C(_defaultTabInterval);
  C(_hyphenationFactor);
  C(_lineHeightMultiple);
  C(_tighteningFactorForTruncation);
  C(_headerLevel);
#undef C

  return [_tabStops isEqualToArray: other->_tabStops];
}

- (NSUInteger) hash
{
  return _alignment + _lineBreakMode;
}


@end



@implementation NSMutableParagraphStyle 

+ (NSParagraphStyle*) defaultParagraphStyle
{
  return AUTORELEASE ([[NSParagraphStyle defaultParagraphStyle] mutableCopy]);
}

- (void) setLineSpacing: (CGFloat)aFloat
{
  NSAssert (aFloat >= 0.0, NSInvalidArgumentException);
  _lineSpacing = aFloat;
}

- (void) setParagraphSpacing: (CGFloat)aFloat
{
  NSAssert (aFloat >= 0.0, NSInvalidArgumentException);
  _paragraphSpacing = aFloat;
}

- (void) setAlignment: (NSTextAlignment)newAlignment
{
  _alignment = newAlignment;
}

- (void) setFirstLineHeadIndent: (CGFloat)aFloat
{
  NSAssert (aFloat >= 0.0, NSInvalidArgumentException);
  _firstLineHeadIndent = aFloat;
}

- (void) setHeadIndent: (CGFloat)aFloat
{
  NSAssert (aFloat >= 0.0, NSInvalidArgumentException);
  _headIndent = aFloat;
}

- (void) setTailIndent: (CGFloat)aFloat
{
  _tailIndent = aFloat;
}

- (void) setLineBreakMode: (NSLineBreakMode)mode
{
  _lineBreakMode = mode;
}

- (void) setMinimumLineHeight: (CGFloat)aFloat
{
  NSAssert (aFloat >= 0.0, NSInvalidArgumentException);
  _minimumLineHeight = aFloat;
}

- (void) setMaximumLineHeight: (CGFloat)aFloat
{
  NSAssert (aFloat >= 0.0, NSInvalidArgumentException);
  _maximumLineHeight = aFloat;
}

- (void) setBaseWritingDirection: (NSWritingDirection)direction
{
  /*
   * FIXME there is some confusion regarding natural writing direction.
   * 
   * this method is documented as setting
   * NSWritingDirectionLeftToRight or NSWritingDirectionRightToLeft
   * based on the users language preferences.
   * when encountering NSWritingDirectionNaturalDirection
   *
   * NSWritingDirectionNatural constant is documented as using the
   * unicode bidi algorithm. 
   *
   * no idea what the constant name or behaviour actually is.
   */
  _baseDirection = direction;
}

- (void) setDefaultTabInterval: (CGFloat)interval
{
  _defaultTabInterval = interval;
}

- (void) setLineHeightMultiple: (CGFloat)factor
{
  _lineHeightMultiple = factor;
}

- (void) setParagraphSpacingBefore: (CGFloat)spacing
{
  _paragraphSpacingBefore = spacing;
}

- (void) setHeaderLevel: (NSInteger)level
{
  _headerLevel = level;
}

- (void) setHyphenationFactor: (float)factor
{
  _hyphenationFactor = factor;
}

- (void) setTextBlocks: (NSArray *)blocks
{
  ASSIGN(_textBlocks, blocks);
}

- (void) setTextLists: (NSArray *)lists
{
  ASSIGN(_textLists, lists);
}

- (void) setTighteningFactorForTruncation: (float)factor
{
  _tighteningFactorForTruncation = factor;
}

- (void) addTabStop: (NSTextTab*)anObject
{
  NSUInteger count = [_tabStops count];

  if (count == 0)
    {
      [_tabStops addObject: anObject];
    }
  else
    {
      while (count-- > 0)
	{
	  NSTextTab *tab;

	  tab = [_tabStops objectAtIndex: count];
	  if ([tab compare: anObject] != NSOrderedDescending)
	    {
	      [_tabStops insertObject: anObject atIndex: count + 1];
	      return;
	    }
	}
      [_tabStops insertObject: anObject atIndex: 0];
    }
}

- (void) removeTabStop: (NSTextTab*)anObject
{
  NSUInteger i = [_tabStops indexOfObject: anObject];

  if (i != NSNotFound)
    [_tabStops removeObjectAtIndex: i];
}

- (void) setTabStops: (NSArray *)array
{
  if (array != _tabStops)
    {
      [_tabStops removeAllObjects];
      [_tabStops addObjectsFromArray: array];
      [_tabStops sortUsingSelector: @selector(compare:)];
    }
}

- (void) setParagraphStyle: (NSParagraphStyle*)obj
{
  NSMutableParagraphStyle *p = (NSMutableParagraphStyle*)obj;

  if (p == self)
    return;

  /* Can add tab stops without sorting as we know they are already sorted. */
  [_tabStops removeAllObjects];
  [_tabStops addObjectsFromArray: p->_tabStops];

  if (p->_textBlocks)
    [self setTextBlocks: p->_textBlocks];
  if (p->_textLists)
    [self setTextLists: p->_textLists];

  _alignment = p->_alignment;
  _firstLineHeadIndent = p->_firstLineHeadIndent;
  _headIndent = p->_headIndent;
  _lineBreakMode = p->_lineBreakMode;
  _lineSpacing = p->_lineSpacing;
  _maximumLineHeight = p->_maximumLineHeight;
  _minimumLineHeight = p->_minimumLineHeight;
  _paragraphSpacing = p->_paragraphSpacing;
  _tailIndent = p->_tailIndent;
  _baseDirection = p->_baseDirection;
  _paragraphSpacingBefore = p->_paragraphSpacingBefore;
  _defaultTabInterval = p->_defaultTabInterval;
  _hyphenationFactor = p->_hyphenationFactor;
  _lineHeightMultiple = p->_lineHeightMultiple;
  _tighteningFactorForTruncation = p->_tighteningFactorForTruncation;
  _headerLevel = p->_headerLevel;
}

- (id) copyWithZone: (NSZone*)aZone
{
  NSMutableParagraphStyle *c;

  c = (NSMutableParagraphStyle*)NSCopyObject (self, 0, aZone);
  GSClassSwizzle(c, [NSParagraphStyle class]);
  c->_tabStops = [_tabStops mutableCopyWithZone: aZone];
  c->_textBlocks = [_textBlocks mutableCopyWithZone: aZone];
  c->_textLists = [_textLists mutableCopyWithZone: aZone];
  return c;
}

@end

/** <title>NSSecureTextField</title>

   <abstract>Secure Text field control class for hidden text entry</abstract>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Gregory John Casamento <greg.casamento@gmail.com>
   Date: 2000

   Author: Nicola Pero <nicola@brainstorm.co.uk>
   Date: October 2002

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

#import "config.h"
#import <Foundation/NSException.h>

#import "AppKit/NSAttributedString.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSGlyphGenerator.h"
#import "AppKit/NSLayoutManager.h"
#import "AppKit/NSSecureTextField.h"
#import "AppKit/NSStringDrawing.h"
#import "AppKit/NSTextContainer.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSWindow.h"
#import "GNUstepGUI/GSFontInfo.h"

// the Unicode code point for a bullet
#define BULLET 0x2022

/* 'Secure' subclasses */
@interface NSSecureTextView : NSTextView
{
}
- (void) setEchosBullets:(BOOL)flag;
- (BOOL) echosBullets;
@end

@interface GSSimpleSecureGlyphGenerator : NSGlyphGenerator
@end

@interface NSGlyphGenerator (Private)
- (NSFont *) fontForCharactersWithAttributes: (NSDictionary *)attributes;
@end

@interface GSSimpleSecureLayoutManager : NSLayoutManager
{
  BOOL _echosBullets;
}
- (void) setEchosBullets:(BOOL)flag;
- (BOOL) echosBullets;
@end

@implementation NSSecureTextField

+ (void) initialize
{
  if (self == [NSSecureTextField class])
    {
      [self setVersion:2];
    }
}

+ (Class) cellClass
{
  /* Hard code here to make sure no other class is used.  */
  return [NSSecureTextFieldCell class];
}

+ (void) setCellClass: (Class)factoryId
{
  /* Ward off interlopers with a stern message.  */
  [NSException raise: NSInvalidArgumentException
	       format: @"NSSecureTextField only uses NSSecureTextFieldCells."];
}

- (id) initWithCoder: (NSCoder *)coder
{
  if((self = [super initWithCoder: coder]) != nil)
    {
      [self setEchosBullets: YES];
    }
  return self;
}

- (id) initWithFrame:(NSRect)frameRect
{
  self = [super initWithFrame: frameRect];
  if (nil == self)
    return nil;

  [self setEchosBullets: YES];

  return self;
}

- (void) setEchosBullets: (BOOL)flag
{
  [_cell setEchosBullets: flag];
}

- (BOOL) echosBullets
{
  return [_cell echosBullets];
}
@end /* NSSecureTextField */

@implementation NSSecureTextFieldCell

+ (void)initialize
{
  if (self == [NSSecureTextFieldCell class])
    {
      [self setVersion:2];
    }
}

- (BOOL) echosBullets
{
  return _echosBullets;
}

/* Functionality not implemented.  */
- (void) setEchosBullets: (BOOL)flag
{
  _echosBullets = flag;
}

/* Substitute a fixed-pitch font for correct bullet drawing */
- (void) setFont: (NSFont *) f
{
  if (![f isFixedPitch])
    {
      f = [NSFont userFixedPitchFontOfSize: [f pointSize]];
    }

  [super setFont: f];
}

- (NSAttributedString *)_replacementAttributedString
{
  NSDictionary *attributes;
  NSMutableString *string;
  unsigned int length;
  unsigned int i;
  unichar *buf;

  length = [[self stringValue] length];
  buf = NSZoneMalloc (NSDefaultMallocZone (), length * sizeof (unichar));
  for (i = 0; i < length; i++)
    {
      buf[i] = BULLET;
    }

  string = [[NSMutableString alloc]
    initWithCharactersNoCopy: buf length: length freeWhenDone: YES];
  AUTORELEASE (string);

  attributes = [self _nonAutoreleasedTypingAttributes];
  return AUTORELEASE([[NSAttributedString alloc] initWithString: string 
                                                 attributes: attributes]);
}

- (NSAttributedString*) _drawAttributedString
{
  if (_echosBullets)
    {
      if (!_cell.is_disabled)
        {
          return [self _replacementAttributedString];
        }
      else
        {
          NSAttributedString *attrStr = [self _replacementAttributedString];
          NSDictionary *attribs;
          NSMutableDictionary *newAttribs;
          
          attribs = [attrStr attributesAtIndex: 0 
                             effectiveRange: NULL];
          newAttribs = [NSMutableDictionary 
                           dictionaryWithDictionary: attribs];
          [newAttribs setObject: [NSColor disabledControlTextColor]
                      forKey: NSForegroundColorAttributeName];
          
          return AUTORELEASE([[NSAttributedString alloc]
                                 initWithString: [attrStr string]
                                 attributes: newAttribs]);
        }
    }
  else
    {
      /* .. do nothing.  */
      return nil;
    }
}

- (NSText *) setUpFieldEditorAttributes: (NSText *)textObject
{
  NSSecureTextView *secureView;

  /* Replace the text object with a secure instance.  It's not shared.  */
  secureView = AUTORELEASE([[NSSecureTextView alloc] init]);

  [secureView setEchosBullets: [self echosBullets]];
  return [super setUpFieldEditorAttributes: secureView];
}

- (id) initWithCoder: (NSCoder *)decoder
{
  self = [super initWithCoder: decoder];
  if (nil == self)
    return nil;

  if ([decoder allowsKeyedCoding])
    {
      _echosBullets = [decoder decodeBoolForKey: @"GSEchoBullets"];
    }
  else
    {
      [decoder decodeValueOfObjCType: @encode(BOOL) at: &_echosBullets];
    }

  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder: coder];
  if([coder allowsKeyedCoding])
    {
      [coder encodeBool: _echosBullets forKey: @"GSEchoBullets"];
    }
  else
    {
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_echosBullets];
    }
}
@end

@implementation GSSimpleSecureGlyphGenerator
- (void) generateGlyphsForGlyphStorage: (id <NSGlyphStorage>)storage
             desiredNumberOfCharacters: (NSUInteger)num
                            glyphIndex: (NSUInteger*)glyph
                        characterIndex: (NSUInteger*)index
{
  NSGlyph glyphs[num];
  NSGlyph gl;
  NSAttributedString *attrstr = [storage attributedString];
  GSFontInfo *fi;
  int i;
  NSRange maxRange = NSMakeRange(*index, num);
  NSRange curRange;
  NSDictionary *attributes;

  attributes = [attrstr attributesAtIndex: *index
                        longestEffectiveRange: &curRange
                        inRange: maxRange];
  fi = [[self fontForCharactersWithAttributes: attributes] fontInfo];
  if (!fi)
    {
      [NSException raise: NSGenericException
                   format: @"Glyph generation with no font."];
      return;
    }

  gl = [fi glyphForCharacter: BULLET];
  for (i = 0; i < num; i++)
    {
      glyphs[i] = gl;
    }

  [storage insertGlyphs: glyphs
                 length: num
           forStartingGlyphAtIndex: *glyph
         characterIndex: *index];
}

@end

@implementation GSSimpleSecureLayoutManager

- (BOOL) echosBullets
{
  return _echosBullets;
}

- (void) setEchosBullets: (BOOL)flag
{
  _echosBullets = flag;
}

- (void) drawGlyphsForGlyphRange: (NSRange)glyphRange
                         atPoint: (NSPoint)containerOrigin
{
  if ([self echosBullets])
    {
      [super drawGlyphsForGlyphRange: glyphRange
                             atPoint: containerOrigin];
    }
  else
    {
      /* Do nothing.  */
    }
}

@end

@implementation NSSecureTextView

- (id) initWithFrame: (NSRect)frameRect
       textContainer: (NSTextContainer*)aTextContainer
{
  GSSimpleSecureLayoutManager *m;
  GSSimpleSecureGlyphGenerator *g;

  /* Perform the normal init.  */
  self = [super initWithFrame: frameRect  textContainer: aTextContainer];
  if (nil == self)
    return nil;

  /* Then, replace the layout manager with a
   * GSSimpleSecureLayoutManager.  */
  m = [[GSSimpleSecureLayoutManager alloc] init];
  g = [[GSSimpleSecureGlyphGenerator alloc] init];
  [m setGlyphGenerator: g];
  RELEASE(g);
  [[self textContainer] replaceLayoutManager: m];
  RELEASE(m);

  [self setFieldEditor: YES];

  return self;
}

- (BOOL) echosBullets
{
  return [(GSSimpleSecureLayoutManager*)[self layoutManager] echosBullets];
}

- (void) setEchosBullets: (BOOL)flag
{
  [(GSSimpleSecureLayoutManager*)[self layoutManager] setEchosBullets: flag];
}

- (BOOL) writeSelectionToPasteboard: (NSPasteboard*)pboard
			      types: (NSArray*)types
{
  /* Return NO since the selection should never be written to the
   * pasteboard */
  return NO;
}

- (id) validRequestorForSendType: (NSString*) sendType
                      returnType: (NSString*) returnType
{
  /* Return nil to indicate that no type can be sent to the pasteboard
   * for an object of this class.  */
  return nil;
}
@end

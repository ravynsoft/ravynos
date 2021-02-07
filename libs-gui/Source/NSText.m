/** <title>NSText</title>

   <abstract>The RTFD text class</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: July 1998
   Author:  Daniel Boehringer <boehring@biomed.ruhr-uni-bochum.de>
   Date: August 1998
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2000
   Reorganised and cleaned up code, added some action methods
   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: December 2000
   Made class abstract, moved most code to NSTextView.

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

#import "AppKit/NSText.h"

#import "AppKit/NSAttributedString.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSSpellChecker.h"
#import "AppKit/NSTextView.h"


static	Class	abstract;
static	Class	concrete;

@implementation NSText

/*
 * Class methods
 */
+ (void)initialize
{
  if (self  == [NSText class])
    {
      [self setVersion: 1];

      abstract = self;
      concrete = [NSTextView class];
    }
}

+ (id) allocWithZone: (NSZone*)zone
{
  if (self == abstract)
    return NSAllocateObject (concrete, 0, zone);
  else
    return NSAllocateObject (self, 0, zone);
}

/*
 * Instance methods
 */

/*
 * Getting and Setting Contents
 */
- (void) replaceCharactersInRange: (NSRange)aRange
                          withRTF: (NSData *)rtfData
{
  NSAttributedString *attr;

  attr = [[NSAttributedString alloc] initWithRTF: rtfData 
				     documentAttributes: NULL];
  AUTORELEASE (attr);
  [self replaceCharactersInRange: aRange  withAttributedString: attr];
}

- (void) replaceCharactersInRange: (NSRange)aRange  
			 withRTFD: (NSData *)rtfdData
{
  NSAttributedString *attr;

  attr = [[NSAttributedString alloc] initWithRTFD: rtfdData 
				     documentAttributes: NULL];
  AUTORELEASE (attr);
  [self replaceCharactersInRange: aRange  withAttributedString: attr];
}

/* PRIMITIVE */
- (void) replaceCharactersInRange: (NSRange)aRange
		       withString: (NSString*)aString
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) replaceCharactersInRange: (NSRange)aRange
             withAttributedString: (NSAttributedString*)attrString
{
  [self subclassResponsibility: _cmd];
}


/* PRIMITIVE */
- (NSData*) RTFDFromRange: (NSRange)aRange
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/* PRIMITIVE */
- (NSData*) RTFFromRange: (NSRange)aRange
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/* TODO: is this correct? seems that this would use the attributes of the
old text. */
- (void) setString: (NSString*)aString
{
  [self replaceCharactersInRange: NSMakeRange (0, [self textLength])
	withString: aString];
}

/* PRIMITIVE */
- (NSString*) string
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/*
 * old OpenStep methods doing the same
 */
- (void) replaceRange: (NSRange)aRange withRTFD: (NSData*)rtfdData
{
  [self replaceCharactersInRange: aRange withRTFD: rtfdData];
}

- (void) replaceRange: (NSRange)aRange withRTF: (NSData*)rtfData
{
  [self replaceCharactersInRange: aRange withRTF: rtfData];
}

- (void) replaceRange: (NSRange)aRange withString: (NSString*)aString
{
  [self replaceCharactersInRange: aRange withString: aString];
}

- (void) setText: (NSString*)aString range: (NSRange)aRange
{
  [self replaceCharactersInRange: aRange withString: aString];
}

- (void) setText: (NSString*)aString
{
  [self setString: aString];
}

- (NSString*) text
{
  return [self string];
}

/*
 * Graphic attributes
 */

/* PRIMITIVE */
- (NSColor*) backgroundColor
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/* PRIMITIVE */
- (BOOL) drawsBackground
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/* PRIMITIVE */
- (void) setBackgroundColor: (NSColor*)color
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) setDrawsBackground: (BOOL)flag
{
  [self subclassResponsibility: _cmd];
}

/*
 * Managing Global Characteristics
 */
/* PRIMITIVE */
- (BOOL) importsGraphics
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/* PRIMITIVE */
- (BOOL) isEditable
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/* PRIMITIVE */
- (BOOL) isFieldEditor
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/* PRIMITIVE */
- (BOOL) isRichText
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/* PRIMITIVE */
- (BOOL) isSelectable
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/* PRIMITIVE */
- (void) setEditable: (BOOL)flag
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) setFieldEditor: (BOOL)flag
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) setImportsGraphics: (BOOL)flag
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) setRichText: (BOOL)flag
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void)setSelectable: (BOOL)flag
{
  [self subclassResponsibility: _cmd];
}

/*
 * Using the font panel
 */
/* PRIMITIVE */
- (BOOL) usesFontPanel
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/* PRIMITIVE */
- (void) setUsesFontPanel: (BOOL)flag
{
  [self subclassResponsibility: _cmd];
}

/*
 * Managing the Ruler
 */
/* PRIMITIVE */
- (BOOL) isRulerVisible
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/* PRIMITIVE */
- (void) toggleRuler: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/*
 * Managing the Selection
 */
/* PRIMITIVE */
- (NSRange) selectedRange
{
  [self subclassResponsibility: _cmd];
  return NSMakeRange (NSNotFound, 0);
}

/* PRIMITIVE */
- (void) setSelectedRange: (NSRange)range
{
  [self subclassResponsibility: _cmd];
}

/*
 * Copy and paste
 */
/* PRIMITIVE */
- (void) copy: (id)sender
{  
  [self subclassResponsibility: _cmd];
}

/* Copy the current font to the font pasteboard */
/* PRIMITIVE */
- (void) copyFont: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/* Copy the current ruler settings to the ruler pasteboard */
/* PRIMITIVE */
- (void) copyRuler: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) delete: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) cut: (id)sender
{
  [self copy: sender];
  [self delete: sender];
}

/* PRIMITIVE */
- (void) paste: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) pasteFont: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) pasteRuler: (id)sender
{
  [self subclassResponsibility: _cmd];
}

- (void) selectAll: (id)sender
{
  [self setSelectedRange: NSMakeRange (0, [self textLength])];
}

/*
 * Managing Font
 */
/* PRIMITIVE */
- (NSFont*) font
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/*
 * This action method changes the font of the selection for a rich
 * text object, or of all text for a plain text object. If the
 * receiver doesn't use the Font Panel, however, this method does
 * nothing.  */
/* PRIMITIVE */
- (void) changeFont: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) setFont: (NSFont*)font
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) setFont: (NSFont*)font  range: (NSRange)aRange
{
  [self subclassResponsibility: _cmd];
}

- (void) setFont: (NSFont*)font  ofRange: (NSRange)aRange
{
  [self setFont: font  range: aRange];
}

/*
 * Managing Alingment
 */
/* PRIMITIVE */
- (NSTextAlignment) alignment
{
  [self subclassResponsibility: _cmd];
  return 0;
}

/* PRIMITIVE */
- (void) setAlignment: (NSTextAlignment)mode
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) alignCenter: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) alignLeft: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) alignRight: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/*
 * Text colour
 */
/* PRIMITIVE */
- (NSColor*) textColor
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/* PRIMITIVE */
- (void) setTextColor: (NSColor*)color
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) setTextColor: (NSColor*)color  range: (NSRange)aRange
{
  [self subclassResponsibility: _cmd];
}

/* Old OpenStep method to do the same */
- (void) setColor: (NSColor*)color  ofRange: (NSRange)aRange
{
  [self setTextColor: color  range: aRange];
}

/*
 * Text attributes
 */
/* PRIMITIVE */
- (void) subscript: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) superscript: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) unscript: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) underline: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/*
 * Reading and Writing RTFD Files
 */
/* PRIMITIVE */
- (BOOL) readRTFDFromFile: (NSString*)path
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/* PRIMITIVE */
- (BOOL) writeRTFDToFile: (NSString*)path  atomically: (BOOL)flag
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/*
 * Sizing the Frame Rectangle
 */
/* PRIMITIVE */
- (BOOL) isHorizontallyResizable
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/* PRIMITIVE */
- (BOOL) isVerticallyResizable
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/* PRIMITIVE */
- (NSSize) maxSize
{
  [self subclassResponsibility: _cmd];
  return NSMakeSize(0,0);
}

/* PRIMITIVE */
- (NSSize) minSize
{
  [self subclassResponsibility: _cmd];
  return NSMakeSize(0,0);
}

/* PRIMITIVE */
- (void) setHorizontallyResizable: (BOOL)flag
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) setVerticallyResizable: (BOOL)flag
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) setMaxSize: (NSSize)newMaxSize
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) setMinSize: (NSSize)newMinSize
{
  [self subclassResponsibility: _cmd];
}

/* PRIMITIVE */
- (void) sizeToFit
{
  [self subclassResponsibility: _cmd];
}

/*
 * Spelling
 */

/* PRIMITIVE */
- (void) checkSpelling: (id)sender
{
  [self subclassResponsibility: _cmd];
}

- (void) showGuessPanel: (id)sender
{
  NSSpellChecker *sp = [NSSpellChecker sharedSpellChecker];

  [[sp spellingPanel] orderFront: self];
}

/*
 * Scrolling
 */
/* PRIMITIVE */
- (void) scrollRangeToVisible: (NSRange)aRange
{
  [self subclassResponsibility: _cmd];
}

/*
 * Managing the Delegate
 */
/* PRIMITIVE */
- (id) delegate
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/* PRIMITIVE */
- (void) setDelegate: (id)anObject
{
  [self subclassResponsibility: _cmd];
}


/*
 * NSChangeSpelling protocol
 */
/* PRIMITIVE */
- (void) changeSpelling: (id)sender
{
  [self subclassResponsibility: _cmd];
}

/*
 * NSIgnoreMisspelledWords protocol
 */
/* PRIMITIVE */
- (void) ignoreSpelling: (id)sender
{
  [self subclassResponsibility: _cmd];
}

@end

@implementation NSText (GNUstepExtensions)

/* PRIMITIVE */
- (NSUInteger) textLength
{
  [self subclassResponsibility: _cmd];
  return 0;
}

@end

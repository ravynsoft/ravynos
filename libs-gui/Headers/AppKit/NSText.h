/* -*-objc-*-
   NSText.h

   The text object

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: July 1998
   Author: Daniel Böhringer <boehring@biomed.ruhr-uni-bochum.de>
   Date: August 1998
   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: December 2000

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

#ifndef _GNUstep_H_NSText
#define _GNUstep_H_NSText

/*
 * The NSText class is now an abstract class.  When you allocate an
 * instance of NSText, an instance of NSTextView is always allocated
 * instead.
 *
 * But you can still subclass NSText to implement your own text
 * editing class not derived from NSTextView.  NSText declares general
 * methods that a text editing object should have; it has some helper
 * methods which are simple wrappers around the real basic editing
 * methods.  The real editing methods are not implemented in NSText,
 * which is why it is abstract. To make a working subclass, you need to
 * implement these methods, marked as "PRIMITIVE" below.
 *
 * The working subclass could potentially be implemented in absolutely
 * *any* way you want.  I have been told that some versions of Emacs can
 * be embedded as an X subwindow inside alien widgets and windows - so
 * yes, potentially if you are able to figure out how to embed Emacs
 * inside the GNUstep NSView tree, you can write a subclass of NSText
 * which just uses Emacs. */
 
#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>
#import <AppKit/NSView.h>
#import <AppKit/NSSpellProtocol.h>
//#import <AppKit/NSStringDrawing.h>

@class NSAttributedString;
@class NSData;
@class NSNotification;
@class NSString;
@class NSColor;
@class NSFont;

typedef NSInteger NSTextAlignment;
enum {
  NSLeftTextAlignment = 0,
  NSRightTextAlignment,
  NSCenterTextAlignment,
  NSJustifiedTextAlignment,
  NSNaturalTextAlignment
};

#if OS_API_VERSION(MAC_OS_X_VERSION_10_11, GS_API_LATEST)
enum {
  NSTextAlignmentLeft = NSLeftTextAlignment,
  NSTextAlignmentRight = NSRightTextAlignment,
  NSTextAlignmentCenter = NSCenterTextAlignment,
  NSTextAlignmentJustified = NSJustifiedTextAlignment,
  NSTextAlignmentNatural = NSNaturalTextAlignment
};
#endif

enum {
  NSTextWritingDirectionEmbedding = (0 << 1),
  NSTextWritingDirectionOverride = (1 << 1)
};

enum {
  NSIllegalTextMovement	= 0,
  NSReturnTextMovement	= 0x10,
  NSTabTextMovement	= 0x11,
  NSBacktabTextMovement	= 0x12,
  NSLeftTextMovement	= 0x13,
  NSRightTextMovement	= 0x14,
  NSUpTextMovement	= 0x15,
  NSDownTextMovement	= 0x16,
  NSCancelTextMovement  = 0x17,
  NSOtherTextMovement   = 0
};

enum {
  NSParagraphSeparatorCharacter	= 0x2029,
  NSLineSeparatorCharacter	= 0x2028,
  NSTabCharacter		= 0x0009,
  NSFormFeedCharacter		= 0x000c,
  NSNewlineCharacter		= 0x000a,
  NSCarriageReturnCharacter	= 0x000d,
  NSEnterCharacter		= 0x0003,
  NSBackspaceCharacter		= 0x0008,
  NSBackTabCharacter		= 0x0019,
  NSDeleteCharacter		= 0x007f,
};


/* The following are required by the original openstep doc.  */
enum {
  NSBackspaceKey      = 8,
  NSCarriageReturnKey = 13,
  NSDeleteKey         = 0x7f,
  NSBacktabKey        = 25
};

@interface NSText : NSView <NSChangeSpelling, NSIgnoreMisspelledWords>
{
}

/*
 * Getting and Setting Contents 
 */
/* these aren't in OPENSTEP */
- (void) replaceCharactersInRange: (NSRange)aRange  
			  withRTF: (NSData*)rtfData;
- (void) replaceCharactersInRange: (NSRange)aRange  
			 withRTFD: (NSData*)rtfdData;
/* PRIMITIVE */
- (void) replaceCharactersInRange: (NSRange)aRange  
		       withString: (NSString*)aString;
/* PRIMITIVE */
-(void) replaceCharactersInRange: (NSRange)aRange /* GNUstep extension */
	    withAttributedString: (NSAttributedString *)attrString;
/* PRIMITIVE */
- (void) setString: (NSString*)aString;
/* PRIMITIVE */
- (NSString*) string;

/*
 * Old fashioned OpenStep methods (wrappers for the previous ones)
 */
- (void) replaceRange: (NSRange)aRange  withString: (NSString*)aString; /* not OPENSTEP */
- (void) replaceRange: (NSRange)aRange  withRTF: (NSData*)rtfData;
- (void) replaceRange: (NSRange)aRange  withRTFD: (NSData*)rtfdData;
- (void) setText: (NSString*)aString;
- (void) setText: (NSString*)aString  range: (NSRange)aRange; /* not OPENSTEP */
- (NSString*) text;

/*
 * Graphic attributes
 */
/* PRIMITIVE */
- (NSColor*) backgroundColor;
/* PRIMITIVE */
- (BOOL) drawsBackground;
/* PRIMITIVE */
- (void) setBackgroundColor: (NSColor*)color;
/* PRIMITIVE */
- (void) setDrawsBackground: (BOOL)flag;

/*
 * Managing Global Characteristics
 */
- (BOOL) importsGraphics; /* PRIMITIVE */
- (BOOL) isEditable; /* PRIMITIVE */
- (BOOL) isFieldEditor; /* PRIMITIVE */
- (BOOL) isRichText; /* PRIMITIVE */
- (BOOL) isSelectable; /* PRIMITIVE */
- (void) setEditable: (BOOL)flag; /* PRIMITIVE */
- (void) setFieldEditor: (BOOL)flag; /* PRIMITIVE */
- (void) setImportsGraphics: (BOOL)flag; /* PRIMITIVE */
- (void) setRichText: (BOOL)flag; /* PRIMITIVE */
- (void) setSelectable: (BOOL)flag; /* PRIMITIVE */

/*
 * Using the font panel
 */ 
- (void) setUsesFontPanel: (BOOL)flag; /* PRIMITIVE */
- (BOOL) usesFontPanel; /* PRIMITIVE */

/*
 * Managing the Ruler
 */
- (BOOL) isRulerVisible; /* PRIMITIVE */
- (void) toggleRuler: (id)sender; /* PRIMITIVE */

/*
 * Managing the Selection
 */
- (NSRange) selectedRange;
- (void) setSelectedRange: (NSRange)range;

/*
 * Responding to Editing Commands
 */
- (void) copy: (id)sender; /* PRIMITIVE */
- (void) copyFont: (id)sender; /* PRIMITIVE */
- (void) copyRuler: (id)sender; /* PRIMITIVE */
- (void) cut: (id)sender;
- (void) delete: (id)sender; /* PRIMITIVE */
- (void) paste: (id)sender; /* PRIMITIVE */
- (void) pasteFont: (id)sender; /* PRIMITIVE */
- (void) pasteRuler: (id)sender; /* PRIMITIVE */
- (void) selectAll: (id)sender;

/*
 * Managing Font
 */
- (void) changeFont: (id)sender; /* PRIMITIVE */
- (NSFont*) font; /* PRIMITIVE */
- (void) setFont: (NSFont*)font; /* PRIMITIVE */
- (void) setFont: (NSFont*)font  range: (NSRange)aRange; /* PRIMITIVE */
/* Old OpenStep name for the same method.  */
- (void) setFont: (NSFont*)font  ofRange: (NSRange)aRange;

/*
 * Managing Alignment
 */
- (NSTextAlignment) alignment; /* PRIMITIVE */
- (void) setAlignment: (NSTextAlignment)mode; /* PRIMITIVE */
- (void) alignCenter: (id)sender; /* PRIMITIVE */
- (void) alignLeft: (id)sender; /* PRIMITIVE */
- (void) alignRight: (id)sender; /* PRIMITIVE */

/*
 * Text colour
 */
- (void) setTextColor: (NSColor*)color range: (NSRange)aRange; /* PRIMITIVE */ /* not OPENSTEP */
- (void) setColor: (NSColor*)color ofRange: (NSRange)aRange; /* PRIMITIVE */
- (void) setTextColor: (NSColor*)color; /* PRIMITIVE */
- (NSColor*) textColor; /* PRIMITIVE */

/*
 * Text attributes
 */
- (void) subscript: (id)sender; /* PRIMITIVE */
- (void) superscript: (id)sender; /* PRIMITIVE */
- (void) underline: (id)sender; /* PRIMITIVE */
- (void) unscript: (id)sender; /* PRIMITIVE */

/*
 * Reading and Writing RTFD Files
 */
-(BOOL) readRTFDFromFile: (NSString*)path; /* PRIMITIVE */
-(BOOL) writeRTFDToFile: (NSString*)path atomically: (BOOL)flag; /* PRIMITIVE */
-(NSData*) RTFDFromRange: (NSRange)aRange; /* PRIMITIVE */
-(NSData*) RTFFromRange: (NSRange)aRange; /* PRIMITIVE */

/*
 * Sizing the Frame Rectangle
 */
- (BOOL) isHorizontallyResizable; /* PRIMITIVE */
- (BOOL) isVerticallyResizable; /* PRIMITIVE */
- (NSSize) maxSize; /* PRIMITIVE */
- (NSSize) minSize; /* PRIMITIVE */
- (void) setHorizontallyResizable: (BOOL)flag; /* PRIMITIVE */
- (void) setMaxSize: (NSSize)newMaxSize; /* PRIMITIVE */
- (void) setMinSize: (NSSize)newMinSize; /* PRIMITIVE */
- (void) setVerticallyResizable: (BOOL)flag; /* PRIMITIVE */
- (void) sizeToFit; /* PRIMITIVE */

/*
 * Spelling
 */
- (void) checkSpelling: (id)sender; /* PRIMITIVE */
- (void) showGuessPanel: (id)sender;

/*
 * Scrolling
 */
- (void) scrollRangeToVisible: (NSRange)aRange; /* PRIMITIVE */

/*
 * Managing the Delegate
 */
- (id) delegate; /* PRIMITIVE */
- (void) setDelegate: (id)anObject; /* PRIMITIVE */

/*
 * NSChangeSpelling protocol
 */
- (void) changeSpelling: (id)sender; /* PRIMITIVE */

/*
 * NSIgnoreMisspelledWords protocol
 */
- (void) ignoreSpelling: (id)sender; /* PRIMITIVE */ /* not OPENSTEP */
@end

@interface NSText (GNUstepExtensions)

- (NSUInteger) textLength; /* PRIMITIVE */

@end

/* Notifications */
APPKIT_EXPORT NSString *NSTextDidBeginEditingNotification;
APPKIT_EXPORT NSString *NSTextDidEndEditingNotification;
APPKIT_EXPORT NSString *NSTextDidChangeNotification;

@interface NSObject (NSTextDelegate)
- (BOOL) textShouldBeginEditing: (NSText*)textObject; /* YES means do it */
- (BOOL) textShouldEndEditing: (NSText*)textObject; /* YES means do it */
- (void) textDidBeginEditing: (NSNotification*)notification;
- (void) textDidEndEditing: (NSNotification*)notification;
- (void) textDidChange: (NSNotification*)notification; /* Any keyDown or paste which changes the contents causes this */
@end

#endif // _GNUstep_H_NSText


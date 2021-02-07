/*
   NSTextStorage.h

   Copyright (C) 1996,1999 Free Software Foundation, Inc.

   Author:  Daniel Böhringer <boehring@biomed.ruhr-uni-bochum.de>
   Date: August 1998
   Source by Daniel Böhringer integrated into GNUstep gui
   by Felipe A. Rodriguez <far@ix.netcom.com> 
   Update: Richard Frith-Macdonald <richard@brainstorm.co.uk>

   Documentation written from scratch by: Nicola Pero
   <nicola@brainstorm.co.uk>
   
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

#ifndef _GNUstep_H_NSTextStorage
#define _GNUstep_H_NSTextStorage

#import <Foundation/NSAttributedString.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

@class NSNotification;
@class NSString;
@class GSLayoutManager;
@class NSFont;
@class NSColor;

/*
 * When edit:range:changeInLength: is called, it takes a mask saying
 * what has been edited.  The mask is NSTextStorageEditedAttributes
 * if the attributes have been changed, NSTextStorageEditedCharacters
 * if the characters have been changed, and 
 * NSTextStorageEditedAttributes | NSTextStorageEditedCharacters if both
 * characters and attributes were edited.
 */
enum
{
  NSTextStorageEditedAttributes = 1,
  NSTextStorageEditedCharacters = 2
};

/* 
 * The NSTextStorage 
 */
@interface NSTextStorage : NSMutableAttributedString
{
  NSRange		_editedRange;
  int			_editedDelta;
  NSMutableArray	*_layoutManagers;
  id			_delegate;
  unsigned		_editedMask;
  unsigned		_editCount;
}

- (void) addLayoutManager: (GSLayoutManager*)obj;
- (void) removeLayoutManager: (GSLayoutManager*)obj;
- (NSArray*) layoutManagers;

/*
 * This method is normally called between a beginEditing and an
 * endEditing message to record any changes which were made to the
 * receiver.  It is automatically called by the NSTextStorage
 * primitives, so in other words, NSTextStorage calls it for you, and
 * you don't normally need to call this method.  You might (a more
 * theoretical than practical option) need to subclass it and override
 * this method to take into account changes done.
 *
 * If the method is called outside a beginEditing/endEditing calls, it
 * calls processEditing immediately.  As far as I understand, that would
 * happen if you modify the NSTextStorage without enclosing your changes
 * inside a beginEditing/endEditing calls.
 *
 * maks can be NSTextStorageEditedAttributes or
 * NSTextStorageEditedCharacters, or and | of the two.
 *
 * the old range is the range affected by the change in the string ... in the
 * original string.
 *
 * the changeInLength is, well, positive if you added characters, negative
 * if you removed characters.  */
- (void) edited: (unsigned)mask range: (NSRange)old changeInLength: (int)delta;

/*
 * This method is called to process the editing once it's finished.
 * Normally it is called by endEditing; but if you modify the NSTextStorage
 * without enclosing the modifications into a beginEditing/endEditing pair,
 * this method will be called directly by edited:range:changeInLength:.
 * 
 * But in practice, what does this method do ?  Well, it posts the
 * NSTextStorageWillProcessEditing notification.  Then, it calls
 * fixAttributesAfterEditingRange:.  Then, it posts the
 * NSTextStorageDidProcessEditing notification.  Finally, it tells the
 * layout manager(s) about this change in the text storage by calling
 * textStorage:edited:range:changeInLength:invalidatedRange:.  */
- (void) processEditing;

/*
 * Start a set of changes to the text storage.  All the changes are
 * recorded and merged - without any layout to be done.  When you call
 * endEditing, the layout will be done in one single operation.
 *
 * In practice, you should call beginEditing before starting to modify
 * the NSTextStorage.  When you are finished modifying it and you want
 * your changes to be propagated to the layout manager(s), you call
 * endEditing.
 *
 * If you don't enclose your changes into a beginEditing/endEditing pair,
 * it still works, but it's less efficient.
 */
- (void) beginEditing;

/* 
 * End a set of changes, and calls processEditing.
 */
- (void) endEditing;

/*
 * The delegate can use the following methods when it receives a
 * notification that a change was made.  The methods tell him what
 * kind of change was made.  */
- (unsigned) editedMask;
- (NSRange) editedRange;
- (int) changeInLength;

- (void) setDelegate: (id)delegate;
- (id) delegate;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) ensureAttributesAreFixedInRange: (NSRange)range;
- (BOOL) fixesAttributesLazily;
- (void) invalidateAttributesInRange: (NSRange)range;
#endif

/** Returns the string data stored by the receiver.<br />
 * For performance reasons (and OSX compatibility) this is actually
 * a proxy to the internal representation of the string.<br />
 * This proxy provides an immutable string interface,
 * but you must be aware that the underlying information may be modified
 * by the receiver at any point, so if you need a consistent/fixed
 * snapshot of the data (or if you are going to pass the string to other
 * code which expects to be working with a constant string), you should
 * copy the object returned by this method rather than simply retaining it.
 */
- (NSString*) string;
@end


/****  NSTextStorage delegate methods ****/

@interface NSObject (NSTextStorageDelegate)

/*
 * The delegate is automatically registered to receive the
 * NSTextStorageWillProcessEditingNotification, and the
 * NSTextStorageDidProcessEditingNotification via these methods, so
 * once the notifications are sent, these methods of the delegate will
 * be invokes.  In these methods the delegate can use editedMask,
 * editedRange, changeInLength to figure out what the actual change
 * is.
 */
- (void) textStorageWillProcessEditing: (NSNotification*)notification;
- (void) textStorageDidProcessEditing: (NSNotification*)notification;

@end

@interface NSTextStorage (Scripting)
/*
 * Attributes for string...
*/
- (NSFont*) font;
- (void) setFont: (NSFont*)font;

/*
 * The text storage contents as an array of attribute runs.
 */
- (NSArray *)attributeRuns;

/*
 * The text storage contents as an array of paragraphs.
 */
- (NSArray *)paragraphs;

/*
 * The text storage contents as an array of words.
 */
- (NSArray *)words;

/*
 * The text storage contents as an array of characters.
 */
- (NSArray *)characters;

/*
 * The font color used when drawing text.
 */
- (NSColor *)foregroundColor;
@end

/**** Notifications ****/

/* The object of the notification is the NSTextStorage itself.  */
APPKIT_EXPORT NSString *NSTextStorageWillProcessEditingNotification;
APPKIT_EXPORT NSString *NSTextStorageDidProcessEditingNotification;

#endif

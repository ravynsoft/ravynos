/* 
   NSResponder.h

   Abstract class which is basis of command and event processing

   Copyright (C) 1996,1999 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   
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

#ifndef _GNUstep_H_NSResponder
#define _GNUstep_H_NSResponder
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <AppKit/NSInterfaceStyle.h>
#import <AppKit/AppKitDefines.h>

@class NSCoder;
@class NSError;
@class NSString;

@class NSEvent;
@class NSMenu;
@class NSUndoManager;
@class NSWindow;

@interface NSResponder : NSObject <NSCoding>
{
PACKAGE_SCOPE
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
  NSInterfaceStyle	_interface_style;
#else
  int			_interface_style;
#endif
@protected
  NSResponder		*_next_responder;

  /*
  Due to interface brain-damage, both NSResponder and NSMenuView have -menu
  and -setMenu: methods, but for different menus. Thus, to prevent (future,
  there have already been some) problems and confusion, this ivar is
  private (iow, it can't be accidentally used in NSMenuView).
  */
@private
  NSMenu                *_menu;
}

/*
 * Instance methods
 */

/*
 * Managing the next responder
 */
- (NSResponder*) nextResponder;
- (void) setNextResponder: (NSResponder*)aResponder;

/*
 * Determining the first responder
 */
- (BOOL) acceptsFirstResponder;
- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;

/*
 * Aid event processing
 */
- (BOOL) performKeyEquivalent: (NSEvent*)theEvent;
- (BOOL) tryToPerform: (SEL)anAction with: (id)anObject;

/*
 * Forwarding event messages
 */
- (void) flagsChanged: (NSEvent*)theEvent;
- (void) helpRequested: (NSEvent*)theEvent;
- (void) keyDown: (NSEvent*)theEvent;
- (void) keyUp: (NSEvent*)theEvent;
- (void) mouseDown: (NSEvent*)theEvent;
- (void) mouseDragged: (NSEvent*)theEvent;
- (void) mouseEntered: (NSEvent*)theEvent;
- (void) mouseExited: (NSEvent*)theEvent;
- (void) mouseMoved: (NSEvent*)theEvent;
- (void) mouseUp: (NSEvent*)theEvent;
- (void) noResponderFor: (SEL)eventSelector;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) otherMouseDown: (NSEvent*)theEvent;
- (void) otherMouseDragged: (NSEvent*)theEvent;
- (void) otherMouseUp: (NSEvent*)theEvent;
#endif
- (void) rightMouseDown: (NSEvent*)theEvent;
- (void) rightMouseDragged: (NSEvent*)theEvent;
- (void) rightMouseUp: (NSEvent*)theEvent;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) scrollWheel: (NSEvent *)theEvent;
#endif

/*
 * Services menu support
 */
- (id) validRequestorForSendType: (NSString*)typeSent
		      returnType: (NSString*)typeReturned;

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder;
- (id) initWithCoder: (NSCoder*)aDecoder;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) interpretKeyEvents: (NSArray*)eventArray;
- (BOOL) performMnemonic: (NSString*)aString;
- (void) flushBufferedKeyEvents;
- (void) doCommandBySelector: (SEL)aSelector;

/** <p>Tells the receiver to insert the given string. In a text view
    the text is typically inserted at the insertion point, and replaces
    any selection.
    </p><p>
    Subclasses should override this method. The implementation in
    NSResponder just sends the message on to the next responder.
    </p><p>
    Normally, aString will be an NSString, but in some cases, it might
    be an NSAttributedString.
    </p>  */
- (void) insertText: (id)aString;
- (NSUndoManager*) undoManager;

/*
 * Menu
 */
- (NSMenu*) menu;
- (void) setMenu: (NSMenu*)aMenu;

/*
 * Setting the interface
 */
- (NSInterfaceStyle) interfaceStyle;
- (void) setInterfaceStyle: (NSInterfaceStyle)aStyle;

- (BOOL) shouldBeTreatedAsInkEvent: (NSEvent *)theEvent;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL)presentError:(NSError *)error;
- (void)presentError:(NSError *)error
      modalForWindow:(NSWindow *)window
            delegate:(id)delegate
  didPresentSelector:(SEL)sel
         contextInfo:(void *)context;
- (NSError *)willPresentError:(NSError *)error;
#endif

@end

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
@interface NSResponder (OptionalActionMethods)
- (void) capitalizeWord: (id)sender;
- (void) centerSelectionInVisibleArea: (id)sender;
- (void) changeCaseOfLetter: (id)sender;
- (void) complete: (id)sender;
- (void) deleteBackward: (id)sender;
- (void) deleteForward: (id)sender;
- (void) deleteToBeginningOfLine: (id)sender;
- (void) deleteToBeginningOfParagraph: (id)sender;
- (void) deleteToEndOfLine: (id)sender;
- (void) deleteToEndOfParagraph: (id)sender;
- (void) deleteToMark: (id)sender;
- (void) deleteWordBackward: (id)sender;
- (void) deleteWordForward: (id)sender;
- (void) indent: (id)sender;
- (void) insertBacktab: (id)sender;
- (void) insertNewline: (id)sender;
- (void) insertNewlineIgnoringFieldEditor: (id)sender;
- (void) insertParagraphSeparator: (id)sender;
- (void) insertTab: (id)sender;
- (void) insertTabIgnoringFieldEditor: (id)sender;
- (void) lowercaseWord: (id)sender;
- (void) moveBackward: (id)sender;
- (void) moveBackwardAndModifySelection: (id)sender;
- (void) moveDown: (id)sender;
- (void) moveDownAndModifySelection: (id)sender;
- (void) moveForward: (id)sender;
- (void) moveForwardAndModifySelection: (id)sender;
- (void) moveLeft: (id)sender;
- (void) moveRight: (id)sender;
- (void) moveToBeginningOfDocument: (id)sender;
- (void) moveToBeginningOfLine: (id)sender;
- (void) moveToBeginningOfParagraph: (id)sender;
- (void) moveToEndOfDocument: (id)sender;
- (void) moveToEndOfLine: (id)sender;
- (void) moveToEndOfParagraph: (id)sender;
- (void) moveUp: (id)sender;
- (void) moveUpAndModifySelection: (id)sender;
- (void) moveWordBackward: (id)sender;
- (void) moveWordBackwardAndModifySelection: (id)sender;
- (void) moveWordForward: (id)sender;
- (void) moveWordForwardAndModifySelection: (id)sender;
- (void) pageDown: (id)sender;
- (void) pageUp: (id)sender;
- (void) scrollLineDown: (id)sender;
- (void) scrollLineUp: (id)sender;
- (void) scrollPageDown: (id)sender;
- (void) scrollPageUp: (id)sender;
- (void) scrollToBeginningOfDocument: (id)sender;
- (void) scrollToEndOfDocument: (id)sender;
- (void) selectAll: (id)sender;
- (void) selectLine: (id)sender;
- (void) selectParagraph: (id)sender;
- (void) selectToMark: (id)sender;
- (void) selectWord: (id)sender;
- (void) setMark: (id)sender;
- (void) showContextHelp: (id)sender;
- (void) swapWithMark: (id)sender;
- (void) transpose: (id)sender;
- (void) transposeWords: (id)sender;
- (void) uppercaseWord: (id)sender;
- (void) yank: (id)sender;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void) cancelOperation: (id)sender;
- (void) deleteBackwardByDecomposingPreviousCharacter: (id)sender;
- (void) moveLeftAndModifySelection: (id)sender;
- (void) moveRightAndModifySelection: (id)sender;
- (void) moveWordLeft: (id)sender;
- (void) moveWordLeftAndModifySelection: (id)sender;
- (void) moveWordRight: (id)sender;
- (void) moveWordRightAndModifySelection: (id)sender;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (void) insertContainerBreak: (id)sender;
- (void) insertLineBreak: (id)sender;
- (void) tabletPoint:(NSEvent *)event;
- (void) tabletProximity:(NSEvent *)event;
#endif

@end
#endif

#endif /* _GNUstep_H_NSResponder */

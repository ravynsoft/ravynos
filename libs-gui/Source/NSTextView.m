/** <title>NSTextView</title>

   Copyright (C) 1996, 1998, 2000, 2001, 2002, 2003, 2008, 2020 Free Software Foundation, Inc.

   Much code of this class was originally derived from code which was
   in NSText.m.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996

   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: July 1998

   Author: Daniel Boehringer <boehring@biomed.ruhr-uni-bochum.de>
   Date: August 1998

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2000, September 2000, January 2008

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: 2000, 2001, 2002

   Author: Pierre-Yves Rivaille <pyrivail@ens-lyon.fr>
   Date: September 2002

   Extensive reworking: Alexander Malmberg <alexander@malmberg.org>
   Date: December 2002 - February 2003

   Implementing Catalina Extensions: Gregory Casamento <greg.casamento@gmail.com>
   Date: August 2020

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
#import <Foundation/NSArchiver.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSString.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSUndoManager.h>
#import <Foundation/NSValue.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSClipView.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSColorPanel.h"
#import "AppKit/NSControl.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSDragging.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFileWrapper.h"
#import "AppKit/NSFileWrapperExtensions.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSKeyValueBinding.h"
#import "AppKit/NSLayoutManager.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSMenuItem.h"
#import "AppKit/NSParagraphStyle.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSRulerMarker.h"
#import "AppKit/NSRulerView.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSSpellChecker.h"
#import "AppKit/NSTextAttachment.h"
#import "AppKit/NSTextContainer.h"
#import "AppKit/NSTextStorage.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSWindow.h"

#import "GSGuiPrivate.h"
#import "GSTextFinder.h"
#import "GSToolTips.h"
#import "GSFastEnumeration.h"
#import "GSAutocompleteWindow.h"


/*
NSTextView_actions.m has comments about what methods dealing with user
actions and user-initiated changes need to do. These also apply to some
methods in this file.
*/


/*
TODOs:

think hard about insertion point management


How should resizing work? If the text view is set to track the used part
of the text container, when does it actually update its size? Might need
a new internal method called from NSLayoutManager when text has changed.
(Currently NSLayoutManager calls -sizeToFit when text has changed.)

*/


@interface NSTextViewUndoObject : NSObject
{
  NSRange range;
  NSAttributedString *string;
}
- (id) initWithRange: (NSRange)aRange
    attributedString: (NSAttributedString *)aString;
- (NSRange) range;
- (void) setRange: (NSRange)aRange;
- (void) performUndo: (NSTextStorage *)aTextStorage;
- (NSTextView *) bestTextViewForTextStorage: (NSTextStorage *)aTextStorage;
@end


/*
Interface for a bunch of internal methods that need to be cleaned up.
*/
@interface NSTextView (GNUstepPrivate)
- (NSUInteger) _characterIndexForPoint: (NSPoint)point
                       respectFraction: (BOOL)respectFraction;

/*
 * Used to implement the blinking insertion point
 */
- (void) _blink: (NSTimer *)t;
- (void) _stopInsertionTimer;
- (void) _startInsertionTimer;

/*
 * these NSLayoutManager- like method is here only informally
 */
- (NSRect) rectForCharacterRange: (NSRange)aRange;

//
// GNU utility methods
//
- (void) copySelection;
- (void) pasteSelection;

/*
 * Text checking
 */
- (void) _scheduleTextCheckingInVisibleRectIfNeeded;
- (void) _textDidChange: (NSNotification*)notif;
- (void) _textCheckingTimerFired: (NSTimer *)t;

/*
 * helper method for ruler view
 */
- (void) _becomeRulerClient;
- (void) _resignRulerClient;
@end


// This class is a helper for keyed unarchiving only
@interface NSTextViewSharedData : NSObject 
{
@private
  NSColor *backgroundColor;
  NSParagraphStyle *paragraphStyle;
  unsigned int flags;
  NSColor *insertionColor;
  NSDictionary *linkAttr;
  NSDictionary *markAttr;
  NSDictionary *selectedAttr;
  NSTextView *textView;
}
- (NSColor *) backgroundColor;
- (NSParagraphStyle *) paragraphStyle;
- (unsigned int) flags;
- (NSColor *) insertionColor;
- (NSDictionary *) linkAttributes;
- (NSDictionary *) markAttributes;
- (NSDictionary *) selectedAttributes;
- (NSTextView *) textView;
@end

@implementation NSTextViewSharedData

- (id) initWithTextView: (NSTextView *)tv
{
  if ((self = [super init]) != nil)
    {
      flags = (([tv isSelectable]?0x01:0) |
	       ([tv isEditable]?0x02:0) |
	       ([tv isRichText]?0x04:0) |
	       ([tv importsGraphics]?0x08:0) |
	       ([tv isFieldEditor]?0x10:0) |
	       ([tv usesFontPanel]?0x20:0) |
	       ([tv isRulerVisible]?0x40:0) |
	       ([tv isContinuousSpellCheckingEnabled]?0x80:0) |
	       ([tv usesRuler]?0x100:0) |
	       ([tv smartInsertDeleteEnabled]?0x200:0) |
	       ([tv allowsUndo]?0x400:0) |
	       ([tv drawsBackground]?0x800:0) |
	       ([tv usesFindPanel]?0x2000:0));

      ASSIGN(backgroundColor, [tv backgroundColor]);
      ASSIGN(paragraphStyle, [tv defaultParagraphStyle]);
      ASSIGN(insertionColor, [tv insertionPointColor]);
      ASSIGN(markAttr, [tv markedTextAttributes]);
      ASSIGN(linkAttr, [tv linkTextAttributes]);
      ASSIGN(selectedAttr, [tv selectedTextAttributes]);
      
      linkAttr = nil;
      textView = tv;
    }

  return self;
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      ASSIGN(backgroundColor,
	[aDecoder decodeObjectForKey: @"NSBackgroundColor"]);
      ASSIGN(paragraphStyle,
	[aDecoder decodeObjectForKey: @"NSDefaultParagraphStyle"]);
      flags = [aDecoder decodeIntForKey: @"NSFlags"];
      ASSIGN(insertionColor,
	[aDecoder decodeObjectForKey: @"NSInsertionColor"]);
      ASSIGN(linkAttr, [aDecoder decodeObjectForKey: @"NSLinkAttributes"]);
      ASSIGN(markAttr, [aDecoder decodeObjectForKey: @"NSMarkedAttributes"]);
      ASSIGN(selectedAttr,
	[aDecoder decodeObjectForKey: @"NSSelectedAttributes"]);
    }
  
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {	
      [coder encodeObject: backgroundColor forKey: @"NSBackgroundColor"];
      [coder encodeObject: paragraphStyle forKey: @"NSDefaultParagraphStyle"];
      [coder encodeInt: flags forKey: @"NSFlags"];
      [coder encodeObject: insertionColor forKey: @"NSInsertionColor"];
      [coder encodeObject: linkAttr forKey: @"NSLinkAttributes"];
      [coder encodeObject: markAttr forKey: @"NSMarkedAttributes"];
      [coder encodeObject: selectedAttr forKey: @"NSSelectedAttributes"];
    }
}

- (void) dealloc
{
  RELEASE(backgroundColor);
  RELEASE(paragraphStyle);
  RELEASE(insertionColor);
  RELEASE(linkAttr);
  RELEASE(markAttr);
  RELEASE(selectedAttr);
  [super dealloc];
}

- (NSColor *)backgroundColor
{
  return backgroundColor;
}

- (NSParagraphStyle *) paragraphStyle
{
  return paragraphStyle;
}

- (unsigned int) flags
{
  return flags;
}

- (NSColor *) insertionColor
{
  return insertionColor;
}

- (NSDictionary *) linkAttributes
{
  return linkAttr;
}

- (NSDictionary *) markAttributes
{
  return markAttr;
}

- (NSDictionary *) selectedAttributes
{
  return selectedAttr;
}

- (NSTextView *) textView
{
  return textView;
}
@end


@interface NSTextStorage(NSTextViewUndoSupport)
- (void) _undoTextChange: (NSTextViewUndoObject *)anObject;
@end

/**** Misc. helpers and stuff ****/

static const int currentVersion = 4;

static BOOL noLayoutManagerException(void)
{
  [NSException raise: NSGenericException
	      format: @"Can't edit a NSTextView without a layout manager!"];
  return YES;
}

/* The shared notification center */
static NSNotificationCenter *notificationCenter;

/* Default max. size. Don't change this without understanding and checking
for roundoff issues. Whole points should be representable. */
#define GSHUGE 1e7


/**** Synchronization stuff ****/

/* For when more than one text view is connected to a layout manager.
Helpers are here, the actual methods are in the main implementation. */
@implementation NSTextView (GSTextViewSync)


#define SET_DELEGATE_NOTIFICATION(notif_name) \
  if ([_delegate respondsToSelector: @selector(text##notif_name:)]) \
    [notificationCenter addObserver: _delegate \
           selector: @selector(text##notif_name:) \
               name: NSText##notif_name##Notification \
             object: _notifObject]

/*
 * Synchronizing flags.  Used to manage synchronizing shared
 * attributes between textviews coupled with the same layout manager.
 * These synchronizing flags are only accessed when
 * _tf.multiple_textviews == YES and this can only happen if we have
 * a non-nil NSLayoutManager - so we don't check. */

/* YES when in the process of synchronizing text view attributes.  
   Used to avoid recursive synchronizations. */
#define IS_SYNCHRONIZING_FLAGS _layoutManager->_isSynchronizingFlags 
/* YES when in the process of synchronizing delegates.
   Used to avoid recursive synchronizations. */ 
#define IS_SYNCHRONIZING_DELEGATES _layoutManager->_isSynchronizingDelegates


/*
This gets sent to all text views connected to a layout manager whenever
a text container is added or removed, or if a text container changes text
view. When a text container is removed, it is also sent to the text view
of the removed text view. It is also sent by -setTextContainer:.

This will be sent several times to the same text view for one change in
some cases, so it needs to be safe wrt. that.
*/
- (void) _updateMultipleTextViews
{
  id oldNotifObject = _notifObject;

  if ([[_layoutManager textContainers] count] > 1)
    {
      _tf.multiple_textviews = YES;
      _notifObject = [_layoutManager firstTextView];
    }
  else
    {
      _tf.multiple_textviews = NO;
      _notifObject = self;
    }  

  /*
  The notifications only need to be reset once for each change, but doing it
  several times doesn't hurt.
  */
  if ((_delegate != nil) && (oldNotifObject != _notifObject))
    {
      [notificationCenter removeObserver: _delegate
	name: nil
	object: oldNotifObject];

      if ([_delegate respondsToSelector:
	    @selector(textView:shouldChangeTextInRange:replacementString:)])
	{
	  _tf.delegate_responds_to_should_change = YES;
	}
      else
	{
	  _tf.delegate_responds_to_should_change = NO;
	}

      /* SET_DELEGATE_NOTIFICATION defined at the beginning of file */

      /* NSText notifications */
      SET_DELEGATE_NOTIFICATION(DidBeginEditing);
      SET_DELEGATE_NOTIFICATION(DidChange);
      SET_DELEGATE_NOTIFICATION(DidEndEditing);
      /* NSTextView notifications */
      SET_DELEGATE_NOTIFICATION(ViewDidChangeSelection);
      SET_DELEGATE_NOTIFICATION(ViewWillChangeNotifyingTextView);
      SET_DELEGATE_NOTIFICATION(ViewDidChangeTypingAttributes);
    }
}


/* 
_syncTextViewsByCalling:withFlag: calls a set method on all text
views sharing the same layout manager as this one.  It sets the
IS_SYNCHRONIZING_FLAGS flag to YES to prevent recursive calls;
calls the specified action on all the textviews (this one included)
with the specified flag; sets back the IS_SYNCHRONIZING_FLAGS flag
to NO; then returns.

We need to explicitly call the methods - we can't copy the flags
directly from one textview to another - to allow subclasses to
override eg. -setEditable: to take some particular action when
editing is turned on or off.
*/
- (void) _syncTextViewsByCalling: (SEL)action
		       withFlag: (BOOL)flag
{
  NSArray *array;
  NSUInteger i, count;

  if (IS_SYNCHRONIZING_FLAGS == YES)
    {
      [NSException raise: NSGenericException
	format: @"_syncTextViewsByCalling:withFlag: called recursively"];
    }

  array = [_layoutManager textContainers];
  count = [array count];

  IS_SYNCHRONIZING_FLAGS = YES;

  for (i = 0; i < count; i++)
    {
      NSTextView *tv; 
      void (*msg)(id, SEL, BOOL);

      tv = [(NSTextContainer *)[array objectAtIndex: i] textView];
      msg = (void (*)(id, SEL, BOOL))[tv methodForSelector: action];
      if (msg != NULL)
	{
	  (*msg) (tv, action, flag);
	}
      else
	{
	  /* Ahm.  What shall we do here.  It can't happen, can it ? */
	  NSLog(@"Weird error - _syncTextViewsByCalling:withFlag: couldn't find method for selector");
	}
    }

  IS_SYNCHRONIZING_FLAGS = NO;
}


/*
This must be called whenever the delegate changes (directly, through calls
to -setDelegate:, or indirectly due to changes in the text system, or being
decoded.
*/
- (void) _recacheDelegateResponses
{
  SEL selector;

  selector = @selector(textView:shouldChangeTextInRange:replacementString:);
  if ([_delegate respondsToSelector: selector])
    {
      _tf.delegate_responds_to_should_change = YES;
    }
  else
    {
      _tf.delegate_responds_to_should_change = NO;
    }

  selector = @selector(textView:willChangeSelectionFromCharacterRange:toCharacterRange:);
  if ([_delegate respondsToSelector: selector])
    {
      _tf.delegate_responds_to_will_change_sel = YES;
    }
  else
    {
      _tf.delegate_responds_to_will_change_sel = NO;
    }
}


/*
Called when our state needs updating due to external changes. Currently,
this happens when layout has been invalidated, and when we are resized.
*/
- (void) _updateState: (id)sender
{
  [self sizeToFit];
  /* TODO: we don't have to redisplay the entire view */
  [self setNeedsDisplay: YES];
  [self updateInsertionPointStateAndRestartTimer:
    [self shouldDrawInsertionPoint]];
  [self _updateInputMethodState];
  /* In case any sections of text with custom cursors were moved */
  [[self window] invalidateCursorRectsForView: self];
}

- (void) _layoutManagerDidInvalidateLayout
{
  /*
  We don't want to do the update right away, since the invalidation might
  be followed by further invalidation if the user of the text system is
  making large rearrangements of the text network. This also makes us a
  lot more forgiving; no real works is done until "later", so the user can
  have the text network in an inconsistent state for short periods of time.
  (This shouldn't be relied on, though. If possible, avoid it.)

  Actually, we want to run as late as possible. However, it seems to me that
  we must run before [NSWindow -_handleWindowNeedsDisplay:] (or that it
  would be a very good idea to do so, since we might end up resizing
  ourselves and stuff like that). Thus, we use -performSelector:... with
  an order 1 under that for window updating.

  If the update method is not called before the real redisplay (if someone
  forces a redisplay right away), we might draw incorrectly, but it shouldn't
  cause any other harm.
  */
  [[NSRunLoop currentRunLoop] cancelPerformSelector: @selector(_updateState:)
    target: self
    argument: nil];
  [[NSRunLoop currentRunLoop]
    performSelector: @selector(_updateState:)
	     target: self
	   argument: nil
	      order: 599999
	      modes: [NSArray arrayWithObjects: /* TODO: set up array in advance */
		       NSDefaultRunLoopMode,
		       NSModalPanelRunLoopMode,
		       NSEventTrackingRunLoopMode, nil]];
}


@end

@interface NSUndoManager(UndoCoalescing)
/* Auxiliary method to support coalescing undo actions for typing events
   in NSTextView.  However, the implementation is not restricted to that
   purpose. */
- (BOOL) _canCoalesceUndoWithTarget: (id)target
			   selector: (SEL)aSelector
			     object: (id)anObject;
@end


@implementation NSTextView


/* The sets smartLeftChars and smartRightChars are used for smart insert
   and delete. If users paste text into a text view and the character to
   the left (right) side of the new word is not in the set smartLeftChars
   (smartRightChars), an extra space is added to that side. Inversely, if
   users delete a word followed (preceded) by a character in smartRightChars
   (smartLeftChars), a space character on the other side of the word is
   deleted as well.
 */
static NSCharacterSet *smartLeftChars;
static NSCharacterSet *smartRightChars;

/* Private pasteboard type for adjoining smart paste information with
   text on the pasteboard. */
static NSString *NSSmartPastePboardType = @"NSSmartPastePboardType";

static NSMenu *textViewMenu;

/**** Misc. class methods ****/

+(void) initialize
{
  if ([self class] == [NSTextView class])
    {
      NSMutableCharacterSet *temp;

      [self setVersion: currentVersion];
      notificationCenter = [NSNotificationCenter defaultCenter];

      /* The sets smartLeftChars and smartRightChars were derived from OS X. */
      temp = [[NSCharacterSet whitespaceAndNewlineCharacterSet] mutableCopy];
      [temp addCharactersInString: @"\"#$\'(-/[`{"];
      smartLeftChars = [temp copy];
      [temp formUnionWithCharacterSet:
	      [NSCharacterSet punctuationCharacterSet]];
      smartRightChars = [temp copy];
      [temp release];

      [self exposeBinding: NSEditableBinding];
    }
}

static BOOL did_register_for_services;

+(void) registerForServices
{
  NSArray *types;

  did_register_for_services = YES;

  types = [NSArray arrayWithObjects: NSStringPboardType,
		    NSRTFPboardType, NSRTFDPboardType, nil];

  NSAssert(NSApp, @"Called before the shared application object was created.");
  [NSApp registerServicesMenuSendTypes: types
			   returnTypes: types];
}

+(NSDictionary *) defaultTypingAttributes
{
static NSDictionary *defaultTypingAttributes;

  if (!defaultTypingAttributes)
    {
      defaultTypingAttributes = [[NSDictionary alloc] initWithObjectsAndKeys:
	[NSParagraphStyle defaultParagraphStyle], NSParagraphStyleAttributeName,
	[NSFont userFontOfSize: 0], NSFontAttributeName,
	[NSColor textColor], NSForegroundColorAttributeName,
	nil];
    }
  return defaultTypingAttributes;
}

+ (NSMenu *) defaultMenu
{
  if (!textViewMenu)
    {
      textViewMenu = [[NSMenu alloc] initWithTitle: @""];
      [textViewMenu insertItemWithTitle: _(@"Cut") action:@selector(cut:) keyEquivalent:@"x" atIndex:0];
      [textViewMenu insertItemWithTitle: _(@"Copy") action:@selector(copy:) keyEquivalent:@"c" atIndex:1];
      [textViewMenu insertItemWithTitle: _(@"Paste") action:@selector(paste:) keyEquivalent:@"v" atIndex:2];
    }
  return textViewMenu;
}

/**** Initialization ****/

/*
Note that -init* must be completely side-effect-less (outside this
NSTextView). In particular, they must _not_ touch the font panel or rulers
until whoever created us has a chance to eg. -setUsesFontPanel: NO.
Calling any other methods here is dangerous as a sub-class might have
overridden them and given them side-effects, so we don't.


Note also that if a text view is added to an existing text network, the
new text view must not change any attributes of the existing text views.
Instead, it sets its own attributes to match the others'. This also applies
when a text view's moves to another text network.

The only method that is allowed to change which text network a text view
belongs to is -setTextContainer:, and it must be called when it changes
(NSLayoutManager and NSTextContainer do this for us).

Since we can't have any side-effects, we can't call methods to set the
values. A sub-class that wants to react to changes caused by moving to
a different text network will have to override -setTextContainer: and do
whatever it needs to do after calling [super setTextContainer: foo].
(TODO: check that this behavior is acceptable)

If a text view is added to an empty text network, it keeps its attributes.
*/


- (NSTextContainer *) buildUpTextNetwork: (NSSize)aSize
{
  NSTextContainer *textContainer;
  NSLayoutManager *layoutManager;
  NSTextStorage *textStorage;

  textStorage = [[NSTextStorage alloc] init];

  layoutManager = [[NSLayoutManager alloc] init];

  [textStorage addLayoutManager: layoutManager];
  RELEASE(layoutManager);

  textContainer = [[NSTextContainer alloc] initWithContainerSize: aSize];
  [layoutManager addTextContainer: textContainer];
  RELEASE(textContainer);

  /* We keep a flag to remember that we are directly responsible for 
     managing the text network. */
  _tf.owns_text_network = YES;
  // This value gets set once more in setTextContainer: to the same value.
  _textStorage = textStorage;

  /* The situation at this point is as follows: 

     textView (us) --RETAINs--> textStorage 
     textStorage   --RETAINs--> layoutManager 
     layoutManager --RETAINs--> textContainer */

  return textContainer;
}


/* Designated initializer. */
- (id) initWithFrame: (NSRect)frameRect
       textContainer: (NSTextContainer *)container
{
  self = [super initWithFrame: frameRect];
  if (!self)
    return nil;

  if (!did_register_for_services)
    [[self class] registerForServices];

  _minSize = frameRect.size;
  _maxSize = NSMakeSize(frameRect.size.width, GSHUGE);
  _textContainerInset = NSZeroSize;

  ASSIGN(_insertionPointColor, [NSColor textColor]);
  ASSIGN(_backgroundColor, [NSColor textBackgroundColor]);
  ASSIGN(_defaultParagraphStyle, [NSParagraphStyle defaultParagraphStyle]);
  _linkTextAttributes = [[NSDictionary alloc] initWithObjectsAndKeys:
                          [NSNumber numberWithBool: YES], NSUnderlineStyleAttributeName,
                          [NSColor blueColor], NSForegroundColorAttributeName,
                          nil];

  _tf.draws_background = YES;
  _tf.is_horizontally_resizable = NO;
  _tf.is_vertically_resizable = NO;

  /* We set defaults for all shared attributes here. If container is already
  part of a text network, we reset the attributes in -setTextContainer:. */
  _tf.is_field_editor = NO;
  _tf.is_editable = YES;
  _tf.is_selectable = YES;
  _tf.is_rich_text = YES;
  _tf.imports_graphics = NO;
  _tf.uses_font_panel = YES;
  _tf.uses_ruler = YES;
  _tf.is_ruler_visible = NO;
  _tf.allows_undo = NO;
  _tf.smart_insert_delete = YES;
  _tf.uses_find_panel = NO;
  _tf.accepts_glyph_info = NO;
  _tf.allows_document_background_color_change = YES;

  _dragTargetLocation = NSNotFound;

  _markedRange = NSMakeRange(NSNotFound, 0);
  [container setTextView: self];
  [self invalidateTextContainerOrigin];

  [self setPostsFrameChangedNotifications: YES];
  [notificationCenter addObserver: self
    selector: @selector(_updateState:)
    name: NSViewFrameDidChangeNotification
    object: self];

  [notificationCenter addObserver: self
			 selector: @selector(_textDidChange:)
			     name: NSTextDidChangeNotification
			   object: self];
  return self;
}


- (id) initWithFrame: (NSRect)frameRect
{
  NSTextContainer *aTextContainer;

  NSSize containerSize = NSMakeSize(NSWidth(frameRect), 1e7);
  aTextContainer = [self buildUpTextNetwork: containerSize];

  self = [self initWithFrame: frameRect textContainer: aTextContainer];
  [self setVerticallyResizable: YES];
  [aTextContainer setWidthTracksTextView: YES];

  /* At this point the situation is as follows: 

     textView (us)  --RETAINs--> textStorage
     textStorage    --RETAINs--> layoutManager 
     layoutManager  --RETAINs--> textContainer 
     textContainer  --RETAINs--> textView (us) */

  /* The text system should be destroyed when the textView (us) is
     released.  To get this result, we send a RELEASE message to us
     breaking the RETAIN cycle. */
  RELEASE(self);

  return self;
}


/*
In earlier versions, some of these ivar:s were in NSText instead of
NSTextView, and parts of their handling (including encoding and decoding)
were there. This has been fixed and the ivar:s moved here, but in a way
that makes decoding and encoding compatible with the old code.
*/
- (void) encodeWithCoder: (NSCoder *)aCoder
{
  BOOL flag;
  NSSize containerSize = [_textContainer containerSize];

  [super encodeWithCoder: aCoder];

  if ([aCoder allowsKeyedCoding])
    {
      int flags =
	(_tf.is_horizontally_resizable ? 0x01 : 0) |
	(_tf.is_vertically_resizable ? 0x02 : 0) |
	(_tf.owns_text_network ? 0x04 : 0);
      NSTextViewSharedData *tvsd =
	[[NSTextViewSharedData alloc] initWithTextView: self];

      [aCoder encodeConditionalObject: _delegate forKey: @"NSDelegate"];
      [aCoder encodeSize: [self maxSize] forKey: @"NSMaxSize"];
      [aCoder encodeSize: [self minSize] forKey: @"NSMinSize"];
      [aCoder encodeObject: tvsd forKey: @"NSSharedData"];
      RELEASE(tvsd);
      [aCoder encodeObject: [self textStorage] forKey: @"NSTextStorage"];
      [aCoder encodeObject: [self textContainer] forKey: @"NSTextContainer"];
      [aCoder encodeInt: flags forKey: @"NSTVFlags"];
    }
  else
    {
      [aCoder encodeConditionalObject: _delegate];
      
      flag = _tf.is_field_editor;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _tf.is_editable;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _tf.is_selectable;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _tf.is_rich_text;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _tf.imports_graphics;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _tf.draws_background;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _tf.is_horizontally_resizable;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _tf.is_vertically_resizable;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _tf.uses_font_panel;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _tf.uses_ruler;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _tf.is_ruler_visible;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      [aCoder encodeObject: _backgroundColor];
      [aCoder encodeValueOfObjCType: @encode(NSSize) at: &_minSize];
      [aCoder encodeValueOfObjCType: @encode(NSSize) at: &_maxSize];
      flag = _tf.smart_insert_delete;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _tf.allows_undo;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];

      // version 3
      flag = _tf.uses_find_panel;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];

      // version 2
      [aCoder encodeObject: _insertionPointColor];
      [aCoder encodeValueOfObjCType: @encode(NSSize) at: &containerSize];
      flag = [_textContainer widthTracksTextView];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = [_textContainer heightTracksTextView];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];

      // version 4
      [aCoder encodeObject: _defaultParagraphStyle];
      [aCoder encodeObject: _markedTextAttributes];
      [aCoder encodeObject: _linkTextAttributes];
      flag = _tf.continuous_spell_checking;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
    }
}

- (id) initWithCoder: (NSCoder *)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (!self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {  
      if ([aDecoder containsValueForKey: @"NSDelegate"])
        {
          [self setDelegate: [aDecoder decodeObjectForKey: @"NSDelegate"]];
        }

      if ([aDecoder containsValueForKey: @"NSMaxSize"])
        {
          [self setMaxSize: [aDecoder decodeSizeForKey: @"NSMaxSize"]];
        }
      else
        {
          [self setMaxSize: NSMakeSize(_frame.size.width, GSHUGE)];            
        }

      if ([aDecoder containsValueForKey: @"NSMinSize"])
        {
          // If NSMinSize is present we want to use it.
          [self setMinSize: [aDecoder decodeSizeForKey: @"NSMinSize"]];
        }
      else if ([aDecoder containsValueForKey: @"NSMinize"])
        {
          // it's NSMinize in pre-10.3 formats.
          [self setMinSize: [aDecoder decodeSizeForKey: @"NSMinize"]];
        }
      else
        {
          [self setMinSize: _frame.size];
        }

      if ([aDecoder containsValueForKey: @"NSSharedData"])
        {
          NSTextViewSharedData *shared;
          unsigned int flags;
          
          shared = [aDecoder decodeObjectForKey: @"NSSharedData"];
          flags = [shared flags];
          ASSIGN(_insertionPointColor, [shared insertionColor]);
          ASSIGN(_backgroundColor, [shared backgroundColor]);
          ASSIGN(_defaultParagraphStyle, [shared paragraphStyle]);
          ASSIGN(_markedTextAttributes, [shared markAttributes]);
          ASSIGN(_linkTextAttributes, [shared linkAttributes]);

          _tf.is_selectable = ((0x01 & flags) > 0);
          _tf.is_editable = ((0x02 & flags) > 0);
          _tf.is_rich_text = ((0x04 & flags) > 0);
          _tf.imports_graphics = ((0x08 & flags) > 0);
          _tf.is_field_editor = ((0x10 & flags) > 0);
          _tf.uses_font_panel = ((0x20 & flags) > 0);
          _tf.is_ruler_visible = ((0x40 & flags) > 0);
	  _tf.continuous_spell_checking = ((0x80 & flags) > 0);
          _tf.uses_ruler = ((0x100 & flags) > 0);
          _tf.smart_insert_delete = ((0x200 & flags) > 0);
          _tf.allows_undo = ((0x400 & flags) > 0);	  
          _tf.draws_background = ((0x800 & flags) > 0);
	  _tf.uses_find_panel = ((0x2000 & flags) > 0);
        }
     
      if ([aDecoder containsValueForKey: @"NSTVFlags"])
        {
          int flags = [aDecoder decodeIntForKey: @"NSTVFlags"];

          _tf.is_horizontally_resizable = ((flags & 0x01) != 0);
          _tf.is_vertically_resizable = ((flags & 0x02) != 0);
          _tf.owns_text_network = ((flags & 0x04) != 0);
        }
 
      // Get the text container to retrieve the text which was previously archived
      // so that it can be inserted into the current textview.
      if ([aDecoder containsValueForKey: @"NSTextContainer"])
        { 
	  NSTextContainer *container = [aDecoder decodeObjectForKey: @"NSTextContainer"];
	  [container setTextView: self];
        }
      else
        {
	  NSTextContainer *container = [self buildUpTextNetwork: _frame.size];
	  [container setTextView: self];

          // These calls are here to make the text container aware of these settings
          [self setHorizontallyResizable: _tf.is_horizontally_resizable];
          [self setVerticallyResizable: _tf.is_vertically_resizable];
        }

      //@"NSDragTypes"

      if (_tf.owns_text_network && _textStorage != nil)
	{
	  // See initWithFrame: for comments on this RELEASE
	  RELEASE(self);
	}
    }
  else
    {
      BOOL flag;
      NSTextContainer *aTextContainer; 
      int version = [aDecoder versionForClassName: 
				  @"NSTextView"];

      /* Common stuff for all versions. */
      _delegate  = [aDecoder decodeObject];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.is_field_editor = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.is_editable = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.is_selectable = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.is_rich_text = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.imports_graphics = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.draws_background = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.is_horizontally_resizable = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.is_vertically_resizable = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.uses_font_panel = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.uses_ruler = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.is_ruler_visible = flag;
      _backgroundColor  = RETAIN([aDecoder decodeObject]);
      [aDecoder decodeValueOfObjCType: @encode(NSSize) at: &_minSize];
      [aDecoder decodeValueOfObjCType: @encode(NSSize) at: &_maxSize];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.smart_insert_delete = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _tf.allows_undo = flag;

      if (version >= 3)
	{
	  [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
	  _tf.uses_find_panel = flag;	  
	}

      /* build up the rest of the text system, which doesn't get stored 
	 <doesn't even implement the Coding protocol>. */
      aTextContainer = [self buildUpTextNetwork: _frame.size];
      [aTextContainer setTextView: (NSTextView *)self];
      /* See initWithFrame: for comments on this RELEASE */
      RELEASE(self);

      if (version >= 2)
        {
	  NSSize containerSize;
	  	  
	  _insertionPointColor  = RETAIN([aDecoder decodeObject]);
	  [aDecoder decodeValueOfObjCType: @encode(NSSize) at: &containerSize];
	  [aTextContainer setContainerSize: containerSize];
	  [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
	  [aTextContainer setWidthTracksTextView: flag];
	  [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
	  [aTextContainer setHeightTracksTextView: flag];
	}

      if (version >= 4)
        {
	  _defaultParagraphStyle  = RETAIN([aDecoder decodeObject]);
	  _markedTextAttributes  = RETAIN([aDecoder decodeObject]);
	  _linkTextAttributes  = RETAIN([aDecoder decodeObject]);
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
          _tf.continuous_spell_checking = flag;
        }
    }

  _dragTargetLocation = NSNotFound;

  [self _recacheDelegateResponses];
  [self invalidateTextContainerOrigin];

  // register for services...
  if (!did_register_for_services)
    [[self class] registerForServices];

  [self updateDragTypeRegistration];

  [self setPostsFrameChangedNotifications: YES];
  [notificationCenter addObserver: self
		      selector: @selector(_updateState:)
		      name: NSViewFrameDidChangeNotification
		      object: self];
 [notificationCenter addObserver: self
			 selector: @selector(_textDidChange:)
			     name: NSTextDidChangeNotification
			   object: self];
  return self;
}

- (void) dealloc
{
  if (_tf.owns_text_network == YES)
    {
      if (_textStorage != nil)
	{
	  /*
	   * Destroying our _textStorage releases all the text objects
	   * (us included) which means this method will be called again ...
	   * so this time round we should just return, and the rest of
	   * the deallocation can be done on the next call to dealloc.
	   *
	   * However, the dealloc methods of any subclasses should
	   * already have been called before this method is called,
	   * and those subclasses don't know how to cope with being
	   * deallocated more than once ... to deal with that we
	   * set the isa pointer so that the subclass dealloc methods
	   * won't get called again.
	   */
          GSClassSwizzle(self, [NSTextView class]);
	  DESTROY(_textStorage);
	  return;
	}
    }

  [notificationCenter removeObserver: self
    name: NSViewFrameDidChangeNotification
    object: self];
  [notificationCenter removeObserver: self
    name: NSTextDidChangeNotification
    object: self];
  [_textCheckingTimer invalidate];
  [self _stopInsertionTimer];

  [[NSRunLoop currentRunLoop] cancelPerformSelector: @selector(_updateState:)
    target: self
    argument: nil];

  if (_delegate)
    {
      [notificationCenter removeObserver: _delegate
                          name: nil
                          object: _notifObject];
    }

  DESTROY(_selectedTextAttributes);
  DESTROY(_markedTextAttributes);
  DESTROY(_insertionPointColor);
  DESTROY(_backgroundColor);
  DESTROY(_defaultParagraphStyle);
  DESTROY(_linkTextAttributes);
  DESTROY(_undoObject);

  [super dealloc];
}

/**** Managing the text network ****/

/* This should only be called by [NSTextContainer -setTextView:]. If the
text container has had its layout manager changed, it will make a dummy call
to this method and container==_textContainer. We still need to do a full
update in that case.

This is assumed to be the __only__ place where _textContainer,
_layoutManager, or _textStorage changes. Re-synchronizing the text network
is hairy, and this is the only place where it happens.

TODO: Make sure the assumption holds; might need to add more dummy calls
to this method from the text container or layout manager.
*/
- (void) setTextContainer: (NSTextContainer *)container
{
  NSUInteger i, c;
  NSArray *tcs;
  NSTextView *other;

  /* Any of these three might be nil. */
  _textContainer = container;
  _layoutManager = (NSLayoutManager *)[container layoutManager];
  if (_tf.owns_text_network)
    {
      ASSIGN(_textStorage, [_layoutManager textStorage]);
    }
  else
    {
      _textStorage = [_layoutManager textStorage];
    }

  /* Search for an existing text view attached to this layout manager. */
  tcs = [_layoutManager textContainers];
  c = [tcs count];
  for (i = 0; i < c; i++)
    {
      other = [[tcs objectAtIndex: i] textView];
      if (other && other != self)
        break;
    }

  if (i < c)
    {
      /* There is already an NSTextView attached to this text network, so
      all shared attributes, including those in the layout manager, are
      already set up. We copy the shared attributes to us. */

      _delegate = other->_delegate;
      _tf.is_field_editor = other->_tf.is_field_editor;
      _tf.is_editable = other->_tf.is_editable;
      _tf.is_selectable = other->_tf.is_selectable;
      _tf.is_rich_text = other->_tf.is_rich_text;
      _tf.imports_graphics = other->_tf.imports_graphics;
      _tf.uses_font_panel = other->_tf.uses_font_panel;
      _tf.uses_ruler = other->_tf.uses_ruler;
      _tf.is_ruler_visible = other->_tf.is_ruler_visible;
      _tf.allows_undo = other->_tf.allows_undo;
      _tf.smart_insert_delete = other->_tf.smart_insert_delete;

      /* TODO: might need to update other things */
      [self _recacheDelegateResponses];
      [self updateDragTypeRegistration];
    }
  else if (_layoutManager)
    {
      /* There is no text network, and the layout manager's attributes
      might not be set up. We reset them to standard values. */

      DESTROY(_layoutManager->_typingAttributes);

      _layoutManager->_typingAttributes = [[[self class] defaultTypingAttributes] mutableCopy];
      _layoutManager->_original_selected_range.location = NSNotFound;
      _layoutManager->_selected_range = NSMakeRange(0,0);
    }

  _currentInsertionPointMovementDirection = 0;

  [self _updateMultipleTextViews];
}

- (void) replaceTextContainer: (NSTextContainer *)newContainer
{
  NSLayoutManager *lm = _layoutManager;
  NSUInteger index = [[lm textContainers] indexOfObject: _textContainer];
  NSTextStorage *ts = _textStorage;

  RETAIN(self);
  RETAIN(ts);
  [_textContainer setTextView: nil];
  [lm removeTextContainerAtIndex: index];
  [lm insertTextContainer: newContainer atIndex: index];
  [newContainer setTextView: self];
  RELEASE(ts);
  RELEASE(self);
}

- (NSTextContainer *) textContainer
{
  return _textContainer;
}

- (NSLayoutManager *) layoutManager
{
  return _layoutManager;
}

- (NSTextStorage *) textStorage
{
  return _textStorage;
}


/**** Managing shared attributes ****/


/*
Note: You might override these methods in subclasses, as in the
following example:
 - (void) setEditable: (BOOL)flag
 {
   [super setEditable: flag];
   XXX your custom code here XXX
 }

If you override them in this way, they are automatically synchronized
between multiple text views - ie., when it is called on one, it will be
automatically called on all other text views attached to thesame layout
manager.


TODO: Not all combinations of flags are allowed, eg. editable implies
selectable, imports graphics implies rich-text, etc. In these cases, when
making a change that forces a value on another attribute, the attribute is
directly changed. Need to check if we should call the method instead. Need
to make sure syncing is handled properly in all cases.
*/


/* Helper macro for these methods. */
#define NSTEXTVIEW_SYNC \
  if (_tf.multiple_textviews && (IS_SYNCHRONIZING_FLAGS == NO)) \
    {  [self _syncTextViewsByCalling: _cmd  withFlag: flag]; \
    return; }


/* Delegate */

- (id) delegate
{
  return _delegate;
}

- (void) setDelegate: (id)anObject
{
  /* Code to allow sharing the delegate */
  if (_tf.multiple_textviews && (IS_SYNCHRONIZING_DELEGATES == NO))
    {
      /* Invoke setDelegate: on all the textviews which share this
         delegate. */
      NSArray *array;
      NSUInteger i, count;

      IS_SYNCHRONIZING_DELEGATES = YES;

      array = [_layoutManager textContainers];
      count = [array count];

      for (i = 0; i < count; i++)
	{
	  NSTextView *view;

	  view = [(NSTextContainer *)[array objectAtIndex: i] textView];
	  [view setDelegate: anObject];
	}

      IS_SYNCHRONIZING_DELEGATES = NO;
    }

  /* Now the real code to set the delegate */

  if (_delegate != nil)
    {
      [notificationCenter removeObserver: _delegate
                          name: nil
                          object: _notifObject];
    }

  _delegate = anObject;

  /* SET_DELEGATE_NOTIFICATION defined near the beginning of file */

  /* NSText notifications */
  SET_DELEGATE_NOTIFICATION(DidBeginEditing);
  SET_DELEGATE_NOTIFICATION(DidChange);
  SET_DELEGATE_NOTIFICATION(DidEndEditing);

  /* NSTextView notifications */
  SET_DELEGATE_NOTIFICATION(ViewDidChangeSelection);
  SET_DELEGATE_NOTIFICATION(ViewWillChangeNotifyingTextView);

  [self _recacheDelegateResponses];
}


/* Editable */

- (BOOL) isEditable
{
  return _tf.is_editable;
}

- (void) setEditable: (BOOL)flag
{
  NSTEXTVIEW_SYNC;
  _tf.is_editable = flag;
  if (flag)
    {
      _tf.is_selectable = YES;
    }
  if ([self shouldDrawInsertionPoint])
    {
      [self updateInsertionPointStateAndRestartTimer: YES];
    }   
  else
    {
      /* TODO: insertion point */
    }
  [self updateDragTypeRegistration];
}


/* Selectable */

- (BOOL) isSelectable
{
  return _tf.is_selectable;
}

- (void) setSelectable: (BOOL)flag
{
  NSTEXTVIEW_SYNC;
  _tf.is_selectable = flag;
  if (flag == NO)
    {
      _tf.is_editable = NO;
    }
  [self updateDragTypeRegistration];
}


/* Field editor */

- (BOOL) isFieldEditor
{
  return _tf.is_field_editor;
}

- (void) setFieldEditor: (BOOL)flag
{
  NSTEXTVIEW_SYNC;
  if (flag)
    {
      [self setHorizontallyResizable: NO]; /* TODO: why? */
      [self setVerticallyResizable: NO];
      [_textContainer setLineFragmentPadding: 0];
    }
  else
    {
      [self setHorizontallyResizable: NO]; /* TODO: why? */
      [self setVerticallyResizable: YES];
      [_textContainer setLineFragmentPadding: 5.0];
    }
  _tf.is_field_editor = flag;
}


/* Rich-text */

- (BOOL) isRichText
{
  return _tf.is_rich_text;
}

- (void) setRichText: (BOOL)flag
{
  NSTEXTVIEW_SYNC;
  _tf.is_rich_text  = flag;
  if (flag == NO)
    {
      _tf.imports_graphics = NO;
      /* TODO: convert text to plain text, ie. make attributes the same in
      all text? */
    }
  [self updateDragTypeRegistration];
}


/* Imports graphics */

- (BOOL) importsGraphics
{
  return _tf.imports_graphics;
}

- (void) setImportsGraphics: (BOOL)flag
{
  NSTEXTVIEW_SYNC;
  _tf.imports_graphics = flag;
  if (flag == YES)
    {
      _tf.is_rich_text = YES;
    }
  [self updateDragTypeRegistration];
}


/* Uses ruler */

- (BOOL) usesRuler
{
  return _tf.uses_ruler;
}

/* TODO: set ruler visible to NO if flag==NO? */
- (void) setUsesRuler: (BOOL)flag
{
  NSTEXTVIEW_SYNC;
  _tf.uses_ruler = flag;
}

/* Ruler visible (TODO: is this really supposed to be shared??) */

- (BOOL) isRulerVisible
{
  return _tf.is_ruler_visible;
}

- (void) setRulerVisible: (BOOL)flag
{
  NSScrollView *sv;
  NSRulerView *rv;

  NSTEXTVIEW_SYNC;

  sv = [self enclosingScrollView];
  _tf.is_ruler_visible = flag;
  if (sv != nil)
    {
      if (_tf.is_ruler_visible && ![sv hasHorizontalRuler])
	{
	  [sv setHasHorizontalRuler: YES];
	}
      [sv setRulersVisible: _tf.is_ruler_visible];
      if (self == [_window firstResponder] &&
          (rv = [sv horizontalRulerView]) != nil)
       {
          if (flag)
            {
              [rv setClientView: self];
            }
          else
            {
              [rv setClientView: nil];
            }
        }
    }
}


/* Uses font panel */

- (BOOL) usesFontPanel
{
  return _tf.uses_font_panel;
}

- (void) setUsesFontPanel: (BOOL)flag
{
  NSTEXTVIEW_SYNC;
  _tf.uses_font_panel = flag;
}

- (BOOL) usesFindPanel
{
  return _tf.uses_find_panel;
}

- (void) setUsesFindPanel: (BOOL)flag
{
  _tf.uses_find_panel = flag;
}


/* Smart insert/delete */

- (BOOL) smartInsertDeleteEnabled
{
  return _tf.smart_insert_delete;
}

- (void) setSmartInsertDeleteEnabled: (BOOL)flag
{
  NSTEXTVIEW_SYNC;
  _tf.smart_insert_delete = flag;
}


/* Undo */

- (BOOL) allowsUndo
{
  return _tf.allows_undo;
}

- (void) setAllowsUndo: (BOOL)flag
{
  NSTEXTVIEW_SYNC;
  _tf.allows_undo = flag;
}


/* Continuous spell checking */

- (BOOL) isContinuousSpellCheckingEnabled
{
  return _tf.continuous_spell_checking;
}

- (void) setContinuousSpellCheckingEnabled: (BOOL)flag
{
  NSTEXTVIEW_SYNC;

  if (_tf.continuous_spell_checking && !flag)
    {
      const NSRange allRange = NSMakeRange(0, [[self string] length]);

      _tf.continuous_spell_checking = 0;
      
      [_layoutManager removeTemporaryAttribute: @"NSTextChecked"
			     forCharacterRange: allRange];
      [_layoutManager removeTemporaryAttribute: NSSpellingStateAttributeName
			     forCharacterRange: allRange];
      _lastCheckedRect = NSZeroRect;

      [_textCheckingTimer invalidate];
      _textCheckingTimer = nil;
    }
  else if (!_tf.continuous_spell_checking && flag)
    {
      _tf.continuous_spell_checking = 1;

      [self _scheduleTextCheckingInVisibleRectIfNeeded];
    }
}


/* Don't need this anymore. */
#undef NSTEXTVIEW_SYNC



/**** Basic view stuff ****/

- (BOOL) isFlipped
{
  return YES;
}

- (BOOL) isOpaque
{
  if (_tf.draws_background == NO
      || _backgroundColor == nil
      || [_backgroundColor alphaComponent] < 1.0)
    return NO;
  else
    return YES;
}

- (BOOL) needsPanelToBecomeKey
{
  return _tf.is_editable || _tf.is_selectable;
}

- (BOOL) acceptsFirstResponder
{
  if (_tf.is_selectable)
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

- (BOOL) resignFirstResponder
{
  /* Check if another text view attached to the same layout manager is the
  new first responder. If so, we always let it become first responder, and
  we don't send any notifications. */
  if (_tf.multiple_textviews == YES)
    {
      id futureFirstResponder;
      NSArray *textContainers;
      NSUInteger i, count;
      
      futureFirstResponder = [_window _futureFirstResponder];
      textContainers = [_layoutManager textContainers];
      count = [textContainers count];
      for (i = 0; i < count; i++)
	{
	  NSTextContainer *container;
	  NSTextView *view;
	  
	  container = (NSTextContainer *)[textContainers objectAtIndex: i];
	  view = [container textView];

	  if (view == futureFirstResponder)
	    {
	      /* NB: We do not reset the BEGAN_EDITING flag so that no
		 spurious notification is generated. */
	      return YES;
	    }
	}
    }


  /* NB: Possible change: ask always - not only if editable - but we
     need to change NSTextField etc to allow this. */
  if ((_tf.is_editable)
      && ([_delegate respondsToSelector: @selector(textShouldEndEditing:)])
      && ([_delegate textShouldEndEditing: self] == NO))
    {
      return NO;
    }


  /* Add any clean-up stuff here */
  [self _resignRulerClient];

  if ([self shouldDrawInsertionPoint])
    {
      [self updateInsertionPointStateAndRestartTimer: NO];
    }    

  if (_layoutManager != nil)
    {
      _layoutManager->_beganEditing = NO;
    }

  /* NB: According to the doc (and to the tradition), we post this
     notification even if no real editing was actually done (only
     selection of text) [Note: in this case, no editing was started,
     so the notification does not come after a
     NSTextDidBeginEditingNotification!].  The notification only means
     that we are resigning first responder status.  This makes sense
     because many objects inside the gui need this notification anyway
     - typically, it is needed to remove a field editor (editable or
     not) when the user presses TAB to move to the next view.  Anyway
     yes, the notification name is misleading. */
  [notificationCenter postNotificationName: NSTextDidEndEditingNotification
      object: _notifObject];

  if (_tf.is_field_editor)
    {
      [[self undoManager] removeAllActions];
    }

  return YES;
}

/* Note that when this method is called, editing might already have
started (in another text view attached to the same layout manager). */
- (BOOL) becomeFirstResponder
{
  if (_tf.is_selectable == NO)
    {
      return NO;
    }

  /* Note: Notifications (NSTextBeginEditingNotification etc) are sent
  the first time the user tries to edit us. */
  [self _becomeRulerClient];

  /* Draw selection, update insertion point */
  if ([self shouldDrawInsertionPoint])
    {
      [self updateInsertionPointStateAndRestartTimer: YES];
    }

  return YES;
}

- (void) resignKeyWindow
{
  if ([self shouldDrawInsertionPoint])
    {
      [self updateInsertionPointStateAndRestartTimer: NO];
    }
}

- (void) becomeKeyWindow
{
  if ([self shouldDrawInsertionPoint])
    {
      [self updateInsertionPointStateAndRestartTimer: YES];
    }
}


/**** Unshared attributes ****/

- (NSColor *) backgroundColor
{
  return _backgroundColor;
}

- (BOOL) drawsBackground
{
  return _tf.draws_background;
}

- (void) setBackgroundColor: (NSColor *)color
{
  if (![_backgroundColor isEqual: color])
    {
      ASSIGN(_backgroundColor, color);
      [self setNeedsDisplay: YES];
    }
}

- (void) setDrawsBackground: (BOOL)flag
{
  if (_tf.draws_background != flag)
    {
      _tf.draws_background = flag;
      [self setNeedsDisplay: YES];
    }
}


- (void) setInsertionPointColor: (NSColor *)color
{
  ASSIGN(_insertionPointColor, color);
}

- (NSColor *) insertionPointColor
{
  return _insertionPointColor;
}


- (void) setSelectedTextAttributes: (NSDictionary *)attributeDictionary
{
  ASSIGN(_selectedTextAttributes, attributeDictionary);
}

- (NSDictionary *) selectedTextAttributes
{
  return _selectedTextAttributes;
}


- (void) setMarkedTextAttributes: (NSDictionary *)attributeDictionary
{
  ASSIGN(_markedTextAttributes, attributeDictionary);
}

- (NSDictionary *) markedTextAttributes
{
  return _markedTextAttributes;
}

- (NSDictionary *) linkTextAttributes
{
  return _linkTextAttributes;
}

- (void) setLinkTextAttributes: (NSDictionary *)attributeDictionary
{
  ASSIGN(_linkTextAttributes, attributeDictionary);
}

/**** Size management ****/

/*
If we are the document view of a clip view, we always have an effective
minimum size of the clip view's bounds size. This ensures that we always
fill the clip view with our background, and that we always handle clicks
events everywhere inside it (and not just on a small part of it if we are
partially filled with text and track the text container vertically).

Note that this only happens if the text view is the document view of a clip
view; if several text views are embedded in a clip view (eg. a multi-column
text document in a scroll view), we do _not_ want to extend any of the text
views to cover the entire clip view.

TODO: what if the max. size is smaller than the effective min. size?


TODO: Check and fix bounds vs. frame issues.
*/


/* TODO: The safety calls below don't protect against all misuse (doing that
would be very tricky), so is it worth having them there at all? The docs are
pretty clear about what happens (ie. breakage) if you set the flags
incorrectly. */
- (void) setHorizontallyResizable: (BOOL)flag
{
  /* Safety call */
  [_textContainer setWidthTracksTextView: !flag];
  if (flag)
    {
      NSSize size;
      NSSize inset;
      
      inset = [self textContainerInset];
      size = [_textContainer containerSize];
      size.width = MAX(_maxSize.width - (inset.width * 2.0), 0.0);
      [_textContainer setContainerSize: size];
    }

  _tf.is_horizontally_resizable = flag;
}

- (void) setVerticallyResizable: (BOOL)flag
{
  /* Safety call */
  [_textContainer setHeightTracksTextView: !flag];
  if (flag)
    {
      NSSize size;
      NSSize inset;
      
      inset = [self textContainerInset];
      size = [_textContainer containerSize];
      size.height = MAX(_maxSize.height - (inset.height * 2.0), 0.0);
      [_textContainer setContainerSize: size];
    }

  _tf.is_vertically_resizable = flag;
}

- (BOOL) isHorizontallyResizable
{
  return _tf.is_horizontally_resizable;
}

- (BOOL) isVerticallyResizable
{
  return _tf.is_vertically_resizable;
}


- (NSSize) maxSize
{
  return _maxSize;
}

- (NSSize) minSize
{
  return _minSize;
}

- (void) setMaxSize: (NSSize)newMaxSize
{
  _maxSize = newMaxSize;
}

- (void) setMinSize: (NSSize)newMinSize
{
  _minSize = newMinSize;
}


- (void) sizeToFit
{
  NSSize size;

  if (!_layoutManager)
    return;

  size = _bounds.size;

  if (_tf.is_horizontally_resizable || _tf.is_vertically_resizable)
    {
      NSRect r = [_layoutManager usedRectForTextContainer: _textContainer];
      NSSize s2 = NSMakeSize(NSMaxX(r), NSMaxY(r));

      if (_tf.is_horizontally_resizable)
	size.width = s2.width + 2 * _textContainerInset.width;

      if (_tf.is_vertically_resizable)
	size.height = s2.height + 2 * _textContainerInset.height;
    }

  [self setConstrainedFrameSize: size];
}

/*
TODO: There is code in TextEdit that implies that the minimum size is
mostly ignored, and that the size of the containing clip view is always
used instead. Should test on OS to find out what the proper behavior is.

UPDATE: current behavior is correct, but must be documented properly
before this TODO can be removed
*/
- (void) setConstrainedFrameSize: (NSSize)desiredSize
{
  NSSize newSize;
  NSSize effectiveMinSize = _minSize;
  NSClipView *cv = (NSClipView *)[self superview];

  if (cv && [cv isKindOfClass: [NSClipView class]] &&
      [cv documentView] == self)
    {
      NSSize b = [cv bounds].size;
      effectiveMinSize.width  = MAX(effectiveMinSize.width , b.width);
      effectiveMinSize.height = MAX(effectiveMinSize.height, b.height);
    }

  if (_tf.is_horizontally_resizable)
    {
      newSize.width = desiredSize.width;
      newSize.width = MAX(newSize.width, effectiveMinSize.width);
      newSize.width = MIN(newSize.width, _maxSize.width);
    }
  else
    {
      newSize.width  = _frame.size.width;
    }

  if (_tf.is_vertically_resizable)
    {
      newSize.height = desiredSize.height;
      newSize.height = MAX(newSize.height, effectiveMinSize.height);
      newSize.height = MIN(newSize.height, _maxSize.height);
    }
  else
    {
      newSize.height = _frame.size.height;
    }

  if (NSEqualSizes(_frame.size, newSize) == NO)
    {
      [self setFrameSize: newSize];
    }
}

- (void) resizeSubviewsWithOldSize: (NSSize)oldSize
{
  NSSize curSize = [self frame].size;

  if (_minSize.width > curSize.width)
    _minSize.width = curSize.width;
  if (_minSize.height > curSize.height)
    _minSize.height = curSize.height;

  if (_maxSize.width < curSize.width)
    _maxSize.width = curSize.width;
  if (_maxSize.height < curSize.height)
    _maxSize.height = curSize.height;

  [super resizeSubviewsWithOldSize: oldSize];
}


/**** Text container origin ****/

/*
The text container origin is the origin of the text container's coordinate
system in our coordinate system.
*/

- (void) setTextContainerInset: (NSSize)inset
{
  _textContainerInset = inset;
  [self invalidateTextContainerOrigin];
  /* We send this so our text container can react to the change (it might
  need to resize itself it it's set to track our size). */
  [notificationCenter postNotificationName: NSViewFrameDidChangeNotification
      object: self];
}

- (NSSize) textContainerInset
{
  return _textContainerInset;
}

- (NSPoint) textContainerOrigin
{
  return _textContainerOrigin;
}

/*
TODO: There used to be a bunch of complex code in here, but I couldn't
really see what it did, and I didn't see anything in the docs that imply
that anything complex needs to be done, so I removed it. Should double-check
and bring it back if necessary.
*/
- (void) invalidateTextContainerOrigin
{
  _textContainerOrigin.x = NSMinX(_bounds);
  _textContainerOrigin.x += _textContainerInset.width;

  _textContainerOrigin.y = NSMinY(_bounds);
  _textContainerOrigin.y += _textContainerInset.height;
}



/**** Methods of the NSTextInput protocol ****/

/* -selectedRange is a part of this protocol, but it's also a method in
NSText. The implementation is among the selection handling methods and not
here. */

/* TODO: currently no support for marked text */

- (NSAttributedString *) attributedSubstringFromRange: (NSRange)theRange
{
  if (theRange.location >= [_textStorage length])
    return nil;
  if (theRange.location + theRange.length > [_textStorage length])
    theRange.length = [_textStorage length] - theRange.location;
  return [_textStorage attributedSubstringFromRange: theRange];
}

// This method takes screen coordinates as input.
- (NSUInteger) characterIndexForPoint: (NSPoint)point
{
 point = [[self window] convertScreenToBase: point];
 point = [self convertPoint:point fromView: nil];
 return [self _characterIndexForPoint: point respectFraction: NO];
}
 
- (NSRange) markedRange
{
  return _markedRange;
}

- (void) setMarkedText: (id)aString 
         selectedRange: (NSRange)selRange
{
  NSAttributedString *as;

  _markedRange = selRange;

  as = [[NSAttributedString alloc] initWithString: aString
                                   attributes: [self markedTextAttributes]];
  [self replaceCharactersInRange: selRange
        withAttributedString: as];
  DESTROY(as);
}

- (BOOL) hasMarkedText
{
  return (_markedRange.location != NSNotFound);
}

- (void) unmarkText
{
  if (![self hasMarkedText])
    return;
    
  [_textStorage setAttributes: _layoutManager->_typingAttributes 
                range: _markedRange];
  _markedRange = NSMakeRange(NSNotFound, 0);
}

- (NSArray *) validAttributesForMarkedText
{
  return [NSArray arrayWithObjects: NSBackgroundColorAttributeName, 
            NSForegroundColorAttributeName, NSFontAttributeName, nil];
}

- (NSInteger) conversationIdentifier
{
  return (NSInteger)_textStorage;
}

- (NSRect) firstRectForCharacterRange: (NSRange)theRange
{
  NSUInteger rectCount = 0;
  NSRect *rects;

  if (!_layoutManager)
    return NSZeroRect;

  rects = [_layoutManager 
		      rectArrayForCharacterRange: theRange
		      withinSelectedCharacterRange: NSMakeRange(NSNotFound, 0)
		      inTextContainer: _textContainer
		      rectCount: &rectCount];

  if (rectCount)
    return rects[0];
  else
    return NSZeroRect;
}

/* Unlike NSResponder, we should _not_ send the selector down the responder
chain if we can't handle it. */
- (void) doCommandBySelector: (SEL)aSelector
{
  if (!_layoutManager)
    {
      NSBeep();
      return;
    }

  /* Give the delegate a chance to handle it. */
  if ([_delegate respondsToSelector: @selector(textView:doCommandBySelector:)]
      && [_delegate textView: _notifObject
            doCommandBySelector: aSelector])
    {
      return;
    }
  if ([self respondsToSelector: aSelector])
    {
      [self performSelector: aSelector withObject: nil];
    }
  else
    {
      NSBeep();
    }
}

/* insertString may actually be an NSAttributedString. If it is, and the
text view isn't rich-text, we ignore the attributes and use the typing
attributes.

This method is for user changes; see NSTextView_actions.m.
*/
- (void) insertText: (id)insertString
{
  NSRange insertRange = [self rangeForUserTextChange];
  NSString *string;
  BOOL isAttributed;

  if (insertRange.location == NSNotFound)
    {
      NSBeep();
      return;
    }

  if (insertString == nil)
    {
      return;
    }

  isAttributed = [insertString isKindOfClass: [NSAttributedString class]];

  if (isAttributed)
    string = [(NSAttributedString *)insertString string];
  else
    string = insertString;

  if (![self shouldChangeTextInRange: insertRange
		   replacementString: string])
    {
      return;
    }

  if (_tf.is_rich_text)
    {
      if (isAttributed)
	{
	  [_textStorage replaceCharactersInRange: insertRange
	    withAttributedString: (NSAttributedString *)insertString];
	}
      else
	{
	  [_textStorage replaceCharactersInRange: insertRange
	    withAttributedString: AUTORELEASE([[NSAttributedString alloc]
	      initWithString: insertString
	      attributes: _layoutManager->_typingAttributes])];
	}
    }
  else
    {
      if (isAttributed)
	{
	  [self replaceCharactersInRange: insertRange
	    withString: [(NSAttributedString *)insertString string]];
	}
      else
	{
	  [self replaceCharactersInRange: insertRange
	    withString: insertString];
	}
    }

  [self didChangeText];
}


/**** Text modification methods (programmatic) ****/

/* These are methods for programmatic changes, ie. they do _not_ check
with the delegate, they don't send notifications, and they work in un-
editable text views, etc. */


/*
Replace the characters in the given range with the given string.

In a non-rich-text view, we use the same attributes as the rest of the
text storage (ie. the typing attributes).

In a rich-text text view, attributes for the new characters are computed
thusly:

1. If there is a first character in the replaced range, its attributes are
   used.

   (If not, the range must have length 0.)

2. If there is a character right before the range, its attributed are used.

   (If not, the range must have location 0, so the range must be length=0,
   location=0.)

3. If there is a character after the range, we use its attributes.

   (If not, the text storage must be empty.)

4. We use the typing attributes. (Note that if the text storage is empty,
   this is the only valid case.)

Since 1. - 3. correspond to the normal NSMutableAttributedString
behavior, we only need to handle 4. explicitly, and we can detect it
by checking if the text storage is empty.

*/
- (void) replaceCharactersInRange: (NSRange)aRange
		       withString: (NSString *)aString
{
  if (aRange.location == NSNotFound) /* TODO: throw exception instead? */
    return;

  if ([_textStorage length] == 0)
    {
      NSAttributedString *as;
      as = [[NSAttributedString alloc]
               initWithString: aString
               attributes: _layoutManager->_typingAttributes];
      [_textStorage replaceCharactersInRange: aRange
                    withAttributedString: as];
      DESTROY(as);
    }
  else
    {
      [_textStorage replaceCharactersInRange: aRange
        withString: aString];
    }
}

/*
GNUstep extension. Like the above, but uses the attributes from the
string if the text view is rich-text, and otherwise the typing
attributes.
*/
- (void) replaceCharactersInRange: (NSRange)aRange
	    withAttributedString: (NSAttributedString *)aString
{
  if (aRange.location == NSNotFound) /* TODO: throw exception instead? */
    return;

  if (_tf.is_rich_text)
    {
      [_textStorage replaceCharactersInRange: aRange
        withAttributedString: aString];
    }
  else
    {
      /* Let the other method deal with the empty text storage case. */
      [self replaceCharactersInRange: aRange
        withString: [aString string]];
    }
}


/*
Some attribute-modification methods.

The range-less methods change the attribute for all text and update the
typing attributes.

The range methods, which only work in rich-text text views, only change
the attributes for the range, and do not update the typing attributes.
*/


- (void) setFont: (NSFont *)font
{
  if (!font)
    return;

  [_textStorage addAttribute: NSFontAttributeName
    value: font
    range: NSMakeRange(0,[_textStorage length])];
  [_layoutManager->_typingAttributes setObject: font
    forKey: NSFontAttributeName];
  [notificationCenter postNotificationName: NSTextViewDidChangeTypingAttributesNotification
                      object: _notifObject];
}

- (void) setFont: (NSFont *)font  range: (NSRange)aRange
{
  if (!_tf.is_rich_text || !font)
    return;

  [_textStorage addAttribute: NSFontAttributeName
    value: font
    range: aRange];
}


- (void) setAlignment: (NSTextAlignment)alignment
{
  NSParagraphStyle *style;
  NSMutableParagraphStyle *mstyle;
  
  [_textStorage setAlignment: alignment
    range: NSMakeRange(0, [_textStorage length])];

  /* Update the typing attributes. */
  style = [_layoutManager->_typingAttributes objectForKey: NSParagraphStyleAttributeName];
  if (style == nil)
    style = [self defaultParagraphStyle];

  mstyle = [style mutableCopy];

  [mstyle setAlignment: alignment];
  [_layoutManager->_typingAttributes setObject: mstyle
    forKey: NSParagraphStyleAttributeName];
  DESTROY(mstyle);
  [notificationCenter postNotificationName: NSTextViewDidChangeTypingAttributesNotification
                      object: _notifObject];
}

- (void) setAlignment: (NSTextAlignment)alignment
	       range: (NSRange)range
{
  if (!_tf.is_rich_text)
    return;

  [_textStorage setAlignment: alignment range: range];
}

- (void) setTextColor: (NSColor *)color
{
  if (!color)
    {
      [_textStorage removeAttribute: NSForegroundColorAttributeName
                    range: NSMakeRange(0, [_textStorage length])];
      [_layoutManager->_typingAttributes
                     removeObjectForKey: NSForegroundColorAttributeName];
    }
  else
    {
      [_textStorage addAttribute: NSForegroundColorAttributeName
                    value: color
                    range: NSMakeRange(0, [_textStorage length])];
      [_layoutManager->_typingAttributes setObject: color
                     forKey:  NSForegroundColorAttributeName];
    }
  [notificationCenter postNotificationName: NSTextViewDidChangeTypingAttributesNotification
                      object: _notifObject];
}

- (void) setTextColor: (NSColor *)color  range: (NSRange)aRange
{
  if (!_tf.is_rich_text)
    return;

  if (color)
    {
      [_textStorage addAttribute: NSForegroundColorAttributeName
	value: color
	range: aRange];
    }
  else
    {
      [_textStorage removeAttribute: NSForegroundColorAttributeName
	range: aRange];
    }
}

- (NSParagraphStyle *) defaultParagraphStyle
{
  return _defaultParagraphStyle;
}

- (void) setDefaultParagraphStyle: (NSParagraphStyle *)style
{
  ASSIGN(_defaultParagraphStyle, style);
}

/**** Text access methods ****/

- (NSData *) RTFDFromRange: (NSRange)aRange
{
  return [_textStorage RTFDFromRange: aRange  documentAttributes: nil];
}

- (NSData *) RTFFromRange: (NSRange)aRange
{
  return [_textStorage RTFFromRange: aRange  documentAttributes: nil];
}

- (NSString *) string
{
  return [_textStorage string];
}

- (NSUInteger) textLength
{
  return [_textStorage length];
}


- (NSFont *) font
{
  if ([_textStorage length] > 0)
    {
      return [_textStorage attribute: NSFontAttributeName
			     atIndex: 0
		      effectiveRange: NULL];
    }

  return [_layoutManager->_typingAttributes
	   objectForKey: NSFontAttributeName];
}

/*
Returns alignment of first selected paragraph (which will be the alignment
for all text in a non-rich-text text view).

Since the alignment of the typing attributes will always be the same as the
alignment of the first selected paragraph, we can simply return the typing
attributes' alignment. (TODO: double-check this assumption)
*/
- (NSTextAlignment) alignment
{
  return [[_layoutManager->_typingAttributes objectForKey: NSParagraphStyleAttributeName] 
	   alignment];
}

- (NSColor *) textColor
{
  if ([_textStorage length] > 0)
    {
      return [_textStorage attribute: NSForegroundColorAttributeName
			     atIndex: 0
		      effectiveRange: NULL];
    }
  return [_layoutManager->_typingAttributes
	   objectForKey: NSForegroundColorAttributeName];
}



/**** Pasteboard actions ****/

/*
TODO:
Move to NSTextView_actions.m?
*/

- (void) copy: (id)sender
{  
  NSMutableArray *types = [NSMutableArray array];

  if (_tf.imports_graphics)
    [types addObject: NSRTFDPboardType];

  if (_tf.is_rich_text)
    [types addObject: NSRTFPboardType];

  if ([self smartInsertDeleteEnabled] &&
      [self selectionGranularity] == NSSelectByWord)
    [types addObject: NSSmartPastePboardType];

  [types addObject: NSStringPboardType];

  [self writeSelectionToPasteboard: [NSPasteboard generalPasteboard]
			     types: types];
}

/* Copy the current font to the font pasteboard */
- (void) copyFont: (id)sender
{
  NSPasteboard *pb = [NSPasteboard pasteboardWithName: NSFontPboard];

  [self writeSelectionToPasteboard: pb
			     types: [NSArray arrayWithObject: NSFontPboardType]];
}

/* Copy the current ruler settings to the ruler pasteboard */
- (void) copyRuler: (id)sender
{
  NSPasteboard *pb = [NSPasteboard pasteboardWithName: NSRulerPboard];

  [self writeSelectionToPasteboard: pb
			     types: [NSArray arrayWithObject: NSRulerPboardType]];
}


- (void) paste: (id)sender
{
  [self readSelectionFromPasteboard: [NSPasteboard generalPasteboard]];
}

- (void) pasteFont: (id)sender
{
  NSPasteboard *pb = [NSPasteboard pasteboardWithName: NSFontPboard];

  [self readSelectionFromPasteboard: pb
			       type: NSFontPboardType];
}

- (void) pasteRuler: (id)sender
{
  NSPasteboard *pb = [NSPasteboard pasteboardWithName: NSRulerPboard];

  [self readSelectionFromPasteboard: pb
			       type: NSRulerPboardType];
}

- (void) pasteAsPlainText: (id)sender
{
  [self readSelectionFromPasteboard: [NSPasteboard generalPasteboard]
			       type: NSStringPboardType];
}

- (void) pasteAsRichText: (id)sender
{
  [self readSelectionFromPasteboard: [NSPasteboard generalPasteboard]
			       type: NSRTFPboardType];
}



/**** Handling user changes ****/

- (NSUndoManager *) undoManager
{
  NSUndoManager *undo = nil;

  if (![_delegate respondsToSelector: @selector(undoManagerForTextView:)]
      || ((undo = [_delegate undoManagerForTextView: self]) == nil))
    {
      undo = [super undoManager];
    }

  return undo;
}

/*
 * Began editing flag.  There are quite some different ways in which
 * editing can be started.  Each time editing is started, we need to check
 * with the delegate if it is OK to start editing - but we need to check 
 * only once.  So, we use a flag.  */
/* TODO: comment seems incorrect. macros are only used here. _beganEditing
flag is set to NO in -resignFirstResponder. */


/* YES when editing has already began.  If NO, then we need to ask to
   the delegate for permission to begin editing before allowing any
   change to be made.  We explicitly check for a layout manager, and
   raise an exception if not found. */
#define BEGAN_EDITING \
  (_layoutManager ? _layoutManager->_beganEditing : noLayoutManagerException ())
#define SET_BEGAN_EDITING(X) \
  if (_layoutManager != nil) _layoutManager->_beganEditing = X


/*
Whenever text is to be changed due to some user-induced action, this method
should be called with information on the change. This method will check
if the change is allowed (by checking if the text view is editable and by
asking the delegate for permission). It will return YES if the change is
allowed. It will also send notifications as necessary.

replacementString should be the string that will replace the affected range
(disregarding attributes), or nil if only attributes are changed.

If the affected range or replacement string can't be determined, pass in
NSNotFound for the range's location and nil for the string. (Ie. even if
you know the range, you should pass in NSNotFound so the delegate can tell
the difference between a pure attribute change and an unknown change.)


TODO: What if the view isn't first responder? It should be impossible for
a user change to occur in that case, but user change methods might be
called anyway.

To be safe, changes are currently always disallowed if the text view
isn't the first responder.

(2003-02-01): GNUMail does it by having an "attach" button when editing
mails. It adds an attachment to the text using -insertText:, and it is
a fairly reasonable thing to do.

Thus, if we aren't the first responder, we still proceed as normal here.
In -didChangeText, if we still aren't the first responder, we send the
TextDidEndEditing notification _without_ asking the delegate
(-; since we can't handle a NO return).
*/
- (BOOL) shouldChangeTextInRange: (NSRange)affectedCharRange
	      replacementString: (NSString *)replacementString
{
  BOOL result = YES;

  if (_tf.is_editable == NO)
    return NO;

  /*
  We need to send the textShouldBeginEditing: /
  textDidBeginEditingNotification only once.
  */

  if (BEGAN_EDITING == NO)
    {
      if (([_delegate respondsToSelector: @selector(textShouldBeginEditing:)])
	  && ([_delegate textShouldBeginEditing: _notifObject] == NO))
	return NO;
      
      SET_BEGAN_EDITING(YES);
      
      [notificationCenter postNotificationName: NSTextDidBeginEditingNotification
	object: _notifObject];
    }

  if (_tf.delegate_responds_to_should_change)
    {
      result = [_delegate textView: self
	   shouldChangeTextInRange: affectedCharRange
	         replacementString: replacementString];
    }

  if (result && [self allowsUndo])
    {
      NSUndoManager *undo;
      NSRange undoRange;
      NSAttributedString *undoString;
      NSTextViewUndoObject *undoObject;
      BOOL isTyping;
      NSEvent *event;
      static BOOL undoManagerCanCoalesce = NO;

      {
	// FIXME This code (together with undoManagerCanCoalesce) is a
	// temporary workaround to allow using an out of date version of
	// base. Removed this upon the next release of base.
	static BOOL didCheck = NO;
	if (!didCheck)
	  {
	    undoManagerCanCoalesce =
	      [NSUndoManager instancesRespondToSelector:
		 @selector(_canCoalesceUndoWithTarget:selector:object:)];
	    if (!undoManagerCanCoalesce)
	      {
		NSLog(@"This version of NSUndoManager does not\n"
		      @"support coalescing undo operations. "
		      @"Upgrade gnustep-base to r29163 or newer to\n"
		      @"get rid of this one-time warning.");
	      }
	    didCheck = YES; 
	  }
      }

      undo = [self undoManager];

      /* Coalesce consecutive typing events into a single undo action using
	 currently private undo manager functionality. An event is considered
	 a typing event if it is a keyboard event and the event's characters
	 match our replacement string.
	 Note: Typing events are coalesced only when the previous action was
	 a typing event too and the current character follows the previous one
	 immediately. We never coalesce actions when the current selection is
	 not empty. */
      event = [NSApp currentEvent];
      isTyping = [event type] == NSKeyDown
	      && [[event characters] isEqualToString: replacementString];
      if (undoManagerCanCoalesce && _undoObject)
	{
	  undoRange = [_undoObject range];
	  if (isTyping &&
	      NSMaxRange(undoRange) == affectedCharRange.location &&
	      affectedCharRange.length == 0 &&	      
	      [undo _canCoalesceUndoWithTarget: _textStorage
				      selector: @selector(_undoTextChange:)
					object: _undoObject])
	    {
	      undoRange.length += [replacementString length];
	      [_undoObject setRange: undoRange];
	      return result;
	    }
	  DESTROY(_undoObject);
        }

      // The length of the undoRange is the length of the replacement, if any.
      if (replacementString != nil)
        {
          undoRange = NSMakeRange(affectedCharRange.location, 
                                  [replacementString length]);
        }
      else
        {
          undoRange = affectedCharRange;
        }
      undoString = [self attributedSubstringFromRange: affectedCharRange];

      undoObject =
	[[NSTextViewUndoObject alloc] initWithRange: undoRange
				   attributedString: undoString];
      [undo registerUndoWithTarget: _textStorage
			  selector: @selector(_undoTextChange:)
			    object: undoObject];
      if (isTyping)
	_undoObject = undoObject;
      else
	RELEASE(undoObject);
    }

  return result;
}

- (BOOL) shouldChangeTextInRanges: (NSArray *)ranges
               replacementStrings: (NSArray *)strings
{
  // FIXME
  NSRange range;

  range = [(NSValue*)[ranges objectAtIndex: 0] rangeValue];
  return [self shouldChangeTextInRange: range  
               replacementString: [strings objectAtIndex: 0]];
}

/*
After each user-induced change, this method should be called.
*/
- (void) didChangeText
{
  [self scrollRangeToVisible: [self selectedRange]];
  [notificationCenter postNotificationName: NSTextDidChangeNotification
    object: _notifObject];

  if ([_window firstResponder] != self)
    { /* Copied from -resignFirstResponder . See comment above. */
      if ([self shouldDrawInsertionPoint])
	{
	  [self updateInsertionPointStateAndRestartTimer: NO];
	}

      if (_layoutManager != nil)
	{
	  _layoutManager->_beganEditing = NO;
	}

      [notificationCenter postNotificationName: NSTextDidEndEditingNotification
	object: _notifObject];
    }
}

/*
Returns the ranges to which various kinds of user changes should apply.
*/

- (NSRange) rangeForUserCharacterAttributeChange
{
  if (!_tf.is_editable || !_tf.uses_font_panel || !_layoutManager)
    {
      return NSMakeRange(NSNotFound, 0);
    }

  if (_tf.is_rich_text)
    {
      return _layoutManager->_selected_range;
    }
  else
    {
      return NSMakeRange(0, [_textStorage length]);
    }
}

- (NSRange) rangeForUserParagraphAttributeChange
{
  if (!_tf.is_editable || !_tf.uses_ruler || !_layoutManager)
    {
      return NSMakeRange(NSNotFound, 0);
    }

  if (_tf.is_rich_text)
    {
      return [self selectionRangeForProposedRange: _layoutManager->_selected_range
				      granularity: NSSelectByParagraph];
    }
  else
    {
      return NSMakeRange(0, [_textStorage length]);
    }
}

- (NSRange) rangeForUserTextChange
{
  if (!_tf.is_editable || !_layoutManager)
    {
      return NSMakeRange(NSNotFound, 0);
    }  

  return _layoutManager->_selected_range;
}

- (NSRange) rangeForUserCompletion
{
  NSUInteger length, location;
  NSRange range, space;

  // Get the current location.
  location = [self selectedRange].location;

  // Find the first space starting from current location, backwards.
  space = [[self string] rangeOfCharacterFromSet:
			   [NSCharacterSet whitespaceAndNewlineCharacterSet]
					 options: NSBackwardsSearch
					   range: NSMakeRange(0, location)];

  if (space.location == NSNotFound)
    {
      // No space was found.
      if (location > 0)
	{
	  // Return the range of the whole substring.
	  range = NSMakeRange(0, location);
	}
      else
	{
	  // There isn't word.
	  range = NSMakeRange(NSNotFound, 0);
	}
    }
  else
    {
      length = location - space.location - 1;

      if (length > 0)
	{
	  // Return the range of the last word.
	  range = NSMakeRange(space.location + 1, length);
	}
      else
	{
	  // There isn't word at the end.
	  range = NSMakeRange(NSNotFound, 0);
	}
    }

  return range;
}

- (NSArray *) rangesForUserCharacterAttributeChange
{
  // FIXME
  NSRange range;

  range = [self rangeForUserCharacterAttributeChange];
  if (range.location == NSNotFound)
    return nil;

  return [NSArray arrayWithObject: [NSValue valueWithRange: range]];
}

- (NSArray *) rangesForUserParagraphAttributeChange
{
  // FIXME
  NSRange range;

  range = [self rangeForUserParagraphAttributeChange];
  if (range.location == NSNotFound)
    return nil;

  return [NSArray arrayWithObject: [NSValue valueWithRange: range]];
}

- (NSArray *) rangesForUserTextChange
{
  // FIXME
  NSRange range;

  range = [self rangeForUserTextChange];
  if (range.location == NSNotFound)
    return nil;

  return [NSArray arrayWithObject: [NSValue valueWithRange: range]];
}

/**** Misc. ****/

/*
Scroll so that the beginning of the range is visible.
*/
- (void) scrollRangeToVisible: (NSRange)aRange
{
  NSRect rect, r;
  NSView *cv;
  CGFloat width;
  NSPoint p0, p1;
  NSRange ourCharRange;

  /*
  Make sure that our size is up-to-date. If the scroll is in response to
  a change, the delayed size updating might not have run yet. To avoid
  scrolling outside the view, we force an update now.

  (An alternative would be to delay the scroll, too. If problems using
  this method turn up, it could be changed.)
  */
  [self sizeToFit];

  if (_layoutManager == nil)
    return;

  /* See if the requested range lies outside of the receiver. If so
   * forward the call to the appropriate text view.
   */
  ourCharRange = [_layoutManager characterRangeForGlyphRange: [_layoutManager glyphRangeForTextContainer:
										[self textContainer]]
					    actualGlyphRange: NULL];

  if (NSMaxRange(aRange) < ourCharRange.location ||
      aRange.location > NSMaxRange(ourCharRange))
    {
      // FIXME: The following snippet could be refactored to a method called 
      // _textContainerForCharacterRange:effectiveCharacterRange:
      NSTextContainer *tc;
      NSTextView *tv;
      NSUInteger glyphIndex = [_layoutManager glyphRangeForCharacterRange: aRange
						     actualCharacterRange: NULL].location;

      // If we are asked to scroll to the empty range at the end of the string,
      // adjust the glyph index to be the last glyph, instead of one after the last glyph
      if (![_layoutManager isValidGlyphIndex: glyphIndex] && 
	  [_layoutManager isValidGlyphIndex: glyphIndex - 1])
	{
	  glyphIndex--;
	}

      tc = [_layoutManager textContainerForGlyphAtIndex: glyphIndex
							  effectiveRange: NULL];
      tv = [tc textView];

      if (tv != self)
	{
	  [tv scrollRangeToVisible: aRange];
	  return;
	}
    }

  if (aRange.length > 0)
    {
      aRange.length = 1;
      rect = [self rectForCharacterRange: aRange];
    }
  else
    {
      rect = [_layoutManager
	insertionPointRectForCharacterIndex: aRange.location
			    inTextContainer: _textContainer];
      rect.origin.x += _textContainerOrigin.x;
      rect.origin.y += _textContainerOrigin.y;
    }

  cv = [self superview];

  /*
  If we are a non-rich-text field editor in a clip view, we use some
  magic to get scrolling to behave better; if not, just scroll and return.
  */
  if (!_tf.is_field_editor || _tf.is_rich_text
      || ![cv isKindOfClass: [NSClipView class]])
    {
      [self scrollRectToVisible: rect];
      return;
    }

  /*
  The basic principle is that we want to keep as much text as possible
  visible, and we want the text to appear to keep its alignment. This is
  especially important for centered text where the auto-scrolling will
  scroll so it appears right- or left-aligned if we don't do anything here.
  We also want to avoid spurious scrolling when the user is just moving
  the insertion point.
  */

  width = [cv frame].size.width;

  /* Get the left and right edges of the text. */
  r = [_layoutManager
	insertionPointRectForCharacterIndex: 0
			    inTextContainer: _textContainer];
  p0 = r.origin;
  r = [_layoutManager
	insertionPointRectForCharacterIndex: [_textStorage length]
			    inTextContainer: _textContainer];
  p1 = r.origin;
  p1.x += r.size.width;

  /* We only try to be smart for single-line text. */
  if (p0.y != p1.y)
    {
      [self scrollRectToVisible: rect];
      return;
    }

  p0.x += _textContainerOrigin.x;
  p1.x += _textContainerOrigin.x;
  switch ([self alignment])
    {
      case NSLeftTextAlignment:
      case NSNaturalTextAlignment: /* TODO? check default writing direction
				   for language? */
      case NSJustifiedTextAlignment:
	/* We don't want blank space to the right of the text; scroll
	further to the left if possible. */
	p1.x -= width;
	if (p1.x < 0)
	  p1.x = 0;

	if (p1.x < rect.origin.x)
	  {
	    rect.size.width += rect.origin.x - p1.x;
	    rect.origin.x = p1.x;
	  }

	break;

      case NSRightTextAlignment:
	/* We don't want blank space to the left of the text; scroll further
	to the right if possible. */
	p0.x += width;
	if (p0.x > p1.x)
	  p0.x = p1.x;

	if (p0.x > NSMaxX(rect))
	  {
	    rect.size.width += p0.x - NSMaxX(rect);
	  }

	break;

      case NSCenterTextAlignment:
	/* If all the text fits in the clip view, just center on the text. */
	if (p1.x - p0.x <= width)
	  {
	    rect.origin.x = (p1.x + p0.x - width) / 2;
	    rect.size.width = width;
	  }
	else
	  {
	    /* Otherwise, fill the clip view with text; no blank space on
	    either side. */
	    CGFloat x;

	    x = p1.x - width;
	    if (x < rect.origin.x)
	      {
		rect.size.width += rect.origin.x - x;
		rect.origin.x = x;
	      }
	    x = p0.x + width;
	    if (x > NSMaxX(rect))
	      {
		rect.size.width += x - NSMaxX(rect);
	      }
	  }
	break;
    }

  [self scrollRectToVisible: rect];
}

- (NSRange) selectedRange
{
  if (!_layoutManager)
    return NSMakeRange(0, 0);
  return _layoutManager->_selected_range;
}

- (BOOL) validateMenuItem: (NSMenuItem *)item
{
  return [self validateUserInterfaceItem: item];
}

- (BOOL) validateUserInterfaceItem: (id<NSValidatedUserInterfaceItem>)item
{
  SEL action = [item action];

  // FIXME The list of validated actions below is far from complete

  if (sel_isEqual(action, @selector(cut:)) || sel_isEqual(action, @selector(delete:)))
    return [self isEditable] && [self selectedRange].length > 0;

  if (sel_isEqual(action, @selector(copy:)))
    return [self selectedRange].length > 0;

  if (sel_isEqual(action, @selector(copyFont:))
      || sel_isEqual(action, @selector(copyRuler:)))
    return [self selectedRange].location != NSNotFound;

  if (sel_isEqual(action, @selector(paste:))
      || sel_isEqual(action, @selector(pasteAsPlainText:))
      || sel_isEqual(action, @selector(pasteAsRichText:))
      || sel_isEqual(action, @selector(pasteFont:))
      || sel_isEqual(action, @selector(pasteRuler:)))
    {
      if ([self isEditable])
	{
	  NSArray *types = nil;
          NSPasteboard *pb = nil;
	  NSString *available;

	  if (sel_isEqual(action, @selector(paste:)))
            {
              types = [self readablePasteboardTypes];
              pb = [NSPasteboard generalPasteboard];
            }
	  else if (sel_isEqual(action, @selector(pasteAsPlainText:)))
            {
              types = [NSArray arrayWithObject: NSStringPboardType];
              pb = [NSPasteboard generalPasteboard];
            }
	  else if (sel_isEqual(action, @selector(pasteAsRichText:)))
            {
              types = [NSArray arrayWithObject: NSRTFPboardType];
              pb = [NSPasteboard generalPasteboard];
            }
	  else if (sel_isEqual(action, @selector(pasteFont:)))
            {
              types = [NSArray arrayWithObject: NSFontPboardType];
              pb = [NSPasteboard pasteboardWithName: NSFontPboard];
            }
	  else if (sel_isEqual(action, @selector(pasteRuler:)))
            {
              types = [NSArray arrayWithObject: NSRulerPboard];
              pb = [NSPasteboard pasteboardWithName: NSRulerPboard];
            }

	  available = [pb availableTypeFromArray: types];
	  return available != nil;
	}
      else
 	return NO;
    }

  if (sel_isEqual(action, @selector(selectAll:))
      || sel_isEqual(action, @selector(centerSelectionInVisibleArea:)))
    return [self isSelectable];

  if (sel_isEqual(action, @selector(performFindPanelAction:)))
    {
      if ([self usesFindPanel] == NO)
	{
	  return NO;
	}
      return [[GSTextFinder sharedTextFinder]
	       validateFindPanelAction: item
			  withTextView: self];
    }

  return YES;
}

/* Private, internal methods to help input method handling in some backends
(XIM, currently). Backends may override these in categories with the real
(backend-specific) handling. */

- (void) _updateInputMethodState
{
}
- (void) _updateInputMethodWithInsertionPoint: (NSPoint)insertionPoint
{
}

@end


/**** TODO: below this line hasn't been done yet ****/


/* not the same as NSMakeRange! */
static inline
NSRange MakeRangeFromAbs (unsigned a1, unsigned a2)
{
  if (a1 < a2)
    return NSMakeRange(a1, a2 - a1);
  else
    return NSMakeRange(a2, a1 - a2);
}



@implementation NSTextView (leftovers)


/**** Misc. stuff not yet categorized ****/

/*
This method is for user changes; see NSTextView_actions.m.
*/
- (void) changeColor: (id)sender
{
  NSColor *aColor = (NSColor *)[sender color];
  NSRange aRange = [self rangeForUserCharacterAttributeChange];

  // FIXME: support undo

  /* Set typing attributes */
  if (_layoutManager)
    {
      [_layoutManager->_typingAttributes setObject: aColor
					    forKey: NSForegroundColorAttributeName];
      [notificationCenter postNotificationName: NSTextViewDidChangeTypingAttributesNotification
					object: _notifObject];
    }

  if (aRange.location == NSNotFound || aRange.length == 0)
    return;

  if (![self shouldChangeTextInRange: aRange
		   replacementString: nil])
    return;
  [self setTextColor: aColor
	       range: aRange];
  [self didChangeText];
}

/*
This method is for user changes; see NSTextView_actions.m.
*/
- (void) changeFont: (id)sender
{
  NSRange foundRange;
  NSUInteger maxSelRange;
  NSRange aRange= [self rangeForUserCharacterAttributeChange];
  NSRange searchRange = aRange;
  NSFont *font;

  if (aRange.location == NSNotFound)
    return;

  if (![self shouldChangeTextInRange: aRange  replacementString: nil])
    return;

  [_textStorage beginEditing];
  for (maxSelRange = NSMaxRange (aRange);
       searchRange.location < maxSelRange;
       searchRange = NSMakeRange (NSMaxRange (foundRange),
				  maxSelRange - NSMaxRange (foundRange)))
    {
      font = [_textStorage attribute: NSFontAttributeName
			   atIndex: searchRange.location
			   longestEffectiveRange: &foundRange
			   inRange: searchRange];
      if (font != nil)
	{
	  [self setFont: [sender convertFont: font]  ofRange: foundRange];
	}
    }
  [_textStorage endEditing];
  [self didChangeText];
  /* Set typing attributes */
  font = [_layoutManager->_typingAttributes objectForKey: NSFontAttributeName];
  if (font != nil)
    {
      [_layoutManager->_typingAttributes setObject: [sender convertFont: font] 
                     forKey: NSFontAttributeName];
      [notificationCenter postNotificationName: NSTextViewDidChangeTypingAttributesNotification
                          object: _notifObject];

    }
}


/*
 * [NSText] Reading and Writing RTFD files
 */
- (BOOL) readRTFDFromFile: (NSString *)path
{
  NSAttributedString *peek;

  peek = [[NSAttributedString alloc] initWithPath: path 
				     documentAttributes: NULL];
  if (peek != nil)
    {
      if (!_tf.is_rich_text)
	{
	  [self setRichText: YES];
	}
      [self replaceCharactersInRange: NSMakeRange (0, [_textStorage length])
		withAttributedString: peek];
      RELEASE(peek);
      return YES;
    }
  return NO;
}

- (BOOL) writeRTFDToFile: (NSString *)path  atomically: (BOOL)flag
{
  NSFileWrapper *wrapper;
  NSRange range = NSMakeRange (0, [_textStorage length]);
  
  wrapper = [_textStorage RTFDFileWrapperFromRange: range  
			  documentAttributes: nil];
  return [wrapper writeToFile: path  atomically: flag  updateFilenames: YES];
}


/*
 * [NSResponder] Handle enabling/disabling of services menu items.
 */
- (id) validRequestorForSendType: (NSString *)sendType
		      returnType: (NSString *)returnType
{
  BOOL sendOK = NO;
  BOOL returnOK = NO;

  if (!_layoutManager)
    return [super validRequestorForSendType: sendType  returnType: returnType];

  if (sendType == nil)
    {
      sendOK = YES;
    }
  else if (_layoutManager->_selected_range.length && [sendType isEqual: NSStringPboardType])
    {
      sendOK = YES;
    }

  if (returnType == nil)
    {
      returnOK = YES;
    }
  else if (_tf.is_editable && [returnType isEqual: NSStringPboardType])
    {
      returnOK = YES;
    }

  if (sendOK && returnOK)
    {
      return self;
    }

  return [super validRequestorForSendType: sendType  returnType: returnType];
}


- (void) setTypingAttributes: (NSDictionary *)attrs
{
  NSDictionary *old_attrs;
  NSString *names[] = {NSParagraphStyleAttributeName,
                       NSFontAttributeName,
                       NSForegroundColorAttributeName};
  int i;

  if (_layoutManager == nil)
    return;

  if (attrs == nil)
    {
      attrs = [[self class] defaultTypingAttributes];
    }

  old_attrs = _layoutManager->_typingAttributes;

  if (_delegate)
    {
      SEL selector = @selector(textView:shouldChangeTypingAttributes:toAttributes:);
		
      if ([_delegate respondsToSelector: selector])
		  {
          if (![_delegate textView: self 
                          shouldChangeTypingAttributes: old_attrs 
                          toAttributes: attrs])
            return;
      }
    }

  _layoutManager->_typingAttributes = [[NSMutableDictionary alloc]
			    initWithDictionary: attrs];

  // make sure to keep the main attributes set.
  for (i = 0; i < 3; i++)
    {
      NSString *name = names[i];

      if ([attrs objectForKey: name] == nil)
        {
          [_layoutManager->_typingAttributes setObject: [old_attrs objectForKey: name]
                         forKey: name];
        }
    }
  RELEASE(old_attrs);

  [self updateFontPanel];
  [self updateRuler];
  [notificationCenter postNotificationName: NSTextViewDidChangeTypingAttributesNotification
                      object: _notifObject];
}

- (NSDictionary *) typingAttributes
{
  return [NSDictionary dictionaryWithDictionary: _layoutManager->_typingAttributes];
}

- (void) clickedOnLink: (id)link
	       atIndex: (NSUInteger)charIndex
{
  if (_delegate != nil)
    {
      SEL selector = @selector(textView:clickedOnLink:atIndex:);

      if ([_delegate respondsToSelector: selector])
        {
          [_delegate textView: self clickedOnLink: link atIndex: charIndex];
        }
    }
}

- (void) updateFontPanel
{
  /* Update fontPanel only if told so */
  if (_tf.uses_font_panel && _layoutManager)
    {
      NSRange longestRange;
      NSFontManager *fm = [NSFontManager sharedFontManager];
      NSFont *currentFont;

      if (_layoutManager->_selected_range.length > 0) /* Multiple chars selection */
	{
	  currentFont = [_textStorage attribute: NSFontAttributeName
				      atIndex: _layoutManager->_selected_range.location
				      longestEffectiveRange: &longestRange
				      inRange: _layoutManager->_selected_range];
	  [fm setSelectedFont: currentFont
	      isMultiple: !NSEqualRanges (longestRange, _layoutManager->_selected_range)];
	}
      else /* Just Insertion Point. */ 
	{
	  currentFont = [_layoutManager->_typingAttributes objectForKey: NSFontAttributeName];
	  [fm setSelectedFont: currentFont  isMultiple: NO];
	}
    }
}


/**** Smart insert/delete ****/

static inline BOOL
is_preceded_by_smart_char(NSString *string, NSRange range)
{
  NSUInteger start = range.location;
  return start == 0
    || [smartLeftChars characterIsMember: [string characterAtIndex: start - 1]];
}

static inline BOOL
is_followed_by_smart_char(NSString *string, NSRange range)
{
  NSUInteger end = NSMaxRange(range);
  return end == [string length]
    || [smartRightChars characterIsMember: [string characterAtIndex: end]];
}

- (NSRange) smartDeleteRangeForProposedRange: (NSRange)proposedCharRange
{
  if ([self smartInsertDeleteEnabled])
    {
      NSString *string = [_textStorage string];
      NSUInteger start = proposedCharRange.location;
      NSUInteger end = NSMaxRange(proposedCharRange);

      /* Like Mac OS X, we only propose a space character (0x20) for deletion
	 and we choose either the character preceding the selection or the one
	 after the selection, but not both. */
      if (start > 0 && [string characterAtIndex: start - 1] == ' ' &&
	  is_followed_by_smart_char(string, proposedCharRange))
	{
	  proposedCharRange.location--;
	  proposedCharRange.length++;
	}
      else if (end < [string length] && [string characterAtIndex: end] == ' ' &&
	       is_preceded_by_smart_char(string, proposedCharRange))
	{
	  proposedCharRange.length++;
	}
    }

  return proposedCharRange;
}

- (NSString *)smartInsertAfterStringForString: (NSString *)aString
			       replacingRange: (NSRange)charRange
{
  if ([self smartInsertDeleteEnabled] && aString != nil &&
      !is_followed_by_smart_char([_textStorage string], charRange))
    {
      NSUInteger l = [aString length];
      NSCharacterSet *ws = [NSCharacterSet whitespaceAndNewlineCharacterSet];
      if (l == 0 || ![ws characterIsMember: [aString characterAtIndex: l - 1]])
	return @" ";
    }
  return nil;
}

- (NSString *)smartInsertBeforeStringForString: (NSString *)aString
				replacingRange: (NSRange)charRange
{
  if ([self smartInsertDeleteEnabled] && aString != nil &&
      !is_preceded_by_smart_char([_textStorage string], charRange))
    {
      NSUInteger l = [aString length];
      NSCharacterSet *ws = [NSCharacterSet whitespaceAndNewlineCharacterSet];
      if (l == 0 || ![ws characterIsMember: [aString characterAtIndex: 0]])
	return @" ";
    }
  return nil;
}

- (void) smartInsertForString: (NSString *)aString
	       replacingRange: (NSRange)charRange
		 beforeString: (NSString **)beforeString
		  afterString: (NSString **)afterString
{
/*
Determines whether whitespace needs to be added around aString to
preserve proper spacing and punctuation when it's inserted into the
receiver's text over charRange. Returns by reference in beforeString and
afterString any whitespace that should be added, unless either or both is
nil. Both are returned as nil if aString is nil or if smart insertion and
deletion is disabled.

As part of its implementation, this method calls
-smartInsertAfterStringForString: replacingRange: and
-smartInsertBeforeStringForString: replacingRange: .To change this method's
behavior, override those two methods instead of this one.

NSTextView uses this method as necessary. You can also use it in
implementing your own methods that insert text. To do so, invoke this
method with the proper arguments, then insert beforeString, aString, and
afterString in order over charRange.
*/
  if (beforeString)
    *beforeString = [self smartInsertBeforeStringForString: aString 
			  replacingRange: charRange];

  if (afterString)
    *afterString = [self smartInsertAfterStringForString: aString 
			 replacingRange: charRange];
}

- (void) toggleSmartInsertDelete: (id)sender
{
  [self setSmartInsertDeleteEnabled: ![self smartInsertDeleteEnabled]];
}

/**** Selection management ****/

- (void) setSelectedRange: (NSRange)charRange
{
  [self setSelectedRange: charRange  
        affinity: [self selectionAffinity]
        stillSelecting: NO];
}

/**
 * Return a range of text which encompasses proposedCharRange but is
 * extended (if necessary) to match the type of selection specified by gr.
 */
- (NSRange) selectionRangeForProposedRange: (NSRange)proposedCharRange
			       granularity: (NSSelectionGranularity)gr
{
  NSUInteger index;
  NSRange aRange;
  NSRange newRange;
  NSString *string = [self string];
  NSUInteger length = [string length];

  if (NSMaxRange (proposedCharRange) > length)
    {
      proposedCharRange.length = length - proposedCharRange.location;
    }

  if (length == 0)
    {
      return proposedCharRange;
    }

  switch (gr)
    {
      case NSSelectByWord:
	index = proposedCharRange.location;
	if (index >= length)
	  {
	    index = length - 1;
	  }
	newRange = [_textStorage doubleClickAtIndex: index];
	if (proposedCharRange.length > 1)
	  {
	    index = NSMaxRange(proposedCharRange) - 1;
	    if (index >= length)
	      {
		index = length - 1;
	      }
	    aRange = [_textStorage doubleClickAtIndex: index];
	    newRange = NSUnionRange(newRange, aRange);
	  }
	return newRange;

      case NSSelectByParagraph:
	return [string lineRangeForRange: proposedCharRange];

      case NSSelectByCharacter:
      default:
	if (proposedCharRange.length == 0)
	  return proposedCharRange;

	/* Expand the beginning character */
	index = proposedCharRange.location;
	newRange = [string rangeOfComposedCharacterSequenceAtIndex: index];
	/* If the proposedCharRange is empty we only ajust the beginning */
	if (proposedCharRange.length == 0)
	  {
	    return newRange;
	  }
	/* Expand the finishing character */
	index = NSMaxRange (proposedCharRange) - 1;
	aRange = [string rangeOfComposedCharacterSequenceAtIndex: index];
	newRange.length = NSMaxRange(aRange) - newRange.location;
	return newRange;
    }
}

- (void) setSelectedRange: (NSRange)charRange
		 affinity: (NSSelectionAffinity)affinity
	   stillSelecting: (BOOL)stillSelectingFlag
{
  /*
  Note that this method might be called from the layout manager to update
  the selection after the text storage has been changed. If text was deleted,
  the old selected range might extend outside the current string. Also note
  that this happens during the processing of the changes in the text storage.
  Thus, it isn't safe to modify the text storage.
  */

  /* The `official' (the last one the delegate approved of) selected
     range before this one. */
  NSRange oldRange;
  /* If the user was interactively changing the selection, the last
     displayed selection could have been a temporary selection,
     different from the last official one: */
  NSRange oldDisplayedRange;
  NSUInteger length = [_textStorage length];

  oldDisplayedRange = _layoutManager->_selected_range;

  if (NSMaxRange(charRange) > length)
    {
      if (charRange.location > length)
	{
	  charRange = NSMakeRange(length, 0);
	}
      else
	{
	  charRange.length = length - charRange.location;
	}
    }

  if (stillSelectingFlag == YES)
    {
      /* Store the original range before the interactive selection
	 process begin.  That's because we will need to ask the delegate
	 if it's all right for him to do the change, and then notify 
	 him we did.  In both cases, we need to post the original selection 
	 together with the new one. */
      if (_layoutManager->_original_selected_range.location == NSNotFound)
	{
	  oldRange
	    = _layoutManager->_original_selected_range
	    = _layoutManager->_selected_range;
	}
    }
  else 
    {
      /* Retrieve the original range */
      if (_layoutManager->_original_selected_range.location != NSNotFound)
	{
	  oldRange = _layoutManager->_original_selected_range;
	  _layoutManager->_original_selected_range.location = NSNotFound;
	}
      else
	{
	  oldRange = _layoutManager->_selected_range;
	}
      
      /* Ask delegate to modify the range */
      if (_tf.delegate_responds_to_will_change_sel)
	{
	  charRange = [_delegate textView: _notifObject
			     willChangeSelectionFromCharacterRange: oldRange
			     toCharacterRange: charRange];
	}
    }

  /* Set the new selected range */
  _layoutManager->_selected_range = charRange;

  /* Clear the remembered position and direction for insertion point
  movement.

  Note that we must _not_ clear the index. Many movement actions will
  reset the direction to a valid value, and they assume that the index
  remains unchanged here. */
  _currentInsertionPointMovementDirection = 0;

  /* TODO: when and if to restart timer <and where to stop it before> */
  [self updateInsertionPointStateAndRestartTimer: !stillSelectingFlag];

  if (stillSelectingFlag == NO)
    {
      [self updateFontPanel];
      
      /* Insertion Point */
      if (charRange.length)
	{
	  /*
	  TODO: this is a big bottle-neck since it involves talking to
	  the pasteboard server. should do this "later"
	  */
	  // Store the selected text in the selection pasteboard
	  [self copySelection];

	  if (_tf.is_rich_text)
	    {
	      NSDictionary *dict;

	      /* always set the typing attributes to the attributes of the
		 first selected character */
	      dict = [_textStorage attributesAtIndex: charRange.location
				      effectiveRange: NULL];
	      [self setTypingAttributes: dict];
	    }

	  /* TODO: insertion point */
	}
      else  /* no selection, only insertion point */
	{
	  if (_tf.is_rich_text && length)
	    {
	      NSDictionary *dict;
	    
	      if (charRange.location > 0 &&
		  [[_textStorage string] characterAtIndex:
		      (charRange.location - 1)] != '\n')
		{
		  /* If the insertion point is after a bold word, for
		     example, we need to use bold for further
		     insertions - this is why we take the attributes
		     from range.location - 1. */
		  dict = [_textStorage attributesAtIndex:
		    (charRange.location - 1) effectiveRange: NULL];
		}
	      else
		{
		  /* Unless we are at the beginning of text - we use the 
		     first valid attributes then */
		  dict = [_textStorage attributesAtIndex: charRange.location
				       effectiveRange: NULL];
		}
	      [self setTypingAttributes: dict];
	    }
	}
    }

  if (_window != nil)
    {
      [_layoutManager invalidateDisplayForCharacterRange: oldDisplayedRange];
      [_layoutManager invalidateDisplayForCharacterRange: charRange];
    }
  
  [self setSelectionGranularity: NSSelectByCharacter];
  _layoutManager->_selectionAffinity = affinity;
  
  /* TODO: Remove the marking from marked text if the new selection is
     greater than the marked region. */
  
  if (stillSelectingFlag == NO)
    {
      NSDictionary *userInfo;

      userInfo = [NSDictionary dictionaryWithObjectsAndKeys:
				 [NSValue valueWithBytes: &oldRange 
					  objCType: @encode(NSRange)],
			       NSOldSelectedCharacterRange, nil];
       
      [notificationCenter postNotificationName: NSTextViewDidChangeSelectionNotification
	  object: _notifObject  userInfo: userInfo];
    }
}

- (NSSelectionAffinity) selectionAffinity 
{ 
  return _layoutManager->_selectionAffinity;
}

- (void) setSelectionGranularity: (NSSelectionGranularity)granularity
{
  _layoutManager->_selectionGranularity = granularity;
}

- (NSSelectionGranularity) selectionGranularity
{
  return _layoutManager->_selectionGranularity;
}

- (NSArray *) selectedRanges
{
  // FIXME
  NSRange range;

  range = [self selectedRange];
  return [NSArray arrayWithObject: [NSValue valueWithRange: range]];
    
}

- (void) setSelectedRanges: (NSArray *)ranges
{
  [self setSelectedRanges: ranges
        affinity: [self selectionAffinity]
        stillSelecting: NO];
}

- (void) setSelectedRanges: (NSArray *)ranges
                  affinity: (NSSelectionAffinity)affinity
            stillSelecting: (BOOL)flag
{
  // FIXME
  NSRange range;

  range = [(NSValue*)[ranges objectAtIndex: 0] rangeValue];
  [self setSelectedRange: range  
        affinity: affinity
        stillSelecting: flag];

}


/**** Drawing ****/

/*
TODO:
Figure out how the additional layout stuff is supposed to work.
*/

- (void) setNeedsDisplayInRect: (NSRect)rect
	 avoidAdditionalLayout: (BOOL)flag
{
  /* TODO: This is here until the layout manager is working */
  /* This is very important */
  [super setNeedsDisplayInRect: rect];
}

/* We override NSView's setNeedsDisplayInRect: */

- (void) setNeedsDisplayInRect: (NSRect)aRect
{
  [self setNeedsDisplayInRect: aRect avoidAdditionalLayout: NO];
}

- (BOOL) shouldDrawInsertionPoint
{
  if (!_layoutManager)
    return NO;
  if (_dragTargetLocation != NSNotFound)
    return YES;
  if (_layoutManager->_selected_range.length != 0)
    return NO;
  if (_tf.is_editable == NO)
    return NO;
  if ([_window isKeyWindow] == YES && [_window firstResponder] == self)
    return YES;
  return NO;
}

/*
 * It only makes real sense to call this method with `flag == YES'.
 * If you want to delete the insertion point, what you want is rather
 * to redraw what was under the insertion point - which can't be done
 * here - you need to set the rect as needing redisplay (without
 * additional layout) instead.  NB: You need to flush the window after
 * calling this method if you want the insertion point to appear on
 * the screen immediately.  This could only be needed to implement
 * blinking insertion point - but even there, it could probably be
 * done without. */
- (void) drawInsertionPointInRect: (NSRect)rect
			    color: (NSColor *)color
			 turnedOn: (BOOL)flag
{
  if (_window == nil)
    {
      return;
    }

  if (flag)
    {
      if (color == nil)
	{
	  color = _insertionPointColor;
	}
      
      [color set];
      NSRectFill (rect);
    }
  else
    {
      [_backgroundColor set];
      NSRectFill (rect);
    }
}

- (void) drawViewBackgroundInRect: (NSRect)rect
{
  if (_tf.draws_background)
    {
      [_backgroundColor set];
      NSRectFill(rect);
    }
}

- (void) drawRect: (NSRect)rect
{
  NSRange drawnRange;
  NSRect containerRect = rect;

  containerRect.origin.x -= _textContainerOrigin.x;
  containerRect.origin.y -= _textContainerOrigin.y;
  if (_layoutManager)
    {
      drawnRange = [_layoutManager glyphRangeForBoundingRect: containerRect 
                                   inTextContainer: _textContainer];
    }
  else
    {
      drawnRange = NSMakeRange(0, 0);
    }

  [self _scheduleTextCheckingInVisibleRectIfNeeded];

  /* FIXME: We should only draw inside of rect. This code is necessary
   * to remove markings of old glyphs.  These would not be removed
   * by the following call to the layout manager because that only
   * paints the background of new glyphs.  Depending on the
   * situation, there might be no new glyphs where the old glyphs
   * were!  */
  [self drawViewBackgroundInRect: [self bounds]];

  [self drawCharactersInRange: drawnRange
               forContentView: self];

  if ([self shouldDrawInsertionPoint] &&
      [NSGraphicsContext currentContextDrawingToScreen])
    {
      if (NSIntersectsRect(rect, _insertionPointRect))
        {
          [self drawInsertionPointInRect: _insertionPointRect
                color: _insertionPointColor
                turnedOn: _drawInsertionPointNow];
        }
    }

  // Remove any existing tooltips in the redrawn rectangle.
  [[GSToolTips tipsForView: self] removeToolTipsInRect: rect];
  {
    NSRange r;
    NSUInteger i = drawnRange.location;
    NSUInteger end = i + drawnRange.length;
    while (i < end)
      {
        NSUInteger rectCount;
        NSRectArray rects; 
        NSUInteger j;

        // Find the next tooltip
        id text = [_textStorage attribute: NSToolTipAttributeName
                                  atIndex: i
                           effectiveRange: &r];
        if (r.location > end) { return; }
        if (nil == text) 
          {
            i = r.location + r.length;
            continue;
          }
        // If there is one, find the rectangles it uses.
        rects = 
          [_layoutManager rectArrayForCharacterRange: r
                        withinSelectedCharacterRange: NSMakeRange(0, 0)
                                     inTextContainer: _textContainer
                                           rectCount: &rectCount];
        // Add this object as the tooltip provider for each rectangle
        for (j=0 ; j<rectCount ; j++)
          {
            [self addToolTipRect: rects[j]
                           owner: self
                        userData: nil];
          }
        i = r.location + r.length;
      }
  }
}

- (void) resetCursorRects
{
  if ([self isSelectable])
    {
      const NSRect visibleRect = [self visibleRect];

      if (!NSEqualRects(NSZeroRect, visibleRect))
	{
	  [self addCursorRect: visibleRect cursor: [NSCursor IBeamCursor]];
	}

      /**
       * Add pointing hand cursor to any visible links
       * FIXME: Also look for NSCursorAttributeName
       * FIXME: Too complex, factor out common patterns into private methods
       */
      if (_layoutManager != nil && _textContainer != nil)
	{
	  NSInteger i;
	  NSRange visibleGlyphs, visibleCharacters;
	  const NSPoint containerOrigin = [self textContainerOrigin];

	  NSRect visibleRectInContainerCoordinates = visibleRect;
	  visibleRectInContainerCoordinates.origin.x -= containerOrigin.x;
	  visibleRectInContainerCoordinates.origin.y -= containerOrigin.y;

	  visibleGlyphs = [_layoutManager glyphRangeForBoundingRectWithoutAdditionalLayout: visibleRectInContainerCoordinates
									   inTextContainer: _textContainer];
	  visibleCharacters = [_layoutManager characterRangeForGlyphRange: visibleGlyphs
							 actualGlyphRange: NULL];
	  
	  for (i = visibleCharacters.location; i < NSMaxRange(visibleCharacters); )
	    {
	      NSRange linkCharacterRange;
	      id linkValue = [[self textStorage] attribute: NSLinkAttributeName
						   atIndex: i
				     longestEffectiveRange: &linkCharacterRange
						   inRange: visibleCharacters];
	      if (linkValue != nil)
		{
		  NSUInteger count, j;
		  NSRectArray rects = [_layoutManager rectArrayForCharacterRange: linkCharacterRange
						    withinSelectedCharacterRange: linkCharacterRange
								 inTextContainer: _textContainer
								       rectCount: &count];
		  for (j = 0; j < count; j++)
		    {
		      NSRect rectInViewCoordinates = rects[j];
		      rectInViewCoordinates.origin.x += containerOrigin.x;
		      rectInViewCoordinates.origin.y += containerOrigin.y;
		      [self addCursorRect: rectInViewCoordinates
				   cursor: [NSCursor pointingHandCursor]];
		    }
		}
	      i += linkCharacterRange.length;
	    }  	  
	}
    }
}

-  (NSString*)view: (NSView *)view
  stringForToolTip: (NSToolTipTag)tag 
             point: (NSPoint)point
          userData: (void*)userData
{
  // Find the rectangle for this tag
  FOR_IN(GSTrackingRect*, rect, _tracking_rects)
    if (rect->tag == tag)
      {
        NSPoint origin;
        NSUInteger startIndex;
	id value;

        // Origin is in window coordinate space
        origin = rect->rectangle.origin;
        // Point is an offset from this origin - translate it to the window's
        // coordinate space
        point.x += origin.x;
        point.y += origin.y;
        // Then translate it to the view's coordinate space
        point = [self convertPoint: point
                          fromView: nil];
        // Find out what the corresponding text is.
        startIndex = [self _characterIndexForPoint: point
                                   respectFraction: NO];
        // Look up what the tooltip text should be.
        value = [_textStorage attribute: NSToolTipAttributeName
				atIndex: startIndex
			 effectiveRange: NULL];

	// The value of NSToolTipAttributeName should be an NSString,
	// but we deal with it being something else too.
	if (![value isKindOfClass: [NSString class]])
	  {
	    value = [value description];
	  }
	return value;
      }
  END_FOR_IN(_tracking_rects)
  return nil;
}

- (void) updateInsertionPointStateAndRestartTimer: (BOOL)restartFlag
{
  NSRect new;

  if (!_layoutManager)
    {
      _insertionPointRect = NSZeroRect;
      return;
    }

  if (_dragTargetLocation != NSNotFound)
    {
      _tf.drag_target_hijacks_insertion_point = YES;

      new = [_layoutManager
	      insertionPointRectForCharacterIndex: _dragTargetLocation
	      inTextContainer: _textContainer];

      new.origin.x += _textContainerOrigin.x;
      new.origin.y += _textContainerOrigin.y;

      /* If the insertion would extend outside the view (e.g. because it's
      just to the right of a character on the far right edge of the view,
      a common case for right-aligned text), we force it back in. */
      if (NSMaxX(new) > NSMaxX(_bounds))
	{
	  new.origin.x = NSMaxX(_bounds) - new.size.width;
	}
    }
  else if (_layoutManager->_selected_range.length > 0 ||
      _layoutManager->_selected_range.location == NSNotFound ||
      !restartFlag)
    {
      new = NSZeroRect;
    }
  else
    {
      new = [_layoutManager
	      insertionPointRectForCharacterIndex: _layoutManager->_selected_range.location
	      inTextContainer: _textContainer];

      new.origin.x += _textContainerOrigin.x;
      new.origin.y += _textContainerOrigin.y;

      /* If the insertion would extend outside the view (e.g. because it's
      just to the right of a character on the far right edge of the view,
      a common case for right-aligned text), we force it back in. */
      if (NSMaxX(new) > NSMaxX(_bounds))
	{
	  new.origin.x = NSMaxX(_bounds) - new.size.width;
	}
    }

  /* Handle hijacked insertion point (either set above when entering this
     method or during the previous call to this method) */
  if (_tf.drag_target_hijacks_insertion_point)
    {
      _drawInsertionPointNow = NO;
      [self setNeedsDisplayInRect: _insertionPointRect
	    avoidAdditionalLayout: YES];
      _insertionPointRect = new;

      if (_dragTargetLocation != NSNotFound)
        {
	  _drawInsertionPointNow = YES;
	  [self setNeedsDisplayInRect: _insertionPointRect
		avoidAdditionalLayout: YES];
	}
      else
	_tf.drag_target_hijacks_insertion_point = NO;
    }

  /* Otherwise, draw insertion point only if there is a need to do so */
  else if ([self shouldDrawInsertionPoint] || _drawInsertionPointNow)
    {
      if (restartFlag)
        {
	  /* Start blinking timer if not yet started */
	  if (_insertionPointTimer == nil  &&  [self shouldDrawInsertionPoint])
	    {
              // Save new insertion point rectangle before starting the insertion timer...
	      _insertionPointRect = new;
              [self _startInsertionTimer];
	    }
	  else if (_insertionPointTimer != nil)
	    {
	      if (!NSEqualRects(new, _insertionPointRect))
	        {
                  // Erase previous insertion point line...
		  _drawInsertionPointNow = NO;
		  [self setNeedsDisplayInRect: _insertionPointRect
		    avoidAdditionalLayout: YES];
		  _insertionPointRect = new;
		}
	    }

	  /* Ok - blinking has just been turned on.  Make sure we start
	   * the on/off/on/off blinking from the 'on', because in that way
	   * the user can see where the insertion point is as soon as
	   * possible.  
	   */
	  _drawInsertionPointNow = YES;
	  [self setNeedsDisplayInRect: _insertionPointRect
		avoidAdditionalLayout: YES];
	}
      else if ([self shouldDrawInsertionPoint] && (_insertionPointTimer != nil))
        {
	  // restartFlag is set to NO when control resigns first responder
	  // status or window resings key window status. So we invalidate
	  // timer to  avoid extra method calls
          [self _stopInsertionTimer];

	  _drawInsertionPointNow = NO;
	  [self setNeedsDisplayInRect: _insertionPointRect
		avoidAdditionalLayout: YES];

	  _insertionPointRect = new;
	}

      [self _updateInputMethodWithInsertionPoint: _insertionPointRect.origin];
    }
}

/**** Pagination *****/

- (void) adjustPageHeightNew: (CGFloat*)newBottom
			 top: (CGFloat)oldTop
		      bottom: (CGFloat)oldBottom
		       limit: (CGFloat)bottomLimit
{
  BOOL needsToMoveBottom = NO;

  NSRect proposedPage = NSMakeRect([self bounds].origin.x,
				   oldTop,
				   [self bounds].size.width,
				   oldBottom - oldTop);

  NSRange pageGlyphRange = [_layoutManager glyphRangeForBoundingRect: proposedPage
						     inTextContainer: _textContainer];

  CGFloat actualTextBottom = oldBottom; // the maximum Y value of text less than oldBottom

  NSInteger i;
  for (i = NSMaxRange(pageGlyphRange) - 1; i >= (NSInteger)pageGlyphRange.location; )
    {
      NSRange lineFragGlyphRange = NSMakeRange(0, 0);
      NSRect lineFragRect = [_layoutManager lineFragmentRectForGlyphAtIndex: i
						      effectiveRange: &lineFragGlyphRange];

      if (NSMaxY(lineFragRect) <= NSMaxY(proposedPage))
	{
	  actualTextBottom = NSMaxY(lineFragRect);
	  break;
	}
      else
	{
	  // We encountered a visible glyph fragment which extents below
	  // the bottom of the page
	  needsToMoveBottom = YES;
	}

      i = lineFragGlyphRange.location - 1;
    }

  if (needsToMoveBottom)
    {
      *newBottom = actualTextBottom;
    }
  else
    {
      *newBottom = oldBottom;
    }

  // FIXME: Do we need a special case so text attachments aren't split in half?
}

- (CGFloat)heightAdjustLimit
{
  return 1.0;
}



/**** Ruler handling ****/

/**
 * Mote movement of marker
 */
- (void) rulerView: (NSRulerView *)ruler
     didMoveMarker: (NSRulerMarker *)marker
{
  NSTextTab *old_tab = (NSTextTab*)[marker representedObject];
  NSTextTab *new_tab = [[NSTextTab alloc] initWithType: [old_tab tabStopType]
					  location: [marker markerLocation]];
  NSRange range = [self rangeForUserParagraphAttributeChange];
  NSUInteger loc = range.location;
  NSParagraphStyle *style;
  NSMutableParagraphStyle *mstyle;

  [_textStorage beginEditing];
  while (loc < NSMaxRange(range))
    {
      id	value;
      NSRange	effRange;
      NSRange	newRange;

      value = [_textStorage attribute: NSParagraphStyleAttributeName
		      atIndex: loc
	       effectiveRange: &effRange];
      newRange = NSIntersectionRange (effRange, range);

      if (value == nil)
        {
          value = [self defaultParagraphStyle];
        }

      value = [value mutableCopy];
      [value removeTabStop: old_tab];
      [value addTabStop: new_tab];

      [_textStorage addAttribute: NSParagraphStyleAttributeName
		   value: value
		   range: newRange];
      RELEASE(value);
      loc = NSMaxRange (effRange);
    }
  [_textStorage endEditing];
  [self didChangeText];

  // Set the typing attributes
  style = [_layoutManager->_typingAttributes objectForKey: NSParagraphStyleAttributeName];
  if (style == nil)
    style = [self defaultParagraphStyle];

  mstyle = [style mutableCopy];

  [mstyle removeTabStop: old_tab];
  [mstyle addTabStop: new_tab];
  // TODO: Should use setTypingAttributes
  [_layoutManager->_typingAttributes setObject: mstyle forKey: NSParagraphStyleAttributeName];
  [notificationCenter postNotificationName: NSTextViewDidChangeTypingAttributesNotification
                      object: _notifObject];
  RELEASE(mstyle);

  [marker setRepresentedObject: new_tab];
  RELEASE(new_tab);
}

/**
 * Handle removal of marker.
 */
- (void) rulerView: (NSRulerView *)ruler
   didRemoveMarker: (NSRulerMarker *)marker
{
  NSTextTab *tab = (NSTextTab *)[marker representedObject];
  NSRange range = [self rangeForUserParagraphAttributeChange];
  NSUInteger loc = range.location;
  NSParagraphStyle *style;
  NSMutableParagraphStyle *mstyle;

  [_textStorage beginEditing];
  while (loc < NSMaxRange(range))
    {
      id	value;
      NSRange	effRange;
      NSRange	newRange;

      value = [_textStorage attribute: NSParagraphStyleAttributeName
		      atIndex: loc
	       effectiveRange: &effRange];
      newRange = NSIntersectionRange (effRange, range);

      if (value == nil)
        {
          value = [self defaultParagraphStyle];
        }

      value = [value mutableCopy];
      [value removeTabStop: tab];

      [_textStorage addAttribute: NSParagraphStyleAttributeName
		   value: value
		   range: newRange];
      RELEASE(value);
      loc = NSMaxRange (effRange);
    }
  [_textStorage endEditing];
  [self didChangeText];

  // Set the typing attributes
  style = [_layoutManager->_typingAttributes objectForKey: NSParagraphStyleAttributeName];
  if (style == nil)
    style = [self defaultParagraphStyle];

  mstyle = [style mutableCopy];

  [mstyle removeTabStop: tab];
  // TODO: Should use setTypingAttributes
  [_layoutManager->_typingAttributes setObject: mstyle forKey: NSParagraphStyleAttributeName];
  [notificationCenter postNotificationName: NSTextViewDidChangeTypingAttributesNotification
                      object: _notifObject];
  RELEASE(mstyle);
}

- (void) rulerView: (NSRulerView *)ruler 
      didAddMarker: (NSRulerMarker *)marker
{
  NSTextTab *old_tab = (NSTextTab *)[marker representedObject];
  NSTextTab *new_tab = [[NSTextTab alloc] initWithType: [old_tab tabStopType]
					  location: [marker markerLocation]];
  NSRange range = [self rangeForUserParagraphAttributeChange];
  NSUInteger loc = range.location;
  NSParagraphStyle *style;
  NSMutableParagraphStyle *mstyle;

  [_textStorage beginEditing];
  while (loc < NSMaxRange(range))
    {
      id	value;
      NSRange	effRange;
      NSRange	newRange;

      value = [_textStorage attribute: NSParagraphStyleAttributeName
		      atIndex: loc
	       effectiveRange: &effRange];
      newRange = NSIntersectionRange (effRange, range);

      if (value == nil)
        {
          value = [self defaultParagraphStyle];
        }
      
      value = [value mutableCopy];
      [value addTabStop: new_tab];

      [_textStorage addAttribute: NSParagraphStyleAttributeName
		   value: value
		   range: newRange];
      RELEASE(value);
      loc = NSMaxRange (effRange);
    }
  [_textStorage endEditing];
  [self didChangeText];

  // Set the typing attributes
  style = [_layoutManager->_typingAttributes objectForKey: NSParagraphStyleAttributeName];
  if (style == nil)
    style = [self defaultParagraphStyle];

  mstyle = [style mutableCopy];

  [mstyle addTabStop: new_tab];
  // TODO: Should use setTypingAttributes
  [_layoutManager->_typingAttributes setObject: mstyle forKey: NSParagraphStyleAttributeName];
  [notificationCenter postNotificationName: NSTextViewDidChangeTypingAttributesNotification
                      object: _notifObject];
  RELEASE(mstyle);

  [marker setRepresentedObject: new_tab];
  RELEASE(new_tab);
}

/**
 * Set new marker position from mouse down location.
 */
- (void) rulerView: (NSRulerView *)ruler
   handleMouseDown: (NSEvent *)event
{
  NSPoint point = [ruler convertPoint: [event locationInWindow] 
			     fromView: nil];
  CGFloat location = point.x;
  NSRulerMarker *marker = [[NSRulerMarker alloc] 
			      initWithRulerView: ruler
			      markerLocation: location
			      image: [NSImage imageNamed: @"common_LeftTabStop"]
			      imageOrigin: NSMakePoint(0, 0)];
  NSTextTab *tab = [[NSTextTab alloc] initWithType: NSLeftTabStopType
					  location: location];

  [marker setRepresentedObject: tab];
  [ruler trackMarker: marker withMouseEvent: event];
  RELEASE(marker);
  RELEASE(tab);
}

/**
 * Return YES if the marker should be added, NO otherwise.
 */
- (BOOL) rulerView: (NSRulerView *)ruler
   shouldAddMarker: (NSRulerMarker *)marker
{
  return [self shouldChangeTextInRange:
    [self rangeForUserParagraphAttributeChange] replacementString: nil];
}


/**
 * Return YES if the marker should be moved, NO otherwise.
 */
- (BOOL) rulerView: (NSRulerView *)ruler
  shouldMoveMarker: (NSRulerMarker *)marker
{
  return [self shouldChangeTextInRange:
    [self rangeForUserParagraphAttributeChange] replacementString: nil];
}

/**
 * Return YES if the marker should be removed, NO otherwise.
 */
- (BOOL) rulerView: (NSRulerView *)ruler
shouldRemoveMarker: (NSRulerMarker *)marker
{
  return [(NSObject *)[marker representedObject] isKindOfClass: [NSTextTab class]];
}

/**
 * Return a position for adding by constraining the specified location.
 */
- (CGFloat) rulerView: (NSRulerView *)ruler
        willAddMarker: (NSRulerMarker *)marker
           atLocation: (CGFloat)location
{
  NSSize size = [_textContainer containerSize];

  if (location < 0.0)
    return 0.0;

  if (location > size.width)
    return size.width;

  return location;
}

/**
 * Return a new position by constraining the specified location.
 */
- (CGFloat) rulerView: (NSRulerView *)ruler
       willMoveMarker: (NSRulerMarker *)marker
           toLocation: (CGFloat)location
{
  NSSize size = [_textContainer containerSize];

  if (location < 0.0)
    return 0.0;

  if (location > size.width)
    return size.width;

  return location;
}

- (void) updateRuler
{
  NSScrollView *sv;
  NSRulerView *rv;

  /* Update ruler view only if told so */
  if (_tf.uses_ruler && _tf.is_ruler_visible &&
      (sv = [self enclosingScrollView]) != nil && 
      (rv = [sv horizontalRulerView]) != nil &&
      [rv clientView] == self)
    {
      NSParagraphStyle *paraStyle;
      NSArray *makers;

      if (_layoutManager->_selected_range.length > 0) /* Multiple chars selection */
	{
	  paraStyle = [_textStorage attribute: NSParagraphStyleAttributeName
			     atIndex: _layoutManager->_selected_range.location
			     effectiveRange: NULL];
	}
      else
        {
	  paraStyle = [_layoutManager->_typingAttributes objectForKey: 
					     NSParagraphStyleAttributeName];
	}

      makers = [_layoutManager rulerMarkersForTextView: self
			       paragraphStyle: paraStyle
			       ruler: rv];
      [rv setMarkers: makers];
    }
}


/**** Spell checking ****/

- (void) checkSpelling: (id)sender
{
  NSSpellChecker *sp = [NSSpellChecker sharedSpellChecker];
  NSString *misspelledWord = nil;
  NSRange errorRange;
  int count = 0;

  if (sp == nil)
    {
      return;
    }
  errorRange = [sp checkSpellingOfString: [self string]
		              startingAt: NSMaxRange (_layoutManager->_selected_range)
		                language: [sp language]
		                    wrap: YES
		  inSpellDocumentWithTag: [self spellCheckerDocumentTag]
		               wordCount: &count];
  
  if (errorRange.length)
    {
      [self setSelectedRange: errorRange];
      [self scrollRangeToVisible: errorRange];

      misspelledWord = [[self string] substringFromRange: errorRange];
      [sp updateSpellingPanelWithMisspelledWord: misspelledWord];
    }
  else
    {
      [sp updateSpellingPanelWithMisspelledWord: @""];
    }
}

- (void) changeSpelling: (id)sender
{
  [self insertText: [[(NSControl *)sender selectedCell] stringValue]];
}

- (void) ignoreSpelling: (id)sender
{
  NSSpellChecker *sp = [NSSpellChecker sharedSpellChecker];

  [sp ignoreWord: [[(NSControl *)sender selectedCell] stringValue]
      inSpellDocumentWithTag: [self spellCheckerDocumentTag]];
}


- (NSInteger) spellCheckerDocumentTag
{
  if (!_spellCheckerDocumentTag)
    _spellCheckerDocumentTag = [NSSpellChecker uniqueSpellDocumentTag];

  return _spellCheckerDocumentTag;
}


/**** Pasteboard handling ****/

- (NSString *) preferredPasteboardTypeFromArray: (NSArray *)availableTypes
		    restrictedToTypesFromArray: (NSArray *)allowedTypes
{
  NSEnumerator *enumerator;
  NSString *type;

  if (availableTypes == nil)
    return nil;

  if (allowedTypes == nil)
    return [availableTypes objectAtIndex: 0];
    
  enumerator = [allowedTypes objectEnumerator];
  while ((type = [enumerator nextObject]) != nil)
    {
      if ([availableTypes containsObject: type])
        {
	  return type;
        }
    }
  return nil;  
}

- (BOOL) readSelectionFromPasteboard: (NSPasteboard *)pboard
{
/*
  Reads the text view's preferred type of data from the pasteboard
  specified by the pboard parameter. This method invokes the
  preferredPasteboardTypeFromArray: restrictedToTypesFromArray: method
  to determine the text view's preferred type of data and then reads
  the data using the readSelectionFromPasteboard: type:
  method. Returns YES if the data was successfully read.  */
  NSString *type;

  type = [self preferredPasteboardTypeFromArray: [pboard types]
	       restrictedToTypesFromArray: [self readablePasteboardTypes]];
  
  if (type == nil)
    return NO;
  
  return [self readSelectionFromPasteboard: pboard  type: type];
}

/*
This method is for user changes; see NSTextView_actions.m.

(Note that this makes it impossible to paste programmatically, but it's
necessary to get the -shouldChangeTextInRange:replacementString: calls
right.)
*/
- (BOOL) readSelectionFromPasteboard: (NSPasteboard *)pboard
				type: (NSString *)type
{
/*
  Reads data of the given type from pboard. The new data is placed at
  the current insertion point, replacing the current selection if one
  exists.  Returns YES if the data was successfully read.

  You should override this method to read pasteboard types other than
  the default types. Use the rangeForUserTextChange method to obtain
  the range of characters (if any) to be replaced by the new data.  */
  NSRange changeRange = [self rangeForUserTextChange];

  if ([type isEqualToString: NSStringPboardType])
    {
      if (changeRange.location != NSNotFound)
        {
	  NSString *s = [pboard stringForType: NSStringPboardType];

	  if (s != nil)
	    {
	      if ([self smartInsertDeleteEnabled] &&
		  [pboard dataForType: NSSmartPastePboardType] != nil)
		{
		  NSString *after, *before;
		  [self smartInsertForString: s
			      replacingRange: changeRange
				beforeString: &before
				 afterString: &after];
		  if (before)
		    s = [before stringByAppendingString: s];
		  if (after)
		    s = [s stringByAppendingString: after];
		}
	      if ([self shouldChangeTextInRange: changeRange
			replacementString: s])
	        {
		  [self replaceCharactersInRange: changeRange
			withString: s];
		  [self didChangeText];
		  changeRange.length = [s length];
		  [self setSelectedRange: NSMakeRange(NSMaxRange(changeRange),0)];
		}
	    }
	  else
	    return NO;
	}
      return YES;
    } 

  /* TODO: When inserting attributed strings, I call
  -shouldChangeTextInRange:replacementString: with the string from the
  attributed string. Need to decide if this is correct (probably; not much
  else to do), or if I need to do something else. */

  if (_tf.is_rich_text)
    {
      if ([type isEqualToString: NSRTFPboardType])
	{
	  if (changeRange.location != NSNotFound)
	    {
	      NSMutableAttributedString *as;
	      NSData *d = [pboard dataForType: NSRTFPboardType];

	      as = [[NSMutableAttributedString alloc] initWithRTF: d
					       documentAttributes: NULL];
	      if ([self smartInsertDeleteEnabled] &&
		  [pboard dataForType: NSSmartPastePboardType] != nil)
		{
		  NSString *after, *before;

		  [self smartInsertForString: [as string]
			      replacingRange: changeRange
				beforeString: &before
				 afterString: &after];
		  if (before)
		    {
		      [as replaceCharactersInRange: NSMakeRange(0, 0)
					withString: before];
		    }
		  if (after)
		    {
		      [as replaceCharactersInRange: NSMakeRange([as length], 0)
					withString: after];
		    }
		}
	      if ([self shouldChangeTextInRange: changeRange
			      replacementString: [as string]])
		{
		  [self replaceCharactersInRange: changeRange
			    withAttributedString: as];
		  [self didChangeText];
		  changeRange.length = [as length];
		  [self setSelectedRange: NSMakeRange(NSMaxRange(changeRange),0)];
		}

	      DESTROY(as);
	    }
	  return YES;
	}
    }

  if (_tf.imports_graphics)
    {
      if ([type isEqualToString: NSRTFDPboardType])
	{
	  if (changeRange.location != NSNotFound)
	    {
	      NSMutableAttributedString *as;
	      NSData *d = [pboard dataForType: NSRTFDPboardType];

	      as = [[NSMutableAttributedString alloc] initWithRTFD: d
						documentAttributes: NULL];

	      if ([self smartInsertDeleteEnabled] &&
		  [pboard dataForType: NSSmartPastePboardType] != nil)
		{
		  NSString *after, *before;

		  [self smartInsertForString: [as string]
			      replacingRange: changeRange
				beforeString: &before
				 afterString: &after];
		  if (before)
		    {
		      [as replaceCharactersInRange: NSMakeRange(0, 0)
					withString: before];
		    }
		  if (after)
		    {
		      [as replaceCharactersInRange: NSMakeRange([as length], 0)
					withString: after];
		    }
		}
	      if ([self shouldChangeTextInRange: changeRange
			      replacementString: [as string]])
		{
		  [self replaceCharactersInRange: changeRange
			    withAttributedString: as];
		  [self didChangeText];
		  changeRange.length = [as length];
		  [self setSelectedRange: NSMakeRange(NSMaxRange(changeRange),0)];
		}

	      DESTROY(as);
	    }
	  return YES;
	}
      if ([type isEqualToString: NSTIFFPboardType])
	{
	  if (changeRange.location != NSNotFound)
	    {
	      NSData *pboardData = [pboard dataForType: NSTIFFPboardType];
	      NSFileWrapper *wrapper = [[NSFileWrapper alloc] initRegularFileWithContents: pboardData];
	      NSImage *image = [[NSImage alloc] initWithData: pboardData];
	      NSTextAttachment *attachment = [[NSTextAttachment alloc] initWithFileWrapper: wrapper];
	      NSAttributedString *as = [NSAttributedString attributedStringWithAttachment: attachment];

	      [wrapper setIcon: image];
	      /* We changed the wrapper's icon. Reset the attachment to force
	      an update of the attachment's cell's image. */
	      [attachment setFileWrapper: wrapper];
	      if ([self shouldChangeTextInRange: changeRange
			      replacementString: [as string]])
		{
		  [self replaceCharactersInRange: changeRange
			    withAttributedString: as];
		  [self didChangeText];
		  changeRange.length = [as length];
		  [self setSelectedRange: NSMakeRange(NSMaxRange(changeRange),0)];
		}
	      RELEASE(attachment);
	      RELEASE(image);
	      RELEASE(wrapper);
	    }
	  return YES;
        }
      if ([type isEqualToString: NSFileContentsPboardType])
	{
	  NSTextAttachment *attachment = [[NSTextAttachment alloc] 
					      initWithFileWrapper: 
						  [pboard readFileWrapper]];
	  NSAttributedString *as =
	    [NSAttributedString attributedStringWithAttachment: attachment];

	  if (changeRange.location != NSNotFound &&
	      [self shouldChangeTextInRange: changeRange
		replacementString: [as string]])
	    {
	      [self replaceCharactersInRange: changeRange
		withAttributedString: as];
	      [self didChangeText];
	      changeRange.length = [as length];
	      [self setSelectedRange: NSMakeRange(NSMaxRange(changeRange),0)];
	    }
	  RELEASE(attachment);
	  return YES;
	}
    }

  // color accepting
  if ([type isEqualToString: NSColorPboardType])
    {
      /* This is an attribute change, so we use a different range. */
      NSRange aRange = [self rangeForUserCharacterAttributeChange];

      if (aRange.location != NSNotFound &&
	  [self shouldChangeTextInRange: aRange
	    replacementString: nil])
	{
          NSColor *color = [NSColor colorFromPasteboard: pboard];
          NSMutableDictionary *d = [[self typingAttributes] mutableCopy];

	  [self setTextColor: color range: aRange];
	  [d setObject: color forKey: NSForegroundColorAttributeName];
	  [self setTypingAttributes: d];
	  RELEASE(d);
	  [self didChangeText];
	}

      return YES;
    }

  // font pasting
  if ([type isEqualToString: NSFontPboardType])
    {
      NSData *data = [pboard dataForType: NSFontPboardType];
      NSDictionary *dict = [NSUnarchiver unarchiveObjectWithData: data];

      if (dict != nil)
	{
	  /* This is an attribute change, so we use a different range. */
	  NSRange aRange = [self rangeForUserCharacterAttributeChange];
	  NSMutableDictionary	*d;

	  if (aRange.location != NSNotFound &&
	      [self shouldChangeTextInRange: aRange
		replacementString: nil])
	    {
	      [_textStorage addAttributes: dict range: aRange];

	      d = [[self typingAttributes] mutableCopy];
	      [d addEntriesFromDictionary: dict];
	      [self setTypingAttributes: d];
	      RELEASE(d);
	      [self didChangeText];
	    }
	  return YES;
	}
      return NO;
    }

  // ruler pasting
  if ([type isEqualToString: NSRulerPboardType])
    {
      NSData *data = [pboard dataForType: NSRulerPboardType];
      NSDictionary *dict = [NSUnarchiver unarchiveObjectWithData: data];

      if (dict != nil)
	{
	  /* This is an attribute change, so we use a different range. */
	  NSRange aRange = [self rangeForUserParagraphAttributeChange];
	  NSMutableDictionary	*d;

	  if (aRange.location != NSNotFound &&
	      [self shouldChangeTextInRange: aRange
		replacementString: nil])
	    {
	      [_textStorage addAttributes: dict range: aRange];
	      d = [[self typingAttributes] mutableCopy];
	      [d addEntriesFromDictionary: dict];
	      [self setTypingAttributes: d];
	      RELEASE(d);
	      [self didChangeText];
	    }
	  return YES;
	}
      return NO;
    }
 
  return NO;
}

- (NSArray *) readablePasteboardTypes
{
  // get default types, what are they?
  NSMutableArray *ret = [NSMutableArray arrayWithObjects: NSRulerPboardType,
    NSColorPboardType, NSFontPboardType, nil];

  if (_tf.imports_graphics)
    {
      [ret addObject: NSRTFDPboardType];
      [ret addObject: NSTIFFPboardType];
      [ret addObject: NSFileContentsPboardType];
    }
  if (_tf.is_rich_text)
    {
      [ret addObject: NSRTFPboardType];
    }
  [ret addObject: NSStringPboardType];

  return ret;
}

- (NSArray *) writablePasteboardTypes
{
  // the selected text can be written to the pasteboard with which types.
  if ([self smartInsertDeleteEnabled])
    {
      /* We write this type to the pasteboard, but it does not have a meaning
	 by itself. Therefore, we do not add it in -readablePasteboardTypes. */
      NSMutableArray *ret = [[self readablePasteboardTypes] mutableCopy];
      [ret addObject: NSSmartPastePboardType];
      return [ret autorelease];
    }
  return [self readablePasteboardTypes];
}

- (BOOL) writeSelectionToPasteboard: (NSPasteboard *)pboard
			       type: (NSString *)type
{
/*
Writes the current selection to pboard using the given type. Returns YES
if the data was successfully written. You can override this method to add
support for writing new types of data to the pasteboard. You should invoke
super's implementation of the method to handle any types of data your
overridden version does not.
*/
  BOOL ret = NO;

  if ([type isEqualToString: NSStringPboardType])
    {
      ret = [pboard setString: [[self string] substringWithRange: _layoutManager->_selected_range] 
                      forType: NSStringPboardType];
    }
  else if ([type isEqualToString: NSRTFPboardType])
    {
      ret = [pboard setData: [self RTFFromRange: _layoutManager->_selected_range]
                    forType: NSRTFPboardType];
    }
  else if ([type isEqualToString: NSRTFDPboardType])
    {
      ret = [pboard setData: [self RTFDFromRange: _layoutManager->_selected_range]
                    forType: NSRTFDPboardType];
    }
  else if ([type isEqualToString: NSSmartPastePboardType] &&
      [self selectionGranularity] == NSSelectByWord)
    {
      ret = [pboard setData: [NSData data]
			forType: NSSmartPastePboardType];
    }
  else if ([type isEqualToString: NSColorPboardType])
    {
      NSColor *color;
      
      color = [_textStorage attribute: NSForegroundColorAttributeName
                              atIndex: _layoutManager->_selected_range.location
                       effectiveRange: 0];
      if (color != nil)
        {
          [color writeToPasteboard:  pboard];
          ret = YES;
        }
    }
  else if ([type isEqualToString: NSFontPboardType])
    {
      NSDictionary *dict;
      
      dict = [_textStorage fontAttributesInRange: _layoutManager->_selected_range];
      if (dict != nil)
        {
          ret = [pboard setData: [NSArchiver archivedDataWithRootObject: dict]
                        forType: NSFontPboardType];
        }
    }
  else if ([type isEqualToString: NSRulerPboardType])
    {
      NSDictionary *dict;
      
      dict = [_textStorage rulerAttributesInRange: _layoutManager->_selected_range];
      if (dict != nil)
        {
          ret = [pboard setData: [NSArchiver archivedDataWithRootObject: dict]
                        forType: NSRulerPboardType];
        }
    }

  return ret;
}

- (BOOL) writeSelectionToPasteboard: (NSPasteboard *)pboard
			      types: (NSArray *)types
{

/* Writes the current selection to pboard under each type in the types
array. Returns YES if the data for any single type was written
successfully.

You should not need to override this method. You might need to invoke this
method if you are implementing a new type of pasteboard to handle services
other than copy/paste or dragging. */
  BOOL ret = NO;
  NSEnumerator *enumerator;
  NSString *type;

  if (types == nil)
    {
      return NO;
    }
  
  if (_layoutManager->_selected_range.location == NSNotFound)
    {
      return NO;
    }

  [pboard declareTypes: types owner: self];
    
  enumerator = [types objectEnumerator];
  while ((type = [enumerator nextObject]) != nil)
    {
      if ([self writeSelectionToPasteboard: pboard type: type])
        {
          ret = YES;
        }
    }

  return ret;
}

- (NSArray *) acceptableDragTypes
{
  return [self readablePasteboardTypes];
}

- (void) updateDragTypeRegistration
{
  /* TODO: Should change registration for all our text views

  This seems to be handled already; this will be called when the flags
  change, and those calls are made on all connected text views.
  */
  
  if (_tf.is_editable)
    [self registerForDraggedTypes: [self acceptableDragTypes]];
  else
    [self unregisterDraggedTypes];
}



/**** Drag and drop handling ****/

/*
 * Note: Dragging uses a transient insertion point distinct from the text
 * view's current selection. This allows subclasses of NSTextView to determine
 * the origin of the dragged selection during a drag operation and provides
 * better visual feedback for users, since the origin of the dragged selection
 * remains visible.
 */

// dragging of text, colors and files
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal
{
    return (NSDragOperationGeneric | NSDragOperationCopy);
}

- (void) _updateDragTargetLocation: (id <NSDraggingInfo>)sender
			 operation: (NSDragOperation *)flags
{
  if (*flags != NSDragOperationNone)
    {
      NSPoint dragPoint;
      NSUInteger dragIndex;

      dragPoint = [sender draggingLocation];
      dragPoint = [self convertPoint: dragPoint fromView: nil];
      dragIndex = [self _characterIndexForPoint: dragPoint
                                respectFraction: YES];

      if ([sender draggingSource] != self ||
          dragIndex <= [self selectedRange].location ||
          dragIndex >= NSMaxRange([self selectedRange]))
	{
	  _dragTargetLocation = dragIndex;
	}
      else
        {
	  _dragTargetLocation = NSNotFound;
	  *flags = NSDragOperationNone;
        }
    }
  else if (_dragTargetLocation != NSNotFound)
    {
      _dragTargetLocation = NSNotFound;
    }
  [self updateInsertionPointStateAndRestartTimer: NO];
  [self displayIfNeeded];
}

- (NSDragOperation) draggingEntered: (id <NSDraggingInfo>)sender
{
  NSPasteboard	*pboard = [sender draggingPasteboard];
  NSArray	*types = [self readablePasteboardTypes];
  NSString	*type = [self preferredPasteboardTypeFromArray: [pboard types]
				    restrictedToTypesFromArray: types];
  NSDragOperation flags = [self dragOperationForDraggingInfo: sender type: type];

  if (![type isEqual: NSColorPboardType])
    {
      [self _updateDragTargetLocation: sender operation: &flags];
    }
  return flags;
}

- (NSDragOperation) draggingUpdated: (id <NSDraggingInfo>)sender
{
  NSPasteboard	*pboard = [sender draggingPasteboard];
  NSArray	*types = [self readablePasteboardTypes];
  NSString	*type = [self preferredPasteboardTypeFromArray: [pboard types]
				    restrictedToTypesFromArray: types];
  NSDragOperation flags = [self dragOperationForDraggingInfo: sender type: type];

  NSPoint	dragPoint;
  NSRect	vRect;

  dragPoint = [sender draggingLocation];
  dragPoint = [self convertPoint: dragPoint fromView: nil];

  /* Note: Keep DRAGGING_SCROLL_BORDER in sync with a similar value used
   * in -draggingUpdated: of NSOutlineView and NSTableView.
   * FIXME: Is the value of DRAGGING_SCROLL_DIST reasonable?
   */
#define DRAGGING_SCROLL_BORDER 3
#define DRAGGING_SCROLL_DIST 10
  vRect = [self visibleRect];
  if (dragPoint.x <= NSMinX(vRect) + DRAGGING_SCROLL_BORDER)
    {
      vRect.origin.x -= DRAGGING_SCROLL_DIST;
      if (NSMinX(vRect) < NSMinX(_bounds))
	vRect.origin.x = NSMinX(_bounds);
    }
  else if (dragPoint.x >= NSMaxX(vRect) - DRAGGING_SCROLL_BORDER)
    {
      vRect.origin.x += DRAGGING_SCROLL_DIST;
      if (NSMaxX(vRect) > NSMaxX(_bounds))
	vRect.origin.x = NSMaxX(_bounds) - NSWidth(vRect);
    }

  if (dragPoint.y <= NSMinY(vRect) + DRAGGING_SCROLL_BORDER)
    {
      vRect.origin.y -= DRAGGING_SCROLL_DIST;
      if (NSMinY(vRect) < NSMinY(_bounds))
	vRect.origin.y = NSMinY(_bounds);
    }
  else if (dragPoint.y >= NSMaxY(vRect) - DRAGGING_SCROLL_BORDER)
    {
      vRect.origin.y += DRAGGING_SCROLL_DIST;
      if (NSMaxY(vRect) > NSMaxY(_bounds))
	vRect.origin.y = NSMaxY(_bounds) - NSHeight(vRect);
    }
  [self scrollPoint: vRect.origin];
#undef DRAGGING_SCROLL_BORDER
#undef DRAGGING_SCROLL_DIST

  if (![type isEqual: NSColorPboardType])
    {
      [self _updateDragTargetLocation: sender operation: &flags];
    }
  return flags;
}

- (void) draggingExited: (id <NSDraggingInfo>)sender
{
  NSDragOperation flags = NSDragOperationNone;
  [self _updateDragTargetLocation: sender operation: &flags];
}

- (BOOL) prepareForDragOperation: (id <NSDraggingInfo>)sender
{
  return YES;
}

- (BOOL) performDragOperation: (id <NSDraggingInfo>)sender
{
  /* In general, the position where the dragging source was dropped is given by
     _dragTargetLocation. However, when dragging a color onto a text view, the
     cursor is not updated, since the color change always effects the current
     selection and hence _dragTargetLocation==NSNotFound in that case. */
  NSSelectionGranularity gran = [self selectionGranularity];
  NSRange sourceRange = [self selectedRange];
  NSRange changeRange;
  NSPasteboard *pboard;
  NSString *type;

  if (_dragTargetLocation != NSNotFound)
    {
      changeRange = NSMakeRange(_dragTargetLocation, 0);
      _dragTargetLocation = NSNotFound;
      [self setSelectedRange: changeRange];
    }

  if ([sender draggingSource] == self &&
      ([sender draggingSourceOperationMask] & NSDragOperationGeneric))
    {
      if ([self smartInsertDeleteEnabled] && gran == NSSelectByWord)
	sourceRange = [self smartDeleteRangeForProposedRange: sourceRange];
      if (![self shouldChangeTextInRange: sourceRange replacementString: @""])
        {
	  return NO;
	}
      [self replaceCharactersInRange: sourceRange withString: @""];
    }

  changeRange = [self rangeForUserTextChange];
  pboard = [sender draggingPasteboard];
  type = [self preferredPasteboardTypeFromArray: [pboard types]
	       restrictedToTypesFromArray: [self readablePasteboardTypes]];
  if ([self readSelectionFromPasteboard: pboard type: type])
    {
      if (![type isEqual: NSColorPboardType])
	{
	  changeRange.length =
	    [self selectedRange].location - changeRange.location;
	  [self setSelectedRange: changeRange];
	}
      return YES;
    }
  return NO;
}

- (void) concludeDragOperation: (id <NSDraggingInfo>)sender
{
  [self cleanUpAfterDragOperation];
}

- (void) cleanUpAfterDragOperation
{
  // Cleanup information after dragging operation completes...
  _dragTargetLocation = NSNotFound;
  [self updateInsertionPointStateAndRestartTimer: NO];
  [self displayIfNeeded];
}

- (NSDragOperation) dragOperationForDraggingInfo: (id <NSDraggingInfo>)dragInfo 
                                            type: (NSString *)type
{
  //TODO
  return NSDragOperationCopy | NSDragOperationGeneric;
}

- (NSImage *) dragImageForSelectionWithEvent: (NSEvent *)event
				      origin: (NSPoint *)origin
{
  if (origin)
    *origin = NSMakePoint(0, 0);

  // FIXME
  return nil;
}

- (BOOL) dragSelectionWithEvent: (NSEvent *)event
			 offset: (NSSize)mouseOffset
		      slideBack: (BOOL)slideBack
{
  NSPoint point;
  NSImage *image = [self dragImageForSelectionWithEvent: event
			 origin: &point];
  NSPasteboard *pboard = [NSPasteboard pasteboardWithName: NSDragPboard];
  NSPoint location = [self convertPoint: [event locationInWindow] fromView: nil];
  NSMutableArray *types = [NSMutableArray array];

  if (_tf.imports_graphics)
    [types addObject: NSRTFDPboardType];

  if (_tf.is_rich_text)
    [types addObject: NSRTFPboardType];

  [types addObject: NSStringPboardType];
  if ([self smartInsertDeleteEnabled])
    [types addObject: NSSmartPastePboardType];

  [self writeSelectionToPasteboard: pboard types: types];

  [self dragImage: image at: location offset: mouseOffset event: event  
	pasteboard: pboard source: self slideBack: slideBack];

  return YES;
}


/**** Event handling ****/

- (void) mouseDown: (NSEvent *)theEvent
{
  BOOL canDrag = NO;
  NSSelectionAffinity affinity = [self selectionAffinity];
  NSSelectionGranularity granularity = NSSelectByCharacter;
  NSRange chosenRange, proposedRange;
  NSPoint point, startPoint;
  NSUInteger startIndex, endIndex;

  /* If non selectable then ignore the mouse down. */
  if (_tf.is_selectable == NO)
    {
      return;
    }

  if (!_layoutManager)
    return;

  /* Otherwise, NSWindow has already made us first responder (if
     possible) */

  startPoint = [self convertPoint: [theEvent locationInWindow] fromView: nil];
  startIndex = [self _characterIndexForPoint: startPoint
                             respectFraction: [theEvent clickCount] == 1];

  if ([theEvent modifierFlags] & NSShiftKeyMask)
    {
      /* Shift-click is for extending or shrinking an existing selection using 
	 the existing granularity */
      proposedRange = _layoutManager->_selected_range;
      granularity = _layoutManager->_selectionGranularity;

      /* If the clicked point is closer to the left end of the current selection
	 adjust the left end of the selected range */
      if (startIndex < proposedRange.location + proposedRange.length / 2)
	{
	  proposedRange = NSMakeRange(startIndex,
				      NSMaxRange(proposedRange) - startIndex);
	  proposedRange = [self selectionRangeForProposedRange: proposedRange
						   granularity: granularity];
	  /* Prepare for shift-dragging. Anchor is at the right end. */
	  startIndex = NSMaxRange(proposedRange);
	}
      /* otherwise, adjust the right end of the selected range */
      else
	{
	  proposedRange = NSMakeRange(proposedRange.location,
				      startIndex - proposedRange.location);
	  proposedRange = [self selectionRangeForProposedRange: proposedRange
						   granularity: granularity];
	  /* Prepare for shift-dragging. Anchor is at the left end. */
	  startIndex = proposedRange.location;
	}
    }
  else /* No shift */
    {
      switch ([theEvent clickCount])
	{
	case 1: granularity = NSSelectByCharacter;
	  break;
	case 2: granularity = NSSelectByWord;
	  break;
	case 3: granularity = NSSelectByParagraph;
	  break;
	}

      /* A single click into the selected range can start a drag operation */
      canDrag = granularity == NSSelectByCharacter
	  && NSLocationInRange(startIndex, _layoutManager->_selected_range);
      proposedRange = NSMakeRange (startIndex, 0);

      /* We manage clicks on attachments and links only on the first
	 click, so that if you double-click on them, only the first
	 click gets sent to them; the other clicks select by
	 word/paragraph as usual.  */
      if (granularity == NSSelectByCharacter)
	{
	  NSTextAttachment *attachment;

	  /* Check if the click was on an attachment cell.  */
	  attachment = [_textStorage attribute: NSAttachmentAttributeName
				       atIndex: startIndex
				effectiveRange: NULL];
	      
	  if (attachment != nil)
	    {
	      id <NSTextAttachmentCell> cell = [attachment attachmentCell];

	      if (cell != nil)
		{
		  NSRect cellFrame;
		  NSRect lfRect;
		  NSUInteger glyphIndex;

		  glyphIndex =
		    [_layoutManager
		      glyphRangeForCharacterRange: NSMakeRange(startIndex, 1)
		      actualCharacterRange: NULL].location;
		  lfRect =
		    [_layoutManager
		      lineFragmentRectForGlyphAtIndex: glyphIndex
		      effectiveRange: NULL];
		  cellFrame.origin =
		    [_layoutManager
		      locationForGlyphAtIndex: glyphIndex];
		  cellFrame.origin.x += lfRect.origin.x;
		  cellFrame.origin.y += lfRect.origin.y;
                  cellFrame.origin.x += _textContainerOrigin.x;
                  cellFrame.origin.y += _textContainerOrigin.y;
		  cellFrame.size =
		    [_layoutManager 
		      attachmentSizeForGlyphAtIndex: glyphIndex];
		  cellFrame.origin.y -= cellFrame.size.height;

		  /* TODO: What about the insertion point ? */
		  if ([cell wantsToTrackMouseForEvent: theEvent
			inRect: cellFrame
			ofView: self
			atCharacterIndex: startIndex]
		      && [cell trackMouse: theEvent
				   inRect: cellFrame 
				   ofView: self
			 atCharacterIndex: startIndex
			     untilMouseUp: NO])
		    {
		      return;
		    }
		}
	    }

	  /* This is the code for handling click event on a link (a link
	     is some chars with the NSLinkAttributeName set to something
	     which is not-null, a NSURL object usually).  */
	  {
	    /* What exactly is this link object, it's up to the
	       programmer who is using the NSTextView and who
	       originally created the link object and saved it under
	       the NSLinkAttributeName in the text.  Normally, a NSURL
	       object is used. */
	    /* TODO: should call -clickedOnLink:atIndex: instead */
	    id link = [_textStorage attribute: NSLinkAttributeName
				    atIndex: startIndex
				    effectiveRange: NULL];
	    if (link != nil  &&  _delegate != nil)
	      {
		SEL selector = @selector(textView:clickedOnLink:atIndex:);
		
		if ([_delegate respondsToSelector: selector])
		  {
		    /* Move the insertion point over the link.  */
		    chosenRange = [self selectionRangeForProposedRange: 
					  proposedRange
					granularity: granularity];

		    [self setSelectedRange: chosenRange  affinity: affinity  
			  stillSelecting: NO];

		    [self displayIfNeeded];

		    /* Now 'activate' the link.  The _delegate returns
		       YES if it handles the click, NO if it doesn't
		       -- and if it doesn't, we need to pass the click
		       to the next responder.  */
		    if ([_delegate textView: self
                              clickedOnLink: link  
                                    atIndex: startIndex])
		      {
			return;
		      }
		    else
		      {
			[super mouseDown: theEvent];
			return; 
		      }
		  }
	      }
	  }
	}
    }

  chosenRange = [self selectionRangeForProposedRange: proposedRange
                                         granularity: granularity];
  if (canDrag)
  {
    NSUInteger mask = NSLeftMouseDraggedMask | NSLeftMouseUpMask;
    NSEvent *currentEvent;

    currentEvent = [_window nextEventMatchingMask: mask
  			      untilDate: [NSDate distantFuture]
  			      inMode: NSEventTrackingRunLoopMode
  			      dequeue: NO];
    if ([currentEvent type] == NSLeftMouseDragged)
      {
  	if (![self dragSelectionWithEvent: theEvent
  				   offset: NSMakeSize(0, 0)
  				slideBack: YES])
  	  {
  	    NSBeep();
  	  }
  	return;
      }
  }
  
  /* Enter modal loop tracking the mouse */
  {
    NSUInteger mask = NSLeftMouseDraggedMask | NSLeftMouseUpMask
      | NSPeriodicMask;
    NSEvent *currentEvent;
    NSEvent *lastEvent = nil; /* Last non-periodic event. */
    NSDate *distantPast = [NSDate distantPast];
    BOOL gettingPeriodic, gotPeriodic;

    [self setSelectedRange: chosenRange  affinity: affinity  
	  stillSelecting: YES];

    currentEvent = [_window nextEventMatchingMask: mask
		     untilDate: [NSDate distantFuture]
		     inMode: NSEventTrackingRunLoopMode
		     dequeue: YES];
    gettingPeriodic = NO;
    do
      {
	gotPeriodic = NO;
	while (currentEvent && [currentEvent type] != NSLeftMouseUp)
	  {
	    if ([currentEvent type] == NSPeriodic)
	      {
		gotPeriodic = YES;
	      }
	    else
	      lastEvent = currentEvent;
	    currentEvent = [_window nextEventMatchingMask: mask
			     untilDate: distantPast
			     inMode: NSEventTrackingRunLoopMode
			     dequeue: YES];
	  }
	if (currentEvent && [currentEvent type] == NSLeftMouseUp)
	  break;

	/*
	Automatic scrolling.

	If we aren't getting periodic events, we check all events. If any of
	them causes us to scroll, we scroll and start up the periodic events.

	If we are getting periodic events, we only scroll when we receive a
	periodic event (and we use the last non-periodic event to determine
	how far). If no scrolling occurred, we stop the periodic events.
	*/
	if (!gettingPeriodic)
	  {
	    if ([self autoscroll: lastEvent])
	      {
		gettingPeriodic = YES;
		[NSEvent startPeriodicEventsAfterDelay: 0.1
					    withPeriod: 0.1];
		
	      }
	  }
	else if (gotPeriodic)
	  {
	    if (![self autoscroll: lastEvent])
	      {
		gettingPeriodic = NO;
		[NSEvent stopPeriodicEvents];
	      }
	  }


	point = [self convertPoint: [lastEvent locationInWindow]
		  fromView: nil];

	endIndex = [self _characterIndexForPoint: point
				 respectFraction: YES];
	
	/**
	 * If the mouse is not inside the receiver, see if it is over another
	 * text view with the same layout manager. If so, use that text view
	 * to compute endIndex
	 */
	if (![self mouse: point inRect: [self bounds]])
	  {
	    BOOL found = NO;
	    // FIXME: Is there an easier way to find the view under a point?
	    NSView *winContentView = [[self window] contentView];
	    NSView *view = [winContentView hitTest: 
					     [[winContentView superview] convertPoint: [lastEvent locationInWindow]
									     fromView: nil]];
	    for (; view != nil; view = [view superview])
	      {
		if ([view isKindOfClass: [NSTextView class]] && 
		    [(NSTextView*)view layoutManager] == [self layoutManager])
		  {
		    found = YES;
		    break;
		  }
	      }

	    if (found)
	      {
		NSTextView *textview = (NSTextView*)view;
		NSPoint mouse = [textview convertPoint: [lastEvent locationInWindow]
					      fromView: nil];

		endIndex = [textview _characterIndexForPoint: mouse
					     respectFraction: YES];
	      }
	  }

	proposedRange = MakeRangeFromAbs(endIndex,
					 startIndex);

	chosenRange = [self selectionRangeForProposedRange: proposedRange
			granularity: granularity];

	[self setSelectedRange: chosenRange  affinity: affinity
	  stillSelecting: YES];

	currentEvent = [_window nextEventMatchingMask: mask
			 untilDate: [NSDate distantFuture]
			 inMode: NSEventTrackingRunLoopMode
			 dequeue: YES];
      } while ([currentEvent type] != NSLeftMouseUp);

    if (gettingPeriodic)
      {
	[NSEvent stopPeriodicEvents];
      }
  }

  NSDebugLog(@"chosenRange. location  = %d, length  = %d\n",
	     (int)chosenRange.location, (int)chosenRange.length);

  [self setSelectedRange: chosenRange  affinity: affinity  
	stillSelecting: NO];

  /* Remember granularity till a new selection destroys the memory */
  [self setSelectionGranularity: granularity];
}

static const NSInteger GSSpellingSuggestionMenuItemTag = 1;

- (void) _acceptGuess: (id)sender
{
  NSString *guess = [(NSMenuItem*)sender title];
  NSRange range = [self selectedRange];

  if (![self shouldChangeTextInRange: range replacementString: guess])
    return;

  [_textStorage beginEditing];
  [self replaceCharactersInRange: range withString: guess];
  [_textStorage endEditing];
  [self didChangeText];
}

- (void) rightMouseDown: (NSEvent *)theEvent
{
  NSInteger i;
  NSMenu *menu = [[self class] defaultMenu];
  NSPoint point = [self convertPoint: [theEvent locationInWindow] fromView: nil];
  NSUInteger index = [self _characterIndexForPoint: point
				   respectFraction: YES]; 

  if (_tf.is_selectable == NO)
    return;

  if (!_layoutManager)
    return;
  
  for (i = [[menu itemArray] count] - 1; i >= 0; --i)
    {
      NSMenuItem *item = [menu itemAtIndex: i];
      if ([item tag] == GSSpellingSuggestionMenuItemTag)
	{
	  [menu removeItemAtIndex: i];
	}
    }

  if (!(index >= [self selectedRange].location &&
	index <= NSMaxRange([self selectedRange])))
    {
      NSRange wordRange = [self selectionRangeForProposedRange: NSMakeRange(index, 0)
						   granularity: NSSelectByWord];

      [self setSelectedRange: wordRange
		    affinity: [self selectionAffinity]
	      stillSelecting: NO];
    }

  if (NSEqualRanges([self selectedRange],
		    [self selectionRangeForProposedRange: NSMakeRange([self selectedRange].location, 0)
					     granularity: NSSelectByWord])
      && [self selectedRange].length > 0
      && [self isEditable])
    {
      // Possibly show spelling suggestions
      
      NSSpellChecker *sp = [NSSpellChecker sharedSpellChecker];
      NSString *word = [[self string] substringWithRange: [self selectedRange]];
      if (sp != nil && [sp checkSpellingOfString: word startingAt: 0].location != NSNotFound)
	{
	  /*NSArray *guesses = [sp guessesForWordRange: wordRange
	    inString: [self string]
	    language: [sp language]
	    inSpellDocumentWithTag: [self spellCheckerDocumentTag]];*/
	  NSArray *guesses = [sp guessesForWord: word];
      
	  if ([guesses count] > 0)
	    {
	      for (i=0; i < [guesses count] && i < 5; i++)
		{
		  NSString *guess = [guesses objectAtIndex: i];

		  [menu insertItemWithTitle: guess action:@selector(_acceptGuess:) keyEquivalent:@"" atIndex:i];
		  [[menu itemAtIndex: i] setTarget: self];
		  [[menu itemAtIndex: i] setTag: GSSpellingSuggestionMenuItemTag];
		}
	      [menu insertItem: [NSMenuItem separatorItem] atIndex: i];
	      [[menu itemAtIndex: i] setTag: GSSpellingSuggestionMenuItemTag];
	    }
	  else
	    {
	      [menu insertItemWithTitle: _(@"No Suggestions") action:(SEL)0 keyEquivalent:@"" atIndex:0];
	      [[menu itemAtIndex: 0] setTag: GSSpellingSuggestionMenuItemTag];
	      [[menu itemAtIndex: 0] setEnabled: NO];
	      [menu insertItem: [NSMenuItem separatorItem] atIndex: 1];
	      [[menu itemAtIndex: 1] setTag: GSSpellingSuggestionMenuItemTag];
	    }
	}
    }

  [NSMenu popUpContextMenu: menu 
		 withEvent: theEvent
		   forView: self]; 
}

- (void) keyDown: (NSEvent *)theEvent
{
  // If not editable, don't recognize the key down
  if (_tf.is_editable == NO)
    {
      [super keyDown: theEvent];
    }
  else
    {
      [self interpretKeyEvents: [NSArray arrayWithObject: theEvent]];
    }
}

/* Bind other mouse up to pasteSelection. This should be done via
configuation! */
- (void) otherMouseUp: (NSEvent *)theEvent
{
  // TODO: Should we change the insertion point, based on the event position?
  [self pasteSelection];
}

- (void) startSpeaking:(id) sender
{
  // FIXME
}

- (void) stopSpeaking:(id) sender
{
  // FIXME
}

- (BOOL) acceptsGlyphInfo
{
  return _tf.accepts_glyph_info;
}

- (void) setAcceptsGlyphInfo: (BOOL)flag
{
  _tf.accepts_glyph_info = flag;
}


- (BOOL) allowsDocumentBackgroundColorChange
{
  return _tf.allows_document_background_color_change;
}

- (void) setAllowsDocumentBackgroundColorChange: (BOOL)flag
{
  _tf.allows_document_background_color_change = flag;
}

- (void) changeDocumentBackgroundColor: (id)sender
{
  if ([self allowsDocumentBackgroundColorChange])
    {
      NSColor *aColor = (NSColor *)[sender color];

      [self setBackgroundColor: aColor];
    }
}

- (void) changeAttributes: (id)sender
{
  NSDictionary *attributes;
  NSRange aRange = [self rangeForUserCharacterAttributeChange];
  NSRange foundRange;
  NSUInteger location;
  NSUInteger maxSelRange = NSMaxRange(aRange);

  if (aRange.location == NSNotFound)
    return;

  if (![self shouldChangeTextInRange: aRange
		   replacementString: nil])
    return;

  [_textStorage beginEditing];
  for (location = aRange.location;
       location < maxSelRange;
       location = NSMaxRange(foundRange))
    {
      attributes = [_textStorage attributesAtIndex: location 
                                 effectiveRange: &foundRange];
      if (attributes != nil)
        {
          attributes = [sender convertAttributes: attributes];
          [_textStorage setAttributes: attributes range: foundRange];
        }
    }
  [_textStorage endEditing];
  [self didChangeText];

  [self setTypingAttributes: [sender convertAttributes: 
                                         [self typingAttributes]]];
}

- (void) breakUndoCoalescing
{
  DESTROY(_undoObject);
}

- (void) complete: (id)sender
{
  NSRange range = [self rangeForUserCompletion];

  if ([self isEditable] &&
      range.location != NSNotFound && range.length != 0)
    {
      GSAutocompleteWindow *window = [GSAutocompleteWindow defaultWindow];
      [window displayForTextView: self];
    }
}

- (NSArray *) completionsForPartialWordRange: (NSRange)range
                         indexOfSelectedItem: (NSInteger *)index
{
  if ([_delegate respondsToSelector:
      @selector(textView:completions:forPartialWordRange:indexOfSelectedItem:)])
    {
      return [_delegate textView: self
		     completions: [[GSAutocompleteWindow defaultWindow] words]
     	     forPartialWordRange: range
	     indexOfSelectedItem: index];
    }

  return nil;
}

- (void) insertCompletion: (NSString *)word
      forPartialWordRange: (NSRange)range
                 movement: (NSInteger)movement
                  isFinal: (BOOL)flag
{
  NSString *complete;
  NSString *partial = [word substringToIndex: range.length];

  if (![self shouldChangeTextInRange: range replacementString: partial])
    return;

  complete = [word stringByDeletingPrefix: partial];

  [_textStorage beginEditing];
  [self replaceCharactersInRange: range withString: partial];
  [_textStorage endEditing];
  [self didChangeText];

  [self insertText: complete];
  [self didChangeText];

  if (!flag && ([self selectedRange].length == 0) )
    {
      [self setSelectedRange:
	      NSMakeRange(NSMaxRange(range), [complete length])];
    }
}

- (void) orderFrontLinkPanel: (id)sender
{
  // FIXME
}

- (void) orderFrontListPanel: (id)sender
{
  // FIXME
}

- (void) orderFrontTablePanel: (id)sender
{
  // FIXME
}

- (void) performFindPanelAction: (id)sender
{
    [[GSTextFinder sharedTextFinder]
      performFindPanelAction: sender
		withTextView: self];
}

// NSTextFinder methods implementation...
// isSelectable, isEditable, string, selectedRanges, setSelectedRanges, replaceCharactersInRange:withString:
// implemented by NSTextView already...

- (BOOL) allowsMultipleSelection
{
  return NO;
}

- (NSString *) stringAtIndex: (NSUInteger)characterIndex
              effectiveRange: (NSRangePointer)outRange
      endsWithSearchBoundary: (BOOL *)outFlag
{
  return [self string];
}

- (NSUInteger) stringLength
{
  return [[self string] length];
}

- (NSRange) firstSelectedRange
{
  NSValue *r = [[self selectedRanges] objectAtIndex: 0];
  return [r rangeValue];
}

- (BOOL) shouldReplaceCharactersInRanges: (NSArray *)ranges withStrings: (NSArray *)strings
{
  NSUInteger idx = 0;
  FOR_IN(NSValue*, rv, ranges)
    {
      NSRange r = [rv rangeValue];
      NSString *str = [strings objectAtIndex: idx];
      if (![self shouldChangeTextInRange: r replacementString: str])
        {
          return NO;
        }
      idx++;
    }
  END_FOR_IN(ranges);
  return YES;
}

- (void) didReplaceCharacters
{
  [self didChangeText];
}

- (NSView *) contentViewAtIndex: (NSUInteger)index effectiveCharacterRange: (NSRangePointer)outRange
{
  return nil;
}

- (NSArray *) rectsForCharacterRange: (NSRange)range
{
  NSUInteger rectCount = 0;
  NSRect *rects;
  NSMutableArray *result = [NSMutableArray array];
  NSUInteger idx = 0;

  rects = [_layoutManager rectArrayForCharacterRange: range
                        withinSelectedCharacterRange: NSMakeRange(NSNotFound, 0)
                                     inTextContainer: _textContainer
                                           rectCount: &rectCount];
  
  for (idx = 0; idx < rectCount; idx++)
    {
      NSRect r = rects[idx];
      NSValue *v = [NSValue valueWithRect: r];
      [result addObject: v];
    }
  
  return result;
}

- (NSArray *) visibleCharacterRanges
{
  NSArray *result = nil;
  
  if (_layoutManager)
    {
      const NSRect visibleRect = [self visibleRect];
      
      NSRange visibleGlyphRange = [_layoutManager glyphRangeForBoundingRect: visibleRect
                                                            inTextContainer: _textContainer];
      
      NSRange visibleRange = [_layoutManager characterRangeForGlyphRange: visibleGlyphRange
                                                        actualGlyphRange: NULL];

      NSValue *value = [NSValue valueWithRange: visibleRange];
      result = [NSArray arrayWithObject: value];
    }
 
  return result;
}

- (void) drawCharactersInRange: (NSRange)range forContentView: (NSView *)view
{
  /* Then draw the special background of the new glyphs.  */
  [_layoutManager drawBackgroundForGlyphRange: range
                  atPoint: _textContainerOrigin];

  [_layoutManager drawGlyphsForGlyphRange: range 
                  atPoint: _textContainerOrigin];
}

@end


@implementation NSTextView (GNUstepPrivate)

/*
TODO: make sure this is only called when _layoutManager is known non-nil,
or add guards
*/
- (NSUInteger) _characterIndexForPoint: (NSPoint)point
                       respectFraction: (BOOL)respectFraction
{
  NSUInteger index;
  CGFloat fraction;

  point.x -= _textContainerOrigin.x;
  point.y -= _textContainerOrigin.y;

  if ([_layoutManager extraLineFragmentTextContainer] == _textContainer)
    {
      NSRect extraRect = [_layoutManager extraLineFragmentRect];
      if (point.y >= NSMinY(extraRect))
	return [_textStorage length];
    }

  index = [_layoutManager glyphIndexForPoint: point 
			     inTextContainer: _textContainer
	      fractionOfDistanceThroughGlyph: &fraction];
  // FIXME The layoutManager should never return -1.
  // Assume that the text is empty if this happens.
  if (index == NSNotFound)
    return 0;

  index = [_layoutManager characterIndexForGlyphAtIndex: index];
  if (respectFraction && fraction > 0.5 && index < [_textStorage length] &&
      [[_textStorage string] characterAtIndex:index] != '\n')
    {
      index++;
    }
  return index;
}

- (void) _blink: (NSTimer *)t
{
  if (_drawInsertionPointNow)
    {
      _drawInsertionPointNow = NO;
    }
  else
    {
      _drawInsertionPointNow = YES;
    }
  
  [self setNeedsDisplayInRect: _insertionPointRect
	avoidAdditionalLayout: YES];
  /* Because we are called by a timer which is independent of any
     event processing in the gui runloop, we need to manually update
     the window.  */
  [self displayIfNeeded];
}

- (void) _stopInsertionTimer
{
  if (_insertionPointTimer != nil)
    {
      [_insertionPointTimer invalidate];
      DESTROY(_insertionPointTimer);
    }
}
 	  	 
- (void) _startInsertionTimer
{
  if (_insertionPointTimer != nil)
    {
      NSWarnMLog(@"Starting insertion timer with existing one running");
      [self _stopInsertionTimer];
    }
  _insertionPointTimer = [NSTimer scheduledTimerWithTimeInterval: 0.5
                                                          target: self
                                                        selector: @selector(_blink:)
                                                        userInfo: nil
                                                         repeats: YES];
  [[NSRunLoop currentRunLoop] addTimer: _insertionPointTimer
                               forMode: NSModalPanelRunLoopMode];
  RETAIN(_insertionPointTimer);
}

- (NSRect) rectForCharacterRange: (NSRange)aRange
{
  NSRange glyphRange;
  NSRect rect;

  if (!aRange.length || !_layoutManager)
    return NSZeroRect;
  glyphRange = [_layoutManager glyphRangeForCharacterRange: aRange 
			       actualCharacterRange: NULL];
  rect = [_layoutManager boundingRectForGlyphRange: glyphRange 
			 inTextContainer: _textContainer];
  rect.origin.x += _textContainerOrigin.x;
  rect.origin.y += _textContainerOrigin.y;
  return rect;
}

/** Extension method that copies the current selected text to the 
    special section pasteboard */
- (void) copySelection
{
#if 1
  NSString *newSelection;
  NSString *oldSelection;
  NSRange range = _layoutManager->_selected_range;

  if (range.location != NSNotFound && range.length != 0)
    {
        newSelection = [[self string] substringWithRange: range];
        oldSelection = [[NSPasteboard pasteboardWithName: @"Selection"] 
                           stringForType: NSStringPboardType];

        if (oldSelection != nil && 
            ![newSelection isEqualToString: oldSelection])
          {
            NSPasteboard *secondary;

            secondary = [NSPasteboard pasteboardWithName: @"Secondary"];
            [secondary declareTypes: [NSArray arrayWithObject: NSStringPboardType] 
                       owner: self];
            [secondary setString: oldSelection
                forType: NSStringPboardType];
          }
        else
          {
            return;
          }
    }
  else
    {
      return;
    }
#endif

  [self writeSelectionToPasteboard: 
            [NSPasteboard pasteboardWithName: @"Selection"]
        types: [NSArray arrayWithObject: NSStringPboardType]];
}

/** Extension method that pastes the current selected text from the 
    special section pasteboard */
- (void) pasteSelection
{
#if 1
  NSString *newSelection;
  NSString *oldSelection;
  NSRange range = _layoutManager->_selected_range;

  if (range.location != NSNotFound && range.length != 0)
    {
        newSelection = [[self string] substringWithRange: range];
        oldSelection = [[NSPasteboard pasteboardWithName: @"Selection"] 
                           stringForType: NSStringPboardType];

        if (oldSelection != nil && 
            [newSelection isEqualToString: oldSelection])
          {
            [self readSelectionFromPasteboard: 
                      [NSPasteboard pasteboardWithName: @"Secondary"] 
                  type: NSStringPboardType];
            return;
          }
    }
#endif

  [self readSelectionFromPasteboard: 
            [NSPasteboard pasteboardWithName: @"Selection"]
        type: NSStringPboardType];
}

/**
 * Text checking
 */

- (void) _checkTextInRange: (NSRange)aRange
{
  NSRange longestRange;
  id value = [_layoutManager temporaryAttribute: @"NSTextChecked" 
			       atCharacterIndex: aRange.location
			  longestEffectiveRange: &longestRange
					inRange: aRange];
  longestRange = NSIntersectionRange(longestRange, aRange);
  
  if ([value boolValue] && NSEqualRanges(longestRange, aRange))
    {
      //NSLog(@"No need to check in range %@", NSStringFromRange(aRange));
      return;
    }
  
  // Check all of aRange

  [_layoutManager removeTemporaryAttribute: NSSpellingStateAttributeName
			 forCharacterRange: aRange];
  
  {
    NSSpellChecker *sp = [NSSpellChecker sharedSpellChecker];
    NSInteger start = 0;
    int count;
    // FIXME: doing the spellcheck on this substring could create false-positives
    // if a word is split in half.. so the range should be expanded to the 
    // nearest word boundary
    NSString *substring = [[self string] substringWithRange: aRange];
    
    if (sp == nil)
      {
        return;
      }

    do {
      NSRange errorRange = [sp checkSpellingOfString: substring
					  startingAt: start
					    language: [sp language]
						wrap: YES
			      inSpellDocumentWithTag: [self spellCheckerDocumentTag]
					   wordCount: &count];
      
      if (errorRange.location < start)
	{
	  break;
	}
      
      if (errorRange.length > 0)
	{
	  start = NSMaxRange(errorRange);
	}
      else
	{
	  break;
	}
      
      errorRange = NSMakeRange(aRange.location + errorRange.location, errorRange.length);
      
      //NSLog(@"highlighting mistake: %@", [[self string] substringWithRange: errorRange]);
      
      [_layoutManager addTemporaryAttribute: NSSpellingStateAttributeName
				      value: [NSNumber numberWithInteger: NSSpellingStateSpellingFlag]
			  forCharacterRange: errorRange];
      
    } while (1);
    
    [_layoutManager addTemporaryAttribute: @"NSTextChecked"
				    value: [NSNumber numberWithBool: YES]
			forCharacterRange: aRange];
  }
}

- (void) _textCheckingTimerFired: (NSTimer *)t
{
  _textCheckingTimer = nil;

  if (nil == _layoutManager)
    return;

  {
    const NSRect visibleRect = [self visibleRect];
    
    NSRange visibleGlyphRange = [_layoutManager glyphRangeForBoundingRect: visibleRect
							  inTextContainer: _textContainer];
    
    NSRange visibleRange = [_layoutManager characterRangeForGlyphRange: visibleGlyphRange
						      actualGlyphRange: NULL];
    
    [self _checkTextInRange: visibleRange];
    
    _lastCheckedRect = visibleRect;
  }
}

- (void) _scheduleTextCheckingTimer
{
  [_textCheckingTimer invalidate];
  _textCheckingTimer = [NSTimer scheduledTimerWithTimeInterval: 0.5
							target: self
						      selector: @selector(_textCheckingTimerFired:) 
						      userInfo: [NSValue valueWithRect: [self visibleRect]]
						       repeats: NO];
  
}

- (void) _scheduleTextCheckingInVisibleRectIfNeeded
{
  if (_tf.continuous_spell_checking)
    {
      const NSRect visibleRect = [self visibleRect];
      if (!NSEqualRects(visibleRect, _lastCheckedRect))
	{
	   if (nil != _textCheckingTimer 
	       && NSEqualRects(visibleRect, [[_textCheckingTimer userInfo] rectValue]))
	     {
	       return;
	     }
	   [self _scheduleTextCheckingTimer];
	}
    }
}

/**
 * If selected has a nonzero length, return it unmodified.
 * If selected is touching a word, expand it to cover the entire word
 * Otherwise return selected unmodified
 */
- (NSRange) _rangeToInvalidateSpellingForSelectionRange: (NSRange)selected
{
  if (selected.length > 0)
    {
      return selected;
    }
  else
    {
      NSRange prevNonCharacter;
      NSRange nextNonCharacter;
      NSRange range;
      NSCharacterSet *boundary = [[NSCharacterSet letterCharacterSet] invertedSet];
  
      if (selected.location == [[self string] length] && selected.location > 0)
	{
	  selected.location--;
	}

      prevNonCharacter = [[self string] rangeOfCharacterFromSet: boundary options: NSBackwardsSearch range: NSMakeRange(0, selected.location)];
      nextNonCharacter = [[self string] rangeOfCharacterFromSet: boundary options: 0 range: NSMakeRange(NSMaxRange(selected), [[self string] length] - NSMaxRange(selected))];
  
      if (prevNonCharacter.length == 0)
	{
	  range.location = 0;
	}
      else
	{
	  range.location = prevNonCharacter.location + 1;
	}

      if (nextNonCharacter.length == 0)
	{
	  range.length = [[self string] length] - range.location;
	}
      else
	{
	  range.length = nextNonCharacter.location - range.location;
	}

      return range;
    }
}

- (void) _textDidChange: (NSNotification*)notif
{
  if (_tf.continuous_spell_checking)
    {
      // FIXME: This uses the caret position to guess what change caused
      // the NSTextDidChangeNotification and mark that range of the text as requiring re-checking.
      //
      // It would be better to use accurate information to decide what range
      // need its spelling state invalidated.

      NSRange range = [self _rangeToInvalidateSpellingForSelectionRange: [self selectedRange]];
      if (range.length > 0)
	{
	  [_layoutManager removeTemporaryAttribute: @"NSTextChecked"
				 forCharacterRange: range];
	  [_layoutManager removeTemporaryAttribute: NSSpellingStateAttributeName
				 forCharacterRange: range];
	}

      [self _scheduleTextCheckingTimer];
    }
}

- (void) _becomeRulerClient
{
  NSScrollView *sv;
  NSRulerView *rv;

  if (_tf.uses_ruler && _tf.is_ruler_visible &&
      (sv = [self enclosingScrollView]) != nil && 
      (rv = [sv horizontalRulerView]) != nil)
    {
      [rv setClientView: self];
    }
}

- (void) _resignRulerClient
{
  NSScrollView *sv;
  NSRulerView *rv;

  if (_tf.uses_ruler && _tf.is_ruler_visible &&
      (sv = [self enclosingScrollView]) != nil && 
      (rv = [sv horizontalRulerView]) != nil)
    {
      [rv setClientView: nil];
    }
}

@end

@implementation NSTextViewUndoObject

- (id) initWithRange: (NSRange)aRange
    attributedString: (NSAttributedString *)aString
{
  if ((self = [super init]) != nil)
    {
      range = aRange;
      string = RETAIN(aString);
    }
  return self;
}

- (void) dealloc
{
  RELEASE(string);
  [super dealloc];
}

- (NSRange) range
{
  return range;
}

- (void) setRange: (NSRange)aRange
{
  range = aRange;
}

- (void) performUndo: (NSTextStorage *)aTextStorage
{
  NSTextView *tv = [self bestTextViewForTextStorage: aTextStorage];

  if ([tv shouldChangeTextInRange: range
		replacementString: (string ? (NSString*)[string string]
					   : (NSString*)@"")])
    {
      if ([string length] > 0)
	{
	  [aTextStorage replaceCharactersInRange: range
			    withAttributedString: string];
	}
      else
	{
	  [aTextStorage replaceCharactersInRange: range withString: @""];
	}
      range.length = [string length];
      if ([[tv undoManager] isUndoing])
	[tv setSelectedRange: range];
      else
	[tv setSelectedRange: NSMakeRange(NSMaxRange(range), 0)];
      [tv didChangeText];
    }
}

- (NSTextView *) bestTextViewForTextStorage: (NSTextStorage *)aTextStorage
{
  NSUInteger i, j;
  NSArray *layoutManagers, *textContainers;
  NSTextView *tv, *first = nil, *best = nil;
  NSWindow *win;

  /* The "best" view will be one in the key window followed by one in the
   * main window.  In either case, we prefer the window's first responder.
   * If no view is in the key or main window, we simply use the first text
   * view.  Since -shouldChangeTextInRange:replacementString: returns NO
   * by default if a NSTextView is not editable, we consider only editable
   * views here.
   */
  layoutManagers = [aTextStorage layoutManagers];
  for (i = 0; i < [layoutManagers count]; i++)
    {
      textContainers = [[layoutManagers objectAtIndex: i] textContainers];
      for (j = 0; j < [textContainers count]; j++)
        {
          tv = [[textContainers objectAtIndex: j] textView];
          if ([tv isEditable])
            {
              win = [tv window];
              if (first == nil)
                first = tv;
              if ([win isKeyWindow])
                {
                  if ([win firstResponder] == tv)
                    return tv;
                  else if (best == nil)
                    best = tv;
                }
              else if ([win isMainWindow])
                {
                  if ([win firstResponder] == tv)
                    {
                      if (best == nil || ![[best window] isKeyWindow])
                        best = tv;
                    }
                  else if (best == nil)
                    best = tv;
                }
            }
        }
    }
  return best != nil ? best : first;
}

@end

@implementation NSTextStorage(NSTextViewUndoSupport)

- (void) _undoTextChange: (NSTextViewUndoObject *)anObject
{
  [anObject performUndo: self];
}

@end

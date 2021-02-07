/*                                                    -*-objc-*-
   NSTextView.h

   Copyright (C) 1999, 2000, 2001, 2008 Free Software Foundation, Inc.

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: September 2000, January 2008
   Reformatted and cleaned up.

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

#ifndef _GNUstep_H_NSTextView
#define _GNUstep_H_NSTextView
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSText.h>
#import <AppKit/NSTextFinder.h>
#import <AppKit/NSInputManager.h>
#import <AppKit/NSDragging.h>
#import <AppKit/NSTextAttachment.h>
#import <AppKit/NSUserInterfaceValidation.h>

@class NSTimer;
@class NSTextContainer;
@class NSTextStorage;
@class NSLayoutManager;
@class NSRulerView,NSRulerMarker;


enum _NSSelectionGranularity {
  NSSelectByCharacter	= 0,
  NSSelectByWord	= 1,
  NSSelectByParagraph	= 2,
};
typedef NSUInteger NSSelectionGranularity;

/* TODO: this looks like a mosx extension, not a GNUstep extension. should
double-check */
enum _NSSelectionAffinity {
  NSSelectionAffinityUpstream	= 0,
  NSSelectionAffinityDownstream	= 1,
};
typedef NSUInteger NSSelectionAffinity;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
enum _NSFindPanelAction
{
  NSFindPanelActionShowFindPanel = 1,
  NSFindPanelActionNext,
  NSFindPanelActionPrevious,
  NSFindPanelActionReplaceAll,
  NSFindPanelActionReplace,
  NSFindPanelActionReplaceAndFind,
  NSFindPanelActionSetFindString,
  NSFindPanelActionReplaceAllInSelection
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
  ,
  NSFindPanelActionSelectAll,
  NSFindPanelActionSelectAllInSelection
#endif
};
typedef NSUInteger NSFindPanelAction;
#endif

/*
If several NSTextView:s are connected to one NSLayoutManager, some attributes
are shared between them. This is done in two ways: storing the attributes in
the NSLayoutManager, or storing a copy in each NSTextView and ensuring that
the any changes are replicated in all of them.

Persistant attributes (ie. attributes that are encoded and decoded) need to
be stored in the NSTextView. Non-persistant attributes don't, and should
therefore be stored in the NSLayoutManager to avoid problems.
*/

@interface NSTextView : NSText <NSTextInput, NSUserInterfaceValidations, NSTextFinderClient>
{
  /* These attributes are shared by all text views attached to a layout
  manager. Any changes must be replicated in all those text views. */
  id _delegate;
  struct GSTextViewFlagsType {
    unsigned is_field_editor:1;
    unsigned is_editable:1;
    unsigned is_selectable:1;
    unsigned is_rich_text:1;
    unsigned imports_graphics:1;

    unsigned uses_font_panel:1;

    unsigned uses_ruler:1;
    unsigned is_ruler_visible:1;

    /* I don't know for sure that these are supposed to be shared, but it
    would be very awkward if they weren't. */
    unsigned allows_undo:1;
    unsigned smart_insert_delete:1;
    unsigned continuous_spell_checking:1;
  /* End of shared attributes. */


    unsigned draws_background:1;

    unsigned is_horizontally_resizable:1;
    unsigned is_vertically_resizable:1;

    /* owns_text_network is YES if we have created the whole network
       of text classes (and thus we are responsible to release them
       when we are released).
       
       owns_text_network in NO if the text network was assembled by
       hand, and the text storage owns everything - thus we need to
       release nothing.

       See -initWithFrame: for more comments about this. */
    unsigned owns_text_network:1;


    /* multiple_textviews is YES if more than one NSTextView are
       sharing this layout manager.  In this case, we need to keep the
       views in sync. */
    unsigned multiple_textviews:1;

    /* These two really are shared, but they're cached, They must be updated
    whenever the delegate changes (including indirect changes). */
    /* YES if delegate responds to
       `shouldChangeTextInRange:replacementString:' */
    unsigned delegate_responds_to_should_change:1;
    /* YES if delegate responds to
       `textView:willChangeSelectionFromCharacterRange:toCharacterRange:' */
    unsigned delegate_responds_to_will_change_sel:1;
    /* YES if a DnD operation controls the insertion point in this text view */
    unsigned drag_target_hijacks_insertion_point:1;

    unsigned uses_find_panel:1;
    unsigned accepts_glyph_info:1;
    unsigned allows_document_background_color_change:1;
  } _tf;


  /* TODO: figure out whether these should be shared or not */
  NSColor *_insertionPointColor; /* shared? */
  
  NSDictionary *_selectedTextAttributes; /* shared? */
  NSDictionary *_markedTextAttributes; /* shared? */


  /* shared by all text views attached to one NSTextStorage */
  NSInteger _spellCheckerDocumentTag;

  NSColor *_backgroundColor;

  NSSize _minSize;
  NSSize _maxSize;
  
  /* The following is the object used when posting notifications.  
     It is usually `self' - but in the case of multiple textviews 
     it is the firstTextView returned by the layout manager - which 
     might or might not be `self'.  This must *not* be retained. */
  NSTextView *_notifObject;

  /* other members of the text network */
  NSTextContainer *_textContainer;
  NSLayoutManager *_layoutManager;
  NSTextStorage *_textStorage;

  /* container inset and origin */
  NSSize _textContainerInset;
  NSPoint _textContainerOrigin;

  /* TODO: move to NSLayoutManager? */
  /* Probably not necessary; the insertion point is always drawn in the
  text view that is the first responder, so there shouldn't be any
  problems with keeping track of it locally. Still need to think hard
  about handling of it, though. */
//#if 0
  /* Timer used to redraw the insertion point ... FIXME - think what 
     happens with multiple textviews */
  NSTimer *_insertionPointTimer;
  /* Blinking of the insertion point is controlled by the following
     ivar ...  it is YES during the little period in which the
     insertion point should be drawn on screen, and NO during the
     following little period in which the insertion point should not
     be drawn */
  BOOL _drawInsertionPointNow;
//#endif

  /* Stores the insertion point rect - updated by
     updateInsertionPointStateAndRestartTimer: - we must make sure we
     call the method every time that the insertion point rect might
     need to be recomputed <eg, relayout>. */
  NSRect _insertionPointRect;

  /*
  This is the character index a sequence of moves in one dimension (ie.
  up/down or left/right) started at. It is passed to the layout manager
  and is used when an NSTextView is deciding where to move. (Eg.
  NSLayoutManager tries to maintain the original horizontal position when
  moving up/down.)
  */
  NSUInteger _originalInsertionPointCharacterIndex;

  /*
   * GSInsertionPointMovementDirection
   * 0=no information (_originalInsertionPointCharacterIndex undefined)
   * 1=horizontal
   * 2=vertical
  */
  int _currentInsertionPointMovementDirection;

  /* Ivar to store the location where text is going to be inserted during
   * a DnD operation.
   */
  NSUInteger _dragTargetLocation;

  /* Ivar used to implement coalescing of undo operations */
  id _undoObject;

  /*
  TODO:
  Still need to figure out what "proper behavior" is when moving between two
  NSTextView:s, though.
  */

  NSParagraphStyle *_defaultParagraphStyle;
  NSDictionary *_linkTextAttributes;
  NSRange _markedRange;

  // Text checking (spelling/grammar)
  NSTimer *_textCheckingTimer;
  NSRect _lastCheckedRect;
}


/**
Returns the default typing attributes: black text, default paragraph
style (as returned by [NSParagraphStyle defaultParagraphStyle]), and default
user font and size (as returned by [NSFont userFontOfSize: 0.0]).

GNUstep extension.
*/
+(NSDictionary *) defaultTypingAttributes;


/**** Initializing ****/

/* This is sent each time a view is initialized.  If you subclass you
should ensure that you only register once. */
+(void) registerForServices;

/* Designated Initializer. container may be nil. */
-(id) initWithFrame: (NSRect)frameRect
      textContainer: (NSTextContainer *)container;

/* This variant will create the text network (NSTextStorage, NSLayoutManager,
and a NSTextContainer). The network will be owned by the NSTextView;
releasing it will release all parts of the network. */
-(id) initWithFrame: (NSRect)frameRect;


/**** Text network management ****/

/* The set method should not be called directly, but you might want to
override it. Gets or sets the text container for this view.
Setting the text container marks the view as needing display.  The
text container calls the set method from its setTextView: method. */
-(NSTextContainer *) textContainer;
-(void) setTextContainer: (NSTextContainer *)container;

/* This method should be used instead of the primitive
-setTextContainer: if you need to replace a view's text container
with a new one leaving the rest of the text network intact.  This method
deals with all the work of making sure the view doesn't get
deallocated and removing the old container from the layoutManager
and replacing it with the new one. */
-(void) replaceTextContainer: (NSTextContainer *)newContainer;

-(NSLayoutManager *) layoutManager;
-(NSTextStorage *) textStorage;



/*
These two methods are for modifying the text programmatically. They
don't ask the delegate are send any notifications, and they always work
(ie. even if the text view isn't editable).

(They are really from NSText, but redeclared here so they can be documented
here.)
*/

- (void) replaceCharactersInRange: (NSRange)aRange  
		       withString: (NSString*)aString;

/*
If the text view isn't rich-text, the attributes of aString will be ignored
and the typing attributed will be used.
*/
-(void) replaceCharactersInRange: (NSRange)aRange /* GNUstep extension. */
	    withAttributedString: (NSAttributedString *)aString;



/*** Additional Font menu commands ***/

/* These complete the set of range: type set methods. to be equivalent
to the set of non-range taking varieties. */
-(void) setAlignment: (NSTextAlignment)alignment  range: (NSRange)range;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (NSParagraphStyle *) defaultParagraphStyle;
- (void) setDefaultParagraphStyle: (NSParagraphStyle *)style;
#endif

-(void) setRulerVisible: (BOOL)flag;
-(BOOL) usesRuler;
-(void) setUsesRuler: (BOOL)flag;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (BOOL) usesFindPanel;
- (void) setUsesFindPanel: (BOOL)flag;
#endif
-(BOOL) isContinuousSpellCheckingEnabled; /* mosx */
-(void) setContinuousSpellCheckingEnabled: (BOOL)flag; /* mosx */
-(BOOL) allowsUndo; /* mosx */
-(void) setAllowsUndo: (BOOL)flag; /* mosx */
-(BOOL) smartInsertDeleteEnabled;
-(void) setSmartInsertDeleteEnabled: (BOOL)flag;


/* These methods are like paste: (from NSResponder) but they restrict
the acceptable type of the pasted data.  They are suitable as menu
actions for appropriate "Paste As" submenu commands. */
-(void) pasteAsPlainText: (id)sender;
-(void) pasteAsRichText: (id)sender;


/*** Dealing with user changes ***/

-(BOOL) shouldChangeTextInRange: (NSRange)affectedCharRange
              replacementString: (NSString *)replacementString;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL) shouldChangeTextInRanges: (NSArray *)ranges
               replacementStrings: (NSArray *)strings;
#endif
-(void) didChangeText;

-(NSRange) rangeForUserTextChange;
-(NSRange) rangeForUserCharacterAttributeChange;
-(NSRange) rangeForUserParagraphAttributeChange;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (NSRange) rangeForUserCompletion;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSArray *) rangesForUserCharacterAttributeChange;
- (NSArray *) rangesForUserParagraphAttributeChange;
- (NSArray *) rangesForUserTextChange;
#endif

/*** Text container stuff ***/


/* The text container inset determines the padding that the view provides
around the container. The text container is placed in a rectangle in the
text view's bounds rectangle, inset by inset.width on the left and right
edge and inset.height on the top and bottom edge.

Thus, setting this to (3,5) will give a 3 unit border on the left edge of
the text container, a 3 unit border on the right edge, a 5 unit border on
the top edge, and a 5 unit border on the bottom edge.
*/
-(void) setTextContainerInset: (NSSize)inset;
-(NSSize) textContainerInset;

/* The text container's origin is the origin of the text container's
coordinate system in the text view's coordinate system. It is determined
from the current usage of the container, the container inset, and the view
size. textContainerOrigin returns this point.
{TODO: why describe how the origin is determined? atm, it's even incorrect}

invalidateTextContainerOrigin is sent automatically whenever something
changes that might cause the origin to move. You usually do not need to call
it yourself. */
-(NSPoint) textContainerOrigin;
-(void) invalidateTextContainerOrigin;


/*** Sizing methods ***/

/* Sets the frame size of the view to desiredSize constrained within
(effective) minimum size and maximum size, and to the directions in which
the text view is resizable. */
-(void) setConstrainedFrameSize: (NSSize)desiredSize;



-(void) setSelectedTextAttributes: (NSDictionary *)attributeDictionary;
-(NSDictionary *) selectedTextAttributes;

-(void) setInsertionPointColor: (NSColor *)color;
-(NSColor *) insertionPointColor;


/*** Marked range ***/

-(void) setMarkedTextAttributes: (NSDictionary *)attributeDictionary;
-(NSDictionary *) markedTextAttributes;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (NSDictionary *) linkTextAttributes;
- (void) setLinkTextAttributes: (NSDictionary *)attributeDictionary;
#endif

#ifdef GNUSTEP
/*** Private, internal methods for (currently only) XIM support ***/

/*
The implementations for these are empty in NSTextView. Backends can override
these in a category.
*/

-(void) _updateInputMethodState;
-(void) _updateInputMethodWithInsertionPoint: (NSPoint)insertionPoint;

#endif

@end


/*
These methods are implemented in NSTextView_actions.m. See the comment in
that file for details on the split and which methods are for
user/programmatic changes of the text.
*/
@interface NSTextView (UserActions)

-(void) alignJustified: (id)sender; /* mosx */

-(void) turnOffKerning: (id)sender;
-(void) tightenKerning: (id)sender;
-(void) loosenKerning: (id)sender;
-(void) useStandardKerning: (id)sender;

-(void) turnOffLigatures: (id)sender;
-(void) useStandardLigatures: (id)sender;
-(void) useAllLigatures: (id)sender;

-(void) raiseBaseline: (id)sender;
-(void) lowerBaseline: (id)sender;

-(void) toggleTraditionalCharacterShape: (id)sender; /* mosx */

-(void) transpose: (id)sender; /* not OPENSTEP */

-(void) toggleContinuousSpellChecking: (id)sender; /* mosx */
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void) outline: (id)sender;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (void) setBaseWritingDirection: (NSWritingDirection)direction
                           range: (NSRange)range;
- (void) toggleBaseWritingDirection: (id)sender;
#endif

@end


@interface NSTextView (leftovers)

/*** Fine display control ***/

/* Like -setNeedsDisplayInRect: (NSView), bit if flag is YES, won't do any
layout. This means that it will only display the glyphs in rect that have
already been laid out. */
-(void) setNeedsDisplayInRect: (NSRect)rect
	avoidAdditionalLayout: (BOOL)flag;

-(BOOL) shouldDrawInsertionPoint;
-(void) drawInsertionPointInRect: (NSRect)rect
			   color: (NSColor *)color
			turnedOn: (BOOL)flag;


/*** Pasteboard management ***/

-(NSString *) preferredPasteboardTypeFromArray: (NSArray *)availableTypes
                    restrictedToTypesFromArray: (NSArray *)allowedTypes; /* mosx */
-(BOOL) readSelectionFromPasteboard: (NSPasteboard *)pboard; /* mosx */
-(BOOL) readSelectionFromPasteboard: (NSPasteboard *)pboard
			       type: (NSString *)type; /* mosx */
-(NSArray *) readablePasteboardTypes; /* mosx */
-(NSArray *) writablePasteboardTypes; /* mosx */
-(BOOL) writeSelectionToPasteboard: (NSPasteboard *)pboard
			      type: (NSString *)type; /* mosx */
-(BOOL) writeSelectionToPasteboard: (NSPasteboard *)pboard
			     types: (NSArray *)types; /* mosx */

/* TODO: check that this belongs under pasteboard management */
-(id) validRequestorForSendType: (NSString *)sendType
		     returnType: (NSString *)returnType; /* mosx */


/*** Drag and drop handling ***/

-(NSImage *) dragImageForSelectionWithEvent: (NSEvent *)event
				     origin: (NSPoint *)origin; /* mosx */
-(NSDragOperation) dragOperationForDraggingInfo: (id <NSDraggingInfo>)dragInfo
                                           type: (NSString *)type; /* mosx */
-(BOOL) dragSelectionWithEvent: (NSEvent *)event
			offset: (NSSize)mouseOffset
		     slideBack: (BOOL)slideBack; /* mosx */

-(void) cleanUpAfterDragOperation; /* mosx */


/*** Selected range ***/

-(NSRange) selectionRangeForProposedRange: (NSRange)proposedCharRange
                              granularity: (NSSelectionGranularity)gr;

-(void) setSelectedRange: (NSRange)charRange;

-(void) setSelectedRange: (NSRange)charRange
                affinity: (NSSelectionAffinity)affinity
          stillSelecting: (BOOL)stillSelectingFlag;
/* Called by drawing routines to determine where to draw insertion
   point */
-(NSSelectionAffinity) selectionAffinity;
-(NSSelectionGranularity) selectionGranularity;
-(void) setSelectionGranularity: (NSSelectionGranularity)granularity;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSArray *) selectedRanges;
- (void) setSelectedRanges: (NSArray *)ranges;
- (void) setSelectedRanges: (NSArray *)ranges
                  affinity: (NSSelectionAffinity)affinity
            stillSelecting: (BOOL)flag;
#endif

-(void) updateInsertionPointStateAndRestartTimer: (BOOL)restartFlag;



/*** Spell checking ***/

-(NSInteger) spellCheckerDocumentTag;


/*** Smart copy/paste/delete support ***/

- (NSRange) smartDeleteRangeForProposedRange: (NSRange)proposedCharRange;
- (void) smartInsertForString: (NSString *)aString
	       replacingRange: (NSRange)charRange
		 beforeString: (NSString **)beforeString
		  afterString: (NSString **)afterString;
- (NSString *) smartInsertAfterStringForString: (NSString *)aString
				replacingRange: (NSRange)charRange;
- (NSString *) smartInsertBeforeStringForString: (NSString *)aString
				 replacingRange: (NSRange)charRange;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (void) toggleSmartInsertDelete: (id)sender;
#endif


/** TODO: categorize */


-(NSDictionary *) typingAttributes;
-(void) setTypingAttributes: (NSDictionary *)attrs;

- (void) clickedOnLink: (id)link
               atIndex: (NSUInteger)charIndex;

-(void) updateRuler;
-(void) updateFontPanel;

-(NSArray *) acceptableDragTypes;
-(void) updateDragTypeRegistration;

- (void) startSpeaking:(id) sender;
- (void) stopSpeaking:(id) sender;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (BOOL) acceptsGlyphInfo;
- (void) setAcceptsGlyphInfo: (BOOL)flag;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (BOOL) allowsDocumentBackgroundColorChange;
- (void) setAllowsDocumentBackgroundColorChange: (BOOL)flag;
- (void) changeDocumentBackgroundColor: (id)sender;
- (void) drawViewBackgroundInRect: (NSRect)rect;

- (void) changeAttributes: (id)sender;

- (void) complete: (id)sender;
- (NSArray *) completionsForPartialWordRange: (NSRange)range
                         indexOfSelectedItem: (NSInteger *)index;
- (void) insertCompletion: (NSString *)word
      forPartialWordRange: (NSRange)range
                 movement: (NSInteger)movement
                  isFinal: (BOOL)flag;

- (void) performFindPanelAction: (id)sender;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (void) breakUndoCoalescing;
- (void) orderFrontLinkPanel: (id)sender;
- (void) orderFrontListPanel: (id)sender;
- (void) orderFrontTablePanel: (id)sender;
#endif

@end


@interface NSTextView (GSTextViewSync)
/*
 * This queries the NSLayoutManager to see if it is using multiple
 * text views, and saves this information in a flag, and caches the
 * first text view object.  The NSLayoutManager needs to call this
 * method to update this information. */
-(void) _updateMultipleTextViews;

/* For internal use. */
-(void) _syncTextViewsByCalling: (SEL)action  withFlag: (BOOL)flag;
-(void) _recacheDelegateResponses;

/*
NSLayoutManager will send this message to a text view when layout information
for that text view has been invalidated.
*/
-(void) _layoutManagerDidInvalidateLayout;
@end


/* Note that all delegation messages come from the first text view of a
layout manager. */

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
@protocol NSTextViewDelegate <NSObject>
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST) && GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSTextViewDelegate)
#endif

-(void) textView: (NSTextView *)textView
   clickedOnCell: (id <NSTextAttachmentCell>)cell
	  inRect: (NSRect)cellFrame;
-(void) textView: (NSTextView *)textView
   clickedOnCell: (id <NSTextAttachmentCell>)cell
	  inRect: (NSRect)cellFrame
	 atIndex: (NSUInteger)charIndex;

-(BOOL) textView: (NSTextView *)textView  clickedOnLink: (id)link;
-(BOOL) textView: (NSTextView *)textView
   clickedOnLink: (id)link
	 atIndex: (NSUInteger)charIndex;

-(void) textView: (NSTextView *)textView
doubleClickedOnCell: (id <NSTextAttachmentCell>)cell 
	  inRect: (NSRect)cellFrame;
-(void) textView: (NSTextView *)textView
doubleClickedOnCell: (id <NSTextAttachmentCell>)cell 
	  inRect: (NSRect)cellFrame
	 atIndex: (NSUInteger)charIndex;

-(void) textView: (NSTextView *)view
     draggedCell: (id <NSTextAttachmentCell>)cell
	  inRect: (NSRect)rect
           event:(NSEvent *)event;
-(void) textView: (NSTextView *)view
     draggedCell: (id <NSTextAttachmentCell>)cell
	  inRect: (NSRect)rect
	   event: (NSEvent *)event
	 atIndex: (NSUInteger)charIndex;

-(NSRange) textView: (NSTextView *)textView
willChangeSelectionFromCharacterRange: (NSRange)oldSelectedCharRange 
   toCharacterRange: (NSRange)newSelectedCharRange;

-(void) textViewDidChangeSelection: (NSNotification *)notification;

/* If characters are changing, replacementString is what will replace
the affectedCharRange.  If attributes only are changing,
replacementString will be nil. */
-(BOOL) textView: (NSTextView *)textView
  shouldChangeTextInRange: (NSRange)affectedCharRange
  replacementString: (NSString *)replacementString;

-(BOOL) textView: (NSTextView *)textView
  doCommandBySelector: (SEL)commandSelector;

-(NSUndoManager *) undoManagerForTextView: (NSTextView *)view;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (NSArray *) textView: (NSTextView *)textView
           completions: (NSArray *)words
   forPartialWordRange: (NSRange)range
   indexOfSelectedItem: (NSInteger *)index;
- (NSString *) textView: (NSTextView *)textView
	 willDisplayToolTip: (NSString *)tooltip
	forCharacterAtIndex: (NSUInteger)index;
- (void) textViewDidChangeTypingAttributes: (NSNotification *)notification;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL) textView: (NSTextView *)textView 
shouldChangeTextInRanges: (NSArray *)ranges 
replacementStrings: (NSArray *)strings; 
- (NSRange) textView: (NSTextView *)textView 
willChangeSelectionFromCharacterRanges: (NSArray *)oldSelectedCharRanges 
			toCharacterRanges: (NSArray *)newSelectedCharRanges;
- (BOOL) textView: (NSTextView *)textView 
shouldChangeTypingAttributes: (NSDictionary *)oldAttribs 
     toAttributes: (NSDictionary *)newAttribs;
#endif

@end
#endif	// GS_API_MACOSX

/* NSOldNotifyingTextView -> the old view, NSNewNotifyingTextView ->
the new view.  The text view delegate is not automatically
registered to receive this notification because the text machinery
will automatically switch over the delegate to observe the new
first text view as the first text view changes. */
APPKIT_EXPORT NSString *NSTextViewWillChangeNotifyingTextViewNotification;
APPKIT_EXPORT NSString *NSTextViewDidChangeSelectionNotification;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
APPKIT_EXPORT NSString *NSTextViewDidChangeTypingAttributesNotification;
#endif
APPKIT_EXPORT NSString *NSOldSelectedCharacterRange;

#endif /* _GNUstep_H_NSTextView */


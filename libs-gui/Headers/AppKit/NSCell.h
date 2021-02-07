/* -*-objc-*-
   NSCell.h

   The abstract cell class

   Copyright (C) 1996 Free Software Foundation, Inc.

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

#ifndef _GNUstep_H_NSCell
#define _GNUstep_H_NSCell
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSGeometry.h>

// For tint
#import <AppKit/NSColor.h>
// for NSWritingDirection
#import <AppKit/NSParagraphStyle.h>
// For text alignment
#import <AppKit/NSText.h>
// for NSFocusRingType
#import <AppKit/NSView.h>
#import <AppKit/NSUserInterfaceLayout.h>

@class NSString;
@class NSMutableDictionary;
@class NSView;
@class NSFont;
@class NSText;
@class NSFormatter;

enum _NSCellType {
  NSNullCellType,
  NSTextCellType,
  NSImageCellType
};
typedef NSUInteger NSCellType;

enum {
  NSAnyType,
  NSIntType,
  NSPositiveIntType,   
  NSFloatType,
  NSPositiveFloatType,   
  NSDoubleType,   
  NSPositiveDoubleType,
  NSDateType
};

enum {
  NSNoImage = 0,
  NSImageOnly,
  NSImageLeft,
  NSImageRight,
  NSImageBelow,
  NSImageAbove,
  NSImageOverlaps
};
typedef NSUInteger NSCellImagePosition;

enum _NSCellAttribute {
  NSCellDisabled,
  NSCellState,
  NSPushInCell,
  NSCellEditable,
  NSChangeGrayCell,
  NSCellHighlighted,   
  NSCellLightsByContents,  
  NSCellLightsByGray,   
  NSChangeBackgroundCell,  
  NSCellLightsByBackground,  
  NSCellIsBordered,  
  NSCellHasOverlappingImage,  
  NSCellHasImageHorizontal,  
  NSCellHasImageOnLeftOrBottom, 
  NSCellChangesContents,  
  NSCellIsInsetButton,
  NSCellAllowsMixedState
};
typedef NSUInteger NSCellAttribute;

enum {
  NSNoCellMask			= 0,
  NSContentsCellMask		= 1,
  NSPushInCellMask		= 2,
  NSChangeGrayCellMask		= 4,
  NSChangeBackgroundCellMask	= 8
};

enum {
  GSCellTextImageXDist = 2,	// horizontal distance between the text and image rects.
  GSCellTextImageYDist = 2	// vertical distance between the text and image rects.
};

/* 
 * We try to do as in macosx. 
 */
enum { 
  NSMixedState			= -1,
  NSOffState			= 0,
  NSOnState			= 1
};
typedef NSUInteger NSCellStateValue;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_13, GS_API_LATEST)
/*
 * Control state values as of 10.13
 */ 
enum { 
  NSControlStateValueMixed = -1,
  NSControlStateValueOff   =  0,
  NSControlStateValueOn    =  1
};
typedef NSUInteger NSControlStateValue;
#endif

/**
 *  <p>Enumeration of the ways that you can display an image in an
 *  NSImageCell.  The available ones are:</p>
 *  <p><code>NSScaleNone</code>: The image is always displayed with
 *  its natural size.  If it's bigger than the cell size, it is
 *  cropped.</p>
 *  <p><code>NSScaleProportionally</code>: If the image is bigger
 *  than the cell size, it is displayed in its natural size.  If it
 *  is smaller than the cell size, it is resized down proportionally
 *  to fit the cell size.</p>
 *  <p><code>NSScaleToFit</code>: The image is always resized (up
 *  or down) to fit exactly in the cell size.</p>
 */
enum {
  NSScaleProportionally = 0,
  NSScaleToFit = 1,
  NSScaleNone = 2,
  NSImageScaleProportionallyDown = 0,
  NSImageScaleAxesIndependently = 1,
  NSImageScaleNone = 2,
  NSImageScaleProportionallyUpOrDown = 3
};
typedef NSUInteger NSImageScaling;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
enum {
  NSCellHitNone = 0,
  NSCellHitContentArea = 1,
  NSCellHitEditableTextArea = 2,
  NSCellHitTrackableArea = 4
};
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
enum {
  NSBackgroundStyleLight = 0,
  NSBackgroundStyleDark = 1,
  NSBackgroundStyleRaised = 2,
  NSBackgroundStyleLowered = 3
};
typedef NSInteger NSBackgroundStyle;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_12, GS_API_LATEST)
enum __NSControlSize {
  NSControlSizeRegular,
  NSControlSizeSmall,
  NSControlSizeMini
};
#endif

@interface NSCell : NSObject <NSCopying, NSCoding>
{
  // Attributes
  id _contents;
  NSImage *_cell_image;
  NSFont *_font;
  id _object_value;
  struct GSCellFlagsType { 
    // total 32 bits.  0 bits left.
    unsigned contents_is_attributed_string: 1;
    unsigned is_highlighted: 1;
    unsigned is_disabled: 1;
    unsigned is_editable: 1;
    unsigned is_rich_text: 1;
    unsigned imports_graphics: 1;
    unsigned shows_first_responder: 1; 
    unsigned refuses_first_responder: 1; 
    unsigned sends_action_on_end_editing: 1; 
    unsigned is_bordered: 1;   
    unsigned is_bezeled: 1;   
    unsigned is_scrollable: 1;
    unsigned reserved: 1;
    unsigned text_align: 3; // 5 values
    unsigned is_selectable: 1;
    unsigned allows_mixed_state: 1;
    unsigned has_valid_object_value: 1;
    unsigned type: 2;           // 3 values
    unsigned image_position: 3; // 7 values
    unsigned entry_type: 4;     // 8 values
    unsigned allows_undo: 1;
    unsigned line_break_mode: 3; // 6 values

    // 23 bits for NSCell use, 4 bits for subclass use.
    // 5 bits remain unused.
    int state: 2; // 3 values but one negative
    unsigned mnemonic_location: 8;
    unsigned control_tint: 3;
    unsigned control_size: 2;
    unsigned focus_ring_type: 2; // 3 values
    unsigned base_writing_direction: 2; // 3 values
    // 4 bits reserved for subclass use
    unsigned subclass_bool_one: 1;
    unsigned subclass_bool_two: 1;
    unsigned subclass_bool_three: 1;
    unsigned subclass_bool_four: 1;
    // Set while the cell is edited/selected
    unsigned in_editing: 1;
    // Set if cell uses single line mode.
    unsigned uses_single_line_mode:1;
    unsigned background_style: 2; // 3 values
  } _cell;
  NSUInteger _mouse_down_flags;
  NSUInteger _action_mask;
  NSFormatter *_formatter;
  NSMenu *_menu;
  id _represented_object; 
  void *_reserved1;
}

//
// Class methods
// 
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
+ (NSFocusRingType)defaultFocusRingType;
#endif
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (NSMenu *)defaultMenu;
#endif
+ (BOOL)prefersTrackingUntilMouseUp;

//
// Initializing an NSCell 
//
- (id)initImageCell:(NSImage *)anImage;
- (id)initTextCell:(NSString *)aString;

//
// Setting the NSCell's Value 
//
- (id)objectValue;
- (BOOL)hasValidObjectValue;
- (double)doubleValue;
- (float)floatValue;
- (int)intValue;
- (NSString *)stringValue;
- (void) setObjectValue:(id)object;
- (void)setDoubleValue:(double)aDouble;
- (void)setFloatValue:(float)aFloat;
- (void)setIntValue:(int)anInt;
- (void)setStringValue:(NSString *)aString;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (NSInteger) integerValue;
- (void) setIntegerValue: (NSInteger)anInt;
- (void) takeIntegerValueFrom: (id)sender;
#endif
//
// Setting Parameters 
//
- (NSInteger)cellAttribute:(NSCellAttribute)aParameter;
- (void)setCellAttribute:(NSCellAttribute)aParameter
		      to:(NSInteger)value;

//
// Setting the NSCell's Type 
//
- (void)setType:(NSCellType)aType;
- (NSCellType)type;

//
// Enabling and Disabling the NSCell 
//
- (BOOL)isEnabled;
- (void)setEnabled:(BOOL)flag;

//
// Modifying Graphic Attributes 
//
- (BOOL)isBezeled;
- (BOOL)isBordered;
- (BOOL)isOpaque;
- (void)setBezeled:(BOOL)flag;
- (void)setBordered:(BOOL)flag;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (NSFocusRingType)focusRingType;
- (void)setFocusRingType:(NSFocusRingType)type;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)
#if GS_HAS_DECLARED_PROPERTIES
@property NSUserInterfaceLayoutDirection userInterfaceLayoutDirection;
#else
- (NSUserInterfaceLayoutDirection) userInterfaceLayoutDirection;
- (void) setUserInterfaceLayoutDirection: (NSUserInterfaceLayoutDirection)dir;
#endif
#endif

//
// Setting the NSCell's State 
//
- (void)setState:(NSInteger)value;
- (NSInteger)state;
- (BOOL)allowsMixedState;
- (void)setAllowsMixedState:(BOOL)flag;
- (NSInteger)nextState;
- (void)setNextState;

//
// Modifying Text Attributes 
//
- (NSTextAlignment)alignment;
- (NSFont *)font;
- (BOOL)isEditable;
- (BOOL)isSelectable;
- (BOOL)isScrollable;
- (void)setAlignment:(NSTextAlignment)mode;
- (void)setEditable:(BOOL)flag;
- (void)setFont:(NSFont *)fontObject;
- (void)setSelectable:(BOOL)flag;
- (void)setScrollable:(BOOL)flag;
- (void)setWraps:(BOOL)flag;
- (BOOL)wraps;
- (NSText *)setUpFieldEditorAttributes:(NSText *)textObject;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)setAttributedStringValue:(NSAttributedString *)attribStr;
- (NSAttributedString *)attributedStringValue;
- (void)setAllowsEditingTextAttributes:(BOOL)flag;
- (BOOL)allowsEditingTextAttributes;
- (void)setImportsGraphics:(BOOL)flag;
- (BOOL)importsGraphics;
- (void)setTitle:(NSString *)aString;
- (NSString *)title;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSWritingDirection)baseWritingDirection;
- (void)setBaseWritingDirection:(NSWritingDirection)direction;
- (NSLineBreakMode)lineBreakMode;
- (void)setLineBreakMode:(NSLineBreakMode)mode;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (void) setUsesSingleLineMode: (BOOL)flag;
- (BOOL) usesSingleLineMode;
#endif

//
// Target and Action 
//
- (SEL)action;
- (BOOL)isContinuous;
- (NSInteger)sendActionOn:(NSInteger)mask;
- (void)setAction:(SEL)aSelector;
- (void)setContinuous:(BOOL)flag;
- (void)setTarget:(id)anObject;
- (id)target;

//
// Setting the Image 
//
- (NSImage *)image;
- (void)setImage:(NSImage *)anImage;

//
// Assigning a Tag 
//
- (void)setTag:(NSInteger)anInt;
- (NSInteger)tag;

//
// Formatting Data and Validating Input 
//
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)setFormatter:(NSFormatter *)newFormatter;
- (id)formatter;
#endif
- (NSInteger)entryType;
- (BOOL)isEntryAcceptable:(NSString *)aString;
- (void)setEntryType:(NSInteger)aType;
- (void)setFloatingPointFormat:(BOOL)autoRange
                          left:(NSUInteger)leftDigits
                         right:(NSUInteger)rightDigits;

//
// Menu
//
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)setMenu:(NSMenu *)aMenu;
- (NSMenu *)menu;
- (NSMenu *)menuForEvent:(NSEvent *)anEvent 
                  inRect:(NSRect)cellFrame 
                  ofView:(NSView *)aView;
#endif

//
// Comparing to Another NSCell 
//
- (NSComparisonResult)compare:(id)otherCell;

//
// respond to keyboard
//
// All these methods except -performClick: are provided only for some
// compatibility with MacOS-X code and their use in new programs is
// deprecated.  Please use -isEnabled, -setEnabled: instead of
// -acceptsFirstReponder, -refusesFirstResponder,
// -setRefusesFirstResponder:.  Mnemonics (eg 'File' with the 'F'
// underlined as in MS Windows(tm) menus) are not part of GNUstep's
// interface so methods referring to mnemonics do nothing -- they are
// provided for compatibility only; please use key equivalents instead
// in your GNUstep programs.
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (BOOL)acceptsFirstResponder;
- (void)setShowsFirstResponder:(BOOL)flag;
- (BOOL)showsFirstResponder;
- (void)setTitleWithMnemonic:(NSString *)aString;
- (NSString *)mnemonic;
- (void)setMnemonicLocation:(NSUInteger)location;
- (NSUInteger)mnemonicLocation;
- (BOOL)refusesFirstResponder;
- (void)setRefusesFirstResponder:(BOOL)flag;

// deprecated method now in favor of performClickWithFrame:inView:
- (void)performClick:(id)sender; 

- (void)performClickWithFrame: (NSRect)cellFrame 
                       inView: (NSView *)controlView;
#endif

//
// Interacting with Other NSCells 
//
- (void)takeObjectValueFrom: (id)sender;
- (void)takeDoubleValueFrom:(id)sender;
- (void)takeFloatValueFrom:(id)sender;
- (void)takeIntValueFrom:(id)sender;
- (void)takeStringValueFrom:(id)sender;

//
// Using the NSCell to Represent an Object
//
- (id)representedObject;
- (void)setRepresentedObject:(id)anObject;

//
// Tracking the Mouse 
//
- (BOOL)continueTracking:(NSPoint)lastPoint
		      at:(NSPoint)currentPoint
		  inView:(NSView *)controlView;
- (NSInteger)mouseDownFlags;
- (void)getPeriodicDelay:(float *)delay
		interval:(float *)interval;
- (BOOL)startTrackingAt:(NSPoint)startPoint
		 inView:(NSView *)controlView;
- (void)stopTracking:(NSPoint)lastPoint
		  at:(NSPoint)stopPoint
	      inView:(NSView *)controlView
		  mouseIsUp:(BOOL)flag;
- (BOOL)trackMouse:(NSEvent *)theEvent
	    inRect:(NSRect)cellFrame
	    ofView:(NSView *)controlView
	    untilMouseUp:(BOOL)flag;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (NSBackgroundStyle)backgroundStyle;
- (void)setBackgroundStyle:(NSBackgroundStyle)backgroundStyle;
- (NSUInteger)hitTestForEvent:(NSEvent *)event
                       inRect:(NSRect)cellFrame
                       ofView:(NSView *)controlView;
#endif

//
// Managing the Cursor 
//
- (void)resetCursorRect:(NSRect)cellFrame
		 inView:(NSView *)controlView;

//
// Handling Keyboard Alternatives 
//
- (NSString *)keyEquivalent;

//
// Determining Component Sizes 
//
- (void)calcDrawInfo:(NSRect)aRect;
- (NSSize)cellSize;
- (NSSize)cellSizeForBounds:(NSRect)aRect;
- (NSRect)drawingRectForBounds:(NSRect)theRect;
- (NSRect)imageRectForBounds:(NSRect)theRect;
- (NSRect)titleRectForBounds:(NSRect)theRect;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)setControlSize:(NSControlSize)controlSize;
- (NSControlSize)controlSize;
#endif

//
// Displaying 
//
- (NSView *)controlView;
- (void)drawInteriorWithFrame:(NSRect)cellFrame
		       inView:(NSView *)controlView;
- (void)drawWithFrame:(NSRect)cellFrame
	       inView:(NSView *)controlView;
- (void)highlight:(BOOL)lit
	withFrame:(NSRect)cellFrame
	   inView:(NSView *)controlView;
- (BOOL)isHighlighted;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)setHighlighted: (BOOL) flag;
- (NSColor*)highlightColorWithFrame:(NSRect)cellFrame
			     inView:(NSView *)controlView;
- (void)setControlTint:(NSControlTint)controlTint;
- (NSControlTint)controlTint;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (void)setControlView:(NSView*)view;
#endif


//
// Editing Text 
//
- (void)editWithFrame:(NSRect)aRect
	       inView:(NSView *)controlView	
	       editor:(NSText *)textObject	
	       delegate:(id)anObject	
		event:(NSEvent *)theEvent;
- (void)selectWithFrame:(NSRect)aRect
		 inView:(NSView *)controlView	 
		 editor:(NSText *)textObject	 
		 delegate:(id)anObject	 
		  start:(NSInteger)selStart	 
		 length:(NSInteger)selLength;
- (void)endEditing:(NSText *)textObject;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (BOOL)sendsActionOnEndEditing;
- (void)setSendsActionOnEndEditing:(BOOL)flag;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL)allowsUndo;
- (void)setAllowsUndo:(BOOL)flag;
#endif

@end

// 
// Methods that are private GNUstep extensions
//
@interface NSCell (PrivateMethods)

- (NSDictionary*) _nonAutoreleasedTypingAttributes;
- (NSColor*) textColor;
- (NSSize) _sizeText: (NSString*) title;
- (void) _drawText: (NSString*)aString  inFrame: (NSRect)cellFrame;
- (void) _drawAttributedText: (NSAttributedString*)aString  
		     inFrame: (NSRect)aRect;
- (BOOL) _sendsActionOn:(NSUInteger)eventTypeMask;
- (NSAttributedString*) _drawAttributedString;
- (void) _drawBorderAndBackgroundWithFrame: (NSRect)cellFrame 
                                    inView: (NSView*)controlView;
- (void) _drawFocusRingWithFrame: (NSRect)cellFrame 
                          inView: (NSView*)controlView;
- (void) _drawEditorWithFrame: (NSRect)cellFrame
		       inView: (NSView*)controlView;
- (void) _setInEditing: (BOOL)flag;
- (void) _updateFieldEditor: (NSText*)textObject;
@end

#endif // _GNUstep_H_NSCell


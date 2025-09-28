/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/NSView.h>
#import <AppKit/NSSliderCell.h>

@class NSImage, NSColor;

typedef struct _Margins {
    float left;
    float right;
    float top;
    float bottom;
} Margins;

@interface NSGraphicsStyle : NSObject {
    NSView *_view;
}

@end

@interface NSGraphicsStyle (NSMenu)
- (NSSize)menuItemSeparatorSize;
- (NSSize)menuItemBranchArrowSize;
- (NSSize)menuItemCheckMarkSize;
- (NSSize)menuItemGutterSize;
- (NSSize)menuItemTextSize:(NSString *)title;
- (NSSize)menuItemAttributedTextSize:(NSAttributedString *)title;
- (float)menuBarHeight;
- (float)menuItemGutterGap;

- (Margins)menuItemBranchArrowMargins;
- (Margins)menuItemGutterMargins;
- (Margins)menuItemTextMargins;

- (void)drawMenuSeparatorInRect:(NSRect)rect;
- (void)drawMenuGutterInRect:(NSRect)rect;
- (void)drawMenuCheckmarkInRect:(NSRect)rect enabled:(BOOL)enabled selected:(BOOL)selected;
- (void)drawMenuItemText:(NSString *)string inRect:(NSRect)rect enabled:(BOOL)enabled selected:(BOOL)selected;
- (void)drawAttributedMenuItemText:(NSAttributedString *)string inRect:(NSRect)rect enabled:(BOOL)enabled selected:(BOOL)selected;
- (void)drawMenuBranchArrowInRect:(NSRect)rect enabled:(BOOL)enabled selected:(BOOL)selected;
- (void)drawMenuWindowBackgroundInRect:(NSRect)rect;
- (void)drawMenuBarBackgroundInRect:(NSRect)rect;
- (void)drawMenuSelectionInRect:(NSRect)rect enabled:(BOOL)enabled;
- (void)drawMenuBarItemBorderInRect:(NSRect)rect hover:(BOOL)hovering selected:(BOOL)selected;

@end

@interface NSGraphicsStyle (NSButton)
- (void)drawUnborderedButtonInRect:(NSRect)rect defaulted:(BOOL)defaulted;
- (void)drawPushButtonNormalInRect:(NSRect)rect defaulted:(BOOL)defaulted;
- (void)drawPushButtonPressedInRect:(NSRect)rect;
- (void)drawPushButtonHighlightedInRect:(NSRect)rect;
- (NSSize)sizeOfButtonImage:(NSImage *)image enabled:(BOOL)enabled mixed:(BOOL)mixed;
- (void)drawButtonImage:(NSImage *)image inRect:(NSRect)rect enabled:(BOOL)enabled mixed:(BOOL)mixed;
@end

@interface NSGraphicsStyle (NSBrowser)
- (void)drawBrowserTitleBackgroundInRect:(NSRect)rect;
- (void)drawBrowserHorizontalScrollerWellInRect:(NSRect)rect clipRect:(NSRect)clipRect;
@end

@interface NSGraphicsStyle (NSColorWell)
- (NSRect)drawColorWellBorderInRect:(NSRect)rect enabled:(BOOL)enabled bordered:(BOOL)bordered active:(BOOL)active;
@end

@interface NSGraphicsStyle (Buttons)
- (void)drawPopUpButtonWindowBackgroundInRect:(NSRect)rect;
@end

@interface NSGraphicsStyle (NSOutlineView)
- (void)drawOutlineViewGridInRect:(NSRect)rect;
@end

@interface NSGraphicsStyle (NSProgressIndicator)
- (NSRect)drawProgressIndicatorBackground:(NSRect)rect clipRect:(NSRect)clipRect bezeled:(BOOL)bezeled;
- (void)drawProgressIndicatorChunk:(NSRect)rect;
- (void)drawProgressIndicatorIndeterminate:(NSRect)rect clipRect:(NSRect)clipRect bezeled:(BOOL)bezeled animation:(double)animation;
- (void)drawProgressIndicatorDeterminate:(NSRect)rect clipRect:(NSRect)clipRect bezeled:(BOOL)bezeled value:(double)value;
@end

@interface NSGraphicsStyle (NSScroller)
- (void)drawScrollerButtonInRect:(NSRect)rect enabled:(BOOL)enabled pressed:(BOOL)pressed vertical:(BOOL)vertical upOrLeft:(BOOL)upOrLeft;
- (void)drawScrollerKnobInRect:(NSRect)rect vertical:(BOOL)vertical highlight:(BOOL)highlight;
- (void)drawScrollerTrackInRect:(NSRect)rect vertical:(BOOL)vertical upOrLeft:(BOOL)upOrLeft;
- (void)drawScrollerTrackInRect:(NSRect)rect vertical:(BOOL)vertical;
@end

@interface NSGraphicsStyle (NSSlider)
- (NSSize)sliderKnobSizeForControlSize:(NSControlSize)controlSize;
- (void)drawSliderKnobInRect:(NSRect)rect vertical:(BOOL)vertical highlighted:(BOOL)highlighted hasTickMarks:(BOOL)hasTickMarks tickMarkPosition:(NSTickMarkPosition)tickMarkPosition;
- (void)drawSliderTrackInRect:(NSRect)rect vertical:(BOOL)vertical hasTickMarks:(BOOL)hasTickMarks;
- (void)drawSliderTickInRect:(NSRect)rect;
@end

@interface NSGraphicsStyle (NSStepper)
- (void)drawStepperButtonInRect:(NSRect)rect clipRect:(NSRect)clipRect enabled:(BOOL)enabled highlighted:(BOOL)highlighted upNotDown:(BOOL)upNotDown;
@end

@interface NSGraphicsStyle (NSTableView)
- (void)drawTableViewHeaderInRect:(NSRect)rect highlighted:(BOOL)highlighted;
- (void)drawTableViewCornerInRect:(NSRect)rect;
@end

@interface NSGraphicsStyle (NSBox)
- (void)drawBoxWithLineInRect:(NSRect)rect;
- (void)drawBoxWithBezelInRect:(NSRect)rect clipRect:(NSRect)clipRect;
- (void)drawBoxWithGrooveInRect:(NSRect)rect clipRect:(NSRect)clipRect;
@end

@interface NSGraphicsStyle (NSComboBox)
- (void)drawComboBoxButtonInRect:(NSRect)rect enabled:(BOOL)enabled bordered:(BOOL)bordered pressed:(BOOL)pressed;
@end

@interface NSGraphicsStyle (NSTabView)
- (void)drawTabInRect:(NSRect)rect clipRect:(NSRect)clipRect color:(NSColor *)color selected:(BOOL)selected;
- (void)drawTabPaneInRect:(NSRect)rect;
- (void)drawTabViewBackgroundInRect:(NSRect)rect;
@end

@interface NSGraphicsStyle (NSTextField)
- (void)drawTextFieldBorderInRect:(NSRect)rect bezeledNotLine:(BOOL)bezeledNotLine;
- (void)drawTextViewInsertionPointInRect:(NSRect)rect color:(NSColor *)color;
@end

@interface NSView (NSGraphicsStyle)
- (NSGraphicsStyle *)graphicsStyle;
@end

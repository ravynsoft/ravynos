/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSImage, NSColor, NSEvent;

@interface NSCursor : NSObject <NSCoding> {
    NSImage *_image;
    NSPoint _hotSpot;
    BOOL _isSetOnMouseEntered;
    BOOL _isSetOnMouseExited;
    id _platformCursor;
}

+ (NSCursor *)currentCursor;
+ (NSCursor *)currentSystemCursor;

+ (NSCursor *)arrowCursor;
+ (NSCursor *)closedHandCursor;
+ (NSCursor *)contextualMenuCursor;
+ (NSCursor *)crosshairCursor;
+ (NSCursor *)disappearingItemCursor;
+ (NSCursor *)IBeamCursor;
+ (NSCursor *)openHandCursor;
+ (NSCursor *)pointingHandCursor;
+ (NSCursor *)resizeDownCursor;
+ (NSCursor *)resizeLeftCursor;
+ (NSCursor *)resizeLeftRightCursor;
+ (NSCursor *)resizeRightCursor;
+ (NSCursor *)resizeUpCursor;
+ (NSCursor *)resizeUpDownCursor;

+ (NSCursor *)dragCopyCursor;
+ (NSCursor *)dragLinkCursor;
+ (NSCursor *)operationNotAllowedCursor;

+ (void)hide;
+ (void)unhide;

+ (void)setHiddenUntilMouseMoves:(BOOL)flag;

- initWithImage:(NSImage *)image foregroundColorHint:(NSColor *)foregroundHint backgroundColorHint:(NSColor *)backgroundHint hotSpot:(NSPoint)hotSpot;
- initWithImage:(NSImage *)image hotSpot:(NSPoint)hotSpot;

- (NSImage *)image;
- (NSPoint)hotSpot;

- (BOOL)isSetOnMouseEntered;
- (BOOL)isSetOnMouseExited;

- (void)setOnMouseEntered:(BOOL)value;
- (void)setOnMouseExited:(BOOL)value;

- (void)mouseEntered:(NSEvent *)event;
- (void)mouseExited:(NSEvent *)event;

- (void)pop;

- (void)set;

- (void)push;
+ (void)pop;

@end

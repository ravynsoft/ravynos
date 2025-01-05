/*
 * Copyright (c) 2006-2007 Christopher J. W. Lloyd
 * Copyright (C) 2022-2025 Zoe Knox <zoe@ravynsoft.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#import <Foundation/Foundation.h>
#import <AppKit/NSColorSpace.h>
#import <AppKit/NSGraphics.h>

extern NSString * const NSScreenColorSpaceDidChangeNotification;

@interface NSScreen : NSObject

@property(class, strong, readonly) NSScreen *mainScreen;
@property(class, strong, readonly) NSScreen *deepestScreen;
@property(class, copy, readonly) NSArray *screens;
@property(class, readonly) BOOL screensHaveSeparateSpaces;

@property(readonly) NSWindowDepth depth;
@property(strong, readonly) NSColorSpace *colorSpace;
@property(readonly) NSRect frame;
@property(readonly) NSRect visibleFrame;
@property(readonly) const NSWindowDepth *supportedWindowDepths;
@property(copy, readonly) NSDictionary *deviceDescription;
@property(copy, readonly) NSString *localizedName;
@property(readonly) CGFloat backingScaleFactor;
@property(readonly) CGFloat maximumPotentialExtendedDynamicRangeColorComponentsValue;
@property(readonly) CGFloat maximumExtendedDynamicRangeColorComponentValue;
@property(readonly) CGFloat maximumReferenceExtendedDynamicRangeColorComponentValue;

-(NSScreen *)initWithFrame:(NSRect)frame visibleFrame:(NSRect)visibleFrame;
-(BOOL)canRepresentDisplayGamut:(NSDisplayGamut)displayGamut;
-(NSRect)backingAlignedRect:(NSRect)rect options:(NSAlignmentOptions)options;
-(NSRect)convertRectFromBacking:(NSRect)rect;
-(NSRect)convertRectToBacking:(NSRect)rect;

@end

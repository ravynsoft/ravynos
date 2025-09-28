/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSControl.h>
#import <AppKit/NSCell.h>

enum {
    NSSegmentStyleAutomatic = 0,
    NSSegmentStyleRounded = 1,
    NSSegmentStyleTexturedRounded = 2,
    NSSegmentStyleRoundRect = 3,
    NSSegmentStyleTexturedSquare = 4,
    NSSegmentStyleCapsule = 5,
    NSSegmentStyleSmallSquare = 6,
};
typedef NSInteger NSSegmentStyle;

@interface NSSegmentedControl : NSControl

- (NSInteger)segmentCount;
- (NSSegmentStyle)segmentStyle;

- (NSInteger)tagForSegment:(NSInteger)segment;
- (NSImage *)imageForSegment:(NSInteger)segment;
- (BOOL)isEnabledForSegment:(NSInteger)segment;
- (NSString *)labelForSegment:(NSInteger)segment;
- (NSMenu *)menuForSegment:(NSInteger)segment;
- (NSString *)toolTipForSegment:(NSInteger)segment;
- (CGFloat)widthForSegment:(NSInteger)segment;
- (NSImageScaling)imageScalingForSegment:(NSInteger)segment;

- (NSInteger)selectedSegment;
- (BOOL)isSelectedForSegment:(NSInteger)segment;

- (void)setSegmentCount:(NSInteger)count;
- (void)setSegmentStyle:(NSSegmentStyle)value;

- (void)setTag:(NSInteger)tag forSegment:(NSInteger)segment;
- (void)setImage:(NSImage *)image forSegment:(NSInteger)segment;
- (void)setEnabled:(BOOL)enabled forSegment:(NSInteger)segment;
- (void)setLabel:(NSString *)label forSegment:(NSInteger)segment;
- (void)setMenu:(NSMenu *)menu forSegment:(NSInteger)segment;
- (void)setToolTip:(NSString *)string forSegment:(NSInteger)segment;
- (void)setWidth:(CGFloat)width forSegment:(NSInteger)segment;
- (void)setImageScaling:(NSImageScaling)value forSegment:(NSInteger)segment;

- (BOOL)selectSegmentWithTag:(NSInteger)tag;
- (void)setSelected:(BOOL)flag forSegment:(NSInteger)segment;
- (void)setSelectedSegment:(NSInteger)segment;

@end

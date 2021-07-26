/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <AppKit/NSImageCell.h>

@class NSImage, NSMenu;

@interface NSSegmentItem : NSObject {
    int _tag;
    NSImage *_image;
    BOOL _isEnabled;
    BOOL _isSelected;
    NSString *_label;
    NSMenu *_menu;
    NSString *_toolTip;
    CGFloat _width;
    NSImageScaling _imageScaling;
}

- (int)tag;
- (NSImage *)image;
- (BOOL)isEnabled;
- (BOOL)isSelected;
- (NSString *)label;
- (NSMenu *)menu;
- (NSString *)toolTip;
- (CGFloat)width;
- (NSImageScaling)imageScaling;

- (void)setTag:(int)tag;
- (void)setImage:(NSImage *)image;
- (void)setEnabled:(BOOL)flag;
- (void)setSelected:(BOOL)flag;
- (void)setLabel:(NSString *)label;
- (void)setMenu:(NSMenu *)menu;
- (void)setToolTip:(NSString *)toolTip;
- (void)setWidth:(CGFloat)width;
- (void)setImageScaling:(NSImageScaling)value;

@end

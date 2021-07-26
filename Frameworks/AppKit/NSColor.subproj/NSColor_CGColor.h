/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSColor.h>
#import <ApplicationServices/ApplicationServices.h>

@interface NSColor_CGColor : NSColor {
    CGColorRef _colorRef;
    NSString *_colorSpaceName;
}

- initWithColorRef:(CGColorRef)colorRef spaceName:(NSString *)spaceName;

+ (NSColor *)colorWithColorRef:(CGColorRef)colorRef spaceName:(NSString *)spaceName;
+ (NSColor *)colorWithGray:(float)gray alpha:(float)alpha spaceName:(NSString *)spaceName;
+ (NSColor *)colorWithCyan:(float)cyan magenta:(float)magenta yellow:(float)yellow black:(float)black alpha:(float)alpha spaceName:(NSString *)spaceName;
+ (NSColor *)colorWithRed:(float)red green:(float)green blue:(float)blue alpha:(float)alpha spaceName:(NSString *)spaceName;
+ (NSColor *)colorWithHue:(float)hue saturation:(float)saturation brightness:(float)brightness alpha:(float)alpha spaceName:(NSString *)spaceName;

@end

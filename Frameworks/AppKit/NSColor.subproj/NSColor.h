/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <AppKit/AppKitExport.h>
#import <ApplicationServices/ApplicationServices.h>

@class NSImage;
@class NSPasteboard;

@interface NSColor : NSObject <NSCopying, NSCoding>

+ (NSColor *)highlightColor;
+ (NSColor *)shadowColor;
+ (NSColor *)gridColor;

+ (NSColor *)alternateSelectedControlColor;
+ (NSColor *)alternateSelectedControlTextColor;
+ (NSColor *)controlColor;
+ (NSColor *)secondarySelectedControlColor;
+ (NSColor *)selectedControlColor;
+ (NSColor *)controlTextColor;
+ (NSColor *)selectedControlTextColor;
+ (NSColor *)disabledControlTextColor;
+ (NSColor *)controlBackgroundColor;
+ (NSColor *)controlDarkShadowColor;
+ (NSColor *)controlHighlightColor;
+ (NSColor *)controlLightHighlightColor;
+ (NSColor *)controlShadowColor;
+ (NSArray *)controlAlternatingRowBackgroundColors;

+ (NSColor *)keyboardFocusIndicatorColor;

+ (NSColor *)textColor;
+ (NSColor *)textBackgroundColor;
+ (NSColor *)selectedTextColor;
+ (NSColor *)selectedTextBackgroundColor;

+ (NSColor *)headerColor;
+ (NSColor *)headerTextColor;

+ (NSColor *)scrollBarColor;
+ (NSColor *)knobColor;
+ (NSColor *)selectedKnobColor;

+ (NSColor *)windowBackgroundColor;
+ (NSColor *)windowFrameColor;

+ (NSColor *)selectedMenuItemColor;
+ (NSColor *)selectedMenuItemTextColor;

// private
+ (NSColor *)mainMenuBarColor;
+ (NSColor *)menuBackgroundColor;
+ (NSColor *)menuItemTextColor;

+ (NSColor *)clearColor;

+ (NSColor *)blackColor;
+ (NSColor *)blueColor;
+ (NSColor *)brownColor;
+ (NSColor *)cyanColor;
+ (NSColor *)darkGrayColor;
+ (NSColor *)grayColor;
+ (NSColor *)greenColor;
+ (NSColor *)lightGrayColor;
+ (NSColor *)magentaColor;
+ (NSColor *)orangeColor;
+ (NSColor *)purpleColor;
+ (NSColor *)redColor;
+ (NSColor *)whiteColor;
+ (NSColor *)yellowColor;

+ (NSColor *)colorWithDeviceWhite:(CGFloat)white alpha:(CGFloat)alpha;
+ (NSColor *)colorWithDeviceRed:(CGFloat)red green:(CGFloat)green blue:(CGFloat)blue alpha:(CGFloat)alpha;
+ (NSColor *)colorWithDeviceHue:(CGFloat)hue saturation:(CGFloat)saturation brightness:(CGFloat)brightness alpha:(CGFloat)alpha;
+ (NSColor *)colorWithDeviceCyan:(CGFloat)cyan magenta:(CGFloat)magenta yellow:(CGFloat)yellow black:(CGFloat)black alpha:(CGFloat)alpha;

+ (NSColor *)colorWithCalibratedWhite:(CGFloat)white alpha:(CGFloat)alpha;
+ (NSColor *)colorWithCalibratedRed:(CGFloat)red green:(CGFloat)green blue:(CGFloat)blue alpha:(CGFloat)alpha;
+ (NSColor *)colorWithCalibratedHue:(CGFloat)hue saturation:(CGFloat)saturation brightness:(CGFloat)brightness alpha:(CGFloat)alpha;

+ (NSColor *)colorWithCatalogName:(NSString *)catalogName colorName:(NSString *)colorName;

+ (NSColor *)colorFromPasteboard:(NSPasteboard *)pasteboard;

+ (NSColor *)colorWithPatternImage:(NSImage *)image;

- (NSString *)colorSpaceName;

- (NSInteger)numberOfComponents;
- (void)getComponents:(CGFloat *)components;

- (void)getWhite:(CGFloat *)white alpha:(CGFloat *)alpha;
- (void)getRed:(CGFloat *)red green:(CGFloat *)green blue:(CGFloat *)blue alpha:(CGFloat *)alpha;
- (void)getHue:(CGFloat *)hue saturation:(CGFloat *)saturation brightness:(CGFloat *)brightness alpha:(CGFloat *)alpha;
- (void)getCyan:(CGFloat *)cyan magenta:(CGFloat *)magenta yellow:(CGFloat *)yellow black:(CGFloat *)black alpha:(CGFloat *)alpha;

- (CGFloat)whiteComponent;

- (CGFloat)redComponent;
- (CGFloat)greenComponent;
- (CGFloat)blueComponent;

- (CGFloat)hueComponent;
- (CGFloat)saturationComponent;
- (CGFloat)brightnessComponent;

- (CGFloat)cyanComponent;
- (CGFloat)magentaComponent;
- (CGFloat)yellowComponent;
- (CGFloat)blackComponent;

- (CGFloat)alphaComponent;

- (NSColor *)colorWithAlphaComponent:(CGFloat)alpha;

- (NSColor *)colorUsingColorSpaceName:(NSString *)colorSpace;
- (NSColor *)colorUsingColorSpaceName:(NSString *)colorSpace device:(NSDictionary *)device;

- (NSColor *)blendedColorWithFraction:(CGFloat)fraction ofColor:(NSColor *)color;

- (void)set;
- (void)setStroke;
- (void)setFill;

- (void)drawSwatchInRect:(NSRect)rect;

- (void)writeToPasteboard:(NSPasteboard *)pasteboard;

@end

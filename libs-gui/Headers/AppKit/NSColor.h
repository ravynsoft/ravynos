/*
   NSColor.h

   The colorful color class

   Copyright (C) 1996, 1998 Free Software Foundation, Inc.

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

#ifndef _GNUstep_H_NSColor
#define _GNUstep_H_NSColor
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSCoder.h>
#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

@class NSString;
@class NSDictionary;
@class NSPasteboard;
@class NSImage;
@class NSColorSpace;

enum _NSControlTint {
    NSDefaultControlTint,
    NSBlueControlTint,
    NSGraphiteControlTint = 6,
    NSClearControlTint
};
typedef NSUInteger NSControlTint;

enum _NSControlSize {
    NSRegularControlSize,
    NSSmallControlSize,
    NSMiniControlSize
};
typedef NSUInteger NSControlSize;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_14, GS_API_LATEST)
enum _NSColorType {
    NSColorTypeComponentBased,
    NSColorTypePattern,
    NSColorTypeCatalog
};
typedef NSInteger NSColorType;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_14, GS_API_LATEST)
enum _NSColorSystemEffect {
    NSColorSystemEffectNone,
    NSColorSystemEffectPressed,
    NSColorSystemEffectDeepPressed,
    NSColorSystemEffectDisabled,
    NSColorSystemEffectRollover
};
typedef NSInteger NSColorSystemEffect;
#endif

/*
 * NSColor is an abstract super class of the class cluster of the real colour classes.
 * For each colour space exists a specific subclass that implements the behaviour for
 * this colour space.
 * The colour spaces NSDeviceBlackColorSpace and NSCalibratedBlackColorSpace
 * are no longer supported by this class. They were not in the old OpenStep
 * specification, and are not used in the new Apple specification. The names are
 * used as synonyms to NSDeviceWhiteColorSpace and NSCalibratedWhiteColorSpace.
 */

@interface NSColor : NSObject <NSCoding, NSCopying>
{
}

//
// Creating an NSColor from Component Values
//
+ (NSColor *)colorWithCalibratedHue:(CGFloat)hue
			 saturation:(CGFloat)saturation
			 brightness:(CGFloat)brightness
			      alpha:(CGFloat)alpha;
+ (NSColor *)colorWithCalibratedRed:(CGFloat)red
			      green:(CGFloat)green
			       blue:(CGFloat)blue
			      alpha:(CGFloat)alpha;
+ (NSColor *)colorWithCalibratedWhite:(CGFloat)white
				alpha:(CGFloat)alpha;
+ (NSColor *)colorWithCatalogName:(NSString *)listName
			colorName:(NSString *)colorName;
+ (NSColor *)colorWithDeviceCyan:(CGFloat)cyan
			 magenta:(CGFloat)magenta
			  yellow:(CGFloat)yellow
			   black:(CGFloat)black
			   alpha:(CGFloat)alpha;
+ (NSColor *)colorWithDeviceHue:(CGFloat)hue
		     saturation:(CGFloat)saturation
		     brightness:(CGFloat)brightness
			  alpha:(CGFloat)alpha;
+ (NSColor *)colorWithDeviceRed:(CGFloat)red
			  green:(CGFloat)green
			   blue:(CGFloat)blue
			  alpha:(CGFloat)alpha;
+ (NSColor *)colorWithDeviceWhite:(CGFloat)white
			    alpha:(CGFloat)alpha;

//
// Creating an NSColor With Preset Components
//
+ (NSColor *)blackColor;
+ (NSColor *)blueColor;
+ (NSColor *)brownColor;
+ (NSColor *)clearColor;
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

//
// Ignoring Alpha Components
//
+ (BOOL)ignoresAlpha;
+ (void)setIgnoresAlpha:(BOOL)flag;

//
// Retrieving a Set of Components
//
- (void)getCyan:(CGFloat *)cyan
	magenta:(CGFloat *)magenta
	 yellow:(CGFloat *)yellow
	  black:(CGFloat *)black
	  alpha:(CGFloat *)alpha;
- (void)getHue:(CGFloat *)hue
    saturation:(CGFloat *)saturation
    brightness:(CGFloat *)brightness
	 alpha:(CGFloat *)alpha;
- (void)getRed:(CGFloat *)red
	 green:(CGFloat *)green
	  blue:(CGFloat *)blue
	 alpha:(CGFloat *)alpha;
- (void)getWhite:(CGFloat *)white
	   alpha:(CGFloat *)alpha;

//
// Retrieving Individual Components
//
- (CGFloat)alphaComponent;
- (CGFloat)blackComponent;
- (CGFloat)blueComponent;
- (CGFloat)brightnessComponent;
- (NSString *)catalogNameComponent;
- (NSString *)colorNameComponent;
- (CGFloat)cyanComponent;
- (CGFloat)greenComponent;
- (CGFloat)hueComponent;
- (NSString *)localizedCatalogNameComponent;
- (NSString *)localizedColorNameComponent;
- (CGFloat)magentaComponent;
- (CGFloat)redComponent;
- (CGFloat)saturationComponent;
- (CGFloat)whiteComponent;
- (CGFloat)yellowComponent;

//
// Converting to Another Color Space
//
- (NSString *)colorSpaceName;
- (NSColor *)colorUsingColorSpaceName:(NSString *)colorSpace;
- (NSColor *)colorUsingColorSpaceName:(NSString *)colorSpace
			       device:(NSDictionary *)deviceDescription;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
// + (NSColor *)colorWithCIColor:(CIColor *)color;
+ (NSColor *)colorWithColorSpace:(NSColorSpace *)space
                      components:(const CGFloat *)comp
                           count:(NSInteger)number;
- (NSColorSpace *)colorSpace;
- (NSColor *)colorUsingColorSpace:(NSColorSpace *)space;
- (void)getComponents:(CGFloat *)components;
- (NSInteger)numberOfComponents;
#endif

//
// Changing the Color
//
- (NSColor *)blendedColorWithFraction:(CGFloat)fraction
			      ofColor:(NSColor *)aColor;
- (NSColor *)colorWithAlphaComponent:(CGFloat)alpha;

//
// Copying and Pasting
//
+ (NSColor *)colorFromPasteboard:(NSPasteboard *)pasteBoard;
- (void)writeToPasteboard:(NSPasteboard *)pasteBoard;

//
// Drawing
//
- (void)drawSwatchInRect:(NSRect)rect;
- (void)set;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void)setFill;
- (void)setStroke;
#endif

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
//
// Changing the color
//
- (NSColor*) highlightWithLevel: (CGFloat)level;
- (NSColor*) shadowWithLevel: (CGFloat)level;

+ (NSColor*)colorWithPatternImage:(NSImage*)image;
+ (NSColor*)colorForControlTint:(NSControlTint)controlTint;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
+ (NSControlTint)currentControlTint;
#endif

//
// System colors stuff.
//
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
+ (NSColor*) alternateSelectedControlColor;
+ (NSColor*) alternateSelectedControlTextColor;
#endif
+ (NSColor*) controlBackgroundColor;
+ (NSColor*) controlColor;
+ (NSColor*) controlHighlightColor;
+ (NSColor*) controlLightHighlightColor;
+ (NSColor*) controlShadowColor;
+ (NSColor*) controlDarkShadowColor;
+ (NSColor*) controlTextColor;
+ (NSColor*) disabledControlTextColor;
+ (NSColor*) gridColor;
+ (NSColor*) headerColor;
+ (NSColor*) headerTextColor;
+ (NSColor*) highlightColor;
+ (NSColor*) keyboardFocusIndicatorColor;
+ (NSColor*) knobColor;
+ (NSColor*) scrollBarColor;
+ (NSColor*) secondarySelectedControlColor;
+ (NSColor*) selectedControlColor;
+ (NSColor*) selectedControlTextColor;
+ (NSColor*) selectedKnobColor;
+ (NSColor*) selectedMenuItemColor;
+ (NSColor*) selectedMenuItemTextColor;
+ (NSColor*) selectedTextBackgroundColor;
+ (NSColor*) selectedTextColor;
+ (NSColor*) shadowColor;
+ (NSColor*) textBackgroundColor;
+ (NSColor*) textColor;
+ (NSColor*) windowBackgroundColor;
+ (NSColor*) windowFrameColor;
+ (NSColor*) windowFrameTextColor;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)
+ (NSColor*) labelColor;
+ (NSColor*) secondaryLabelColor;
+ (NSColor*) tertiaryLabelColor;
+ (NSColor*) quaternaryLabelColor;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
+ (NSArray*) controlAlternatingRowBackgroundColors;
#endif

// Pattern colour
- (NSImage*) patternImage;

// Tooltip colours
+ (NSColor*) toolTipColor;
+ (NSColor*) toolTipTextColor;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
+ (NSColor *)colorWithSRGBRed:(CGFloat)red
                        green:(CGFloat)green
                         blue:(CGFloat)blue
                        alpha:(CGFloat)alpha;
+ (NSColor *)colorWithGenericGamma22White:(CGFloat)white
                                    alpha:(CGFloat)alpha;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_9, GS_API_LATEST)
+ (NSColor *)colorWithRed:(CGFloat)red
                    green:(CGFloat)green
                     blue:(CGFloat)blue
                    alpha:(CGFloat)alpha;
+ (NSColor *)colorWithHue:(CGFloat)hue
               saturation:(CGFloat)saturation
               brightness:(CGFloat)brightness
                    alpha:(CGFloat)alpha;
+ (NSColor *)colorWithWhite:(CGFloat)white
                      alpha:(CGFloat)alpha;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)
+ (NSColor *)systemBlueColor;
+ (NSColor *)systemBrownColor;
+ (NSColor *)systemGrayColor;
+ (NSColor *)systemGreenColor;
+ (NSColor *)systemOrangeColor;
+ (NSColor *)systemPinkColor;
+ (NSColor *)systemPurpleColor;
+ (NSColor *)systemRedColor;
+ (NSColor *)systemYellowColor;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_12, GS_API_LATEST)
+ (NSColor *)colorWithDisplayP3Red:(CGFloat)red
                             green:(CGFloat)green
                              blue:(CGFloat)blue
                             alpha:(CGFloat)alpha;
+ (NSColor *)colorWithColorSpace:(NSColorSpace *)space
                             hue:(CGFloat)hue
                      saturation:(CGFloat)saturation
                      brightness:(CGFloat)brightness
                           alpha:(CGFloat)alpha;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_13, GS_API_LATEST)
- (NSColor *)colorUsingType:(NSColorType)type;
- (NSColorType)type;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_14, GS_API_LATEST)
- (NSColor *)colorWithSystemEffect:(NSColorSystemEffect)systemEffect;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_15, GS_API_LATEST)
+ (NSColor *)systemIndigoColor;
+ (NSColor *)systemTealColor;
#endif

#endif

@end

APPKIT_EXPORT NSString	*NSSystemColorsDidChangeNotification;

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
@interface NSCoder (NSCoderAdditions)

//
// Converting an Archived NXColor to an NSColor
//
- (NSColor *)decodeNXColor;

@end
#endif

typedef struct CGColor *CGColorRef;
@interface NSColor (GSQuartz)
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (CGColorRef)CGColor;
#endif
@end

#endif // _GNUstep_H_NSColor

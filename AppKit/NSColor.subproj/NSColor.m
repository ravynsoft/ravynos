/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSColor.h>
#import <AppKit/NSColor_catalog.h>
#import <AppKit/NSColor_CGColor.h>
#import <AppKit/NSRaise.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSBezierPath.h>

#import <AppKit/NSGraphics.h>
#import <AppKit/NSGraphicsContext.h>

#import <AppKit/NSPasteboard.h>
#import <Foundation/NSKeyedArchiver.h>

@interface NSColor(private)
-(NSString *)catalogName;
-(NSString *)colorName;
@end

@implementation NSColor

-(void)encodeWithCoder:(NSCoder *)coder {

   if([coder allowsKeyedCoding]){
    NSString *spaceName=[self colorSpaceName];

    if([spaceName isEqualToString:NSCalibratedRGBColorSpace]){
     CGFloat r,g,b,a;
     
     [self getRed:&r green:&g blue:&b alpha:&a];
     [coder encodeInt:1 forKey:@"NSColorSpace"];
     NSString *string=[NSString stringWithFormat:@"%f %f %f %f",r,g,b,a];
     NSData   *data=[string dataUsingEncoding:NSASCIIStringEncoding];
     [coder encodeBytes:[data bytes] length:[data length] forKey:@"NSRGB"];
    }
    else if([spaceName isEqualToString:NSDeviceRGBColorSpace]){
     CGFloat r,g,b,a;
     
     [self getRed:&r green:&g blue:&b alpha:&a];
     [coder encodeInt:2 forKey:@"NSColorSpace"];
     NSString *string=[NSString stringWithFormat:@"%f %f %f %f",r,g,b,a];
     NSData   *data=[string dataUsingEncoding:NSASCIIStringEncoding];
     [coder encodeBytes:[data bytes] length:[data length] forKey:@"NSRGB"];
    }
    else if([spaceName isEqualToString:NSCalibratedWhiteColorSpace]){
     CGFloat g,a;
     
     [self getWhite:&g alpha:&a];
     [coder encodeInt:3 forKey:@"NSColorSpace"];
     NSString *string=[NSString stringWithFormat:@"%f %f",g,a];
     NSData   *data=[string dataUsingEncoding:NSASCIIStringEncoding];
     [coder encodeBytes:[data bytes] length:[data length] forKey:@"NSWhite"];
    }
    else if([spaceName isEqualToString:NSDeviceWhiteColorSpace]){
     CGFloat g,a;
     
     [self getWhite:&g alpha:&a];
     [coder encodeInt:4 forKey:@"NSColorSpace"];
     NSString *string=[NSString stringWithFormat:@"%f %f",g,a];
     NSData   *data=[string dataUsingEncoding:NSASCIIStringEncoding];
     [coder encodeBytes:[data bytes] length:[data length] forKey:@"NSWhite"];
    }
    else if([spaceName isEqualToString:NSDeviceCMYKColorSpace]){
     CGFloat c,m,y,k,a;
     
     [self getCyan:&c magenta:&m yellow:&y black:&k alpha:&a];
     [coder encodeInt:5 forKey:@"NSColorSpace"];
     NSString *string=[NSString stringWithFormat:@"%f %f %f %f %f",c,m,y,k,a];
     NSData   *data=[string dataUsingEncoding:NSASCIIStringEncoding];
     [coder encodeBytes:[data bytes] length:[data length] forKey:@"NSCMYK"];
    }
    else if([spaceName isEqualToString:NSNamedColorSpace]){
     [coder encodeInt:6 forKey:@"NSColorSpace"];
     [coder encodeObject:[self catalogName] forKey:@"NSCatalogName"];
     [coder encodeObject:[self colorName] forKey:@"NSColorName"];
// FIXME: encode @"NSColor"
    }


   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"%@ can not encodeWithCoder:%@",isa,[coder class]];
   }
}

-(Class)classForCoder {
	return objc_lookUpClass("NSColor");
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    int                colorSpace=[keyed decodeIntForKey:@"NSColorSpace"];
    NSColor           *result;

    switch(colorSpace){
    
     case 1:{
// NSComponents data
// NSCustomColorSpace NSColorSpace
       NSUInteger    length;
       const uint8_t *rgb=[keyed decodeBytesForKey:@"NSRGB" returnedLength:&length];
       NSString   *string=[[[NSString alloc] initWithBytes:rgb length:length encoding:NSUTF8StringEncoding] autorelease];
       NSArray    *components=[string componentsSeparatedByString:@" "];
       float       values[4]={0,0,0,1};
       int         i,count=[components count];
       
       for(i=0;i<count && i<4;i++)
        values[i]=[[components objectAtIndex:i] floatValue];
       
       result=[NSColor colorWithCalibratedRed:values[0] green:values[1] blue:values[2] alpha:values[3]];
      }
      break;
      
     case 2:{
       NSUInteger    length;
       const uint8_t *rgb=[keyed decodeBytesForKey:@"NSRGB" returnedLength:&length];
       NSString   *string=[[[NSString alloc] initWithBytes:rgb length:length encoding:NSUTF8StringEncoding] autorelease];
       NSArray    *components=[string componentsSeparatedByString:@" "];
       float       values[4]={0,0,0,1};
       int         i,count=[components count];
       
       for(i=0;i<count && i<4;i++)
        values[i]=[[components objectAtIndex:i] floatValue];

       result=[NSColor colorWithDeviceRed:values[0] green:values[1] blue:values[2] alpha:values[3]];
      }
      break;
      
     case 3:{
       NSUInteger    length;
       const uint8_t *white=[keyed decodeBytesForKey:@"NSWhite" returnedLength:&length];
       NSString   *string=[[[NSString alloc] initWithBytes:white length:length encoding:NSUTF8StringEncoding] autorelease];
       NSArray    *components=[string componentsSeparatedByString:@" "];
       float       values[2]={0,1};
       int         i,count=[components count];
              
       for(i=0;i<count && i<2;i++)
        values[i]=[[components objectAtIndex:i] floatValue];

       result=[NSColor colorWithCalibratedWhite:values[0] alpha:values[1]];
      }
      break;
      
     case 4:{
       NSUInteger    length;
       const uint8_t *white=[keyed decodeBytesForKey:@"NSWhite" returnedLength:&length];
       NSString   *string=[[[NSString alloc] initWithBytes:white length:length encoding:NSUTF8StringEncoding] autorelease];
       NSArray    *components=[string componentsSeparatedByString:@" "];
       float       values[2]={0,1};
       int         i,count=[components count];
       
       for(i=0;i<count && i<2;i++)
        values[i]=[[components objectAtIndex:i] floatValue];

       result=[NSColor colorWithDeviceWhite:values[0] alpha:values[1]];
      }
      break;
      
     case 5:{
// NSComponents data
// NSCustomColorSpace NSColorSpace
       NSUInteger    length;
       const uint8_t *cmyk=[keyed decodeBytesForKey:@"NSCMYK" returnedLength:&length];
       NSString   *string=[[[NSString alloc] initWithBytes:cmyk length:length-1 encoding:NSUTF8StringEncoding] autorelease];
       NSArray    *components=[string componentsSeparatedByString:@" "];
       float       values[5]={0,0,0,0,1};
       int         i,count=[components count];
       
       for(i=0;i<count && i<5;i++)
        values[i]=[[components objectAtIndex:i] floatValue];

       result=[NSColor colorWithDeviceCyan:values[0] magenta:values[1] yellow:values[2] black:values[3] alpha:values[4]];
      }
      break;
      
     case 6:{
       NSString *catalogName=[keyed decodeObjectForKey:@"NSCatalogName"];
       NSString *colorName=[keyed decodeObjectForKey:@"NSColorName"];
       NSColor  *color=[keyed decodeObjectForKey:@"NSColor"];
       
       result=[NSColor_catalog colorWithCatalogName: catalogName colorName: colorName color:color];
      }
      break;

     default:
      NSLog(@"-[%@ %s] unknown color space %d",isa,sel_getName(_cmd),colorSpace);
      result=[NSColor blackColor];
      break;
    }
    
    return [result retain];
   }

   
   else {
    [NSException raise:NSInvalidArgumentException format:@"%@ can not initWithCoder:%@",isa,[coder class]];
    return nil;
   }
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

+(NSColor *)alternateSelectedControlColor {
    return [NSColor colorWithCatalogName:@"System" colorName:@"alternateSelectedControlColor"];
}

+(NSColor *)alternateSelectedControlTextColor {
    return [NSColor colorWithCatalogName:@"System" colorName:@"alternateSelectedControlTextColor"];
}

+ (NSColor *)keyboardFocusIndicatorColor {
    return [NSColor colorWithCatalogName:@"System" colorName:@"keyboardFocusIndicatorColor"];
}

+(NSColor *)highlightColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"highlightColor"];
}

+(NSColor *)shadowColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"shadowColor"];
}

+(NSColor *)gridColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"gridColor"];
}

+(NSColor *)controlColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"controlColor"];
}

+(NSColor *)selectedControlColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"selectedControlColor"];
}

+(NSColor *)secondarySelectedControlColor {
    return [NSColor colorWithCatalogName:@"System" colorName:@"secondarySelectedControlColor"];
}

+(NSColor *)controlTextColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"controlTextColor"];
}

+(NSColor *)selectedControlTextColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"selectedControlTextColor"];
}

+(NSColor *)disabledControlTextColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"disabledControlTextColor"];
}

+(NSColor *)controlBackgroundColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"controlBackgroundColor"];
}

+(NSColor *)controlDarkShadowColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"controlDarkShadowColor"];
}

+(NSColor *)controlHighlightColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"controlHighlightColor"];
}

+(NSColor *)controlLightHighlightColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"controlLightHighlightColor"];
}

+(NSColor *)controlShadowColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"controlShadowColor"];
}

// overwrite this, if you need a different set of alternating background colors
// NOTE: the list may contain more then two colors
+(NSArray *)controlAlternatingRowBackgroundColors {
   return [NSArray arrayWithObjects:
    [NSColor whiteColor],
    [NSColor colorWithCalibratedRed:0.90 green:0.95 blue:1.0 alpha:1.0],
    nil];
}

+(NSColor *)textColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"textColor"];
}

+(NSColor *)textBackgroundColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"textBackgroundColor"];
}

+(NSColor *)selectedTextColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"selectedTextColor"];
}

+(NSColor *)selectedTextBackgroundColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"selectedTextBackgroundColor"];
}

+(NSColor *)headerColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"headerColor"];
}

+(NSColor *)headerTextColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"headerTextColor"];
}

+(NSColor *)scrollBarColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"scrollBarColor"];
}

+(NSColor *)knobColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"knobColor"];
}

+(NSColor *)selectedKnobColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"selectedKnobColor"];
}

+(NSColor *)windowBackgroundColor {
   return [NSColor controlColor];
}

+(NSColor *)windowFrameColor {
    return [NSColor colorWithCatalogName:@"System" colorName:@"windowFrameColor"];
}

+ (NSColor *)selectedMenuItemColor {
    return [NSColor colorWithCatalogName:@"System" colorName:@"selectedMenuItemColor"];
}

+ (NSColor *)selectedMenuItemTextColor {
    return [NSColor colorWithCatalogName:@"System" colorName:@"selectedMenuItemTextColor"];
}

+(NSColor *)menuBackgroundColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"menuBackgroundColor"];
}

+(NSColor *)mainMenuBarColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"mainMenuBarColor"];
}

+(NSColor *)menuItemTextColor {
   return [NSColor colorWithCatalogName:@"System" colorName:@"menuItemTextColor"];
}

+(NSColor *)clearColor {
   return [NSColor colorWithCalibratedRed:0 green:0 blue:0 alpha:0];
}

+(NSColor *)blackColor {
   return [NSColor colorWithCalibratedWhite:0 alpha:1.0];
}

+(NSColor *)blueColor {
   return [NSColor colorWithCalibratedRed:0.0 green:0.0 blue:1.0 alpha:1.0];
}

+(NSColor *)brownColor {
   return [NSColor colorWithCalibratedRed:0.6 green:0.4 blue:0.2 alpha:1.0];
}

+(NSColor *)cyanColor {
   return [NSColor colorWithCalibratedRed:0.0 green:1.0 blue:1.0 alpha:1.0];
}

+(NSColor *)darkGrayColor {
   return [NSColor colorWithCalibratedWhite:1.0/3.0 alpha:1.0];
}

+(NSColor *)grayColor {
   return [NSColor colorWithCalibratedWhite:0.5 alpha:1.0];
}

+(NSColor *)greenColor {
   return [NSColor colorWithCalibratedRed:0.0 green:1.0 blue:0.0 alpha:1.0];
}

+(NSColor *)lightGrayColor {
   return [NSColor colorWithCalibratedWhite:2.0/3.0 alpha:1.0];
}

+(NSColor *)magentaColor {
   return [NSColor colorWithCalibratedRed:1.0 green:0.0 blue:1.0 alpha:1.0];
}

+(NSColor *)orangeColor {
   return [NSColor colorWithCalibratedRed:1.0 green:0.5 blue:0.0 alpha:1.0];
}

+(NSColor *)purpleColor {
   return [NSColor colorWithCalibratedRed:0.5 green:0.0 blue:0.5 alpha:1.0];
}

+(NSColor *)redColor {
   return [NSColor colorWithCalibratedRed:1.0 green:0.0 blue:0.0 alpha:1.0];
}

+(NSColor *)whiteColor {
   return [NSColor colorWithCalibratedWhite:1 alpha:1.0];
}

+(NSColor *)yellowColor {
   return [NSColor colorWithCalibratedRed:1.0 green:1.0 blue:0.0 alpha:1.0];
}

+(NSColor *)colorWithDeviceWhite:(CGFloat)white alpha:(CGFloat)alpha {
   return [NSColor_CGColor colorWithGray:white alpha:alpha spaceName:NSDeviceWhiteColorSpace];
}

+(NSColor *)colorWithDeviceRed:(CGFloat)red green:(CGFloat)green blue:(CGFloat)blue alpha:(CGFloat)alpha {
   return [NSColor_CGColor colorWithRed:red green:green blue:blue alpha:alpha spaceName:NSDeviceRGBColorSpace];
}

+(NSColor *)colorWithDeviceHue:(CGFloat)hue saturation:(CGFloat)saturation brightness:(CGFloat)brightness alpha:(CGFloat)alpha {
   return [NSColor_CGColor colorWithHue:hue saturation:saturation brightness:brightness alpha:alpha spaceName:NSDeviceRGBColorSpace];
}

+(NSColor *)colorWithDeviceCyan:(CGFloat)cyan magenta:(CGFloat)magenta yellow:(CGFloat)yellow black:(CGFloat)black alpha:(CGFloat)alpha {
   return [NSColor_CGColor colorWithCyan:cyan magenta:magenta yellow:yellow black:black alpha:alpha spaceName:NSDeviceCMYKColorSpace];
}

+(NSColor *)colorWithCalibratedWhite:(CGFloat)white alpha:(CGFloat)alpha {
   return [NSColor_CGColor colorWithGray:white alpha:alpha spaceName:NSCalibratedWhiteColorSpace];
}

+(NSColor *)colorWithCalibratedRed:(CGFloat)red green:(CGFloat)green blue:(CGFloat)blue alpha:(CGFloat)alpha {
   return [NSColor_CGColor colorWithRed:red green:green blue:blue alpha:alpha spaceName:NSCalibratedRGBColorSpace];
}

+(NSColor *)colorWithCalibratedHue:(CGFloat)hue saturation:(CGFloat)saturation brightness:(CGFloat)brightness alpha:(CGFloat)alpha {
   return [NSColor_CGColor colorWithHue:hue saturation:saturation brightness:brightness alpha:alpha spaceName:NSCalibratedRGBColorSpace];
}

+(NSColor *)colorWithCatalogName:(NSString *)catalogName colorName:(NSString *)colorName {
   return [NSColor_catalog colorWithCatalogName:catalogName colorName:colorName];
}

+(NSColor *)colorFromPasteboard:(NSPasteboard *)pasteboard {
   NSData *data=[pasteboard dataForType:NSColorPboardType];

   return [NSKeyedUnarchiver unarchiveObjectWithData:data];
}

static void drawPattern(void *info,CGContextRef cgContext){
   NSImage *image=(NSImage *)info;
   NSSize   size=[image size];
   NSGraphicsContext *context=[NSGraphicsContext graphicsContextWithGraphicsPort:cgContext flipped:NO];
   
   [NSGraphicsContext saveGraphicsState];
   [NSGraphicsContext setCurrentContext:context];
   [image drawInRect:NSMakeRect(0,0,size.width,size.height) fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
   [NSGraphicsContext restoreGraphicsState];
}

static void releasePatternInfo(void *info){
   [(NSImage *)info release];
}

+(NSColor *)colorWithPatternImage:(NSImage *)image {
   NSSize       size=[image size];
   CGPatternCallbacks callbacks={0,drawPattern,releasePatternInfo};
   [image retain];
   CGPatternRef    pattern=CGPatternCreate(image,CGRectMake(0,0,size.width,size.height),CGAffineTransformIdentity,size.width,size.height,kCGPatternTilingNoDistortion,YES,&callbacks);
   CGColorSpaceRef colorSpace=CGColorSpaceCreateDeviceRGB();
   CGFloat         components[4]={1,1,1,1};
   CGColorRef      cgColor=CGColorCreateWithPattern(colorSpace,pattern,components);
   
   NSColor *result=[[[NSColor_CGColor alloc] initWithColorRef:cgColor spaceName:NSPatternColorSpace] autorelease];
   
   CGColorRelease(cgColor);
   CGColorSpaceRelease(colorSpace);
   CGPatternRelease(pattern);
   
   return result;
}

-(NSString *)colorSpaceName {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSInteger)numberOfComponents {
   NSInvalidAbstractInvocation();
   return 0;
}

-(void)getComponents:(CGFloat *)components {
   NSInvalidAbstractInvocation();
}

-(void)getWhite:(float *)white alpha:(float *)alpha {
    NSInvalidAbstractInvocation();
}

-(void)getRed:(float *)red green:(float *)green blue:(float *)blue alpha:(float *)alpha {
   NSInvalidAbstractInvocation();
}

-(void)getHue:(float *)hue saturation:(float *)saturation brightness:(float *)brightness alpha:(float *)alpha {
   NSInvalidAbstractInvocation();
}

-(void)getCyan:(float *)cyan magenta:(float *)magenta yellow:(float *)yellow black:(float *)black alpha:(float *)alpha {
    NSInvalidAbstractInvocation();
}

-(CGFloat)whiteComponent {
    float white;

    [self getWhite:&white alpha:NULL];
    return white;
}

-(CGFloat)redComponent {
   float red;

   [self getRed:&red green:NULL blue:NULL alpha:NULL];

   return red;
}

-(CGFloat)greenComponent {
   float green;

   [self getRed:NULL green:&green blue:NULL alpha:NULL];

   return green;
}

-(CGFloat)blueComponent {
   float blue;

   [self getRed:NULL green:NULL blue:&blue alpha:NULL];

   return blue;
}

-(CGFloat)hueComponent {
   float hue;

   [self getHue:&hue saturation:NULL brightness:NULL alpha:NULL];

   return hue;
}

-(CGFloat)saturationComponent {
   float saturation;

   [self getHue:NULL saturation:&saturation brightness:NULL alpha:NULL];

   return saturation;
}

-(CGFloat)brightnessComponent {
   float brightness;

   [self getHue:NULL saturation:NULL brightness:&brightness alpha:NULL];

   return brightness;
}

-(CGFloat)cyanComponent {
    float cyan;

    [self getCyan:&cyan magenta:NULL yellow:NULL black:NULL alpha:NULL];
    return cyan;
}

-(CGFloat)magentaComponent {
    float magenta;

    [self getCyan:NULL magenta:&magenta yellow:NULL black:NULL alpha:NULL];
    return magenta;
}

-(CGFloat)yellowComponent {
    float yellow;

    [self getCyan:NULL magenta:NULL yellow:&yellow black:NULL alpha:NULL];
    return yellow;
}

-(CGFloat)blackComponent {
    float black;

    [self getCyan:NULL magenta:NULL yellow:NULL black:&black alpha:NULL];
    return black;
}

-(CGFloat)alphaComponent {
    return 1.0;
}

-(NSColor *)colorWithAlphaComponent:(CGFloat)alpha {
   if (alpha >= 1.0)
    return self; 
   return nil; 
}

-(NSColor *)colorUsingColorSpaceName:(NSString *)colorSpace {
    return [self colorUsingColorSpaceName:colorSpace device:nil];
}

-(NSColor *)colorUsingColorSpaceName:(NSString *)colorSpace device:(NSDictionary *)device {
   if([[self colorSpaceName] isEqualToString:colorSpace])
    return self;

   //NSLog(@"Warning, ignoring differences between color space %@ and %@", colorSpace, [self colorSpaceName]);
   return self;
}

-(NSColor *)blendedColorWithFraction:(CGFloat)fraction ofColor:(NSColor *)color {
   NSColor *primary=[color colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
   NSColor *secondary=[self colorUsingColorSpaceName:NSCalibratedRGBColorSpace];

   if(primary==nil || secondary==nil)
    return nil;
   else {
    float pr,pg,pb,pa;
    float sr,sg,sb,sa;
    float rr,rg,rb,ra;

    [primary getRed:&pr green:&pg blue:&pb alpha:&pa];
    [secondary getRed:&sr green:&sg blue:&sb alpha:&sa];

    rr=pr*fraction+sr*(1-fraction);
    rg=pg*fraction+sg*(1-fraction);
    rb=pb*fraction+sb*(1-fraction);
    ra=pa*fraction+sa*(1-fraction);

    return [NSColor colorWithCalibratedRed:rr green:rg blue:rb alpha:ra];
   }
}

-(void)set {
    [self setStroke];
    [self setFill];
}

-(void)setFill {
    NSInvalidAbstractInvocation();
}

-(void)setStroke {
    NSInvalidAbstractInvocation();
}

-(void)drawSwatchInRect:(NSRect)rect {
    // Draw some B&W triangle background so we can see the color alpha component
    [[NSColor whiteColor] setFill];
    NSRectFill(rect);
    [[NSColor blackColor] setFill];
    NSBezierPath *path = [NSBezierPath bezierPath];
    [path moveToPoint:NSMakePoint(NSMinX(rect), NSMaxY(rect))];
    [path lineToPoint:NSMakePoint(NSMaxX(rect), NSMaxY(rect))];
    [path lineToPoint:NSMakePoint(NSMinX(rect), NSMinY(rect))];
    [path closePath];
    [path fill];
    
    [self setFill];
    NSRectFillUsingOperation(rect, NSCompositeSourceOver);
}

-(void)writeToPasteboard:(NSPasteboard *)pasteboard {
   NSData *data=[NSKeyedArchiver archivedDataWithRootObject:self];

   [pasteboard setData:data forType:NSColorPboardType];
}

@end

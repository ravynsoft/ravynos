/* Copyright (c) 2008 Sijmen Mulder

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSGradient.h"
#import <AppKit/NSBezierPath.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSColorSpace.h>
#import <AppKit/NSGraphicsContext.h>
#import <AppKit/NSRaise.h>

@implementation NSGradient

static void evaluate(void *info,float const *input,float *output) {
   NSGradient *self=(NSGradient *)info;
   CGFloat   **components=self->_components;
   CGFloat    *locations=self->_locations;
   
    // Find where we are within the gradient location range - this will establish
    // which color pair we are blending between
    NSInteger colorIndex = 0;
    for(colorIndex = 0; colorIndex < self->_numberOfColors; colorIndex++) {
       if(locations[colorIndex]>=input[0]) {
           // We've found the right side of the color range
           break;
       }
    }
    
    NSInteger   startColorIndex = 0, endColorIndex = 0;
    // it could be right at the limit
    if (colorIndex >= self->_numberOfColors) {
        startColorIndex = endColorIndex = self->_numberOfColors - 1;
    } else {
        endColorIndex = colorIndex;
        // Start index must then be the preceding index
        startColorIndex = colorIndex - 1;
        if (startColorIndex < 0) {
            // Make sure we don't go out of range to the left
            startColorIndex = 0;
        }
    }
    
    // Now here's the tricky part, we need to find out the ratio between the two colors.
    // This means figuring out the distance between the two and then figuring out how far along we
    // are between them
    float start = locations[startColorIndex];
    float length = locations[endColorIndex] - locations[startColorIndex];
    float offset = input[0] - start;

    float ratio = 0;
    // Make sure we don't divide by 0!
    if (length > 0) {
        ratio = offset/length;
    }
    
    // now blend all the components using the ratio
    NSInteger componentIndex;
   for(componentIndex = 0; componentIndex < self->_numberOfComponents; componentIndex++){
    output[componentIndex] = (components[startColorIndex][componentIndex] +
                 (ratio * (components[endColorIndex][componentIndex] - components[startColorIndex][componentIndex])));
   }
}


-initWithStartingColor:(NSColor *)startingColor endingColor:(NSColor *)endingColor {
   NSArray *colors=[NSArray arrayWithObjects:startingColor,endingColor,nil];
   CGFloat  locations[2]={0.0,1.0};
   
   return [self initWithColors:colors atLocations:locations colorSpace:[NSColorSpace deviceRGBColorSpace]];
}

-initWithColors:(NSArray *)colors {
    NSAssert([colors count] > 1, @"A gradient needs at least 2 colors!");
   NSInteger count=[colors count];
   CGFloat   locations[count];
   NSInteger i;
   
   for (i = 0; i < count; i++)
    locations[i]=i/(float)(count-1);

   return [self initWithColors:colors atLocations:locations colorSpace:[NSColorSpace deviceRGBColorSpace]];
}

-initWithColorsAndLocations:(NSColor *)firstColor,... {
   NSMutableArray *colors=[NSMutableArray array];
   CGFloat         locations[256]; // FIXME: seems reasonable for now
   NSInteger       i;
   
   va_list arguments;
    
   va_start(arguments,firstColor);
   
   NSColor *color=firstColor;
   for(i=0;color!=nil && i<256;i++){    
    [colors addObject:color];
    locations[i]=va_arg(arguments,double);
    color=va_arg(arguments,NSColor *);
   }
   
   va_end(arguments);
   
   return [self initWithColors:colors atLocations:locations colorSpace:[NSColorSpace deviceRGBColorSpace]];
}

-initWithColors:(NSArray *)colors atLocations:(const CGFloat *)locations colorSpace:(NSColorSpace *)colorSpace {
   _colorSpace=[[NSColorSpace deviceRGBColorSpace] retain];
   _numberOfColors=[colors count];
   _numberOfComponents=4;
   _components=NSZoneMalloc(NULL,sizeof(CGFloat *)*_numberOfColors);
   _locations=NSZoneMalloc(NULL,sizeof(CGFloat)*_numberOfColors);
   
   NSInteger i;
   
   for(i=0;i<_numberOfColors;i++){
    NSColor *color=[colors objectAtIndex:i];
    
    color=[color colorUsingColorSpaceName:NSDeviceRGBColorSpace];
    _components[i]=NSZoneMalloc(NULL,sizeof(CGFloat)*_numberOfComponents);
    [color getComponents:_components[i]];
    
    _locations[i]=locations[i];
   }
   
   return self;
}

-(void)dealloc {
   NSInteger i;
   
   [_colorSpace release];
   
   for(i=0;i<_numberOfColors;i++)
    NSZoneFree(NULL,_components[i]);
    
   NSZoneFree(NULL,_components);
   NSZoneFree(NULL,_locations);
	
   [super dealloc];
}


-(void)drawFromPoint:(NSPoint)startingPoint toPoint:(NSPoint)endingPoint options:(NSGradientDrawingOptions)options {
   CGContextRef context=[[NSGraphicsContext currentContext] graphicsPort];
   CGFunctionCallbacks callbacks = { 0, evaluate, NULL }; 
   CGFunctionRef function = CGFunctionCreate(self, 1, NULL, _numberOfComponents, NULL, &callbacks);
   CGColorSpaceRef colorSpace = [_colorSpace CGColorSpace];
   CGShadingRef shading = CGShadingCreateAxial(colorSpace, startingPoint, endingPoint, function, NO, NO);
   
   CGContextDrawShading(context,shading);
   
   CGFunctionRelease(function);
   CGShadingRelease(shading);
}

- (void)drawFromCenter:(NSPoint)startCenter radius:(CGFloat)startRadius toCenter:(NSPoint)endCenter radius:(CGFloat)endRadius options:(NSGradientDrawingOptions)options {
	NSUnimplementedMethod();
}

- (void)drawInRect:(NSRect)rect angle:(CGFloat)angle
{
	if (_numberOfColors < 2 || 0 == rect.size.width)
		return;

	CGPoint start; //start coordinate of gradient
	CGPoint end; //end coordinate of gradient
	//tanSize is the rectangle size for atan2 operation. It is the size of the rect in relation to the offset start point
	CGPoint tanSize; 

	angle = (CGFloat)fmod(angle, 360);
	
	if (angle < 90)
	{
		start = CGPointMake(rect.origin.x, rect.origin.y);
		tanSize = CGPointMake(rect.size.width, rect.size.height);
	}
	else if (angle < 180)
	{
		start = CGPointMake(rect.origin.x + rect.size.width, rect.origin.y);
		tanSize = CGPointMake(-rect.size.width, rect.size.height);
	}
	else if (angle < 270)
	{
		start = CGPointMake(rect.origin.x + rect.size.width, rect.origin.y + rect.size.height);
		tanSize = CGPointMake(-rect.size.width, -rect.size.height);
	}
	else
	{
		start = CGPointMake(rect.origin.x, rect.origin.y + rect.size.height);
		tanSize = CGPointMake(rect.size.width, -rect.size.height);
	}
	
	
	CGFloat radAngle = angle / 180 * M_PI; //Angle in radians
	//The trig for this is difficult to describe without an illustration, so I'm attaching an illustration
	//to Cocotron issue number 438 along with this patch
	CGFloat distanceToEnd = cos(atan2(tanSize.y,tanSize.x) - radAngle) * sqrt(rect.size.width * rect.size.width + rect.size.height * rect.size.height);
	end = CGPointMake(cos(radAngle) * distanceToEnd + start.x, sin(radAngle) * distanceToEnd + start.y);
		
	CGContextRef context =[[NSGraphicsContext currentContext] graphicsPort];
	CGContextSaveGState(context);
	CGContextClipToRect(context, rect);
    [self drawFromPoint:start toPoint:end options:NSGradientDrawsBeforeStartingLocation|NSGradientDrawsAfterEndingLocation];
	CGContextRestoreGState(context);
	
}

-(void)drawInBezierPath:(NSBezierPath *)path angle:(CGFloat)angle {
   NSRect rect=[path bounds];
   [NSGraphicsContext saveGraphicsState];

   [path addClip];
   [self drawInRect:rect angle:angle];  

   [NSGraphicsContext restoreGraphicsState];
}

-(void)drawInRect:(NSRect)rect relativeCenterPosition:(NSPoint)center {
	NSUnimplementedMethod();
}

-(void)drawInBezierPath:(NSBezierPath *)path relativeCenterPosition:(NSPoint)center {
	NSUnimplementedMethod();
}

- (NSColorSpace *)colorSpace {
	return _colorSpace;
}

- (NSInteger)numberOfColorStops {
	return _numberOfColors;
}

-(void)getColor:(NSColor **)color location:(CGFloat *)location atIndex:(NSInteger)index {
   if (location)
    *location=_locations[index];
    
   if (color)
    *color=[NSColor colorWithCalibratedRed:_components[index][0] green:_components[index][1] blue:_components[index][2] alpha:_components[index][3]];
}

-(NSColor *)interpolatedColorAtLocation:(CGFloat)location {
   float input[1]={location};
   float output[4];
   
   evaluate(self,input,output);

   return [NSColor colorWithCalibratedRed:output[0] green:output[1] blue:output[2] alpha:output[3]];
}

@end


#import <AppKit/AppKit.h>

#import "NSColorPickerWheelView.h"


@implementation NSColorPickerWheelView

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    return self;
}

#pragma mark -
#pragma mark Internal Helpers

- (void)_updateHandleLocation
{
	NSRect bounds = [self bounds];
	NSPoint location = NSMakePoint(NSMidX(bounds), NSMidY(bounds));
	
	float colorWheelRadius = .5 * MIN(NSWidth(bounds), NSHeight(bounds));
	
	float angle = M_PI * _hueValue/ 180.f;
	
	location.x -= colorWheelRadius * sin(angle) * _saturationValue/100.f;
	location.y += colorWheelRadius * cos(angle) * _saturationValue/100.f;
	
	if (!NSEqualPoints(location, _handleLocation)) {
		_handleLocation = location;
		[self setNeedsDisplay: YES];
	}
}

#pragma mark -
#pragma mark Properties

- (BOOL)isOpaque
{
	return YES;
}

- (void)setHue:(CGFloat)hue
{
	if (hue != _hueValue) {
		_hueValue = hue;
		[self _updateHandleLocation];
	}
}

- (CGFloat)hue
{
	return _hueValue;
}

- (void)setSaturation:(CGFloat)saturation
{
	if (saturation != _saturationValue) {		
		_saturationValue = saturation;
		[self _updateHandleLocation];
	}
}

- (CGFloat)saturation
{
	return _saturationValue;
}

- (void)setBrightness:(CGFloat)brightness
{
	if (brightness != _brightnessValue) {
		_brightnessValue = brightness;
		[self setNeedsDisplay: YES];
	}
}

- (CGFloat)brightness
{
	return _brightnessValue;
}

- (void)drawRect:(NSRect)dirtyRect
{
	[[NSColor controlColor] set];
	NSRectFill(dirtyRect);
	
	NSRect bounds = [self bounds];
	NSImage* image = [NSImage imageNamed: @"NSColorWheel"];
	
	NSRect imageRect = bounds;
	if (NSWidth(bounds) > NSHeight(bounds)) {
		CGFloat delta = NSWidth(bounds)-NSHeight(bounds);
		imageRect = NSInsetRect(imageRect, delta/2.f, 0);
	} else {
		CGFloat delta = NSHeight(bounds)-NSWidth(bounds);
		imageRect = NSInsetRect(imageRect, 0, delta/2.f);
	}
	NSBezierPath* ovalPath = [NSBezierPath bezierPathWithOvalInRect: imageRect];
	
	[[NSColor blackColor] set];
	[ovalPath fill];

	[image drawInRect: imageRect fromRect: NSZeroRect operation: NSCompositeSourceOver fraction: _brightnessValue/100.f];

	NSRect handleRect = { _handleLocation, NSZeroSize};
	handleRect = NSInsetRect(handleRect, -2, -2);

	[[NSColor whiteColor] set];
	NSRectFill(handleRect);
	
	[[NSColor blackColor] set];	
	NSFrameRect(handleRect);
}


- (void)mouseDragged:(NSEvent*)event
{
	NSPoint location = [self convertPoint:[event locationInWindow] fromView:nil];
	
	NSRect bounds = [self bounds];
	float colorWheelRadius = .5 * MIN(NSWidth(bounds), NSHeight(bounds));
	
	NSPoint center = NSMakePoint(NSMidX(bounds), NSMidY(bounds));
	location.x -= center.x;
	location.y -= center.y;
	
	float relativeDistanceFromCenter = hypot(location.x, location.y)/colorWheelRadius;
	if (relativeDistanceFromCenter > 1)
		relativeDistanceFromCenter = 1;
	
	// Get the angle and ensure it's in 0..2*PI
	float angle = fmod(atan2(location.y, location.x) - M_PI_2 + M_PI * 2.f, M_PI * 2.f);
	
	// Hue is stored in degrees
	_hueValue = angle * 180.f / M_PI;
	
	_saturationValue = relativeDistanceFromCenter * 100.f;
	
	[self _updateHandleLocation];
	
	[_delegate colorPickerWheelView: self didSelectHue: _hueValue saturation: _saturationValue andBrightness: _brightnessValue];
}

- (IBAction)brightnessChanged:(id)sender
{
	[self setBrightness: [sender floatValue]];
	
	[_delegate colorPickerWheelView: self didSelectHue: _hueValue saturation: _saturationValue andBrightness: _brightnessValue];
}

-(void)resizeSubviewsWithOldSize:(NSSize)oldSize
{
	[self _updateHandleLocation];
	[super resizeSubviewsWithOldSize: oldSize];
}

@end

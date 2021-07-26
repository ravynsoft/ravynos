
#import <AppKit/AppKit.h>
#import "NSColorPickerWheel.h"

@implementation NSColorPickerWheel

-initWithPickerMask:(NSUInteger)mask colorPanel:(NSColorPanel *)colorPanel {
	
	if ((self = [super initWithPickerMask:mask colorPanel:colorPanel])) {
	}
    return self;
}

- (void)_syncToNewColor
{
	NSColor* color = [[self colorPanel] color];
	
	float hue, saturation, brightness, alpha;

	color = [color colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
	
	[color getHue: &hue saturation: &saturation brightness: &brightness alpha: &alpha];
	
	[_wheelView setHue: hue * 360];
	[_wheelView setSaturation: saturation * 100];
	[_wheelView setBrightness: brightness * 100];
	
	[valueSlider setFloatValue: brightness * 100];
}

- (void)awakeFromNib
{
	_subview = currentView;
}

- (NSImage *)provideNewButtonImage
{
    return [NSImage imageNamed:@"NSColorPickerWheelIcon"];
}

- (void)colorPickerWheelView:(NSColorPickerWheelView*)view didSelectHue:(CGFloat)hue saturation:(CGFloat)saturation andBrightness:(CGFloat)brightness
{

	[[self colorPanel] setColor:[NSColor colorWithCalibratedHue: hue/360.0
													 saturation: saturation/100.0
													 brightness: brightness/100.0
														  alpha:[[self colorPanel] alpha]]];
}

- (void)setColor:(NSColor *)color
{
	[self _syncToNewColor];
}

@end


#import <AppKit/NSView.h>

@interface NSColorPickerWheelView : NSView {

    IBOutlet id _delegate;

    NSPoint _handleLocation;
    CGFloat _hueValue; // Varies
    CGFloat _saturationValue;
    CGFloat _brightnessValue;
}

- (void)setHue:(CGFloat)hue; // 0-359
- (CGFloat)hue;

- (void)setSaturation:(CGFloat)saturation; // 0-99
- (CGFloat)saturation;

- (void)setBrightness:(CGFloat)brightness; // 0-99
- (CGFloat)brightness;

- (IBAction)brightnessChanged:(id)sender;

@end

@interface NSObject (NSColorPickerWheelViewDelegate)

- (void)colorPickerWheelView:(NSColorPickerWheelView *)view didSelectHue:(CGFloat)hue saturation:(CGFloat)saturation andBrightness:(CGFloat)brightness;

@end
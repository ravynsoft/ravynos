#import <QuartzCore/CIFilter.h>

@interface CIBloom : CIFilter {
    double _inputRadius;
    double _inputIntensity;
    BOOL _enabled;
}

@end

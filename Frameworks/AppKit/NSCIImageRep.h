#import <AppKit/NSImageRep.h>
#import <QuartzCore/CIImage.h>

@class NSBitmapImageRep;

@interface CIImage (CIImageRepAdditions)
- initWithBitmapImageRep:(NSBitmapImageRep *)bitmapImageRep;
@end

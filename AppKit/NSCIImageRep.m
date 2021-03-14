#import <AppKit/NSCIImageRep.h>
#import <AppKit/NSRaise.h>
#import <AppKit/NSBitmapImageRep.h>

@implementation CIImage(CIImageRepAdditions)

-initWithBitmapImageRep:(NSBitmapImageRep *)bitmapImageRep {
   return [self initWithCGImage:[bitmapImageRep CGImage]];
}

@end

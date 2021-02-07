#import "ObjectTesting.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSGraphics.h>

int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  NSBitmapImageRep *origBitmap, *copy1Bitmap, *copy2Bitmap;

  origBitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes: NULL
                                                       pixelsWide: 4
                                                       pixelsHigh: 4
                                                    bitsPerSample: 8
                                                  samplesPerPixel: 4
                                                         hasAlpha: YES
                                                         isPlanar: NO
                                                   colorSpaceName: NSCalibratedRGBColorSpace
                                                      bytesPerRow: 0
                                                     bitsPerPixel: 0];

  copy1Bitmap = [origBitmap copy];

  // Copying immutable NSData reuses the data pointer instead of allocating new memory, so 
  // copying a bitmap with immutable _imageData causes both bitmaps to point to the same memory;
  // Writing to either copy's pixels will overwrite both, corrupting the image data
  
  copy2Bitmap = [copy1Bitmap copy];
    
  pass([copy1Bitmap bitmapData] != [copy2Bitmap bitmapData],
       "Copied bitmaps have a different image data pointer - could cause image data corruption.");
  
  [arp release];
  return 0;
}

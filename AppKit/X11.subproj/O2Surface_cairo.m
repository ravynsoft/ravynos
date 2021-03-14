#import "O2Surface_cairo.h"

@implementation O2Surface_cairo

-initWithWidth:(size_t)width height:(size_t)height compatibleWithContext:(O2Context_cairo *)compatible {

   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceRGB();

   if([super initWithBytes:NULL width:width height:height bitsPerComponent:8 bytesPerRow:0 colorSpace:colorSpace bitmapInfo:kO2ImageAlphaPremultipliedFirst|kO2BitmapByteOrder32Little]==nil){
    O2ColorSpaceRelease(colorSpace);
    [self dealloc];
    return nil;
   }
   
   O2ColorSpaceRelease(colorSpace);
   
   if((_cairo_surface=cairo_image_surface_create_for_data(_pixelBytes,CAIRO_FORMAT_ARGB32,width,height,_bytesPerRow))==NULL){
    NSLog(@"%s %d cairo_image_surface_create_for_data failed",__FILE__,__LINE__);
    [self dealloc];
    return NULL;
   }

   return self;   
}

-(void)dealloc {
   cairo_surface_destroy(_cairo_surface);
   [super dealloc];
}

-(cairo_surface_t *)cairo_surface {
   return _cairo_surface;
}

@end

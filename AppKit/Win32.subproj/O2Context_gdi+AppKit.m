#import "O2Context_gdi+AppKit.h"
#import "Win32Window.h"
#import "Win32DeviceContextPrinter.h"
#import "Win32DeviceContextWindow.h"
#import <Onyx2D/O2GraphicsState.h>
#import <Onyx2D/O2PDFContext.h>
#import <Onyx2D/O2Surface_DIBSection.h>

@implementation O2Context_gdi(AppKit)

-initWithHWND:(HWND)handle {
   O2DeviceContext_gdi    *deviceContext=[[[Win32DeviceContextWindow alloc] initWithWindowHandle:handle] autorelease];
   NSSize                  size=[deviceContext pixelSize];
   O2GState        *gState=[[[O2GState alloc] initFlippedWithDeviceHeight:size.height] autorelease];

   return [self initWithGraphicsState:gState deviceContext:deviceContext];
}

-initWithPrinterDC:(HDC)printer auxiliaryInfo:(NSDictionary *)auxiliaryInfo {
   O2DeviceContext_gdi    *deviceContext=[[[Win32DeviceContextPrinter alloc] initWithDC:printer] autorelease];
   NSSize                  pointSize=[deviceContext pointSize];
   NSSize                  pixelsPerInch=[deviceContext pixelsPerInch];

   O2AffineTransform       scale=O2AffineTransformMakeScale(pixelsPerInch.width/72.0,pixelsPerInch.height/72.0);
   O2GState              *gState=[[[O2GState alloc] initFlippedWithDeviceHeight:pointSize.height concat:scale] autorelease];
      
   if([self initWithGraphicsState:gState deviceContext:deviceContext]==nil)
    return nil;
   
   NSString *title=[auxiliaryInfo objectForKey:kO2PDFContextTitle];
   
   if(title==nil)
    title=@"Untitled";

   [[self deviceContext] beginPrintingWithDocumentName:title];
   
   return self;
}

- (BOOL)isBitmapContext
{
    return [[self deviceContext] isKindOfClass:[Win32DeviceContextPrinter class]] == NO;
}

-initWithSize:(NSSize)size window:(CGWindow *)window {
   O2GState        *gState=[[[O2GState alloc] initFlippedWithDeviceHeight:size.height] autorelease];
   HWND                    handle=[(Win32Window *)window windowHandle];
   O2DeviceContext_gdi    *deviceContext=[[[Win32DeviceContextWindow alloc] initWithWindowHandle:handle] autorelease];

   return [self initWithGraphicsState:gState deviceContext:deviceContext];
}

-initWithSize:(NSSize)size context:(O2Context *)otherX {
   O2GState         *gState=[[[O2GState alloc] initFlippedWithDeviceHeight:size.height] autorelease];
   O2Context_gdi    *other=(O2Context_gdi *)otherX;
   O2Surface_DIBSection *surface=[[O2Surface_DIBSection alloc] initWithWidth:size.width height:size.height compatibleWithDeviceContext:[other deviceContext]];

   self=[self initWithGraphicsState:gState deviceContext:[surface deviceContext]];
   
   _surface=surface;
   
   return self;
}

-(HWND)windowHandle {
   return [[[self deviceContext] windowDeviceContext] windowHandle];
}

@end

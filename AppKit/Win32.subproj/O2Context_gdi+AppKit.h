#import <Onyx2D/O2Context_gdi.h>

@interface O2Context_gdi (AppKit)

- initWithHWND:(HWND)handle;
- initWithPrinterDC:(HDC)printer auxiliaryInfo:(NSDictionary *)auxiliaryInfo;
- (HWND)windowHandle;

@end

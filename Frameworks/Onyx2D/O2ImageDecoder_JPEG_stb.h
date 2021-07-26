#import <Onyx2D/O2ImageDecoder.h>

@interface O2ImageDecoder_JPEG_stb : O2ImageDecoder {
    CFDataRef _pixelData;
}

- initWithDataProvider:(O2DataProviderRef)dataProvider;

@end

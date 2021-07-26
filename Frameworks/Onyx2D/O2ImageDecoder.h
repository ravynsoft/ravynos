#import <Foundation/NSObject.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2ColorSpace.h>

@class O2ImageDecoder;

typedef O2ImageDecoder *O2ImageDecoderRef;

#import <Onyx2D/O2Image.h>

typedef enum {
    O2ImageCompressionPrivate,
    O2ImageCompressionJPEG,
} O2ImageCompressionType;

@interface O2ImageDecoder : NSObject {
    O2ImageCompressionType _compressionType;
    O2DataProviderRef _dataProvider;
    size_t _width;
    size_t _height;
    size_t _bitsPerComponent;
    size_t _bitsPerPixel;
    size_t _bytesPerRow;
    O2ColorSpaceRef _colorSpace;
    O2BitmapInfo _bitmapInfo;
}

O2ImageCompressionType O2ImageDecoderGetCompressionType(O2ImageDecoderRef self);

O2DataProviderRef O2ImageDecoderGetDataProvider(O2ImageDecoderRef self);

size_t O2ImageDecoderGetWidth(O2ImageDecoderRef self);
size_t O2ImageDecoderGetHeight(O2ImageDecoderRef self);

size_t O2ImageDecoderGetBitsPerComponent(O2ImageDecoderRef self);
size_t O2ImageDecoderGetBitsPerPixel(O2ImageDecoderRef self);
size_t O2ImageDecoderGetBytesPerRow(O2ImageDecoderRef self);
O2ColorSpaceRef O2ImageDecoderGetColorSpace(O2ImageDecoderRef self);

O2BitmapInfo O2ImageDecoderGetBitmapInfo(O2ImageDecoderRef self);

- (CFDataRef)createPixelData;

CFDataRef O2ImageDecoderCreatePixelData(O2ImageDecoderRef self);
O2DataProviderRef O2ImageDecoderCreatePixelDataProvider(O2ImageDecoderRef self);

@end

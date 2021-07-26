#import <Onyx2D/O2ImageDecoder.h>

@implementation O2ImageDecoder

O2ImageCompressionType O2ImageDecoderGetCompressionType(O2ImageDecoderRef self) {
    if(self==NULL)
        return O2ImageCompressionPrivate;
    
    return self->_compressionType;
}

O2DataProviderRef O2ImageDecoderGetDataProvider(O2ImageDecoderRef self) {
    if(self==NULL)
        return NULL;

    return self->_dataProvider;
}

size_t O2ImageDecoderGetWidth(O2ImageDecoderRef self) {
    if(self==NULL)
        return 0;
    
    return self->_width;
}

size_t O2ImageDecoderGetHeight(O2ImageDecoderRef self) {
    if(self==NULL)
        return 0;
    
    return self->_height;
}

size_t O2ImageDecoderGetBitsPerComponent(O2ImageDecoderRef self) {
    if(self==NULL)
        return 0;
    
    return self->_bitsPerComponent;
}

size_t O2ImageDecoderGetBitsPerPixel(O2ImageDecoderRef self) {
    if(self==NULL)
        return 0;
    
    return self->_bitsPerPixel;
}

size_t O2ImageDecoderGetBytesPerRow(O2ImageDecoderRef self) {
    if(self==NULL)
        return 0;
    
    return self->_bytesPerRow;
}

O2ColorSpaceRef O2ImageDecoderGetColorSpace(O2ImageDecoderRef self) {
    if(self==NULL)
        return NULL;
    
    return self->_colorSpace;
}

O2BitmapInfo O2ImageDecoderGetBitmapInfo(O2ImageDecoderRef self) {
    if(self==NULL)
        return 0;
    
    return self->_bitmapInfo;
}

-(CFDataRef)createPixelData {
    return nil;
}

CFDataRef O2ImageDecoderCreatePixelData(O2ImageDecoderRef self) {
    return [self createPixelData];
}

O2DataProviderRef O2ImageDecoderCreatePixelDataProvider(O2ImageDecoderRef self) {
    CFDataRef bitmap=O2ImageDecoderCreatePixelData(self);
    
    if(bitmap==NULL)
        return NULL;
    
    O2DataProviderRef result=O2DataProviderCreateWithCFData(bitmap);
    
    CFRelease(bitmap);
    
    return result;
}

@end
/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Onyx2D/O2Image.h>

@class O2ImageSource;

typedef O2ImageSource *O2ImageSourceRef;

@class NSData, NSDictionary, NSURL, O2Image, O2DataProvider;

// The O2 & CG constants must have the same values
extern NSString *kO2ImagePropertyDPIWidth;
extern NSString *kO2ImagePropertyDPIHeight;
extern NSString *kO2ImagePropertyPixelHeight;
extern NSString *kO2ImagePropertyPixelWidth;
extern NSString *kO2ImagePropertyOrientation;

extern NSString *kO2ImagePropertyTIFFDictionary;
extern NSString *kO2ImagePropertyExifDictionary;

extern NSString *kO2ImagePropertyTIFFXResolution;
extern NSString *kO2ImagePropertyTIFFYResolution;
extern NSString *kO2ImagePropertyTIFFOrientation;

@interface O2ImageSource : NSObject {
    O2DataProvider *_provider;
    NSDictionary *_options;
}

+ (O2ImageSourceRef)newImageSourceWithDataProvider:(O2DataProvider *)provider options:(CFDictionaryRef)options;
+ (O2ImageSourceRef)newImageSourceWithData:(CFDataRef)data options:(CFDictionaryRef)options;
+ (O2ImageSourceRef)newImageSourceWithURL:(NSURL *)url options:(CFDictionaryRef)options;

+ (BOOL)isPresentInDataProvider:(O2DataProvider *)provider;

- initWithDataProvider:(O2DataProvider *)provider options:(NSDictionary *)options;

- (unsigned)count;
- (CFStringRef)type;

- (CFDictionaryRef)copyPropertiesAtIndex:(unsigned)index options:(CFDictionaryRef)options;
- (O2ImageRef)createImageAtIndex:(unsigned)index options:(CFDictionaryRef)options;

O2ImageRef O2ImageSourceCreateImageAtIndex(O2ImageSourceRef self, size_t index, CFDictionaryRef options);

@end

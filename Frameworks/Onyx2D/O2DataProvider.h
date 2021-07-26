/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>
#import <Onyx2D/O2Geometry.h>

@class NSData, NSInputStream, NSURL;

@class O2DataProvider;

typedef O2DataProvider *O2DataProviderRef;

typedef void (*O2DataProviderReleaseDataCallback)(void *info, const void *data, size_t size);

@interface O2DataProvider : NSObject {
    NSInputStream *_inputStream;
    NSData *_data;
    NSString *_path;
    BOOL _isDirectAccess;
    const void *_bytes;
    size_t _length;
}

- initWithURL:(NSURL *)url;
- initWithBytes:(const void *)bytes length:(size_t)length;

O2DataProviderRef O2DataProviderCreateWithData(void *info, const void *data, size_t size, O2DataProviderReleaseDataCallback releaseCallback);
O2DataProviderRef O2DataProviderCreateWithCFData(CFDataRef data);
O2DataProviderRef O2DataProviderCreateWithURL(NSURL *url);
O2DataProviderRef O2DataProviderCreateWithFilename(const char *pathCString);
O2DataProviderRef O2DataProviderRetain(O2DataProviderRef self);
void O2DataProviderRelease(O2DataProviderRef self);
CFDataRef O2DataProviderCopyData(O2DataProviderRef self);

size_t O2DataProviderRewind(O2DataProviderRef self);
size_t O2DataProviderGetBytesAtPosition(O2DataProviderRef self, void *buffer, size_t length, size_t position);
size_t O2DataProviderGetBytes(O2DataProviderRef self, void *buffer, size_t length);

- (NSInputStream *)inputStream;

- (BOOL)isDirectAccess;

- (NSString *)path;

- (NSData *)data;
- (const void *)bytes;
- (size_t)length;

- (void)rewind;
- (NSInteger)getBytes:(void *)bytes range:(NSRange)range;

@end

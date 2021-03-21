/* Copyright (c) 2009 Christopher J. W. Lloyd <cjwl@objc.net>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>
#import <Foundation/NSData.h>
#import <Foundation/NSFileHandle.h>
#import <CoreFoundation/CFData.h>
#import <CoreFoundation/CFURL.h>

@class O2DataConsumer;
typedef O2DataConsumer *O2DataConsumerRef;

// These types and structures must match the corresponding CG ones
typedef size_t (*O2DataConsumerPutBytesCallback)(void *info, const void *buffer, size_t count);

typedef void (*O2DataConsumerReleaseInfoCallback)(void *info);

struct O2DataConsumerCallbacks {
    O2DataConsumerPutBytesCallback putBytes;
    O2DataConsumerReleaseInfoCallback releaseConsumer;
};

typedef struct O2DataConsumerCallbacks O2DataConsumerCallbacks;

@interface O2DataConsumer : NSObject {
    void *_info;
    O2DataConsumerCallbacks _callbacks;
}

- (NSMutableData *)mutableData;

O2DataConsumerRef O2DataConsumerCreate(void *info, const O2DataConsumerCallbacks *callbacks);
O2DataConsumerRef O2DataConsumerCreateWithCFData(CFMutableDataRef data);
O2DataConsumerRef O2DataConsumerCreateWithURL(CFURLRef url);
O2DataConsumerRef O2DataConsumerRetain(O2DataConsumerRef self);
void O2DataConsumerRelease(O2DataConsumerRef self);

// private internal use
size_t O2DataConsumerPutBytes(O2DataConsumerRef self, const void *buffer, size_t count);

@end

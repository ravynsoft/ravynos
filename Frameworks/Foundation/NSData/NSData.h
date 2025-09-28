/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>

@class NSURL, NSError;

enum {
    NSDataReadingMapped = 0x01,
    NSDataReadingUncached = 0x02,

    // deprecated
    NSMappedRead = NSDataReadingMapped,
    NSUncachedRead = NSDataReadingUncached,
};
typedef NSUInteger NSDataReadingOptions;

enum {
    NSDataWritingAtomic = 0x01,

    // deprecated
    NSAtomicWrite = NSDataWritingAtomic,
};

typedef NSUInteger NSDataWritingOptions;

enum {
    NSDataSearchBackwards = 0x01,
    NSDataSearchAnchored = 0x02,
};
typedef NSUInteger NSDataSearchOptions;

@interface NSData : NSObject <NSCopying, NSMutableCopying, NSCoding>

- initWithBytesNoCopy:(void *)bytes length:(NSUInteger)length freeWhenDone:(BOOL)freeWhenDone;
- initWithBytesNoCopy:(void *)bytes length:(NSUInteger)length;
- initWithBytes:(const void *)bytes length:(NSUInteger)length;
- initWithData:(NSData *)data;
- initWithContentsOfFile:(NSString *)path;
- initWithContentsOfMappedFile:(NSString *)path;
- initWithContentsOfURL:(NSURL *)url;
- initWithContentsOfFile:(NSString *)path options:(NSUInteger)options error:(NSError **)errorp;
- initWithContentsOfURL:(NSURL *)url options:(NSUInteger)options error:(NSError **)errorp;

+ data;
+ dataWithBytesNoCopy:(void *)bytes length:(NSUInteger)length freeWhenDone:(BOOL)freeWhenDone;
+ dataWithBytesNoCopy:(void *)bytes length:(NSUInteger)length;
+ dataWithBytes:(const void *)bytes length:(NSUInteger)length;
+ dataWithData:(NSData *)data;
+ dataWithContentsOfFile:(NSString *)path;
+ dataWithContentsOfMappedFile:(NSString *)path;
+ dataWithContentsOfURL:(NSURL *)url;
+ dataWithContentsOfFile:(NSString *)path options:(NSUInteger)options error:(NSError **)errorp;
+ dataWithContentsOfURL:(NSURL *)url options:(NSUInteger)options error:(NSError **)errorp;

- (const void *)bytes;
- (NSUInteger)length;

- (BOOL)isEqualToData:(NSData *)data;

- (void)getBytes:(void *)result range:(NSRange)range;
- (void)getBytes:(void *)result length:(NSUInteger)length;
- (void)getBytes:(void *)result;

- (NSData *)subdataWithRange:(NSRange)range;

- (BOOL)writeToFile:(NSString *)path atomically:(BOOL)atomically;
- (BOOL)writeToURL:(NSURL *)url atomically:(BOOL)atomically;
- (BOOL)writeToFile:(NSString *)path options:(NSUInteger)options error:(NSError **)errorp;
- (BOOL)writeToURL:(NSURL *)url options:(NSUInteger)options error:(NSError **)errorp;

- (NSString *)description;

@end

#import <Foundation/NSMutableData.h>

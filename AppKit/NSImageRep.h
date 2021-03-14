/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSArray, NSData, NSPasteboard, NSURL;

@interface NSImageRep : NSObject <NSCopying> {
    NSSize _size;
    NSString *_colorSpaceName;
    NSInteger _bitsPerSample;
    int _pixelsWide;
    int _pixelsHigh;

    BOOL _hasAlpha;
    BOOL _isOpaque;
}

+ (NSArray *)registeredImageRepClasses;
+ (void)registerImageRepClass:(Class)aClass;
+ (void)unregisterImageRepClass:(Class)aClass;

+ (NSArray *)imageFileTypes;
+ (NSArray *)imageUnfilteredFileTypes;
+ (NSArray *)imagePasteboardTypes;
+ (NSArray *)imageUnfilteredPasteboardTypes;

+ (BOOL)canInitWithData:(NSData *)data;
+ (BOOL)canInitWithPasteboard:(NSPasteboard *)pasteboard;

+ (Class)imageRepClassForData:(NSData *)data;
+ (Class)imageRepClassForFileType:(NSString *)type;
+ (Class)imageRepClassForPasteboardType:(NSString *)type;

+ (NSArray *)imageRepsWithContentsOfFile:(NSString *)path;
+ (NSArray *)imageRepsWithContentsOfURL:(NSURL *)url;
+ (NSArray *)imageRepsWithPasteboard:(NSPasteboard *)pasteboard;
+ imageRepWithContentsOfFile:(NSString *)path;
+ imageRepWithContentsOfURL:(NSURL *)url;
+ imageRepWithPasteboard:(NSPasteboard *)pasteboard;

- (NSSize)size;
- (int)pixelsWide;
- (int)pixelsHigh;
- (BOOL)isOpaque;
- (BOOL)hasAlpha;
- (NSString *)colorSpaceName;
- (NSInteger)bitsPerSample;

- (void)setSize:(NSSize)value;
- (void)setPixelsWide:(int)value;
- (void)setPixelsHigh:(int)value;
- (void)setOpaque:(BOOL)value;
- (void)setAlpha:(BOOL)value;
- (void)setColorSpaceName:(NSString *)value;
- (void)setBitsPerSample:(int)value;

- (BOOL)draw;
- (BOOL)drawAtPoint:(NSPoint)point;
- (BOOL)drawInRect:(NSRect)rect;

@end

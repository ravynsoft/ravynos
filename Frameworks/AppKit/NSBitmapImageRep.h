/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSImageRep.h>
#import <AppKit/NSGraphics.h>
#import <ApplicationServices/ApplicationServices.h>

typedef enum {
    NSTIFFFileType,
    NSBMPFileType,
    NSGIFFileType,
    NSJPEGFileType,
    NSPNGFileType,
    NSJPEG2000FileType,
} NSBitmapImageFileType;

typedef enum {
    NSTIFFCompressionNone = 1,
    NSTIFFCompressionCCITTFAX3 = 3,
    NSTIFFCompressionCCITTFAX4 = 4,
    NSTIFFCompressionLZW = 5,
    NSTIFFCompressionJPEG = 6,
    NSTIFFCompressionNEXT = 32766,
    NSTIFFCompressionPackBits = 32773,
    NSTIFFCompressionOldJPEG = 32865,
} NSTIFFCompression;

typedef enum {
    NSAlphaFirstBitmapFormat = 0x01,
    NSAlphaNonpremultipliedBitmapFormat = 0x02,
    NSFloatingPointSamplesBitmapFormat = 0x04,
} NSBitmapFormat;

APPKIT_EXPORT NSString *NSImageCompressionFactor;

@interface NSBitmapImageRep : NSImageRep {
    int _samplesPerPixel;
    int _bitsPerPixel;
    int _bytesPerRow;
    int _bytesPerPlane;
    NSBitmapFormat _bitmapFormat;
    BOOL _freeWhenDone;
    BOOL _isPlanar;
    unsigned char **_bitmapPlanes;
    NSMutableDictionary *_properties;

    CGImageRef _cgImage;
}

+ (void)getTIFFCompressionTypes:(const NSTIFFCompression **)types count:(int *)count;
+ (NSString *)localizedNameForTIFFCompressionType:(NSTIFFCompression)type;
+ (NSData *)TIFFRepresentationOfImageRepsInArray:(NSArray *)array;
+ (NSData *)TIFFRepresentationOfImageRepsInArray:(NSArray *)array usingCompression:(NSTIFFCompression)compression factor:(float)factor;

+ (NSData *)representationOfImageRepsInArray:(NSArray *)array usingType:(NSBitmapImageFileType)type properties:(NSDictionary *)properties;

+ (NSArray *)imageRepsWithData:(NSData *)data;
+ imageRepWithData:(NSData *)data;

- initWithBitmapDataPlanes:(unsigned char **)planes pixelsWide:(int)width pixelsHigh:(int)height bitsPerSample:(int)bitsPerSample samplesPerPixel:(int)samplesPerPixel hasAlpha:(BOOL)hasAlpha isPlanar:(BOOL)isPlanar colorSpaceName:(NSString *)colorSpaceName bitmapFormat:(NSBitmapFormat)bitmapFormat bytesPerRow:(int)bytesPerRow bitsPerPixel:(int)bitsPerPixel;

- initWithBitmapDataPlanes:(unsigned char **)planes pixelsWide:(int)width pixelsHigh:(int)height bitsPerSample:(int)bitsPerSample samplesPerPixel:(int)samplesPerPixel hasAlpha:(BOOL)hasAlpha isPlanar:(BOOL)isPlanar colorSpaceName:(NSString *)colorSpaceName bytesPerRow:(int)bytesPerRow bitsPerPixel:(int)bitsPerPixel;

- initForIncrementalLoad;

- initWithFocusedViewRect:(NSRect)rect;

- initWithData:(NSData *)data;
- initWithContentsOfFile:(NSString *)path;
- initWithCGImage:(CGImageRef)cgImage;

- (int)incrementalLoadFromData:(NSData *)data complete:(BOOL)complete;

- (int)bitsPerPixel;
- (int)samplesPerPixel;
- (int)bytesPerRow;
- (BOOL)isPlanar;
- (int)numberOfPlanes;
- (int)bytesPerPlane;

- (NSBitmapFormat)bitmapFormat;
- (unsigned char *)bitmapData;

- (void)getBitmapDataPlanes:(unsigned char **)planes;

- (void)getPixel:(NSUInteger[])pixel atX:(NSInteger)x y:(NSInteger)y;
- (void)setPixel:(NSUInteger[])pixel atX:(NSInteger)x y:(NSInteger)y;

- (NSColor *)colorAtX:(NSInteger)x y:(NSInteger)y;
- (void)setColor:(NSColor *)color atX:(NSInteger)x y:(NSInteger)y;

- valueForProperty:(NSString *)property;
- (void)setProperty:(NSString *)property withValue:value;

- (void)colorizeByMappingGray:(float)gray toColor:(NSColor *)color blackMapping:(NSColor *)blackMapping whiteMapping:(NSColor *)whiteMapping;

- (void)getCompression:(NSTIFFCompression *)compression factor:(float *)factor;
- (void)setCompression:(NSTIFFCompression)compression factor:(float)factor;

- (BOOL)canBeCompressedUsing:(NSTIFFCompression)compression;

- (NSData *)representationUsingType:(NSBitmapImageFileType)type properties:(NSDictionary *)properties;

- (NSData *)TIFFRepresentation;
- (NSData *)TIFFRepresentationUsingCompression:(NSTIFFCompression)compression factor:(float)factor;

- (CGImageRef)CGImage;

@end

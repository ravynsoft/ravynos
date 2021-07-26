/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSGraphics.h>
#import <AppKit/NSBitmapImageRep.h>

@class NSImageRep;

typedef enum {
    NSImageCacheDefault,
    NSImageCacheAlways,
    NSImageCacheBySize,
    NSImageCacheNever,
} NSImageCacheMode;

@interface NSImage : NSObject <NSCopying, NSCoding> {
    NSString *_name;
    NSSize _size;
    NSColor *_backgroundColor;
    NSMutableArray *_representations;
    id _delegate;
    BOOL _isFlipped;
    BOOL _isTemplate;
    BOOL _scalesWhenResized;
    BOOL _matchesOnMultipleResolution;
    BOOL _usesEPSOnResolutionMismatch;
    BOOL _prefersColorMatch;
    BOOL _isCachedSeparately;
    BOOL _cacheDepthMatchesImageDepth;
    BOOL _isDataRetained;
    BOOL _cacheIsValid;
    NSImageCacheMode _cacheMode;
}

+ (NSArray *)imageFileTypes;
+ (NSArray *)imageUnfilteredFileTypes;
+ (NSArray *)imagePasteboardTypes;
+ (NSArray *)imageUnfilteredPasteboardTypes;

+ (BOOL)canInitWithPasteboard:(NSPasteboard *)pasteboard;

+ imageNamed:(NSString *)name;

- initWithSize:(NSSize)size;
- initWithData:(NSData *)data;
- initWithContentsOfFile:(NSString *)path;
- initWithContentsOfURL:(NSURL *)url;
- initWithCGImage:(CGImageRef)cgImage size:(NSSize)size;

- initWithPasteboard:(NSPasteboard *)pasteboard;
- initByReferencingFile:(NSString *)path;
- initByReferencingURL:(NSURL *)url;

- (NSString *)name;
- (NSSize)size;
- (NSColor *)backgroundColor;
- (BOOL)isFlipped;
- (BOOL)isTemplate;
- (BOOL)scalesWhenResized;
- (BOOL)matchesOnMultipleResolution;
- (BOOL)usesEPSOnResolutionMismatch;
- (BOOL)prefersColorMatch;
- (NSImageCacheMode)cacheMode;
- (BOOL)isCachedSeparately;
- (BOOL)cacheDepthMatchesImageDepth;
- (BOOL)isDataRetained;
- delegate;

- (BOOL)setName:(NSString *)value;
- (void)setSize:(NSSize)value;
- (void)setBackgroundColor:(NSColor *)value;
- (void)setFlipped:(BOOL)value;
- (void)setTemplate:(BOOL)value;
- (void)setScalesWhenResized:(BOOL)value;
- (void)setMatchesOnMultipleResolution:(BOOL)value;
- (void)setUsesEPSOnResolutionMismatch:(BOOL)value;
- (void)setPrefersColorMatch:(BOOL)value;
- (void)setCacheMode:(NSImageCacheMode)value;
- (void)setCachedSeparately:(BOOL)value;
- (void)setCacheDepthMatchesImageDepth:(BOOL)value;
- (void)setDataRetained:(BOOL)value;
- (void)setDelegate:delegate;

- (BOOL)isValid;

- (NSArray *)representations;
- (void)addRepresentation:(NSImageRep *)representation;
- (void)addRepresentations:(NSArray *)array;
- (void)removeRepresentation:(NSImageRep *)representation;
- (NSImageRep *)bestRepresentationForDevice:(NSDictionary *)device;

- (void)recache;
- (void)cancelIncrementalLoad;

- (NSData *)TIFFRepresentation;
- (NSData *)TIFFRepresentationUsingCompression:(NSTIFFCompression)compression factor:(float)factor;

- (void)lockFocus;
- (void)lockFocusOnRepresentation:(NSImageRep *)representation;
- (void)unlockFocus;

- (BOOL)drawRepresentation:(NSImageRep *)representation inRect:(NSRect)rect;

- (void)compositeToPoint:(NSPoint)point fromRect:(NSRect)rect operation:(NSCompositingOperation)operation;
- (void)compositeToPoint:(NSPoint)point fromRect:(NSRect)rect operation:(NSCompositingOperation)operation fraction:(float)fraction;

- (void)compositeToPoint:(NSPoint)point operation:(NSCompositingOperation)operation;
- (void)compositeToPoint:(NSPoint)point operation:(NSCompositingOperation)operation fraction:(float)fraction;

- (void)dissolveToPoint:(NSPoint)point fraction:(float)fraction;
- (void)dissolveToPoint:(NSPoint)point fromRect:(NSRect)rect fraction:(float)fraction;

- (void)drawAtPoint:(NSPoint)point fromRect:(NSRect)source operation:(NSCompositingOperation)operation fraction:(float)fraction;
- (void)drawInRect:(NSRect)rect fromRect:(NSRect)source operation:(NSCompositingOperation)operation fraction:(float)fraction;

@end

@interface NSBundle (NSImage)
- (NSString *)pathForImageResource:(NSString *)name;
@end

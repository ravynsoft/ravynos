/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSRulerView, NSImage, NSEvent;

@interface NSRulerMarker : NSObject <NSCopying> //, NSCoding>
                           {
    NSRulerView *_ruler;
    float _markerLocation;
    NSImage *_image;
    NSPoint _imageOrigin;
    id _representedObject;
    BOOL _isRemovable;
    BOOL _isMovable;
    BOOL _isDragging;
    BOOL _isPinned;
}

+ (NSImage *)defaultMarkerImage;

- initWithRulerView:(NSRulerView *)ruler markerLocation:(float)location image:(NSImage *)image imageOrigin:(NSPoint)point;

- (NSRulerView *)ruler;
- (float)markerLocation;
- (NSImage *)image;
- (NSPoint)imageOrigin;

- (id<NSCopying>)representedObject;
- (BOOL)isRemovable;
- (BOOL)isMovable;

- (void)setMarkerLocation:(float)location;
- (void)setImage:(NSImage *)image;
- (void)setImageOrigin:(NSPoint)point;

- (void)setRepresentedObject:(id<NSCopying>)object;
- (void)setRemovable:(BOOL)flag;
- (void)setMovable:(BOOL)flag;

- (float)thicknessRequiredInRuler;
- (NSRect)imageRectInRuler;
- (void)drawRect:(NSRect)rect;
- (BOOL)isDragging;
- (BOOL)trackMouse:(NSEvent *)event adding:(BOOL)adding;

@end

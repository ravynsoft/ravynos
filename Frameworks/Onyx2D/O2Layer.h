/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>
@class O2Context, NSDictionary, O2Layer, O2Surface;

typedef O2Layer *O2LayerRef;

#import <Onyx2D/O2Context.h>
#import <Onyx2D/O2Image.h>

@interface O2Layer : NSObject {
    O2ContextRef _context;
    O2Size _size;
    NSDictionary *_unused;
    O2Surface *_surface;
}

O2Surface *O2LayerGetSurface(O2LayerRef self);

O2LayerRef O2LayerCreateWithContext(O2ContextRef context, O2Size size, NSDictionary *unused);
O2LayerRef O2LayerRetain(O2LayerRef self);
void O2LayerRelease(O2LayerRef self);
O2Size O2LayerGetSize(O2LayerRef self);
O2ContextRef O2LayerGetContext(O2LayerRef self);

@end

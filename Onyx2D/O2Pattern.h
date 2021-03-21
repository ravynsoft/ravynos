/* Copyright (c) 2008 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>
#import <Onyx2D/O2AffineTransform.h>
#import <stdbool.h>

@class O2Pattern;

typedef O2Pattern *O2PatternRef;

typedef enum {
    kO2PatternTilingNoDistortion,
    kO2PatternTilingConstantSpacingMinimalDistortion,
    kO2PatternTilingConstantSpacing,
} O2PatternTiling;

#import <Onyx2D/O2Context.h>

typedef struct {
    unsigned int version;
    void (*drawPattern)(void *, O2ContextRef);
    void (*releaseInfo)(void *);
} O2PatternCallbacks;

@interface O2Pattern : NSObject {
    void *_info;
    O2Rect _bounds;
    O2AffineTransform _matrix;
    O2Float _xstep;
    O2Float _ystep;
    O2PatternTiling _tiling;
    BOOL _isColored;
    O2PatternCallbacks _callbacks;
}

- (O2Rect)bounds;
- (O2AffineTransform)matrix;
- (O2Float)xstep;
- (O2Float)ystep;

- (void)drawInContext:(O2ContextRef)context;

O2PatternRef O2PatternCreate(void *info, O2Rect bounds, O2AffineTransform matrix, O2Float xStep, O2Float yStep, O2PatternTiling tiling, bool isColored, const O2PatternCallbacks *callbacks);

O2PatternRef O2PatternRetain(O2PatternRef self);
void O2PatternRelease(O2PatternRef self);

@end

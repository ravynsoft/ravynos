/*------------------------------------------------------------------------
 *
 * Derivative of the OpenVG 1.0.1 Reference Implementation
 * -------------------------------------
 *
 * Copyright (c) 2007 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and /or associated documentation files
 * (the "Materials "), to deal in the Materials without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Materials,
 * and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions: 
 *
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Materials. 
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR
 * THE USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 *-------------------------------------------------------------------*/

#import <Onyx2D/O2Surface.h>

@class O2Paint;
typedef O2Paint *O2PaintRef;

// this function returns the number of pixels read as a positive value or skipped as a negative value
typedef int (*O2PaintReadSpan_argb8u_PRE_function)(O2Paint *self, int x, int y, O2argb8u *span, int length);
typedef int (*O2PaintReadSpan_largb32f_PRE_function)(O2Paint *self, int x, int y, O2argb32f *span, int length);

@interface O2Paint : NSObject {
  @public
    O2PaintReadSpan_argb8u_PRE_function _paint_largb8u_PRE;
    O2PaintReadSpan_largb32f_PRE_function _paint_largb32f_PRE;
    bool isOpaque;
  @protected
    O2AffineTransform m_surfaceToPaintMatrix;
}

O2PaintRef O2PaintInitWithTransform(O2PaintRef self, O2AffineTransform transform);

O2PaintRef O2PaintRetain(O2PaintRef self);
void O2PaintRelease(O2PaintRef self);

@end

// _always_ generates alpha=1.0
static inline bool O2PaintIsOpaque(O2PaintRef self) {
    return self->isOpaque;
}

static inline int O2PaintReadSpan_argb8u_PRE(O2Paint *self, int x, int y, O2argb8u *span, int length) {
    return self->_paint_largb8u_PRE(self, x, y, span, length);
}

static inline int O2PaintReadSpan_largb32f_PRE(O2Paint *self, int x, int y, O2argb32f *span, int length) {
    return self->_paint_largb32f_PRE(self, x, y, span, length);
}

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

#import <Onyx2D/VGmath.h>
#import <Onyx2D/O2Path.h>
#import <Onyx2D/O2Context_builtin.h>

@interface VGPath : NSObject {
    O2Path *_path;
    int _vertexCount;
    int _vertexCapacity;
    struct Vertex *_vertices;

    int _segmentToVertexCapacity;
    struct VertexIndex *_segmentToVertex;

    O2Float m_userMinx;
    O2Float m_userMiny;
    O2Float m_userMaxx;
    O2Float m_userMaxy;
}

- initWithKGPath:(O2Path *)path;

void VGPathFill(VGPath *self, O2AffineTransform pathToSurface, O2Context_builtin *context);
void VGPathStroke(VGPath *self, O2AffineTransform pathToSurface, O2Context_builtin *context, const O2Float *dashPattern, int dashPatternSize, O2Float dashPhase, BOOL dashPhaseReset, O2Float strokeWidth, O2LineCap capStyle, O2LineJoin joinStyle, O2Float miterLimit);

void VGPathGetPointAlong(VGPath *self, int startIndex, int numSegments, O2Float distance, O2Point *p, O2Point *t);
O2Float VGPathGetLength(VGPath *self, int startIndex, int numSegments);
void VGPathGetPathBounds(VGPath *self, O2Float *minx, O2Float *miny, O2Float *maxx, O2Float *maxy);
void VGPathGetPathTransformedBounds(VGPath *self, O2AffineTransform pathToSurface, O2Float *minx, O2Float *miny, O2Float *maxx, O2Float *maxy);

void VGPathTessellateIfNeeded(VGPath *self);

@end

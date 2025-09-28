/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>

@class O2Function;
typedef O2Function *O2FunctionRef;

typedef struct {
    unsigned version;
    void (*evaluate)(void *, const float *, float *);
    void (*releaseInfo)(void *);
} O2FunctionCallbacks;

@interface O2Function : NSObject {
    void *_info;
    unsigned _domainCount;
    float *_domain;
    unsigned _rangeCount;
    float *_range;
    O2FunctionCallbacks _callbacks;
}

O2FunctionRef O2FunctionCreate(void *info, size_t domainDimension, const O2Float *domain, size_t rangeDimension, const O2Float *range, const O2FunctionCallbacks *callbacks);

O2FunctionRef O2FunctionRetain(O2FunctionRef self);
void O2FunctionRelease(O2FunctionRef self);

// FIX, only works for one input value
void O2FunctionEvaluate(O2FunctionRef self, O2Float in, O2Float *out);

- (unsigned)domainCount;
- (const float *)domain;
- (unsigned)rangeCount;
- (const float *)range;

- (BOOL)isLinear;

@end

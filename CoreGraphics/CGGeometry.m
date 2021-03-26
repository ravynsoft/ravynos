/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef MIN
#define MIN(a,b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b); (_a < _b) ? _a : _b; })
#else
#warning MIN is already defined, MIN(a, b) may not behave as expected.
#endif

#ifndef MAX
#define MAX(a,b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b); (_a > _b) ? _a : _b; })
#else
#warning MAX is already defined, MAX(a, b) may not not behave as expected.
#endif

#import <CoreGraphics/CGGeometry.h>

const CGRect CGRectZero={{0,0},{0,0}};
const CGRect CGRectNull={{INFINITY,INFINITY},{0,0}};
const CGPoint CGPointZero={0,0};
const CGSize CGSizeZero={0,0};

CGRect CGRectIntersection(CGRect rect0, CGRect rect1) {
	CGRect result;
	
	if(CGRectGetMaxX(rect0)<=CGRectGetMinX(rect1) || CGRectGetMinX(rect0)>=CGRectGetMaxX(rect1) ||
	   CGRectGetMaxY(rect0)<=CGRectGetMinY(rect1) || CGRectGetMinY(rect0)>=CGRectGetMaxY(rect1))
		return CGRectZero;
	
	result.origin.x=MAX(CGRectGetMinX(rect0), CGRectGetMinX(rect1));
	result.origin.y=MAX(CGRectGetMinY(rect0), CGRectGetMinY(rect1));
	result.size.width=MIN(CGRectGetMaxX(rect0), CGRectGetMaxX(rect1))-result.origin.x;
	result.size.height=MIN(CGRectGetMaxY(rect0), CGRectGetMaxY(rect1))-result.origin.y;
	
	return result;
}

CGRect CGRectIntegral(CGRect rect) {
	if (!CGRectIsEmpty(rect)) {
		float maxX = ceil(CGRectGetMaxX(rect));
		float maxY = ceil(CGRectGetMaxY(rect));
		rect.origin.x = floor(rect.origin.x); 
		rect.origin.y = floor(rect.origin.y);
		rect.size.width =  maxX - CGRectGetMinX(rect); 
		rect.size.height = maxY - CGRectGetMinY(rect); 
	}
	return rect; 
	
}

CGRect CGRectUnion(CGRect a, CGRect b) {
	// make sure we handle null!
	if (CGRectIsNull(a)) {
		return b;
	}
	
	if (CGRectIsNull(b)) {
		return a;
	}
	
	float minX = MIN(CGRectGetMinX(a), CGRectGetMinX(b));
	float minY = MIN(CGRectGetMinY(a), CGRectGetMinY(b));
	float maxX = MAX(CGRectGetMaxX(a), CGRectGetMaxX(b));
	float maxY = MAX(CGRectGetMaxY(a), CGRectGetMaxY(b));
	return CGRectMake(minX, minY, maxX - minX, maxY - minY);
}

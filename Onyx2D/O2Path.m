/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Path.h>
#import <Onyx2D/O2MutablePath.h>
#import <Onyx2D/O2Exceptions.h>

@implementation O2Path

-initWithOperators:(unsigned char *)elements numberOfElements:(unsigned)numberOfElements points:(O2Point *)points numberOfPoints:(unsigned)numberOfPoints {
   return O2PathInitWithOperators(self,elements,numberOfElements,points,numberOfPoints);
}

id O2PathInitWithOperators(O2Path *self,unsigned char *elements,unsigned numberOfElements,O2Point *points,unsigned numberOfPoints) {
   int i;
   
   self->_numberOfElements=numberOfElements;
   self->_elements=NSZoneMalloc(NULL,(self->_numberOfElements==0)?1:self->_numberOfElements);
   for(i=0;i<self->_numberOfElements;i++)
    self->_elements[i]=elements[i];

   self->_numberOfPoints=numberOfPoints;
   self->_points=NSZoneMalloc(NULL,(self->_numberOfPoints==0?1:self->_numberOfPoints)*sizeof(O2Point));
   for(i=0;i<self->_numberOfPoints;i++)
    self->_points[i]=points[i];
    
   return self;
}

-init {
   return [self initWithOperators:NULL numberOfElements:0 points:NULL numberOfPoints:0];
}

-(void)dealloc {
   if(_elements!=NULL)
   NSZoneFree(NULL,_elements);
   if(_points!=NULL)
   NSZoneFree(NULL,_points);
   [super dealloc];
}

void O2PathRelease(O2PathRef self) {
   if(self!=NULL)
    CFRelease(self);
}

O2PathRef O2PathRetain(O2PathRef self) {
   return (self!=NULL)?(O2PathRef)CFRetain(self):NULL;
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

O2PathRef O2PathCreateCopy(O2PathRef self) {
   return [self copyWithZone:NULL];
}

O2MutablePathRef O2PathCreateMutableCopy(O2PathRef self) {
   return [[O2MutablePath allocWithZone:NULL] initWithOperators:self->_elements numberOfElements:self->_numberOfElements points:self->_points numberOfPoints:self->_numberOfPoints];
}

unsigned O2PathNumberOfElements(O2PathRef self) {
   return self->_numberOfElements;
}

const unsigned char *O2PathElements(O2PathRef self) {
   return self->_elements;
}

unsigned O2PathNumberOfPoints(O2PathRef self) {
   return self->_numberOfPoints;
}

const O2Point *O2PathPoints(O2PathRef self) {
   return self->_points;
}

O2Point O2PathGetCurrentPoint(O2PathRef self) {
// this is wrong w/ closepath last
   return (self->_numberOfPoints==0)?O2PointZero:self->_points[self->_numberOfPoints-1];
}

BOOL    O2PathEqualToPath(O2PathRef self,O2PathRef other){
   unsigned otherNumberOfOperators=O2PathNumberOfElements(other);
   
   if(self->_numberOfElements==otherNumberOfOperators){
    unsigned otherNumberOfPoints=O2PathNumberOfPoints(other);
    
    if(self->_numberOfPoints==otherNumberOfPoints){
     const unsigned char *otherOperators=O2PathElements(other);
     const O2Point       *otherPoints;
     int i;
     
     for(i=0;i<self->_numberOfElements;i++)
      if(self->_elements[i]!=otherOperators[i])
       return NO;
       
     otherPoints=O2PathPoints(other);
     for(i=0;i<self->_numberOfPoints;i++)
      if(!O2PointEqualToPoint(self->_points[i],otherPoints[i]))
       return NO;
       
     return YES;
    }
    
   }
   return NO;
}

BOOL O2PathIsEmpty(O2PathRef self) {
   return self->_numberOfElements==0;
}

BOOL O2PathIsRect(O2PathRef self,O2Rect *rect) {
   if(self->_numberOfElements!=5)
    return NO;
   if(self->_elements[0]!=kO2PathElementMoveToPoint)
    return NO;
   if(self->_elements[1]!=kO2PathElementAddLineToPoint)
    return NO;
   if(self->_elements[2]!=kO2PathElementAddLineToPoint)
    return NO;
   if(self->_elements[3]!=kO2PathElementAddLineToPoint)
    return NO;
   if(self->_elements[4]!=kO2PathElementCloseSubpath)
    return NO;
   
   if(self->_points[0].y!=self->_points[1].y)
    return NO;
   if(self->_points[1].x!=self->_points[2].x)
    return NO;
   if(self->_points[2].y!=self->_points[3].y)
    return NO;
   if(self->_points[3].x!=self->_points[0].x)
    return NO;
   
// FIXME: this is probably wrong, coordinate order   
   rect->origin=self->_points[0];
   rect->size=O2SizeMake(self->_points[2].x-self->_points[0].x,self->_points[2].y-self->_points[0].y);
   
   return YES;
}

// _positionOfPointToLine(): tests if a point is Left|On|Right of an infinite line.
//    Input:  three points lineStart, lineEnd, and the point to test
//    Return: >0 for point left of the line
//            =0 for point on the line
//            <0 for point right of the line
//    See: the January 2001 Algorithm "Area of 2D and 3D Triangles and Polygons"

static float _positionOfPointToLine(O2Point point, O2Point lineStart, O2Point lineEnd)
{
	float position = ((lineEnd.x - lineStart.x) * (point.y - lineStart.y) - 
					(point.x - lineStart.x) * (lineEnd.y - lineStart.y));
	
	return position;
}

static BOOL _curveIsFlat(float desiredFlatness, O2Point start, O2Point cp1, O2Point cp2, O2Point end)
{
	// Roughly compute the furthest distance of the curved path from the line connecting start to end
	double ux = 3.0*cp1.x - 2.0*start.x - end.x; ux *= ux;
	double uy = 3.0*cp1.y - 2.0*start.y - end.y; uy *= uy;
	double vx = 3.0*cp2.x - 2.0*end.x - start.x; vx *= vx;
	double vy = 3.0*cp2.y - 2.0*end.y - start.y; vy *= vy;
	if (ux < vx) ux = vx;
	if (uy < vy) uy = vy;
	return (ux+uy <= desiredFlatness);
}

int _countEvenOddCrossingOfPointAndLine(O2Point point, O2Point lineStart, O2Point lineEnd)
{
	if (((lineStart.y <= point.y) && (lineEnd.y > point.y)) ||  // an upward crossing
		((lineStart.y > point.y) && (lineEnd.y <= point.y))) {	// a downward crossing

		// compute the actual edge-ray intersect x-coordinate
		float vt = (float)(point.y - lineStart.y) / (lineEnd.y - lineStart.y);

		if (point.x < lineStart.x + vt * (lineEnd.x - lineStart.x)) { // point.x < intersect
			return 1;   // a valid crossing of y=point.y right of point.x
		}
	}
	return 0;
}

int _countWindingAroundPointByLine(O2Point point, O2Point lineStart, O2Point lineEnd)
{
	if (lineStart.y <= point.y) {
		if (lineEnd.y > point.y)  { // an upward crossing 
			if (_positionOfPointToLine(point, lineStart, lineEnd) > 0) { // point is left of edge
				return 1;   // so we have a valid "up" intersect
			}
		}
	}
	else {   // lineStart.y > point.y (no test needed)
		
		if (lineEnd.y <= point.y) { // a downward crossing

			if (_positionOfPointToLine(point, lineStart, lineEnd) < 0) { // point is right of edge
				return -1;   // so we have a valid "down" intersect
			}
		}
	}
	return 0;
}

// Count the crossings on the Y axis from the point to a bezier curve
int _countYCrossingFromPointToCurve(O2Point point, O2Point curveFromPoint, O2Point curveToPoint, O2Point tan1, O2Point tan2, BOOL evenOdd)
{
	int count = 0;
	
	// No need to test if there no chance of any crossing
	float minY = MIN(MIN(curveFromPoint.y, curveToPoint.y), MIN(tan1.y, tan2.y));
	float maxY = MAX(MAX(curveFromPoint.y, curveToPoint.y), MAX(tan1.y, tan2.y));
	if (minY > point.y ||  maxY < point.y) {
		return 0;
	}
	
	if (_curveIsFlat(0.6, curveFromPoint, tan1, tan2, curveToPoint)) {
		if (evenOdd) {
			// Flat enough curve : handle it like a segment
			count += _countEvenOddCrossingOfPointAndLine(point, curveFromPoint, curveToPoint);
		} else {
			count += _countWindingAroundPointByLine(point, curveFromPoint, curveToPoint);
		}
	} else {
		// Subdivide the bezier path and test both subpaths - adapted from the flatten path code
		O2Point sub1_start = curveFromPoint;
		O2Point sub1_cp1 = O2PointMake((curveFromPoint.x + tan1.x)/2, (curveFromPoint.y + tan1.y)/2);
		O2Point T = O2PointMake((tan1.x + tan2.x)/2, (tan1.y + tan2.y)/2);
		O2Point sub1_cp2 = O2PointMake((sub1_cp1.x + T.x)/2, (sub1_cp1.y + T.y)/2);
		O2Point sub2_end = curveToPoint;
		O2Point sub2_cp2 = O2PointMake((tan2.x + curveToPoint.x)/2, (tan2.y + curveToPoint.y)/2);
		O2Point sub2_cp1 = O2PointMake((T.x + sub2_cp2.x)/2, (T.y + sub2_cp2.y)/2);
		O2Point sub2_start = O2PointMake((sub1_cp2.x + sub2_cp1.x)/2, (sub1_cp2.y + sub2_cp1.y)/2);
		O2Point sub1_end = sub2_start;
		
		count += _countYCrossingFromPointToCurve(point, sub1_start, sub1_end, sub1_cp1, sub1_cp2, evenOdd);
		count += _countYCrossingFromPointToCurve(point, sub2_start, sub2_end, sub2_cp1, sub2_cp2, evenOdd);
	}
	return count;
}

BOOL O2PathContainsPoint(O2PathRef self,const O2AffineTransform *xform,O2Point point,BOOL evenOdd)
{
	if (O2PathIsEmpty(self)) {
		return NO;
	}
	
	// Some quick test first
	if (O2RectContainsPoint(O2PathGetBoundingBox(self), point) == NO) {
		return NO;
	}
	
	// Adapted from the implementation in NSBezierPath which is in turn taken from
	// http://softsurfer.com/Archive/algorithm_0103/algorithm_0103.htm
	
	int  cn = 0;    // the crossing number counter
	
	int count = O2PathNumberOfElements(self);
	int i = 0;
	O2Point startPoint = O2PointZero, currentPoint = O2PointZero, toPoint = O2PointZero;
	O2Point *points = self->_points;
	uint8_t *element = self->_elements;
	for (i = 0; i < count; ++i) {
		switch (*element++) {
			case kO2PathElementMoveToPoint:
				startPoint = currentPoint = points[0];
				points++;
				break;
			case kO2PathElementAddLineToPoint:
				toPoint = points[0];
				if (evenOdd) {
					cn += _countEvenOddCrossingOfPointAndLine(point, currentPoint, toPoint);
				} else {
					cn += _countWindingAroundPointByLine(point, currentPoint, toPoint);
				}
				currentPoint = toPoint;
				points++;
				break;

			case kO2PathElementAddQuadCurveToPoint:
			{
				toPoint = points[1];
				CGPoint control = points[0];

				// Calc the equivalent Cubic curve control points
				CGPoint control1;
				control1.x = currentPoint.x/3.f + 2.f*control.x/3.f;
				control1.y = currentPoint.y/3.f + 2.f*control.y/3.f;
				CGPoint control2;
				control2.x = toPoint.x/3.f + 2.f*control.x/3.f;
				control2.y = toPoint.y/3.f + 2.f*control.y/3.f;
				
				// And carry on
				cn += _countYCrossingFromPointToCurve(point, currentPoint, toPoint, control1, control2, evenOdd);
				currentPoint = toPoint;
				points+=2;
			}
				break;
				
			case kO2PathElementAddCurveToPoint:
				toPoint = points[2];
				cn += _countYCrossingFromPointToCurve(point, currentPoint, toPoint, points[0], points[1], evenOdd);
				currentPoint = toPoint;
				points+=3;
				break;

			case kO2PathElementCloseSubpath:
				toPoint = startPoint;
				if (evenOdd) {
					cn += _countEvenOddCrossingOfPointAndLine(point, currentPoint, toPoint);
				} else {
					cn += _countWindingAroundPointByLine(point, currentPoint, toPoint);
				}
				currentPoint = startPoint;
				break;
		}
	}
	if (evenOdd) {
		return (cn&1);    // 0 if even (out), and 1 if odd (in)
	} else {
		return cn != 0;   // non-zero means we're inside
	}
}

O2Rect O2PathGetBoundingBox(O2PathRef self) {
   O2Rect result;
   int i;
   
   if(self->_numberOfPoints==0)
    return O2RectZero;
   
   result.origin=self->_points[0];
   result.size=O2SizeMake(0,0);
   for(i=1;i<self->_numberOfPoints;i++){
    O2Point point=self->_points[i];
    
    if(point.x>O2RectGetMaxX(result))
     result.size.width=point.x-O2RectGetMinX(result);
    else if(point.x<result.origin.x){
     result.size.width=O2RectGetMaxX(result)-point.x;
     result.origin.x=point.x;
    }
    
    if(point.y>O2RectGetMaxY(result))
     result.size.height=point.y-O2RectGetMinY(result);
    else if(point.y<result.origin.y){
     result.size.height=O2RectGetMaxY(result)-point.y;
     result.origin.y=point.y;
    }
   }   

   return result;
}

void O2PathApply(O2PathRef self,void *info,O2PathApplierFunction function) {
   int           i,pointIndex=0;
   O2PathElement element;
   
   for(i=0;i<self->_numberOfElements;i++){
    element.type=self->_elements[i];
    element.points=self->_points+pointIndex;

    switch(element.type){
    
     case kO2PathElementMoveToPoint:
      pointIndex+=1;
      break;
       
     case kO2PathElementAddLineToPoint:
      pointIndex+=1;
      break;

     case kO2PathElementAddCurveToPoint:
      pointIndex+=3;
      break;

     case kO2PathElementAddQuadCurveToPoint:
      pointIndex+=2;
      break;

     case kO2PathElementCloseSubpath:
      pointIndex+=0;
      break;
     }
    if(function!=NULL) // O2 will ignore function if NULL
     function(info,&element);
   }
}

@end

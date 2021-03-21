/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2MutablePath.h>
#import <Onyx2D/O2Path.h>
#import <Onyx2D/O2Exceptions.h>
#import <math.h>

// ellipse to 4 spline bezier, http://www.tinaja.com/glib/ellipse4.pdf
void O2MutablePathEllipseToBezier(O2Point *cp,float x,float y,float xrad,float yrad){
   float magic=0.551784;
   float xmag=xrad*magic;
   float ymag=yrad*magic;
   int   i=0;

	cp[i++]=O2PointMake(-xrad,0);
	
	cp[i++]=O2PointMake(-xrad,-ymag);
	cp[i++]=O2PointMake(-xmag,-yrad);
	cp[i++]=O2PointMake(0,-yrad);
	
	cp[i++]=O2PointMake(xmag,-yrad);
	cp[i++]=O2PointMake(xrad,-ymag);
	cp[i++]=O2PointMake(xrad,0);
	
	cp[i++]=O2PointMake(xrad,ymag);
	cp[i++]=O2PointMake(xmag,yrad);
	cp[i++]=O2PointMake(0,yrad);
	
	cp[i++]=O2PointMake(-xmag,yrad);
	cp[i++]=O2PointMake(-xrad,ymag);
	cp[i++]=O2PointMake(-xrad,0);
	
	for(i=0;i<13;i++){
		cp[i].x+=x;
		cp[i].y+=y;
	}
}

@implementation O2MutablePath

// Bezier and arc to bezier algorithms from: Windows Graphics Programming by Feng Yuan
#if 0
static void bezier(O2GState *self,double x1,double y1,double x2, double y2,double x3,double y3,double x4,double y4){
   // Ax+By+C=0 is the line (x1,y1) (x4,y4);
   double A=y4-y1;
   double B=x1-x4;
   double C=y1*(x4-x1)-x1*(y4-y1);
   double AB=A*A+B*B;

   if((A*x2+B*y2+C)*(A*x2+B*y2+C)<AB &&
      (A*x3+B*y3+C)*(A*x3+B*y3+C)<AB){
    [self moveToPoint:x1:y1];
    [self addLineToPoint:x4:y4];
    return;
   }
   else {
    double x12=x1+x2;
    double y12=y1+y2;
    double x23=x2+x3;
    double y23=y2+y3;
    double x34=x3+x4;
    double y34=y3+y4;
    double x1223=x12+x23;
    double y1223=y12+y23;
    double x2334=x23+x34;
    double y2334=y23+y34;
    double x=x1223+x2334;
    double y=y1223+y2334;

    bezier(self,x1,y1,x12/2,y12/2,x1223/4,y1223/4,x/8,y/8);
    bezier(self,x/8,y/8,x2334/4,y2334/4,x34/2,y34/2,x4,y4);
   }
}

#endif

O2MutablePathRef O2PathCreateMutable(void) {
   return [[O2MutablePath allocWithZone:NULL] initWithOperators:NULL numberOfElements:0 points:NULL numberOfPoints:0];
}

-initWithOperators:(unsigned char *)elements numberOfElements:(unsigned)numberOfElements points:(O2Point *)points numberOfPoints:(unsigned)numberOfPoints {
   O2PathInitWithOperators(self,elements,numberOfElements,points,numberOfPoints);
   _capacityOfElements=numberOfElements;
   _capacityOfPoints=numberOfPoints;
   return self;
}

-copyWithZone:(NSZone *)zone {
   return O2PathInitWithOperators([O2Path allocWithZone:zone],_elements,_numberOfElements,_points,_numberOfPoints);
}

void O2PathReset(O2MutablePathRef self) {
   self->_numberOfElements=0;
   self->_numberOfPoints=0;
}

static inline void expandOperatorCapacity(O2MutablePath *self,unsigned delta){
   if(self->_numberOfElements+delta>self->_capacityOfElements){
    self->_capacityOfElements=MAX(1,self->_capacityOfElements);
    
    while(self->_numberOfElements+delta>self->_capacityOfElements)
     self->_capacityOfElements*=2;
     
    self->_elements=NSZoneRealloc(NULL,self->_elements,self->_capacityOfElements);
   }
}

static inline void expandPointCapacity(O2MutablePath *self,unsigned delta){
   if(self->_numberOfPoints+delta>self->_capacityOfPoints){
    self->_capacityOfPoints=MAX(1,self->_capacityOfPoints);
    
    while(self->_numberOfPoints+delta>self->_capacityOfPoints)
     self->_capacityOfPoints*=2;
     
    self->_points=NSZoneRealloc(NULL,self->_points,self->_capacityOfPoints*sizeof(O2Point));
   }
}

void O2PathMoveToPoint(O2MutablePathRef self,const O2AffineTransform *matrix,O2Float x,O2Float y) {
   O2Point point=O2PointMake(x,y);
   
   if(matrix!=NULL)
    point=O2PointApplyAffineTransform(point,*matrix);

   expandOperatorCapacity(self,1);
   expandPointCapacity(self,1);
   self->_elements[self->_numberOfElements++]=kO2PathElementMoveToPoint;
   self->_points[self->_numberOfPoints++]=point;
}

void O2PathAddLineToPoint(O2MutablePathRef self,const O2AffineTransform *matrix,O2Float x,O2Float y) {
   O2Point point=O2PointMake(x,y);

   if(matrix!=NULL)
    point=O2PointApplyAffineTransform(point,*matrix);

   expandOperatorCapacity(self,1);
   expandPointCapacity(self,1);
   self->_elements[self->_numberOfElements++]=kO2PathElementAddLineToPoint;
   self->_points[self->_numberOfPoints++]=point;
}

void O2PathAddCurveToPoint(O2MutablePathRef self,const O2AffineTransform *matrix,O2Float cp1x,O2Float cp1y,O2Float cp2x,O2Float cp2y,O2Float x,O2Float y) {
   O2Point cp1=O2PointMake(cp1x,cp1y);
   O2Point cp2=O2PointMake(cp2x,cp2y);
   O2Point endPoint=O2PointMake(x,y);
   
   if(matrix!=NULL){
    cp1=O2PointApplyAffineTransform(cp1,*matrix);
    cp2=O2PointApplyAffineTransform(cp2,*matrix);
    endPoint=O2PointApplyAffineTransform(endPoint,*matrix);
   }

   expandOperatorCapacity(self,1);
   expandPointCapacity(self,3);
   self->_elements[self->_numberOfElements++]=kO2PathElementAddCurveToPoint;   
   self->_points[self->_numberOfPoints++]=cp1;
   self->_points[self->_numberOfPoints++]=cp2;
   self->_points[self->_numberOfPoints++]=endPoint;
}

void O2PathAddQuadCurveToPoint(O2MutablePathRef self,const O2AffineTransform *matrix,O2Float cpx,O2Float cpy,O2Float x,O2Float y) {
   O2Point cp1=O2PointMake(cpx,cpy);
   O2Point endPoint=O2PointMake(x,y);

   if(matrix!=NULL){
    cp1=O2PointApplyAffineTransform(cp1,*matrix);
    endPoint=O2PointApplyAffineTransform(endPoint,*matrix);
   }

   expandOperatorCapacity(self,1);
   expandPointCapacity(self,2);
   self->_elements[self->_numberOfElements++]=kO2PathElementAddQuadCurveToPoint;   
   self->_points[self->_numberOfPoints++]=cp1;
   self->_points[self->_numberOfPoints++]=endPoint;
}

void O2PathCloseSubpath(O2MutablePathRef self) {
   expandOperatorCapacity(self,1);
   self->_elements[self->_numberOfElements++]=kO2PathElementCloseSubpath;   
}

void O2PathAddLines(O2MutablePathRef self,const O2AffineTransform *matrix,const O2Point *points,size_t count) {
   int i;
   
   if(count==0)
    return;
    
   O2PathMoveToPoint(self,matrix,points[0].x,points[0].y);
   for(i=1;i<count;i++)
    O2PathAddLineToPoint(self,matrix,points[i].x,points[i].y);
}

void O2PathAddRect(O2MutablePathRef self,const O2AffineTransform *matrix,O2Rect rect) {
// The line order is correct per documentation, do not change.
   O2PathMoveToPoint(self,matrix,O2RectGetMinX(rect),O2RectGetMinY(rect));
   O2PathAddLineToPoint(self,matrix,O2RectGetMaxX(rect),O2RectGetMinY(rect));
   O2PathAddLineToPoint(self,matrix,O2RectGetMaxX(rect),O2RectGetMaxY(rect));
   O2PathAddLineToPoint(self,matrix,O2RectGetMinX(rect),O2RectGetMaxY(rect));
   O2PathCloseSubpath(self);
}

void O2PathAddRects(O2MutablePathRef self,const O2AffineTransform *matrix,const O2Rect *rects,size_t count) {
   int i;
   
   for(i=0;i<count;i++)
    O2PathAddRect(self,matrix,rects[i]);
}

void O2PathAddArc(O2MutablePathRef self,const O2AffineTransform *matrix,O2Float x,O2Float y,O2Float radius,O2Float startRadian,O2Float endRadian,BOOL clockwise) {
   startRadian=fmod(startRadian,M_PI*2);
   endRadian=fmod(endRadian,M_PI*2);

   if(clockwise){
    float tmp=startRadian;
    startRadian=endRadian;
    endRadian=tmp;
   }

   if(endRadian<startRadian){
    endRadian+=M_PI*2;
   }
   
   float   radiusx=radius,radiusy=radius;
   double  remainder=ABS(endRadian-startRadian);
   double  delta=M_PI_2; // 90 degrees
   int     i;
   O2Point points[4*((int)ceil(remainder/delta)+1)];
   int     pointsIndex=0;
   
   for(;remainder>0;startRadian+=delta,remainder-=delta){
    double  sweepangle=(remainder>delta)?delta:remainder;
    double  XY[8];
    double  B=sin(sweepangle/2);
    double  C=cos(sweepangle/2);
    double  A=1-C;
    double  X=A*4/3;
    double  Y=B-X*(1-A)/B;
    double  s=sin(startRadian+sweepangle/2);
    double  c=cos(startRadian+sweepangle/2);

    XY[0]= C;
    XY[1]=-B;
    XY[2]= C+X;
    XY[3]=-Y;
    XY[4]= C+X;
    XY[5]= Y;
    XY[6]= C;
    XY[7]= B;

    for(i=0;i<4;i++){
     points[pointsIndex].x=x+(XY[i*2]*c-XY[i*2+1]*s)*radiusx;
     points[pointsIndex].y=y+(XY[i*2]*s+XY[i*2+1]*c)*radiusy;
     pointsIndex++;
    }
   }
   

   if(clockwise){
    // just reverse points
    for(i=0;i<pointsIndex/2;i++){
     O2Point tmp;
     
     tmp=points[i];
     points[i]=points[(pointsIndex-1)-i];
     points[(pointsIndex-1)-i]=tmp;
    }     
   }
   
   if(pointsIndex>0){
     if(O2PathIsEmpty(self)) {
      O2PathMoveToPoint(self,matrix,points[0].x,points[0].y);
     } else {
      O2PathAddLineToPoint(self,matrix,points[0].x,points[0].y);
     }
    }
   
   for(i=0;i<pointsIndex;i+=4){
    O2PathAddCurveToPoint(self,matrix,points[i+1].x,points[i+1].y,points[i+2].x,points[i+2].y,points[i+3].x,points[i+3].y);
   }
}

void O2PathAddArcToPoint(O2MutablePathRef self,const O2AffineTransform *matrix,O2Float tx1,O2Float ty1,O2Float tx2,O2Float ty2,O2Float radius) {
#if 0
// ignores arc and draws a sharp corner
   O2PathAddLineToPoint(self,matrix,tx1,ty1);
#else
   if(self->_numberOfPoints==0)
    return;
    
   O2Point start=self->_points[self->_numberOfPoints-1];
   O2Point mid=O2PointMake(tx1,ty1);
   O2Point end=O2PointMake(tx2,ty2);
   
   if(matrix!=NULL){
// If a matrix is specified, either the start point needs to be transformed, or the arc needs to be
// transformed. Since we really want the result curve  transformed to get non-uniform scale/skew results to come
// out correctly, we just back-transform the start point. But the assumption here is that the start point was transformed
// using the same matrix as is being passed in, otherwise we have to track the last point and the matrix that goes with it
// which doesn't seem tidy. Further testing required for this case.

    start=O2PointApplyAffineTransform(start,O2AffineTransformInvert(*matrix));
   }
   
   double x1=start.x-mid.x;
   double y1=start.y-mid.y;
   double x2=end.x-mid.x;
   double y2=end.y-mid.y;
   double n1=sqrt(x1*x1+y1*y1);
   double n2=sqrt(x2*x2+y2*y2);
   
   float startAngle=acos(x1/n1);
   if(y1<0)
    startAngle=M_PI*2-startAngle;
    
   float endAngle=acos(x2/n2);
   if(y2<0) 
    endAngle=M_PI*2-endAngle;
    
   float angleBetweenLines = (endAngle-startAngle);
   int clockwise=1;
   float clockwiseRotate=0;
   
   if(angleBetweenLines<0)
    angleBetweenLines=M_PI*2+angleBetweenLines;

   CGFloat arcAngle=M_PI-angleBetweenLines;

   if(angleBetweenLines>M_PI){
    arcAngle=M_PI*2-angleBetweenLines;
    clockwise=0;
    clockwiseRotate=M_PI;
   }

   /* The triangle formed by the center of the arc, the midpoint and where the radius and tangent line meet
   
      Using the law of sines we can figure the other two sides of the triangle using the radius and the angles.
      
      The radius meets the tangent at 90 degrees.
      The tangent meets the mid->center line at half the angle between the tangents
      The radius meets the mid->center line at 180 - the other angles
    */
   CGFloat angleOfRadiusAndTangent=M_PI/2;
   CGFloat angleOfTangentAndLineToCenter=angleBetweenLines/2.0;
   
   CGFloat midToCenterLength=(radius*sin(angleOfRadiusAndTangent))/sin(angleOfTangentAndLineToCenter);
   
   CGPoint center;
   center.x=cos(startAngle+angleBetweenLines/2+clockwiseRotate)*midToCenterLength;
   center.y=sin(startAngle+angleBetweenLines/2+clockwiseRotate)*midToCenterLength;
   center.x+=mid.x;
   center.y+=mid.y;

   CGFloat arcStartAngle;
   CGFloat arcEndAngle;

   if(clockwise){
    arcStartAngle=(M_PI*2-(M_PI/2-startAngle));
    arcEndAngle=(arcStartAngle-arcAngle);
   }
   else {
    arcStartAngle=(M_PI*2-(M_PI/2-startAngle))+clockwiseRotate;
    arcEndAngle=(arcStartAngle-arcAngle)+clockwiseRotate;
   }
      
   O2PathAddArc(self,matrix,center.x,center.y,radius,arcStartAngle,arcEndAngle,clockwise);
#endif

}

void O2PathAddEllipseInRect(O2MutablePathRef self,const O2AffineTransform *matrix,O2Rect rect) {
   float             xradius=rect.size.width/2;
   float             yradius=rect.size.height/2;
   float             x=rect.origin.x+xradius;
   float             y=rect.origin.y+yradius;
   O2Point           cp[13];
   int               i;
    
   O2MutablePathEllipseToBezier(cp,x,y,xradius,yradius);
    
   O2PathMoveToPoint(self,matrix,cp[0].x,cp[0].y);
   for(i=1;i<13;i+=3)
    O2PathAddCurveToPoint(self,matrix,cp[i].x,cp[i].y,cp[i+1].x,cp[i+1].y,cp[i+2].x,cp[i+2].y);
   O2PathCloseSubpath(self);
}

void O2PathAddPath(O2MutablePathRef self,const O2AffineTransform *matrix,O2PathRef path) {
   unsigned             opsCount=O2PathNumberOfElements(path);
   const unsigned char *ops=O2PathElements(path);
   unsigned             pointCount=O2PathNumberOfPoints(path);
   const O2Point       *points=O2PathPoints(path);
   unsigned             i;
   
   expandOperatorCapacity(self,opsCount);
   expandPointCapacity(self,pointCount);
   
   for(i=0;i<opsCount;i++)
    self->_elements[self->_numberOfElements++]=ops[i];
    
   if(matrix==NULL){
    for(i=0;i<pointCount;i++)
     self->_points[self->_numberOfPoints++]=points[i];
   }
   else {    
    for(i=0;i<pointCount;i++)
     self->_points[self->_numberOfPoints++]=O2PointApplyAffineTransform(points[i],*matrix);
   }
}

void O2PathApplyTransform(O2MutablePathRef self,const O2AffineTransform matrix) {
   int i;
   
   for(i=0;i<self->_numberOfPoints;i++)
    self->_points[i]=O2PointApplyAffineTransform(self->_points[i],matrix);
}

@end

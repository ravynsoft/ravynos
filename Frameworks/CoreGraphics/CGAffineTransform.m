/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreGraphics/CGAffineTransform.h>

const CGAffineTransform CGAffineTransformIdentity={1,0,0,1,0,0};

bool CGAffineTransformIsIdentity(CGAffineTransform xform)
{
    return xform.a == 1 && xform.b == 0 && xform.c == 0 && xform.d == 1 && xform.tx == 0 && xform.ty == 0;
}

CGAffineTransform CGAffineTransformMake(CGFloat a,CGFloat b,CGFloat c,CGFloat d,CGFloat tx,CGFloat ty){
   CGAffineTransform xform={a,b,c,d,tx,ty};
   return xform;
}

CGAffineTransform CGAffineTransformMakeRotation(CGFloat radians){
   CGAffineTransform xform={cos(radians),sin(radians),-sin(radians),cos(radians),0,0};
   return xform;
}

CGAffineTransform CGAffineTransformMakeScale(CGFloat scalex,CGFloat scaley){
   CGAffineTransform xform={scalex,0,0,scaley,0,0};
   return xform;
}

CGAffineTransform CGAffineTransformMakeTranslation(CGFloat tx,CGFloat ty){
   CGAffineTransform xform={1,0,0,1,tx,ty};
   return xform;
}

CGAffineTransform CGAffineTransformConcat(CGAffineTransform xform,CGAffineTransform append){
   CGAffineTransform result;

   result.a=xform.a*append.a+xform.b*append.c;
   result.b=xform.a*append.b+xform.b*append.d;
   result.c=xform.c*append.a+xform.d*append.c;
   result.d=xform.c*append.b+xform.d*append.d;
   result.tx=xform.tx*append.a+xform.ty*append.c+append.tx;
   result.ty=xform.tx*append.b+xform.ty*append.d+append.ty;

   return result;
}

CGAffineTransform CGAffineTransformInvert(CGAffineTransform xform){
   CGAffineTransform result;
   CGFloat determinant;

   determinant=xform.a*xform.d-xform.c*xform.b;
   if(determinant==0){
    return xform;
   }

   result.a=xform.d/determinant;
   result.b=-xform.b/determinant;
   result.c=-xform.c/determinant;
   result.d=xform.a/determinant;
   result.tx=(-xform.d*xform.tx+xform.c*xform.ty)/determinant;
   result.ty=(xform.b*xform.tx-xform.a*xform.ty)/determinant;

   return result;
}

CGAffineTransform CGAffineTransformRotate(CGAffineTransform xform,CGFloat radians){
   CGAffineTransform rotate=CGAffineTransformMakeRotation(radians);
   return CGAffineTransformConcat(rotate,xform);
}

CGAffineTransform CGAffineTransformScale(CGAffineTransform xform,CGFloat scalex,CGFloat scaley){
   CGAffineTransform scale=CGAffineTransformMakeScale(scalex,scaley);
   return CGAffineTransformConcat(scale,xform);
}

CGAffineTransform CGAffineTransformTranslate(CGAffineTransform xform,CGFloat tx,CGFloat ty){
   CGAffineTransform translate=CGAffineTransformMakeTranslation(tx,ty);
   return CGAffineTransformConcat(translate,xform);
}

CGPoint CGPointApplyAffineTransform(CGPoint point,CGAffineTransform xform){
    CGPoint p;

    p.x=xform.a*point.x+xform.c*point.y+xform.tx;
    p.y=xform.b*point.x+xform.d*point.y+xform.ty;

    return p;
}

CGSize CGSizeApplyAffineTransform(CGSize size,CGAffineTransform xform){
    CGSize s;

    s.width=xform.a*size.width+xform.c*size.height;
    s.height=xform.b*size.width+xform.d*size.height;

    return s;
}

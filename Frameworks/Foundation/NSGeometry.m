/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
   Copyright (c) 2022 Zoe Knox <zoe@pixin.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSGeometry.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSStringFormatter.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSRaise.h>
#include <math.h>
#include <stdio.h>
#include <locale.h>

const NSPoint NSZeroPoint={0,0};

BOOL NSEqualPoints(NSPoint point0,NSPoint point1) {
   return ((point0.x==point1.x) && (point0.y==point1.y))?YES:NO;
}

NSString *NSStringFromPoint(NSPoint point) {
    struct lconv *lconv = localeconv();
    return [NSString stringWithFormat:@"{%g%s %g}", point.x, lconv->thousands_sep, point.y];
}


NSPoint NSPointFromString(NSString *string)
{
    NSPoint result = NSZeroPoint;
    
    if (string != nil) {
        struct lconv *lconv = localeconv();
        NSString *fmt = [NSString stringWithFormat:@"{%s%s %s}",
            CGFLOAT_SCAN, lconv->thousands_sep, CGFLOAT_SCAN];
        sscanf([string UTF8String], [fmt UTF8String], &result.x, &result.y);
        [fmt release];
    }
    return result;
}


//
const NSSize NSZeroSize={0,0};

BOOL NSEqualSizes(NSSize size0,NSSize size1) {
   return (size0.width==size1.width) && (size0.height==size1.height);
}

NSString *NSStringFromSize(NSSize size) {
    struct lconv *lconv = localeconv();
    return [NSString stringWithFormat:@"{%g%s %g}", size.width, lconv->thousands_sep, size.height];
}


NSSize NSSizeFromString(NSString *string)
{
    NSSize result = NSZeroSize;
    
    if (string != nil) {
        struct lconv *lconv = localeconv();
        NSString *fmt = [NSString stringWithFormat:@"{%s%s %s}",
            CGFLOAT_SCAN, lconv->thousands_sep, CGFLOAT_SCAN];
        sscanf([string UTF8String], [fmt UTF8String], &result.width, &result.height);
        [fmt release];
    }
    return result;
}


//
const NSRect NSZeroRect={{0,0},{0,0}};

BOOL NSEqualRects(NSRect rect0,NSRect rect1) {
   return NSEqualPoints(rect0.origin,rect1.origin) && NSEqualSizes(rect0.size,rect1.size);
}

BOOL NSIsEmptyRect(NSRect rect) {
   if(rect.size.width>0 && rect.size.height>0)
    return NO;

   return YES;
}


NSRect NSInsetRect(NSRect rect,CGFloat dx,CGFloat dy) {
   rect.origin.x+=dx;
   rect.origin.y+=dy;
   rect.size.width-=dx*2;
   rect.size.height-=dy*2;
   return rect;
}

NSRect NSOffsetRect(NSRect rect,CGFloat dx,CGFloat dy) {
   rect.origin.x+=dx; 
   rect.origin.y+=dy; 
   return rect; 

}

NSRect NSIntegralRect(NSRect rect) {
   if (!NSIsEmptyRect(rect)) { 
      rect.origin.x = floor(rect.origin.x); 
      rect.origin.y = floor(rect.origin.y); 
      rect.size.width = ceil(rect.size.width); 
      rect.size.height = ceil(rect.size.height); 
   } 
   return rect; 

}

NSRect NSUnionRect(NSRect rect0,NSRect rect1) {
   if(NSIsEmptyRect(rect0))
    if(NSIsEmptyRect(rect1))
     return NSZeroRect;
    else
     return rect1;
   else
    if(NSIsEmptyRect(rect1))
     return rect0;
    else {
     NSRect result;

     result.origin.x=MIN(rect0.origin.x,rect1.origin.x);
     result.origin.y=MIN(rect0.origin.y,rect1.origin.y);
     result.size.width=MAX(NSMaxX(rect0),NSMaxX(rect1))-result.origin.x;
     result.size.height=MAX(NSMaxY(rect0),NSMaxY(rect1))-result.origin.y;

     return result;
    }
}

NSRect NSIntersectionRect(NSRect rect0,NSRect rect1) {
   NSRect result;

   if(NSMaxX(rect0)<=NSMinX(rect1) || NSMinX(rect0)>=NSMaxX(rect1) ||
      NSMaxY(rect0)<=NSMinY(rect1) || NSMinY(rect0)>=NSMaxY(rect1))
    return NSZeroRect;

   result.origin.x=MAX(NSMinX(rect0),NSMinX(rect1));
   result.origin.y=MAX(NSMinY(rect0),NSMinY(rect1));
   result.size.width=MIN(NSMaxX(rect0),NSMaxX(rect1))-result.origin.x;
   result.size.height=MIN(NSMaxY(rect0),NSMaxY(rect1))-result.origin.y;

   return result;
}


void NSDivideRect(NSRect rect,NSRect *slicep,NSRect *remainderp,CGFloat amount,NSRectEdge edge) {
   *slicep = rect;
   *remainderp = rect;
   CGFloat temp;

   switch(edge) {
    case NSMinXEdge:
	 slicep->size.width = amount;
	 remainderp->size.width -= amount;
	 remainderp->origin.x += amount;
	 break;
    case NSMinYEdge:
     slicep->size.height = amount;
	 remainderp->size.height -= amount;
	 remainderp->origin.y += amount;
	 break;
    case NSMaxXEdge:
	 temp = slicep->size.width - amount;
	 remainderp->size.width = temp;
	 slicep->size.width = amount;
	 slicep->origin.x += temp;
	 break;
    case NSMaxYEdge:
	 temp = slicep->size.height - amount;
	 remainderp->size.height = temp;
	 slicep->size.height = amount;
	 slicep->origin.y += temp;
	 break;
   }
}


BOOL NSContainsRect(NSRect rect0,NSRect rect1) {
  return ((NSMinX(rect0) <=NSMinX(rect1)) && (NSMinY(rect0) <=NSMinY(rect1)) &&
          (NSMaxX(rect0) >=NSMaxX(rect1)) && (NSMaxY(rect0) >=NSMaxY(rect1)))?YES:NO;
}

BOOL NSIntersectsRect(NSRect rect0,NSRect rect1) {
   if(NSMaxX(rect0)<=NSMinX(rect1) || NSMinX(rect0)>=NSMaxX(rect1) ||
      NSMaxY(rect0)<=NSMinY(rect1) || NSMinY(rect0)>=NSMaxY(rect1))
    return NO;

  return YES;
}


NSString *NSStringFromRect(NSRect rect) {
    struct lconv *lconv = localeconv();
    return [NSString stringWithFormat:@"{{%g%s %g}%s {%g%s %g}}", rect.origin.x, lconv->thousands_sep, rect.origin.y, lconv->thousands_sep, rect.size.width, lconv->thousands_sep, rect.size.height];
}


NSRect NSRectFromString(NSString *string)
{
    NSRect result = NSZeroRect;
    
    if (string != nil) {
        struct lconv *lconv = localeconv();
        NSString *fmt = [NSString stringWithFormat:@"{{%s%s %s}%s {%s%s %s}}",
            CGFLOAT_SCAN, lconv->thousands_sep, CGFLOAT_SCAN, lconv->thousands_sep,
            CGFLOAT_SCAN, lconv->thousands_sep, CGFLOAT_SCAN];
        sscanf([string UTF8String], [fmt UTF8String], &result.origin.x, &result.origin.y, &result.size.width, &result.size.height);
        [fmt release];
    }
    return result;
}


BOOL NSPointInRect(NSPoint point,NSRect rect) {
   return (point.x>=NSMinX(rect) && point.x<=NSMaxX(rect)) &&
          (point.y>=NSMinY(rect) && point.y<=NSMaxY(rect));
}

BOOL NSMouseInRect(NSPoint point,NSRect rect,BOOL flipped) {
  if(flipped)
    return (point.x>=NSMinX(rect) && point.x<NSMaxX(rect) &&
            point.y>=NSMinY(rect) && point.y<NSMaxY(rect));
  else
    return (point.x>=NSMinX(rect) && point.x<NSMaxX(rect) &&
            point.y>NSMinY(rect)  && point.y<=NSMaxY(rect));
}


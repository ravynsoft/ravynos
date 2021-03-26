/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSInterfaceGraphics.h"
#import <AppKit/NSGraphicsContextFunctions.h>
#import <ApplicationServices/ApplicationServices.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSView.h>

static NSRect NSInterfaceDrawRects(NSRect boundsRect,NSRect clipRect,const NSRect *sides,NSColor **colors,int count) {
   CGContextRef graphicsPort=NSCurrentGraphicsPort();

   CGContextSaveGState(graphicsPort);
   CGContextClipToRect(graphicsPort,clipRect);
   NSRectFillListWithColors(sides,colors,count);
   CGContextRestoreGState(graphicsPort);

   return boundsRect;
}

void NSInterfaceDrawButton(NSRect rect,NSRect clipRect) {
   NSDrawButton(rect,clipRect);
}

void NSInterfaceDrawHighlightedButton(NSRect rect,NSRect clipRect) {
   NSRect   rects[7];
   NSColor *colors[7];
   int      i;

   for(i=0;i<7;i++)
    rects[i]=rect;

   if([[NSGraphicsContext currentContext] isFlipped]){
    colors[0]=[NSColor blackColor];
    rects[0].origin.y+=rect.size.height-1;
    rects[0].size.height=1;

    colors[1]=[NSColor blackColor];
    rects[1].origin.x+=rect.size.width-1;
    rects[1].size.width=1;

    colors[2]=[NSColor darkGrayColor];
    rects[2].origin.x+=1;
    rects[2].size.width-=2;
    rects[2].origin.y+=rect.size.height-2;
    rects[2].size.height=1;

    colors[3]=[NSColor darkGrayColor];
    rects[3].origin.x+=rect.size.width-2;
    rects[3].origin.y+=1;
    rects[3].size.width=1;
    rects[3].size.height-=2;

    colors[4]=[NSColor whiteColor];
    rects[4].size.height=1;
    rects[4].size.width-=1;

    colors[5]=[NSColor whiteColor];
    rects[5].size.width=1;
    rects[5].size.height-=1;
 
    colors[6]=[NSColor highlightColor];
    rects[6].origin.x+=1;
    rects[6].origin.y+=1;
    rects[6].size.width-=3;
    rects[6].size.height-=3;
   }
   else {
    colors[0]=[NSColor blackColor];
    rects[0].size.height=1;

    colors[1]=[NSColor blackColor];
    rects[1].origin.x+=rect.size.width-1;
    rects[1].size.width=1;

    colors[2]=[NSColor darkGrayColor];
    rects[2].origin.x+=1;
    rects[2].origin.y+=1;
    rects[2].size.width-=2;
    rects[2].size.height=1;

    colors[3]=[NSColor darkGrayColor];
    rects[3].origin.x+=rect.size.width-2;
    rects[3].origin.y+=2;
    rects[3].size.width=1;
    rects[3].size.height-=2;

    colors[4]=[NSColor whiteColor];
    rects[4].origin.y+=1;
    rects[4].size.width=1;
    rects[4].size.height-=1;

    colors[5]=[NSColor whiteColor];
    rects[5].origin.y+=rect.size.height-1;
    rects[5].size.width-=1;
    rects[5].size.height=1;

    colors[6]=[NSColor highlightColor];
    rects[6].origin.x+=1;
    rects[6].origin.y+=2;
    rects[6].size.width-=3;
    rects[6].size.height-=3;
   }

   NSInterfaceDrawRects(rect,clipRect,rects,colors,7);
}

void NSInterfaceDrawDepressedButton(NSRect rect,NSRect clipRect) {
   NSDrawGrayBezel(rect,clipRect);
}

void NSInterfaceDrawDepressedHighlightedButton(NSRect rect,NSRect clipRect) {
}

void NSInterfaceDrawScrollerButton(NSRect rect,NSRect clipRect) {
   NSDrawButton(rect,clipRect);
}

void NSInterfaceDrawDepressedScrollerButton(NSRect rect,NSRect clipRect) {
   NSRect   rects[5];
   NSColor *colors[5];
   int      i;

   for(i=0;i<5;i++)
    rects[i]=rect;

   // center
   colors[0]=[NSColor controlColor];
   rects[0].origin.x+=1;
   rects[0].origin.y+=1;
   rects[0].size.width-=2;
   rects[0].size.height-=2;

   // top
   colors[1]=[NSColor controlShadowColor];
   rects[1].size.height=1;

   // right
   colors[2]=[NSColor controlShadowColor];
   rects[2].size.width=1;

   // left
   colors[3]=[NSColor controlShadowColor];
   rects[3].origin.x+=rect.size.width-1;
   rects[3].size.width=1;

   // bottom
   colors[4]=[NSColor controlShadowColor];
   rects[4].origin.y+=rect.size.height-1;
   rects[4].size.height=1;

   NSInterfaceDrawRects(rect,clipRect,rects,colors,5);
}

void NSInterfaceDrawBrowserHeader(NSRect rect,NSRect clipRect) {
   NSRect rects[3];
   NSColor *colors[3];
   int i;

   for (i = 0; i < 3; ++i)
       rects[i]=rect;

   if ([[NSGraphicsContext currentContext] isFlipped]) {
       colors[0] = [NSColor controlShadowColor];
       colors[1] = [NSColor whiteColor];
       rects[1].origin.x+=1;
       rects[1].origin.y+=1;
       rects[1].size.width-=1;
       rects[1].size.height-=1;
       colors[2] = [NSColor controlColor];
       rects[2].origin.x+=1;
       rects[2].origin.y+=1;
       rects[2].size.width-=2;
       rects[2].size.height-=2;
   }
   else {
       colors[0] = [NSColor controlShadowColor];
       colors[1] = [NSColor whiteColor];
       rects[1].origin.x+=1;
       rects[1].size.width-=1;
       rects[1].size.height-=1;
       colors[2] = [NSColor controlColor];
       rects[2].origin.x+=1;
       rects[2].origin.y+=1;
       rects[2].size.width-=2;
       rects[2].size.height-=2;
   }

   NSInterfaceDrawRects(rect,clipRect,rects,colors,3);
}

void NSInterfaceDrawProgressIndicatorBezel(NSRect rect,NSRect clipRect) {
   NSInterfaceDrawBrowserHeader(rect,clipRect);
}


void NSInterfaceDrawOutlineGrid(NSRect rect,CGContextRef context) {
/* Win32 has poor dashed line support...
 */
   if(rect.size.width<0)
    return;
   if(rect.size.height<0)
    return;
   {
    NSRect rects[(int)rect.size.width+2+(int)rect.size.height+2];
    int    count=0;
    int    x,y,maxx=NSMaxX(rect),maxy=NSMaxY(rect);
    BOOL   on=NO;

    for(x=rect.origin.x;x<maxx;x++,on=!on)
     if(on)
      rects[count++]=NSMakeRect(x,rect.origin.y,1,1);

    for(y=rect.origin.y;y<maxy;y++,on=!on)
     if(on)
      rects[count++]=NSMakeRect(maxx-1,y,1,1);

    CGContextFillRects(context,rects,count);
   }
}

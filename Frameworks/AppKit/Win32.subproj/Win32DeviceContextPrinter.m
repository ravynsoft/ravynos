/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/Win32DeviceContextPrinter.h>
#import <Foundation/NSString.h>

@implementation Win32DeviceContextPrinter

-(void)dealloc {
   DeleteDC(_dc);
   [super dealloc];
}

-(NSSize)pageSize {
	return [self paperRect].size;
}

-(void)beginPrintingWithDocumentName:(NSString *)name {
   DOCINFO info;

   info.cbSize=sizeof(DOCINFO);
   info.lpszDocName=[name cString];
   info.lpszOutput=NULL;
   info.lpszDatatype=NULL;
   info.fwType=0;

   if(StartDoc(_dc,&info)==SP_ERROR)
    return;

   return;
}

-(void)endPrinting {
   if(EndDoc(_dc)==SP_ERROR)
    NSLog(@"EndDoc failed");
}


-(void)beginPage {
   if(StartPage(_dc)==SP_ERROR)
    NSLog(@"StartPage failed");
}

-(void)endPage {
   if(EndPage(_dc)==SP_ERROR)
    NSLog(@"EndPage failed");
}

-(void)abortDocument {
   if(AbortDoc(_dc)==SP_ERROR)
    NSLog(@"AbortDoc failed");
}

-(NSSize)pixelsPerInch {
   float dpix=GetDeviceCaps(_dc,LOGPIXELSX);
   float dpiy=GetDeviceCaps(_dc,LOGPIXELSY);
   
   return NSMakeSize(dpix,dpiy);
}

-(NSRect)paperRect {
   NSSize dpi=[self pixelsPerInch];
   float width=GetDeviceCaps(_dc,PHYSICALWIDTH);
   float height=GetDeviceCaps(_dc,PHYSICALHEIGHT);
      
   return NSMakeRect(0,0,(width/dpi.width)*72,(height/dpi.height)*72);
}

-(NSRect)imageableRect {
   NSSize dpi=[self pixelsPerInch];
   NSRect result=NSZeroRect;
   float  offsetx=GetDeviceCaps(_dc,PHYSICALOFFSETX);
   float  offsety=GetDeviceCaps(_dc,PHYSICALOFFSETY);

   offsetx/=dpi.width;
   offsety/=dpi.height;
   
   offsetx*=72;
   offsety*=72;

	float width = GetDeviceCaps(_dc,HORZRES);
	float height = GetDeviceCaps(_dc,VERTRES);

	width/=dpi.width;
	height/=dpi.height;
	
	width*=72;
	height*=72;

	result.origin.x=offsetx;
	result.origin.y=offsety;
    result.size.width=width;
    result.size.height=height;

	return result;
}

#if 0
-(void)scalePage:(float)scalex:(float)scaley {
   HDC colorDC=_dc;
   int xmul=scalex*1000;
   int xdiv=1000;
   int ymul=scaley*1000;
   int ydiv=1000;

   int width=GetDeviceCaps(colorDC,HORZRES);
   int height=GetDeviceCaps(colorDC,VERTRES);

   SetWindowExtEx(colorDC,width,height,NULL);
   SetViewportExtEx(colorDC,width,height,NULL);

   ScaleWindowExtEx(colorDC,xdiv,xmul,ydiv,ymul,NULL);
}
#endif

@end

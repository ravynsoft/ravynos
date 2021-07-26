/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/Win32Cursor.h>

@implementation Win32Cursor

-initWithHCURSOR:(HCURSOR)handle {
   _handle=handle;
   _destroy=YES;
   return self;
}

-initWithName:(NSString *)name {
   LPCSTR idc=NULL;

   if([name isEqualToString:@"arrowCursor"])
    idc=IDC_ARROW;
   else if([name isEqualToString:@"closedHandCursor"])
    idc=IDC_HAND;
   else if([name isEqualToString:@"crosshairCursor"])
    idc=IDC_CROSS;
   else if([name isEqualToString:@"disappearingItemCursor"])
    idc=IDC_ARROW;
   else if([name isEqualToString:@"IBeamCursor"])
    idc=IDC_IBEAM;
   else if([name isEqualToString:@"openHandCursor"])
    idc=IDC_HAND;
   else if([name isEqualToString:@"pointingHandCursor"])
    idc=IDC_HAND;
   else if([name isEqualToString:@"resizeDownCursor"])
    idc=IDC_SIZENS;
   else if([name isEqualToString:@"resizeLeftCursor"])
    idc=IDC_SIZEWE;
   else if([name isEqualToString:@"resizeLeftRightCursor"])
    idc=IDC_SIZEWE;
   else if([name isEqualToString:@"resizeRightCursor"])
    idc=IDC_SIZEWE;
   else if([name isEqualToString:@"resizeUpCursor"])
    idc=IDC_SIZENS;
   else if([name isEqualToString:@"resizeUpDownCursor"])
    idc=IDC_SIZENS;

   _handle=LoadCursor(NULL,idc);
   return self;
}

-(void)dealloc {
   if(_destroy)
    DestroyIcon(_handle);
    
   [super dealloc];
}

-(HCURSOR)cursorHandle {
   return _handle;
}

@end

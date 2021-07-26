/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>

#import <AppKit/Win32DeviceContextWindow.h>
@interface Win32DeviceContextWindow (ThemeChange)
- (void)openTheme;
- (void)closeTheme;
@end

@implementation Win32DeviceContextWindow

// defined in NSGraphicsStyle_uxtheme.m
HANDLE openThemeData(HWND window,LPCWSTR classList);
void closeThemeData(HANDLE theme);

-initWithWindowHandle:(HWND)handle {
   [super initWithDC:GetDC(handle)];
   if (handle)
   {
      _handle=handle;
	   [self openTheme];
   }
   return self;
}

-(void)dealloc {
   [self closeTheme];
   ReleaseDC(_handle,_dc);
   [super dealloc];
}

-(HWND)windowHandle {
   return _handle;
}

-(HANDLE)theme:(int)uxthClassId {
   return _theme[uxthClassId];
}

- (void)themeChanged
{
	[self closeTheme];
	[self openTheme];
}


-(Win32DeviceContextWindow *)windowDeviceContext {
   return self;
}
#if DEBUG
#define THEME_CHECK(idx,class) if ((_theme[idx]   =openThemeData(_handle, L##class)) == NULL) { NSLog(@"%s theme data == NULL",class); }
#else
#define THEME_CHECK(idx,class) _theme[idx]   =openThemeData(_handle, L##class);
#endif

- (void)openTheme {
   THEME_CHECK(uxthBUTTON,"BUTTON");
   THEME_CHECK(uxthCOMBOBOX,"COMBOBOX");
   THEME_CHECK(uxthEDIT,"EDIT");
   THEME_CHECK(uxthHEADER,"HEADER");
   THEME_CHECK(uxthMENU,"MENU");
   THEME_CHECK(uxthPROGRESS,"PROGRESS");
   THEME_CHECK(uxthSCROLLBAR,"SCROLLBAR");
   THEME_CHECK(uxthSPIN,"SPIN");
   THEME_CHECK(uxthTAB,"TAB");
   THEME_CHECK(uxthTRACKBAR,"TRACKBAR");
   THEME_CHECK(uxthTREEVIEW,"TREEVIEW");
}

- (void)closeTheme {
   int i;
   for (i=0;i<uxthNumClasses;i++)
      if (_theme[i])
         closeThemeData(_theme[i]);
}

@end

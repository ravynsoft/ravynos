/** -*-ObjC-*- 
   Copyright (C) 2002 Free Software Foundation, Inc.

   Author: Frederic De Jaeger
   Date: Nov 2002
   
   This file is part of the GNU Objective C User interface library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#ifndef _NSOpenGL_h_INCLUDE
#define _NSOpenGL_h_INCLUDE
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>

@class NSView;

typedef enum {
  NSOpenGLPFAAllRenderers = 1,
  NSOpenGLPFADoubleBuffer = 5,
  NSOpenGLPFAStereo = 6,
  NSOpenGLPFAAuxBuffers = 7,
  NSOpenGLPFAColorSize = 8,
  NSOpenGLPFAAlphaSize = 11,
  NSOpenGLPFADepthSize = 12,
  NSOpenGLPFAStencilSize = 13,
  NSOpenGLPFAAccumSize = 14,
  NSOpenGLPFAMinimumPolicy = 51,
  NSOpenGLPFAMaximumPolicy = 52,
  NSOpenGLPFAOffScreen = 53,
  NSOpenGLPFAFullScreen = 54,
  NSOpenGLPFASampleBuffers = 55,        // 10.2
  NSOpenGLPFASamples = 56,              // 10.2
  NSOpenGLPFAAuxDepthStencil = 57,      // 10.2
  NSOpenGLPFAColorFloat = 58,           // 10.4
  NSOpenGLPFAMultisample = 59,          // 10.4
  NSOpenGLPFASupersample = 60,          // 10.4
  NSOpenGLPFASampleAlpha = 61,          // 10.4
  NSOpenGLPFARendererID = 70,
  NSOpenGLPFASingleRenderer = 71,
  NSOpenGLPFANoRecovery = 72,
  NSOpenGLPFAAccelerated = 73,
  NSOpenGLPFAClosestPolicy = 74,
  NSOpenGLPFARobust = 75,
  NSOpenGLPFABackingStore = 76,
  NSOpenGLPFAMPSafe = 78,
  NSOpenGLPFAWindow = 80,
  NSOpenGLPFAMultiScreen = 81,
  NSOpenGLPFACompliant = 83,
  NSOpenGLPFAScreenMask = 84,
  NSOpenGLPFAPixelBuffer = 90,          // 10.3
  NSOpenGLPFAAllowOfflineRenderers = 96,// 10.5
  NSOpenGLPFAVirtualScreenCount = 128   // 10.2
} NSOpenGLPixelFormatAttribute;

typedef enum {
  NSOpenGLCPSwapRectangle = 200,
  NSOpenGLCPSwapRectangleEnable = 201,
  NSOpenGLCPRasterizationEnable = 221,
  NSOpenGLCPSwapInterval = 222,
  NSOpenGLCPSurfaceOrder = 235,
  NSOpenGLCPSurfaceOpacity = 236,
  NSOpenGLCPStateValidation = 301
} NSOpenGLContextParameter;

typedef enum {
  NSOpenGLGOFormatCacheSize = 501,
  NSOpenGLGOClearFormatCache = 502,
  NSOpenGLGORetainRenderers = 503,
  NSOpenGLGOResetLibrary = 504
} NSOpenGLGlobalOption;

@interface NSOpenGLPixelFormat : NSObject <NSCoding>
{
}
- (void)getValues:(int *)vals 
     forAttribute:(NSOpenGLPixelFormatAttribute)attrib 
 forVirtualScreen:(int)screen;
- (id)initWithAttributes:(NSOpenGLPixelFormatAttribute *)attribs;
- (int)numberOfVirtualScreens;
@end

@interface NSOpenGLContext : NSObject
{
}

+ (void)clearCurrentContext;
+ (NSOpenGLContext *)currentContext;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void *)CGLContextObj;
#endif
- (void)clearDrawable;
- (void)copyAttributesFromContext:(NSOpenGLContext *)context 
			 withMask:(unsigned long)mask;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (void)createTexture:(unsigned long)target 
	     fromView:(NSView*)view 
       internalFormat:(unsigned long)format;
- (int)currentVirtualScreen;
#endif

- (void)flushBuffer;

- (void)getValues:(long *)vals 
     forParameter:(NSOpenGLContextParameter)param;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (id)initWithCGLContextObj:(void *)context;
#endif
- (id)initWithFormat:(NSOpenGLPixelFormat *)format 
	shareContext:(NSOpenGLContext *)share;


- (void)makeCurrentContext;

- (void)setCurrentVirtualScreen:(int)screen;

- (void)setFullScreen;

- (void)setOffScreen:(void *)baseaddr 
	       width:(long)width 
	      height:(long)height 
	    rowbytes:(long)rowbytes;

- (void)setValues:(const long *)vals 
     forParameter:(NSOpenGLContextParameter)param;

- (void)setView:(NSView *)view;

- (void)update;

- (NSView *)view;

@end



#endif

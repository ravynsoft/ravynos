/* -*- mode:ObjC -*-
   XGGLContext - backend implementation of NSOpenGLContext

   Copyright (C) 1998,2002 Free Software Foundation, Inc.

   Written by:  Frederic De Jaeger
   Date: Nov 2002
   
   This file is part of the GNU Objective C User Interface Library.

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

#include "config.h"

#ifdef HAVE_GLX

#include <Foundation/NSDebug.h>
#include <Foundation/NSException.h>
#include <Foundation/NSData.h>
#include <Foundation/NSDictionary.h>
#include <GNUstepGUI/GSDisplayServer.h>
#include "x11/XGServer.h"
#include "x11/XGOpenGL.h"
#include <X11/Xlib.h>
#include <math.h>

#ifdef XRENDER
#include <X11/extensions/Xrender.h>
#endif

@implementation XGGLPixelFormat

+ (int) glxMinorVersion
{
  static int cachedVersion = -1;

  if (cachedVersion == -1)
    {
      Display * display = [(XGServer *)GSCurrentServer() xDisplay];
      NSDictionary * attributes = [GSCurrentServer() attributes];
      NSString * sn = [attributes objectForKey: GSScreenNumber];
      
      // This is due to some OpenGL drivers reporting a lower overall GLX version than they actually implement.
      NSString *glxServerVersion = 
        [NSString stringWithFormat:@"%s", glXQueryServerString(display, [sn intValue], GLX_VERSION)];
      NSString *glxClientVersion =
        [NSString stringWithFormat:@"%s", glXGetClientString(display, GLX_VERSION)];
      float serverversion = [glxServerVersion floatValue];
      float clientversion = [glxClientVersion floatValue];
      float serverIntegerPart;
      float clientIntegerPart;
      float fracServer = modff(serverversion, &serverIntegerPart);
      float fracClient = modff(clientversion, &clientIntegerPart);
      
      if ( serverIntegerPart == 1.0f && clientIntegerPart == 1.0f )
        {
          fracServer = rintf(fracServer * 10.0f);
          fracClient = rintf(fracClient * 10.0f);
          
          NSDebugMLLog(@"GLX", @"server %f client %f", fracServer, fracClient);
          cachedVersion = (int)MIN(fracServer, fracClient);
        }
    }

  return cachedVersion;
}

// Works for some attributes only
- (void) getValues: (GLint *)vals 
      forAttribute: (NSOpenGLPixelFormatAttribute)attrib 
  forVirtualScreen: (GLint)screen
{
  int error;

  NSAssert((fbconfig != NULL || visualinfo != NULL) && configurationCount > 0,
            NSInternalInconsistencyException);

  if (glxminorversion >= 3)
    {
      error = glXGetFBConfigAttrib(display, fbconfig[pickedFBConfig], attrib, vals);
      if ( error != 0 )
          NSDebugMLLog( @"GLX", @"Can not get FB attribute for pixel format %@ - Error %u",
                                self, error );
    }
  else
    {
      error = glXGetConfig(display, visualinfo, attrib, vals);
      if ( error != 0 )
          NSDebugMLLog( @"GLX", @"Can not get FB attribute for pixel format %@ - Error %u",
                                self, error );
    }
}

- (NSMutableData *) assembleGLXAttributes:(NSOpenGLPixelFormatAttribute *)pixelFormatAttributes
{
  int AccumSize;
  NSOpenGLPixelFormatAttribute *ptr = pixelFormatAttributes;
  NSMutableData *data = [NSMutableData data];
  
  shouldRequestARGBVisual = NO;

#define append(a, b) \
do \
{ \
  int v1 = a; \
  int v2 = b; \
  [data appendBytes: &v1 length: sizeof(v1)]; \
  [data appendBytes: &v2 length: sizeof(v2)]; \
} while (0)

#define append1(a) \
do \
{ \
  int v1 = a; \
  [data appendBytes: &v1 length: sizeof(v1)]; \
} while (0)

  GLint drawable_type = 0;

  if (glxminorversion < 3)
    {
      append1(GLX_RGBA);
    }
  else
    {
      append(GLX_RENDER_TYPE, GLX_RGBA_BIT);
    }

  while (*ptr)
    {
      switch(*ptr)
        {
          // it means all the same on GLX - there is no diffrent here
          case NSOpenGLPFASingleRenderer:
          case NSOpenGLPFAAllRenderers:
          case NSOpenGLPFAAccelerated:
            if (glxminorversion < 3)
			  {
                append(GLX_USE_GL, YES);
                break;
			  }
          case  NSOpenGLPFADoubleBuffer:
            {
              append(GLX_DOUBLEBUFFER, YES);
              break;
            }
          case NSOpenGLPFAStereo:
            {
              append(GLX_STEREO, YES);
              break;
            }
          case NSOpenGLPFAAuxBuffers:
            {
              ptr++;
              append(GLX_AUX_BUFFERS, *ptr);
              break;
            }
          case NSOpenGLPFAColorSize:
            {
                ptr++;
                append(GLX_BUFFER_SIZE, *ptr);
/*
                append(GLX_RED_SIZE, *ptr);
                append(GLX_GREEN_SIZE, *ptr);
                append(GLX_BLUE_SIZE, *ptr);
*/
                break;
            }
          case NSOpenGLPFAAlphaSize:
            {
                ptr++;
                append(GLX_ALPHA_SIZE, *ptr);
                if (*ptr > 0)
                  shouldRequestARGBVisual = YES;
                break;
            }
          case NSOpenGLPFADepthSize:
            {
                ptr++;
                append(GLX_DEPTH_SIZE, *ptr);
                break;
            }
          case NSOpenGLPFAStencilSize:
            {
                ptr++;
                append(GLX_STENCIL_SIZE, *ptr);
                break;
            }
          case NSOpenGLPFAAccumSize:
            {
                ptr++;
                AccumSize=*ptr;  
                switch (AccumSize)
                  {
                    case 8:
                      append(GLX_ACCUM_RED_SIZE, 3);
                      append(GLX_ACCUM_GREEN_SIZE, 3);
                      append(GLX_ACCUM_BLUE_SIZE, 2);
                      append(GLX_ACCUM_ALPHA_SIZE, 0);
                      break;
                    case 15:
                    case 16:
                      append(GLX_ACCUM_RED_SIZE, 5);
                      append(GLX_ACCUM_GREEN_SIZE, 5);
                      append(GLX_ACCUM_BLUE_SIZE, 5);
                      append(GLX_ACCUM_ALPHA_SIZE, 0);
                      break;
                    case 24:
                      append(GLX_ACCUM_RED_SIZE, 8);
                      append(GLX_ACCUM_GREEN_SIZE, 8);
                      append(GLX_ACCUM_BLUE_SIZE, 8);
                      append(GLX_ACCUM_ALPHA_SIZE, 0);
                      break;
                    case 32:
                      append(GLX_ACCUM_RED_SIZE, 8);
                      append(GLX_ACCUM_GREEN_SIZE, 8);
                      append(GLX_ACCUM_BLUE_SIZE, 8);
                      append(GLX_ACCUM_ALPHA_SIZE, 8);
                      break;
                  }
                break;
            }

          case NSOpenGLPFAWindow:
            {
              drawable_type |= GLX_WINDOW_BIT;
              break;
            }
          case NSOpenGLPFAPixelBuffer:
            { 
              drawable_type |= GLX_PBUFFER_BIT;
              break;
            }
          case NSOpenGLPFAOffScreen:
            {
              drawable_type |= GLX_PIXMAP_BIT;
              break;
            }

          //can not be handled by X11
          case NSOpenGLPFAMinimumPolicy:
            {
              break;
            }
          // can not be handled by X11
          case NSOpenGLPFAMaximumPolicy:
            {
              break;
            }
          // Not supported, would be a lot of work to implement.
          case NSOpenGLPFAFullScreen:
            {
                break;
            }

          case NSOpenGLPFASampleBuffers:
            {
              #ifdef GLX_SAMPLE_BUFFERS
              if ( glxminorversion >= 4 )
                {
                  ptr++;
                  append(GLX_SAMPLE_BUFFERS, *ptr);
                }
              #endif
              break;
            }
          case NSOpenGLPFASamples:
            {
              #ifdef GLX_SAMPLES
              if ( glxminorversion >= 4 )
                {
                  ptr++;
                  append(GLX_SAMPLES, *ptr);
                }
              #endif
              break;
            }

          case NSOpenGLPFAAuxDepthStencil:
          case NSOpenGLPFARendererID:
          case NSOpenGLPFANoRecovery:
          case NSOpenGLPFAClosestPolicy:
          case NSOpenGLPFARobust:
          case NSOpenGLPFABackingStore:
          case NSOpenGLPFAMPSafe:
          case NSOpenGLPFAMultiScreen:
          case NSOpenGLPFACompliant:
          case NSOpenGLPFAScreenMask:
          case NSOpenGLPFAVirtualScreenCount:
          case NSOpenGLPFAAllowOfflineRenderers:
          case NSOpenGLPFAColorFloat:
          case NSOpenGLPFAMultisample:
          case NSOpenGLPFASupersample:
          case NSOpenGLPFASampleAlpha:
            break;
        }
      ptr++;
    }

  if ( drawable_type )
    {
      append(GLX_DRAWABLE_TYPE, drawable_type);
    }

  append1(None);

  return data;

#undef append
#undef append1
}

- (id)initWithAttributes:(NSOpenGLPixelFormatAttribute *)attribs
{
  NSMutableData * glxAttributes;  
  NSDictionary * dsattributes;

  self = [super init];
  if (self == nil)
    {
      return nil;
    }

  fbconfig = NULL;
  visualinfo = NULL;

  display = [(XGServer *)GSCurrentServer() xDisplay];
  NSAssert(display != NULL, NSInternalInconsistencyException);

  glxminorversion = [XGGLPixelFormat glxMinorVersion];
  NSDebugMLLog(@"GLX", @"minor version %d", glxminorversion);

  glxAttributes = [self assembleGLXAttributes:attribs];  
  dsattributes = [GSCurrentServer() attributes];

  if (glxminorversion >= 3)
    {
      fbconfig = glXChooseFBConfig(display,
                        [[dsattributes objectForKey: GSScreenNumber] intValue],
                        [glxAttributes mutableBytes],
                        &configurationCount);

      #if defined(XRENDER)
      int i; 
      for (i = 0; i < configurationCount; i++)
        {
          XVisualInfo * vinfo = glXGetVisualFromFBConfig(display, fbconfig[i]);
          XRenderPictFormat* pictFormat = XRenderFindVisualFormat (display, vinfo->visual);
	  if ((NULL != pictFormat
               && (pictFormat->type == PictTypeDirect)
               && (pictFormat->direct.alphaMask))
              || !shouldRequestARGBVisual)
	    {
              pickedFBConfig = i;
              visualinfo = vinfo;
              break;
            }
        }
      #endif

      if (!visualinfo && configurationCount > 0)
        {
          visualinfo = glXGetVisualFromFBConfig(display,fbconfig[0]);
          pickedFBConfig = 0;
        }
    }
  else
    {
      visualinfo = glXChooseVisual(display,
                        [[dsattributes objectForKey: GSScreenNumber] intValue],
                        [glxAttributes mutableBytes]);
    }
  
  if (fbconfig == NULL && visualinfo == NULL)
    {
      NSDebugMLLog(@"GLX", @"no pixel format found matching what is required");
      RELEASE(self);

      return nil;
    }
  else
    {
      NSDebugMLLog(@"GLX", @"We found %d pixel formats", configurationCount);
      
      return self;
    }
}

- (Display *) display
{
  return display;
}

- (XVisualInfo *) visualinfo
{
  return visualinfo;
}

- (GLXContext)createGLXContext: (XGGLContext *)share
{
  GLXContext context;

  if (glxminorversion >= 3)
    {
      context = glXCreateNewContext(display, fbconfig[pickedFBConfig], 
                                 GLX_RGBA_TYPE, [share glxcontext], YES);
    }
  else
    {
      context = glXCreateContext(display, visualinfo, 
                              [share glxcontext], GL_TRUE);
    }

  if ( context == NULL )
    {
      NSDebugMLLog(@"GLX", 
        @"Cannot create GL context for pixel format %@ - Error %s",
        self, glGetString(glGetError()));
    }

  return context;
}

- (GLXWindow) drawableForWindow: (Window)xwindowid
{
  GLXWindow win;
  GLenum error;

  if (glxminorversion >= 3)
    {
      win = glXCreateWindow(display, fbconfig[pickedFBConfig], xwindowid, NULL);
    }
  else
    {
      win = xwindowid;
    }

  error = glGetError();
  if ( error != GL_NO_ERROR )
    {
      NSDebugMLLog(@"GLX", 
                   @"Cannot create GL window for pixel format %@ - Error %s",
                   self, glGetString(error));
    }

  return win;
}

- (void) dealloc
{
  if (glxminorversion >= 3)
    {
      XFree(fbconfig);
    }

  XFree(visualinfo);

  NSDebugMLLog(@"GLX", @"deallocation");
  [super dealloc];
}

- (int)numberOfVirtualScreens
{
  //FIXME
  return 1;
}

@end

#endif

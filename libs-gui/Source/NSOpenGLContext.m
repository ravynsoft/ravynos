/** <title>NSOpenGLContext.m </title>

   <abstract>Context for openGL drawing</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Frederic De Jaeger
   Date: 2002

   This file is part of the GNUstep GUI Library.

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

#import <Foundation/NSDebug.h>
#import "AppKit/NSOpenGL.h"
#import "GNUstepGUI/GSDisplayServer.h"

@implementation NSOpenGLContext
+ _classContext
{
  Class glContextClass = [GSCurrentServer() glContextClass];

  if (glContextClass == nil)
    {
      NSWarnMLog(@"Backend doesn't have any gl context");
      return nil;
    }
  else
    {
      NSDebugMLLog(@"GLX", @"found something");
      return glContextClass;
    }
}


+ allocWithZone: (NSZone *) z
{
  Class c = [self _classContext];
  if (c)
    return NSAllocateObject(c, 0, z);
  else
    return nil;
}

+ (void)clearCurrentContext
{
  [[self _classContext] clearCurrentContext];
}

+ (NSOpenGLContext *)currentContext
{
  /* FIXME - There doesn't seem to be a way to fix the following
   * warning.  */
  return [[self _classContext] currentContext];
}

- (void *)CGLContextObj
{
  [self subclassResponsibility: _cmd];
  return NULL;
}

- (void)clearDrawable
{
  [self subclassResponsibility: _cmd];
}

- (void)copyAttributesFromContext:(NSOpenGLContext *)context 
			 withMask:(unsigned long)mask
{
  [self subclassResponsibility: _cmd];
}

- (void)createTexture:(unsigned long)target 
	     fromView:(NSView*)view 
       internalFormat:(unsigned long)format
{
  [self subclassResponsibility: _cmd];
}


- (int)currentVirtualScreen
{
  [self subclassResponsibility: _cmd];
  return 0;
}


- (void)flushBuffer
{
  [self subclassResponsibility: _cmd];
}


- (void)getValues:(long *)vals 
     forParameter:(NSOpenGLContextParameter)param
{
  [self subclassResponsibility: _cmd];
}

- (id)initWithCGLContextObj:(void *)context
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (id)initWithFormat:(NSOpenGLPixelFormat *)format 
	shareContext:(NSOpenGLContext *)share
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (void)makeCurrentContext
{
  [self subclassResponsibility: _cmd];
}


- (void)setCurrentVirtualScreen:(int)screen
{
  [self subclassResponsibility: _cmd];
}


- (void)setFullScreen
{
  [self subclassResponsibility: _cmd];
}


- (void)setOffScreen:(void *)baseaddr 
	       width:(long)width 
	      height:(long)height 
	    rowbytes:(long)rowbytes
{
  [self subclassResponsibility: _cmd];
}


- (void)setValues:(const long *)vals 
     forParameter:(NSOpenGLContextParameter)param
{
  [self subclassResponsibility: _cmd];
}


- (void)setView:(NSView *)view
{
  [self subclassResponsibility: _cmd];
}


- (void)update
{
  [self subclassResponsibility: _cmd];
}


- (NSView *)view
{
  [self subclassResponsibility: _cmd];
  return nil;
}
@end



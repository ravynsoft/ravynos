/** <title>NSOpenGLView.m </title>

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
#import <Foundation/NSException.h>
#import <Foundation/NSNotification.h>
#import "AppKit/NSOpenGL.h"
#import "AppKit/NSOpenGLView.h"

// Declare a private method of NSView
@interface NSView (Private)
- (void) _lockFocusInContext: (NSGraphicsContext *)ctxt inRect: (NSRect)rect;
- (void) _viewWillMoveToWindow: (NSWindow*)newWindow;
@end

/**
  <unit>
  <heading>NSOpenGLView</heading>

  <p>
  This class is simply a view with a NSOpenGLContext attached to it.
  This is the simplest way to initialize a GL context within GNUstep.
  </p>

  
  <p>
  There is a mechanism to update the context when the view is moved
  or resize.
  </p>

  </unit>
*/

static NSOpenGLPixelFormat *fmt = nil;
static NSOpenGLPixelFormatAttribute attrs[] =
    {	
      NSOpenGLPFADoubleBuffer,
      NSOpenGLPFADepthSize, 16,
      NSOpenGLPFAColorSize, 1,
      0
    };
//   NSOpenGLPixelFormatAttribute attrs[] =
//     {
//       NSOpenGLPFADoubleBuffer,
//       NSOpenGLPFADepthSize, 32,
//       0
//     };

@implementation NSOpenGLView
/**
   return a standard NSOpenGLPixelFormat you can pass to the 
   initWithFrame: pixelFormat: method
 */
+ (NSOpenGLPixelFormat*) defaultPixelFormat
{
  // Initialize it once
  if (!fmt)
    fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: attrs];

  if (fmt)
    return fmt;
  else
    {
      NSWarnMLog(@"could not find a reasonable pixel format...");
      return nil;
    }
}
  
/**
   detach from the current context.  You should call it before releasing this 
   object.
 */
- (void) clearGLContext
{
  if (glcontext)
    {
      [glcontext clearDrawable];
      DESTROY(glcontext);
      prepared = NO;
    }
}

- (void) setOpenGLContext: (NSOpenGLContext*)context
{
  if ( context != glcontext )
    {
      [self clearGLContext];
      ASSIGN(glcontext, context);
    }
}

- (void) prepareOpenGL
{
}

/**
   return the current gl context associated with this view
*/
- (NSOpenGLContext*) openGLContext
{
  if (glcontext == nil)
    {
      NSOpenGLContext *context = [[NSOpenGLContext alloc] 
                                     initWithFormat: pixel_format
                                     shareContext: nil];

      [self setOpenGLContext: context];
      [context setView: self];

      RELEASE(context);
    }
  return glcontext;
}

-(id) initWithFrame: (NSRect)frameRect
{  
  return [self initWithFrame: frameRect
               pixelFormat: [[self class] defaultPixelFormat]];
  
}

/** default initializer.  Can be passed [NSOpenGLView defaultPixelFormat]
    as second argument
*/
- (id) initWithFrame: (NSRect)frameRect 
         pixelFormat: (NSOpenGLPixelFormat*)format
{
  self = [super initWithFrame: frameRect];
  if (!self)
    return nil;

  ASSIGN(pixel_format, format);

  [[NSNotificationCenter defaultCenter] 
    addObserver: self
    selector: @selector(_frameChanged:)
    name: NSViewGlobalFrameDidChangeNotification
    object: self];

  [self setPostsFrameChangedNotifications: YES];
  [[NSNotificationCenter defaultCenter] 
    addObserver: self
    selector: @selector(_frameChanged:)
    name: NSViewFrameDidChangeNotification
    object: self];

  return self;
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  [self clearGLContext];
  RELEASE(pixel_format);
  NSDebugMLLog(@"GL", @"deallocating");
  [super dealloc];
}

- (NSOpenGLPixelFormat*) pixelFormat
{
  return pixel_format;
}

- (void) setPixelFormat: (NSOpenGLPixelFormat*)pixelFormat
{
  ASSIGN(pixel_format, pixelFormat);
}

- (void) reshape
{
}

- (void) update
{
  NSOpenGLContext *context;
  context = [self openGLContext];
  if ([context view] == self)
    {
      [context update];
    }
}

- (BOOL) isOpaque
{
  return YES;
}

- (void) _frameChanged: (NSNotification *) aNot
{
  if (prepared)
    {
      [[self openGLContext] makeCurrentContext];
      [self update];
      [self reshape];
    }
}

/* FIXME: this should be done in [lockFocus] or [lockFocusInRect:].
 */
- (void) _lockFocusInContext: (NSGraphicsContext *)ctxt inRect: (NSRect)rect
{
  NSOpenGLContext *context;
  context = [self openGLContext];

  [super _lockFocusInContext: ctxt inRect: rect];
  [context makeCurrentContext];
  if ([context view] != self)
    {
      [context setView:self];
    }
  if (!prepared)
    {
      [self prepareOpenGL];
      prepared = YES;
      [self reshape];
    }
}

-(void) _viewWillMoveToWindow: (NSWindow *) newWindow
{
  [super _viewWillMoveToWindow: newWindow];

  if ([self window] != newWindow)
    {
      // the context will be recreated in the new window if needed
      [[self openGLContext] clearDrawable];
    }
}

- (id) initWithCoder: (NSCoder *)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (!self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      [self setPixelFormat: [aDecoder decodeObjectForKey: @"NSPixelFormat"]];
    }
  else
    {
      [self setPixelFormat: [[self class] defaultPixelFormat]];
    }
 
  [[NSNotificationCenter defaultCenter] 
    addObserver: self
    selector: @selector(_frameChanged:)
    name: NSViewGlobalFrameDidChangeNotification
    object: self];

  [self setPostsFrameChangedNotifications: YES];
  [[NSNotificationCenter defaultCenter] 
    addObserver: self
    selector: @selector(_frameChanged:)
    name: NSViewFrameDidChangeNotification
    object: self];

  return self;
}

@end



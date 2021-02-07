/* WIN32Context - Implements graphic context for MSWindows

   Copyright (C) 2002 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2002
   
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

#include <Foundation/NSDebug.h>
#include <Foundation/NSString.h>
#include <AppKit/NSBitmapImageRep.h>
#include <AppKit/NSGraphics.h>

#include "winlib/WIN32GState.h"
#include "winlib/WIN32FontEnumerator.h"
#include "winlib/WIN32FontInfo.h"
#include "winlib/WIN32Context.h"

/* Common graphics functions */
@implementation WIN32Context

/* Initialize AppKit backend */
+ (void)initializeBackend
{
  NSDebugLog(@"Initializing GNUstep GUI Win32 backend.\n");

  [NSGraphicsContext setDefaultContextClass: [WIN32Context class]];
  [GSFontEnumerator setDefaultClass: [WIN32FontEnumerator class]];
  [GSFontInfo setDefaultClass: [WIN32FontInfo class]];
}

+ (Class) GStateClass
{
  return [WIN32GState class];
}

- (void)flushGraphics
{
}

// Try to match restrictions in GSCreateBitmap()
- (BOOL) isCompatibleBitmap: (NSBitmapImageRep*)bitmap
{
  NSString *colorSpaceName;
  NSInteger numColors;

  if ([bitmap bitmapFormat] != 0)
    {
      return NO;
    }

  if ([bitmap isPlanar])
    {
      return NO;
    }

  if ([bitmap bitsPerSample] != 8)
    {
      return NO;
    }

  numColors = [bitmap samplesPerPixel] - ([bitmap hasAlpha] ? 1 : 0);
   colorSpaceName = [bitmap colorSpaceName];
  if ([colorSpaceName isEqualToString: NSDeviceRGBColorSpace] ||
      [colorSpaceName isEqualToString: NSCalibratedRGBColorSpace])
    {
      return (numColors == 3);
    }
  else if ([colorSpaceName isEqualToString: NSDeviceWhiteColorSpace] ||
      [colorSpaceName isEqualToString: NSCalibratedWhiteColorSpace])
    {
      return (numColors == 1);
    }
  else if ([colorSpaceName isEqualToString: NSDeviceBlackColorSpace] ||
      [colorSpaceName isEqualToString: NSCalibratedBlackColorSpace])
    {
      return (numColors == 1);
    }
  else
    {
      return NO;
    }
 }

@end

@implementation WIN32Context (Ops)

- (void) GSCurrentDevice: (void **)device : (int *)x : (int *)y
{
  void *windevice = [(WIN32GState *)gstate window];
  if (device)
    *device =  windevice;
  if (x && y)
    {
      NSPoint offset = [gstate offset];
      *x = offset.x;
      *y = offset.y;
    }
}

- (void) GSSetDevice: (void *)device : (int)x : (int)y
{
  [(WIN32GState*)gstate setWindow: (HWND)device];
  [gstate setOffset: NSMakePoint(x, y)];
}

- (BOOL) supportsDrawGState
{
        return YES;
}

@end

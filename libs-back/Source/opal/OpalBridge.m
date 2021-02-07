/*
   OpalBridge.m

   Copyright (C) 2017 Free Software Foundation, Inc.

   Author: Daniel Ferreira <dtf@stanford.edu>
   Date: July 2017

   This file is part of GNUstep.

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

#import <AppKit/NSColor.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSColorSpace.h>
#import <AppKit/NSGraphics.h>
#import <CoreGraphics/CGColor.h>
#import <CoreGraphics/CGImage.h>
#import <objc/runtime.h>

@implementation NSColor (GSQuartz)
/*
 * FIXME:
 * NOTE 1: GNUstep-GUI does not allow an NSColor to be created with a custom
 * NSColorSpace. If this were allowed, we'd have to:
 *   1) implement a bridge for -[NSColorSpace CGColorSpace]
 *   2) for each color, extract that color space and generate a CGColorRef.
 * NOTE 2: GNUstep-GUI makes no distinction of device and generic color spaces.
 * If this ever ceases to be the case, some adjustment might be necessary here.
 */
- (CGColorRef)CGColor
{
  NSString *name = [self colorSpaceName];
  
  // FIXME: we should handle black color spaces here, which we currently
  // ignore in the implementation.
  CFStringRef cgColorSpaceName = NULL;
  if ([name isEqualToString: NSCalibratedRGBColorSpace]
    || [name isEqualToString: NSDeviceRGBColorSpace])
    cgColorSpaceName = kCGColorSpaceSRGB;
  else if ([name isEqualToString: NSCalibratedBlackColorSpace]
    || [name isEqualToString: NSCalibratedWhiteColorSpace]
    || [name isEqualToString: NSDeviceBlackColorSpace]
    || [name isEqualToString: NSDeviceWhiteColorSpace])
    cgColorSpaceName = kCGColorSpaceGenericGray;
  else if ([name isEqualToString: NSDeviceCMYKColorSpace])
    cgColorSpaceName = kCGColorSpaceGenericCMYK;
  else if ([name isEqualToString: NSNamedColorSpace])
    return [[self colorUsingColorSpaceName: NSDeviceRGBColorSpace] CGColor];

  if (cgColorSpaceName == NULL)
    return NULL;

  CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(cgColorSpaceName);
  CGFloat values[10];
  [self getComponents: values];

  CGColorRef color = CGColorCreate(colorSpace, values);
  CFRelease(colorSpace);

  return (CGColorRef)[(id)color autorelease];
}
@end

@implementation NSImageRep (GSQuartz)
- (CGImageRef)CGImageForProposedRect: (NSRect *)proposedDestRect 
                             context: (NSGraphicsContext *)referenceContext 
                               hints: (NSDictionary *)hints
{
  /*
   * FIXME Must implement this.
   * A note for future implementors:
    > Apparently each NSImageRep subclass implements this method, with the
    > base implementation being[1], as I understand it (which may be wrong):
    > i. create a new blank context with *proposedDestRect.size and set it
    > as the current context. This context should theoretically be
    > constructed with properties extracted from `referenceContext` and/or
    > `hints`, although I suppose our first implementation can go without
    > that.
    > ii. call [self draw];
    > iii. adjust *proposedDestRect to round half-pixels
    > iv. extract a CGImage from the bitmap context
    > If NSImage.size == *proposedDestRect.size, we should just CGImageCreate
    > directly from our representation data.
   */

  return NULL;
}
@end

@implementation NSImage (GSQuartz)
- (CGImageRef)CGImageForProposedRect: (NSRect *)proposedDestRect 
                             context: (NSGraphicsContext *)referenceContext 
                               hints: (NSDictionary *)hints
{
  // FIXME: Must implement this.
  // This should pick the best NSImageRep for this NSImage and call
  // -[NSImageRep CGImageForProposedRect:...].
  return NULL;
}
@end

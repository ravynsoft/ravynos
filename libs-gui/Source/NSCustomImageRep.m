/** <title>NSCustomImageRep</title>

   <abstract>Custom image representation.</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.
   
   Author:  Adam Fedor <fedor@colorado.edu>
   Date: Mar 1996
   
   This file is part of the GNUstep Application Kit Library.

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
#import <Foundation/NSCoder.h>
#import <Foundation/NSDebug.h>
#import "AppKit/NSCustomImageRep.h"
#import "AppKit/NSGraphicsContext.h"
#import "AppKit/NSView.h"
#import "AppKit/NSColor.h"
#import "AppKit/DPSOperators.h"

/** <p>TODO : Desciption</p>
 */
@implementation NSCustomImageRep

/**<p> Initializes a new NSCustomImageRep with. When a -draw message is
   recieved it send aSelector message to the delegate anObject.
   The delegate is not retained, so it is the caller's responsibility
   to ensure the delegate remains valid for the life of the receiver.</p>
   <p>See Also: -delegate -drawSelector [NSImageRep-draw]</p>
 */
- (id) initWithDrawSelector: (SEL)aSelector
		   delegate: (id)anObject
{
  if ( ! ( self = [super init] ) ) 
    return nil;

  _delegate = anObject;
  _selector = aSelector;
  return self;
}

- (void) dealloc
{
  [super dealloc];
}

/** <p>Returns the NSCustomImageRep's delegate.</p>
    <p>See Also: -initWithDrawSelector:delegate:</p>
 */
- (id) delegate
{
  return _delegate;
}

/**<p>Returns the draw method sent to the delegate when NSCustomImageRep 
   recieved a -draw message.</p>
   <p>See Also: -initWithDrawSelector:delegate: [NSImageRep-draw]</p>
 */
- (SEL) drawSelector
{
  return _selector;
}

- (BOOL) draw
{
  [_delegate performSelector: _selector withObject: self];
  return YES;
}

//
// TODO: For both of the following methods we can extract the 
// logic in the superclass to another method and call it here 
// if the delegate is set from both places.
//
- (BOOL) drawAtPoint: (NSPoint)aPoint
{
  BOOL ok, reset;
  NSGraphicsContext *ctxt;
  NSAffineTransform *ctm = nil;

  // if both are zero and the delegate isn't set, return no.
  if (_size.width == 0 && _size.height == 0 && _delegate == nil)
    return NO;

  NSDebugLLog(@"NSImage", @"Drawing at point %f %f\n", aPoint.x, aPoint.y);
  reset = 0;
  ctxt = GSCurrentContext();
  if (aPoint.x != 0 || aPoint.y != 0)
    {
      ctm = GSCurrentCTM(ctxt);
      DPStranslate(ctxt, aPoint.x, aPoint.y);
      reset = 1;
    }
  ok = [self draw];
  if (reset)
    GSSetCTM(ctxt, ctm);
  return ok;
}

- (BOOL) drawInRect: (NSRect)aRect
{
  NSSize scale;
  BOOL ok;
  NSGraphicsContext *ctxt;
  NSAffineTransform *ctm;

  NSDebugLLog(@"NSImage", @"Drawing in rect (%f %f %f %f)\n", 
	      NSMinX(aRect), NSMinY(aRect), NSWidth(aRect), NSHeight(aRect));

  // if both are zero and the delegate isn't set.
  if (_size.width == 0 && _size.height == 0 && _delegate == nil)
    return NO;

  ctxt = GSCurrentContext();
  
  // if either is zero, don't scale at all.
  if (_size.width == 0 || _size.height == 0)
    {
      scale = NSMakeSize(1, 1); 
    }
  else
    {
      scale = NSMakeSize(NSWidth(aRect) / _size.width, 
			 NSHeight(aRect) / _size.height);
    }

  ctm = GSCurrentCTM(ctxt);
  DPStranslate(ctxt, NSMinX(aRect), NSMinY(aRect));
  DPSscale(ctxt, scale.width, scale.height);
  ok = [self draw];
  GSSetCTM(ctxt, ctm);
  return ok;
}

// NSCoding protocol
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];

  if ([aCoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      // FIXME: Should this be changed to encodeConditionalObject: ?
      [aCoder encodeObject: _delegate];
      [aCoder encodeValueOfObjCType: @encode(SEL) at: &_selector];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];

  if ([aDecoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_delegate];
      [aDecoder decodeValueOfObjCType: @encode(SEL) at: &_selector];
    }
  return self;
}

@end

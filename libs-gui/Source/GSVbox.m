/** <title>GSVbox</title>

   <abstract>The GSVbox class (a GNU extension)</abstract>

   Copyright (C) 1999-2010 Free Software Foundation, Inc.

   Author:  Nicola Pero <n.pero@mi.flashnet.it>
   Date: 1999

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

// See GSHbox.m for comments
// This file is derived from GSVbox.m
#import "GNUstepGUI/GSVbox.h"
#import "AppKit/NSBox.h"

@implementation GSVbox: GSTable
//
// Class methods
//
+(void) initialize
{
  if (self == [GSVbox class])
    [self setVersion: 1];
}

//
// Instance Methods
//
-(id) init
{
  self = [super initWithNumberOfRows: 1
                     numberOfColumns: 1];
  if (nil == self)
    return nil;

  _haveViews = NO;
  _defaultMinYMargin = 0;
  return self;
}

-(void) dealloc
{
  [super dealloc];
}

// 
// Adding Views 
// 
-(void) addView: (NSView *)aView
{
  [self addView: aView
	enablingYResizing: YES
	withMinYMargin: _defaultMinYMargin];
}

-(void)   addView: (NSView *)aView
enablingYResizing: (BOOL)aFlag
{
  [self addView: aView
	enablingYResizing: aFlag
	withMinYMargin: _defaultMinYMargin];
}

-(void) addView: (NSView *)aView
 withMinYMargin: (float) aMargin
{
  [self addView: aView
	enablingYResizing: YES
	withMinYMargin: aMargin];
}

-(void)   addView: (NSView *)aView
enablingYResizing: (BOOL)aFlag
   withMinYMargin: (float)aMargin	 
{
  if (_haveViews)
    {
      int entries = _numberOfRows;

      [super addRow];
      
      [super setYResizingEnabled: aFlag
	     forRow: entries];
      
      [super putView: aView
	     atRow: entries
	     column: 0
	     withMinXMargin: 0
	     maxXMargin: 0
	     minYMargin: aMargin	 
	     maxYMargin: 0];
    }
  else // !_haveViews
    {
      [super setYResizingEnabled: aFlag
	     forRow: 0];
      
      [super putView: aView
	     atRow: 0
	     column: 0
	     withMinXMargin: 0
	     maxXMargin: 0
	     minYMargin: 0	 
	     maxYMargin: 0];
      
      _haveViews = YES;
    }
  
}

//
// Adding a Separator
//
-(void) addSeparator
{
  [self addSeparatorWithMinYMargin: _defaultMinYMargin];
}

-(void) addSeparatorWithMinYMargin: (float)aMargin
{
  NSBox *separator;
  
  separator = [[NSBox alloc] initWithFrame: NSMakeRect (0, 0, 2, 2)];
  [separator setAutoresizingMask: (NSViewWidthSizable 
				   | NSViewMinYMargin | NSViewMaxYMargin)];
  [separator setTitlePosition: NSNoTitle];
  [separator setBorderType: NSGrooveBorder];
  [self addView: separator
	enablingYResizing: NO
	withMinYMargin: aMargin];
  [separator release];
}

//
// Setting Margins
//
-(void) setDefaultMinYMargin: (float)aMargin
{
  _defaultMinYMargin = aMargin;
}

//
// Getting the number of Entries
//
-(int) numberOfViews
{
  if (_haveViews)
    return _numberOfRows;
  else
    return 0;
}

//
// NSCoding protocol
//
-(void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeBool: _haveViews forKey: @"GSHaveViews"];
      [aCoder encodeFloat: _defaultMinYMargin forKey: @"GSDefaultMinYMargin"];
    }
  else
    {
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_haveViews];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_defaultMinYMargin];
    }
}

-(id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (nil == self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      _haveViews = [aDecoder decodeBoolForKey: @"GSHaveViews"];
      _defaultMinYMargin = [aDecoder decodeFloatForKey: @"GSDefaultMinYMargin"];
    }
  else
    {
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_haveViews];
      [aDecoder decodeValueOfObjCType: @encode(float) at: &_defaultMinYMargin];
    }
  return self;
}
@end





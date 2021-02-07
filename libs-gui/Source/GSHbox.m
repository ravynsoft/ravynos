/** <title>GSHbox</title>

   <abstract>The GSHbox class (a GNU extension)</abstract>

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

#import "GNUstepGUI/GSHbox.h"
// For the separator
#import "AppKit/NSBox.h"

@implementation GSHbox: GSTable
//
// Class methods
//
+(void) initialize
{
  if (self == [GSHbox class])
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
  _defaultMinXMargin = 0;
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
	enablingXResizing: YES
	withMinXMargin: _defaultMinXMargin];
}

-(void)   addView: (NSView *)aView
enablingXResizing: (BOOL)aFlag
{
  [self addView: aView
	enablingXResizing: aFlag
	withMinXMargin: _defaultMinXMargin];
}

-(void) addView: (NSView *)aView
 withMinXMargin: (float) aMargin
{
  [self addView: aView
	enablingXResizing: YES
	withMinXMargin: aMargin];
}

-(void)   addView: (NSView *)aView
enablingXResizing: (BOOL)aFlag
   withMinXMargin: (float)aMargin	 
{
  if (_haveViews)
    {
      int entries = _numberOfColumns;

      [super addColumn];
      
      [super setXResizingEnabled: aFlag
	     forColumn: entries];
      
      [super putView: aView
	     atRow: 0
	     column: entries
	     withMinXMargin: aMargin
	     maxXMargin: 0
	     minYMargin: 0	 
	     maxYMargin: 0];
    }
  else // !_haveViews
    {
      [super setXResizingEnabled: aFlag
	     forColumn: 0];
      
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
  [self addSeparatorWithMinXMargin: _defaultMinXMargin];
}

-(void) addSeparatorWithMinXMargin: (float)aMargin
{
  NSBox *separator;
  
  separator = [[NSBox alloc] initWithFrame: NSMakeRect (0, 0, 2, 2)];
  [separator setAutoresizingMask: (NSViewMinXMargin | NSViewMaxXMargin
				   | NSViewHeightSizable)];
  [separator setTitlePosition: NSNoTitle];
  [separator setBorderType: NSGrooveBorder];
  [self addView: separator
	enablingXResizing: NO
	withMinXMargin: aMargin];
  [separator release];
}

//
// Setting Margins
//
-(void) setDefaultMinXMargin: (float)aMargin
{
  _defaultMinXMargin = aMargin;
}

//
// Getting the number of Entries
//
-(int) numberOfViews
{
  if (_haveViews)
    return _numberOfColumns;
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
      [aCoder encodeFloat: _defaultMinXMargin forKey: @"GSDefaultMinXMargin"];
    }
  else
    {
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_haveViews];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_defaultMinXMargin];
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
      _defaultMinXMargin = [aDecoder decodeFloatForKey: @"GSDefaultMinXMargin"];
    }
  else
    {
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_haveViews];
      [aDecoder decodeValueOfObjCType: @encode(float) at: &_defaultMinXMargin];
    }
  return self;
}
@end





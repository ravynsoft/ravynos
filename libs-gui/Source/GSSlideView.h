/** -*- mode: ObjC -*-
  <title>GSSlideView</title>

   <abstract>View that is slid by NSWorkspace</abstract>

   Copyright (C) 2002 Free Software Foundation, Inc.

   Written By: <author name="Enrico Sersale"><email>enrico@imago.ro</email></author>
   Date: Jan 2002
   
   This file is part of the GNU Objective C User Interface library.

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

#import <Foundation/NSGeometry.h>
#import "AppKit/NSView.h"

@class NSCell;

@interface GSSlideView : NSView
{
  NSCell *slideCell;  
}

+ (BOOL) _slideImage: (NSImage*)image 
                from: (NSPoint)fromPoint 
                  to: (NSPoint)toPoint;
@end

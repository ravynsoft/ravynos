/*
   <title>NSNibControlConnector</title>

   <abstract>Implementation of NSNibControlConnector</abstract>

   Copyright (C) 1999, 2015 Free Software Foundation, Inc.

   Author:  Richard Frith-Macdonald <richard@branstorm.co.uk>
   Date: 1999
   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: August 2015

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

#import "AppKit/NSControl.h"
#import "AppKit/NSNibControlConnector.h"

@implementation	NSNibControlConnector

- (void) establishConnection
{
  SEL sel = NSSelectorFromString(_tag);

  [_src setTarget: _dst];
  [_src setAction: sel];
}

@end

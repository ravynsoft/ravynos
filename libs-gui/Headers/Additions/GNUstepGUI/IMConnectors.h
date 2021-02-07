/*
   IMConnectors.h

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: November 1997
   
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

/* These classes were inspired by IBConnectors classes from objcX, "an
   Objective-C class library for a window system". The code was originally
   written by Scott Francis, Paul Kunz, Imran Qureshi and Libing Wang. */

#ifndef _GNUstep_H_IMConnectors
#define _GNUstep_H_IMConnectors

#ifndef GNUSTEP
#include <Foundation/Foundation.h>
#else
#include <Foundation/NSObject.h>
#endif

@interface IMConnector : NSObject
{
  id source;
  id destination;
  NSString* label;
}

- source;
- destination;
- label;
@end

@interface IMControlConnector:IMConnector
- (void)establishConnection;
@end

@interface IMOutletConnector : IMConnector
- (void)establishConnection;
@end

#endif /* _GNUstep_H_IMConnectors */

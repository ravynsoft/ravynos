/*
   IMConnectors.m

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

#include <string.h>

#ifndef NeXT_Foundation_LIBRARY
#include <GNUstepBase/GSObjCRuntime.h>
#endif
#include <Foundation/NSObjCRuntime.h>
#include <AppKit/NSActionCell.h>
#include <GNUstepGUI/GMArchiver.h>
#include "GNUstepGUI/IMCustomObject.h"
#include "IMConnectors.h"

@implementation IMConnector

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [archiver encodeObject:source withName:@"source"];
  [archiver encodeObject:destination withName:@"destination"];
  [archiver encodeObject:label withName:@"label"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  source = [unarchiver decodeObjectWithName:@"source"];
  destination = [unarchiver decodeObjectWithName:@"destination"];
  label = [unarchiver decodeObjectWithName:@"label"];
  return self;
}

- (id)source			{ return source; }
- (id)destination		{ return destination; }
- (id)label			{ return label; }

@end /* IMConnector */


@implementation IMControlConnector:IMConnector

- (void)establishConnection
{
  id _source = [source nibInstantiate];
  id _destination = [destination nibInstantiate];
  SEL action = NSSelectorFromString (label);

  if ([_source respondsToSelector:@selector(setTarget:)])
    {
//    NSLog (@"%@: setting target to %@", _source, _destination);
      [_source setTarget:_destination];
    }
#ifndef NeXT_Foundation_LIBRARY
  else
    {
      const char	*type;
      unsigned int	size;
      int		offset;

      /*
       * Use the GNUstep additional function to set the instance
       * variable directly.
       */
      if (GSObjCFindVariable(_source, "target", &type, &size, &offset))
	{
	  GSObjCSetVariable(_source, offset, size, (void*)&_destination); 
	}
    }
#endif

  if ([_source respondsToSelector:@selector(setAction:)])
    {
//    NSLog (@"%@: setting action to %@",
//	    _source, NSStringFromSelector(action));
      [_source setAction:action];
    }
#ifndef NeXT_Foundation_LIBRARY
  else
    {
      const char	*type;
      unsigned int	size;
      int		offset;

      /*
       * Use the GNUstep additional function to set the instance
       * variable directly.
       * FIXME - need some way to do this for libFoundation and
       * Foundation based systems.
       */
      if (GSObjCFindVariable(_source, "action", &type, &size, &offset))
	{
	  GSObjCSetVariable(_source, offset, size, (void*)&action); 
	}
    }
#endif
}

@end /* IMControlConnector:IMConnector */


@implementation IMOutletConnector

- (void)establishConnection
{
  id _source = [source nibInstantiate];
  id _destination = [destination nibInstantiate];
  NSString* setMethodName; 
  SEL setSelector;

  if ([label length] > 1)
    {
      setMethodName = [[[label substringToIndex: 1] capitalizedString]
			stringByAppendingString: 
			  [label substringFromIndex: 1]];
      
      setMethodName = [[@"set" stringByAppendingString: setMethodName]
			stringByAppendingString:@":"];
    }
  else 
    setMethodName = [[@"set" stringByAppendingString:
			 [label capitalizedString]]
		      stringByAppendingString:@":"];
  
  setSelector = NSSelectorFromString (setMethodName);

  // NSLog (@"establish connection: source %@, destination %@, label %@",
  //	 _source, _destination, label);
  // NSLog (@"Method Name: %@", setMethodName);

  if (setSelector && [_source respondsToSelector:setSelector])
    {
      [_source performSelector:setSelector withObject:_destination];
    }
#ifndef NeXT_Foundation_LIBRARY
  else
    {
      const char	*nam = [label cString];
      const char	*type;
      unsigned int	size;
      int		offset;

      /*
       * Use the GNUstep additional function to set the instance
       * variable directly.
       * FIXME - need some way to do this for libFoundation and
       * Foundation based systems.
       */
      if (GSObjCFindVariable(_source, nam, &type, &size, &offset))
	{
	  GSObjCSetVariable(_source, offset, size, (void*)&_destination); 
	}
    }
#endif
}

@end /* IMOutletConnector */

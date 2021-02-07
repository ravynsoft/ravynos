/*
   IMLoading.m

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

#ifndef GNUSTEP
#include <Foundation/Foundation.h>
#else
#include <Foundation/NSDictionary.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSPathUtilities.h>
#endif

#include "GNUstepGUI/GMArchiver.h"
#include "GNUstepGUI/IMLoading.h"
#include "GNUstepGUI/IMCustomObject.h"

/* For awakeFromNib */
#include <AppKit/NSNibLoading.h>

void __dummy_IMLoading_functionForLinking()
{
}

@implementation GMModel

id _nibOwner = nil;
BOOL _fileOwnerDecoded = NO;

+ (void)initialize
{
  /* Force linking of AppKit GModel categories */
  extern void __dummy_GMAppKit_functionForLinking();
  __dummy_GMAppKit_functionForLinking ();
}

+ (BOOL)loadIMFile:(NSString*)path owner:(id)owner
{
  return [self loadIMFile:path owner:owner bundle:[NSBundle mainBundle]];
}

+ (BOOL)loadIMFile:(NSString*)path owner:(id)owner bundle:(NSBundle*)mainBundle
{
#ifdef GNU_GUI_LIBRARY
  NSString* resourcePath = [mainBundle resourcePath];
  GMUnarchiver* unarchiver;
  id previousNibOwner = _nibOwner;
  GMModel* decoded;

  if (![[path pathExtension] isEqualToString:@"gmodel"])
    {
      path = [path stringByAppendingPathExtension:@"gmodel"];
    }

  /* First check to see if path is an absolute path; if so try to load the
     pointed file. */
  if ([path isAbsolutePath]) {
    if (![[NSFileManager defaultManager] fileExistsAtPath:path]) 
      {
	/* The file is an absolute path name but the model file
           doesn't exist. */
	return NO;
      }
  }
  else 
    {
      /* The path is a relative path; search it in the current bundle. */
      NSString *abspath = [resourcePath stringByAppendingPathComponent:path];
      if (![[NSFileManager defaultManager] fileExistsAtPath:abspath]) 
	{
	  NSArray  *paths;
	  NSString *root;
	  
	  paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
                                                      NSSystemDomainMask, YES);
          if ((paths != nil) && ([paths count] > 0))
	    {
              root = [paths objectAtIndex: 0];
	      root = [root stringByAppendingPathComponent:@"Model"];
	      abspath = [root stringByAppendingPathComponent:path];
	      if (![[NSFileManager defaultManager] fileExistsAtPath:abspath])
		{
		  return NO;
		}
	    }
	  else
	    {
	      return NO;
	    }
      }
      path = abspath;
    }
  
  NSLog (@"loading model file %@...", path);
  unarchiver = [GMUnarchiver unarchiverWithContentsOfFile:path];
  
  if (!unarchiver)
    {
      return NO;
    }

  /* Set the _nibOwner to `owner' so that the first decoded custom object
     replaces itself with `owner'. Also set _fileOwnerDecoded so that the
     first custom object knows it's the first. */
  _nibOwner = owner;
  _fileOwnerDecoded = NO;

  decoded = [unarchiver decodeObjectWithName:@"RootObject"];
  [decoded _makeConnections];
  
  /* Restore the previous nib owner. We do this because loadIMFile:owner: can
     be invoked recursively. */
  _nibOwner = previousNibOwner;

#endif
  return YES;
}

- (void)_makeConnections
{
  int i, count;

#ifdef __APPLE__
  [connections makeObjectsPerformSelector:@selector(establishConnection)];
#else
  [connections makeObjectsPerform:@selector(establishConnection)];
#endif

  /* Send the -awakeFromModel method */
  for (i = 0, count = [objects count]; i < count; i++) {
    id object = [[objects objectAtIndex:i] nibInstantiate];

    // If the object responds to the awakeFromModel
    // method, send that message.  If it doesn't then
    // check for the awakeFromNib selector and send that.
    // This ensures compatibility w/ apps ported from OPENSTEP.
    if ([object respondsToSelector:@selector(awakeFromModel)])
      {
	[object awakeFromModel];
      }
    else
      if ([object respondsToSelector:@selector(awakeFromNib)])
	{
	  [object awakeFromNib];
	}
  }
}

- (void)dealloc
{
  [objects release];
  [connections release];
  [super dealloc];
}

- (void)_setObjects:_objects connections:_connections
{
  objects = [_objects retain];
  connections = [_connections retain];
}

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [archiver encodeObject:objects withName:@"Objects"];
  [archiver encodeObject:connections withName:@"Connections"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  objects = [[unarchiver decodeObjectWithName:@"Objects"] retain];
  connections = [[unarchiver decodeObjectWithName:@"Connections"] retain];
  return self;
}

- (NSArray *) objects
{
  return objects;
}

- (NSArray *) connections
{
  return connections;
}

@end /* GMModel */

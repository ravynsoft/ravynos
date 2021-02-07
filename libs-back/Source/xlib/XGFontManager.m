/*
   XGFontManager.m

   NSFontManager helper for GNUstep GUI X/GPS Backend

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@bx.logicnet.ro>
   Date: February 1997
   A completely rewritten version of the original source of Scott Christley.
   Modified:  Fred Kiefer <FredKiefer@gmx.de>
   Date: Febuary 2000
   Added some X calls and changed the overall structure
 
   This file is part of the GNUstep GUI X/GPS Library.

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

#include <config.h>
#include <stdio.h>

#include <GNUstepGUI/GSFontInfo.h>
#include <Foundation/NSArchiver.h>
#include <Foundation/NSBundle.h>
#include <Foundation/NSException.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSPathUtilities.h>
#include <Foundation/NSProcessInfo.h>
#include <Foundation/NSTask.h>
#include <Foundation/NSValue.h>
#include "xlib/XGContext.h"
#include "xlib/XGPrivate.h"
#include "x11/XGServer.h"

#define stringify_it(X) #X

@interface NSBundle (Private)
// Maybe we should rather use -pathForAuxiliaryExecutable:?
+ (NSString *) _absolutePathOfExecutable: (NSString *)path;
@end

static NSMutableDictionary* creationDictionary;

// Fills in the size into an creation string to make it an X font name
NSString *XGXFontName(NSString *fontName, float size)
{
  NSString *creationName = [creationDictionary objectForKey: fontName];

  if (creationName != nil)
    return [NSString stringWithFormat: creationName, (int)size];
  else
    return nil;
}

@implementation XGFontEnumerator

static NSDictionary	*cache;

static NSString* 
cache_name()
{
  static NSString *cacheName = nil;

  if (cacheName == nil)
    {
      NSFileManager *mgr;
      BOOL flag;
      Display *dpy = [XGServer xDisplay];
      NSString *file_name;
      NSArray *paths;
      NSString *path = nil;

      file_name = XGFontCacheName(dpy);

      paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
                                                  NSUserDomainMask, YES);
      if ((paths != nil) && ([paths count] > 0))
        {
          path = [paths objectAtIndex: 0];
        }
      /*
       * If standard search paths are not set up, try a default location.
       */
      if (path == nil)
	{
	  path = [[NSHomeDirectory() stringByAppendingPathComponent:
	    @"GNUstep"] stringByAppendingPathComponent: @"Library"];
	}

      mgr = [NSFileManager defaultManager];
      if ([mgr fileExistsAtPath: path isDirectory: &flag] == NO || flag == NO)
	{
	  NSLog(@"Library directory '%@' not available!", path);
	  return nil;
	}
      path = [path stringByAppendingPathComponent: @"Fonts"];
      if ([mgr fileExistsAtPath: path] == NO)
	{
	  [mgr createDirectoryAtPath: path attributes: nil];
	}
      if ([mgr fileExistsAtPath: path isDirectory: &flag] == NO || flag == NO)
	{
	  NSLog(@"Fonts directory '%@' not available!", path);
	  return nil;
	}
      path = [path stringByAppendingPathComponent: @"Cache"];
      if ([mgr fileExistsAtPath: path] == NO)
	{
	  [mgr createDirectoryAtPath: path attributes: nil];
	}
      if ([mgr fileExistsAtPath: path isDirectory: &flag] == NO || flag == NO)
	{
	  NSLog(@"Fonts directory '%@' not available!", path);
	  return nil;
	}
      cacheName = [path stringByAppendingPathComponent: file_name];
      RETAIN(cacheName);
    }

  return cacheName;
}

static BOOL
load_cache(NSString *cacheName, BOOL async)
{
  NSNumber	*v;
  id		o;

  NS_DURING
    {
      o = [NSUnarchiver unarchiveObjectWithFile: cacheName];
    }
  NS_HANDLER
    {
      NSLog(@"Exception while attempting to load font cache file %@ - %@: %@",
	    cacheName, [localException name], [localException reason]);
      o = nil;
    }
  NS_ENDHANDLER

  if ((o == nil)
    || ((v = [o objectForKey: @"Version"]) == nil)
    || ([v intValue] != 3))
    {
      NSString *file_name = [cacheName lastPathComponent];
      NSString *path;
      NSTask *task;
   
      if (async == NO)
	{
	  NSLog(@"No font cache available - building new one - this may "
	    @"take several seconds (or minutes on a slow machine with "
	    @"lots of fonts)");
	}
      path = [NSBundle _absolutePathOfExecutable: @"font_cacher"];
      if (path == nil)
	{
	  NSLog(@"Could not find font_cacher program. Please run manually");
	  return NO;
	}
      NSLog(@"Running %@", path);
      task = [NSTask launchedTaskWithLaunchPath: path
	arguments: [NSArray arrayWithObject: file_name]];
      if (task == nil || async == YES)
	{
	  return NO;
	}
      [task waitUntilExit];
      o = [NSUnarchiver unarchiveObjectWithFile: cacheName];
      if (o == nil)
	{
	  NSLog(@"Error - font cache doesn't exist");
	  return NO;
	}
    }
  else
    {
      // Ensure archive is written in latest format.
      // No longer needed as NSString coding format did not change in 
      // the last two years. 
      //[NSArchiver archiveRootObject: o toFile: cacheName];
    }

  ASSIGN(cache, o);
  return YES;
}

- (void) enumerateFontsAndFamilies
{
  if (cache == nil)
    {
      if (load_cache(cache_name(), NO))
        {
	  allFontNames = RETAIN([[cache objectForKey: @"AllFontNames"] allObjects]);
	  allFontFamilies = [cache objectForKey: @"AllFontFamilies"];
	  // This dictionary stores the XLFD for each font
	  creationDictionary = [cache objectForKey: @"CreationDictionary"];
	}
    }
}

@end

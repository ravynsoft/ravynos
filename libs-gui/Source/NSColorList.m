/** <title>NSColorList</title>

   <abstract>Manage named lists of NSColors.</abstract>

   Copyright (C) 1996, 2000 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: January 2000
   
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

#import "config.h"
#import <Foundation/NSNotification.h>
#import <Foundation/NSNotificationQueue.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSArchiver.h>
#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSScanner.h>
#import <Foundation/NSString.h>

#import "AppKit/NSColorList.h"
#import "AppKit/NSColor.h"
#import "AppKit/AppKitExceptions.h"

// The list of available color lists is cached and re-loaded only
// after a time.
static NSMutableArray *_availableColorLists = nil;
static NSLock *_colorListLock = nil;

static NSColorList *defaultSystemColorList = nil;
static NSColorList *themeColorList = nil;

@interface NSColorList (GNUstepPrivate)

/* Loads the available color lists from standard directories.<br />
 * If called with a nil argument, this will check to see if the
 * lists have already been loaded, and only load if they haven't been.
 */
+ (void) _loadAvailableColorLists: (NSNotification*)aNotification;

/* Set the default system color list ... to be used if no system color
 * list has been loaded from file.  This should always be the last of
 * the array of available color lists even though it has noit been
 * written to file.
 */
+ (void) _setDefaultSystemColorList: (NSColorList*)aList;

/* Set the theme system color list ... if this is not nil, it is placed
 * at the start of the array of available lists and is used as the system
 * color list.
 */
+ (void) _setThemeSystemColorList: (NSColorList*)aList;

@end

@implementation NSColorList

//
// Class methods
//
+ (void) initialize
{
  if (self == [NSColorList class])
    {
      [self setVersion: 2];
      _colorListLock = (NSLock *)[NSRecursiveLock new];
    }
}

/*
 * Getting All Color Lists
 */
+ (NSArray*) availableColorLists
{
  NSArray	*a;

  // Serialize access to color list
  [_colorListLock lock];
  [NSColor whiteColor]; // NB This ensures that the System color list is defined
  [NSColorList _loadAvailableColorLists: nil];
  a =  [NSArray arrayWithArray: _availableColorLists];
  [_colorListLock unlock];

  return a;
}

/*
 * Getting a Color List by Name
 */
+ (NSColorList *) colorListNamed: (NSString *)name
{
  NSColorList  *r;
  NSEnumerator *e;

  // Serialize access to color list
  [_colorListLock lock];

  [NSColorList _loadAvailableColorLists: nil];
  e = [_availableColorLists objectEnumerator];
  
  while ((r = (NSColorList *)[e nextObject]) != nil) 
    {
      if ([[r name] isEqualToString: name])
	{
	  RETAIN(r);
	  break;
	}
    }
  
  [_colorListLock unlock];

  return AUTORELEASE(r);
}


/*
 * Instance methods
 */

/*
 * Private method for reading text color list files, with following format:
 * first line  =  <#/colors>
 * each subsequent line describes a color as <int float+ string>
 * the first int describes the method (ARGB, etc.), the floats
 * provide its arguments (e.g., r, g, b, alpha), and string is name.
 */
- (BOOL) _readTextColorFile: (NSString *) filepath
{
  int nColors;
  int method;
  float r;
  float g;
  float b;
  float alpha;
  NSString *cname;
  int i;
  BOOL st;
  NSColor *color;
  NSCharacterSet *newlineSet =
    [NSCharacterSet characterSetWithCharactersInString: @"\n"];
  NSScanner *scanner =
    [NSScanner scannerWithString:
                 [NSString stringWithContentsOfFile: _fullFileName]];

  if ([scanner scanInt: &nColors] == NO)
    {
      NSLog(@"Unable to read color file at \"%@\" -- unknown format.",
            _fullFileName);
      return NO;
    }

  for (i = 0; i < nColors; i++)
    {
      if ([scanner scanInt: &method] == NO)
        {
          NSLog(@"Unable to read color file at \"%@\" -- unknown format.",
                _fullFileName);
          break;
        }
      //FIXME- replace this by switch on method to different
      //       NSColor initializers
      if (method != 0)
        {
          NSLog(@"Unable to read color file at \"%@\" -- only RGBA form "
                @"supported.", _fullFileName);
          break;
        }
      st = [scanner scanFloat: &r];
      st = st && [scanner scanFloat: &g];
      st = st && [scanner scanFloat: &b];
      st = st && [scanner scanFloat: &alpha];
      st = st && [scanner scanUpToCharactersFromSet: newlineSet
                                         intoString: &cname];
      if (st == NO)
        {
          NSLog(@"Unable to read color file at \"%@\" -- unknown format.",
                _fullFileName);
          break;
        }
      color = [NSColor colorWithCalibratedRed: r green: g blue: b alpha: alpha];
      [self insertColor: color key: cname atIndex: i];
    }

  return i == nColors;
}

- (id) initWithName: (NSString *)name
{
  return [self initWithName: name
		   fromFile: nil];
}

- (id) initWithName: (NSString *)name
	   fromFile: (NSString *)path
{
  NSColorList *cl;
  BOOL        could_load = NO; 

  ASSIGN (_name, name);

  if (path != nil)
    {
      BOOL isDir = NO;
      // previously impl wrongly expected directory containing color file
      // rather than color file; we support this for apps that rely on it
      if (([[NSFileManager defaultManager] fileExistsAtPath: path
                                                isDirectory: &isDir] == NO)
          || (isDir == YES))
        {
          NSLog(@"NSColorList -initWithName:fromFile: warning: excluding "
                @"filename from path (%@) is deprecated.", path);
          ASSIGN (_fullFileName, [[path stringByAppendingPathComponent: name] 
                                   stringByAppendingPathExtension: @"clr"]);
        }
      else
        {
          ASSIGN (_fullFileName, path);
        }
  
      // Unarchive the color list
      
      // TODO [Optm]: Rewrite to initialize directly without unarchiving 
      // in another object
      NS_DURING
        {
          cl =
            (NSColorList*)[NSUnarchiver unarchiveObjectWithFile: _fullFileName];
        }
      NS_HANDLER
        {
          cl = nil;
        }
      NS_ENDHANDLER ;

      if (cl && [cl isKindOfClass: [NSColorList class]])
	{
	  could_load = YES;

	  _is_editable = [[NSFileManager defaultManager] 
	    isWritableFileAtPath: _fullFileName];
	  
	  ASSIGN(_colorDictionary, [NSMutableDictionary 
	    dictionaryWithDictionary: cl->_colorDictionary]);

	  ASSIGN(_orderedColorKeys, [NSMutableArray 
	    arrayWithArray: cl->_orderedColorKeys]);
	}
      else if ([[NSFileManager defaultManager] fileExistsAtPath: path])
        {
          _colorDictionary = [[NSMutableDictionary alloc] init];
          _orderedColorKeys = [[NSMutableArray alloc] init];
	  _is_editable = YES;

          if ([self _readTextColorFile: _fullFileName])
            {
              could_load = YES;
              _is_editable = [[NSFileManager defaultManager] 
                                   isWritableFileAtPath: _fullFileName];
            }
          else
            {
              RELEASE (_colorDictionary);
              RELEASE (_orderedColorKeys);
            }
        }
    }
  
  if (could_load == NO)
    {
      _fullFileName = nil;
      _colorDictionary = [[NSMutableDictionary alloc] init];
      _orderedColorKeys = [[NSMutableArray alloc] init];
      _is_editable = YES;  
    }
  
  return self;
}

- (void) dealloc
{
  RELEASE (_name);
  TEST_RELEASE (_fullFileName);
  RELEASE (_colorDictionary);
  RELEASE (_orderedColorKeys);
  [super dealloc];
}

/*
 * Getting a Color List by Name
 */
- (NSString *) name
{
  return _name;
}

/*
 * Managing Colors by Key
 */
- (NSArray *) allKeys
{
  return [NSArray arrayWithArray: _orderedColorKeys];
}

- (NSColor *) colorWithKey: (NSString *)key
{
  return [_colorDictionary objectForKey: key];
}

- (void) insertColor: (NSColor *)color
		 key: (NSString *)key
	     atIndex: (unsigned)location
{
  NSNotification	*n;

  if (_is_editable == NO)
    [NSException raise: NSColorListNotEditableException
		format: @"Color list cannot be edited\n"];

  [_colorDictionary setObject: color forKey: key];
  [_orderedColorKeys removeObject: key];
  [_orderedColorKeys insertObject: key atIndex: location];
  
  n = [NSNotification notificationWithName: NSColorListDidChangeNotification
				    object: self
				  userInfo: nil];
  [[NSNotificationQueue defaultQueue] 
    enqueueNotification: n
	   postingStyle: NSPostASAP
	   coalesceMask: NSNotificationCoalescingOnSender
	       forModes: nil];
}

- (void) removeColorWithKey: (NSString *)key
{
  NSNotification	*n;

  if (_is_editable == NO)
    [NSException raise: NSColorListNotEditableException
		format: @"Color list cannot be edited\n"];
  
  [_colorDictionary removeObjectForKey: key];
  [_orderedColorKeys removeObject: key];

  n = [NSNotification notificationWithName: NSColorListDidChangeNotification
				    object: self
				  userInfo: nil];
  [[NSNotificationQueue defaultQueue] 
    enqueueNotification: n
	   postingStyle: NSPostASAP
	   coalesceMask: NSNotificationCoalescingOnSender
	       forModes: nil];
}

- (void) setColor: (NSColor *)aColor
	   forKey: (NSString *)key
{
  NSNotification	*n;

  if (_is_editable == NO)
    [NSException raise: NSColorListNotEditableException
		format: @"Color list cannot be edited\n"];
  
  [_colorDictionary setObject: aColor forKey: key];

  if ([_orderedColorKeys containsObject: key] == NO)
    [_orderedColorKeys addObject: key];

  n = [NSNotification notificationWithName: NSColorListDidChangeNotification
				    object: self
				  userInfo: nil];
  [[NSNotificationQueue defaultQueue] 
    enqueueNotification: n
	   postingStyle: NSPostASAP
	   coalesceMask: NSNotificationCoalescingOnSender
	       forModes: nil];
}

/*
 * Editing
 */
- (BOOL) isEditable
{
  return _is_editable;
}

/*
 * Writing and Removing Files
 */
- (BOOL) writeToFile: (NSString *)path
{
  NSFileManager *fm = [NSFileManager defaultManager];
  NSString      *tmpPath;
  BOOL          isDir;
  BOOL          success;
  BOOL          path_is_standard = YES;

  /*
   * We need to initialize before saving, to avoid the new file being 
   * counted as a different list thus making us appear twice
   */
  [NSColorList _loadAvailableColorLists: nil];

  if (path == nil)
    {
      NSArray	*paths;

      // FIXME the standard path for saving color lists?
      paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
	NSUserDomainMask, YES);
      if ([paths count] == 0)
	{
	  NSLog (@"Failed to find Library directory for user");
	  return NO;	// No directory to save to.
	}
      path = [[paths objectAtIndex: 0]
	stringByAppendingPathComponent: @"Colors"]; 
      isDir = YES;
    }
  else
    {
      [fm fileExistsAtPath: path isDirectory: &isDir];
    }

  if (isDir)
    {
      ASSIGN (_fullFileName, [[path stringByAppendingPathComponent: _name] 
        stringByAppendingPathExtension: @"clr"]);
    }
  else // it is a file
    {
      if ([[path pathExtension] isEqual: @"clr"] == YES)
	{
	  ASSIGN (_fullFileName, path);
	}
      else
	{
	  ASSIGN (_fullFileName, [[path stringByDeletingPathExtension]
	    stringByAppendingPathExtension: @"clr"]);
	}
      path = [path stringByDeletingLastPathComponent];
    }

  // Check if the path is a standard path
  if ([[path lastPathComponent] isEqualToString: @"Colors"] == NO)
    {
      path_is_standard = NO;
    }
  else 
    {
      tmpPath = [path stringByDeletingLastPathComponent];
      if (![NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
	NSAllDomainsMask, YES) containsObject: tmpPath])
	{
	  path_is_standard = NO;
	}
    }

  /*
   * If path is standard and it does not exist, try to create it.
   * System standard paths should always be assumed to exist; 
   * this will normally then only try to create user paths.
   */
  if (path_is_standard && ([fm fileExistsAtPath: path] == NO))
    {
      if ([fm createDirectoryAtPath: path 
        withIntermediateDirectories: YES
			 attributes: nil
                              error: NULL])
	{
	  NSLog (@"Created standard directory %@", path);
	}
      else
	{
	  NSLog (@"Failed attempt to create directory %@", path);
	}
    }

  success = [NSArchiver archiveRootObject: self 
				   toFile: _fullFileName];

  if (success && path_is_standard)
    {
      [_colorListLock lock];
      if ([_availableColorLists containsObject: self] == NO)
	[_availableColorLists addObject: self];
      [_colorListLock unlock];      
      return YES;
    }
  
  return success;
}

- (void) removeFile
{
  if (_fullFileName && _is_editable)
    {
      // Remove the file
      [[NSFileManager defaultManager] removeFileAtPath: _fullFileName
					       handler: nil];
      
      // Remove the color list from the global list of colors
      [_colorListLock lock];
      [_availableColorLists removeObject: self];
      [_colorListLock unlock];

      // Reset file name
      _fullFileName = nil;
    }
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      NSMutableArray *colors = [[NSMutableArray alloc] init];
      NSInteger i;

      for (i = 0; i < [_orderedColorKeys count]; i++)
        {
          NSString *name = [_orderedColorKeys objectAtIndex: i];
          [colors insertObject: [_colorDictionary objectForKey: name]
                       atIndex: i];
        }
      [aCoder encodeObject: _name forKey: @"NSName"];
      [aCoder encodeObject: _orderedColorKeys forKey: @"NSKeys"];
      [aCoder encodeObject: colors forKey: @"NSColors"];
      RELEASE(colors);
     }
  else
    {
      [aCoder encodeObject: _name];
      [aCoder encodeObject: _colorDictionary];
      [aCoder encodeObject: _orderedColorKeys];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      NSArray *colors;

      ASSIGN(_name, [aDecoder decodeObjectForKey: @"NSName"]);
      ASSIGN(_orderedColorKeys, [aDecoder decodeObjectForKey: @"NSKeys"]);
      colors = [aDecoder decodeObjectForKey: @"NSColors"];
      _colorDictionary = [[NSMutableDictionary alloc]
                           initWithObjects: colors
                                   forKeys: _orderedColorKeys];
    }
  else
    {
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_name];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_colorDictionary];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_orderedColorKeys];
    }
  return self;
}

@end

@implementation NSColorList (GNUstepPrivate)

+ (void) _loadAvailableColorLists: (NSNotification*)aNotification
{
  [_colorListLock lock];
  /* FIXME ... we should ensure that we get housekeeping notifications */
  if (_availableColorLists != nil && aNotification == nil)
    {
      // Nothing to do ... already loaded
      [_colorListLock unlock];
    }
  else
    {
      NSString			*dir;
      NSString			*file;
      NSEnumerator		*e;
      NSFileManager		*fm = [NSFileManager defaultManager];
      NSDirectoryEnumerator	*de;
      NSColorList		*newList;

      if (_availableColorLists == nil)
	{
	  // Create the global array of color lists
	  _availableColorLists = [[NSMutableArray alloc] init];
	}
      else
	{
	  [_availableColorLists removeAllObjects];
	}
      
      /*
       * Keep any pre-loaded system color list.
       */
      if (themeColorList != nil)
	{
	  [_availableColorLists addObject: themeColorList];
	}

      /*
       * Load color lists found in standard paths into the array
       * FIXME: Check exactly where in the directory tree we should scan.
       */
      e = [NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
	NSAllDomainsMask, YES) objectEnumerator];

      while ((dir = (NSString *)[e nextObject])) 
	{
	  BOOL flag;

	  dir = [dir stringByAppendingPathComponent: @"Colors"];
	  if (![fm fileExistsAtPath: dir isDirectory: &flag] || !flag)
	    {
	      // Only process existing directories
	      continue;
	    }

	  de = [fm enumeratorAtPath: dir];
	  while ((file = [de nextObject])) 
	    {
	      if ([[file pathExtension] isEqualToString: @"clr"])
		{
		  NSString	*name;

		  name = [file stringByDeletingPathExtension];
		  newList = [[NSColorList alloc] initWithName: name
		    fromFile: [dir stringByAppendingPathComponent: file]];
		  [_availableColorLists addObject: newList];
		  RELEASE(newList);
		}
	    }
	}  

      if (defaultSystemColorList != nil)
        {
	  [_availableColorLists addObject: defaultSystemColorList];
	}
      [_colorListLock unlock];
    }
}

+ (void) _setDefaultSystemColorList: (NSColorList*)aList
{
  [_colorListLock lock];
  if (defaultSystemColorList != aList)
    {
      if (defaultSystemColorList != nil
        && [_availableColorLists lastObject] == defaultSystemColorList)
	{
	  [_availableColorLists removeLastObject];
	}
      ASSIGN(defaultSystemColorList, aList);
      [_availableColorLists addObject: aList];
    }
  [_colorListLock unlock];
}

+ (void) _setThemeSystemColorList: (NSColorList*)aList
{
  [_colorListLock lock];
  if (themeColorList != aList)
    {
      if (themeColorList != nil && [_availableColorLists count] > 0
        && [_availableColorLists objectAtIndex: 0] == themeColorList)
	{
	  [_availableColorLists removeObjectAtIndex: 0];
	}
      ASSIGN(themeColorList, aList);
      [_availableColorLists insertObject: aList atIndex: 0];
    }
  [_colorListLock unlock];
}

@end


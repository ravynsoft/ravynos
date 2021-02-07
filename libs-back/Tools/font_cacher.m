/*
   Font cacher for GNUstep GUI X/GPS Backend

   Copyright (C) 2000 Free Software Foundation, Inc.

   Author:  Fred Kiefer <FredKiefer@gmx.de> and Richard Frith-Macdonald
   Date: Febuary 2000
 
   This file is part of the GNUstep GUI X/GPS Library.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 3
   of the License, or (at your option) any later version.
    
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public  
   License along with this library; see the file COPYING.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <Foundation/Foundation.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "xlib/XGPrivate.h"

#define	stringify_it(X)	#X
#define	makever(X) stringify_it(X)

@interface XFontCacher : NSObject
{
  NSMutableSet		*allFontNames;
  NSMutableDictionary	*allFontFamilies;
  NSMutableDictionary	*creationDictionary;
  NSMutableDictionary	*xFontDictionary;
  NSMutableSet		*knownFonts;
  Display		*dpy;
}

- (NSString*) faceNameFromParts: (NSArray*) parts;
- (NSString*) creationNameFromParts: (NSArray*) parts;
- (NSString*) getPathFor: (NSString*) display;
- (BOOL) fontLoop;
- (BOOL) processPattern: (const char *) pattern;
- (void) processFont: (char*) name;
- (void) sortResults;
- (void) writeCacheTo: (NSString*)path;
@end

@implementation XFontCacher

- (id) init
{
  allFontNames = RETAIN([NSMutableSet setWithCapacity: 1000]);
  allFontFamilies = RETAIN([NSMutableDictionary dictionaryWithCapacity: 1000]);
  creationDictionary = RETAIN([NSMutableDictionary dictionaryWithCapacity: 1000]);
  xFontDictionary = RETAIN([NSMutableDictionary dictionaryWithCapacity: 1000]);
  knownFonts = RETAIN([NSMutableSet setWithCapacity: 1000]);

  return self;
}

- (void) dealloc
{
  RELEASE(allFontNames);
  RELEASE(allFontFamilies);
  RELEASE(creationDictionary);
  RELEASE(xFontDictionary);
  RELEASE(knownFonts);

  if (dpy)
    XCloseDisplay(dpy);

  [super dealloc];
}

/*
 * Find a suitable type face name for a full X font name, which 
 * has already been split into parts seperated by -
 * -sony-fixed-medium-r-normal--16-150-75-75-c-80-iso8859-1
 * becomes Normal
 */
- (NSString*) faceNameFromParts: (NSArray*) parts
{
  NSString *faceName;
  NSString *face = [[parts objectAtIndex: 3] capitalizedString];
  NSString *slant = [[parts objectAtIndex: 4] capitalizedString];
  NSString *weight = [[parts objectAtIndex: 5] capitalizedString];
  NSString *add = [[parts objectAtIndex: 6] capitalizedString];
 
  if ([face length] == 0 || [face isEqualToString: @"Medium"])
    faceName = @"";
  else
    faceName = face;

  if ([slant isEqualToString: @"I"])
    faceName = [NSString stringWithFormat: @"%@%@", faceName, @"Italic"];
  else if ([slant isEqualToString: @"O"])
    faceName = [NSString stringWithFormat: @"%@%@", faceName, @"Oblique"];

  if ([weight length] != 0 && ![weight isEqualToString: @"Normal"])
    {
      if ([faceName length] != 0)
	faceName = [NSString stringWithFormat: @"%@-%@", faceName, weight];
      else
	faceName = weight;
    }

  if ([add length] != 0)
    {
      if ([faceName length] != 0)
	faceName = [NSString stringWithFormat: @"%@-%@", faceName, add];
      else
	faceName = add;
    }

  if ([faceName length] == 0)
    faceName = @"Normal";
  
  return faceName;
}

/*
 * Build up an X font creation string
 * -sony-fixed-medium-r-normal--16-150-75-75-c-80-iso8859-1
 * becomes 
 * -*-fixed-medium-r-normal--%d-*-*-*-c-*-iso8859-1
*/
- (NSString*) creationNameFromParts: (NSArray*) parts
{
  NSString *creationName;
  
  creationName = [NSString stringWithFormat: 
			     @"-%@-%@-%@-%@-%@-%@-%@-%@-%@-%@-%@-%@-%@-%@", 
			   @"*", 
			   [parts objectAtIndex: 2], 
			   [parts objectAtIndex: 3], 
			   [parts objectAtIndex: 4], 
			   [parts objectAtIndex: 5], 
			   [parts objectAtIndex: 6],
			   @"%d",//[parts objectAtIndex: 7], 
			   @"*", // place holder for integer size (points *10)
			   @"*",//[parts objectAtIndex: 9], 
			   @"*",//[parts objectAtIndex: 10], 
			   [parts objectAtIndex: 11], 
			   @"*",//[parts objectAtIndex: 12], 
			   [parts objectAtIndex: 13], 
			   [parts objectAtIndex: 14]];
  
  return creationName;
}

- (NSString*) getPathFor: (NSString*) display
{
  NSArray		*paths;
  NSFileManager		*mgr;
  NSString		*path;
  BOOL			flag;

  if (display != nil)
    dpy = XOpenDisplay([display cString]);
  else
    dpy = XOpenDisplay(NULL);
  if (dpy == 0)
    {
      NSLog(@"Unable to open X display - no font information available");
      return nil;
    }
  /* Standardize the name */
  display = XGFontCacheName(dpy);

  paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
                                              NSUserDomainMask, YES);
  if ((paths != nil) && ([paths count] > 0))
    {
      path = [paths objectAtIndex: 0];
    }
  else 
    {
      NSLog(@"  No valid path for cached information exists. You ");
      NSLog(@"  should create ~/GNUstep/Library by hand");
      return nil;
    }

  mgr = [NSFileManager defaultManager];
  if ([mgr fileExistsAtPath: path] == NO)
    {
      NSError *err;
      BOOL r;
      r = [mgr createDirectoryAtPath: path
	       withIntermediateDirectories: YES
			  attributes: nil
			       error: &err];
      if (r == NO)
	NSLog(@"font_cacher: Library directory creation error: %@", err);
    }
  if ([mgr fileExistsAtPath: path isDirectory: &flag] == NO || flag == NO)
    {
      NSLog(@"font_cacher: Library directory '%@' not available!", path);
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

  return [path stringByAppendingPathComponent: display];
}

- (BOOL) fontLoop
{
  NSUserDefaults *defs;
  const char *pattern = "*";
  BOOL result;

  defs = [NSUserDefaults standardUserDefaults];
  if (defs == nil)
    NSLog(@"Unable to access defaults database!");
  else
    {
      NSString *font_mask;
      
      font_mask = [defs stringForKey: @"GSFontMask"];
      if ((font_mask != nil) && [font_mask length])
	// only fonts matching the supplied mask will be cached
	pattern = [font_mask lossyCString];
    }

  // In a next step we allow multiple font masks
  result = [self processPattern: pattern];

  // No longer needed
  DESTROY(knownFonts);

  return result;
}

- (BOOL) processPattern: (const char *) pattern
{
  NSAutoreleasePool     *loopPool;
  int			nnames = 10000;
  int			available = nnames+1;
  int			i;
  char			**fonts;

  /* Get list of all fonts */
  for (;;) 
    {
      fonts = XListFonts(dpy, pattern, nnames, &available);
      if (fonts == NULL || available < nnames)
	break;
      /* There are more fonts then we expected, so just start 
	 again and request more */
      XFreeFontNames(fonts);
      nnames = available * 2;
    }

  if (fonts == NULL) 
    {
      NSLog(@"No X fonts found for pattern %s", pattern);
      return NO;
    }

  NSDebugFLog(@"Fonts loaded, now the loop.  Available: %d", available);

  loopPool = [NSAutoreleasePool new];

  for (i = 0; i < available; i++) 
    {
      char *name = fonts[i];
      NSDebugLLog(@"Fonts", @"%s", name);

      NS_DURING
        [self processFont: name];
      NS_HANDLER
	NSLog(@"Problem during processing of font %s", name);
      NS_ENDHANDLER

      if (i && i % 500 == 0)
	{
	  NSDebugLog(@"Finished %d", i);
	  RELEASE (loopPool);
	  loopPool = [NSAutoreleasePool new];
	}
    }
  RELEASE (loopPool);
  XFreeFontNames(fonts);
  NSDebugLog(@"Finished loop");

  return YES;
}

- (void) processFont: (char*) name 
{
  NSString	*alias = [[NSString stringWithCString: name] lowercaseString];
  NSString	*fontName;
      
  /*
   * First time we find this font. A font may be found more than 
   * once, when the XFontPath has the same directory twice.
   * Somehow this is the case on my machine.
   */
  if ([xFontDictionary objectForKey: alias] == nil)
    {
      XFontStruct	*info = XLoadQueryFont(dpy, name);
      NSString	*family;
      NSString	*baseName;
      NSString	*face;
      NSString	*creationName;
      NSArray	*parts;
	  
      if (info == 0) 
        {
	  NSDebugLog(@"No information for font %s", name);
	  return;
	}
      
      fontName = XGFontName(dpy, info);
      if (fontName == nil)
        {
	  NSDebugLog(@"No font Name in info for font %s", name);
	  XFreeFont(dpy, info);      
	  return;
	}

      if ([alias isEqualToString: fontName] == NO)
        {
	  // We have got an alias name, store it
	  [xFontDictionary setObject: fontName forKey: alias];
	}
      
      // Use the normal function to keep the names consistent
      family = XGFontFamily(dpy, info);	  

      parts = [fontName componentsSeparatedByString: @"-"];
      if ([parts count] == 15)
        {
	  face = [self faceNameFromParts: parts];
	  if ([face length] == 0 || [face isEqualToString: @"Normal"])
	    {
	      baseName = family;
	    }
	  else
	    baseName = [NSString stringWithFormat: @"%@-%@", family, face];
	  
	  creationName = [self creationNameFromParts: parts];
	}
      else
        {
	  baseName = [fontName capitalizedString];
	  face = @"Normal";
	  creationName = fontName;
	}
      
      // Store the alias to baseName
      [xFontDictionary setObject: baseName forKey: fontName];
      // it might already have been found with another size
      if ([knownFonts member: baseName] == nil)
        {
	  int		weight;
	  NSFontTraitMask	traits;
	  NSMutableArray	*fontDefs;
	  NSMutableArray	*fontDef;
	  
	  // the first time we find that base font.
	  
	  // Store the font name
	  [knownFonts addObject: baseName];
	  [allFontNames addObject: baseName];
	  [creationDictionary setObject: creationName forKey: baseName];
	      
	  weight = XGWeightOfFont(dpy, info);
	  traits = XGTraitsOfFont(dpy, info);
	  
	  // Store the family name and information
	  fontDefs = [allFontFamilies objectForKey: family];
	  if (fontDefs == nil)
	    {
	      fontDefs = [NSMutableArray array];
	      [allFontFamilies setObject: fontDefs forKey: family];
	    }
	  // Fill the structure for the font
	  fontDef = [NSMutableArray arrayWithCapacity: 4];  
	  [fontDef addObject: baseName];
	  [fontDef addObject: face];
	  [fontDef addObject: [NSNumber numberWithInt: weight]];
	  [fontDef addObject: [NSNumber numberWithUnsignedInt: traits]];
	  // Add to the family information
	  [fontDefs addObject: fontDef];
	}
      // Release the font
      XFreeFont(dpy, info);
    }
}

static NSComparisonResult fontDefSorter(id e1, id e2, void *context)
{
  // This is not exactly the order the OpenStep specification states.
  NSArray *el1 = (NSArray *)e1;
  NSArray *el2 = (NSArray *)e2;
  NSFontTraitMask t1 = [[el1 objectAtIndex: 3] unsignedIntValue];
  NSFontTraitMask t2 = [[el2 objectAtIndex: 3] unsignedIntValue];
  int w1 = [[el1 objectAtIndex: 2] intValue];
  int w2 = [[el2 objectAtIndex: 2] intValue];

  if (t1 < t2)
    return NSOrderedAscending;
  else if (t2 < t1)
    return NSOrderedDescending;
  else if (w1 < w2)
    return NSOrderedAscending;
  else if (w2 < w1)
    return NSOrderedDescending;
      
  return NSOrderedSame;
}

- (void) sortResults
{
  NSEnumerator		*enumerator;
  id			key;

  // Now sort the fonts of each family
  enumerator = [allFontFamilies keyEnumerator];
  while ((key = [enumerator nextObject])) 
    {
      NSMutableArray *fontDefs = [allFontFamilies objectForKey: key];
      
      [fontDefs sortUsingFunction: fontDefSorter context: nil];
    }
  

  // define a creation string for alias names
  enumerator = [xFontDictionary keyEnumerator];
  while ((key = [enumerator nextObject])) 
    {
      id name = key;
      id creationName;
      
      while ((name != nil)
	&& (creationName = [creationDictionary objectForKey: name]) == nil)
	{
	  name = [xFontDictionary objectForKey: name];
	}

      if (creationName != nil)
	{
	  [creationDictionary setObject: creationName forKey: key];
	}
    }
  // No longer needed
  DESTROY(xFontDictionary);
}

- (void) writeCacheTo: (NSString*)path
{
  NSData		*data;
  NSMutableDictionary   *cache = [NSMutableDictionary dictionaryWithCapacity: 4];

  [cache setObject: [NSNumber numberWithInt: 3] forKey: @"Version"];
  [cache setObject: allFontNames forKey: @"AllFontNames"];
  [cache setObject: allFontFamilies forKey: @"AllFontFamilies"];
  [cache setObject: creationDictionary forKey: @"CreationDictionary"];
  data = [NSArchiver archivedDataWithRootObject: cache];
  [data writeToFile: path atomically: YES];
}

@end

int
main(int argc, char **argv, char **env)
{
  NSArray		*args;
  NSArray		*paths;
  NSString		*path;
  NSString              *file_name;
  XFontCacher *cacher;
  CREATE_AUTORELEASE_POOL(pool);

#ifdef GS_PASS_ARGUMENTS
  [NSProcessInfo initializeWithArguments: argv count: argc environment: env];
#endif

  args = [[NSProcessInfo processInfo] arguments];
  if ([args containsObject: @"--help"])
    {
      paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
                                                  NSUserDomainMask, YES);
      if ((paths != nil) && ([paths count] > 0))
        path = [paths objectAtIndex: 0];
      else
	path = nil;

      NSLog(@"This tool caches system font information\n");
      if (path != nil)
	NSLog(@"  Information is cached in %@/Fonts/Cache/<XDisplayName>", path); 
      else
	{
	  NSLog(@"  No valid path for cached information exists. You ");
	  NSLog(@"  should create ~/GNUstep/Library by hand");
	}
      RELEASE(pool);
      return 0;
    }
  if ([args containsObject: @"--version"])
    {
      NSLog(@"%@ version %s", [[args objectAtIndex: 0] lastPathComponent],
	makever(GNUSTEP_VERSION));
      RELEASE(pool);
      return 0;
    }

  if ([args count] > 1)
    file_name = [args objectAtIndex: 1];
  else
    file_name = nil;

  cacher = [[XFontCacher alloc] init];
  path = [cacher getPathFor: file_name];
  if (path == nil)
    {
      RELEASE(pool);
      return 1;
    }

  if (![cacher fontLoop])
    {
      RELEASE(pool);
      return 2;
    }

  [cacher sortResults];
  NS_DURING
    [cacher writeCacheTo: path];
  NS_HANDLER
    NSLog(@"Problem during writing of font cache");
  NS_ENDHANDLER
  
  RELEASE(cacher);
  RELEASE(pool);
  return 0;
}


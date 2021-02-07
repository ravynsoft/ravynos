/** <title>NSHelpManager</title>

   <abstract>NSHelpManager is the class responsible for managing context help
   for the application, and its mapping to the graphic elements.</abstract>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Pedro Ivo Andrade Tavares <ptavares@iname.com>
   Date: September 1999
   
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

#import <Foundation/NSArchiver.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSData.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSString.h>
#import <Foundation/NSUserDefaults.h>
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSWorkspace.h"
#import "AppKit/NSFileWrapper.h"
#import "AppKit/NSHelpManager.h"
#import "AppKit/NSHelpPanel.h"
#import "AppKit/NSHelpPanel.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSTextStorage.h"

#import "GNUstepGUI/GSHelpManagerPanel.h"

@implementation NSBundle (NSHelpManager)

- (NSString *) pathForHelpResource: (NSString *)fileName
{
  NSFileManager *fm = [NSFileManager defaultManager];
  NSMutableArray *array = [NSMutableArray array];
  NSArray *languages;
  NSString *rootPath = [self bundlePath];
  NSString *primary;
  NSString *language;
  NSEnumerator *enumerator;
  
  languages = [[NSUserDefaults standardUserDefaults]
    stringArrayForKey: @"NSLanguages"];
  primary = [rootPath stringByAppendingPathComponent: @"Resources"];
  
  enumerator = [languages objectEnumerator];
  
  while ((language = [enumerator nextObject]))
    {
      NSString *langDir = [NSString stringWithFormat: @"%@.lproj", language];

      [array addObject: [primary stringByAppendingPathComponent: langDir]];
    }
  
  [array addObject: primary];
  
  primary = rootPath;
  
  enumerator = [languages objectEnumerator];
  
  while ((language = [enumerator nextObject]))
    {
      NSString *langDir = [NSString stringWithFormat: @"%@.lproj", language];

      [array addObject: [primary stringByAppendingPathComponent: langDir]];
    }
  
  [array addObject: primary];
  
  enumerator = [array objectEnumerator];
  
  while ((rootPath = [enumerator nextObject]) != nil)
    {
      NSString *helpDir;
      NSString *helpPath;
      BOOL isdir;
      
      helpPath = [rootPath stringByAppendingPathComponent: fileName];
      
      if ([fm fileExistsAtPath: helpPath])
        {
          return helpPath;
        }
      
      helpDir = [rootPath stringByAppendingPathComponent: @"Help"];
      
      if ([fm fileExistsAtPath: helpDir isDirectory: & isdir] && isdir)
        {
          helpPath = [helpDir stringByAppendingPathComponent: fileName];
          
          if ([fm fileExistsAtPath: helpPath])
            {
              return helpPath;
            }
        }
    }
  
  return nil;
}

- (NSAttributedString *) contextHelpForKey: (NSString *)key
{
  NSFileManager *fm = [NSFileManager defaultManager];
  NSString *dictPath = [self pathForResource: @"Help" ofType: @"plist"];
  NSDictionary *contextHelp = nil;
  id helpFile = nil;
  
  if (dictPath && [fm fileExistsAtPath: dictPath])
    {
      contextHelp = [NSDictionary dictionaryWithContentsOfFile: dictPath];
    }
  
  if (contextHelp)
    {
      helpFile = [contextHelp objectForKey: key];
    }
  
  if (helpFile)
    {
      NSData *data = [helpFile objectForKey: @"NSHelpRTFContents"];
      return ((data != nil) ? [NSUnarchiver unarchiveObjectWithData: data]
	: nil);
      
    } 
  else
    {
      helpFile = [self pathForHelpResource: key];

      if (helpFile)
        {
          NSAttributedString *helpstr;

          helpstr = [[NSAttributedString alloc] initWithPath: helpFile
					  documentAttributes: NULL];
          return TEST_AUTORELEASE (helpstr);
        }
    }
      
  return nil;
}

@end

@implementation NSApplication (NSHelpManager)

- (void) showHelp: (id)sender
{
  NSBundle	*mb = [NSBundle mainBundle];
  NSDictionary	*info = [mb infoDictionary];
  NSString	*help = [info objectForKey: @"GSHelpContentsFile"];

  if (help == nil)
    {
      /* If there's no specification, we look for a files named
       * "appname.rtfd" or "appname.rtf"
       */
      help = [info objectForKey: @"NSExecutable"];
    }

  if (help != nil)
    {
      NSString	*file;

      if ([[help pathExtension] length] == 0)
        {
          file = [mb pathForHelpResource:
	    [help stringByAppendingPathExtension: @"rtfd"]];

          if (file == nil)
            {
              file = [mb pathForHelpResource:
		[help stringByAppendingPathExtension: @"rtf"]];
            }
        }
      else
        {
	  file = [mb pathForHelpResource: help];
	}

      if (file != nil)
	{
	  BOOL		result = NO;
	  NSString	*ext = [file pathExtension];
	  NSWorkspace	*ws = [NSWorkspace sharedWorkspace];
	  NSString	*viewer;

	  viewer = [[NSUserDefaults standardUserDefaults]
	    stringForKey: @"GSHelpViewer"];

	  if ([viewer isEqual: @"NSHelpPanel"] == NO)
	    {
	      if ([viewer length] == 0)
		{
	          viewer = [ws getBestAppInRole: @"Viewer" forExtension: ext];
		}
	      if (viewer != nil)
		{
		  result = [[NSWorkspace sharedWorkspace] openFile: file
						   withApplication: viewer];
		}
	    }

	  if (result == NO)
	    {
	      NSHelpPanel	*panel;
	      NSTextView	*tv;
	      id		object = nil;

	      panel = [NSHelpPanel sharedHelpPanel];
	      tv = [(NSScrollView*)[panel contentView] documentView];
	      if (ext == nil  
		|| [ext isEqualToString: @""]	 
		|| [ext isEqualToString: @"txt"] 
		|| [ext isEqualToString: @"text"])
		{
		  object = [NSString stringWithContentsOfFile: file];
		}
	      else if ([ext isEqualToString: @"rtf"])
		{
		  NSData *data = [NSData dataWithContentsOfFile: file];
		  
		  object = [[NSAttributedString alloc] initWithRTF: data
		    documentAttributes: 0];
		  AUTORELEASE (object);
		}
	      else if ([ext isEqualToString: @"rtfd"])
		{
		  NSFileWrapper *wrapper;
		  
		  wrapper = [[NSFileWrapper alloc] initWithPath: file];
		  AUTORELEASE (wrapper);
		  object = [[NSAttributedString alloc]
		    initWithRTFDFileWrapper: wrapper
		    documentAttributes: 0];
		  AUTORELEASE (object);
		}
	      
	      if (object != nil)
		{
		  [[tv textStorage] setAttributedString: object];
		  [tv sizeToFit];
		}
	      [tv setNeedsDisplay: YES];
	      [panel makeKeyAndOrderFront: self];
	      return;
	    }
	}
    }
  
  NSBeep();
}

- (void) activateContextHelpMode: (id)sender
{
  [NSHelpManager setContextHelpModeActive: YES];
}

@end

@implementation NSHelpManager

static NSHelpManager *_gnu_sharedHelpManager = nil;
static BOOL _gnu_contextHelpActive = NO;
static NSCursor *helpCursor = nil;


//
// Class methods
//
+ (NSHelpManager*) sharedHelpManager
{
  if (!_gnu_sharedHelpManager)
    {
      _gnu_sharedHelpManager = [NSHelpManager alloc];
      [_gnu_sharedHelpManager init];
    }
  return _gnu_sharedHelpManager;
}

+ (BOOL) isContextHelpModeActive
{
  return _gnu_contextHelpActive;
}

+ (void) setContextHelpModeActive: (BOOL) flag
{
  if (flag != _gnu_contextHelpActive)
    {
      _gnu_contextHelpActive = flag;
      if (flag)
	{
	  if (helpCursor == nil)
	    {
	      helpCursor = [[NSCursor alloc]
		initWithImage: [NSImage imageNamed: @"common_HelpCursor"]
		hotSpot: NSMakePoint(8, 2)];
	      [helpCursor setOnMouseEntered: NO];
	      [helpCursor setOnMouseExited: NO];
	    }
	  [helpCursor push];
	  [[NSNotificationCenter defaultCenter] 
	    postNotificationName: NSContextHelpModeDidActivateNotification 
	    object: [self sharedHelpManager]];
	}
      else
	{
	  [helpCursor pop];
	  [[NSNotificationCenter defaultCenter] 
	    postNotificationName: NSContextHelpModeDidDeactivateNotification 
	    object: [self sharedHelpManager]];
	}
    }
}

//
// Instance methods
//
- (id) init
{
  contextHelpTopics = NSCreateMapTable(NSObjectMapKeyCallBacks,
				       NSObjectMapValueCallBacks,
				       64);
  return self;
}

- (NSAttributedString*) contextHelpForObject: (id)object
{
  /* Help is kept on the contextHelpTopics NSMapTable, with
     the object for it as the key. 
     
     Help is loaded on demand:
     If it's an NSAttributedString which is stored, then it's already 
     loaded. 
     If it's nil, there's no help for this object, and that's what we return.
     If it's an NSString, it's the path for the help, and we ask NSBundle
     for it. */
  // FIXME: Check this implementation when NSResponders finally store what
  // their context help is.
     
  id hc = NSMapGet(contextHelpTopics, object);
  if (hc)
    {
      if (![hc isKindOfClass: [NSAttributedString class]])
	{
	  hc = [[NSBundle mainBundle] contextHelpForKey: hc];
	  /* We store the retrieved value, or remove the key from
	     the table if nil returns (note that it's OK if the key
	     does not exist already. */
	  if (hc)
	    NSMapInsert(contextHelpTopics, object, hc);
	  else 	    
	    NSMapRemove(contextHelpTopics, object);
	}
    }
  return hc;
}

- (void) removeContextHelpForObject: (id)object
{
  NSMapRemove(contextHelpTopics, object);
}

- (void) setContextHelp: (NSAttributedString *)help forObject: (id)object
{
  NSMapInsert(contextHelpTopics, object, help);
}

/**
 * Deprecated ... do not use.
 * Use -setContextHelp:forObject: instead.
 */
- (void) setContextHelp: (NSAttributedString*) help withObject: (id) object
{
  NSMapInsert(contextHelpTopics, object, help);
}

- (BOOL) showContextHelpForObject: (id)object locationHint: (NSPoint) point
{
  NSAttributedString *contextHelp = [self contextHelpForObject: object];

  if (contextHelp)
    {
      GSHelpManagerPanel *helpPanel;

      // FIXME: We should position the window at point! 
      // runModalForWindow will centre the window.
      helpPanel = [GSHelpManagerPanel sharedHelpManagerPanel];
      [helpPanel setHelpText: contextHelp];
      [NSApp runModalForWindow: helpPanel];
      return YES;
    }
  else 
    return NO;
}

@end


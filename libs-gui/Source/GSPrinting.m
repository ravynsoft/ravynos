/** <title>GSPrinting.m</title>

   <abstract>GSPrinting loads the proper bundle for the printing backend.</abstract>

   Copyright (C) 2004 Free Software Foundation, Inc.

   Author:  Chad Elliott Hardin <cehardin@mac.com>
   Date: June 2004
   
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

#import <Foundation/NSBundle.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSUserDefaults.h>
#import "AppKit/NSPanel.h"
#import "GNUstepGUI/GSPrinting.h"
#import "GNUstepGUI/GSTheme.h"

static NSBundle *printingBundle = nil;


/**
  <unit>
  <heading>Class Description</heading>
  <p>
  GSPrinting is used by all of the NSPrint and the NSPageLayout
  class(es) so that a printing backend bundle can be loaded.  It
  first utilizes NSUserDefaults to find the user's preferred printing
  backend bundle.  It looks for the key GSPrinting.  If the user's
  preferred bundle cannot be loaded, it tries to load any working
  printing bundle.
  </p>
  </unit>
*/ 
@implementation GSPrinting

+(NSBundle*) loadPrintingBundle: (NSString*) bundleName
{
  NSString *path;
  NSEnumerator *libraryPathsEnumerator;
    
  bundleName = [bundleName stringByAppendingString: @".bundle"];	
    
  NSDebugLLog(@"GSPrinting", @"Looking for %@", bundleName);
    
  libraryPathsEnumerator = [NSStandardLibraryPaths() objectEnumerator];
    
  while ((path = [libraryPathsEnumerator nextObject]))
    {
      path = [path stringByAppendingPathComponent: @"Bundles"];
      path = [path stringByAppendingPathComponent: @"GSPrinting"];
      path = [path stringByAppendingPathComponent: bundleName];

      if ([[NSFileManager defaultManager] fileExistsAtPath: path])
        {
          NSBundle *bundle;
	    
          bundle = [NSBundle bundleWithPath: path];
          if ([bundle load] == NO)
            {
              NSDebugLLog(@"GSPrinting", @"Error loading printing bundle at %@", 
                          path);
              return nil;
            }
	    
          //one last check to make sure the principle class can be loaded
          if ([bundle principalClass] == Nil)
            {
              NSDebugLLog(@"GSPrinting", 
                          @"Error loading principal class from printing bundle at %@", 
                          path);
              return nil;
            }
	    
          //if we get here, finally, then everything should be ok, barring 
	    //errors from later in loading resources.
          NSDebugLLog(@"GSPrinting", @"Loaded printing bundle at %@", path);
          return bundle;
        }
    }
    
  NSDebugLLog(@"GSPrinting", @"Unable to find printing bundle %@", bundleName);
  return nil;
}



+(NSBundle*) loadAnyWorkingPrintingBundle
{
  NSBundle *bundle;
    
  if ((bundle = [GSPrinting loadPrintingBundle: @"GSCUPS"]))
    return bundle;

  if ((bundle = [GSPrinting loadPrintingBundle: @"GSWIN32"]))
    return bundle;
		
  if ((bundle = [GSPrinting loadPrintingBundle: @"GSLPR"]))
    return bundle;
	
  return nil;
}



+ (NSBundle*) printingBundle
{
  NSString *defaultBundleName;
  NSBundle *bundle;
    
  if (printingBundle)
    {
      return printingBundle;
    }
    
  NSDebugLLog(@"GSPrinting", @"Bundle has not been loaded.  Loading in progress...");
    
  defaultBundleName = [[NSUserDefaults standardUserDefaults] 
                                       stringForKey: @"GSPrinting"];
    
  /*Which Printing Bundle?*/
  if (defaultBundleName == nil)
    {
      NSDebugLLog(@"GSPrinting", 
	     @"User did not set a printing bundle, trying till something works");
	     
      bundle = [GSPrinting loadAnyWorkingPrintingBundle];
      if (bundle == nil)
        {
          NSDebugLLog(@"GSPrinting", 
                      @"Could not load any working printing bundle");
			    
          NSRunAlertPanel(@"GNUstep Printing Backend System", 
           @"Could not open any working printing bundle.  Printing will not work.", 
           @"Ok", NULL, NULL);
	     
          return nil;
	  }
    }
  else
    {	
      NSDebugLLog(@"GSPrinting", 
	            @"User set %@ as the printing bundle", 
			defaultBundleName);
    
      bundle = [GSPrinting loadPrintingBundle: defaultBundleName];
    
      if (bundle == nil)
	  {
          NSDebugLLog(@"GSPrinting", 
          @"User set %@ as the printing bundle but that did not work.\
	      Will try to find anything that will work", 
          defaultBundleName);
	    
          bundle = [GSPrinting loadAnyWorkingPrintingBundle];
          if (bundle == nil)
            {
              NSDebugLLog(@"GSPrinting", 
                          @"Could not load any working printing bundle");
              NSRunAlertPanel(@"GNUstep Printing Backend System", 
               @"Could not open any working printing bundle.  Printing will not work.", 
               @"Ok", NULL, NULL);
              return nil;
            }
          else
            {
              NSString *msg;
              msg = [NSString stringWithFormat: 
                     @"Your chosen printing bundle, %@, could not be loaded.\
			     %@ was loaded instead", 
                     defaultBundleName, [[bundle bundlePath] lastPathComponent]];
			   
              NSRunAlertPanel(@"GNUstep Printing Backend System", 
		                  msg, @"Ok", NULL, NULL);
            }
        }
    }
  
  printingBundle = bundle;
  [printingBundle retain];
  return printingBundle;
}

@end







/**
  <unit>
  <heading>Class Description</heading>
  <p>
  GSPrintingPrincipleClass is the base class that all
  Printing backend bundles should use as their principle
  class.  Subclasses are responsible for the allocating
  of that bundle's particular NSPrint, NSPageLayout, and
  GSPrintOperation sublclassed objects.
  </p>
  </unit>
*/ 
@implementation GSPrintingPrincipalClass : NSObject
{
}

+(Class) pageLayoutClass
{
  return [[GSTheme theme] pageLayoutClass];
}

+(Class) printInfoClass
{
  return Nil;
}

+(Class) printOperationClass
{
  return Nil;
}

+(Class) printPanelClass
{
  return [[GSTheme theme] printPanelClass];
}

+(Class) printerClass
{
  return Nil;
}

+(Class) gsPrintOperationClass
{
  return Nil;
}


@end

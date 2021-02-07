/* 
   GSLPRPrintInfo.m

   Stores information used in printing

   Copyright (C) 1996,1997,2004 Free Software Foundation, Inc.

   Author:  Simon Frankau <sgf@frankau.demon.co.uk>
   Date: July 1997
   Modified for Printing Backend Support
   Author: Chad Hardin <cehardin@mac.com>
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

#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSPrinter.h"
#import "GSLPRPrintInfo.h"


@implementation GSLPRPrintInfo

//
// Class methods
//
+ (void)initialize
{
  if (self == [GSLPRPrintInfo class])
    {
      // Initial version
      [self setVersion:1];
    }
}


+ (id) allocWithZone: (NSZone*)zone
{
  return NSAllocateObject(self, 0, zone);
}


+(NSPrinter*) defaultPrinter
{
  NSUserDefaults *defaults;
  NSString *name;
    
  NSDebugMLLog(@"GSPrinting", @"defaultPrinter");
  defaults = [NSUserDefaults standardUserDefaults];
  name = [defaults objectForKey: @"GSLPRDefaultPrinter"];
  
  if (name == nil)
    {
      name = [[NSPrinter printerNames] objectAtIndex: 0];
    }
  else
    {
      if ([NSPrinter printerWithName: name] == nil)
        {
          name = [[NSPrinter printerNames] objectAtIndex: 0];
        }
    }
  return [NSPrinter printerWithName: name];
}


+ (void)setDefaultPrinter:(NSPrinter *)printer
{
  NSUserDefaults *defaults;
  NSMutableDictionary *globalDomain;
  
  NSDebugMLLog(@"GSPrinting", @"setDefaultPrinter");  
  defaults = [NSUserDefaults standardUserDefaults];
  
  globalDomain = (NSMutableDictionary*)[defaults persistentDomainForName: NSGlobalDomain];
  
  if (globalDomain)
    {
      globalDomain = [globalDomain mutableCopy];
  
      [globalDomain setObject: [printer name]
                       forKey: @"GSLPRDefaultPrinter"];
  
      [defaults setPersistentDomain: globalDomain
                            forName: NSGlobalDomain];
      RELEASE(globalDomain);
    }
  else
    {
      NSDebugMLLog(@"GSPrinting", @"(GSLPR) Could not save default printer named %@ to NSUserDefaults GSLPRDefaultPrinter in NSGlobalDomain.", [printer name]);
    }
}

@end

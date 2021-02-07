/* 
   GSPrintOperation.m

   Controls operations generating print jobs.

   Copyright (C) 1996,2004 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: November 2000
   Started implementation.
   Author: Chad Hardin <cehardin@mac.com>
   Date: June 2004
   Modified for printing backend support, split off from NSPrintOperation.m

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

#import <Foundation/NSData.h>
#import <Foundation/NSException.h>
#import <Foundation/NSTask.h>
#import <Foundation/NSUserDefaults.h>
#import "AppKit/NSPrintPanel.h"
#import "AppKit/NSPrintInfo.h"
#import "AppKit/NSView.h"
#import "AppKit/NSWorkspace.h"
#import "GNUstepGUI/GSPrinting.h"
#import "GNUstepGUI/GSPrintOperation.h"
#import "GSGuiPrivate.h"



/**
  <unit>
  <heading>Class Description</heading>
  <p>
  GSPrintOperation is a generic class that should be subclasses
  by printing backend classes for the purpose of controlling and
  sending print jobs to a printing system.
  </p>
  </unit>
*/ 




@implementation GSPrintOperation

/** Load the appropriate bundle for the PrintInfo
    (eg: GSLPRPrintInfo, GSCUPSPrintInfo).
*/
+ (id) allocWithZone: (NSZone*) zone
{
  Class principalClass;

  principalClass = [[GSPrinting printingBundle] principalClass];

  if (principalClass == nil)
    return nil;
	
  return [[principalClass gsPrintOperationClass] allocWithZone: zone];
}



- (id)initWithView:(NSView *)aView
         printInfo:(NSPrintInfo *)aPrintInfo
{
      
  self = [self initWithView: aView
                 insideRect: [aView bounds]
                     toData: [NSMutableData data]
                  printInfo: aPrintInfo];
                  
  [self setShowPanels: YES];

  return self;
}

- (NSGraphicsContext*)createContext
{
  [self subclassResponsibility: _cmd];
  return nil;
}


/**
/ !!!Here is the method that will be overridden in the printer bundle
*/
- (BOOL) _deliverSpooledResult
{
  [self subclassResponsibility: _cmd];
  return NO;
}


- (BOOL) deliverResult
{
  BOOL success;
  NSString *job;
  
  success = YES;
  job = [[self printInfo] jobDisposition];
  if ([job isEqual: NSPrintPreviewJob])
    {
      /* Check to see if there is a GNUstep app that can preview PS files.
	       It's not likely at this point, so also check for a standards
	       previewer, like gv.
      */
      NSTask *task;
      NSString *preview;
      NSWorkspace *ws = [NSWorkspace sharedWorkspace];
      [[self printPanel] _setStatusStringValue: @"Opening in previewer..."];
      
      preview = [ws getBestAppInRole: @"Viewer" 
                        forExtension: @"ps"];
      if (preview)
        {
          [ws openFile: _path withApplication: preview];
        }
      else
        {
          NSUserDefaults *def = [NSUserDefaults standardUserDefaults];
          preview = [def objectForKey: @"NSPreviewApp"];
          
	  if (preview == nil || [preview length] == 0)
	    preview = @"gv";
            
	  NS_DURING
	    {
	      task = AUTORELEASE([NSTask new]);
	      [task setLaunchPath: preview];
	      [task setArguments: [NSArray arrayWithObject: _path]];
	      [task launch];
	    }
	  NS_HANDLER
	    {
	      NSRunAlertPanel(_(@"Preview"),
_(@"Problem running the preview application '%@' perhaps you need to set your NSPreviewApp user default"),
		_(@"Dismiss"), nil, nil, preview);
	    }
	  NS_ENDHANDLER
        }
    }
  else if ([job isEqual: NSPrintSpoolJob])
    {
      success = [self _deliverSpooledResult];
    }
  else if ([job isEqual: NSPrintFaxJob])
    {
    }

  /* We can't remove the temp file because the previewer might still be
     using it, perhaps the printer is also?
  if (_path)
    {
      [[NSFileManager defaultManager] removeFileAtPath: _path
                                               handler: nil];
    }
  */
  return success;
}

@end


/* Copyright (C) 2009 Free Software Foundation, Inc.

   Written by:  German Arias <german@xelalug.org>
   Created: December 2009

   This file is part of the GNUstep Project

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

#import <Foundation/NSConnection.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSProcessInfo.h>

#import <GNUstepGUI/GSDisplayServer.h>
#import "AppKit/NSGraphics.h"
#import "GSIconManager.h"

@protocol GSIconManager
 - (NSRect) setWindow: (unsigned int)aWindowNumber appProcessId: (int)aProcessId;
 - (void) removeWindow: (unsigned int)aWindowNumber;
 - (NSSize) getSizeWindow;
 - (id) retain;
 - (void) release; 
@end

static BOOL verify = NO;
static id <GSIconManager>gsim = nil;
static int appId = 0;
static int iconCount = 0;

static void
GSGetIconManager(void)
{
  if ([[NSUserDefaults standardUserDefaults] boolForKey: @"GSUseIconManager"])
    {
      appId = [[NSProcessInfo processInfo] processIdentifier];

      gsim = (id <GSIconManager>)[NSConnection rootProxyForConnectionWithRegisteredName: @"GSIconManager" 
                                                                                   host: @""];
   
      if (gsim == nil)
	{
	  NSLog (@"Error: could not connect to server GSIconManager");
	}

      [gsim retain];
    }
}

static inline void
checkVerify()
{
  if (!verify)
   {
      GSGetIconManager();
      verify = YES;
   }
}

NSSize
GSGetIconSize(void)
{
  NSSize iconSize;

  checkVerify();

  if (gsim != nil)
    {
      iconSize = [gsim getSizeWindow];
    }
  else
    {
      iconSize = [GSCurrentServer() iconSize];
    }

  return iconSize;
}

void
GSRemoveIcon(NSWindow *window)
{
  checkVerify();

  if (gsim != nil)
    {
      unsigned int winNum = 0;

      NSConvertWindowNumberToGlobal([window windowNumber], &winNum);
      [gsim removeWindow: winNum];

      iconCount--;

      if (iconCount == 0)
	{
	  DESTROY(gsim);
	  verify = NO;
	}
    }
}

NSRect
GSGetIconFrame(NSWindow *window)
{
  NSRect iconRect;

  checkVerify();

  if (gsim != nil)
    {
      unsigned int winNum = 0;

      NSConvertWindowNumberToGlobal([window windowNumber], &winNum);
      iconRect = [gsim setWindow: winNum
                    appProcessId: appId];

      iconCount++;
    }
  else
    {
      iconRect = [window frame];
      iconRect.size = [GSCurrentServer() iconSize];
    }

  return iconRect;
}

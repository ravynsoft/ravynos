/** <title>GSHelpAttachment</title>

   Copyright (C) 2011 Free Software Foundation, Inc.

   Author: Wolfgang Lux <wolfgang.lux@gmail.com>
   Date:   Jan 2011
   
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


#import "AppKit/NSFileWrapper.h"
#import "AppKit/NSFileWrapperExtensions.h"
#import "AppKit/NSImage.h"
#import "GNUstepGUI/GSHelpAttachment.h"

@implementation GSHelpLinkAttachment

static NSImage *sharedHelpLinkIcon;

+ (void) initialize
{
  sharedHelpLinkIcon = [NSImage imageNamed: @"common_HelpLink"];
}

- (id) initWithFileName: (NSString *)aFileName
	     markerName: (NSString *)aMarkerName
{
  NSFileWrapper *wrapper;

  /* Create an empty wrapper so that we can supply an icon to the
     attachment cell. */
  wrapper = [[NSFileWrapper alloc] init];
  [wrapper setIcon: sharedHelpLinkIcon];

  if ((self = [super initWithFileWrapper: wrapper]) != nil)
    {
      ASSIGN(fileName, aFileName);
      ASSIGN(markerName, aMarkerName);
    }
  RELEASE(wrapper);
  return self;
}

- (void) dealloc
{
  RELEASE(fileName);
  RELEASE(markerName);
  [super dealloc];
}

- (NSString *) fileName
{
  return fileName;
}

- (NSString *) markerName
{
  return markerName;
}

@end

@implementation GSHelpMarkerAttachment

- (id) initWithMarkerName: (NSString *)aMarkerName
{
  NSFileWrapper *wrapper;

  /* Create an empty wrapper so that we can supply an (nil) icon to
     the attachment cell. */
  wrapper = [[NSFileWrapper alloc] init];

  if ((self = [super initWithFileWrapper: wrapper]) != nil)
    {
      ASSIGN(markerName, aMarkerName);
    }
  RELEASE(wrapper);
  return self;
}

- (void) dealloc
{
  RELEASE(markerName);
  [super dealloc];
}

- (NSString *) markerName
{
  return markerName;
}

@end

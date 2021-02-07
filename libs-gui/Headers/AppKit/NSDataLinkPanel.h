/* 
   NSDataLinkPanel.h

   Standard panel for inspecting data links

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   
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

#ifndef _GNUstep_H_NSDataLinkPanel
#define _GNUstep_H_NSDataLinkPanel
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSApplication.h>
#import <AppKit/NSPanel.h>

@class NSDataLink;
@class NSDataLinkManager;
@class NSView;

@interface NSApplication (NSDataLinkPanel)
- (void) orderFrontDataLinkPanel: (id)sender;
@end

@interface NSDataLinkPanel : NSPanel
{
  // Outlets
  id _sourceField;
  id _lastUpdateField;
  id _openSourceButton;
  id _updateDestinationButton;
  id _breakLinkButton;
  id _breakAllLinksButton;
  id _updateModeButton;

  // Attributes
  NSDataLinkManager *_currentDataLinkManager;
  NSDataLink *_currentDataLink;
  BOOL _multipleSelection;
  NSView *_accessoryView;
}

//
// Initializing
//
+ (NSDataLinkPanel *)sharedDataLinkPanel;

//
// Keeping the Panel Up to Date
//
+ (void)getLink:(NSDataLink **)link
	manager:(NSDataLinkManager **)linkManager
     isMultiple:(BOOL *)flag;
+ (void)setLink:(NSDataLink *)link
	manager:(NSDataLinkManager *)linkManager
     isMultiple:(BOOL)flag;
- (void)getLink:(NSDataLink **)link
	manager:(NSDataLinkManager **)linkManager
     isMultiple:(BOOL *)flag;
- (void)setLink:(NSDataLink *)link
	manager:(NSDataLinkManager *)linkManager
     isMultiple:(BOOL)flag;

//
// Customizing the Panel
//
- (NSView *)accessoryView;
- (void)setAccessoryView:(NSView *)aView;

//
// Responding to User Input
//
- (void)pickedBreakAllLinks:(id)sender;
- (void)pickedBreakLink:(id)sender;
- (void)pickedOpenSource:(id)sender;
- (void)pickedUpdateDestination:(id)sender;
- (void)pickedUpdateMode:(id)sender;

@end

#endif // _GNUstep_H_NSDataLinkPanel

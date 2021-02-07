/* 
   NSHelpPanel.h

   Standard panel for showing help information

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

#ifndef _GNUstep_H_NSHelpPanel
#define _GNUstep_H_NSHelpPanel
#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(GS_API_OPENSTEP,GS_API_MACOSX) || GS_API_VERSION(GS_API_NONE, GS_API_LATEST)

#import <AppKit/NSApplication.h>
#import <AppKit/NSPanel.h>

@class NSString;

@interface NSApplication (NSHelpPanel)
- (void) orderFrontHelpPanel: (id)sender;
@end

@interface NSHelpPanel : NSPanel
{
  // Attributes
}

//
// Accessing the Help Panel
//
+ (NSHelpPanel *) sharedHelpPanel;
+ (NSHelpPanel *) sharedHelpPanelWithDirectory: (NSString *)helpDirectory;

//
// Managing the Contents
//
+ (void) setHelpDirectory: (NSString *)helpDirectory;
- (void) addSupplement: (NSString *)helpDirectory
	        inPath: (NSString *)supplementPath;
- (NSString *) helpDirectory;
- (NSString *) helpFile;

//
// Attaching Help to Objects 
//
+ (void) attachHelpFile: (NSString *)filename
	     markerName: (NSString *)markerName
		     to: (id)anObject;
+ (void) detachHelpFrom: (id)anObject;

//
// Showing Help 
//
- (void) showFile: (NSString *)filename
	 atMarker: (NSString *)markerName;
- (BOOL) showHelpAttachedTo: (id)anObject;

//
// Printing 
//
- (void) print: (id)sender;

@end

#endif // !GS_API_MACOSX
#endif // _GNUstep_H_NSHelpPanel


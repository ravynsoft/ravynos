/* 
   NSHelpManager.m

   Description...

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Pedro Ivo Andrade Tavares <ptavares@iname.com>
   Date: 1999
   
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

#ifndef __GNUstep_H_NSHelpManager
#define __GNUstep_H_NSHelpManager
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSBundle.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/NSApplication.h>

@class NSAttributedString;
@class NSString;
@class NSMapTable;

@interface NSBundle (NSHelpManager)
- (NSString *)pathForHelpResource:(NSString *)fileName;
- (NSAttributedString *)contextHelpForKey:(NSString *)key;
@end

@interface NSApplication (NSHelpManager)
- (void) showHelp: (id) sender;
- (void) activateContextHelpMode: (id) sender;
@end

@interface NSHelpManager: NSObject
{
@private
  NSMapTable* contextHelpTopics;
}

//
// Class methods
//
+ (NSHelpManager*)sharedHelpManager;

+ (BOOL)isContextHelpModeActive;

+ (void)setContextHelpModeActive: (BOOL) flag;

//
// Instance methods
//
- (NSAttributedString*) contextHelpForObject: (id)object;

- (void) removeContextHelpForObject: (id)object;

- (void)setContextHelp:(NSAttributedString *)help forObject:(id)object;

- (void) setContextHelp: (NSAttributedString*) help withObject: (id) object;

- (BOOL) showContextHelpForObject: (id)object locationHint: (NSPoint) point;

@end

// Notifications
APPKIT_EXPORT NSString* NSContextHelpModeDidActivateNotification;
APPKIT_EXPORT NSString* NSContextHelpModeDidDeactivateNotification;

#endif // GNUstep_H_NSHelpManager

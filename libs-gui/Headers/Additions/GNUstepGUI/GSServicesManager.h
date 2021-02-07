/* 
   GSServicesManager.h

   Copyright (C) 1998 Free Software Foundation, Inc.

   Author:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: Novemeber 1998
  
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

#ifndef _GNUstep_H_GSServicesManager
#define _GNUstep_H_GSServicesManager

#import <Foundation/NSObject.h>
/* Forward declaring the NSMenuItem protocol here would be nicer, but older
   versions of gcc can't handle that.  Thus, we include the header
   instead. */
#import "AppKit/NSMenuItem.h"

@class	NSApplication;
@class	NSArray;
@class	NSDate;
@class	NSMenu;
@class	NSMenuItem;
@class	NSMutableArray;
@class	NSMutableDictionary;
@class	NSMutableSet;
@class	NSString;
@class	NSTimer;

@interface      GSServicesManager : NSObject
{
  NSApplication         *_application;
  NSMenu                *_servicesMenu;
  NSMutableArray        *_languages;
  NSMutableSet          *_returnInfo;
  NSMutableDictionary   *_combinations;
  NSMutableDictionary   *_title2info;
  NSArray               *_menuTitles;
  NSString		*_disabledPath;
  NSString		*_servicesPath;
  NSDate		*_disabledStamp;
  NSDate		*_servicesStamp;
  NSMutableSet		*_allDisabled;
  NSMutableDictionary	*_allServices;
  NSTimer		*_timer;
  NSString		*_port;
}
+ (GSServicesManager*) newWithApplication: (NSApplication*)app;
+ (GSServicesManager*) manager;
- (BOOL) application: (NSApplication*)theApp
	    openFile: (NSString*)file;
- (BOOL) application: (NSApplication*)theApp
   openFileWithoutUI: (NSString*)file;
- (BOOL) application: (NSApplication*)theApp
	openTempFile: (NSString*)file;
- (BOOL) application: (NSApplication*)theApp
	   printFile: (NSString*)file;
- (void) doService: (NSMenuItem*)item;
- (NSArray*) filters;
- (BOOL) hasRegisteredTypes: (NSDictionary*)service;
- (NSString*) item2title: (id<NSMenuItem>)item;
- (void) loadServices;
- (NSDictionary*) menuServices;
- (NSString*) port;
- (void) rebuildServices;
- (void) rebuildServicesMenu;
- (void) registerAsServiceProvider;
- (void) registerSendTypes: (NSArray *)sendTypes
               returnTypes: (NSArray *)returnTypes;
- (NSMenu *) servicesMenu;
- (id) servicesProvider;
- (void) setServicesMenu: (NSMenu *)anObject;
- (void) setServicesProvider: (id)anObject;
- (int) setShowsServicesMenuItem: (NSString*)item to: (BOOL)enable;
- (BOOL) showsServicesMenuItem: (NSString*)item;
- (BOOL) validateMenuItem: (id<NSMenuItem>)item;
- (void) updateServicesMenu;
@end

#endif


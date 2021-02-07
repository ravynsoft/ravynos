/* Definition of class NSStoryboardSegue
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory Casamento
   Date: Mon Jan 20 15:57:31 EST 2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSStoryboardSegue_h_GNUSTEP_GUI_INCLUDE
#define _NSStoryboardSegue_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/NSPopover.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

typedef NSString *NSStoryboardSegueIdentifier;

DEFINE_BLOCK_TYPE_NO_ARGS(GSStoryboardSeguePerformHandler, void);
  
@interface NSStoryboardSegue : NSObject
{
  id _sourceController;
  id _destinationController;
  NSStoryboardSegueIdentifier _identifier;
  NSString *_kind;
  NSString *_relationship;
  id _popoverAnchorView;
  NSPopoverBehavior _popoverBehavior;
  NSRectEdge _preferredEdge;
  GSStoryboardSeguePerformHandler _handler;
}

- (id) sourceController;
- (id) destinationController;
- (NSStoryboardSegueIdentifier) identifier;

+ (instancetype) segueWithIdentifier: (NSStoryboardSegueIdentifier)identifier 
                              source: (id)sourceController 
                         destination: (id)destinationController 
                      performHandler: (GSStoryboardSeguePerformHandler)performHandler;

- (instancetype) initWithIdentifier: (NSStoryboardSegueIdentifier)identifier 
                             source: (id)sourceController 
                        destination: (id)destinationController;

- (void) perform;
  
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSStoryboardSegue_h_GNUSTEP_GUI_INCLUDE */


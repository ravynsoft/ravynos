/* 
   GSNibTemplates.h

   Copyright (C) 1997, 1999 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>
   Date: 2002
   
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

#ifndef _GNUstep_H_GSNibTemplates
#define _GNUstep_H_GSNibTemplates

#import <Foundation/NSObject.h>
#import "GNUstepGUI/GSNibContainer.h"


// version of the nib container and the templates.
#define GNUSTEP_NIB_VERSION 2
#define GSSWAPPER_VERSION   0
#define GSWINDOWT_VERSION   1
#define GSVIEWT_VERSION     0
#define GSCONTROLT_VERSION  0
#define GSTEXTT_VERSION     0
#define GSTEXTVIEWT_VERSION 0
#define GSMENUT_VERSION     0
#define GSOBJECTT_VERSION   0

@class NSString;
@class NSDictionary;
@class NSMutableDictionary;
@class NSMutableSet;
@class NSWindow;

/** Window template autopositioning constants */
enum {
  GSWindowAutoPositionNone = 0,
  GSWindowMinXMargin = 1,
  GSWindowMaxXMargin = 2,
  GSWindowMinYMargin = 4,
  GSWindowMaxYMargin = 8
};

/*
 * This is the class that holds objects within a nib.
 */
@interface GSNibContainer : NSObject <NSCoding, GSNibContainer>
{
  NSMutableDictionary	*nameTable;
  NSMutableArray	*connections;
  NSMutableArray	*visibleWindows;
  NSMutableArray	*deferredWindows;
  NSMutableSet          *topLevelObjects;
  NSMutableDictionary	*customClasses;
  BOOL			isAwake;
}
- (NSMutableArray*) visibleWindows;
- (NSMutableArray*) deferredWindows;
- (NSMutableDictionary *) customClasses;
@end

/*
 * Template classes
 */
@protocol GSTemplate
- (id) initWithObject: (id)object className: (NSString *)className superClassName: (NSString *)superClassName;
- (void) setClassName: (NSString *)className;
- (NSString *)className;
@end

@interface GSClassSwapper : NSObject <GSTemplate, NSCoding>
{
  id                   _object;
  NSString            *_className;
  Class                _superClass;
}
- (BOOL) shouldSwapClass;
@end

@interface GSNibItem : NSObject <NSCoding> 
{
  NSString		*theClass;
  NSRect		theFrame;
  unsigned int          autoresizingMask;
}
@end

@interface GSCustomView : GSNibItem <NSCoding>  
{
}
@end

@interface GSWindowTemplate : GSClassSwapper
{
  BOOL                 _deferFlag;
  unsigned int         _autoPositionMask;
  NSRect               _screenRect;
}
// auto position the window.
- (unsigned int) autoPositionMask;
- (void) setAutoPositionMask: (unsigned int)flag;
- (void) autoPositionWindow: (NSWindow *)window;

// set attributes specific to the template.
- (void) setDeferFlag: (BOOL)flag;
- (BOOL) deferFlag;
@end

@interface GSViewTemplate : GSClassSwapper
@end

@interface GSTextTemplate : GSClassSwapper
@end

@interface GSTextViewTemplate : GSClassSwapper 
@end

@interface GSMenuTemplate : GSClassSwapper
@end

@interface GSControlTemplate : GSClassSwapper
@end

@interface GSObjectTemplate : GSClassSwapper
@end

@interface GSTemplateFactory : NSObject
+ (id) templateForObject: (id) object 
	   withClassName: (NSString *)className
      withSuperClassName: (NSString *)superClassName;
@end
#endif /* _GNUstep_H_GSNibTemplates */

/** <title>NSUserDefaultsController</title>

   <abstract>Controller class for user defaults</abstract>

   Copyright <copy>(C) 2006 Free Software Foundation, Inc.</copy>

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: September 2006

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

#ifndef _GNUstep_H_NSUserDefaultsController
#define _GNUstep_H_NSUserDefaultsController
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSController.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)

@class NSUserDefaults;
@class NSDictionary;
@class NSMutableDictionary;

@interface NSUserDefaultsController : NSController
{
  NSUserDefaults* _defaults;
  NSDictionary* _initial_values;
  id _values;
  BOOL _applies_immediately;
}

+ (id) sharedUserDefaultsController;

- (id) initWithDefaults: (NSUserDefaults*)defaults
          initialValues: (NSDictionary*)initialValues;

- (NSUserDefaults*) defaults;
- (id) values;
- (NSDictionary*) initialValues;
- (void) setInitialValues: (NSDictionary*)values;
- (BOOL) appliesImmediately;
- (void) setAppliesImmediately: (BOOL)flag;
- (void) revert: (id)sender;
- (void) revertToInitialValues: (id)sender;
- (void) save: (id)sender;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL) hasUnappliedChanges;
#endif

@end

#endif // OS_API_VERSION

#endif // _GNUstep_H_NSUserDefaultsController

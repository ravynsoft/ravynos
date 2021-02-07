/** <title>NSObjectController</title>

   <abstract>Controller class</abstract>

   Copyright <copy>(C) 2006 Free Software Foundation, Inc.</copy>

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: June 2006

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

#ifndef _GNUstep_H_NSObjectController
#define _GNUstep_H_NSObjectController
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSController.h>
#import <AppKit/NSMenuItem.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3,GS_API_LATEST)

@class NSArray;
@class NSMutableArray;
@class NSString;
@class NSPredicate;
@class NSError;
@class NSFetchRequest;
@class NSManagedObjectContext;

@interface NSObjectController : NSController
{
  @protected
  Class _object_class;

  NSString *_object_class_name;
  NSString *_entity_name_key;
  id _managed_proxy;
  id _content;
  NSMutableArray *_selection;
  NSPredicate *_fetch_predicate;
  NSManagedObjectContext *_managed_object_context;

  BOOL _is_editable;
  BOOL _automatically_prepares_content;
  BOOL _is_using_managed_proxy;
}

- (id) initWithContent: (id)content;
- (id) content;
- (void) setContent: (id)content;
- (Class) objectClass;
- (void) setObjectClass: (Class)aClass;

- (id) newObject;
- (void) prepareContent;
- (BOOL) automaticallyPreparesContent;
- (void) setAutomaticallyPreparesContent: (BOOL)flag;

- (void) add: (id)sender;
- (void) addObject: (id)obj;
- (void) remove: (id)sender;
- (void) removeObject: (id)obj;
- (BOOL) canAdd;
- (BOOL) canRemove;

- (BOOL) isEditable;
- (void) setEditable: (BOOL)flag;

- (NSArray*) selectedObjects;
- (id) selection;

- (BOOL) validateMenuItem: (id <NSMenuItem>)item;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4,GS_API_LATEST)
- (NSString*) entityNameKey;
- (void) setEntityName: (NSString*)entityName;
- (NSPredicate*) fetchPredicate;
- (void) setFetchPredicate: (NSPredicate*)predicate;
- (void) fetch: (id)sender;
- (BOOL) fetchWithRequest: (NSFetchRequest*)fetchRequest
                    merge: (BOOL)merge
                    error: (NSError**)error;
- (NSManagedObjectContext*) managedObjectContext;
- (void) setManagedObjectContext: (NSManagedObjectContext*)managedObjectContext;
#endif //OS_API_VERSION

@end

#endif // OS_API_VERSION

#endif // _GNUstep_H_NSObjectController

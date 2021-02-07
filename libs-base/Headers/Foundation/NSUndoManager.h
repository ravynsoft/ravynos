/* Interface for <NSUndoManager> for GNUStep
   Copyright (C) 1998 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/ 

#ifndef __NSUndoManager_h_OBJECTS_INCLUDE
#define __NSUndoManager_h_OBJECTS_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if	OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSArray;
@class NSString;
@class NSMutableArray;
@class NSInvocation;

/** Defines run loop ordering for closing undo groupings. */
enum {
  NSUndoCloseGroupingRunLoopOrdering = 350000
};

/* Public notification */

/**
 *  Notification posted whenever [NSUndoManager] opens or closes an undo
 *  group.  The undo manager itself is the notification object, with no
 *  <em>userInfo</em> dictionary.
 */
GS_EXPORT NSString* const NSUndoManagerCheckpointNotification;

/**
 * Notification posted after an [NSUndoManager] opens an undo group.
 */
GS_EXPORT NSString* const NSUndoManagerDidOpenUndoGroupNotification;

/**
 * Notification posted after an [NSUndoManager] executes a redo operation.
 */
GS_EXPORT NSString* const NSUndoManagerDidRedoChangeNotification;

/**
 * Notification posted after an [NSUndoManager] executes an undo operation.
 */
GS_EXPORT NSString* const NSUndoManagerDidUndoChangeNotification;

/**
 * Notification posted before an [NSUndoManager] closes an undo group.
 */
GS_EXPORT NSString* const NSUndoManagerWillCloseUndoGroupNotification;

/**
 * Notification posted before an [NSUndoManager] will execute a redo operation.
 */
GS_EXPORT NSString* const NSUndoManagerWillRedoChangeNotification;

/**
 * Notification posted before an [NSUndoManager] will execute an undo operation.
 */
GS_EXPORT NSString* const NSUndoManagerWillUndoChangeNotification;

GS_EXPORT_CLASS
@interface NSUndoManager : NSObject
{
#if	GS_EXPOSE(NSUndoManager)
@private
  NSMutableArray	*_redoStack;
  NSMutableArray	*_undoStack;
  id			_group;
  id			_nextTarget;
  NSArray		*_modes;
  BOOL			_isRedoing;
  BOOL			_isUndoing;
  BOOL			_groupsByEvent;
  BOOL			_runLoopGroupingPending;
  unsigned		_disableCount;
  unsigned		_levelsOfUndo;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}

- (void) beginUndoGrouping;
- (BOOL) canRedo;
- (BOOL) canUndo;
- (void) disableUndoRegistration;
- (void) enableUndoRegistration;
- (void) endUndoGrouping;
- (void) forwardInvocation: (NSInvocation*)anInvocation;
- (NSInteger) groupingLevel;
- (BOOL) groupsByEvent;
- (BOOL) isRedoing;
- (BOOL) isUndoing;
- (BOOL) isUndoRegistrationEnabled;
- (NSUInteger) levelsOfUndo;
- (id) prepareWithInvocationTarget: (id)target;
- (void) redo;
- (NSString*) redoActionName;
- (NSString*) redoMenuItemTitle;
- (NSString*) redoMenuTitleForUndoActionName: (NSString*)actionName;
- (void) registerUndoWithTarget: (id)target
		       selector: (SEL)aSelector
			 object: (id)anObject;
- (void) removeAllActions;
- (void) removeAllActionsWithTarget: (id)target;
- (NSArray*) runLoopModes;
- (void) setActionName: (NSString*)name;
- (void) setGroupsByEvent: (BOOL)flag;
- (void) setLevelsOfUndo: (NSUInteger)num;
- (void) setRunLoopModes: (NSArray*)newModes;
- (void) undo;
- (NSString*) undoActionName;
- (NSString*) undoMenuItemTitle;
- (NSString*) undoMenuTitleForUndoActionName: (NSString*)actionName;
- (void) undoNestedGroup;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif /* __NSUndoManager_h_OBJECTS_INCLUDE */

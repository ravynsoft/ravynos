/** Implementation for NSUndoManager for GNUStep
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

   <title>NSUndoManager class reference</title>
   $Date$ $Revision$
*/

#import "common.h"
#define	EXPOSE_NSUndoManager_IVARS	1
#import "Foundation/NSArray.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSInvocation.h"
#import "Foundation/NSException.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSUndoManager.h"

/*
 *	Private class for grouping undo/redo actions.
 */
@interface	PrivateUndoGroup : NSObject
{
  PrivateUndoGroup	*parent;
  NSMutableArray	*actions;
  NSString              *actionName;
}
- (NSMutableArray*) actions;

- (NSString*) actionName;
- (void) addInvocation: (NSInvocation*)inv;
- (id) initWithParent: (PrivateUndoGroup*)parent;
- (void) orphan;
- (PrivateUndoGroup*) parent;
- (void) perform;
- (BOOL) removeActionsForTarget: (id)target;
- (void) setActionName: (NSString*)name;
@end

@implementation	PrivateUndoGroup

- (NSMutableArray*) actions
{
  return actions;
}

- (NSString*) actionName
{
  return actionName;
}

- (void) addInvocation: (NSInvocation*)inv
{
  if (actions == nil)
    {
      actions = [[NSMutableArray alloc] initWithCapacity: 2];
    }
  [actions addObject: inv];
}

- (void) dealloc
{
  RELEASE(actions);
  RELEASE(parent);
  RELEASE(actionName);
  [super dealloc];
}

- (id) initWithParent: (PrivateUndoGroup*)p
{
  self = [super init];
  if (self)
    {
      parent = RETAIN(p);
      actions = nil;
      actionName = @"";
    }
  return self;
}

- (void) orphan
{
  DESTROY(parent);
}

- (PrivateUndoGroup*) parent
{
  return parent;
}

- (void) perform
{
  if (actions != nil)
    {
      unsigned	i = [actions count];

      while (i-- > 0)
	{
	  [[actions objectAtIndex: i] invoke];
	}
    }
}

- (BOOL) removeActionsForTarget: (id)target
{
  if (actions != nil)
    {
      unsigned	i = [actions count];

      while (i-- > 0)
	{
	  NSInvocation	*inv = [actions objectAtIndex: i];

	  if ([inv target] == target)
	    {
	      [actions removeObjectAtIndex: i];
	    }
	}
      if ([actions count] > 0)
	{
	  return YES;
	}
    }
  return NO;
}

- (void) setActionName: (NSString *)name
{
  ASSIGNCOPY(actionName,name);
}

@end



/*
 *	Private category for the method used to handle default grouping
 */
@interface NSUndoManager (Private)
- (void) _begin;
- (void) _loop: (id)arg;
@end

@implementation NSUndoManager (Private)
/* This method is used to begin undo grouping internally.
 * It's necessary to have a different mechanism from the -beginUndoGrouping
 * because it seems that in MacOS X a call to -beginUndoGrouping when at the
 * top level will actually create two undo groups.
 */
- (void) _begin
{
  PrivateUndoGroup	*parent;

  parent = (PrivateUndoGroup*)_group;
  _group = [[PrivateUndoGroup alloc] initWithParent: parent];
  if (_group == nil)
    {
      _group = parent;
      [NSException raise: NSInternalInconsistencyException
		  format: @"beginUndoGrouping failed to greate group"];
    }
  else
    {
      RELEASE(parent);

      if (_isUndoing == NO && _isRedoing == NO)
        [[NSNotificationCenter defaultCenter]
	    postNotificationName: NSUndoManagerDidOpenUndoGroupNotification
			  object: self];
    }
}

- (void) _loop: (id)arg
{
  if (_groupsByEvent && _group != nil)
    {
      [self endUndoGrouping];
    }
  _runLoopGroupingPending = NO;
}
@end



/**
 *  NSUndoManager provides a general mechanism supporting implementation of
 *  user action "undo" in applications.  Essentially, it allows you to store
 *  sequences of messages and receivers that need to be invoked to undo or
 *  redo an action.  The various methods in this class provide for grouping
 *  of sets of actions, execution of undo or redo actions, and tuning behavior
 *  parameters such as the size of the undo stack.  Each application entity
 *  with its own editing history (e.g., a document) should have its own undo
 *  manager instance.  Obtain an instance through a simple
 *  <code>[[NSUndoManager alloc] init]</code> message.
 */
@implementation NSUndoManager

/**
 * Starts a new grouping of undo actions which can be
 * atomically undone by an [-undo] invocation.<br />
 * This method posts an NSUndoManagerDidOpenUndoGroupNotification
 * upon creating the grouping.<br />
 * It first posts an NSUndoManagerCheckpointNotification
 * unless an undo is currently in progress.<br />
 * The order of these notifications is undefined, but the GNUstep
 * implementation currently mimics the observed order in MacOS X 10.5
 */
- (void) beginUndoGrouping
{
  /* It seems that MacOS X implicitly creates a top-level group if this
   * method is called when groupsByEvent is set and there is no existing
   * top level group.  There is no checkpoint notification posted for the
   * implicit group creation.
   */
  if (_group == nil && [self groupsByEvent])
    {
      [self _begin];	// Start top level group
    }

  /* Post the checkpoint notification and THEN create the group.
   */
  [[NSNotificationCenter defaultCenter]
      postNotificationName: NSUndoManagerCheckpointNotification
		    object: self];
  [self _begin];	// Start a new group
}

/**
 * Returns whether the receiver can service redo requests and
 * posts a NSUndoManagerCheckpointNotification.
 */
- (BOOL) canRedo
{
  [[NSNotificationCenter defaultCenter]
      postNotificationName: NSUndoManagerCheckpointNotification
		    object: self];
  if ([_redoStack count] > 0)
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

/**
 * Returns whether the receiver has any action groupings
 * on the stack to undo.  It does not imply, that the
 * receiver is currently in a state to service an undo
 * request.  Make sure [-endUndoGrouping] is invoked before
 * requesting either an [-undo] or an [-undoNestedGroup].
 */
- (BOOL) canUndo
{
  if ([_undoStack count] > 0)
    {
      return YES;
    }
  if (_group != nil && [[_group actions] count] > 0)
    {
      return YES;
    }
  return NO;
}

- (void) dealloc
{
  [[NSRunLoop currentRunLoop] cancelPerformSelector: @selector(_loop:)
					     target: self
					   argument: nil];
  RELEASE(_redoStack);
  RELEASE(_undoStack);
  RELEASE(_group);
  RELEASE(_modes);
  [super dealloc];
}

/**
 * Disables the registration of operations with with either
 * [-registerUndoWithTarget:selector:object:] or
 * [-forwardInvocation:].  This method may be called multiple
 * times.  Each will need to be paired to a call of
 * [-enableUndoRegistration] before registration is actually
 * reenabled.
 */
- (void) disableUndoRegistration
{
  _disableCount++;
}

/**
 * Matches previous calls of to [-disableUndoRegistration].
 * Only call this method to that end.  Once all are matched,
 * the registration of [-registerUndoWithTarget:selector:object:]
 * and [-forwardInvocation:] are reenabled.  If this method is
 * called without a matching -disableUndoRegistration,
 * it will raise an NSInternalInconsistencyException.
 */
- (void) enableUndoRegistration
{
  if (_disableCount > 0)
    {
      _disableCount--;
    }
  else
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"enableUndoRegistration without disable"];
    }
}

/**
 * Matches previous calls of to [-beginUndoGrouping] and
 * puts the group on the undo stack.  This method posts
 * an NSUndoManagerCheckpointNotification and
 * a NSUndoManagerWillCloseUndoGroupNotification.
 * If there was no matching call to -beginUndoGrouping,
 * this method will raise an NSInternalInconsistencyException.
 */
- (void) endUndoGrouping
{
  PrivateUndoGroup	*g;
  PrivateUndoGroup	*p;

  if (_group == nil)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"endUndoGrouping without beginUndoGrouping"];
    }
  [[NSNotificationCenter defaultCenter]
      postNotificationName: NSUndoManagerCheckpointNotification
		    object: self];
  if (_isUndoing == NO && _isRedoing == NO)
    [[NSNotificationCenter defaultCenter]
        postNotificationName: NSUndoManagerWillCloseUndoGroupNotification
		      object: self];
  g = (PrivateUndoGroup*)_group;
  p = RETAIN([g parent]);
  _group = p;
  [g orphan];
  if (p == nil)
    {
      if (_isUndoing)
	{
	  if (_levelsOfUndo > 0
	    && [_redoStack count] == _levelsOfUndo
	    && [[g actions] count] > 0)
	    {
	      [_redoStack removeObjectAtIndex: 0];
	    }

	  if (g != nil)
	    {
	      if ([[g actions] count] > 0)
		[_redoStack addObject: g];
	    }
	}
      else
	{
	  if (_levelsOfUndo > 0
	    && [_undoStack count] == _levelsOfUndo
	    && [[g actions] count] > 0)
	    {
	      [_undoStack removeObjectAtIndex: 0];
	    }

	  if (g != nil)
	    {
	      if ([[g actions] count] > 0)
		[_undoStack addObject: g];
	    }
	}
    }
  else if ([g actions] != nil)
    {
      NSArray	*a = [g actions];
      unsigned	i;

      for (i = 0; i < [a count]; i++)
	{
	  [p addInvocation: [a objectAtIndex: i]];
	}
    }
  RELEASE(g);
}

/**
 * Registers the invocation with the current undo grouping.
 * This method is part of the NSInvocation-based undo registration
 * as opposed to the simpler [-registerUndoWithTarget:selector:object:]
 * technique.<br />
 * You generally never invoke this method directly.
 * Instead invoke [-prepareWithInvocationTarget:] with the target of the
 * undo action and then invoke the targets method to undo the action
 * on the return value of -prepareWithInvocationTarget:
 * which actually is the undo manager.
 * The runtime will then fallback to -forwardInvocation: to do the actual
 * registration of the invocation.
 * The invocation will added to the current grouping.<br />
 * If the registrations have been disabled through [-disableUndoRegistration],
 * this method does nothing.<br />
 * Unless the receiver implicitly
 * groups operations by event, the this method must have been preceded
 * with a [-beginUndoGrouping] message.  Otherwise it will raise an
 * NSInternalInconsistencyException. <br />
 * Unless this method is invoked as part of a [-undo] or [-undoNestedGroup]
 * processing, the redo stack is cleared.<br />
 * If the receiver [-groupsByEvent] and this is the first call to this
 * method since the last run loop processing, this method sets up
 * the receiver to process the [-endUndoGrouping] at the
 * end of the event loop.
 */
- (void) forwardInvocation: (NSInvocation*)anInvocation
{
  if (_disableCount == 0)
    {
      if (_nextTarget == nil)
	{
	  [NSException raise: NSInternalInconsistencyException
		      format: @"forwardInvocation without preparation"];
	}
      if (_group == nil)
	{
	  if ([self groupsByEvent])
	    {
	      [self _begin];
	    }
	  else
	    {
	      [NSException raise: NSInternalInconsistencyException
		format: @"forwardInvocation without beginUndoGrouping"];
	    }
	}
      [anInvocation retainArgumentsIncludingTarget: NO];
      [anInvocation setTarget: _nextTarget];
      _nextTarget = nil;
      [_group addInvocation: anInvocation];
      if (_isUndoing == NO && _isRedoing == NO && [_group actions] > 0)
	{
	  [_redoStack removeAllObjects];
	}
      if ((_runLoopGroupingPending == NO) && ([self groupsByEvent] == YES))
	{
	  [[NSRunLoop currentRunLoop] performSelector: @selector(_loop:)
				      target: self
				      argument: nil
				      order: NSUndoCloseGroupingRunLoopOrdering
				      modes: _modes];
	  _runLoopGroupingPending = YES;
	}
    }
}

/**
 * If the receiver was sent a [-prepareWithInvocationTarget:] and
 * the target's method hasn't been invoked on the receiver yet, this
 * method forwards the request to the target.
 * Otherwise or if the target didn't return a signature, the message
 * is sent to super.
 */
- (NSMethodSignature*) methodSignatureForSelector: (SEL)selector
{
  NSMethodSignature *sig = nil;

  if (_nextTarget != nil)
    {
      sig = [_nextTarget methodSignatureForSelector: selector];
    }
  if (sig == nil)
    {
      sig = [super methodSignatureForSelector: selector];
    }
  return sig;
}

/**
 * Returns the current number of groupings.  These are the current
 * groupings which can be nested, not the number of of groups on either
 * the undo or redo stack.
 */
- (NSInteger) groupingLevel
{
  PrivateUndoGroup	*g = (PrivateUndoGroup*)_group;
  int			level = 0;

  while (g != nil)
    {
      level++;
      g = [g parent];
    }
  return level;
}

/**
 * Returns whether the receiver currently groups undo
 * operations by events.  When it does, so it implicitly
 * invokes [-beginUndoGrouping] upon registration of undo
 * operations and registers an internal call to insure
 * the invocation of [-endUndoGrouping] at the end of the
 * run loop.
 */
- (BOOL) groupsByEvent
{
  return _groupsByEvent;
}

- (id) init
{
  self = [super init];
  if (self)
    {
      _redoStack = [[NSMutableArray alloc] initWithCapacity: 16];
      _undoStack = [[NSMutableArray alloc] initWithCapacity: 16];
      _groupsByEvent = YES;
      [self setRunLoopModes:
	[NSArray arrayWithObjects: NSDefaultRunLoopMode, nil]];
    }
  return self;
}

/**
 * Returns whether the receiver is currently processing a redo.
 */
- (BOOL) isRedoing
{
  return _isRedoing;
}

/**
 * Returns whether the receiver is currently processing an undo.
 */
- (BOOL) isUndoing
{
  return _isUndoing;
}

/**
 * Returns whether the receiver will currently register undo operations.
 */
- (BOOL) isUndoRegistrationEnabled
{
  if (_disableCount == 0)
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

/**
 * Returns the maximum number of undo groupings the receiver will maintain.
 * The default value is 0 meaning the number is only limited by
 * memory availability.
 */
- (NSUInteger) levelsOfUndo
{
  return _levelsOfUndo;
}

/**
 * Prepares the receiver to registers an invocation-based undo operation.
 * This method is part of the NSInvocation-based undo registration
 * as opposed to the simpler [-registerUndoWithTarget:selector:object:]
 * technique. <br />
 * You invoke this method with the target of the
 * undo action and then invoke the targets method to undo the action
 * on the return value of this invocation
 * which actually is the undo manager.
 * The runtime will then fallback to [-forwardInvocation:] to do the actual
 * registration of the invocation.
 */
- (id) prepareWithInvocationTarget: (id)target
{
  _nextTarget = target;
  return self;
}

/**
 * Performs a redo of previous undo request by taking the top grouping
 * from the redo stack and invoking them.  This method posts an
 * NSUndoManagerCheckpointNotification notification to allow the client
 * to process any pending changes before proceeding.  If there are groupings
 * on the redo stack, the top object is popped off the stack and invoked
 * within a nested [-beginUndoGrouping]/[-endUndoGrouping].  During this
 * processing, the operations registered for undo are recorded on the undo
 * stack again.<br />
 */
- (void) redo
{
  NSString *name = nil;

  if (_isUndoing || _isRedoing)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"redo while undoing or redoing"];
    }
  [[NSNotificationCenter defaultCenter]
      postNotificationName: NSUndoManagerCheckpointNotification
		    object: self];
  if ([_redoStack count] > 0)
    {
      PrivateUndoGroup	*oldGroup;
      PrivateUndoGroup	*groupToRedo;

      [[NSNotificationCenter defaultCenter]
	  postNotificationName: NSUndoManagerWillRedoChangeNotification
		    object: self];

      groupToRedo = RETAIN([_redoStack lastObject]);
      [_redoStack removeLastObject];

      name = [NSString stringWithString: [groupToRedo actionName]];

      oldGroup = _group;
      _group = nil;
      _isRedoing = YES;

      [self _begin];
      [groupToRedo perform];
      RELEASE(groupToRedo);
      [self endUndoGrouping];

      _isRedoing = NO;
      _group = oldGroup;

      [[_undoStack lastObject] setActionName: name];

      [[NSNotificationCenter defaultCenter]
	  postNotificationName: NSUndoManagerDidRedoChangeNotification
			object: self];
    }
}

/**
 * If the receiver can perform a redo, this method returns
 * the action name previously associated with the top grouping with
 * [-setActionName:].  This name should identify the action to be redone.
 * If there are no items on the redo stack this method returns nil.
 * If no action name has been set, this method returns an empty string.
 */
- (NSString*) redoActionName
{
  if ([self canRedo] == NO)
    {
      return nil;
    }
  return [[_redoStack lastObject] actionName];
}

/**
 * Returns the full localized title of the actions to be displayed
 * as a menu item.  This method first invokes [-redoActionName] and
 * passes it to [-redoMenuTitleForUndoActionName:] and returns the result.
 */
- (NSString*) redoMenuItemTitle
{
  return [self redoMenuTitleForUndoActionName: [self redoActionName]];
}

/**
 * Returns the localized title of the actions to be displayed
 * as a menu item identified by actionName, by appending a
 * localized command string like @"Redo &lt;localized(actionName)&gt;".
 */
- (NSString*) redoMenuTitleForUndoActionName: (NSString*)actionName
{
  if (actionName)
    {
      if ([actionName isEqual: @""])
	{
	  return _(@"Redo");
	}
      else
	{
	  return [NSString stringWithFormat: @"%@ %@", _(@"Redo"), actionName];
	}
    }
  return _(@"Redo");
}

/**
 * Registers an undo operation.
 * This method is the simple target-action-based undo registration
 * as opposed to the sophisticated [-forwardInvocation:]
 * mechanism. <br />
 * You invoke this method with the target of the
 * undo action providing the selector which can perform the undo with
 * the provided object.  The object is often a dictionary of the
 * identifying the attribute and their values before the change. The object
 * will be retained. The invocation will added to the current grouping.<br />
 * If the registrations have been disabled through [-disableUndoRegistration],
 * this method does nothing.<br />
 * Unless the receiver implicitly
 * groups operations by event, the this method must have been preceded
 * with a [-beginUndoGrouping] message.  Otherwise it will raise an
 * NSInternalInconsistencyException. <br />
 * Unless this method is invoked as part of a [-undo] or [-undoNestedGroup]
 * processing, the redo stack is cleared.<br />
 * If the receiver [-groupsByEvent] and this is the first call to this
 * method since the last run loop processing, this method sets up
 * the receiver to process the [-endUndoGrouping] at the
 * end of the event loop.
 */
- (void) registerUndoWithTarget: (id)target
		       selector: (SEL)aSelector
			 object: (id)anObject
{
  if (_disableCount == 0)
    {
      NSMethodSignature	*sig;
      NSInvocation	*inv;
      PrivateUndoGroup	*g;

      if (_group == nil)
	{
	  if ([self groupsByEvent])
	    {
	      [self _begin];
	    }
	  else
	    {
	      [NSException raise: NSInternalInconsistencyException
			   format: @"registerUndo without beginUndoGrouping"];
	    }
	}
      g = _group;
      sig = [target methodSignatureForSelector: aSelector];
      NSAssert2(sig,@"No methodSignatureForSelector:%@ for target of class %@",
		NSStringFromSelector(aSelector),
		[target class]);
      inv = [NSInvocation invocationWithMethodSignature: sig];
      [inv retainArgumentsIncludingTarget: NO];
      [inv setTarget: target];
      [inv setSelector: aSelector];
      [inv setArgument: &anObject atIndex: 2];
      [g addInvocation: inv];
      if (_isUndoing == NO && _isRedoing == NO)
	{
	  [_redoStack removeAllObjects];
	}
      if ((_runLoopGroupingPending == NO) && ([self groupsByEvent] == YES))
	{
	  [[NSRunLoop currentRunLoop] performSelector: @selector(_loop:)
				      target: self
				      argument: nil
				      order: NSUndoCloseGroupingRunLoopOrdering
				      modes: _modes];
	  _runLoopGroupingPending = YES;
	}
    }
}

/**
 * Removes all grouping stored in the receiver.  This clears the both
 * the undo and the redo stacks.  This method is if the sole client
 * of the undo manager will be unable to service any undo or redo events.
 * The client can call this method in its -dealloc method, unless the
 * undo manager has several clients, in which case
 * [-removeAllActionsWithTarget:] is more appropriate.
 */
- (void) removeAllActions
{
  while (_group != nil)
    {
      [self endUndoGrouping];
    }
  [_redoStack removeAllObjects];
  [_undoStack removeAllObjects];
  _isRedoing = NO;
  _isUndoing = NO;
  _disableCount = 0;
}

/**
 * Removes all actions recorded for the given target.  This method is
 * is useful when a client of the undo manager will be unable to
 * service any undo or redo events.  Clients should call this method
 * in their dealloc method, unless they are the sole client of the
 * undo manager in which case [-removeAllActions] is more appropriate.
 */
- (void) removeAllActionsWithTarget: (id)target
{
  unsigned 	i;

  i = [_redoStack count];
  while (i-- > 0)
    {
      PrivateUndoGroup	*g;

      g = [_redoStack objectAtIndex: i];
      if ([g removeActionsForTarget: target] == NO)
	{
	  [_redoStack removeObjectAtIndex: i];
	}
    }
  i = [_undoStack count];
  while (i-- > 0)
    {
      PrivateUndoGroup	*g;

      g = [_undoStack objectAtIndex: i];
      if ([g removeActionsForTarget: target] == NO)
	{
	  [_undoStack removeObjectAtIndex: i];
	}
    }
}

/**
 * Returns the run loop modes in which the receiver registers
 * the [-endUndoGrouping] processing when it [-groupsByEvent].
 */
- (NSArray*) runLoopModes
{
  return _modes;
}

/**
 * Sets the name associated with the actions of the current group.
 * Typically you can call this method while registering the actions
 * for the current group.  This name will be used to determine the
 * name in the [-undoMenuTitleForUndoActionName:] and
 * [-redoMenuTitleForUndoActionName:] names typically displayed
 * in the menu.
 */
- (void) setActionName: (NSString*)name
{
  if ((name != nil) && (_group != nil))
    {
      [_group setActionName: name];
    }
}

/**
 * Sets whether the receiver should implicitly call [-beginUndoGrouping] when
 * necessary and register a call to invoke [-endUndoGrouping] at the end
 * of the current event loop.  The grouping is turned on by default.
 */
- (void) setGroupsByEvent: (BOOL)flag
{
  if (_groupsByEvent != flag)
    {
      _groupsByEvent = flag;
    }
}

/**
 * Sets the maximum number of groups in either the undo or redo stack.
 * Use this method to limit memory usage if you either expect very many
 * actions to be recorded or the recorded objects require a lot of memory.
 * When set to 0 the stack size is limited by the range of a unsigned int,
 * available memory.
 */
- (void) setLevelsOfUndo: (NSUInteger)num
{
  _levelsOfUndo = num;
  if (num > 0)
    {
      while ([_undoStack count] > num)
	{
	  [_undoStack removeObjectAtIndex: 0];
	}
      while ([_redoStack count] > num)
	{
	  [_redoStack removeObjectAtIndex: 0];
	}
    }
}

/**
 * Sets the modes in which the receiver registers the calls
 * with the current run loop to invoke
 * [-endUndoGrouping] when it [-groupsByEvent].  This method
 * first cancels any pending registrations in the old modes and
 * registers the invocation in the new modes.
 */
- (void) setRunLoopModes: (NSArray*)newModes
{
  if (_modes != newModes)
    {
      ASSIGN(_modes, newModes);
      if (_runLoopGroupingPending)
	{
	  NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
	  [runLoop cancelPerformSelector: @selector(_loop:)
				  target: self
				argument: nil];
	  [runLoop performSelector: @selector(_loop:)
			    target: self
			  argument: nil
			     order: NSUndoCloseGroupingRunLoopOrdering
			     modes: _modes];
	}
    }
}

/**
 * This method performs an undo by invoking [-undoNestedGroup].
 * If current group of the receiver is the top group this method first
 * calls [-endUndoGrouping].  This method may only be called on the top
 * level group, otherwise it will raise an NSInternalInconsistencyException.
 */
- (void) undo
{
  if ([self groupingLevel] == 1)
    {
      [self endUndoGrouping];
    }
  if (_group != nil)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"undo with nested groups"];
    }
  [self undoNestedGroup];
}

/**
 * If the receiver can perform an undo, this method returns
 * the action name previously associated with the top grouping with
 * [-setActionName:].  This name should identify the action to be undone.
 * If there are no items on the undo stack this method returns nil.
 * If no action name has been set, this method returns an empty string.
 */
- (NSString*) undoActionName
{
  if ([self canUndo] == NO)
    {
      return nil;
    }

  if (_group != nil)
    {
      return [_group actionName];
    }
  else
    {
      return [[_undoStack lastObject] actionName];
    }
}

/**
 * Returns the full localized title of the actions to be displayed
 * as a menu item.  This method first invokes [-undoActionName] and
 * passes it to [-undoMenuTitleForUndoActionName:] and returns the result.
 */
- (NSString*) undoMenuItemTitle
{
  return [self undoMenuTitleForUndoActionName: [self undoActionName]];
}

/**
 * Returns the localized title of the actions to be displayed
 * as a menu item identified by actionName, by appending a
 * localized command string like @"Undo &lt;localized(actionName)&gt;".
 */
- (NSString*) undoMenuTitleForUndoActionName: (NSString*)actionName
{
  if (actionName)
    {
      if ([actionName isEqual: @""])
	{
	  return _(@"Undo");
	}
      else
	{
	  return [NSString stringWithFormat: @"%@ %@", _(@"Undo"), actionName];
	}
    }
  return _(@"Undo");
}

/**
 * Performs an undo by taking the top grouping
 * from the undo stack and invoking them.  This method posts an
 * NSUndoManagerCheckpointNotification notification to allow the client
 * to process any pending changes before proceeding.  If there are groupings
 * on the undo stack, the top object is popped off the stack and invoked
 * within a nested beginUndoGrouping/endUndoGrouping.  During this
 * processing, the undo operations registered for undo are recorded on the redo
 * stack.<br />
 */
- (void) undoNestedGroup
{
  NSString *name = nil;
  PrivateUndoGroup	*oldGroup;
  PrivateUndoGroup	*groupToUndo;

  [[NSNotificationCenter defaultCenter]
      postNotificationName: NSUndoManagerCheckpointNotification
		    object: self];

  if (_group != nil)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"undoNestedGroup before endUndoGrouping"];
    }

  if (_isUndoing || _isRedoing)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"undoNestedGroup while undoing or redoing"];
    }

  if ([_undoStack count] == 0)
    {
      return;
    }

  [[NSNotificationCenter defaultCenter]
      postNotificationName: NSUndoManagerWillUndoChangeNotification
		    object: self];

  oldGroup = _group;
  _group = nil;
  _isUndoing = YES;

  if (oldGroup)
    {
      groupToUndo = oldGroup;
      oldGroup = RETAIN([oldGroup parent]);
      [groupToUndo orphan];
      [_redoStack addObject: groupToUndo];
    }
  else
    {
      groupToUndo = RETAIN([_undoStack lastObject]);
      [_undoStack removeLastObject];
    }

  name = [NSString stringWithString: [groupToUndo actionName]];

  [self _begin];
  [groupToUndo perform];
  RELEASE(groupToUndo);
  [self endUndoGrouping];

  _isUndoing = NO;
  _group = oldGroup;

  [[_redoStack lastObject] setActionName: name];

  [[NSNotificationCenter defaultCenter]
      postNotificationName: NSUndoManagerDidUndoChangeNotification
		    object: self];
}

@end

/*
 * Category with auxiliary method to support coalescing undo actions
 * for typing events in NSTextView. However, the implementation is
 * not restricted to that purpose.
 */
@interface NSUndoManager(UndoCoalescing)
- (BOOL) _canCoalesceUndoWithTarget: (id)target
			   selector: (SEL)aSelector
			     object: (id)anObject;
@end

@implementation NSUndoManager(UndoCoalescing)
- (BOOL) _canCoalesceUndoWithTarget: (id)target
			   selector: (SEL)aSelector
			     object: (id)anObject
{
  if (_isUndoing == NO && _isRedoing == NO && [_undoStack count] > 0)
    {
      int      i;
      NSArray *a = [[_undoStack lastObject] actions];

      for (i = 0; i < [a count]; i++)
        {
	  NSInvocation *inv = [a objectAtIndex: i];
	  if ([inv target] == target && [inv selector] == aSelector)
	    {
	      id object;
	      [inv getArgument: &object atIndex: 2];
	      if (object == anObject)
		return YES;
	    }
	}
    }
  return NO;
}
@end

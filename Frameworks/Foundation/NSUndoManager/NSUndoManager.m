/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSUndoManager.h>
#import <Foundation/NSUndoGroup.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSNotificationCenter.h>
#import <Foundation/NSException.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSInvocation.h>

enum _NSUndoManagerState {
    NSUndoManagerNormal,
    NSUndoManagerUndoing,
    NSUndoManagerRedoing
};

NSString * const NSUndoManagerCheckpointNotification=@"NSUndoManagerCheckpointNotification";
NSString * const NSUndoManagerDidOpenUndoGroupNotification=@"NSUndoManagerDidOpenUndoGroupNotification";
NSString * const NSUndoManagerWillCloseUndoGroupNotification=@"NSUndoManagerWillCloseUndoGroupNotification";
NSString * const NSUndoManagerWillUndoChangeNotification=@"NSUndoManagerWillUndoChangeNotification";
NSString * const NSUndoManagerDidUndoChangeNotification=@"NSUndoManagerDidUndoChangeNotification";
NSString * const NSUndoManagerWillRedoChangeNotification=@"NSUndoManagerWillRedoChangeNotification";
NSString * const NSUndoManagerDidRedoChangeNotification=@"NSUndoManagerDidRedoChangeNotification";

@implementation NSUndoManager

-(void)_registerPerform {
   if(!_performRegistered){
    _performRegistered=YES;
    [[NSRunLoop currentRunLoop] performSelector:@selector(runLoopUndo:) target:self argument:nil order:NSUndoCloseGroupingRunLoopOrdering modes:_modes];

   }
}

-(void)_unregisterPerform {
   if(_performRegistered){
    _performRegistered=NO;
    [[NSRunLoop currentRunLoop] cancelPerformSelector:@selector(runLoopUndo:) target:self argument:nil];    
   }
}


- (id)init
{
    _undoStack = [[NSMutableArray alloc] init];
    _redoStack = [[NSMutableArray alloc] init];
    _state = NSUndoManagerNormal;

    [self setRunLoopModes:[NSArray arrayWithObject:NSDefaultRunLoopMode]];
    [self setGroupsByEvent:YES];
    _performRegistered=NO;

    return self;
}

- (void)dealloc
{
   [self _unregisterPerform];

    [_undoStack release];
    [_redoStack release];
    [_currentGroup release];
    [_modes release];
    [_actionName release];
        
    [super dealloc];
}

- (NSArray *)runLoopModes
{
    return _modes;
}

- (NSUInteger)levelsOfUndo
{
    return _levelsOfUndo;
}

- (BOOL)groupsByEvent
{
    return _groupsByEvent;
}

- (void)setRunLoopModes:(NSArray *)modes
{
    [_modes release];
    _modes = [modes retain];
   [self _unregisterPerform];
   if (_groupsByEvent)
    [self _registerPerform];
}

- (void)setLevelsOfUndo:(NSUInteger)levels
{
    _levelsOfUndo = levels;
    while ([_undoStack count] > _levelsOfUndo)
        [_undoStack removeObjectAtIndex:0];
    while ([_redoStack count] > _levelsOfUndo)
        [_redoStack removeObjectAtIndex:0];
}

- (void)setGroupsByEvent:(BOOL)flag {
    _groupsByEvent = flag;
   if (_groupsByEvent)
    [self _registerPerform];
   else
    [self _unregisterPerform];
}

- (BOOL)isUndoRegistrationEnabled
{
    return (_disableCount == 0);
}

- (void)disableUndoRegistration
{
    _disableCount++;
}

- (void)enableUndoRegistration
{
    if (_disableCount == 0)
        [NSException raise:NSInternalInconsistencyException
                    format:@"Attempt to enable registration with no disable message in effect"];
    
    _disableCount--;
}

- (void)beginUndoGrouping
{
    NSUndoGroup *undoGroup = [NSUndoGroup undoGroupWithParentGroup:_currentGroup];

    if (!([_currentGroup parentGroup] == nil && _state == NSUndoManagerUndoing))
        [[NSNotificationCenter defaultCenter] postNotificationName:NSUndoManagerCheckpointNotification
                                                            object:self];

    [_currentGroup release];
    _currentGroup = [undoGroup retain];

    [[NSNotificationCenter defaultCenter] postNotificationName:NSUndoManagerDidOpenUndoGroupNotification object:self];
}

- (void)endUndoGrouping
{
    NSMutableArray *stack = nil;
    NSUndoGroup *parentGroup = [[_currentGroup parentGroup] retain];

    if (_currentGroup == nil)
        [NSException raise:NSInternalInconsistencyException
                    format:@"endUndoGrouping called without first calling beginUndoGrouping"];

    [[NSNotificationCenter defaultCenter] postNotificationName:NSUndoManagerCheckpointNotification
                                                        object:self];

    if (parentGroup == nil && [[_currentGroup invocations] count] > 0) {
        switch (_state) {
            case NSUndoManagerNormal:
                [[NSNotificationCenter defaultCenter] postNotificationName:NSUndoManagerWillCloseUndoGroupNotification object:self];
                
            case NSUndoManagerRedoing:
                stack = _undoStack;
                break;

            case NSUndoManagerUndoing:
                stack = _redoStack;
                break;
        }

        [stack addObject:_currentGroup];
        if (_levelsOfUndo > 0)
            if ([stack count] > _levelsOfUndo)
                [stack removeObjectAtIndex:0];
    }
    else {
        // a nested group was closed. fold its invocations into its parent, preserving the
        // order for future changes on the parent.
        [parentGroup addInvocationsFromArray:[_currentGroup invocations]];
    }

    [_currentGroup release];
    _currentGroup = parentGroup;
}

- (NSInteger)groupingLevel
{
    NSUndoGroup *temp = _currentGroup;
    int level = (_currentGroup != nil);

    while ((temp = [temp parentGroup])!=nil)
        level++;

    return level;
}

- (void)runLoopUndo:(id)dummy
{
    if (_groupsByEvent == YES) {
        if (_currentGroup != nil)
            [self endUndoGrouping];
		_performRegistered=NO;
    }
}

- (BOOL)canUndo
{
    if ([_undoStack count] > 0)
        return YES;

    if ([[_currentGroup invocations] count] > 0)
        return YES;
    
    return NO;
}

- (void)undoNestedGroup
{
    NSUndoGroup *undoGroup;

    if (_currentGroup != nil)
        [NSException raise:NSInternalInconsistencyException
                    format:@"undo called with open nested group"];

    [[NSNotificationCenter defaultCenter] postNotificationName:NSUndoManagerCheckpointNotification
                                                        object:self];

    [[NSNotificationCenter defaultCenter] postNotificationName:NSUndoManagerWillUndoChangeNotification
                                                        object:self];

    _state = NSUndoManagerUndoing;
    undoGroup = [[_undoStack lastObject] retain];
    [_undoStack removeLastObject];
    [self beginUndoGrouping];
    [undoGroup invokeInvocations];
    [self endUndoGrouping];
    [undoGroup release];
    _state = NSUndoManagerNormal;

    [[NSNotificationCenter defaultCenter] postNotificationName:NSUndoManagerDidUndoChangeNotification
                                                        object:self];
}

- (void)undo
{
    if ([self groupingLevel] == 1)
        [self endUndoGrouping];
    
    [self undoNestedGroup];
}

- (BOOL)isUndoing
{
    return (_state == NSUndoManagerUndoing);
}


- (BOOL)canRedo
{
    [[NSNotificationCenter defaultCenter] postNotificationName:NSUndoManagerCheckpointNotification
                                                        object:self];
    return ([_redoStack count] > 0);
}

- (void)redo
{
    NSUndoGroup *undoGroup;

    if (_state == NSUndoManagerUndoing)
        [NSException raise:NSInternalInconsistencyException
                    format:@"redo called while undoing"];

    [[NSNotificationCenter defaultCenter] postNotificationName:NSUndoManagerCheckpointNotification
                                                        object:self];

    [[NSNotificationCenter defaultCenter] postNotificationName:NSUndoManagerWillRedoChangeNotification
                                                        object:self];

    _state = NSUndoManagerRedoing;
    undoGroup = [[_redoStack lastObject] retain];
    [_redoStack removeLastObject];
    [self beginUndoGrouping];
    [undoGroup invokeInvocations];
    [self endUndoGrouping];
    [undoGroup release];
    _state = NSUndoManagerNormal;

    [[NSNotificationCenter defaultCenter] postNotificationName:NSUndoManagerDidRedoChangeNotification
                                                        object:self];
}


- (BOOL)isRedoing
{
    return (_state == NSUndoManagerRedoing);
}

- (void)registerUndoWithTarget:(id)target selector:(SEL)selector object:(id)object
{
    NSInvocation *invocation;
    NSMethodSignature *signature;
    
    if (_disableCount > 0)
        return;

	if (_groupsByEvent && _currentGroup == nil) {
		[self _registerPerform];
        [self beginUndoGrouping];
	}		
	
    if (_currentGroup == nil)
        [NSException raise:NSInternalInconsistencyException
                    format:@"forwardInvocation called without first opening an undo group"];

    signature = [target methodSignatureForSelector:selector];
    invocation = [NSInvocation invocationWithMethodSignature:signature];

    [invocation setTarget:target];
    [invocation setSelector:selector];
    [invocation setArgument:&object atIndex:2];
    [invocation retainArguments];

    [_currentGroup addInvocation:invocation];

    if (_state == NSUndoManagerNormal)
        [_redoStack removeAllObjects];
}

- (void)removeAllActions
{
    [_undoStack removeAllObjects];
    [_redoStack removeAllObjects];
    _disableCount = 0;
}

- (void)removeAllActionsWithTarget:(id)target
{
    NSUndoGroup *undoGroup;
    int i;

    [_currentGroup removeInvocationsWithTarget:target];

    for (i = 0; i < [_undoStack count]; ++i) {
        undoGroup = [_undoStack objectAtIndex:i];

        [undoGroup removeInvocationsWithTarget:target];
        if ([[undoGroup invocations] count] == 0)
            [_undoStack removeObject:undoGroup];
    }
    for (i = 0; i < [_redoStack count]; ++i) {
        undoGroup = [_redoStack objectAtIndex:i];

        [undoGroup removeInvocationsWithTarget:target];
        if ([[undoGroup invocations] count] == 0)
            [_redoStack removeObject:undoGroup];
    }
}

- (id)prepareWithInvocationTarget:(id)target
{
    _preparedTarget = [target retain];
    
    return self;
}

-(NSMethodSignature *)methodSignatureForSelector:(SEL)selector {
    return [_preparedTarget methodSignatureForSelector:selector];
}

- (void)forwardInvocation:(NSInvocation *)invocation
{
    if (_disableCount > 0)
        return;
    
    if (_preparedTarget == nil)
        [NSException raise:NSInternalInconsistencyException
                    format:@"forwardInvocation called without first preparing a target"];
	
	if (_groupsByEvent && _currentGroup == nil) {
		[self _registerPerform];
		[self beginUndoGrouping];
	}		
	
    if (_currentGroup == nil)
        [NSException raise:NSInternalInconsistencyException
                    format:@"forwardInvocation called without first opening an undo group"];

    [invocation setTarget:_preparedTarget];
    [_currentGroup addInvocation:invocation];
    [invocation retainArguments];

    if (_state == NSUndoManagerNormal)
        [_redoStack removeAllObjects];
    
    [_preparedTarget release];
    _preparedTarget = nil;
}

- (void)setActionName:(NSString *)name
{
    [_actionName release];
    _actionName = [name retain];
}

- (NSString *)undoActionName
{
    if ([self canUndo])
        return _actionName;
    
    return nil;
}

- (NSString *)undoMenuItemTitle
{
    return [self undoMenuTitleForUndoActionName:[self undoActionName]];
}

// needs localization
- (NSString *)undoMenuTitleForUndoActionName:(NSString *)name
{
    if (name != nil) {
        if ([name length] > 0)
            return [NSString stringWithFormat:@"Undo %@", name];

        return @"Undo";
    }

    return name;
}

- (NSString *)redoActionName
{
    if ([self canRedo])
        return _actionName;

    return nil;
}

- (NSString *)redoMenuItemTitle
{
    return [self redoMenuTitleForUndoActionName:[self redoActionName]];
}

- (NSString *)redoMenuTitleForUndoActionName:(NSString *)name
{
    if (name != nil) {
        if ([name length] > 0)
            return [NSString stringWithFormat:@"Redo %@", name];

        return @"Redo";
    }

    return name;
}

- (void)clearRedoStackIfStateIsNormal
{
  if (_state == NSUndoManagerNormal)
      [_redoStack removeAllObjects];
}

@end


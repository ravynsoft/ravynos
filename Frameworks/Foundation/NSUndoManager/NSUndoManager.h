/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>

@class NSArray, NSMutableArray, NSInvocation;

enum {
    NSUndoCloseGroupingRunLoopOrdering = 350000,
};

FOUNDATION_EXPORT NSString *const NSUndoManagerCheckpointNotification;

FOUNDATION_EXPORT NSString *const NSUndoManagerDidOpenUndoGroupNotification;
FOUNDATION_EXPORT NSString *const NSUndoManagerWillCloseUndoGroupNotification;

FOUNDATION_EXPORT NSString *const NSUndoManagerWillUndoChangeNotification;
FOUNDATION_EXPORT NSString *const NSUndoManagerDidUndoChangeNotification;

FOUNDATION_EXPORT NSString *const NSUndoManagerWillRedoChangeNotification;
FOUNDATION_EXPORT NSString *const NSUndoManagerDidRedoChangeNotification;

@interface NSUndoManager : NSObject {
    NSMutableArray *_undoStack;
    NSMutableArray *_redoStack;
    BOOL _groupsByEvent;
    NSArray *_modes;
    int _disableCount;
    NSInteger _levelsOfUndo;
    id _currentGroup;
    int _state;
    NSString *_actionName;
    id _preparedTarget;
    BOOL _performRegistered;
}

- (NSArray *)runLoopModes;
- (NSUInteger)levelsOfUndo;
- (BOOL)groupsByEvent;

- (void)setRunLoopModes:(NSArray *)modes;
- (void)setLevelsOfUndo:(NSUInteger)levels;
- (void)setGroupsByEvent:(BOOL)flag;

- (BOOL)isUndoRegistrationEnabled;
- (void)disableUndoRegistration;
- (void)enableUndoRegistration;

- (void)beginUndoGrouping;
- (void)endUndoGrouping;

- (NSInteger)groupingLevel;

- (BOOL)canUndo;
- (void)undo;
- (void)undoNestedGroup;
- (BOOL)isUndoing;

- (BOOL)canRedo;
- (void)redo;
- (BOOL)isRedoing;

- (void)registerUndoWithTarget:(id)target selector:(SEL)selector object:(id)object;

- (void)removeAllActions;
- (void)removeAllActionsWithTarget:(id)target;

- (id)prepareWithInvocationTarget:(id)target;
- (void)forwardInvocation:(NSInvocation *)invocation;

- (NSString *)undoActionName;
- (NSString *)undoMenuItemTitle;
- (NSString *)undoMenuTitleForUndoActionName:(NSString *)name;
- (void)setActionName:(NSString *)name;

- (NSString *)redoActionName;
- (NSString *)redoMenuItemTitle;
- (NSString *)redoMenuTitleForUndoActionName:(NSString *)name;

- (void)clearRedoStackIfStateIsNormal;
@end

/* Copyright (c) 2009 Andy Balholm
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSCellUndoManager.h"

#import <Foundation/Foundation.h>


@implementation NSCellUndoManager

- (void)dealloc
{
  [_nextUndoManager release];
  [super dealloc];
}

- (BOOL)canUndo
{
  return [super canUndo] || [_nextUndoManager canUndo];
}

- (void)undo
{
  if ([super canUndo])
    {
      [super undo];
    }
  else
    {
      [_nextUndoManager undo];
    }
}

- (BOOL)canRedo
{
  return [_nextUndoManager canRedo] || [super canRedo];
}

- (void)redo
{
  if ([_nextUndoManager canRedo])
    {
      [_nextUndoManager redo];
    }
  else
    {
      [super redo];
    }
}

- (void)setNextUndoManager:(NSUndoManager *)manager
{
  [manager retain];
  [_nextUndoManager release];
  _nextUndoManager = manager;
}

- (void)forwardInvocation:(NSInvocation *)invocation
{
  [super forwardInvocation:invocation];
  [_nextUndoManager clearRedoStackIfStateIsNormal];
}

- (void)registerUndoWithTarget:(id)target selector:(SEL)selector object:(id)object
{
  [super registerUndoWithTarget:target selector:selector object:object];
  [_nextUndoManager clearRedoStackIfStateIsNormal];
}

@end

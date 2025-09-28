/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - David Young <daver@geeks.org>
#import <Foundation/NSUndoGroup.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSInvocation.h>
#import <Foundation/NSString.h>

@implementation NSUndoGroup

+ (NSUndoGroup *)undoGroupWithParentGroup:(NSUndoGroup *)parentGroup
{
    return [[[self alloc] initWithParentGroup:parentGroup] autorelease];
}

- (id)initWithParentGroup:(NSUndoGroup *)parentGroup
{
    _parentGroup = [parentGroup retain];
    _invocations = [[NSMutableArray alloc] init];

    return self;
}

- (void)dealloc
{
    [_parentGroup release];
    [_invocations release];

    [super dealloc];
}

- (NSUndoGroup *)parentGroup
{
    return _parentGroup;
}

- (void)addInvocation:(NSInvocation *)invocation
{
    [_invocations addObject:invocation];
}

- (void)addInvocationsFromArray:(NSArray *)array
{
    [_invocations addObjectsFromArray:array];
}

- (NSArray *)invocations
{
    return _invocations;
}

- (void)removeInvocationsWithTarget:(id)target
{
    int i;

    for (i = 0; i < [_invocations count]; ++i)
        if ([[_invocations objectAtIndex:i] target] == target)
            [_invocations removeObjectAtIndex:i];
}

- (void)invokeInvocations
{
    // LIFO
    while ([_invocations count] > 0) {
        [[_invocations lastObject] invoke];
        [_invocations removeLastObject];
    }
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@[0x%lx] parentGroup: %@, %d invocations>",
        [self class], self, _parentGroup, [_invocations count]];
}

@end

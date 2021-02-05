/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSSet_placeholder.h>
#import <Foundation/NSSet_concrete.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSRaise.h>

@implementation NSSet_placeholder


- init
{
    [self dealloc];
    return (NSSet_placeholder *)NSSet_concreteNew(NULL, NULL, 0);
}


- initWithArray:(NSArray *)array
{
    NSUInteger count = [array count];
    id *objects = __builtin_alloca(sizeof(id) * count);

    [array getObjects:objects];

    [self dealloc];

    return (NSSet_placeholder *)NSSet_concreteNew(NULL, objects, count);
}


- initWithObjects:(id *)objects count:(NSUInteger)count
{
    [self dealloc];
    return (NSSet_placeholder *)NSSet_concreteNew(NULL, objects, count);
}


- initWithObjects:(id)object,...
{
    va_list arguments;
    NSUInteger i, count;
    id *objects;

    if (object == nil) {
        return [self init];
    }

    va_start(arguments, object);
    count = 1;
    while (va_arg(arguments, id) != nil) {
        count++;
    }
    va_end(arguments);

    objects = __builtin_alloca(sizeof(id) * count);

    va_start(arguments, object);
    objects[0] = object;
    for (i = 1; i < count; i++) {
        objects[i] = va_arg(arguments, id);
    }
    va_end(arguments);

    [self dealloc];

    return (NSSet_placeholder *)NSSet_concreteNew(NULL, objects, count);
}


@end

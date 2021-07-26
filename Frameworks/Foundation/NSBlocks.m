#import <Foundation/NSProcessInfo.h>
#import <stdio.h>
#import <string.h>
#include <Block_private.h>
#include <Block.h>
#include <objc/objc-class.h>

/* Copyright (c) 2011 Tobias Platen
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */



static void init_block_class(void* nsblock,char* class)
{
	memcpy(nsblock,objc_getClass(class),sizeof(struct objc_class));
	//TODO support objective_c garbage collection
}


@interface NSBlock : NSObject
@end
@implementation NSBlock
-(void) invoke
{
	void (^blk)()= (void (^)())self;
	blk();
}
- (id) copyWithZone: (NSZone *) zone 
{
	return (id) _Block_copy(self);
}
- (void) release
{
	_Block_release(self);
}

+(void) load
{
//	object_setClass(self, (Class) &_NSConcreteStackBlock);	
	init_block_class((Class) &_NSConcreteStackBlock,"NSBlock");
	init_block_class((Class) &_NSConcreteMallocBlock,"NSBlock");
	init_block_class((Class) &_NSConcreteAutoBlock,"NSBlock");
	init_block_class((Class) &_NSConcreteFinalizingBlock,"NSBlock");
	init_block_class((Class) &_NSConcreteGlobalBlock,"NSBlock");
	init_block_class((Class) &_NSConcreteWeakBlockVariable,"NSBlock");
	//TODO support objective-c gc
	[^(){printf("cocotron blocks runtime selftest\n");} invoke];
}
@end


//
//  MRCBase.m
//  TestARCLayouts
//
//  Created by Patrick Beard on 3/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "MRCBase.h"
#include "test.h"

// MRCBase->alignment ensures that there is a gap between the end of 
// NSObject's ivars and the start of MRCBase's ivars, which exercises 
// handling of storage that is not represented in any class's ivar 
// layout bitmaps.

#if __has_feature(objc_arc_weak)
bool supportsMRCWeak = true;
#else
bool supportsMRCWeak = false;
#endif

@interface MRCBase () {
@private
    double DOUBLEWORD_ALIGNED alignment;
    uintptr_t pad[3]; // historically this made OBJC2 layout bitmaps match OBJC1
    double number;
    id object;
    void *pointer;
#if __has_feature(objc_arc_weak)
    __weak 
#endif
    id delegate;
}
@end

@implementation MRCBase
@synthesize number, object, pointer, delegate;
@end

// Call object_copy from MRC.
extern id __attribute__((ns_returns_retained)) 
docopy(id obj)
{
    return object_copy(obj, 0);
}

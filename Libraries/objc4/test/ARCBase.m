//
//  ARCBase.m
//  TestARCLayouts
//
//  Created by Patrick Beard on 3/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "ARCBase.h"

// ARCMisalign->misalign1 and ARCBase->misalign2 together cause 
// ARCBase's instanceStart to be misaligned, which exercises handling 
// of storage that is not represented in the class's ivar layout bitmaps.

@implementation ARCMisalign
@end

@interface ARCBase () {
@private
    char misalign2;
    long number;
    id object;
    void *pointer;
    __weak id delegate;
}
@end

@implementation ARCBase
@synthesize number, object, pointer, delegate;
@end

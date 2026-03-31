//
//  MRCBase.h
//  TestARCLayouts
//
//  Created by Patrick Beard on 3/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/NSObject.h>

// YES if MRC compiler supports ARC-style weak
extern bool supportsMRCWeak;

#if __LP64__
#define DOUBLEWORD_ALIGNED __attribute__((aligned(16)))
#else
#define DOUBLEWORD_ALIGNED __attribute__((aligned(8)))
#endif

@interface MRCBase : NSObject
@property double number;
@property(retain) id object;
@property void *pointer;
@property(weak) __weak id delegate;
@end

// Call object_copy from MRC.
extern id __attribute__((ns_returns_retained)) docopy(id obj);

//
//  ARCBase.h
//  TestARCLayouts
//
//  Created by Patrick Beard on 3/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/NSObject.h>

@interface ARCMisalign : NSObject {
    char misalign1;
}
@end

@interface ARCBase : ARCMisalign
@property long number;
@property(retain) id object;
@property void *pointer;
@property(weak) __weak id delegate;
@end

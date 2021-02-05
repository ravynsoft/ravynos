//
//  NSCondition_posix.h
//  Foundation
//
//  Created by Sven Weidauer on 08.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Foundation/NSLock.h>
#include <pthread.h>

@interface NSCondition_posix : NSCondition {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
}

@end

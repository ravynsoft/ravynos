/* Copyright (c) 2008 Johannes Fortmann

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef PLATFORM_IS_POSIX
#import <Foundation/NSConditionLock_posix.h>
#import <Foundation/NSException.h>
#import <Foundation/NSRaiseException.h>
#import <Foundation/NSString.h>
#import <Foundation/NSThread.h>
#include <sys/time.h>
#include <math.h>
#include <errno.h>

@implementation NSConditionLock_posix
-(id)init {
   return [self initWithCondition:0];
}

-(id)initWithCondition:(NSInteger)value {
   if((self = [super init])) {
      pthread_cond_init(&_cond, NULL);
      pthread_mutex_init(&_mutex, NULL);
      _value=value;
   }
   return self;
}

-(void)dealloc {
   pthread_mutex_destroy(&_mutex);
   pthread_cond_destroy(&_cond);
   [_name release]; _name = nil;
   [super dealloc];
}

-(NSInteger)condition {
   return _value;
}

-(void)lock {
    int rc;
    if((rc = pthread_mutex_lock(&_mutex)) != 0) {
        [NSException raise:NSInvalidArgumentException format:@"failed to lock %@ (errno: %d)", self, rc];
    }
    _lockingThread=NSCurrentThread();
}

-(void)unlock {
   if(_lockingThread!=NSCurrentThread()) {
       NSCLog("trying to unlock 0x%x from thread 0x%x, was locked from 0x%x", self, NSCurrentThread(), _lockingThread);
       return;
   }

   _lockingThread=nil;
   pthread_mutex_unlock(&_mutex);
}

-(BOOL)tryLock {
   if(pthread_mutex_trylock(&_mutex))
      return NO;
   _lockingThread=NSCurrentThread();
   return YES;
}

-(BOOL)tryLockWhenCondition:(NSInteger)condition {
   if([self tryLock]) {
      return NO;
   }

   if(_value==condition) {
      return YES;
   }
   [self unlock];
   return NO;
}

-(void)lockWhenCondition:(NSInteger)condition {

    int rc;

    if((rc = pthread_mutex_lock(&_mutex)) != 0) {
        [NSException raise:NSInvalidArgumentException format:@"failed to lock %@ (errno: %d)", self, rc];
    }

    while(_value!=condition) {
        switch ((rc = pthread_cond_wait(&_cond, &_mutex))) {
            case 0:
                break;
            default: {
                int r;
                if((r = pthread_mutex_unlock(&_mutex)) != 0) {
                    [NSException raise:NSInvalidArgumentException format:@"failed to unlock %@ (errno: %d)", self, r];
                }
                [NSException raise:NSInvalidArgumentException format:@"failed to lock %@ (errno: %d)", self, rc];
            }
        }

    }

    _lockingThread=NSCurrentThread();
}

-(void)unlockWithCondition:(NSInteger)condition {
   if(_lockingThread!=NSCurrentThread()) {
       NSCLog("trying to unlock 0x%x from thread 0x%x, was locked from 0x%x", self, NSCurrentThread(), _lockingThread);
       return;
   }

    _lockingThread=nil;
    _value=condition;
    int rc;
    if((rc = pthread_mutex_unlock(&_mutex)) != 0) {
        [NSException raise:NSInvalidArgumentException format:@"failed to unlock %@ (errno: %d)", self, rc];
    }
    if((rc = pthread_cond_broadcast(&_cond)) != 0) {
        [NSException raise:NSInvalidArgumentException format:@"failed to broadcast %@ (errno: %d)", self, rc];
    }
}

-(BOOL)lockBeforeDate:(NSDate *)date {
    struct timeval tv;
    struct timespec t;
    int rc;
    gettimeofday(&tv,NULL);
    NSTimeInterval d=[date timeIntervalSinceNow];
    t.tv_sec= tv.tv_sec + (unsigned int)d;
    t.tv_nsec=tv.tv_usec*1000 + fmod(d, 1.0)*1000000.0;
    
    if (t.tv_nsec >= 1000000000) {
        t.tv_sec++;
        t.tv_nsec -= 1000000000;
    }

    if((rc = pthread_mutex_lock(&_mutex)) != 0) {
        [NSException raise:NSInvalidArgumentException format:@"failed to lock %@ (errno: %d)", self, rc];
    }

    switch ((rc = pthread_cond_timedwait(&_cond, &_mutex, &t))) {
        case 0:
            _lockingThread=NSCurrentThread();
            return YES;
        case ETIMEDOUT:
            if((rc = pthread_mutex_unlock(&_mutex)) != 0) {
                [NSException raise:NSInvalidArgumentException format:@"failed to unlock %@ (errno: %d)", self, rc];
            }
            return NO;
        default: {
            int r;
            if((r = pthread_mutex_unlock(&_mutex)) != 0) {
                [NSException raise:NSInvalidArgumentException format:@"failed to unlock %@ (errno: %d)", self, r];
            }
            [NSException raise:NSInvalidArgumentException format:@"failed to lock %@ before date %@ (errno: %d)", self, date, rc];
            return NO;
        }
    }
}

-(BOOL)lockWhenCondition:(NSInteger)condition beforeDate:(NSDate *)date {
    struct timeval tv;
    struct timespec t;
    int rc;
    gettimeofday(&tv,NULL);
    NSTimeInterval d=[date timeIntervalSinceNow];
    t.tv_sec= tv.tv_sec + (unsigned int)d;
    t.tv_nsec=tv.tv_usec*1000 + fmod(d, 1.0)*1000000.0;

    if (t.tv_nsec >= 1000000000) {
        t.tv_sec++;
        t.tv_nsec -= 1000000000;
    }

    if((rc = pthread_mutex_lock(&_mutex)) != 0) {
        [NSException raise:NSInvalidArgumentException format:@"failed to lock %@ (errno: %d)", self, rc];
    }

    while(_value!=condition) {
        switch ((rc = pthread_cond_timedwait(&_cond, &_mutex, &t))) {
            case 0:
                break;
            case ETIMEDOUT:
                if((rc = pthread_mutex_unlock(&_mutex)) != 0) {
                    [NSException raise:NSInvalidArgumentException format:@"failed to unlock %@ (errno: %d)", self, rc];
                }
                return NO;
            default: {
                int r;
                if((r = pthread_mutex_unlock(&_mutex)) != 0) {
                    [NSException raise:NSInvalidArgumentException format:@"failed to unlock %@ (errno: %d)", self, r];
                }
                [NSException raise:NSInvalidArgumentException format:@"failed to lock %@ before date %@ (errno: %d)", self, date, rc];
                return NO;
            }
        }
    }

    _lockingThread=NSCurrentThread();
    return YES;
}

- (NSString *)name {
    return _name; }

- (void)setName:(NSString *)name {
    [_name release];
    _name = [name copy];
}

@end
#endif

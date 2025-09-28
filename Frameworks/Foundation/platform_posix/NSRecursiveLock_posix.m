/* Copyright (c) 2008 Johannes Fortmann
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef PLATFORM_IS_POSIX
#import "NSRecursiveLock_posix.h"
#import <Foundation/NSRecursiveLock.h>
#import <Foundation/NSRaiseException.h>
#import <Foundation/NSPlatform.h>

@implementation NSRecursiveLock_posix

-(id)init
{
    _lock=[NSLock new];

    return self;
}

-(void)dealloc
{
    [_lock release];
    [super dealloc];
}


-(void)lock
{
    if(_lockingThread==[NSThread currentThread])
    {
        _numberOfLocks++;
        return;
    }
    
    [_lock lock];
    // got the lock. so it's ours now
    _lockingThread=[NSThread currentThread];
    _numberOfLocks=1;
}

-(void)unlock
{
    if(_lockingThread==[NSThread currentThread])
    {
        _numberOfLocks--;
        if(_numberOfLocks==0)
        {
            _lockingThread=nil;
            [_lock unlock];
        }
    }
    else
        NSCLog("tried to unlock lock 0x%x owned by thread 0x%x from thread 0x%x", self, _lockingThread, [NSThread currentThread]);
}

-(BOOL)tryLock
{
    BOOL ret=[_lock tryLock];
    if(ret)
    {
        // got the lock. so it's ours now
        _lockingThread=[NSThread currentThread];
        _numberOfLocks=1;
        return YES;
    }
    else if(_lockingThread==[NSThread currentThread])
    {
        // didn't get the lock, but just because our thread already had it
        _numberOfLocks++;
        return YES;
    }
    return NO;
}

-(BOOL)lockBeforeDate:(NSDate *)value;
{
    if([self tryLock])
        return YES;
    // tryLock failed. That means someone else owns the lock. So we wait it out:
    BOOL ret=[_lock lockBeforeDate:value];
    if(ret)
    {
        _lockingThread=[NSThread currentThread];
        _numberOfLocks=1;
    }
    return ret;
}

-(BOOL)isLocked
{
    return _numberOfLocks!=0;
}

-(id)description
{
    return [NSString stringWithFormat:@"(%@, name %@, locked %i times", [super description], _name, _numberOfLocks];
}
@end
#endif


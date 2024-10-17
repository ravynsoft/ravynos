/*
 * Copyright (C) 2022-2024 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import <Foundation/NSMutableArray.h>
#import <Foundation/NSString.h>
#import "common.h"
#import "WindowServer.h"

@implementation WSAppRecord
-init {
    _windows = [NSMutableArray new];
    _mouseCursorConnected = YES;
    return self;
}

-(void)addWindow:(WSWindowRecord *)window {
    [_windows addObject:window];
}

-(void)removeWindowWithID:(int)number {
    for(int i = 0; i < [_windows count]; i++) {
        WSWindowRecord *r = [_windows objectAtIndex:i];
        if(r.number == number) {
            [_windows removeObjectAtIndex:i];
            return;
        }
    }
}

-(WSWindowRecord *)windowWithID:(int)number {
    for(int i = 0; i < [_windows count]; i++) {
        WSWindowRecord *r = [_windows objectAtIndex:i];
        if(r.number == number) {
            return r;
        }
    }
    return nil;
}

-(NSMutableArray *)windows {
    return _windows;
}

-(void)mouseCursorConnected:(int)connected {
    _mouseCursorConnected = connected;
}

-(BOOL)mouseCursorConnected {
    return _mouseCursorConnected;
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@ 0x%x> %@ pid:%u port:%u windows:%u",
           [self class], (uint32_t)self, self.bundleID, self.pid, self.port,
           [[self windows] count]];
}

@end


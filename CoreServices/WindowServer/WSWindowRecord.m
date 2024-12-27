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

#import <Onyx2D/O2Context.h>
#import <Onyx2D/O2Surface.h>
#import <Onyx2D/O2Image.h>
#import <Onyx2D/O2ImageSource.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSWindow.h>
#import "common.h"
#import "WindowServer.h"

@implementation WSWindowRecord
-init {
    self = [super init];
    _level = kCGNormalWindowLevelKey;
    return self;
}

-(void)dealloc {
    if(_surfaceBuf != NULL)
        munmap(_surfaceBuf, _bufSize);
    if(shm_unlink([_shmPath cString]) != 0)
        perror("shm_unlink");
}

-(void)setOrigin:(NSPoint)pos {
    _geometry.origin = pos;
    _frame = _geometry;
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@ 0x%x> %@ %@ title:%@ state:%u style:0x%x level:%d",
           [self class], (uint32_t)self, self.shmPath, NSStringFromRect(self.geometry),
           self.title, self.state, self.styleMask, self.level];
}

-(void)moveByX:(double)x Y:(double)y {
    _geometry.origin.x += x;
    _geometry.origin.y += y;
    _frame = _geometry;
}

@end


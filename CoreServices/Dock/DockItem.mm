/*
 * airyxOS Application Launcher & Status Bar
 *
 * Copyright (C) 2021-2022 Zoe Knox <zoe@pixin.net>
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

#import "DockItem.h"

@implementation DockItem

+dockItemWithPath:(NSString *)path {
    return [[self alloc] initWithPath:path];
}

-initWithPath:(NSString *)path {
    _path = path;
    _type = DIT_APP_BUNDLE;
    _flags = DIF_NORMAL;
    _pid = 0;
    _label = @"Empty";
    return self;
}

-(NSString *)path {
    return _path;
}

-(DockItemType)type {
    return _type;
}

-(DockItemFlags)flags {
    return (DockItemFlags)_flags;
}

-(BOOL)isNormal {
    return (_flags == DIF_NORMAL);
}

-(BOOL)isLocked {
    return (_flags & DIF_LOCKED);
}

-(BOOL)isRunning {
    return (_pid != 0);
}

-(BOOL)isResident {
    return (_flags & DIF_RESIDENT);
}

-(BOOL)needsAttention {
    return (_flags & DIF_ATTENTION);
}

-(pid_t)pid {
    return _pid;
}

-(void)setFlags:(DockItemFlags)flags {
    _flags = flags;
}

-(void)setNormal {
    _flags = DIF_NORMAL;
}

-(void)setLocked:(BOOL)value {
    if(value)
        _flags |= DIF_LOCKED;
    else
        _flags &= ~DIF_LOCKED;
}

-(void)setRunning:(pid_t)pid {
    _pid = pid;
}

-(void)setResident:(BOOL)value {
    if(value)
        _flags |= DIF_RESIDENT;
    else
        _flags &= ~DIF_RESIDENT;
}

-(void)setNeedsAttention:(BOOL)value {
    if(value)
        _flags |= DIF_ATTENTION;
    else
        _flags &= ~DIF_ATTENTION;
}
@end

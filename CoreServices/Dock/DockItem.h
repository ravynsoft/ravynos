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

#pragma once

#import <Foundation/Foundation.h>

enum DockItemType : unsigned int {
    DIT_APP_BUNDLE,
    DIT_APP_DESKTOP,
    DIT_FOLDER,
    DIT_MAX = DIT_FOLDER
};

enum DockItemFlags : unsigned int {
    DIF_NORMAL = 0x0,
    DIF_LOCKED = 0x1,       // can't modify or remove
    DIF_RESIDENT = 0x2,     // app is resident in dock
    DIF_ATTENTION = 0x4     // app wants attention
};

@interface DockItem : NSObject {
    NSString *_path;    // path of the item represented
    DockItemType _type;
    unsigned int _flags;
    pid_t _pid;         // PID, if running
    NSString *_label;   // displayed on hover
}

+dockItemWithPath:(NSString *)path;
-initWithPath:(NSString *)path;

-(NSString *)path;
-(DockItemType)type;
-(DockItemFlags)flags;
-(BOOL)isNormal;
-(BOOL)isLocked;
-(BOOL)isRunning;
-(BOOL)isResident;
-(BOOL)needsAttention;
-(pid_t)pid;

-(void)setFlags:(DockItemFlags)flags;
-(void)setNormal; // clears all flags
-(void)setLocked:(BOOL)value;
-(void)setRunning:(pid_t)pid;
-(void)setResident:(BOOL)value;
-(void)setNeedsAttention:(BOOL)value;
@end

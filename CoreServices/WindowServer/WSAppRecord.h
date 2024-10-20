/*
 * Copyright (C) 2024 Zoe Knox <zoe@pixin.net>
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

 #import <Foundation/NSObject.h>
 #import <Foundation/NSString.h>
 #import <mach/mach.h>

@interface WSAppRecord : NSObject {
    NSMutableArray *_windows;
    BOOL _mouseCursorConnected;
}

@property NSString *bundleID;           // CFBundleID
@property NSString *name;               // Display name
@property unsigned int pid;             // process ID
@property mach_port_t port;             // reply port for events
@property NSImage *icon;                // Shown in task switcher 

-init;
-(void)addWindow:(WSWindowRecord *)window;
-(void)removeWindowWithID:(int)number;
-(WSWindowRecord *)windowWithID:(int)number;
-(NSArray *)windows;
-(void)removeAllWindows;
-(void)mouseCursorConnected:(int)connected;
-(BOOL)mouseCursorConnected;
@end


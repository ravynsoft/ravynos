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

#define WINDOWSERVER

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSImage.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/event.h>
#include <pthread.h>
#include <pwd.h>
#include <grp.h>
#include <login_cap.h>
#include <kvm.h>

#import "message.h"
#import "BSDFramebuffer.h"
#import "WSInput.h"

extern const float WSWindowTitleHeight;
extern const float WSWindowEdgePad;
extern const float WSWindowCornerRadius;
extern const float WSWindowControlDiameter;
extern const float WSWindowControlSpacing;

NSRect WSOutsetFrame(NSRect rect, int style);

@interface WSWindowRecord : NSObject {
    CGGlyph _titleGlyphs[128];          // AppKit can't pass anything longer
}

@property int number;                   // internal window ID
@property void *surfaceBuf;             // mmaped shared graphics memory
@property size_t bufSize;               // size of surfaceBuf (bytes)
@property O2Surface *surface;           // rendering surface
@property enum WindowState state;       // state
@property NSRect geometry;              // position and size of client window
@property NSRect frame;                 // position and size with decorations
@property NSString *title;              // titlebar string
@property NSSize titleSize;             // bounding box of glyphs
@property BOOL titleSet;                // YES if ready to display title
@property NSImage *icon;                // window icon
@property NSString *shmPath;
@property int styleMask;                // NSWindow style flags
@property(readonly) NSRect closeButtonRect;
@property(readonly) NSRect miniButtonRect;
@property(readonly) NSRect zoomButtonRect;

-(void)dealloc;
-(void)setOrigin:(NSPoint)pos;
-(void)drawFrame:(O2Context *)_context;
-(void)moveByX:(double)x Y:(double)y;
-(void)setGlyphs:(CGGlyph *)glyphs;

@end

@interface WSAppRecord : NSObject {
    NSMutableArray *_windows;
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
@end

@interface WindowServer : NSObject {
    BOOL ready;
    BOOL stopOnErr;
    char **envp;
    unsigned int nobodyUID;
    unsigned int videoGID;
    unsigned int logLevel;
    enum ShellType curShell;
    BSDFramebuffer *fb;
    NSRect _geometry;
    CGFontRef _titleFont;

    WSInput *input;

    NSMutableDictionary *apps;          // key is CFBundleID
    WSAppRecord *curApp;
    WSWindowRecord *curWindow;

    mach_port_name_t _servicePort;
    int _kq;
    kvm_t *kvm;
}

-init;
-(void)dealloc;
-(void)setLogLevel:(int)level;
-(BOOL)launchShell;
-(BOOL)isReady;
-(NSRect)geometry;
-(void)draw;
-(O2BitmapContext *)context;
-(BOOL)setUpEnviron:(uid_t)uid;
-(void)freeEnviron;
-(void)dispatchEvent:(struct libinput_event *)event;
-(uint32_t)windowCreate:(struct mach_win_data *)data forApp:(WSAppRecord *)app;
-(void)run;
-(void)processKernelQueue;
-(void)receiveMachMessage;
-(BOOL)sendEventToApp:(struct mach_event *)event;
-(BOOL)sendInlineData:(void *)data length:(int)length withCode:(int)code toApp:(WSAppRecord *)app;
-(void)watchForProcessExit:(unsigned int)pid;
-(WSAppRecord *)findAppByPID:(unsigned int)pid;
-(void)signalQuit;
-(CGFontRef)titleFont;
-(NSSize)sizeOfTitleText:(NSString *)title;
@end

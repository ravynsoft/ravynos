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

#import <Foundation/Foundation.h>
#import <AppKit/NSEvent.h>
#include <libudev.h>
#include <libinput.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#undef direction
#include <linux/input.h>

static unichar translateKeySym(xkb_keysym_t keysym);

static int open_restricted_cb(const char *path, int flags, void *data) {
    int fd = open(path, flags);
    return fd < 0 ? -errno : fd;
}

static void close_restricted_cb(int fd, void *data) {
    close(fd);
}

const static struct libinput_interface interface = {
    .open_restricted = open_restricted_cb,
    .close_restricted = close_restricted_cb,
};

@interface NSObject(WSInput)
-(BOOL)sendEventToApp:(struct mach_event *)event;
-(void)signalQuit;
-(void)switchApp;
@end

@interface WSInput : NSObject {
    int logLevel;
    struct udev *udev;
    struct libinput *li;
    struct xkb_context *xkbCtx;
    struct xkb_keymap *xkb_keymap;
    struct xkb_state *xkb_state;
    struct xkb_state *xkb_state_unmodified;
    uint32_t keyRepeatDelay;

    NSRect geometry; // size of the active screen
    double pointerX;
    double pointerY;
    BOOL buttonDown[3]; // L R M
}

-init;
-(void)dealloc;
-(void)run:(NSObject *)target; /* target obj receives the events */
-(void)processEvent:(struct libinput_event *)event target:(NSObject *)target;
-(void)setLogLevel:(int)level;
-(void)setKeymap;
-(unsigned int)modifierFlagsForState:(struct xkb_state *)state;
-(NSPoint)pointerPos;
-(NSPoint)setPointerPos:(NSPoint)pos;
-(void)setGeometry:(NSRect)geom;
-(int)fileDescriptor;

@end


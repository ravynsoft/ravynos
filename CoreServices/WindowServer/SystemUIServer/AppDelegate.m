/*
 * Copyright (C) 2022 Zoe Knox <zoe@pixin.net>
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

#import <AppKit/AppKit.h>
#import <sys/event.h>
#import "desktop.h"
#import <servers/bootstrap.h>

#define MSG_ID_PORT 90210
#define MSG_ID_INLINE 90211
#define CODE_ADD_RECENT_ITEM 1
#define CODE_ITEM_CLICKED 2

typedef struct {
    mach_msg_header_t header;
    mach_msg_size_t msgh_descriptor_count;
    mach_msg_port_descriptor_t descriptor;
    unsigned int pid;
    mach_msg_trailer_t trailer;
} PortMessage;

typedef struct {
    mach_msg_header_t header;
    unsigned int code;
    unsigned char data[64*1024];
    unsigned int len;
    mach_msg_trailer_t trailer;
} Message;

typedef union {
    PortMessage portMsg;
    Message msg;
} ReceiveMessage;

@implementation AppDelegate
- (AppDelegate *)init {
    menuBar = [MenuBarWindow alloc];

    kern_return_t kr;
    if((kr = bootstrap_check_in(bootstrap_port, SERVICE_NAME, &_servicePort)) != KERN_SUCCESS) {
        NSLog(@"Failed to check-in service: %d", kr);
        return nil;
    }

    _kq = kqueue();
    if(_kq < 0) {
        perror("kqueue");
        exit(-1);
    }

    return self;
}

- (void)receiveMachMessage {
    ReceiveMessage msg = {0};
    mach_msg_return_t result = mach_msg((mach_msg_header_t *)&msg, MACH_RCV_MSG, 0, sizeof(msg),
        _servicePort, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
    if(result != MACH_MSG_SUCCESS)
        NSLog(@"mach_msg receive error");
    else {
        switch(msg.msg.header.msgh_id) {
            case MSG_ID_PORT:
            {
                mach_port_t port = msg.portMsg.descriptor.name;
                pid_t pid = msg.portMsg.pid;
                //NSLog(@"received port %d for PID %d",port,pid);
                [menuBar setPort:port forPID:pid];
                break;
            }
            case MSG_ID_INLINE:
                switch(msg.msg.code) {
                    case CODE_ADD_RECENT_ITEM:
                    {
                        NSURL *url = [NSURL URLWithString:
                            [[NSString alloc] initWithBytes:msg.msg.data
                            length:msg.msg.len encoding:NSUTF8StringEncoding]];
                        if(!url)
                            break;
                        [menuBar addRecentItem:url];
                    }
                }
                break;
        }
    }
}

// called from our kq watcher thread
- (void)processKernelQueue {
    struct kevent out[128];
    int count = kevent(_kq, NULL, 0, out, 128, NULL);

    for(int i = 0; i < count; ++i) {
        switch(out[i].filter) {
            case EVFILT_PROC:
                if((out[i].fflags & NOTE_EXIT)) {
                    //NSLog(@"PID %lu exited", out[i].ident);
                    [menuBar removePortForPID:out[i].ident];
                    [menuBar removeMenuForPID:out[i].ident];
                }
                break;
            default:
                NSLog(@"unknown filter");
        }
    }
}

- (void)createWindows:(NSNotification *)note {
    NSArray *screens = [NSScreen screens];
    NSScreen *output = [screens objectAtIndex:0]; //FIXME: select preferred display from prefs

    [menuBar initWithFrame:[output visibleFrame] forOutput:output];
    [menuBar setDelegate:self];
    [menuBar makeKeyAndOrderFront:nil];
}

/* Recursively set all menu targets and delegates to our proxy */
-(void)_menuEnumerateAndChange:(NSMenu *)menu {
    NSArray *items = [menu itemArray];
    [menu setDelegate:self];
    for(int i = 0; i < [items count]; ++i) {
        NSMenuItem *item = [items objectAtIndex:i];
        if([item isSeparatorItem] || [item isHidden] || ![item isEnabled])
            continue;
        [item setTarget:self];
        [item setAction:@selector(dump:)];
        if([item hasSubmenu])
            [self _menuEnumerateAndChange:[item submenu]];
    }
}

- (void)menuDidUpdate:(NSNotification *)note {
    NSMutableDictionary *dict = (NSMutableDictionary *)[note userInfo];
    pid_t pid = [[dict objectForKey:@"ProcessID"] intValue];
    NSMenu *mainMenu = [dict objectForKey:@"MainMenu"];
    [self _menuEnumerateAndChange:mainMenu];
    [menuBar setMenu:mainMenu forPID:pid];

    if(![menuBar activateMenuForPID:pid]) // FIXME: don't activate right away?
        NSLog(@"could not activate menus!");

    // watch for this PID to exit
    struct kevent kev[1];
    EV_SET(kev, pid, EVFILT_PROC, EV_ADD|EV_ONESHOT, NOTE_EXIT, 0, NULL);
    kevent(_kq, kev, 1, NULL, 0, NULL);
}

- (void)dump:(NSMenuItem *)object {
    int itemID = [object tag];
    //NSLog(@"DUMP clicked: %@ ID: %d", object, itemID);

    Message clicked = {0};
    clicked.header.msgh_remote_port = [menuBar activePort];
    clicked.header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND, 0, 0, 0);
    clicked.header.msgh_id = MSG_ID_INLINE;
    clicked.header.msgh_size = sizeof(clicked) - sizeof(mach_msg_trailer_t);
    clicked.code = CODE_ITEM_CLICKED;
    memcpy(clicked.data, &itemID, sizeof(itemID));
    clicked.len = sizeof(itemID);

    if(mach_msg((mach_msg_header_t *)&clicked, MACH_SEND_MSG, sizeof(clicked) - sizeof(mach_msg_trailer_t),
        0, MACH_PORT_NULL, 2000 /* ms timeout */, MACH_PORT_NULL) != MACH_MSG_SUCCESS)
        NSLog(@"Failed to send menu click to PID %d on port %d", [menuBar activeProcessID],
            clicked.header.msgh_remote_port);
}
@end


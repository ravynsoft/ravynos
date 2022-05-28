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
#import "desktop.h"
#import <servers/bootstrap.h>

#define MSG_ID_PORT 90210
#define MSG_ID_INLINE 90211

typedef struct {
    mach_msg_header_t header;
    mach_msg_size_t msgh_descriptor_count;
    mach_msg_port_descriptor_t descriptor;
    mach_msg_trailer_t trailer;
} PortMessage;

typedef struct {
    mach_msg_header_t header;
    unsigned char data[32];
    unsigned int len;
    mach_msg_trailer_t trailer;
} Message;

typedef union {
    PortMessage portMsg;
    Message msg;
} ReceiveMessage;

@implementation AppDelegate
- (AppDelegate *)init {
    desktops = [NSMutableDictionary new];
    menuBar = nil;

    kern_return_t kr;
    if((kr = bootstrap_check_in(bootstrap_port, SERVICE_NAME, &_servicePort) != KERN_SUCCESS)) {
        NSLog(@"Failed to check-in service: %d", kr);
        return nil;
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
                // FIXME: save port for menu/PID
                break;
            }
            case MSG_ID_INLINE:
                break;
        }
    }
}

- (void)screenDidResize:(NSNotification *)note {
    NSMutableDictionary *dict = (NSMutableDictionary *)[note userInfo];
    NSNumber *key = [dict objectForKey:@"WLOutputXDGOutput"];

    if(key == nil) {
        NSLog(@"ERROR: screenDidResize for null output key");
        return;
    }

    NSRect frame = NSZeroRect;
    frame.size = NSSizeFromString([dict objectForKey:@"WLOutputSize"]);
    frame.origin = NSPointFromString([dict objectForKey:@"WLOutputPosition"]);

    DesktopWindow *desktop = [[DesktopWindow alloc] initWithFrame:frame forOutput:key];
    [desktops setObject:desktop forKey:key];
    [desktop setDelegate:self];
    [desktop makeKeyAndOrderFront:nil];
    if([desktop isPrimaryDisplay])
        menuBar = [desktop menuBar];
}

- (void)updateBackground {
    NSArray *values = [desktops allValues];
    for(int i = 0; i < [values count]; ++i)
        [[values objectAtIndex:i] updateBackground];
}

/* Recursively set all menu targets and delegates to our proxy */
-(void)_menuEnumerateAndChange:(NSMenu *)menu {
    NSArray *items = [menu itemArray];
    [menu setDelegate:self];
    for(int i = 0; i < [items count]; ++i) {
        NSMenuItem *item = [items objectAtIndex:i];
        NSLog(@"enumerating %@", item);
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
    NSLog(@"received menu %@ for pid %d\n",mainMenu,pid);
    [self _menuEnumerateAndChange:mainMenu];
    [menuBar setMenu:mainMenu forPID:pid];
    if(![menuBar activateMenuForPID:pid]) // FIXME: don't activ8 right away
        NSLog(@"could not activate menus!");
}

- (void)dump:(id)object {
    NSLog(@"DUMP: %@", object);
}
@end


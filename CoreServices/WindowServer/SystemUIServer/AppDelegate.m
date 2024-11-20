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

#define WINDOWSERVER 1

#import <AppKit/AppKit.h>
#import <sys/event.h>
#import "desktop.h"
#import <servers/bootstrap.h>
#import <WindowServer/message.h>


@implementation AppDelegate
- (AppDelegate *)init {
    menuBar = [MenuBarWindow alloc];
    return self;
}

#if 0
                    case CODE_ADD_RECENT_ITEM:
                    {
                        NSURL *url = [NSURL URLWithString:
                            [[NSString alloc] initWithBytes:msg.msg.data
                            length:msg.msg.len encoding:NSUTF8StringEncoding]];
                        if(!url)
                            break;
                        [menuBar addRecentItem:url];
                        break;
                    }
		    case CODE_ADD_STATUS_ITEM:
		    {
			NSData *data = [NSData
			    dataWithBytes:msg.msg.data length:msg.msg.len];
			NSObject *o = nil;
			@try {
			    o = [NSKeyedUnarchiver unarchiveObjectWithData:data];
			}
			@catch(NSException *localException) {
			    NSLog(@"%@",localException);
			}

			if(o == nil || [o isKindOfClass:[NSDictionary class]] == NO ||
			    [(NSDictionary *)o objectForKey:@"StatusItem"] == nil ||
			    [(NSDictionary *)o objectForKey:@"ProcessID"] == nil ||
			    ![[(NSDictionary *)o objectForKey:@"StatusItem"] isKindOfClass:[NSStatusItem class]]) {
			    fprintf(stderr, "archiver: bad input\n");
			    break;
			}

			[menuBar
			    addStatusItem:[dict objectForKey:@"StatusItem"]
			    pid:pid];
			break;
		    }
                }
                break;
        }
    }
}
#endif

-(void)applicationWillFinishLaunching:(NSNotification *)note {
    NSScreen *mainDisplay = [[NSScreen screens] objectAtIndex:0];
    [menuBar initWithFrame:[mainDisplay frame]];
    [menuBar setDelegate:self];
    [[menuBar contentView] setNeedsDisplay:YES];
    [menuBar makeKeyAndOrderFront:self];
    [menuBar makeMainWindow];

    NSNotificationCenter *defaultCenter = [NSNotificationCenter defaultCenter];
    [defaultCenter addObserver:self selector:@selector(menuDidUpdate:)
        name:NSMenuDidUpdateNotification object:nil];
    [defaultCenter addObserver:self selector:@selector(appDidQuit:)
        name:NSApplicationDidQuitNotification object:nil];
    [defaultCenter addObserver:self selector:@selector(appDidActivate:)
        name:NSApplicationDidBecomeActiveNotification object:nil];
    [defaultCenter addObserver:self selector:@selector(appDidDeactivate:)
        name:NSApplicationDidResignActiveNotification object:nil];
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
        [item setAction:@selector(clicked:)];
        if([item hasSubmenu])
            [self _menuEnumerateAndChange:[item submenu]];
    }
}

- (void)menuDidUpdate:(NSNotification *)note {
    NSMutableDictionary *dict = (NSMutableDictionary *)[note userInfo];
    pid_t pid = [[dict objectForKey:@"ProcessID"] intValue];
    NSString *bundleID = [dict objectForKey:@"BundleID"];
    NSMenu *mainMenu = [dict objectForKey:@"MainMenu"];
    [self _menuEnumerateAndChange:mainMenu];

    [menuBar setMenu:mainMenu forApp:bundleID];
}

-(void)appDidQuit:(NSNotification *)note {
    NSMutableDictionary *dict = (NSMutableDictionary *)[note userInfo];
    NSString *bundleID = [dict objectForKey:@"BundleID"];
    [menuBar removeMenuForApp:bundleID];
}

-(void)appDidActivate:(NSNotification *)note {
    NSMutableDictionary *dict = (NSMutableDictionary *)[note userInfo];
    NSString *bundleID = [dict objectForKey:@"BundleID"];
    [menuBar activateApp:bundleID];
}

-(void)appDidDeactivate:(NSNotification *)note {
    NSMutableDictionary *dict = (NSMutableDictionary *)[note userInfo];
    NSString *bundleID = [dict objectForKey:@"BundleID"];
    [menuBar setMenu:nil];
}

- (void)clicked:(NSMenuItem *)object {
    int itemID = [object tag];

    Message clicked = {0};
    clicked.header.msgh_remote_port = [NSApp _wsServicePort];
    clicked.header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND, 0, 0, 0);
    clicked.header.msgh_id = MSG_ID_INLINE;
    clicked.header.msgh_size = sizeof(clicked) - sizeof(mach_msg_trailer_t);
    clicked.code = CODE_ITEM_CLICKED;
    clicked.pid = getpid();
    memcpy(clicked.data, &itemID, sizeof(itemID));
    clicked.len = sizeof(itemID);

    kern_return_t ret = 0;
    if((ret = mach_msg((mach_msg_header_t *)&clicked, MACH_SEND_MSG, sizeof(clicked) - sizeof(mach_msg_trailer_t),
        0, MACH_PORT_NULL, 2000 /* ms timeout */, MACH_PORT_NULL)) != MACH_MSG_SUCCESS)
        NSLog(@"Failed to notify WindowServer of menu item click: 0x%x", ret);
}

-(void)mouseDown:(NSEvent *)event {
//    NSLog(@"mouse down at %@", event);
}

-(void)mouseMoved:(NSEvent *)event {
//    NSLog(@"mouse moved at %@", event);
}

@end


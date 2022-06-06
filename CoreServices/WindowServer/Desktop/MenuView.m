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
#import <Foundation/NSPlatform.h>
#import "desktop.h"

@interface NSMenu(private)
-(NSString *)_name;
@end

@interface NSMainMenuView(private)
-(void)setWindow:(NSWindow *)window;
@end

static NSString *_formatAsGB(unsigned long bytes)
{
    double gb = (double)bytes;
    gb /=  (1024.0 * 1024.0 * 1024.0);
    return [NSString stringWithFormat:@"%.1f GB", gb];
}

@implementation MenuView
- init {
    NSRect frame = [[NSScreen mainScreen] visibleFrame];

    self = [super initWithFrame:NSMakeRect(0, 0, frame.size.width/2, menuBarHeight)];
    aboutWindow = nil;

    NSMenuItem *item;
    sysMenu = [NSMenu new];
    [sysMenu setDelegate:self];
    [sysMenu setAutoenablesItems:YES];
    [[sysMenu addItemWithTitle:@"About This Computer" action:@selector(aboutThisComputer:) 
        keyEquivalent:@""] setTarget:self];
    [[sysMenu addItemWithTitle:@"System Preferences..." action:NULL keyEquivalent:@""]
        setTarget:self];
    [[sysMenu addItemWithTitle:@"Software Store..." action:NULL keyEquivalent:@""] setEnabled:NO];
    [sysMenu addItem:[NSMenuItem separatorItem]];
    [[sysMenu addItemWithTitle:@"Recent Items" action:NULL keyEquivalent:@""] setTarget:self];
    [sysMenu addItem:[NSMenuItem separatorItem]];
    [[sysMenu addItemWithTitle:@"Force Quit..." action:NULL keyEquivalent:@""] setTarget:self];
    [sysMenu addItem:[NSMenuItem separatorItem]];
    [[sysMenu addItemWithTitle:@"Sleep" action:NULL keyEquivalent:@""] setTarget:self];
    [[sysMenu addItemWithTitle:@"Restart..." action:NULL keyEquivalent:@""] setTarget:self];
    [[sysMenu addItemWithTitle:@"Shut Down..." action:NULL keyEquivalent:@""] setTarget:self];
    [sysMenu addItem:[NSMenuItem separatorItem]];
    [[sysMenu addItemWithTitle:@"Lock Screen" action:NULL keyEquivalent:@""] setTarget:self];
    [[sysMenu addItemWithTitle:@"Log Out" action:NULL keyEquivalent:@""] setTarget:self];

    NSString *ravyn = [[NSBundle mainBundle] pathForResource:@"ravynos-mark-64" ofType:@"png"];
    NSImage *logo = [[NSImage alloc] initWithContentsOfFile:ravyn];
    [logo setScalesWhenResized:YES];
    [logo setSize:NSMakeSize(16,16)];
    NSMenu *logoMenu = [NSMenu new];
    NSMenuItem *logoItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [logoItem setImage:logo];
    [logoItem setSubmenu:sysMenu];
    [logoMenu addItem:logoItem];

    NSRect rect = NSMakeRect(menuBarHPad, 0, 64, menuBarHeight);
    NSMainMenuView *mv = [[NSMainMenuView alloc] initWithFrame:rect menu:logoMenu];
    [self addSubview:mv];

    [self setNeedsDisplay:YES];
    return self;
}

- (BOOL)isFlipped {
    return YES;
}

- (void)setMenu:(NSMenu *)menu {
    NSMenuItem *item = [menu itemAtIndex:0];
    if([item hasSubmenu] && [[[item submenu] _name] isEqualToString:@"NSAppleMenu"]) {
        NSFontManager *fontmgr = [NSFontManager sharedFontManager];
        NSDictionary *attr = [NSDictionary dictionaryWithObject:[fontmgr convertFont:
            [NSFont menuFontOfSize:15] toHaveTrait:NSBoldFontMask] forKey:NSFontAttributeName];
        [item setAttributedTitle:[[NSAttributedString alloc] initWithString:[item title]
            attributes:attr]];
    }

    NSRect rect = NSMakeRect(menuBarHPad*3, 0, _frame.size.width, menuBarHeight);
    appMenuView = [[NSMainMenuView alloc] initWithFrame:rect menu:menu];
    [appMenuView setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    [self addSubview:appMenuView];
    [appMenuView setWindow:[self window]];

    [self setNeedsDisplay:YES];
}

#define LOGO_WIDTH 140
#define LOGO_HEIGHT 140
#define WIN_WIDTH 550
#define WIN_HEIGHT 300
#define V_SPACER 40
#define H_SPACER 15
- (void)aboutThisComputer:(id)sender {
    if(aboutWindow) {
        NSLog(@"re-using aboutWindow");
        [aboutWindow orderFront:nil];
        return;
    }

    aboutWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,WIN_WIDTH,WIN_HEIGHT)
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];
    [aboutWindow setTitle:@"About This Computer"];
    [aboutWindow setDelegate:self];

    NSImageView *iv = [[NSImageView alloc] initWithFrame:NSMakeRect(0, WIN_HEIGHT/2 - LOGO_HEIGHT/2,
        LOGO_WIDTH, LOGO_HEIGHT)];
    NSString *releaseLogo = [[NSBundle mainBundle] pathForResource:@"ReleaseLogo" ofType:@"tiff"];
    NSImage *img = [[NSImage alloc] initWithContentsOfFile:releaseLogo];
    [img setScalesWhenResized:YES];
    [img setSize:NSMakeSize(LOGO_WIDTH, LOGO_HEIGHT)];
    [iv setImage:img];
    [[aboutWindow contentView] addSubview:iv];

    NSDictionary *osVersionDictionary = [NSDictionary
        dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];

    NSFontManager *fontmgr = [NSFontManager sharedFontManager];
    NSFont *fontBigBold = [fontmgr convertFont:[NSFont systemFontOfSize:32] toHaveTrait:NSBoldFontMask];
    NSFont *fontMed = [NSFont systemFontOfSize:20];
    NSFont *fontNormal = [NSFont systemFontOfSize:14];
    NSFont *fontBold = [fontmgr convertFont:fontNormal toHaveTrait:NSBoldFontMask];

    NSTextView *tv = [[NSTextView alloc] initWithFrame:NSMakeRect(LOGO_WIDTH + H_SPACER, V_SPACER,
        WIN_WIDTH - LOGO_WIDTH - H_SPACER, WIN_HEIGHT - V_SPACER - 10)];
    [tv setDrawsBackground:NO];
    NSTextStorage *ts = [tv textStorage];

    [ts beginEditing];

    NSAttributedString *s = [[NSAttributedString alloc] initWithString:
        [osVersionDictionary objectForKey:@"ProductName"]
        attributes:[NSDictionary dictionaryWithObject:fontBigBold forKey:NSFontAttributeName]];
    [ts appendAttributedString:s];

    s = [[NSAttributedString alloc] initWithString:[NSString stringWithFormat:@" %@\n",
        [osVersionDictionary objectForKey:@"ProductFamily"]]
        attributes:[NSDictionary dictionaryWithObject:fontMed forKey:NSFontAttributeName]];
    [ts appendAttributedString:s];

    s = [[NSAttributedString alloc] initWithString:[NSString stringWithFormat:@"Version: %@\n\n\n\n",
        [osVersionDictionary objectForKey:@"ProductUserVisibleVersion"]]
        attributes:[NSDictionary dictionaryWithObject:fontNormal forKey:NSFontAttributeName]];
    [ts appendAttributedString:s];

    FILE *fp = popen("/System/Library/CoreServices/DMIHelper", "r");
    if(fp) {
        char buf1[80];
        char buf2[80];
        memset(buf1, 0, sizeof(buf1));
        memset(buf2, 0, sizeof(buf2));
        if(fgets(buf1, sizeof(buf1), fp) != NULL)
            buf1[strlen(buf1) - 1] = 0;
        if(fgets(buf2, sizeof(buf2), fp) != NULL)
            buf2[strlen(buf2) - 1] = 0;
        pclose(fp);
        s = [[NSAttributedString alloc] initWithString:[NSString 
            stringWithFormat:@"%s %s", buf1, buf2]
            attributes:[NSDictionary dictionaryWithObject:fontBold forKey:NSFontAttributeName]];
        [ts appendAttributedString:s];
    }

    NSPlatform *platform = [NSPlatform currentPlatform];
    s = [[NSAttributedString alloc] initWithString:@"\n\nProcessor: "
        attributes:[NSDictionary dictionaryWithObject:fontBold forKey:NSFontAttributeName]];
    [ts appendAttributedString:s];
    s = [[NSAttributedString alloc] initWithString:[NSString stringWithFormat:@"%u-core %@",
            [platform processorCount], [platform CPUModel]]
            attributes:[NSDictionary dictionaryWithObject:fontNormal forKey:NSFontAttributeName]];
    [ts appendAttributedString:s];

    s = [[NSAttributedString alloc] initWithString:@"\n\nMemory: "
        attributes:[NSDictionary dictionaryWithObject:fontBold forKey:NSFontAttributeName]];
    [ts appendAttributedString:s];
    s = [[NSAttributedString alloc] initWithString:_formatAsGB([platform physicalMemory])
            attributes:[NSDictionary dictionaryWithObject:fontNormal forKey:NSFontAttributeName]];
    [ts appendAttributedString:s];

    s = [[NSAttributedString alloc] initWithString:@"\n\nGraphics: "
        attributes:[NSDictionary dictionaryWithObject:fontBold forKey:NSFontAttributeName]];
    [ts appendAttributedString:s];

    char *cards[] = {"vgapci0", "vgapci1", NULL};
    char buf[256];
    for(int i = 0; cards[i] != NULL; ++i) {
        char adaptor[300];

        memset(adaptor, 0, sizeof(adaptor));
        snprintf(buf, sizeof(buf), "/usr/sbin/pciconf -lv %s", cards[i]);
        FILE *fp = popen(buf, "r");
        if(fp == NULL)
            continue;
        
        while(fgets(buf, sizeof(buf), fp) != NULL) {
            NSString *line = [[NSString alloc] initWithUTF8String:buf];
        
            if([line rangeOfString:@"vendor"].location != NSNotFound) {
                for(int start = 0; start < [line length]; ++start) {
                    if([line characterAtIndex:start] == '\'') {
                        strncat(adaptor, 
                            [[line substringWithRange:NSMakeRange(start + 1,
                            [line length] - start - 3)] UTF8String],
                            sizeof(adaptor) - strlen(adaptor));
                        strcat(adaptor, " ");
                        break;
                    }
                }
            } else if([line rangeOfString:@"device"].location != NSNotFound) {
                for(int start = 0; start < [line length]; ++start) {
                    if([line characterAtIndex:start] == '\'') {
                        strncat(adaptor, 
                            [[line substringWithRange:NSMakeRange(start + 1,
                            [line length] - start - 3)] UTF8String],
                            sizeof(adaptor) - strlen(adaptor));
                        strcat(adaptor, " ");
                        break;
                    }
                }
            } else if([line rangeOfString:@"subclass"].location != NSNotFound) {
                if(adaptor[0] != 0)
                    continue; // skip subclass unless it's all we got
                for(int start = 0; start < [line length]; ++start) {
                    if([line characterAtIndex:start] == '=') {
                        strncat(adaptor, 
                            [[line substringWithRange:NSMakeRange(start + 1,
                            [line length] - start - 2)] UTF8String],
                            sizeof(adaptor) - strlen(adaptor));
                        break;
                    }
                }
            }
        }

        s = [[NSAttributedString alloc]
            initWithString:[NSString stringWithFormat:@"%s\n", adaptor]
            attributes:[NSDictionary dictionaryWithObject:fontNormal forKey:NSFontAttributeName]];
        [ts appendAttributedString:s];

        pclose(fp);
    }

    [ts endEditing];
    [tv setEditable:NO];

    [[aboutWindow contentView] addSubview:tv];
    [aboutWindow makeKeyAndOrderFront:nil];
}

- (BOOL)windowShouldClose:(NSWindow *)window {
    NSLog(@"window should close: %@", window);
    if([window isEqual:aboutWindow]) {
        NSLog(@"aboutWindow should close");
        aboutWindow = nil;
    }
    return YES;
}

- (void)windowWillClose:(NSNotification *)note {
    NSLog(@"window closing: %@", [note object]);
    if([[note object] isEqual:aboutWindow]) {
        NSLog(@"aboutWindow closing");
        aboutWindow = nil;
    }
}

@end


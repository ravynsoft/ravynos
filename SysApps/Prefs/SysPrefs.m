/*
 * Copyright (C) 2024-2025 Zoe Knox <zoe@ravynsoft.com>
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

#import "SysPrefs.h"

@implementation SystemPreferences

const float windowWidth = 668; // Apple uses a fixed width and grows vertically
const float size = 96; // icon & label container size
const float spacer = 8; // icon spacing

-(void)applicationWillFinishLaunching:(NSNotification *)note {
    doubleClickInterval = [[NSDisplay currentDisplay] doubleClickInterval];
    lastClickTime = 0;
    lastClickObject = nil;
    clickCount = 0;

    NSBundle *mainBundle = [NSBundle mainBundle];
    NSString *displayName = [[mainBundle infoDictionary] objectForKey:@"CFBundleDisplayName"];
    NSRect rect = NSMakeRect(100, 100, windowWidth, 200);
    window = [[NSWindow alloc] initWithContentRect:rect
                                         styleMask:NSTitledWindowMask
                                           backing:NSBackingStoreBuffered
                                             defer:NO];
    [window setTitle:displayName];

    panes = [NSMutableArray new];
    font = [NSFont systemFontOfSize:15];

    [self loadPanes];
    [window makeKeyAndOrderFront:self];
}

-(void)loadPanes {
    NSArray *dirs = @[@"/System/Library/PreferencePanes", @"/Library/PreferencePanes"];
    NSFileManager *fm = [NSFileManager defaultManager];

    [panes removeAllObjects]; 

    float x = 0, y = 0;
    for(int d = 0; d < [dirs count]; ++d) {
        NSString *dirPath = [dirs objectAtIndex:d];
        NSArray *files = [fm contentsOfDirectoryAtPath:dirPath error:NULL];

        for(int i = 0; i < [files count]; ++i) {
            NSString *path = [dirPath stringByAppendingPathComponent:[files objectAtIndex:i]];
            if([path hasSuffix:@".prefPane"]) {
                NSBundle *b = [NSBundle bundleWithPath:path];
                if(b) {
                    [panes addObject:b];
                    NSView *item = [self loadPaneIcon:b]; 
                    [(NSButton *)item setTag:[panes indexOfObject:b]];
                    [item setFrameOrigin:NSMakePoint(x, y)];
                    x += size;
                    x += spacer;
                    if(x > (windowWidth - size - spacer)) {
                        x = 0;
                        y += size;
                        y += spacer;
                    }
                    [[window contentView] addSubview:item];
                } else
                    NSLog(@"error loading prefPane %@", path);
            }
        }
    }
}


-(NSView *)loadPaneIcon:(NSBundle *)b {
        NSString *label = [b objectForInfoDictionaryKey:@"CFBundleDisplayName"];
        if(!label)
            label = [b objectForInfoDictionaryKey:@"CFBundleName"];

        NSString *rsc = [b resourcePath];
        NSString *exe = [b executablePath];

        NSString *iconFile = [b objectForInfoDictionaryKey:@"CFBundleIconFile"];
        if(!iconFile)
            iconFile = [b objectForInfoDictionaryKey:@"NSIcon"];
        NSString *iconPath = [NSString stringWithFormat:@"%@/%@",rsc,iconFile];

        NSRect rect = NSMakeRect(0, 0, size, size);
        NSButton *container = [[NSButton alloc] initWithFrame:rect];
        [container setBezeled:NO];
        [container setBordered:NO];
        [container setImagePosition:NSImageAbove];
        [container setAction:@selector(iconClicked:)];

        [container setImage:[[NSImage alloc] initWithContentsOfFile:iconPath]];
        [[container image] setScalesWhenResized:YES];
        [[container image] setSize:NSMakeSize(size - 24, size - 24)];

        [container setFont:font];
        [container setTitle:label];

        return container;
}

// This is a little hacky but it works :)
-(void)iconClicked:(NSButton *)button {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    NSTimeInterval now = ts.tv_sec + ts.tv_nsec / 1000000000;

    if(button != lastClickObject) {
        clickCount = 1;
        lastClickTime = now;
        lastClickObject = button;
        return;
    }

    if((now - lastClickTime) < doubleClickInterval)
        clickCount++;
    else
        clickCount = 1;
    lastClickTime = now;
    lastClickObject = button;

    if(clickCount >= 2) {
        [self openPane:[button tag]];
        clickCount = 0;
    }
}

-(void)openPane:(int)tag {
    NSLog(@"opening pane tag %d, %@", tag, [panes objectAtIndex:tag]);
}

@end

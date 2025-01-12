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
    attr = [NSDictionary dictionaryWithObjects:@[font, [NSColor blackColor]]
            forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];

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


        NSAttributedString *as = [[NSAttributedString alloc] initWithString:label
                                                                 attributes:attr];
        NSRect frame = NSMakeRect(0, 0, size, 32);

        NSTextField *tf = [[NSTextField alloc] initWithFrame:frame];
        [tf setAttributedStringValue:as];
        [tf setBordered:NO];
        [tf setEditable:NO];

        NSString *rsc = [b resourcePath];
        NSString *exe = [b executablePath];

        NSString *iconFile = [b objectForInfoDictionaryKey:@"CFBundleIconFile"];
        if(!iconFile)
            iconFile = [b objectForInfoDictionaryKey:@"NSIcon"];
        NSString *iconPath = [NSString stringWithFormat:@"%@/%@",rsc,iconFile];

        NSRect rect = NSMakeRect(0, 0, size, size);
        NSView *container = [[NSView alloc] initWithFrame:rect];

        NSImageView *iconView = [[NSImageView alloc] initWithFrame:NSMakeRect(
                rect.origin.x + 8, rect.origin.y + 32, rect.size.width - 8, rect.size.height - 32)];
        [iconView setImage:[[NSImage alloc] initWithContentsOfFile:iconPath]];
        [[iconView image] setScalesWhenResized:YES];
        [iconView setImageScaling:NSImageScaleProportionallyUpOrDown];

        [container addSubview:iconView];
        [container addSubview:tf];

        return container;
}

@end

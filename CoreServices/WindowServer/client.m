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

#import <AppKit/AppKit.h>

@interface Delegate : NSWindow {
    NSTextView *textfield;
    NSImage *cursor;
    NSWindow *win;
}
@end

@implementation Delegate
-init {
    self = [super initWithContentRect:NSMakeRect(200, 200, 640, 480)
                            styleMask:NSTitledWindowMask
                              backing:NSBackingStoreRetained
                                defer:NO];
    NSRect bounds = [[self contentView] bounds];
    bounds.size.width -= 200;
    textfield = [[NSTextView alloc] initWithFrame:bounds];
    [textfield setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

    return self;
}

-(void)applicationWillFinishLaunching:(NSNotification *)note {
    [super setTitle:@"Client App Demo"];
    NSView *v = [self contentView];
    [v addSubview:textfield];
    [textfield setEditable:YES];
    [textfield setNeedsDisplay:YES];
    NSString *path = [[NSBundle mainBundle] pathForResource:@"SneakySnek" ofType:@"png"];
    NSImage *img = [[NSImage alloc] initWithContentsOfFile:path];
    NSRect bounds = [[self contentView] bounds];
    bounds.origin.x = bounds.size.width - 190;
    bounds.size.width = 180;
    NSImageView *iv = [[NSImageView alloc] initWithFrame:bounds];
    [iv setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [iv setImage:img];
    [v addSubview:iv];
    [v setNeedsDisplay:YES];

    [self makeKeyAndOrderFront:self];

    NSScreen *main = [NSScreen mainScreen];
    NSDictionary *desc = [main deviceDescription];
    unsigned int depth = [main depth];
    NSColorSpace *cs = [main colorSpace];

    NSLog(@"ID %x name %@ size %@ res %@\ncs %@ %@ %d\ncomps %ld bps %ld (%d) bpp %ld\ndepth %x",
            [[desc objectForKey:@"NSScreenNumber"] intValue], [main localizedName],
            NSStringFromSize([[desc objectForKey:NSDeviceSize] sizeValue]),
            NSStringFromSize([[desc objectForKey:NSDeviceResolution] sizeValue]),
            NSColorSpaceFromDepth(depth), cs, CGColorSpaceGetModel([cs CGColorSpace]),
            NSNumberOfColorComponents(NSColorSpaceFromDepth(depth)),
            NSBitsPerSampleFromDepth(depth), [[desc objectForKey:NSDeviceBitsPerSample] intValue],
            NSBitsPerPixelFromDepth(depth), depth);
}

-(void)keyDown:(NSEvent *)e {
    if([[e characters] isEqual:@"\e"])
        [NSApp terminate:self];
    else
        [textfield insertText:[e characters]];
}

#if 0
-(void)mouseMoved:(NSEvent *)e {
    NSLog(@"mouseMoved %@", e);
}
#endif

@end

#ifdef __RAVYN__
@interface NSMenu(client)
-(NSMenu *)initApplicationMenu:(NSString *)name;
@end

void __NSInitializeProcess(int argc, char *argv[]);
#endif

int main(int argc, const char *argv[]) {
#ifdef __RAVYN__
    __NSInitializeProcess(argc, argv);
#endif

    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        NSLog(@"Press Esc to exit");
        Delegate *del = [Delegate new];

        NSMenu *menu = [NSMenu new];
        [menu setAutoenablesItems:YES];
        [[menu addItemWithTitle:@"File" action:@selector(fileMenu:) keyEquivalent:@""] setTarget:del];
        [[menu addItemWithTitle:@"Edit" action:@selector(editMenu:) keyEquivalent:@""] setTarget:del];

        NSMenu *appMenu = [[NSMenu alloc]
#ifdef __RAVYN__
        // Hack up an application menu since we don't load a nib
        initApplicationMenu:@"Client Demo"];
#else
        init];
#endif
        NSMenuItem *appMenuItem = [NSMenuItem new];
        [appMenuItem setTitle:@"Client Demo"];
        [appMenuItem setSubmenu:appMenu];
        [menu insertItem:appMenuItem atIndex:0];
        [NSApp setMainMenu:menu];

        [NSApp setDelegate:del];
        [NSApp run];
    }

    exit(0);
}


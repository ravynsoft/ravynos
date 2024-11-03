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
    textfield = [[NSTextView alloc] initWithFrame:NSMakeRect(1,1,438,478)];

    return self;
}

-(void)applicationWillFinishLaunching:(NSNotification *)note {
    [super setTitle:@"Client App Demo"];
    NSView *v = [self contentView];
    [v addSubview:textfield];
    [textfield setEditable:YES];
    [textfield setNeedsDisplay:YES];
    NSImage *img = [[NSImage alloc] initWithContentsOfFile:@"SneakySnek.png"];
    NSImageView *iv = [[NSImageView alloc] initWithFrame:NSMakeRect(439,1,200,478)];
    [iv setImage:img];
    [v addSubview:iv];
    [v setNeedsDisplay:YES];

#if 0
    win = [[NSWindow alloc] initWithContentRect:NSMakeRect(400, 400, 640, 480)
                                     styleMask:NSTitledWindowMask
                                       backing:NSBackingStoreRetained
                                         defer:NO];
    NSImageView *iv2 = [[NSImageView alloc] initWithFrame:NSMakeRect(0, 0, 640, 480)];
    [iv2 setImage:img];
    [[win contentView] addSubview:iv2];
    [[win contentView] setNeedsDisplay:YES];
    [win orderFront:self];
#endif
    [self makeKeyAndOrderFront:self];
}

-(void)keyDown:(NSEvent *)e {
    if([[e characters] isEqual:@"\e"])
        [NSApp terminate:self];
    else
        [textfield insertText:[e characters]];
}

-(void)mouseMoved:(NSEvent *)e {
    NSLog(@"mouseMoved %@", e);
}

@end

@interface NSMenu(client)
-(NSMenu *)initApplicationMenu:(NSString *)name;
@end

int main(int argc, const char *argv[]) {
    __NSInitializeProcess(argc, argv);
    
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        NSLog(@"Press Esc to exit");
        Delegate *del = [Delegate new];

        NSMenu *menu = [NSMenu new];
        [menu setAutoenablesItems:YES];
        [[menu addItemWithTitle:@"File" action:@selector(fileMenu:) keyEquivalent:@""] setTarget:del];
        [[menu addItemWithTitle:@"Edit" action:@selector(editMenu:) keyEquivalent:@""] setTarget:del];
        // Hack up an application menu since we don't load a nib
        NSMenu *appMenu = [[NSMenu alloc] initApplicationMenu:@"Client Demo"];
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


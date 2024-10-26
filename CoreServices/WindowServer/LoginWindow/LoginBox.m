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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#import <AppKit/AppKit.h>
#import "desktop.h"

const int width = 480;
const int height = 640;

@implementation LoginBox
- initWithWindow:(NSWindow *)window {
    NSRect rect = [window frame];
    NSPoint center = NSMakePoint(rect.size.width / 2, rect.size.height / 2);

    NSRect boxRect = NSMakeRect(center.x - width/2, center.y - height / 2, width, height); 
    self = [super initWithFrame:boxRect];

    // give us a background for the fields
    center.x = width / 2;
    center.y = height / 2;
    NSBox *bg = [[NSBox alloc] initWithFrame:NSMakeRect(0, 0, width, height)];
    [bg setTransparent:NO];
    [bg setBoxType:NSBoxCustom];
    [bg setCornerRadius:8.0];
    [self addSubview:bg];

    wordmark = [[NSImageView alloc] initWithFrame:NSMakeRect(20, height - 120, width - 40, 100)];
    NSString *path = [[NSBundle mainBundle] pathForResource:@"ravynos-full" ofType:@"png"];
    NSImage *ravyn = [[NSImage alloc] initWithContentsOfFile:path];
    [ravyn setScalesWhenResized:YES];
    [wordmark setImage:ravyn];
    [bg addSubview:wordmark];

    logoView = [[NSImageView alloc] initWithFrame:NSMakeRect(center.x - 80, 40, 160, 160)];
    [logoView setImageScaling:NSImageScaleAxesIndependently];
    [logoView setImageAlignment:NSImageAlignCenter];

    path = [[NSBundle mainBundle] pathForResource:@"SneakySnek" ofType:@"png"];
    NSImage *logo = [[NSImage alloc] initWithContentsOfFile:path];
    [logo setScalesWhenResized:YES];
    [logoView setImage:logo];
    [bg addSubview:logoView];

    NSFont *font = [[NSFontManager sharedFontManager] convertFont:[NSFont systemFontOfSize:16]
        toHaveTrait:NSBoldFontMask];
    NSDictionary *attr = [NSDictionary dictionaryWithObjects:
        @[font, [NSColor blackColor]]
        forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];

    NSDictionary *osVersionDictionary = [NSDictionary
        dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
    NSString *versionString = [NSString stringWithFormat:@"%@ %@",
             [osVersionDictionary objectForKey:@"ProductFamily"],
             [osVersionDictionary objectForKey:@"ProductUserVisibleVersion"]];
    NSAttributedString *v  = [[NSAttributedString alloc] initWithString:versionString
        attributes:[NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName]];
    Label *versionLabel = [Label labelWithText:v atPoint:NSMakePoint(center.x - [v size].width / 2,
                    height - 150) withMaxWidth:width/2];
    [bg addSubview:versionLabel];

    int ypos = height / 2 + 40;

    NSAttributedString *s = [[NSAttributedString alloc] initWithString:@"User name" attributes:attr];
    Label *usernameLabel = [Label labelWithText:s atPoint:NSMakePoint(center.x - 140, ypos) withMaxWidth:width/2];
    [bg addSubview:usernameLabel];
    [usernameLabel setEnabled:NO];
    userField = [[NSTextField alloc] initWithFrame:NSMakeRect(center.x - 40, ypos, 140, 24)];
    [userField setEditable:YES];
    [userField setEnabled:YES];
    [bg addSubview:userField];
    ypos -= 48;

    s = [[NSAttributedString alloc] initWithString:@"Password" attributes:attr];
    Label *passwordLabel = [Label labelWithText:s atPoint:NSMakePoint(center.x - 130, ypos) withMaxWidth:width/2];
    [bg addSubview:passwordLabel];
    [passwordLabel setEnabled:NO];
    passField = [[NSSecureTextField alloc] initWithFrame:NSMakeRect(center.x - 40, ypos, 140, 24)];
    [passField setEditable:YES];
    [passField setEnabled:YES];
    [bg addSubview:passField];
    ypos -= 48;
    
    NSButton *ok = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 72, 32)];
    [ok setTarget:self];
    [ok setFont:font];
    [ok setAction:@selector(checkFields:)];
    [ok setTitle:@"Log in"];

    NSView *bv = [[NSView alloc] initWithFrame:NSMakeRect(width/2 - 36, ypos, 74, 34)];
    [bv addSubview:ok];
    [bg addSubview:bv];

    [self setNextKeyView:userField];
    [userField setNextKeyView:passField];
    [passField setNextKeyView:ok];
    [ok setNextKeyView:userField];

    [self setNeedsDisplay:YES];
    return self;
}

-(void)keyDown:(NSEvent *)event {
    NSLog(@"key %@", event);
}

static void badLogin() {
    NSLog(@"bad login");
    // FIXME: clear input fields
}

- (void)checkFields:(id)sender {
    NSString *username = [userField stringValue];
    NSString *password = [passField stringValue];
    NSLog(@"checkFields %@ %@", username, password);

    struct passwd *pw = getpwnam([username cString]);
    if(!pw) {
        badLogin();
        return;
    }

    char *enc = crypt([password cString], pw->pw_passwd);
    if(strcmp(enc, pw->pw_passwd)) {
        badLogin();
        return;
    }

    NSLog(@"username %s uid %d gid %d", pw->pw_name, pw->pw_uid, pw->pw_gid);

    exit(pw->pw_uid);
}

@end


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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#import <AppKit/AppKit.h>
#import "desktop.h"

const int width = 320;
const int height = 160;

@implementation LoginBox
- initWithDesktopWindow:(DesktopWindow *)window {
    NSRect rect = [window frame];
    NSPoint center = NSMakePoint(rect.size.width / 2, rect.size.height / 2);

    self = [super initWithFrame:NSMakeRect(center.x - width/2, center.y + height/2, width, height)];

    NSFont *font = [[NSFontManager sharedFontManager] convertFont:[NSFont systemFontOfSize:14]
        toHaveTrait:NSBoldFontMask];
    NSDictionary *attr = [NSDictionary dictionaryWithObjects:
        @[font, [NSColor whiteColor]]
        forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];

    int ypos = height - 32;

    NSAttributedString *s = [[NSAttributedString alloc] initWithString:@"User name" attributes:attr];
    Label *usernameLabel = [Label labelWithText:s atPoint:NSMakePoint(10, ypos) withMaxWidth:width/2];
    [self addSubview:usernameLabel];
    [usernameLabel setEnabled:NO];
    userField = [[NSTextField alloc] initWithFrame:NSMakeRect(width/2 + 2, ypos, width/2, 24)];
    [userField setEditable:YES];
    [userField setEnabled:YES];
    [self addSubview:userField];
    ypos -= 42;

    s = [[NSAttributedString alloc] initWithString:@"Password" attributes:attr];
    Label *passwordLabel = [Label labelWithText:s atPoint:NSMakePoint(10, ypos) withMaxWidth:width/2];
    [self addSubview:passwordLabel];
    [passwordLabel setEnabled:NO];
    passField = [[NSSecureTextField alloc] initWithFrame:NSMakeRect(width/2 + 2, ypos, width/2, 24)];
    [passField setEditable:YES];
    [passField setEnabled:YES];
    [self addSubview:passField];
    ypos -= 42;
    
    NSButton *ok = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 64, 24)];
    [ok setTarget:self];
    [ok setAction:@selector(checkFields:)];
    [ok setTitle:@"Log in"];

    NSView *bv = [[NSView alloc] initWithFrame:NSMakeRect(width/2 - 34, ypos, 66, 26)];
    [bv addSubview:ok];
    [self addSubview:bv];

    [self setNextKeyView:userField];
    [userField setNextKeyView:passField];
    [passField setNextKeyView:ok];
    [ok setNextKeyView:userField];

    [self setNeedsDisplay:YES];
    return self;
}

static void badLogin() {
    NSLog(@"bad login");
}

- (void)checkFields:(id)sender {
    NSString *username = [userField stringValue];
    NSString *password = [passField stringValue];

    seteuid(0);
    struct passwd *pw = getpwnam([username cString]);
    setresuid(65534, 65534, 0);
    if(!pw) {
        badLogin();
        return;
    }

    char *enc = crypt([password cString], pw->pw_passwd);
    if(strcmp(enc, pw->pw_passwd)) {
        badLogin();
        return;
    }

    if(fd >= 0)
        write(fd, &pw->pw_uid, sizeof(int));
    exit(0);
}

@end


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
    [bg setCornerRadius:12.0];
    [bg setFillColor:[NSColor colorWithDeviceRed:0.8 green:0.8 blue:0.8 alpha:1.0]];
    [self addSubview:bg];

    NSDictionary *attr = [NSDictionary dictionaryWithObjects:
        @[[NSFont systemFontOfSize:72], [NSColor darkGrayColor]]
        forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];

    NSDictionary *osVersionDictionary = [NSDictionary
        dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];

    NSAttributedString *osnameString = [[NSAttributedString alloc] initWithString:@"ravynOS" attributes:attr];
    osname = [[Label alloc] initWithText:osnameString
                                 atPoint:NSMakePoint(width/2 - [osnameString size].width / 2, height - 120)
                            withMaxWidth:width];
    [bg addSubview:osname];

    int diameter = 168;
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(NULL, diameter, diameter, 8, diameter*4, cs,
            kCGBitmapByteOrderDefault|kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(cs);
    CGContextSetRGBFillColor(ctx, 0.9, 0.9, 0.9, 1);
    CGContextFillEllipseInRect(ctx, NSMakeRect(0, 0, diameter, diameter));
    CGImageRef circle = CGBitmapContextCreateImage(ctx);
    NSImage *nscircle = [[NSImage alloc] initWithCGImage:circle size:NSMakeSize(diameter, diameter)];
    [nscircle setScalesWhenResized:NO];
    NSImageView *circleView = [[NSImageView alloc] initWithFrame:NSMakeRect(center.x - 84, 116, diameter, diameter)];
    [circleView setImage:nscircle];
    [bg addSubview:circleView];

    logoView = [[NSImageView alloc] initWithFrame:NSMakeRect(center.x - 80, 120, 160, 160)];
    [logoView setImageScaling:NSImageScaleAxesIndependently];
    [logoView setImageAlignment:NSImageAlignCenter];

    NSString *path = [[NSBundle mainBundle] pathForResource:@"SneakySnek" ofType:@"png"];
    NSImage *logo = [[NSImage alloc] initWithContentsOfFile:path];
    [logo setScalesWhenResized:YES];
    [logoView setImage:logo];
    [bg addSubview:logoView];

    attr = [NSDictionary dictionaryWithObjects:
                @[[NSFont systemFontOfSize:16], [NSColor grayColor]]
                forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];
    NSString *versionString = [NSString stringWithFormat:@"ravynOS %@",
             [osVersionDictionary objectForKey:@"ProductUserVisibleVersion"]];
    NSAttributedString *v  = [[NSAttributedString alloc] initWithString:versionString attributes:attr];
    Label *versionLabel = [Label labelWithText:v
                                       atPoint:NSMakePoint(center.x - [v size].width / 2, 80)
                                  withMaxWidth:width/2];
    [bg addSubview:versionLabel];

    versionString = [NSString stringWithFormat:@"\"%@\"", [osVersionDictionary objectForKey:@"ProductFamily"]];
    v  = [[NSAttributedString alloc] initWithString:versionString attributes:attr];
    versionLabel = [Label labelWithText:v
                                atPoint:NSMakePoint(center.x - [v size].width / 2, 50)
                           withMaxWidth:width/2];
    [bg addSubview:versionLabel];

    int ypos = height / 2 + 150;
    [bg addSubview:[Label labelWithText:[[NSAttributedString alloc]
                                        initWithString:@"Username" attributes:attr]
                                atPoint:NSMakePoint(center.x - 70, ypos)
                           withMaxWidth:width/2]];
    ypos -= 36;
    userField = [[NSTextField alloc] initWithFrame:NSMakeRect(center.x - 70, ypos, 140, 36)];
    [userField setEditable:YES];
    [userField setEnabled:YES];
    [bg addSubview:userField];
    ypos -= 24;

    [bg addSubview:[Label labelWithText:[[NSAttributedString alloc]
                                        initWithString:@"Password" attributes:attr]
                                atPoint:NSMakePoint(center.x - 70, ypos)
                           withMaxWidth:width/2]];
    ypos -= 36;
    passField = [[NSSecureTextField alloc] initWithFrame:NSMakeRect(center.x - 70, ypos, 140, 36)];
    [passField setEditable:YES];
    [passField setEnabled:YES];
    [bg addSubview:passField];

    ypos -= 48;
    ok = [[NSButton alloc] initWithFrame:NSMakeRect(center.x - 70, ypos, 140, 36)];
    [ok setTitle:@"Log In"];
    [ok setFont:[NSFont systemFontOfSize:16]];
    [ok setTarget:self];
    [ok setAction:@selector(checkFields:)];
    [ok setBezeled:YES];
    [ok setBezelStyle:NSRoundedBezelStyle];
    [bg addSubview:ok];
    
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


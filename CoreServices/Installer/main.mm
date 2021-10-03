/*
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
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

#import "ContentWindow.h"
#import <AppKit/NSRichTextReader.h>

int main(int argc, const char *argv[])
{
    __NSInitializeProcess(argc, argv);
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    NSString *appName = [[NSProcessInfo processInfo] processName];
    [NSApplication sharedApplication];

    NSMenu *menubar = [[NSMenu new] autorelease];
    [NSApp setMainMenu:menubar];

    NSBundle *myBundle = [NSBundle mainBundle];
    NSString *imgName = [myBundle objectForInfoDictionaryKey:@"SideImage"];
    if(imgName == nil)
        imgName = @"Emblem.jpg";

    ContentWindow *w = [[[ContentWindow alloc]
        initWithSize:NSMakeSize(720,480)
        bgColor:[NSColor
            colorWithDeviceRed:245.0 green:245.0 blue:245.0 alpha:255.0]]
        autorelease];
    [w setTitle:appName];
    [w setSideImage:[myBundle pathForResource:imgName ofType:@"jpg"]
        size:NSMakeSize(160,160)];

    NSAttributedString *text = [NSRichTextReader
        attributedStringWithContentsOfFile:[myBundle pathForResource:@"terms"
        ofType:@"rtf"]];
    [w setText:text];

    [NSApp run];
    return 0;
}

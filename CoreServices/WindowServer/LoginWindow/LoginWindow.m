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

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <desktop.h>

const NSString *WLOutputDidResizeNotification = @"WLOutputDidResizeNotification";
int fd = -1;

int main(int argc, const char *argv[]) {
    if(setresuid(65534, 65534, 0) != 0) {
        perror("setresuid");
        exit(-1);
    }

    __NSInitializeProcess(argc, argv);

    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    [NSApplication sharedApplication];
    AppDelegate *del = [AppDelegate new];
    if(!del || argc != 2)
        goto fail;

    fd = strtoul(argv[1], NULL, 10);

    [[NSNotificationCenter defaultCenter] addObserver:del selector:@selector(screenDidResize:)
        name:WLOutputDidResizeNotification object:nil];
    [pool drain];

    [NSApp run];

fail:
    exit(-1);
}


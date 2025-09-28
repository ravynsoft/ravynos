/*
 * Simple screengrab utility for WindowServer
 *
 * `grabscr filename.png` will save the main display (the display with
 * origin 0,0) contents as filename.png. Use `grabscr -d displayID` to
 * choose an alternate display and `grabscr -l` to list displays.
 *
 * Copyright (C) 2024 Zoe Knox <zoe@ravynsoft.com>
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

#import <stdio.h>
#import <unistd.h>
#import <CoreGraphics/CoreGraphics.h>
#import <AppKit/NSImage.h>

int listDisplays(void) {
    CGDirectDisplayID displayList[8];
    unsigned int displayCount = 0;

    @autoreleasepool {
        int ret = CGGetActiveDisplayList(8, displayList, &displayCount);
        if(ret != kCGErrorSuccess) {
            fprintf(stderr, "CGGetActiveDisplayList failed: %u\n", ret);
            return ret;
        }
    }

    for(int i = 0; i < displayCount; ++i) {
        printf("%x", displayList[i]);
        if(i+1 < displayCount)
            printf(" ");
    }
    printf("\n");
    return 0;
}

int main(int argc, char **argv) {
    int ret = 0;
    CGDirectDisplayID displayID = 0;

    while((ret = getopt(argc, argv, "ld:")) != -1) {
        switch(ret) {
            case 'l': exit(listDisplays());
            case 'd': displayID = strtoul(optarg, NULL, 16); break;
        }
    }

    argc -= optind;
    argv += optind;
    if(argc < 1) {
        fprintf(stderr, "Usage: grabscr [-l][-d display] filename.png\n\n");
        exit(1);
    }
    char *filename = argv[0];

    @autoreleasepool {
        if(displayID == 0)
            displayID = CGMainDisplayID();

        if(displayID == 0) {
            fprintf(stderr, "CGMainDisplayID failed\n");
            exit(1);
        }
        
        CGImageRef cgimage = CGDisplayCreateImage(displayID);
        NSBitmapImageRep *nsimage = [[NSBitmapImageRep alloc] initWithCGImage:cgimage];
        NSData *pngdata = [nsimage representationUsingType:NSPNGFileType properties:nil];
        if([pngdata writeToFile:[NSString stringWithCString:filename] atomically:NO] == NO)
            exit(1);
        printf("Wrote display %x contents to file %s\n", displayID, filename);
    }
    exit(0);
}


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

#import <Foundation/Foundation.h>
#include <unistd.h>
#include <string.h>
#include "GSGeomDisk.h"

const char *GEOM_CMD = "/sbin/geom";
NSMutableArray *disks = nil;

// FIXME: this should be replaced with libgeom
NSData *runCommand(const char *tool, const char *args) {
    int filedesc[2];

    signal(SIGCHLD, SIG_IGN); // no walking dead please.
    pipe(filedesc);
    pid_t pid = fork();

    if(pid == 0) { // child
        close(filedesc[0]);
        dup2(filedesc[1], 1); // pipe stdout to parent
        char **argv = malloc(sizeof(char *)*8);
        argv[0] = (char *)tool;

        char *mutable = strdup(args);
        int i = 0;
        char *arg = NULL;
        do {
            arg = strsep(&mutable, " ");
            argv[++i] = arg;
        } while(arg != NULL);

        execv(tool, argv);
        return nil;
    } else if(pid < 0) { // error
        perror(tool);
        close(filedesc[0]);
        close(filedesc[1]);
        return nil;
    } else { // parent
        close(filedesc[1]);
        NSFileHandle *reader = [[NSFileHandle alloc] initWithFileDescriptor:filedesc[0]];
        NSData *data = [[reader readDataToEndOfFile] retain];
        [reader release];
        return data; // caller must release
    }
}

BOOL parserError(NSString *msg) {
    NSLog(@"Parser error: %@",msg);
    return NO;
}

BOOL discoverGEOMs() {
    if(disks != nil)
        [disks release];
    disks = [[NSMutableArray arrayWithCapacity:4] retain];

    NSData *result = runCommand(GEOM_CMD, "disk list");
    NSString *str = [[NSString alloc] initWithData:result encoding:NSUTF8StringEncoding];
    NSArray *lines = [str componentsSeparatedByString:@"\n"];

    GSGeomDisk *curDisk = nil;
    for(int x = 0; x < [lines count]; ++x) {
        NSString *line = [lines objectAtIndex:x];
        if([line hasPrefix:@"Geom name:"]) {
            curDisk = [GSGeomDisk new];
            [curDisk setName:[line substringFromIndex:11]];
            [disks addObject:curDisk];
        } else if([line length] < 2) { // blank line
            curDisk = nil;
        } else if([line hasPrefix:@"   Mediasize:"]) {
            if(curDisk == nil)
                return parserError(@"Mediasize with curDisk==nil");
            [curDisk setMediaSize:[[line substringFromIndex:14] longLongValue]];
        } else if([line hasPrefix:@"   Sectorsize:"]) {
            if(curDisk == nil)
                return parserError(@"Sectorsize with curDisk==nil");
            [curDisk setSectorSize:[[line substringFromIndex:15] integerValue]];
        } else if([line hasPrefix:@"   descr:"]) {
            if(curDisk == nil)
                return parserError(@"descr with curDisk==nil");
            [curDisk setMediaDescription:[line substringFromIndex:10]];
        }
    }

    return YES;
}

int GSListDisks() {
    return 0;
}

#if defined(TEST)
int main(int argc, char **argv)
{
    discoverGEOMs();
    NSLog(@"%@",disks);
    return 0;
}
#endif

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

#import <AppKit/AppKit.h>
#import "GSGeomDisk.h"

#include <unistd.h>
#include <string.h>

const char *GEOM_CMD = "/sbin/geom";
NSMutableArray *disks = nil;

const long KB = 1024;
const long MB = 1024 * KB;
const long GB = 1024 * MB;
const long TB = 1024 * GB;

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

BOOL discoverGEOMs(BOOL onlyUsable) {
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
        } else if([line length] < 2) { // blank line
            if(onlyUsable) {
                if([curDisk mediaSize] >= 10*GB) {
                    switch([curDisk type]) {
                    case GS_DISK_TYPE_ATA:
                    case GS_DISK_TYPE_SCSI:
                        [disks addObject:curDisk];
                        break;
                    }
                }
            } else // onlyUsable == NO
                [disks addObject:curDisk];
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

NSString *formatMediaSize(long bytes) {
    double value;
    const char *suffix = "";

    if(bytes >= TB) {
        value = (double)bytes / (double)TB;
        suffix = "TB";
    } else if(bytes >= GB) {
        value = (double)bytes / (double)GB;
        suffix = "GB";
    } else if(bytes >= MB) {
        value = (double)bytes / (double)MB;
        suffix = "MB";
    } else {
        value = (double)bytes / (double)KB;
        suffix = "KB";
    }
    return [NSString stringWithFormat:@"%.2f %s", value, suffix];
}

@implementation GSGeomDisk

-init {
    _type = GS_DISK_TYPE_OTHER;
    _mediaSize = _sectorSize = 0;
    return self;
}

-(GSDiskType)type {
    return _type;
}

-(NSString *)name {
    return _name;
}

-(NSString *)mediaDescription {
    return _description;
}

-(long)mediaSize {
    return _mediaSize;
}

-(int)sectorSize {
    return _sectorSize;
}

-(void)setName:(NSString *)name {
    _name = [name retain];

    if([_name hasPrefix:@"ada"])
        _type = GS_DISK_TYPE_ATA;
    else if([_name hasPrefix:@"da"])
        _type = GS_DISK_TYPE_SCSI;
    else if([_name hasPrefix:@"cd"])
        _type = GS_DISK_TYPE_CD;
}

-(void)setMediaDescription:(NSString *)description {
    _description = [description retain];
}

-(void)setMediaSize:(long)size {
    _mediaSize = size;
}

-(void)setSectorSize:(int)size {
    _sectorSize = size;
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@:%08x> type:%d name:%@ size:%ld",[self class],self,_type,_name,_mediaSize];
}

// act as data source for NSTableView
- (int)numberOfRowsInTableView:(NSTableView *)tableView {
    return (disks == nil) ? 0 : [disks count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row {
    if(row > [disks count] || row < 0)
        return @"";

    GSGeomDisk *disk = [disks objectAtIndex:row];
    NSString *columnID = [tableColumn identifier];
    if([columnID isEqualToString:@"device"])
        return [disk name];
    if([columnID isEqualToString:@"size"])
        return formatMediaSize([disk mediaSize]);
    if([columnID isEqualToString:@"descr"])
        return [disk mediaDescription];
    return @"";
}

@end

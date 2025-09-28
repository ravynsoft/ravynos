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

extern NSMutableArray *disks;

extern const long KB;
extern const long MB;
extern const long GB;
extern const long TB;

typedef enum {
    GS_DISK_TYPE_OTHER = 0,
    GS_DISK_TYPE_ATA,
    GS_DISK_TYPE_SCSI,
    GS_DISK_TYPE_MMC,
    GS_DISK_TYPE_VIRTIO, // bhyve image
    GS_DISK_TYPE_CD
} GSDiskType;

NSData *runCommand(const char *tool, const char *args);
BOOL parserError(NSString *msg);
BOOL discoverGEOMs(BOOL onlyUsable);
NSString *formatMediaSize(long bytes);

@interface GSGeomDisk: NSObject {
    GSDiskType _type;
    NSString *_name;
    NSString *_description;
    long _mediaSize;
    int _sectorSize;
    id _delegate;
}

-(NSString *)name;
-(NSString *)mediaDescription;
-(long)mediaSize;
-(int)sectorSize;
-(GSDiskType)type;
-(id)delegate;

-(void)setName:(NSString *)name;
-(void)setMediaDescription:(NSString *)description;
-(void)setMediaSize:(long)size;
-(void)setSectorSize:(int)size;
-(void)setDelegate:(id)delegate;

-(void)deletePartitions;
-(void)createGPT;
-(void)createPartitions;
-(void)createPools;
-(void)initializeEFI;
-(void)copyFilesystem;
-(void)finalizeInstallation;

// act as data source for NSTableView
- (int)numberOfRowsInTableView:(NSTableView *)tableView;
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;

@end


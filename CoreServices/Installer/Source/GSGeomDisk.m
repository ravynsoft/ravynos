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

#import "GSGeomDisk.h"

@implementation GSGeomDisk

-init {
    [super init];
    _type = GS_DISK_TYPE_OTHER;
    _mediaSize = _sectorSize = 0;
    return self;
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

@end

/*
 * Copyright (C) 2024 Zoe Knox <zoe@pixin.net>
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

#import <sys/types.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

typedef enum WSDisplayFlags : uint32_t {
    kWSDisplayActive = (1 << 0),
    kWSDisplayOnline = (1 << 1),
    kWSDisplaySleeping = (1 << 2),
    kWSDisplayMirrored = (1 << 3),
    kWSDisplayPrimary = (1 << 4),
    kWSDisplayMain = (1 << 5),
    kWSDisplayBuiltin = (1 << 6),
    kWSDisplayStereo = (1 << 7),
    kWSDisplayHWMirror = (1 << 8)
} WSDisplayFlags;

@interface WSDisplay : NSObject {
    uint32_t _ID;               // CGDirectDisplayID
    uint32_t _flags;            // status flags                                
    uint32_t _openGLMask;       // CGDisplayOpenGLDisplayMask
}

-(uint32_t)getDisplayID;
-(BOOL)isActive;
-(BOOL)isOnline;
-(BOOL)isSleeping;
-(BOOL)isMain;
-(uint32_t)openGLMask;

// to implement in subclasses
-(CGRect)geometry;
@end


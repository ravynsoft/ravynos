/*
 * ravynOS Application Launcher & Status Bar
 *
 * Copyright (C) 2021-2022 Zoe Knox <zoe@pixin.net>
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

#pragma once

#import <AppKit/AppKit.h>
#import <unistd.h>
#import <sys/event.h>

#import "DockItem.h"

#define RADIUS 10      // rounded corner radius
#define CELL_SPACER 4  // pixels between grid cells
#define ICON_MIN 24
#define DOCK_HEIGHT_MAX 136
#define DOCK_HEIGHT_MIN 32
#define DOCK_LENGTH_MIN 128

#define INFOKEY_CUR_SIZE @"CurrentSize"
#define INFOKEY_LOCATION @"Location"
#define INFOKEY_CUR_ITEMS @"CurrentItems"
#define INFOKEY_FILER_DEF_FOLDER @"FilerDefaultFolder"

#define DIVIDER_MARGIN 10

#define DEBUG 1

#ifdef DEBUG
#define NSDebugLog NSLog
#else
#define NSDebugLog(fmt,...)
#endif

extern int kqPIDs;

enum Location : int {
    LOCATION_BOTTOM = 0,
    LOCATION_LEFT = 1,
    LOCATION_RIGHT = 2
};



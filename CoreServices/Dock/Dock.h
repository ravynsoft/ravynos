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
#define END_CAP 8
#define TILESIZE_MIN 24
#define TILESIZE_MAX 256

#define INFOKEY_LOCATION @"Location"
#define INFOKEY_OPACITY @"Opacity"
#define INFOKEY_WALLPAPER @"Wallpaper"

// These properties are for compatibility with com.apple.Dock.plist
// Source: https://gist.github.com/brandonb927/3195465/
#define INFOKEY_PERSISTENT_APPS @"persistent-apps" // array
#define INFOKEY_PERSISTENT_OTHERS @"persistent-others" // array (of tile data?)
#define INFOKEY_TILESIZE @"tilesize" // int
#define INFOKEY_AUTOHIDE @"autohide" // BOOL
#define INFOKEY_AUTOHIDE_DELAY @"autohide-delay" // float
#define INFOKEY_AUTOHIDE_TIME_MODIFIER @"autohide-time-modifier" // float

// others found at https://real-world-systems.com/docs/defaults.1.html
/*
com.apple.dock persistent-others -array-add '{ "tile-data" = { "list-type" = 1; }; "tile-type" = "recents-tile"; }'; killall Dock
com.apple.dock largesize -int 512; killall Dock
com.apple.dock show-exposemenus -boolean no; killall Dock
com.apple.dock showhidden -bool YES; killall Dock
com.apple.dock no-glass -boolean YES; killall Dock
com.apple.dock mouse-over-hilitestack -boolean YES; killall Dock
com.apple.dock use-new-liststack -boolean YES; killall Dock 
defaults write com.apple.dock no-bouncing -bool TRUE

*/

#define DIVIDER_MARGIN 10

#define DEBUG 1

#ifdef DEBUG
#define NSDebugLog NSLog
#else
#define NSDebugLog(fmt,...)
#endif

extern int _kq;

enum Location : int {
    LOCATION_BOTTOM = 0,
    LOCATION_LEFT = 1,
    LOCATION_RIGHT = 2
};
typedef enum Location Location;

@interface Dock: NSObject {
    pid_t _PID;
    NSUserDefaults *_prefs;
    NSMutableArray *_items;
    Location _location;
    NSScreen *_screen;
    NSWindow *_window;
    NSMutableDictionary *_desktops;
    NSSize _currentSize;
    int _tileSize;
    float _alpha;
    pthread_t kqThread;
}

-(id)init;
-(void)screenDidResize:(NSNotification *)note;
-(void)updateBackground;
-(int)fitWindowToItems;
-(void)placeItemsInWindow:(int)maxItems;
-(void)loadItems;
-(void)relocate;
-(void)processKernelQueue;
-(DockItem *)dockItemForPath:(NSString *)path;

@end

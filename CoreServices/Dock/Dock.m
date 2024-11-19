/*
 * ravynOS Application Launcher & Status Bar
 *
 * Copyright (C) 2021-2024 Zoe Knox <zoe@pixin.net>
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
#import <CoreGraphics/CGWindowLevel.h>
#import "Dock.h"
#import "DockTileData.h"
#import "DesktopWindow.h"

@implementation Dock

-(id)init {
    _PID = getpid();
    _prefs = [NSUserDefaults standardUserDefaults];
    _desktops = [NSMutableDictionary new];
    _currentSize = NSZeroSize;

    _tileSize = [_prefs integerForKey:INFOKEY_TILESIZE];
    if(_tileSize < TILESIZE_MIN)
        _tileSize = 64;

    int pos = [_prefs integerForKey:INFOKEY_LOCATION];
    _location = (Location)pos;

    _alpha = [_prefs floatForKey:INFOKEY_OPACITY];
    if(_alpha <= 1.0)
        _alpha = 0.85;

    [self loadItems];

    int max = [self fitWindowToItems];
    NSRect frame = NSMakeRect(0,0,_currentSize.width,_currentSize.height);
    [self createWindowWithFrame:frame];
    [self placeItemsInWindow:max];

    [_window orderFront:nil];
    return self;
}

-(void)applicationWillFinishLaunching:(NSNotification *)note {
    DesktopWindow *desktop = [[DesktopWindow alloc] initForScreen:[NSScreen mainScreen]];
    [_desktops setObject:desktop forKey:@"NSMainScreen"]; // FIXME: use NSScreen deviceDescription dict for display ID
    [desktop setDelegate:self];
    [desktop makeKeyAndOrderFront:nil];
    [self savePrefs];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(appDidQuit:)
        name:@"NSApplicationDidQuit" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(appDidLaunch:)
        name:@"NSApplicationDidLaunch" object:nil];
}

-(void)appDidLaunch:(NSNotification *)note {
    NSMutableDictionary *dict = (NSMutableDictionary *)[note userInfo];
    NSString *bundleID = [dict objectForKey:@"BundleID"];
    pid_t pid = [[dict objectForKey:@"ProcessID"] intValue];
    NSString *path = [dict objectForKey:@"Path"];

    // just to be safe
    if([bundleID isEqualToString:@"com.ravynos.Dock"] ||
        [bundleID isEqualToString:@"com.ravynos.WindowServer"] ||
        [bundleID isEqualToString:@"com.ravynos.SystemUIServer"] ||
        [bundleID isEqualToString:@"com.ravynOS.LoginWindow"])
            return;

    DockItem *item = [self dockItemForPath:path];
    if(item == nil) {
        item = [DockItem dockItemWithPath:path];
        if([item type] == DIT_INVALID) {
            NSDebugLog(@"Invalid bundle type for %@", path);
            return;
        }

        [item setRunning:YES];
        [_items addObject:item];
        [self relocate];
    } else if([item isPersistent])
        [item setRunning:YES];
    else
        NSDebugLog(@"Attempted to add launched bundle %@ with path %@, already added",
                bundleID, path);
    [self setNeedsDisplay:YES];
}

-(void)appDidQuit:(NSNotification *)note {
    NSMutableDictionary *dict = (NSMutableDictionary *)[note userInfo];
    NSString *path = [dict objectForKey:@"Path"];
    DockItem *item = [self dockItemForPath:path];

    if(item == nil) {
        NSDebugLog(@"App %@ exited but not found in our list");
        return;
    }

    [item setRunning:NO];
    if(![item isPersistent]) {
        [_items removeObjectIdenticalTo:item];
        [self relocate];
    }
    [self setNeedsDisplay:YES];
}

-(DockItem *)dockItemForPath:(NSString *)path {
    for(int i = 0; i < [_items count]; ++i) {
        DockItem *item = [_items objectAtIndex:i];
        if([item hasPath:path])
            return item;
    }
    return nil;
}

-(NSRect)positionWindowWithFrame:(NSRect)frame {
    NSScreen *mainScreen = [NSScreen mainScreen];
    switch(_location) {
        case LOCATION_LEFT:
            frame.origin.x = 0;
            frame.origin.y = [mainScreen frame].size.height / 2 - frame.size.height / 2;
            break;
        case LOCATION_RIGHT:
            frame.origin.x = [mainScreen frame].size.width - frame.size.width;
            frame.origin.y = [mainScreen frame].size.height / 2 - frame.size.height / 2;
            break;
        default:
            frame.origin.x = [mainScreen frame].size.width / 2 - frame.size.width / 2;
            frame.origin.y = 0;
    }
    return frame;
}

-(NSWindow *)createWindowWithFrame:(NSRect)frame {
    _window = [[NSWindow alloc] initWithContentRect:[self positionWindowWithFrame:frame]
                                          styleMask:NSBorderlessWindowMask
                                            backing:NSBackingStoreBuffered
                                              defer:NO];
    [_window setBackgroundColor:[NSColor colorWithDeviceRed:0.666 green:0.666
        blue:0.666 alpha:_alpha]];
    [_window setLevel:kCGDockWindowLevelKey];
    
    return _window;
}

-(NSArray *)tileDataForAppItems {
    NSMutableArray *a = [NSMutableArray new];
    for(int i = 0; i < [_items count]; ++i) {
        DockItem *di = [_items objectAtIndex:i];
        if(![di isPersistent] || [[di bundleIdentifier] hasPrefix:@"com.ravynos.Dock"])
            continue;
        [a addObject:[di tileData]];
    }
    return [NSArray arrayWithArray:a];
}

-(NSArray *)tileDataForOtherItems {
    return [NSArray new];
}

-(void)savePrefs {
    [_prefs setInteger:_tileSize forKey:INFOKEY_TILESIZE];
    [_prefs setInteger:_location forKey:INFOKEY_LOCATION];
    [_prefs setFloat:_alpha forKey:INFOKEY_OPACITY];

    NSMutableDictionary *_wallpaper = [NSMutableDictionary new];
    NSArray *values = [_desktops allValues];
    for(unsigned i = 0; i < [values count]; ++i) {
        DesktopWindow *dw = [values objectAtIndex:i];
        [_wallpaper setObject:[dw wallpaperPath] forKey:@"NSMainScreen"]; // FIXME: CGDisplayID
    }
    [_prefs setObject:_wallpaper forKey:INFOKEY_WALLPAPER];
    [_prefs setObject:[self tileDataForAppItems] forKey:INFOKEY_PERSISTENT_APPS];
    //[_prefs setObject:[self tileDataForOtherItems] forKey:INFOKEY_PERSISTENT_OTHERS];

    [_prefs synchronize];
}

// size the window to our number of items. if it exceeds the screen size, shrink the tiles
// until it fits or we reach the minimum tile size. truncate items if we can't display them.
-(int)fitWindowToItems {
    NSSize scrSize = [[NSScreen mainScreen] visibleFrame].size;

    int maxLength = (_location == LOCATION_BOTTOM) ? scrSize.width : scrSize.height;
    int numItems = [_items count];
    int needLength;

    do {
        needLength = numItems * _tileSize + numItems * CELL_SPACER + 2 * END_CAP;
        if(needLength > maxLength)
            _tileSize -= 8;
    } while(needLength > maxLength && _tileSize > TILESIZE_MIN);
    
    int maxItems = numItems;
    while(needLength > maxLength && maxItems > 0) {
        --maxItems;
        needLength = maxItems * _tileSize + maxItems * CELL_SPACER + 2 * END_CAP;
    }

    if(_location == LOCATION_BOTTOM) {
        _currentSize.height = _tileSize + 16;
        _currentSize.width = needLength;
    } else {
        _currentSize.height = needLength;
        _currentSize.width = _tileSize + 16;
    }

    return maxItems;
}

-(void)placeItemsInWindow:(int)maxItems {
    if(maxItems < [_items count])
        NSLog(@"Warning: truncating some items to fit the screen");
    NSPoint itemPos = NSMakePoint(8, 0);
    NSSize size = NSMakeSize(_tileSize, _tileSize);
    [[_window contentView] setSubviews:nil];

    for(int i = 0; i < maxItems; ++i) {
        DockItem *item = [_items objectAtIndex:i];
        [item setFrameOrigin:itemPos];
        [item setTileSize:size];
        NSLog(@"placing item %d at %@ frame %@", i, NSStringFromPoint(itemPos), NSStringFromRect([item frame]));
        [[_window contentView] addSubview:item];
        if(_location == LOCATION_BOTTOM)
            itemPos.x += _tileSize + CELL_SPACER / 2;
        else
            itemPos.y += _tileSize + CELL_SPACER / 2;
    }
    [[_window contentView] setNeedsDisplay:YES];
}

-(void)loadItems {
    if(_items == nil)
        _items = [NSMutableArray arrayWithCapacity:10];

    NSMutableArray *pa = [NSMutableArray arrayWithCapacity:10];
    [pa addObjectsFromArray:[_prefs objectForKey:INFOKEY_PERSISTENT_APPS]];

    if(!pa || [pa count] == 0) {
        // populate default apps
        [pa addObject:dockTileData(@"/System/Library/CoreServices/Filer.app")];
        [pa addObject:dockTileData(@"/Applications/Utilities/Terminal.app")];
        //[pa addObject:dockTileData(@"/Applications/Utilities/Install ravynOS.app")];
    }

    for(int i = 0; i < [pa count]; ++i) {
        NSDictionary *dict = [pa objectAtIndex:i];
        DockItem *di = [DockItem dockItemWithPath:CFURLString(dict)];
        [di setPersistent:YES];
        [_items addObject:di];
    }

    // FIXME: load persistent-other items

    NSString *specials[] = {@"Downloads",@"Trash"};
    for(int x = 0; x < 2; ++x) {
        NSString *bundle = [[NSBundle mainBundle] pathForResource:specials[x] ofType:@"app"];
        DockItem *di = [DockItem dockItemWithPath:bundle];
        [di setPersistent:YES];
        [_items addObject:di];
    }
}

// FIXME: call this when _location changes
-(void)relocate {
    if(_window)
        [_window orderOut:nil];
    _window = nil;

    int maxItems = [self fitWindowToItems];
    NSRect frame = NSZeroRect;
    frame.size = _currentSize;
    [self createWindowWithFrame:frame];
    [self placeItemsInWindow:maxItems];
    [self savePrefs];

    [_window orderFront:nil];
}

@end

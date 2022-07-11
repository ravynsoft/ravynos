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

#import <AppKit/AppKit.h>
#import "Dock.h"
#import "DesktopWindow.h"

static const NSString *WLOutputDidResizeNotification = @"WLOutputDidResizeNotification";

@implementation Dock

-(id)init {
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
    [self savePrefs];

    int max = [self fitWindowToItems];
    NSRect frame = NSMakeRect(0,0,_currentSize.width,_currentSize.height);
    [self createWindowWithFrame:frame];
    [self placeItemsInWindow:max];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(screenDidResize:)
        name:WLOutputDidResizeNotification object:nil];

    [_window orderFront:nil];
    return self;
}

-(NSWindow *)createWindowWithFrame:(NSRect)frame {
    int mask = NSBorderlessWindowMask|WLWindowLayerOverlay;
    switch(_location) {
        case LOCATION_LEFT:
            mask |= WLWindowLayerAnchorLeft;
            break;
        case LOCATION_RIGHT:
            mask |= WLWindowLayerAnchorRight;
            break;
        default:
            mask |= WLWindowLayerAnchorBottom;
    }

    _window = [[NSWindow alloc] initWithContentRect:frame styleMask:mask
        backing:NSBackingStoreBuffered defer:NO];
    [_window setBackgroundColor:[NSColor colorWithDeviceRed:0.666 green:0.666
        blue:0.666 alpha:_alpha]];
    
    return _window;
}

-(void)screenDidResize:(NSNotification *)note {
    NSMutableDictionary *dict = (NSMutableDictionary *)[note userInfo];

    NSRect frame = NSZeroRect;
    frame.size = NSSizeFromString([dict objectForKey:@"WLOutputSize"]);
    frame.origin = NSPointFromString([dict objectForKey:@"WLOutputPosition"]);

    DesktopWindow *desktop = [[DesktopWindow alloc] initWithFrame:frame forOutput:dict];
    [_desktops setObject:desktop forKey:[dict objectForKey:@"WLOutputXDGOutput"]];
    [desktop setDelegate:self];
    [desktop makeKeyAndOrderFront:nil];

    [self savePrefs];
}

-(void)savePrefs {
    [_prefs setInteger:_tileSize forKey:INFOKEY_TILESIZE];
    [_prefs setInteger:_location forKey:INFOKEY_LOCATION];
    [_prefs setFloat:_alpha forKey:INFOKEY_OPACITY];

    NSMutableDictionary *_wallpaper = [NSMutableDictionary new];
    NSArray *values = [_desktops allValues];
    for(unsigned i = 0; i < [values count]; ++i) {
        DesktopWindow *dw = [values objectAtIndex:i];
        NSDictionary *props = [dw properties];

        [_wallpaper setObject:[dw wallpaperPath] forKey:[props objectForKey:@"WLOutputModel"]];
    }
    [_prefs setObject:_wallpaper forKey:INFOKEY_WALLPAPER];

    [_prefs synchronize];
}

// size the window to our number of items. if it exceeds the screen size, shrink the tiles
// until it fits or we reach the minimum tile size. truncate items if we can't display them.
-(int)fitWindowToItems {
    NSSize scrSize = [_screen visibleFrame].size;
    if(_screen == nil)
        scrSize = NSMakeSize(1280,720); // minimum resolution

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

    // FIXME: load from persistent-apps key
    DockItem *di =[DockItem dockItemWithPath:@"/System/Library/CoreServices/Filer.app"]; 
    [di setLocked:YES];
    [di setResident:YES];
    [_items addObject:di];

    di = [DockItem dockItemWithPath:@"/Applications/Utilities/Install ravynOS.app"];
    [di setResident:YES];
    [_items addObject:di];

    di = [DockItem dockItemWithMinimizedWindow:1 forApp:di];
    [_items addObject:di];

    NSString *specials[] = {@"Downloads",@"Trash"};
    for(int x = 0; x < 2; ++x) {
        NSString *bundle = [[NSBundle mainBundle] pathForResource:specials[x] ofType:@"app"];
        di = [DockItem dockItemWithPath:bundle];
        [di setResident:YES];
        [di setLocked:YES];
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

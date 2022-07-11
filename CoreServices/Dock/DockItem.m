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

#import "Dock.h"
#import "DockItem.h"

#define MSG_ID_INLINE 90211
#define CODE_APP_ACTIVATE 5
#define CODE_APP_HIDE 6

#define RUNMK_DIAMETER 8
#define RUNMK_SPACER 2

typedef struct {
    mach_msg_header_t header;
    unsigned int code;
    unsigned char data[64*1024];
    unsigned int len;
    mach_msg_trailer_t trailer;
} Message;

@implementation DockItem

+dockItemWithPath:(NSString *)path {
    return [[DockItem alloc] initWithPath:path];
}

+dockItemWithMinimizedWindow:(unsigned int)window forApp:(DockItem *)appItem {
    return [[DockItem alloc] initWithMinimizedWindow:window forApp:appItem];
}

+(int)iconSize {
    NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
    int size = [prefs integerForKey:INFOKEY_TILESIZE];
    return size ? size : 64;
}

/* Application Dock Tiles

   An application Dock tile defaults to display the application’s
   applicationIconImage.

   The application Dock tile never shows a smaller application icon badge.

   Whether using the default or custom view, the application Dock tile may be
   badged with a short custom string.
*/

-initWithPath:(NSString *)path {
    _path = path;
    _label = nil;
    _type = DIT_INVALID;
    _icon = nil;
    _bundleID = nil;
    _app = nil;
    _isRunning = NO;
    int size = [DockItem iconSize];

    self = [super initWithFrame:NSMakeRect(0,0,size,size + 16)];
    // first, walk the path for .app or .AppDir in case this is the
    // path to the actual executable inside
    NSArray *comps = [path pathComponents];
    for(int i = [comps count] - 1; i >= 0; --i) {
        if([[comps objectAtIndex:i] hasSuffix:@"app"] == YES) {
            path = [NSString stringWithFormat:@"/%@",
                [NSString pathWithComponents:
                [comps subarrayWithRange:NSMakeRange(0,i+1)]]];
            break;
        }
    }

    NSURL *url = [NSURL fileURLWithPath:path];
    if(LSIsNSBundle((__bridge CFURLRef)url)) {
        _type = DIT_APP_BUNDLE;
        NSBundle *b = [NSBundle bundleWithPath:path];
        _label = [b objectForInfoDictionaryKey:@"CFBundleDisplayName"];
        if(!_label)
            _label = [b objectForInfoDictionaryKey:@"CFBundleName"];
        _execPath = [b executablePath];

        NSString *iconFile = [b objectForInfoDictionaryKey:
            @"CFBundleIconFile"];
        if(!iconFile)
            iconFile = [b objectForInfoDictionaryKey:@"NSIcon"];
        NSString *iconPath = [NSString stringWithFormat:@"%@/Resources/%@",path,iconFile];

        _icon = [[NSImageView alloc] initWithFrame:NSMakeRect(
            0,RUNMK_DIAMETER+2*RUNMK_SPACER,size,size)];
        [_icon setImage:[[NSImage alloc] initWithContentsOfFile:iconPath]];
        [[_icon image] setScalesWhenResized:YES];
        [_icon setImageScaling:NSImageScaleProportionallyUpOrDown];

        _bundleID = [b objectForInfoDictionaryKey:@"CFBundleIdentifier"];
    } else if(LSIsAppDir((__bridge CFURLRef)url)) {
        _type = DIT_APP_APPDIR;
        _execPath = [_path stringByAppendingPathComponent:@"AppRun"];
        _label = [[path lastPathComponent] stringByDeletingPathExtension];
        NSString *iconFile = [NSString stringWithFormat:@"%@/.DirIcon", path];
        if([[NSFileManager defaultManager] fileExistsAtPath:iconFile]) {
            _icon = [[NSImageView alloc] initWithFrame:NSMakeRect(
                0,RUNMK_DIAMETER+2*RUNMK_SPACER,size,size)];
            [_icon setImage:[[NSImage alloc] initWithContentsOfFile:iconFile]];
            [[_icon image] setScalesWhenResized:YES];
            [_icon setImageScaling:NSImageScaleProportionallyUpOrDown];
        }
    }

    if(_icon == nil) {
        NSString *windowPNG = [[NSBundle mainBundle] pathForResource:@"window" ofType:@"png"];
        _icon = [[NSImageView alloc] initWithFrame:NSMakeRect(
            0,RUNMK_DIAMETER+2*RUNMK_SPACER,size,size)];
        [_icon setImage:[[NSImage alloc] initWithContentsOfFile:windowPNG]];
        [[_icon image] setScalesWhenResized:YES];
        [_icon setImageScaling:NSImageScaleProportionallyUpOrDown];
    }

    _origIcon = [_icon copy];
    [self addSubview:_icon];

    _flags = DIF_NORMAL;
    _pids = [NSMutableArray new];
    _windows = [NSMutableArray new];

    NSString *marker = [[NSBundle mainBundle] pathForResource:@"running" ofType:@"png"];
    _runMarker = [[NSImageView alloc] initWithFrame:NSMakeRect(
        size/2 - RUNMK_DIAMETER/2, RUNMK_SPACER, RUNMK_DIAMETER, RUNMK_DIAMETER)];
    [_runMarker setImage:[[NSImage alloc] initWithContentsOfFile:marker]];
    [[_runMarker image] setScalesWhenResized:YES];

    [_icon setTarget:self];
    [_icon setAction:@selector(openApp:)];
    return self;
}

/*
  Window Dock Tiles
  
  A window Dock tile defaults to display a miniaturized version of the windows
  contents with a badge derived from the application Dock icon, including any
  customized application Dock icon. The default window Dock tile image may not
  be badged with a custom string.

  A window Dock tile can use a custom view to draw the Dock icon. If a custom
  view is used, no application badge will be added, but the text label will be
  overlaid on top of the icon.
*/

-initWithMinimizedWindow:(unsigned int)window forApp:(DockItem *)appItem {
    _path = nil;
    _execPath = nil;
    _label = nil;
    _app = appItem;
    _type = DIT_WINDOW;
    _isRunning = NO;
    int size = [DockItem iconSize];
    self = [super initWithFrame:NSMakeRect(0,0,size,size+16)];

    NSString *windowPNG = [[NSBundle mainBundle] pathForResource:@"window" ofType:@"png"];
    _icon = [[NSImageView alloc] initWithFrame:NSMakeRect(
        size*0.2, RUNMK_DIAMETER+2*RUNMK_SPACER, size-(size*0.2), size-RUNMK_DIAMETER-RUNMK_SPACER)];
    [_icon setImageScaling:NSImageScaleProportionallyUpOrDown];
    [_icon setImage:[[NSImage alloc] initWithContentsOfFile:windowPNG]];
    [[_icon image] setScalesWhenResized:YES];

    _origIcon = [_icon copy];
    [self addSubview:_icon];

    if(appItem != nil) {
        _badge = [[NSImageView alloc] initWithFrame:
            NSMakeRect(0,RUNMK_DIAMETER+2*RUNMK_SPACER,size*0.55,size*0.55)];
        [_badge setImageScaling:NSImageScaleProportionallyUpOrDown];
        [_badge setImage:[appItem icon]];
        [[_badge image] setScalesWhenResized:YES];
        [self addSubview:_badge];
    }

    _bundleID = nil;
    _flags = DIF_NORMAL;
    _pids = [NSMutableArray new];
    _windows = [NSMutableArray new];
    [_windows addObject:[NSNumber numberWithInteger:window]];

    [_icon setTarget:self];
    [_icon setAction:@selector(activateWindow:)];
    return self;
}

-(void)setTileSize:(NSSize)size {
    NSSize framesize = size;
    framesize.height += 16;
    [self setFrameSize:framesize];
    [_icon setFrameSize:size];
    [self setNeedsDisplay:YES];
}

-(NSString *)path {
    return _path;
}

-(NSString *)execPath {
    return _execPath;
}

-(NSString *)bundleIdentifier {
    return _bundleID;
}

-(NSString *)label {
    return _label;
}

-(DockItemType)type {
    return _type;
}

-(DockItemFlags)flags {
    return (DockItemFlags)_flags;
}

-(BOOL)isNormal {
    return (_flags == DIF_NORMAL) ? YES : NO;
}

-(BOOL)isLocked {
    return (_flags & DIF_LOCKED) ? YES : NO;
}

-(BOOL)isRunning {
    if(_pids && [_pids count])
        return YES;
    return NO;
}

-(BOOL)isResident {
    return (_flags & DIF_RESIDENT) ? YES : NO;
}

-(BOOL)needsAttention {
    return (_flags & DIF_ATTENTION) ? YES : NO;
}

-(pid_t)pid {
    if(_pids && [_pids count])
        return [[_pids firstObject] intValue];
    return 0;
}

-(NSArray *)pids {
    return [NSArray arrayWithArray:_pids];
}

-(unsigned int)window {
    if(_windows && [_windows count])
        return [[_windows firstObject] intValue];
    return 0;
}

-(NSArray *)windows {
    return [NSArray arrayWithArray:_windows];
}

-(NSImage *)icon {
    return [_icon image];
}

// YES if equal to either item path
// YES if bundle and path is within bundle
// NO otherwise
-(BOOL)hasPath:(NSString *)path {
    if([path isEqualToString:_execPath] || [path isEqualToString:_path])
        return YES;

    if(_type != DIT_APP_BUNDLE)
        return NO;

    NSArray *comps = [path pathComponents];
    // FIXME: should this be reverse order to find sub bundles?
    for(int i = 0; i < [comps count]; ++i) {
        if([[comps objectAtIndex:i] hasSuffix:@"app"] == YES) {
            NSString *newpath = [NSString stringWithFormat:@"/%@",
            [NSString pathWithComponents:
                [comps subarrayWithRange:NSMakeRange(0,i+1)]]];
            NSBundle *b = [NSBundle bundleWithModulePath:newpath];
            if([[b bundlePath] isEqualToString:_path])
                return YES;
        }
    }
    return NO;
}

-(void)setFlags:(DockItemFlags)flags {
    _flags = flags;
}

-(void)setNormal {
    _flags = DIF_NORMAL;
}

-(void)setLocked:(BOOL)value {
    if(value == YES)
        _flags |= DIF_LOCKED;
    else
        _flags &= ~DIF_LOCKED;
}

-(BOOL)_checkIsRunning {
    BOOL _currentlyRunning = _isRunning;
    _isRunning = ([_pids count] == 0) ? NO : YES;
    if(_currentlyRunning && !_isRunning) {
        [self replaceSubview:_runMarker with:nil];
    } else if(!_currentlyRunning && _isRunning) {
        [self addSubview:_runMarker];
    }
    [self setNeedsDisplay:YES];
}

-(void)addPID:(pid_t)pid {
    if(pid == 0)
        NSLog(@"addPID: pid of 0 is invalid");
    else {
//        struct kevent e[1];

        for(int i = 0; i < [_pids count]; ++i) {
            if([[_pids objectAtIndex:i] intValue] == pid)
                return; // already have this PID
        }
        [_pids addObject:[NSNumber numberWithInteger:pid]];
//        EV_SET(e, pid, EVFILT_PROC, EV_ADD, NOTE_FORK|NOTE_EXEC|NOTE_TRACK|NOTE_EXIT, 0, (__bridge void *)self);

        // wake up kqueue to insert the event
//        write(piper(1), e, sizeof(struct kevent));
    }
    [self _checkIsRunning];
}

-(void)removePID:(pid_t)pid {
    if(pid == 0)
        NSLog(@"removePID: pid of 0 is invalid");
    else {
//        struct kevent e[1];

        for(int i = 0; i < [_pids count]; ++i) {
            if([[_pids objectAtIndex:i] intValue] == pid) {
                [_pids removeObjectAtIndex:i];
//                EV_SET(e, pid, EVFILT_PROC, EV_DELETE, 0, 0, (__bridge void *)self);

                // wake up kqueue to insert the event
//                write(piper(1), e, sizeof(struct kevent));
                return;
            }
        }
        NSDebugLog(@"removePID: pid %d not found", pid);
    }

    [self _checkIsRunning];
}

-(void)addWindow:(unsigned int)window {
    for(int i = 0; i < [_windows count]; ++i) {
        if([[_windows objectAtIndex:i] intValue] == window)
            return;
    }
    [_windows addObject:[NSNumber numberWithInteger:window]];
}

-(void)removeWindow:(unsigned int)window {
    for(int i = 0; i < [_windows count]; ++i) {
        if([[_windows objectAtIndex:i] intValue] == window) {
            [_windows removeObjectAtIndex:i];
            return;
        }
    }
}

-(void)setResident:(BOOL)value {
    if(value == YES)
        _flags |= DIF_RESIDENT;
    else
        _flags &= ~DIF_RESIDENT;
}

-(void)setNeedsAttention:(BOOL)value {
    if(value == YES)
        _flags |= DIF_ATTENTION;
    else
        _flags &= ~DIF_ATTENTION;
}

-(void)setApplicationIconImage:(NSImage *)image {
    if(image == nil) { // user wants to remove customization
        [_icon removeFromSuperview];
        _icon = [_origIcon copy];
        [self addSubview:_icon];
        return;
    }

    [image setScalesWhenResized:YES];
    [_icon setImage:image];
}

/*
The view you specify should be height and width resizable.

Cocoa does not automatically redraw the contents of your dock tile. Instead,
your application must explicitly send display messages to the dock tile object
whenever the contents of your view change and need to be redrawn. Your dock
tile view is responsible for drawing the entire contents of the dock tile. Your
view does not need to draw the application or custom string badges.
*/

-(void)setContentView:(NSView *)contentView {
    [_icon removeFromSuperview];
    _icon = contentView;
    [self addSubview:_icon];
}

-(void)activateWindow:(id)sender {
    int windowNumber = [[_windows firstObject] intValue];
    int processID = [[[_app pids] firstObject] intValue];
    Message activate = {0};
    activate.header.msgh_remote_port = [NSApp _wsServicePort];
    activate.header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND, 0, 0, 0);
    activate.header.msgh_id = MSG_ID_INLINE;
    activate.header.msgh_size = sizeof(activate) - sizeof(mach_msg_trailer_t);
    activate.code = CODE_APP_ACTIVATE;
    memcpy(activate.data, &processID, sizeof(int));
    memcpy(activate.data+sizeof(int), &windowNumber, sizeof(int));
    activate.len = sizeof(int)*2;
    mach_msg((mach_msg_header_t *)&activate, MACH_SEND_MSG,
        sizeof(activate) - sizeof(mach_msg_trailer_t),
        0, MACH_PORT_NULL, 2000 /* ms timeout */, MACH_PORT_NULL);
}

-(void)openApp:(id)sender {
    NSURL *url = [NSURL fileURLWithPath:_path];
    LSOpenCFURLRef((__bridge_retained CFURLRef)url, NULL);
}

-(NSString *)description {
    return [NSString stringWithFormat:
    @"<%@ 0x%p> path:%@ exec:%@ label:%@ pid:%@ windows:%@ flags:0x%02x id:%@",
    [self class], self, _path, _execPath, _label, _pids, _windows, _flags,
    _bundleID];
}
@end


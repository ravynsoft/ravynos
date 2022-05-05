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
#import "Utils.h"
#import "WindowTracker.h"
#import <LaunchServices/LaunchServices.h>
#import <XdgDesktopFile>
#import <QMouseEvent>

extern Dock *g_dock;

DIWidget::DIWidget(NSObject *owner)
{
    _owner = owner;
    setAlignment(Qt::AlignCenter);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

DIWidget::~DIWidget()
{
}

void DIWidget::mousePressEvent(QMouseEvent *e)
{
    switch(e->button()) {
        case Qt::LeftButton: // logical left, the primary action
            NSDebugLog(@"click res=%d type=%d %@, start timer for menu",
                [(DockItem*)_owner isResident],
                [(DockItem*)_owner type],
                [(DockItem*)_owner label]);
            e->accept();
            break;
        case Qt::RightButton: // logical right, the secondary action
            NSDebugLog(@"right click %@, show the menu now",
                [(DockItem*)_owner label]);
            e->accept();
            break;
        default: break;
    }
}

void DIWidget::mouseReleaseEvent(QMouseEvent *e)
{
    DockItem *owner = (DockItem *)_owner;
    switch(e->button()) {
        case Qt::LeftButton: // logical left, the primary action
        {
            if([owner type] == DIT_WINDOW) { // unminimizing?
                unsigned int window = [owner window];
                WindowTracker::activateWindow(window);
                g_dock->clearRunningLabel((__bridge void *)owner);
                e->accept();
                break;
            }
            if([owner isRunning]) { // clicked a running app?
                unsigned int window = [owner window];
                if(window) {
                    WindowTracker::activateWindow(window);
                    DockItem *di =
                        g_dock->findDockItemForMinimizedWindow(window);
                    if(di)
                        g_dock->clearRunningLabel((__bridge void *)di);
                }

                // If Filer with no windows, fall through & launch folder
                if(![[owner bundleIdentifier] isEqualToString:@"com.ravynos.Filer"] ||
                    [[owner windows] count] > 1) {
                    e->accept();
                    break;
                }
            }
            LSLaunchURLSpec spec = { 0 };
            spec.appURL = (__bridge CFURLRef)[NSURL fileURLWithPath:
                [owner path]];
            if([[owner bundleIdentifier]
                isEqualToString:@"com.ravynos.Filer"]) // Filer is special
                spec.itemURLs = (__bridge CFArrayRef)[NSArray arrayWithObject:
                    [NSURL fileURLWithPath:
                    [[[NSUserDefaults standardUserDefaults]
                    stringForKey:INFOKEY_FILER_DEF_FOLDER]
                    stringByStandardizingPath]]];
            LSOpenFromURLSpec(&spec, NULL);
            [owner setNeedsAttention:YES]; // bouncy bouncy
            e->accept();
            break;
        }
        default: break;
    }
}

void DIWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    NSLog(@"Mouse DoubleClick %@", _owner);
    e->accept();
}

void DIWidget::mouseMoveEvent(QMouseEvent *e)
{
    NSLog(@"Mouse Move %@", _owner);
    e->accept();
}


@implementation DockItem

+dockItemWithPath:(NSString *)path {
    return [[DockItem alloc] initWithPath:path];
}

+dockItemWithWindow:(unsigned int)window path:(const char *)path {
    return [[DockItem alloc] initWithWindow:window path:path];
}

+dockItemWithMinimizedWindow:(unsigned int)window {
    return [[DockItem alloc] initWithMinimizedWindow:window];
}

/* We can't rely on g_dock->iconSize because DockItems are constructed
 * from within the Dock constructor. Boooo.
 */
+(int)iconSize {
    NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
    NSString *s = [prefs stringForKey:INFOKEY_CUR_SIZE];
    if(s) {
        NSSize sz = NSSizeFromString(s);
        return ((sz.width < sz.height) ? sz.width : sz.height) - 16;
    }
    return 64;
}

-initWithPath:(NSString *)path {
    _path = path;
    _label = nil;
    _type = DIT_INVALID;
    _icon = NULL;
    _bundleID = nil;

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
        QString iconPath(QString::fromUtf8(
            [[NSString stringWithFormat:@"%@/Resources/%@",path,iconFile]
            UTF8String]));
        NSLog(@"initWithPath %@ iconPath %s",_path, iconPath.toLocal8Bit().data());
        _icon = new QIcon(iconPath);
        NSLog(@"_icon created at %p", _icon);
        _bundleID = [b objectForInfoDictionaryKey:@"CFBundleIdentifier"];
    } else if(LSIsAppDir((__bridge CFURLRef)url)) {
        _type = DIT_APP_APPDIR;
        _execPath = [_path stringByAppendingPathComponent:@"AppRun"];
        _label = [[path lastPathComponent] stringByDeletingPathExtension];
        NSString *iconFile = [NSString stringWithFormat:@"%@/.DirIcon", path];
        if([[NSFileManager defaultManager] fileExistsAtPath:iconFile])
            _icon = new QIcon(QString::fromUtf8([iconFile UTF8String]));
    } else if([[path pathExtension] isEqualToString:@"desktop"]) {
        _type = DIT_APP_DESKTOP;

        // Simplified version of the LSAppRecord parser
        XdgDesktopFile df;
        if(df.load([path UTF8String]) && df.isValid()
            && df.type() == XdgDesktopFile::ApplicationType) {
            _label = [NSString stringWithUTF8String:
                df.name().toLocal8Bit().constData()];
            _execPath = [[[NSString stringWithCString:
                df.value("Exec").toString().toLocal8Bit().constData()]
                componentsSeparatedByString:@" "] firstObject];
            _icon = new QIcon(df.icon());
        }
    }

    if(_icon == NULL)
        _icon = new QIcon(QString::fromUtf8([[[NSBundle mainBundle]
            pathForResource:@"window" ofType:@"png"] UTF8String]));

    _flags = DIF_NORMAL;
    _pids = [NSMutableArray new];
    _windows = [NSMutableArray new];
    _widget = new DIWidget(self);
    int size = [DockItem iconSize];
    NSLog(@"size is %d",size);
    _widget->setPixmap(_icon->pixmap(size, size));
    _widget->setToolTip(QString::fromUtf8([_label UTF8String]));
    return self;
}

// FIXME: try to extract PID from _NET_WM_PID and identify bundle etc
// FIXME: set a standard icon
-initWithWindow:(unsigned int)window path:(const char *)path {
    _path = [NSString stringWithCString:path];
    _execPath = [_path copy];
    _label = [[_path copy] lastPathComponent];
    _type = DIT_APP_X11;

    _icon = new QIcon(QString::fromUtf8([[[NSBundle mainBundle]
        pathForResource:@"window" ofType:@"png"] UTF8String]));

    _bundleID = nil;
    _flags = DIF_NORMAL;
    _pids = [NSMutableArray new];
    _windows = [NSMutableArray new];
    [_windows addObject:[NSNumber numberWithInteger:window]];
    _widget = new DIWidget(self);
    int size = [DockItem iconSize];
    _widget->setPixmap(_icon->pixmap(size, size));
    _widget->setToolTip(QString::fromUtf8([_label UTF8String]));
    return self;
}

-initWithMinimizedWindow:(unsigned int)window {
    _path = nil;
    _execPath = nil;
    _label = nil;
    _type = DIT_WINDOW;

    _icon = new QIcon(QString::fromUtf8([[[NSBundle mainBundle]
        pathForResource:@"window" ofType:@"png"] UTF8String]));

    _bundleID = nil;
    _flags = DIF_NORMAL;
    _pids = [NSMutableArray new];
    _windows = [NSMutableArray new];
    [_windows addObject:[NSNumber numberWithInteger:window]];
    _widget = new DIWidget(self);
    int size = [DockItem iconSize];
    _widget->setPixmap(_icon->pixmap(size, size));
    _widget->setToolTip(QString::fromUtf8([_label UTF8String]));
    return self;
}

-(void)dealloc {
    if(_icon)
        delete _icon;
    if(_runMarker)
        _runMarker->deleteLater();
    if(_widget)
        _widget->deleteLater();
//     [super dealloc];
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

-(QIcon *)icon {
    return _icon;
}

-(QLabel *)widget {
    return static_cast<QLabel *>(_widget);
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

-(void)addPID:(pid_t)pid {
    if(pid == 0)
        NSLog(@"addPID: pid of 0 is invalid");
    else {
        struct kevent e[1];

        for(int i = 0; i < [_pids count]; ++i) {
            if([[_pids objectAtIndex:i] intValue] == pid)
                return; // already have this PID
        }
        [_pids addObject:[NSNumber numberWithInteger:pid]];
        EV_SET(e, pid, EVFILT_PROC, EV_ADD, NOTE_FORK|NOTE_EXEC|NOTE_TRACK|NOTE_EXIT, 0, (__bridge void *)self);

        // wake up kqueue to insert the event
        write(piper(1), e, sizeof(struct kevent));
    }
}

-(void)removePID:(pid_t)pid {
    if(pid == 0)
        NSLog(@"removePID: pid of 0 is invalid");
    else {
        struct kevent e[1];

        for(int i = 0; i < [_pids count]; ++i) {
            if([[_pids objectAtIndex:i] intValue] == pid) {
                [_pids removeObjectAtIndex:i];
                EV_SET(e, pid, EVFILT_PROC, EV_DELETE, 0, 0, (__bridge void *)self);

                // wake up kqueue to insert the event
                write(piper(1), e, sizeof(struct kevent));
                return;
            }
        }
        NSDebugLog(@"removePID: pid %d not found", pid);
    }
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

-(void)setRunningMarker:(QLabel *)label {
    if(_runMarker)
        _runMarker->deleteLater();
    _runMarker = label;
}

-(QLabel *)_getRunMarker {
    return _runMarker;
}

-(void)setLabel:(const char *)label {
    _label = [NSString stringWithUTF8String:label];
    _widget->setToolTip(label);
}

-(void)setIcon:(QIcon)icon {
    if(_icon)
        delete _icon;
    _icon = new QIcon(icon);
    int size = [DockItem iconSize];
    _widget->setPixmap(_icon->pixmap(size, size));
}

-(void)resize:(int)size {
    _widget->setPixmap(_icon->pixmap(size, size));
}

-(NSString *)description {
    return [NSString stringWithFormat:
    @"<%@ 0x%p> path:%@ exec:%@ label:%@ pid:%@ windows:%@ flags:0x%02x id:%@",
    [self class], self, _path, _execPath, _label, _pids, _windows, _flags,
    _bundleID];
}
@end

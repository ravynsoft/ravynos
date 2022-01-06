/*
 * airyxOS Application Launcher & Status Bar
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
#import <LaunchServices/LaunchServices.h>
#include <XdgDesktopFile>

extern unsigned long eventID();
extern int piper(int n);

@implementation DockItem

+dockItemWithPath:(NSString *)path {
    return [[self alloc] initWithPath:path];
}

-initWithPath:(NSString *)path {
    _path = path;
    _label = nil;
    _type = DIT_INVALID;
    _icon = NULL;
    _bundleID = nil;

    NSURL *url = [NSURL fileURLWithPath:path];
    if(LSIsNSBundle((CFURLRef)url)) {
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
        _icon = new QIcon(iconPath);
        _bundleID = [b objectForInfoDictionaryKey:@"CFBundleIdentifier"];
    } else if(LSIsAppDir((CFURLRef)url)) {
        _type = DIT_APP_APPDIR;
        _execPath = [_path stringByAppendingPathComponent:@"AppRun"];
        _label = [[path lastPathComponent] stringByDeletingPathExtension];
        NSString *iconFile = [NSString stringWithFormat:@"%@/.DirIcon", path];
        if([[NSFileManager defaultManager] fileExistsAtPath:iconFile])
            _icon = new QIcon(QString::fromUtf8([iconFile UTF8String]));
    } else if([[path pathExtension] isEqualToString:@"desktop"]) {
        _type = DIT_APP_DESKTOP;

        // Simplified version of the LSAppRecord parser. Desktop files must
        // contain an absolute Exec= path and Icon to go on the Dock
        XdgDesktopFile df;
        if(df.load([path UTF8String]) && df.isValid()
            && df.type() == XdgDesktopFile::ApplicationType) {
            _label = [NSString stringWithCString:
                df.name().toLocal8Bit().constData()];
            _execPath = [[[NSString stringWithCString:
                df.value("Exec").toString().toLocal8Bit().constData()]
                componentsSeparatedByString:@" "] firstObject];
            _icon = new QIcon(df.icon());
        }
    }

    _flags = DIF_NORMAL;
    _pid = 0;
    return self;
}

-(void)dealloc {
    if(_icon)
        delete _icon;
    if(_runMarker)
        delete _runMarker;
    [super dealloc];
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
    return (_flags == DIF_NORMAL);
}

-(BOOL)isLocked {
    return (_flags & DIF_LOCKED);
}

-(BOOL)isRunning {
    return (_pid != 0);
}

-(BOOL)isResident {
    return (_flags & DIF_RESIDENT);
}

-(BOOL)needsAttention {
    return (_flags & DIF_ATTENTION);
}

-(pid_t)pid {
    return _pid;
}

-(QIcon *)icon {
    return _icon;
}

// YES if equal to either item path
// YES if bundle and path is within bundle
// NO otherwise
-(BOOL)hasPath:(NSString *)path {
    if([path isEqualToString:_execPath] || [path isEqualToString:_path])
        return YES;

    if(_type == DIT_APP_BUNDLE) {
        while(path && [path hasSuffix:@"app"] == NO)
            path = [path stringByDeletingLastPathComponent];
        if(!path)
            return NO;
        NSBundle *b = [NSBundle bundleWithModulePath:path];
        if([[b bundlePath] isEqualToString:_path])
            return YES;
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
    if(value)
        _flags |= DIF_LOCKED;
    else
        _flags &= ~DIF_LOCKED;
}

-(void)setRunning:(pid_t)pid {
    struct kevent e[1];

    // trying to stop already stopped process?
    if(!pid && !_pid)
        return;

    if(pid != 0) {
        EV_SET(e, pid, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, self);
    } else {
        EV_SET(e, _pid, EVFILT_PROC, EV_DELETE, NOTE_EXIT, 0, self);
    }
    _pid = pid;
    // wake up kqueue to insert the event
    write(piper(1), e, sizeof(struct kevent));
}

-(void)setResident:(BOOL)value {
    if(value)
        _flags |= DIF_RESIDENT;
    else
        _flags &= ~DIF_RESIDENT;
}

-(void)setNeedsAttention:(BOOL)value {
    if(value)
        _flags |= DIF_ATTENTION;
    else
        _flags &= ~DIF_ATTENTION;
}

-(void)setRunningMarker:(QLabel *)label {
    if(_runMarker)
        delete _runMarker;
    _runMarker = label;
}

-(QLayoutItem *)_getRunMarker {
    return (QLayoutItem *)_runMarker;
}

-(NSString *)description {
    return [NSString stringWithFormat:
        @"<%@ 0x%p> path:%@ exec:%@ label:%@ pid:%u flags:0x%02x id:%@",
        [self class], self, _path, _execPath, _label, _pid, _flags, _bundleID];
}
@end

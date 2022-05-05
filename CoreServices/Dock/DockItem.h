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

#import <Foundation/Foundation.h>
#import <QIcon>
#import <QLabel>

enum DockItemType : unsigned int {
    DIT_INVALID,
    DIT_APP_BUNDLE,     // item is NSBundle
    DIT_APP_DESKTOP,    // item is XDG desktop file
    DIT_APP_APPDIR,     // item is AppDir
    DIT_APP_X11,        // item is unknown app found by window event
    DIT_WINDOW,         // item is X11 window
    DIT_FOLDER,         // item is a folder shortcut
    DIT_MAX = DIT_FOLDER
};

enum DockItemFlags : unsigned int {
    DIF_NORMAL = 0x0,
    DIF_LOCKED = 0x1,       // can't modify or remove
    DIF_RESIDENT = 0x2,     // app is resident in dock
    DIF_ATTENTION = 0x4     // app wants attention
};

class DIWidget : public QLabel
{
public:
    DIWidget(NSObject *owner);
    ~DIWidget();

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

private:
    NSObject *_owner;
};

@interface DockItem : NSObject {
    NSString *_path;        // path of the item represented
    NSString *_execPath;    // actual executable for _path
    NSString *_bundleID;    // CFBundleIdentifier if one exists
    DockItemType _type;
    unsigned int _flags;
    NSMutableArray *_pids;  // PIDs, if running
    NSMutableArray *_windows; // Window IDs
    NSString *_label;       // displayed on hover
    QIcon *_icon;
    QLabel *_runMarker;
    DIWidget *_widget;
}

+(DockItem *)dockItemWithPath:(NSString *)path;
+(DockItem *)dockItemWithWindow:(unsigned int)window path:(const char *)path;
+(DockItem *)dockItemWithMinimizedWindow:(unsigned int)window;

-(DockItem *)initWithPath:(NSString *)path;
-(DockItem *)initWithWindow:(unsigned int)window path:(const char *)path;
-(DockItem *)initWithMinimizedWindow:(unsigned int)window;

-(NSString *)path;
-(NSString *)execPath;
-(NSString *)bundleIdentifier;
-(NSString *)label;
-(DockItemType)type;
-(DockItemFlags)flags;
-(BOOL)isNormal;
-(BOOL)isLocked;
-(BOOL)isRunning;
-(BOOL)isResident;
-(BOOL)needsAttention;
-(pid_t)pid;
-(NSArray *)pids;
-(unsigned int)window;
-(NSArray *)windows;
-(QIcon *)icon;
-(BOOL)hasPath:(NSString *)path;

-(QLabel *)widget;
-(QLabel *)_getRunMarker;

-(void)setFlags:(DockItemFlags)flags;
-(void)setNormal; // clears all flags
-(void)setLocked:(BOOL)value;
-(void)addPID:(pid_t)pid;
-(void)removePID:(pid_t)pid;
-(void)addWindow:(unsigned int)window;
-(void)removeWindow:(unsigned int)window;
-(void)setResident:(BOOL)value;
-(void)setNeedsAttention:(BOOL)value;
-(void)setRunningMarker:(QLabel *)label;
-(void)setLabel:(const char *)label;
-(void)setIcon:(QIcon)icon;
-(void)resize:(int)size;

@end

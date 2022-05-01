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

#import <Foundation/Foundation.h>
#import <LaunchServices/LaunchServices.h>

#include "Dock.h"
#include "WindowTracker.h"
#include "Utils.h"

#include <QPainterPath>
#include <QRegion>
#include <QWindow>
#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QMutex>
#include <QMutexLocker>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

QMutex mutex;

Dock::Dock()
    :QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
            Qt::WindowOverridesSystemGestures | Qt::WindowDoesNotAcceptFocus)
    ,m_currentSize(900,64)
{
    m_prefs = [NSUserDefaults standardUserDefaults];
    m_items = m_itemsPinned = m_itemsSpecial = nil;
    m_emptyItem = [DockItem new];

    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    m_iconRun = new QPixmap(QIcon(QString::fromUtf8(
        [[[NSBundle mainBundle] pathForResource:@"running" ofType:@"png"]
        UTF8String])).pixmap(8, 8));

    NSString *s = [m_prefs stringForKey:INFOKEY_CUR_SIZE];
    if(s) {
        NSSize sz = NSSizeFromString(s);
        m_currentSize.setWidth(sz.width);
        m_currentSize.setHeight(sz.height);
    }

    int pos = [m_prefs integerForKey:INFOKEY_LOCATION];
    switch(pos) {
        case LOCATION_LEFT:
        case LOCATION_RIGHT:
            m_location = (Location)pos;
            break;
        default:
            m_location = LOCATION_BOTTOM;
    }

    m_box = new QBoxLayout(m_location == LOCATION_BOTTOM
        ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom, this);
    m_cells = new QGridLayout();
    m_cellsPinned = new QGridLayout();
    m_cellsSpecial = new QGridLayout();

    m_divider = makeDivider();
    m_divider2 = makeDivider();

    m_box->addLayout(m_cellsPinned);
    m_box->addWidget(m_divider);
    m_box->addLayout(m_cells);
    m_box->setStretchFactor(m_cells, 10);
    m_box->addWidget(m_divider2);
    m_box->addLayout(m_cellsSpecial);

    m_box->setSpacing(4);
    m_box->setContentsMargins(0, 0, 0, 0);

    this->relocate();
    QPalette pal;
    pal.setColor(QPalette::Window, QColor(0x66,0x66,0x66,0xff));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->setWindowOpacity(0.92);

    Display *display = XOpenDisplay(":0");
    Atom wintype = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", True);

    XChangeProperty(display, this->winId(),
        XInternAtom(display, "_NET_WM_WINDOW_TYPE", True),
        XA_ATOM, 32, PropModeReplace,
        (const unsigned char *)&wintype, 1L);
    XFlush(display);

    connect(this, &Dock::itemShouldClearIndicator, this,
        &Dock::clearRunningLabel);
    connect(this, &Dock::itemShouldSetIndicator, this,
        &Dock::setRunningLabel);
    connect(this, &Dock::dockShouldAddNonResident, this,
        &Dock::addNonResident);

    connect(m_screen, &QScreen::availableGeometryChanged, this,
        &Dock::relocate);

    this->loadItems();
    this->loadProcessTable();

    // Track process launches from Dock and per-user launchd
    struct kevent e[3];
    EV_SET(&e[0], getpid(), EVFILT_PROC, EV_ADD, NOTE_FORK|NOTE_EXEC|NOTE_TRACK, 0, (__bridge void *)nil);
    EV_SET(&e[1], getppid(), EVFILT_PROC, EV_ADD, NOTE_FORK|NOTE_EXEC|NOTE_TRACK, 0, (__bridge void *)nil);

    // Wake up kq when this descriptor has data to read
    EV_SET(&e[2], piper(0), EVFILT_READ, EV_ADD, 0, 0, NULL);

    kevent(kqPIDs, e, 3, NULL, 0, NULL);
}

Dock::~Dock()
{
    delete m_cells;
    delete m_iconRun;
//     [m_prefs release];
//     [m_items release];
}

// Helpers to emit signals which is needed because our kq loop
// is not part of the class. It can't 'emit' on its own.
void Dock::emitStarted(void *di)
{
    emit itemShouldSetIndicator(di);
}

void Dock::emitStopped(void *di)
{
    emit itemShouldClearIndicator(di);
}

void Dock::emitAddNonResident(unsigned int pid, const char *path)
{
    char *copyPath = strdup(path);
    emit dockShouldAddNonResident(pid, copyPath);
}

void Dock::addNonResident(unsigned int pid, const char *path)
{

    NSLog(@"addNonRes %u %s",pid,path);

    if(!path)
        return;

    NSString *nsPath = [NSString stringWithUTF8String:path];
    free((void *)path);

    // We can be invoked multiple times by the kq thread due to fork/exec
    // The first time should create the item and the rest should only
    // add PIDs.
    DockItem *di = findDockItemForPath((char *)[nsPath UTF8String]);
    if(di != nil) {
        [di addPID:pid];
        NSDebugLog(@"addNonResident: adding PID %u to %@", pid, [di label]);
        return;
    }

    QMutexLocker locker(&mutex);
    // not found so it must truly be a new item
    NSLog(@"making new di\n");
    di = [DockItem dockItemWithPath:nsPath];
    if([di type] == DIT_INVALID) {
        // Not a bundle or AppDir so handle as Window
        NSDebugLog(@"addNonResident: %lu %@ is not bundle or AppDir",
            pid, nsPath);
        return;
    }

    fprintf(stderr, "checking bundle id\n");
    // never show icons for my own services, which can happen because
    // of weird fork() scenarios like fail to exec()
    if([[di bundleIdentifier] isEqualToString:@"com.ravynos.Dock"] ||
       [[di bundleIdentifier] hasPrefix:@"com.ravynos.Dock."]) {
       fprintf(stderr, "early return for Dock component\n");
        return;
    }

    fprintf(stderr, "adding as non-res item di=%p\n", di);
    _addNonResident(di);
}

// This is split out so it can be called by above (add by PID) or by
// WindowTracker for DIs created by window.
void Dock::_addNonResident(DockItem *di)
{
    [m_items addObject:di];
    if(!adjustSize()) {
        NSLog(@"addNonResident: too many items!");
        // FIXME: should we just make icons smaller and pack them in?
        [m_items removeObjectIdenticalTo:di];
        return;
    }

    if(m_location == LOCATION_BOTTOM)
        m_cells->addWidget([di widget],0,m_cells->count(),Qt::AlignCenter);
    else
        m_cells->addWidget([di widget],m_cells->count(),0,Qt::AlignCenter);

    setRunningLabel((__bridge void *)di);
}


void Dock::loadItems()
{
    int size = iconSize();
    NSArray *pinnedItems = [m_prefs objectForKey:INFOKEY_CUR_ITEMS];

    if(m_itemsPinned == nil)
        m_itemsPinned = [NSMutableArray arrayWithCapacity:[pinnedItems count]];

    if(m_itemsSpecial == nil)
        m_itemsSpecial = [NSMutableArray arrayWithCapacity:2];

    if(m_items == nil)
        m_items = [NSMutableArray arrayWithCapacity:3];

    int items = [pinnedItems count] + 3; // 3 special items
    int length = size * items + CELL_SPACER*items + CELL_SPACER*2;

    setLength(length);
    this->relocate();

    // Relocate caps our length to the screen size if needed. If length
    // has changed, recalculate how many items we can display
    int curlength = currentLength();
    if(length != curlength)
        items = curlength / size;

    NSMutableArray *apps = [NSMutableArray new];
    [apps addObject:@"/System/Library/CoreServices/Filer.app"];
    NSEnumerator *currentItems = [pinnedItems objectEnumerator];
    NSString *cur;
    while(cur = [currentItems nextObject])
        [apps addObject:cur];

    for(int x = 0; x < [apps count]; ++x) {
        if(x > items) {
            NSLog(@"Too many items!");
            break;
        }

        NSString *s = [apps objectAtIndex:x];
        DockItem *info = [DockItem dockItemWithPath:s];

        if(x == 0)
            [info setLocked:YES];
        [info setResident:YES];
        [m_itemsPinned addObject:info];

        if(m_location == LOCATION_BOTTOM)
            m_cellsPinned->addWidget([info widget], 0, x, Qt::AlignCenter);
        else
            m_cellsPinned->addWidget([info widget], x, 0, Qt::AlignCenter);
    }

    NSString *specials[] = {@"Downloads",@"Trash"};
    for(int x = 0; x < 2; ++x) {
        NSString *bundle = [[NSBundle mainBundle]
            pathForResource:specials[x] ofType:@"app"];
        DockItem *it = [DockItem dockItemWithPath:bundle];
        [it setResident:YES];
        [it setLocked:YES];
        [m_itemsSpecial addObject:it];

        if(m_location == LOCATION_BOTTOM)
            m_cellsSpecial->addWidget([it widget], 0, x, Qt::AlignCenter);
        else
            m_cellsSpecial->addWidget([it widget], x, 0, Qt::AlignCenter);
    }
}

// TODO: connect primaryScreenChanged signal to this
void Dock::relocate()
{
    m_screen = QGuiApplication::primaryScreen();
    QRect geom(m_screen->availableGeometry());

    int edgeGap = 0;
    int ypos, xpos;

    if(m_location == LOCATION_BOTTOM) {
        edgeGap = 8;
        m_maxLength = geom.right() - geom.left() - 16;

        // currently taller than wide? swap width & height for new layout
        if(m_currentSize.width() < m_currentSize.height())
            swapWH();
        capLength();

        ypos = geom.bottom() - edgeGap - m_currentSize.height();
        xpos = geom.center().x() - (m_currentSize.width() / 2);

        m_divider->setFrameShape(QFrame::VLine);
        m_divider2->setFrameShape(QFrame::VLine);
        m_divider->setContentsMargins(0, DIVIDER_MARGIN, 0, DIVIDER_MARGIN);
        m_divider->setContentsMargins(0, DIVIDER_MARGIN, 0, DIVIDER_MARGIN);
    } else {
        // available geom should already exclude menu bar
        m_maxLength = geom.bottom() - geom.top() - 16;

        // wider than tall? swap width & height for new layout
        if(m_currentSize.width() > m_currentSize.height())
            swapWH();
        capLength();

        ypos = geom.center().y() - edgeGap - (m_currentSize.height() / 2);
        
        if(m_location == LOCATION_LEFT)
            xpos = edgeGap;
        else
            xpos = geom.right() - edgeGap - m_currentSize.width();

        m_divider->setFrameShape(QFrame::HLine);
        m_divider2->setFrameShape(QFrame::HLine);
        m_divider->setContentsMargins(DIVIDER_MARGIN, 0, DIVIDER_MARGIN, 0);
        m_divider->setContentsMargins(DIVIDER_MARGIN, 0, DIVIDER_MARGIN, 0);
    }

    // Get in position and update size
    this->move(xpos, ypos);
    this->setMinimumSize(0, 0);
    this->setMaximumSize(m_currentSize);
    this->resize(m_currentSize);

    // Round our corners
    QPainterPath roundy;
    roundy.addRoundedRect(this->rect(), RADIUS, RADIUS);
    QRegion mask = QRegion(roundy.toFillPolygon().toPolygon());
    this->clearMask();
    this->setMask(mask);

    this->savePrefs();
    this->show();
}

void Dock::mousePressEvent(QMouseEvent *e)
{
    NSDebugLog(@"Dock::mousePressEvent - ignored");
}

void Dock::mouseReleaseEvent(QMouseEvent *e)
{
    NSDebugLog(@"Dock::mouseReleaseEvent - ignored");
}

DockItem *Dock::findDockItemForPath(char *path)
{
    NSString *s = [NSString stringWithUTF8String:path];

    QMutexLocker locker(&mutex);

    // Check pinned items
    for(int i = 0; i < [m_itemsPinned count]; ++i) {
        DockItem *di = [m_itemsPinned objectAtIndex:i];
        if([di hasPath:s])
            return di;
    }

    // Not pinned. Check non-resident items.
    for(int i = 0; i < [m_items count]; ++i) {
        DockItem *di = [m_items objectAtIndex:i];
        if([di hasPath:s])
            return di;
    }

    return nil;
}

DockItem *Dock::findDockItemForMinimizedWindow(unsigned int window)
{
    QMutexLocker locker(&mutex);

    // No need to check Pinned items; window icons are always non-resident
    for(int i = 0; i < [m_items count]; ++i) {
        DockItem *di = [m_items objectAtIndex:i];
        if([di type] == DIT_WINDOW && [di window] == window)
            return di;
    }
    return nil;
}

void Dock::removeWindowFromAll(unsigned int window)
{
    for(int i = 0; i < [m_itemsPinned count]; ++i)
        [[m_itemsPinned objectAtIndex:i] removeWindow:window];

    for(int i = 0; i < [m_items count]; ++i) {
        DockItem *di = [m_items objectAtIndex:i];
        if([di type] != DIT_WINDOW)
            [di removeWindow:window];
    }
}


// At startup, load a snapshot of running processes to init status indicators
// After startup, we track launch/exits in other ways
void Dock::loadProcessTable()
{
    NSMutableDictionary *processDict = [NSMutableDictionary
        dictionaryWithCapacity:200];

    NSFileManager *fm = [NSFileManager defaultManager];
    NSEnumerator *procs = [[fm directoryContentsAtPath:@"/proc"]
        objectEnumerator];

    NSString *spid;
    char buf[PATH_MAX];
    while((spid = [procs nextObject])) {
        int pid = [spid intValue];
        if(pid < 2)
            continue;

        // Read the actual path of the executable
        NSString *path = [NSString stringWithFormat:@"/proc/%@/file", spid];
        int len = readlink([path UTF8String], buf, PATH_MAX-1);
        if(len <= 0)
            continue;
        buf[len] = 0;

        NSString *name = [NSString stringWithUTF8String:buf];
        NSBundle *bundle = [NSBundle bundleWithModulePath:name];
        NSString *bname = [bundle bundlePath];

        if([processDict objectForKey:name] == nil) {
            NSMutableArray *pids = [NSMutableArray new];
            [pids addObject:[NSNumber numberWithInteger:pid]];
            [processDict setObject:pids forKey:name];
        } else {
            NSMutableArray *pids = [processDict objectForKey:name];
            [pids addObject:[NSNumber numberWithInteger:pid]];
        }

        if(![bname isEqualToString:@"/"] && ![bname isEqualToString:name]) {
            if([processDict objectForKey:bname] == nil) {
                NSMutableArray *pids = [NSMutableArray new];
                [pids addObject:[NSNumber numberWithInteger:pid]];
                [processDict setObject:pids forKey:bname];
            } else {
                NSMutableArray *pids = [processDict objectForKey:bname];
                [pids addObject:[NSNumber numberWithInteger:pid]];
            }
        }
    }

    // FIXME: also create items for any non-resident running bundles,
    // even if they have no windows

    for(int i = 0; i < [m_itemsPinned count]; ++i) {
        DockItem *di = [m_itemsPinned objectAtIndex:i];
        pid_t pid = 0;
        NSArray *pids = [processDict objectForKey:[di execPath]];
        if(pids)
            for(int j = 0; j < [pids count]; ++j)
                [di addPID:[[pids objectAtIndex:j] intValue]];

        pids = [processDict objectForKey:[di path]];
        if(pids)
            for(int j = 0; j < [pids count]; ++j)
                [di addPID:[[pids objectAtIndex:j] intValue]];

        [di isRunning] ? setRunningLabel((__bridge void *)di) : clearRunningLabel((__bridge void *)di);
    }
}

// FIXME: refactor into DockItem
void Dock::setRunningLabel(void *di)
{
    DockItem *odi = (__bridge DockItem *)di;
    if([odi _getRunMarker] != NULL || [odi type] == DIT_WINDOW)
        return;

    QLabel *l = new QLabel;
    l->setPixmap(*m_iconRun);
    [odi setRunningMarker:l]; // save this so we can delete later

    int i;
    QGridLayout *cells = ([odi isResident] ? m_cellsPinned : m_cells);

    for(i = 0; i < cells->count(); ++i) {
        QLayoutItem *layout = cells->itemAt(i);
        if(layout && layout->widget() == [odi widget]) {
            switch(m_location) {
                case LOCATION_RIGHT:
                case LOCATION_LEFT:
                    cells->addWidget(l, i, 0,
                        Qt::AlignVCenter |
                        (m_location == LOCATION_RIGHT
                        ? Qt::AlignRight : Qt::AlignLeft));
                    break;
                default:
                    cells->addWidget(l, 0, i,
                        Qt::AlignHCenter | Qt::AlignBottom);
            }
            break;
        }
    }

    if(i > cells->count())
        NSLog(@"setRunningLabel: item %@ not found in layout", [odi label]);
}

// FIXME: refactor into DockItem
void Dock::clearRunningLabel(void *item)
{
    DockItem *di = (__bridge DockItem *)item;
    QLabel *marker = [di _getRunMarker];
    m_cells->removeWidget(marker);
    [di setRunningMarker:NULL];

    QWidget *w = (QWidget *)[di widget];

    if(![di isResident] && ![di isLocked] && w) {
        m_cells->removeWidget(w);
        w->deleteLater();

        [m_items removeObjectIdenticalTo:di];
//         [di release];
        adjustSize();
    }
}

#include "Dock.moc"

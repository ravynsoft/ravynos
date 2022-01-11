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

#include <X11/Xlib.h>
#include <X11/Xatom.h>

Dock::Dock()
    :QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
            Qt::WindowOverridesSystemGestures | Qt::WindowDoesNotAcceptFocus)
    ,m_currentSize(900,64)
    ,m_itemSlots(0)
{
    m_prefs = [[NSUserDefaults standardUserDefaults] retain];
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
    EV_SET(&e[0], getpid(), EVFILT_PROC, EV_ADD, NOTE_FORK|NOTE_EXEC|NOTE_TRACK, 0, nil);
    EV_SET(&e[1], getppid(), EVFILT_PROC, EV_ADD, NOTE_FORK|NOTE_EXEC|NOTE_TRACK, 0, nil);

    // Wake up kq when this descriptor has data to read
    EV_SET(&e[2], piper(0), EVFILT_READ, EV_ADD, 0, 0, NULL);

    kevent(kqPIDs, e, 3, NULL, 0, NULL);
}

Dock::~Dock()
{
    delete m_cells;
    delete m_iconRun;
    [m_prefs release];
    [m_items release];
}

// Helper to emit signals which is needed because our kq loop
// is not part of the class. It can't 'emit' on its own.
void Dock::emitSignal(int i, void *di)
{
    if(di)
        emit itemShouldClearIndicator(di);
    else
        emit itemShouldSetIndicator(i);
}

void Dock::emitAddNonResident(unsigned int pid, const char *path)
{
    emit dockShouldAddNonResident(pid, path);
}

void Dock::addNonResident(unsigned int pid, const char *path)
{
    // We can be invoked multiple times by the kq thread due to fork/exec
    // The first time should create the item and the rest should only
    // add PIDs.
    DockItem *di = findDockItemForPath((char *)path);
    if(di != nil) {
        [di addPID:pid];
        NSDebugLog(@"addNonResident: adding PID %u to %@", pid, [di label]);
        return;
    }

    // not found so it must truly be a new item
    di = [DockItem dockItemWithPath:[NSString stringWithUTF8String:path]];
    if([di type] == DIT_INVALID) {
        // Not a bundle or AppDir so handle as Window
        NSDebugLog(@"addNonResident: %lu %s is not bundle or AppDir",
            pid, path);
        return;
    }

    // never show icons for my own services, which can happen because
    // of weird fork() scenarios like fail to exec()
    if([[di bundleIdentifier] isEqualToString:@"org.airyx.Dock"] ||
       [[di bundleIdentifier] isEqualToString:@"org.airyx.Dock.Trash"])
        return;

    _addNonResident(di);
}

// This is split out so it can be called by above (add by PID) or by
// WindowTracker for DIs created by window.
void Dock::_addNonResident(DockItem *di)
{
    int size = (m_location == LOCATION_BOTTOM
        ? m_currentSize.height() : m_currentSize.width()) - 16;

    int index;
    for(index = 0; index < [m_items count]; ++index) {
        if([[m_items objectAtIndex:index] isEqual:m_emptyItem]) {
            [m_items replaceObjectAtIndex:index withObject:di];
            break;
        }
    }

    if(index >= [m_items count]) {
        // All slots are full. Can we expand Dock?
        bool expanded = addSlot();
        if(expanded)
            [m_items addObject:di];
        else {
            NSLog(@"addNonResident: too many items!");
            // FIXME: should we just make icons smaller and pack them in?
            return;
        }
    }

    NSDebugLog(@"addNonResident at %u", index);

    if(m_location == LOCATION_BOTTOM)
        m_cells->addWidget([di widget], 0, index, Qt::AlignCenter);
    else
        m_cells->addWidget([di widget], index, 0, Qt::AlignCenter);

    setRunningLabel(index);
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

    int items = [pinnedItems count] + 6; // 3 transient & 3 special items
    int length = size * items;

    setLength(length);
    m_itemSlots = items;
    this->relocate();

    // Relocate caps our length to the screen size if needed. If length
    // has changed, recalculate how many items we can display
    int curlength = currentLength();
    if(length != curlength) {
        items = curlength / size;
        m_itemSlots = items;
    }

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
        [m_itemsPinned addObject:info]; // must keep in same order as cells!

        if(m_location == LOCATION_BOTTOM)
            m_cellsPinned->addWidget([info widget], 0, x, Qt::AlignCenter);
        else
            m_cellsPinned->addWidget([info widget], x, 0, Qt::AlignCenter);
    }

    // All the pinned items are done. Now create 3 placeholders for
    // non-resident apps or minimized windows. We expand this area as
    // needed.
    for(int x = 0; x < 3; ++x)
        [m_items addObject:m_emptyItem];

    NSString *dlapp = [[NSBundle mainBundle] pathForResource:@"Trash"
        ofType:@"app"]; // FIXME: Downloads
    DockItem *downloads = [DockItem dockItemWithPath:dlapp];
    [downloads setIcon:QIcon([[[NSBundle mainBundle] pathForResource:@"folder"
        ofType:@"png"] UTF8String])];
    [downloads setLabel:"Downloads"];
    [downloads setResident:YES];
    [downloads setLocked:YES];
    [m_itemsSpecial addObject:downloads];

    if(m_location == LOCATION_BOTTOM)
        m_cellsSpecial->addWidget([downloads widget], 0, 0, Qt::AlignCenter);
    else
        m_cellsSpecial->addWidget([downloads widget], 0, 0, Qt::AlignCenter);

    NSString *trashapp = [[NSBundle mainBundle] pathForResource:@"Trash"
        ofType:@"app"];
    DockItem *trashitem = [DockItem dockItemWithPath:trashapp];
    [trashitem setResident:YES];
    [trashitem setLocked:YES];
    [m_itemsSpecial addObject:trashitem];

    if(m_location == LOCATION_BOTTOM)
        m_cellsSpecial->addWidget([trashitem widget], 0, 1, Qt::AlignCenter);
    else
        m_cellsSpecial->addWidget([trashitem widget], 1, 0, Qt::AlignCenter);
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

int Dock::itemFromPos(int x, int y)
{
    int size = (m_location == LOCATION_BOTTOM)
        ? m_currentSize.width() : m_currentSize.height();
    int cellsize = size / m_itemSlots;

    int item = -1;
    if(m_location == LOCATION_BOTTOM)
        item = x / cellsize;
    else
        item = y / cellsize;
    return item;
}

void Dock::mousePressEvent(QMouseEvent *e)
{
    int itempos = itemFromPos(e->x(), e->y());
    DockItem *item = [m_items objectAtIndex:itempos];
    if(!item)
        return;

    switch(e->button()) {
        case Qt::LeftButton: // logical left, the primary action
            NSDebugLog(@"click %d, start long-press timer for menu", itempos);
            break;
        case Qt::RightButton: // logical right, the secondary action
            NSDebugLog(@"right click %d, show the menu now",itempos);
            break;
        default: break;
    }
}

void Dock::mouseReleaseEvent(QMouseEvent *e)
{
    int itempos = itemFromPos(e->x(), e->y());
    DockItem *item = [m_items objectAtIndex:itempos];
    if(!item)
        return;

    switch(e->button()) {
        case Qt::LeftButton: // logical left, the primary action
        {
            if([item type] == DIT_WINDOW) { // unminimizing?
                unsigned int window = [item window];
                WindowTracker::activateWindow(window);
                clearRunningLabel(item);
                break;
            }
            if([item isRunning]) { // clicked a running app?
                unsigned int window = [item window];
                if(window) {
                    WindowTracker::activateWindow(window);
                    DockItem *winDI = findDockItemForMinimizedWindow(window);
                    if(winDI) {
                        clearRunningLabel(winDI);
                        [winDI release];
                    }
                }

                // If Filer with no windows, fall through & launch folder
                if(itempos != 0 || [[item windows] count] > 1)
                    break;
            }
            LSLaunchURLSpec spec = { 0 };
            spec.appURL = (CFURLRef)[NSURL fileURLWithPath:[item path]];
            if(itempos == 0) // Filer is special
                spec.itemURLs = (CFArrayRef)[NSArray arrayWithObject:
                    [NSURL fileURLWithPath:
                    [[m_prefs stringForKey:INFOKEY_FILER_DEF_FOLDER]
                    stringByStandardizingPath]]];
            LSOpenFromURLSpec(&spec, NULL);
            [item setNeedsAttention:YES]; // bouncy bouncy
            break;
        }
        default: break;
    }
}

void Dock::mouseDoubleClickEvent(QMouseEvent *e)
{
    NSDebugLog(@"mouseDouble");
}

void Dock::mouseMoveEvent(QMouseEvent *e)
{
    NSDebugLog(@"mouseMove - dragging item %d", itemFromPos(e->x(), e->y()));
}

DockItem *Dock::findDockItemForPath(char *path)
{
    NSString *s = [NSString stringWithUTF8String:path];

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
        [[m_items objectAtIndex:i] removeWindow:window];

    for(int i = 0; i < [m_items count]; ++i) {
        DockItem *di = [m_items objectAtIndex:i];
        if([di type] != DIT_WINDOW)
            [di removeWindow:window];
    }
}

int Dock::indexOfItem(DockItem *di)
{
    NSUInteger i = [m_items indexOfObjectIdenticalTo:di];
    if(i != NSNotFound)
        return (int)i;
    return -1;
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

        [di isRunning] ? setRunningLabel(i) : clearRunningLabel(di);
    }
}

void Dock::setRunningLabel(int i)
{
    DockItem *di = [m_items objectAtIndex:i];
    if([di _getRunMarker] != NULL || [di type] == DIT_WINDOW)
        return;

    QLabel *l = new QLabel;
    l->setPixmap(*m_iconRun);
    [di setRunningMarker:l]; // save this so we can delete later

    switch(m_location) {
        case LOCATION_LEFT:
            m_cells->addWidget(l, i, 0,
                Qt::AlignVCenter | Qt::AlignLeft);
            break;
        case LOCATION_RIGHT:
            m_cells->addWidget(l, i, 0,
                Qt::AlignVCenter | Qt::AlignRight);
            break;
        default:
            m_cells->addWidget(l, 0, i,
                Qt::AlignHCenter | Qt::AlignBottom);
            break;
    }
}

// FIXME: this needs to shift icons to keep display position aligned
// with m_items index or else clicking the displayed icon invokes the
// wrong item slot
void Dock::clearRunningLabel(void *item)
{
    DockItem *di = (DockItem *)item;
    QLabel *marker = [di _getRunMarker];
    m_cells->removeWidget(marker);
    [di setRunningMarker:NULL];

    if([di isResident] == NO && [di isLocked] == NO) {
        int index = [m_items indexOfObjectIdenticalTo:di];
        if(index < 0 || index > m_itemSlots)
            return;

        QLayoutItem *layout = m_cells->itemAtPosition(
            m_location == LOCATION_BOTTOM ? 0 : index,
            m_location == LOCATION_BOTTOM ? index : 0);

        if(layout == nullptr)
            return;

        QWidget *w = layout->widget();

        if(w == nullptr)
            return;

        NSDebugLog(@"Removing non-resident app %@ from slot %d",
            [di label], index);

        m_cells->removeWidget(w);
        w->deleteLater();

        [di release];
        [m_items replaceObjectAtIndex:index withObject:m_emptyItem];
    }
}

#include "Dock.moc"

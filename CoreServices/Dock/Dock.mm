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

#include "Dock.h"
#include <QPainterPath>
#include <QRegion>
#include <QWindow>
#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

Dock::Dock()
    :QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
            Qt::WindowOverridesSystemGestures | Qt::WindowDoesNotAcceptFocus)
    ,m_currentSize(900,64)
{
    m_prefs = [[NSUserDefaults standardUserDefaults] retain];
    NSString *s = [m_prefs stringForKey:INFOKEY_CUR_SIZE];
    if(s) {
        NSSize sz = NSSizeFromString(s);
        m_currentSize.setWidth(sz.width);
        m_currentSize.setHeight(sz.height);
    }

    int pos = [m_prefs integerForKey:INFOKEY_LOCATION];
    switch(pos) {
        LOCATION_RIGHT:
        LOCATION_LEFT:
            m_location = (Location)pos;
            break;
        default:
            m_location = LOCATION_BOTTOM;
    }

    Display *display = XOpenDisplay(":0");
    Atom wintype = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", True);

    XChangeProperty(display, this->winId(),
        XInternAtom(display, "_NET_WM_WINDOW_TYPE", True),
        XA_ATOM, 32, PropModeReplace,
        (const unsigned char *)&wintype, 1L);
    XFlush(display);

    this->relocate();
    this->setWindowOpacity(0.92);

    m_cells = new QGridLayout(this);
    m_cells->setHorizontalSpacing(CELL_SPACER);

    this->loadItems();
}

Dock::~Dock()
{
    if(m_cells)
        delete m_cells;

    if(m_prefs)
        [m_prefs release];
}

void Dock::loadItems()
{
    int size = (m_location == LOCATION_BOTTOM
        ? m_currentSize.height() : m_currentSize.width()) - 16;

    int items = (m_location == LOCATION_BOTTOM
        ? m_currentSize.width() : m_currentSize.height())
        / (size + CELL_SPACER + CELL_SPACER);

    // addItem( trash, items );
    --items;
    if(m_location == LOCATION_BOTTOM)
        m_cells->setColumnStretch(items, 100);
    else
        m_cells->setRowStretch(items, 100);
    --items;

    int item = 0;

    NSFileManager *fm = [NSFileManager defaultManager];
    NSMutableArray *apps = [NSMutableArray new];
    [apps addObject:@"/System/Library/CoreServices/Filer.app"];

    NSArray *apps2 = [fm directoryContentsAtPath:@"/Applications"];
    for(int x = 0; x < [apps2 count]; ++x) {
        NSString *s = [NSString stringWithFormat:@"/Applications/%@",
            [apps2 objectAtIndex:x]];
        if(![s hasSuffix:@"app"])
            continue;
        [apps addObject:s];
    }

    for(int x = 0; x < [apps count]; ++x) {
        NSString *s = [apps objectAtIndex:x];
        NSBundle *b = [NSBundle bundleWithPath:s];
        if(!b)
            continue;

        NSString *iconFile = [b objectForInfoDictionaryKey:
            @"CFBundleIconFile"];
        QString iconPath(QString::fromUtf8(
            [[NSString stringWithFormat:@"%@/Resources/%@",s,iconFile]
            UTF8String]));
        QIcon icon(iconPath);

        QLabel *iconLabel = new QLabel;
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        iconLabel->setMinimumSize(ICON_MIN, ICON_MIN);

        QPixmap pix(icon.pixmap(QSize(size, size)));
        iconLabel->setPixmap(pix);

        if(m_location == LOCATION_BOTTOM)
            m_cells->addWidget(iconLabel, 0, item++, Qt::AlignCenter);
        else
            m_cells->addWidget(iconLabel, item++, 0, Qt::AlignCenter);

        if(item > items) {
            // FIXME: do resize up to maximum
            NSLog(@"Too many items!");
            return;
        }
    }
}

void Dock::swapWH()
{
    int h = m_currentSize.height();
    m_currentSize.setHeight(m_currentSize.width());
    m_currentSize.setWidth(h);
}

void Dock::savePrefs()
{
    NSSize sz = NSMakeSize(m_currentSize.width(), m_currentSize.height());
    [m_prefs setObject:NSStringFromSize(sz) forKey:INFOKEY_CUR_SIZE];
    [m_prefs setInteger:m_location forKey:INFOKEY_LOCATION];
}

bool Dock::capLength()
{
    bool capped = false;

    switch(m_location) {
        case LOCATION_BOTTOM:
            if(m_currentSize.width() > m_maxLength) {
                m_currentSize.setWidth(m_maxLength);
                capped = true;
            }
            if(m_currentSize.width() < DOCK_LENGTH_MIN)
                m_currentSize.setWidth(DOCK_LENGTH_MIN);
            break;
        default:
            if(m_currentSize.height() > m_maxLength) {
                m_currentSize.setHeight(m_maxLength);
                capped = true;
            }
            if(m_currentSize.height() < DOCK_LENGTH_MIN)
                m_currentSize.setHeight(DOCK_LENGTH_MIN);
    }
    return capped;
}

// TODO: connect primaryScreenChanged signal to this
// TODO: connect availableGeometryChanged signal to this
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

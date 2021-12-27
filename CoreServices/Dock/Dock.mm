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
    ,m_location(LOCATION_BOTTOM)
    ,m_currentSize(900,64) // TODO: calculate this from icons
{
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
    delete m_cells;
}

void Dock::loadItems()
{
    int columns = m_currentSize.width() / (48 + CELL_SPACER + CELL_SPACER);
    // addItem( trash, column );
    --columns;
    m_cells->setColumnStretch(columns, 100);
    --columns;

    int column = 0;

    NSFileManager *fm = [NSFileManager defaultManager];
    NSArray *apps = [fm directoryContentsAtPath:@"/Applications"];

    for(int x = 0; x < [apps count]; ++x) {
        NSString *s = [NSString stringWithFormat:@"/Applications/%@",
            [apps objectAtIndex:x]];

        if(![s hasSuffix:@"app"])
            continue;
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
        iconLabel->setMinimumSize(48, 48);

        QPixmap pix(icon.pixmap(QSize(48, 48)));
        iconLabel->setPixmap(pix);

        m_cells->addWidget(iconLabel, 0, column++, Qt::AlignCenter);

        if(column > columns) {
            NSLog(@"Too many items!");
            return;
        }
    }
}


// TODO: connect primaryScreenChanged signal to this
// TODO: connect availableGeometryChanged signal to this
void Dock::relocate()
{
    m_screen = QGuiApplication::primaryScreen();
    QRect geom(m_screen->availableGeometry());

    int edgeGap = (m_location == LOCATION_BOTTOM) ? 8 : 0;

    int ypos = (geom.bottom() - edgeGap - m_currentSize.height());
    int xpos = (geom.center().x() - (m_currentSize.width() / 2));

    // Get in position and update size
    this->move(xpos, ypos);
    this->resize(m_currentSize);

    // Round our corners
    QPainterPath roundy;
    roundy.addRoundedRect(this->rect(), RADIUS, RADIUS);
    QRegion mask = QRegion(roundy.toFillPolygon().toPolygon());
    this->setMask(mask);

    this->show();
}

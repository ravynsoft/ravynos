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
#import <fcntl.h>
#import <unistd.h>

#import "Dock.h"
#import "Utils.h"

unsigned long eventID()
{
    static unsigned long _ID = 256000;
    return ++_ID;
}

int piper(int n)
{
    static int pipefd[2] = {-1};

    if(pipefd[0] == -1)
        pipe(pipefd);
    return pipefd[n];
}

int Dock::iconSize(void)
{
    // Figure out our size by taking the height to get an icon square.
    return (m_location == LOCATION_BOTTOM
        ? m_currentSize.height() : m_currentSize.width()) - 16;
}

void Dock::setLength(int length)
{
    if(m_location == LOCATION_BOTTOM)
        m_currentSize.setWidth(length);
    else
        m_currentSize.setHeight(length);
}

int Dock::currentLength(void)
{
    return (m_location == LOCATION_BOTTOM)
        ? m_currentSize.width()
        : m_currentSize.height();
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
    if([m_prefs stringForKey:INFOKEY_FILER_DEF_FOLDER] == nil)
        [m_prefs setObject:@"~" forKey:INFOKEY_FILER_DEF_FOLDER];
    [m_prefs synchronize];
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

bool Dock::adjustSize()
{
    int count =[m_itemsPinned count] + [m_itemsSpecial count] +
        [m_items count];
    int length = iconSize()*count + CELL_SPACER*count + CELL_SPACER*2;
    setLength(length);

    relocate();
    return true;
}

QFrame *Dock::makeDivider()
{
    QFrame *f = new QFrame();
    f->setFrameShadow(QFrame::Sunken);
    f->setLineWidth(3);
    if(m_location == LOCATION_BOTTOM) {
        f->setFrameShape(QFrame::VLine);
        f->setContentsMargins(0, DIVIDER_MARGIN, 0, DIVIDER_MARGIN);
    } else {
        f->setFrameShape(QFrame::HLine);
        f->setContentsMargins(DIVIDER_MARGIN, 0, DIVIDER_MARGIN, 0);
    }
    return f;
}

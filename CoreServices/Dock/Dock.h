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

#include <QWidget>
#include <QScreen>
#include <QApplication>
#include <QGridLayout>

#define RADIUS 10      // rounded corner radius
#define CELL_SPACER 4  // pixels between grid cells
#define ICON_MIN 24
#define DOCK_HEIGHT_MAX 136
#define DOCK_HEIGHT_MIN 32
#define DOCK_LENGTH_MIN 300

const NSString *INFOKEY_CUR_SIZE = @"CurrentSize";
const NSString *INFOKEY_LOCATION = @"Location";

class Dock : public QWidget {
public:
    Dock();
    virtual ~Dock();

    enum Location {
        LOCATION_BOTTOM,
        LOCATION_LEFT,
        LOCATION_RIGHT
    };

    void relocate();    // Move self to preferred location & size
    void loadItems();   // Load the items we should display

private:
    void savePrefs(void);
    void swapWH(void);  // swap current width and height
    bool capLength(void); // cap size at max for screen. Ret true if capped

    NSUserDefaults *m_prefs;
    Location m_location;
    int m_maxLength;
    QScreen *m_screen;
    QSize m_currentSize;
    QGridLayout *m_cells;
};

/***************************************************************************
 *   Copyright (C) 2017 by Nathan Osman                                    *
 *   nathan@quickmediasolutions.com                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef TABBAR_H
#define TABBAR_H

#include <QSize>
#include <QTabBar>

class TabBar : public QTabBar
{
    Q_OBJECT

public:

    explicit TabBar(QWidget *parent);

    void setFixedWidth(bool fixedWidth);
    void setFixedWidthValue(int value);
    void updateWidth();

protected:

    QSize tabSizeHint(int index) const override;

private:

    bool mFixedWidth;
    int mFixedWidthValue;
};

#endif // TABBAR_H

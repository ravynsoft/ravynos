/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Joseph Wenninger <jowenn@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _OUTPUTWIDGET_H_
#define _OUTPUTWIDGET_H_

#include <QTabWidget>

class TextOutputWidget;
class DataOutputWidget;

class KateSQLOutputWidget : public QTabWidget
{
    Q_OBJECT

public:
    KateSQLOutputWidget(QWidget *parent);

    ~KateSQLOutputWidget() override;

    TextOutputWidget *textOutputWidget() const
    {
        return m_textOutputWidget;
    }
    DataOutputWidget *dataOutputWidget() const
    {
        return m_dataOutputWidget;
    }

private:
    TextOutputWidget *m_textOutputWidget;
    DataOutputWidget *m_dataOutputWidget;
};

#endif
// kate: space-indent on; indent-width 2; replace-tabs on;

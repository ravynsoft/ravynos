/***************************************************************************
 *   This file is part of Kate search plugin                               *
 *   SPDX-FileCopyrightText: 2014 Kåre Särs <kare.sars@iki.fi>                           *
 *                                                                         *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 ***************************************************************************/

#ifndef UrlInserter_H
#define UrlInserter_H

#include <QLineEdit>
#include <QToolButton>
#include <QUrl>
#include <QWidget>

class UrlInserter : public QWidget
{
    Q_OBJECT
public:
    UrlInserter(const QUrl &startUrl, QWidget *parent);
    QLineEdit *lineEdit()
    {
        return m_lineEdit;
    }
    void setReplace(bool replace);

public Q_SLOTS:
    void insertFolder();

private:
    QLineEdit *m_lineEdit;
    QToolButton *m_toolButton;
    QUrl m_startUrl;
    bool m_replace;
};

#endif

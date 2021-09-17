/*
    SPDX-FileCopyrightText: 2011 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef HTML_DELEGATE_H
#define HTML_DELEGATE_H

#include <QFont>
#include <QStyledItemDelegate>

class SPHtmlDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit SPHtmlDelegate(QObject *parent);
    ~SPHtmlDelegate() override;

    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setDisplayFont(const QFont &font)
    {
        m_font = font;
    }

private:
    QFont m_font;
};

#endif

/*
    SPDX-FileCopyrightText: 2011 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "htmldelegate.h"

#include <KLocalizedString>
#include <QAbstractTextDocumentLayout>
#include <QModelIndex>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextDocument>

// make list spacing resemble the default list spacing
// (which would not be the case with default QTextDocument margin)
static const int s_ItemMargin = 1;

SPHtmlDelegate::SPHtmlDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

SPHtmlDelegate::~SPHtmlDelegate()
{
}

void SPHtmlDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setDefaultFont(m_font);
    doc.setDocumentMargin(s_ItemMargin);
    doc.setHtml(index.data().toString());

    painter->save();
    options.text = QString(); // clear old text
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

    // draw area
    QRect clip = options.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &options);
    if (index.flags() == Qt::NoItemFlags) {
        painter->setBrush(QBrush(QWidget().palette().color(QPalette::Base)));
        painter->setPen(QWidget().palette().color(QPalette::Base));
        painter->drawRect(QRect(clip.topLeft() - QPoint(20, 0), clip.bottomRight()));
        painter->translate(clip.topLeft() - QPoint(20, 0));
    } else {
        painter->translate(clip.topLeft() - QPoint(0, 0));
    }
    QAbstractTextDocumentLayout::PaintContext pcontext;
    pcontext.palette.setColor(QPalette::Text, options.palette.text().color());
    doc.documentLayout()->draw(painter, pcontext);

    painter->restore();
}

QSize SPHtmlDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex &index) const
{
    QTextDocument doc;
    doc.setDefaultFont(m_font);
    doc.setDocumentMargin(s_ItemMargin);
    doc.setHtml(index.data().toString());
    // qDebug() << doc.toPlainText() << doc.size().toSize();
    return doc.size().toSize() + QSize(30, 0); // add margin for the check-box
}

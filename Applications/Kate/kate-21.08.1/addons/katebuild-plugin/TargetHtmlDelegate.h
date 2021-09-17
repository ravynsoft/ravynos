/***************************************************************************
 *   This file is part of Kate build plugin                                *
 *   SPDX-FileCopyrightText: 2014 Kåre Särs <kare.sars@iki.fi>                           *
 *                                                                         *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 ***************************************************************************/

#ifndef TargetHtmlDelegate_H
#define TargetHtmlDelegate_H

#include <QStyledItemDelegate>

class TargetHtmlDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TargetHtmlDelegate(QObject *parent);
    ~TargetHtmlDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    bool isEditing() const;

public Q_SLOTS:
    void editStarted();
    void editEnded();

Q_SIGNALS:
    void sendEditStart() const;

private:
    bool m_isEditing;
};

#endif

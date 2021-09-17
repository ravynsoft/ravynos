/***************************************************************************
 *   This file is part of Kate build plugin                                *
 *   SPDX-FileCopyrightText: 2014 Kåre Särs <kare.sars@iki.fi>                           *
 *                                                                         *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 ***************************************************************************/
#ifndef SelectTargetView_H
#define SelectTargetView_H

#include "ui_SelectTargetUi.h"
#include <QAbstractItemModel>

class TargetFilterProxyModel;

class SelectTargetView : public QDialog, public Ui::SelectTargetUi
{
    Q_OBJECT
public:
    SelectTargetView(QAbstractItemModel *model, QWidget *parent = nullptr);
    const QModelIndex currentIndex() const;

    void setCurrentIndex(const QModelIndex &index);

public Q_SLOTS:
    void setFilter(const QString &filter);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    TargetFilterProxyModel *m_proxyModel;
};

#endif

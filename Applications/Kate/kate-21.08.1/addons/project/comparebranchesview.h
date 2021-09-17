/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef COMAREBRANCHESVIEW_H
#define COMAREBRANCHESVIEW_H

#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QWidget>

#include "git/gitstatus.h"

class KateProjectPluginView;
class CompareBranchesView : public QWidget
{
    Q_OBJECT
public:
    explicit CompareBranchesView(QWidget *parent, const QString &gitPath, const QString fromB, const QString &toBr, QVector<GitUtils::StatusItem> items);
    void setPluginView(KateProjectPluginView *pv)
    {
        m_pluginView = pv;
    }

    Q_SIGNAL void backClicked();

private Q_SLOTS:
    void showDiff(const QModelIndex &idx);

private:
    QPushButton m_backBtn;
    QTreeView m_tree;
    QStandardItemModel m_model;
    QString m_gitDir;
    QString m_fromBr;
    QString m_toBr;
    KateProjectPluginView *m_pluginView;
};

#endif // COMAREBRANCHESVIEW_H

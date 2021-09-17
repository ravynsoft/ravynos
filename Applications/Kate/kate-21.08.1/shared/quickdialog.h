/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef QUICKDIALOG_H
#define QUICKDIALOG_H

#include <QLineEdit>
#include <QMenu>
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QTreeView>

class QAbstractItemModel;
namespace KTextEditor
{
class MainWindow;
}

class QuickDialog : public QMenu
{
    Q_OBJECT
public:
    QuickDialog(QWidget *parent, QWidget *mainWindow);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void updateViewGeometry();
    void clearLineEdit();

protected Q_SLOTS:
    virtual void slotReturnPressed() = 0;

protected:
    QTreeView m_treeView;
    QLineEdit m_lineEdit;

private:
    QPointer<QWidget> m_mainWindow;
};

#endif // QUICKDIALOG_H

/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef BRANCHES_DIALOG_H
#define BRANCHES_DIALOG_H

#include <QFutureWatcher>
#include <QMenu>

#include "git/gitutils.h"
#include "quickdialog.h"

class QTreeView;
class QLineEdit;
class BranchesDialogModel;
class QAction;
class BranchFilterModel;
class KActionCollection;
class KateProjectPluginView;

namespace KTextEditor
{
class MainWindow;
}

class BranchesDialog : public QuickDialog
{
    Q_OBJECT
public:
    BranchesDialog(QWidget *window, KateProjectPluginView *pluginView, QString projectPath);
    void openDialog(GitUtils::RefType r);
    void sendMessage(const QString &message, bool warn);
    QString branch() const
    {
        return m_branch;
    }

Q_SIGNALS:
    void branchSelected(const QString &branch);

private Q_SLOTS:
    void slotReturnPressed() override;
    void reselectFirst();

protected:
    BranchesDialogModel *m_model;
    QSortFilterProxyModel *m_proxyModel;
    QString m_projectPath;

private:
    KateProjectPluginView *m_pluginView;
    QString m_branch;
};

#endif

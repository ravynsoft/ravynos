/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <QMenu>

#include "quickdialog.h"

class QTreeView;
class QLineEdit;
class BranchesDialogModel;
class QAction;
class StashFilterModel;
class KActionCollection;
class QStandardItemModel;
class QProcess;
class GitWidget;

namespace KTextEditor
{
class MainWindow;
}

namespace GitUtils
{
struct CheckoutResult;
}

enum class StashMode : uint8_t {
    None = 0,
    Stash,
    StashKeepIndex,
    StashUntrackIncluded,
    StashPopLast,
    StashPop,
    StashDrop,
    StashApply,
    StashApplyLast,
    ShowStashContent,
};

class StashDialog : public QuickDialog
{
    Q_OBJECT
public:
    StashDialog(QWidget *parent, QWidget *window, const QString &gitPath);

    void openDialog(StashMode mode);

    Q_SIGNAL void message(const QString &msg, bool warn);
    Q_SIGNAL void done();
    Q_SIGNAL void showStashDiff(const QByteArray &diff);

protected Q_SLOTS:
    void slotReturnPressed() override;

private:
    QProcess *gitp();
    void stash(bool keepIndex, bool includeUntracked);
    void getStashList();
    void popStash(const QByteArray &index, const QString &command = QStringLiteral("pop"));
    void applyStash(const QByteArray &index);
    void dropStash(const QByteArray &index);
    void showStash(const QByteArray &index);

    QStandardItemModel *m_model;
    StashFilterModel *m_proxyModel;
    QString m_gitPath;
    QString m_projectPath;
    StashMode m_currentMode = StashMode::None;
};

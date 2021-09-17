/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef BRANCHCHECKOUTDIALOG_H
#define BRANCHCHECKOUTDIALOG_H

#include "branchesdialog.h"

class BranchCheckoutDialog : public BranchesDialog
{
    Q_OBJECT
public:
    BranchCheckoutDialog(QWidget *mainWindow, KateProjectPluginView *pluginView, QString projectPath);
    ~BranchCheckoutDialog();
    void openDialog();

private Q_SLOTS:
    void slotReturnPressed() override;
    void reselectFirst();
    void onCheckoutDone();

private:
    void resetValues();
    void createNewBranch(const QString &branch, const QString &fromBranch = QString());

private:
    QFutureWatcher<GitUtils::CheckoutResult> m_checkoutWatcher;
    QString m_checkoutBranchName;
    bool m_checkingOutFromBranch = false;
};

#endif // BRANCHCHECKOUTDIALOG_H

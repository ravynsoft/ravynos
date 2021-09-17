/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef GITCOMMITDIALOG_H
#define GITCOMMITDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

class QFont;

class GitCommitDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GitCommitDialog(const QString &lastCommit, const QFont &font, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    QString subject() const;
    QString description() const;
    bool signoff() const;
    bool amendingLastCommit() const;
    void setAmendingCommit();

private:
    Q_SLOT void updateLineSizeLabel();
    void loadCommitMessage(const QString &lastCommit);

    QLineEdit m_le;
    QPlainTextEdit m_pe;
    QPushButton ok;
    QPushButton cancel;
    QLabel m_leLen;
    QLabel m_peLen;
    QCheckBox m_cbSignOff;
    QCheckBox m_cbAmend;
};

#endif // GITCOMMITDIALOG_H

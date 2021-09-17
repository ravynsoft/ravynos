/*
 * SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-only WITH Qt-GPL-exception-1.0
 */

#pragma once

#include <QApplication>

QT_FORWARD_DECLARE_CLASS(QSharedMemory)

namespace SharedTools
{
class QtLocalPeer;

class QtSingleApplication : public QApplication
{
    Q_OBJECT

public:
    QtSingleApplication(const QString &id, int &argc, char **argv);
    ~QtSingleApplication();

    bool isRunning(qint64 pid = -1);

    void setActivationWindow(QWidget *aw, bool activateOnMessage = true);
    QWidget *activationWindow() const;
    bool event(QEvent *event);

    QString applicationId() const;
    void setBlock(bool value);

public Q_SLOTS:
    bool sendMessage(const QString &message, int timeout = 5000, qint64 pid = -1);
    void activateWindow();

Q_SIGNALS:
    void messageReceived(const QString &message, QObject *socket);
    void fileOpenRequest(const QString &file);

private:
    QString instancesFileName(const QString &appId);

    qint64 firstPeer;
    QSharedMemory *instances;
    QtLocalPeer *pidPeer;
    QWidget *actWin;
    QString appId;
    bool block;
};

} // namespace SharedTools

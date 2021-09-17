/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2018 Gregor Mi <codestruct@posteo.org>
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#pragma once

#include <QMainWindow>
#include <memory>

class TsTestApp : public QMainWindow
{
    Q_OBJECT

public:
    explicit TsTestApp(QWidget *parent = nullptr);
    ~TsTestApp() override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

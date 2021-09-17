/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2014 Martin Sandsmark <martin.sandsmark@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef REPLICODEPLUGIN_H
#define REPLICODEPLUGIN_H

#include "replicodeview.h"
#include <KTextEditor/ConfigPage>
#include <KTextEditor/Plugin>

class ReplicodePlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    // Constructor
    explicit ReplicodePlugin(QObject *parent = nullptr, const QList<QVariant> &args = QList<QVariant>());
    // Destructor
    ~ReplicodePlugin() override;

    QObject *createView(KTextEditor::MainWindow *mainWindow) override
    {
        return new ReplicodeView(this, mainWindow);
    }

    // Config interface
    int configPages() const override
    {
        return 1;
    }
    KTextEditor::ConfigPage *configPage(int number = 0, QWidget *parent = nullptr) override;
};

#endif

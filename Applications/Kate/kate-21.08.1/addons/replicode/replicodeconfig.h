/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2014 Martin Sandsmark <martin.sandsmark@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef REPLICODECONFIG_H
#define REPLICODECONFIG_H

#include <QTabWidget>
#include <QVariant>

class Ui_tabWidget;
class ReplicodeSettings;

class ReplicodeConfig : public QTabWidget
{
    Q_OBJECT
public:
    explicit ReplicodeConfig(QWidget *parent = nullptr);
    ~ReplicodeConfig() override;

public Q_SLOTS:
    void reset();
    void save();
    void load();

    ReplicodeSettings *settingsObject()
    {
        save();
        return m_settings;
    }

private:
    Ui_tabWidget *m_ui;
    ReplicodeSettings *m_settings;
};

#endif // REPLICODECONFIG_H

/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KATESQLCONFIGPAGE_H
#define KATESQLCONFIGPAGE_H

class OutputStyleWidget;
class QCheckBox;

#include "katesqlplugin.h"

#include <ktexteditor/configpage.h>

/// TODO: add options to change datetime and numbers format

class KateSQLConfigPage : public KTextEditor::ConfigPage
{
    Q_OBJECT

public:
    explicit KateSQLConfigPage(QWidget *parent = nullptr);
    ~KateSQLConfigPage() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public Q_SLOTS:
    void apply() override;
    void reset() override;
    void defaults() override;

private:
    KateSQLPlugin *m_plugin = nullptr;
    QCheckBox *m_box;
    OutputStyleWidget *m_outputStyleWidget;

Q_SIGNALS:
    void settingsChanged();
};

#endif // KATESQLCONFIGPAGE_H

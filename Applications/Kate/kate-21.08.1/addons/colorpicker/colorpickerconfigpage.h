/*
    SPDX-FileCopyrightText: 2018 Sven Brauch <mail@svenbrauch.de>
    SPDX-FileCopyrightText: 2018 Michal Srb <michalsrb@gmail.com>
    SPDX-FileCopyrightText: 2020 Jan Paul Batrina <jpmbatrina01@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_COLORPICKER_CONFIGPAGE_H
#define KATE_COLORPICKER_CONFIGPAGE_H

#include "katecolorpickerplugin.h"
#include <KTextEditor/ConfigPage>

#include <QCheckBox>
#include <QMap>

class KateColorPickerConfigPage : public KTextEditor::ConfigPage
{
    Q_OBJECT
public:
    explicit KateColorPickerConfigPage(QWidget *parent = nullptr, KateColorPickerPlugin *plugin = nullptr);
    ~KateColorPickerConfigPage() override
    {
    }

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    void apply() override;
    void reset() override;
    void defaults() override
    {
    }

private:
    QCheckBox *chkNamedColors;
    QCheckBox *chkPreviewAfterColor;
    QMap<int, QCheckBox *> chkHexLengths;
    KateColorPickerPlugin *m_plugin;
    bool m_colorConfigChanged = false;
};

#endif // KATE_COLORPICKER_H

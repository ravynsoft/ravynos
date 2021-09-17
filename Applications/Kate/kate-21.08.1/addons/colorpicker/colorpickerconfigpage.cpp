/*
    SPDX-FileCopyrightText: 2018 Sven Brauch <mail@svenbrauch.de>
    SPDX-FileCopyrightText: 2018 Michal Srb <michalsrb@gmail.com>
    SPDX-FileCopyrightText: 2020 Jan Paul Batrina <jpmbatrina01@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "colorpickerconfigpage.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KTextEditor/ConfigPage>

#include <QApplication>
#include <QCheckBox>
#include <QGroupBox>
#include <QStyle>
#include <QVBoxLayout>

KTextEditor::ConfigPage *KateColorPickerPlugin::configPage(int number, QWidget *parent)
{
    if (number != 0) {
        return nullptr;
    }

    return new KateColorPickerConfigPage(parent, this);
}

KateColorPickerConfigPage::KateColorPickerConfigPage(QWidget *parent, KateColorPickerPlugin *plugin)
    : KTextEditor::ConfigPage(parent)
    , m_plugin(plugin)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    chkNamedColors = new QCheckBox(i18n("Show preview for known color names"), this);
    chkNamedColors->setToolTip(i18n(
        "Also show the color picker for known color names (e.g. skyblue).\nSee https://www.w3.org/TR/SVG11/types.html#ColorKeywords for the list of colors."));
    layout->addWidget(chkNamedColors);

    chkPreviewAfterColor = new QCheckBox(i18n("Place preview after text color"), this);
    layout->addWidget(chkPreviewAfterColor);

    connect(chkNamedColors, &QCheckBox::stateChanged, this, &KateColorPickerConfigPage::changed);
    connect(chkPreviewAfterColor, &QCheckBox::stateChanged, this, &KateColorPickerConfigPage::changed);

    QGroupBox *hexGroup = new QGroupBox(i18n("Hex color matching"), this);
    QVBoxLayout *hexLayout = new QVBoxLayout();
    // Hex color formats supported by QColor. See https://doc.qt.io/qt-5/qcolor.html#setNamedColor
    chkHexLengths.insert(12, new QCheckBox(i18n("12 digits (#RRRRGGGGBBBB)"), this));
    chkHexLengths.insert(9, new QCheckBox(i18n("9 digits (#RRRGGGBBB)"), this));
    chkHexLengths.insert(8, new QCheckBox(i18n("8 digits (#AARRGGBB)"), this));
    chkHexLengths.insert(6, new QCheckBox(i18n("6 digits (#RRGGBB)"), this));
    chkHexLengths.insert(3, new QCheckBox(i18n("3 digits (#RGB)"), this));

    for (QCheckBox *chk : chkHexLengths.values()) {
        hexLayout->addWidget(chk);
        connect(chk, &QCheckBox::stateChanged, this, &KateColorPickerConfigPage::changed);
    }
    hexGroup->setLayout(hexLayout);
    layout->addWidget(hexGroup);

    layout->addStretch();

    connect(this, &KateColorPickerConfigPage::changed, this, [this]() {
        m_colorConfigChanged = true;
    });

    reset();
}

QString KateColorPickerConfigPage::name() const
{
    return i18n("Color Picker");
}

QString KateColorPickerConfigPage::fullName() const
{
    return i18n("Color Picker Settings");
}

QIcon KateColorPickerConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("color-picker"));
}

void KateColorPickerConfigPage::apply()
{
    if (!m_colorConfigChanged) {
        // apply() gets called when the "Apply" or "OK" button is pressed. This means that if a user presses "Apply" THEN "OK", the config is updated twice
        // Since the the regeneration of color note positions is expensive, we only update on the first call to apply() before changes are made again
        return;
    }

    KConfigGroup config(KSharedConfig::openConfig(), "ColorPicker");
    config.writeEntry("NamedColors", chkNamedColors->isChecked());
    config.writeEntry("PreviewAfterColor", chkPreviewAfterColor->isChecked());

    QList<int> hexLengths;
    for (auto it = chkHexLengths.cbegin(); it != chkHexLengths.cend(); ++it) {
        if (it.value()->isChecked()) {
            hexLengths.append(it.key());
        }
    }
    config.writeEntry("HexLengths", hexLengths);

    config.sync();
    m_plugin->readConfig();
    m_colorConfigChanged = false;
}

void KateColorPickerConfigPage::reset()
{
    KConfigGroup config(KSharedConfig::openConfig(), "ColorPicker");
    chkNamedColors->setChecked(config.readEntry("NamedColors", false));
    chkPreviewAfterColor->setChecked(config.readEntry("PreviewAfterColor", true));

    QList<int> enabledHexLengths = config.readEntry("HexLengths", QList<int>{12, 9, 6, 3});
    for (const int hexLength : chkHexLengths.keys()) {
        chkHexLengths[hexLength]->setChecked(enabledHexLengths.contains(hexLength));
    }
}

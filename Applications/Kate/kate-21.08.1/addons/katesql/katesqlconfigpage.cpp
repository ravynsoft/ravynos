/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "katesqlconfigpage.h"
#include "outputstylewidget.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QBoxLayout>
#include <QCheckBox>
#include <QGroupBox>

KateSQLConfigPage::KateSQLConfigPage(QWidget *parent)
    : KTextEditor::ConfigPage(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_box = new QCheckBox(i18nc("@option:check", "Save and restore connections in Kate session"), this);

    QGroupBox *stylesGroupBox = new QGroupBox(i18nc("@title:group", "Output Customization"), this);
    QVBoxLayout *stylesLayout = new QVBoxLayout(stylesGroupBox);

    m_outputStyleWidget = new OutputStyleWidget(this);

    stylesLayout->addWidget(m_outputStyleWidget);

    layout->addWidget(m_box);
    layout->addWidget(stylesGroupBox, 1);

    setLayout(layout);

    reset();

    connect(m_box, &QCheckBox::stateChanged, this, &KateSQLConfigPage::changed);
    connect(m_outputStyleWidget, &OutputStyleWidget::changed, this, &KateSQLConfigPage::changed);
}

KateSQLConfigPage::~KateSQLConfigPage()
{
}

QString KateSQLConfigPage::name() const
{
    return i18nc("@title", "SQL");
}

QString KateSQLConfigPage::fullName() const
{
    return i18nc("@title:window", "SQL ConfigPage Settings");
}

QIcon KateSQLConfigPage::icon() const
{
    return QIcon::fromTheme(QLatin1String("server-database"));
}

void KateSQLConfigPage::apply()
{
    KConfigGroup config(KSharedConfig::openConfig(), "KateSQLPlugin");

    config.writeEntry("SaveConnections", m_box->isChecked());

    m_outputStyleWidget->writeConfig();

    config.sync();

    Q_EMIT settingsChanged();
}

void KateSQLConfigPage::reset()
{
    KConfigGroup config(KSharedConfig::openConfig(), "KateSQLPlugin");

    m_box->setChecked(config.readEntry("SaveConnections", true));

    m_outputStyleWidget->readConfig();
}

void KateSQLConfigPage::defaults()
{
    KConfigGroup config(KSharedConfig::openConfig(), "KateSQLPlugin");

    config.revertToDefault("SaveConnections");
    config.revertToDefault("OutputCustomization");
}

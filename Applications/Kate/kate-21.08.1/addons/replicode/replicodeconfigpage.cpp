/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2014 Martin Sandsmark <martin.sandsmark@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "replicodeconfigpage.h"
#include "replicodeconfig.h"
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KUrlRequester>

#include <KLocalizedString>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QTabWidget>

ReplicodeConfigPage::ReplicodeConfigPage(QWidget *parent)
    : KTextEditor::ConfigPage(parent)
    , m_config(new ReplicodeConfig(this))
{
    QGridLayout *gridlayout = new QGridLayout;
    setLayout(gridlayout);
    gridlayout->addWidget(new QLabel(i18n("Path to replicode executor:")), 0, 0);

    m_requester = new KUrlRequester;
    m_requester->setMode(KFile::File | KFile::ExistingOnly);
    gridlayout->addWidget(m_requester, 0, 1);

    gridlayout->addWidget(m_config, 1, 0, 1, 2);

    reset();

    connect(m_requester, &KUrlRequester::textChanged, this, &ReplicodeConfigPage::changed);
}

QString ReplicodeConfigPage::name() const
{
    return i18n("Replicode");
}

QString ReplicodeConfigPage::fullName() const
{
    return i18n("Replicode configuration");
}

void ReplicodeConfigPage::apply()
{
    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("Replicode"));
    config.writeEntry("replicodePath", m_requester->text());
    m_config->save();
}

void ReplicodeConfigPage::reset()
{
    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("Replicode"));
    m_requester->setText(config.readEntry<QString>("replicodePath", QString()));
    m_config->load();
}

void ReplicodeConfigPage::defaults()
{
    m_requester->setText(QString());
    m_config->reset();
}

/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "katesqlplugin.h"
#include "katesqlconfigpage.h"
#include "katesqlview.h"

#include <ktexteditor/document.h>

#include <KAboutData>
#include <KLocalizedString>

#include <QIcon>

K_PLUGIN_FACTORY_WITH_JSON(KateSQLFactory, "katesql.json", registerPlugin<KateSQLPlugin>();)

// BEGIN KateSQLPLugin
KateSQLPlugin::KateSQLPlugin(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
{
}

KateSQLPlugin::~KateSQLPlugin()
{
}

QObject *KateSQLPlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    KateSQLView *view = new KateSQLView(this, mainWindow);

    connect(this, &KateSQLPlugin::globalSettingsChanged, view, &KateSQLView::slotGlobalSettingsChanged);

    return view;
}

KTextEditor::ConfigPage *KateSQLPlugin::configPage(int number, QWidget *parent)
{
    if (number != 0) {
        return nullptr;
    }

    KateSQLConfigPage *page = new KateSQLConfigPage(parent);
    connect(page, &KateSQLConfigPage::settingsChanged, this, &KateSQLPlugin::globalSettingsChanged);

    return page;
}

// END KateSQLPlugin

#include "katesqlplugin.moc"

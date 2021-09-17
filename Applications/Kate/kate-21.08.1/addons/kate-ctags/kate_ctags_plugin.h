#ifndef KATE_CTAGS_PLUGIN_H
#define KATE_CTAGS_PLUGIN_H
/* Description : Kate CTags plugin
 *
 * SPDX-FileCopyrightText: 2008-2011 Kare Sars <kare.sars@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <KTextEditor/ConfigPage>
#include <KTextEditor/Plugin>
#include <ktexteditor/application.h>
#include <ktexteditor/mainwindow.h>

#include "kate_ctags_view.h"
#include "ui_CTagsGlobalConfig.h"

//******************************************************************/
class KateCTagsPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit KateCTagsPlugin(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());
    ~KateCTagsPlugin() override
    {
    }

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

    int configPages() const override
    {
        return 1;
    }
    KTextEditor::ConfigPage *configPage(int number = 0, QWidget *parent = nullptr) override;
    void readConfig();

    KateCTagsView *m_view = nullptr;
};

//******************************************************************/
class KateCTagsConfigPage : public KTextEditor::ConfigPage
{
    Q_OBJECT
public:
    explicit KateCTagsConfigPage(QWidget *parent = nullptr, KateCTagsPlugin *plugin = nullptr);
    ~KateCTagsConfigPage() override
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

private Q_SLOTS:
    void addGlobalTagTarget();
    void delGlobalTagTarget();
    void updateGlobalDB();
    void updateDone(int exitCode, QProcess::ExitStatus status);

private:
    bool listContains(const QString &target);

    QProcess m_proc;
    KateCTagsPlugin *m_plugin;
    Ui_CTagsGlobalConfig m_confUi{};
};

#endif

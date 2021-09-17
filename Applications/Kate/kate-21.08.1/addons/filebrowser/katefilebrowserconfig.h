/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>
   SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
   SPDX-FileCopyrightText: 2009 Dominik Haumann <dhaumann kde org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KATE_FILEBROWSER_CONFIG_H
#define KATE_FILEBROWSER_CONFIG_H

#include <ktexteditor/configpage.h>

class KateFileBrowser;
class KActionSelector;

class KateFileBrowserConfigPage : public KTextEditor::ConfigPage
{
    Q_OBJECT

public:
    explicit KateFileBrowserConfigPage(QWidget *parent = nullptr, KateFileBrowser *kfb = nullptr);
    ~KateFileBrowserConfigPage() override
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
    void slotMyChanged();

private:
    void init();

    KateFileBrowser *fileBrowser;
    KActionSelector *acSel;

    bool m_changed = false;
};

#endif // KATE_FILEBROWSER_CONFIG_H

// kate: space-indent on; indent-width 2; replace-tabs on;

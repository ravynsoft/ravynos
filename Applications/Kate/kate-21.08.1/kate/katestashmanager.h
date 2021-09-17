/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2021 Méven Car <meven.car@kdemail.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATESTASHMANAGER_H
#define KATESTASHMANAGER_H

#include "katesession.h"
#include "kconfiggroup.h"

namespace KTextEditor
{
class Document;
}

class KateViewManager;

class KateStashManager : QObject
{
    Q_OBJECT
public:
    KateStashManager(QObject *parent = nullptr);

    bool stashUnsavedChanges()
    {
        return m_stashUnsavedChanges;
    }

    void setStashUnsavedChanges(bool stashUnsavedChanges)
    {
        m_stashUnsavedChanges = stashUnsavedChanges;
    }

    bool stashNewUnsavedFiles()
    {
        return m_stashNewUnsavedFiles;
    }

    void setStashNewUnsavedFiles(bool stashNewUnsavedFiles)
    {
        m_stashNewUnsavedFiles = stashNewUnsavedFiles;
    }

    void stashDocuments(KConfig *cfg, const QList<KTextEditor::Document *> &documents);

    bool willStashDoc(KTextEditor::Document *doc);

    void stashDocument(KTextEditor::Document *doc, const QString &stashfileName, KConfigGroup &kconfig, const QString &path);
    bool popDocument(KTextEditor::Document *doc, const KConfigGroup &kconfig);

    void clearStashForSession(const KateSession::Ptr session);

private:
    bool m_stashUnsavedChanges = false;
    bool m_stashNewUnsavedFiles = true;
};

#endif // KATESTASHMANAGER_H

/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2021 Méven Car <meven.car@kdemail.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katestashmanager.h"

#include "katedebug.h"

#include "kateapp.h"
#include "katedocmanager.h"
#include "kateviewmanager.h"

#include "kconfiggroup.h"
#include "ksharedconfig.h"

#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QTextCodec>
#include <QUrl>

KateStashManager::KateStashManager(QObject *parent)
    : QObject(parent)
{
}

void KateStashManager::clearStashForSession(const KateSession::Ptr session)
{
    const QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    if (dir.exists(QStringLiteral("stash"))) {
        dir.cd(QStringLiteral("stash"));
        const QString sessionName = session->name();
        if (dir.exists(sessionName)) {
            dir.cd(sessionName);
            dir.removeRecursively();
        }
    }
}

void KateStashManager::stashDocuments(KConfig *config, const QList<KTextEditor::Document *> &documents)
{
    // prepare stash directory
    const QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    dir.mkdir(QStringLiteral("stash"));
    dir.cd(QStringLiteral("stash"));

    const auto activeSession = KateApp::self()->sessionManager()->activeSession();
    if (!activeSession || activeSession->isAnonymous() || activeSession->name().isEmpty()) {
        qDebug(LOG_KATE) << "Could not stash files without a session";
        return;
    }

    const QString sessionName = activeSession->name();
    dir.mkdir(sessionName);
    dir.cd(sessionName);

    int i = 0;
    for (KTextEditor::Document *doc : documents) {
        const QString entryName = QStringLiteral("Document %1").arg(i);
        KConfigGroup cg(config, entryName);

        // stash the file content
        if (doc->isModified()) {
            stashDocument(doc, entryName, cg, dir.path());
        }

        i++;
    }
}

bool KateStashManager::willStashDoc(KTextEditor::Document *doc)
{
    const auto activeSession = KateApp::self()->sessionManager()->activeSession();
    if (!activeSession || activeSession->isAnonymous() || activeSession->name().isEmpty()) {
        return false;
    }
    if (doc->text().isEmpty()) {
        return false;
    }
    if (doc->url().isEmpty()) {
        return m_stashNewUnsavedFiles;
    }
    if (doc->url().isLocalFile()) {
        const QString path = doc->url().toLocalFile();
        if (path.startsWith(QDir::tempPath())) {
            return false; // inside tmp resource, do not stash
        }
        return m_stashUnsavedChanges;
    }
    return false;
}

void KateStashManager::stashDocument(KTextEditor::Document *doc, const QString &stashfileName, KConfigGroup &kconfig, const QString &path)
{
    if (!willStashDoc(doc)) {
        return;
    }
    // Stash changes
    QString stashedFile = path + QStringLiteral("/") + stashfileName;

    // save the current document changes to stash
    if (!doc->saveAs(QUrl::fromLocalFile(stashedFile))) {
        qCWarning(LOG_KATE) << "Could not write to stash file" << stashedFile;
        return;
    }

    // write stash metadata to config
    kconfig.writeEntry("stashedFile", stashedFile);
    if (doc->url().isValid()) {
        // save checksum for already-saved documents
        kconfig.writeEntry("checksum", doc->checksum());
    }

    kconfig.sync();
}

bool KateStashManager::popDocument(KTextEditor::Document *doc, const KConfigGroup &kconfig)
{
    if (!(kconfig.hasKey("stashedFile"))) {
        return false;
    }
    qCDebug(LOG_KATE) << "popping stashed document" << doc->url();

    // read metadata
    auto stashedFile = kconfig.readEntry("stashedFile");
    auto url = QUrl(kconfig.readEntry("URL"));

    bool checksumOk = true;
    if (url.isValid()) {
        const auto sum = kconfig.readEntry(QStringLiteral("checksum")).toLatin1().constData();
        checksumOk = sum != doc->checksum();
    }

    if (checksumOk) {
        // open file with stashed content
        QFile file(stashedFile);
        file.open(QIODevice::ReadOnly);
        QTextStream out(&file);
        const auto codec = QTextCodec::codecForName(kconfig.readEntry("Encoding").toLocal8Bit());
        if (codec != nullptr) {
            out.setCodec(codec);
        }

        doc->setText(out.readAll());

        // clean stashed file
        if (!file.remove()) {
            qCWarning(LOG_KATE) << "Could not remove stash file" << stashedFile;
        }

        return true;
    } else {
        return false;
    }
}

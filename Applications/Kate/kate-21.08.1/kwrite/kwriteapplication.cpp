/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kwriteapplication.h"
#include "kwrite.h"

#include <KConfigGui>
#include <QApplication>

KWriteApplication::KWriteApplication::KWriteApplication()
{
    m_application = new KTextEditor::Application(this);
    KTextEditor::Editor::instance()->setApplication(m_application);
}

KWriteApplication::~KWriteApplication()
{
    delete m_application;
}

KWrite *KWriteApplication::newWindow(KTextEditor::Document *doc)
{
    KWrite *k = new KWrite(doc, this);
    m_kwrites.append(k);
    return k;
}

void KWriteApplication::restore()
{
    KConfig *config = KConfigGui::sessionConfig();

    if (!config) {
        return;
    }

    int docs, windows;
    QString buf;
    KTextEditor::Document *doc;
    KWrite *t;

    KConfigGroup numberConfig(config, "Number");
    docs = numberConfig.readEntry("NumberOfDocuments", 0);
    windows = numberConfig.readEntry("NumberOfWindows", 0);

    for (int z = 1; z <= docs; z++) {
        buf = QStringLiteral("Document %1").arg(z);
        KConfigGroup cg(config, buf);
        doc = KTextEditor::Editor::instance()->createDocument(nullptr);
        doc->readSessionConfig(cg);
        addDocument(doc);
    }

    for (int z = 1; z <= windows; z++) {
        buf = QStringLiteral("Window %1").arg(z);
        KConfigGroup cg(config, buf);
        t = newWindow(m_documents.at(cg.readEntry("DocumentNumber", 0) - 1));
        t->restore(config, z);
    }
}

void KWriteApplication::saveProperties(KConfig *config)
{
    config->group("Number").writeEntry("NumberOfDocuments", m_documents.count());

    for (int z = 1; z <= m_documents.count(); z++) {
        QString buf = QStringLiteral("Document %1").arg(z);
        KConfigGroup cg(config, buf);
        KTextEditor::Document *doc = m_documents.at(z - 1);
        doc->writeSessionConfig(cg);
    }

    for (int z = 1; z <= m_kwrites.count(); z++) {
        QString buf = QStringLiteral("Window %1").arg(z);
        KConfigGroup cg(config, buf);
        cg.writeEntry("DocumentNumber", m_documents.indexOf(m_kwrites.at(z - 1)->activeView()->document()) + 1);
    }
}

bool KWriteApplication::quit()
{
    QList<KWrite *> copy(m_kwrites);
    for (auto kwrite : copy) {
        if (!kwrite->close()) {
            return false;
        }

        m_kwrites.removeAll(kwrite);
        delete kwrite;
    }
    return true;
}

KTextEditor::MainWindow *KWriteApplication::activeMainWindow()
{
    for (auto kwrite : m_kwrites) {
        if (kwrite->isActiveWindow()) {
            return kwrite->mainWindow();
        }
    }

    return nullptr;
}

QList<KTextEditor::MainWindow *> KWriteApplication::mainWindows()
{
    QList<KTextEditor::MainWindow *> windows;
    for (auto kwrite : m_kwrites) {
        windows.append(kwrite->mainWindow());
    }

    return windows;
}

bool KWriteApplication::closeDocument(KTextEditor::Document *document)
{
    QList<KWrite *> copy(m_kwrites);
    for (auto kwrite : copy) {
        if (kwrite->activeView()->document() == document) {
            if (!kwrite->close()) {
                return false;
            }
            m_kwrites.removeAll(kwrite);
            delete kwrite;
        }
    }

    return true;
}

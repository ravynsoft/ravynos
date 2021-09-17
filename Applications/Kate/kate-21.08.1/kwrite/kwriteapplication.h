/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KWRITE_APPLICATION_H
#define KWRITE_APPLICATION_H

#include <ktexteditor/application.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>

class KWrite;

class KWriteApplication : public QObject
{
    Q_OBJECT

public:
    KWriteApplication();
    ~KWriteApplication() override;

    void addDocument(KTextEditor::Document *doc)
    {
        m_documents.append(doc);
    }
    void removeDocument(KTextEditor::Document *doc)
    {
        m_documents.removeAll(doc);
    }
    void removeWindow(KWrite *kwrite)
    {
        m_kwrites.removeAll(kwrite);
    }

    bool noWindows()
    {
        return m_kwrites.isEmpty();
    }

    KWrite *newWindow(KTextEditor::Document *doc = nullptr);

    void restore();
    void saveProperties(KConfig *config);

public Q_SLOTS:
    QList<KTextEditor::Document *> documents()
    {
        return m_documents;
    }
    bool quit();
    KTextEditor::MainWindow *activeMainWindow();
    QList<KTextEditor::MainWindow *> mainWindows();
    bool closeDocument(KTextEditor::Document *document);

private:
    KTextEditor::Application *m_application;
    QList<KTextEditor::Document *> m_documents;
    QList<KWrite *> m_kwrites;
};

#endif

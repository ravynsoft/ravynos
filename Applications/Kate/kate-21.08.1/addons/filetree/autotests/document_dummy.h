/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2005 Dominik Haumann (dhaumann@kde.org) (documentation)

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DOCUMENT_DUMMY_H
#define DOCUMENT_DUMMY_H

#include <KTextEditor/Document>
#include <KTextEditor/ModificationInterface>

class DummyDocument : public KTextEditor::Document
{
    Q_OBJECT

public:
    DummyDocument(const QString &url = QString())
        : KTextEditor::Document(nullptr, nullptr)
        , m_name(QStringLiteral("foo"))
        , m_encoding()
        , m_views()
    {
        setUrl(url);
        m_name = url.section(QLatin1Char('/'), -1); // some nice default to mimic the KateDocument
    }
    DummyDocument(const char *url)
        : DummyDocument(QString::fromLatin1(url))
    {
    }

    ~DummyDocument() override
    {
    }
    KTextEditor::View *createView(QWidget *, KTextEditor::MainWindow * = nullptr) override
    {
        return nullptr;
    }
    virtual KTextEditor::View *activeView() const
    {
        return nullptr;
    }
    QList<KTextEditor::View *> views() const override
    {
        return m_views;
    }

    QString documentName() const override
    {
        return m_name;
    }

    QString mimeType() override
    {
        return QString();
    }
    QByteArray checksum() const override
    {
        return QByteArray();
    }
    bool setEncoding(const QString &) override
    {
        return false;
    }
    QString encoding() const override
    {
        return m_encoding;
    }

    bool documentReload() override
    {
        return true;
    }
    bool documentSave() override
    {
        return true;
    }
    bool documentSaveAs() override
    {
        return true;
    }

    bool isEditingTransactionRunning() const override
    {
        return false;
    }
    QString text() const override
    {
        return QString();
    }
    QString text(const KTextEditor::Range &, bool = false) const override
    {
        return QString();
    }
    QChar characterAt(const KTextEditor::Cursor &) const override
    {
        return QChar();
    }
    QString wordAt(const KTextEditor::Cursor &) const override
    {
        return QString();
    }
    KTextEditor::Range wordRangeAt(const KTextEditor::Cursor &) const override
    {
        return KTextEditor::Range();
    }
    bool isValidTextPosition(const KTextEditor::Cursor &) const override
    {
        return true;
    }
    QStringList textLines(const KTextEditor::Range &, bool = false) const override
    {
        return QStringList();
    }
    QString line(int) const override
    {
        return QString();
    }
    int lines() const override
    {
        return 0;
    }
    KTextEditor::Cursor documentEnd() const override
    {
        return KTextEditor::Cursor();
    }
    int totalCharacters() const override
    {
        return 0;
    }
    int lineLength(int) const override
    {
        return 0;
    }

    bool setText(const QString &) override
    {
        return false;
    }
    bool setText(const QStringList &) override
    {
        return false;
    }
    bool clear() override
    {
        return true;
    }
    bool insertText(const KTextEditor::Cursor &, const QString &, bool = false) override
    {
        return false;
    }
    bool insertText(const KTextEditor::Cursor &, const QStringList &, bool = false) override
    {
        return false;
    }
    bool removeText(const KTextEditor::Range &, bool = false) override
    {
        return false;
    }
    bool insertLine(int, const QString &) override
    {
        return false;
    }
    bool insertLines(int, const QStringList &) override
    {
        return false;
    }
    bool removeLine(int) override
    {
        return false;
    }
    KTextEditor::DefaultStyle defaultStyleAt(const KTextEditor::Cursor &) const override
    {
        return KTextEditor::dsNormal;
    }
    QString mode() const override
    {
        return QString();
    }
    QString highlightingMode() const override
    {
        return QString();
    }
    QStringList modes() const override
    {
        return QStringList();
    }
    QStringList highlightingModes() const override
    {
        return QStringList();
    }
    bool setMode(const QString &) override
    {
        return false;
    }
    bool setHighlightingMode(const QString &) override
    {
        return false;
    }
    QString highlightingModeSection(int) const override
    {
        return QString();
    }
    QString modeSection(int) const override
    {
        return QString();
    }
    bool print() override
    {
        return false;
    }
    void printPreview() override
    {
    }
    QStringList embeddedHighlightingModes() const override
    {
        return QStringList();
    }
    QString highlightingModeAt(const KTextEditor::Cursor &) override
    {
        return QString();
    }
    bool isLineModified(int) const override
    {
        return false;
    }
    bool isLineSaved(int) const override
    {
        return false;
    }
    bool isLineTouched(int) const override
    {
        return false;
    }

    // KParts::ReadWritePart
    bool saveFile() override
    {
        return false;
    }

    // make QObject happy
Q_SIGNALS:
    void modifiedOnDisk(KTextEditor::Document *, bool, KTextEditor::ModificationInterface::ModifiedOnDiskReason);

public:
    void setUrl(const QString &url)
    {
        KParts::ReadOnlyPart::setUrl(QUrl(url));
    }
    void setName(const QString &name)
    {
        m_name = name;
    }

    void readSessionConfig(const KConfigGroup &, const QSet<QString> & = QSet<QString>()) override
    {
    }
    void writeSessionConfig(KConfigGroup &, const QSet<QString> & = QSet<QString>()) override
    {
    }

    bool postMessage(KTextEditor::Message *) override
    {
        return false;
    }

    bool isDataRecoveryAvailable() const override
    {
        return false;
    }
    void recoverData() override
    {
    }
    void discardDataRecovery() override
    {
    }

private:
    QString m_name, m_encoding;
    QList<KTextEditor::View *> m_views;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;

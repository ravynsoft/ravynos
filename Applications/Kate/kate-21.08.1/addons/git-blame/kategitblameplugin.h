/*
    SPDX-FileCopyrightText: 2021 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KateGitBlamePlugin_h
#define KateGitBlamePlugin_h

#include "gitblametooltip.h"

#include <KTextEditor/ConfigPage>
#include <KTextEditor/InlineNoteProvider>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>

#include <QProcess>

#include <QDateTime>
#include <QHash>
#include <QList>
#include <QLocale>
#include <QRegularExpression>
#include <QVariant>
#include <QVector>

enum class KateGitBlameMode { None, SingleLine, AllLines, Count = AllLines };

struct KateGitBlameInfo {
    QString commitHash;
    QString name;
    QDateTime date;
    QString title;
    QString line;
};

class KateGitBlamePluginView;
class GitBlameTooltip;

class GitBlameInlineNoteProvider : public KTextEditor::InlineNoteProvider
{
    Q_OBJECT
public:
    GitBlameInlineNoteProvider(KateGitBlamePluginView *view);
    ~GitBlameInlineNoteProvider();

    QVector<int> inlineNotes(int line) const override;
    QSize inlineNoteSize(const KTextEditor::InlineNote &note) const override;
    void paintInlineNote(const KTextEditor::InlineNote &note, QPainter &painter) const override;
    void inlineNoteActivated(const KTextEditor::InlineNote &note, Qt::MouseButtons buttons, const QPoint &globalPos) override;
    void cycleMode();
    void setMode(KateGitBlameMode mode);

private:
    KateGitBlamePluginView *m_pluginView;
    QLocale m_locale;
    KateGitBlameMode m_mode;
};

class KateGitBlamePlugin : public KTextEditor::Plugin
{
    Q_OBJECT
public:
    explicit KateGitBlamePlugin(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());
    ~KateGitBlamePlugin() override;

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;
};

class KateGitBlamePluginView : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    KateGitBlamePluginView(KateGitBlamePlugin *plugin, KTextEditor::MainWindow *mainwindow);
    ~KateGitBlamePluginView() override;

    QPointer<KTextEditor::View> activeView() const;
    QPointer<KTextEditor::Document> activeDocument() const;

    bool hasBlameInfo() const;

    const KateGitBlameInfo &blameInfo(int lineNr);

    void showCommitInfo(const QString &hash, KTextEditor::View *view);

    void setToolTipIgnoreKeySequence(QKeySequence sequence);

private:
    struct CommitInfo {
        QString m_hash;
        QString m_title;
        QString m_content;
        void clear();
    };

    void viewChanged(KTextEditor::View *view);

    void startBlameProcess(const QUrl &url);
    void blameFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void startShowProcess(const QUrl &url, const QString &hash);
    void showFinished(int exitCode, QProcess::ExitStatus exitStatus);

    const KateGitBlameInfo &blameGetUpdateInfo(int lineNr);

    KTextEditor::MainWindow *m_mainWindow;

    GitBlameInlineNoteProvider m_inlineNoteProvider;

    QProcess m_blameInfoProc;
    QProcess m_showProc;
    QVector<KateGitBlameInfo> m_blameInfo;
    QUrl m_blameUrl;
    QPointer<KTextEditor::View> m_lastView;
    int m_lineOffset{0};
    GitBlameTooltip m_tooltip;
    QString m_showHash;
    CommitInfo m_activeCommitInfo;
};

#endif // KateGitBlamePlugin_h

#ifndef KATE_CTAGS_VIEW_H
#define KATE_CTAGS_VIEW_H
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

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <KTextEditor/Application>
#include <KTextEditor/MainWindow>
#include <ktexteditor/sessionconfiginterface.h>

#include <KXMLGUIClient>
#include <QProcess>

#include <KActionMenu>
#include <QPointer>
#include <QStack>
#include <QTimer>

#include "tags.h"

#include "ui_kate_ctags.h"

#include "gotosymbolwidget.h"

const static QString DEFAULT_CTAGS_CMD = QStringLiteral("ctags -R --c++-types=+px --extra=+q --excmd=pattern --exclude=Makefile --exclude=.");

typedef struct {
    QUrl url;
    KTextEditor::Cursor cursor;
} TagJump;

/******************************************************************/
class KateCTagsView : public QObject, public KXMLGUIClient, public KTextEditor::SessionConfigInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::SessionConfigInterface)

public:
    KateCTagsView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainWin);
    ~KateCTagsView() override;

    // reimplemented: read and write session config
    void readSessionConfig(const KConfigGroup &config) override;
    void writeSessionConfig(KConfigGroup &config) override;

    void jumpToTag(const QString &file, const QString &pattern, const QString &word);

public Q_SLOTS:
    void gotoDefinition();
    void gotoDeclaration();
    void lookupTag();
    void stepBack();
    void editLookUp();
    void aboutToShow();
    void tagHitClicked(QTreeWidgetItem *);
    void startEditTmr();

    void addTagTarget();
    void delTagTarget();

    void updateSessionDB();
    void updateDone(int exitCode, QProcess::ExitStatus status);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private Q_SLOTS:
    void resetCMD();
    void handleEsc(QEvent *e);
    void showSymbols();
    void showGlobalSymbols();

private:
    bool listContains(const QString &target);

    QString currentWord();

    void setNewLookupText(const QString &newText);
    void displayHits(const Tags::TagList &list);

    void gotoTagForTypes(const QString &tag, QStringList const &types);

    QPointer<KTextEditor::MainWindow> m_mWin;
    QPointer<QWidget> m_toolView;
    Ui::kateCtags m_ctagsUi{};
    GotoSymbolWidget *m_gotoSymbWidget;

    QPointer<KActionMenu> m_menu;
    QAction *m_gotoDef;
    QAction *m_gotoDec;
    QAction *m_lookup;

    QProcess m_proc;
    QString m_commonDB;

    QTimer m_editTimer;
    QStack<TagJump> m_jumpStack;
};

#endif

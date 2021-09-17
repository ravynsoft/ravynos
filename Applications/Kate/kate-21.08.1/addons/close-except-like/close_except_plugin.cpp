/**
 * \file
 *
 * \brief Kate Close Except/Like plugin implementation
 *
 * SPDX-FileCopyrightText: 2012 Alex Turbov <i.zaufi@gmail.com>
 *
 * \date Thu Mar  8 08:13:43 MSK 2012 -- Initial design
 */
/*
 * KateCloseExceptPlugin is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * KateCloseExceptPlugin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Project specific includes
#include "close_except_plugin.h"
#include "close_confirm_dialog.h"

// Standard includes
#include <KAboutData>
#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KTextEditor/Application>
#include <KTextEditor/MainWindow>
#include <KXMLGUIFactory>
#include <QFileInfo>
#include <QUrl>
#include <kio/global.h>

K_PLUGIN_FACTORY_WITH_JSON(CloseExceptPluginFactory, "katecloseexceptplugin.json", registerPlugin<kate::CloseExceptPlugin>();)

namespace kate
{
// BEGIN CloseExceptPlugin
CloseExceptPlugin::CloseExceptPlugin(QObject *application, const QList<QVariant> &)
    : KTextEditor::Plugin(application)
{
}

QObject *CloseExceptPlugin::createView(KTextEditor::MainWindow *parent)
{
    return new CloseExceptPluginView(parent, this);
}

void CloseExceptPlugin::readSessionConfig(const KConfigGroup &config)
{
    const KConfigGroup scg(&config, QStringLiteral("menu"));
    m_show_confirmation_needed = scg.readEntry(QStringLiteral("ShowConfirmation"), true);
}

void CloseExceptPlugin::writeSessionConfig(KConfigGroup &config)
{
    KConfigGroup scg(&config, QStringLiteral("menu"));
    scg.writeEntry(QStringLiteral("ShowConfirmation"), m_show_confirmation_needed);
    scg.sync();
}
// END CloseExceptPlugin

// BEGIN CloseExceptPluginView
CloseExceptPluginView::CloseExceptPluginView(KTextEditor::MainWindow *mw, CloseExceptPlugin *plugin)
    : QObject(mw)
    , m_plugin(plugin)
    , m_show_confirmation_action(new KToggleAction(i18nc("@action:inmenu", "Show Confirmation"), this))
    , m_except_menu(new KActionMenu(i18nc("@action:inmenu close docs except the following...", "Close Except"), this))
    , m_like_menu(new KActionMenu(i18nc("@action:inmenu close docs like the following...", "Close Like"), this))
    , m_mainWindow(mw)
{
    KXMLGUIClient::setComponentName(QStringLiteral("katecloseexceptplugin"), i18n("Close Except/Like Plugin"));
    setXMLFile(QStringLiteral("ui.rc"));

    actionCollection()->addAction(QStringLiteral("file_close_except"), m_except_menu);
    actionCollection()->addAction(QStringLiteral("file_close_like"), m_like_menu);

    connect(KTextEditor::Editor::instance(), &KTextEditor::Editor::documentCreated, this, &CloseExceptPluginView::documentCreated);
    // Configure toggle action and connect it to update state
    m_show_confirmation_action->setChecked(m_plugin->showConfirmationNeeded());
    connect(m_show_confirmation_action.data(), &KToggleAction::toggled, m_plugin, &CloseExceptPlugin::toggleShowConfirmation);
    //
    connect(m_mainWindow, &KTextEditor::MainWindow::viewCreated, this, &CloseExceptPluginView::viewCreated);
    // Fill menu w/ currently opened document masks/groups
    updateMenu();

    m_mainWindow->guiFactory()->addClient(this);
}

CloseExceptPluginView::~CloseExceptPluginView()
{
    m_mainWindow->guiFactory()->removeClient(this);
}

void CloseExceptPluginView::viewCreated(KTextEditor::View *view)
{
    connectToDocument(view->document());
    updateMenu();
}

void CloseExceptPluginView::documentCreated(KTextEditor::Editor *, KTextEditor::Document *document)
{
    connectToDocument(document);
    updateMenu();
}

void CloseExceptPluginView::connectToDocument(KTextEditor::Document *document)
{
    // Subscribe self to document close and name changes
    connect(document, &KTextEditor::Document::aboutToClose, this, &CloseExceptPluginView::updateMenuSlotStub);
    connect(document, &KTextEditor::Document::documentNameChanged, this, &CloseExceptPluginView::updateMenuSlotStub);
    connect(document, &KTextEditor::Document::documentUrlChanged, this, &CloseExceptPluginView::updateMenuSlotStub);
}

void CloseExceptPluginView::updateMenuSlotStub(KTextEditor::Document *)
{
    updateMenu();
}

void CloseExceptPluginView::appendActionsFrom(const std::set<QUrl> &paths, actions_map_type &actions, KActionMenu *menu, CloseFunction closeFunction)
{
    for (const QUrl &path : paths) {
        QString action = path.path() + QLatin1Char('*');
        actions[action] = QPointer<QAction>(new QAction(action, menu));
        menu->addAction(actions[action]);
        connect(actions[action].data(), &QAction::triggered, this, [this, closeFunction, action]() {
            (this->*closeFunction)(action);
        });
    }
}

void CloseExceptPluginView::appendActionsFrom(const std::set<QString> &masks, actions_map_type &actions, KActionMenu *menu, CloseFunction closeFunction)
{
    for (const QString &mask : masks) {
        QString action = mask.startsWith(QLatin1Char('*')) ? mask : mask + QLatin1Char('*');
        actions[action] = QPointer<QAction>(new QAction(action, menu));
        menu->addAction(actions[action]);
        connect(actions[action].data(), &QAction::triggered, this, [this, closeFunction, action]() {
            (this->*closeFunction)(action);
        });
    }
}

void CloseExceptPluginView::updateMenu(const std::set<QUrl> &paths,
                                       const std::set<QString> &masks,
                                       actions_map_type &actions,
                                       KActionMenu *menu,
                                       CloseFunction closeFunction)
{
    // turn menu ON or OFF depending on collected results
    menu->setEnabled(!paths.empty());

    // Clear previous menus
    for (actions_map_type::iterator it = actions.begin(), last = actions.end(); it != last;) {
        menu->removeAction(*it);
        actions.erase(it++);
    }
    // Form a new one
    appendActionsFrom(paths, actions, menu, closeFunction);
    if (!masks.empty()) {
        if (!paths.empty()) {
            menu->addSeparator(); // Add separator between paths and file's ext filters
        }
        appendActionsFrom(masks, actions, menu, closeFunction);
    }
    // Append 'Show Confirmation' toggle menu item
    menu->addSeparator(); // Add separator between paths and show confirmation
    menu->addAction(m_show_confirmation_action);
}

void CloseExceptPluginView::updateMenu()
{
    const QList<KTextEditor::Document *> &docs = KTextEditor::Editor::instance()->application()->documents();
    if (docs.size() < 2) {
        // qDebug() << "No docs r (or the only) opened right now --> disable menu";
        m_except_menu->setEnabled(false);
        m_except_menu->addSeparator();
        m_like_menu->setEnabled(false);
        m_like_menu->addSeparator();
        /// \note It seems there is always a document present... it named \em 'Untitled'
    } else {
        // Iterate over documents and form a set of candidates
        typedef std::set<QUrl> paths_set_type;
        typedef std::set<QString> paths_set_type_masks;
        paths_set_type doc_paths;
        paths_set_type_masks masks;
        for (KTextEditor::Document *document : docs) {
            const QString &ext = QFileInfo(document->url().path()).completeSuffix();
            if (!ext.isEmpty()) {
                masks.insert(QStringLiteral("*.") + ext);
            }
            doc_paths.insert(KIO::upUrl(document->url()));
        }
        paths_set_type paths = doc_paths;
        // qDebug() << "stage #1: Collected" << paths.size() << "paths and" << masks.size() << "masks";
        // Add common paths to the collection
        for (paths_set_type::iterator it = doc_paths.begin(), last = doc_paths.end(); it != last; ++it) {
            for (QUrl url = *it; (!url.path().isEmpty()) && url.path() != QLatin1String("/"); url = KIO::upUrl(url)) {
                paths_set_type::iterator not_it = it;
                for (++not_it; not_it != last; ++not_it) {
                    if (!not_it->path().startsWith(url.path())) {
                        break;
                    }
                }
                if (not_it == last) {
                    paths.insert(url);
                    break;
                }
            }
        }
        // qDebug() << "stage #2: Collected" << paths.size() << "paths and" << masks.size() << "masks";
        //
        updateMenu(paths, masks, m_except_actions, m_except_menu, &CloseExceptPluginView::closeExcept);
        updateMenu(paths, masks, m_like_actions, m_like_menu, &CloseExceptPluginView::closeLike);
    }
}

void CloseExceptPluginView::close(const QString &item, const bool close_if_match)
{
    QChar asterisk = QLatin1Char('*');
    assert("Parameter seems invalid! Is smth has changed in the code?" && !item.isEmpty() && (item[0] == asterisk || item[item.size() - 1] == asterisk));

    const bool is_path = item[0] != asterisk;
    const QString mask = is_path ? item.left(item.size() - 1) : item;
    // qDebug() << "Going to close items [" << close_if_match << "/" << is_path << "]: " << mask;

    QList<KTextEditor::Document *> docs2close;
    const QList<KTextEditor::Document *> &docs = KTextEditor::Editor::instance()->application()->documents();
    for (KTextEditor::Document *document : docs) {
        const QString &path = KIO::upUrl(document->url()).path();
        /// \note Take a dot in account, so \c *.c would not match for \c blah.kcfgc
        const QString &ext = QLatin1Char('.') + QFileInfo(document->url().fileName()).completeSuffix();
        const bool match = (!is_path && mask.endsWith(ext)) || (is_path && path.startsWith(mask));
        if (match == close_if_match) {
            // qDebug() << "*** Will close: " << document->url();
            docs2close.push_back(document);
        }
    }
    if (docs2close.isEmpty()) {
        displayMessage(i18nc("@title:window", "Error"), i18nc("@info:tooltip", "No files to close ..."), KTextEditor::Message::Error);
        return;
    }
    // Show confirmation dialog if needed
    const bool removeNeeded =
        !m_plugin->showConfirmationNeeded() || CloseConfirmDialog(docs2close, m_show_confirmation_action, qobject_cast<QWidget *>(this)).exec();
    if (removeNeeded) {
        if (docs2close.isEmpty()) {
            displayMessage(i18nc("@title:window", "Error"), i18nc("@info:tooltip", "No files to close ..."), KTextEditor::Message::Error);
        } else {
            // Close 'em all!
            KTextEditor::Editor::instance()->application()->closeDocuments(docs2close);
            updateMenu();
            displayMessage(i18nc("@title:window", "Done"), i18np("%1 file closed", "%1 files closed", docs2close.size()), KTextEditor::Message::Positive);
        }
    }
}

void CloseExceptPluginView::displayMessage(const QString &title, const QString &msg, KTextEditor::Message::MessageType level)
{
    KTextEditor::View *kv = m_mainWindow->activeView();
    if (!kv) {
        return;
    }

    delete m_infoMessage;
    m_infoMessage = new KTextEditor::Message(xi18nc("@info", "<title>%1</title><nl/>%2", title, msg), level);
    m_infoMessage->setWordWrap(true);
    m_infoMessage->setPosition(KTextEditor::Message::TopInView);
    m_infoMessage->setAutoHide(5000);
    m_infoMessage->setAutoHideMode(KTextEditor::Message::Immediate);
    m_infoMessage->setView(kv);
    kv->document()->postMessage(m_infoMessage);
}

// END CloseExceptPluginView
} // namespace kate

#include "close_except_plugin.moc"

// kate: hl C++11/Qt4;

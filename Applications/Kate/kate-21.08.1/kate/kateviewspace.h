/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KATE_VIEWSPACE_H
#define KATE_VIEWSPACE_H

#include <ktexteditor/document.h>
#include <ktexteditor/modificationinterface.h>
#include <ktexteditor/view.h>

#include "katetabbar.h"

#include <QHash>
#include <QWidget>

class KConfigBase;
class KateViewManager;
class QStackedWidget;
class QToolButton;

class KateViewSpace : public QWidget
{
    Q_OBJECT

public:
    explicit KateViewSpace(KateViewManager *, QWidget *parent = nullptr, const char *name = nullptr);

    /**
     * Returns \e true, if this view space is currently the active view space.
     */
    bool isActiveSpace();

    /**
     * Depending on @p active, mark this view space as active or inactive.
     * Called from the view manager.
     */
    void setActive(bool active);

    /**
     * Create new view for given document
     * @param doc document to create view for
     * @return new created view
     */
    KTextEditor::View *createView(KTextEditor::Document *doc);
    void removeView(KTextEditor::View *v);

    bool showView(KTextEditor::View *view)
    {
        return showView(view->document());
    }
    bool showView(KTextEditor::Document *document);

    // might be nullptr, if there is no view
    KTextEditor::View *currentView();

    void saveConfig(KConfigBase *config, int myIndex, const QString &viewConfGrp);
    void restoreConfig(KateViewManager *viewMan, const KConfigBase *config, const QString &group);

    /**
     * Returns the document list of this tab bar.
     * @return document list in order of tabs
     */
    QVector<KTextEditor::Document *> documentList() const
    {
        return m_tabBar->documentList();
    }

    /**
     * Register one document for this view space.
     * Each registered document will get e.g. a tab bar button.
     */
    void registerDocument(KTextEditor::Document *doc);

    /**
     * Event filter to catch events from view space tool buttons.
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

    /**
     * Focus the previous tab in the tabbar.
     */
    void focusPrevTab();

    /**
     * Focus the next tab in the tabbar.
     */
    void focusNextTab();

    /** BEGIN Location History Stuff **/

    /**
     * go forward in location history
     */
    void goForward();

    /**
     * go back in location history
     */
    void goBack();

    /**
     * Is back history avail?
     */
    bool isHistoryBackEnabled() const;

    /**
     * Is forward history avail?
     */
    bool isHistoryForwardEnabled() const;

    /**
     * Add a jump location for jumping back and forth between history
     */
    void addPositionToHistory(const QUrl &url, KTextEditor::Cursor, bool calledExternally = false);

    /** END Location History Stuff **/

public Q_SLOTS:
    void documentDestroyed(QObject *doc);
    void updateDocumentName(KTextEditor::Document *doc);
    void updateDocumentUrl(KTextEditor::Document *doc);
    void updateDocumentState(KTextEditor::Document *doc);

private Q_SLOTS:
    void statusBarToggled();
    void tabBarToggled();
    void changeView(int buttonId);

    /**
     * Calls this slot to make this view space the currently active view space.
     * Making it active goes through the KateViewManager.
     * @param focusCurrentView if @e true, the current view will get focus
     */
    void makeActive(bool focusCurrentView = true);

    /**
     * This slot is called by the tabbar, if tab @p id was closed through the
     * context menu.
     */
    void closeTabRequest(int id);

    /**
     * This slot is called when the context menu is requested for button
     * @p id at position @p globalPos.
     * @param id the button, or -1 if the context menu was requested on
     *        at a place where no tab exists
     * @param globalPos the position of the context menu in global coordinates
     */
    void showContextMenu(int id, const QPoint &globalPos);

    /**
     * Called to create a new empty document.
     */
    void createNewDocument();

private:
    /**
     * Returns the amount of documents in KateDocManager that currently
     * have no tab in this tab bar.
     */
    int hiddenDocuments() const;

private:
    // Kate's view manager
    KateViewManager *m_viewManager;

    // config group string, used for restoring View session configuration
    QString m_group;

    // flag that indicates whether this view space is the active one.
    // correct setter: m_viewManager->setActiveSpace(this);
    bool m_isActiveSpace;

    // widget stack that contains all KTE::Views
    QStackedWidget *stack;

    // jump location history for this view-space
    struct Location {
        QUrl url;
        KTextEditor::Cursor cursor;
    };

    std::vector<Location> m_locations;
    size_t currentLocation = 0;

    /**
     * all documents this view space is aware of
     * depending on the limit of tabs, not all will have a corresponding
     * tab in the KateTabBar
     * these are stored in used order (MRU last)
     */
    QVector<KTextEditor::Document *> m_registeredDocuments;

    // the list of views that are contained in this view space,
    // mapped through a hash from Document to View.
    // note: the number of entries match stack->count();
    std::unordered_map<KTextEditor::Document *, KTextEditor::View *> m_docToView;

    // tab bar that contains viewspace tabs
    KateTabBar *m_tabBar;

    // split action
    QToolButton *m_split;

    // quick open action
    QToolButton *m_quickOpen;

    // go back in history button (only visible when the tab bar is visible)
    QToolButton *m_historyBack;

    // go forward in history button (only visible when the tab bar is visible)
    QToolButton *m_historyForward;
};

#endif

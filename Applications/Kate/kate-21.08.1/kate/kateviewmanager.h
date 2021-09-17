/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __KATE_VIEWMANAGER_H__
#define __KATE_VIEWMANAGER_H__

#include "katedocmanager.h"

#include <QList>
#include <QMap>
#include <QPointer>
#include <QSplitter>

#include <unordered_map>

namespace KActivities
{
class ResourceInstance;
}

namespace KTextEditor
{
class View;
class Document;
}

class KateDocumentInfo;

class KConfigGroup;
class KConfigBase;
class KateMainWindow;
class KateViewSpace;

class KateViewManager : public QSplitter
{
    Q_OBJECT

public:
    KateViewManager(QWidget *parentW, KateMainWindow *parent);
    ~KateViewManager() override;

private:
    /**
     * create all actions needed for the view manager
     */
    void setupActions();

    void updateViewSpaceActions();

public:
    /* This will save the splitter configuration */
    void saveViewConfiguration(KConfigGroup &group);

    /* restore it */
    void restoreViewConfiguration(const KConfigGroup &group);

    KTextEditor::Document *
    openUrl(const QUrl &url, const QString &encoding, bool activate = true, bool isTempFile = false, const KateDocumentInfo &docInfo = KateDocumentInfo());

    KTextEditor::Document *
    openUrls(const QList<QUrl> &url, const QString &encoding, bool isTempFile = false, const KateDocumentInfo &docInfo = KateDocumentInfo());

    KTextEditor::View *openUrlWithView(const QUrl &url, const QString &encoding);

public Q_SLOTS:
    void openUrl(const QUrl &url);
    void addPositionToHistory(const QUrl &url, KTextEditor::Cursor pos);

public:
    void closeView(KTextEditor::View *view);
    KateMainWindow *mainWindow();

private Q_SLOTS:
    void activateView(KTextEditor::View *view);
    void activateSpace(KTextEditor::View *v);

public Q_SLOTS:
    void slotDocumentNew();
    void slotDocumentOpen();
    void slotDocumentClose();
    void slotDocumentClose(KTextEditor::Document *document);

    void setActiveSpace(KateViewSpace *vs);
    void setActiveView(KTextEditor::View *view);

    void activateNextView();
    void activatePrevView();

Q_SIGNALS:
    void viewChanged(KTextEditor::View *);
    void viewCreated(KTextEditor::View *);

    void historyBackEnabled(bool e);
    void historyForwardEnabled(bool e);

public:
    /**
     * create and activate a new view for doc, if doc == 0, then
     * create a new document.
     * Can return NULL.
     */
    KTextEditor::View *createView(KTextEditor::Document *doc = nullptr, KateViewSpace *vs = nullptr);

private:
    bool deleteView(KTextEditor::View *view);

    void moveViewtoSplit(KTextEditor::View *view);
    void moveViewtoStack(KTextEditor::View *view);

    /* Save the configuration of a single splitter.
     * If child splitters are found, it calls it self with those as the argument.
     * If a viewspace child is found, it is asked to save its filelist.
     */
    QString saveSplitterConfig(QSplitter *s, KConfigBase *config, const QString &viewConfGrp);

    /** Restore a single splitter.
     * This is all the work is done for @see saveSplitterConfig()
     */
    void restoreSplitter(const KConfigBase *config, const QString &group, QSplitter *parent, const QString &viewConfGrp);

    void removeViewSpace(KateViewSpace *viewspace);

public:
    KTextEditor::View *activeView();
    KateViewSpace *activeViewSpace();

private Q_SLOTS:
    void slotViewChanged();

    void documentCreated(KTextEditor::Document *doc);
    void documentWillBeDeleted(KTextEditor::Document *doc);

    void documentSavedOrUploaded(KTextEditor::Document *document, bool saveAs);

    /**
     * This signal is emitted before the documents batch is going to be deleted
     *
     * note that the batch can be interrupted in the middle and only some
     * of the documents may be actually deleted. See documentsDeleted() signal.
     *
     * @param documents documents we want to delete, may not be deleted
     */
    void aboutToDeleteDocuments(const QList<KTextEditor::Document *> &documents);

    /**
     * This signal is emitted after the documents batch was deleted
     *
     * This is the batch closing signal for aboutToDeleteDocuments
     * @param documents the documents that weren't deleted after all
     */
    void documentsDeleted(const QList<KTextEditor::Document *> &documents);

public Q_SLOTS:
    /**
     * Splits a KateViewSpace into two in the following steps:
     * 1. create a QSplitter in the parent of the KateViewSpace to be split
     * 2. move the to-be-split KateViewSpace to the new splitter
     * 3. create new KateViewSpace and added to the new splitter
     * 4. create KateView to populate the new viewspace.
     * 5. The new KateView is made the active one, because createView() does that.
     * If no viewspace is provided, the result of activeViewSpace() is used.
     * The orientation of the new splitter is determined by the value of o.
     * Note: horizontal splitter means vertically aligned views.
     */
    void splitViewSpace(KateViewSpace *vs = nullptr, Qt::Orientation o = Qt::Horizontal);

    /**
     * Close the view space that contains the given view. If no view was
     * given, then the active view space will be closed instead.
     */
    void closeViewSpace(KTextEditor::View *view = nullptr);

    /**
     * @returns true of the two given views share the same view space.
     */
    bool viewsInSameViewSpace(KTextEditor::View *view1, KTextEditor::View *view2);

    /**
     * activate view for given document
     * @param doc document to activate view for
     */
    KTextEditor::View *activateView(KTextEditor::Document *doc);

    /** Splits the active viewspace horizontally */
    void slotSplitViewSpaceHoriz()
    {
        splitViewSpace(nullptr, Qt::Vertical);
    }

    /** Splits the active viewspace vertically */
    void slotSplitViewSpaceVert()
    {
        splitViewSpace();
    }

    /**  moves the splitter according to the key that has been pressed */
    void moveSplitter(Qt::Key key, int repeats = 1);

    /** moves the splitter to the right  */
    void moveSplitterRight()
    {
        moveSplitter(Qt::Key_Right);
    }

    /** moves the splitter to the left  */
    void moveSplitterLeft()
    {
        moveSplitter(Qt::Key_Left);
    }

    /** moves the splitter up  */
    void moveSplitterUp()
    {
        moveSplitter(Qt::Key_Up);
    }

    /** moves the splitter down  */
    void moveSplitterDown()
    {
        moveSplitter(Qt::Key_Down);
    }

    /** closes the current view space. */
    void slotCloseCurrentViewSpace()
    {
        closeViewSpace();
    }

    /** closes every view but the active one */
    void slotCloseOtherViews();

    /** hide every view but the active one */
    void slotHideOtherViews(bool hideOthers);

    void reactivateActiveView();

    /**
     * Toogle the orientation of current split view
     */
    void toggleSplitterOrientation();

    /**
     * Get a list of all views.
     * @return all views
     */
    QList<KTextEditor::View *> views() const
    {
        QList<KTextEditor::View *> ret;
        ret.reserve(m_views.size());
        std::transform(m_views.begin(), m_views.end(), std::back_inserter(ret), [](const std::pair<KTextEditor::View *, ViewData> &p) {
            return p.first;
        });
        return ret;
    }

    /**
     * get views in lru order
     * @return views in lru order
     */
    std::vector<KTextEditor::View *> sortedViews() const
    {
        std::vector<std::pair<KTextEditor::View *, qint64>> sorted;

        // extract into a list
        std::transform(m_views.begin(), m_views.end(), std::back_inserter(sorted), [](const std::pair<KTextEditor::View *, ViewData> &p) {
            return std::pair<KTextEditor::View *, qint64>{p.first, p.second.lruAge};
        });
        // sort the views based on lru
        std::sort(sorted.begin(), sorted.end(), [](const std::pair<KTextEditor::View *, qint64> &l, const std::pair<KTextEditor::View *, qint64> &r) {
            return l.second < r.second;
        });

        // extract the views only and return
        std::vector<KTextEditor::View *> ret;
        ret.reserve(sorted.size());
        std::transform(sorted.begin(), sorted.end(), std::back_inserter(ret), [](const std::pair<KTextEditor::View *, qint64> &p) {
            return p.first;
        });
        return ret;
    }

private:
    KateMainWindow *m_mainWindow;
    bool m_init;

    QAction *m_splitViewVert = nullptr;
    QAction *m_splitViewHoriz = nullptr;
    QAction *m_closeView = nullptr;
    QAction *m_closeOtherViews = nullptr;
    QAction *m_toggleSplitterOrientation = nullptr;
    QAction *m_hideOtherViews = nullptr;
    QAction *goNext = nullptr;
    QAction *goPrev = nullptr;

    std::vector<KateViewSpace *> m_viewSpaceList;

    bool m_blockViewCreationAndActivation;

    bool m_activeViewRunning;

    int m_splitterIndex = 0; // used during saving splitter config.

    /**
     * View meta data
     */
    class ViewData
    {
    public:
        /**
         * view active?
         */
        bool active = false;

        /**
         * lru age of the view
         * important: smallest age ==> latest used view
         */
        qint64 lruAge = 0;

        /**
         * activity resource for the view
         */
        KActivities::ResourceInstance *activityResource = nullptr;
    };

    /**
     * central storage of all views known in the view manager
     * maps the view to meta data
     */
    std::unordered_map<KTextEditor::View *, ViewData> m_views;

    /**
     * current minimal age
     */
    qint64 m_minAge;

    /**
     * the view that is ATM merged to the xml gui factory
     */
    QPointer<KTextEditor::View> m_guiMergedView;

    /**
     * last url of open file dialog, used if current document has no valid url
     */
    QUrl m_lastOpenDialogUrl;
};

#endif

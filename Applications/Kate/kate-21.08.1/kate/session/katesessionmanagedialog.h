/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __KATE_SESSION_MANAGE_DIALOG_H__
#define __KATE_SESSION_MANAGE_DIALOG_H__

#include <QDialog>

class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class KateSessionChooserItem;

#include "katesession.h"
#include "ui_katesessionmanagedialog.h"

class KateSessionManageDialog : public QDialog, public Ui::KateSessionManageDialogUi
{
    Q_OBJECT

public:
    /**
     * The normal ctor for manage mode
     */
    KateSessionManageDialog(QWidget *parent);

    /**
     * The special ctor for chooser mode
     * Set a different window title, enables some extra widget and try to select
     * the @p lastSession in the session list.
     */
    KateSessionManageDialog(QWidget *parent, const QString &lastSession);

protected Q_SLOTS:
    /**
     * Re-implemented to save in chooser mode users choice when needed and to
     * exit the dialog with a return code of @c 0/1 fitting to the code of
     * @p result to indicate that the user chose a session to open not.
     * @see KateSessionManager::chooseSession()
     * @param result has to be one of enum @c ResultCode
     */
    void done(int result) override;

    /**
     * To update the button states
     */
    void selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    /**
     * Close the dialog and open the selected session
     */
    void openSession();

    /**
     * Use the selected session as template for a new session
     */
    void openSessionAsTemplate();

    /**
     * Open new anonymous session
     */
    void openNewSession();

    /**
     * Copy the selected session
     */
    void copySession();

    /**
     * Try to rename the session hold by @c m_editByUser
     * @see  editDone(), editBegin(), m_editByUser
     */
    void editApply();

    /**
     * Open the inline editor on the selected session, set @c m_editByUser to
     * the selected session and connect @c QTreeWidget::itemChanged signal to
     * @c editApply().
     * @see editDone(), editApply(), m_editByUser
     */
    void editBegin();

    /**
     * Finish the edit process, reset intern settings.
     * Calling this function without to call @c editApply() will abort edit.
     */
    void editDone();

    /**
     * To close the dialog
     */
    void closeDialog();

    /**
     * Slot for the delete button
     * @see m_deleteList, deleteSessions()
     */
    void updateDeleteList();

    /**
     * Update the list of sessions in @c m_sessionList and trigger some needed
     * actions belong to the editing of session names.
     */
    void updateSessionList();

    /**
     * To enable/disable not useful buttons
     */
    void dontAskToggled();

    /**
     * Slot for @c m_filterField
     */
    void filterChanged();

private:
    /**
     * Result codes used to call @c done()
     */
    enum ResultCode {
        ResultQuit = QDialog::Rejected,
        ResultOpen,
        ResultNew,
    };

    /**
     * Re-implemented to avoid crash when in edit state
     */
    void closeEvent(QCloseEvent *event) override;

    /**
     * Disables all buttons on the "normal" button stack page and the close button
     */
    void disableButtons();

    /**
     * To handle the rename process
     */
    bool eventFilter(QObject *object, QEvent *event) override;

    /**
     * @return current selected item in @c m_sessionList or @c nullptr
     */
    KateSessionChooserItem *currentSessionItem() const;

    /**
     * @return current selected session in @c m_sessionList or empty @c KateSession::Ptr()
     */
    KateSession::Ptr currentSelectedSession() const;

    /**
     * Display @p item in a striking way to indicate that the session represent
     * by @p item will be deleted
     */
    void markItemAsToBeDeleted(QTreeWidgetItem *item);

    /**
     * The item which is currently edited by the user or @c nullptr to indicate
     * that nothing is on edit.
     */
    KateSessionChooserItem *m_editByUser = nullptr;

    /**
     * Used by @c updateSessionList() to choose a new current item
     */
    QString m_preferredSession;

    /**
     * Used in dtor to do some savings or not
     */
    bool m_chooserMode = false;

    /**
     * Will filled with sessions to be deleted by @c updateDeleteList() and process
     * by @c deleteSessions()
     */
    QSet<KateSession::Ptr> m_deleteList;
};

#endif

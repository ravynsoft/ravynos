/*
    SPDX-FileCopyrightText: 2021 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_OUTPUT_VIEW_H
#define KATE_OUTPUT_VIEW_H

#include <QLineEdit>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QWidget>

class KateMainWindow;
class KateOutputTreeView;
class QSortFilterProxyModel;

/**
 * Widget to output stuff e.g. for plugins.
 */
class KateOutputView : public QWidget
{
    Q_OBJECT

public:
    /**
     * Construct new output, we do that once per main window
     * @param mainWindow parent main window
     * @param parent parent widget (e.g. the tool view in the main window)
     */
    KateOutputView(KateMainWindow *mainWindow, QWidget *parent);

public Q_SLOTS:
    /**
     * Read and apply our configuration.
     */
    void readConfig();

    /**
     * slot for incoming messages
     * @param message incoming message we shall handle
     *
     * details of message format:
     *
     * IMPORTANT: at the moment, most stuff is key/value with strings, if the format is more finalized, one can introduce better type for more efficiency/type
     * safety
     *
     * message text, will be trimmed before output
     *
     *    message["text"] = i18n("your cool message")
     *
     * the text will be split in lines, all lines beside the first can be collapsed away
     *
     * message type, we support at the moment
     *
     *    message["type"] = "Error"
     *    message["type"] = "Warning"
     *    message["type"] = "Info"
     *    message["type"] = "Log"
     *
     * this is take from https://microsoft.github.io/language-server-protocol/specification#window_showMessage MessageType of LSP
     *
     * will lead to appropriate icons/... in the output view
     *
     * a message should have some category, like Git, LSP, ....
     *
     *    message["category"] = i18n(...)
     *
     * will be used to allow the user to filter for
     *
     * one can additionally provide a categoryIcon
     *
     *    message["categoryIcon"] = QIcon(...)
     *
     * the categoryIcon icon QVariant must contain a QIcon, nothing else!
     *
     */
    void slotMessage(const QVariantMap &message);

private:
    /**
     * the main window we belong to
     * each main window has exactly one KateOutputView
     */
    KateMainWindow *const m_mainWindow = nullptr;

    /**
     * Internal tree view to display the messages we get
     */
    KateOutputTreeView *m_messagesTreeView = nullptr;

    /**
     * Our message model, at the moment a standard item model
     */
    QStandardItemModel m_messagesModel;

    /**
     * Our proxy model for filtering
     */
    QSortFilterProxyModel *m_proxyModel = nullptr;

    /**
     * fuzzy filter line edit
     */
    QLineEdit m_filterLine;

    /**
     * When to show output view
     * 0 => never
     * 1 => on error
     * 2 => on warning or above
     * 3 => on info or above
     * 4 => on log or above
     */
    int m_showOutputViewForMessageType = 1;
};

#endif

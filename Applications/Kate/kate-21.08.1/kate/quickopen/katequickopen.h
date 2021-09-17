/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2007, 2009 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_QUICK_OPEN_H
#define KATE_QUICK_OPEN_H

#include <QMenu>

#include "katequickopenlineedit.h"

class KateMainWindow;

class QModelIndex;
class QStandardItemModel;
class QSortFilterProxyModel;
class QuickOpenStyleDelegate;
class QTreeView;
class KateQuickOpenModel;
enum KateQuickOpenModelList : int;

class QuickOpenFilterProxyModel;

class KateQuickOpen : public QMenu
{
    Q_OBJECT
public:
    KateQuickOpen(KateMainWindow *mainWindow);

    /**
     * update state
     * will fill model with current open documents, project documents, ...
     */
    void updateState();
    void updateViewGeometry();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private Q_SLOTS:
    void reselectFirst();

    /**
     * Return pressed, activate the selected document
     * and go back to background
     */
    void slotReturnPressed();

    void slotListModeChanged(KateQuickOpenModelList mode);

private:
    KateMainWindow *m_mainWindow;
    QTreeView *m_listView;
    QuickOpenLineEdit *m_inputLine;
    QuickOpenStyleDelegate *m_styleDelegate;

    /**
     * our model we search in
     */
    KateQuickOpenModel *m_base_model;

    /**
     * filtered model we search in
     */
    QuickOpenFilterProxyModel *m_model;
};

#endif

/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2020 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef QUICKOPENLINEEDIT_H
#define QUICKOPENLINEEDIT_H

#include <QLineEdit>
#include <memory>

#include "katequickopenmodel.h"

class QuickOpenLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit QuickOpenLineEdit(QWidget *parent);
    ~QuickOpenLineEdit();

    KateQuickOpenModelList listMode() const
    {
        return m_listMode;
    }

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void setupMenu();

private:
    std::unique_ptr<QMenu> menu;
    KateQuickOpenModelList m_listMode;

Q_SIGNALS:
    void listModeChanged(KateQuickOpenModelList mode);
};

#endif // QUICKOPENLINEEDIT_H

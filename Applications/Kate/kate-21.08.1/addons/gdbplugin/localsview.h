//
// Description: Widget that local variables of the gdb inferior
//
// SPDX-FileCopyrightText: 2010 Kåre Särs <kare.sars@iki.fi>
//
//  SPDX-License-Identifier: LGPL-2.0-only

#ifndef LOCALSVIEW_H
#define LOCALSVIEW_H

#include <QTreeWidget>
#include <QTreeWidgetItem>

class LocalsView : public QTreeWidget
{
    Q_OBJECT
public:
    LocalsView(QWidget *parent = nullptr);
    ~LocalsView() override;

public Q_SLOTS:
    // An empty value string ends the locals
    void addLocal(const QString &vString);
    void addStruct(QTreeWidgetItem *parent, const QString &vString);
    void addArray(QTreeWidgetItem *parent, const QString &vString);

Q_SIGNALS:
    void localsVisible(bool visible);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void createWrappedItem(QTreeWidgetItem *parent, const QString &name, const QString &value);
    void createWrappedItem(QTreeWidget *parent, const QString &name, const QString &value);
    bool m_allAdded = true;
    QString m_local;
};

#endif

#ifndef GOTOSYMBOLTREEVIEW_H
#define GOTOSYMBOLTREEVIEW_H
/*
    SPDX-FileCopyrightText: 2020 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <QTreeView>

namespace KTextEditor
{
class MainWindow;
}

class GotoSymbolTreeView : public QTreeView
{
    Q_OBJECT

public:
    GotoSymbolTreeView(KTextEditor::MainWindow *mainWindow, QWidget *parent = nullptr);
    int sizeHintWidth() const;
    void setGlobalMode(bool value)
    {
        globalMode = value;
    }

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

private:
    KTextEditor::MainWindow *m_mainWindow;
    bool globalMode = false;
};

#endif // GOTOSYMBOLTREEVIEW_H

/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef LSPGOTOSYMBOLDIALOG_H
#define LSPGOTOSYMBOLDIALOG_H

#include <quickdialog.h>

#include "lspclientprotocol.h"

class QStandardItemModel;
class LSPClientServer;

namespace KTextEditor
{
class MainWindow;
}

class GotoSymbolHUDDialog : public QuickDialog
{
public:
    GotoSymbolHUDDialog(KTextEditor::MainWindow *mainWindow, QSharedPointer<LSPClientServer> server);

    void openDialog();

protected Q_SLOTS:
    void slotReturnPressed() override final;

private:
    void slotTextChanged(const QString &text);
    QIcon iconForSymbolKind(LSPSymbolKind kind) const;
    void setPaletteToEditorColors();

    QStandardItemModel *model = nullptr;
    KTextEditor::MainWindow *mainWindow;
    QSharedPointer<LSPClientServer> server;

    const QIcon m_icon_pkg = QIcon::fromTheme(QStringLiteral("code-block"));
    const QIcon m_icon_class = QIcon::fromTheme(QStringLiteral("code-class"));
    const QIcon m_icon_typedef = QIcon::fromTheme(QStringLiteral("code-typedef"));
    const QIcon m_icon_function = QIcon::fromTheme(QStringLiteral("code-function"));
    const QIcon m_icon_var = QIcon::fromTheme(QStringLiteral("code-variable"));
};

#endif

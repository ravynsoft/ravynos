/*
    SPDX-FileCopyrightText: 2020 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef GOTOSYMBOLWIDGET_H
#define GOTOSYMBOLWIDGET_H

#include <KTextEditor/Cursor>
#include <QWidget>

class GotoSymbolTreeView;
class GotoSymbolModel;
class QLineEdit;
class QuickOpenFilterProxyModel;
class QTreeView;
class GotoGlobalSymbolModel;
class KateCTagsView;
class GotoStyleDelegate;

namespace KTextEditor
{
class MainWindow;
}

class GotoSymbolWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GotoSymbolWidget(KTextEditor::MainWindow *mainWindow, KateCTagsView *pluginView, QWidget *parent = nullptr);

    bool eventFilter(QObject *watched, QEvent *event) override;
    void updateViewGeometry();
    void showSymbols(const QString &filePath);
    void showGlobalSymbols(const QString &tagFilePath);
    void loadGlobalSymbols(const QString &text);
    void reselectFirst();

    enum Mode { Global, Local };

private Q_SLOTS:
    void slotReturnPressed();

private:
    void changeMode(Mode newMode);

private:
    Mode mode;
    KateCTagsView *ctagsPluginView;
    GotoStyleDelegate *m_styleDelegate;
    KTextEditor::MainWindow *m_mainWindow;
    GotoSymbolTreeView *m_treeView;
    QuickOpenFilterProxyModel *m_proxyModel;
    GotoSymbolModel *m_symbolsModel;
    GotoGlobalSymbolModel *m_globalSymbolsModel;
    QLineEdit *m_lineEdit;
    KTextEditor::Cursor oldPos;
    QString m_tagFile;
};

#endif // GOTOSYMBOLWIDGET_H

/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>

    SPDX-License-Identifier: MIT
*/

#ifndef LSPCLIENTCONFIGPAGE_H
#define LSPCLIENTCONFIGPAGE_H

#include <KTextEditor/ConfigPage>

class LSPClientPlugin;

namespace Ui
{
class LspConfigWidget;
}

class LSPClientConfigPage : public KTextEditor::ConfigPage
{
    Q_OBJECT

public:
    explicit LSPClientConfigPage(QWidget *parent = nullptr, LSPClientPlugin *plugin = nullptr);
    ~LSPClientConfigPage() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public Q_SLOTS:
    void apply() override;
    void defaults() override;
    void reset() override;
    void configTextChanged();
    void configUrlChanged();
    void updateHighlighters();

private:
    void readUserConfig(const QString &fileName);
    void updateConfigTextErrorState();

    Ui::LspConfigWidget *ui;
    LSPClientPlugin *m_plugin;
};

#endif

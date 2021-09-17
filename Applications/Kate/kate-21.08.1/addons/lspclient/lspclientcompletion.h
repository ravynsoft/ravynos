/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>

    SPDX-License-Identifier: MIT
*/

#ifndef LSPCLIENTCOMPLETION_H
#define LSPCLIENTCOMPLETION_H

#include "lspclientserver.h"
#include "lspclientservermanager.h"

#include <KTextEditor/CodeCompletionModel>
#include <KTextEditor/CodeCompletionModelControllerInterface>

class LSPClientCompletion : public KTextEditor::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface
{
    Q_OBJECT

    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)

public:
    // implementation factory method
    static LSPClientCompletion *new_(QSharedPointer<LSPClientServerManager> manager);

    LSPClientCompletion(QObject *parent)
        : KTextEditor::CodeCompletionModel(parent)
    {
    }

    virtual void setServer(QSharedPointer<LSPClientServer> server) = 0;
    virtual void setSelectedDocumentation(bool) = 0;
    virtual void setSignatureHelp(bool) = 0;
    virtual void setCompleteParens(bool) = 0;
};

#endif

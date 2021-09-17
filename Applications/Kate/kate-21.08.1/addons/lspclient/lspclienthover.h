/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>
    SPDX-FileCopyrightText: 2019 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef LSPCLIENTHOVER_H
#define LSPCLIENTHOVER_H

class LSPClientServerManager;
class LSPClientServer;

#include <KTextEditor/TextHintInterface>

class LSPClientHover : public QObject, public KTextEditor::TextHintProvider
{
    Q_OBJECT

public:
    // implementation factory method
    static LSPClientHover *new_(QSharedPointer<LSPClientServerManager> manager);

    LSPClientHover()
        : KTextEditor::TextHintProvider()
    {
    }

    virtual void setServer(QSharedPointer<LSPClientServer> server) = 0;

    // support additional parameters besides the usual interface signature
    virtual QString showTextHint(KTextEditor::View *view, const KTextEditor::Cursor &position, bool manual) = 0;
};

#endif

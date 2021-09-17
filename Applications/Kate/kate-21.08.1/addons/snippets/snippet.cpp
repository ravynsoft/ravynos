/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
 *  SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "snippet.h"
#include "katesnippetglobal.h"
#include "ktexteditor/application.h"
#include "ktexteditor/editor.h"
#include "ktexteditor/mainwindow.h"

#include <KActionCollection>
#include <KColorScheme>
#include <KLocalizedString>

#include <QAction>

Snippet::Snippet()
    : QStandardItem(i18n("<empty snippet>"))
{
    setIcon(QIcon::fromTheme(QStringLiteral("text-plain")));
}

Snippet::~Snippet()
{
    delete m_action;
}

QString Snippet::snippet() const
{
    return m_snippet;
}

void Snippet::setSnippet(const QString &snippet)
{
    m_snippet = snippet;
}

void Snippet::registerActionForView(QWidget *view)
{
    if (view->actions().contains(m_action)) {
        return;
    }
    view->addAction(m_action);
}

QAction *Snippet::action()
{
    /// TODO: this is quite ugly, or is it? if someone knows how to do it better, please refactor
    if (!m_action) {
        static int actionCount = 0;
        actionCount += 1;
        m_action = new QAction(QStringLiteral("insertSnippet%1").arg(actionCount), KateSnippetGlobal::self());
        m_action->setData(QVariant::fromValue<Snippet *>(this));
        KateSnippetGlobal::self()->connect(m_action, &QAction::triggered, KateSnippetGlobal::self(), &KateSnippetGlobal::insertSnippetFromActionData);
    }
    m_action->setText(i18n("insert snippet %1", text()));
    return m_action;
}

QVariant Snippet::data(int role) const
{
    if (role == Qt::ToolTipRole) {
        return m_snippet;
    } else if ((role == Qt::ForegroundRole || role == Qt::BackgroundRole) && parent()->checkState() != Qt::Checked) {
        /// TODO: make the selected items also "disalbed" so the toggle action is seen directly
        KColorScheme scheme(QPalette::Disabled, KColorScheme::View);
        if (role == Qt::ForegroundRole) {
            return scheme.foreground(KColorScheme::NormalText).color();
        } else {
            return scheme.background(KColorScheme::NormalBackground).color();
        }
    }
    return QStandardItem::data(role);
}

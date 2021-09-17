/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Joseph Wenninger <jowenn@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectinfoviewnotes.h"
#include "kateproject.h"
#include "kateprojectpluginview.h"

#include <QVBoxLayout>

KateProjectInfoViewNotes::KateProjectInfoViewNotes(KateProjectPluginView *pluginView, KateProject *project)
    : m_pluginView(pluginView)
    , m_project(project)
    , m_edit(new QPlainTextEdit())
{
    /*
     * layout widget
     */
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->addWidget(m_edit);
    setLayout(layout);
    m_edit->setDocument(project->notesDocument());
    setFocusProxy(m_edit);
}

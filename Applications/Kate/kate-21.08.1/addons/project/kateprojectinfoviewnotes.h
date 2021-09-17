/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Joseph Wenninger <jowenn@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_PROJECT_INFO_VIEW_NOTES_H
#define KATE_PROJECT_INFO_VIEW_NOTES_H

#include <QPlainTextEdit>

class KateProjectPluginView;
class KateProject;

/**
 * Class representing a view of a project.
 * A tree like view of project content.
 */
class KateProjectInfoViewNotes : public QWidget
{
    Q_OBJECT

public:
    /**
     * construct project info view for given project
     * @param pluginView our plugin view
     * @param project project this view is for
     */
    KateProjectInfoViewNotes(KateProjectPluginView *pluginView, KateProject *project);

    /**
     * our project.
     * @return project
     */
    KateProject *project() const
    {
        return m_project;
    }

private:
    /**
     * our plugin view
     */
    KateProjectPluginView *m_pluginView;

    /**
     * our project
     */
    KateProject *m_project;

    /**
     * edit widget bound to notes document of project
     */
    QPlainTextEdit *m_edit;
};

#endif

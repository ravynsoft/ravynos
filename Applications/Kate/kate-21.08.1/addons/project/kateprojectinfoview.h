/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_PROJECT_INFO_VIEW_H
#define KATE_PROJECT_INFO_VIEW_H

#include <QTabWidget>

class KateProjectPluginView;
class KateProject;
class KateProjectInfoViewTerminal;

/**
 * Class representing a view of a project.
 * A tree like view of project content.
 */
class KateProjectInfoView : public QTabWidget
{
    Q_OBJECT

public:
    /**
     * construct project info view for given project
     * @param pluginView our plugin view
     * @param project project this view is for
     */
    KateProjectInfoView(KateProjectPluginView *pluginView, KateProject *project);

    /**
     * deconstruct info view
     */
    ~KateProjectInfoView() override;

    /**
     * our project.
     * @return project
     */
    KateProject *project() const
    {
        return m_project;
    }

    void showEvent(QShowEvent *) override;

    /**
     * Shall the ESC key press be ignored?
     * If not, the toolview will be hidden.
     * @return ignore ESC shortcut?
     */
    bool ignoreEsc() const;

    void resetTerminal(const QString &directory);

private:
    /**
     * our plugin view
     */
    KateProjectPluginView *m_pluginView;

    /**
     * our project
     */
    KateProject *m_project;

    KateProjectInfoViewTerminal *m_terminal;
};

#endif

/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2010 Thomas Fjellstrom <thomas@fjellstrom.ca>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_FILETREE_PLUGIN_SETTINGS_H
#define KATE_FILETREE_PLUGIN_SETTINGS_H

#include <KConfigGroup>
#include <QColor>

// Geh, should make this into a proper class, setters and getters.
class KateFileTreePluginSettings
{
public:
    KateFileTreePluginSettings();

    void save();

    bool shadingEnabled() const;
    void setShadingEnabled(bool);

    const QColor &viewShade() const;
    void setViewShade(const QColor &);

    const QColor &editShade() const;
    void setEditShade(const QColor &);

    bool listMode() const;
    void setListMode(bool);

    int sortRole() const;
    void setSortRole(int);

    bool showFullPathOnRoots() const;
    void setShowFullPathOnRoots(bool);

    bool showToolbar() const;
    void setShowToolbar(bool);

    bool showCloseButton() const;
    void setShowCloseButton(bool);

private:
    KConfigGroup m_group;

    bool m_shadingEnabled;
    QColor m_viewShade;
    QColor m_editShade;

    bool m_listMode;
    int m_sortRole;

    bool m_showFullPathOnRoots;
    bool m_showToolbar;
    bool m_showCloseButton;
};

#endif // KATE_FILETREE_PLUGIN_H

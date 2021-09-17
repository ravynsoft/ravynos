/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
 *  SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __SNIPPET_H__
#define __SNIPPET_H__

#include <QStandardItem>

class QAction;

/**
 * One object of this class represents a single snippet.
 * Multiple snippets are stored in one repository (XML-file).
 *
 * To access the snippet's name (which should also be used for matching
 * during code completion) use @p QStandardItem::text().
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 * @author Milian Wolff <mail@milianw.de>
 */
class Snippet : public QStandardItem
{
public:
    /**
     * Construct an empty snippet.
     */
    Snippet();
    ~Snippet() override;

    /**
     * Returns the actual contents of this snippet.
     */
    QString snippet() const;
    /**
     * Sets the actual contents of this snippet.
     */
    void setSnippet(const QString &snippet);

    /**
     * Action to trigger insertion of this snippet.
     */
    QAction *action();

    void registerActionForView(QWidget *view);

    QVariant data(int role = Qt::UserRole + 1) const override;

private:
    /// the actual snippet contents aka \code<fillin>\endcode
    QString m_snippet;
    /// the insertion action for this snippet.
    QAction *m_action = nullptr;
};

Q_DECLARE_METATYPE(Snippet *)

#endif

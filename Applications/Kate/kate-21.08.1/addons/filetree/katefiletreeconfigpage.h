/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001, 2006 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001, 2007 Anders Lund <anders@alweb.dk>
   SPDX-FileCopyrightText: 2010 Thomas Fjellstrom <thomas@fjellstrom.ca>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_FILETREE_CONFIGPAGE_H
#define KATE_FILETREE_CONFIGPAGE_H

#include <QWidget>

#include <ktexteditor/configpage.h>

class KateFileTreePlugin;
class QComboBox;

class KateFileTreeConfigPage : public KTextEditor::ConfigPage
{
    Q_OBJECT
public:
    explicit KateFileTreeConfigPage(QWidget *parent = nullptr, KateFileTreePlugin *plug = nullptr);
    ~KateFileTreeConfigPage() override
    {
    }

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public Q_SLOTS:
    void apply() override;
    void defaults() override;
    void reset() override;

    // Q_SIGNALS:
    //  void changed();

private Q_SLOTS:
    void slotMyChanged();

private:
    class QGroupBox *gbEnableShading;
    class KColorButton *kcbViewShade, *kcbEditShade;
    class QLabel *lEditShade, *lViewShade, *lSort, *lMode;
    QComboBox *cmbSort, *cmbMode;
    class QCheckBox *cbShowFullPath;
    class QCheckBox *cbShowToolbar;
    class QCheckBox *cbShowClose;
    KateFileTreePlugin *m_plug;

    bool m_changed = false;
};

#endif /* KATE_FILETREE_CONFIGPAGE_H */

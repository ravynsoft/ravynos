/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001, 2007 Anders Lund <anders@alweb.dk>
   SPDX-FileCopyrightText: 2009 Abhishek Patil <abhishekworld@gmail.com>
   SPDX-FileCopyrightText: 2010 Thomas Fjellstrom <thomas@fjellstrom.ca>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*
Config stuff plan:
-----------------

main default config is stored in KSharedConfig::openConfig()+":filetree"
when main config is set, it needs to tell view's to delete
existing customized settings, and use the global ones (somehow)
(maybe some kind of "customized" flag?)

view needs to pull default settings from the main plugin config

*/

#include "katefiletreeconfigpage.h"
#include "katefiletreedebug.h"
#include "katefiletreemodel.h"
#include "katefiletreeplugin.h"
#include "katefiletreeproxymodel.h"

#include <KColorButton>
#include <KLocalizedString>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

KateFileTreeConfigPage::KateFileTreeConfigPage(QWidget *parent, KateFileTreePlugin *fl)
    : KTextEditor::ConfigPage(parent)
    , m_plug(fl)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    gbEnableShading = new QGroupBox(i18n("Background Shading"), this);
    gbEnableShading->setCheckable(true);
    layout->addWidget(gbEnableShading);

    QGridLayout *lo = new QGridLayout(gbEnableShading);

    kcbViewShade = new KColorButton(gbEnableShading);
    lViewShade = new QLabel(i18n("&Viewed documents' shade:"), gbEnableShading);
    lViewShade->setBuddy(kcbViewShade);
    lo->addWidget(lViewShade, 2, 0);
    lo->addWidget(kcbViewShade, 2, 1);

    kcbEditShade = new KColorButton(gbEnableShading);
    lEditShade = new QLabel(i18n("&Modified documents' shade:"), gbEnableShading);
    lEditShade->setBuddy(kcbEditShade);
    lo->addWidget(lEditShade, 3, 0);
    lo->addWidget(kcbEditShade, 3, 1);

    // sorting
    QHBoxLayout *lo2 = new QHBoxLayout;
    layout->addLayout(lo2);
    lSort = new QLabel(i18n("&Sort by:"), this);
    lo2->addWidget(lSort);
    cmbSort = new QComboBox(this);
    lo2->addWidget(cmbSort);
    lSort->setBuddy(cmbSort);
    cmbSort->addItem(i18n("Opening Order"), static_cast<int>(KateFileTreeModel::OpeningOrderRole));
    cmbSort->addItem(i18n("Document Name"), static_cast<int>(Qt::DisplayRole));
    cmbSort->addItem(i18n("Url"), static_cast<int>(KateFileTreeModel::PathRole));

    // view mode
    QHBoxLayout *lo3 = new QHBoxLayout;
    layout->addLayout(lo3);
    lMode = new QLabel(i18n("&View Mode:"), this);
    lo3->addWidget(lMode);
    cmbMode = new QComboBox(this);
    lo3->addWidget(cmbMode);
    lMode->setBuddy(cmbMode);
    cmbMode->addItem(i18n("Tree View"), QVariant(false));
    cmbMode->addItem(i18n("List View"), QVariant(true));

    // Show Full Path on Roots?
    QHBoxLayout *lo4 = new QHBoxLayout;
    layout->addLayout(lo4);
    cbShowFullPath = new QCheckBox(i18n("&Show Full Path"), this);
    lo4->addWidget(cbShowFullPath);

    QHBoxLayout *lo5 = new QHBoxLayout;
    layout->addLayout(lo5);
    cbShowToolbar = new QCheckBox(i18n("Show &Toolbar"), this);
    lo5->addWidget(cbShowToolbar);

    cbShowClose = new QCheckBox(i18n("Show Close Button"), this);
    layout->addWidget(cbShowClose);
    layout->addWidget(new QLabel(i18n("When enabled, this will show a close button for opened documents on hover.")));

    layout->insertStretch(-1, 10);

    gbEnableShading->setWhatsThis(
        i18n("When background shading is enabled, documents that have been viewed "
             "or edited within the current session will have a shaded background. "
             "The most recent documents have the strongest shade."));
    kcbViewShade->setWhatsThis(i18n("Set the color for shading viewed documents."));
    kcbEditShade->setWhatsThis(
        i18n("Set the color for modified documents. This color is blended into "
             "the color for viewed files. The most recently edited documents get "
             "most of this color."));

    cbShowFullPath->setWhatsThis(
        i18n("When enabled, in tree mode, top level folders will show up with their full path "
             "rather than just the last folder name."));

    cbShowToolbar->setWhatsThis(i18n("When enabled, a toolbar with actions like “Save” are displayed above the list of documents."));

    //   cmbSort->setWhatsThis( i18n(
    //       "Set the sorting method for the documents.") );

    reset();

    connect(gbEnableShading, &QGroupBox::toggled, this, &KateFileTreeConfigPage::slotMyChanged);
    connect(kcbViewShade, &KColorButton::changed, this, &KateFileTreeConfigPage::slotMyChanged);
    connect(kcbEditShade, &KColorButton::changed, this, &KateFileTreeConfigPage::slotMyChanged);
    connect(cmbSort, QOverload<int>::of(&QComboBox::activated), this, &KateFileTreeConfigPage::slotMyChanged);
    connect(cmbMode, QOverload<int>::of(&QComboBox::activated), this, &KateFileTreeConfigPage::slotMyChanged);
    connect(cbShowFullPath, &QCheckBox::stateChanged, this, &KateFileTreeConfigPage::slotMyChanged);
    connect(cbShowToolbar, &QCheckBox::stateChanged, this, &KateFileTreeConfigPage::slotMyChanged);
    connect(cbShowClose, &QCheckBox::stateChanged, this, &KateFileTreeConfigPage::slotMyChanged);
}

QString KateFileTreeConfigPage::name() const
{
    return QString(i18n("Documents"));
}

QString KateFileTreeConfigPage::fullName() const
{
    return QString(i18n("Configure Documents"));
}

QIcon KateFileTreeConfigPage::icon() const
{
    return QIcon::fromTheme(QLatin1String("view-list-tree"));
}

void KateFileTreeConfigPage::apply()
{
    if (!m_changed) {
        return;
    }

    m_changed = false;

    // apply config to views
    m_plug->applyConfig(gbEnableShading->isChecked(),
                        kcbViewShade->color(),
                        kcbEditShade->color(),
                        cmbMode->itemData(cmbMode->currentIndex()).toBool(),
                        cmbSort->itemData(cmbSort->currentIndex()).toInt(),
                        cbShowFullPath->checkState() == Qt::Checked,
                        cbShowToolbar->checkState() == Qt::Checked,
                        cbShowClose->isChecked());
}

void KateFileTreeConfigPage::reset()
{
    const KateFileTreePluginSettings &settings = m_plug->settings();

    gbEnableShading->setChecked(settings.shadingEnabled());
    kcbEditShade->setColor(settings.editShade());
    kcbViewShade->setColor(settings.viewShade());
    cmbSort->setCurrentIndex(cmbSort->findData(settings.sortRole()));
    cmbMode->setCurrentIndex(settings.listMode());
    cbShowFullPath->setCheckState(settings.showFullPathOnRoots() ? Qt::Checked : Qt::Unchecked);
    cbShowToolbar->setCheckState(settings.showToolbar() ? Qt::Checked : Qt::Unchecked);
    cbShowClose->setChecked(settings.showCloseButton());

    m_changed = false;
}

void KateFileTreeConfigPage::defaults()
{
    // m_plug->settings().revertToDefaults() ??
    // not sure the above is ever needed...
    reset();
}

void KateFileTreeConfigPage::slotMyChanged()
{
    m_changed = true;
    Q_EMIT changed();
}

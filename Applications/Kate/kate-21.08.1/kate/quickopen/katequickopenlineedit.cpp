/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2020 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "katequickopenlineedit.h"

#include <QContextMenuEvent>
#include <QMenu>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

static const QString CONFIG_QUICKOPEN_LISTMODE{QStringLiteral("Quickopen List Mode")};

QuickOpenLineEdit::QuickOpenLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setPlaceholderText(i18n("Quick Open Search (configure via context menu)"));

    // ensure config is read (menu only created upon demand)
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig();
    KConfigGroup cg(cfg, "General");

    const bool cfgListMode = cg.readEntry(CONFIG_QUICKOPEN_LISTMODE, true);
    m_listMode = cfgListMode ? KateQuickOpenModelList::CurrentProject : KateQuickOpenModelList::AllProjects;
}

QuickOpenLineEdit::~QuickOpenLineEdit()
{
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig();
    KConfigGroup cg(cfg, "General");

    cg.writeEntry(CONFIG_QUICKOPEN_LISTMODE, m_listMode == KateQuickOpenModelList::CurrentProject);
}

void QuickOpenLineEdit::contextMenuEvent(QContextMenuEvent *event)
{
    // on demand construction
    if (!menu) {
        menu.reset(createStandardContextMenu());
        setupMenu();
    }
    menu->exec(event->globalPos());
}

void QuickOpenLineEdit::setupMenu()
{
    const bool cfgListMode = m_listMode == CurrentProject;

    menu->addSeparator();

    QActionGroup *actGp = new QActionGroup(this);

    auto act = menu->addAction(i18n("All Projects"));
    act->setCheckable(true);
    connect(act, &QAction::toggled, this, [this](bool checked) {
        if (checked) {
            m_listMode = AllProjects;
            Q_EMIT listModeChanged(KateQuickOpenModelList::AllProjects);
        }
    });
    act->setChecked(!cfgListMode);

    actGp->addAction(act);

    act = menu->addAction(i18n("Current Project"));
    act->setCheckable(true);
    connect(act, &QAction::toggled, this, [this](bool checked) {
        if (checked) {
            m_listMode = CurrentProject;
            Q_EMIT listModeChanged(KateQuickOpenModelList::CurrentProject);
        }
    });
    act->setChecked(cfgListMode);

    actGp->addAction(act);
}

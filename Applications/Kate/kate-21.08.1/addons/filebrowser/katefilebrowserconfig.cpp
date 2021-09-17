/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>
   SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "katefilebrowserconfig.h"

#include "katefilebrowser.h"

#include <QAction>
#include <QApplication>
#include <QGroupBox>
#include <QListWidget>
#include <QRegularExpression>
#include <QStyle>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KActionSelector>
#include <KConfigGroup>
#include <KDirOperator>
#include <KLocalizedString>
#include <KSharedConfig>

// BEGIN ACtionLBItem
/*
  QListboxItem that can store and return a string,
  used for the toolbar action selector.
*/
class ActionLBItem : public QListWidgetItem
{
public:
    ActionLBItem(QListWidget *lb = nullptr, const QIcon &pm = QIcon(), const QString &text = QString(), const QString &str = QString())
        : QListWidgetItem(pm, text, lb, 0)
        , _str(str)
    {
    }
    QString idstring()
    {
        return _str;
    }

private:
    QString _str;
};
// END ActionLBItem

// BEGIN KateFileBrowserConfigPage
KateFileBrowserConfigPage::KateFileBrowserConfigPage(QWidget *parent, KateFileBrowser *kfb)
    : KTextEditor::ConfigPage(parent)
    , fileBrowser(kfb)
{
    QVBoxLayout *lo = new QVBoxLayout(this);
    int spacing = QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing);
    lo->setSpacing(spacing);
    lo->setContentsMargins(0, 0, 0, 0);

    // Toolbar - a lot for a little...
    QGroupBox *gbToolbar = new QGroupBox(i18n("Toolbar"), this);
    acSel = new KActionSelector(gbToolbar);
    acSel->setAvailableLabel(i18n("A&vailable actions:"));
    acSel->setSelectedLabel(i18n("S&elected actions:"));

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(acSel);
    gbToolbar->setLayout(vbox);

    lo->addWidget(gbToolbar);
    connect(acSel, &KActionSelector::added, this, &KateFileBrowserConfigPage::slotMyChanged);
    connect(acSel, &KActionSelector::removed, this, &KateFileBrowserConfigPage::slotMyChanged);
    connect(acSel, &KActionSelector::movedUp, this, &KateFileBrowserConfigPage::slotMyChanged);
    connect(acSel, &KActionSelector::movedDown, this, &KateFileBrowserConfigPage::slotMyChanged);

    init();
}

QString KateFileBrowserConfigPage::name() const
{
    return i18n("Filesystem Browser");
}

QString KateFileBrowserConfigPage::fullName() const
{
    return i18n("Filesystem Browser Settings");
}

QIcon KateFileBrowserConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("document-open"));
}

void KateFileBrowserConfigPage::apply()
{
    if (!m_changed) {
        return;
    }

    m_changed = false;

    KConfigGroup config(KSharedConfig::openConfig(), "filebrowser");
    QStringList l;
    ActionLBItem *aItem;
    const QList<QListWidgetItem *> list = acSel->selectedListWidget()->findItems(QStringLiteral("*"), Qt::MatchWildcard);
    for (QListWidgetItem *item : list) {
        aItem = static_cast<ActionLBItem *>(item);
        l << aItem->idstring();
    }
    config.writeEntry("toolbar actions", l);

    fileBrowser->setupToolbar();
}

void KateFileBrowserConfigPage::reset()
{
    // hmm, what is this supposed to do, actually??
    init();
    m_changed = false;
}

void KateFileBrowserConfigPage::init()
{
    KConfigGroup config(KSharedConfig::openConfig(), "filebrowser");
    // toolbar
    QStringList l = config.readEntry("toolbar actions", QStringList());
    if (l.isEmpty()) { // default toolbar
        l << QStringLiteral("back") << QStringLiteral("forward") << QStringLiteral("bookmarks") << QStringLiteral("sync_dir") << QStringLiteral("configure");
    }

    // actions from diroperator + two of our own
    const QStringList allActions{QStringLiteral("up"),
                                 QStringLiteral("back"),
                                 QStringLiteral("forward"),
                                 QStringLiteral("home"),
                                 QStringLiteral("reload"),
                                 QStringLiteral("mkdir"),
                                 QStringLiteral("delete"),
                                 QStringLiteral("short view"),
                                 QStringLiteral("detailed view"),
                                 QStringLiteral("tree view"),
                                 QStringLiteral("detailed tree view"),
                                 QStringLiteral("show hidden"),
                                 // QStringLiteral("view menu"),
                                 // QStringLiteral("properties"),
                                 QStringLiteral("bookmarks"),
                                 QStringLiteral("sync_dir"),
                                 QStringLiteral("configure")};

    QRegularExpression re(QStringLiteral("&(?=[^&])"));
    QAction *ac = nullptr;
    QListWidget *lb;
    for (const auto &actionName : allActions) {
        lb = l.contains(actionName) ? acSel->selectedListWidget() : acSel->availableListWidget();

        if (actionName == QLatin1String("bookmarks") || actionName == QLatin1String("sync_dir") || actionName == QLatin1String("configure")) {
            ac = fileBrowser->actionCollection()->action(actionName);
        } else {
            ac = fileBrowser->dirOperator()->actionCollection()->action(actionName);
        }

        if (ac) {
            QString text = ac->text().remove(re);
// CJK languages need a filtering message for action texts in lists,
// to remove special accelerators that they use.
// The exact same filtering message exists in kdelibs; hence,
// avoid extraction here and let it be sourced from kdelibs.
#define i18ncX i18nc
            text = i18ncX("@item:intable Action name in toolbar editor", "%1", text);
            new ActionLBItem(lb, ac->icon(), text, actionName);
        }
    }
}

void KateFileBrowserConfigPage::slotMyChanged()
{
    m_changed = true;
    Q_EMIT changed();
}
// END KateFileBrowserConfigPage

// kate: space-indent on; indent-width 2; replace-tabs on;

/* This file is part of the KDE project

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateprojectconfigpage.h"
#include "kateprojectplugin.h"

#include <KLocalizedString>
#include <KUrlRequester>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

KateProjectConfigPage::KateProjectConfigPage(QWidget *parent, KateProjectPlugin *plugin)
    : KTextEditor::ConfigPage(parent)
    , m_plugin(plugin)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *vbox = new QVBoxLayout;
    QGroupBox *group = new QGroupBox(i18nc("Groupbox title", "Autoload Repositories"), this);
    group->setWhatsThis(
        i18n("Project plugin is able to autoload repository working copies when "
             "there is no .kateproject file defined yet."));

    m_cbAutoGit = new QCheckBox(i18n("&Git"), this);
    vbox->addWidget(m_cbAutoGit);

    m_cbAutoSubversion = new QCheckBox(i18n("&Subversion"), this);
    vbox->addWidget(m_cbAutoSubversion);
    m_cbAutoMercurial = new QCheckBox(i18n("&Mercurial"), this);
    vbox->addWidget(m_cbAutoMercurial);

    vbox->addStretch(1);
    group->setLayout(vbox);

    layout->addWidget(group);

    vbox = new QVBoxLayout();
    group = new QGroupBox(i18nc("Groupbox title", "Project Index"), this);
    group->setWhatsThis(i18n("Project ctags index settings"));
    m_cbIndexEnabled = new QCheckBox(i18n("Enable indexing"), this);
    vbox->addWidget(m_cbIndexEnabled);
    auto label = new QLabel(this);
    label->setText(i18n("Directory for index files"));
    vbox->addWidget(label);
    m_indexPath = new KUrlRequester(this);
    m_indexPath->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    m_indexPath->setToolTip(i18n("The system temporary directory is used if not specified, which may overflow for very large repositories"));
    vbox->addWidget(m_indexPath);
    vbox->addStretch(1);
    group->setLayout(vbox);
    layout->addWidget(group);

    vbox = new QVBoxLayout;
    group = new QGroupBox(i18nc("Groupbox title", "Cross-Project Functionality"), this);
    group->setWhatsThis(i18n("Project plugin is able to perform some operations across multiple projects"));
    m_cbMultiProjectCompletion = new QCheckBox(i18n("Cross-Project Completion"), this);
    vbox->addWidget(m_cbMultiProjectCompletion);
    m_cbMultiProjectGoto = new QCheckBox(i18n("Cross-Project Goto Symbol"), this);
    vbox->addWidget(m_cbMultiProjectGoto);
    vbox->addStretch(1);
    group->setLayout(vbox);
    layout->addWidget(group);

    /** Git specific **/
    vbox = new QVBoxLayout;
    group = new QGroupBox(i18nc("Groupbox title", "Git"), this);
    m_cbGitStatusDiffNumStat = new QCheckBox(i18n("Show number of changed lines in git status"));
    vbox->addWidget(m_cbGitStatusDiffNumStat);

    auto hbox = new QHBoxLayout;
    label = new QLabel(i18n("Single click action in the git status view"), this);
    m_cmbSingleClick = new QComboBox(this);
    m_cmbSingleClick->addItem(i18n("No Action"));
    m_cmbSingleClick->addItem(i18n("Show Diff"));
    m_cmbSingleClick->addItem(i18n("Open file"));
    m_cmbSingleClick->addItem(i18n("Stage / Unstage"));
    hbox->addWidget(label);
    hbox->addWidget(m_cmbSingleClick);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout;
    label = new QLabel(i18n("Double click action in the git status view"), this);
    m_cmbDoubleClick = new QComboBox(this);
    m_cmbDoubleClick->addItem(i18n("No Action"));
    m_cmbDoubleClick->addItem(i18n("Show Diff"));
    m_cmbDoubleClick->addItem(i18n("Open file"));
    m_cmbDoubleClick->addItem(i18n("Stage / Unstage"));
    hbox->addWidget(label);
    hbox->addWidget(m_cmbDoubleClick);
    vbox->addLayout(hbox);

    vbox->addStretch(1);
    group->setLayout(vbox);
    layout->addWidget(group);

    layout->insertStretch(-1, 10);

    connect(m_cbAutoGit, &QCheckBox::stateChanged, this, &KateProjectConfigPage::slotMyChanged);
    connect(m_cbAutoSubversion, &QCheckBox::stateChanged, this, &KateProjectConfigPage::slotMyChanged);
    connect(m_cbAutoMercurial, &QCheckBox::stateChanged, this, &KateProjectConfigPage::slotMyChanged);
    connect(m_cbIndexEnabled, &QCheckBox::stateChanged, this, &KateProjectConfigPage::slotMyChanged);
    connect(m_indexPath, &KUrlRequester::textChanged, this, &KateProjectConfigPage::slotMyChanged);
    connect(m_indexPath, &KUrlRequester::urlSelected, this, &KateProjectConfigPage::slotMyChanged);
    connect(m_cbMultiProjectCompletion, &QCheckBox::stateChanged, this, &KateProjectConfigPage::slotMyChanged);
    connect(m_cbMultiProjectGoto, &QCheckBox::stateChanged, this, &KateProjectConfigPage::slotMyChanged);

    connect(m_cbGitStatusDiffNumStat, &QCheckBox::stateChanged, this, &KateProjectConfigPage::slotMyChanged);
    connect(m_cmbSingleClick, QOverload<int>::of(&QComboBox::activated), this, &KateProjectConfigPage::slotMyChanged);
    connect(m_cmbDoubleClick, QOverload<int>::of(&QComboBox::activated), this, &KateProjectConfigPage::slotMyChanged);

    reset();
}

QString KateProjectConfigPage::name() const
{
    return i18n("Projects");
}

QString KateProjectConfigPage::fullName() const
{
    return i18nc("Groupbox title", "Projects Properties");
}

QIcon KateProjectConfigPage::icon() const
{
    return QIcon::fromTheme(QLatin1String("view-list-tree"));
}

void KateProjectConfigPage::apply()
{
    if (!m_changed) {
        return;
    }

    m_changed = false;

    m_plugin->setAutoRepository(m_cbAutoGit->checkState() == Qt::Checked,
                                m_cbAutoSubversion->checkState() == Qt::Checked,
                                m_cbAutoMercurial->checkState() == Qt::Checked);
    m_plugin->setIndex(m_cbIndexEnabled->checkState() == Qt::Checked, m_indexPath->url());
    m_plugin->setMultiProject(m_cbMultiProjectCompletion->checkState() == Qt::Checked, m_cbMultiProjectGoto->checkState() == Qt::Checked);

    m_plugin->setGitStatusShowNumStat(m_cbGitStatusDiffNumStat->isChecked());
    m_plugin->setSingleClickAction((ClickAction)m_cmbSingleClick->currentIndex());
    m_plugin->setDoubleClickAction((ClickAction)m_cmbDoubleClick->currentIndex());
}

void KateProjectConfigPage::reset()
{
    m_cbAutoGit->setCheckState(m_plugin->autoGit() ? Qt::Checked : Qt::Unchecked);
    m_cbAutoSubversion->setCheckState(m_plugin->autoSubversion() ? Qt::Checked : Qt::Unchecked);
    m_cbAutoMercurial->setCheckState(m_plugin->autoMercurial() ? Qt::Checked : Qt::Unchecked);
    m_cbIndexEnabled->setCheckState(m_plugin->getIndexEnabled() ? Qt::Checked : Qt::Unchecked);
    m_indexPath->setUrl(m_plugin->getIndexDirectory());
    m_cbMultiProjectCompletion->setCheckState(m_plugin->multiProjectCompletion() ? Qt::Checked : Qt::Unchecked);
    m_cbMultiProjectGoto->setCheckState(m_plugin->multiProjectGoto() ? Qt::Checked : Qt::Unchecked);

    m_cbGitStatusDiffNumStat->setChecked(m_plugin->showGitStatusWithNumStat());
    m_cmbSingleClick->setCurrentIndex((int)m_plugin->singleClickAcion());
    m_cmbDoubleClick->setCurrentIndex((int)m_plugin->doubleClickAcion());

    m_changed = false;
}

void KateProjectConfigPage::defaults()
{
    reset();
}

void KateProjectConfigPage::slotMyChanged()
{
    m_changed = true;
    Q_EMIT changed();
}

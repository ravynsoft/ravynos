//
// configview.cpp
//
// Description: View for configuring the set of targets to be used with the debugger
//
//
// SPDX-FileCopyrightText: 2010 Ian Wakeling <ian.wakeling@ntlworld.com>
// SPDX-FileCopyrightText: 2012 Kåre Särs <kare.sars@iki.fi>
//
//  SPDX-License-Identifier: LGPL-2.0-only

#include "configview.h"

#include <QCompleter>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QLayout>
#include <QPushButton>
#include <QTimer>

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <KLocalizedString>
#include <KMessageBox>

#ifdef WIN32
static const QLatin1Char pathSeparator(';');
#else
static const QLatin1Char pathSeparator(':');
#endif

ConfigView::ConfigView(QWidget *parent, KTextEditor::MainWindow *mainWin)
    : QWidget(parent)
    , m_mainWindow(mainWin)
{
    m_targetCombo = new QComboBox();
    m_targetCombo->setEditable(true);
    // don't let Qt insert items when the user edits; new targets are only
    // added when the user explicitly says so
    m_targetCombo->setInsertPolicy(QComboBox::NoInsert);
    m_targetCombo->setDuplicatesEnabled(true);

    m_addTarget = new QToolButton();
    m_addTarget->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
    m_addTarget->setToolTip(i18n("Add new target"));

    m_copyTarget = new QToolButton();
    m_copyTarget->setIcon(QIcon::fromTheme(QStringLiteral("document-copy")));
    m_copyTarget->setToolTip(i18n("Copy target"));

    m_deleteTarget = new QToolButton();
    m_deleteTarget->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    m_deleteTarget->setToolTip(i18n("Delete target"));

    m_line = new QFrame(this);
    m_line->setFrameShadow(QFrame::Sunken);

    m_execLabel = new QLabel(i18n("Executable:"));
    m_execLabel->setBuddy(m_targetCombo);

    m_executable = new QLineEdit();
    QCompleter *completer1 = new QCompleter(this);
    QFileSystemModel *model = new QFileSystemModel(this);
    model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    completer1->setModel(model);
    m_executable->setCompleter(completer1);
    m_executable->setClearButtonEnabled(true);
    m_browseExe = new QToolButton(this);
    m_browseExe->setIcon(QIcon::fromTheme(QStringLiteral("application-x-executable")));

    m_workingDirectory = new QLineEdit();
    QCompleter *completer2 = new QCompleter(this);
    QFileSystemModel *model2 = new QFileSystemModel(completer2);

    completer2->setModel(model2);
    m_workingDirectory->setCompleter(completer2);
    m_workingDirectory->setClearButtonEnabled(true);
    m_workDirLabel = new QLabel(i18n("Working Directory:"));
    m_workDirLabel->setBuddy(m_workingDirectory);
    m_browseDir = new QToolButton(this);
    m_browseDir->setIcon(QIcon::fromTheme(QStringLiteral("inode-directory")));

    m_arguments = new QLineEdit();
    m_arguments->setClearButtonEnabled(true);
    m_argumentsLabel = new QLabel(i18nc("Program argument list", "Arguments:"));
    m_argumentsLabel->setBuddy(m_arguments);

    m_takeFocus = new QCheckBox(i18nc("Checkbox to for keeping focus on the command line", "Keep focus"));
    m_takeFocus->setToolTip(i18n("Keep the focus on the command line"));

    m_redirectTerminal = new QCheckBox(i18n("Redirect IO"));
    m_redirectTerminal->setToolTip(i18n("Redirect the debugged programs IO to a separate tab"));

    m_advancedSettings = new QPushButton(i18n("Advanced Settings"));

    m_checBoxLayout = nullptr;

    // first false then true to make sure a layout is set
    m_useBottomLayout = false;
    resizeEvent(nullptr);
    m_useBottomLayout = true;
    resizeEvent(nullptr);

    m_advanced = new AdvancedGDBSettings(this);
    m_advanced->hide();

    connect(m_targetCombo, &QComboBox::editTextChanged, this, &ConfigView::slotTargetEdited);
    connect(m_targetCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ConfigView::slotTargetSelected);
    connect(m_addTarget, &QToolButton::clicked, this, &ConfigView::slotAddTarget);
    connect(m_copyTarget, &QToolButton::clicked, this, &ConfigView::slotCopyTarget);
    connect(m_deleteTarget, &QToolButton::clicked, this, &ConfigView::slotDeleteTarget);
    connect(m_browseExe, &QToolButton::clicked, this, &ConfigView::slotBrowseExec);
    connect(m_browseDir, &QToolButton::clicked, this, &ConfigView::slotBrowseDir);
    connect(m_redirectTerminal, &QCheckBox::toggled, this, &ConfigView::showIO);
    connect(m_advancedSettings, &QPushButton::clicked, this, &ConfigView::slotAdvancedClicked);
}

ConfigView::~ConfigView()
{
}

void ConfigView::registerActions(KActionCollection *actionCollection)
{
    m_targetSelectAction = actionCollection->add<KSelectAction>(QStringLiteral("targets"));
    m_targetSelectAction->setText(i18n("Targets"));
    connect(m_targetSelectAction, &KSelectAction::indexTriggered, this, &ConfigView::slotTargetSelected);
}

void ConfigView::readConfig(const KConfigGroup &group)
{
    m_targetCombo->clear();

    int version = group.readEntry(QStringLiteral("version"), 4);
    int targetCount = group.readEntry(QStringLiteral("targetCount"), 1);
    int lastTarget = group.readEntry(QStringLiteral("lastTarget"), 0);
    QString targetKey(QStringLiteral("target_%1"));

    QStringList targetConfStrs;

    for (int i = 0; i < targetCount; i++) {
        targetConfStrs = group.readEntry(targetKey.arg(i), QStringList());
        if (targetConfStrs.count() == 0) {
            continue;
        }

        if ((version == 1) && (targetConfStrs.count() == 3)) {
            // valid old style config, translate it now; note the
            // reordering happening here!
            QStringList temp;
            temp << targetConfStrs[2];
            temp << targetConfStrs[1];
            targetConfStrs = temp;
        }

        if (version < 4) {
            targetConfStrs.prepend(targetConfStrs[0].right(15));
        }

        if (targetConfStrs.count() > NameIndex) {
            m_targetCombo->addItem(targetConfStrs[NameIndex], targetConfStrs);
        }
    }

    if (version < 4) {
        // all targets now have only one argument string
        int argListsCount = group.readEntry(QStringLiteral("argsCount"), 0);
        QString argsKey(QStringLiteral("args_%1"));
        QString targetName(QStringLiteral("%1<%2>"));

        QString argStr;
        int count = m_targetCombo->count();

        for (int i = 0; i < argListsCount; i++) {
            argStr = group.readEntry(argsKey.arg(i), QString());
            for (int j = 0; j < count; j++) {
                targetConfStrs = m_targetCombo->itemData(j).toStringList();
                if (i > 0) {
                    // copy the firsts and change the arguments
                    targetConfStrs[0] = targetName.arg(targetConfStrs[0]).arg(i + 1);
                    if (targetConfStrs.count() > 3) {
                        targetConfStrs[3] = argStr;
                    }
                    m_targetCombo->addItem(targetConfStrs[0], targetConfStrs);
                }
            }
        }
    }
    // make sure there is at least one item.
    if (m_targetCombo->count() == 0) {
        slotAddTarget();
    }

    QStringList targetNames;
    for (int i = 0; i < m_targetCombo->count(); i++) {
        targetNames << m_targetCombo->itemText(i);
    }
    m_targetSelectAction->setItems(targetNames);

    if (lastTarget < 0 || lastTarget >= m_targetCombo->count()) {
        lastTarget = 0;
    }
    m_targetCombo->setCurrentIndex(lastTarget);

    m_takeFocus->setChecked(group.readEntry("alwaysFocusOnInput", false));

    m_redirectTerminal->setChecked(group.readEntry("redirectTerminal", false));
}

void ConfigView::writeConfig(KConfigGroup &group)
{
    // make sure the data is up to date before writing
    saveCurrentToIndex(m_currentTarget);

    group.writeEntry("version", 4);

    QString targetKey(QStringLiteral("target_%1"));
    QStringList targetConfStrs;

    group.writeEntry("targetCount", m_targetCombo->count());
    group.writeEntry("lastTarget", m_targetCombo->currentIndex());
    for (int i = 0; i < m_targetCombo->count(); i++) {
        targetConfStrs = m_targetCombo->itemData(i).toStringList();
        group.writeEntry(targetKey.arg(i), targetConfStrs);
    }

    group.writeEntry("alwaysFocusOnInput", m_takeFocus->isChecked());
    group.writeEntry("redirectTerminal", m_redirectTerminal->isChecked());
}

const GDBTargetConf ConfigView::currentTarget() const
{
    GDBTargetConf cfg;
    cfg.targetName = m_targetCombo->currentText();
    cfg.executable = m_executable->text();
    cfg.workDir = m_workingDirectory->text();
    cfg.arguments = m_arguments->text();
    cfg.customInit = m_advanced->configs();
    // Note: AdvancedGDBSettings::GDBIndex == 0
    if ((cfg.customInit.size() >= 0) && !cfg.customInit[0].isEmpty()) {
        cfg.gdbCmd = cfg.customInit[0];
        cfg.customInit.removeFirst();
    } else {
        cfg.gdbCmd = QStringLiteral("gdb");
    }
    // remove empty strings in the customInit
    int i = cfg.customInit.size() - 1;
    while (i >= 0) {
        if (cfg.customInit[i].isEmpty()) {
            cfg.customInit.removeAt(i);
        } else if (cfg.customInit[i].startsWith(QLatin1String("set directories "))) {
            QString paths = cfg.customInit[i];
            paths.remove(QStringLiteral("set directories "));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            cfg.srcPaths = paths.split(pathSeparator, QString::SkipEmptyParts);
#else
            cfg.srcPaths = paths.split(pathSeparator, Qt::SkipEmptyParts);
#endif
        }
        i--;
    }
    return cfg;
}

bool ConfigView::takeFocusAlways() const
{
    return m_takeFocus->isChecked();
}

bool ConfigView::showIOTab() const
{
    return m_redirectTerminal->isChecked();
}

void ConfigView::slotTargetEdited(const QString &newText)
{
    QString newComboText(newText);
    for (int i = 0; i < m_targetCombo->count(); ++i) {
        if (i != m_targetCombo->currentIndex() && m_targetCombo->itemText(i) == newComboText) {
            newComboText = newComboText + QStringLiteral(" 2");
        }
    }
    int cursorPosition = m_targetCombo->lineEdit()->cursorPosition();
    m_targetCombo->setItemText(m_targetCombo->currentIndex(), newComboText);
    m_targetCombo->lineEdit()->setCursorPosition(cursorPosition);

    // rebuild the target menu
    QStringList targets;
    for (int i = 0; i < m_targetCombo->count(); ++i) {
        targets.append(m_targetCombo->itemText(i));
    }
    m_targetSelectAction->setItems(targets);
    m_targetSelectAction->setCurrentItem(m_targetCombo->currentIndex());
}

void ConfigView::slotTargetSelected(int index)
{
    if ((index < 0) || (index >= m_targetCombo->count())) {
        return;
    }

    if ((m_currentTarget > 0) && (m_currentTarget < m_targetCombo->count())) {
        saveCurrentToIndex(m_currentTarget);
    }

    loadFromIndex(index);
    m_currentTarget = index;

    setAdvancedOptions();

    // Keep combo box and menu in sync
    m_targetCombo->setCurrentIndex(index);
    m_targetSelectAction->setCurrentItem(index);
}

void ConfigView::slotAddTarget()
{
    QStringList targetConfStrs;

    targetConfStrs << i18n("Target %1", m_targetCombo->count() + 1);
    targetConfStrs << QString();
    targetConfStrs << QString();
    targetConfStrs << QString();

    m_targetCombo->addItem(targetConfStrs[NameIndex], targetConfStrs);
    m_targetCombo->setCurrentIndex(m_targetCombo->count() - 1);
}

void ConfigView::slotCopyTarget()
{
    QStringList tmp = m_targetCombo->itemData(m_targetCombo->currentIndex()).toStringList();
    if (tmp.empty()) {
        slotAddTarget();
        return;
    }
    tmp[NameIndex] = i18n("Target %1", m_targetCombo->count() + 1);
    m_targetCombo->addItem(tmp[NameIndex], tmp);
    m_targetCombo->setCurrentIndex(m_targetCombo->count() - 1);
}

void ConfigView::slotDeleteTarget()
{
    m_targetCombo->blockSignals(true);
    int currentIndex = m_targetCombo->currentIndex();
    m_targetCombo->removeItem(currentIndex);
    if (m_targetCombo->count() == 0) {
        slotAddTarget();
    }

    loadFromIndex(m_targetCombo->currentIndex());
    m_targetCombo->blockSignals(false);
}

void ConfigView::resizeEvent(QResizeEvent *)
{
    if (m_useBottomLayout && size().height() > size().width()) {
        // Set layout for the side
        delete m_checBoxLayout;
        m_checBoxLayout = nullptr;
        delete layout();
        QGridLayout *layout = new QGridLayout(this);

        layout->addWidget(m_targetCombo, 0, 0);
        layout->addWidget(m_addTarget, 0, 1);
        layout->addWidget(m_copyTarget, 0, 2);
        layout->addWidget(m_deleteTarget, 0, 3);
        m_line->setFrameShape(QFrame::HLine);
        layout->addWidget(m_line, 1, 0, 1, 4);

        layout->addWidget(m_execLabel, 3, 0, Qt::AlignLeft);
        layout->addWidget(m_executable, 4, 0, 1, 3);
        layout->addWidget(m_browseExe, 4, 3);

        layout->addWidget(m_workDirLabel, 5, 0, Qt::AlignLeft);
        layout->addWidget(m_workingDirectory, 6, 0, 1, 3);
        layout->addWidget(m_browseDir, 6, 3);

        layout->addWidget(m_argumentsLabel, 7, 0, Qt::AlignLeft);
        layout->addWidget(m_arguments, 8, 0, 1, 4);

        layout->addWidget(m_takeFocus, 9, 0, 1, 4);
        layout->addWidget(m_redirectTerminal, 10, 0, 1, 4);
        layout->addWidget(m_advancedSettings, 11, 0, 1, 4);

        layout->addItem(new QSpacerItem(1, 1), 12, 0);
        layout->setColumnStretch(0, 1);
        layout->setRowStretch(12, 1);
        m_useBottomLayout = false;
    } else if (!m_useBottomLayout && (size().height() < size().width())) {
        // Set layout for the bottom
        delete m_checBoxLayout;
        delete layout();
        m_checBoxLayout = new QHBoxLayout();
        m_checBoxLayout->addWidget(m_takeFocus, 10);
        m_checBoxLayout->addWidget(m_redirectTerminal, 10);
        m_checBoxLayout->addWidget(m_advancedSettings, 0);

        QGridLayout *layout = new QGridLayout(this);

        layout->addWidget(m_targetCombo, 0, 0, 1, 3);
        layout->addWidget(m_addTarget, 1, 0);
        layout->addWidget(m_copyTarget, 1, 1);
        layout->addWidget(m_deleteTarget, 1, 2);
        m_line->setFrameShape(QFrame::VLine);
        layout->addWidget(m_line, 0, 3, 4, 1);

        layout->addWidget(m_execLabel, 0, 5, Qt::AlignRight);
        layout->addWidget(m_executable, 0, 6);
        layout->addWidget(m_browseExe, 0, 7);

        layout->addWidget(m_workDirLabel, 1, 5, Qt::AlignRight);
        layout->addWidget(m_workingDirectory, 1, 6);
        layout->addWidget(m_browseDir, 1, 7);

        layout->addWidget(m_argumentsLabel, 2, 5, Qt::AlignRight);
        layout->addWidget(m_arguments, 2, 6, 1, 2);

        layout->addLayout(m_checBoxLayout, 3, 5, 1, 3);

        layout->addItem(new QSpacerItem(1, 1), 4, 0);
        layout->setColumnStretch(6, 100);
        layout->setRowStretch(4, 100);
        m_useBottomLayout = true;
    }
}

void ConfigView::setAdvancedOptions()
{
    QStringList tmp = m_targetCombo->itemData(m_targetCombo->currentIndex()).toStringList();

    // make sure we have enough strings;
    while (tmp.count() < CustomStartIndex) {
        tmp << QString();
    }

    if (tmp[GDBIndex].isEmpty()) {
        tmp[GDBIndex] = QStringLiteral("gdb");
    }

    // Remove the strings that are not part of the advanced settings
    for (int i = 0; i < GDBIndex; i++) {
        tmp.takeFirst();
    }

    m_advanced->setConfigs(tmp);
}

void ConfigView::slotAdvancedClicked()
{
    setAdvancedOptions();

    QStringList newList = m_targetCombo->itemData(m_targetCombo->currentIndex()).toStringList();
    // make sure we have enough strings;
    while (newList.count() < GDBIndex) {
        newList << QString();
    }
    // Remove old advanced settings
    while (newList.count() > GDBIndex) {
        newList.takeLast();
    }

    if (m_advanced->exec() == QDialog::Accepted) {
        // save the new values
        newList << m_advanced->configs();
        m_targetCombo->setItemData(m_targetCombo->currentIndex(), newList);
        Q_EMIT configChanged();
    }
}

void ConfigView::slotBrowseExec()
{
    QString exe = m_executable->text();

    if (m_executable->text().isEmpty()) {
        // try current document dir
        KTextEditor::View *view = m_mainWindow->activeView();

        if (view != nullptr) {
            exe = view->document()->url().toLocalFile();
        }
    }
    m_executable->setText(QFileDialog::getOpenFileName(nullptr, QString(), exe, QStringLiteral("application/x-executable")));
}

void ConfigView::slotBrowseDir()
{
    QString dir = m_workingDirectory->text();

    if (m_workingDirectory->text().isEmpty()) {
        // try current document dir
        KTextEditor::View *view = m_mainWindow->activeView();

        if (view != nullptr) {
            dir = view->document()->url().toLocalFile();
        }
    }
    m_workingDirectory->setText(QFileDialog::getExistingDirectory(this, QString(), dir));
}

void ConfigView::saveCurrentToIndex(int index)
{
    if ((index < 0) || (index >= m_targetCombo->count())) {
        return;
    }

    QStringList tmp = m_targetCombo->itemData(index).toStringList();
    // make sure we have enough strings. The custom init strings are set in slotAdvancedClicked().
    while (tmp.count() < CustomStartIndex) {
        tmp << QString();
    }

    tmp[NameIndex] = m_targetCombo->itemText(index);
    tmp[ExecIndex] = m_executable->text();
    tmp[WorkDirIndex] = m_workingDirectory->text();
    tmp[ArgsIndex] = m_arguments->text();

    m_targetCombo->setItemData(index, tmp);
}

void ConfigView::loadFromIndex(int index)
{
    if ((index < 0) || (index >= m_targetCombo->count())) {
        return;
    }

    QStringList tmp = m_targetCombo->itemData(index).toStringList();
    // make sure we have enough strings. The custom init strings are set in slotAdvancedClicked().
    while (tmp.count() < CustomStartIndex) {
        tmp << QString();
    }

    m_executable->setText(tmp[ExecIndex]);
    m_workingDirectory->setText(tmp[WorkDirIndex]);
    m_arguments->setText(tmp[ArgsIndex]);
}

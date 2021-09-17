/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kateconfigdialog.h"

#include "kateapp.h"
#include "kateconfigplugindialogpage.h"
#include "katedebug.h"
#include "katedocmanager.h"
#include "katemainwindow.h"
#include "katepluginmanager.h"
#include "katequickopenmodel.h"
#include "katesessionmanager.h"
#include "kateviewmanager.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluralHandlingSpinBox>
#include <KSharedConfig>
#include <KStandardAction>

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

KateConfigDialog::KateConfigDialog(KateMainWindow *parent)
    : KPageDialog(parent)
    , m_mainWindow(parent)
{
    setWindowTitle(i18n("Configure"));
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Help);

    // first: add the KTextEditor config pages
    // rational: most people want to alter e.g. the fonts, the colors or some other editor stuff first
    addEditorPages();

    // second: add out own config pages
    // this includes all plugin config pages, added to the bottom
    addBehaviorPage();
    addSessionPage();
    addFeedbackPage();
    addPluginsPage();
    addPluginPages();

    // ensure no stray signals already set this!
    m_dataChanged = false;
    buttonBox()->button(QDialogButtonBox::Apply)->setEnabled(false);

    // handle dialog actions
    connect(this, &KateConfigDialog::accepted, this, &KateConfigDialog::slotApply);
    connect(buttonBox()->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &KateConfigDialog::slotApply);
    connect(buttonBox()->button(QDialogButtonBox::Help), &QPushButton::clicked, this, &KateConfigDialog::slotHelp);
}

void KateConfigDialog::addBehaviorPage()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup cgGeneral = KConfigGroup(config, "General");

    QFrame *generalFrame = new QFrame;
    KPageWidgetItem *item = addPage(generalFrame, i18n("Behavior"));
    item->setHeader(i18n("Behavior Options"));
    item->setIcon(QIcon::fromTheme(QStringLiteral("preferences-system-windows-behavior")));

    QVBoxLayout *layout = new QVBoxLayout(generalFrame);
    layout->setContentsMargins(0, 0, 0, 0);

    // GROUP with the one below: "Behavior"
    QGroupBox *buttonGroup = new QGroupBox(i18n("&Behavior"), generalFrame);
    QVBoxLayout *vbox = new QVBoxLayout;
    layout->addWidget(buttonGroup);

    auto hlayout = new QHBoxLayout;
    auto label = new QLabel(i18n("&Switch to output view upon message type:"), buttonGroup);
    hlayout->addWidget(label);
    m_messageTypes = new QComboBox(buttonGroup);
    hlayout->addWidget(m_messageTypes);
    label->setBuddy(m_messageTypes);
    m_messageTypes->addItems({i18n("Never"), i18n("Error"), i18n("Warning"), i18n("Info"), i18n("Log")});
    m_messageTypes->setCurrentIndex(cgGeneral.readEntry("Show output view for message type", 1));
    connect(m_messageTypes, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &KateConfigDialog::slotChanged);
    vbox->addLayout(hlayout);

    // modified files notification
    m_modNotifications = new QCheckBox(i18n("Use a separate &dialog for handling externally modified files"), buttonGroup);
    m_modNotifications->setChecked(m_mainWindow->modNotificationEnabled());
    m_modNotifications->setWhatsThis(
        i18n("If enabled, a modal dialog will be used to show all of the modified files. "
             "If not enabled, you will be individually asked what to do for each modified file "
             "only when that file's view receives focus."));
    connect(m_modNotifications, &QCheckBox::toggled, this, &KateConfigDialog::slotChanged);

    vbox->addWidget(m_modNotifications);

    buttonGroup->setLayout(vbox);

    // tabbar => we allow to configure some limit on number of tabs to show
    buttonGroup = new QGroupBox(i18n("&Tabs"), generalFrame);
    vbox = new QVBoxLayout;
    buttonGroup->setLayout(vbox);
    hlayout = new QHBoxLayout;
    label = new QLabel(i18n("&Limit number of tabs:"), buttonGroup);
    hlayout->addWidget(label);
    m_tabLimit = new QSpinBox(buttonGroup);
    hlayout->addWidget(m_tabLimit);
    label->setBuddy(m_tabLimit);
    m_tabLimit->setRange(0, 256);
    m_tabLimit->setSpecialValueText(i18n("Unlimited"));
    m_tabLimit->setValue(cgGeneral.readEntry("Tabbar Tab Limit", 0));
    connect(m_tabLimit, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &KateConfigDialog::slotChanged);
    vbox->addLayout(hlayout);

    m_showTabCloseButton = new QCheckBox(i18n("&Show close button"), buttonGroup);
    m_showTabCloseButton->setChecked(cgGeneral.readEntry("Show Tabs Close Button", true));
    m_showTabCloseButton->setToolTip(i18n("When checked each tab will display a close button."));
    connect(m_showTabCloseButton, &QCheckBox::toggled, this, &KateConfigDialog::slotChanged);
    vbox->addWidget(m_showTabCloseButton);

    m_expandTabs = new QCheckBox(i18n("&Expand tabs"), buttonGroup);
    m_expandTabs->setChecked(cgGeneral.readEntry("Expand Tabs", true));
    m_expandTabs->setToolTip(i18n("When checked tabs take as much size as possible."));
    connect(m_expandTabs, &QCheckBox::toggled, this, &KateConfigDialog::slotChanged);
    vbox->addWidget(m_expandTabs);

    m_tabDoubleClickNewDocument = new QCheckBox(i18n("&Double click opens a new document"), buttonGroup);
    m_tabDoubleClickNewDocument->setChecked(cgGeneral.readEntry("Tab Double Click New Document", true));
    m_tabDoubleClickNewDocument->setToolTip(i18n("When checked double click opens a new document."));
    connect(m_tabDoubleClickNewDocument, &QCheckBox::toggled, this, &KateConfigDialog::slotChanged);
    vbox->addWidget(m_tabDoubleClickNewDocument);

    m_tabMiddleClickCloseDocument = new QCheckBox(i18n("&Middle click closes a document"), buttonGroup);
    m_tabMiddleClickCloseDocument->setChecked(cgGeneral.readEntry("Tab Middle Click Close Document", true));
    m_tabMiddleClickCloseDocument->setToolTip(i18n("When checked middle click closes a document."));
    connect(m_tabMiddleClickCloseDocument, &QCheckBox::toggled, this, &KateConfigDialog::slotChanged);
    vbox->addWidget(m_tabMiddleClickCloseDocument);

    layout->addWidget(buttonGroup);

    layout->addStretch(1); // :-] works correct without autoadd
}

void KateConfigDialog::addSessionPage()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup cgGeneral = KConfigGroup(config, "General");

    QWidget *sessionsPage = new QWidget();
    auto item = addPage(sessionsPage, i18n("Session"));
    item->setHeader(i18n("Session Management"));
    item->setIcon(QIcon::fromTheme(QStringLiteral("view-history")));

    sessionConfigUi.setupUi(sessionsPage);

    // save meta infos
    sessionConfigUi.saveMetaInfos->setChecked(KateApp::self()->documentManager()->getSaveMetaInfos());
    connect(sessionConfigUi.saveMetaInfos, &QGroupBox::toggled, this, &KateConfigDialog::slotChanged);

    // meta infos days
    sessionConfigUi.daysMetaInfos->setMaximum(180);
    sessionConfigUi.daysMetaInfos->setSpecialValueText(i18nc("The special case of 'Delete unused meta-information after'", "(never)"));
    sessionConfigUi.daysMetaInfos->setSuffix(ki18ncp("The suffix of 'Delete unused meta-information after'", " day", " days"));
    sessionConfigUi.daysMetaInfos->setValue(KateApp::self()->documentManager()->getDaysMetaInfos());
    connect(sessionConfigUi.daysMetaInfos,
            static_cast<void (KPluralHandlingSpinBox::*)(int)>(&KPluralHandlingSpinBox::valueChanged),
            this,
            &KateConfigDialog::slotChanged);

    // restore view  config
    sessionConfigUi.restoreVC->setChecked(cgGeneral.readEntry("Restore Window Configuration", true));
    connect(sessionConfigUi.restoreVC, &QCheckBox::toggled, this, &KateConfigDialog::slotChanged);

    sessionConfigUi.spinBoxRecentFilesCount->setValue(recentFilesMaxCount());
    connect(sessionConfigUi.spinBoxRecentFilesCount, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &KateConfigDialog::slotChanged);

    QString sesStart(cgGeneral.readEntry("Startup Session", "manual"));
    if (sesStart == QLatin1String("new")) {
        sessionConfigUi.startNewSessionRadioButton->setChecked(true);
    } else if (sesStart == QLatin1String("last")) {
        sessionConfigUi.loadLastUserSessionRadioButton->setChecked(true);
    } else {
        sessionConfigUi.manuallyChooseSessionRadioButton->setChecked(true);
    }

    connect(sessionConfigUi.startNewSessionRadioButton, &QRadioButton::toggled, this, &KateConfigDialog::slotChanged);
    connect(sessionConfigUi.loadLastUserSessionRadioButton, &QRadioButton::toggled, this, &KateConfigDialog::slotChanged);
    connect(sessionConfigUi.manuallyChooseSessionRadioButton, &QRadioButton::toggled, this, &KateConfigDialog::slotChanged);

    // Closing last file closes Kate
    sessionConfigUi.modCloseAfterLast->setChecked(m_mainWindow->modCloseAfterLast());
    connect(sessionConfigUi.modCloseAfterLast, &QCheckBox::toggled, this, &KateConfigDialog::slotChanged);

    // stash unsave changes
    sessionConfigUi.stashNewUnsavedFiles->setChecked(KateApp::self()->stashManager()->stashNewUnsavedFiles());
    sessionConfigUi.stashUnsavedFilesChanges->setChecked(KateApp::self()->stashManager()->stashUnsavedChanges());
    connect(sessionConfigUi.stashNewUnsavedFiles, &QRadioButton::toggled, this, &KateConfigDialog::slotChanged);
    connect(sessionConfigUi.stashUnsavedFilesChanges, &QRadioButton::toggled, this, &KateConfigDialog::slotChanged);
}

void KateConfigDialog::addPluginsPage()
{
    QFrame *page = new QFrame(this);
    QVBoxLayout *vlayout = new QVBoxLayout(page);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(0);

    KateConfigPluginPage *configPluginPage = new KateConfigPluginPage(page, this);
    vlayout->addWidget(configPluginPage);
    connect(configPluginPage, &KateConfigPluginPage::changed, this, &KateConfigDialog::slotChanged);

    auto item = addPage(page, i18n("Plugins"));
    item->setHeader(i18n("Plugin Manager"));
    item->setIcon(QIcon::fromTheme(QStringLiteral("preferences-plugin")));
}

void KateConfigDialog::addFeedbackPage()
{
#ifdef WITH_KUSERFEEDBACK
    // KUserFeedback Config
    auto page = new QFrame(this);
    auto vlayout = new QVBoxLayout(page);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(0);

    m_userFeedbackWidget = new KUserFeedback::FeedbackConfigWidget(page);
    m_userFeedbackWidget->setFeedbackProvider(&KateApp::self()->userFeedbackProvider());
    connect(m_userFeedbackWidget, &KUserFeedback::FeedbackConfigWidget::configurationChanged, this, &KateConfigDialog::slotChanged);
    vlayout->addWidget(m_userFeedbackWidget);

    auto item = addPage(page, i18n("User Feedback"));
    item->setHeader(i18n("User Feedback"));
    item->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-locale")));
#endif
}

void KateConfigDialog::addPluginPages()
{
    const KatePluginList &pluginList(KateApp::self()->pluginManager()->pluginList());
    for (const KatePluginInfo &plugin : pluginList) {
        if (plugin.load) {
            addPluginPage(plugin.plugin);
        }
    }
}

void KateConfigDialog::addEditorPages()
{
    for (int i = 0; i < KTextEditor::Editor::instance()->configPages(); ++i) {
        KTextEditor::ConfigPage *page = KTextEditor::Editor::instance()->configPage(i, this);
        connect(page, &KTextEditor::ConfigPage::changed, this, &KateConfigDialog::slotChanged);
        m_editorPages.push_back(page);
        KPageWidgetItem *item = addPage(page, page->name());
        item->setHeader(page->fullName());
        item->setIcon(page->icon());
    }
}

void KateConfigDialog::addPluginPage(KTextEditor::Plugin *plugin)
{
    for (int i = 0; i < plugin->configPages(); i++) {
        KTextEditor::ConfigPage *cp = plugin->configPage(i, this);
        KPageWidgetItem *item = addPage(cp, cp->name());
        item->setHeader(cp->fullName());
        item->setIcon(cp->icon());

        PluginPageListItem info;
        info.plugin = plugin;
        info.pluginPage = cp;
        info.idInPlugin = i;
        info.pageWidgetItem = item;
        connect(info.pluginPage, &KTextEditor::ConfigPage::changed, this, &KateConfigDialog::slotChanged);
        m_pluginPages.insert(item, info);
    }
}

void KateConfigDialog::removePluginPage(KTextEditor::Plugin *plugin)
{
    QList<KPageWidgetItem *> remove;
    for (QHash<KPageWidgetItem *, PluginPageListItem>::const_iterator it = m_pluginPages.constBegin(); it != m_pluginPages.constEnd(); ++it) {
        const PluginPageListItem &pli = it.value();

        if (pli.plugin == plugin) {
            remove.append(it.key());
        }
    }

    qCDebug(LOG_KATE) << remove.count();
    while (!remove.isEmpty()) {
        KPageWidgetItem *wItem = remove.takeLast();
        PluginPageListItem item = m_pluginPages.take(wItem);
        delete item.pluginPage;
        removePage(wItem);
    }
}

void KateConfigDialog::slotApply()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    // if data changed apply the kate app stuff
    if (m_dataChanged) {
        KConfigGroup cg = KConfigGroup(config, "General");

        cg.writeEntry("Restore Window Configuration", sessionConfigUi.restoreVC->isChecked());

        cg.writeEntry("Recent File List Entry Count", sessionConfigUi.spinBoxRecentFilesCount->value());

        if (sessionConfigUi.startNewSessionRadioButton->isChecked()) {
            cg.writeEntry("Startup Session", "new");
        } else if (sessionConfigUi.loadLastUserSessionRadioButton->isChecked()) {
            cg.writeEntry("Startup Session", "last");
        } else {
            cg.writeEntry("Startup Session", "manual");
        }

        cg.writeEntry("Save Meta Infos", sessionConfigUi.saveMetaInfos->isChecked());
        KateApp::self()->documentManager()->setSaveMetaInfos(sessionConfigUi.saveMetaInfos->isChecked());

        cg.writeEntry("Days Meta Infos", sessionConfigUi.daysMetaInfos->value());
        KateApp::self()->documentManager()->setDaysMetaInfos(sessionConfigUi.daysMetaInfos->value());

        cg.writeEntry("Modified Notification", m_modNotifications->isChecked());
        m_mainWindow->setModNotificationEnabled(m_modNotifications->isChecked());

        cg.writeEntry("Close After Last", sessionConfigUi.modCloseAfterLast->isChecked());
        m_mainWindow->setModCloseAfterLast(sessionConfigUi.modCloseAfterLast->isChecked());

        cg.readEntry("Show output view for message type", m_messageTypes->currentIndex());

        cg.writeEntry("Stash unsaved file changes", sessionConfigUi.stashUnsavedFilesChanges->isChecked());
        KateApp::self()->stashManager()->setStashUnsavedChanges(sessionConfigUi.stashUnsavedFilesChanges->isChecked());
        cg.writeEntry("Stash new unsaved files", sessionConfigUi.stashNewUnsavedFiles->isChecked());
        KateApp::self()->stashManager()->setStashNewUnsavedFiles(sessionConfigUi.stashNewUnsavedFiles->isChecked());

        cg.writeEntry("Tabbar Tab Limit", m_tabLimit->value());

        cg.writeEntry("Show Tabs Close Button", m_showTabCloseButton->isChecked());

        cg.writeEntry("Expand Tabs", m_expandTabs->isChecked());

        cg.writeEntry("Tab Double Click New Document", m_tabDoubleClickNewDocument->isChecked());
        cg.writeEntry("Tab Middle Click Close Document", m_tabMiddleClickCloseDocument->isChecked());

        // patch document modified warn state
        const QList<KTextEditor::Document *> &docs = KateApp::self()->documentManager()->documentList();
        for (KTextEditor::Document *doc : docs) {
            if (auto modIface = qobject_cast<KTextEditor::ModificationInterface *>(doc)) {
                modIface->setModifiedOnDiskWarning(!m_modNotifications->isChecked());
            }
        }

        m_mainWindow->saveOptions();

        // save plugin config !!
        KateSessionManager *sessionmanager = KateApp::self()->sessionManager();
        KConfig *sessionConfig = sessionmanager->activeSession()->config();
        KateApp::self()->pluginManager()->writeConfig(sessionConfig);

#ifdef WITH_KUSERFEEDBACK
        // set current active mode + write back the config for future starts
        KateApp::self()->userFeedbackProvider().setTelemetryMode(m_userFeedbackWidget->telemetryMode());
        KateApp::self()->userFeedbackProvider().setSurveyInterval(m_userFeedbackWidget->surveyInterval());
#endif
    }

    for (const PluginPageListItem &plugin : qAsConst(m_pluginPages)) {
        if (plugin.pluginPage) {
            plugin.pluginPage->apply();
        }
    }

    // apply ktexteditor pages
    for (KTextEditor::ConfigPage *page : qAsConst(m_editorPages)) {
        page->apply();
    }

    config->sync();

    // emit config change
    if (m_dataChanged) {
        KateApp::self()->configurationChanged();
    }

    m_dataChanged = false;
    buttonBox()->button(QDialogButtonBox::Apply)->setEnabled(false);
}

void KateConfigDialog::slotChanged()
{
    m_dataChanged = true;
    buttonBox()->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void KateConfigDialog::showAppPluginPage(KTextEditor::Plugin *p, int id)
{
    for (const PluginPageListItem &plugin : qAsConst(m_pluginPages)) {
        if ((plugin.plugin == p) && (id == plugin.idInPlugin)) {
            setCurrentPage(plugin.pageWidgetItem);
            break;
        }
    }
}

void KateConfigDialog::slotHelp()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("help:/")));
}

int KateConfigDialog::recentFilesMaxCount()
{
    int maxItems = KConfigGroup(KSharedConfig::openConfig(), "General").readEntry("Recent File List Entry Count", 10);
    return maxItems;
}

void KateConfigDialog::closeEvent(QCloseEvent *event)
{
    if (!m_dataChanged) {
        event->accept();
        return;
    }

    const auto response = KMessageBox::warningYesNoCancel(this,
                                                          i18n("You have unsaved changes. Do you want to apply the changes or discard them?"),
                                                          i18n("Warning"),
                                                          KStandardGuiItem::save(),
                                                          KStandardGuiItem::discard(),
                                                          KStandardGuiItem::cancel());
    switch (response) {
    case KMessageBox::Yes:
        slotApply();
        Q_FALLTHROUGH();
    case KMessageBox::No:
        event->accept();
        break;
    case KMessageBox::Cancel:
        event->ignore();
        break;
    default:
        break;
    }
}

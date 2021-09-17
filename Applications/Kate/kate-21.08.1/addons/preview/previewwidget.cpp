/*
 *  SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "previewwidget.h"

#include "kpartview.h"
#include "ktexteditorpreviewplugin.h"
#include <ktexteditorpreview_debug.h>

// KF
#include <KAboutPluginDialog>
#include <KConfigGroup>
#include <KGuiItem>
#include <KLocalizedString>
#include <KParts/PartLoader>
#include <KParts/ReadOnlyPart>
#include <KPluginMetaData>
#include <KService>
#include <KSharedConfig>
#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>
#include <KToggleAction>
#include <KXMLGUIFactory>

// Qt
#include <QAction>
#include <QDomElement>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QToolButton>
#include <QWidgetAction>

using namespace KTextEditorPreview;

PreviewWidget::PreviewWidget(KTextEditorPreviewPlugin *core, KTextEditor::MainWindow *mainWindow, QWidget *parent)
    : QStackedWidget(parent)
    , KXMLGUIBuilder(this)
    , m_core(core)
    , m_mainWindow(mainWindow)
    , m_xmlGuiFactory(new KXMLGUIFactory(this, this))
{
    m_lockAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("object-unlocked")), i18n("Lock Current Document"), this);
    m_lockAction->setToolTip(i18n("Lock preview to current document"));
    m_lockAction->setCheckedState(KGuiItem(i18n("Unlock Current View"), QIcon::fromTheme(QStringLiteral("object-locked")), i18n("Unlock current view")));
    m_lockAction->setChecked(false);
    connect(m_lockAction, &QAction::triggered, this, &PreviewWidget::toggleDocumentLocking);
    addAction(m_lockAction);

    // TODO: better icon(s)
    const QIcon autoUpdateIcon = QIcon::fromTheme(QStringLiteral("media-playback-start"));
    m_autoUpdateAction = new KToggleAction(autoUpdateIcon, i18n("Automatically Update Preview"), this);
    m_autoUpdateAction->setToolTip(i18n("Enable automatic updates of the preview to the current document content"));
    m_autoUpdateAction->setCheckedState(KGuiItem(i18n("Manually Update Preview"), //
                                                 autoUpdateIcon,
                                                 i18n("Disable automatic updates of the preview to the current document content")));
    m_autoUpdateAction->setChecked(false);
    connect(m_autoUpdateAction, &QAction::triggered, this, &PreviewWidget::toggleAutoUpdating);
    addAction(m_autoUpdateAction);

    m_updateAction = new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")), i18n("Update Preview"), this);
    m_updateAction->setToolTip(i18n("Update the preview to the current document content"));
    connect(m_updateAction, &QAction::triggered, this, &PreviewWidget::updatePreview);
    m_updateAction->setEnabled(false);
    addAction(m_updateAction);

    // manually prepare a proper dropdown menu button, because Qt itself does not do what one would expect
    // when adding a default menu->menuAction() to a QToolbar
    const auto kPartMenuIcon = QIcon::fromTheme(QStringLiteral("application-menu"));
    const auto kPartMenuText = i18n("View");

    // m_kPartMenu may not be a child of this, because otherwise its XMLGUI-menu is deleted when switching views
    // and therefore closing the tool view, which is a QMainWindow in KDevelop (IdealController::addView).
    // see KXMLGUIBuilder::createContainer => tagName == d->tagMenu
    m_kPartMenu = new QMenu;

    QToolButton *toolButton = new QToolButton();
    toolButton->setMenu(m_kPartMenu);
    toolButton->setIcon(kPartMenuIcon);
    toolButton->setText(kPartMenuText);
    toolButton->setPopupMode(QToolButton::InstantPopup);

    m_kPartMenuAction = new QWidgetAction(this);
    m_kPartMenuAction->setIcon(kPartMenuIcon);
    m_kPartMenuAction->setText(kPartMenuText);
    m_kPartMenuAction->setMenu(m_kPartMenu);
    m_kPartMenuAction->setDefaultWidget(toolButton);
    m_kPartMenuAction->setEnabled(false);
    addAction(m_kPartMenuAction);

    m_aboutKPartAction = new QAction(this);
    connect(m_aboutKPartAction, &QAction::triggered, this, &PreviewWidget::showAboutKPartPlugin);
    m_aboutKPartAction->setEnabled(false);

    auto label = new QLabel(i18n("No preview available."), this);
    label->setAlignment(Qt::AlignHCenter);
    addWidget(label);

    connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &PreviewWidget::setTextEditorView);

    setTextEditorView(m_mainWindow->activeView());
}

PreviewWidget::~PreviewWidget()
{
    delete m_kPartMenu;
}

void PreviewWidget::readSessionConfig(const KConfigGroup &configGroup)
{
    // TODO: also store document id/url and see to catch the same document on restoring config
    m_lockAction->setChecked(configGroup.readEntry("documentLocked", false));
    m_autoUpdateAction->setChecked(configGroup.readEntry("automaticUpdate", false));
}

void PreviewWidget::writeSessionConfig(KConfigGroup &configGroup) const
{
    configGroup.writeEntry("documentLocked", m_lockAction->isChecked());
    configGroup.writeEntry("automaticUpdate", m_autoUpdateAction->isChecked());
}

void PreviewWidget::setTextEditorView(KTextEditor::View *view)
{
    if ((view && view == m_previewedTextEditorView && view->document() == m_previewedTextEditorDocument
         && (!m_previewedTextEditorDocument || m_previewedTextEditorDocument->mode() == m_currentMode))
        || !view || !isVisible() || m_lockAction->isChecked()) {
        return;
    }

    m_previewedTextEditorView = view;
    m_previewedTextEditorDocument = view ? view->document() : nullptr;

    resetTextEditorView(m_previewedTextEditorDocument);
}

std::optional<KPluginMetaData> KTextEditorPreview::PreviewWidget::findPreviewPart(const QStringList mimeTypes)
{
    for (const auto &mimeType : qAsConst(mimeTypes)) {
        const auto offers = KParts::PartLoader::partsForMimeType(mimeType);

        if (offers.isEmpty()) {
            continue;
        }

        const KPluginMetaData service = offers.first();
        qCDebug(KTEPREVIEW) << "Found preferred kpart named" << service.name() << "with library" << service.fileName() << "for mimetype" << mimeType;

        // no interest in kparts which also just display the text (like katepart itself)
        // TODO: what about parts which also support importing plain text and turning into richer format
        // and thus have it in their mimetypes list?
        // could that perhaps be solved by introducing the concept of "native" and "imported" mimetypes?
        // or making a distinction between source editors/viewers and final editors/viewers?
        // latter would also help other source editors/viewers like a hexeditor, which "supports" any mimetype
        if (service.mimeTypes().contains(QLatin1String("text/plain"))) {
            qCDebug(KTEPREVIEW) << "Blindly discarding preferred kpart as it also supports text/plain, to avoid useless plain/text preview.";
            continue;
        }

        return service;
    }
    return {};
}

void PreviewWidget::resetTextEditorView(KTextEditor::Document *document)
{
    if (!isVisible() || m_previewedTextEditorDocument != document) {
        return;
    }

    std::optional<KPluginMetaData> service;

    if (m_previewedTextEditorDocument) {
        // TODO: mimetype is not set for new documents which have not been saved yet.
        // Maybe retry to guess as soon as content is inserted.
        m_currentMode = m_previewedTextEditorDocument->mode();

        // Get mimetypes assigned to the currently set mode.
        auto mimeTypes = KConfigGroup(KSharedConfig::openConfig(QStringLiteral("katemoderc")), m_currentMode).readXdgListEntry("Mimetypes");
        // Also try to guess from the content, if the above fails.
        mimeTypes << m_previewedTextEditorDocument->mimeType();

        service = findPreviewPart(mimeTypes);

        if (!service) {
            qCDebug(KTEPREVIEW) << "Found no preferred kpart service for mimetypes" << mimeTypes;
        }

        // Update if the mode is changed. The signal may also be emitted, when a new
        // url is loaded, therefore wait (QueuedConnection) for the document to load.
        connect(m_previewedTextEditorDocument,
                &KTextEditor::Document::modeChanged,
                this,
                &PreviewWidget::resetTextEditorView,
                static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
        // Explicitly clear the old document, which otherwise might be accessed in
        // m_partView->setDocument.
        connect(m_previewedTextEditorDocument, &KTextEditor::Document::aboutToClose, this, &PreviewWidget::unsetDocument, Qt::UniqueConnection);
    } else {
        m_currentMode.clear();
    }

    // change of preview type?
    // TODO: find a better id than library?
    const QString serviceId = service ? service->pluginId() : QString();

    if (serviceId != m_currentServiceId) {
        if (m_partView) {
            clearMenu();
        }

        m_currentServiceId = serviceId;

        if (service) {
            qCDebug(KTEPREVIEW) << "Creating new kpart service instance.";
            m_partView = new KPartView(*service, this);
            const bool autoupdate = m_autoUpdateAction->isChecked();
            m_partView->setAutoUpdating(autoupdate);
            int index = addWidget(m_partView->widget());
            setCurrentIndex(index);

            // update kpart menu
            const auto kPart = m_partView->kPart();
            if (kPart) {
                m_xmlGuiFactory->addClient(kPart);

                m_aboutKPartAction->setText(i18n("About %1", kPart->metaData().name()));
                m_aboutKPartAction->setEnabled(true);
                m_kPartMenu->addSeparator();
                m_kPartMenu->addAction(m_aboutKPartAction);
                m_kPartMenuAction->setEnabled(true);
            }

            m_updateAction->setEnabled(!autoupdate);
        } else {
            m_partView = nullptr;
        }
    } else if (m_partView) {
        qCDebug(KTEPREVIEW) << "Reusing active kpart service instance.";
    }

    if (m_partView) {
        m_partView->setDocument(m_previewedTextEditorDocument);
    }
}

void PreviewWidget::unsetDocument(KTextEditor::Document *document)
{
    if (!m_partView || m_previewedTextEditorDocument != document) {
        return;
    }

    m_partView->setDocument(nullptr);
    m_previewedTextEditorDocument = nullptr;

    // remove any current partview
    clearMenu();
    m_partView = nullptr;

    m_currentServiceId.clear();
}

void PreviewWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    m_updateAction->setEnabled(m_partView && !m_autoUpdateAction->isChecked());

    if (m_lockAction->isChecked()) {
        resetTextEditorView(m_previewedTextEditorDocument);
    } else {
        setTextEditorView(m_mainWindow->activeView());
    }
}

void PreviewWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);

    // keep active part for reuse, but close preview document
    // TODO: we also get hide event in kdevelop when the view is changed,
    // need to find out how to filter this out or how to fix kdevelop
    // so currently keep the preview document
    //     unsetDocument(m_previewedTextEditorDocument);

    m_updateAction->setEnabled(false);
}

void PreviewWidget::toggleDocumentLocking(bool locked)
{
    if (!locked) {
        setTextEditorView(m_mainWindow->activeView());
    }
}

void PreviewWidget::toggleAutoUpdating(bool autoRefreshing)
{
    if (!m_partView) {
        // nothing to do
        return;
    }

    m_updateAction->setEnabled(!autoRefreshing && isVisible());
    m_partView->setAutoUpdating(autoRefreshing);
}

void PreviewWidget::updatePreview()
{
    if (m_partView && m_partView->document()) {
        m_partView->updatePreview();
    }
}

QWidget *PreviewWidget::createContainer(QWidget *parent, int index, const QDomElement &element, QAction *&containerAction)
{
    containerAction = nullptr;

    if (element.attribute(QStringLiteral("deleted")).toLower() == QLatin1String("true")) {
        return nullptr;
    }

    const QString tagName = element.tagName().toLower();
    // filter out things we do not support
    // TODO: consider integrating the toolbars
    if (tagName == QLatin1String("mainwindow") || tagName == QLatin1String("toolbar") || tagName == QLatin1String("statusbar")) {
        return nullptr;
    }

    if (tagName == QLatin1String("menubar")) {
        return m_kPartMenu;
    }

    return KXMLGUIBuilder::createContainer(parent, index, element, containerAction);
}

void PreviewWidget::removeContainer(QWidget *container, QWidget *parent, QDomElement &element, QAction *containerAction)
{
    if (container == m_kPartMenu) {
        return;
    }

    KXMLGUIBuilder::removeContainer(container, parent, element, containerAction);
}

void PreviewWidget::showAboutKPartPlugin()
{
    if (m_partView && m_partView->kPart()) {
        QPointer<KAboutPluginDialog> aboutDialog = new KAboutPluginDialog(m_partView->kPart()->metaData(), this);
        aboutDialog->exec();
        delete aboutDialog;
    }
}

void PreviewWidget::clearMenu()
{
    // clear kpart menu
    m_xmlGuiFactory->removeClient(m_partView->kPart());
    m_kPartMenu->clear();

    removeWidget(m_partView->widget());
    delete m_partView;

    m_updateAction->setEnabled(false);
    m_kPartMenuAction->setEnabled(false);
    m_aboutKPartAction->setEnabled(false);
}

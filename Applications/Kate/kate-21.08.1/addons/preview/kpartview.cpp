/*
 *  SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kpartview.h"

#include <ktexteditorpreview_debug.h>

// KF
#include <KTextEditor/Document>

#include <KActionCollection>
#include <KLocalizedString>
#include <KParts/BrowserExtension>
#include <KParts/ReadOnlyPart>
#include <KPluginFactory>
#include <KPluginLoader>

// Qt
#include <QDesktopServices>
#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QTemporaryFile>

using namespace KTextEditorPreview;

// There are two timers that run on update. One timer is fast, but is
// cancelled each time a new updated comes in. Another timer is slow, but is
// not cancelled if another update comes in. With this, "while typing", the
// preview is updated every 1000ms, thus one sees that something is happening
// from the corner of one's eyes. After stopping typing, the preview is
// updated quickly after 150ms so that the preview has the newest version.
static const int updateDelayFast = 150; // ms
static const int updateDelaySlow = 1000; // ms

KPartView::KPartView(const KPluginMetaData &service, QObject *parent)
    : QObject(parent)
{
    KPluginLoader loader(service.fileName());

    KPluginFactory *factory = loader.factory();

    if (!factory) {
        m_errorLabel = new QLabel(loader.errorString());
    } else {
        m_part = factory->create<KParts::ReadOnlyPart>(nullptr, this);
    }

    if (!m_part) {
        m_errorLabel = new QLabel(loader.errorString());
    } else if (!m_part->widget()) {
        // should not happen, but just be safe
        delete m_part;
        m_errorLabel = new QLabel(QStringLiteral("KPart provides no widget."));
    } else {
        m_updateSquashingTimerFast.setSingleShot(true);
        m_updateSquashingTimerFast.setInterval(updateDelayFast);
        connect(&m_updateSquashingTimerFast, &QTimer::timeout, this, &KPartView::updatePreview);

        m_updateSquashingTimerSlow.setSingleShot(true);
        m_updateSquashingTimerSlow.setInterval(updateDelaySlow);
        connect(&m_updateSquashingTimerSlow, &QTimer::timeout, this, &KPartView::updatePreview);

        auto browserExtension = m_part->browserExtension();
        if (browserExtension) {
            connect(browserExtension, &KParts::BrowserExtension::openUrlRequestDelayed, this, &KPartView::handleOpenUrlRequest);
        }
        m_part->widget()->installEventFilter(this);

        // Register all shortcuts of the KParts actionCollection to eat them in the
        // event filter before they are handled by the application (and potentially
        // identified as ambiguous).
        // Also restrict the shortcuts to the m_part widget by setting the shortcut context.
        m_shortcuts.clear();
        auto ac = m_part->actionCollection();
        for (auto action : ac->actions()) {
            for (auto shortcut : action->shortcuts()) {
                m_shortcuts[shortcut] = action;
            }
            if (action->shortcutContext() != Qt::WidgetShortcut) {
                action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
            }
        }
    }
}

KPartView::~KPartView()
{
    delete m_errorLabel;
}

QWidget *KPartView::widget() const
{
    return m_part ? m_part->widget() : m_errorLabel;
}

KParts::ReadOnlyPart *KPartView::kPart() const
{
    return m_part;
}

KTextEditor::Document *KPartView::document() const
{
    return m_document;
}

bool KPartView::isAutoUpdating() const
{
    return m_autoUpdating;
}

void KPartView::setDocument(KTextEditor::Document *document)
{
    if (m_document == document) {
        return;
    }
    if (!m_part) {
        return;
    }

    if (m_document) {
        disconnect(m_document, &KTextEditor::Document::textChanged, this, &KPartView::triggerUpdatePreview);
        m_updateSquashingTimerFast.stop();
        m_updateSquashingTimerSlow.stop();
    }

    m_document = document;

    // delete any temporary file, to trigger creation of a new if needed
    // for some unique url/path of the temporary file for the new document (or use a counter ourselves?)
    // but see comment for stream url
    delete m_bufferFile;
    m_bufferFile = nullptr;

    if (m_document) {
        m_previewDirty = true;
        updatePreview();
        connect(m_document, &KTextEditor::Document::textChanged, this, &KPartView::triggerUpdatePreview);
    } else {
        m_part->closeUrl();
    }
}

void KPartView::setAutoUpdating(bool autoUpdating)
{
    if (m_autoUpdating == autoUpdating) {
        return;
    }

    m_autoUpdating = autoUpdating;

    if (m_autoUpdating) {
        if (m_document && m_part && m_previewDirty) {
            updatePreview();
        }
    } else {
        m_updateSquashingTimerSlow.stop();
        m_updateSquashingTimerFast.stop();
    }
}

void KPartView::triggerUpdatePreview()
{
    m_previewDirty = true;

    if (m_part->widget()->isVisible() && m_autoUpdating) {
        // Reset fast timer each time
        m_updateSquashingTimerFast.start();
        // Start slow timer, if not already running (don't reset!)
        if (!m_updateSquashingTimerSlow.isActive()) {
            m_updateSquashingTimerSlow.start();
        }
    }
}

void KPartView::updatePreview()
{
    m_updateSquashingTimerSlow.stop();
    m_updateSquashingTimerFast.stop();
    if (!m_part->widget()->isVisible()) {
        return;
    }

    // TODO: some kparts seem to steal the focus after they have loaded a file, sometimes also async
    // that possibly needs fixing in the respective kparts, as that could be considered non-cooperative

    // TODO: investigate if pushing of the data to the kpart could be done in a non-gui-thread,
    // so their loading of the file (e.g. ReadOnlyPart::openFile() is sync design) does not block

    const auto mimeType = m_document->mimeType();
    KParts::OpenUrlArguments arguments;
    arguments.setMimeType(mimeType);
    m_part->setArguments(arguments);

    // try to stream the data to avoid filesystem I/O
    // create url unique for this document
    // TODO: encode existing url instead, and for yet-to-be-stored docs some other unique id
    const QUrl streamUrl(QStringLiteral("ktexteditorpreview:/object/%1").arg(reinterpret_cast<quintptr>(m_document), 0, 16));
    if (m_part->openStream(mimeType, streamUrl)) {
        qCDebug(KTEPREVIEW) << "Pushing data via streaming API, url:" << streamUrl.url();
        m_part->writeStream(m_document->text().toUtf8());
        m_part->closeStream();

        m_previewDirty = false;
        return;
    }

    // have to go via filesystem for now, not nice
    if (!m_bufferFile) {
        m_bufferFile = new QTemporaryFile(this);
        m_bufferFile->open();
    } else {
        // reset position
        m_bufferFile->seek(0);
    }
    const QUrl tempFileUrl(QUrl::fromLocalFile(m_bufferFile->fileName()));
    qCDebug(KTEPREVIEW) << "Pushing data via temporary file, url:" << tempFileUrl.url();

    // write current data
    m_bufferFile->write(m_document->text().toUtf8());
    // truncate at end of new content
    m_bufferFile->resize(m_bufferFile->pos());
    m_bufferFile->flush();

    // TODO: find out why we need to send this queued
    QMetaObject::invokeMethod(m_part, "openUrl", Qt::QueuedConnection, Q_ARG(QUrl, tempFileUrl));

    m_previewDirty = false;
}

void KPartView::handleOpenUrlRequest(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}

bool KPartView::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_part->widget() && event->type() == QEvent::Show) {
        if (m_document && m_autoUpdating && m_previewDirty) {
            updatePreview();
        }
        return true;
    } else if (event->type() == QEvent::ShortcutOverride) {
        auto keyevent = static_cast<QKeyEvent *>(event);
        auto it = m_shortcuts.find(QKeySequence(keyevent->modifiers() | keyevent->key()));
        if (it != m_shortcuts.end()) {
            it.value()->activate(QAction::Trigger);
            event->accept();
            return true;
        }
    }

    return QObject::eventFilter(object, event);
}

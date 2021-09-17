/*
 *  SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KTEXTEDITORPREVIEW_KPARTVIEW_H
#define KTEXTEDITORPREVIEW_KPARTVIEW_H

// KF
#include <KPluginMetaData>

// Qt
#include <QMap>
#include <QObject>
#include <QTimer>

namespace KTextEditor
{
class Document;
}
namespace KParts
{
class ReadOnlyPart;
}
class QAction;
class QKeySequence;
class QLabel;
class QTemporaryFile;

namespace KTextEditorPreview
{
/**
 * Wrapper around a KPart which handles feeding it the content of a text document
 *
 * The class creates a KPart from the service description passed in the constructor
 * and then takes care for feeding the content of the currently set text document
 * to the KPart for preview in the target format, both on changes in the document
 * or when a new document is set.
 *
 * The content is pushed via the KParts stream API, if the KPart instance
 * supports it, or as fallback via the filesystem, using a QTemporaryFile instance.
 * Updates are squashed via a timer, to reduce load.
 */
class KPartView : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     *
     * @param service the description of the KPart which should be used, may not be a nullptr
     * @param parent the object taking ownership, can be a nullptr
     */
    KPartView(const KPluginMetaData &service, QObject *parent);
    ~KPartView() override;

    /**
     * Returns the widget object, ownership is not transferred.
     */
    QWidget *widget() const;

    KParts::ReadOnlyPart *kPart() const;

    /**
     * Sets the current document whose content should be previewed by the KPart.
     *
     * @param document the document or, if there is none to preview, a nullptr
     */
    void setDocument(KTextEditor::Document *document);

    /**
     * Returns the current document whose content is previewed by the KPart.
     *
     * @return current document or, if there is none, a nullptr
     */
    KTextEditor::Document *document() const;

    /**
     * Sets whether the preview should be updating automatically on document changes or not.
     *
     * @param autoUpdating whether the preview should be updating automatically on document changes or not
     */
    void setAutoUpdating(bool autoUpdating);

    /**
     * Returns @c true if the preview is updating automatically on document changes, @c false otherwise.
     */
    bool isAutoUpdating() const;

    /**
     * Update preview to current document content.
     */
    void updatePreview();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void triggerUpdatePreview();
    void handleOpenUrlRequest(const QUrl &url);

private:
    QLabel *m_errorLabel = nullptr;
    KParts::ReadOnlyPart *m_part = nullptr;
    KTextEditor::Document *m_document = nullptr;

    bool m_autoUpdating = true;
    bool m_previewDirty = true;
    QTimer m_updateSquashingTimerFast;
    QTimer m_updateSquashingTimerSlow;
    QTemporaryFile *m_bufferFile = nullptr;
    QMap<QKeySequence, QAction *> m_shortcuts;
};

}

#endif

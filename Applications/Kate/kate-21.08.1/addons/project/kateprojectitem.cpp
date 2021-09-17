/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectitem.h"
#include "kateproject.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QThread>

#include <KIconUtils>
#include <KLocalizedString>

#include <KTextEditor/Document>

KateProjectItem::KateProjectItem(Type type, const QString &text)
    : QStandardItem(text)
    , m_type(type)
{
}

KateProjectItem::~KateProjectItem()
{
    delete m_icon;
}

void KateProjectItem::slotModifiedChanged(KTextEditor::Document *doc)
{
    if (m_icon) {
        delete m_icon;
        m_icon = nullptr;
    }

    if (doc->isModified()) {
        if (m_emblem.isEmpty()) {
            m_icon = new QIcon(QIcon::fromTheme(QStringLiteral("document-save")));
        } else {
            m_icon = new QIcon(KIconUtils::addOverlay(QIcon::fromTheme(QStringLiteral("document-save")), QIcon(m_emblem), Qt::TopLeftCorner));
        }
    }
    emitDataChanged();
}

void KateProjectItem::slotModifiedOnDisk(KTextEditor::Document *document, bool isModified, KTextEditor::ModificationInterface::ModifiedOnDiskReason reason)
{
    Q_UNUSED(document)
    Q_UNUSED(isModified)

    if (m_icon) {
        delete m_icon;
        m_icon = nullptr;
    }

    m_emblem.clear();

    if (reason != KTextEditor::ModificationInterface::OnDiskUnmodified) {
        m_emblem = QStringLiteral("emblem-important");
    }
    emitDataChanged();
}

QVariant KateProjectItem::data(int role) const
{
    if (role == Qt::DecorationRole) {
        /**
         * this should only happen in main thread
         * the background thread should only construct this elements and fill data
         * but never query gui stuff!
         */
        Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
        return QVariant(*icon());
    }

    if (role == TypeRole) {
        return QVariant(m_type);
    }

    return QStandardItem::data(role);
}

bool KateProjectItem::operator<(const QStandardItem &other) const
{
    // let directories stay first
    const auto thisType = data(TypeRole).toInt();
    const auto otherType = other.data(TypeRole).toInt();
    if (thisType != otherType) {
        return thisType < otherType;
    }

    // case-insensitive compare of the filename
    return data(Qt::DisplayRole).toString().compare(other.data(Qt::DisplayRole).toString(), Qt::CaseInsensitive) < 0;
}

QIcon *KateProjectItem::icon() const
{
    if (m_icon) {
        return m_icon;
    }

    switch (m_type) {
    case LinkedProject:
    case Project:
        m_icon = new QIcon(QIcon::fromTheme(QStringLiteral("folder-documents")));
        break;

    case Directory:
        m_icon = new QIcon(QIcon::fromTheme(QStringLiteral("folder")));
        break;

    case File: {
        // ensure we have no empty icons, that breaks layout in tree views
        QIcon icon = QIcon::fromTheme(QMimeDatabase().mimeTypeForUrl(QUrl::fromLocalFile(data(Qt::UserRole).toString())).iconName());
        if (icon.isNull()) {
            icon = QIcon::fromTheme(QStringLiteral("unknown"));
        }
        if (!m_emblem.isEmpty()) {
            m_icon = new QIcon(KIconUtils::addOverlay(icon, QIcon(m_emblem), Qt::TopLeftCorner));
        } else {
            m_icon = new QIcon(icon);
        }
        break;
    }
    }

    return m_icon;
}

void KateProjectItem::setData(const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        auto newFileName = value.toString();
        if (newFileName.isEmpty())
            return;

        /**
         *  retrieve the ref to project that we stored
         *  in KateProjectTreeViewContextMenu
         */
        KateProject *project = data(KateProjectItem::ProjectRole).value<KateProject *>();
        if (!project) {
            return;
        }

        auto oldFileName = data(Qt::DisplayRole).toString();
        auto oldName = data(Qt::UserRole).toString();
        QString newName = oldName;
        newName.replace(oldFileName, newFileName);

        if (oldName == newName) {
            return;
        }

        if (!QFile::rename(oldName, newName)) {
            QMessageBox::critical(nullptr, i18n("Error"), i18n("File name already exists"));
            return;
        }

        /**
         * Update the file2Item
         */
        project->renameFile(newName, oldName);

        // change internal path
        setData(newName, Qt::UserRole);
    }

    QStandardItem::setData(value, role);
}

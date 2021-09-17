/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katequickopenmodel.h"

#include "kateapp.h"
#include "katemainwindow.h"

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <QFileInfo>
#include <QIcon>
#include <QMimeDatabase>

#include <unordered_set>

KateQuickOpenModel::KateQuickOpenModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int KateQuickOpenModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_modelEntries.size();
}

int KateQuickOpenModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant KateQuickOpenModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid()) {
        return {};
    }

    const ModelEntry &entry = m_modelEntries.at(idx.row());
    switch (role) {
    case Qt::DisplayRole:
    case Role::FileName:
        return entry.fileName;
    case Role::FilePath: {
        // no .remove since that might remove all occurrence in rare cases
        const auto &path = entry.filePath;
        return path.startsWith(m_projectBase) ? path.mid(m_projectBase.size()) : path;
    }
    case Qt::FontRole: {
        if (entry.document) {
            QFont font;
            font.setBold(true);
            return font;
        }
        return {};
    }
    case Qt::DecorationRole:
        return QIcon::fromTheme(QMimeDatabase().mimeTypeForFile(entry.fileName, QMimeDatabase::MatchExtension).iconName());
    case Qt::UserRole:
        return entry.url.isEmpty() ? QUrl::fromLocalFile(entry.filePath) : entry.url;
    case Role::Score:
        return entry.score;
    case Role::Document:
        return QVariant::fromValue(entry.document);
    default:
        return {};
    }

    return {};
}

void KateQuickOpenModel::refresh(KateMainWindow *mainWindow)
{
    QObject *projectView = mainWindow->pluginView(QStringLiteral("kateprojectplugin"));
    const std::vector<KTextEditor::View *> sortedViews = mainWindow->viewManager()->sortedViews();
    const QList<KTextEditor::Document *> openDocs = KateApp::self()->documentManager()->documentList();
    const QStringList projectDocs = projectView
        ? (m_listMode == CurrentProject ? projectView->property("projectFiles") : projectView->property("allProjectsFiles")).toStringList()
        : QStringList();
    const QString projectBase = [projectView]() -> QString {
        if (!projectView) {
            return QString();
        }
        QString ret;
        // open files are always included in the listing, even if list mode == CurrentProject
        // those open files may belong to another project than the current one
        // so we should always consistently strip the common base
        // otherwise it will be confusing and the opened files of anther project
        // end up with an unstripped complete file path
        ret = projectView->property("allProjectsCommonBaseDir").toString();
        if (!ret.endsWith(QLatin1Char('/'))) {
            ret.append(QLatin1Char('/'));
        }
        // avoid strip of a single leading /
        if (ret == QStringLiteral("/")) {
            ret.clear();
        }
        return ret;
    }();

    m_projectBase = projectBase;

    std::vector<ModelEntry> allDocuments;
    allDocuments.reserve(sortedViews.size() + projectDocs.size());

    std::unordered_set<QString> openedDocUrls;
    std::unordered_set<KTextEditor::Document *> seenDocuments;
    openedDocUrls.reserve(sortedViews.size());

    const auto collectDoc = [&openedDocUrls, &seenDocuments, &allDocuments](KTextEditor::Document *doc) {
        // We don't want any duplicates, beside for untitled documents
        if (!seenDocuments.insert(doc).second) {
            return;
        }

        // document with set url => use the url for displaying
        if (!doc->url().isEmpty()) {
            auto path = doc->url().toString(QUrl::NormalizePathSegments | QUrl::PreferLocalFile);
            openedDocUrls.insert(path);
            allDocuments.push_back({doc->url(), QFileInfo(path).fileName(), path, doc, -1});
            return;
        }

        // untitled document
        allDocuments.push_back({doc->url(), doc->documentName(), QString(), doc, -1});
    };

    for (auto *view : sortedViews) {
        collectDoc(view->document());
    }

    for (auto *doc : openDocs) {
        collectDoc(doc);
    }

    for (const auto &filePath : projectDocs) {
        // No duplicates
        if (openedDocUrls.count(filePath) != 0) {
            continue;
        }

        // QFileInfo is too expensive just for fileName computation
        const int slashIndex = filePath.lastIndexOf(QLatin1Char('/'));
        QString fileName = filePath.mid(slashIndex + 1);
        allDocuments.push_back({QUrl(), std::move(fileName), filePath, nullptr, -1});
    }

    beginResetModel();
    m_modelEntries = std::move(allDocuments);
    endResetModel();
}

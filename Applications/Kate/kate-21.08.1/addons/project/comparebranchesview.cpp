/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "comparebranchesview.h"
#include "kateprojectpluginview.h"
#include "kateprojectworker.h"

#include <QDir>
#include <QPainter>
#include <QProcess>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include <KColorScheme>
#include <KLocalizedString>

class DiffStyleDelegate : public QStyledItemDelegate
{
public:
    DiffStyleDelegate(QObject *parent)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (index.data(KateProjectItem::TypeRole).toInt() == KateProjectItem::Directory) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        QStyleOptionViewItem options = option;
        initStyleOption(&options, index);

        painter->save();

        // paint background
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else {
            painter->fillRect(option.rect, option.palette.base());
        }

        int add = index.data(Qt::UserRole + 2).toInt();
        int sub = index.data(Qt::UserRole + 3).toInt();
        QString adds = QString(QStringLiteral("+") + QString::number(add));
        QString subs = QString(QStringLiteral(" -") + QString::number(sub));
        QString file = options.text;

        options.text = QString(); // clear old text
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

        QRect r = options.rect;

        // don't draw over icon
        r.setX(r.x() + option.decorationSize.width() + 5);

        const QFontMetrics &fm = options.fontMetrics;

        // adds width
        int aw = fm.horizontalAdvance(adds);
        // subs width
        int sw = fm.horizontalAdvance(subs);

        // subtract this from total width of rect
        int totalw = r.width();
        totalw = totalw - (aw + sw);

        // get file name, elide if necessary
        QString filename = fm.elidedText(file, Qt::ElideRight, totalw);

        painter->drawText(r, Qt::AlignVCenter, filename);

        KColorScheme c;
        const auto red = c.shade(c.foreground(KColorScheme::NegativeText).color(), KColorScheme::MidlightShade, 1);
        const auto green = c.shade(c.foreground(KColorScheme::PositiveText).color(), KColorScheme::MidlightShade, 1);

        r.setX(r.x() + totalw);
        painter->setPen(green);
        painter->drawText(r, Qt::AlignVCenter, adds);

        painter->setPen(red);
        r.setX(r.x() + aw);
        painter->drawText(r, Qt::AlignVCenter, subs);

        painter->restore();
    }
};

static void createFileTree(QStandardItem *parent, const QString &basePath, const QVector<GitUtils::StatusItem> &files)
{
    QDir dir(basePath);
    const QString dirPath = dir.path() + QLatin1Char('/');
    QHash<QString, QStandardItem *> dir2Item;
    dir2Item[QString()] = parent;
    for (const auto &file : qAsConst(files)) {
        const QString filePath = QString::fromUtf8(file.file);
        /**
         * cheap file name computation
         * we do this A LOT, QFileInfo is very expensive just for this operation
         */
        const int slashIndex = filePath.lastIndexOf(QLatin1Char('/'));
        const QString fileName = (slashIndex < 0) ? filePath : filePath.mid(slashIndex + 1);
        const QString filePathName = (slashIndex < 0) ? QString() : filePath.left(slashIndex);
        const QString fullFilePath = dirPath + filePath;

        /**
         * construct the item with right directory prefix
         * already hang in directories in tree
         */
        KateProjectItem *fileItem = new KateProjectItem(KateProjectItem::File, fileName);
        fileItem->setData(fullFilePath, Qt::UserRole);
        fileItem->setData(file.statusChar, Qt::UserRole + 1);
        fileItem->setData(file.linesAdded, Qt::UserRole + 2);
        fileItem->setData(file.linesRemoved, Qt::UserRole + 3);

        // put in our item to the right directory parent
        KateProjectWorker::directoryParent(dir, dir2Item, filePathName)->appendRow(fileItem);
    }
}

CompareBranchesView::CompareBranchesView(QWidget *parent, const QString &gitPath, const QString fromB, const QString &toBr, QVector<GitUtils::StatusItem> items)
    : QWidget(parent)
    , m_gitDir(gitPath)
    , m_fromBr(fromB)
    , m_toBr(toBr)
{
    setLayout(new QVBoxLayout);

    QStandardItem root;
    createFileTree(&root, m_gitDir, items);

    m_model.clear();
    m_model.invisibleRootItem()->appendColumn(root.takeColumn(0));

    m_backBtn.setText(i18n("Back"));
    m_backBtn.setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    connect(&m_backBtn, &QPushButton::clicked, this, &CompareBranchesView::backClicked);
    layout()->addWidget(&m_backBtn);

    m_tree.setModel(&m_model);
    layout()->addWidget(&m_tree);

    m_tree.setHeaderHidden(true);
    m_tree.setEditTriggers(QTreeView::NoEditTriggers);
    m_tree.setItemDelegate(new DiffStyleDelegate(this));
    m_tree.expandAll();

    connect(&m_tree, &QTreeView::clicked, this, &CompareBranchesView::showDiff);
}

void CompareBranchesView::showDiff(const QModelIndex &idx)
{
    auto file = idx.data(Qt::UserRole).toString().remove(m_gitDir + QLatin1Char('/'));
    QProcess git;
    git.setWorkingDirectory(m_gitDir);
    QStringList args{QStringLiteral("diff"), QStringLiteral("%1...%2").arg(m_fromBr).arg(m_toBr), QStringLiteral("--"), file};
    git.start(QStringLiteral("git"), args, QProcess::ReadOnly);

    if (git.waitForStarted() && git.waitForFinished(-1)) {
        if (git.exitStatus() != QProcess::NormalExit || git.exitCode() != 0) {
            return;
        }
    }
    m_pluginView->showDiffInFixedView(git.readAllStandardOutput());
}

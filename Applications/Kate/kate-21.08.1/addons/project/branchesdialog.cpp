/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "branchesdialog.h"
#include "branchesdialogmodel.h"
#include "git/gitutils.h"
#include "kateprojectpluginview.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <QtConcurrentRun>

#include <KTextEditor/MainWindow>
#include <KTextEditor/Message>
#include <KTextEditor/View>

#include <KLocalizedString>

#include <kfts_fuzzy_match.h>

class BranchFilterModel : public QSortFilterProxyModel
{
public:
    BranchFilterModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
    }

    Q_SLOT void setFilterString(const QString &string)
    {
        beginResetModel();
        m_pattern = string;
        endResetModel();
    }

protected:
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override
    {
        if (m_pattern.isEmpty()) {
            const int l = sourceLeft.data(BranchesDialogModel::OriginalSorting).toInt();
            const int r = sourceRight.data(BranchesDialogModel::OriginalSorting).toInt();
            return l > r;
        }
        const int l = sourceLeft.data(BranchesDialogModel::FuzzyScore).toInt();
        const int r = sourceRight.data(BranchesDialogModel::FuzzyScore).toInt();
        return l < r;
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (m_pattern.isEmpty()) {
            return true;
        }

        int score = 0;
        const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
        const QString string = idx.data().toString();
        const bool res = kfts::fuzzy_match(m_pattern, string, score);
        sourceModel()->setData(idx, score, BranchesDialogModel::FuzzyScore);
        return res;
    }

private:
    QString m_pattern;
};

class StyleDelegate : public QStyledItemDelegate
{
public:
    StyleDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem options = option;
        initStyleOption(&options, index);

        auto name = index.data().toString();

        QVector<QTextLayout::FormatRange> formats;
        QTextCharFormat fmt;
        fmt.setForeground(options.palette.link());
        fmt.setFontWeight(QFont::Bold);

        const auto itemType = (BranchesDialogModel::ItemType)index.data(BranchesDialogModel::ItemTypeRole).toInt();
        const bool branchItem = itemType == BranchesDialogModel::BranchItem;
        const int offset = branchItem ? 0 : 2;

        formats = kfts::get_fuzzy_match_formats(m_filterString, name, offset, fmt);

        if (!branchItem) {
            name = QStringLiteral("+ ") + name;
        }

        const int nameLen = name.length();
        int len = 6;
        if (branchItem) {
            const auto refType = (GitUtils::RefType)index.data(BranchesDialogModel::RefType).toInt();
            using RefType = GitUtils::RefType;
            if (refType == RefType::Head) {
                name.append(QStringLiteral(" local"));
            } else if (refType == RefType::Remote) {
                name.append(QStringLiteral(" remote"));
                len = 7;
            }
        }
        QTextCharFormat lf;
        lf.setFontItalic(true);
        lf.setForeground(Qt::gray);
        formats.append({nameLen, len, lf});

        painter->save();

        // paint background
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else {
            painter->fillRect(option.rect, option.palette.base());
        }

        options.text = QString(); // clear old text
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

        // leave space for icon
        if (itemType == BranchesDialogModel::BranchItem) {
            painter->translate(25, 0);
        }
        kfts::paintItemViewText(painter, name, options, formats);

        painter->restore();
    }

public Q_SLOTS:
    void setFilterString(const QString &text)
    {
        m_filterString = text;
    }

private:
    QString m_filterString;
};

BranchesDialog::BranchesDialog(QWidget *window, KateProjectPluginView *pluginView, QString projectPath)
    : QuickDialog(nullptr, window)
    , m_projectPath(projectPath)
    , m_pluginView(pluginView)
{
    m_model = new BranchesDialogModel(this);
    m_proxyModel = new BranchFilterModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_treeView.setModel(m_proxyModel);

    auto delegate = new StyleDelegate(this);

    connect(&m_lineEdit, &QLineEdit::textChanged, this, [this, delegate](const QString &s) {
        static_cast<BranchFilterModel *>(m_proxyModel)->setFilterString(s);
        delegate->setFilterString(s);
    });
}

void BranchesDialog::openDialog(GitUtils::RefType r)
{
    m_lineEdit.setPlaceholderText(i18n("Select Branch..."));

    QVector<GitUtils::Branch> branches = GitUtils::getAllBranchesAndTags(m_projectPath, r);
    m_model->refresh(branches);

    reselectFirst();
    exec();
}

void BranchesDialog::slotReturnPressed()
{
    /** We want display role here */
    const auto branch = m_proxyModel->data(m_treeView.currentIndex(), Qt::DisplayRole).toString();
    const auto itemType = (BranchesDialogModel::ItemType)m_proxyModel->data(m_treeView.currentIndex(), BranchesDialogModel::ItemTypeRole).toInt();
    Q_ASSERT(itemType == BranchesDialogModel::BranchItem);

    m_branch = branch;
    Q_EMIT branchSelected(branch);

    clearLineEdit();
    hide();
}

void BranchesDialog::reselectFirst()
{
    QModelIndex index = m_proxyModel->index(0, 0);
    m_treeView.setCurrentIndex(index);
}

void BranchesDialog::sendMessage(const QString &plainText, bool warn)
{
    // use generic output view
    QVariantMap genericMessage;
    genericMessage.insert(QStringLiteral("type"), warn ? QStringLiteral("Error") : QStringLiteral("Info"));
    genericMessage.insert(QStringLiteral("category"), i18n("Git"));
    genericMessage.insert(QStringLiteral("categoryIcon"), QIcon(QStringLiteral(":/icons/icons/sc-apps-git.svg")));
    genericMessage.insert(QStringLiteral("text"), plainText);
    Q_EMIT m_pluginView->message(genericMessage);
}

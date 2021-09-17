/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "stashdialog.h"
#include "git/gitutils.h"
#include "gitwidget.h"
#include "kateprojectpluginview.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>
#include <QProcess>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTemporaryFile>
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

constexpr int StashIndexRole = Qt::UserRole + 2;

class StashFilterModel final : public QSortFilterProxyModel
{
public:
    StashFilterModel(QObject *parent = nullptr)
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
        return sourceLeft.data(FuzzyScore).toInt() < sourceRight.data(FuzzyScore).toInt();
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
        sourceModel()->setData(idx, score, FuzzyScore);
        return res;
    }

private:
    static constexpr int FuzzyScore = Qt::UserRole + 1;
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

        QString name = index.data().toString();

        QVector<QTextLayout::FormatRange> formats;

        int colon = name.indexOf(QLatin1Char(':'));
        QString stashMessage = name.mid(colon + 1, name.length() - (colon + 1));
        ++colon;

        QTextCharFormat bold;
        bold.setFontWeight(QFont::Bold);
        formats.append({0, colon, bold});

        QTextCharFormat fmt;
        fmt.setForeground(options.palette.link());
        fmt.setFontWeight(QFont::Bold);
        auto resFmts = kfts::get_fuzzy_match_formats(m_filterString, stashMessage, colon, fmt);

        formats.append(resFmts);

        painter->save();

        //        // paint background
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else {
            painter->fillRect(option.rect, option.palette.base());
        }

        options.text = QString(); // clear old text
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

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

StashDialog::StashDialog(QWidget *parent, QWidget *window, const QString &gitPath)
    : QuickDialog(parent, window)
    , m_gitPath(gitPath)
{
    m_model = new QStandardItemModel(this);
    m_proxyModel = new StashFilterModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_treeView.setModel(m_proxyModel);

    StyleDelegate *delegate = new StyleDelegate(this);
    m_treeView.setItemDelegateForColumn(0, delegate);
    connect(&m_lineEdit, &QLineEdit::textChanged, this, [this, delegate](const QString &string) {
        m_proxyModel->setFilterString(string);
        delegate->setFilterString(string);
        // reselect first
        m_treeView.setCurrentIndex(m_proxyModel->index(0, 0));
    });
    m_proxyModel->setFilterRole(Qt::DisplayRole);
}

void StashDialog::openDialog(StashMode m)
{
    m_model->clear();

    switch (m) {
    case StashMode::Stash:
    case StashMode::StashKeepIndex:
    case StashMode::StashUntrackIncluded:
        m_lineEdit.setPlaceholderText(i18n("Stash message (optional). Enter to confirm, Esc to leave."));
        m_currentMode = m;
        break;
    case StashMode::StashPop:
    case StashMode::StashDrop:
    case StashMode::StashApply:
    case StashMode::ShowStashContent:
        m_lineEdit.setPlaceholderText(i18n("Type to filter, Enter to pop stash, Esc to leave."));
        m_currentMode = m;
        getStashList();
        break;
    case StashMode::StashApplyLast:
        applyStash({});
        return;
    case StashMode::StashPopLast:
        popStash({});
        return;
    default:
        return;
    }

    // trigger reselect first
    m_lineEdit.textChanged(QString());
    exec();
}

void StashDialog::slotReturnPressed()
{
    switch (m_currentMode) {
    case StashMode::Stash:
        stash(false, false);
        break;
    case StashMode::StashKeepIndex:
        stash(true, false);
        break;
    case StashMode::StashUntrackIncluded:
        stash(false, true);
        break;
    case StashMode::StashApply:
        applyStash(m_treeView.currentIndex().data(StashIndexRole).toByteArray());
        break;
    case StashMode::StashPop:
        popStash(m_treeView.currentIndex().data(StashIndexRole).toByteArray());
        break;
    case StashMode::StashDrop:
        dropStash(m_treeView.currentIndex().data(StashIndexRole).toByteArray());
        break;
    case StashMode::ShowStashContent:
        showStash(m_treeView.currentIndex().data(StashIndexRole).toByteArray());
        break;
    default:
        break;
    }

    hide();
}

QProcess *StashDialog::gitp()
{
    auto git = new QProcess(this);
    git->setProgram(QStringLiteral("git"));
    git->setWorkingDirectory(m_gitPath);
    return git;
}

void StashDialog::stash(bool keepIndex, bool includeUntracked)
{
    QStringList args{QStringLiteral("stash"), QStringLiteral("-q")};

    if (keepIndex) {
        args.append(QStringLiteral("--keep-index"));
    }
    if (includeUntracked) {
        args.append(QStringLiteral("-u"));
    }

    if (!m_lineEdit.text().isEmpty()) {
        args.append(QStringLiteral("-m"));
        args.append(m_lineEdit.text());
    }

    auto git = gitp();
    connect(git, &QProcess::finished, this, [this, git](int exitCode, QProcess::ExitStatus es) {
        if (es != QProcess::NormalExit || exitCode != 0) {
            qWarning() << git->errorString();
            Q_EMIT message(i18n("Failed to stash changes %1", QString::fromUtf8(git->readAllStandardError())), true);
        } else {
            Q_EMIT message(i18n("Changes stashed successfully."), false);
        }
        Q_EMIT done();
        git->deleteLater();
    });
    git->setArguments(args);
    git->start(QProcess::ReadOnly);
}

void StashDialog::getStashList()
{
    auto git = gitp();
    git->setArguments({QStringLiteral("stash"), QStringLiteral("list")});
    git->start(QProcess::ReadOnly);

    QList<QByteArray> stashList;
    if (git->waitForStarted() && git->waitForFinished(-1)) {
        if (git->exitStatus() == QProcess::NormalExit && git->exitCode() == 0) {
            stashList = git->readAllStandardOutput().split('\n');
        } else {
            Q_EMIT message(i18n("Failed to get stash list. Error: ") + QString::fromUtf8(git->readAll()), true);
        }
    }

    // format stash@{}: message
    for (const auto &stash : stashList) {
        if (!stash.startsWith("stash@{")) {
            continue;
        }
        int brackCloseIdx = stash.indexOf('}', 7);

        if (brackCloseIdx < 0) {
            continue;
        }

        QByteArray stashIdx = stash.mid(0, brackCloseIdx + 1);

        QStandardItem *item = new QStandardItem(QString::fromUtf8(stash));
        item->setData(stashIdx, StashIndexRole);
        m_model->appendRow(item);
    }
}

void StashDialog::popStash(const QByteArray &index, const QString &command)
{
    auto git = gitp();
    QStringList args{QStringLiteral("stash"), command};
    if (!index.isEmpty()) {
        args.append(QString::fromUtf8(index));
    }

    connect(git, &QProcess::finished, this, [this, command, git](int exitCode, QProcess::ExitStatus es) {
        if (es != QProcess::NormalExit || exitCode != 0) {
            if (command == QLatin1String("apply")) {
                Q_EMIT message(i18n("Failed to apply stash. Error: ") + QString::fromUtf8(git->readAll()), true);
            } else if (command == QLatin1String("drop")) {
                Q_EMIT message(i18n("Failed to drop stash. Error: ") + QString::fromUtf8(git->readAll()), true);
            } else {
                Q_EMIT message(i18n("Failed to pop stash. Error: ") + QString::fromUtf8(git->readAll()), true);
            }
        } else {
            if (command == QLatin1String("apply")) {
                Q_EMIT message(i18n("Stash applied successfully."), false);
            } else if (command == QLatin1String("drop")) {
                Q_EMIT message(i18n("Stash dropped successfully."), false);
            } else {
                Q_EMIT message(i18n("Stash popped successfully."), false);
            }
        }
        Q_EMIT done();
        git->deleteLater();
    });
    git->setArguments(args);
    git->start(QProcess::ReadOnly);
}

void StashDialog::applyStash(const QByteArray &index)
{
    popStash(index, QStringLiteral("apply"));
}

void StashDialog::dropStash(const QByteArray &index)
{
    popStash(index, QStringLiteral("drop"));
}

void StashDialog::showStash(const QByteArray &index)
{
    if (index.isEmpty()) {
        return;
    }
    auto git = gitp();

    QStringList args{QStringLiteral("stash"), QStringLiteral("show"), QStringLiteral("-p"), QString::fromUtf8(index)};

    connect(git, &QProcess::finished, this, [this, git](int exitCode, QProcess::ExitStatus es) {
        if (es != QProcess::NormalExit || exitCode != 0) {
            Q_EMIT message(i18n("Show stash failed. Error: ") + QString::fromUtf8(git->readAll()), true);
        } else {
            Q_EMIT showStashDiff(git->readAllStandardOutput());
        }
        Q_EMIT done();
        git->deleteLater();
    });

    git->setArguments(args);
    git->start(QProcess::ReadOnly);
}

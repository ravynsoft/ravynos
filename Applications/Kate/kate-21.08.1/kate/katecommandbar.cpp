/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "katecommandbar.h"
#include "commandmodel.h"

#include <QAction>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QTextLayout>
#include <QTreeView>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KLocalizedString>

#include <kfts_fuzzy_match.h>

class CommandBarFilterModel : public QSortFilterProxyModel
{
public:
    CommandBarFilterModel(QObject *parent = nullptr)
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
        const int l = sourceLeft.data(CommandModel::Score).toInt();
        const int r = sourceRight.data(CommandModel::Score).toInt();
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
        const QStringView actionName = string.splitRef(QLatin1Char(':')).at(1);
        const bool res = kfts::fuzzy_match(m_pattern, actionName.trimmed(), score);
        sourceModel()->setData(idx, score, CommandModel::Score);
        return res;
    }

private:
    QString m_pattern;
};

class CommandBarStyleDelegate : public QStyledItemDelegate
{
public:
    CommandBarStyleDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem options = option;
        initStyleOption(&options, index);

        painter->save();

        // paint background
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else {
            painter->fillRect(option.rect, option.palette.base());
        }

        options.text = QString(); // clear old text
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

        const auto original = index.data().toString();
        const bool rtl = original.isRightToLeft();
        if (rtl) {
            painter->translate(-20, 0);
        } else {
            painter->translate(20, 0);
        }

        // must use QString here otherwise fuzzy matching wont
        // work very well
        QString str = original;
        int componentIdx = original.indexOf(QLatin1Char(':'));
        int actionNameStart = 0;
        if (componentIdx > 0) {
            actionNameStart = componentIdx + 2;
            // + 2 because there is a space after colon
            str = str.mid(actionNameStart);
        }

        QVector<QTextLayout::FormatRange> formats;
        if (componentIdx > 0) {
            QTextCharFormat gray;
            gray.setForeground(Qt::gray);
            formats.append({0, componentIdx, gray});
        }

        QTextCharFormat fmt;
        fmt.setForeground(options.palette.link().color());
        fmt.setFontWeight(QFont::Bold);

        const auto f = kfts::get_fuzzy_match_formats(m_filterString, str, componentIdx + 2, fmt);
        formats.append(f);

        kfts::paintItemViewText(painter, original, options, std::move(formats));

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

class ShortcutStyleDelegate : public QStyledItemDelegate
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    static constexpr auto SkipEmptyParts = QString::SkipEmptyParts;
#else
    static constexpr auto SkipEmptyParts = Qt::SkipEmptyParts;
#endif
public:
    ShortcutStyleDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    static QStringList splitShortcutString(const QString &shortcutString)
    {
        return shortcutString.split(QLatin1String(", "), SkipEmptyParts);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem options = option;
        initStyleOption(&options, index);
        painter->save();

        const auto shortcutString = index.data().toString();

        // paint background
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else {
            painter->fillRect(option.rect, option.palette.base());
        }

        options.text = QString(); // clear old text
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

        if (!shortcutString.isEmpty()) {
            /**
             * Shortcut string splitting
             *
             * We do it in two steps
             * 1. Split on ", " so that if we have multi modifier shortcuts they are nicely
             *    splitted into strings.
             * 2. Split each shortcut from step 1 into individual string.
             *
             * Example:
             *
             * "Ctrl+,, Alt+:"
             * Step 1: [ "Ctrl+," , "Alt+:"]
             * Step 2: [ "Ctrl", ",", "Alt", ":"]
             */
            const auto spaceSplitted = splitShortcutString(shortcutString);
            QStringList list;
            list.reserve(spaceSplitted.size() * 2);
            for (const QString &shortcut : spaceSplitted) {
                list += shortcut.split(QLatin1Char('+'), SkipEmptyParts);
                if (shortcut.endsWith(QLatin1String("+"))) {
                    list.append(QStringLiteral("+"));
                }
            }
            Q_ASSERT(!list.isEmpty());

            /**
             * Create rects for each string from the previous step
             *
             * @todo boundingRect may give issues here, use horizontalAdvance
             * @todo We probably dont need the full rect, just the width so the
             * "btns" vector can just be vector<pair<int, string>>
             */
            QVector<QPair<QRect, QString>> btns;
            btns.reserve(list.size());
            const int height = options.rect.height();
            for (const QString &text : list) {
                if (text.isEmpty()) {
                    continue;
                }
                QRect r = option.fontMetrics.boundingRect(text);
                // this happens on gnome so we manually decrease the
                // height a bit
                if (r.height() == height) {
                    r.setHeight(r.height() - 4);
                }
                r.setWidth(r.width() + 8);
                btns.append({r, text});
            }
            Q_ASSERT(!btns.isEmpty());

            // we have nothing, just return
            if (btns.isEmpty()) {
                return;
            }

            const auto plusRect = option.fontMetrics.boundingRect(QLatin1Char('+'));

            // draw them
            int x = option.rect.x();
            const int y = option.rect.y();
            const int plusY = option.rect.y() + plusRect.height() / 2;
            const int total = btns.size();

            // make sure our rects are nicely V-center aligned in the row
            painter->translate(QPoint(0, (option.rect.height() - btns.at(0).first.height()) / 2));

            int i = 0;
            painter->setRenderHint(QPainter::Antialiasing);
            for (const auto &btn : btns) {
                painter->setPen(Qt::NoPen);
                const QRect &rect = btn.first;

                QRect buttonRect(x, y, rect.width(), rect.height());

                // draw rounded rect shadow
                auto shadowRect = buttonRect.translated(0, 1);
                painter->setBrush(option.palette.shadow());
                painter->drawRoundedRect(shadowRect, 3, 3);

                // draw rounded rect itself
                painter->setBrush(option.palette.button());
                painter->drawRoundedRect(buttonRect, 3, 3);

                // draw text inside rounded rect
                painter->setPen(option.palette.buttonText().color());
                painter->drawText(buttonRect, Qt::AlignCenter, btn.second);

                // draw '+'
                if (i + 1 < total) {
                    x += rect.width() + 5;
                    painter->drawText(QPoint(x, plusY + (rect.height() / 2)), QStringLiteral("+"));
                    x += plusRect.width() + 5;
                }
                i++;
            }
        }

        painter->restore();
    }
};

KateCommandBar::KateCommandBar(QWidget *parent)
    : QMenu(parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(4, 4, 4, 4);
    setLayout(layout);

    m_lineEdit = new QLineEdit(this);
    setFocusProxy(m_lineEdit);

    layout->addWidget(m_lineEdit);

    m_treeView = new QTreeView();
    layout->addWidget(m_treeView, 1);
    m_treeView->setTextElideMode(Qt::ElideLeft);
    m_treeView->setUniformRowHeights(true);

    m_model = new CommandModel(this);

    CommandBarStyleDelegate *delegate = new CommandBarStyleDelegate(this);
    ShortcutStyleDelegate *del = new ShortcutStyleDelegate(this);
    m_treeView->setItemDelegateForColumn(0, delegate);
    m_treeView->setItemDelegateForColumn(1, del);

    m_proxyModel = new CommandBarFilterModel(this);
    m_proxyModel->setFilterRole(Qt::DisplayRole);
    m_proxyModel->setSortRole(CommandModel::Score);
    m_proxyModel->setFilterKeyColumn(0);

    connect(m_lineEdit, &QLineEdit::returnPressed, this, &KateCommandBar::slotReturnPressed);
    connect(m_lineEdit, &QLineEdit::textChanged, m_proxyModel, &CommandBarFilterModel::setFilterString);
    connect(m_lineEdit, &QLineEdit::textChanged, delegate, &CommandBarStyleDelegate::setFilterString);
    connect(m_lineEdit, &QLineEdit::textChanged, this, [this]() {
        m_treeView->viewport()->update();
        reselectFirst();
    });
    connect(m_treeView, &QTreeView::clicked, this, &KateCommandBar::slotReturnPressed);

    m_proxyModel->setSourceModel(m_model);
    m_treeView->setSortingEnabled(true);
    m_treeView->setModel(m_proxyModel);

    m_treeView->installEventFilter(this);
    m_lineEdit->installEventFilter(this);

    m_treeView->setHeaderHidden(true);
    m_treeView->setRootIsDecorated(false);
    m_treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeView->setSelectionMode(QTreeView::SingleSelection);

    setHidden(true);
}

void KateCommandBar::updateBar(const QList<KActionCollection *> &actionCollections, int totalActions)
{
    QVector<QPair<QString, QAction *>> actionList;
    actionList.reserve(totalActions);

    for (const auto collection : actionCollections) {
        const QList<QAction *> collectionActions = collection->actions();
        const QString componentName = collection->componentDisplayName();
        for (const auto action : collectionActions) {
            // sanity + empty check ensures displayable actions and removes ourself
            // from the action list
            if (action && !action->text().isEmpty()) {
                actionList.append({componentName, action});
            }
        }
    }

    m_model->refresh(std::move(actionList));
    reselectFirst();

    updateViewGeometry();
    show();
    setFocus();
}

bool KateCommandBar::eventFilter(QObject *obj, QEvent *event)
{
    // catch key presses + shortcut overrides to allow to have ESC as application wide shortcut, too, see bug 409856
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (obj == m_lineEdit) {
            const bool forward2list = (keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down) || (keyEvent->key() == Qt::Key_PageUp)
                || (keyEvent->key() == Qt::Key_PageDown);
            if (forward2list) {
                QCoreApplication::sendEvent(m_treeView, event);
                return true;
            }

            if (keyEvent->key() == Qt::Key_Escape) {
                m_lineEdit->clear();
                keyEvent->accept();
                hide();
                return true;
            }
        } else {
            const bool forward2input = (keyEvent->key() != Qt::Key_Up) && (keyEvent->key() != Qt::Key_Down) && (keyEvent->key() != Qt::Key_PageUp)
                && (keyEvent->key() != Qt::Key_PageDown) && (keyEvent->key() != Qt::Key_Tab) && (keyEvent->key() != Qt::Key_Backtab);
            if (forward2input) {
                QCoreApplication::sendEvent(m_lineEdit, event);
                return true;
            }
        }
    }

    // hide on focus out, if neither input field nor list have focus!
    else if (event->type() == QEvent::FocusOut && !(m_lineEdit->hasFocus() || m_treeView->hasFocus())) {
        m_lineEdit->clear();
        hide();
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

void KateCommandBar::slotReturnPressed()
{
    auto act = m_proxyModel->data(m_treeView->currentIndex(), Qt::UserRole).value<QAction *>();
    if (act) {
        // if the action is a menu, we take all its actions
        // and reload our dialog with these instead.
        if (auto menu = act->menu()) {
            auto menuActions = menu->actions();
            QVector<QPair<QString, QAction *>> list;
            list.reserve(menuActions.size());

            // if there are no actions, trigger load actions
            // this happens with some menus that are loaded on demand
            if (menuActions.size() == 0) {
                Q_EMIT menu->aboutToShow();
                menuActions = menu->actions();
            }

            for (auto menuAction : qAsConst(menuActions)) {
                if (menuAction) {
                    list.append({KLocalizedString::removeAcceleratorMarker(act->text()), menuAction});
                }
            }
            m_model->refresh(list);
            m_lineEdit->clear();
            return;
        } else {
            m_model->actionTriggered(act->text());
            act->trigger();
        }
    }
    m_lineEdit->clear();
    hide();
}

void KateCommandBar::reselectFirst()
{
    QModelIndex index = m_proxyModel->index(0, 0);
    m_treeView->setCurrentIndex(index);
}

void KateCommandBar::updateViewGeometry()
{
    m_treeView->resizeColumnToContents(0);
    m_treeView->resizeColumnToContents(1);

    const QSize centralSize = parentWidget()->size();

    // width: 2.4 of editor, height: 1/2 of editor
    const QSize viewMaxSize(centralSize.width() / 2.4, centralSize.height() / 2);

    // Position should be central over window
    const int xPos = std::max(0, (centralSize.width() - viewMaxSize.width()) / 2);
    const int yPos = std::max(0, (centralSize.height() - viewMaxSize.height()) * 1 / 4);

    const QPoint p(xPos, yPos);
    move(p + parentWidget()->pos());

    this->setFixedSize(viewMaxSize);
}

void KateCommandBar::setLastUsedCmdBarActions(const QVector<QString> &actionNames)
{
    return m_model->setLastUsedActions(actionNames);
}

QVector<QString> KateCommandBar::lastUsedCmdBarActions() const
{
    return m_model->lastUsedActions();
}

/***************************************************************************
 *   This file is part of Kate build plugin                                *
 *   SPDX-FileCopyrightText: 2014 Kåre Särs <kare.sars@iki.fi>                           *
 *                                                                         *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 ***************************************************************************/
#include "SelectTargetView.h"
#include "TargetHtmlDelegate.h"

#include <QDebug>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QTimer>

class TargetFilterProxyModel : public QSortFilterProxyModel
{
public:
    TargetFilterProxyModel(QObject *parent)
        : QSortFilterProxyModel(parent)
    {
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (m_filter.isEmpty()) {
            return true;
        }

        QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
        QString name = index0.data().toString();

        if (index0.internalId() == 0xffffffff) {
            int i = 0;
            auto childIndex = index0.model()->index(i, 0, index0);
            while (childIndex.data().isValid()) {
                name = childIndex.data().toString();
                if (name.contains(m_filter, Qt::CaseInsensitive)) {
                    return true;
                }
                i++;
                childIndex = index0.model()->index(i, 0, index0);
            }
            return false;
        }
        return name.contains(m_filter, Qt::CaseInsensitive);
    }

    void setFilter(const QString &filter)
    {
        m_filter = filter;
        invalidateFilter();
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (!index.isValid()) {
            return Qt::NoItemFlags;
        }
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    QString m_filter;
};

SelectTargetView::SelectTargetView(QAbstractItemModel *model, QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    m_proxyModel = new TargetFilterProxyModel(this);
    m_proxyModel->setSourceModel(model);
    u_treeView->setModel(m_proxyModel);
    u_treeView->expandAll();
    u_treeView->resizeColumnToContents(0);
    u_treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);

    setFocusProxy(u_filterEdit);

    connect(u_filterEdit, &QLineEdit::textChanged, this, &SelectTargetView::setFilter);
    connect(u_treeView, &QTreeView::doubleClicked, this, &SelectTargetView::accept);

    u_filterEdit->installEventFilter(this);
}

void SelectTargetView::setFilter(const QString &filter)
{
    m_proxyModel->setFilter(filter);
    u_treeView->expandAll();
}

const QModelIndex SelectTargetView::currentIndex() const
{
    return m_proxyModel->mapToSource(u_treeView->currentIndex());
}

void SelectTargetView::setCurrentIndex(const QModelIndex &index)
{
    u_treeView->setCurrentIndex(m_proxyModel->mapFromSource(index));
}

bool SelectTargetView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (obj == u_filterEdit) {
            if ((keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down) || (keyEvent->key() == Qt::Key_PageUp)
                || (keyEvent->key() == Qt::Key_PageDown)) {
                QCoreApplication::sendEvent(u_treeView, event);
                return true;
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

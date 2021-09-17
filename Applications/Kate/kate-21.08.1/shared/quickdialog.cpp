/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "quickdialog.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QVBoxLayout>

#include <QDebug>

QuickDialog::QuickDialog(QWidget *parent, QWidget *mainWindow)
    : QMenu(parent)
    , m_mainWindow(mainWindow)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(4, 4, 4, 4);
    setLayout(layout);

    setFocusProxy(&m_lineEdit);

    layout->addWidget(&m_lineEdit);

    layout->addWidget(&m_treeView, 1);
    m_treeView.setTextElideMode(Qt::ElideLeft);
    m_treeView.setUniformRowHeights(true);

    connect(&m_lineEdit, &QLineEdit::returnPressed, this, &QuickDialog::slotReturnPressed);
    // user can add this as necessary
    //    connect(m_lineEdit, &QLineEdit::textChanged, delegate, &StyleDelegate::setFilterString);
    connect(&m_lineEdit, &QLineEdit::textChanged, this, [this]() {
        m_treeView.viewport()->update();
    });
    connect(&m_treeView, &QTreeView::clicked, this, &QuickDialog::slotReturnPressed);
    m_treeView.setSortingEnabled(true);

    m_treeView.installEventFilter(this);
    m_lineEdit.installEventFilter(this);

    m_treeView.setHeaderHidden(true);
    m_treeView.setRootIsDecorated(false);
    m_treeView.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeView.setSelectionMode(QTreeView::SingleSelection);

    updateViewGeometry();
    setFocus();
}

bool QuickDialog::eventFilter(QObject *obj, QEvent *event)
{
    // catch key presses + shortcut overrides to allow to have ESC as application wide shortcut, too, see bug 409856
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (obj == &m_lineEdit) {
            const bool forward2list = (keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down) || (keyEvent->key() == Qt::Key_PageUp)
                || (keyEvent->key() == Qt::Key_PageDown);
            if (forward2list) {
                QCoreApplication::sendEvent(&m_treeView, event);
                return true;
            }

            if (keyEvent->key() == Qt::Key_Escape) {
                clearLineEdit();
                keyEvent->accept();
                hide();
                return true;
            }
        } else {
            const bool forward2input = (keyEvent->key() != Qt::Key_Up) && (keyEvent->key() != Qt::Key_Down) && (keyEvent->key() != Qt::Key_PageUp)
                && (keyEvent->key() != Qt::Key_PageDown) && (keyEvent->key() != Qt::Key_Tab) && (keyEvent->key() != Qt::Key_Backtab);
            if (forward2input) {
                QCoreApplication::sendEvent(&m_lineEdit, event);
                return true;
            }
        }
    }

    // hide on focus out, if neither input field nor list have focus!
    else if (event->type() == QEvent::FocusOut && !(m_lineEdit.hasFocus() || m_treeView.hasFocus())) {
        clearLineEdit();
        hide();
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

void QuickDialog::updateViewGeometry()
{
    if (!m_mainWindow)
        return;

    const QSize centralSize = m_mainWindow->size();

    // width: 2.4 of editor, height: 1/2 of editor
    const QSize viewMaxSize(centralSize.width() / 2.4, centralSize.height() / 2);

    // Position should be central over window
    const int xPos = std::max(0, (centralSize.width() - viewMaxSize.width()) / 2);
    const int yPos = std::max(0, (centralSize.height() - viewMaxSize.height()) * 1 / 4);
    const QPoint p(xPos, yPos);
    move(p + m_mainWindow->pos());

    this->setFixedSize(viewMaxSize);
}

void QuickDialog::clearLineEdit()
{
    const QSignalBlocker block(m_lineEdit);
    m_lineEdit.clear();
}

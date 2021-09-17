/***************************************************************************
 *   This file is part of Kate search plugin                               *
 *   SPDX-FileCopyrightText: 2014 Kåre Särs <kare.sars@iki.fi>                           *
 *                                                                         *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 ***************************************************************************/

#include "TargetHtmlDelegate.h"
#include "TargetModel.h"

#include <KLocalizedString>
#include <QAbstractTextDocumentLayout>
#include <QCompleter>
#include <QFileSystemModel>
#include <QLineEdit>
#include <QModelIndex>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QToolButton>

#include "UrlInserter.h"

#include <QDebug>

TargetHtmlDelegate::TargetHtmlDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , m_isEditing(false)
{
    connect(this, &TargetHtmlDelegate::sendEditStart, this, &TargetHtmlDelegate::editStarted);
}

TargetHtmlDelegate::~TargetHtmlDelegate()
{
}

void TargetHtmlDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    QTextDocument doc;

    QString str;
    if (!index.parent().isValid()) {
        if (index.column() == 0) {
            str = i18nc("T as in Target set", "<B>T:</B> %1", index.data().toString().toHtmlEscaped());
        } else if (index.column() == 1) {
            str = i18nc("D as in working Directory", "<B>Dir:</B> %1", index.data().toString().toHtmlEscaped());
        }
    } else {
        str = index.data().toString().toHtmlEscaped();
    }

    if (option.state & QStyle::State_Selected) {
        str = QStringLiteral("<font color=\"%1\">").arg(option.palette.highlightedText().color().name()) + str + QStringLiteral("</font>");
    }
    doc.setHtml(str);
    doc.setDocumentMargin(2);

    painter->save();

    // paint background
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->fillRect(option.rect, option.palette.base());
    }

    options.text = QString(); // clear old text
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

    // draw text
    painter->translate(option.rect.x(), option.rect.y());
    if (index.column() == 0 && index.internalId() != TargetModel::InvalidIndex) {
        painter->translate(25, 0);
    }
    doc.drawContents(painter);

    painter->restore();
}

QSize TargetHtmlDelegate::sizeHint(const QStyleOptionViewItem & /* option */, const QModelIndex &index) const
{
    QTextDocument doc;
    doc.setHtml(index.data().toString().toHtmlEscaped());
    doc.setDocumentMargin(2);
    if (index.column() == 0 && index.internalId() != TargetModel::InvalidIndex) {
        return doc.size().toSize() + QSize(30, 0); // add margin for the check-box;
    }
    return doc.size().toSize();
}

QWidget *TargetHtmlDelegate::createEditor(QWidget *dparent, const QStyleOptionViewItem & /* option */, const QModelIndex &index) const
{
    QWidget *editor;
    if (index.internalId() == TargetModel::InvalidIndex && index.column() == 1) {
        UrlInserter *requester = new UrlInserter(parent()->property("docUrl").toUrl(), dparent);
        requester->setReplace(true);
        editor = requester;
        editor->setToolTip(i18n("Leave empty to use the directory of the current document.\nAdd search directories by adding paths separated by ';'"));
    } else if (index.column() == 1) {
        UrlInserter *urlEditor = new UrlInserter(parent()->property("docUrl").toUrl(), dparent);
        editor = urlEditor;
        editor->setToolTip(i18n("Use:\n\"%f\" for current file\n\"%d\" for directory of current file\n\"%n\" for current file name without suffix"));
    } else {
        QLineEdit *e = new QLineEdit(dparent);
        QCompleter *completer = new QCompleter(e);
        QFileSystemModel *model = new QFileSystemModel(e);
        model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
        completer->setModel(model);
        e->setCompleter(completer);
        editor = e;
    }
    editor->setAutoFillBackground(true);
    Q_EMIT sendEditStart();
    connect(editor, &QWidget::destroyed, this, &TargetHtmlDelegate::editEnded);
    return editor;
}

void TargetHtmlDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    if (index.column() == 1) {
        UrlInserter *ledit = static_cast<UrlInserter *>(editor);
        if (ledit) {
            ledit->lineEdit()->setText(value);
        }
    } else {
        QLineEdit *ledit = static_cast<QLineEdit *>(editor);
        if (ledit) {
            ledit->setText(value);
        }
    }
}

void TargetHtmlDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QString value;
    if (index.column() == 1) {
        UrlInserter *ledit = static_cast<UrlInserter *>(editor);
        value = ledit->lineEdit()->text();
    } else {
        QLineEdit *ledit = static_cast<QLineEdit *>(editor);
        value = ledit->text();
    }
    model->setData(index, value, Qt::EditRole);
}

void TargetHtmlDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect rect = option.rect;
    int heightDiff = QToolButton().sizeHint().height() - rect.height();
    int half = heightDiff / 2;
    rect.adjust(0, -half, 0, heightDiff - half);
    if (index.column() == 0 && index.internalId() != TargetModel::InvalidIndex) {
        rect.adjust(25, 0, 0, 0);
    }
    editor->setGeometry(rect);
}

void TargetHtmlDelegate::editStarted()
{
    m_isEditing = true;
}
void TargetHtmlDelegate::editEnded()
{
    m_isEditing = false;
}
bool TargetHtmlDelegate::isEditing() const
{
    return m_isEditing;
}

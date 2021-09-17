/***************************************************************************
 *   This file is part of Kate search plugin                               *
 *   SPDX-FileCopyrightText: 2014 Kåre Särs <kare.sars@iki.fi>                           *
 *                                                                         *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 ***************************************************************************/

#include "UrlInserter.h"
#include <KLocalizedString>
#include <QCompleter>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QIcon>

UrlInserter::UrlInserter(const QUrl &startUrl, QWidget *parent)
    : QWidget(parent)
    , m_startUrl(startUrl)
    , m_replace(false)
{
    m_lineEdit = new QLineEdit();
    QCompleter *completer = new QCompleter(m_lineEdit);
    QFileSystemModel *model = new QFileSystemModel(m_lineEdit);
    model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Executable);
    completer->setModel(model);
    m_lineEdit->setCompleter(completer);

    m_toolButton = new QToolButton();
    m_toolButton->setIcon(QIcon::fromTheme(QStringLiteral("archive-insert-directory")));
    m_toolButton->setToolTip(i18n("Insert path"));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_lineEdit);
    layout->addWidget(m_toolButton);
    setFocusProxy(m_lineEdit);
    connect(m_toolButton, &QToolButton::clicked, this, &UrlInserter::insertFolder);
}

void UrlInserter::insertFolder()
{
    QUrl startUrl;
    if (QFileInfo::exists(m_lineEdit->text())) {
        startUrl.setPath(m_lineEdit->text());
    } else {
        startUrl = m_startUrl;
    }
    QString folder = QFileDialog::getExistingDirectory(this, i18n("Select directory to insert"), startUrl.path());
    if (!folder.isEmpty()) {
        if (!m_replace) {
            m_lineEdit->insert(folder);
        } else {
            m_lineEdit->setText(folder);
        }
    }
}

void UrlInserter::setReplace(bool replace)
{
    m_replace = replace;
}

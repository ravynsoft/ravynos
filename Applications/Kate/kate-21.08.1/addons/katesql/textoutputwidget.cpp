/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "textoutputwidget.h"

#include <KLocalizedString>
#include <KToolBar>
#include <QAction>

#include <QDateTime>
#include <QFontDatabase>
#include <QLayout>
#include <QStyle>
#include <QTextEdit>

TextOutputWidget::TextOutputWidget(QWidget *parent)
    : QWidget(parent)
{
    m_succesTextColor = QColor::fromRgb(3, 191, 3);
    m_succesBackgroundColor = QColor::fromRgb(231, 247, 231);

    m_errorTextColor = QColor::fromRgb(191, 3, 3);
    m_errorBackgroundColor = QColor::fromRgb(247, 231, 231);

    m_layout = new QHBoxLayout(this);

    m_output = new QTextEdit();
    m_output->setReadOnly(true);

    QFont fixedFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    m_output->setCurrentFont(fixedFont);

    KToolBar *toolbar = new KToolBar(this);
    toolbar->setOrientation(Qt::Vertical);
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // ensure reasonable icons sizes, like e.g. the quick-open and co. icons
    // the normal toolbar sizes are TOO large, e.g. for scaled stuff even more!
    const int iconSize = style()->pixelMetric(QStyle::PM_ButtonIconSize, nullptr, this);
    toolbar->setIconSize(QSize(iconSize, iconSize));

    /// TODO: disable actions if no results are displayed

    QAction *action = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear")), i18nc("@action:intoolbar", "Clear"), this);
    toolbar->addAction(action);
    connect(action, &QAction::triggered, m_output, &QTextEdit::clear);
    m_layout->addWidget(toolbar);
    m_layout->addWidget(m_output, 1);
    m_layout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_layout);
}

TextOutputWidget::~TextOutputWidget()
{
}

void TextOutputWidget::showErrorMessage(const QString &message)
{
    QColor previousBackgroundColor = m_output->textBackgroundColor();
    QColor previousColor = m_output->textColor();

    m_output->setTextBackgroundColor(m_errorBackgroundColor);
    m_output->setTextColor(m_errorTextColor);

    writeMessage(message);

    m_output->setTextBackgroundColor(previousBackgroundColor);
    m_output->setTextColor(previousColor);
}

void TextOutputWidget::showSuccessMessage(const QString &message)
{
    QColor previousBackgroundColor = m_output->textBackgroundColor();
    QColor previousColor = m_output->textColor();

    m_output->setTextBackgroundColor(m_succesBackgroundColor);
    m_output->setTextColor(m_succesTextColor);

    writeMessage(message);

    m_output->setTextBackgroundColor(previousBackgroundColor);
    m_output->setTextColor(previousColor);
}

void TextOutputWidget::writeMessage(const QString &msg)
{
    m_output->append(QStringLiteral("%1: %2\n").arg(QDateTime::currentDateTime().toString(Qt::SystemLocaleDate), msg));

    raise();
}

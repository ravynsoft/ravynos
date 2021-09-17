/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef TEXTOUTPUTWIDGET_H
#define TEXTOUTPUTWIDGET_H

class QHBoxLayout;
class QTextEdit;

#include "connection.h"
#include <QWidget>

class TextOutputWidget : public QWidget
{
    Q_OBJECT

public:
    TextOutputWidget(QWidget *parent = nullptr);
    ~TextOutputWidget() override;

public Q_SLOTS:
    void showErrorMessage(const QString &message);
    void showSuccessMessage(const QString &message);

private:
    void writeMessage(const QString &msg);

private:
    QHBoxLayout *m_layout;
    QTextEdit *m_output;

    QColor m_succesTextColor;
    QColor m_succesBackgroundColor;
    QColor m_errorTextColor;
    QColor m_errorBackgroundColor;
};

#endif // TEXTOUTPUTWIDGET_H

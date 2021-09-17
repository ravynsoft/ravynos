//
// ioview.h
//
// Description: Widget that interacts with the debugged application
//
//
// SPDX-FileCopyrightText: 2010 Kåre Särs <kare.sars@iki.fi>
//
//  SPDX-License-Identifier: LGPL-2.0-only

#ifndef IOVIEW_H
#define IOVIEW_H

#include <QFile>
#include <QWidget>

class QTextEdit;
class QLineEdit;
class QSocketNotifier;

class IOView : public QWidget
{
    Q_OBJECT
public:
    IOView(QWidget *parent = nullptr);
    ~IOView() override;

    const QString stdinFifo();
    const QString stdoutFifo();
    const QString stderrFifo();

    void enableInput(bool enable);

    void clearOutput();

public Q_SLOTS:
    void addStdOutText(const QString &text);
    void addStdErrText(const QString &text);

private Q_SLOTS:
    void returnPressed();
    void readOutput();
    void readErrors();

Q_SIGNALS:
    void stdOutText(const QString &text);
    void stdErrText(const QString &text);

private:
    void createFifos();
    QString createFifo(const QString &prefix);

    QTextEdit *m_output;
    QLineEdit *m_input;

    QString m_stdinFifo;
    QString m_stdoutFifo;
    QString m_stderrFifo;

    QFile m_stdin;
    QFile m_stdout;
    QFile m_stderr;

    QFile m_stdoutD;
    QFile m_stderrD;

    int m_stdoutFD = 0;
    int m_stderrFD = 0;

    QSocketNotifier *m_stdoutNotifier = nullptr;
    QSocketNotifier *m_stderrNotifier = nullptr;
};

#endif

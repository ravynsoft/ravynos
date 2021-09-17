//
// configview.cpp
//
// Description: View for configuring the set of targets to be used with the debugger
//
//
// SPDX-FileCopyrightText: 2010 Kåre Särs <kare.sars@iki.fi>
//
//  SPDX-License-Identifier: LGPL-2.0-only

#include "ioview.h"

#include <QDir>
#include <QFontDatabase>
#include <QLineEdit>
#include <QScrollBar>
#include <QSocketNotifier>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>

#include <KColorScheme>
#include <KRandom>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

IOView::IOView(QWidget *parent)
    : QWidget(parent)
{
    m_output = new QTextEdit();
    m_output->setReadOnly(true);
    m_output->setUndoRedoEnabled(false);
    m_output->setAcceptRichText(false);
    // fixed wide font, like konsole
    m_output->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    // alternate color scheme, like konsole
    KColorScheme schemeView(QPalette::Active, KColorScheme::View);
    m_output->setTextBackgroundColor(schemeView.foreground().color());
    m_output->setTextColor(schemeView.background().color());
    QPalette p = m_output->palette();
    p.setColor(QPalette::Base, schemeView.foreground().color());
    m_output->setPalette(p);

    m_input = new QLineEdit();
    m_output->setFocusProxy(m_input); // take the focus from the output

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_output, 10);
    layout->addWidget(m_input, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    connect(m_input, &QLineEdit::returnPressed, this, &IOView::returnPressed);
    createFifos();
}

IOView::~IOView()
{
    m_stdin.close();

    m_stdout.close();
    m_stdout.setFileName(m_stdoutFifo);
    ::close(m_stdoutFD);

    m_stderr.close();
    m_stderr.setFileName(m_stderrFifo);
    ::close(m_stderrFD);

    m_stdin.remove();
    m_stdout.remove();
    m_stderr.remove();
}

void IOView::createFifos()
{
    m_stdinFifo = createFifo(QStringLiteral("stdInFifo"));
    m_stdoutFifo = createFifo(QStringLiteral("stdOutFifo"));
    m_stderrFifo = createFifo(QStringLiteral("stdErrFifo"));

    m_stdin.setFileName(m_stdinFifo);
    if (!m_stdin.open(QIODevice::ReadWrite)) {
        return;
    }

    m_stdoutD.setFileName(m_stdoutFifo);
    m_stdoutD.open(QIODevice::ReadWrite);

    m_stdout.setFileName(m_stdoutFifo);
    m_stdoutFD = ::open(m_stdoutFifo.toLocal8Bit().data(), O_RDWR | O_NONBLOCK);
    if (m_stdoutFD == -1) {
        return;
    }
    if (!m_stdout.open(m_stdoutFD, QIODevice::ReadWrite)) {
        return;
    }

    m_stdoutNotifier = new QSocketNotifier(m_stdoutFD, QSocketNotifier::Read, this);
    connect(m_stdoutNotifier, &QSocketNotifier::activated, this, &IOView::readOutput);
    m_stdoutNotifier->setEnabled(true);

    m_stderrD.setFileName(m_stderrFifo);
    m_stderrD.open(QIODevice::ReadWrite);

    m_stderr.setFileName(m_stderrFifo);
    m_stderrFD = ::open(m_stderrFifo.toLocal8Bit().data(), O_RDONLY | O_NONBLOCK);
    if (m_stderrFD == -1) {
        return;
    }
    if (!m_stderr.open(m_stderrFD, QIODevice::ReadOnly)) {
        return;
    }

    m_stderrNotifier = new QSocketNotifier(m_stderrFD, QSocketNotifier::Read, this);
    connect(m_stderrNotifier, &QSocketNotifier::activated, this, &IOView::readErrors);
    m_stderrNotifier->setEnabled(true);

    return;
}

void IOView::returnPressed()
{
    m_stdin.write(m_input->text().toLocal8Bit());
    m_stdin.write("\n");
    m_stdin.flush();
    m_input->clear();
}

void IOView::readOutput()
{
    m_stdoutNotifier->setEnabled(false);
    qint64 res;
    char chData[256];
    QByteArray data;

    do {
        res = m_stdout.read(chData, 255);
        if (res <= 0) {
            m_stdoutD.flush();
        } else {
            data.append(chData, res);
        }

    } while (res == 255);

    if (data.size() > 0) {
        Q_EMIT stdOutText(QString::fromLocal8Bit(data));
    }
    m_stdoutNotifier->setEnabled(true);
}

void IOView::readErrors()
{
    m_stderrNotifier->setEnabled(false);
    qint64 res;
    char chData[256];
    QByteArray data;

    do {
        res = m_stderr.read(chData, 255);
        if (res <= 0) {
            m_stderrD.flush();
        } else {
            data.append(chData, res);
        }
    } while (res == 255);

    if (data.size() > 0) {
        Q_EMIT stdErrText(QString::fromLocal8Bit(data));
    }
    m_stderrNotifier->setEnabled(true);
}

void IOView::addStdOutText(const QString &text)
{
    QScrollBar *scrollb = m_output->verticalScrollBar();
    if (!scrollb) {
        return;
    }
    bool atEnd = (scrollb->value() == scrollb->maximum());

    QTextCursor cursor = m_output->textCursor();
    if (!cursor.atEnd()) {
        cursor.movePosition(QTextCursor::End);
    }
    cursor.insertText(text);

    if (atEnd) {
        scrollb->setValue(scrollb->maximum());
    }
}

void IOView::addStdErrText(const QString &text)
{
    m_output->setFontItalic(true);
    addStdOutText(text);
    m_output->setFontItalic(false);
}

QString IOView::createFifo(const QString &prefix)
{
    QString fifo = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QDir::separator() + prefix + KRandom::randomString(3);
    int result = mkfifo(QFile::encodeName(fifo).data(), 0666);
    if (result != 0) {
        return QString();
    }
    return fifo;
}

const QString IOView::stdinFifo()
{
    return m_stdinFifo;
}
const QString IOView::stdoutFifo()
{
    return m_stdoutFifo;
}
const QString IOView::stderrFifo()
{
    return m_stderrFifo;
}

void IOView::enableInput(bool enable)
{
    m_input->setEnabled(enable);
}

void IOView::clearOutput()
{
    m_output->clear();
}

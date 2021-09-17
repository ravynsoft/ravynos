/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "gitcommitdialog.h"
#include "git/gitutils.h"
#include "gitwidget.h"

#include <QCoreApplication>
#include <QDebug>
#include <QInputMethodEvent>
#include <QSyntaxHighlighter>
#include <QVBoxLayout>

#include <KColorScheme>
#include <KLocalizedString>

class BadLengthHighlighter : public QSyntaxHighlighter
{
public:
    explicit BadLengthHighlighter(QTextDocument *doc, int badLen)
        : QSyntaxHighlighter(doc)
        , badLength(badLen)
    {
    }

    void highlightBlock(const QString &text) override
    {
        if (text.length() < badLength) {
            return;
        }
        setFormat(badLength, text.length() - badLength, red);
    }

private:
    int badLength = 0;
    const QColor red = KColorScheme().foreground(KColorScheme::NegativeText).color();
};

static void changeTextColorToRed(QLineEdit *lineEdit, const QColor &red)
{
    if (!lineEdit)
        return;

    // Everything > 52 = red color
    QList<QInputMethodEvent::Attribute> attributes;
    if (lineEdit->text().length() > 52) {
        int start = 52 - lineEdit->cursorPosition();
        int len = lineEdit->text().length() - start;
        QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;

        QTextCharFormat fmt;
        fmt.setForeground(red);
        QVariant format = fmt;

        attributes.append(QInputMethodEvent::Attribute(type, start, len, format));
    }
    QInputMethodEvent event(QString(), attributes);
    QCoreApplication::sendEvent(lineEdit, &event);
}

GitCommitDialog::GitCommitDialog(const QString &lastCommit, const QFont &font, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    Q_ASSERT(parent);

    setWindowTitle(i18n("Commit Changes"));

    ok.setText(i18n("Commit"));
    cancel.setText(i18n("Cancel"));

    m_le.setPlaceholderText(i18n("Write commit message..."));
    m_le.setFont(font);

    QFontMetrics fm(font);
    /** Add 8 because 4 + 4 margins on left / right */
    const int width = (fm.averageCharWidth() * 72) + 8;

    m_leLen.setText(QStringLiteral("0 / 52"));

    m_pe.setPlaceholderText(i18n("Extended commit description..."));
    m_pe.setFont(font);

    /** Dialog's main layout **/
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(4, 4, 4, 4);
    setLayout(vlayout);

    /** Setup the label at the top **/
    QHBoxLayout *hLayoutLine = new QHBoxLayout;
    hLayoutLine->addStretch();
    hLayoutLine->addWidget(&m_leLen);

    /** Setup plaintextedit and line edit **/
    vlayout->addLayout(hLayoutLine);
    vlayout->addWidget(&m_le);
    vlayout->addWidget(&m_pe);

    // set 72 chars wide plain text edit
    m_pe.resize(width, m_pe.height());
    resize(width, fm.averageCharWidth() * 52);

    loadCommitMessage(lastCommit);

    auto bottomLayout = new QHBoxLayout;

    /** Setup checkboxes at the bottom **/
    m_cbSignOff.setChecked(false);
    m_cbSignOff.setText(i18n("Sign off"));
    bottomLayout->addWidget(&m_cbSignOff);

    m_cbAmend.setChecked(false);
    m_cbAmend.setText(i18n("Amend"));
    m_cbAmend.setToolTip(i18n("Amend Last Commit"));
    connect(&m_cbAmend, &QCheckBox::stateChanged, this, [this](int state) {
        if (state != Qt::Checked) {
            ok.setText(i18n("Commit"));
            setWindowTitle(i18n("Commit Changes"));
            return;
        }
        setWindowTitle(i18n("Amending Commit"));
        ok.setText(i18n("Amend"));
        const auto [msg, desc] = GitUtils::getLastCommitMessage(static_cast<GitWidget *>(this->parentWidget())->dotGitPath());
        m_le.setText(msg);
        m_pe.setPlainText(desc);
    });

    bottomLayout->addWidget(&m_cbAmend);
    bottomLayout->addStretch();

    vlayout->addLayout(bottomLayout);

    /** Setup Ok / Cancel Button **/
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(&ok);
    hLayout->addWidget(&cancel);

    connect(&ok, &QPushButton::clicked, this, &QDialog::accept);
    connect(&cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(&m_le, &QLineEdit::textChanged, this, &GitCommitDialog::updateLineSizeLabel);

    updateLineSizeLabel();

    vlayout->addLayout(hLayout);

    /**
     * Setup highlighting which changes text color to red if
     * it crosses the 72 char threshold
     */
    auto hl = new BadLengthHighlighter(m_pe.document(), 72);
    Q_UNUSED(hl)
}

void GitCommitDialog::loadCommitMessage(const QString &lastCommit)
{
    if (lastCommit.isEmpty()) {
        return;
    }
    // restore last message ?
    auto msgs = lastCommit.split(QStringLiteral("[[\n\n]]"));
    if (!msgs.isEmpty()) {
        m_le.setText(msgs.at(0));
        if (msgs.length() > 1) {
            m_pe.setPlainText(msgs.at(1));
        }
    }
}

QString GitCommitDialog::subject() const
{
    return m_le.text();
}

QString GitCommitDialog::description() const
{
    return m_pe.toPlainText();
}

bool GitCommitDialog::signoff() const
{
    return m_cbSignOff.isChecked();
}

bool GitCommitDialog::amendingLastCommit() const
{
    return m_cbAmend.isChecked();
}

void GitCommitDialog::setAmendingCommit()
{
    m_cbAmend.setChecked(true);
}

void GitCommitDialog::updateLineSizeLabel()
{
    int len = m_le.text().length();
    if (len < 52) {
        m_leLen.setText(i18nc("Number of characters", "%1 / 52", QString::number(len)));
    } else {
        const QColor red = KColorScheme().foreground(KColorScheme::NegativeText).color();
        changeTextColorToRed(&m_le, red);
        m_leLen.setText(i18nc("Number of characters", "<span style=\"color:%1;\">%2</span> / 52", red.name(), QString::number(len)));
    }
}

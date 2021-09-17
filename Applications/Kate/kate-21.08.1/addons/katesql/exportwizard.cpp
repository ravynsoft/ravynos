/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "exportwizard.h"
#include "dataoutputwidget.h"

#include <KLineEdit>
#include <KLocalizedString>
#include <KUrlRequester>

#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>

// BEGIN ExportWizard
ExportWizard::ExportWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(i18nc("@title:window", "Export Wizard"));

    addPage(new ExportOutputPage(this));
    addPage(new ExportFormatPage(this));
}

ExportWizard::~ExportWizard()
{
}
// END ExportWizard

// BEGIN ExportOutputPage
ExportOutputPage::ExportOutputPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(i18nc("@title Wizard page title", "Output Target"));
    setSubTitle(i18nc("@title Wizard page subtitle", "Select the output target."));

    QVBoxLayout *layout = new QVBoxLayout();

    documentRadioButton = new QRadioButton(i18nc("@option:radio Output target", "Current document"), this);
    clipboardRadioButton = new QRadioButton(i18nc("@option:radio Output target", "Clipboard"), this);
    fileRadioButton = new QRadioButton(i18nc("@option:radio Output target", "File"), this);

    QHBoxLayout *fileLayout = new QHBoxLayout();
    fileLayout->setContentsMargins(20, 0, 0, 0);

    fileUrl = new KUrlRequester(this);
    fileUrl->setMode(KFile::File);
    fileUrl->setFilter(i18n("*.csv|Comma Separated Values\n*|All files"));

    fileLayout->addWidget(fileUrl);

    layout->addWidget(documentRadioButton);
    layout->addWidget(clipboardRadioButton);
    layout->addWidget(fileRadioButton);
    layout->addLayout(fileLayout);

    setLayout(layout);

    registerField(QStringLiteral("outDocument"), documentRadioButton);
    registerField(QStringLiteral("outClipboard"), clipboardRadioButton);
    registerField(QStringLiteral("outFile"), fileRadioButton);
    registerField(QStringLiteral("outFileUrl"), fileUrl, "text");

    connect(fileRadioButton, &QRadioButton::toggled, fileUrl, &KUrlRequester::setEnabled);
}

void ExportOutputPage::initializePage()
{
    documentRadioButton->setChecked(true);
    fileUrl->setEnabled(false);
}

bool ExportOutputPage::validatePage()
{
    if (fileRadioButton->isChecked() && fileUrl->text().isEmpty()) {
        fileUrl->setFocus();
        return false;
    }

    /// TODO: check url validity

    return true;
}
// END ExportOutputPage

// BEGIN ExportFormatPage
ExportFormatPage::ExportFormatPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(i18nc("@title Wizard page title", "Fields Format"));
    setSubTitle(i18nc("@title Wizard page subtitle", "Select fields format.\nClick on \"Finish\" button to export data."));

    QVBoxLayout *layout = new QVBoxLayout();

    QGroupBox *headersGroupBox = new QGroupBox(i18nc("@title:group", "Headers"), this);
    QVBoxLayout *headersLayout = new QVBoxLayout();

    exportColumnNamesCheckBox = new QCheckBox(i18nc("@option:check", "Export column names"), headersGroupBox);
    exportLineNumbersCheckBox = new QCheckBox(i18nc("@option:check", "Export line numbers"), headersGroupBox);

    headersLayout->addWidget(exportColumnNamesCheckBox);
    headersLayout->addWidget(exportLineNumbersCheckBox);

    headersGroupBox->setLayout(headersLayout);

    QGroupBox *quoteGroupBox = new QGroupBox(i18nc("@title:group", "Quotes"), this);
    QGridLayout *quoteLayout = new QGridLayout();

    quoteStringsCheckBox = new QCheckBox(i18nc("@option:check", "Quote strings"), quoteGroupBox);
    quoteNumbersCheckBox = new QCheckBox(i18nc("@option:check", "Quote numbers"), quoteGroupBox);
    quoteStringsLine = new KLineEdit(quoteGroupBox);
    quoteNumbersLine = new KLineEdit(quoteGroupBox);

    quoteStringsLine->setMaxLength(1);
    quoteNumbersLine->setMaxLength(1);

    quoteLayout->addWidget(quoteStringsCheckBox, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    quoteLayout->addWidget(new QLabel(i18nc("@label:textbox", "Character:")), 0, 1, Qt::AlignRight | Qt::AlignVCenter);
    quoteLayout->addWidget(quoteStringsLine, 0, 2, Qt::AlignRight | Qt::AlignVCenter);
    quoteLayout->addWidget(quoteNumbersCheckBox, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    quoteLayout->addWidget(new QLabel(i18nc("@label:textbox", "Character:")), 1, 1, Qt::AlignRight | Qt::AlignVCenter);
    quoteLayout->addWidget(quoteNumbersLine, 1, 2, Qt::AlignRight | Qt::AlignVCenter);
    quoteLayout->setColumnStretch(1, 1);

    quoteGroupBox->setLayout(quoteLayout);

    QGroupBox *delimitersGroupBox = new QGroupBox(i18nc("@title:group", "Delimiters"), this);
    QFormLayout *delimitersLayout = new QFormLayout();

    fieldDelimiterLine = new KLineEdit(delimitersGroupBox);
    fieldDelimiterLine->setMaxLength(3);

    delimitersLayout->addRow(i18nc("@label:textbox", "Field delimiter:"), fieldDelimiterLine);

    delimitersGroupBox->setLayout(delimitersLayout);

    layout->addWidget(headersGroupBox);
    layout->addWidget(quoteGroupBox);
    layout->addWidget(delimitersGroupBox);

    setLayout(layout);

    registerField(QStringLiteral("exportColumnNames"), exportColumnNamesCheckBox);
    registerField(QStringLiteral("exportLineNumbers"), exportLineNumbersCheckBox);
    registerField(QStringLiteral("checkQuoteStrings"), quoteStringsCheckBox);
    registerField(QStringLiteral("checkQuoteNumbers"), quoteNumbersCheckBox);
    registerField(QStringLiteral("quoteStringsChar"), quoteStringsLine);
    registerField(QStringLiteral("quoteNumbersChar"), quoteNumbersLine);
    registerField(QStringLiteral("fieldDelimiter*"), fieldDelimiterLine);

    connect(quoteStringsCheckBox, &QCheckBox::toggled, quoteStringsLine, &KLineEdit::setEnabled);
    connect(quoteNumbersCheckBox, &QCheckBox::toggled, quoteNumbersLine, &KLineEdit::setEnabled);
}

void ExportFormatPage::initializePage()
{
    exportColumnNamesCheckBox->setChecked(true);
    exportLineNumbersCheckBox->setChecked(false);
    quoteStringsCheckBox->setChecked(false);
    quoteNumbersCheckBox->setChecked(false);
    quoteStringsLine->setEnabled(false);
    quoteNumbersLine->setEnabled(false);

    quoteStringsLine->setText(QStringLiteral("\""));
    quoteNumbersLine->setText(QStringLiteral("\""));
    fieldDelimiterLine->setText(QStringLiteral("\\t"));
}

bool ExportFormatPage::validatePage()
{
    if ((quoteStringsCheckBox->isChecked() && quoteStringsLine->text().isEmpty())
        || (quoteNumbersCheckBox->isChecked() && quoteNumbersLine->text().isEmpty())) {
        return false;
    }

    if (fieldDelimiterLine->text().isEmpty()) {
        return false;
    }

    return true;
}
// END ExportFormatPage

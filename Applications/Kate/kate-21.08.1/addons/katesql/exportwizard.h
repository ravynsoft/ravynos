/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef EXPORTWIZARD_H
#define EXPORTWIZARD_H

class KUrlRequester;
class KLineEdit;

class QRadioButton;
class QCheckBox;

#include <qwizard.h>

class ExportWizard : public QWizard
{
    Q_OBJECT
public:
    explicit ExportWizard(QWidget *parent);
    ~ExportWizard() override;
};

class ExportOutputPage : public QWizardPage
{
public:
    explicit ExportOutputPage(QWidget *parent = nullptr);

    void initializePage() override;
    bool validatePage() override;

private:
    QRadioButton *documentRadioButton;
    QRadioButton *clipboardRadioButton;
    QRadioButton *fileRadioButton;
    KUrlRequester *fileUrl;
};

class ExportFormatPage : public QWizardPage
{
public:
    explicit ExportFormatPage(QWidget *parent = nullptr);

    void initializePage() override;
    bool validatePage() override;

private:
    QCheckBox *exportColumnNamesCheckBox;
    QCheckBox *exportLineNumbersCheckBox;
    QCheckBox *quoteStringsCheckBox;
    QCheckBox *quoteNumbersCheckBox;
    KLineEdit *quoteStringsLine;
    KLineEdit *quoteNumbersLine;
    KLineEdit *fieldDelimiterLine;
};

#endif // EXPORTWIZARD_H

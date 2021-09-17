/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef DATAOUTPUTWIDGET_H
#define DATAOUTPUTWIDGET_H

class QTextStream;
class QVBoxLayout;
class QSqlQuery;
class DataOutputModel;
class DataOutputView;

#include <QWidget>

class DataOutputWidget : public QWidget
{
    Q_OBJECT

public:
    enum Option { NoOptions = 0x0, ExportColumnNames = 0x1, ExportLineNumbers = 0x2 };

    Q_DECLARE_FLAGS(Options, Option)

    DataOutputWidget(QWidget *parent);
    ~DataOutputWidget() override;

    void exportData(QTextStream &stream,
                    const QChar stringsQuoteChar = QLatin1Char('\0'),
                    const QChar numbersQuoteChar = QLatin1Char('\0'),
                    const QString &fieldDelimiter = QStringLiteral("\t"),
                    const Options opt = NoOptions);

    DataOutputModel *model() const
    {
        return m_model;
    }
    DataOutputView *view() const
    {
        return m_view;
    }

public Q_SLOTS:
    void showQueryResultSets(QSqlQuery &query);
    void resizeColumnsToContents();
    void resizeRowsToContents();
    void clearResults();

    void slotToggleLocale();
    void slotCopySelected();
    void slotExport();

private:
    QVBoxLayout *m_dataLayout;

    /// TODO: manage multiple views for query with multiple resultsets
    DataOutputModel *m_model;
    DataOutputView *m_view;

    bool m_isEmpty;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DataOutputWidget::Options)

#endif // DATAOUTPUTWIDGET_H

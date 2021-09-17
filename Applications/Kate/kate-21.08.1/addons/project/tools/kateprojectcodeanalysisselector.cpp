/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2017 Héctor Mesa Jiménez <hector@lcc.uma.es>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectcodeanalysisselector.h"

#include "kateprojectcodeanalysistoolclazy.h"
#include "kateprojectcodeanalysistoolclazycurrent.h"
#include "kateprojectcodeanalysistoolcppcheck.h"
#include "kateprojectcodeanalysistoolflake8.h"
#include "kateprojectcodeanalysistoolshellcheck.h"

QStandardItemModel *KateProjectCodeAnalysisSelector::model(QObject *parent)
{
    auto model = new QStandardItemModel(parent);

    /*
     * available linters
     */
    const QList<KateProjectCodeAnalysisTool *> tools = {// cppcheck, for C++
                                                        new KateProjectCodeAnalysisToolCppcheck(model),
                                                        // flake8, for Python
                                                        new KateProjectCodeAnalysisToolFlake8(model),
                                                        // ShellCheck, for sh/bash scripts
                                                        new KateProjectCodeAnalysisToolShellcheck(model),
                                                        // clazy for Qt C++
                                                        new KateProjectCodeAnalysisToolClazy(model),
                                                        // clang-tidy
                                                        new KateProjectCodeAnalysisToolClazyCurrent(model)};

    QList<QStandardItem *> column;

    for (auto tool : tools) {
        auto item = new QStandardItem(tool->name());
        item->setData(QVariant::fromValue(tool), Qt::UserRole + 1);

        column << item;
    }

    model->appendColumn(column);

    return model;
}

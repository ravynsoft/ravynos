/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2017 Héctor Mesa Jiménez <hector@lcc.uma.es>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_PROJECT_CODE_ANALYSIS_SELECTOR_H
#define KATE_PROJECT_CODE_ANALYSIS_SELECTOR_H

#include <QStandardItemModel>

class KateProjectCodeAnalysisSelector
{
public:
    /**
     * Model attachable to a code analysis tool selector.
     *
     * Each item contains a pointer to a KateProjectCodeAnalysisTool
     * associated to the role Qt::UserRole + 1
     */
    static QStandardItemModel *model(QObject *parent = nullptr);
};

#endif // KATE_PROJECT_CODE_ANALYSIS_SELECTOR_H

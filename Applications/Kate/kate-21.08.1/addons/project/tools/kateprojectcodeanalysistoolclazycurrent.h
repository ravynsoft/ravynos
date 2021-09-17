/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KATEPROJECTCODEANALYSISTOOLCLANGTIDY_H
#define KATEPROJECTCODEANALYSISTOOLCLANGTIDY_H

#include "kateprojectcodeanalysistoolclazy.h"

class KateProjectCodeAnalysisToolClazyCurrent : public KateProjectCodeAnalysisToolClazy
{
public:
    KateProjectCodeAnalysisToolClazyCurrent(QObject *parent);

    QString name() const override;
    QString description() const override;
    QStringList arguments() override;
};

#endif // KATEPROJECTCODEANALYSISTOOLCLANGTIDY_H

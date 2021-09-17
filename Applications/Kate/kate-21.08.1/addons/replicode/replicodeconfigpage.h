/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2014 Martin Sandsmark <martin.sandsmark@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef REPLICODECONFIGPAGE_H
#define REPLICODECONFIGPAGE_H
#include <KTextEditor/ConfigPage>

class KUrlRequester;
class ReplicodeConfig;

class ReplicodeConfigPage : public KTextEditor::ConfigPage
{
    Q_OBJECT

public:
    ReplicodeConfigPage(QWidget *parent = nullptr);

    QString name() const override;
    QString fullName() const override;

public Q_SLOTS:
    void apply() override;
    void reset() override;
    void defaults() override;

private:
    KUrlRequester *m_requester;
    ReplicodeConfig *m_config;
};

#endif // REPLICODECONFIGPAGE_H

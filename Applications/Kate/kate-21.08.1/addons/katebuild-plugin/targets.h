//
// Description: Widget for configuring build targets
//
// SPDX-FileCopyrightText: 2011-2014 Kåre Särs <kare.sars@iki.fi>
//
//  SPDX-License-Identifier: LGPL-2.0-only

#ifndef TARGETS_H
#define TARGETS_H

#include "TargetHtmlDelegate.h"
#include "TargetModel.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QToolButton>
#include <QTreeView>
#include <QWidget>

class TargetsUi : public QWidget
{
    Q_OBJECT

public:
    TargetsUi(QObject *view, QWidget *parent = nullptr);

    QLabel *targetLabel;
    QComboBox *targetCombo;
    QToolButton *newTarget;
    QToolButton *copyTarget;
    QToolButton *deleteTarget;

    QTreeView *targetsView;
    TargetModel targetsModel;

    QToolButton *addButton;
    QToolButton *buildButton;

public Q_SLOTS:
    void targetSetSelected(int index);
    void targetActivated(const QModelIndex &index);

Q_SIGNALS:
    void enterPressed();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    TargetHtmlDelegate *m_delegate;
};

#endif

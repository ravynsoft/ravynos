/***************************************************************************
                      bash_parser.cpp  -  description
                             -------------------
    begin                : dec 12 2008
    author               : Daniel Dumitrache
    email                : daniel.dumitrache@gmail.com
 ***************************************************************************/
/***************************************************************************
   SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "plugin_katesymbolviewer.h"

void KatePluginSymbolViewerView::parseBashSymbols(void)
{
    if (!m_mainWindow->activeView()) {
        return;
    }

    QString currline;

    int i;
    // bool mainprog;

    QTreeWidgetItem *node = nullptr;
    QTreeWidgetItem *funcNode = nullptr;
    QTreeWidgetItem *lastFuncNode = nullptr;

    QPixmap func(class_xpm);

    // It is necessary to change names
    m_func->setText(i18n("Show Functions"));

    if (m_treeOn->isChecked()) {
        funcNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Functions")));
        funcNode->setIcon(0, QIcon(func));

        if (m_expandOn->isChecked()) {
            m_symbols->expandItem(funcNode);
        }

        lastFuncNode = funcNode;

        m_symbols->setRootIsDecorated(1);
    } else {
        m_symbols->setRootIsDecorated(0);
    }

    KTextEditor::Document *kDoc = m_mainWindow->activeView()->document();

    for (i = 0; i < kDoc->lines(); i++) {
        currline = kDoc->line(i);
        currline = currline.trimmed();
        currline = currline.simplified();

        bool comment = false;
        // qDebug(13000)<<currline<<endl;
        if (currline.isEmpty()) {
            continue;
        }
        if (currline.at(0) == QLatin1Char('#')) {
            comment = true;
        }

        // mainprog=false;
        if (!comment && m_func->isChecked()) {
            QString funcName;

            // skip line if no function defined
            // note: function name must match regex: [a-zA-Z0-9-_]+
            if (!currline.contains(QRegularExpression(QLatin1String("^(function )*[a-zA-Z0-9-_]+ *\\( *\\)")))
                && !currline.contains(QRegularExpression(QLatin1String("^function [a-zA-Z0-9-_]+")))) {
                continue;
            }

            // strip everything unneeded and get the function's name
            currline.remove(QRegularExpression(QLatin1String("^(function )*")));
            funcName = currline.split(QRegularExpression(QLatin1String("((\\( *\\))|[^a-zA-Z0-9-_])")))[0].simplified();
            if (!funcName.size()) {
                continue;
            }
            funcName.append(QLatin1String("()"));

            if (m_treeOn->isChecked()) {
                node = new QTreeWidgetItem(funcNode, lastFuncNode);
                lastFuncNode = node;
            } else {
                node = new QTreeWidgetItem(m_symbols);
            }

            node->setText(0, funcName);
            node->setIcon(0, QIcon(func));
            node->setText(1, QString::number(i, 10));
        }
    } // for i loop
}

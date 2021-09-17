/***************************************************************************
                          tcl_parser.cpp  -  description
                             -------------------
    begin                : Apr 2 2003
    author               : 2003 Massimo Callegari
    email                : massimocallegari@yahoo.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "plugin_katesymbolviewer.h"
#include <QPixmap>

void KatePluginSymbolViewerView::parseTclSymbols(void)
{
    if (!m_mainWindow->activeView()) {
        return;
    }

    QString currline, prevline;
    bool prevComment = false;
    QString varStr(QStringLiteral("set "));
    QString procStr(QStringLiteral("proc"));
    QString stripped;
    int i, j, args_par = 0, graph = 0;
    char block = 0, parse_func = 0;

    QTreeWidgetItem *node = nullptr;
    QTreeWidgetItem *mcrNode = nullptr, *clsNode = nullptr;
    QTreeWidgetItem *lastMcrNode = nullptr, *lastClsNode = nullptr;

    QPixmap mcr(macro_xpm);
    QPixmap cls(class_xpm);

    if (m_treeOn->isChecked()) {
        clsNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Functions")));
        mcrNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Globals")));
        clsNode->setIcon(0, QIcon(cls));
        mcrNode->setIcon(0, QIcon(mcr));

        lastMcrNode = mcrNode;
        lastClsNode = clsNode;

        if (m_expandOn->isChecked()) {
            m_symbols->expandItem(clsNode);
            m_symbols->expandItem(mcrNode);
        }
        m_symbols->setRootIsDecorated(1);
    } else {
        m_symbols->setRootIsDecorated(0);
    }

    KTextEditor::Document *kDoc = m_mainWindow->activeView()->document();

    // positions.resize(kDoc->numLines() + 3); // Maximum m_symbols number o.O
    // positions.fill(0);

    for (i = 0; i < kDoc->lines(); i++) {
        currline = kDoc->line(i);
        currline = currline.trimmed();
        bool comment = false;
        // qDebug(13000)<<currline;
        if (currline.isEmpty()) {
            continue;
        }
        if (currline.at(0) == QLatin1Char('#')) {
            comment = true;
        }

        if (i > 0) {
            prevline = kDoc->line(i - 1);
            if (prevline.endsWith(QLatin1String("\\")) && prevComment) {
                comment = true;
            }
        }
        prevComment = comment;

        if (!comment) {
            if (currline.startsWith(varStr) && block == 0) {
                if (m_macro->isChecked()) // not really a macro, but a variable
                {
                    stripped = currline.right(currline.length() - 3);
                    stripped = stripped.simplified();
                    int fnd = stripped.indexOf(QLatin1Char(' '));
                    // fnd = stripped.indexOf(QLatin1Char(';'));
                    if (fnd > 0) {
                        stripped = stripped.left(fnd);
                    }

                    if (m_treeOn->isChecked()) {
                        node = new QTreeWidgetItem(mcrNode, lastMcrNode);
                        lastMcrNode = node;
                    } else {
                        node = new QTreeWidgetItem(m_symbols);
                    }
                    node->setText(0, stripped);
                    node->setIcon(0, QIcon(mcr));
                    node->setText(1, QString::number(i, 10));
                    stripped.clear();
                } // macro
            } // starts with "set"

            else if (currline.startsWith(procStr)) {
                parse_func = 1;
            }

            if (parse_func == 1) {
                for (j = 0; j < currline.length(); j++) {
                    if (block == 1) {
                        if (currline.at(j) == QLatin1Char('{')) {
                            graph++;
                        }
                        if (currline.at(j) == QLatin1Char('}')) {
                            graph--;
                            if (graph == 0) {
                                block = 0;
                                parse_func = 0;
                                continue;
                            }
                        }
                    }
                    if (block == 0) {
                        stripped += currline.at(j);
                        if (currline.at(j) == QLatin1Char('{')) {
                            args_par++;
                        }
                        if (currline.at(j) == QLatin1Char('}')) {
                            args_par--;
                            if (args_par == 0) {
                                // stripped = stripped.simplified();
                                if (m_func->isChecked()) {
                                    if (m_treeOn->isChecked()) {
                                        node = new QTreeWidgetItem(clsNode, lastClsNode);
                                        lastClsNode = node;
                                    } else {
                                        node = new QTreeWidgetItem(m_symbols);
                                    }
                                    node->setText(0, stripped);
                                    node->setIcon(0, QIcon(cls));
                                    node->setText(1, QString::number(i, 10));
                                }
                                stripped.clear();
                                block = 1;
                            }
                        }
                    } // block = 0
                } // for j loop
            } // m_func->isChecked()
        } // not a comment
    } // for i loop

    // positions.resize(m_symbols->itemIndex(node) + 1);
}

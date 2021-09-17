/***************************************************************************
                      fortran_parser.cpp  -  description
                             -------------------
    begin                : jul 10 2005
    author               : 2005 Roberto Quitiliani
    email                : roby(dot)q(AT)tiscali(dot)it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "plugin_katesymbolviewer.h"

void KatePluginSymbolViewerView::parseFortranSymbols(void)
{
    if (!m_mainWindow->activeView()) {
        return;
    }

    QString currline;
    QString subrStr(QStringLiteral("subroutine "));
    QString funcStr(QStringLiteral("function "));
    QString modStr(QStringLiteral("module "));

    QString stripped;
    int i;
    int fnd, block = 0, blockend = 0, paro = 0, parc = 0;
    bool mainprog;

    QTreeWidgetItem *node = nullptr;
    QTreeWidgetItem *subrNode = nullptr, *funcNode = nullptr, *modNode = nullptr;
    QTreeWidgetItem *lastSubrNode = nullptr, *lastFuncNode = nullptr, *lastModNode = nullptr;

    QPixmap func(class_xpm);
    QPixmap subr(macro_xpm);
    QPixmap mod(struct_xpm);

    // It is necessary to change names
    m_macro->setText(i18n("Show Subroutines"));
    m_struct->setText(i18n("Show Modules"));
    m_func->setText(i18n("Show Functions"));

    if (m_treeOn->isChecked()) {
        funcNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Functions")));
        subrNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Subroutines")));
        modNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Modules")));
        funcNode->setIcon(0, QIcon(func));
        modNode->setIcon(0, QIcon(mod));
        subrNode->setIcon(0, QIcon(subr));

        if (m_expandOn->isChecked()) {
            m_symbols->expandItem(funcNode);
            m_symbols->expandItem(subrNode);
            m_symbols->expandItem(modNode);
        }

        lastSubrNode = subrNode;
        lastFuncNode = funcNode;
        lastModNode = modNode;
        m_symbols->setRootIsDecorated(1);
    } else {
        m_symbols->setRootIsDecorated(0);
    }

    KTextEditor::Document *kDoc = m_mainWindow->activeView()->document();

    for (i = 0; i < kDoc->lines(); i++) {
        currline = kDoc->line(i);
        currline = currline.trimmed();
        // currline = currline.simplified(); is this really needed ?
        // Fortran is case insensitive
        currline = currline.toLower();
        bool comment = false;
        // kdDebug(13000)<<currline<<endl;
        if (currline.isEmpty()) {
            continue;
        }
        if (currline.at(0) == QLatin1Char('!') || currline.at(0) == QLatin1Char('c')) {
            comment = true;
        }
        // block=0;

        mainprog = false;

        if (!comment) {
            // Subroutines
            if (currline.startsWith(subrStr) || currline.startsWith(QLatin1String("program "))) {
                block = 1;
                stripped.clear();
            }
            // Modules
            else if (currline.startsWith(modStr)) {
                block = 2;
                stripped.clear();
            }
            // Functions
            else if ((((currline.startsWith(QLatin1String("real")) || currline.startsWith(QLatin1String("double"))
                        || currline.startsWith(QLatin1String("integer")) || currline.startsWith(QLatin1String("character")))
                       || currline.startsWith(QLatin1String("logical")) || currline.startsWith(QLatin1String("pure"))
                       || currline.startsWith(QLatin1String("elemental")) || currline.startsWith(QLatin1String("recursive"))
                       || currline.startsWith(QLatin1String("type")))
                      && currline.indexOf(funcStr) > 0)
                     || currline.startsWith(funcStr)) {
                block = 3;
                stripped.clear();
            }

            // Subroutines
            if (block == 1) {
                if (currline.startsWith(QLatin1String("program "))) {
                    mainprog = true;
                }
                if (m_macro->isChecked()) // not really a macro, but a subroutines
                {
                    stripped += currline.rightRef(currline.length());
                    stripped = stripped.simplified();
                    stripped.remove(QLatin1Char('*'));
                    stripped.remove(QLatin1Char('+'));
                    stripped.remove(QLatin1Char('$'));
                    if (blockend == 0) {
                        fnd = stripped.indexOf(QLatin1Char(' '));
                        stripped = currline.right(currline.length() - fnd - 1);
                    }
                    stripped.remove(QLatin1Char(' '));
                    fnd = stripped.indexOf(QLatin1Char('!'));
                    if (fnd > 0) {
                        stripped.truncate(fnd);
                    }
                    paro += currline.count(QLatin1Char(')'), Qt::CaseSensitive);
                    parc += currline.count(QLatin1Char('('), Qt::CaseSensitive);

                    if ((paro == parc || mainprog) && stripped.endsWith(QLatin1Char('&'), Qt::CaseInsensitive) == false) {
                        stripped.remove(QLatin1Char('&'));
                        if (mainprog && stripped.indexOf(QLatin1Char('(')) < 0 && stripped.indexOf(QLatin1Char(')')) < 0) {
                            stripped.prepend(QLatin1String("Main: "));
                        }
                        if (stripped.indexOf(QLatin1Char('=')) == -1) {
                            if (m_treeOn->isChecked()) {
                                node = new QTreeWidgetItem(subrNode, lastSubrNode);
                                lastSubrNode = node;
                            } else {
                                node = new QTreeWidgetItem(m_symbols);
                            }
                            node->setText(0, stripped);
                            node->setIcon(0, QIcon(subr));
                            node->setText(1, QString::number(i, 10));
                        }
                        stripped.clear();
                        block = 0;
                        blockend = 0;
                        paro = 0;
                        parc = 0;
                    } else {
                        blockend = 1;
                    }
                }
            }

            // Modules
            else if (block == 2) {
                if (m_struct->isChecked()) // not really a struct, but a module
                {
                    stripped = currline.right(currline.length());
                    stripped = stripped.simplified();
                    fnd = stripped.indexOf(QLatin1Char(' '));
                    stripped = currline.right(currline.length() - fnd - 1);
                    fnd = stripped.indexOf(QLatin1Char('!'));
                    if (fnd > 0) {
                        stripped.truncate(fnd);
                    }
                    if (stripped.indexOf(QLatin1Char('=')) == -1) {
                        if (m_treeOn->isChecked()) {
                            node = new QTreeWidgetItem(modNode, lastModNode);
                            lastModNode = node;
                        } else {
                            node = new QTreeWidgetItem(m_symbols);
                        }
                        node->setText(0, stripped);
                        node->setIcon(0, QIcon(mod));
                        node->setText(1, QString::number(i, 10));
                    }
                    stripped.clear();
                }
                block = 0;
                blockend = 0;
            }

            // Functions
            else if (block == 3) {
                if (m_func->isChecked()) {
                    stripped += currline.rightRef(currline.length());
                    stripped = stripped.trimmed();
                    stripped.remove(QLatin1String("function"));
                    stripped.remove(QLatin1Char('*'));
                    stripped.remove(QLatin1Char('+'));
                    stripped.remove(QLatin1Char('$'));
                    stripped = stripped.simplified();
                    fnd = stripped.indexOf(QLatin1Char('!'));
                    if (fnd > 0) {
                        stripped.truncate(fnd);
                    }
                    stripped = stripped.trimmed();
                    paro += currline.count(QLatin1Char(')'), Qt::CaseSensitive);
                    parc += currline.count(QLatin1Char('('), Qt::CaseSensitive);

                    if (paro == parc && stripped.endsWith(QLatin1Char('&')) == false) {
                        stripped.remove(QLatin1Char('&'));
                        if (m_treeOn->isChecked()) {
                            node = new QTreeWidgetItem(funcNode, lastFuncNode);
                            lastFuncNode = node;
                        } else {
                            node = new QTreeWidgetItem(m_symbols);
                        }
                        node->setText(0, stripped);
                        node->setIcon(0, QIcon(func));
                        node->setText(1, QString::number(i, 10));
                        stripped.clear();
                        block = 0;
                        paro = 0;
                        parc = 0;
                    }
                    blockend = 0;
                }
            }
        }
    } // for i loop
}

/***************************************************************************
                          cpp_parser.cpp  -  description
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

void KatePluginSymbolViewerView::parseCppSymbols(void)
{
    if (!m_mainWindow->activeView()) {
        return;
    }

    QString cl; // Current Line
    QString stripped;
    int i, j, tmpPos = 0;
    int par = 0, graph = 0 /*, retry = 0*/;
    char mclass = 0, block = 0, comment = 0; // comment: 0-no comment 1-inline comment 2-multiline comment 3-string
    char macro = 0 /*, macro_pos = 0*/, func_close = 0;
    bool structure = false;
    QPixmap cls(class_xpm);
    QPixmap sct(struct_xpm);
    QPixmap mcr(macro_xpm);
    QPixmap mtd(method_xpm);

    // It is necessary to change names to defaults
    m_macro->setText(i18n("Show Macros"));
    m_struct->setText(i18n("Show Structures"));
    m_func->setText(i18n("Show Functions"));

    QTreeWidgetItem *node = nullptr;
    QTreeWidgetItem *mcrNode = nullptr, *sctNode = nullptr, *clsNode = nullptr, *mtdNode = nullptr;
    QTreeWidgetItem *lastMcrNode = nullptr, *lastSctNode = nullptr, *lastClsNode = nullptr, *lastMtdNode = nullptr;

    KTextEditor::Document *kv = m_mainWindow->activeView()->document();

    // qDebug(13000)<<"Lines counted :"<<kv->lines();
    if (m_treeOn->isChecked()) {
        mcrNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Macros")));
        sctNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Structures")));
        clsNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Functions")));
        mcrNode->setIcon(0, QIcon(mcr));
        sctNode->setIcon(0, QIcon(sct));
        clsNode->setIcon(0, QIcon(cls));
        if (m_expandOn->isChecked()) {
            m_symbols->expandItem(mcrNode);
            m_symbols->expandItem(sctNode);
            m_symbols->expandItem(clsNode);
        }
        lastMcrNode = mcrNode;
        lastSctNode = sctNode;
        lastClsNode = clsNode;
        mtdNode = clsNode;
        lastMtdNode = clsNode;
        m_symbols->setRootIsDecorated(1);
    } else {
        m_symbols->setRootIsDecorated(0);
    }

    for (i = 0; i < kv->lines(); i++) {
        // qDebug(13000)<<"Current line :"<<i;
        cl = kv->line(i);
        cl = cl.trimmed();
        func_close = 0;
        if ((cl.length() >= 2) && (cl.at(0) == QLatin1Char('/') && cl.at(1) == QLatin1Char('/'))) {
            continue;
        }
        if (cl.indexOf(QLatin1String("/*")) == 0 && (cl.indexOf(QLatin1String("*/")) == (cl.length() - 2)) && graph == 0) {
            continue; // workaround :(
        }
        if (cl.indexOf(QLatin1String("/*")) >= 0 && graph == 0) {
            comment = 1;
        }
        if (cl.indexOf(QLatin1String("*/")) >= 0 && graph == 0) {
            comment = 0;
        }
        if (cl.indexOf(QLatin1Char('#')) >= 0 && graph == 0) {
            macro = 1;
        }
        if (comment != 1) {
            /* *********************** MACRO PARSING *****************************/
            if (macro == 1) {
                // macro_pos = cl.indexOf(QLatin1Char('#'));
                for (j = 0; j < cl.length(); j++) {
                    if (((j + 1) < cl.length()) && (cl.at(j) == QLatin1Char('/') && cl.at(j + 1) == QLatin1Char('/'))) {
                        macro = 4;
                        break;
                    }
                    if (cl.indexOf(QLatin1String("define")) == j && !(cl.indexOf(QLatin1String("defined")) == j)) {
                        macro = 2;
                        j += 6; // skip the word "define"
                    }
                    if (macro == 2 && j < cl.length() && cl.at(j) != QLatin1Char(' ') && cl.at(j) != QLatin1Char('\t')) {
                        macro = 3;
                    }
                    if (macro == 3) {
                        if (cl.at(j) >= 0x20) {
                            stripped += cl.at(j);
                        }
                        if (cl.at(j) == QLatin1Char(' ') || cl.at(j) == QLatin1Char('\t') || j == cl.length() - 1) {
                            macro = 4;
                        }
                    }
                    // qDebug(13000)<<"Macro -- Stripped : "<<stripped<<" macro = "<<macro;
                }
                // I didn't find a valid macro e.g. include
                if (j == cl.length() && macro == 1) {
                    macro = 0;
                }
                if (macro == 4) {
                    // stripped.replace(0x9, QLatin1String(" "));
                    stripped = stripped.trimmed();
                    if (m_macro->isChecked()) {
                        if (m_treeOn->isChecked()) {
                            node = new QTreeWidgetItem(mcrNode, lastMcrNode);
                            lastMcrNode = node;
                        } else {
                            node = new QTreeWidgetItem(m_symbols);
                        }
                        node->setText(0, stripped);
                        node->setIcon(0, QIcon(mcr));
                        node->setText(1, QString::number(i, 10));
                    }
                    macro = 0;
                    // macro_pos = 0;
                    stripped.clear();
                    // qDebug(13000)<<"Macro -- Inserted : "<<stripped<<" at row : "<<i;
                    if (cl.at(cl.length() - 1) == QLatin1Char('\\')) {
                        macro = 5; // continue in rows below
                    }
                    continue;
                }
            }
            if (macro == 5) {
                if (cl.length() == 0 || cl.at(cl.length() - 1) != QLatin1Char('\\')) {
                    macro = 0;
                }
                continue;
            }

            /* ******************************************************************** */

            if ((cl.indexOf(QLatin1String("class")) >= 0 && graph == 0 && block == 0)) {
                mclass = 1;
                for (j = 0; j < cl.length(); j++) {
                    if (((j + 1) < cl.length()) && (cl.at(j) == QLatin1Char('/') && cl.at(j + 1) == QLatin1Char('/'))) {
                        mclass = 2;
                        break;
                    }
                    if (cl.at(j) == QLatin1Char('{')) {
                        mclass = 4;
                        break;
                    }
                    stripped += cl.at(j);
                }
                if (m_func->isChecked()) {
                    if (m_treeOn->isChecked()) {
                        node = new QTreeWidgetItem(clsNode, lastClsNode);
                        if (m_expandOn->isChecked()) {
                            m_symbols->expandItem(node);
                        }
                        lastClsNode = node;
                        mtdNode = lastClsNode;
                        lastMtdNode = lastClsNode;
                    } else {
                        node = new QTreeWidgetItem(m_symbols);
                    }
                    node->setText(0, stripped);
                    node->setIcon(0, QIcon(cls));
                    node->setText(1, QString::number(i, 10));
                    stripped.clear();
                    if (mclass == 1) {
                        mclass = 3;
                    }
                }
                continue;
            }
            if (mclass == 3) {
                if (cl.indexOf(QLatin1Char('{')) >= 0) {
                    cl = cl.mid(cl.indexOf(QLatin1Char('{')));
                    mclass = 4;
                }
            }

            if (cl.indexOf(QLatin1Char('(')) >= 0 && cl.at(0) != QLatin1Char('#') && block == 0 && comment != 2) {
                structure = false;
                block = 1;
            }
            if ((cl.indexOf(QLatin1String("typedef")) >= 0 || cl.indexOf(QLatin1String("struct")) >= 0) && graph == 0 && block == 0) {
                structure = true;
                block = 2;
                stripped.clear();
            }
            // if(cl.indexOf(QLatin1Char(';')) >= 0 && graph == 0)
            //    block = 0;
            if (block > 0 && mclass != 1) {
                for (j = 0; j < cl.length(); j++) {
                    if (((j + 1) < cl.length()) && (cl.at(j) == QLatin1Char('/') && (cl.at(j + 1) == QLatin1Char('*')) && comment != 3)) {
                        comment = 2;
                    }
                    if (((j + 1) < cl.length()) && (cl.at(j) == QLatin1Char('*') && (cl.at(j + 1) == QLatin1Char('/')) && comment != 3)) {
                        comment = 0;
                        j += 2;
                        if (j >= cl.length()) {
                            break;
                        }
                    }
                    // Skip escaped double quotes
                    if (((j + 1) < cl.length()) && (cl.at(j) == QLatin1Char('\\') && (cl.at(j + 1) == QLatin1Char('"')) && comment == 3)) {
                        j += 2;
                        if (j >= cl.length()) {
                            break;
                        }
                    }

                    // Skip char declarations that could be interpreted as range start/end
                    if (((cl.indexOf(QLatin1String("'\"'"), j) == j) || (cl.indexOf(QLatin1String("'{'"), j) == j)
                         || (cl.indexOf(QLatin1String("'}'"), j) == j))
                        && comment != 3) {
                        j += 3;
                        if (j >= cl.length()) {
                            break;
                        }
                    }

                    // Handles a string. Those are freaking evilish !
                    if (cl.at(j) == QLatin1Char('"') && comment == 3) {
                        comment = 0;
                        j++;
                        if (j >= cl.length()) {
                            break;
                        }
                    } else if (cl.at(j) == QLatin1Char('"') && comment == 0) {
                        comment = 3;
                    }
                    if (((j + 1) < cl.length()) && (cl.at(j) == QLatin1Char('/') && cl.at(j + 1) == QLatin1Char('/')) && comment == 0) {
                        if (block == 1 && stripped.isEmpty()) {
                            block = 0;
                        }
                        break;
                    }
                    if (comment != 2 && comment != 3) {
                        if (block == 1 && graph == 0) {
                            if (cl.at(j) >= 0x20) {
                                stripped += cl.at(j);
                            }
                            if (cl.at(j) == QLatin1Char('(')) {
                                par++;
                            }
                            if (cl.at(j) == QLatin1Char(')')) {
                                par--;
                                if (par == 0) {
                                    stripped = stripped.trimmed();
                                    stripped.remove(QLatin1String("static "));
                                    // qDebug(13000)<<"Function -- Inserted : "<<stripped<<" at row : "<<i;
                                    block = 2;
                                    tmpPos = i;
                                }
                            }
                        } // BLOCK 1
                        if (block == 2 && graph == 0) {
                            if (((j + 1) < cl.length()) && (cl.at(j) == QLatin1Char('/') && cl.at(j + 1) == QLatin1Char('/')) && comment == 0) {
                                break;
                            }
                            // if(cl.at(j)==QLatin1Char(':') || cl.at(j)==QLatin1Char(',')) { block = 1; continue; }
                            if (cl.at(j) == QLatin1Char(':')) {
                                block = 1;
                                continue;
                            }
                            if (cl.at(j) == QLatin1Char(';')) {
                                stripped.clear();
                                block = 0;
                                structure = false;
                                break;
                            }

                            if ((cl.at(j) == QLatin1Char('{') && structure == false && cl.indexOf(QLatin1Char(';')) < 0)
                                || (cl.at(j) == QLatin1Char('{') && structure == false && cl.indexOf(QLatin1Char('}')) > j)) {
                                stripped.replace(0x9, QLatin1String(" "));
                                if (m_func->isChecked()) {
                                    QString strippedWithTypes = stripped;
                                    if (!m_typesOn->isChecked()) {
                                        while (stripped.indexOf(QLatin1Char('(')) >= 0) {
                                            stripped = stripped.left(stripped.indexOf(QLatin1Char('(')));
                                        }
                                        while (stripped.indexOf(QLatin1String("::")) >= 0) {
                                            stripped = stripped.mid(stripped.indexOf(QLatin1String("::")) + 2);
                                        }
                                        stripped = stripped.trimmed();
                                        while (stripped.indexOf(0x20) >= 0) {
                                            stripped = stripped.mid(stripped.indexOf(0x20, 0) + 1);
                                        }
                                        while ((stripped.length() > 0) && ((stripped.at(0) == QLatin1Char('*')) || (stripped.at(0) == QLatin1Char('&')))) {
                                            stripped = stripped.right(stripped.length() - 1);
                                        }
                                    }
                                    if (m_treeOn->isChecked()) {
                                        if (mclass == 4) {
                                            node = new QTreeWidgetItem(mtdNode, lastMtdNode);
                                            lastMtdNode = node;
                                        } else {
                                            node = new QTreeWidgetItem(clsNode, lastClsNode);
                                            lastClsNode = node;
                                        }
                                    } else {
                                        node = new QTreeWidgetItem(m_symbols);
                                    }
                                    node->setText(0, stripped);
                                    if (mclass == 4) {
                                        node->setIcon(0, QIcon(mtd));
                                    } else {
                                        node->setIcon(0, QIcon(cls));
                                    }
                                    node->setText(1, QString::number(tmpPos, 10));
                                    node->setToolTip(0, strippedWithTypes);
                                }
                                stripped.clear();
                                // retry = 0;
                                block = 3;
                            }
                            if (cl.at(j) == QLatin1Char('{') && structure == true) {
                                block = 3;
                                tmpPos = i;
                            }
                            if (cl.at(j) == QLatin1Char('(') && structure == true) {
                                // retry = 1;
                                block = 0;
                                j = 0;
                                // qDebug(13000)<<"Restart from the beginning of line...";
                                stripped.clear();
                                break; // Avoid an infinite loop :(
                            }
                            if (structure == true && cl.at(j) >= 0x20) {
                                stripped += cl.at(j);
                            }
                        } // BLOCK 2

                        if (block == 3) {
                            // A comment...there can be anything
                            if (((j + 1) < cl.length()) && (cl.at(j) == QLatin1Char('/') && cl.at(j + 1) == QLatin1Char('/')) && comment == 0) {
                                break;
                            }
                            if (cl.at(j) == QLatin1Char('{')) {
                                graph++;
                            }
                            if (cl.at(j) == QLatin1Char('}')) {
                                graph--;
                                if (graph == 0 && structure == false) {
                                    block = 0;
                                    func_close = 1;
                                }
                                if (graph == 0 && structure == true) {
                                    block = 4;
                                }
                            }
                        } // BLOCK 3

                        if (block == 4) {
                            if (cl.at(j) == QLatin1Char(';')) {
                                // stripped.replace(0x9, QLatin1String(" "));
                                stripped.remove(QLatin1Char('{'));
                                stripped.replace(QLatin1Char('}'), QLatin1String(" "));
                                if (m_struct->isChecked()) {
                                    if (m_treeOn->isChecked()) {
                                        node = new QTreeWidgetItem(sctNode, lastSctNode);
                                        lastSctNode = node;
                                    } else {
                                        node = new QTreeWidgetItem(m_symbols);
                                    }
                                    node->setText(0, stripped);
                                    node->setIcon(0, QIcon(sct));
                                    node->setText(1, QString::number(tmpPos, 10));
                                }
                                // qDebug(13000)<<"Structure -- Inserted : "<<stripped<<" at row : "<<i;
                                stripped.clear();
                                block = 0;
                                structure = false;
                                // break;
                                continue;
                            }
                            if (cl.at(j) >= 0x20) {
                                stripped += cl.at(j);
                            }
                        } // BLOCK 4
                    } // comment != 2
                      // qDebug(13000)<<"Stripped : "<<stripped<<" at row : "<<i;
                } // End of For cycle
            } // BLOCK > 0
            if (mclass == 4 && block == 0 && func_close == 0) {
                if (cl.indexOf(QLatin1Char('}')) >= 0) {
                    cl = cl.mid(cl.indexOf(QLatin1Char('}')));
                    mclass = 0;
                }
            }
        } // Comment != 1
    } // for kv->numlines

    // for (i= 0; i < (m_symbols->itemIndex(node) + 1); i++)
    //    qDebug(13000)<<"Symbol row :"<<positions.at(i);
}

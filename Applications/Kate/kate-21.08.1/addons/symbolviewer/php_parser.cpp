/***************************************************************************
                          php_parser.cpp  -  description
                             -------------------
    begin         : Apr 1st 2007
    last update   : Sep 14th 2010
    author(s)     : 2007, Massimo Callegari <massimocallegari@yahoo.it>
                  : 2010, Emmanuel Bouthenot <kolter@openics.org>
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "plugin_katesymbolviewer.h"

void KatePluginSymbolViewerView::parsePhpSymbols(void)
{
    if (m_mainWindow->activeView()) {
        QString line, lineWithliterals;
        QPixmap namespacePix(class_int_xpm);
        QPixmap definePix(macro_xpm);
        QPixmap varPix(struct_xpm);
        QPixmap classPix(class_xpm);
        QPixmap constPix(macro_xpm);
        QPixmap functionPix(method_xpm);
        QTreeWidgetItem *node = nullptr;
        QTreeWidgetItem *namespaceNode = nullptr, *defineNode = nullptr, *classNode = nullptr, *functionNode = nullptr;
        QTreeWidgetItem *lastNamespaceNode = nullptr, *lastDefineNode = nullptr, *lastClassNode = nullptr, *lastFunctionNode = nullptr;

        KTextEditor::Document *kv = m_mainWindow->activeView()->document();

        if (m_treeOn->isChecked()) {
            namespaceNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Namespaces")));
            defineNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Defines")));
            classNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Classes")));
            functionNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Functions")));

            namespaceNode->setIcon(0, QIcon(namespacePix));
            defineNode->setIcon(0, QIcon(definePix));
            classNode->setIcon(0, QIcon(classPix));
            functionNode->setIcon(0, QIcon(functionPix));

            if (m_expandOn->isChecked()) {
                m_symbols->expandItem(namespaceNode);
                m_symbols->expandItem(defineNode);
                m_symbols->expandItem(classNode);
                m_symbols->expandItem(functionNode);
            }

            lastNamespaceNode = namespaceNode;
            lastDefineNode = defineNode;
            lastClassNode = classNode;
            lastFunctionNode = functionNode;

            m_symbols->setRootIsDecorated(1);
        } else {
            m_symbols->setRootIsDecorated(0);
        }

        // Namespaces: https://www.php.net/manual/en/language.namespaces.php
        QRegExp namespaceRegExp(QLatin1String("^namespace\\s+([^;\\s]+)"), Qt::CaseInsensitive);
        // defines: https://www.php.net/manual/en/function.define.php
        QRegExp defineRegExp(QLatin1String("(^|\\W)define\\s*\\(\\s*['\"]([^'\"]+)['\"]"), Qt::CaseInsensitive);
        // classes: https://www.php.net/manual/en/language.oop5.php
        QRegExp classRegExp(QLatin1String("^((abstract\\s+|final\\s+)?)class\\s+([\\w_][\\w\\d_]*)\\s*(implements\\s+[\\w\\d_]*)?"), Qt::CaseInsensitive);
        // interfaces: https://www.php.net/manual/en/language.oop5.php
        QRegExp interfaceRegExp(QLatin1String("^interface\\s+([\\w_][\\w\\d_]*)"), Qt::CaseInsensitive);
        // classes constants: https://www.php.net/manual/en/language.oop5.constants.php
        QRegExp constantRegExp(QLatin1String("^const\\s+([\\w_][\\w\\d_]*)"), Qt::CaseInsensitive);
        // functions: https://www.php.net/manual/en/language.oop5.constants.php
        QRegExp functionRegExp(QLatin1String("^((public|protected|private)?(\\s*static)?\\s+)?function\\s+&?\\s*([\\w_][\\w\\d_]*)\\s*(.*)$"),
                               Qt::CaseInsensitive);
        // variables: https://www.php.net/manual/en/language.oop5.properties.php
        QRegExp varRegExp(QLatin1String("^((var|public|protected|private)?(\\s*static)?\\s+)?\\$([\\w_][\\w\\d_]*)"), Qt::CaseInsensitive);

        // function args detection: “function a($b, $c=null)” => “$b, $v”
        QRegExp functionArgsRegExp(QLatin1String("(\\$[\\w_]+)"), Qt::CaseInsensitive);
        QStringList functionArgsList;
        QString nameWithTypes;

        // replace literals by empty strings: “function a($b='nothing', $c="pretty \"cool\" string")” => “function ($b='', $c="")”
        QRegExp literalRegExp(QLatin1String("([\"'])(?:\\\\.|[^\\\\])*\\1"));
        literalRegExp.setMinimal(true);
        // remove useless comments: “public/* static */ function a($b, $c=null) /* test */” => “public function a($b, $c=null)”
        QRegExp blockCommentInline(QLatin1String("/\\*.*\\*/"));
        blockCommentInline.setMinimal(true);

        int i, pos;
        bool isClass, isInterface;
        bool inBlockComment = false;
        bool inClass = false, inFunction = false;

        // QString debugBuffer("SymbolViewer(PHP), line %1 %2 → [%3]");

        for (i = 0; i < kv->lines(); i++) {
            // kdDebug(13000) << debugBuffer.arg(i, 4).arg("=origin", 10).arg(kv->line(i));

            line = kv->line(i).simplified();
            // kdDebug(13000) << debugBuffer.arg(i, 4).arg("+simplified", 10).arg(line);

            // keeping a copy with literals for catching “defines()”
            lineWithliterals = line;

            // reduce literals to empty strings to not match comments separators in literals
            line.replace(literalRegExp, QLatin1String("\\1\\1"));
            // kdDebug(13000) << debugBuffer.arg(i, 4).arg("-literals", 10).arg(line);

            line.remove(blockCommentInline);
            // kdDebug(13000) << debugBuffer.arg(i, 4).arg("-comments", 10).arg(line);

            // trying to find comments and to remove commented parts
            pos = line.indexOf(QLatin1Char('#'));
            if (pos >= 0) {
                line.truncate(pos);
            }
            pos = line.indexOf(QLatin1String("//"));
            if (pos >= 0) {
                line.truncate(pos);
            }
            pos = line.indexOf(QLatin1String("/*"));
            if (pos >= 0) {
                line.truncate(pos);
                inBlockComment = true;
            }
            pos = line.indexOf(QLatin1String("*/"));
            if (pos >= 0) {
                line = line.right(line.length() - pos - 2);
                inBlockComment = false;
            }

            if (inBlockComment) {
                continue;
            }

            // trimming again after having removed the comments
            line = line.simplified();
            // kdDebug(13000) << debugBuffer.arg(i, 4).arg("+simplified", 10).arg(line);

            // detect NameSpaces
            if (namespaceRegExp.indexIn(line) != -1) {
                if (m_treeOn->isChecked()) {
                    node = new QTreeWidgetItem(namespaceNode, lastNamespaceNode);
                    if (m_expandOn->isChecked()) {
                        m_symbols->expandItem(node);
                    }
                    lastNamespaceNode = node;
                } else {
                    node = new QTreeWidgetItem(m_symbols);
                }
                node->setText(0, namespaceRegExp.cap(1));
                node->setIcon(0, QIcon(namespacePix));
                node->setText(1, QString::number(i, 10));
            }

            // detect defines
            if (defineRegExp.indexIn(lineWithliterals) != -1) {
                if (m_treeOn->isChecked()) {
                    node = new QTreeWidgetItem(defineNode, lastDefineNode);
                    lastDefineNode = node;
                } else {
                    node = new QTreeWidgetItem(m_symbols);
                }
                node->setText(0, defineRegExp.cap(2));
                node->setIcon(0, QIcon(definePix));
                node->setText(1, QString::number(i, 10));
            }

            // detect classes, interfaces
            isClass = classRegExp.indexIn(line) != -1;
            isInterface = interfaceRegExp.indexIn(line) != -1;
            if (isClass || isInterface) {
                if (m_treeOn->isChecked()) {
                    node = new QTreeWidgetItem(classNode, lastClassNode);
                    if (m_expandOn->isChecked()) {
                        m_symbols->expandItem(node);
                    }
                    lastClassNode = node;
                } else {
                    node = new QTreeWidgetItem(m_symbols);
                }
                if (isClass) {
                    if (m_typesOn->isChecked()) {
                        if (!classRegExp.cap(1).trimmed().isEmpty() && !classRegExp.cap(4).trimmed().isEmpty()) {
                            nameWithTypes = classRegExp.cap(3) + QLatin1String(" [") + classRegExp.cap(1).trimmed() + QLatin1Char(',')
                                + classRegExp.cap(4).trimmed() + QLatin1Char(']');
                        } else if (!classRegExp.cap(1).trimmed().isEmpty()) {
                            nameWithTypes = classRegExp.cap(3) + QLatin1String(" [") + classRegExp.cap(1).trimmed() + QLatin1Char(']');
                        } else if (!classRegExp.cap(4).trimmed().isEmpty()) {
                            nameWithTypes = classRegExp.cap(3) + QLatin1String(" [") + classRegExp.cap(4).trimmed() + QLatin1Char(']');
                        }
                        node->setText(0, nameWithTypes);
                    } else {
                        node->setText(0, classRegExp.cap(3));
                    }
                } else {
                    if (m_typesOn->isChecked()) {
                        nameWithTypes = interfaceRegExp.cap(1) + QLatin1String(" [interface]");
                        node->setText(0, nameWithTypes);
                    } else {
                        node->setText(0, interfaceRegExp.cap(1));
                    }
                }
                node->setIcon(0, QIcon(classPix));
                node->setText(1, QString::number(i, 10));
                node->setToolTip(0, nameWithTypes);
                inClass = true;
                inFunction = false;
            }

            // detect class constants
            if (constantRegExp.indexIn(line) != -1) {
                if (m_treeOn->isChecked()) {
                    node = new QTreeWidgetItem(lastClassNode);
                } else {
                    node = new QTreeWidgetItem(m_symbols);
                }
                node->setText(0, constantRegExp.cap(1));
                node->setIcon(0, QIcon(constPix));
                node->setText(1, QString::number(i, 10));
            }

            // detect class variables
            if (inClass && !inFunction) {
                if (varRegExp.indexIn(line) != -1) {
                    if (m_treeOn->isChecked() && inClass) {
                        node = new QTreeWidgetItem(lastClassNode);
                    } else {
                        node = new QTreeWidgetItem(m_symbols);
                    }
                    node->setText(0, varRegExp.cap(4));
                    node->setIcon(0, QIcon(varPix));
                    node->setText(1, QString::number(i, 10));
                }
            }

            // detect functions
            if (functionRegExp.indexIn(line) != -1) {
                if (m_treeOn->isChecked() && inClass) {
                    node = new QTreeWidgetItem(lastClassNode);
                } else if (m_treeOn->isChecked()) {
                    node = new QTreeWidgetItem(lastFunctionNode);
                } else {
                    node = new QTreeWidgetItem(m_symbols);
                }

                QString functionArgs(functionRegExp.cap(5));
                pos = 0;
                while (pos >= 0) {
                    pos = functionArgsRegExp.indexIn(functionArgs, pos);
                    if (pos >= 0) {
                        pos += functionArgsRegExp.matchedLength();
                        functionArgsList += functionArgsRegExp.cap(1);
                    }
                }

                nameWithTypes = functionRegExp.cap(4) + QLatin1Char('(') + functionArgsList.join(QLatin1String(", ")) + QLatin1Char(')');
                if (m_typesOn->isChecked()) {
                    node->setText(0, nameWithTypes);
                } else {
                    node->setText(0, functionRegExp.cap(4));
                }

                node->setIcon(0, QIcon(functionPix));
                node->setText(1, QString::number(i, 10));
                node->setToolTip(0, nameWithTypes);

                functionArgsList.clear();

                inFunction = true;
            }
        }
    }
}

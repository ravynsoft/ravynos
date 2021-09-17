/***************************************************************************
                          ruby_parser.cpp  -  description
                             -------------------
    begin                : May 9th 2007
    author               : 2007 Massimo Callegari
    email                : massimocallegari@yahoo.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#include "plugin_katesymbolviewer.h"

void KatePluginSymbolViewerView::parseRubySymbols(void)
{
    if (!m_mainWindow->activeView()) {
        return;
    }

    m_macro->setText(i18n("Show Globals"));
    m_struct->setText(i18n("Show Methods"));
    m_func->setText(i18n("Show Classes"));

    QString cl; // Current Line
    QPixmap cls(class_xpm);
    QPixmap mtd(method_xpm);

    int i;
    QString name;

    QTreeWidgetItem *node = nullptr;
    QTreeWidgetItem *mtdNode = nullptr, *clsNode = nullptr;
    QTreeWidgetItem *lastMtdNode = nullptr, *lastClsNode = nullptr;

    KTextEditor::Document *kv = m_mainWindow->activeView()->document();
    // kdDebug(13000)<<"Lines counted :"<<kv->numLines()<<endl;

    if (m_treeOn->isChecked()) {
        clsNode = new QTreeWidgetItem(m_symbols);
        clsNode->setText(0, i18n("Classes"));
        clsNode->setIcon(0, QIcon(cls));
        if (m_expandOn->isChecked()) {
            m_symbols->expandItem(clsNode);
        }
        lastClsNode = clsNode;
        mtdNode = clsNode;
        lastMtdNode = clsNode;
        m_symbols->setRootIsDecorated(1);
    } else {
        m_symbols->setRootIsDecorated(0);
    }

    for (i = 0; i < kv->lines(); i++) {
        cl = kv->line(i);
        cl = cl.trimmed();

        if (cl.indexOf(QRegularExpression(QLatin1String("^class [a-zA-Z0-9]+[^#]"))) >= 0) {
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
                node->setText(0, cl.mid(6));
                node->setIcon(0, QIcon(cls));
                node->setText(1, QString::number(i, 10));
            }
        }
        if (cl.indexOf(QRegularExpression(QLatin1String("^def [a-zA-Z_]+[^#]"))) >= 0) {
            if (m_struct->isChecked()) {
                if (m_treeOn->isChecked()) {
                    node = new QTreeWidgetItem(mtdNode, lastMtdNode);
                    lastMtdNode = node;
                } else {
                    node = new QTreeWidgetItem(m_symbols);
                }

                name = cl.mid(4);
                node->setToolTip(0, name);
                if (!m_typesOn->isChecked()) {
                    name = name.left(name.indexOf(QLatin1Char('(')));
                }
                node->setText(0, name);
                node->setIcon(0, QIcon(mtd));
                node->setText(1, QString::number(i, 10));
            }
        }
    }
}

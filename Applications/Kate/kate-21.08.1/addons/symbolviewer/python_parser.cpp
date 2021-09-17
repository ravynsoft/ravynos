/***************************************************************************
                           python_parser.cpp  -  description
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

void KatePluginSymbolViewerView::parsePythonSymbols(void)
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
    QPixmap mcr(macro_xpm);

    int in_class = 0, state = 0, j;
    QString name;

    QTreeWidgetItem *node = nullptr;
    QTreeWidgetItem *mcrNode = nullptr, *mtdNode = nullptr, *clsNode = nullptr;
    QTreeWidgetItem *lastMcrNode = nullptr, *lastMtdNode = nullptr, *lastClsNode = nullptr;

    KTextEditor::Document *kv = m_mainWindow->activeView()->document();

    // kdDebug(13000)<<"Lines counted :"<<kv->numLines()<<endl;
    if (m_treeOn->isChecked()) {
        clsNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Classes")));
        mcrNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Globals")));
        mcrNode->setIcon(0, QIcon(mcr));
        clsNode->setIcon(0, QIcon(cls));

        if (m_expandOn->isChecked()) {
            m_symbols->expandItem(mcrNode);
            m_symbols->expandItem(clsNode);
        }
        lastClsNode = clsNode;
        lastMcrNode = mcrNode;
        mtdNode = clsNode;
        lastMtdNode = clsNode;
        m_symbols->setRootIsDecorated(1);
    } else {
        m_symbols->setRootIsDecorated(0);
    }

    for (int i = 0; i < kv->lines(); i++) {
        int line = i;
        cl = kv->line(i);
        // concatenate continued lines and remove continuation marker
        if (cl.length() == 0) {
            continue;
        }
        while (cl[cl.length() - 1] == QLatin1Char('\\')) {
            cl = cl.left(cl.length() - 1);
            i++;
            if (i < kv->lines()) {
                cl += kv->line(i);
            } else {
                break;
            }
        }

        if (cl.indexOf(QRegularExpression(QLatin1String("^class [a-zA-Z0-9_,\\s\\(\\).]+:"))) >= 0) {
            in_class = 1;
        }

        // if(cl.find( QRegularExpression(QLatin1String("[\\s]+def [a-zA-Z_]+[^#]*:")) ) >= 0) in_class = 2;
        if (cl.indexOf(QRegularExpression(QLatin1String("^def\\s+[a-zA-Z_]+[^#]*:"))) >= 0) {
            in_class = 0;
        }

        if (cl.indexOf(QLatin1String("def ")) >= 0 || (cl.indexOf(QLatin1String("class ")) >= 0 && in_class == 1)) {
            if (cl.indexOf(QLatin1String("def ")) >= 0 && in_class == 1) {
                in_class = 2;
            }
            state = 1;
            if (cl.indexOf(QLatin1Char(':')) >= 0) {
                state = 3; // found in the same line. Done
            } else if (cl.indexOf(QLatin1Char('(')) >= 0) {
                state = 2;
            }

            if (state == 2 || state == 3) {
                name = cl.left(cl.indexOf(QLatin1Char('(')));
            }
        }

        if (state > 0 && state < 3) {
            for (j = 0; j < cl.length(); j++) {
                if (cl.at(j) == QLatin1Char('(')) {
                    state = 2;
                } else if (cl.at(j) == QLatin1Char(':')) {
                    state = 3;
                    break;
                }

                if (state == 1) {
                    name += cl.at(j);
                }
            }
        }
        if (state == 3) {
            // qDebug(13000)<<"Function -- Inserted : "<<name<<" at row : "<<i;
            if (in_class == 1) { // strip off the word "class "
                name = name.trimmed().mid(6);
            } else { // strip off the word "def "
                name = name.trimmed().mid(4);
            }

            if (m_func->isChecked() && in_class == 1) {
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

                node->setText(0, name);
                node->setIcon(0, QIcon(cls));
                node->setText(1, QString::number(line, 10));
            }

            if (m_struct->isChecked() && in_class == 2) {
                if (m_treeOn->isChecked()) {
                    node = new QTreeWidgetItem(mtdNode, lastMtdNode);
                    lastMtdNode = node;
                } else {
                    node = new QTreeWidgetItem(m_symbols);
                }

                node->setText(0, name);
                node->setIcon(0, QIcon(mtd));
                node->setText(1, QString::number(line, 10));
            }

            if (m_macro->isChecked() && in_class == 0) {
                if (m_treeOn->isChecked()) {
                    node = new QTreeWidgetItem(mcrNode, lastMcrNode);
                    lastMcrNode = node;
                } else {
                    node = new QTreeWidgetItem(m_symbols);
                }

                node->setText(0, name);
                node->setIcon(0, QIcon(mcr));
                node->setText(1, QString::number(line, 10));
            }

            state = 0;
            name.clear();
        }
    }
}

// kate: space-indent on; indent-width 2; replace-tabs on;

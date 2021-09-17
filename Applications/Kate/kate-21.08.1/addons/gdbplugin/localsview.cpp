//
// Description: Widget that local variables of the gdb inferior
//
// SPDX-FileCopyrightText: 2010 Kåre Särs <kare.sars@iki.fi>
//
//  SPDX-License-Identifier: LGPL-2.0-only

#include "localsview.h"
#include <KLocalizedString>
#include <QDebug>
#include <QLabel>

LocalsView::LocalsView(QWidget *parent)
    : QTreeWidget(parent)
{
    QStringList headers;
    headers << i18n("Symbol");
    headers << i18n("Value");
    setHeaderLabels(headers);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

LocalsView::~LocalsView()
{
}

void LocalsView::showEvent(QShowEvent *)
{
    Q_EMIT localsVisible(true);
}

void LocalsView::hideEvent(QHideEvent *)
{
    Q_EMIT localsVisible(false);
}

void LocalsView::createWrappedItem(QTreeWidgetItem *parent, const QString &name, const QString &value)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList(name));
    QLabel *label = new QLabel(value);
    label->setWordWrap(true);
    setItemWidget(item, 1, label);
    item->setData(1, Qt::UserRole, value);
    item->setToolTip(0, QStringLiteral("<qt>%1<qt>").arg(name));
    item->setToolTip(1, QStringLiteral("<qt>%1<qt>").arg(value));
    parent->setToolTip(0, QStringLiteral("<qt>%1<qt>").arg(parent->text(0)));
}

void LocalsView::createWrappedItem(QTreeWidget *parent, const QString &name, const QString &value)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList(name));
    QLabel *label = new QLabel(value);
    label->setWordWrap(true);
    setItemWidget(item, 1, label);
    item->setToolTip(0, QStringLiteral("<qt>%1<qt>").arg(name));
    item->setToolTip(1, QStringLiteral("<qt>%1<qt>").arg(value));
}

void LocalsView::addLocal(const QString &vString)
{
    static const QRegularExpression isValue(QStringLiteral("\\A(\\S*)\\s=\\s(.*)\\z"));
    static const QRegularExpression isStruct(QStringLiteral("\\A\\{\\S*\\s=\\s.*\\z"));
    static const QRegularExpression isStartPartial(QStringLiteral("\\A\\S*\\s=\\s\\S*\\s=\\s\\{\\z"));
    static const QRegularExpression isPrettyQList(QStringLiteral("\\A\\s*\\[\\S*\\]\\s=\\s\\S*\\z"));
    static const QRegularExpression isPrettyValue(QStringLiteral("\\A(\\S*)\\s=\\s(\\S*)\\s=\\s(.*)\\z"));
    static const QRegularExpression isThisValue(QStringLiteral("\\A\\$\\d+\\z"));

    if (m_allAdded) {
        clear();
        m_allAdded = false;
    }

    if (vString.isEmpty()) {
        m_allAdded = true;
        return;
    }

    QRegularExpressionMatch match = isStartPartial.match(vString);
    if (match.hasMatch()) {
        m_local = vString;
        return;
    }
    match = isPrettyQList.match(vString);
    if (match.hasMatch()) {
        m_local += vString.trimmed();
        if (m_local.endsWith(QLatin1Char(','))) {
            m_local += QLatin1Char(' ');
        }
        return;
    }
    if (vString == QLatin1String("}")) {
        m_local += vString;
    }

    QStringList symbolAndValue;
    QString value;

    if (m_local.isEmpty()) {
        if (vString == QLatin1String("No symbol table info available.")) {
            return; /* this is not an error */
        }
        match = isValue.match(vString);
        if (!match.hasMatch()) {
            qDebug() << "Could not parse:" << vString;
            return;
        }
        symbolAndValue << match.captured(1);
        value = match.captured(2);
        // check out for "print *this"
        match = isThisValue.match(symbolAndValue[0]);
        if (match.hasMatch()) {
            symbolAndValue[0] = QStringLiteral("*this");
        }
    } else {
        match = isPrettyValue.match(m_local);
        if (!match.hasMatch()) {
            qDebug() << "Could not parse:" << m_local;
            m_local.clear();
            return;
        }
        symbolAndValue << match.captured(1) << match.captured(2);
        value = match.captured(3);
    }

    QTreeWidgetItem *item;
    if (value[0] == QLatin1Char('{')) {
        if (value[1] == QLatin1Char('{')) {
            item = new QTreeWidgetItem(this, symbolAndValue);
            addArray(item, value.mid(1, value.size() - 2));
        } else {
            match = isStruct.match(value);
            if (match.hasMatch()) {
                item = new QTreeWidgetItem(this, symbolAndValue);
                addStruct(item, value.mid(1, value.size() - 2));
            } else {
                createWrappedItem(this, symbolAndValue[0], value);
            }
        }
    } else {
        createWrappedItem(this, symbolAndValue[0], value);
    }

    m_local.clear();
}

void LocalsView::addStruct(QTreeWidgetItem *parent, const QString &vString)
{
    static const QRegularExpression isArray(QStringLiteral("\\A\\{\\.*\\s=\\s.*\\z"));
    static const QRegularExpression isStruct(QStringLiteral("\\A\\.*\\s=\\s.*\\z"));
    QTreeWidgetItem *item;
    QStringList symbolAndValue;
    QString subValue;
    int start = 0;
    int end;
    while (start < vString.size()) {
        // Symbol
        symbolAndValue.clear();
        end = vString.indexOf(QLatin1String(" = "), start);
        if (end < 0) {
            // error situation -> bail out
            createWrappedItem(parent, QString(), vString.right(start));
            break;
        }
        symbolAndValue << vString.mid(start, end - start);
        // qDebug() << symbolAndValue;
        // Value
        start = end + 3;
        end = start;
        if (start < 0 || start >= vString.size()) {
            qDebug() << vString << start;
            break;
        }
        if (vString[start] == QLatin1Char('{')) {
            start++;
            end++;
            int count = 1;
            bool inComment = false;
            // search for the matching }
            while (end < vString.size()) {
                if (!inComment) {
                    if (vString[end] == QLatin1Char('"')) {
                        inComment = true;
                    } else if (vString[end] == QLatin1Char('}')) {
                        count--;
                    } else if (vString[end] == QLatin1Char('{')) {
                        count++;
                    }
                    if (count == 0) {
                        break;
                    }
                } else {
                    if ((vString[end] == QLatin1Char('"')) && (vString[end - 1] != QLatin1Char('\\'))) {
                        inComment = false;
                    }
                }
                end++;
            }
            subValue = vString.mid(start, end - start);
            if (isArray.match(subValue).hasMatch()) {
                item = new QTreeWidgetItem(parent, symbolAndValue);
                addArray(item, subValue);
            } else if (isStruct.match(subValue).hasMatch()) {
                item = new QTreeWidgetItem(parent, symbolAndValue);
                addStruct(item, subValue);
            } else {
                createWrappedItem(parent, symbolAndValue[0], vString.mid(start, end - start));
            }
            start = end + 3; // },_
        } else {
            // look for the end of the value in the vString
            bool inComment = false;
            while (end < vString.size()) {
                if (!inComment) {
                    if (vString[end] == QLatin1Char('"')) {
                        inComment = true;
                    } else if (vString[end] == QLatin1Char(',')) {
                        break;
                    }
                } else {
                    if ((vString[end] == QLatin1Char('"')) && (vString[end - 1] != QLatin1Char('\\'))) {
                        inComment = false;
                    }
                }
                end++;
            }
            createWrappedItem(parent, symbolAndValue[0], vString.mid(start, end - start));
            start = end + 2; // ,_
        }
    }
}

void LocalsView::addArray(QTreeWidgetItem *parent, const QString &vString)
{
    // getting here we have this kind of string:
    // "{...}" or "{...}, {...}" or ...
    QTreeWidgetItem *item;
    int count = 1;
    bool inComment = false;
    int index = 0;
    int start = 1;
    int end = 1;

    while (end < vString.size()) {
        if (!inComment) {
            if (vString[end] == QLatin1Char('"')) {
                inComment = true;
            } else if (vString[end] == QLatin1Char('}')) {
                count--;
            } else if (vString[end] == QLatin1Char('{')) {
                count++;
            }
            if (count == 0) {
                QStringList name;
                name << QStringLiteral("[%1]").arg(index);
                index++;
                item = new QTreeWidgetItem(parent, name);
                addStruct(item, vString.mid(start, end - start));
                end += 4; // "}, {"
                start = end;
                count = 1;
            }
        } else {
            if ((vString[end] == QLatin1Char('"')) && (vString[end - 1] != QLatin1Char('\\'))) {
                inComment = false;
            }
        }
        end++;
    }
}

/***************************************************************************
    pseudoDtd.cpp
    copyright           : (C) 2001-2002 by Daniel Naber
    email               : daniel.naber@t-online.de
 ***************************************************************************/

/***************************************************************************
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or ( at your option ) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ***************************************************************************/

#include "pseudo_dtd.h"

#include <QRegExp>

#include <KLocalizedString>
#include <KMessageBox>

PseudoDTD::PseudoDTD()
{
    // "SGML support" only means case-insensivity, because HTML is case-insensitive up to version 4:
    m_sgmlSupport = true; // TODO: make this an run-time option ( maybe automatically set )
}

PseudoDTD::~PseudoDTD()
{
}

void PseudoDTD::analyzeDTD(QString &metaDtdUrl, QString &metaDtd)
{
    QDomDocument doc(QStringLiteral("dtdIn_xml"));
    if (!doc.setContent(metaDtd)) {
        KMessageBox::error(nullptr,
                           i18n("The file '%1' could not be parsed. "
                                "Please check that the file is well-formed XML.",
                                metaDtdUrl),
                           i18n("XML Plugin Error"));
        return;
    }

    if (doc.doctype().name() != QLatin1String("dtd")) {
        KMessageBox::error(nullptr,
                           i18n("The file '%1' is not in the expected format. "
                                "Please check that the file is of this type:\n"
                                "-//Norman Walsh//DTD DTDParse V2.0//EN\n"
                                "You can produce such files with dtdparse. "
                                "See the Kate Plugin documentation for more information.",
                                metaDtdUrl),
                           i18n("XML Plugin Error"));
        return;
    }

    uint listLength = 0;
    listLength += doc.elementsByTagName(QStringLiteral("entity")).count();
    listLength += doc.elementsByTagName(QStringLiteral("element")).count();
    // count this twice, as it will be iterated twice ( TODO: optimize that? ):
    listLength += doc.elementsByTagName(QStringLiteral("attlist")).count() * 2;

    QProgressDialog progress(i18n("Analyzing meta DTD..."), i18n("Cancel"), 0, listLength);
    progress.setMinimumDuration(400);
    progress.setValue(0);

    // Get information from meta DTD and put it in Qt data structures for fast access:
    if (!parseEntities(&doc, &progress)) {
        return;
    }

    if (!parseElements(&doc, &progress)) {
        return;
    }

    if (!parseAttributes(&doc, &progress)) {
        return;
    }

    if (!parseAttributeValues(&doc, &progress)) {
        return;
    }

    progress.setValue(listLength); // just to make sure the dialog disappears
}

// ========================================================================
// DOM stuff:

/**
 * Iterate through the XML to get a mapping which sub-elements are allowed for
 * all elements.
 */
bool PseudoDTD::parseElements(QDomDocument *doc, QProgressDialog *progress)
{
    m_elementsList.clear();
    // We only display a list, i.e. we pretend that the content model is just
    // a set, so we use a map. This is necessary e.g. for xhtml 1.0's head element,
    // which would otherwise display some elements twice.
    QMap<QString, bool> subelementList; // the bool is not used

    QDomNodeList list = doc->elementsByTagName(QStringLiteral("element"));
    uint listLength = list.count(); // speedup (really! )

    for (uint i = 0; i < listLength; i++) {
        if (progress->wasCanceled()) {
            return false;
        }

        progress->setValue(progress->value() + 1);
        // FIXME!:
        // qApp->processEvents();

        subelementList.clear();
        QDomNode node = list.item(i);
        QDomElement elem = node.toElement();

        if (!elem.isNull()) {
            // Enter the expanded content model, which may also include stuff not allowed.
            // We do not care if it's a <sequence-group> or whatever.
            QDomNodeList contentModelList = elem.elementsByTagName(QStringLiteral("content-model-expanded"));
            QDomNode contentModelNode = contentModelList.item(0);
            QDomElement contentModelElem = contentModelNode.toElement();
            if (!contentModelElem.isNull()) {
                // check for <pcdata/>:
                QDomNodeList pcdataList = contentModelElem.elementsByTagName(QStringLiteral("pcdata"));

                // check for other sub elements:
                QDomNodeList subList = contentModelElem.elementsByTagName(QStringLiteral("element-name"));
                uint subListLength = subList.count();
                for (uint l = 0; l < subListLength; l++) {
                    QDomNode subNode = subList.item(l);
                    QDomElement subElem = subNode.toElement();
                    if (!subElem.isNull()) {
                        subelementList[subElem.attribute(QStringLiteral("name"))] = true;
                    }
                }

                // anders: check if this is an EMPTY element, and put "__EMPTY" in the
                // sub list, so that we can insert tags in empty form if required.
                QDomNodeList emptyList = elem.elementsByTagName(QStringLiteral("empty"));
                if (emptyList.count()) {
                    subelementList[QStringLiteral("__EMPTY")] = true;
                }
            }

            // Now remove the elements not allowed (e.g. <a> is explicitly not allowed in <a>
            // in the HTML 4.01 Strict DTD):
            QDomNodeList exclusionsList = elem.elementsByTagName(QStringLiteral("exclusions"));
            if (exclusionsList.length() > 0) {
                // sometimes there are no exclusions ( e.g. in XML DTDs there are never exclusions )
                QDomNode exclusionsNode = exclusionsList.item(0);
                QDomElement exclusionsElem = exclusionsNode.toElement();
                if (!exclusionsElem.isNull()) {
                    QDomNodeList subList = exclusionsElem.elementsByTagName(QStringLiteral("element-name"));
                    uint subListLength = subList.count();
                    for (uint l = 0; l < subListLength; l++) {
                        QDomNode subNode = subList.item(l);
                        QDomElement subElem = subNode.toElement();
                        if (!subElem.isNull()) {
                            QMap<QString, bool>::Iterator it = subelementList.find(subElem.attribute(QStringLiteral("name")));
                            if (it != subelementList.end()) {
                                subelementList.erase(it);
                            }
                        }
                    }
                }
            }

            // turn the map into a list:
            QStringList subelementListTmp;
            QMap<QString, bool>::Iterator it;
            for (it = subelementList.begin(); it != subelementList.end(); ++it) {
                subelementListTmp.append(it.key());
            }

            m_elementsList.insert(elem.attribute(QStringLiteral("name")), subelementListTmp);
        }

    } // end iteration over all <element> nodes
    return true;
}

/**
 * Check which elements are allowed inside a parent element. This returns
 * a list of allowed elements, but it doesn't care about order or if only a certain
 * number of occurrences is allowed.
 */
QStringList PseudoDTD::allowedElements(const QString &parentElement)
{
    if (m_sgmlSupport) {
        // find the matching element, ignoring case:
        QMap<QString, QStringList>::Iterator it;
        for (it = m_elementsList.begin(); it != m_elementsList.end(); ++it) {
            if (it.key().compare(parentElement, Qt::CaseInsensitive) == 0) {
                return it.value();
            }
        }
    } else if (m_elementsList.contains(parentElement)) {
        return m_elementsList[parentElement];
    }

    return QStringList();
}

/**
 * Iterate through the XML to get a mapping which attributes are allowed inside
 * all elements.
 */
bool PseudoDTD::parseAttributes(QDomDocument *doc, QProgressDialog *progress)
{
    m_attributesList.clear();
    //   QStringList allowedAttributes;
    QDomNodeList list = doc->elementsByTagName(QStringLiteral("attlist"));
    uint listLength = list.count();

    for (uint i = 0; i < listLength; i++) {
        if (progress->wasCanceled()) {
            return false;
        }

        progress->setValue(progress->value() + 1);
        // FIXME!!
        // qApp->processEvents();

        ElementAttributes attrs;
        QDomNode node = list.item(i);
        QDomElement elem = node.toElement();
        if (!elem.isNull()) {
            QDomNodeList attributeList = elem.elementsByTagName(QStringLiteral("attribute"));
            uint attributeListLength = attributeList.count();
            for (uint l = 0; l < attributeListLength; l++) {
                QDomNode attributeNode = attributeList.item(l);
                QDomElement attributeElem = attributeNode.toElement();

                if (!attributeElem.isNull()) {
                    if (attributeElem.attribute(QStringLiteral("type")) == QLatin1String("#REQUIRED")) {
                        attrs.requiredAttributes.append(attributeElem.attribute(QStringLiteral("name")));
                    } else {
                        attrs.optionalAttributes.append(attributeElem.attribute(QStringLiteral("name")));
                    }
                }
            }
            m_attributesList.insert(elem.attribute(QStringLiteral("name")), attrs);
        }
    }

    return true;
}

/** Check which attributes are allowed for an element.
 */
QStringList PseudoDTD::allowedAttributes(const QString &element)
{
    if (m_sgmlSupport) {
        // find the matching element, ignoring case:
        QMap<QString, ElementAttributes>::Iterator it;
        for (it = m_attributesList.begin(); it != m_attributesList.end(); ++it) {
            if (it.key().compare(element, Qt::CaseInsensitive) == 0) {
                return it.value().optionalAttributes + it.value().requiredAttributes;
            }
        }
    } else if (m_attributesList.contains(element)) {
        return m_attributesList[element].optionalAttributes + m_attributesList[element].requiredAttributes;
    }

    return QStringList();
}

QStringList PseudoDTD::requiredAttributes(const QString &element) const
{
    if (m_sgmlSupport) {
        QMap<QString, ElementAttributes>::ConstIterator it;
        for (it = m_attributesList.begin(); it != m_attributesList.end(); ++it) {
            if (it.key().compare(element, Qt::CaseInsensitive) == 0) {
                return it.value().requiredAttributes;
            }
        }
    } else if (m_attributesList.contains(element)) {
        return m_attributesList[element].requiredAttributes;
    }

    return QStringList();
}

/**
 * Iterate through the XML to get a mapping which attribute values are allowed
 * for all attributes inside all elements.
 */
bool PseudoDTD::parseAttributeValues(QDomDocument *doc, QProgressDialog *progress)
{
    m_attributevaluesList.clear(); // 1 element : n possible attributes
    QMap<QString, QStringList> attributevaluesTmp; // 1 attribute : n possible values
    QDomNodeList list = doc->elementsByTagName(QStringLiteral("attlist"));
    uint listLength = list.count();

    for (uint i = 0; i < listLength; i++) {
        if (progress->wasCanceled()) {
            return false;
        }

        progress->setValue(progress->value() + 1);
        // FIXME!
        // qApp->processEvents();

        attributevaluesTmp.clear();
        QDomNode node = list.item(i);
        QDomElement elem = node.toElement();
        if (!elem.isNull()) {
            // Enter the list of <attribute>:
            QDomNodeList attributeList = elem.elementsByTagName(QStringLiteral("attribute"));
            uint attributeListLength = attributeList.count();
            for (uint l = 0; l < attributeListLength; l++) {
                QDomNode attributeNode = attributeList.item(l);
                QDomElement attributeElem = attributeNode.toElement();
                if (!attributeElem.isNull()) {
                    QString value = attributeElem.attribute(QStringLiteral("value"));
                    attributevaluesTmp.insert(attributeElem.attribute(QStringLiteral("name")), value.split(QChar(' ')));
                }
            }
            m_attributevaluesList.insert(elem.attribute(QStringLiteral("name")), attributevaluesTmp);
        }
    }
    return true;
}

/**
 * Check which attributes values are allowed for an attribute in an element
 * (the element is necessary because e.g. "href" inside <a> could be different
 * to an "href" inside <link>):
 */
QStringList PseudoDTD::attributeValues(const QString &element, const QString &attribute)
{
    // Direct access would be faster than iteration of course but not always correct,
    // because we need to be case-insensitive.
    if (m_sgmlSupport) {
        // first find the matching element, ignoring case:
        QMap<QString, QMap<QString, QStringList>>::Iterator it;
        for (it = m_attributevaluesList.begin(); it != m_attributevaluesList.end(); ++it) {
            if (it.key().compare(element, Qt::CaseInsensitive) == 0) {
                QMap<QString, QStringList> attrVals = it.value();
                QMap<QString, QStringList>::Iterator itV;
                // then find the matching attribute for that element, ignoring case:
                for (itV = attrVals.begin(); itV != attrVals.end(); ++itV) {
                    if (itV.key().compare(attribute, Qt::CaseInsensitive) == 0) {
                        return (itV.value());
                    }
                }
            }
        }
    } else if (m_attributevaluesList.contains(element)) {
        QMap<QString, QStringList> attrVals = m_attributevaluesList[element];
        if (attrVals.contains(attribute)) {
            return attrVals[attribute];
        }
    }

    // no predefined values available:
    return QStringList();
}

/**
 * Iterate through the XML to get a mapping of all entity names and their expanded
 * version, e.g. nbsp => &#160;. Parameter entities are ignored.
 */
bool PseudoDTD::parseEntities(QDomDocument *doc, QProgressDialog *progress)
{
    m_entityList.clear();
    QDomNodeList list = doc->elementsByTagName(QStringLiteral("entity"));
    uint listLength = list.count();

    for (uint i = 0; i < listLength; i++) {
        if (progress->wasCanceled()) {
            return false;
        }

        progress->setValue(progress->value() + 1);
        // FIXME!!
        // qApp->processEvents();
        QDomNode node = list.item(i);
        QDomElement elem = node.toElement();
        if (!elem.isNull() && elem.attribute(QStringLiteral("type")) != QLatin1String("param")) {
            // TODO: what's cdata <-> gen ?
            QDomNodeList expandedList = elem.elementsByTagName(QStringLiteral("text-expanded"));
            QDomNode expandedNode = expandedList.item(0);
            QDomElement expandedElem = expandedNode.toElement();
            if (!expandedElem.isNull()) {
                QString exp = expandedElem.text();
                // TODO: support more than one &#...; in the expanded text
                /* TODO include do this when the unicode font problem is solved:
                if( exp.contains(QRegularExpression("^&#x[a-zA-Z0-9]+;$")) ) {
                // hexadecimal numbers, e.g. "&#x236;"
                uint end = exp.find( ";" );
                exp = exp.mid( 3, end-3 );
                exp = QChar();
                } else if( exp.contains(QRegularExpression("^&#[0-9]+;$")) ) {
                // decimal numbers, e.g. "&#236;"
                uint end = exp.find( ";" );
                exp = exp.mid( 2, end-2 );
                exp = QChar( exp.toInt() );
                }
                */
                m_entityList.insert(elem.attribute(QStringLiteral("name")), exp);
            } else {
                m_entityList.insert(elem.attribute(QStringLiteral("name")), QString());
            }
        }
    }
    return true;
}

/**
 * Get a list of all ( non-parameter ) entities that start with a certain string.
 */
QStringList PseudoDTD::entities(const QString &start)
{
    QStringList entities;
    QMap<QString, QString>::Iterator it;
    for (it = m_entityList.begin(); it != m_entityList.end(); ++it) {
        if ((*it).startsWith(start)) {
            const QString &str = it.key();
            /* TODO: show entities as unicode character
            if( !it.data().isEmpty() ) {
            //str += " -- " + it.data();
            QRegExp re( "&#(\\d+);" );
            if( re.search(it.data()) != -1 ) {
            uint ch = re.cap( 1).toUInt();
            str += " -- " + QChar( ch).decomposition();
            }
            //qDebug() << "#" << it.data();
            }
            */
            entities.append(str);
            // TODO: later use a table view
        }
    }
    return entities;
}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;

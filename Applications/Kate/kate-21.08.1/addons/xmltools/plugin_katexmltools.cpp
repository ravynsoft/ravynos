/***************************************************************************
  pluginKatexmltools.cpp

  List elements, attributes, attribute values and entities allowed by DTD.
  Needs a DTD in XML format ( as produced by dtdparse ) for most features.

  copyright         : ( C ) 2001-2002 by Daniel Naber
  email             : daniel.naber@t-online.de

  SPDX-FileCopyrightText: 2005 Anders Lund <anders@alweb.dk>

  KDE SC 4 version (C) 2010 Tomas Trnka <tomastrnka@gmx.com>
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

/*
README:
The basic idea is this: certain keyEvents(), namely [<&" ], trigger a completion box.
This is intended as a help for editing. There are some cases where the XML
spec is not followed, e.g. one can add the same attribute twice to an element.
Also see the user documentation. If backspace is pressed after a completion popup
was closed, the popup will re-open. This way typos can be corrected and the popup
will reappear, which is quite comfortable.

FIXME:
-( docbook ) <author lang="">: insert space between the quotes, press "de" and return -> only "d" inserted
-The "Insert Element" dialog isn't case insensitive, but it should be
-See the "fixme"'s in the code

TODO:
-check for mem leaks
-add "Go to opening/parent tag"?
-check doctype to get top-level element
-can undo behaviour be improved?, e.g. the plugins internal deletions of text
 don't have to be an extra step
-don't offer entities if inside tag but outside attribute value

-Support for more than one namespace at the same time ( e.g. XSLT + XSL-FO )?
=>This could also be handled in the XSLT DTD fragment, as described in the XSLT 1.0 spec,
 but then at <xsl:template match="/"><html> it will only show you HTML elements!
=>So better "Assign meta DTD" and "Add meta DTD", the latter will expand the current meta DTD
-Option to insert empty element in <empty/> form
-Show expanded entities with QChar::QChar( int rc ) + unicode font
-Don't ignore entities defined in the document's prologue
-Only offer 'valid' elements, i.e. don't take the elements as a set but check
 if the DTD is matched ( order, number of occurrences, ... )

-Maybe only read the meta DTD file once, then store the resulting QMap on disk ( using QDataStream )?
 We'll then have to compare timeOf_cacheFile <-> timeOf_metaDtd.
-Try to use libxml
*/

#include "plugin_katexmltools.h"

#include <QAction>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>
#include <QVBoxLayout>

#include <ktexteditor/editor.h>

#include <KActionCollection>
#include <KHistoryComboBox>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KXMLGUIClient>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kxmlguifactory.h>

K_PLUGIN_FACTORY_WITH_JSON(PluginKateXMLToolsFactory, "katexmltools.json", registerPlugin<PluginKateXMLTools>();)

PluginKateXMLTools::PluginKateXMLTools(QObject *const parent, const QVariantList &)
    : KTextEditor::Plugin(parent)
{
}

PluginKateXMLTools::~PluginKateXMLTools()
{
}

QObject *PluginKateXMLTools::createView(KTextEditor::MainWindow *mainWindow)
{
    return new PluginKateXMLToolsView(mainWindow);
}

PluginKateXMLToolsView::PluginKateXMLToolsView(KTextEditor::MainWindow *mainWin)
    : QObject(mainWin)
    , m_mainWindow(mainWin)
    , m_model(this)
{
    // qDebug() << "PluginKateXMLTools constructor called";

    KXMLGUIClient::setComponentName(QStringLiteral("katexmltools"), i18n("Kate XML Tools"));
    setXMLFile(QStringLiteral("ui.rc"));

    QAction *actionInsert = new QAction(i18n("&Insert Element..."), this);
    connect(actionInsert, &QAction::triggered, &m_model, &PluginKateXMLToolsCompletionModel::slotInsertElement);
    actionCollection()->addAction(QStringLiteral("xml_tool_insert_element"), actionInsert);
    actionCollection()->setDefaultShortcut(actionInsert, Qt::CTRL | Qt::Key_Return);

    QAction *actionClose = new QAction(i18n("&Close Element"), this);
    connect(actionClose, &QAction::triggered, &m_model, &PluginKateXMLToolsCompletionModel::slotCloseElement);
    actionCollection()->addAction(QStringLiteral("xml_tool_close_element"), actionClose);
    actionCollection()->setDefaultShortcut(actionClose, Qt::CTRL | Qt::Key_Less);

    QAction *actionAssignDTD = new QAction(i18n("Assign Meta &DTD..."), this);
    connect(actionAssignDTD, &QAction::triggered, &m_model, &PluginKateXMLToolsCompletionModel::getDTD);
    actionCollection()->addAction(QStringLiteral("xml_tool_assign"), actionAssignDTD);

    mainWin->guiFactory()->addClient(this);

    connect(KTextEditor::Editor::instance()->application(),
            &KTextEditor::Application::documentDeleted,
            &m_model,
            &PluginKateXMLToolsCompletionModel::slotDocumentDeleted);
}

PluginKateXMLToolsView::~PluginKateXMLToolsView()
{
    m_mainWindow->guiFactory()->removeClient(this);

    // qDebug() << "xml tools destructor 1...";
    // TODO: unregister the model
}

PluginKateXMLToolsCompletionModel::PluginKateXMLToolsCompletionModel(QObject *const parent)
    : CodeCompletionModel(parent)
    , m_viewToAssignTo(nullptr)
    , m_mode(none)
    , m_correctPos(0)
{
}

PluginKateXMLToolsCompletionModel::~PluginKateXMLToolsCompletionModel()
{
    qDeleteAll(m_dtds);
    m_dtds.clear();
}

void PluginKateXMLToolsCompletionModel::slotDocumentDeleted(KTextEditor::Document *doc)
{
    // Remove the document from m_DTDs, and also delete the PseudoDTD
    // if it becomes unused.
    if (m_docDtds.contains(doc)) {
        qDebug() << "XMLTools:slotDocumentDeleted: documents: " << m_docDtds.count() << ", DTDs: " << m_dtds.count();
        PseudoDTD *dtd = m_docDtds.take(doc);

        if (m_docDtds.key(dtd)) {
            return;
        }

        QHash<QString, PseudoDTD *>::iterator it;
        for (it = m_dtds.begin(); it != m_dtds.end(); ++it) {
            if (it.value() == dtd) {
                m_dtds.erase(it);
                delete dtd;
                return;
            }
        }
    }
}

void PluginKateXMLToolsCompletionModel::completionInvoked(KTextEditor::View *kv, const KTextEditor::Range &range, const InvocationType invocationType)
{
    Q_UNUSED(range)
    Q_UNUSED(invocationType)

    qDebug() << "xml tools completionInvoked";

    KTextEditor::Document *doc = kv->document();
    if (!m_docDtds[doc])
    // no meta DTD assigned yet
    {
        return;
    }

    // debug to test speed:
    // QTime t; t.start();

    beginResetModel();
    m_allowed.clear();

    // get char on the left of the cursor:
    KTextEditor::Cursor curpos = kv->cursorPosition();
    uint line = curpos.line(), col = curpos.column();

    QString lineStr = kv->document()->line(line);
    QString leftCh = lineStr.mid(col - 1, 1);
    QString secondLeftCh = lineStr.mid(col - 2, 1);

    if (leftCh == QLatin1String("&")) {
        qDebug() << "Getting entities";
        m_allowed = m_docDtds[doc]->entities(QString());
        m_mode = entities;
    } else if (leftCh == QLatin1String("<")) {
        qDebug() << "*outside tag -> get elements";
        QString parentElement = getParentElement(*kv, 1);
        qDebug() << "parent: " << parentElement;
        m_allowed = m_docDtds[doc]->allowedElements(parentElement);
        m_mode = elements;
    } else if (leftCh == QLatin1String("/") && secondLeftCh == QLatin1String("<")) {
        qDebug() << "*close parent element";
        QString parentElement = getParentElement(*kv, 2);

        if (!parentElement.isEmpty()) {
            m_mode = closingtag;
            m_allowed = QStringList(parentElement);
        }
    } else if (leftCh == QLatin1Char(' ') || (isQuote(leftCh) && secondLeftCh == QLatin1String("="))) {
        // TODO: check secondLeftChar, too?! then you don't need to trigger
        // with space and we yet save CPU power
        QString currentElement = insideTag(*kv);
        QString currentAttribute;
        if (!currentElement.isEmpty()) {
            currentAttribute = insideAttribute(*kv);
        }

        qDebug() << "Tag: " << currentElement;
        qDebug() << "Attr: " << currentAttribute;

        if (!currentElement.isEmpty() && !currentAttribute.isEmpty()) {
            qDebug() << "*inside attribute -> get attribute values";
            m_allowed = m_docDtds[doc]->attributeValues(currentElement, currentAttribute);
            if (m_allowed.count() == 1
                && (m_allowed[0] == QLatin1String("CDATA") || m_allowed[0] == QLatin1String("ID") || m_allowed[0] == QLatin1String("IDREF")
                    || m_allowed[0] == QLatin1String("IDREFS") || m_allowed[0] == QLatin1String("ENTITY") || m_allowed[0] == QLatin1String("ENTITIES")
                    || m_allowed[0] == QLatin1String("NMTOKEN") || m_allowed[0] == QLatin1String("NMTOKENS") || m_allowed[0] == QLatin1String("NAME"))) {
                // these must not be taken literally, e.g. don't insert the string "CDATA"
                m_allowed.clear();
            } else {
                m_mode = attributevalues;
            }
        } else if (!currentElement.isEmpty()) {
            qDebug() << "*inside tag -> get attributes";
            m_allowed = m_docDtds[doc]->allowedAttributes(currentElement);
            m_mode = attributes;
        }
    }

    // qDebug() << "time elapsed (ms): " << t.elapsed();
    qDebug() << "Allowed strings: " << m_allowed.count();

    if (m_allowed.count() >= 1 && m_allowed[0] != QLatin1String("__EMPTY")) {
        m_allowed = sortQStringList(m_allowed);
    }
    setRowCount(m_allowed.count());
    endResetModel();
}

int PluginKateXMLToolsCompletionModel::columnCount(const QModelIndex &) const
{
    return 1;
}

int PluginKateXMLToolsCompletionModel::rowCount(const QModelIndex &parent) const
{
    if (!m_allowed.isEmpty()) { // Is there smth to complete?
        if (!parent.isValid()) { // Return the only one group node for root
            return 1;
        }
        if (parent.internalId() == groupNode) { // Return available rows count for group level node
            return m_allowed.size();
        }
    }
    return 0;
}

QModelIndex PluginKateXMLToolsCompletionModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) { // Is root/invalid index?
        return QModelIndex(); // Nothing to return...
    }
    if (index.internalId() == groupNode) { // Return a root node for group
        return QModelIndex();
    }
    // Otherwise, this is a leaf level, so return the only group as a parent
    return createIndex(0, 0, groupNode);
}

QModelIndex PluginKateXMLToolsCompletionModel::index(const int row, const int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        // At 'top' level only 'header' present, so nothing else than row 0 can be here...
        return row == 0 ? createIndex(row, column, groupNode) : QModelIndex();
    }
    if (parent.internalId() == groupNode) { // Is this a group node?
        if (0 <= row && row < m_allowed.size()) { // Make sure to return only valid indices
            return createIndex(row, column, nullptr); // Just return a leaf-level index
        }
    }
    // Leaf node has no children... nothing to return
    return QModelIndex();
}

QVariant PluginKateXMLToolsCompletionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) { // Nothing to do w/ invalid index
        return QVariant();
    }

    if (index.internalId() == groupNode) { // Return group level node data
        switch (role) {
        case KTextEditor::CodeCompletionModel::GroupRole:
            return QVariant(Qt::DisplayRole);
        case Qt::DisplayRole:
            return currentModeToString();
        default:
            break;
        }
        return QVariant(); // Nothing to return for other roles
    }
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case KTextEditor::CodeCompletionModel::Name:
            return m_allowed.at(index.row());
        default:
            break;
        }
    default:
        break;
    }
    return QVariant();
}

bool PluginKateXMLToolsCompletionModel::shouldStartCompletion(KTextEditor::View *view,
                                                              const QString &insertedText,
                                                              bool userInsertion,
                                                              const KTextEditor::Cursor &position)
{
    Q_UNUSED(view)
    Q_UNUSED(userInsertion)
    Q_UNUSED(position)
    const QString triggerChars = QStringLiteral("&</ '\""); // these are subsequently handled by completionInvoked()

    return triggerChars.contains(insertedText.right(1));
}

/**
 * Load the meta DTD. In case of success set the 'ready'
 * flag to true, to show that we're is ready to give hints about the DTD.
 */
void PluginKateXMLToolsCompletionModel::getDTD()
{
    if (!KTextEditor::Editor::instance()->application()->activeMainWindow()) {
        return;
    }

    KTextEditor::View *kv = KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView();
    if (!kv) {
        qDebug() << "Warning: no KTextEditor::View";
        return;
    }

    // ### replace this with something more sane
    // Start where the supplied XML-DTDs are fed by default unless
    // user changed directory last time:
    QString defaultDir = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("katexmltools")) + "/katexmltools/";
    if (m_urlString.isNull()) {
        m_urlString = defaultDir;
    }

    // Guess the meta DTD by looking at the doctype's public identifier.
    // XML allows comments etc. before the doctype, so look further than
    // just the first line.
    // Example syntax:
    // <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "DTD/xhtml1-transitional.dtd">
    uint checkMaxLines = 200;
    QString documentStart = kv->document()->text(KTextEditor::Range(0, 0, checkMaxLines + 1, 0));
    const QRegularExpression re(QStringLiteral("<!DOCTYPE\\s+\\b(\\w+)\\b\\s+PUBLIC\\s+[\"\']([^\"\']+?)[\"\']"), QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = re.match(documentStart);
    QString filename;
    QString doctype;
    QString topElement;

    if (match.hasMatch()) {
        topElement = match.captured(1);
        doctype = match.captured(2);
        qDebug() << "Top element: " << topElement;
        qDebug() << "Doctype match: " << doctype;
        // XHTML:
        if (doctype == QLatin1String("-//W3C//DTD XHTML 1.0 Transitional//EN")) {
            filename = QStringLiteral("xhtml1-transitional.dtd.xml");
        } else if (doctype == QLatin1String("-//W3C//DTD XHTML 1.0 Strict//EN")) {
            filename = QStringLiteral("xhtml1-strict.dtd.xml");
        } else if (doctype == QLatin1String("-//W3C//DTD XHTML 1.0 Frameset//EN")) {
            filename = QStringLiteral("xhtml1-frameset.dtd.xml");
        }
        // HTML 4.0:
        else if (doctype == QLatin1String("-//W3C//DTD HTML 4.01 Transitional//EN")) {
            filename = QStringLiteral("html4-loose.dtd.xml");
        } else if (doctype == QLatin1String("-//W3C//DTD HTML 4.01//EN")) {
            filename = QStringLiteral("html4-strict.dtd.xml");
        }
        // KDE Docbook:
        else if (doctype == QLatin1String("-//KDE//DTD DocBook XML V4.1.2-Based Variant V1.1//EN")) {
            filename = QStringLiteral("kde-docbook.dtd.xml");
        }
    } else if (documentStart.indexOf(QLatin1String("<xsl:stylesheet")) != -1
               && documentStart.indexOf(QLatin1String("xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\"")) != -1) {
        /* XSLT doesn't have a doctype/DTD. We look for an xsl:stylesheet tag instead.
          Example:
          <xsl:stylesheet version="1.0"
          xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
          xmlns="http://www.w3.org/TR/xhtml1/strict">
        */
        filename = QStringLiteral("xslt-1.0.dtd.xml");
        doctype = QStringLiteral("XSLT 1.0");
    } else {
        qDebug() << "No doctype found";
    }

    QUrl url;
    if (filename.isEmpty()) {
        // no meta dtd found for this file
        url = QFileDialog::getOpenFileUrl(KTextEditor::Editor::instance()->application()->activeMainWindow()->window(),
                                          i18n("Assign Meta DTD in XML Format"),
                                          QUrl::fromLocalFile(m_urlString),
                                          QStringLiteral("*.xml"));
    } else {
        url.setUrl(defaultDir + filename);
        KMessageBox::information(nullptr,
                                 i18n("The current file has been identified "
                                      "as a document of type \"%1\". The meta DTD for this document type "
                                      "will now be loaded.",
                                      doctype),
                                 i18n("Loading XML Meta DTD"),
                                 QStringLiteral("DTDAssigned"));
    }

    if (url.isEmpty()) {
        return;
    }

    m_urlString = url.url(); // remember directory for next time

    if (m_dtds[m_urlString]) {
        assignDTD(m_dtds[m_urlString], kv);
    } else {
        m_dtdString.clear();
        m_viewToAssignTo = kv;

        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        KIO::TransferJob *job = KIO::get(url);
        connect(job, &KIO::TransferJob::result, this, &PluginKateXMLToolsCompletionModel::slotFinished);
        connect(job, &KIO::TransferJob::data, this, &PluginKateXMLToolsCompletionModel::slotData);
    }
    qDebug() << "XMLTools::getDTD: Documents: " << m_docDtds.count() << ", DTDs: " << m_dtds.count();
}

void PluginKateXMLToolsCompletionModel::slotFinished(KJob *job)
{
    if (job->error()) {
        // qDebug() << "XML Plugin error: DTD in XML format (" << filename << " ) could not be loaded";
        static_cast<KIO::Job *>(job)->uiDelegate()->showErrorMessage();
    } else if (static_cast<KIO::TransferJob *>(job)->isErrorPage()) {
        // catch failed loading loading via http:
        KMessageBox::error(nullptr,
                           i18n("The file '%1' could not be opened. "
                                "The server returned an error.",
                                m_urlString),
                           i18n("XML Plugin Error"));
    } else {
        PseudoDTD *dtd = new PseudoDTD();
        dtd->analyzeDTD(m_urlString, m_dtdString);

        m_dtds.insert(m_urlString, dtd);
        assignDTD(dtd, m_viewToAssignTo);

        // clean up a bit
        m_viewToAssignTo = nullptr;
        m_dtdString.clear();
    }
    QGuiApplication::restoreOverrideCursor();
}

void PluginKateXMLToolsCompletionModel::slotData(KIO::Job *, const QByteArray &data)
{
    m_dtdString += QString(data);
}

void PluginKateXMLToolsCompletionModel::assignDTD(PseudoDTD *dtd, KTextEditor::View *view)
{
    m_docDtds.insert(view->document(), dtd);

    // TODO:perhaps for all views()?
    KTextEditor::CodeCompletionInterface *cci = qobject_cast<KTextEditor::CodeCompletionInterface *>(view);

    if (cci) {
        cci->registerCompletionModel(this);
        cci->setAutomaticInvocationEnabled(true);
        qDebug() << "PluginKateXMLToolsView: completion model registered";
    } else {
        qWarning() << "PluginKateXMLToolsView: completion interface unavailable";
    }
}

/**
 * Offer a line edit with completion for possible elements at cursor position and insert the
 * tag one chosen/entered by the user, plus its closing tag. If there's a text selection,
 * add the markup around it.
 */
void PluginKateXMLToolsCompletionModel::slotInsertElement()
{
    if (!KTextEditor::Editor::instance()->application()->activeMainWindow()) {
        return;
    }

    KTextEditor::View *kv = KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView();
    if (!kv) {
        qDebug() << "Warning: no KTextEditor::View";
        return;
    }

    KTextEditor::Document *doc = kv->document();
    PseudoDTD *dtd = m_docDtds[doc];
    QString parentElement = getParentElement(*kv, 0);
    QStringList allowed;

    if (dtd) {
        allowed = dtd->allowedElements(parentElement);
    }

    QString text;
    InsertElement dialog(allowed, kv);
    if (dialog.exec() == QDialog::Accepted) {
        text = dialog.text();
    }

    if (!text.isEmpty()) {
        QStringList list = text.split(QChar(' '));
        QString pre;
        QString post;
        // anders: use <tagname/> if the tag is required to be empty.
        // In that case maybe we should not remove the selection? or overwrite it?
        int adjust = 0; // how much to move cursor.
        // if we know that we have attributes, it goes
        // just after the tag name, otherwise between tags.
        if (dtd && dtd->allowedAttributes(list[0]).count()) {
            adjust++; // the ">"
        }

        if (dtd && dtd->allowedElements(list[0]).contains(QLatin1String("__EMPTY"))) {
            pre = '<' + text + "/>";
            if (adjust) {
                adjust++; // for the "/"
            }
        } else {
            pre = '<' + text + '>';
            post = "</" + list[0] + '>';
        }

        QString marked;
        if (!post.isEmpty()) {
            marked = kv->selectionText();
        }

        KTextEditor::Document::EditingTransaction transaction(doc);

        if (!marked.isEmpty()) {
            kv->removeSelectionText();
        }

        // with the old selection now removed, curPos points to the start of pre
        KTextEditor::Cursor curPos = kv->cursorPosition();
        curPos.setColumn(curPos.column() + pre.length() - adjust);

        kv->insertText(pre + marked + post);

        kv->setCursorPosition(curPos);
    }
}

/**
 * Insert a closing tag for the nearest not-closed parent element.
 */
void PluginKateXMLToolsCompletionModel::slotCloseElement()
{
    if (!KTextEditor::Editor::instance()->application()->activeMainWindow()) {
        return;
    }

    KTextEditor::View *kv = KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView();
    if (!kv) {
        qDebug() << "Warning: no KTextEditor::View";
        return;
    }
    QString parentElement = getParentElement(*kv, 0);

    // qDebug() << "parentElement: '" << parentElement << "'";
    QString closeTag = "</" + parentElement + '>';
    if (!parentElement.isEmpty()) {
        kv->insertText(closeTag);
    }
}

// modify the completion string before it gets inserted
void PluginKateXMLToolsCompletionModel::executeCompletionItem(KTextEditor::View *view, const KTextEditor::Range &word, const QModelIndex &index) const
{
    KTextEditor::Range toReplace = word;
    KTextEditor::Document *document = view->document();

    QString text = data(index.sibling(index.row(), Name), Qt::DisplayRole).toString();

    qDebug() << "executeCompletionItem text: " << text;

    int line, col;
    view->cursorPosition().position(line, col);
    QString lineStr = document->line(line);
    QString rightCh = lineStr.mid(col, 1);

    int posCorrection = 0; // where to move the cursor after completion ( >0 = move right )
    if (m_mode == entities) {
        text = text + ';';
    }

    else if (m_mode == attributes) {
        text = text + "=\"\"";
        posCorrection = -1;
        if (!rightCh.isEmpty() && rightCh != QLatin1String(">") && rightCh != QLatin1String("/") && rightCh != QLatin1String(" ")) {
            // TODO: other whitespaces
            // add space in front of the next attribute
            text = text + ' ';
            posCorrection--;
        }
    }

    else if (m_mode == attributevalues) {
        // TODO: support more than one line
        uint startAttValue = 0;
        uint endAttValue = 0;

        // find left quote:
        for (startAttValue = col; startAttValue > 0; startAttValue--) {
            QString ch = lineStr.mid(startAttValue - 1, 1);
            if (isQuote(ch)) {
                break;
            }
        }

        // find right quote:
        for (endAttValue = col; endAttValue <= static_cast<uint>(lineStr.length()); endAttValue++) {
            QString ch = lineStr.mid(endAttValue - 1, 1);
            if (isQuote(ch)) {
                break;
            }
        }

        // replace the current contents of the attribute
        if (startAttValue < endAttValue) {
            toReplace = KTextEditor::Range(line, startAttValue, line, endAttValue - 1);
        }
    }

    else if (m_mode == elements) {
        // anders: if the tag is marked EMPTY, insert in form <tagname/>
        QString str;
        bool isEmptyTag = m_docDtds[document]->allowedElements(text).contains(QLatin1String("__EMPTY"));
        if (isEmptyTag) {
            str = text + "/>";
        } else {
            str = text + "></" + text + '>';
        }

        // Place the cursor where it is most likely wanted:
        // always inside the tag if the tag is empty AND the DTD indicates that there are attribs)
        // outside for open tags, UNLESS there are mandatory attributes
        if (m_docDtds[document]->requiredAttributes(text).count() || (isEmptyTag && m_docDtds[document]->allowedAttributes(text).count())) {
            posCorrection = text.length() - str.length();
        } else if (!isEmptyTag) {
            posCorrection = text.length() - str.length() + 1;
        }

        text = str;
    }

    else if (m_mode == closingtag) {
        text += '>';
    }

    document->replaceText(toReplace, text);

    // move the cursor to desired position
    KTextEditor::Cursor curPos = view->cursorPosition();
    curPos.setColumn(curPos.column() + posCorrection);
    view->setCursorPosition(curPos);
}

// ========================================================================
// Pseudo-XML stuff:

/**
 * Check if cursor is inside a tag, that is
 * if "<" occurs before ">" occurs ( on the left side of the cursor ).
 * Return the tag name, return "" if we cursor is outside a tag.
 */
QString PluginKateXMLToolsCompletionModel::insideTag(KTextEditor::View &kv)
{
    int line, col;
    kv.cursorPosition().position(line, col);
    int y = line; // another variable because uint <-> int

    do {
        QString lineStr = kv.document()->line(y);
        for (uint x = col; x > 0; x--) {
            QString ch = lineStr.mid(x - 1, 1);
            if (ch == QLatin1String(">")) { // cursor is outside tag
                return QString();
            }

            if (ch == QLatin1String("<")) {
                QString tag;
                // look for white space on the right to get the tag name
                for (int z = x; z <= lineStr.length(); ++z) {
                    ch = lineStr.mid(z - 1, 1);
                    if (ch.at(0).isSpace() || ch == QLatin1String("/") || ch == QLatin1String(">")) {
                        return tag.right(tag.length() - 1);
                    }

                    if (z == lineStr.length()) {
                        tag += ch;
                        return tag.right(tag.length() - 1);
                    }

                    tag += ch;
                }
            }
        }
        y--;
        col = kv.document()->line(y).length();
    } while (y >= 0);

    return QString();
}

/**
 * Check if cursor is inside an attribute value, that is
 * if '="' is on the left, and if it's nearer than "<" or ">".
 *
 * @Return the attribute name or "" if we're outside an attribute
 * value.
 *
 * Note: only call when insideTag() == true.
 * TODO: allow whitespace around "="
 */
QString PluginKateXMLToolsCompletionModel::insideAttribute(KTextEditor::View &kv)
{
    int line, col;
    kv.cursorPosition().position(line, col);
    int y = line; // another variable because uint <-> int
    uint x = 0;
    QString lineStr;
    QString ch;

    do {
        lineStr = kv.document()->line(y);
        for (x = col; x > 0; x--) {
            ch = lineStr.mid(x - 1, 1);
            QString chLeft = lineStr.mid(x - 2, 1);
            // TODO: allow whitespace
            if (isQuote(ch) && chLeft == QLatin1String("=")) {
                break;
            } else if (isQuote(ch) && chLeft != QLatin1String("=")) {
                return QString();
            } else if (ch == QLatin1String("<") || ch == QLatin1String(">")) {
                return QString();
            }
        }
        y--;
        col = kv.document()->line(y).length();
    } while (!isQuote(ch));

    // look for next white space on the left to get the tag name
    QString attr;
    for (int z = x; z >= 0; z--) {
        ch = lineStr.mid(z - 1, 1);

        if (ch.at(0).isSpace()) {
            break;
        }

        if (z == 0) {
            // start of line == whitespace
            attr += ch;
            break;
        }

        attr = ch + attr;
    }

    return attr.left(attr.length() - 2);
}

/**
 * Find the parent element for the current cursor position. That is,
 * go left and find the first opening element that's not closed yet,
 * ignoring empty elements.
 * Examples: If cursor is at "X", the correct parent element is "p":
 * <p> <a x="xyz"> foo <i> test </i> bar </a> X
 * <p> <a x="xyz"> foo bar </a> X
 * <p> foo <img/> bar X
 * <p> foo bar X
 */
QString PluginKateXMLToolsCompletionModel::getParentElement(KTextEditor::View &kv, int skipCharacters)
{
    enum { parsingText, parsingElement, parsingElementBoundary, parsingNonElement, parsingAttributeDquote, parsingAttributeSquote, parsingIgnore } parseState;
    parseState = (skipCharacters > 0) ? parsingIgnore : parsingText;

    int nestingLevel = 0;

    int line, col;
    kv.cursorPosition().position(line, col);
    QString str = kv.document()->line(line);

    while (true) {
        // move left a character
        if (!col--) {
            do {
                if (!line--) {
                    return QString(); // reached start of document
                }
                str = kv.document()->line(line);
                col = str.length();
            } while (!col);
            --col;
        }

        ushort ch = str.at(col).unicode();

        switch (parseState) {
        case parsingIgnore:
            // ignore the specified number of characters
            parseState = (--skipCharacters > 0) ? parsingIgnore : parsingText;
            break;

        case parsingText:
            switch (ch) {
            case '<':
                // hmm... we were actually inside an element
                return QString();

            case '>':
                // we just hit an element boundary
                parseState = parsingElementBoundary;
                break;
            }
            break;

        case parsingElement:
            switch (ch) {
            case '"': // attribute ( double quoted )
                parseState = parsingAttributeDquote;
                break;

            case '\'': // attribute ( single quoted )
                parseState = parsingAttributeSquote;
                break;

            case '/': // close tag
                parseState = parsingNonElement;
                ++nestingLevel;
                break;

            case '<':
                // we just hit the start of the element...
                if (nestingLevel--) {
                    break;
                }

                QString tag = str.mid(col + 1);
                for (uint pos = 0, len = tag.length(); pos < len; ++pos) {
                    ch = tag.at(pos).unicode();
                    if (ch == ' ' || ch == '\t' || ch == '>') {
                        tag.truncate(pos);
                        break;
                    }
                }
                return tag;
            }
            break;

        case parsingElementBoundary:
            switch (ch) {
            case '?': // processing instruction
            case '-': // comment
            case '/': // empty element
                parseState = parsingNonElement;
                break;

            case '"':
                parseState = parsingAttributeDquote;
                break;

            case '\'':
                parseState = parsingAttributeSquote;
                break;

            case '<': // empty tag ( bad XML )
                parseState = parsingText;
                break;

            default:
                parseState = parsingElement;
            }
            break;

        case parsingAttributeDquote:
            if (ch == '"') {
                parseState = parsingElement;
            }
            break;

        case parsingAttributeSquote:
            if (ch == '\'') {
                parseState = parsingElement;
            }
            break;

        case parsingNonElement:
            if (ch == '<') {
                parseState = parsingText;
            }
            break;
        }
    }
}

/**
 * Return true if the tag is neither a closing tag
 * nor an empty tag, nor a comment, nor processing instruction.
 */
bool PluginKateXMLToolsCompletionModel::isOpeningTag(const QString &tag)
{
    return (!isClosingTag(tag) && !isEmptyTag(tag) && !tag.startsWith(QLatin1String("<?")) && !tag.startsWith(QLatin1String("<!")));
}

/**
 * Return true if the tag is a closing tag. Return false
 * if the tag is an opening tag or an empty tag ( ! )
 */
bool PluginKateXMLToolsCompletionModel::isClosingTag(const QString &tag)
{
    return (tag.startsWith(QLatin1String("</")));
}

bool PluginKateXMLToolsCompletionModel::isEmptyTag(const QString &tag)
{
    return (tag.right(2) == QLatin1String("/>"));
}

/**
 * Return true if ch is a single or double quote. Expects ch to be of length 1.
 */
bool PluginKateXMLToolsCompletionModel::isQuote(const QString &ch)
{
    return (ch == QLatin1String("\"") || ch == QLatin1String("'"));
}

// ========================================================================
// Tools:

/// Get string describing current mode
QString PluginKateXMLToolsCompletionModel::currentModeToString() const
{
    switch (m_mode) {
    case entities:
        return i18n("XML entities");
    case attributevalues:
        return i18n("XML attribute values");
    case attributes:
        return i18n("XML attributes");
    case elements:
    case closingtag:
        return i18n("XML elements");
    default:
        break;
    }
    return QString();
}

/** Sort a QStringList case-insensitively. Static. TODO: make it more simple. */
QStringList PluginKateXMLToolsCompletionModel::sortQStringList(QStringList list)
{
    // Sort list case-insensitive. This looks complicated but using a QMap
    // is even suggested by the Qt documentation.
    QMap<QString, QString> mapList;
    for (const auto &str : qAsConst(list)) {
        if (mapList.contains(str.toLower())) {
            // do not override a previous value, e.g. "Auml" and "auml" are two different
            // entities, but they should be sorted next to each other.
            // TODO: currently it's undefined if e.g. "A" or "a" comes first, it depends on
            // the meta DTD ( really? it seems to work okay?!? )
            mapList[str.toLower() + '_'] = str;
        } else {
            mapList[str.toLower()] = str;
        }
    }

    list.clear();
    QMap<QString, QString>::Iterator it;

    // Qt doc: "the items are alphabetically sorted [by key] when iterating over the map":
    for (it = mapList.begin(); it != mapList.end(); ++it) {
        list.append(it.value());
    }

    return list;
}

// BEGIN InsertElement dialog
InsertElement::InsertElement(const QStringList &completions, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Insert XML Element"));

    QVBoxLayout *topLayout = new QVBoxLayout(this);

    // label
    QString text = i18n("Enter XML tag name and attributes (\"<\", \">\" and closing tag will be supplied):");
    QLabel *label = new QLabel(text, this);
    label->setWordWrap(true);
    // combo box
    m_cmbElements = new KHistoryComboBox(this);
    static_cast<KHistoryComboBox *>(m_cmbElements)->setHistoryItems(completions, true);
    connect(m_cmbElements->lineEdit(), &QLineEdit::textChanged, this, &InsertElement::slotHistoryTextChanged);

    // button box
    QDialogButtonBox *box = new QDialogButtonBox(this);
    box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_okButton = box->button(QDialogButtonBox::Ok);
    m_okButton->setDefault(true);

    connect(box, &QDialogButtonBox::accepted, this, &InsertElement::accept);
    connect(box, &QDialogButtonBox::rejected, this, &InsertElement::reject);

    // fill layout
    topLayout->addWidget(label);
    topLayout->addWidget(m_cmbElements);
    topLayout->addWidget(box);

    m_cmbElements->setFocus();

    // make sure the ok button is enabled/disabled correctly
    slotHistoryTextChanged(m_cmbElements->lineEdit()->text());
}

InsertElement::~InsertElement()
{
}

void InsertElement::slotHistoryTextChanged(const QString &text)
{
    m_okButton->setEnabled(!text.isEmpty());
}

QString InsertElement::text() const
{
    return m_cmbElements->currentText();
}
// END InsertElement dialog

#include "plugin_katexmltools.moc"

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;

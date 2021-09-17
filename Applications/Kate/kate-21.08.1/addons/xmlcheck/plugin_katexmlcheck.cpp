/***************************************************************************
                           plugin_katexmlcheck.cpp - checks XML files using xmllint
                           -------------------
    begin                : 2002-07-06
    copyright            : (C) 2002 by Daniel Naber
    email                : daniel.naber@t-online.de
 ***************************************************************************/

/***************************************************************************
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ***************************************************************************/

/*
-fixme: show dock if "Validate XML" is selected (doesn't currently work when Kate
 was just started and the dockwidget isn't yet visible)
-fixme(?): doesn't correctly disappear when deactivated in config
*/

// TODO:
// Cleanup unneeded headers
// Find resources and translate i18n messages
// all translations were deleted in https://websvn.kde.org/?limit_changes=0&view=revision&revision=1433517
// What to do with catalogs? What is it for?
// Implement hot key shortcut to do xml validation
// Remove copyright above due to author orphaned this plugin?
// Possibility to check only well-formdness without validation
// Hide output in dock when switching to another tab
// Make ability to validate against xml schema and then edit docbook
// Should del space in [km] strang in katexmlcheck.desktop?
// Which variant should I choose? QUrl.adjusted(rm filename).path() or QUrl.toString(rm filename|rm schema)
// What about replace xmllint xmlstarlet or something?
// Maybe use QXmlReader to take dtds and xsds?

#include "plugin_katexmlcheck.h"
#include <QHBoxLayout>
//#include "plugin_katexmlcheck.moc" this goes to end

#include <KActionCollection>
#include <QApplication>
#include <QFile>
#include <QHeaderView>
#include <QInputDialog>
#include <QRegExp>
#include <QString>
#include <QTextStream>
#include <QTreeWidget>

#include <KCursor>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <QAction>
#include <QTemporaryFile>

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExp>
#include <QStandardPaths>
#include <QString>
#include <QUrl>
#include <QVBoxLayout>

#include <ktexteditor/editor.h>

#include <kxmlguifactory.h>

K_PLUGIN_FACTORY_WITH_JSON(PluginKateXMLCheckFactory, "katexmlcheck.json", registerPlugin<PluginKateXMLCheck>();)

PluginKateXMLCheck::PluginKateXMLCheck(QObject *const parent, const QVariantList &)
    : KTextEditor::Plugin(parent)
{
    qDebug() << "PluginXmlCheck()";
}

PluginKateXMLCheck::~PluginKateXMLCheck()
{
}

QObject *PluginKateXMLCheck::createView(KTextEditor::MainWindow *mainWindow)
{
    return new PluginKateXMLCheckView(this, mainWindow);
}

//---------------------------------
PluginKateXMLCheckView::PluginKateXMLCheckView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainwin)
    : QObject(mainwin)
    , m_mainWindow(mainwin)
{
    KXMLGUIClient::setComponentName(QStringLiteral("katexmlcheck"), i18n("Kate XML check")); // where i18n resources?
    setXMLFile(QStringLiteral("ui.rc"));

    dock = m_mainWindow->createToolView(plugin,
                                        QStringLiteral("kate_plugin_xmlcheck_ouputview"),
                                        KTextEditor::MainWindow::Bottom,
                                        QIcon::fromTheme(QStringLiteral("misc")),
                                        i18n("XML Checker Output"));
    listview = new QTreeWidget(dock);
    m_tmp_file = nullptr;
    QAction *a = actionCollection()->addAction(QStringLiteral("xml_check"));
    a->setText(i18n("Validate XML"));
    connect(a, &QAction::triggered, this, &PluginKateXMLCheckView::slotValidate);
    // TODO?:
    //(void)  new KAction ( i18n("Indent XML"), KShortcut(), this,
    //	SLOT(slotIndent()), actionCollection(), "xml_indent" );

    listview->setFocusPolicy(Qt::NoFocus);
    QStringList headers;
    headers << i18n("#");
    headers << i18n("Line");
    headers << i18n("Column");
    headers << i18n("Message");
    listview->setHeaderLabels(headers);
    listview->setRootIsDecorated(false);
    connect(listview, &QTreeWidget::itemClicked, this, &PluginKateXMLCheckView::slotClicked);

    QHeaderView *header = listview->header();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    /* TODO?: invalidate the listview when document has changed
       Kate::View *kv = application()->activeMainWindow()->activeView();
       if( ! kv ) {
       qDebug() << "Warning: no Kate::View";
       return;
       }
       connect(kv, SIGNAL(modifiedChanged()), this, SLOT(slotUpdate()));
    */

    connect(&m_proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &PluginKateXMLCheckView::slotProcExited);
    // we currently only want errors:
    m_proc.setProcessChannelMode(QProcess::SeparateChannels);
    // m_proc.setProcessChannelMode(QProcess::ForwardedChannels); // For Debugging. Do not use this.
    mainwin->guiFactory()->addClient(this);
}

PluginKateXMLCheckView::~PluginKateXMLCheckView()
{
    m_mainWindow->guiFactory()->removeClient(this);
    delete m_tmp_file;
    delete dock;
}

void PluginKateXMLCheckView::slotProcExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);

    // FIXME: doesn't work correct the first time:
    // if( m_dockwidget->isDockBackPossible() ) {
    //	m_dockwidget->dockBack();
    //	}

    if (exitStatus != QProcess::NormalExit) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, QStringLiteral("1").rightJustified(4, ' '));
        item->setText(3, QStringLiteral("Validate process crashed."));
        listview->addTopLevelItem(item);
        return;
    }

    qDebug() << "slotProcExited()";
    QApplication::restoreOverrideCursor();
    delete m_tmp_file;
    QString proc_stderr = QString::fromLocal8Bit(m_proc.readAllStandardError());
    m_tmp_file = nullptr;
    listview->clear();
    uint list_count = 0;
    uint err_count = 0;
    if (!m_validating) {
        // no i18n here, so we don't get an ugly English<->Non-english mixup:
        QString msg;
        if (m_dtdname.isEmpty()) {
            msg = QStringLiteral("No DOCTYPE found, will only check well-formedness.");
        } else {
            msg = '\'' + m_dtdname + "' not found, will only check well-formedness.";
        }
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, QStringLiteral("1").rightJustified(4, ' '));
        item->setText(3, msg);
        listview->addTopLevelItem(item);
        list_count++;
    }
    if (!proc_stderr.isEmpty()) {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        QStringList lines = proc_stderr.split('\n', QString::SkipEmptyParts);
#else
        QStringList lines = proc_stderr.split('\n', Qt::SkipEmptyParts);
#endif
        QString linenumber, msg;
        int line_count = 0;
        for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it) {
            QString line = *it;
            line_count++;
            int semicolon_1 = line.indexOf(':');
            int semicolon_2 = line.indexOf(':', semicolon_1 + 1);
            int semicolon_3 = line.indexOf(':', semicolon_2 + 2);
            int caret_pos = line.indexOf('^');
            if (semicolon_1 != -1 && semicolon_2 != -1 && semicolon_3 != -1) {
                linenumber = line.mid(semicolon_1 + 1, semicolon_2 - semicolon_1 - 1).trimmed();
                linenumber = linenumber.rightJustified(6, ' '); // for sorting numbers
                msg = line.mid(semicolon_3 + 1, line.length() - semicolon_3 - 1).trimmed();
            } else if (caret_pos != -1 || line_count == lines.size()) {
                // TODO: this fails if "^" occurs in the real text?!
                if (line_count == lines.size() && caret_pos == -1) {
                    msg = msg + '\n' + line;
                }
                QString col = QString::number(caret_pos);
                if (col == QLatin1String("-1")) {
                    col = QLatin1String("");
                }
                err_count++;
                list_count++;
                QTreeWidgetItem *item = new QTreeWidgetItem();
                item->setText(0, QString::number(list_count).rightJustified(4, ' '));
                item->setText(1, linenumber);
                item->setTextAlignment(1, (item->textAlignment(1) & ~Qt::AlignHorizontal_Mask) | Qt::AlignRight);
                item->setText(2, col);
                item->setTextAlignment(2, (item->textAlignment(2) & ~Qt::AlignHorizontal_Mask) | Qt::AlignRight);
                item->setText(3, msg);
                listview->addTopLevelItem(item);
            } else {
                msg = msg + '\n' + line;
            }
        }
    }
    if (err_count == 0) {
        QString msg;
        if (m_validating) {
            msg = QStringLiteral("No errors found, document is valid."); // no i18n here
        } else {
            msg = QStringLiteral("No errors found, document is well-formed."); // no i18n here
        }
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, QString::number(list_count + 1).rightJustified(4, ' '));
        item->setText(3, msg);
        listview->addTopLevelItem(item);
    }
}

void PluginKateXMLCheckView::slotClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    qDebug() << "slotClicked";
    if (item) {
        bool ok = true;
        uint line = item->text(1).toUInt(&ok);
        bool ok2 = true;
        uint column = item->text(2).toUInt(&ok);
        if (ok && ok2) {
            KTextEditor::View *kv = m_mainWindow->activeView();
            if (!kv) {
                return;
            }

            kv->setCursorPosition(KTextEditor::Cursor(line - 1, column));
        }
    }
}

void PluginKateXMLCheckView::slotUpdate()
{
    qDebug() << "slotUpdate() (not implemented yet)";
}

bool PluginKateXMLCheckView::slotValidate()
{
    qDebug() << "slotValidate()";

    m_mainWindow->showToolView(dock);
    m_validating = false;
    m_dtdname = QLatin1String("");

    KTextEditor::View *kv = m_mainWindow->activeView();
    if (!kv) {
        return false;
    }
    delete m_tmp_file;
    m_tmp_file = new QTemporaryFile();
    if (!m_tmp_file->open()) {
        qDebug() << "Error (slotValidate()): could not create '" << m_tmp_file->fileName() << "': " << m_tmp_file->errorString();
        KMessageBox::error(nullptr,
                           i18n("<b>Error:</b> Could not create "
                                "temporary file '%1'.",
                                m_tmp_file->fileName()));
        delete m_tmp_file;
        m_tmp_file = nullptr;
        return false;
    }

    QTextStream s(m_tmp_file);
    s << kv->document()->text();
    s.flush();

    QString exe = QStandardPaths::findExecutable(QStringLiteral("xmllint"));
    if (exe.isEmpty()) {
        exe = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, QStringLiteral("xmllint"));
    }
    // qDebug() << "exe=" <<exe;
    // 	// use catalogs for KDE docbook:
    // 	if( ! getenv("XML_CATALOG_FILES") ) {
    // 		KComponentData ins("katexmlcheckplugin");
    // 		QString catalogs;
    // 		catalogs += ins.dirs()->findResource("data", "ksgmltools2/customization/catalog.xml");
    // 		qDebug() << "catalogs: " << catalogs;
    // 		setenv("XML_CATALOG_FILES", QFile::encodeName( catalogs ).data(), 1);
    // 	}
    // qDebug() << "**catalogs: " << getenv("XML_CATALOG_FILES");
    QStringList args;
    args << QStringLiteral("--noout");

    // tell xmllint the working path of the document's file, if possible.
    // otherwise it will not find relative DTDs

    // I should give path to location of file, but remove filename
    // I can make QUrl.adjusted(rm filename).path()
    // or QUrl.toString(rm filename|rm schema)
    // Result is the same. Which variant should I choose?
    // QString path = kv->document()->url().adjusted(QUrl::RemoveFilename).path();
    // xmllint uses space- or colon-separated path option, so spaces should be encoded to %20. It is done with EncodeSpaces.

    // Now what about colons in file names or paths?
    // This way xmllint works normally:
    // xmllint --noout --path "/home/user/my/with:colon/" --valid "/home/user/my/with:colon/demo-1.xml"
    // but because this plugin makes temp file path to file is another and this way xmllint refuses to find dtd:
    // xmllint --noout --path "/home/user/my/with:colon/" --valid "/tmp/kate.X23725"
    // As workaround we can encode ':' with %3A
    QString path = kv->document()->url().toString(QUrl::RemoveFilename | QUrl::PreferLocalFile | QUrl::EncodeSpaces);
    path.replace(':', QLatin1String("%3A"));
    // because of such inconvenience with xmllint and paths, maybe switch to xmlstarlet?

    qDebug() << "path=" << path;

    if (!path.isEmpty()) {
        args << QStringLiteral("--path") << path;
    }

    // heuristic: assume that the doctype is in the first 10,000 bytes:
    QString text_start = kv->document()->text().left(10000);
    // remove comments before looking for doctype (as a doctype might be commented out
    // and needs to be ignored then):
    QRegExp re("<!--.*-->");
    re.setMinimal(true);
    text_start.remove(re);
    QRegExp re_doctype("<!DOCTYPE\\s+(.*)\\s+(?:PUBLIC\\s+[\"'].*[\"']\\s+[\"'](.*)[\"']|SYSTEM\\s+[\"'](.*)[\"'])", Qt::CaseInsensitive);
    re_doctype.setMinimal(true);

    if (re_doctype.indexIn(text_start) != -1) {
        QString dtdname;
        if (!re_doctype.cap(2).isEmpty()) {
            dtdname = re_doctype.cap(2);
        } else {
            dtdname = re_doctype.cap(3);
        }
        if (!dtdname.startsWith(QLatin1String("http:"))) { // todo: u_dtd.isLocalFile() doesn't work :-(
            // a local DTD is used
            m_validating = true;
            args << QStringLiteral("--valid");
        } else {
            m_validating = true;
            args << QStringLiteral("--valid");
        }
    } else if (text_start.indexOf(QLatin1String("<!DOCTYPE")) != -1) {
        // DTD is inside the XML file
        m_validating = true;
        args << QStringLiteral("--valid");
    }
    args << m_tmp_file->fileName();
    qDebug() << "m_tmp_file->fileName()=" << m_tmp_file->fileName();

    m_proc.start(exe, args);
    qDebug() << "m_proc.program():" << m_proc.program(); // I want to see parameters
    qDebug() << "args=" << args;
    qDebug() << "exit code:" << m_proc.exitCode();
    if (!m_proc.waitForStarted(-1)) {
        KMessageBox::error(nullptr,
                           i18n("<b>Error:</b> Failed to execute xmllint. Please make "
                                "sure that xmllint is installed. It is part of libxml2."));
        return false;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    return true;
}

#include "plugin_katexmlcheck.moc"

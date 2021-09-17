/***************************************************************************
   pluginKatexmltools.cpp
   copyright            : (C) 2001-2002 by Daniel Naber
   email                : daniel.naber@t-online.de
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

#ifndef PLUGIN_KATEXMLTOOLS_H
#define PLUGIN_KATEXMLTOOLS_H

#include "pseudo_dtd.h"

#include <ktexteditor/application.h>
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/plugin.h>
#include <ktexteditor/view.h>

#include <QString>
#include <QVariantList>

class QComboBox;
class QPushButton;

class PluginKateXMLTools : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit PluginKateXMLTools(QObject *parent = nullptr, const QVariantList & = QVariantList());
    ~PluginKateXMLTools() override;
    QObject *createView(KTextEditor::MainWindow *mainWindow) override;
};

class PluginKateXMLToolsCompletionModel : public KTextEditor::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)

public:
    PluginKateXMLToolsCompletionModel(QObject *parent);
    ~PluginKateXMLToolsCompletionModel() override;

    //
    // KTextEditor::CodeCompletionModel
    //
public:
    int columnCount(const QModelIndex &) const override;
    int rowCount(const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &idx, int role) const override;

    void executeCompletionItem(KTextEditor::View *view, const KTextEditor::Range &word, const QModelIndex &index) const override;

    //
    // KTextEditor::CodeCompletionModelControllerInterface
    //
public:
    bool shouldStartCompletion(KTextEditor::View *view, const QString &insertedText, bool userInsertion, const KTextEditor::Cursor &position) override;

public Q_SLOTS:

    void getDTD();

    void slotInsertElement();
    void slotCloseElement();

    void slotFinished(KJob *job);
    void slotData(KIO::Job *, const QByteArray &data);

    void completionInvoked(KTextEditor::View *kv, const KTextEditor::Range &range, InvocationType invocationType) override;

    /// Connected to the document manager, to manage the dtd collection.
    void slotDocumentDeleted(KTextEditor::Document *doc);

protected:
    QString currentModeToString() const;
    static QStringList sortQStringList(QStringList list);
    // bool eventFilter( QObject *object, QEvent *event );

    QString insideTag(KTextEditor::View &kv);
    QString insideAttribute(KTextEditor::View &kv);

    static bool isOpeningTag(const QString &tag);
    static bool isClosingTag(const QString &tag);
    static bool isEmptyTag(const QString &tag);
    static bool isQuote(const QString &ch);

    QString getParentElement(KTextEditor::View &view, int skipCharacters);

    enum Mode { none, entities, attributevalues, attributes, elements, closingtag };
    enum PopupMode { noPopup, tagname, attributename, attributevalue, entityname };

    enum Level { groupNode = 1 };

    /// Assign the PseudoDTD @p dtd to the Kate::View @p view
    void assignDTD(PseudoDTD *dtd, KTextEditor::View *view);

    /// temporary placeholder for the metaDTD file
    QString m_dtdString;
    /// temporary placeholder for the view to assign a DTD to while the file is loaded
    KTextEditor::View *m_viewToAssignTo;
    /// URL of the last loaded meta DTD
    QString m_urlString;

    QStringList m_allowed;

    Mode m_mode;
    int m_correctPos;

    // code completion stuff:
    KTextEditor::CodeCompletionInterface *m_codeInterface = nullptr;

    /// maps KTE::Document -> DTD
    QHash<KTextEditor::Document *, PseudoDTD *> m_docDtds;

    /// maps DTD filename -> DTD
    QHash<QString, PseudoDTD *> m_dtds;
};

class PluginKateXMLToolsView : public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    explicit PluginKateXMLToolsView(KTextEditor::MainWindow *mainWin);
    ~PluginKateXMLToolsView() override;

protected:
    KTextEditor::MainWindow *m_mainWindow;
    PluginKateXMLToolsCompletionModel m_model;
};

class InsertElement : public QDialog
{
    Q_OBJECT

public:
    InsertElement(const QStringList &completions, QWidget *parent);
    ~InsertElement() override;

    QString text() const;

private Q_SLOTS:
    void slotHistoryTextChanged(const QString &);

private:
    QComboBox *m_cmbElements;
    QPushButton *m_okButton;
};

#endif // PLUGIN_KATEXMLTOOLS_H

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;

/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2004 Anders Lund <anders@alweb.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KATE_MW_MODONHD_DIALOG_H_
#define _KATE_MW_MODONHD_DIALOG_H_

#include <ktexteditor/document.h>

#include <QDialog>
#include <QVector>

class KProcess;
class QTemporaryFile;
class QTreeWidget;
class QTreeWidgetItem;

typedef QVector<KTextEditor::Document *> DocVector;

/**
 * A dialog for handling multiple documents modified on disk
 * from within KateMainWindow
 */
class KateMwModOnHdDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KateMwModOnHdDialog(DocVector docs, QWidget *parent = nullptr, const char *name = nullptr);
    ~KateMwModOnHdDialog() override;
    void addDocument(KTextEditor::Document *doc);

Q_SIGNALS:
    void requestOpenDiffDocument(const QUrl &documentUrl);

private Q_SLOTS:
    void slotIgnore();
    void slotOverwrite();
    void slotReload();
    void slotDiff();
    void slotSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *);
    void slotCheckedFilesChanged(QTreeWidgetItem *, int column);
    void slotDataAvailable();
    void slotPDone();

private:
    enum Action { Ignore, Overwrite, Reload };
    void handleSelected(int action);
    class QTreeWidget *twDocuments;
    class QDialogButtonBox *dlgButtons;
    class QPushButton *btnDiff;
    KProcess *m_proc;
    QTemporaryFile *m_diffFile;
    QStringList m_stateTexts;
    bool m_blockAddDocument;

protected:
    void closeEvent(QCloseEvent *e) override;
    void keyPressEvent(QKeyEvent *) override;
};

#endif // _KATE_MW_MODONHD_DIALOG_H_

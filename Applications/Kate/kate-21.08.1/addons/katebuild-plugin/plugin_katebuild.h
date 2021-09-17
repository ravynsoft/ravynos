#ifndef PLUGIN_KATEBUILD_H
#define PLUGIN_KATEBUILD_H
/* plugin_katebuild.h                    Kate Plugin
**
** SPDX-FileCopyrightText: 2008-2015 Kåre Särs <kare.sars@iki.fi>
**
** This code is almost a total rewrite of the GPL'ed Make plugin
** by Adriaan de Groot.
*/

/*
** SPDX-License-Identifier: GPL-2.0-or-later
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

#include <KProcess>
#include <QHash>
#include <QPointer>
#include <QRegularExpression>
#include <QStack>
#include <QString>

#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/MarkInterface>
#include <KTextEditor/Message>
#include <KTextEditor/Plugin>
#include <KTextEditor/SessionConfigInterface>
#include <KTextEditor/View>

#include <KConfigGroup>
#include <KXMLGUIClient>

#include "targets.h"
#include "ui_build.h"

/******************************************************************/
class KateBuildView : public QObject, public KXMLGUIClient, public KTextEditor::SessionConfigInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::SessionConfigInterface)
    Q_PROPERTY(QUrl docUrl READ docUrl)

public:
    enum ResultDetails { FullOutput, ParsedOutput, ErrorsAndWarnings, OnlyErrors };

    enum TreeWidgetRoles { ErrorRole = Qt::UserRole + 1, DataRole };

    enum ErrorCategory { CategoryInfo, CategoryWarning, CategoryError };

    KateBuildView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mw);
    ~KateBuildView() override;

    // reimplemented: read and write session config
    void readSessionConfig(const KConfigGroup &config) override;
    void writeSessionConfig(KConfigGroup &config) override;

    bool buildCurrentTarget();

    QUrl docUrl();

private Q_SLOTS:

    // Building
    void slotSelectTarget();
    void slotBuildActiveTarget();
    void slotBuildPreviousTarget();
    void slotBuildDefaultTarget();
    bool slotStop();

    // Parse output
    void slotProcExited(int exitCode, QProcess::ExitStatus exitStatus);
    void slotReadReadyStdErr();
    void slotReadReadyStdOut();

    // Selecting warnings/errors
    void slotNext();
    void slotPrev();
    void slotErrorSelected(QTreeWidgetItem *item);

    // Settings
    void targetSetNew();
    void targetOrSetCopy();
    void targetDelete();

    void slotAddTargetClicked();

    void slotDisplayMode(int mode);

    void handleEsc(QEvent *e);

    void slotViewChanged();
    void slotDisplayOption();
    void slotMarkClicked(KTextEditor::Document *doc, KTextEditor::Mark mark, bool &handled);
    void slotInvalidateMoving(KTextEditor::Document *doc);
    /**
     * keep track if the project plugin is alive and if the project map did change
     */
    void slotPluginViewCreated(const QString &name, QObject *pluginView);
    void slotPluginViewDeleted(const QString &name, QObject *pluginView);
    void slotProjectMapChanged();
    void slotAddProjectTarget();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
#ifdef Q_OS_WIN
    QString caseFixed(const QString &path);
#endif
    void processLine(const QString &);
    void addError(const QString &filename, const QString &line, const QString &column, const QString &message);
    bool startProcess(const QString &dir, const QString &command);
    bool checkLocal(const QUrl &dir);
    void clearBuildResults();

    void displayBuildResult(const QString &message, KTextEditor::Message::MessageType level);
    void displayMessage(const QString &message, KTextEditor::Message::MessageType level);

    void clearMarks();
    void addMarks(KTextEditor::Document *doc, bool mark);

    KTextEditor::MainWindow *m_win;
    QWidget *m_toolView;
    Ui::build m_buildUi{};
    QWidget *m_buildWidget;
    int m_outputWidgetWidth;
    TargetsUi *m_targetsUi;
    KProcess m_proc;
    QString m_stdOut;
    QString m_stdErr;
    QString m_currentlyBuildingTarget;
    bool m_buildCancelled;
    int m_displayModeBeforeBuild;
    QString m_make_dir;
    QStack<QString> m_make_dir_stack;
    QStringList m_searchPaths;
    QRegularExpression m_filenameDetector;
    bool m_ninjaBuildDetected = false;
    QRegularExpression m_newDirDetector;
    unsigned int m_numErrors = 0;
    unsigned int m_numWarnings = 0;
    QString m_prevItemContent;
    QModelIndex m_previousIndex;
    QPointer<KTextEditor::Message> m_infoMessage;
    QPointer<QAction> m_showMarks;
    QHash<KTextEditor::Document *, QPointer<KTextEditor::Document>> m_markedDocs;

    /**
     * current project plugin view, if any
     */
    QObject *m_projectPluginView = nullptr;
};

typedef QList<QVariant> VariantList;

/******************************************************************/
class KateBuildPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit KateBuildPlugin(QObject *parent = nullptr, const VariantList & = VariantList());
    ~KateBuildPlugin() override
    {
    }

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;
};

#endif

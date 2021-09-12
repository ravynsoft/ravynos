%Module QTermWidget

%ModuleHeaderCode
#pragma GCC visibility push(default)
%End

%Import QtGui/QtGuimod.sip
%Import QtCore/QtCoremod.sip
%Import QtWidgets/QtWidgetsmod.sip

class QTermWidget : QWidget {

%TypeHeaderCode
#include <../lib/qtermwidget.h>
%End

public:
    enum ScrollBarPosition
    {
        NoScrollBar=0,
        ScrollBarLeft=1,
        ScrollBarRight=2
    };

    enum class KeyboardCursorShape
    {
        BlockCursor=0,
        UnderlineCursor=1,
        IBeamCursor=2
    };

    QTermWidget(int startnow = 1, QWidget *parent = 0);
    ~QTermWidget();
    void startTerminalTeletype();
    QSize sizeHint() const;
    void setTerminalSizeHint(bool on);
    bool terminalSizeHint();
    void startShellProgram();
    int getShellPID();
    void changeDir(const QString & dir);
    void setTerminalFont(QFont &font);
    QFont getTerminalFont();
    void setTerminalOpacity(qreal level);
    void setEnvironment(const QStringList & environment);
    void setShellProgram(const QString & progname);
    void setWorkingDirectory(const QString & dir);
    QString workingDirectory();
    void setArgs(QStringList & args);
    void setTextCodec(QTextCodec *codec);
    void setColorScheme(const QString & name);
    static QStringList availableColorSchemes();
    static void addCustomColorSchemeDir(const QString& custom_dir);
    void setHistorySize(int lines);
    void setScrollBarPosition(ScrollBarPosition);
    void scrollToEnd();
    void sendText(QString &text);
    void setFlowControlEnabled(bool enabled);
    bool flowControlEnabled();
    void setFlowControlWarningEnabled(bool enabled);
    static QStringList availableKeyBindings();
    QString keyBindings();
    void setMotionAfterPasting(int);
    int historyLinesCount();
    int screenColumnsCount();
    void setSelectionStart(int row, int column);
    void setSelectionEnd(int row, int column);
    void getSelectionStart(int& row, int& column);
    void getSelectionEnd(int& row, int& column);
    QString selectedText(bool preserveLineBreaks = true);
    void setMonitorActivity(bool);
    void setMonitorSilence(bool);
    void setSilenceTimeout(int seconds);
    int getPtySlaveFd() const;
    void setKeyboardCursorShape(KeyboardCursorShape shape);
    void setAutoClose(bool);
    QString title() const;
    QString icon() const;
signals:
    void finished();
    void copyAvailable(bool);
    void termGetFocus();
    void termLostFocus();
    void termKeyPressed(QKeyEvent *);
    void urlActivated(const QUrl&, bool fromContextMenu);
    void bell(const QString& message);
    void activity();
    void silence();
    void sendData(const char *,int);
    void titleChanged();
    void receivedData(const QString &text);
    void profileChanged(const QString & profile);
public slots:
    void copyClipboard();
    void pasteClipboard();
    void pasteSelection();
    void zoomIn();
    void zoomOut();
    void setSize(const QSize &);
    void setKeyBindings(const QString & kb);
    void clear();
    void toggleShowSearchBar();
protected:
    virtual void resizeEvent(QResizeEvent *);
protected slots:
    void sessionFinished();
    void selectionChanged(bool textSelected);
private:
    void search(bool forwards, bool next);
    void setZoom(int step);
    void init(int startnow);
private slots:
    void find();
    void findNext();
    void findPrevious();
    void matchFound(int startColumn, int startLine, int endColumn, int endLine);
    void noMatchFound();
};

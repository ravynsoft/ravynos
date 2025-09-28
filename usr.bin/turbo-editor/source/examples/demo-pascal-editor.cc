#define Uses_TApplication
#define Uses_TButton
#define Uses_TDeskTop
#define Uses_TDialog
#define Uses_TFrame
#define Uses_TKeys
#define Uses_TListViewer
#define Uses_TMenuBar
#define Uses_TScrollBar
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TStatusLine
#define Uses_TSubMenu
#include <tvision/tv.h>
#include <turbo/turbo.h>

#include <forward_list>
#include <string>

// This sample application demonstrates how to use Turbo as a code editor
// for a specific language, using a custom window class, a custom color scheme
// and a TListViewer to list the open editors.

enum : ushort
{
    // Commands 0..99 and  256..999 are reserved by Turbo Vision.
    cmEditorFocused = 1000,
    cmNewFile,
    cmOpenFile,
    cmSaveFile,
    cmSaveFileAs,
    cmRenameFile,
    cmCloseFile,
};

class DemoEditorListView;
class DemoPascalFileEditor;

// The application class, which defines a custom menu bar and status line and
// inserts a DemoEditorWindow on the desktop.

class DemoApplication : public TApplication
{
public:

    DemoApplication() noexcept;

private:

    static TMenuBar *initMenuBar(TRect r) noexcept;
    static TStatusLine *initStatusLine(TRect r) noexcept;
};

// The main window, which contains an editor view and a list of editors.
// This class must implement the 'turbo::EditorParent' interface so that it
// can receive notifications from the editor.

class DemoEditorWindow : public TDialog, public turbo::EditorParent
{
public:

    DemoEditorWindow(const TRect &bounds) noexcept;

private:

    enum { listWidth = 20 };

    // These views are used to display the editor's contents.
    turbo::EditorView *editorView;
    turbo::LeftMarginView *leftMargin;
    TScrollBar *hScrollBar, *vScrollBar;

    // This view shows the list of open editors.
    DemoEditorListView *listView;
    std::forward_list<DemoPascalFileEditor> editors;

    // Storage for the title text.
    std::string title;

    // TDialog overriden methods.
    void shutDown() override;
    void handleEvent(TEvent &ev) override;
    Boolean valid(ushort command) override;
    void dragView(TEvent& event, uchar mode, TRect& limits, TPoint minSize, TPoint maxSize) override;
    const char *getTitle(short) override;

    // turbo::EditorParent overriden methods.
    void handleNotification(const SCNotification &, turbo::Editor &) override;

    // Internal methods.
    void addEditor(turbo::TScintilla &, const char *filePath);
    void removeEditor(DemoPascalFileEditor &aEditor);
    bool closeAllEditors();

    // Constructor helpers.
    static TFrame *initFrame(TRect bounds) noexcept;
    auto makeEditorViewBounds() noexcept;
    auto makeListViewBounds(const TRect &editorViewBounds) noexcept;
    auto createEditorView(const TRect &editorViewBounds) noexcept;
    auto createLeftMarginView() noexcept;
    auto createHScrollBar(const TRect &viewBounds) noexcept;
    auto createVScrollBar(const TRect &viewBounds) noexcept;
    auto createListView( const TRect &listViewBounds,
                         TScrollBar *listHScrollBar,
                         TScrollBar *listVScrollBar ) noexcept;
};

// Our custom editor, which assumes that all files are in the Pascal language.

class DemoPascalFileEditor : public turbo::FileEditor
{
public:

    DemoPascalFileEditor(turbo::TScintilla &aScintilla, std::string aFilePath) noexcept;

    void onFilePathSet() noexcept override;
};

// A view that shows the open editors and which can be used to select the
// active editor.

class DemoEditorListView : public TListViewer
{
public:

    DemoEditorListView( const TRect& bounds, TScrollBar *aHScrollBar,
                        TScrollBar *aVScrollBar,
                        std::forward_list<DemoPascalFileEditor> &aList ) noexcept;

    void getText(char *dest, short item, short maxLen) override;
    void focusItemNum(short item) override;
    void setRange(short aRange);

private:

    std::forward_list<DemoPascalFileEditor> &list;

    int maxWidth();
};

/////////////////////////////////////////////////////////////////////////
// DemoApplication implementation

int main()
{
    DemoApplication app;
    app.run();
    app.shutDown();
}

DemoApplication::DemoApplication() noexcept :
    TProgInit(&DemoApplication::initStatusLine,
              &DemoApplication::initMenuBar,
              &TApplication::initDeskTop)
{
    insertWindow(
        new DemoEditorWindow(
            deskTop->getExtent().grow(-2, -2)
        )
    );
}


TMenuBar *DemoApplication::initMenuBar(TRect r) noexcept
{
    r.b.y = r.a.y+1;
    return new TMenuBar( r,
        *new TSubMenu( "~F~ile", kbAltF, hcNoContext ) +
            *new TMenuItem( "~N~ew", cmNewFile, kbCtrlN, hcNoContext, "Ctrl-N" ) +
            *new TMenuItem( "~O~pen", cmOpenFile, kbCtrlO, hcNoContext, "Ctrl-O" ) +
            newLine() +
            *new TMenuItem( "~S~ave", cmSaveFile, kbCtrlS, hcNoContext, "Ctrl-S" ) +
            *new TMenuItem( "S~a~ve As...", cmSaveFileAs, kbNoKey, hcNoContext ) +
            *new TMenuItem( "~R~ename...", cmRenameFile, kbF2, hcNoContext, "F2" ) +
            newLine() +
            *new TMenuItem( "~C~lose", cmCloseFile, kbCtrlW, hcNoContext, "Ctrl-W" ) +
            newLine() +
            *new TMenuItem( "S~u~spend", cmDosShell, kbNoKey, hcNoContext ) +
            *new TMenuItem( "E~x~it", cmQuit, kbAltX, hcNoContext, "Alt-X" )
            );
}

TStatusLine *DemoApplication::initStatusLine(TRect r) noexcept
{
    r.a.y = r.b.y-1;
    return new TStatusLine( r,
        *new TStatusDef( 0, 0xFFFF ) +
            *new TStatusItem( 0, kbAltX, cmQuit ) +
            *new TStatusItem( "~Ctrl-N~ New", kbNoKey, cmNewFile ) +
            *new TStatusItem( "~Ctrl-O~ Open", kbNoKey, cmOpenFile ) +
            *new TStatusItem( "~Ctrl-S~ Save", kbNoKey, cmSaveFile ) +
            *new TStatusItem( "~F12~ Menu", kbF12, cmMenu ) +
            *new TStatusItem( 0, kbCtrlX, cmCut ) +
            *new TStatusItem( 0, kbCtrlC, cmCopy ) +
            *new TStatusItem( 0, kbCtrlV, cmPaste ) +
            *new TStatusItem( 0, kbShiftDel, cmCut ) +
            *new TStatusItem( 0, kbCtrlIns, cmCopy ) +
            *new TStatusItem( 0, kbShiftIns, cmPaste ) +
            *new TStatusItem( 0, kbCtrlF5, cmResize )
            );
}

/////////////////////////////////////////////////////////////////////////
// DemoEditorWindow implementation

TFrame* DemoEditorWindow::initFrame(TRect bounds) noexcept
{
    // Turbo's BasicEditorFrame will automatically display the cursor position
    // indicator.
    return new turbo::BasicEditorFrame(bounds);
}

auto DemoEditorWindow::makeEditorViewBounds() noexcept
{
    TRect r = getExtent().grow(-1, -1);
    r.b.x -= listWidth + 1;
    return r;
}

auto DemoEditorWindow::makeListViewBounds(const TRect &editorViewBounds) noexcept
{
    TRect r = editorViewBounds;
    r.a.x = r.b.x + 1;
    r.b.x += listWidth - 1;
    r.move(1, 0);
    return r;
}

auto DemoEditorWindow::createEditorView(const TRect &editorViewBounds) noexcept
{
    auto *editorView = new turbo::EditorView(editorViewBounds);
    editorView->options |= ofFramed;
    return editorView;
}

auto DemoEditorWindow::createLeftMarginView() noexcept
{
    auto *leftMargin = new turbo::LeftMarginView(1);
    leftMargin->options |= ofFramed;
    return leftMargin;
}

auto DemoEditorWindow::createHScrollBar(const TRect &viewBounds) noexcept
{
    TRect r = viewBounds;
    r.a.y = r.b.y;
    r.b.y += 1;
    return new TScrollBar(r);
}

auto DemoEditorWindow::createVScrollBar(const TRect &viewBounds) noexcept
{
    TRect r = viewBounds;
    r.a.x = r.b.x;
    r.b.x += 1;
    return new TScrollBar(r);
}

auto DemoEditorWindow::createListView( const TRect &listViewBounds,
                                       TScrollBar *listHScrollBar,
                                       TScrollBar *listVScrollBar ) noexcept
{
    auto *listView = new DemoEditorListView( listViewBounds, listHScrollBar,
                                             listVScrollBar, editors );
    listHScrollBar->growMode |= gfGrowLoX;
    listView->growMode = gfGrowLoX | gfGrowHiX | gfGrowHiY;
    return listView;
}

DemoEditorWindow::DemoEditorWindow(const TRect &bounds) noexcept :
    TWindowInit(&initFrame),
    TDialog(bounds, nullptr)
{
    // In this example we are only using a single window that can't be closed.
    flags = wfMove | wfGrow | wfZoom;
    // Use the blue dialog palette.
    palette = dpBlueDialog;

    TRect editorViewBounds = makeEditorViewBounds();
    editorView = createEditorView(editorViewBounds);
    leftMargin = createLeftMarginView();
    hScrollBar = createHScrollBar(editorViewBounds);
    vScrollBar = createVScrollBar(editorViewBounds);
    // Make room for the indicator on the bottom left corner of the frame.
    hScrollBar->size.x -= turbo::BasicEditorFrame::indicatorWidth;
    hScrollBar->origin.x += turbo::BasicEditorFrame::indicatorWidth;

    TRect listViewBounds = makeListViewBounds(editorViewBounds);
    auto *listHScrollBar = createHScrollBar(listViewBounds);
    auto *listVScrollBar = createVScrollBar(listViewBounds);
    listView = createListView(listViewBounds, listHScrollBar, listVScrollBar);

    // The insertion order matters. We want the views related to the editor
    // to be inserted in last place, so that they get precedence in receiving
    // focus and events. Among these, the editor view must be inserted the
    // first.
    insert(listView);
    insert(listHScrollBar);
    insert(listVScrollBar);
    insert(editorView);
    insert(leftMargin);
    insert(hScrollBar);
    insert(vScrollBar);
}

void DemoEditorWindow::shutDown()
{
    if (editorView && editorView->editor)
        editorView->editor->disassociate();
    editorView = nullptr;
    leftMargin = nullptr;
    hScrollBar = nullptr;
    vScrollBar = nullptr;
    listView = nullptr;
    TDialog::shutDown();
}

void DemoEditorWindow::handleEvent(TEvent &ev)
{
    if (ev.what == evCommand && editorView)
    {
        auto *editor = (DemoPascalFileEditor *) editorView->editor;
        switch (ev.message.command)
        {
            case cmEditorFocused:
            {
                auto *newEditor = (DemoPascalFileEditor *) ev.message.infoPtr;
                if (newEditor)
                {
                    newEditor->associate(this, editorView, leftMargin, hScrollBar, vScrollBar);
                    newEditor->redraw();
                }
                else
                {
                    editorView->drawView();
                    if (frame)
                        frame->drawView();
                }
                clearEvent(ev);
                break;
            }
            case cmNewFile:
                addEditor(turbo::createScintilla(), "");
                clearEvent(ev);
                break;
            case cmOpenFile:
                turbo::openFile(
                    turbo::createScintilla,
                    [&] (turbo::TScintilla &scintilla, const char *path) {
                        addEditor(scintilla, path);
                    }
                );
                clearEvent(ev);
                break;
            case cmSaveFile:
                if (editor)
                    editor->save();
                clearEvent(ev);
                break;
            case cmSaveFileAs:
                if (editor)
                    editor->saveAs();
                clearEvent(ev);
                break;
            case cmRenameFile:
                if (editor)
                    editor->rename();
                clearEvent(ev);
                break;
            case cmCloseFile:
                if (editor && editor->close())
                    removeEditor(*editor);
                clearEvent(ev);
                break;
        }
    }
    TDialog::handleEvent(ev);
}

Boolean DemoEditorWindow::valid(ushort command)
{
    if (TDialog::valid(command))
        switch (command)
        {
            case cmQuit:
                return closeAllEditors();
            default:
                return true;
        }
    return false;
}

void DemoEditorWindow::dragView(TEvent& event, uchar mode, TRect& limits, TPoint minSize, TPoint maxSize)
{
    if (editorView && editorView->editor)
    {
        // Do not reflow the editor's contents while resizing the window.
        editorView->editor->lockReflow([&] {
            TDialog::dragView(event, mode, limits, minSize, maxSize);
        });
        // Then redraw the editor once finished.
        editorView->editor->redraw();
    }
    else
        TDialog::dragView(event, mode, limits, minSize, maxSize);
}

const char *DemoEditorWindow::getTitle(short)
{
    if (editorView && editorView->editor)
    {
        // Recalculate the window title.
        auto &editor = *(DemoPascalFileEditor *) editorView->editor;
        auto name = TPath::basename(editor.filePath);
        if (name.empty())
            name = "Untitled";
        bool dirty = !editor.inSavePoint();
        title.clear();
        title.append(name);
        title.append(dirty ? "*" : "");
        return title.c_str();
    }
    return "Pascal Editor";
}

void DemoEditorWindow::handleNotification(const SCNotification &scn, turbo::Editor &editor)
{
    switch (scn.nmhdr.code)
    {
        case SCN_PAINTED:
            // The frame is sensible to the cursor position and the save point state.
            if (frame)
                frame->drawView();
            break;
        case SCN_SAVEPOINTREACHED:
            editor.redraw();
            listView->drawView();
            break;
    }
}

void DemoEditorWindow::addEditor(turbo::TScintilla &scintilla, const char *filePath)
{
    // Create the instance of DemoPascalFileEditor.
    editors.emplace_front(scintilla, filePath);
    listView->setRange(listView->range + 1);
    // This will trigger editorView->editor->associate.
    listView->focusItemNum(0);
    listView->drawView();
}

void DemoEditorWindow::removeEditor(DemoPascalFileEditor &editor)
// Pre: 'editor' belongs to 'editors'.
{
    editors.remove_if([&] (const auto &aEditor) {
        if (&aEditor == &editor)
            return editor.disassociate(), true;
        return false;
    });
    listView->setRange(listView->range - 1);
    listView->focusItemNum(listView->focused);
    listView->drawView();
}

bool DemoEditorWindow::closeAllEditors()
{
    if (editorView)
        while (editorView->editor)
        {
            auto &editor = *(DemoPascalFileEditor *) editorView->editor;
            if (editor.close())
                removeEditor(editor);
            else
                return false;
        }
    return true;
}

/////////////////////////////////////////////////////////////////////////
// DemoPascalFileEditor implementation

// A custom color scheme for the editor.
extern constexpr turbo::ColorScheme demoScheme =
{
                         // { Foreground | Background | Style  }
    /* sNormal           */ { '\xE'      , '\x1'               },
    /* sSelection        */ { '\x1'      , '\x7'               },
    /* sWhitespace       */ { '\xC'      , {}                  },
    /* sCtrlChar         */ { '\xD'      , {}                  },
    /* sLineNums         */ { '\xF'      , {}                  },
    /* sKeyword1         */ { '\xA'      , {}                  },
    /* sKeyword2         */ { '\xA'      , {}                  },
    /* sMisc             */ { '\xA'      , {}                  },
    /* sPreprocessor     */ { '\xA'      , {}                  },
    /* sOperator         */ { '\xF'      , {}                  },
    /* sComment          */ { '\x7'      , {}                  },
    /* sStringLiteral    */ { '\xB'      , {}                  },
    /* sCharLiteral      */ { '\xB'      , {}                  },
    /* sNumberLiteral    */ { '\xB'      , {}                  },
    /* sEscapeSequence   */ { '\xB'      , {}                  },
    /* sError            */ { '\x0'      , '\x4'               },
    /* sBraceMatch       */ { '\xC'      , {}         , slBold },
    /* sReplaceHighlight */ { '\x0'      , '\xA'               },
};

DemoPascalFileEditor::DemoPascalFileEditor( turbo::TScintilla &aScintilla,
                                            std::string aFilePath ) noexcept :
    turbo::FileEditor(aScintilla, std::move(aFilePath))
{
    scheme = &demoScheme;
    onFilePathSet();
    lineNumbers.setState(true);
}

void DemoPascalFileEditor::onFilePathSet() noexcept
{
    // Specify the language manually.
    language = &turbo::Language::Pascal;
    lexer = turbo::findBuiltInLexer(language);
    turbo::applyTheming(lexer, scheme, scintilla);
}

/////////////////////////////////////////////////////////////////////////
// DemoEditorListView implementation

DemoEditorListView::DemoEditorListView( const TRect& bounds, TScrollBar *aHScrollBar,
                                TScrollBar *aVScrollBar, std::forward_list<DemoPascalFileEditor> &aList ) noexcept :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar),
    list(aList)
{
}

void DemoEditorListView::getText(char *dest, short item, short maxChars)
{
    short i = 0;
    for (auto &editor : list)
        if (i++ == item)
        {
            if (!editor.filePath.empty())
                strnzcpy(dest, editor.filePath, maxChars + 1);
            else
                strnzcpy(dest, "Untitled", maxChars + 1);
            return;
        }
    snprintf(dest, maxChars, "<ERROR: out-of-bounds index %hd>", item);
}

void DemoEditorListView::focusItemNum(short item)
{
    TListViewer::focusItemNum(item);
    short i = 0;
    for (auto &editor : list)
        if (i++ == focused)
        {
            message(owner, evCommand, cmEditorFocused, &editor);
            return;
        }
    message(owner, evCommand, cmEditorFocused, nullptr);
}

void DemoEditorListView::setRange(short aRange)
{
    TListViewer::setRange(aRange);
    if (hScrollBar != 0)
        hScrollBar->setRange(0, maxWidth() - size.x + 2);
}

int DemoEditorListView::maxWidth()
{
    char text[256];
    int width = 0;
    for (short i = 0; i < range; ++i)
    {
        getText(text, i, sizeof(text) - 1);
        width = max(strwidth(text), width);
    }
    return width;
}

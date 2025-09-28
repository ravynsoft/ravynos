#define Uses_TApplication
#define Uses_TDialog
#define Uses_TDeskTop
#define Uses_TButton
#define Uses_TFrame
#define Uses_TScrollBar
#define Uses_TListViewer
#include <tvision/tv.h>
#include <turbo/turbo.h>

#include <forward_list>
#include <string>

// This sample application demonstrates how to use Turbo in a custom
// window class (instead of turbo::BasicEditorWindow), using a TListViewer
// to list the open editors.

enum : ushort
{
    cmToggleLineNumbers = 1000,
    cmToggleLineWrapping,
    cmEditorFocused,
    cmNewFile,
    cmOpenFile,
    cmSaveFile,
    cmSaveFileAs,
    cmRenameFile,
    cmCloseFile,
};

class DemoEditorListView;
using EditorList = std::forward_list<turbo::FileEditor>;

class DemoApplication : public TApplication
{
public:

    DemoApplication() noexcept;
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

    // These views are used to show the editor's contents.
    turbo::EditorView *editorView;
    turbo::LeftMarginView *leftMargin;
    TScrollBar *hScrollBar, *vScrollBar;

    // This view shows the list of open editors.
    DemoEditorListView *listView;
    EditorList editors;

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
    void removeEditor(turbo::FileEditor &aEditor);
    bool closeAllEditors();

    // Constructor helpers.
    auto makeEditorViewBounds() noexcept;
    auto makeListViewBounds(const TRect &editorViewBounds) noexcept;
    auto createEditorView(const TRect &editorViewBounds) noexcept;
    auto createLeftMarginView() noexcept;
    auto createHScrollBar(const TRect &viewBounds) noexcept;
    auto createVScrollBar(const TRect &viewBounds) noexcept;
    auto createListView( const TRect &listViewBounds,
                         TScrollBar *listHScrollBar,
                         TScrollBar *listVScrollBar ) noexcept;
    auto initButtons(const TRect &editorViewBounds) noexcept;
};

// A view that shows the open editors and which can be used to select the
// active editor.

class DemoEditorListView : public TListViewer
{
public:

    DemoEditorListView( const TRect& bounds, TScrollBar *aHScrollBar,
                        TScrollBar *aVScrollBar, EditorList &aList ) noexcept;

    void getText(char *dest, short item, short maxLen) override;
    void focusItemNum(short item) override;
    void setRange(short aRange);

private:

    EditorList &list;

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
    TProgInit(&TApplication::initStatusLine,
              &TApplication::initMenuBar,
              &TApplication::initDeskTop)
{
    insertWindow(
        new DemoEditorWindow(
            deskTop->getExtent().grow(-2, -2)
        )
    );
}

/////////////////////////////////////////////////////////////////////////
// DemoEditorWindow implementation


auto DemoEditorWindow::makeEditorViewBounds() noexcept
{
    TRect r = getExtent().grow(-2, -4).move(0, -2);
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
    return new turbo::EditorView(editorViewBounds);
}

auto DemoEditorWindow::createLeftMarginView() noexcept
{
    return new turbo::LeftMarginView(1);
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

auto DemoEditorWindow::initButtons(const TRect &editorViewBounds) noexcept
{
    static const struct { ushort command; TStringView text; } buttonDefinitions[] =
    {
        {cmToggleLineNumbers, "Line Numbers"},
        {cmToggleLineWrapping, "Line Wrapping"},
        {cmNewFile, "New File"},
        {cmOpenFile, "Open File"},
        {cmSaveFile, "Save"},
        {cmSaveFileAs, "Save As"},
        {cmRenameFile, "Rename"},
        {cmCloseFile, "Close"},
    };

    TPoint butOrigin = {editorViewBounds.a.x, editorViewBounds.b.y + 2};
    for (const auto &b : buttonDefinitions)
    {
        TRect r = TRect(butOrigin, butOrigin + TPoint {cstrlen(b.text) + 4, 2});
        auto *button = new TButton(r, b.text, b.command, bfNormal);
        button->growMode = gfGrowLoY | gfGrowHiY;
        insert(button);
        butOrigin = {r.b.x + 1, r.a.y};
    }
}

DemoEditorWindow::DemoEditorWindow(const TRect &bounds) noexcept :
    TWindowInit(&TDialog::initFrame),
    TDialog(bounds, nullptr)
{
    flags |= wfGrow;

    TRect editorViewBounds = makeEditorViewBounds();
    editorView = createEditorView(editorViewBounds);
    leftMargin = createLeftMarginView();
    hScrollBar = createHScrollBar(editorViewBounds);
    vScrollBar = createVScrollBar(editorViewBounds);
    // Of all the views related to the editor, the editor view must be inserted
    // in the first place.
    insert(editorView);
    insert(leftMargin);
    insert(hScrollBar);
    insert(vScrollBar);

    initButtons(editorViewBounds);

    TRect listViewBounds = makeListViewBounds(editorViewBounds);
    auto *listHScrollBar = createHScrollBar(listViewBounds);
    auto *listVScrollBar = createVScrollBar(listViewBounds);
    listView = createListView(listViewBounds, listHScrollBar, listVScrollBar);
    insert(listView);
    insert(listHScrollBar);
    insert(listVScrollBar);
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
        auto *editor = (turbo::FileEditor *) editorView->editor;
        switch (ev.message.command)
        {
            case cmToggleLineNumbers:
                if (editor)
                {
                    editor->lineNumbers.toggle();
                    editor->redraw();
                    clearEvent(ev);
                }
                break;
            case cmToggleLineWrapping:
                if (editor)
                {
                    editor->wrapping.toggle(editor->scintilla);
                    editor->redraw();
                    clearEvent(ev);
                }
                break;
            case cmEditorFocused:
            {
                auto *editor = (turbo::FileEditor *) ev.message.infoPtr;
                if (editor)
                {
                    editor->associate(this, editorView, leftMargin, hScrollBar, vScrollBar);
                    editor->redraw();
                }
                else
                {
                    editorView->drawView();
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
            {
                turbo::openFile(
                    turbo::createScintilla,
                    [&] (turbo::TScintilla &scintilla, const char *path) {
                        addEditor(scintilla, path);
                    }
                );
                clearEvent(ev);
                break;
            }
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
            case cmClose:
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
        auto lastSize = size;
        editorView->editor->lockReflow([&] {
            TDialog::dragView(event, mode, limits, minSize, maxSize);
        });
        if (lastSize != size)
            editorView->editor->redraw(); // Redraw without reflow lock.
    }
    else
        TDialog::dragView(event, mode, limits, minSize, maxSize);
}

const char *DemoEditorWindow::getTitle(short)
{
    if (editorView && editorView->editor)
    {
        auto &editor = *(turbo::FileEditor *) editorView->editor;
        auto name = TPath::basename(editor.filePath);
        if (name.empty())
            name = "Untitled";
        bool dirty = !editor.inSavePoint();
        title.clear();
        title.append(name);
        title.append(dirty ? "*" : "");
        return title.c_str();
    }
    return nullptr;
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
    // Create the instance of turbo::FileEditor.
    editors.emplace_front(scintilla, filePath);
    listView->setRange(listView->range + 1);
    // This will trigger editorView->editor->associate.
    listView->focusItemNum(0);
    listView->drawView();
}

void DemoEditorWindow::removeEditor(turbo::FileEditor &editor)
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
            auto &editor = *(turbo::FileEditor *) editorView->editor;
            if (editor.close())
                removeEditor(editor);
            else
                return false;
        }
    return true;
}

/////////////////////////////////////////////////////////////////////////
// DemoEditorListView implementation

DemoEditorListView::DemoEditorListView( const TRect& bounds, TScrollBar *aHScrollBar,
                                TScrollBar *aVScrollBar, EditorList &aList ) noexcept :
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

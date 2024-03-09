#define Uses_TApplication
#define Uses_TDialog
#define Uses_TScrollBar
#include <tvision/tv.h>

#include <turbo/basicwindow.h>
#include <turbo/basicframe.h>
#include <iostream>

namespace turbo {

TFrame* BasicEditorWindow::initFrame(TRect bounds)
{
    return new BasicEditorFrame(bounds);
}

BasicEditorWindow::BasicEditorWindow(const TRect &bounds, Editor &aEditor) :
    TWindowInit(&initFrame),
    TWindow(bounds, nullptr, wnNoNumber),
    editor(aEditor)
{
    options |= ofTileable | ofFirstClick;
    setState(sfShadow, False);

    auto *editorView = new EditorView(TRect(1, 1, size.x - 1, size.y - 1));
    insert(editorView);

    auto *leftMargin = new LeftMarginView(leftMarginSep);
    leftMargin->options |= ofFramed;
    insert(leftMargin);

    auto *hScrollBar = new TScrollBar(TRect(18, size.y - 1, size.x - 2, size.y));
    hScrollBar->hide();
    insert(hScrollBar);

    auto *vScrollBar = new TScrollBar(TRect(size.x - 1, 1, size.x, size.y - 1));
    vScrollBar->hide();
    insert(vScrollBar);

    editor.associate(this, editorView, leftMargin, hScrollBar, vScrollBar);
}

BasicEditorWindow::~BasicEditorWindow()
{
    delete &editor;
}

void BasicEditorWindow::shutDown()
{
    editor.disassociate();
    TWindow::shutDown();
}

void BasicEditorWindow::setState(ushort aState, Boolean enable)
{
    TWindow::setState(aState, enable);
    if (aState == sfActive && editor.parent == this)
    {
        editor.hScrollBar->setState(sfVisible, enable);
        editor.vScrollBar->setState(sfVisible, enable);
    }
}

void BasicEditorWindow::dragView(TEvent& event, uchar mode, TRect& limits, TPoint minSize, TPoint maxSize)
{
    auto lastSize = size;
    editor.lockReflow([&] {
        TWindow::dragView(event, mode, limits, minSize, maxSize);
    });
    if (lastSize != size)
        editor.redraw(); // Redraw without reflow lock.
}

void BasicEditorWindow::sizeLimits(TPoint &min, TPoint &max)
{
    TView::sizeLimits(min, max);
    min = minSize;
}

TColorAttr BasicEditorWindow::mapColor(uchar index) noexcept
{
    if (0 < index && index - 1 < WindowPaletteItemCount)
        return getScheme()[index - 1];
    return errorAttr;
}

void BasicEditorWindow::handleNotification(const SCNotification &scn, Editor &editor)
{
    switch (scn.nmhdr.code)
    {
        case SCN_PAINTED:
            if (!(state & sfDragging) && frame) // It already gets drawn when resizing.
                frame->drawView(); // The frame is sensible to the cursor position and the save point state.
            break;
    }
}

#define dialogColor(i) cpAppColor[(uchar) (cpDialog[i] - 1)]

extern constexpr WindowColorScheme windowSchemeDefault =
{
    /* wndFramePassive             */ '\x07',
    /* wndFrameActive              */ '\x0F',
    /* wndFrameIcon                */ '\x0A',
    /* wndScrollBarPageArea        */ '\x30',
    /* wndScrollBarControls        */ '\x30',
    /* wndStaticText               */ '\x0F',
    /* wndLabelNormal              */ '\x07',
    /* wndLabelSelected            */ '\x0F',
    /* wndLabelShortcut            */ '\x06',
    /* wndButtonNormal             */ '\x20',
    /* wndButtonDefault            */ '\x2B',
    /* wndButtonSelected           */ '\x2F',
    /* wndButtonDisabled           */ '\x78',
    /* wndButtonShortcut           */ '\x2E',
    /* wndButtonShadow             */ '\x08',
    /* wndClusterNormal            */ '\x07',
    /* wndClusterSelected          */ '\x0F',
    /* wndClusterShortcut          */ '\x06',
    dialogColor(wndInputLineNormal         ),
    dialogColor(wndInputLineSelected       ),
    dialogColor(wndInputLineArrows         ),
    /* wndHistoryArrow             */ '\x20',
    /* wndHistorySides             */ '\x02',
    dialogColor(wndHistWinScrollBarPageArea),
    dialogColor(wndHistWinScrollBarControls),
    dialogColor(wndListViewerNormal        ),
    dialogColor(wndListViewerFocused       ),
    dialogColor(wndListViewerSelected      ),
    dialogColor(wndListViewerDivider       ),
    dialogColor(wndInfoPane                ),
    /* wndClusterDisabled          */ '\x08',
};

#undef dialogColor

} // namespace turbo

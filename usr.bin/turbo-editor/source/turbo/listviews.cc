#define Uses_TKeys
#define Uses_TEvent
#define Uses_TScrollBar
#include <tvision/tv.h>

#include "listviews.h"

size_t maxWidth(const ListModel &model) noexcept
{
    size_t width = 0, elems = model.size();
    for (size_t i = 0; i < elems; ++i)
    {
        size_t w = cstrlen(model.getText(model.at(i)));
        if (w > width)
            width = w;
    }
    return width;
}

#define cpListWindow "\x32\x32\x34\x37\x36\x32\x33"

const TPoint ListWindow::minSize = {5, 3};

ListWindow::ListWindow( const TRect &bounds, const char *aTitle, const ListModel &aModel,
                        TFuncView<ListView *(TRect, TWindow *, const ListModel &)> createListViewer ) :
    TWindowInit(&initFrame),
    TWindow(bounds, aTitle, wnNoNumber)
{
    flags = wfClose | wfMove;
    viewer = createListViewer(getExtent(), this, aModel);
    viewer->growMode = gfGrowHiX | gfGrowHiY;
    insert(viewer);
}

void ListWindow::shutDown()
{
    viewer = nullptr;
    TWindow::shutDown();
}

TPalette &ListWindow::getPalette() const
{
    static TPalette palette(cpListWindow, sizeof(cpListWindow) - 1);
    return palette;
}

void* ListWindow::getCurrent() const noexcept
{
    if (viewer)
        return viewer->getCurrent();
    return nullptr;
}

short ListWindow::getCurrentIndex() const noexcept
{
    if (viewer)
        return viewer->focused;
    return 0;
}

void ListWindow::setCurrentIndex(short i) noexcept
{
    if (viewer)
        viewer->focusItemNum(i);
}

void ListWindow::handleEvent(TEvent& event)
{
    TWindow::handleEvent(event);
    if (event.what == evMouseDown && !mouseInView(event.mouse.where))
    {
        endModal(cmCancel);
        clearEvent(event);
    }
}

void ListWindow::sizeLimits(TPoint &min, TPoint &max)
{
    TView::sizeLimits(min, max);
    min = minSize;
}

#define cpListViewer "\x06\x06\x07\x06\x06"

ListView::ListView( const TRect& bounds, TScrollBar *aHScrollBar,
                    TScrollBar *aVScrollBar, const ListModel &aModel,
                    ushort aFlags ) :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar),
    model(aModel),
    flags(aFlags)
{
    setRange(model.size());
    if (range > 1)
        focusItem(1);
    if (hScrollBar)
        hScrollBar->setRange(0, maxWidth(model) - size.x + 3);
}

TPalette &ListView::getPalette() const
{
    static TPalette palette(cpListViewer, sizeof(cpListViewer) - 1);
    return palette;
}

void ListView::getText(char *dest, short item, short maxChars)
{
    TStringView text = model.getText(model.at(item));
    strnzcpy(dest, text, maxChars + 1);
}

void *ListView::getCurrent() noexcept
{
    return model.at(focused);
}

void ListView::handleEvent(TEvent &event)
{
    if (event.what == evMouseDown)
    {
        int mouseAutosToSkip = 4;
        int newItem = focused;
        int oldItem = focused;
        int count = 0;
        do
        {
            TPoint mouse = makeLocal(event.mouse.where);
            if (range <= size.y || (0 <= mouse.y && mouse.y < size.y))
                newItem = mouse.y + topItem;
            else
            {
                if (event.what == evMouseAuto)
                    count++;
                if (count == mouseAutosToSkip)
                {
                    count = 0;
                    if (mouse.y < 0)
                        newItem = focused - 1;
                    else if (mouse.y >= size.y)
                        newItem = focused + 1;
                }
            }
            if (newItem != oldItem)
            {
                focusItemNum(newItem);
                drawView();
            }
            oldItem = newItem;
            if (event.mouse.eventFlags & meDoubleClick)
                break;
        }
        while (mouseEvent(event, evMouseMove | evMouseAuto));
        focusItemNum(newItem);
        drawView();
        if ( ((event.mouse.eventFlags & meDoubleClick) || (flags & lvSelectSingleClick)) &&
             0 <= newItem && newItem < range )
            endModal(cmOK);
        clearEvent(event);
    }
    else if (event.what == evKeyDown && event.keyDown.keyCode == kbEnter)
    {
        endModal(cmOK);
        clearEvent(event);
    }
    else if ( (event.what == evKeyDown && event.keyDown.keyCode == kbEsc) ||
              (event.what == evCommand && event.message.command == cmCancel) )
    {
        endModal(cmCancel);
        clearEvent(event);
    }
    else
        TListViewer::handleEvent(event);
}

#include "cmds.h"

static int mod(int a, int b)
{
    int m = a % b;
    if (m < 0)
        return b < 0 ? m - b : m + b;
    return m;
}

void EditorListView::handleEvent(TEvent &ev)
{
    if (ev.what == evCommand && ev.message.command == cmEditorNext)
    {
        focusItemNum(mod(focused + 1, range));
        clearEvent(ev);
    }
    else if (ev.what == evCommand && ev.message.command == cmEditorPrev)
    {
        focusItemNum(mod(focused - 1, range));
        clearEvent(ev);
    }
    else
        ListView::handleEvent(ev);

}

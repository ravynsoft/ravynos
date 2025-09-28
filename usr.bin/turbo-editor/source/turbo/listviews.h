#ifndef TURBO_LISTVIEWS_H
#define TURBO_LISTVIEWS_H

#define Uses_TWindow
#define Uses_TPalette
#define Uses_TListViewer
#include <tvision/tv.h>

#include <turbo/funcview.h>

class ListModel
{
public:
    virtual size_t size() const noexcept = 0;
    virtual void *at(size_t i) const noexcept = 0;
    virtual TStringView getText(void *) const noexcept = 0;
};

size_t maxWidth(const ListModel &model) noexcept;

class ListView;

enum ListViewFlags : uint8_t
{
    lvNoScrollBars = 0x01,
    lvSelectSingleClick = 0x02,
};

template <class Viewer>
class ListViewCreator
{
    ushort listViewFlags;

public:

    ListViewCreator(ushort aListViewFlags = 0) noexcept :
        listViewFlags(aListViewFlags)
    {
    }

    ListView *operator()(TRect, TWindow *, const ListModel &) noexcept;
};

class ListWindow : public TWindow
{
    ListView *viewer;

public:

    static const TPoint minSize;

    // The lifetime of 'aList' must exceed that of 'this'.
    // The lifetime of 'createListViewer' must exceed that of the constructor invocation.
    ListWindow( const TRect &bounds, const char *title, const ListModel &model,
                TFuncView<ListView *(TRect, TWindow *, const ListModel &)> createListViewer = ListViewCreator<ListView>() );

    void shutDown() override;
    void handleEvent(TEvent& event) override;
    TPalette &getPalette() const override;
    void sizeLimits(TPoint &min, TPoint &max) override;

    void *getCurrent() const noexcept;
    short getCurrentIndex() const noexcept;
    void setCurrentIndex(short i) noexcept;
};

struct MouseEventType;

class ListView : public TListViewer
{
    const ListModel &model;
    ushort flags;

public:

    // The lifetime of 'aList' must exceed that of 'this'.
    ListView( const TRect& bounds, TScrollBar *hScrollBar, TScrollBar *vScrollBar,
              const ListModel &model, ushort flags );

    void *getCurrent() noexcept;

    void getText(char *dest, short item, short maxLen) override;
    void handleEvent(TEvent& ev) override;

    TPalette &getPalette() const override;
};

template <class Viewer>
inline ListView *ListViewCreator<Viewer>::operator()(TRect r, TWindow *win, const ListModel &model) noexcept
{
    r.grow(-1, -1);
    TScrollBar *hScrollBar = nullptr, *vScrollBar = nullptr;
    if (!(listViewFlags & lvNoScrollBars))
    {
        hScrollBar = win->standardScrollBar(sbHorizontal | sbHandleKeyboard);
        vScrollBar = win->standardScrollBar(sbVertical | sbHandleKeyboard);
    }
    auto *viewer = new Viewer(r, hScrollBar, vScrollBar, model, listViewFlags);
    return static_cast<ListView *>(viewer);
}

class EditorListView : public ListView
{
public:
    using ListView::ListView;

    void handleEvent(TEvent &ev) override;
};

#include "apputils.h"
#include "editwindow.h"

class EditorListModel : public ListModel
{
    mutable list_head_iterator<EditorWindow> list;

public:

    EditorListModel(list_head<EditorWindow> &aList) :
        list(&aList)
    {
    }

    size_t size() const noexcept override
    {
        return list.size();
    }

    void *at(size_t i) const noexcept override
    {
        return list.at(i)->self;
    }

    TStringView getText(void *item) const noexcept override
    {
        if (auto *wnd = (EditorWindow *) item)
            return wnd->title;
        return "";
    }
};

template <class T>
struct SpanListModelEntry
{
    T data;
    TStringView text;
};

template <class T>
class SpanListModel : public ListModel
{
    TSpan<const SpanListModelEntry<T>> list;

public:

    // The lifetime of 'aList' must exceed that of 'this'.
    constexpr SpanListModel(TSpan<const SpanListModelEntry<T>> aList) noexcept :
        list(aList)
    {
    }

    size_t size() const noexcept override
    {
        return list.size();
    }

    void *at(size_t i) const noexcept override
    {
        return (void *) &list[i];
    }

    TStringView getText(void *item) const noexcept override
    {
        if (auto *entry = (const SpanListModelEntry<T> *) item)
            return entry->text;
        return "";
    }
};

#endif // TURBO_LISTVIEWS_H

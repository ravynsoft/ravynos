#ifndef TURBO_BASICWINDOW_H
#define TURBO_BASICWINDOW_H

#define Uses_TWindow
#include <tvision/tv.h>
#include <turbo/editor.h>

namespace turbo {

enum WindowPaletteItems : uchar
{
    // The following are the TDialog palette items, so that you can insert
    // all these kinds of views on a BasicEditorWindow.
    wndFramePassive,
    wndFrameActive,
    wndFrameIcon,
    wndScrollBarPageArea,
    wndScrollBarControls,
    wndStaticText,
    wndLabelNormal,
    wndLabelSelected,
    wndLabelShortcut,
    wndButtonNormal,
    wndButtonDefault,
    wndButtonSelected,
    wndButtonDisabled,
    wndButtonShortcut,
    wndButtonShadow,
    wndClusterNormal,
    wndClusterSelected,
    wndClusterShortcut,
    wndInputLineNormal,
    wndInputLineSelected,
    wndInputLineArrows,
    wndHistoryArrow,
    wndHistorySides,
    wndHistWinScrollBarPageArea,
    wndHistWinScrollBarControls,
    wndListViewerNormal,
    wndListViewerFocused,
    wndListViewerSelected,
    wndListViewerDivider,
    wndInfoPane,
    wndClusterDisabled,
    WindowPaletteItemCount,
};

using WindowColorScheme = TColorAttr[WindowPaletteItemCount];

extern const WindowColorScheme windowSchemeDefault;

class BasicEditorWindow : public TWindow, public EditorParent
{

    const WindowColorScheme *scheme {nullptr};

public:

    static constexpr int leftMarginSep {1};
    static constexpr TPoint minSize {24, 6};

    Editor &editor;

    static TFrame* initFrame(TRect bounds);

    // Takes ownership over 'aEditor'.
    // Assumes 'TWindow::frame' (the result of 'initFrame') to be a 'BasicEditorFrame'.
    BasicEditorWindow(const TRect &bounds, Editor &aEditor);
    ~BasicEditorWindow();

    void shutDown() override;
    void setState(ushort aState, Boolean enable) override;
    void dragView(TEvent& event, uchar mode, TRect& limits, TPoint minSize, TPoint maxSize) override;
    void sizeLimits(TPoint &min, TPoint &max) override;
    TColorAttr mapColor(uchar index) noexcept override;

    void handleNotification(const SCNotification &scn, turbo::Editor &) override;

    // Sets the color scheme, but the changes won't be visible until the
    // subviews are redrawn (e.g. via 'TGroup::redraw()').
    // * 'aScheme': non-owning. Lifetime must exceed that of 'this'.
    inline void setScheme(const WindowColorScheme *aScheme);
    inline const WindowColorScheme &getScheme() const;

};

inline void BasicEditorWindow::setScheme(const WindowColorScheme *aScheme)
{
    scheme = aScheme;
}

inline const WindowColorScheme &BasicEditorWindow::getScheme() const
{
    return scheme ? *scheme : windowSchemeDefault;
}

} // namespace turbo

#endif // TURBO_BASICWINDOW_H

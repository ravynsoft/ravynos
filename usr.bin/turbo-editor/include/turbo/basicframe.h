#ifndef TURBO_BASICFRAME_H
#define TURBO_BASICFRAME_H

#define Uses_TFrame
#include <tvision/tv.h>

namespace turbo {

class Editor;

// Window frame with a cursor position indicator in the bottom left border.
// For the indicator to be shown and updated, an 'EditorView' must be inserted
// in the same window, and 'frame->drawView()' must be invoked manually when
// the editor changes (e.g. when it emits a SCN_PAINTED notification).

class BasicEditorFrame : public TFrame
{
public:

    enum { indicatorWidth = 16 };

    BasicEditorFrame(const TRect &bounds) noexcept;
    void draw() override;

private:

    void drawIndicator(Editor &);
};

} // namespace turbo

#endif // TURBO_BASICFRAME_H

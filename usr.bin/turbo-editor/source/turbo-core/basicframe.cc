#define Uses_TRect
#define Uses_TGroup
#include <tvision/tv.h>

#include <turbo/basicframe.h>
#include <turbo/editor.h>
#include "icmds.h"

namespace turbo {

BasicEditorFrame::BasicEditorFrame(const TRect &bounds) noexcept :
    TFrame(bounds)
{
}

void BasicEditorFrame::draw()
{
    TFrame::draw();
    if (auto *editor = (Editor *) internalMessage(owner, InternalCommands::cmGetEditor))
        drawIndicator(*editor);
}

static TPoint cursorPos(Editor &editor)
{
    auto pos = editor.callScintilla(SCI_GETCURRENTPOS, 0U, 0U);
    return {
        (int) editor.callScintilla(SCI_GETCOLUMN, pos, 0U),
        (int) editor.callScintilla(SCI_LINEFROMPOSITION, pos, 0U),
    };
}

void BasicEditorFrame::drawIndicator(Editor &editor)
{
    // └─ XXXXXXXXXXXXXX
    //   ^2              ^18
    TRect r(2, size.y - 1, min(size.x - 2, 2 + indicatorWidth), size.y);
    if ( (state & sfActive) &&
         0 <= r.a.x && r.a.x < r.b.x && 0 <= r.a.y && r.a.y < r.b.y )
    {
        TDrawBuffer b;
        char s[64];
        auto color = mapColor((state & sfDragging) ? 5 : 4);
        TPoint pos = cursorPos(editor);
        sprintf(s, " %d:%d ", pos.y + 1, pos.x + 1);
        ushort maxWidth = r.b.x - r.a.x;
        ushort width = b.moveStr(0, s, color, maxWidth);
        ushort left = 5;
        ushort offs = max(left - max(width - (maxWidth - (left + 1)), 0), 0);
        writeLine(r.a.x + offs, r.a.y, min(width, maxWidth), 1, b);
    }
}

} // namespace turbo

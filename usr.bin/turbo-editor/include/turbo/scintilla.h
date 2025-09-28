#ifndef TURBO_SCINTILLA_H
#define TURBO_SCINTILLA_H

#include "scintilla/include/Scintilla.h"
#include "scintilla/include/SciLexer.h"

class TPoint;
class TRect;
class TStringView;
class TDrawSurface;
struct TColorAttr;
struct KeyDownEvent;
struct MouseEventType;

namespace Scintilla {
class TScintilla;
} // namespace Scintilla

namespace turbo {

using TScintilla = Scintilla::TScintilla;
class TScintillaParent;

// The definition of 'TScintilla' is hidden from the public headers just
// like other Scintilla-related projects also do. Note that this saves you
// from including >= 60K lines of code.
// Should you need to interact with 'TScintilla' directly, the 'createScintilla',
// 'destroyScintilla' and 'call' functions below should be enough.

// Returns a heap-allocated instance of 'TScintilla'.
TScintilla &createScintilla() noexcept;
// Destroys a heap-allocated instance of 'TScintilla'.
void destroyScintilla(TScintilla &) noexcept;
// Invokes 'TScintilla::WndProc'. See 'https://www.scintilla.org/ScintillaDoc.html'.
sptr_t call(TScintilla &, unsigned int iMessage, uptr_t wParam, sptr_t lParam);

// These invoke the similarly-named methods of 'TScintilla'.
void setParent(TScintilla &, TScintillaParent *aParent);
void changeSize(TScintilla &);
void clearBeforeTentativeStart(TScintilla &);
void insertPasteStream(TScintilla &, TStringView text);
void insertCharacter(TScintilla &, TStringView mbc);
void idleWork(TScintilla &);
TPoint pointMainCaret(TScintilla &);
bool handleKeyDown(TScintilla &, const KeyDownEvent &keyDown);
bool handleMouse(TScintilla &, unsigned short what, const MouseEventType &mouse);
void paint(TScintilla &, TDrawSurface &surface, TRect area);
void setStyleColor(TScintilla &, int style, TColorAttr attr);
TColorAttr getStyleColor(TScintilla &, int style);
void setSelectionColor(TScintilla &, TColorAttr attr);
void setWhitespaceColor(TScintilla &, TColorAttr attr);
TStringView getRangePointer(TScintilla &, Sci_Position start, Sci_Position end);

enum CaseConversion
{
    caseConvNone,
    caseConvUpper,
    caseConvLower,
    caseConvCapitalize,
};

void changeCaseOfSelection(TScintilla &, CaseConversion);

enum Indicator
{
    idtrReplaceHighlight = INDICATOR_CONTAINER,
};

void setIndicatorColor(TScintilla &, Indicator indicator, TColorAttr attr);

class TScintillaParent
{
public:
    virtual TPoint getEditorSize() noexcept = 0;
    virtual void invalidate(TRect area) noexcept = 0;
    virtual void handleNotification(const SCNotification &scn) = 0;
    virtual void setVerticalScrollPos(int delta, int limit) noexcept = 0;
    virtual void setHorizontalScrollPos(int delta, int limit) noexcept = 0;
};

} // namespace turbo

#endif // TURBO_SCINTILLA_H

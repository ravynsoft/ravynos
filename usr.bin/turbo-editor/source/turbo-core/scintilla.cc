#define Uses_TKeys
#define Uses_TEvent
#define Uses_TDrawSurface
#include <tvision/tv.h>

#include <turbo/scintilla/tscintilla.h>
#include <turbo/scintilla.h>
#include "platform/surface.h"
#include <chrono>

namespace turbo {

TScintilla &createScintilla() noexcept
{
    return *new TScintilla;
}

void destroyScintilla(TScintilla &self) noexcept
{
    delete &self;
}

sptr_t call(TScintilla &self, uint iMessage, uptr_t wParam, sptr_t lParam)
{
    return self.WndProc(iMessage, wParam, lParam);
}

void setParent(TScintilla &self, TScintillaParent *aParent)
{
    self.setParent(aParent);
}

void changeSize(TScintilla &self)
{
    self.ChangeSize();
}

void clearBeforeTentativeStart(TScintilla &self)
{
    self.ClearBeforeTentativeStart();
}

void insertPasteStream(TScintilla &self, TStringView text)
{
    self.InsertPasteShape(text.data(), text.size(), TScintilla::pasteStream);
}

void insertCharacter(TScintilla &self, TStringView text)
{
    self.InsertCharacter(text, TScintilla::CharacterSource::directInput);
}

void idleWork(TScintilla &self)
{
    self.IdleWork();
}

TPoint pointMainCaret(TScintilla &self)
{
    auto p = self.PointMainCaret();
    return {(int) p.x, (int) p.y};
}

static int convertModifiers(ulong controlKeyState)
{
    static constexpr struct { ushort tv; int scmod; } modifiersTable[] =
    {
        {kbShift,       SCMOD_SHIFT},
        {kbCtrlShift,   SCMOD_CTRL},
        {kbAltShift,    SCMOD_ALT}
    };

    int modifiers = 0;
    for (const auto &m : modifiersTable)
        if (controlKeyState & m.tv)
            modifiers |= m.scmod;
    return modifiers;
}

bool handleKeyDown(TScintilla &self, const KeyDownEvent &keyDown)
{
    static constexpr struct { ushort tv; int sck; } keysTable[] =
    {
        {kbDown,        SCK_DOWN},
        {kbUp,          SCK_UP},
        {kbLeft,        SCK_LEFT},
        {kbRight,       SCK_RIGHT},
        {kbHome,        SCK_HOME},
        {kbEnd,         SCK_END},
        {kbPgUp,        SCK_PRIOR},
        {kbPgDn,        SCK_NEXT},
        {kbDel,         SCK_DELETE},
        {kbIns,         SCK_INSERT},
        {kbTab,         SCK_TAB},
        {kbEnter,       SCK_RETURN},
        {kbBack,        SCK_BACK},
        {kbShiftDel,    SCK_DELETE},
        {kbShiftIns,    SCK_INSERT},
        {kbShiftTab,    SCK_TAB},
        {kbCtrlDown,    SCK_DOWN},
        {kbCtrlUp,      SCK_UP},
        {kbCtrlLeft,    SCK_LEFT},
        {kbCtrlRight,   SCK_RIGHT},
        {kbCtrlHome,    SCK_HOME},
        {kbCtrlEnd,     SCK_END},
        {kbCtrlPgUp,    SCK_PRIOR},
        {kbCtrlPgDn,    SCK_NEXT},
        {kbCtrlDel,     SCK_DELETE},
        {kbCtrlIns,     SCK_INSERT},
        {kbCtrlEnter,   SCK_RETURN},
        {kbCtrlBack,    SCK_BACK},
        {kbAltDown,     SCK_DOWN},
        {kbAltUp,       SCK_UP},
        {kbAltLeft,     SCK_LEFT},
        {kbAltRight,    SCK_RIGHT},
        {kbAltHome,     SCK_HOME},
        {kbAltEnd,      SCK_END},
        {kbAltPgUp,     SCK_PRIOR},
        {kbAltPgDn,     SCK_NEXT},
        {kbAltDel,      SCK_DELETE},
        {kbAltIns,      SCK_INSERT},
        {kbAltBack,     SCK_BACK},
    };

    int modifiers = convertModifiers(keyDown.controlKeyState);
    bool specialKey = modifiers && !keyDown.textLength;

    int key;
    if (keyDown.keyCode <= kbCtrlZ)
        key = keyDown.keyCode + 'A' - 1;
    else
    {
        key = keyDown.charScan.charCode;
        for (const auto [tv, sck] : keysTable)
            if (keyDown.keyCode == tv)
            {
                key = sck;
                specialKey = true;
                break;
            }
    }

    if (specialKey)
    {
        bool consumed = false;
        self.KeyDownWithModifiers(key, modifiers, &consumed);
        return consumed;
    }
    else
    {
        self.InsertCharacter({keyDown.text, keyDown.textLength}, TScintilla::CharacterSource::directInput);
        return true;
    }
}

bool handleMouse(TScintilla &self, ushort what, const MouseEventType &mouse)
{
    using namespace Scintilla;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::steady_clock;
    auto pt = Point::FromInts(mouse.where.x, mouse.where.y);
    uint time = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    int modifiers = convertModifiers(mouse.controlKeyState); // Very few environments do support this.
    if (mouse.buttons & mbLeftButton)
    {
        // Scintilla actually assumes these functions are invoked only for the
        // left button mouse.
        switch (what)
        {
            case evMouseDown:
                self.ButtonDownWithModifiers(pt, time, modifiers);
                break;
            case evMouseUp:
                self.ButtonUpWithModifiers(pt, time, modifiers);
                break;
            case evMouseMove:
            case evMouseAuto:
                self.ButtonMoveWithModifiers(pt, time, modifiers);
                break;
        }
        return true;
    }
    return false;
}

TColorAttr getStyleColor(TScintilla &self, int style)
{
    using namespace Scintilla;
    ColourDesired fore {(int) call(self, SCI_STYLEGETFORE, style, 0U)};
    ColourDesired back {(int) call(self, SCI_STYLEGETBACK, style, 0U)};
    auto styleWeight = call(self, SCI_STYLEGETWEIGHT, style, 0U);
    return {
        convertColor(fore),
        convertColor(back),
        (ushort) styleWeight,
    };
}

void paint(TScintilla &self, TDrawSurface &d, TRect area)
{
    using namespace Scintilla;
    TScintillaSurface s;
    s.surface = &d;
    s.defaultTextAttr = getStyleColor(self, STYLE_DEFAULT);
    self.Paint(
        &s,
        PRectangle::FromInts(area.a.x, area.a.y, area.b.x, area.b.y)
    );
}

void setStyleColor(TScintilla &self, int style, TColorAttr attr)
{
    using namespace Scintilla;
    call(self, SCI_STYLESETFORE, style, convertColor(::getFore(attr)).AsInteger());
    call(self, SCI_STYLESETBACK, style, convertColor(::getBack(attr)).AsInteger());
    call(self, SCI_STYLESETWEIGHT, style, ::getStyle(attr));
}

void setSelectionColor(TScintilla &self, TColorAttr attr)
{
    using namespace Scintilla;
    auto fg = ::getFore(attr),
         bg = ::getBack(attr);
    call(self, SCI_SETSELFORE, !fg.isDefault(), convertColor(fg).AsInteger());
    call(self, SCI_SETSELBACK, !bg.isDefault(), convertColor(bg).AsInteger());
}

void setWhitespaceColor(TScintilla &self, TColorAttr attr)
{
    using namespace Scintilla;
    auto fg = ::getFore(attr),
         bg = ::getBack(attr);
    call(self, SCI_SETWHITESPACEFORE, !fg.isDefault(), convertColor(fg).AsInteger());
    call(self, SCI_SETWHITESPACEBACK, !bg.isDefault(), convertColor(bg).AsInteger());
}

TStringView getRangePointer(TScintilla &self, Sci_Position start, Sci_Position end)
{
    auto length = end - start;
    if (length <= 0)
        return TStringView();
    return TStringView {
        (const char *) self.WndProc(SCI_GETRANGEPOINTER, (uptr_t) start, (sptr_t) length),
        size_t(length),
    };
}

void changeCaseOfSelection(TScintilla &self, CaseConversion cnv)
{
    self.ChangeCaseOfSelection(cnv);
}

void setIndicatorColor(TScintilla &self, Indicator indicator, TColorAttr attr)
{
    using namespace Scintilla;
    auto fg = ::getFore(attr),
         bg = ::getBack(attr);
    call(self, SCI_INDICSETSTYLE, indicator, INDIC_FULLBOX);
    call(self, SCI_INDICSETFORE, indicator, convertColor(bg).AsInteger());
    call(self, SCI_INDICSETOUTLINEALPHA, indicator, convertColor(fg).AsInteger());
}

} // namespace turbo

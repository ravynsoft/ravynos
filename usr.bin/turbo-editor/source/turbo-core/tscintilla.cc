#define Uses_TText
#define Uses_TClipboard
#include <tvision/tv.h>

#include <turbo/scintilla.h>
#include <turbo/scintilla/tscintilla.h>

#include "platform/surface.h"

namespace Scintilla {

TScintilla::TScintilla()
{
    // Block caret for both Insertion and Overwrite mode.
    WndProc(SCI_SETCARETSTYLE, CARETSTYLE_BLOCK | CARETSTYLE_OVERSTRIKE_BLOCK, 0U);
    // Disable margin on line numbers.
    vs.marginNumberPadding = 0;
    // Disable margin pixels
    WndProc(SCI_SETMARGINLEFT, 0U, 0);
    WndProc(SCI_SETMARGINRIGHT, 0U, 0);
    // Disable buffered fraw
    WndProc(SCI_SETBUFFEREDDRAW, 0, 0U);
    // Disable space between lines
    WndProc(SCI_SETEXTRADESCENT, -1, 0U);
    vs.maxAscent = 0;
    vs.maxDescent = 0;
    // Set our custom representations.
    reprs.Clear();
    {
        constexpr int ranges[][2] = {{0, ' '}, {0x7F, 0x100}};
        for (auto [beg, end] : ranges) {
            for (int i = beg; i < end; ++i) {
                char c[2] = {(char) i};
                char r[8] = {};
                sprintf(r, "\\x%02X", i);
                reprs.SetRepresentation(c, r);
            }
        }
        reprs.SetRepresentation("\t", "»");
    }
    // Do not use padding for control characters.
    vs.ctrlCharPadding = 0;
    view.tabWidthMinimumPixels = 0; // Otherwise, tabs will be more than 8 columns wide.
    // Always draw tabulators.
    WndProc(SCI_SETVIEWWS, SCWS_VISIBLEALWAYS, 0U);
    // Process mouse down events:
    WndProc(SCI_SETMOUSEDOWNCAPTURES, true, 0U);
    // Double clicks only in the same cell.
    doubleClickCloseThreshold = Point(0, 0);
    // Set our custom function to draw wrap markers.
    view.customDrawWrapMarker = drawWrapMarker;

    // Extra key shortcuts.

    // Some Ctrl+key combinations are not supported by many terminals,
    // so allow using Alt instead.
    WndProc(SCI_ASSIGNCMDKEY, SCK_LEFT | (SCMOD_ALT << 16), SCI_WORDLEFT);
    WndProc(SCI_ASSIGNCMDKEY, SCK_LEFT | ((SCMOD_SHIFT | SCMOD_ALT) << 16), SCI_WORDLEFTEXTEND);
    WndProc(SCI_ASSIGNCMDKEY, SCK_RIGHT | (SCMOD_ALT << 16), SCI_WORDRIGHT);
    WndProc(SCI_ASSIGNCMDKEY, SCK_RIGHT | ((SCMOD_SHIFT | SCMOD_ALT) << 16), SCI_WORDRIGHTEXTEND);
    WndProc(SCI_ASSIGNCMDKEY, SCK_UP | ((SCMOD_SHIFT | SCMOD_CTRL) << 16), SCI_MOVESELECTEDLINESUP);
    WndProc(SCI_ASSIGNCMDKEY, SCK_UP | ((SCMOD_SHIFT | SCMOD_ALT) << 16), SCI_MOVESELECTEDLINESUP);
    WndProc(SCI_ASSIGNCMDKEY, SCK_DOWN | ((SCMOD_SHIFT | SCMOD_CTRL) << 16), SCI_MOVESELECTEDLINESDOWN);
    WndProc(SCI_ASSIGNCMDKEY, SCK_DOWN | ((SCMOD_SHIFT | SCMOD_ALT) << 16), SCI_MOVESELECTEDLINESDOWN);
    WndProc(SCI_ASSIGNCMDKEY, SCK_BACK | ((SCMOD_ALT) << 16), SCI_DELWORDLEFT);

    // Home/End keys should respect line wrapping.
    WndProc(SCI_ASSIGNCMDKEY, SCK_HOME | (SCI_NORM << 16), SCI_VCHOMEWRAP);
    WndProc(SCI_ASSIGNCMDKEY, SCK_HOME | (SCI_SHIFT << 16), SCI_VCHOMEWRAPEXTEND);
    WndProc(SCI_ASSIGNCMDKEY, SCK_END | (SCI_NORM << 16), SCI_LINEENDWRAP);
    WndProc(SCI_ASSIGNCMDKEY, SCK_END | (SCI_SHIFT << 16), SCI_LINEENDWRAPEXTEND);

    // Delete current line without altering the clipboard.
    WndProc(SCI_ASSIGNCMDKEY, 'L' | (SCMOD_CTRL << 16), SCI_LINEDELETE);
}

void TScintilla::SetVerticalScrollPos()
{
    auto *parent = getParent();
    if (parent) {
        auto limit = LinesOnScreen() + MaxScrollPos();
        parent->setVerticalScrollPos(topLine, limit);
    }
}

void TScintilla::SetHorizontalScrollPos()
{
    auto *parent = getParent();
    if (parent)
        parent->setHorizontalScrollPos(xOffset, vs.wrapState == SC_WRAP_NONE ? scrollWidth : 1);
}

bool TScintilla::ModifyScrollBars(Sci::Line nMax, Sci::Line nPage)
{
    SetVerticalScrollPos();
    SetHorizontalScrollPos();
    return false;
}

void TScintilla::Copy()
{
    if (!sel.Empty())
    {
        SelectionText selText;
        CopySelectionRange(&selText);
        TClipboard::setText({selText.Data(), selText.Length()});
    }
}

void TScintilla::Paste()
{
    TClipboard::requestText();
}

void TScintilla::ClaimSelection()
{
}

void TScintilla::NotifyChange()
{
}

void TScintilla::NotifyParent(SCNotification scn)
{
    auto *parent = getParent();
    if (parent)
        parent->handleNotification(scn);
}

void TScintilla::CopyToClipboard(const SelectionText &selectedText)
{
}

bool TScintilla::FineTickerRunning(TickReason reason)
{
    return false;
}

void TScintilla::FineTickerStart(TickReason reason, int millis, int tolerance)
{
}

void TScintilla::FineTickerCancel(TickReason reason)
{
}

void TScintilla::SetMouseCapture(bool on)
{
}

bool TScintilla::HaveMouseCapture()
{
    return true;
}

sptr_t TScintilla::DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
    return 0;
}

void TScintilla::CreateCallTipWindow(PRectangle rc)
{
}

void TScintilla::AddToPopUp(const char *label, int cmd, bool enabled)
{
}

CaseFolder *TScintilla::CaseFolderForEncoding()
{
    if (IsUnicodeMode())
        return new CaseFolderUnicode();
    return super::CaseFolderForEncoding();
}

template <size_t (& next)(TStringView) noexcept>
static std::string capitalize(TStringView s, const Scintilla::Document &doc)
{
    std::string result;
    size_t i = 0;
    while (i < s.size())
    {
        size_t spaceBegin = i;
        while (i < s.size() && doc.WordCharacterClass(s[i]) != CharClassify::ccWord)
            ++i;
        auto space = s.substr(spaceBegin, i - spaceBegin);
        result.append(space.data(), space.size());

        size_t firstBegin = i;
        i += next(s.substr(i));
        auto first = s.substr(firstBegin, i - firstBegin);
        result.append(CaseConvertString(first, CaseConversionUpper));

        size_t tailBegin = i;
        while (i < s.size() && doc.WordCharacterClass(s[i]) == CharClassify::ccWord)
            ++i;
        auto tail = s.substr(tailBegin, i - tailBegin);
        result.append(CaseConvertString(tail, CaseConversionLower));
    }
    return result;
}

static size_t nextUnicode(TStringView s) noexcept
{
    return TText::next(s);
}

static size_t nextAscii(TStringView s) noexcept
{
    return min<size_t>(s.size(), 1);
}

std::string TScintilla::CaseMapString(const std::string &s, int mapping)
{
    if (IsUnicodeMode())
        switch (turbo::CaseConversion(mapping))
        {
            case turbo::caseConvNone:
                return s;
            case turbo::caseConvUpper:
                return CaseConvertString(s, CaseConversionUpper);
            case turbo::caseConvLower:
                return CaseConvertString(s, CaseConversionLower);
            case turbo::caseConvCapitalize:
                return capitalize<nextUnicode>(s, *pdoc);
        }
    switch (turbo::CaseConversion(mapping))
    {
        case turbo::caseConvCapitalize:
            return capitalize<nextAscii>(s, *pdoc);
        default:
            return super::CaseMapString(s, mapping);
    }
}

int TScintilla::KeyDefault(int key, int modifiers) {
    if (!modifiers)
    {
        super::AddChar(key);
        return 1;
    }
    return 0;
}

void TScintilla::drawWrapMarker(Surface *surface, PRectangle rcPlace, bool isEndMarker, ColourDesired wrapColour)
{
    auto *s = (TScintillaSurface *) surface;
    Font f {};
    if (isEndMarker)
        // Imitate the Tilde text editor.
        s->DrawTextTransparent(rcPlace, f, rcPlace.bottom, "↵", wrapColour);
}

} // namespace Scintilla

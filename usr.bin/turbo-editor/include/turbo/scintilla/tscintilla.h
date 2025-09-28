#ifndef TURBO_TSCINTILLA_H
#define TURBO_TSCINTILLA_H

#define Uses_TPoint
#include <tvision/tv.h>

#include <turbo/scintilla/internals.h>

namespace turbo {
class TScintillaParent;
} // namespace turbo

namespace Scintilla {

class TScintilla : public ScintillaBase
{
    using super = ScintillaBase;

    static void drawWrapMarker(Surface *, PRectangle, bool, ColourDesired);

protected:

    void SetVerticalScrollPos() override;
    void SetHorizontalScrollPos() override;
    bool ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) override;
    void Copy() override;
    void Paste() override;
    void ClaimSelection() override;
    void NotifyChange() override;
    void NotifyParent(SCNotification scn) override;
    void CopyToClipboard(const SelectionText &selectedText) override;
    bool FineTickerRunning(TickReason reason) override;
    void FineTickerStart(TickReason reason, int millis, int tolerance) override;
    void FineTickerCancel(TickReason reason) override;
    void SetMouseCapture(bool on) override;
    bool HaveMouseCapture() override;
    sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) override;
    void CreateCallTipWindow(Scintilla::PRectangle rc) override;
    void AddToPopUp(const char *label, int cmd=0, bool enabled=true) override;

    CaseFolder *CaseFolderForEncoding() override;
    std::string CaseMapString(const std::string &, int) override;
    int KeyDefault(int key, int modifiers) override;

public:

    TScintilla();

    void setParent(turbo::TScintillaParent *aParent);
    turbo::TScintillaParent *getParent() const;
    using super::ChangeSize;
    using super::ClearBeforeTentativeStart;
    using super::InsertPasteShape;
    using super::InsertCharacter;
    using super::IdleWork;
    using super::PointMainCaret;
    using super::KeyDownWithModifiers;
    using super::ButtonDownWithModifiers;
    using super::ButtonUpWithModifiers;
    using super::ButtonMoveWithModifiers;
    using super::Paint;
    using super::pasteStream;
    using super::CharacterSource;
    using super::ChangeCaseOfSelection;

};

inline void TScintilla::setParent(turbo::TScintillaParent *aParent)
{
    wMain = aParent;
}

inline turbo::TScintillaParent *TScintilla::getParent() const
{
    return (turbo::TScintillaParent *) wMain.GetID();
}

} // namespace Scintilla

#endif

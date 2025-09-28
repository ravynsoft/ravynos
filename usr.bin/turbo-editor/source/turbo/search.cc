#define Uses_TKeys
#define Uses_TEvent
#define Uses_TLabel
#define Uses_TIndicator
#define Uses_TButton
#define Uses_TFrame
#define Uses_TDrawSurface
#define Uses_TCheckBoxes
#define Uses_TSItem
#include <tvision/tv.h>

#include "search.h"
#include "checkbox.h"
#include "combobox.h"
#include <turbo/editstates.h>
#include <turbo/util.h>

static constexpr SpanListModelEntry<turbo::SearchMode> searchModes[] =
{
    {turbo::smPlainText, "Plain text"},
    {turbo::smWholeWords, "Whole words"},
    {turbo::smRegularExpression, "Regular expression"},
};

static constexpr SpanListModel<turbo::SearchMode> searchModeListModel {searchModes};

SearchBox::SearchBox( const TRect &bounds, SearchState &aSearchState,
                      ushort aFindCommand ) noexcept :
    TGroup(bounds),
    searchState(aSearchState),
    findCommand(aFindCommand)
{
    options |= ofFramed | ofFirstClick;

    auto *bckgrnd = new TView(getExtent());
    bckgrnd->growMode = gfGrowHiX | gfGrowHiY;
    bckgrnd->eventMask = 0;
    insert(bckgrnd);
}

void SearchBox::shutDown()
{
    cmbMode = nullptr;
    cbCaseSensitive = nullptr;
    TGroup::shutDown();
}

void SearchBox::handleEvent(TEvent &ev)
{
    TGroup::handleEvent(ev);
    if (ev.what == evKeyDown)
    {
        switch (ev.keyDown.keyCode)
        {
            case kbTab:
                focusNext(False);
                clearEvent(ev);
                break;
            case kbShiftTab:
                focusNext(True);
                clearEvent(ev);
                break;
        }
    }
    else if (ev.what == evCommand && ev.message.command == findCommand)
        clearEvent(ev);
    else if (ev.what == evBroadcast && ev.message.command == cmStateChanged)
        storeSettings();
}

void SearchBox::loadSettings()
{
    using namespace turbo;
    auto settings = searchState.settingsPreset.get();
    if (cmbMode)
        cmbMode->setCurrentIndex(settings.mode);
    if (cbCaseSensitive)
        cbCaseSensitive->setChecked(settings.flags & sfCaseSensitive);
}

void SearchBox::storeSettings()
{
    using namespace turbo;
    auto settings = searchState.settingsPreset.get();
    if (cmbMode)
        if (auto *entry = (const SpanListModelEntry<turbo::SearchMode> *) cmbMode->getCurrent())
            settings.mode = entry->data;
    if (cbCaseSensitive)
        settings.flags = -cbCaseSensitive->isChecked() & sfCaseSensitive;
    searchState.settingsPreset.set(settings);
}

FindBox::FindBox(const TRect &bounds, SearchState &aSearchState) noexcept :
    SearchBox(bounds, aSearchState, cmFindFindBox)
{
    auto *findText = "~F~ind:";
    auto *nextText = "~N~ext";
    auto *prevText = "~P~revious";
    auto *modeText = "~M~ode:";
    auto *caseText = "~C~ase sensitive";
    TRect rLabelF {0, 0, 1 + cstrlen(findText), 1};
    TRect rPrev {size.x - 1 - cstrlen(prevText) - 4, 0, size.x - 1, 2};
    TRect rNext {rPrev.a.x - cstrlen(nextText) - 4, 0, rPrev.a.x, 2};
    TRect rBoxF {rLabelF.b.x + 1, 0, rNext.a.x, 1};
    TRect rCase {size.x - cstrlen(caseText) - 6, 2, size.x, 3};
    TRect rLabelM {0, 2, 1 + cstrlen(modeText), 3};
    TRect rBoxM {rLabelM.b.x + 1, 2, rCase.a.x, 3};

    auto *ilFind = new SearchInputLine(rBoxF, searchState.findText, imFind);
    ilFind->growMode = gfGrowHiX;
    ilFind->selectAll(true);

    auto *lblFind = new TLabel(rLabelF, findText, ilFind);
    lblFind->growMode = 0;
    insert(lblFind);

    cmbMode = new ComboBox(rBoxM, searchModeListModel);
    cmbMode->growMode = gfGrowHiX;
    insert(cmbMode);

    auto *btnNext = new TButton(rNext, nextText, cmSearchAgain, bfNormal);
    btnNext->growMode = gfGrowLoX | gfGrowHiX;
    insert(btnNext);

    auto *btnPrev = new TButton(rPrev, prevText, cmSearchPrev, bfNormal);
    btnPrev->growMode = gfGrowLoX | gfGrowHiX;
    insert(btnPrev);

    auto *lblMode = new TLabel(rLabelM, modeText, cmbMode);
    lblMode->growMode = 0;
    insert(lblMode);

    cbCaseSensitive = new CheckBox(rCase, caseText);
    cbCaseSensitive->growMode = gfGrowLoX | gfGrowHiX;
    insert(cbCaseSensitive);

    // This must be the first selectable view.
    insert(ilFind);

    loadSettings();
}

ReplaceBox::ReplaceBox(const TRect &bounds, SearchState &aSearchState) noexcept :
    SearchBox(bounds, aSearchState, cmFindReplaceBox)
{
    auto *findText = "~F~ind:";
    auto *nextText = "~N~ext";
    auto *prevText = "~P~revious";
    auto *replText = "~R~eplace:";
    auto *oneText = "~O~ne";
    auto *allText = "~A~ll";
    auto *modeText = "~M~ode:";
    auto *caseText = "~C~ase sensitive";
    int findReplLen = max(max(cstrlen(findText), cstrlen(replText)), cstrlen(modeText));
    int nextOneLen = max(cstrlen(nextText), cstrlen(oneText));
    int prevAllLen = max(cstrlen(prevText), cstrlen(allText));
    TRect rLabelF {0, 0, 1 + findReplLen, 1};
    TRect rPrev {size.x - 1 - prevAllLen - 4, 0, size.x - 1, 2};
    TRect rNext {rPrev.a.x - nextOneLen - 4, 0, rPrev.a.x, 2};
    TRect rBoxF {rLabelF.b.x + 1, 0, rNext.a.x, 1};
    TRect rLabelR {0, 2, 1 + findReplLen, 3};
    TRect rAll {size.x - 1 - prevAllLen - 4, 2, size.x - 1, 4};
    TRect rOne {rAll.a.x - nextOneLen - 4, 2, rAll.a.x, 4};
    TRect rBoxR {rLabelR.b.x + 1, 2, rOne.a.x, 3};
    TRect rCase {size.x - cstrlen(caseText) - 6, 4, size.x, 5};
    TRect rLabelM {0, 4, 1 + findReplLen, 5};
    TRect rBoxM {rLabelM.b.x + 1, 4, rCase.a.x, 5};

    auto *ilFind = new SearchInputLine(rBoxF, searchState.findText, imFind);
    ilFind->growMode = gfGrowHiX;
    ilFind->selectAll(true);

    auto *lblFind = new TLabel(rLabelF, findText, ilFind);
    lblFind->growMode = 0;
    insert(lblFind);

    auto *ilRepl = new SearchInputLine(rBoxR, searchState.replaceText, imReplace);
    ilRepl->growMode = gfGrowHiX;
    insert(ilRepl);

    cmbMode = new ComboBox(rBoxM, searchModeListModel);
    cmbMode->growMode = gfGrowHiX;
    insert(cmbMode);

    auto *btnNext = new TButton(rNext, nextText, cmSearchAgain, bfNormal);
    btnNext->growMode = gfGrowLoX | gfGrowHiX;
    insert(btnNext);

    auto *btnPrev = new TButton(rPrev, prevText, cmSearchPrev, bfNormal);
    btnPrev->growMode = gfGrowLoX | gfGrowHiX;
    insert(btnPrev);

    auto *lblRepl = new TLabel(rLabelR, replText, ilRepl);
    lblRepl->growMode = 0;
    insert(lblRepl);

    auto *btnOne = new TButton(rOne, oneText, cmReplaceOne, bfNormal);
    btnOne->growMode = gfGrowLoX | gfGrowHiX;
    insert(btnOne);

    auto *btnAll = new TButton(rAll, allText, cmReplaceAll, bfNormal);
    btnAll->growMode = gfGrowLoX | gfGrowHiX;
    insert(btnAll);

    auto *lblMode = new TLabel(rLabelM, modeText, cmbMode);
    lblMode->growMode = 0;
    insert(lblMode);

    cbCaseSensitive = new CheckBox(rCase, caseText);
    cbCaseSensitive->growMode = gfGrowLoX | gfGrowHiX;
    insert(cbCaseSensitive);

    // This must be the first selectable view.
    insert(ilFind);

    loadSettings();
}

void ReplaceBox::shutDown()
{
    TEvent ev;
    ev.what = evCommand;
    ev.message.command = cmClearReplace;
    ev.message.infoPtr = nullptr;
    putEvent(ev);
    SearchBox::shutDown();
}

SearchInputLine::SearchInputLine( const TRect &bounds, char (&aData)[256],
                                  SearchInputLineMode aMode ) noexcept :
    TInputLine(bounds, 256, new SearchValidator(*this)),
    backupData(data),
    mode(aMode)
{
    data = aData;
}

void SearchInputLine::shutDown()
{
    data = backupData;
    TInputLine::shutDown();
}

void SearchInputLine::handleEvent(TEvent &ev)
{
    if (ev.what == evKeyDown && ev.keyDown.keyCode == kbEnter)
    {
        bool shift = (ev.keyDown.controlKeyState & kbShift);
        ev.what = evCommand;
        if (mode == imFind)
            ev.message.command = shift ? cmSearchPrev : cmSearchAgain;
        else
            ev.message.command = cmReplaceOne;
        ev.message.infoPtr = nullptr;
        putEvent(ev);
        clearEvent(ev);
    }
    else
        TInputLine::handleEvent(ev);
}

Boolean SearchValidator::isValidInput(char *text, Boolean)
{
    // Invoked while typing.
    if (inputLine.mode == imFind)
    {
        TEvent ev;
        ev.what = evCommand;
        ev.message.command = cmSearchIncr;
        ev.message.infoPtr = nullptr;
        inputLine.putEvent(ev);
    }
    return True;
}

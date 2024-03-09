#ifndef TURBO_SEARCH_H
#define TURBO_SEARCH_H

#define Uses_TGroup
#define Uses_TInputLine
#define Uses_TValidator
#include <tvision/tv.h>

#include <turbo/editstates.h>
#include "cmds.h"
#include "apputils.h"

namespace turbo
{
class Editor;
}

class CheckBox;
class ComboBox;

struct SearchState
{
    Preset<turbo::SearchSettings> settingsPreset;
    char findText[256] {0};
    char replaceText[256] {0};
};

class SearchBox : public TGroup
{
protected:

    SearchState &searchState;
    ushort findCommand;
    ComboBox *cmbMode;
    CheckBox *cbCaseSensitive;

    void handleEvent(TEvent &ev) override;
    void shutDown() override;

    void loadSettings();
    void storeSettings();

    SearchBox(const TRect &bounds, SearchState &searchState, ushort findCommand) noexcept;
};

class FindBox : public SearchBox
{
public:

    enum { findCommand = cmFindFindBox };
    enum { height = 3 };

    FindBox(const TRect &bounds, SearchState &searchState) noexcept;
};

class ReplaceBox : public SearchBox
{
public:

    enum { findCommand = cmFindReplaceBox };
    enum { height = 5 };

    ReplaceBox(const TRect &bounds, SearchState &searchState) noexcept;

    void shutDown() override;
};

enum SearchInputLineMode
{
    imFind,
    imReplace,
};

class SearchInputLine : public TInputLine
{
    char *backupData;

    void handleEvent(TEvent &ev) override;
    void shutDown() override;

public:

    SearchInputLineMode mode;

    SearchInputLine( const TRect &bounds, char (&data)[256],
                     SearchInputLineMode mode ) noexcept;
};

class SearchValidator : public TValidator
{
    SearchInputLine &inputLine;

    Boolean isValidInput(char *, Boolean) override;

public:

    SearchValidator(SearchInputLine &aInputLine) noexcept :
        inputLine(aInputLine)
    {
    }
};

#endif // TURBO_SEARCH_H

#ifndef TURBO_GOTOLINE_H
#define TURBO_GOTOLINE_H

#define Uses_TGroup
#define Uses_TInputLine
#define Uses_TRangeValidator
#include <tvision/tv.h>

#include "cmds.h"

namespace turbo
{
class Editor;
}

class GoToLineInputLine;

class GoToLineBox : public TGroup
{
    turbo::Editor &editor;
    GoToLineInputLine *ilLine;

    void handleEvent(TEvent &ev) override;
    void shutDown() override;

    void goToLine(const char *input);

public:

    enum { findCommand = cmFindGoToLineBox };
    enum { height = 2 };

    GoToLineBox(const TRect &bounds, turbo::Editor &editor) noexcept;
};

class GoToLineValidator : public TRangeValidator
{
    turbo::Editor &editor;

    Boolean isValid(const char *) override;
    Boolean isValidInput(char *, Boolean) override;

public:

    GoToLineValidator(turbo::Editor &aEditor) :
        TRangeValidator(1, 1),
        editor(aEditor)
    {
    }
};

class GoToLineInputLine : public TInputLine
{
    void handleEvent(TEvent &ev) override;

public:

    GoToLineInputLine(const TRect &bounds, GoToLineValidator &validator) noexcept :
        TInputLine(bounds, 32, &validator)
    {
    }
};

#endif // TURBO_GOTOLINE_H

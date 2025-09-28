#ifndef TVISION_NCURSINP_H
#define TVISION_NCURSINP_H

#include <internal/platform.h>

#ifdef HAVE_NCURSES

#define Uses_TKeys
#define Uses_TPoint
#define Uses_TEvent
#include <tvision/tv.h>

#include <internal/terminal.h>

namespace tvision
{

class NcursesDisplay;

struct NcursesInputGetter final : public InputGetter
{
    size_t pendingCount {0};

    int get() noexcept override;
    void unget(int k) noexcept override;
};

class NcursesInput : public InputStrategy
{
    enum : char { KEY_ESC = '\x1B' };
    enum { readTimeoutMs = 10 };

    StdioCtl &io;
    InputState &state;
    bool mouseEnabled;
    NcursesInputGetter in;

    int getChNb() noexcept;
    void detectAlt(int keys[4], bool &Alt) noexcept;
    void parsePrintableChar(TEvent &ev, int keys[4], int &num_keys) noexcept;
    void readUtf8Char(int keys[4], int &num_keys) noexcept;

    bool parseCursesMouse(TEvent&) noexcept;

public:

    // Lifetimes of 'io', 'display' and 'state' must exceed that of 'this'.
    NcursesInput(StdioCtl &io, NcursesDisplay &display, InputState &state, bool mouse) noexcept;
    ~NcursesInput();

    bool getEvent(TEvent &ev) noexcept override;
    int getButtonCount() noexcept override;
    bool hasPendingEvents() noexcept override;
};

} // namespace tvision

#endif // HAVE_NCURSES

#endif // TVISION_NCURSINP_H

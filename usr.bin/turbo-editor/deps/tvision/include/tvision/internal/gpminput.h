#ifndef TVISION_GPMINPUT_H
#define TVISION_GPMINPUT_H

#include <internal/platform.h>

#ifdef HAVE_GPM

#define Uses_TPoint
#define Uses_TEvent
#include <tvision/tv.h>

#include <internal/cursor.h>
#include <gpm.h>

namespace tvision
{

class GpmInput final : public InputStrategy
{
    NegativeScreenCursor cursor;
    uchar buttonState;

    static void fitEvent(Gpm_Event&) noexcept;
    GpmInput() noexcept;

public:

    static GpmInput *create() noexcept;
    ~GpmInput();
    bool getEvent(TEvent &ev) noexcept;
    int getButtonCount() noexcept;
};

} // namespace tvision

#else

namespace tvision
{

class GpmInput : public InputStrategy
{
public:
    static GpmInput *create() noexcept { return nullptr; }
};

} // namespace tvision

#endif // HAVE_GPM

#endif // TVISION_GPMINPUT_H

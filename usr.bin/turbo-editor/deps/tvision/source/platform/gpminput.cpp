#ifdef HAVE_GPM

#define Uses_TPoint
#define Uses_TEvent
#define Uses_TKeys
#define Uses_TScreen
#include <tvision/tv.h>

#include <internal/gpminput.h>
#include <internal/linuxcon.h>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <gpm.h>

namespace tvision
{

GpmInput *GpmInput::create() noexcept
{
    // Let coordinates begin at zero instead of one.
    gpm_zerobased = 1;
    Gpm_Connect conn = {
        .eventMask = GPM_DOWN | GPM_UP | GPM_DRAG | GPM_MOVE,
        .defaultMask = 0, // Disable cursor drawing by the server.
        /* Disable mouse event reporting when keyboard modifiers are active.
         * In such case, GPM text selection and copy/paste will be active. */
        .minMod = 0,
        .maxMod = 0 };
    // Because we only instantiate GPM in the Linux console, discard the
    // TERM variable during Gpm_Open so that GPM won't assume it is being
    // ran under xterm (e.g. if TERM=xterm), and 'gpm_fd' won't be -2.
    {
        std::unique_ptr<char[]> term {newStr(getenv("TERM"))};
        if (term) unsetenv("TERM");
        Gpm_Open(&conn, 0);
        if (term) setenv("TERM", term.get(), 1);
    }
    if (gpm_fd != -1)
        return new GpmInput;
    return nullptr;
}

GpmInput::GpmInput() noexcept :
    InputStrategy(gpm_fd),
    buttonState(0)
{
}

GpmInput::~GpmInput()
{
    Gpm_Close();
}

int GpmInput::getButtonCount() noexcept
{
    return 2;
}

void GpmInput::fitEvent(Gpm_Event &gpmEvent) noexcept
{
    short &x = gpmEvent.x, &y = gpmEvent.y;
    x = std::min<short>(std::max<short>(x, 0), TScreen::screenWidth - 1);
    y = std::min<short>(std::max<short>(y, 0), TScreen::screenHeight - 1);
}

static constexpr struct { uchar gpm, mb; } gpmButtonFlags[] =
{
    {GPM_B_LEFT, mbLeftButton},
    {GPM_B_RIGHT, mbRightButton},
    {GPM_B_MIDDLE, mbMiddleButton},
};

bool GpmInput::getEvent(TEvent &ev) noexcept
{
    Gpm_Event gpmEvent;
    if (Gpm_GetEvent(&gpmEvent) == 1)
    {
        fitEvent(gpmEvent);
        cursor.setPos({gpmEvent.x, gpmEvent.y});
        cursor.show();

        ev.what = evMouse;
        ev.mouse.where.x = gpmEvent.x;
        ev.mouse.where.y = gpmEvent.y;
        for (const auto &flag : gpmButtonFlags)
            if (gpmEvent.buttons & flag.gpm)
            {
                if (gpmEvent.type & GPM_DOWN)
                    buttonState |= flag.mb;
                if (gpmEvent.type & GPM_UP)
                    buttonState &= ~flag.mb;
            }
        ev.mouse.buttons = buttonState;
        if (gpmEvent.wdy)
            ev.mouse.wheel = gpmEvent.wdy > 0 ? mwUp : mwDown;
        else if (gpmEvent.wdx)
            ev.mouse.wheel = gpmEvent.wdx > 0 ? mwRight : mwLeft;
        else
            ev.mouse.wheel = 0;
        return true;
    }
    return false;
}

} // namespace tvision

#endif // HAVE_GPM

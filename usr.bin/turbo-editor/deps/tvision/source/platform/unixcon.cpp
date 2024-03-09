#include <internal/unixcon.h>

#ifdef _TV_UNIX

#include <internal/sigwinch.h>
#include <internal/terminal.h>
#include <internal/dispbuff.h>
#include <internal/unixclip.h>

namespace tvision
{

inline UnixConsoleStrategy::UnixConsoleStrategy( DisplayStrategy &aDisplay,
                                                 InputStrategy &aInput,
                                                 StdioCtl &aIo,
                                                 DisplayBuffer &aDisplayBuf,
                                                 InputState &aInputState,
                                                 SigwinchHandler *aSigwinch ) noexcept :
    ConsoleStrategy(aDisplay, aInput, {&aInput, aSigwinch}),
    io(aIo),
    displayBuf(aDisplayBuf),
    inputState(aInputState),
    sigwinch(aSigwinch)
{
}

UnixConsoleStrategy &UnixConsoleStrategy::create( StdioCtl &io,
                                                  DisplayBuffer &displayBuf,
                                                  InputState &inputState,
                                                  DisplayStrategy &display,
                                                  InputStrategy &input ) noexcept
{
    auto *sigwinch = SigwinchHandler::create();
    return *new UnixConsoleStrategy(display, input, io, displayBuf, inputState, sigwinch);
}

UnixConsoleStrategy::~UnixConsoleStrategy()
{
    delete sigwinch;
    delete &input;
    delete &display;
    delete &inputState;
}

bool UnixConsoleStrategy::setClipboardText(TStringView text) noexcept
{
    if (UnixClipboard::setClipboardText(text))
        return true;
    if (TermIO::setClipboardText(io, text, inputState))
        return true;
    // On non-success, 'setClipboardText' prints an OSC sequence not all
    // terminals can handle; redraw the screen in case it has been messed up.
    displayBuf.redrawScreen(display);
    return false;
}

bool UnixConsoleStrategy::requestClipboardText(void (&accept)(TStringView)) noexcept
{
    if (UnixClipboard::requestClipboardText(accept))
        return true;
    if (TermIO::requestClipboardText(io, accept, inputState))
        return true;
    return false;
}

} // namespace tvision

#endif // _TV_UNIX

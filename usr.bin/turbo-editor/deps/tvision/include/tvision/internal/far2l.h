#ifndef TVISION_FAR2L_H
#define TVISION_FAR2L_H

#include <internal/terminal.h>

namespace tvision
{

class EventSource;

#define far2lEnableSeq "\x1B_far2l1\x1B\\"
#define far2lDisableSeq "\x1B_far2l0\x1B\\"

ParseResult parseFar2lAnswer(GetChBuf &, TEvent &, InputState &) noexcept;
ParseResult parseFar2lInput(GetChBuf &, TEvent &, InputState &) noexcept;

bool setFar2lClipboard(StdioCtl &, TStringView, InputState &) noexcept;
bool requestFar2lClipboard(StdioCtl &, InputState &) noexcept;

} // namespace tvision

#endif // TVISION_FAR2L_H

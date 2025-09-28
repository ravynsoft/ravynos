#ifndef TVISION_UNIXCLIP_H
#define TVISION_UNIXCLIP_H

#include <tvision/tv.h>

#ifdef _TV_UNIX

namespace tvision
{

class UnixClipboard
{
public:

    static bool setClipboardText(TStringView text) noexcept;
    static bool requestClipboardText(void (&accept)(TStringView)) noexcept;

};

} // namespace tvision

#endif // _TV_UNIX

#endif // TVISION_UNIXCLIP_H

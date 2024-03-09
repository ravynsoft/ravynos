#include <internal/cursor.h>

namespace tvision
{

void ReverseScreenCursor::draw(TColorAttr &attr) const noexcept
{
    attr = reverseAttribute(attr);
}

void NegativeScreenCursor::draw(TColorAttr &attr) const noexcept
{
    auto fg = ::getFore(attr),
         bg = ::getBack(attr);
    ::setFore(attr, TColorBIOS(fg.toBIOS(true) ^ 0x7));
    ::setBack(attr, TColorBIOS(bg.toBIOS(false) ^ 0x7));
}

} // namespace tvision

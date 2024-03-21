#ifndef TVISION_NCURDISP_H
#define TVISION_NCURDISP_H

#include <internal/termdisp.h>

#ifdef HAVE_NCURSES

#include <unordered_map>
#include <ncurses.h>

namespace tvision
{

class NcursesDisplay : public TerminalDisplay
{
    SCREEN *term;

    bool hasColors;
    std::unordered_map<ushort, int> pairIdentifiers;
    ushort definedPairs;

    bool usesNcursesDraw;

    void getCaretPosition(int &x, int &y) noexcept;
    uint translateAttributes(TColorAttr attr) noexcept;
    uint getColorPair(uchar pairKey) noexcept;

public:

    // The lifetime of 'aIo' exceeds that of 'this'.
    NcursesDisplay(StdioCtl &io) noexcept;
    ~NcursesDisplay();

    void reloadScreenInfo() noexcept override;
    TPoint getScreenSize() noexcept override;
    int getCaretSize() noexcept override;
    int getColorCount() noexcept override;

    void clearScreen() noexcept override;

protected:

    void lowlevelWriteChars(TStringView chars, TColorAttr attr) noexcept override;
    void lowlevelMoveCursor(uint x, uint y) noexcept override;
    void lowlevelCursorSize(int size) noexcept override;
    void lowlevelFlush() noexcept override;
};

} // namespace tvision

#else

namespace tvision
{

class NcursesDisplay : public DisplayStrategy {};

} // namespace tvision

#endif // HAVE_NCURSES

#endif // TVISION_NCURDISP_H

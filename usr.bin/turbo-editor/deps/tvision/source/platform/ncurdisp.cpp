#ifdef HAVE_NCURSES

#define Uses_TColorAttr
#include <tvision/tv.h>

#include <internal/ncurdisp.h>
#include <internal/stdioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

namespace tvision
{

NcursesDisplay::NcursesDisplay(StdioCtl &aIo) noexcept :
    TerminalDisplay(aIo),
    definedPairs(0),
    usesNcursesDraw(false)
{
    // Start curses mode.
    term = newterm(nullptr, io.fout(), io.fin());
    if (!term)
    {
        fputs("Cannot initialize Ncurses: 'newterm' failed.\n", stderr);
        exit(1);
    }
    // Enable colors if the terminal supports it.
    if ((hasColors = has_colors()))
    {
        start_color();
        // Use default colors when clearing the screen.
        use_default_colors();
    }
    initCapabilities();
    /* Refresh now so that a possible first getch() doesn't make any relevant
     * changes to the screen due to its implicit refresh(). */
    wrefresh(stdscr);
}

NcursesDisplay::~NcursesDisplay()
{
    // End curses mode.
    endwin();
    delscreen(term);
}

void NcursesDisplay::reloadScreenInfo() noexcept
{
    TPoint size = io.getSize();
    // When Ncurses is not used for drawing (e.g. AnsiDisplay<NcursesDisplay>),
    // 'resizeterm' causes terrible flickering, so we better use 'resize_term'.
    // However, when Ncurses is used for drawing, 'resizeterm' is necessary, as
    // the screen becomes garbled otherwise.
    if (usesNcursesDraw)
        resizeterm(size.y, size.x);
    else
        resize_term(size.y, size.x);
}

TPoint NcursesDisplay::getScreenSize() noexcept
{
    int y, x;
    getmaxyx(stdscr, y, x);
    return {max(x, 0), max(y, 0)};
}

int NcursesDisplay::getCaretSize() noexcept
{
    int size = curs_set(0);
    curs_set(size);
    return size <= 0 ? 0 : size == 1 ? 1 : 100;
}

int NcursesDisplay::getColorCount() noexcept
{
    return COLORS;
}

void NcursesDisplay::clearScreen() noexcept { wclear(stdscr); }
void NcursesDisplay::lowlevelMoveCursor(uint x, uint y) noexcept { wmove(stdscr, y, x); }
void NcursesDisplay::lowlevelFlush() noexcept { wrefresh(stdscr); }

void NcursesDisplay::lowlevelCursorSize(int size) noexcept
{
/* The caret is the keyboard cursor. If size is 0, the caret is hidden. The
 * other possible values are from 1 to 100, theoretically, and represent the
 * percentage of the character cell the caret fills.
 * https://docs.microsoft.com/en-us/windows/console/console-cursor-info-str
 *
 * ncurses supports only three levels: invisible (0), normal (1) and
 * very visible (2). They don't make a difference in all terminals, but
 * we can try mapping them to the values requested by Turbo Vision. */
    curs_set(size > 0 ? size == 100 ? 2 : 1 : 0); // Implies refresh().
}

/* Turbo Vision stores char/attribute information in a CHAR_INFO struct:
 * https://docs.microsoft.com/en-us/windows/console/char-info-str
 * The lower 16 bits store the char value and the higher 16 bits store the
 * character attributes. Turbo Vision usually only uses 8 bits for the char
 * value because it doesn't support Unicode. */

/* From the 8 bits used for attributes, the lower half defines the foreground
 * color while the upper half defines the background color. Each color is
 * defined as Intensity-Red-Green-Blue, while ncurses can handle 4-bit colors
 * in the format Bright-Blue-Green-Red.
 *
 * Bright and Intense are the same except for the fact that not all terminals
 * support bright background colors, and when they do, they do so in different
 * ways. What's certain is that terminals with limited color support (such as
 * the linux console) can display bright foreground colors by using the Bold
 * attribute. Terminals supporting at least 16 colors should support both
 * foreground and background bright colors without any special attribute. The
 * number of supported colors is represented by the global variable COLORS,
 * set by ncurses.
 *
 * Some examples here:
 * https://www.linuxjournal.com/content/about-ncurses-colors-0 */

/* The best way to use colors in ncurses is to define <foreground, background>
 * color pairs. The number of color pairs supported by the terminal is
 * represented by the global variable COLOR_PAIRS. Pair number 0 is reserved
 * for the terminal's default color. Other pairs need to be first defined with
 * init_pair and are assigned an identifier number. The easiest is to begin
 * numerating pairs from one as they get defined. This avoids problems in
 * terminals with limited color support. For instance, the example linked above
 * doesn't work on the linux console because it doesn't take this approach. */

void NcursesDisplay::lowlevelWriteChars(TStringView chars, TColorAttr attr) noexcept
{
    usesNcursesDraw = true;
    // Translate and apply text attributes.
    uint curses_attr = translateAttributes(attr);
    wattron(stdscr, curses_attr);
    // Print characters.
    waddnstr(stdscr, chars.data(), chars.size());
    wattroff(stdscr, curses_attr);
}

uint NcursesDisplay::translateAttributes(TColorAttr attr) noexcept
{
    /* To understand the bit masks, please read:
     * https://docs.microsoft.com/en-us/windows/console/char-info-str
     * Support for bright colors is a bit inconsistent between terminals, so
     * we do the following: if it doesn't support 16 colors, then we provide
     * the terminal with 3-bit colors and use Bold to represent a bright
     * foreground. Otherwise, we provide 4-bit colors directly to the terminal. */
    auto fg = BIOStoXTerm16(::getFore(attr).toBIOS(true)),
         bg = BIOStoXTerm16(::getBack(attr).toBIOS(false));
    uchar idx = fg | (bg << 4);
    uchar pairKey = idx & (COLORS < 16 ? 0x77 : 0xFF);
    bool fgIntense = (COLORS < 16) && (fg & 0x8);
    return fgIntense*A_BOLD | (hasColors ? getColorPair(pairKey) : 0);
}

uint NcursesDisplay::getColorPair(uchar pairKey) noexcept
{
    /* Color pairs are defined as they are used, counting from one, in order
     * not to make any assumptions on the amount of color pairs supported by
     * the terminal. */
    int id = pairIdentifiers[pairKey];
    if (id == 0)
    {
        // Foreground color in the lower half, background in the upper half.
        init_pair(++definedPairs, pairKey & 0xF, pairKey >> 4);
        id = pairIdentifiers[pairKey] = definedPairs;
    }
    return COLOR_PAIR(id);
}

} // namespace tvision

#endif // HAVE_NCURSES

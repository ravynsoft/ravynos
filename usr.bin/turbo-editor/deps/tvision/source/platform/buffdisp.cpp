#define Uses_TScreenCell
#include <tvision/tv.h>

#include <internal/dispbuff.h>
#include <internal/platform.h>
#include <internal/codepage.h>
#include <internal/getenv.h>
#include <internal/cursor.h>
#include <chrono>
using std::chrono::microseconds;
using std::chrono::steady_clock;

#ifdef _MSC_VER
#define __builtin_expect(x, y) x
#endif

namespace tvision
{

DisplayBuffer *DisplayBuffer::instance = 0;

DisplayBuffer::DisplayBuffer() noexcept :
    // This could be checked at runtime, but for now this is as much as I know.
#ifdef _WIN32
    wideOverlapping(false)
#else
    wideOverlapping(true)
#endif
{
    instance = this;
    // Check if FPS shall be limited.
    int fps = getEnv<int>("TVISION_MAX_FPS", defaultFPS);
    limitFPS = (fps > 0);
    if (limitFPS)
        flushDelay = microseconds((int) 1e6/fps);
}

DisplayBuffer::~DisplayBuffer()
{
    instance = 0;
}

TScreenCell *DisplayBuffer::reloadScreenInfo(DisplayStrategy &display) noexcept
{
    display.reloadScreenInfo();
    size = display.getScreenSize();
    caretSize = display.getCaretSize();
    resizeBuffer();
    return buffer.data();
}

void DisplayBuffer::resizeBuffer() noexcept
{
    for (auto *buf : {&buffer, &flushBuffer})
    {
        buf->resize(0);
        buf->resize(size.x*size.y); // Zero-initialized.
    }

    rowDamage.resize(0);
    rowDamage.resize(size.y, {INT_MAX, INT_MIN});
}

void DisplayBuffer::clearScreen(DisplayStrategy &display) noexcept
{
    display.clearScreen();
    display.lowlevelFlush();
    resizeBuffer();
}

void DisplayBuffer::redrawScreen(DisplayStrategy &display) noexcept
{
    screenTouched = true;
    lastFlush = {};
    memset(&flushBuffer[0], 0, flushBuffer.size()*sizeof(TScreenCell));
    for (auto &range : rowDamage)
        range = {0, size.x - 1};
    flushScreen(display);
}

void DisplayBuffer::setCaretSize(int size) noexcept
{
    newCaretSize = size;
}

void DisplayBuffer::setCaretPosition(int x, int y) noexcept
{
    caretPosition = {x, y};
    caretMoved = true;
}

void DisplayBuffer::screenWrite(int x, int y, TScreenCell *buf, int len) noexcept
{
    if (inBounds(x, y) && len)
    {
        len = min(len, size.x - x);
        // Since 'buffer' is used as 'TScreen::screenBuffer' and is written to
        // directly, we can avoid a copy operation in most cases.
        if (buf < buffer.data() || buf >= buffer.data() + buffer.size())
            memcpy(&buffer[y*size.x + x], buf, len*sizeof(TScreenCell));

        setDirty(x, y, len);
        screenTouched = true;
    }
}

void DisplayBuffer::setDirty(int x, int y, int len) noexcept
{
    Range dam = rowDamage[y];
    if (x < dam.begin)
        dam.begin = x;
    if (x + len - 1 > dam.end)
        dam.end = x + len - 1;
    rowDamage[y] = dam;
}

bool DisplayBuffer::timeToFlush() noexcept
{
    // Avoid flushing faster than the maximum FPS.
    if (limitFPS)
    {
        auto now = steady_clock::now();
        if (now - lastFlush >= flushDelay)
            lastFlush = now;
        else
            return false;
    }
    return true;
}

void DisplayBuffer::drawCursors() noexcept
{
    for (auto* cursor : cursors)
        if (cursor->isVisible())
        {
            auto pos = cursor->getPos();
            auto x = pos.x, y = pos.y;
            if (inBounds(x, y))
            {
                auto *cell = &buffer[y*size.x + x];
                if ( cell->_ch.isWideCharTrail() &&
                     x > 0 && (cell - 1)->isWide() )
                    --cell, --x;
                cursor->apply(cell->attr);
                setDirty(x, y, 1);
            }
        }
}

void DisplayBuffer::undrawCursors() noexcept
{
    for (const auto* cursor : cursors)
        if (cursor->isVisible())
        {
            auto pos = cursor->getPos();
            auto x = pos.x, y = pos.y;
            if (inBounds(x, y))
            {
                auto *cell = &buffer[y*size.x + x];
                if ( cell->_ch.isWideCharTrail() &&
                     x > 0 && (cell - 1)->isWide() )
                    --cell, --x;
                cursor->restore(cell->attr);
                setDirty(x, y, 1);
            }
        }
}

bool DisplayBuffer::needsFlush() const noexcept
{
    return screenTouched || caretMoved || caretSize != newCaretSize;
}

namespace
{
void flushScreenAlgorithm(DisplayBuffer &, DisplayStrategy &) noexcept;
}

void DisplayBuffer::flushScreen(DisplayStrategy &display) noexcept
{
    if (needsFlush() && timeToFlush())
    {
        drawCursors();
        flushScreenAlgorithm(*this, display);
        if (caretPosition.x != -1)
            display.lowlevelMoveCursor(caretPosition.x, caretPosition.y);
        undrawCursors();
        if (caretSize != newCaretSize)
            display.lowlevelCursorSize(newCaretSize);
        display.lowlevelFlush();
        screenTouched = false;
        caretMoved = false;
        caretSize = newCaretSize;
    }
}

inline void DisplayBuffer::validateCell(TScreenCell &cell) const noexcept
{
    auto &ch = cell._ch;
    if (!ch[1]) // size 1
    {
        uchar c = ch[0];
        if (c == '\0')
            ch[0] = ' ';
        else if (c < ' ' || 0x7F <= c)
            // Translate from codepage as fallback.
            ch.moveInt(CpTranslator::toPackedUtf8(c));
    }
}

//////////////////////////////////////////////////////////////////////////
// FlushScreenAlgorithm

namespace
{

struct FlushScreenAlgorithm
{
    DisplayBuffer &disp;
    DisplayStrategy &display;
    TPoint size;
    TPoint last;
    int x, y, rowOffs;
    TScreenCell *cell;
    DisplayBuffer::Range damage;

    const TScreenCell &cellAt(int x) const noexcept;
    void getCell() noexcept;
    bool cellDirty() const noexcept;
    bool wideCanOverlap() const noexcept;

    void run() noexcept;
    void processCell() noexcept;
    void writeCell() noexcept;
    void writeSpace() noexcept;
    void writeCell(const TCellChar &Char, const TColorAttr &Attr, bool wide) noexcept;
    void commitDirty() noexcept;
    void handleWideCharSpill() noexcept;
    void handleTrail() noexcept;
};

inline bool isTrail(const TScreenCell &cell) noexcept
{
    return __builtin_expect(cell._ch.isWideCharTrail(), 0);
}

inline bool isWide(const TScreenCell &cell) noexcept
{
    return __builtin_expect(cell.isWide(), 0);
}

inline const TScreenCell& FlushScreenAlgorithm::cellAt(int x) const noexcept
{
    return disp.buffer[rowOffs + x];
}

inline void FlushScreenAlgorithm::getCell() noexcept
{
    cell = &disp.buffer[rowOffs + x];
    disp.validateCell(*cell);
}

inline bool FlushScreenAlgorithm::cellDirty() const noexcept
{
    return *cell != disp.flushBuffer[cell - &disp.buffer[0]];
}

inline void FlushScreenAlgorithm::commitDirty() noexcept
{
    disp.flushBuffer[cell - &disp.buffer[0]] = *cell;
}

inline bool FlushScreenAlgorithm::wideCanOverlap() const noexcept
{
    return disp.wideOverlapping;
}

inline void flushScreenAlgorithm(DisplayBuffer &disp, DisplayStrategy &display) noexcept
{
    FlushScreenAlgorithm {disp, display}.run();
}

inline void FlushScreenAlgorithm::run() noexcept
{
    size = disp.size;
    last = {INT_MIN, INT_MIN};
    rowOffs = 0;
    for (y = 0; y < size.y; ++y, rowOffs += size.x)
    {
        damage = disp.rowDamage[y];
        if (damage.begin <= damage.end)
        {
            x = damage.begin;
            // Check for a wide character before the first draw position
            // if overlapping is not allowed.
            bool wideSpillBefore = !wideCanOverlap() && 0 < x && isWide(cellAt(x-1));
            for (; x <= damage.end; ++x)
            {
                getCell();
                if (cellDirty() || isTrail(*cell)) {
                    if (wideSpillBefore) {
                        --x;
                        getCell();
                        wideSpillBefore = false;
                    }
                    commitDirty();
                    processCell();
                } else
                    wideSpillBefore = !wideCanOverlap() && isWide(*cell);
            }
            if (x < size.x) {
                getCell();
                if (isTrail(*cell))
                    // The beginning of a wide character has been overwritten
                    // and attribute spill may happen in the remaining trails.
                    handleTrail();
            }
        }
        disp.rowDamage[y] = {INT_MAX, INT_MIN};
    }
}

inline void FlushScreenAlgorithm::processCell() noexcept
{
    if (isWide(*cell)) {
        handleWideCharSpill();
        return;
    } else if (isTrail(*cell)) {
        handleTrail();
        return;
    }
    writeCell();
}

inline void FlushScreenAlgorithm::writeCell() noexcept
{
    writeCell(cell->_ch, cell->attr, cell->isWide());
}

inline void FlushScreenAlgorithm::writeSpace() noexcept
{
    TCellChar ch;
    ch.moveChar(' ');
    writeCell(ch, cell->attr, 0);
}

inline void FlushScreenAlgorithm::writeCell( const TCellChar &Char,
                                             const TColorAttr &Attr,
                                             bool wide ) noexcept
{
    // 'last' is the last written cell occupied by text.
    // That is, the hardware cursor is located at {last.x + 1, last.y}.

    if (y != last.y)
        display.lowlevelMoveCursor(x, y);
    else if (x != last.x + 1)
        display.lowlevelMoveCursorX(x, y);

    display.lowlevelWriteChars(Char.getText(), Attr);
    last = {x + wide, y};
}

void FlushScreenAlgorithm::handleWideCharSpill() noexcept
{
    uchar width = cell->isWide();
    const auto Attr = cell->attr;
    if (x + width < size.x)
        writeCell();
    else {
        // Replace with spaces if it would otherwise be printed on the next line.
        writeSpace();
        while (--width && ++x < size.x) {
            getCell();
            if (!isTrail(*cell)) {
                --x;
                return;
            }
            writeSpace();
        }
        return;
    }
    // Ensure character does not spill on next cells.
    int wbegin = x;
    while (width-- && ++x < size.x) {
        getCell();
        commitDirty();
        if (!isTrail(*cell)) {
            if (wideCanOverlap()) {
                // Write over the wide character.
                writeCell();
            } else {
                // Replace the whole wide character with spaces.
                int wend = x;
                x = wbegin;
                do {
                    getCell();
                    writeSpace();
                } while (++x < wend);
                getCell();
                writeCell();
            }
            return;
        }
    }
    // If the next character has a different attribute, print it anyway
    // to avoid attribute spill.
    if (x + 1 < size.x) {
        ++x;
        getCell();
        if (Attr != cell->attr) {
            commitDirty();
            processCell();
        } else
            --x;
    }
}

void FlushScreenAlgorithm::handleTrail() noexcept
{
    // Having TCellChar::wideCharTrail in a cell implies wide characters
    // can spill, as the value is otherwise discarded in ensurePrintable().
    const auto Attr = cell->attr;
    if (x > 0) {
        --x;
        getCell();
        // Check the character behind the placeholder.
        if (cell->isWide()) {
            handleWideCharSpill();
            return;
        }
        ++x;
        getCell();
    }
    // Print successive placeholders as spaces.
    do {
        commitDirty();
        writeSpace();
        ++x;
        if (x < size.x)
            getCell();
        else
            return;
    } while (isTrail(*cell));
    // We now got a normal character.
    if (x > damage.end && Attr != cell->attr) {
        // Redraw a character that would otherwise not be printed,
        // to prevent attribute spill.
        processCell();
    } else {
        // Decrease for next iteration
        --x;
    }

}

} // namespace

} // namespace tvision

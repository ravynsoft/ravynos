#include <tvision/tv.h>

#include <internal/codepage.h>
#include <internal/strings.h>
#include <internal/getenv.h>
#include <internal/utf8.h>

namespace tvision
{

constexpr char cp437toUtf8[256][4] =
{
    "\0", "☺", "☻", "♥", "♦", "♣", "♠", "•", "◘", "○", "◙", "♂", "♀", "♪", "♫", "☼",
    "►", "◄", "↕", "‼", "¶", "§", "▬", "↨", "↑", "↓", "→", "←", "∟", "↔", "▲", "▼",
    " ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "⌂",
    "Ç", "ü", "é", "â", "ä", "à", "å", "ç", "ê", "ë", "è", "ï", "î", "ì", "Ä", "Å",
    "É", "æ", "Æ", "ô", "ö", "ò", "û", "ù", "ÿ", "Ö", "Ü", "¢", "£", "¥", "₧", "ƒ",
    "á", "í", "ó", "ú", "ñ", "Ñ", "ª", "º", "¿", "⌐", "¬", "½", "¼", "¡", "«", "»",
    "░", "▒", "▓", "│", "┤", "╡", "╢", "╖", "╕", "╣", "║", "╗", "╝", "╜", "╛", "┐",
    "└", "┴", "┬", "├", "─", "┼", "╞", "╟", "╚", "╔", "╩", "╦", "╠", "═", "╬", "╧",
    "╨", "╤", "╥", "╙", "╘", "╒", "╓", "╫", "╪", "┘", "┌", "█", "▄", "▌", "▐", "▀",
    "α", "ß", "Γ", "π", "Σ", "σ", "µ", "τ", "Φ", "Θ", "Ω", "δ", "∞", "φ", "ε", "∩",
    "≡", "±", "≥", "≤", "⌠", "⌡", "÷", "≈", "°", "∙", "·", "√", "ⁿ", "²", "■", " "
};

constexpr char cp850toUtf8[256][4] =
{
    "\0", "☺", "☻", "♥", "♦", "♣", "♠", "•", "◘", "○", "◙", "♂", "♀", "♪", "♫", "☼",
    "►", "◄", "↕", "‼", "¶", "§", "▬", "↨", "↑", "↓", "→", "←", "∟", "↔", "▲", "▼",
    " ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "⌂",
    "Ç", "ü", "é", "â", "ä", "à", "å", "ç", "ê", "ë", "è", "ï", "î", "ì", "Ä", "Å",
    "É", "æ", "Æ", "ô", "ö", "ò", "û", "ù", "ÿ", "Ö", "Ü", "ø", "£", "Ø", "×", "ƒ",
    "á", "í", "ó", "ú", "ñ", "Ñ", "ª", "º", "¿", "®", "¬", "½", "¼", "¡", "«", "»",
    "░", "▒", "▓", "│", "┤", "Á", "Â", "À", "©", "╣", "║", "╗", "╝", "¢", "¥", "┐",
    "└", "┴", "┬", "├", "─", "┼", "ã", "Ã", "╚", "╔", "╩", "╦", "╠", "═", "╬", "¤",
    "ð", "Ð", "Ê", "Ë", "È", "ı", "Í", "Î", "Ï", "┘", "┌", "█", "▄", "¦", "Ì", "▀",
    "Ó", "ß", "Ô", "Ò", "õ", "Õ", "µ", "þ", "Þ", "Ú", "Û", "Ù", "ý", "Ý", "¯", "´",
    "-", "±", "‗", "¾", "¶", "§", "÷", "¸", "°", "¨", "·", "¹", "³", "²", "■", " "
    // Note that <last row, first column> should be soft hyphen ("\u00AD"), but
    // it is often represented as a regular hyphen.
};

// Provide a default value for 'currentToUtf8' in case it gets used before
// calling 'init()'.
const char (*CpTranslator::currentToUtf8)[256][4] = &cp437toUtf8;
const std::unordered_map<uint32_t, char> *CpTranslator::currentFromUtf8 = nullptr;

static std::unordered_map<uint32_t, char> initMap(const char toUtf8[256][4]) noexcept
{
    std::unordered_map<uint32_t, char> map;
    for (size_t i = 0; i < 256; ++i)
    {
        const char *ch = toUtf8[i];
        size_t length = 1 + Utf8BytesLeft(ch[0]);
        map.emplace(string_as_int<uint32_t>(TStringView(ch, length)), char(i));
    }
    return map;
}

struct CpTable
{
    TStringView cp;
    const char (&toUtf8)[256][4];
    const std::unordered_map<uint32_t, char> fromUtf8;

    CpTable( TStringView cp,
             const char (&toUtf8)[256][4] ) noexcept :
        cp(cp),
        toUtf8(toUtf8),
        fromUtf8(initMap(toUtf8))
    {
    }
};

void CpTranslator::init() noexcept
{
    static const CpTable tables[] =
    {
        CpTable("437", cp437toUtf8),
        CpTable("850", cp850toUtf8),
    };

    static int init = [] ()
    {
        TStringView cp = getEnv<TStringView>("TVISION_CODEPAGE", "437");
        const CpTable *newTable = &tables[0];

        for (const CpTable &table : tables)
            if (table.cp == cp)
            {
                newTable = &table;
                break;
            }

        currentToUtf8 = &newTable->toUtf8;
        currentFromUtf8 = &newTable->fromUtf8;

        (void) init;
        return 0;
    }();
}

char CpTranslator::fromUtf8(TStringView s) noexcept
{
    init();

    auto it = currentFromUtf8->find(string_as_int<uint32_t>(s));
    if (it != currentFromUtf8->end())
        return it->second;
    return 0;
}

} // namespace tvision

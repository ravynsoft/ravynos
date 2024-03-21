#ifndef TURBO_EDITSTATES_H
#define TURBO_EDITSTATES_H

#include <tvision/tv.h>
#include <turbo/styles.h>
#include <turbo/funcview.h>
#include <turbo/scintilla.h>

namespace turbo {

class LineNumbersWidth
{
    int minWidth;
    bool enabled {false};

    int calcWidth(TScintilla &scintilla);

public:

    LineNumbersWidth(int min) :
        minWidth(min)
    {
    }

    inline void setState(bool enable);
    inline void toggle();

    int update(TScintilla &scintilla);
};

inline void LineNumbersWidth::setState(bool enable)
{
    enabled = enable;
}

inline void LineNumbersWidth::toggle()
{
    enabled ^= true;
}

class WrapState
{
    bool enabled {false};
    bool confirmedOnce {false};

public:

    static bool defConfirmWrap(int width);

    // * 'confirmWrap' shall return whether line wrapping should be activated
    //   even if the document is quite large (>= 512 KiB).
    void setState( bool enable, TScintilla &scintilla,
                   TFuncView<bool(int width)> confirmWrap = defConfirmWrap );
    inline void toggle( TScintilla &scintilla,
                        TFuncView<bool(int width)> confirmWrap = defConfirmWrap );
};

inline void WrapState::toggle(TScintilla &scintilla, TFuncView<bool(int width)> confirmWrap)
{
    setState(!enabled, scintilla, confirmWrap);
}

class AutoIndent
{
    bool enabled {true};

public:

    inline void setState(bool enable);
    inline void toggle();

    void applyToCurrentLine(TScintilla &scintilla);
};

inline void AutoIndent::setState(bool enable)
{
    enabled = enable;
}

inline void AutoIndent::toggle()
{
    enabled ^= true;
}

enum SearchDirection : uint8_t
{
    sdForward,
    sdForwardIncremental,
    sdBackwards,
};

enum SearchMode : uint8_t
{
    smPlainText,
    smWholeWords,
    smRegularExpression,
};

enum SearchFlags : uint8_t
{
    sfCaseSensitive = 0x01,
};

struct SearchSettings
{
    SearchMode mode {smPlainText};
    uint8_t flags {0};
};

void search(TScintilla &scintilla, TStringView text, SearchDirection direction, SearchSettings settings);

enum ReplaceMethod : uint8_t
{
    rmReplaceOne,
    rmReplaceAll,
};

void replace(TScintilla &scintilla, TStringView text, TStringView withText, ReplaceMethod method, SearchSettings settings);
void clearIndicator(TScintilla &scintilla, Indicator indicator);

// Updates 'scintilla' so that it makes use of the current state of
// 'lexer' and 'scheme'. If 'scheme' is null, 'schemeDefault' is used instead.
void applyTheming(const LexerSettings *lexer, const ColorScheme *scheme, TScintilla &scintilla);

// Highlights matching braces if there are any.
void updateBraces(const ColorScheme *scheme, TScintilla &scintilla);

// Toggles comment in selected text.
void toggleComment(TScintilla &scintilla, const Language *language);

void stripTrailingSpaces(TScintilla &scintilla);
void ensureNewlineAtEnd(TScintilla &scintilla);

} // namespace turbo

#endif

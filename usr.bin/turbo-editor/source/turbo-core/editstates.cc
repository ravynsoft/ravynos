#define Uses_MsgBox
#include <tvision/tv.h>

#include <turbo/editstates.h>
#include <turbo/scintilla.h>
#include <turbo/scintilla/internals.h>

namespace turbo {

/////////////////////////////////////////////////////////////////////////
// LineNumbersWidth

int LineNumbersWidth::update(TScintilla &scintilla)
{
    int newWidth = enabled ? calcWidth(scintilla) : 0;
    call(scintilla, SCI_SETMARGINWIDTHN, 0, newWidth); // Does nothing if width hasn't changed.
    return newWidth;
}

int LineNumbersWidth::calcWidth(TScintilla &scintilla)
{
    int width = 1;
    size_t lines = call(scintilla, SCI_GETLINECOUNT, 0U, 0U);
    while (lines /= 10)
        ++width;
    if (width < minWidth)
        width = minWidth;
    return width;
}

/////////////////////////////////////////////////////////////////////////
// WrapState

void WrapState::setState(bool enable, TScintilla &scintilla, TFuncView<bool(int)> confirmWrap)
{
    if (!enable)
    {
        auto line = call(scintilla, SCI_GETFIRSTVISIBLELINE, 0U, 0U);
        call(scintilla, SCI_SETWRAPMODE, SC_WRAP_NONE, 0U);
        call(scintilla, SCI_SETFIRSTVISIBLELINE, line, 0U);
        enabled = false;
    }
    else
    {
        bool proceed = true;
        int size = call(scintilla, SCI_GETLENGTH, 0U, 0U);
        bool documentBig = size >= (1 << 19);
        if (documentBig && !confirmedOnce)
        {
            int width = call(scintilla, SCI_GETSCROLLWIDTH, 0U, 0U);
            proceed = confirmedOnce = confirmWrap(width);
        }
        if (proceed)
        {
            call(scintilla, SCI_SETWRAPMODE, SC_WRAP_WORD, 0U);
            enabled = true;
        }
    }
}

bool WrapState::defConfirmWrap(int width)
{
    return cmYes == messageBox( mfInformation | mfYesButton | mfNoButton,
                                "This document is quite large and the longest of its lines is at least %d characters long.\nAre you sure you want to enable line wrapping?", width );
}

/////////////////////////////////////////////////////////////////////////
// AutoIndent

void AutoIndent::applyToCurrentLine(TScintilla &scintilla)
{
    if (enabled)
    {
        auto pos = call(scintilla, SCI_GETCURRENTPOS, 0U, 0U);
        auto line = call(scintilla, SCI_LINEFROMPOSITION, pos, 0U);
        if (line > 0)
        {
            auto indentation = call(scintilla, SCI_GETLINEINDENTATION, line - 1, 0U);
            if (indentation > 0)
            {
                call(scintilla, SCI_SETLINEINDENTATION, line, indentation);
                call(scintilla, SCI_VCHOME, 0U, 0U);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////
// Search

static void initSearchFlags(TScintilla &scintilla, SearchSettings settings)
{
    int searchFlags =
          (-(settings.mode == smWholeWords) & SCFIND_WHOLEWORD)
        | (-(settings.mode == smRegularExpression) & (SCFIND_REGEXP | SCFIND_CXX11REGEX))
        | (-!!(settings.flags & sfCaseSensitive) & SCFIND_MATCHCASE)
        ;
    call(scintilla, SCI_SETSEARCHFLAGS, searchFlags, 0U);
}

static void initSearchTarget(TScintilla &scintilla, SearchDirection direction)
{
    Sci::Position selStart = call(scintilla, SCI_GETSELECTIONSTART, 0U, 0U);
    Sci::Position selEnd = call(scintilla, SCI_GETSELECTIONEND, 0U, 0U);
    Sci::Position targetStart, targetEnd;
    if (direction == sdForward)
        targetStart = selEnd;
    else
        targetStart = selStart;
    if (direction == sdBackwards)
        targetEnd = 0;
    else
        targetEnd = call(scintilla, SCI_GETTEXTLENGTH, 0U, 0U);
    call(scintilla, SCI_SETTARGETRANGE, targetStart, targetEnd);
}

static void wrapSearchTarget(TScintilla &scintilla)
{
    sptr_t docEnd = call(scintilla, SCI_GETTEXTLENGTH, 0U, 0U);
    sptr_t targetStart = call(scintilla, SCI_GETTARGETSTART, 0U, 0U);
    sptr_t targetEnd = call(scintilla, SCI_GETTARGETEND, 0U, 0U);
    call(scintilla, SCI_SETTARGETRANGE, docEnd - targetEnd, targetStart);
}

static bool searchInTarget(TScintilla &scintilla, TStringView s, SearchDirection direction, bool select = true)
{
    sptr_t result = call(scintilla, SCI_SEARCHINTARGET, s.size(), (sptr_t) s.data());
    if (result != -1)
    {
        sptr_t resultEnd = call(scintilla, SCI_GETTARGETEND, 0U, 0U);
        if (select)
            call(scintilla, SCI_SETSEL, result, resultEnd);
        return true;
    }
    else if (direction == sdForwardIncremental)
    {
        sptr_t cur = call(scintilla, SCI_GETCURRENTPOS, 0U, 0U);
        if (select)
            call(scintilla, SCI_SETEMPTYSELECTION, cur, 0U);
    }
    return false;
}

static void searchInTargetOrWrap(TScintilla &scintilla, TStringView s, SearchDirection direction)
{
    if (!searchInTarget(scintilla, s, direction) && direction != sdForwardIncremental)
    {
        wrapSearchTarget(scintilla);
        searchInTarget(scintilla, s, direction);
    }
}

void search(TScintilla &scintilla, TStringView text, SearchDirection direction, SearchSettings settings)
{
    if (!text.empty())
    {
        initSearchFlags(scintilla, settings);
        initSearchTarget(scintilla, direction);
        searchInTargetOrWrap(scintilla, text, direction);
    }
}

static bool selectionMatches(TScintilla &scintilla, TStringView text)
{
    Sci::Position selStart = call(scintilla, SCI_GETSELECTIONSTART, 0U, 0U);
    Sci::Position selEnd = call(scintilla, SCI_GETSELECTIONEND, 0U, 0U);
    call(scintilla, SCI_TARGETFROMSELECTION, 0U, 0U);
    Sci::Position targetStart = call(scintilla, SCI_SEARCHINTARGET, text.size(), (sptr_t) text.data());
    return selStart == targetStart &&
           selEnd == call(scintilla, SCI_GETTARGETEND, 0U, 0U);
}

void clearIndicator(TScintilla &scintilla, Indicator indicator)
{
    call(scintilla, SCI_SETINDICATORCURRENT, indicator, 0U);
    call(scintilla, SCI_INDICATORCLEARRANGE, 0, call(scintilla, SCI_GETTEXTLENGTH, 0U, 0U));
}

static void fillTargetWithIndicator(TScintilla &scintilla, Indicator indicator)
{
    Sci::Position targetStart = call(scintilla, SCI_GETTARGETSTART, 0U, 0U);
    Sci::Position targetEnd = call(scintilla, SCI_GETTARGETEND, 0U, 0U);
    call(scintilla, SCI_SETINDICATORCURRENT, indicator, 0U);
    call(scintilla, SCI_INDICATORFILLRANGE, targetStart, targetEnd - targetStart);
}

static void replaceSelectionAndMoveCaret(TScintilla &scintilla, TStringView withText)
{
    call(scintilla, SCI_TARGETFROMSELECTION, 0U, 0U);
    call(scintilla, SCI_REPLACETARGET, withText.size(), (sptr_t) withText.data());
    Sci::Position resultEnd = call(scintilla, SCI_GETTARGETEND, 0U, 0U);
    call(scintilla, SCI_GOTOPOS, resultEnd, 0U);
}

static void initReplaceOneSearchTarget(TScintilla &scintilla)
{
    Sci::Position targetStart = call(scintilla, SCI_GETSELECTIONSTART, 0U, 0U);
    Sci::Position targetEnd = call(scintilla, SCI_GETTEXTLENGTH, 0U, 0U);
    call(scintilla, SCI_SETTARGETRANGE, targetStart, targetEnd);
}

static void targetWholeDocument(TScintilla &scintilla)
{
    call(scintilla, SCI_TARGETWHOLEDOCUMENT, 0U, 0U);
}

static void targetUntilEnd(TScintilla &scintilla)
{
    Sci::Position targetStart = call(scintilla, SCI_GETTARGETEND, 0U, 0U);
    Sci::Position targetEnd = call(scintilla, SCI_GETTEXTLENGTH, 0U, 0U);
    call(scintilla, SCI_SETTARGETRANGE, targetStart, targetEnd);
}

static void replaceTarget(TScintilla &scintilla, TStringView withText)
{
    call(scintilla, SCI_REPLACETARGET, withText.size(), (sptr_t) withText.data());
}

void replace(TScintilla &scintilla, TStringView text, TStringView withText, ReplaceMethod method, SearchSettings settings)
{
    if (!text.empty())
    {
        call(scintilla, SCI_BEGINUNDOACTION, 0U, 0U);
        clearIndicator(scintilla, idtrReplaceHighlight);
        initSearchFlags(scintilla, settings);
        if (method == rmReplaceOne)
        {
            if (selectionMatches(scintilla, text))
            {
                replaceSelectionAndMoveCaret(scintilla, withText);
                fillTargetWithIndicator(scintilla, idtrReplaceHighlight);
            }
            initReplaceOneSearchTarget(scintilla);
            searchInTargetOrWrap(scintilla, text, sdForward);
        }
        else if (method == rmReplaceAll)
        {
            targetWholeDocument(scintilla);
            while (searchInTarget(scintilla, text, sdForward, false))
            {
                replaceTarget(scintilla, withText);
                fillTargetWithIndicator(scintilla, idtrReplaceHighlight);
                targetUntilEnd(scintilla);
            }
        }
        call(scintilla, SCI_ENDUNDOACTION, 0U, 0U);
    }
}

/////////////////////////////////////////////////////////////////////////
// Comment toggling

static bool removeComment(TScintilla &, const Language &);
static bool removeBlockComment(TScintilla &, const Language &);
static Sci::Position getSelectionEndSkippingEmptyLastLine(TScintilla &, Sci::Position);
static void getLineStartAndEnd(TScintilla &, Sci::Position &, Sci::Position &);
static size_t findCommentAtStart(TStringView, TStringView);
static size_t findCommentAtEnd(TStringView, TStringView);
static bool removeLineComments(TScintilla &, const Language &);
static bool noLinesBeginWithoutLineComment(TScintilla &, const Language &, Sci::Line, Sci::Line);
static void removeLineCommentFromLine(TScintilla &, const Language &, Sci::Line);
static void insertComment(TScintilla &, const Language &);
static bool thereIsTextBeforeOrAfterSelection(TScintilla &);
static void insertBlockComment(TScintilla &, const Language &);
static void restoreSelection(TScintilla &, Sci::Position, Sci::Position, Sci::Position, size_t, size_t);
static void insertLineComments(TScintilla &, const Language &);
static size_t minIndentationInLines(TScintilla &, Sci::Line, Sci::Line);
static size_t insertLineCommentIntoLine(TScintilla &, const Language &, Sci::Line, size_t);

void toggleComment(TScintilla &scintilla, const Language *language)
{
    if (language && (language->hasLineComments() || language->hasBlockComments()))
    {
        if (!removeComment(scintilla, *language))
            insertComment(scintilla, *language);
        call(scintilla, SCI_SCROLLCARET, 0U, 0U);
    }
}

static bool removeComment(TScintilla &scintilla, const Language &language)
{
    return removeBlockComment(scintilla, language)
        || removeLineComments(scintilla, language);
}

static bool removeBlockComment(TScintilla &scintilla, const Language &language)
{
    if (language.hasBlockComments())
    {
        Sci::Position posStart = call(scintilla, SCI_GETSELECTIONSTART, 0U, 0U);
        Sci::Position posEnd = getSelectionEndSkippingEmptyLastLine(scintilla, posStart);
        if (posStart == posEnd)
            getLineStartAndEnd(scintilla, posStart, posEnd);
        TStringView text = getRangePointer(scintilla, posStart, posEnd);

        size_t openStart = findCommentAtStart(text, language.blockCommentOpen);
        if (openStart < text.size())
        {
            size_t closeStart = findCommentAtEnd(text, language.blockCommentClose);
            if (closeStart < text.size())
            {
                size_t openSize = language.blockCommentOpen.size();
                size_t closeSize = language.blockCommentClose.size();
                call(scintilla, SCI_BEGINUNDOACTION, 0U, 0U);
                call(scintilla, SCI_DELETERANGE, posStart + openStart, openSize);
                call(scintilla, SCI_DELETERANGE, posStart + closeStart - openSize, closeSize);
                call(scintilla, SCI_ENDUNDOACTION, 0U, 0U);
                return true;
            }
        }
    }
    return false;
}

static Sci::Position getSelectionEndSkippingEmptyLastLine(TScintilla &scintilla, Sci::Position selStart)
{
    Sci::Position selEnd = call(scintilla, SCI_GETSELECTIONEND, 0U, 0U);
    if (selStart < selEnd)
    {
        Sci::Line line = call(scintilla, SCI_LINEFROMPOSITION, selEnd, 0U);
        Sci::Line prevPosLine = call(scintilla, SCI_LINEFROMPOSITION, selEnd - 1, 0U);
        if (prevPosLine < line)
            return call(scintilla, SCI_GETLINEENDPOSITION, prevPosLine, 0U);
    }
    return selEnd;
}

static void getLineStartAndEnd(TScintilla &scintilla, Sci::Position &posStart, Sci::Position &posEnd)
{
    Sci::Line line = call(scintilla, SCI_LINEFROMPOSITION, posStart, 0U);
    posStart = call(scintilla, SCI_POSITIONFROMLINE, line, 0U);
    posEnd = call(scintilla, SCI_GETLINEENDPOSITION, line, 0U);
}

static size_t findCommentAtStart(TStringView text, TStringView comment)
{
    size_t i = 0;
    while (i < text.size() && Scintilla::IsSpaceOrTab(text[i]))
        ++i;
    size_t j = 0;
    while (j < comment.size())
        if (!(i < text.size() && text[i++] == comment[j++]))
            return text.size();
    return i - comment.size();
}

static size_t findCommentAtEnd(TStringView text, TStringView comment)
{
    size_t i = text.size();
    while (i > 0 && Scintilla::IsSpaceOrTab(text[i - 1]))
        --i;
    size_t j = comment.size();
    while (j > 0)
        if (!(i > 0 && text[--i] == comment[--j]))
            return text.size();
    return i;
}

static bool removeLineComments(TScintilla &scintilla, const Language &language)
{
    if (language.hasLineComments())
    {
        Sci::Position selStart = call(scintilla, SCI_GETSELECTIONSTART, 0U, 0U);
        Sci::Position selEnd = getSelectionEndSkippingEmptyLastLine(scintilla, selStart);
        Sci::Line firstLine = call(scintilla, SCI_LINEFROMPOSITION, selStart, 0U);
        Sci::Line lastLine = call(scintilla, SCI_LINEFROMPOSITION, selEnd, 0U);

        if (noLinesBeginWithoutLineComment(scintilla, language, firstLine, lastLine))
        {
            call(scintilla, SCI_BEGINUNDOACTION, 0U, 0U);
            for (Sci::Line line = firstLine; line <= lastLine; ++line)
                removeLineCommentFromLine(scintilla, language, line);
            call(scintilla, SCI_ENDUNDOACTION, 0U, 0U);
            return true;
        }
    }
    return false;
}

static bool noLinesBeginWithoutLineComment(TScintilla &scintilla, const Language &language, Sci::Line firstLine, Sci::Line lastLine)
{
    bool atLeastOneIsNotEmpty = false;
    for (Sci::Line line = firstLine; line <= lastLine; ++line)
    {
        Sci::Position lineStart = call(scintilla, SCI_POSITIONFROMLINE, line, 0U);
        Sci::Position lineEnd = call(scintilla, SCI_GETLINEENDPOSITION, line, 0U);
        TStringView text = getRangePointer(scintilla, lineStart, lineEnd);
        if (!text.empty() && text.size() == findCommentAtStart(text, language.lineComment))
            return false;
        else if (!text.empty())
            atLeastOneIsNotEmpty = true;
    }
    return atLeastOneIsNotEmpty;
}

static void removeLineCommentFromLine(TScintilla &scintilla, const Language &language, Sci::Line line)
// Pre: 'language.lineComment' is not empty.
{
    Sci::Position lineStart = call(scintilla, SCI_POSITIONFROMLINE, line, 0U);
    Sci::Position lineEnd = call(scintilla, SCI_GETLINEENDPOSITION, line, 0U);
    TStringView text = getRangePointer(scintilla, lineStart, lineEnd);
    TStringView comment = language.lineComment;
    size_t commentStart = findCommentAtStart(text, comment);
    if (commentStart < text.size())
    {
        size_t commentEnd = commentStart + comment.size();
        if (comment.back() != ' ' && commentEnd < text.size() && text[commentEnd] == ' ')
            commentEnd += 1;
        call(scintilla, SCI_DELETERANGE, lineStart + commentStart, commentEnd - commentStart);
    }
}

static void insertComment(TScintilla &scintilla, const Language &language)
// Pre: language supports at least one kind of comment.
{
    if ( !language.hasLineComments()
         || (language.hasBlockComments() && thereIsTextBeforeOrAfterSelection(scintilla)) )
        insertBlockComment(scintilla, language);
    else
        insertLineComments(scintilla, language);
}

bool thereIsTextBeforeOrAfterSelection(TScintilla &scintilla)
{
    Sci::Position selStart = call(scintilla, SCI_GETSELECTIONSTART, 0U, 0U);
    Sci::Position selEnd = getSelectionEndSkippingEmptyLastLine(scintilla, selStart);
    if (selStart < selEnd)
    {
        Sci::Line firstLine = call(scintilla, SCI_LINEFROMPOSITION, selStart, 0U);
        Sci::Position firstLineStart = call(scintilla, SCI_POSITIONFROMLINE, firstLine, 0U);
        TStringView textBefore = getRangePointer(scintilla, firstLineStart, selStart);
        for (char c : textBefore)
            if (!Scintilla::IsSpaceOrTab(c))
                return true;
        Sci::Line lastLine = call(scintilla, SCI_LINEFROMPOSITION, selEnd, 0U);
        Sci::Position lastLineEnd = call(scintilla, SCI_GETLINEENDPOSITION, lastLine, 0U);
        TStringView textAfter = getRangePointer(scintilla, selEnd, lastLineEnd);
        for (char c : textAfter)
            if (!Scintilla::IsSpaceOrTab(c))
                return true;
    }
    return false;
}

static void insertBlockComment(TScintilla &scintilla, const Language &language)
// Pre: language.hasBlockComments()
{
    Sci::Position caret = call(scintilla, SCI_GETCURRENTPOS, 0U, 0U);
    Sci::Position anchor = call(scintilla, SCI_GETANCHOR, 0U, 0U);
    Sci::Position posStart = call(scintilla, SCI_GETSELECTIONSTART, 0U, 0U);
    Sci::Position posEnd = getSelectionEndSkippingEmptyLastLine(scintilla, posStart);
    if (posStart == posEnd)
        getLineStartAndEnd(scintilla, posStart, posEnd);

    call(scintilla, SCI_BEGINUNDOACTION, 0U, 0U);
    call(scintilla, SCI_INSERTTEXT, posEnd, (sptr_t) std::string(language.blockCommentClose).c_str());
    call(scintilla, SCI_INSERTTEXT, posStart, (sptr_t) std::string(language.blockCommentOpen).c_str());
    restoreSelection(scintilla, caret, anchor, posStart, language.blockCommentOpen.size(), language.blockCommentClose.size());
    call(scintilla, SCI_ENDUNDOACTION, 0U, 0U);
}

static void restoreSelection(TScintilla &scintilla, Sci::Position caret, Sci::Position anchor, Sci::Position prefixInsertPos, size_t prefixLength, size_t suffixLength)
{
    Sci::Position &selStart = caret < anchor ? caret : anchor;
    Sci::Position &selEnd = caret < anchor ? anchor : caret;
    if (caret == anchor)
    {
        selStart += prefixLength;
        selEnd = selStart;
    }
    else
    {
        if (prefixInsertPos < selStart)
            selStart += prefixLength;
        selEnd += prefixLength + suffixLength;
    }
    call(scintilla, SCI_SETSEL, anchor, caret);
}

static void insertLineComments(TScintilla &scintilla, const Language &language)
// Pre: language.hasLineComments()
{
    Sci::Position caret = call(scintilla, SCI_GETCURRENTPOS, 0U, 0U);
    Sci::Position anchor = call(scintilla, SCI_GETANCHOR, 0U, 0U);
    Sci::Position posStart = call(scintilla, SCI_GETSELECTIONSTART, 0U, 0U);
    Sci::Position posEnd = getSelectionEndSkippingEmptyLastLine(scintilla, posStart);
    Sci::Line firstLine = call(scintilla, SCI_LINEFROMPOSITION, posStart, 0U);
    Sci::Line lastLine = call(scintilla, SCI_LINEFROMPOSITION, posEnd, 0U);
    Sci::Position firstLineStart = call(scintilla, SCI_POSITIONFROMLINE, firstLine, 0U);

    size_t indentation = minIndentationInLines(scintilla, firstLine, lastLine);
    call(scintilla, SCI_BEGINUNDOACTION, 0U, 0U);
    size_t insertLength = 0;
    for (Sci::Line line = firstLine; line <= lastLine; ++line)
        insertLength += insertLineCommentIntoLine(scintilla, language, line, indentation);
    restoreSelection(scintilla, caret, anchor, firstLineStart + indentation, insertLength, 0);
    call(scintilla, SCI_ENDUNDOACTION, 0U, 0U);
}

static size_t minIndentationInLines(TScintilla &scintilla, Sci::Line firstLine, Sci::Line lastLine)
{
    size_t result = SIZE_MAX;
    for (Sci::Line line = firstLine; line <= lastLine; ++line)
    {
        Sci::Position lineStart = call(scintilla, SCI_POSITIONFROMLINE, line, 0U);
        Sci::Position lineEnd = call(scintilla, SCI_GETLINEENDPOSITION, line, 0U);
        TStringView text = getRangePointer(scintilla, lineStart, lineEnd);
        if (!text.empty())
        {
            size_t i = 0;
            while (i < text.size() && Scintilla::IsSpaceOrTab(text[i]))
                ++i;
            if (i != text.size())
                result = min(i, result);
        }
    }
    return result == SIZE_MAX ? 0 : result;
}

static size_t insertLineCommentIntoLine(TScintilla &scintilla, const Language &language, Sci::Line line, size_t indentation)
{
    std::string comment {language.lineComment};
    if (comment.back() != ' ')
        comment.push_back(' ');
    size_t insertLength = 0;
    Sci::Position lineStart = call(scintilla, SCI_POSITIONFROMLINE, line, 0U);
    Sci::Position lineEnd = call(scintilla, SCI_GETLINEENDPOSITION, line, 0U);
    if (lineStart == lineEnd && indentation > 0)
    {
        call(scintilla, SCI_INSERTTEXT, lineStart, (sptr_t) std::string(indentation, ' ').c_str());
        insertLength += indentation;
    }
    call(scintilla, SCI_INSERTTEXT, lineStart + indentation, (sptr_t) comment.c_str());
    insertLength += comment.size();
    return insertLength;
}

/////////////////////////////////////////////////////////////////////////

void applyTheming(const LexerSettings *lexer, const ColorScheme *aScheme, TScintilla &scintilla)
{
    auto &scheme = aScheme ? *aScheme : schemeDefault;
    setStyleColor(scintilla, STYLE_DEFAULT, scheme[sNormal]);
    call(scintilla, SCI_STYLECLEARALL, 0U, 0U); // Must be done before setting other colors.
    setSelectionColor(scintilla, scheme[sSelection]);
    setWhitespaceColor(scintilla, scheme[sWhitespace]);
    setStyleColor(scintilla, STYLE_CONTROLCHAR, normalize(scheme, sCtrlChar));
    setStyleColor(scintilla, STYLE_LINENUMBER, normalize(scheme, sLineNums));
    setIndicatorColor(scintilla, idtrReplaceHighlight, scheme[sReplaceHighlight]);
    if (lexer)
    {
        call(scintilla, SCI_SETLEXER, lexer->id, 0U);
        for (const auto &s : lexer->styles)
            setStyleColor(scintilla, s.id, normalize(scheme, s.style));
        for (const auto &k : lexer->keywords)
            call(scintilla, SCI_SETKEYWORDS, k.id, (sptr_t) k.keywords);
        for (const auto &p : lexer->properties)
            call(scintilla, SCI_SETPROPERTY, (sptr_t) p.name, (sptr_t) p.value);
    }
    else
        call(scintilla, SCI_SETLEXER, SCLEX_CONTAINER, 0U);
    call(scintilla, SCI_COLOURISE, 0, -1);
}

static bool isBrace(char ch)
{
    TStringView braces = "[](){}";
    return memchr(braces.data(), ch, braces.size()) != nullptr;
}

void updateBraces(const ColorScheme *aScheme, TScintilla &scintilla)
{
    auto pos = call(scintilla, SCI_GETCURRENTPOS, 0U, 0U);
    auto ch = call(scintilla, SCI_GETCHARAT, pos, 0U);
    bool braceFound = false;
    if (isBrace(ch))
    {
        // Scintilla already makes sure that both braces have the same style.
        auto matchPos = call(scintilla, SCI_BRACEMATCH, pos, 0U);
        if (matchPos != -1)
        {
            auto &scheme = aScheme ? *aScheme : schemeDefault;
            auto style = call(scintilla, SCI_GETSTYLEAT, pos, 0U);
            auto curAttr = getStyleColor(scintilla, style);
            auto braceAttr = coalesce(scheme[sBraceMatch], curAttr);
            setStyleColor(scintilla, STYLE_BRACELIGHT, braceAttr);
            call(scintilla, SCI_BRACEHIGHLIGHT, pos, matchPos);
            braceFound = true;
        }
    }
    if (!braceFound)
        call(scintilla, SCI_BRACEHIGHLIGHT, -1, -1);
}

void stripTrailingSpaces(TScintilla &scintilla)
{
    Sci::Line lineCount = call(scintilla, SCI_GETLINECOUNT, 0U, 0U);
    for (Sci::Line line = 0; line < lineCount; ++line) {
        Sci::Position lineStart = call(scintilla, SCI_POSITIONFROMLINE, line, 0U);
        Sci::Position lineEnd = call(scintilla, SCI_GETLINEENDPOSITION, line, 0U);
        Sci::Position i;
        for (i = lineEnd - 1; i >= lineStart; --i) {
            char ch = call(scintilla, SCI_GETCHARAT, i, 0U);
            if (ch != ' ' && ch != '\t')
                break;
        }
        if (i != lineEnd - 1) { // Not first iteration, trailing whitespace.
            call(scintilla, SCI_SETTARGETRANGE, i + 1, lineEnd);
            call(scintilla, SCI_REPLACETARGET, 0, (sptr_t) "");
        }
    }
}

void ensureNewlineAtEnd(TScintilla &scintilla)
{
    int EOLType = call(scintilla, SCI_GETEOLMODE, 0U, 0U);
    Sci::Line lineCount = call(scintilla, SCI_GETLINECOUNT, 0U, 0U);
    Sci::Position docEnd = call(scintilla, SCI_POSITIONFROMLINE, lineCount, 0U);
    if ( lineCount == 1 || (lineCount > 1 &&
         docEnd > call(scintilla, SCI_POSITIONFROMLINE, lineCount - 1, 0U)) )
    {
        std::string_view EOL = (EOLType == SC_EOL_CRLF) ? "\r\n" :
                               (EOLType == SC_EOL_CR)   ? "\r"   :
                                                          "\n";
        call(scintilla, SCI_APPENDTEXT, EOL.size(), (sptr_t) EOL.data());
    }
}

} // namespace turbo

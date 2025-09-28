#include <gtest/gtest.h>

#include <test/test.h>
#include <sstream>

namespace turbo
{

TextState TextState::decode(std::string_view input)
{
    size_t caret = input.find(chCaret);
    if (caret == std::string_view::npos)
    {
        std::ostringstream os;
        os << "Input text does not have a caret: '" << input << "'";
        throw std::runtime_error(os.str());
    }
    size_t anchor = input.find(chAnchor);
    if (anchor == std::string_view::npos)
        anchor = caret;
    else if (anchor < caret)
        caret -= 1;
    else
        anchor -= 1;
    auto &&text = std::string(input);
    text.erase(
        std::remove_if(text.begin(), text.end(), [] (auto &c) { return c == chCaret || c == chAnchor; }),
        text.end()
    );
    return {
        std::move(text),
        Sci::Position(caret),
        Sci::Position(anchor),
    };
}

std::string TextState::encode(TextState self)
{
    self.text.insert(self.caret, 1, chCaret);
    if (self.anchor != -1 && self.anchor != self.caret)
        self.text.insert(
            self.anchor + (self.caret < self.anchor),
            1,
            chAnchor
        );
    return std::move(self.text);
}

TScintilla &createScintilla(TextState state)
{
    auto &scintilla = createScintilla();
    call(scintilla, SCI_SETTEXT, 0U, (sptr_t) state.text.c_str());
    call(scintilla, SCI_SETSEL, state.anchor, state.caret);
    return scintilla;
}

TextState getTextState(TScintilla &scintilla)
{
    Sci::Position length = call(scintilla, SCI_GETLENGTH, 0U, 0U);
    std::string text(length, char());
    call(scintilla, SCI_GETTEXT, length + 1, (sptr_t) text.data());
    return {
        std::move(text),
        (Sci::Position) call(scintilla, SCI_GETCURRENTPOS, 0U, 0U),
        (Sci::Position) call(scintilla, SCI_GETANCHOR, 0U, 0U),
    };
}

} // namespace turbo

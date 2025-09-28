#include <turbo/scintilla/internals.h>

namespace Scintilla {

Font::Font() noexcept :
    fid(nullptr)
{
}

Font::~Font()
{
}

void Font::Create(const FontParameters &fp)
{
    Release();
    // Store attributes in 'fid' field.
    fid = (void *)(size_t)(unsigned) fp.weight;
}

void Font::Release()
{
    fid = nullptr;
}

} // namespace Scintilla

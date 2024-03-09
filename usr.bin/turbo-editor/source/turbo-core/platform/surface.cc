#define Uses_TText
#define Uses_TDrawSurface
#include <tvision/tv.h>

#include "surface.h"
#include <turbo/scintilla.h>

namespace Scintilla {

Surface *Surface::Allocate(int technology)
{
    return new TScintillaSurface;
}

void TScintillaSurface::Init(WindowID wid)
{
}

void TScintillaSurface::Init(SurfaceID sid, WindowID wid)
{
}

void TScintillaSurface::InitPixMap(int width, int height, Surface *surface_, WindowID wid)
{
    // We get the actual TDrawSurface object we need to draw on from the 'surface_' parameter,
    // which points to the TScintillaSurface object created in ScintillaEditor::paint().
    surface = ((TScintillaSurface *) surface_)->surface;
    defaultTextAttr = ((TScintillaSurface *) surface_)->defaultTextAttr;
}

void TScintillaSurface::Release()
{
    surface = nullptr;
    defaultTextAttr = {};
}

bool TScintillaSurface::Initialised()
{
    return surface;
}

void TScintillaSurface::PenColour(ColourDesired fore)
{
}

int TScintillaSurface::LogPixelsY()
{
    return 1;
}

int TScintillaSurface::DeviceHeightFont(int points)
{
    return 1;
}

void TScintillaSurface::MoveTo(int x_, int y_)
{
}

void TScintillaSurface::LineTo(int x_, int y_)
{
}

void TScintillaSurface::Polygon(Point *pts, size_t npts, ColourDesired fore, ColourDesired back)
{
}

void TScintillaSurface::RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back)
{
}

void TScintillaSurface::FillRectangle(PRectangle rc, ColourDesired back)
{
    auto r = clipRect(rc);
    if ( surface && 0 <= r.a.x && r.a.x < r.b.x
                 && 0 <= r.a.y && r.a.y < r.b.y )
    {
        // Used to draw text selections and areas without text. The foreground color
        // also needs to be set or else the cursor will have the wrong color when
        // placed on this area.
        auto attr = defaultTextAttr;
        ::setBack(attr, convertColor(back));
        auto *cells = &surface->at(r.a.y, r.a.x);
        size_t count = r.b.x - r.a.x;
        for (int y = r.a.y; y < r.b.y; ++y)
        {
            TText::drawChar({cells, count}, ' ', attr);
            cells += surface->size.x;
        }
    }
}

void TScintillaSurface::FillRectangle(PRectangle rc, Surface &surfacePattern)
{
    FillRectangle(rc, ColourDesired());
}

void TScintillaSurface::RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back)
{
}

void TScintillaSurface::AlphaRectangle( PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
                                        ColourDesired outline, int alphaOutline, int flags )
{
    auto r = clipRect(rc);
    if ( surface && 0 <= r.a.x && r.a.x < r.b.x
                 && 0 <= r.a.y && r.a.y < r.b.y )
    {
        auto fg = convertColor(ColourDesired(alphaOutline)),
             bg = convertColor(fill);
        auto *cells = &surface->at(r.a.y, r.a.x);
        size_t count = r.b.x - r.a.x;
        for (int y = r.a.y; y < r.b.y; ++y)
        {
            for (size_t x = 0; x < count; ++x)
            {
                if (!fg.isDefault())
                    ::setFore(cells[x].attr, fg);
                if (!bg.isDefault())
                    ::setBack(cells[x].attr, bg);
            }
            cells += surface->size.x;
        }
    }
}

void TScintillaSurface::GradientRectangle(PRectangle rc, const std::vector<ColourStop> &stops, GradientOptions options)
{
}

void TScintillaSurface::DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage)
{
}

void TScintillaSurface::Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back)
{
}

void TScintillaSurface::Copy(PRectangle rc, Point from, Surface &surfaceSource)
{
}

std::unique_ptr<IScreenLineLayout> TScintillaSurface::Layout(const IScreenLine *screenLine)
{
    return nullptr;
}

void TScintillaSurface::DrawTextNoClip( PRectangle rc, Font &font_,
                                        XYPOSITION ybase, std::string_view text,
                                        ColourDesired fore, ColourDesired back )
{
    if (surface)
    {
        auto lastClip = clip;
        clip = {0, 0, surface->size.x, surface->size.y};
        DrawTextClipped(rc, font_, ybase, text, fore, back);
        clip = lastClip;
    }
}

void TScintillaSurface::DrawTextClipped( PRectangle rc, Font &font_,
                                         XYPOSITION ybase, std::string_view text,
                                         ColourDesired fore, ColourDesired back )
{
    auto r = clipRect(rc);
    if ( surface && 0 <= r.a.x && r.a.x < r.b.x
                 && 0 <= r.a.y && r.a.y < r.b.y )
    {
        auto attr = convertColorPair(fore, back);
        ::setStyle(attr, getStyle(font_));
        auto *cells = &surface->at(r.a.y, r.a.x);
        size_t count = r.b.x - r.a.x;
        int indent = clip.a.x - (int) rc.left;
        for (int y = r.a.y; y < r.b.y; ++y)
        {
            TText::drawStr({cells, count}, 0, text, indent, attr);
            cells += surface->size.x;
        }
    }
}

void TScintillaSurface::DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore)
{
    auto r = clipRect(rc);
    if ( surface && 0 <= r.a.x && r.a.x < r.b.x
                 && 0 <= r.a.y && r.a.y < r.b.y )
    {
        auto fg = convertColor(fore);
        auto style = getStyle(font_);
        TScreenCell *cells = &surface->at(r.a.y, r.a.x);
        size_t count = r.b.x - r.a.x;
        int indent = clip.a.x - (int) rc.left;
        for (int y = r.a.y; y < r.b.y; ++y)
        {
            TText::drawStrEx({cells, count}, 0, text, indent, [&] (auto &attr) {
                ::setFore(attr, fg);
                ::setStyle(attr, style);
            });
            cells += surface->size.x;
        }
    }
}

void TScintillaSurface::MeasureWidths(Font &font_, std::string_view text, XYPOSITION *positions)
{
    size_t i = 0, j = 1;
    while (i < text.size()) {
        size_t width = 0, k = i;
        TText::next(text, i, width);
        // I don't know why. It just works.
        j += width - 1;
        while (k < i)
            positions[k++] = (int) j;
        ++j;
    }
}

XYPOSITION TScintillaSurface::WidthText(Font &font_, std::string_view text)
{
    return strwidth(text);
}

XYPOSITION TScintillaSurface::Ascent(Font &font_)
{
    return 0;
}

XYPOSITION TScintillaSurface::Descent(Font &font_)
{
    return 0;
}

XYPOSITION TScintillaSurface::InternalLeading(Font &font_)
{
    return 0;
}

XYPOSITION TScintillaSurface::Height(Font &font_)
{
    return 1;
}

XYPOSITION TScintillaSurface::AverageCharWidth(Font &font_)
{
    return 1;
}

void TScintillaSurface::SetClip(PRectangle rc)
{
    clip = rc;
    if (surface)
        clip.intersect({0, 0, surface->size.x, surface->size.y});
}

void TScintillaSurface::FlushCachedState()
{
}

void TScintillaSurface::SetUnicodeMode(bool unicodeMode_)
{
}

void TScintillaSurface::SetDBCSMode(int codePage)
{
}

void TScintillaSurface::SetBidiR2L(bool bidiR2L_)
{
}

} // namespace Scintilla

#define Uses_TProgram
#define Uses_TDrawSurface
#include <tvision/tv.h>

#include <turbo/scintilla.h>
#include <turbo/scintilla/internals.h>
#include "surface.h"

namespace Scintilla {

Window::~Window()
{
}

void Window::Destroy()
{
}

PRectangle Window::GetPosition() const
{
    return PRectangle();
}

void Window::SetPosition(PRectangle rc)
{
}

void Window::SetPositionRelative(PRectangle rc, const Window *relativeTo)
{
}

PRectangle Window::GetClientPosition() const
{
    auto *p = (turbo::TScintillaParent *) wid;
    if (p)
    {
        auto size = p->getEditorSize();
        return PRectangle::FromInts(0, 0, size.x, size.y);
    }
    return PRectangle();
}

void Window::Show(bool show)
{
}

void Window::InvalidateAll()
{
    auto *p = (turbo::TScintillaParent *) wid;
    if (p)
        p->invalidate({{0, 0}, p->getEditorSize()});
}

void Window::InvalidateRectangle(PRectangle rc)
{
    auto *p = (turbo::TScintillaParent *) wid;
    if (p)
        p->invalidate(TPRect(rc));
}

void Window::SetFont(Font &font)
{
}

void Window::SetCursor(Cursor curs)
{
}

PRectangle Window::GetMonitorRect(Point pt)
{
    if (TProgram::application)
    {
        auto size = TProgram::application->size;
        return PRectangle::FromInts(0, 0, size.x, size.y);
    }
    return PRectangle();
}

} // namespace scintilla

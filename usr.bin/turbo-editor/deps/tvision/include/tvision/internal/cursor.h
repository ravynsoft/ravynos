#ifndef TVISION_CURSOR_H
#define TVISION_CURSOR_H

#define Uses_TColorAttr
#include <tvision/tv.h>

#include <internal/dispbuff.h>

namespace tvision
{

class ScreenCursor
{
public:

    ScreenCursor() noexcept;
    ~ScreenCursor();

    void show() noexcept;
    void hide() noexcept;
    bool isVisible() const noexcept;
    void setPos(const TPoint &p) noexcept;
    const TPoint& getPos() const noexcept;
    void apply(TColorAttr &attr) noexcept;
    void restore(TColorAttr &attr) const noexcept;

protected:

    TPoint pos;
    bool visible;
    TColorAttr backup;

    virtual void draw(TColorAttr &attr) const noexcept = 0;
};

inline ScreenCursor::ScreenCursor() noexcept :
    pos({-1, -1}),
    visible(false),
    backup(0)
{
    DisplayBuffer::addCursor(this);
}

inline ScreenCursor::~ScreenCursor()
{
    DisplayBuffer::removeCursor(this);
}

inline void ScreenCursor::show() noexcept
{
    if (!visible)
        DisplayBuffer::changeCursor();
    visible = true;
}

inline void ScreenCursor::hide() noexcept
{
    visible = false;
}

inline bool ScreenCursor::isVisible() const noexcept
{
    return visible;
}

inline void ScreenCursor::setPos(const TPoint &p) noexcept
{
    if (visible && p != pos)
        DisplayBuffer::changeCursor();
    pos = p;
}

inline const TPoint& ScreenCursor::getPos() const noexcept
{
    return pos;
}

inline void ScreenCursor::apply(TColorAttr &attr) noexcept
{
    backup = attr;
    draw(attr);
}

inline void ScreenCursor::restore(TColorAttr &attr) const noexcept
{
    attr = backup;
}

class ReverseScreenCursor : public ScreenCursor
{
    void draw(TColorAttr &attr) const noexcept override;
};

class NegativeScreenCursor : public ScreenCursor
{
    void draw(TColorAttr &attr) const noexcept override;
};

} // namespace tvision

#endif // TVISION_CURSOR_H

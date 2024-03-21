/*------------------------------------------------------------*/
/* filename -       tvwrite.cpp                               */
/*                                                            */
/* function(s)                                                */
/*                  TView write member functions              */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TView
#define Uses_TGroup
#define Uses_TScreen
#define Uses_THardwareInfo
#define Uses_TEventQueue
#include <tvision/tv.h>

#ifdef __FLAT__

#include <string.h>
#include <stdlib.h>
#include <malloc.h>

extern TPoint shadowSize;
extern uchar shadowAttr;

struct TVWrite {

    short X, Y, Count, wOffset;
    const void _FAR *Buffer;
    TView *Target;
    int edx, esi;

    void L0( TView *, short, short, short, const void _FAR* ) noexcept;
    void L10( TView * ) noexcept;
    void L20( TView * ) noexcept;
    void L30( TView * ) noexcept;
    void L40( TView * ) noexcept;
    void L50( TGroup * ) noexcept;
#ifdef __BORLANDC__
    void copyShort( ushort *, const ushort * );
    void copyShort2CharInfo( ushort *, const ushort * );
#else
    void copyCell( TScreenCell *, const TScreenCell * ) noexcept;
    void copyShort2Cell( TScreenCell *, const ushort * ) noexcept;

    bool bufIsShort;

    TVWrite(bool b=true) noexcept :
        bufIsShort(b)
    {
    }
#endif

    static TColorAttr applyShadow(TColorAttr attr) noexcept
    {
#ifdef __BORLANDC__
        // Because we can't know if the cell has already been shadowed,
        // we compare against the shadow attributes. This may yield some false positives.
        TColorAttr shadowAttrInv = reverseAttribute(shadowAttr);
        if (attr == shadowAttr || attr == shadowAttrInv)
            return attr;
        else
            return attr & 0xF0 ? shadowAttr : shadowAttrInv;
#else
        // Here TColorAttr is a struct, so we can have a dedicated field
        // to determine whether the shadow has been applied.
        auto style = ::getStyle(attr);
        if (!(style & slNoShadow))
        {
            if (::getBack(attr).toBIOS(false) != 0)
                attr = shadowAttr;
            else // Reverse the shadow attribute on black areas.
                attr = reverseAttribute(shadowAttr);
            ::setStyle(attr, style | slNoShadow);
        }
        return attr;
#endif
    }

};

void TView::writeView( short x, short y, short count, const void _FAR* b ) noexcept
{
    TVWrite().L0(this, x, y, count, b);
}

#ifndef __BORLANDC__
void TView::writeView( short x, short y, short count, const TScreenCell* b ) noexcept
{
    TVWrite(false).L0(this, x, y, count, b);
}
#endif

void TVWrite::L0( TView *dest, short x, short y, short count, const void _FAR* b ) noexcept
{
    X = x; Y = y; Count = count; Buffer = b;
    wOffset = X;
    Count += X;
    edx = 0;
    if (0 <= Y && Y < dest->size.y)
    {
        if (X < 0)
            X = 0;
        if (Count > dest->size.x)
            Count = dest->size.x;
        if (X < Count)
            L10(dest);
    }
}

void TVWrite::L10( TView *dest ) noexcept
{
    TGroup *owner = dest->owner;
    if ((dest->state & sfVisible) && owner)
    {
        Target = dest;
        Y += dest->origin.y;
        X += dest->origin.x;
        Count += dest->origin.x;
        wOffset += dest->origin.x;
        if (owner->clip.a.y <= Y && Y < owner->clip.b.y)
        {
            if (X < owner->clip.a.x)
                X = owner->clip.a.x;
            if (Count > owner->clip.b.x)
                Count = owner->clip.b.x;
            if (X < Count)
                L20(owner->last);
        }
    }
}

void TVWrite::L20( TView *dest ) noexcept
{
    TView *next = dest->next;
    if (next == Target)
        L40(next);
    else
    {
        if ((next->state & sfVisible) && next->origin.y <= Y)
        do {
            esi = next->origin.y + next->size.y;
            if (Y < esi)
            {
                esi = next->origin.x;
                if (X < esi)
                {
                    if (Count > esi)
                        L30(next);
                    else break;
                }
                esi += next->size.x;
                if (X < esi)
                {
                    if (Count > esi)
                        X = esi;
                    else return;
                }
                if ((next->state & sfShadow) && next->origin.y + shadowSize.y <= Y)
                    esi += shadowSize.x;
                else break;
            }
            else if ((next->state & sfShadow) && Y < esi + shadowSize.y)
            {
                esi = next->origin.x + shadowSize.x;
                if (X < esi)
                {
                    if (Count > esi)
                        L30(next);
                    else break;
                }
                esi += next->size.x;
            }
            else break;
            if (X < esi)
            {
                edx++;
                if (Count > esi)
                {
                    L30(next);
                    edx--;
                }
            }
        } while (0);
        L20(next);
    }
}

void TVWrite::L30( TView *dest ) noexcept
{
    TView *_Target = Target;
    int _wOffset = wOffset, _esi = esi, _edx = edx,
        _count = Count, _y = Y;
    Count = esi;

    L20(dest);

    Y = _y; Count = _count; edx = _edx; esi = _esi;
    wOffset = _wOffset; Target = _Target;
    X = esi;
}

void TVWrite::L40( TView *dest ) noexcept
{
    TGroup *owner = dest->owner;
    if (owner->buffer)
    {
        if (owner->buffer != TScreen::screenBuffer)
            L50(owner);
        else
        {
#ifdef __BORLANDC__
            THWMouse::hide();
#endif
            L50(owner);
#ifdef __BORLANDC__
            THWMouse::show();
#endif
        }
    }
    if (owner->lockFlag == 0)
        L10(owner);
}

void TVWrite::L50( TGroup *owner ) noexcept
{
    TScreenCell *dst = &owner->buffer[Y*owner->size.x + X];
#ifdef __BORLANDC__
    const ushort *src = &((const ushort *) Buffer)[X - wOffset];
    if (owner->buffer != TScreen::screenBuffer)
        copyShort(dst, src);
    else
    {
        copyShort2CharInfo(dst, src);
        THardwareInfo::screenWrite(X, Y, dst, Count - X);
    }
#else
    if (bufIsShort)
    {
        auto *src = &((const ushort *) Buffer)[X - wOffset];
        copyShort2Cell(dst, src);
    }
    else
    {
        auto *src = &((const TScreenCell *) Buffer)[X - wOffset];
        copyCell(dst, src);
    }
    if (owner->buffer == TScreen::screenBuffer)
        THardwareInfo::screenWrite(X, Y, dst, Count - X);
#endif // __BORLANDC__
}

#ifdef __BORLANDC__
// On Windows and DOS, Turbo Vision stores a byte of text and a byte of
// attributes for every cell. On Windows, all TGroup buffers follow this schema
// except the topmost one, which interfaces with the Win32 Console API.

void TVWrite::copyShort( ushort *dst, const ushort *src )
{
    int i;
    if (edx == 0)
        memcpy(dst, src, 2*(Count - X));
    else
    {
#define loByte(w)    (((uchar *)&w)[0])
#define hiByte(w)    (((uchar *)&w)[1])
        for (i = 0; i < Count - X; ++i)
        {
            loByte(dst[i]) = loByte(src[i]);
            hiByte(dst[i]) = applyShadow(hiByte(src[i]));
        }
#undef loByte
#undef hiByte
    }
}

void TVWrite::copyShort2CharInfo( ushort *dst, const ushort *src )
{
    int i;
    if (edx == 0)
        // Expand character/attribute pair
        for (i = 0; i < 2*(Count - X); ++i)
        {
            dst[i] = ((const uchar *) src)[i];
        }
    else
        // Mix in shadow attribute
        for (i = 0; i < 2*(Count - X); i += 2)
        {
            dst[i] = ((const uchar *) src)[i];
            dst[i + 1] = applyShadow(((const uchar *) src)[i + 1]);
        }
}

#else
void TVWrite::copyCell(TScreenCell *dst, const TScreenCell *src) noexcept
{
    int i;
    if (edx == 0)
        memcpy(dst, src, sizeof(TScreenCell)*(Count - X));
    else
        for (i = 0; i < Count - X; ++i)
        {
            auto c = src[i];
            ::setAttr(c, applyShadow(::getAttr(c)));
            dst[i] = c;
        }
}

void TVWrite::copyShort2Cell( TScreenCell *dst, const ushort *src ) noexcept
{
    int i;
    if (edx == 0)
        // Expand character/attribute pair
        for (i = 0; i < Count - X; ++i)
        {
            dst[i] = TScreenCell {src[i]};
        }
    else
        // Mix in shadow attribute
        for (i = 0; i < Count - X; ++i)
        {
            TScreenCell c {src[i]};
            ::setAttr(c, applyShadow(::getAttr(c)));
            dst[i] = c;
        }
}

void TView::writeBuf( short x, short y, short w, short h, const TScreenCell* b ) noexcept
{
    while (h-- > 0)
    {
        writeView(x, y++, w, b);
        b += w;
    }
}

#endif // __BORLANDC__

void TView::writeBuf( short x, short y, short w, short h, const void _FAR* b ) noexcept
{
    while (h-- > 0)
    {
        writeView(x, y++, w, b);
        b = ((const ushort *) b) + w;
    }
}

void TView::writeChar( short x, short y, char c, uchar color, short count ) noexcept
{
    if (count > 0)
    {
        TScreenCell cell;
        ::setCell(cell, c, mapColor(color));
        TScreenCell *buf = (TScreenCell *) alloca(count*sizeof(TScreenCell));
        for (short i = 0; i < count; ++i)
        {
            buf[i] = cell;
        }
        writeView(x, y, count, buf);
    }
}

void TView::writeLine( short x, short y, short w, short h, const void _FAR *b ) noexcept
{
    while (h-- > 0)
    {
        writeView(x, y++, w, b);
    }
}

#ifndef __BORLANDC__
void TView::writeLine( short x, short y, short w, short h, const TScreenCell *b ) noexcept
{
    while (h-- > 0)
    {
        writeView(x, y++, w, b);
    }
}
#endif

void TView::writeStr( short x, short y, const char *str, uchar color ) noexcept
{
    if (str != 0)
    {
        size_t length = strlen(str);
        if (length > 0)
        {
            TColorAttr attr = mapColor(color);
            TScreenCell *buf = (TScreenCell*) alloca(length*sizeof(TScreenCell));
            for (size_t i = 0; i < length; ++i)
            {
                ::setCell(buf[i], str[i], attr);
            }
            writeView(x, y, length, buf);
        }
    }
}

#endif

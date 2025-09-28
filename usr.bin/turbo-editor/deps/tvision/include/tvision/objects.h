/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   OBJECTS.H                                                             */
/*                                                                         */
/*   defines the classes TPoint, TRect, TCollection, and TSortedCollection */
/*                                                                         */
/* ------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if defined( __BORLANDC__ )
#pragma option -Vo-
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po-
#endif

#if defined( Uses_TPoint ) && !defined( __TPoint )
#define __TPoint

class TPoint
{

public:

    TPoint& operator+=( const TPoint& adder ) noexcept;
    TPoint& operator-=( const TPoint& subber ) noexcept;
    friend TPoint operator - ( const TPoint& one, const TPoint& two) noexcept;
    friend TPoint operator + ( const TPoint& one, const TPoint& two) noexcept;
    friend int operator == ( const TPoint& one, const TPoint& two) noexcept;
    friend int operator != ( const TPoint& one, const TPoint& two) noexcept;

    int x,y;

};

inline TPoint& TPoint::operator += ( const TPoint& adder ) noexcept
{
    x += adder.x;
    y += adder.y;
    return *this;
}

inline TPoint& TPoint::operator -= ( const TPoint& subber ) noexcept
{
    x -= subber.x;
    y -= subber.y;
    return *this;
}

inline TPoint operator - ( const TPoint& one, const TPoint& two ) noexcept
{
    TPoint result;
    result.x = one.x - two.x;
    result.y = one.y - two.y;
    return result;
}

inline TPoint operator + ( const TPoint& one, const TPoint& two ) noexcept
{
    TPoint result;
    result.x = one.x + two.x;
    result.y = one.y + two.y;
    return result;
}

inline int operator == ( const TPoint& one, const TPoint& two ) noexcept
{
    return one.x == two.x && one.y == two.y;
}

inline int operator!= ( const TPoint& one, const TPoint& two ) noexcept
{
    return one.x != two.x || one.y != two.y;
}

inline ipstream& operator >> ( ipstream& is, TPoint& p )
    { return is >> p.x >> p.y; }
inline ipstream& operator >> ( ipstream& is, TPoint*& p )
    { return is >> p->x >> p->y; }

inline opstream& operator << ( opstream& os, TPoint& p )
    { return os << p.x << p.y; }
inline opstream& operator << ( opstream& os, TPoint* p )
    { return os << p->x << p->y; }

#endif  // Uses_TPoint

#if defined( Uses_TRect ) && !defined( __TRect )
#define __TRect

class TRect
{

public:

    TRect( int ax, int ay, int bx, int by ) noexcept;
    TRect( TPoint p1, TPoint p2 ) noexcept;
    TRect() noexcept {}

    TRect& move( int aDX, int aDY ) noexcept;
    TRect& grow( int aDX, int aDY ) noexcept;
    TRect& intersect( const TRect& r ) noexcept;
    TRect& Union( const TRect& r ) noexcept;
    Boolean contains( const TPoint& p ) const noexcept;
    Boolean operator == ( const TRect& r ) const noexcept;
    Boolean operator != ( const TRect& r ) const noexcept;
    Boolean isEmpty() noexcept;

    TPoint a, b;

};

inline TRect::TRect( int ax, int ay, int bx, int by) noexcept
{
    a.x = ax;
    a.y = ay;
    b.x = bx;
    b.y = by;
}

inline TRect::TRect( TPoint p1, TPoint p2 ) noexcept
{
    a = p1;
    b = p2;
}

inline TRect& TRect::move( int aDX, int aDY ) noexcept
{
    a.x += aDX;
    a.y += aDY;
    b.x += aDX;
    b.y += aDY;
    return *this;
}

inline TRect& TRect::grow( int aDX, int aDY ) noexcept
{
    a.x -= aDX;
    a.y -= aDY;
    b.x += aDX;
    b.y += aDY;
    return *this;
}

inline TRect& TRect::intersect( const TRect& r ) noexcept
{
    a.x = max( a.x, r.a.x );
    a.y = max( a.y, r.a.y );
    b.x = min( b.x, r.b.x );
    b.y = min( b.y, r.b.y );
    return *this;
}

inline TRect& TRect::Union( const TRect& r ) noexcept
{
    a.x = min( a.x, r.a.x );
    a.y = min( a.y, r.a.y );
    b.x = max( b.x, r.b.x );
    b.y = max( b.y, r.b.y );
    return *this;
}

inline Boolean TRect::contains( const TPoint& p ) const noexcept
{
    return Boolean(
        p.x >= a.x && p.x < b.x && p.y >= a.y && p.y < b.y
        );
}

inline Boolean TRect::operator == ( const TRect& r) const noexcept
{
    return Boolean( a == r.a && b == r.b );
}

inline Boolean TRect::operator != ( const TRect& r ) const noexcept
{
    return Boolean( !(*this == r) );
}

inline Boolean TRect::isEmpty() noexcept
{
    return Boolean( a.x >= b.x || a.y >= b.y );
}

inline ipstream& operator >> ( ipstream& is, TRect& r )
    { return is >> r.a >> r.b; }
inline ipstream& operator >> ( ipstream& is, TRect*& r )
    { return is >> r->a >> r->b; }

inline opstream& operator << ( opstream& os, TRect& r )
    { return os << r.a << r.b; }
inline opstream& operator << ( opstream& os, TRect* r )
    { return os << r->a << r->b; }

#endif  // Uses_TRect

#if defined( Uses_TCollection ) && !defined( __TCollection )
#define __TCollection

class TCollection : public virtual TNSCollection, public TStreamable
{

public:

    TCollection( ccIndex aLimit, ccIndex aDelta ) noexcept
        { delta = aDelta; setLimit( aLimit ); }

private:

    virtual const char *streamableName() const
        { return name; }

    virtual void *readItem( ipstream& ) = 0;
    virtual void writeItem( void *, opstream& ) = 0;


protected:

    TCollection( StreamableInit ) noexcept;
    virtual void *read( ipstream& );
    virtual void write( opstream& );

public:

    static const char * const _NEAR name;

};

inline ipstream& operator >> ( ipstream& is, TCollection& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TCollection*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TCollection& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TCollection* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TCollection

#if defined( Uses_TSortedCollection ) && !defined( __TSortedCollection )
#define __TSortedCollection

class TSortedCollection : public TNSSortedCollection, public TCollection
{

public:

    TSortedCollection( ccIndex aLimit, ccIndex aDelta) noexcept :
        TCollection( aLimit, aDelta ) {}

private:

    virtual int compare( void *key1, void *key2 ) = 0;

    virtual const char *streamableName() const
        { return name; }
    virtual void *readItem( ipstream& ) = 0;
    virtual void writeItem( void *, opstream& ) = 0;

protected:

    TSortedCollection( StreamableInit ) noexcept;
    virtual void *read( ipstream& );
    virtual void write( opstream& );

public:

    static const char * const _NEAR name;

};

inline ipstream& operator >> ( ipstream& is, TSortedCollection& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TSortedCollection*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TSortedCollection& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TSortedCollection* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TSortedCollection

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif

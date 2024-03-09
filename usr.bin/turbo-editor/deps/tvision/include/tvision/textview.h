/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   TEXTVIEW.H                                                            */
/*                                                                         */
/*   defines the classes TTextDevice and TTerminal                         */
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

#if defined( Uses_TTextDevice ) && !defined( __TTextDevice )
#define __TTextDevice

#include <tvision/compat/borland/iostream.h>
#if defined( __BORLANDC__ )
#pragma option -Vo-
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po-
#endif

class _FAR TRect;
class _FAR TScrollBar;

class TTextDevice : public TScroller, public streambuf
{

public:

    TTextDevice( const TRect& bounds,
                 TScrollBar *aHScrollBar,
                 TScrollBar *aVScrollBar
               ) noexcept;

    virtual int do_sputn( const char *s, int count ) = 0;
    virtual int overflow( int = EOF );

protected:

#if !defined( __BORLANDC__ )
    virtual std::streamsize xsputn(const char *s, std::streamsize count);
#endif

};

#endif  // Uses_TTextDevice

#if defined( Uses_TTerminal ) && !defined( __TTerminal )
#define __TTerminal

class _FAR TRect;
class _FAR TScrollBar;

class TTerminal: public TTextDevice
{

public:

    friend void genRefs();

    TTerminal( const TRect& bounds,
               TScrollBar *aHScrollBar,
               TScrollBar *aVScrollBar,
               ushort aBufSize
             ) noexcept;
    ~TTerminal();

    virtual int do_sputn( const char *s, int count );

    void bufInc( ushort& val );
    Boolean canInsert( ushort amount );
    virtual void draw();
    ushort nextLine( ushort pos );
    ushort prevLines( ushort pos, ushort lines );
    Boolean queEmpty();

protected:

    ushort bufSize;
    char *buffer;
    ushort queFront, queBack;
    void bufDec(ushort& val);
};

#endif  // Uses_TTerminal

#if defined( Uses_otstream ) && !defined( __otstream )
#define __otstream

#include <tvision/compat/borland/iostream.h>
#if defined( __BORLANDC__ )
#pragma option -Vo-
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po-
#endif


class otstream : public ostream
{

public:

    otstream( TTerminal * );

};


#endif

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif

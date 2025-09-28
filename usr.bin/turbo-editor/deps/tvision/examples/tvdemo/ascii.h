/*-------------------------------------------------------*/
/*                                                       */
/*   Ascii.h: Header file for Ascii.cpp                  */
/*                                                       */
/*-------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined( __ASCII_H )
#define __ASCII_H

const int cmAsciiTableCmdBase = 910;
const int cmCharFocused       =   0;


class TTable : public TView
{

public:

    TTable( TRect& r );
    TTable( StreamableInit ) : TView(streamableInit) { };
    virtual void draw();
    virtual void handleEvent( TEvent& event );
    void charFocused();

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();

};


inline ipstream& operator >> ( ipstream& is, TTable& cl )
    { return is >> (TStreamable&) cl; }
inline ipstream& operator >> ( ipstream& is, TTable*& cl )
    { return is >> (void *&) cl; }

inline opstream& operator << ( opstream& os, TTable& cl )
    { return os << (TStreamable&) cl; }
inline opstream& operator << ( opstream& os, TTable* cl )
    { return os << (TStreamable *) cl; }


class TReport : public TView
{

public:

    TReport( TRect& r );
    TReport( StreamableInit ) : TView(streamableInit) { };
    virtual void draw();
    virtual void handleEvent( TEvent& event );

private:

    uchar asciiChar;

    virtual const char *streamableName() const
         { return name; }

protected:

    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TReport& cl )
    { return is >> (TStreamable&) cl; }
inline ipstream& operator >> ( ipstream& is, TReport*& cl )
    { return is >> (void *&) cl; }

inline opstream& operator << ( opstream& os, TReport& cl )
    { return os << (TStreamable&) cl; }
inline opstream& operator << ( opstream& os, TReport* cl )
    { return os << (TStreamable *) cl; }


class TAsciiChart : public TWindow
{

public:

    TAsciiChart();
    TAsciiChart( StreamableInit ) :
        TWindowInit(0), TWindow(streamableInit) { };
    virtual void handleEvent( TEvent &event );

private:

    virtual const char *streamableName() const
       { return name; }

protected:

    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TAsciiChart& cl )
    { return is >> (TStreamable&) cl; }
inline ipstream& operator >> ( ipstream& is, TAsciiChart*& cl )
    { return is >> (void *&) cl; }

inline opstream& operator << ( opstream& os, TAsciiChart& cl )
    { return os << (TStreamable&) cl; }
inline opstream& operator << ( opstream& os, TAsciiChart* cl )
    { return os << (TStreamable *) cl; }


#endif      // __ASCII_H

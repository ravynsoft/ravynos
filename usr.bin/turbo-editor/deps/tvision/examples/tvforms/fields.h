/*-------------------------------------------------------*/
/*                                                       */
/*   Turbo Vision Forms Demo                             */
/*                                                       */
/*   Fields.h: Header file for Fields.cpp                */
/*             (Support header file for TVFORMS Demo)    */
/*-------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined( __FIELDS_H )
#define __FIELDS_H

#define Uses_TInputLine
#define Uses_TStreamable
#include <tvision/tv.h>

// Same as TInputLine, except invalid if empty 

class TKeyInputLine : public TInputLine
{

public:

    TKeyInputLine( const TRect&, int );
    virtual Boolean valid( ushort );

protected:

    TKeyInputLine( StreamableInit ) : TInputLine( streamableInit ) {};

private:

    virtual const char *streamableName() const
        { return name; }

public:

    static const char * const name;
    static TStreamable *build();
     
};


inline ipstream& operator >> ( ipstream& is, TKeyInputLine& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TKeyInputLine*& cl )
    { return is >> (void *&)cl; }
inline opstream& operator << ( opstream& os, TKeyInputLine& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TKeyInputLine* cl )
    { return os << (TStreamable *)cl; }


// Accepts only valid numeric input between Min and Max

class TNumInputLine : public TInputLine
{

public:

    TNumInputLine( const TRect&, int, int32_t, int32_t );
    virtual ushort dataSize();
    virtual void getData( void *);
    virtual void setData( void *);
    virtual Boolean valid( ushort );
    int32_t min;
    int32_t max;

protected:

    TNumInputLine( StreamableInit ) : TInputLine( streamableInit ) {};
    virtual void write( opstream& );
    virtual void *read( ipstream& );

private:

    virtual const char *streamableName() const
        { return name; }

public:

    static const char * const name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TNumInputLine& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TNumInputLine*& cl )
    { return is >> (void *&)cl; }
inline opstream& operator << ( opstream& os, TNumInputLine& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TNumInputLine* cl )
    { return os << (TStreamable *)cl; }

#endif  // __FIELDS_H

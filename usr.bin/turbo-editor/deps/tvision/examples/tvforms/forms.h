/*-------------------------------------------------------*/
/*                                                       */
/*   Turbo Vision Forms Demo                             */
/*                                                       */
/*   Forms.h: Header file for Forms.cpp                  */
/*            (Support header file for TVFORMS Demo)     */
/*                                                       */
/*-------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined( __FORMS_H )
#define __FORMS_H

#define Uses_TStreamable
#define Uses_TEvent
#define Uses_TRect
#define Uses_TDialog
#define Uses_TView
#include <tvision/tv.h>

class TForm : public TDialog
{

public:

    TForm( StreamableInit ) : TWindowInit(&TForm::initFrame), TDialog (streamableInit) {};
    TForm( const TRect&, const char* );
    virtual Boolean changed();
    virtual void handleEvent( TEvent& );
    virtual Boolean valid( ushort );

    TView  *listDialog;
    void   *prevData;
    ushort keyWidth;

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

inline ipstream& operator >> ( ipstream& is, TForm& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TForm*& cl )
    { return is >> (void *&)cl; }
inline opstream& operator << ( opstream& os, TForm& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TForm* cl )
    { return os << (TStreamable *)cl; }


#endif  // __FORMS_H

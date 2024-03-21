/*---------------------------------------------------------*/
/*                                                         */
/*   Fileview.h:  Header file for fileview.cpp.            */
/*                                                         */
/*---------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined( __FILEVIEW_H )
#define __FILEVIEW_H

#define Uses_TCollection
#define Uses_TScroller
#define Uses_TWindow
#include <tvision/tv.h>

const int hlChangeDir = cmChangeDir;

class TLineCollection : public TCollection
{

public:

    TLineCollection(short lim, short delta) : TCollection(lim, delta) {}
    virtual void  freeItem(void *p) { delete[] (char *) p; }

private:

    virtual void *readItem( ipstream& ) { return 0; }
    virtual void writeItem( void *, opstream& ) {}

};

class TFileViewer : public TScroller
{

public:

    char *fileName;
    TCollection *fileLines;
    Boolean isValid;
    TFileViewer( const TRect& bounds,
                 TScrollBar *aHScrollBar,
                 TScrollBar *aVScrollBar,
                 const char *aFileName
               );
    ~TFileViewer();
    TFileViewer( StreamableInit ) : TScroller(streamableInit) { };
    void draw();
    void readFile( const char *fName );
    void setState( ushort aState, Boolean enable );
    void scrollDraw();
    Boolean valid( ushort command );

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

class TFileWindow : public TWindow
{

public:

    TFileWindow( const char *fileName );

};

const int maxLineLength = 256;

#endif


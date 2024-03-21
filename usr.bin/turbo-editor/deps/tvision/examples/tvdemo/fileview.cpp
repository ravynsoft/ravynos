/*---------------------------------------------------------*/
/*                                                         */
/*   Turbo Vision FileViewer Demo Support File             */
/*                                                         */
/*---------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_MsgBox
#define Uses_TKeys
#define Uses_TScroller
#define Uses_TDrawBuffer
#define Uses_TRect
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TStreamableClass
#define Uses_ipstream
#define Uses_opstream
#include <tvision/tv.h>
__link(RScroller)
__link(RScrollBar)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <strstrea.h>
#include <fstream.h>

#include "tvcmds.h"
#include "fileview.h"


const char * const TFileViewer::name = "TFileViewer";

TFileViewer::TFileViewer( const TRect& bounds,
                          TScrollBar *aHScrollBar,
                          TScrollBar *aVScrollBar,
                          const char *aFileName) :
    TScroller( bounds, aHScrollBar, aVScrollBar )
{
    growMode = gfGrowHiX | gfGrowHiY;
    isValid = True;
    fileName = 0;
    readFile( aFileName );
}

TFileViewer::~TFileViewer()
{
     delete[] fileName;
     destroy (fileLines);
}

void TFileViewer::draw()
{
    char *p;

    TColorAttr c =  getColor(1);
    for( short i = 0; i < size.y; i++ )
        {
        TDrawBuffer b;
        b.moveChar( 0, ' ', c, (short)size.x );

        if( delta.y + i < fileLines->getCount() )
            {
            p = (char *)( fileLines->at(delta.y+i) );
            if( p )
                b.moveStr( 0, p, c, (short)size.x, (short)delta.x );
            }
        writeBuf( 0, i, (short)size.x, 1, b );
        }
}

void TFileViewer::scrollDraw()
{
    TScroller::scrollDraw();
    draw();
}

void TFileViewer::readFile( const char *fName )
{
    delete[] fileName;

    limit.x = 0;
    fileName = newStr( fName );
    fileLines = new TLineCollection(5, 5);
    ifstream fileToView( fName );
    if( !fileToView )
        {
        char buf[256] = {0};
        ostrstream os( buf, sizeof( buf )-1 );
        os << "Failed to open file '" << fName << "'." << ends;
        messageBox( buf, mfError | mfOKButton );
        isValid = False;
        }
    else
        {
        char *line = (char *) malloc(maxLineLength);
        size_t lineSize = maxLineLength;
        char c;
        while( !lowMemory() &&
               !fileToView.eof() && 
               fileToView.get( c )
             )
            {
            size_t i = 0;
            while ( !fileToView.eof() && c != '\n' && c != '\r' ) // read a whole line
                {
                if (i == lineSize)
                    line = (char *) realloc(line, (lineSize *= 2));
                line[i++] = c ? c : ' ';
                fileToView.get( c );
                }
            line[i] = '\0';
            if ( c == '\r' && fileToView.peek() == '\n')
                fileToView.get( c ); // grab trailing newline on CRLF
            limit.x = max( limit.x, strwidth( line ) );
            fileLines->insert( newStr( line ) );
            }
        isValid = True;
        ::free(line);
        }
    limit.y = fileLines->getCount();
}

void TFileViewer::setState( ushort aState, Boolean enable )
{
    TScroller::setState( aState, enable );
    if( enable && (aState & sfExposed) )
        setLimit( limit.x, limit.y );
}

Boolean TFileViewer::valid( ushort )
{
    return isValid;
}

void *TFileViewer::read(ipstream& is)
{
    char *fName;

    TScroller::read(is);
    fName = is.readString();
    fileName = 0;
    readFile(fName);
    delete[] fName;
    return this;
}

void TFileViewer::write(opstream& os)
{
    TScroller::write(os);
    os.writeString(fileName);
}

TStreamable *TFileViewer::build()
{
    return new TFileViewer( streamableInit );
}


TStreamableClass RFileView( TFileViewer::name,
                            TFileViewer::build,
                              __DELTA(TFileViewer)
                          );



static short winNumber = 0;

TFileWindow::TFileWindow( const char *fileName ) :
    TWindowInit( &TFileWindow::initFrame ),
    TWindow( TProgram::deskTop->getExtent(), fileName, winNumber++ )
{
    options |= ofTileable;
    TRect r( getExtent() );
    r.grow(-1, -1);
    insert(new TFileViewer( r,
                            standardScrollBar(sbHorizontal | sbHandleKeyboard),
                            standardScrollBar(sbVertical | sbHandleKeyboard),
                            fileName) );
}



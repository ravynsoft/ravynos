/*------------------------------------------------------------*/
/* filename -       tdirlist.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TDirListBox member functions              */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TDirListBox
#define Uses_TEvent
#define Uses_TDirCollection
#define Uses_TChDirDialog
#define Uses_TDirEntry
#define Uses_TButton
#include <tvision/tv.h>

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __DIR_H )
#include <dir.h>
#endif  // __DIR_H

#if !defined( __DOS_H )
#include <dos.h>
#endif  // __DOS_H

TDirListBox::TDirListBox( const TRect& bounds, TScrollBar *aScrollBar ) noexcept :
    TListBox( bounds, 1, aScrollBar ),
    cur( 0 )
{
    *dir = EOS;
}

TDirListBox::~TDirListBox()
{
   if ( list() )
      destroy( list() );
}

void TDirListBox::getText( char *text, short item, short maxChars )
{
    strncpy( text, list()->at(item)->text(), maxChars );
    text[maxChars] = '\0';
}

void TDirListBox::selectItem( short item )
{
    message( owner, evCommand, cmChangeDir, list()->at(item) );
}

/*
void TDirListBox::handleEvent( TEvent& event )
{
    if( event.what == evMouseDown && (event.mouse.eventFlags & meDoubleClick) )
        {
        event.what = evCommand;
        event.message.command = cmChangeDir;
        putEvent( event );
        clearEvent( event );
        }
    else
       TListBox::handleEvent( event );
}
*/

Boolean TDirListBox::isSelected( short item )
{
    return Boolean( item == cur );
}

void TDirListBox::showDrives( TDirCollection *dirs )
{
    Boolean isFirst = True;
    char oldc[5];
    strcpy( oldc, "0:\\" );
    for( char c = 'A'; c <= 'Z'; c++ )
        {
        if( c < 'C' || driveValid( c ) )
            {
            if( oldc[0] != '0' )
                {
                char s[ 16 ];
                if( isFirst )
                    {
                    strcpy( s, firstDir );
                    s[ strlen(firstDir) ] = oldc[0];
                    s[ strlen(firstDir)+1 ] = EOS;
                    isFirst = False;
                    }
                else
                    {
                    strcpy( s, middleDir );
                    s[ strlen(middleDir) ] = oldc[0];
                    s[ strlen(middleDir)+1 ] = EOS;
                    }
                dirs->insert( new TDirEntry( s, oldc ) );
                }
            if( c == getdisk() + 'A' )
                cur = dirs->getCount();
            oldc[0] = c;
            }
        }
    if( oldc[0] != '0' )
        {
        char s[ 16 ];
        strcpy( s, lastDir );
        s[ strlen(lastDir) ] = oldc[0];
        s[ strlen(lastDir)+1 ] = EOS;
        dirs->insert( new TDirEntry( s, oldc ) );
        }
}

void TDirListBox::showDirs( TDirCollection *dirs )
{
    const int indentSize = 2;
    int indent = indentSize;

    char buf[MAXPATH+MAXFILE+MAXEXT];
    memset( buf, ' ', sizeof( buf ) );
    char *name = buf + sizeof(buf) - (MAXFILE+MAXEXT);

    char *org = name - strlen(pathDir);
    strcpy( org, pathDir );

    char *curDir = dir;
    char *end = dir + 3;
    char hold = *end;
    *end = EOS;         // mark end of drive name
    strcpy( name, curDir );
    dirs->insert( new TDirEntry( org, name ) );

    *end = hold;        // restore full path
    curDir = end;
    while( (end = strchr( curDir, '\\' )) != 0 )
        {
        *end = EOS;
        strncpy( name, curDir, size_t(end-curDir) );
        name[size_t(end-curDir)] = EOS;
        dirs->insert( new TDirEntry( org - indent, dir ) );
        *end = '\\';
        curDir = end+1;
        indent += indentSize;
        }

    cur = dirs->getCount() - 1;

    end = strrchr( dir, '\\' );
    char path[MAXPATH];
    strncpy( path, dir, size_t(end-dir+1) );
    end = path + unsigned(end-dir)+1;
    strcpy( end, "*.*" );

    Boolean isFirst = True;
    ffblk ff;
    int res = findfirst( path, &ff, FA_DIREC );
    while( res == 0 )
        {
        if( (ff.ff_attrib & FA_DIREC) != 0 && ff.ff_name[0] != '.' )
            {
            if( isFirst )
                {
                memcpy( org, firstDir, strlen(firstDir)+1 );
                isFirst = False;
                }
            else
                memcpy( org, middleDir, strlen(middleDir)+1 );
            strcpy( name, ff.ff_name );
            strcpy( end, ff.ff_name );
            dirs->insert( new TDirEntry( org - indent, path ) );
            }
        res = findnext( &ff );
        }

    char *p = dirs->at(dirs->getCount()-1)->text();
    char *i = strchr( p, graphics[0] );
    if( i == 0 )
        {
        i = strchr( p, graphics[1] );
        if( i != 0 )
            *i = graphics[0];
        }
    else
        {
        *(i+1) = graphics[2];
        *(i+2) = graphics[2];
        }
}

void TDirListBox::newDirectory( TStringView str )
{
    strnzcpy( dir, str, sizeof(dir) );
    TDirCollection *dirs = new TDirCollection( 5, 5 );
    dirs->insert( new TDirEntry( drives, drives ) );
    if( str == drives )
        showDrives( dirs );
    else
        showDirs( dirs );
    newList( dirs );
    focusItem( cur );
}

void TDirListBox::setState( ushort nState, Boolean enable )
{
    TListBox::setState( nState, enable );
    if( (nState & sfFocused) != 0 )
        ((TChDirDialog *)owner)->chDirButton->makeDefault( enable );
}

#if !defined(NO_STREAMABLE)

TStreamable *TDirListBox::build()
{
    return new TDirListBox( streamableInit );
}

#endif

/*------------------------------------------------------------*/
/* filename - tfiledtr.cpp                                    */
/*                                                            */
/* function(s)                                                */
/*            TFileEditor member functions                    */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TProgram
#define Uses_TGroup
#define Uses_TEditor
#define Uses_TFileEditor
#define Uses_TEvent
#define Uses_opstream
#define Uses_ipstream
#include <tvision/tv.h>

#if !defined( __LIMITS_H )
#include <limits.h>
#endif  // __LIMITS_H

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __FSTREAM_H )
#include <fstream.h>
#endif  // __FSTREAM_H

#if !defined( __IO_H )
#include <io.h>
#endif  // __IO_H

#if !defined( __STDIO_H )
#include <stdio.h>
#endif  // __STDIO_H

#if !defined( __STDLIB_H )
#include <alloc.h>
#endif

TFileEditor::TFileEditor( const TRect& bounds,
                          TScrollBar *aHScrollBar,
                          TScrollBar *aVScrollBar,
                          TIndicator *aIndicator,
                          TStringView aFileName
                        ) noexcept :
    TEditor( bounds, aHScrollBar, aVScrollBar, aIndicator, 0 )
{
    TEditor::doneBuffer();
    initBuffer();

    if( aFileName.empty() )
        fileName[0] = EOS;
    else
        {
        strnzcpy( fileName, aFileName, sizeof(fileName) );
        fexpand( fileName );
        if( isValid )
            isValid = loadFile();
        }
}

void TFileEditor::doneBuffer()
{
    free(buffer);
}

void TFileEditor::handleEvent( TEvent& event )
{
    TEditor::handleEvent(event);
    switch( event.what )
        {
        case evCommand:
            switch( event.message.command )
                {
                case cmSave:
                    save();
                    break;
                case cmSaveAs:
                    saveAs();
                    break;
                default:
                    return;
                }
            break;
        default:
            return;
        }
    clearEvent(event);
}

void TFileEditor::initBuffer()
{
    buffer = (char *) malloc(bufSize);
}

Boolean TFileEditor::loadFile() noexcept
{
    ifstream f( fileName, ios::in | ios::binary );
    if( !f )
        {
        setBufLen( 0 );
        return True;
        }
    else
        {
        f.seekg(0, ios::end);
        ulong fSize = f.tellg();
        f.seekg(0);
        if( fSize > UINT_MAX-0x1Fl || setBufSize(uint(fSize)) == False )
            {
            editorDialog( edOutOfMemory );
            return False;
            }
        else
            {
            if ( fSize > INT_MAX )
            {
               f.read( &buffer[bufSize - uint(fSize)], INT_MAX );
               f.read( &buffer[bufSize - uint(fSize) + INT_MAX],
                                uint(fSize - INT_MAX) );

            }
            else
               f.read( &buffer[bufSize - uint(fSize)], uint(fSize) );
            if( !f )
                {
                editorDialog( edReadError, fileName );
                return False;
                }
            else
                {
                setBufLen(uint(fSize));
                return True;
                }
            }
        }
}

Boolean TFileEditor::save() noexcept
{
    if( *fileName == EOS )
        return saveAs();
    else
        return saveFile();
}

Boolean TFileEditor::saveAs() noexcept
{
    Boolean res = False;
    if( editorDialog( edSaveAs, fileName ) != cmCancel )
        {
        fexpand( fileName );
        message( owner, evBroadcast, cmUpdateTitle, 0 );
        res = saveFile();
        if( isClipboard() == True )
            *fileName = EOS;
        }
    return res;
}

static void writeBlock( ofstream& f, char *buf, uint len ) noexcept
{
    while( len > 0 )
        {
        int l = min( len, uint(INT_MAX) );
        f.write( buf, l );
        buf += l;
        len -= l;
        }
}

Boolean TFileEditor::saveFile() noexcept
{
    char drive[MAXDRIVE];
    char dir[MAXDIR];
    char file[MAXFILE];
    char ext[MAXEXT];
    if( (editorFlags & efBackupFiles) != 0 )
        {
        fnsplit( fileName, drive, dir, file, ext );
        char backupName[MAXPATH];
        fnmerge( backupName, drive, dir, file, backupExt );
        unlink( backupName );
        rename( fileName, backupName );
        }

    ofstream f( fileName, ios::out | ios::binary );

    if( !f )
        {
        editorDialog( edCreateError, fileName );
        return False;
        }
    else
        {
        writeBlock( f, buffer, curPtr );
        writeBlock( f, buffer+curPtr+gapLen, bufLen-curPtr );

        if( !f )
            {
            editorDialog( edWriteError, fileName );
            return False;
            }
        else
            {
            modified = False;
            update(ufUpdate);
            }
        }
    return True;
}

Boolean TFileEditor::setBufSize( uint newSize )
{
    if( newSize == 0)
        newSize = 0x1000;
    else if( newSize > uint(-0x1000) )
        newSize = UINT_MAX-0x1F;
    else
        newSize = (newSize + 0x0FFF) & -0x1000; // 0x....F000
    if( newSize != bufSize )
        {
        char *temp = buffer;
        /* Bypass safety pool to allocate buffer, but check for possible
           NULL return value. */
        if( (buffer = (char *) malloc( newSize )) == 0 )
            {
            free(temp);
            return False;
            }
        uint n = bufLen - curPtr + delCount;
        uint min = newSize < bufSize ? newSize : bufSize;
        memcpy( buffer, temp, min );
        memmove( &buffer[newSize - n], &temp[bufSize - n], n );
        free(temp);
        bufSize = newSize;
        gapLen = bufSize - bufLen;
        }
    return True;
}

void TFileEditor::shutDown()
{
    setCmdState(cmSave, False);
    setCmdState(cmSaveAs, False);
    TEditor::shutDown();
}

void TFileEditor::updateCommands()
{
    TEditor::updateCommands();
    setCmdState(cmSave, True);
    setCmdState(cmSaveAs, True);
}

Boolean TFileEditor::valid( ushort command )
{
    if( command == cmValid )
        return isValid;
    else
        {
        if( modified == True )
            {
            int d;
            if( *fileName == EOS )
                d = edSaveUntitled;
            else
                d = edSaveModify;

            switch( editorDialog( d, fileName ) )
                {
                case cmYes:
                    return save();
                case cmNo:
                    modified = False;
                    return True;
                case cmCancel:
                    return False;
                }
            }
        }
    return True;
}

#if !defined(NO_STREAMABLE)

void TFileEditor::write( opstream& os )
{
    TEditor::write( os );
    os.writeString( fileName );
    os << selStart << selEnd << curPtr;
}

void *TFileEditor::read( ipstream& is )
{
    TEditor::read( is );
    bufSize = 0;
    is.readString( fileName, sizeof( fileName ) );
    if( isValid )
        {
        isValid = loadFile();
        uint sStart, sEnd, curs;
        is >> sStart >> sEnd >> curs;
        if( isValid && sEnd <= bufLen )
            {
            setSelect( sStart, sEnd, Boolean(curs == sStart) );
            trackCursor( True );
            }
        }
    return this;
}

TStreamable *TFileEditor::build()
{
    return new TFileEditor( streamableInit );
}

TFileEditor::TFileEditor( StreamableInit ) noexcept : TEditor( streamableInit )
{
}

#endif

/*------------------------------------------------------------*/
/* filename - teditor2.cpp                                    */
/*                                                            */
/* function(s)                                                */
/*            TEditor member functions                        */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TKeys
#define Uses_TEditor
#define Uses_TIndicator
#define Uses_TEvent
#define Uses_TScrollBar
#define Uses_TFindDialogRec
#define Uses_TReplaceDialogRec
#define Uses_TMenuItem
#define Uses_opstream
#define Uses_ipstream
#include <tvision/tv.h>

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __CTYPE_H )
#include <ctype.h>
#endif  // __CTYPE_H

#if !defined( __DOS_H )
#include <dos.h>
#endif  // __DOS_H

extern "C" {
int countLines( const char *buf, uint count );
uint scan( const char *block, uint size, const char *str );
uint iScan( const char *block, uint size, const char *str );
}

static int getCharType( char ch )
{
    if( strchr("\t \0", ch) )
        return 0;
    if( strchr("\n\r", ch) )
        return 1;
    if( strchr("!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~", ch) )
        return 2;
    return 3;
}

static inline int isWordBoundary( char a, char b )
{
    return getCharType(a) != getCharType(b);
}

static inline int isWordChar( int ch )
{
    return strchr(" !\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~\0", ch) == 0;
}

#pragma warn -asc


#pragma warn .asc

void TEditor::detectEol()
{
    for (uint p = 0; p < bufLen; ++p)
        if (bufChar(p) == '\r')
        {
            if (p+1 < bufLen && bufChar(p+1) == '\n')
                eolType = eolCrLf;
            else
                eolType = eolCr;
            return;
        }
        else if (bufChar(p) == '\n')
        {
            eolType = eolLf;
            return;
        }
    // Default to CRLF.
    eolType = eolCrLf;
}

Boolean TEditor::hasSelection()
{
    return Boolean(selStart != selEnd);
}

void TEditor::hideSelect()
{
    selecting = False;
    setSelect(curPtr, curPtr, False);
}

void TEditor::initBuffer()
{
    buffer = new char[bufSize];
}

TMenuItem& TEditor::initContextMenu( TPoint )
// The TPoint argument is the future location of the context menu.
// You can get the text under it with 'getMousePtr'.
{
    return
        *new TMenuItem( "Cu~t~", cmCut, kbShiftDel, hcNoContext, "Shift-Del" ) +
        *new TMenuItem( "~C~opy", cmCopy, kbCtrlIns, hcNoContext, "Ctrl-Ins" ) +
        *new TMenuItem( "~P~aste", cmPaste, kbShiftIns, hcNoContext, "Shift-Ins" ) +
        *new TMenuItem( "~U~ndo", cmUndo, kbCtrlU, hcNoContext, "Ctrl-U" );
}

uint TEditor::insertMultilineText( const char *text, uint length )
{
    size_t i = 0, j = 0;
    do  {
        if( text[i] == '\n' || text[i] == '\r' )
            {
            if( !insertText( &text[j], i - j, False ) ) return j;
            if( !insertEOL( False ) ) return i;
            if( i + 1 < length && text[i] == '\r' && text[i + 1] == '\n' )
                ++i;
            j = i + 1;
            }
        } while( ++i < length );
    if( !insertText( &text[j], i - j, False ) ) return j;
    return i;
}

Boolean TEditor::insertBuffer( const char *p,
                               uint offset,
                               uint length,
                               Boolean allowUndo,
                               Boolean selectText
                             )
{
    selecting = False;
    uint selLen = selEnd - selStart;
    if( selLen == 0 && length == 0 )
        return True;

    uint delLen = 0;
    if( allowUndo == True )
        {
        if( curPtr == selStart )
            delLen = selLen;
        else
            if( selLen > insCount )
                delLen = selLen - insCount;
        }

    ulong newSize = ulong(bufLen + delCount - selLen + delLen) + length;

    if( newSize > bufLen + delCount )
        {
        Boolean bufferText = Boolean( p >= buffer && p < buffer + bufLen );
        if( bufferText )
            p -= ptrdiff_t(buffer);
        if( newSize > UINT_MAX-0x1Fl || setBufSize(uint(newSize)) == False )
            {
            editorDialog( edOutOfMemory );
            selEnd = selStart;
            return False;
            }
        if( bufferText )
            p += ptrdiff_t(buffer);
        }

    uint selLines = countLines( &buffer[bufPtr(selStart)], selLen );
    if( curPtr == selEnd )
        {
        if( allowUndo == True )
            {
            if( delLen > 0 )
                memmove(
                         &buffer[curPtr + gapLen - delCount - delLen],
                         &buffer[selStart],
                         delLen
                       );
            insCount -= selLen - delLen;
            }
        curPtr = selStart;
        curPos.y -= selLines;
        }
    if( delta.y > curPos.y )
        {
        delta.y -= selLines;
        if( delta.y < curPos.y )
            delta.y = curPos.y;
        }

    if( length > 0 )
        memmove(
                &buffer[curPtr],
                &p[offset],
                length
               );

    uint lines = countLines( &buffer[curPtr], length );
    curPtr += length;
    curPos.y += lines;
    drawLine = curPos.y;
    drawPtr = lineStart(curPtr);
    curPos.x = charPos(drawPtr, curPtr);
    if( selectText == False )
        selStart = curPtr;
    selEnd = curPtr;
    bufLen += length - selLen;
    gapLen -= length - selLen;
    if( allowUndo == True )
        {
        delCount += delLen;
        insCount += length;
        }
    limit.y += lines - selLines;
    delta.y = max(0, min(delta.y, limit.y - size.y));
    if( isClipboard() == False )
        modified = True;
    setBufSize(bufLen + delCount);
    if( selLines == 0 && lines == 0 )
        update(ufLine);
    else
        update(ufView);
    return True;
}

Boolean TEditor::insertEOL( Boolean selectText )
{
    static const char * const eolBytes[] = { "\r\n", "\n", "\r" };
    const char *eol = eolBytes[eolType];
    return insertText( eol, strlen(eol), selectText );
}

Boolean TEditor::insertFrom( TEditor *editor )
{
    return insertBuffer( editor->buffer,
                         editor->bufPtr(editor->selStart),
                         editor->selEnd - editor->selStart,
                         canUndo,
                         isClipboard()
                        );
}

Boolean TEditor::insertText( const void *text, uint length, Boolean selectText )
{
  return insertBuffer( (const char *)text, 0, length, canUndo, selectText);
}

Boolean TEditor::isClipboard()
{
    return Boolean(clipboard == this);
}

uint TEditor::lineMove( uint p, int count )
{
    uint i = p;
    p = lineStart(p);
    int pos = charPos(p, i);
    while( count != 0 )
        {
        i = p;
        if( count < 0 )
            {
            p = prevLine(p);
            count++;
            }
        else
            {
            p = nextLine(p);
            count--;
            }
        }
    if( p != i )
        p = charPtr(p, pos);
    return p;
}

void TEditor::lock()
{
    lockCount++;
}

void TEditor::newLine()
{
    uint p = lineStart(curPtr);
    uint i = p;
    while( i < curPtr &&
           ( (buffer[i] == ' ') || (buffer[i] == '\x9'))
         )
         i++;
    insertEOL(False);
    if( autoIndent == True )
        insertText( &buffer[p], i - p, False);
}

uint TEditor::nextLine( uint p )
{
    return nextChar(lineEnd(p));
}

uint TEditor::nextWord( uint p )
{
    if (p < bufLen)
        {
        char a = bufChar(p);
        char b;
        do  {
            b = a;
            p = nextChar(p);
            } while( p < bufLen && !isWordBoundary((a = bufChar(p)), b) );
        }
    return p;
}

uint TEditor::prevLine( uint p )
{
  return lineStart(prevChar(p));
}

uint TEditor::prevWord( uint p )
{
    if (p > 0 && (p = prevChar(p), p > 0))
        {
        char a = bufChar(p);
        char b;
        do  {
            b = a;
            p = prevChar(p);
            a = bufChar(p);
            } while( p > 0 && !isWordBoundary(a, b) );
        if( isWordBoundary(a, b) )
            p = nextChar(p);
        }
    return p;
}

uint TEditor::indentedLineStart( uint P )
{
    uint startPtr = lineStart(P);
    uint destPtr = startPtr;
    char c;
    while( destPtr < bufLen && ((c = bufChar(destPtr)) == ' ' || c == '\t') )
        ++destPtr;
    return destPtr == P ? startPtr : destPtr;
}

void TEditor::replace()
{
    TReplaceDialogRec replaceRec( findStr, replaceStr, editorFlags );
    if( editorDialog( edReplace, &replaceRec ) != cmCancel )
        {
        strcpy( findStr, replaceRec.find );
        strcpy( replaceStr, replaceRec.replace );
        editorFlags = replaceRec.options | efDoReplace;
        doSearchReplace();
        }

}

void TEditor::scrollTo( int x, int y )
{
    x = max(0, min(x, limit.x - size.x));
    y = max(0, min(y, limit.y - size.y));
    if( x != delta.x || y != delta.y )
        {
        delta.x = x;
        delta.y = y;
        update(ufView);
        }
}

Boolean TEditor::search( const char *findStr, ushort opts )
{
    uint pos = curPtr;
    uint i;
    do  {
        if( (opts & efCaseSensitive) != 0 )
            i = scan( &buffer[bufPtr(pos)], bufLen - pos, findStr);
        else
            i = iScan( &buffer[bufPtr(pos)], bufLen - pos, findStr);

        if( i != sfSearchFailed )
            {
            i += pos;
            if( (opts & efWholeWordsOnly) == 0 ||
                !(
                    ( i != 0 && isWordChar(bufChar(i - 1)) != 0 ) ||
                    ( i + strlen(findStr) != bufLen &&
                        isWordChar(bufChar(i + strlen(findStr)))
                    )
                 ))
                {
                lock();
                setSelect(i, i + strlen(findStr), False);
                trackCursor(Boolean(!cursorVisible()));
                unlock();
                return True;
                }
            else
                pos = i + 1;
            }
        } while( i != sfSearchFailed );
    return False;
}

void TEditor::setBufLen( uint length )
{
    bufLen = length;
    gapLen = bufSize - length;
    selStart = 0;
    selEnd = 0;
    curPtr = 0;
    delta.x = 0;
    delta.y = 0;
    curPos = delta;
    limit.x = maxLineLength;
    limit.y = countLines( &buffer[gapLen], bufLen ) + 1;
    drawLine = 0;
    drawPtr = 0;
    delCount = 0;
    insCount = 0;
    modified = False;
    detectEol();
    update(ufView);
}

Boolean TEditor::setBufSize( uint newSize )
{
    return Boolean(newSize <= bufSize);
}

void TEditor::setCmdState( ushort command, Boolean enable )
{
    TCommandSet s;
    s += command;
    if( enable == True && (state & sfActive) != 0 )
        enableCommands(s);
    else
        disableCommands(s);
}

void TEditor::setCurPtr( uint p, uchar selectMode )
{
    uint anchor;
    if( (selectMode & smExtend) == 0 )
        anchor = p;
    else if( curPtr == selStart )
        anchor = selEnd;
    else
        anchor = selStart;

    if( p < anchor )
        {
        if( (selectMode & smDouble) != 0 )
            {
            p = prevWord(nextWord(p));
            anchor = nextWord(prevWord(anchor));
            }
        else if( (selectMode & smTriple) != 0 )
            {
            p = prevLine(nextLine(p));
            anchor = nextLine(prevLine(anchor));
            }
        setSelect(p, anchor, True);
        }
    else
        {
        if( (selectMode & smDouble) != 0 )
            {
            p = nextWord(p);
            anchor = prevWord(nextWord(anchor));
            }
        else if( (selectMode & smTriple) != 0 )
            {
            p = nextLine(p);
            anchor = prevLine(nextLine(anchor));
            }
        setSelect(anchor, p, False);
        }
}

void TEditor::setSelect( uint newStart, uint newEnd, Boolean curStart )
{
    uint p;
    if( curStart != 0 )
        p = newStart;
    else
        p = newEnd;

    uchar flags = ufUpdate;

    if( newStart != selStart || newEnd != selEnd )
        if( newStart != newEnd || selStart != selEnd )
            flags = ufView;

    if( p != curPtr )
        {
        if( p > curPtr )
            {
            uint l = p - curPtr;
            memmove( &buffer[curPtr], &buffer[curPtr + gapLen], l);
            curPos.y += countLines(&buffer[curPtr], l);
            curPtr = p;
            }
        else
            {
            uint l = curPtr - p;
            curPtr = p;
            curPos.y -= countLines(&buffer[curPtr], l);
            memmove( &buffer[curPtr + gapLen], &buffer[curPtr], l);
            }
        delCount = 0;
        insCount = 0;
        setBufSize(bufLen);
        }
    drawLine = curPos.y;
    drawPtr = lineStart(p);
    curPos.x = charPos(drawPtr, p);
    selStart = newStart;
    selEnd = newEnd;
    update(flags);
}

void TEditor::setState( ushort aState, Boolean enable )
{
    TView::setState(aState, enable);
    switch( aState )
        {
        case sfActive:
            if( hScrollBar != 0 )
                hScrollBar->setState(sfVisible, enable);
            if( vScrollBar != 0 )
                vScrollBar->setState(sfVisible, enable);
            if( indicator != 0 )
                indicator->setState(sfVisible, enable);
            updateCommands();
            break;

        case sfExposed:
            if( enable == True )
                unlock();
        }
}

void TEditor::startSelect()
{
    hideSelect();
    selecting = True;
}

void TEditor::toggleEncoding()
{
    if( encoding == encDefault )
        encoding = encSingleByte;
    else
        encoding = encDefault;
    updateFlags |= ufView;
    setSelect(selStart, selEnd, Boolean( curPtr < selEnd ));
}

void TEditor::toggleInsMode()
{
    overwrite = Boolean(!overwrite);
    setState(sfCursorIns, Boolean(!getState(sfCursorIns)));
}

void TEditor::trackCursor( Boolean center )
{
    if( center == True )
        scrollTo( curPos.x - size.x + 1, curPos.y - size.y / 2);
    else
        scrollTo( max(curPos.x - size.x + 1, min(delta.x, curPos.x)),
                  max(curPos.y - size.y + 1, min(delta.y, curPos.y)));
}

void TEditor::undo()
{
    if( delCount != 0 || insCount != 0 )
        {
        selStart = curPtr - insCount;
        selEnd = curPtr;
        uint length = delCount;
        delCount = 0;
        insCount = 0;
        insertBuffer(buffer, curPtr + gapLen - length, length, False, True);
        }
}

void TEditor::unlock()
{
    if( lockCount > 0 )
        {
        lockCount--;
        if( lockCount == 0 )
            doUpdate();
        }
}

void TEditor::update( uchar aFlags )
{
    updateFlags |= aFlags;
    if( lockCount == 0 )
        doUpdate();
}

void TEditor::updateCommands()
{
    setCmdState( cmUndo, Boolean( delCount != 0 || insCount != 0 ) );
    if( isClipboard() == False )
        {
        setCmdState(cmCut, hasSelection());
        setCmdState(cmCopy, hasSelection());
        setCmdState(cmPaste,
                    Boolean(clipboard == 0 || clipboard->hasSelection()) );
        }
    setCmdState(cmClear, hasSelection());
    setCmdState(cmFind, True);
    setCmdState(cmReplace, True);
    setCmdState(cmSearchAgain, True);
}

Boolean TEditor::valid( ushort )
{
  return isValid;
}

#if !defined(NO_STREAMABLE)

void TEditor::write( opstream& os )
{
    TView::write( os );
    os << hScrollBar << vScrollBar << indicator
       << bufSize << (uchar)canUndo << (uchar)eolType
       << (uchar)encoding;
}

void *TEditor::read( ipstream& is )
{
    TView::read( is );
    is >> hScrollBar >> vScrollBar >> indicator
       >> bufSize;
    uchar temp;
    is >> temp; canUndo = Boolean(temp);
    is >> temp; eolType = EolType(temp);
    is >> temp; encoding = Encoding(temp);
    selecting = False;
    overwrite = False;
    autoIndent = True;
    lockCount = 0;
    updateFlags = 0;
    keyState = 0;
    initBuffer();
    if( buffer != 0 )
        isValid = True;
    else
        {
        isValid = False;
        TEditor::editorDialog( edOutOfMemory, 0 );
        bufSize = 0;
        }
    lockCount = 0;
    lock();
    setBufLen( 0 );
    return this;
}

TStreamable *TEditor::build()
{
    return new TEditor( streamableInit );
}

TEditor::TEditor( StreamableInit ) noexcept : TView( streamableInit )
{
}

#endif

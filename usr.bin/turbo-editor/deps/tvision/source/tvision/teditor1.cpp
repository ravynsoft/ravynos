/*------------------------------------------------------------*/
/* filename - teditor1.cpp                                    */
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
#define Uses_opstream
#define Uses_ipstream
#define Uses_TText
#define Uses_TClipboard
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

#if !defined( __MALLOC_H )
#include <malloc.h>
#endif  // __MALLOC_H

#if !defined( __STDLIB_H )
#include <stdlib.h>
#endif  // __STDLIB_H

#ifndef __BORLANDC__
#define register
#endif

const ushort firstKeys[] =
{
    41,
    kbCtrlA, cmSelectAll,
    kbCtrlC, cmPageDown,
    kbCtrlD, cmCharRight,
    kbCtrlE, cmLineUp,
    kbCtrlF, cmWordRight,
    kbCtrlG, cmDelChar,
    kbCtrlH, cmBackSpace,
    kbCtrlK, 0xFF02,
    kbCtrlL, cmSearchAgain,
    kbCtrlM, cmNewLine,
    kbCtrlO, cmIndentMode,
    kbCtrlP, cmEncoding,
    kbCtrlQ, 0xFF01,
    kbCtrlR, cmPageUp,
    kbCtrlS, cmCharLeft,
    kbCtrlT, cmDelWord,
    kbCtrlU, cmUndo,
    kbCtrlV, cmInsMode,
    kbCtrlX, cmLineDown,
    kbCtrlY, cmDelLine,
    kbLeft, cmCharLeft,
    kbRight, cmCharRight,
    kbAltBack, cmDelWordLeft,
    kbCtrlBack, cmDelWordLeft,
    kbCtrlDel, cmDelWord,
    kbCtrlLeft, cmWordLeft,
    kbCtrlRight, cmWordRight,
    kbHome, cmLineStart,
    kbEnd, cmLineEnd,
    kbUp, cmLineUp,
    kbDown, cmLineDown,
    kbPgUp, cmPageUp,
    kbPgDn, cmPageDown,
    kbCtrlHome, cmTextStart,
    kbCtrlEnd, cmTextEnd,
    kbIns, cmInsMode,
    kbDel, cmDelChar,
    kbShiftIns, cmPaste,
    kbShiftDel, cmCut,
    kbCtrlIns, cmCopy,
    kbCtrlDel, cmClear
};

const ushort quickKeys[] =
{   8,
    'A', cmReplace,
    'C', cmTextEnd,
    'D', cmLineEnd,
    'F', cmFind,
    'H', cmDelStart,
    'R', cmTextStart,
    'S', cmLineStart,
    'Y', cmDelEnd
};

const ushort blockKeys[] =
{   5,
    'B', cmStartSelect,
    'C', cmPaste,
    'H', cmHideSelect,
    'K', cmCopy,
    'Y', cmCut
};

const ushort *keyMap[] = { firstKeys, quickKeys, blockKeys };

ushort defEditorDialog( int, ... );

#pragma warn -asc

ushort scanKeyMap( const void *keyMap, ushort keyCode )
{
#if !defined(__FLAT__)
asm {
    PUSH DS
    LDS SI,keyMap
    MOV DX,keyCode
    CLD
    LODSW
    MOV CX,AX
    }
__1:
asm {
    LODSW
    MOV BX,AX
    LODSW
    CMP BL,DL
    JNE __3
    OR  BH,BH
    JE  __4
    CMP BH,DH
    JE  __4
    }
__3:
asm {
    LOOP    __1
    XOR AX,AX
    }
__4:
asm POP DS
    return _AX;
#else
    register ushort *kM = (ushort *)keyMap;
    uchar codeLow = keyCode & 0xff;
    uchar codeHi  = keyCode >> 8;

    int n;

    for (n = *kM++; n--; kM++)
    {
        uchar  mapLow  = *kM & 0xff;
        uchar  mapHi   = *kM >> 8;
        kM++;
        ushort command = *kM;

        if ((mapLow == codeLow) && ((mapHi == 0) || (mapHi == codeHi)))
            return command;
    };
    return 0;
#endif
}

#pragma warn .asc

#define cpEditor    "\x06\x07"

TEditor::TEditor( const TRect& bounds,
                  TScrollBar *aHScrollBar,
                  TScrollBar *aVScrollBar,
                  TIndicator *aIndicator,
                  uint aBufSize ) noexcept :
    TView( bounds ),
    hScrollBar( aHScrollBar ),
    vScrollBar( aVScrollBar ),
    indicator( aIndicator ),
    bufSize( aBufSize ),
    canUndo( True ),
    selecting( False ),
    overwrite( False ),
    autoIndent( True ) ,
    encoding( encDefault ),
    lockCount( 0 ),
    updateFlags( 0 ),
    keyState( 0 )
{
    growMode = gfGrowHiX | gfGrowHiY;
    options |= ofSelectable;
    eventMask = evMouseDown | evKeyDown | evCommand | evBroadcast;
    showCursor();
    initBuffer();
    if( buffer != 0 )
        isValid = True;
    else
    {
        editorDialog( edOutOfMemory );
        bufSize = 0;
        isValid = False;
    }
    setBufLen(0);
}

TEditor::~TEditor()
{
}

void TEditor::shutDown()
{
    doneBuffer();
    TView::shutDown();
}

void TEditor::changeBounds( const TRect& bounds )
{
    setBounds(bounds);
    delta.x = max(0, min(delta.x, limit.x - size.x));
    delta.y = max(0, min(delta.y, limit.y - size.y));
    update(ufView);
}

TStringView TEditor::bufChars( uint P )
{
    static thread_local char buf[maxCharLength];
    if (encoding == encSingleByte)
        {
        buf[0] = bufChar(P);
        return TStringView(buf, 1);
        }
    else
        {
        int len = min(max(max(curPtr, bufLen) - P, 1), sizeof(buf));
        for (int i = 0; i < len; ++i)
            buf[i] = bufChar(P + i);
        return TStringView(buf, len);
        }
}

TStringView TEditor::prevBufChars( uint P )
{
    static thread_local char buf[maxCharLength];
    if (encoding == encSingleByte)
        {
        buf[0] = bufChar(P - 1);
        return TStringView(buf, 1);
        }
    else
        {
        int len = min(max(P, 1), sizeof(buf));
        for (int i = 0; i < len; ++i)
            buf[i] = bufChar(P - len + i);
        return TStringView(buf, len);
        }
}

void TEditor::nextChar( TStringView s, uint &p, uint &width )
{
    if (encoding == encSingleByte || s.size() == 0)
        {
        ++p;
        ++width;
        }
    else
        {
        size_t p_ = 0, w_ = 0;
        TText::next(s, p_, w_);
        p += p_; width += w_;
        }
}

Boolean TEditor::formatCell( TSpan<TScreenCell> cells, uint &width,
                             TStringView text, uint &p, TColorAttr color )
{
    size_t p_ = 0, w_ = width;
    if (TText::drawOne(cells, w_, text, p_, color))
        {
        p += p_; width = w_;
        return True;
        }
    return False;
}

int TEditor::charPos( uint p, uint target )
{
    uint pos = 0;
    while( p < target )
    {
        TStringView chars = bufChars(p);
        if( chars[0] == '\x9' )
            pos |= 7;
        nextChar(chars, p, pos);
    }
    return pos;
}

uint TEditor::charPtr( uint p, int target )
{
    uint pos = 0;
    uint lastP = p;
    char c;
    TStringView chars;
    while( (int) pos < target && p < bufLen &&
           (c = (chars = bufChars(p))[0]) != '\r' && c != '\n' )
        {
        lastP = p;
        if( c == '\x09' )
            pos |= 7;
        nextChar(chars, p, pos);
        }
    if( (int) pos > target)
        p = lastP;
    return p;
}

Boolean TEditor::clipCopy()
{
    Boolean res = False;
    if( clipboard != this )
        {
        if( clipboard != 0 )
            res = clipboard->insertFrom(this);
        else
            {
            TClipboard::setText( TStringView( buffer + bufPtr(selStart),
                                              selEnd - selStart ) );
            res = True;
            }
        selecting = False;
        update(ufUpdate);
        }
    return res;
}

void TEditor::clipCut()
{
    if( clipCopy() == True )
        deleteSelect();
}

void TEditor::clipPaste()
{
    if( clipboard != this )
        {
        if( clipboard != 0 )
            insertFrom(clipboard);
        else
            TClipboard::requestText();
        }
}

void TEditor::convertEvent( TEvent& event )
{
    if( event.what == evKeyDown )
        {
        if( (event.keyDown.controlKeyState & kbShift) != 0 &&
            event.keyDown.charScan.scanCode >= 0x47 &&
            event.keyDown.charScan.scanCode <= 0x51
          )
            event.keyDown.charScan.charCode = 0;

        ushort key = event.keyDown.keyCode;
        if( keyState != 0 )
            {
            if( (key & 0xFF) >= 0x01 && (key & 0xFF) <= 0x1A )
                key += 0x40;
            if( (key & 0xFF) >= 0x61 && (key & 0xFF) <= 0x7A )
                key -= 0x20;
            }
        key = scanKeyMap(keyMap[keyState], key);
        keyState = 0;
        if( key != 0 )
            {
            if( (key & 0xFF00) == 0xFF00 )
                {
                keyState = (key & 0xFF);
                clearEvent(event);
                }
            else
                {
                event.what = evCommand;
                event.message.command = key;
                }
            }
        }
}

Boolean TEditor::cursorVisible()
{
  return Boolean((curPos.y >= delta.y) && (curPos.y < delta.y + size.y));
}

void TEditor::deleteRange( uint startPtr,
                           uint endPtr,
                           Boolean delSelect
                         )
{
    if( hasSelection() == True && delSelect == True )
        deleteSelect();
    else
        {
        setSelect(curPtr, endPtr, True);
        deleteSelect();
        setSelect(startPtr, curPtr, False);
        deleteSelect();
        }
}

void TEditor::deleteSelect()
{
    insertText( 0, 0, False );
}

void TEditor::doneBuffer()
{
    delete[] buffer;
}

void TEditor::doSearchReplace()
{
    int i;
    do  {
        i = cmCancel;
        if( search(findStr, editorFlags) == False )
            {
            if( (editorFlags & (efReplaceAll | efDoReplace)) !=
                (efReplaceAll | efDoReplace) )
                    editorDialog( edSearchFailed );
            }
        else
            if( (editorFlags & efDoReplace) != 0 )
                {
                i = cmYes;
                if( (editorFlags & efPromptOnReplace) != 0 )
                    {
                    TPoint c = makeGlobal( cursor );
                    i = editorDialog( edReplacePrompt, &c );
                    }
                if( i == cmYes )
                    {
                    lock();
                    insertText( replaceStr, strlen(replaceStr), False);
                    trackCursor(False);
                    unlock();
                    }
                }
        } while( i != cmCancel && (editorFlags & efReplaceAll) != 0 );
}

void TEditor::doUpdate()
{
    if( updateFlags != 0 )
        {
        setCursor(curPos.x - delta.x, curPos.y - delta.y);
        if( (updateFlags & ufView) != 0 )
            drawView();
        else
            if( (updateFlags & ufLine) != 0 )
                drawLines( curPos.y-delta.y, 1, lineStart(curPtr) );
        if( hScrollBar != 0 )
            hScrollBar->setParams(delta.x, 0, limit.x - size.x, size.x / 2, 1);
        if( vScrollBar != 0 )
            vScrollBar->setParams(delta.y, 0, limit.y - size.y, size.y - 1, 1);
        if( indicator != 0 )
            indicator->setValue(curPos, modified);
        if( (state & sfActive) != 0 )
            updateCommands();
        updateFlags = 0;
        }
}

void TEditor::draw()
{
    if( drawLine != delta.y )
        {
        drawPtr = lineMove( drawPtr, delta.y - drawLine );
        drawLine = delta.y;
        }
    drawLines( 0, size.y, drawPtr );
}

void TEditor::drawLines( int y, int count, uint linePtr )
{
    TAttrPair color = getColor(0x0201);
#ifndef __FLAT__
    TScreenCell b[maxLineLength];
#else
    TScreenCell *b = (TScreenCell*) alloca(sizeof(TScreenCell)*(delta.x+size.x));
#endif
    while( count-- > 0 )
        {
        formatLine( b, linePtr, delta.x+size.x, color );
        writeBuf(0, y, size.x, 1, &b[delta.x]);
        linePtr = nextLine(linePtr);
        y++;
        }
}

void TEditor::find()
{
    TFindDialogRec findRec( findStr, editorFlags );
    if( editorDialog( edFind, &findRec ) != cmCancel )
        {
        strcpy( findStr, findRec.find );
        editorFlags = findRec.options & ~efDoReplace;
        doSearchReplace();
        }
}

uint TEditor::getMousePtr( TPoint m )
{
    TPoint mouse = makeLocal( m );
    mouse.x = max(0, min(mouse.x, size.x - 1));
    mouse.y = max(0, min(mouse.y, size.y - 1));
    return charPtr(lineMove(drawPtr, mouse.y + delta.y - drawLine),
        mouse.x + delta.x);
}

TPalette& TEditor::getPalette() const
{
    static TPalette palette( cpEditor, sizeof( cpEditor )-1 );
    return palette;
}

void TEditor::checkScrollBar( const TEvent& event,
                              TScrollBar *p,
                              int& d
                            )
{
    if( (event.message.infoPtr == p) && (p->value != d) )
        {
        d = p->value;
        update( ufView );
        }
}

void TEditor::handleEvent( TEvent& event )
{
    TView::handleEvent( event );

    Boolean centerCursor = Boolean(!cursorVisible());
    uchar selectMode = 0;

    if( selecting == True ||
        (event.what & evMouse && (event.mouse.controlKeyState & kbShift) != 0) ||
        (event.what & evKeyboard && (event.keyDown.controlKeyState & kbShift ) != 0)
      )
        selectMode = smExtend;

    convertEvent( event );

    switch( event.what )
        {

        case evMouseDown:
            if( event.mouse.buttons & mbRightButton )
                {
                TMenuItem &menu = initContextMenu( event.mouse.where );
                popupMenu( event.mouse.where, menu, owner );
                break;
                }

            if( event.mouse.buttons & mbMiddleButton )
                {
                TPoint lastMouse = makeLocal( event.mouse.where );
                while( mouseEvent(event, evMouse) )
                    {
                    TPoint mouse = makeLocal( event.mouse.where );
                    TPoint d = delta + (lastMouse - mouse);
                    scrollTo(d.x, d.y);
                    lastMouse = mouse;
                    }
                break;
                }

            if( event.mouse.eventFlags & meDoubleClick )
                selectMode |= smDouble;
            else if( event.mouse.eventFlags & meTripleClick )
                selectMode |= smTriple;

            do  {
                lock();
                if( event.what == evMouseAuto )
                    {
                    TPoint mouse = makeLocal( event.mouse.where );
                    TPoint d = delta;
                    if( mouse.x < 0 )
                        d.x--;
                    if( mouse.x >= size.x )
                        d.x++;
                    if( mouse.y < 0 )
                        d.y--;
                    if( mouse.y >= size.y )
                        d.y++;
                    scrollTo(d.x, d.y);
                    }
                else if( event.what == evMouseWheel )
                    {
                    TEvent ev = event;
                    vScrollBar->handleEvent(ev);
                    hScrollBar->handleEvent(ev);
                    }
                setCurPtr(getMousePtr(event.mouse.where), selectMode);
                selectMode |= smExtend;
                unlock();
                } while( mouseEvent(event, evMouseMove + evMouseAuto + evMouseWheel) );
            break;

        case evKeyDown:
            if( ( encoding != encSingleByte && event.keyDown.textLength > 0 ) ||
                event.keyDown.charScan.charCode == 9 ||
                ( event.keyDown.charScan.charCode >= 32 && event.keyDown.charScan.charCode < 255 )
              )
                {
                lock();
                if( event.keyDown.controlKeyState & kbPaste )
                    {
                    char buf[512];
                    size_t length;
                    while( textEvent( event, TSpan<char>(buf, sizeof(buf)), length ) )
                        insertMultilineText( buf, (uint) length );
                    }
                else
                    {
                    if( overwrite == True && hasSelection() == False )
                        if( curPtr != lineEnd(curPtr) )
                            selEnd = nextChar(curPtr);

                    if( encoding != encSingleByte && event.keyDown.textLength > 0 )
                        insertText( event.keyDown.text, event.keyDown.textLength, False );
                    else
                        insertText( &event.keyDown.charScan.charCode, 1, False );
                    }

                trackCursor(centerCursor);
                unlock();
                }
            else
                return;
            break;

        case evCommand:
            switch( event.message.command )
                {
                case cmFind:
                    find();
                    break;
                case cmReplace:
                    replace();
                    break;
                case cmSearchAgain:
                    doSearchReplace();
                    break;
                case cmEncoding:
                    toggleEncoding();
                    break;
                default:
                    lock();
                    switch( event.message.command )
                        {
                        case cmCut:
                            clipCut();
                            break;
                        case cmCopy:
                            clipCopy();
                            break;
                        case cmPaste:
                            clipPaste();
                            break;
                        case cmUndo:
                            undo();
                            break;
                        case cmClear:
                            deleteSelect();
                            break;
                        case cmCharLeft:
                            setCurPtr(prevChar(curPtr), selectMode);
                            break;
                        case cmCharRight:
                            setCurPtr(nextChar(curPtr), selectMode);
                            break;
                        case cmWordLeft:
                            setCurPtr(prevWord(curPtr), selectMode);
                            break;
                        case cmWordRight:
                            setCurPtr(nextWord(curPtr), selectMode);
                            break;
                        case cmLineStart:
                            setCurPtr(autoIndent ? indentedLineStart(curPtr) : lineStart(curPtr), selectMode);
                            break;
                        case cmLineEnd:
                            setCurPtr(lineEnd(curPtr), selectMode);
                            break;
                        case cmLineUp:
                            setCurPtr(lineMove(curPtr, -1), selectMode);
                            break;
                        case cmLineDown:
                            setCurPtr(lineMove(curPtr, 1), selectMode);
                            break;
                        case cmPageUp:
                            setCurPtr(lineMove(curPtr, -(size.y-1)), selectMode);
                            break;
                        case cmPageDown:
                            setCurPtr(lineMove(curPtr, size.y-1), selectMode);
                            break;
                        case cmTextStart:
                            setCurPtr(0, selectMode);
                            break;
                        case cmTextEnd:
                            setCurPtr(bufLen, selectMode);
                            break;
                        case cmNewLine:
                            newLine();
                            break;
                        case cmBackSpace:
                            deleteRange(prevChar(curPtr), curPtr, True);
                            break;
                        case cmDelChar:
                            deleteRange(curPtr, nextChar(curPtr), True);
                            break;
                        case cmDelWord:
                            deleteRange(curPtr, nextWord(curPtr), False);
                            break;
                        case cmDelWordLeft:
                            deleteRange(prevWord(curPtr), curPtr, False);
                            break;
                        case cmDelStart:
                            deleteRange(lineStart(curPtr), curPtr, False);
                            break;
                        case cmDelEnd:
                            deleteRange(curPtr, lineEnd(curPtr), False);
                            break;
                        case cmDelLine:
                            deleteRange(lineStart(curPtr), nextLine(curPtr), False);
                            break;
                        case cmInsMode:
                            toggleInsMode();
                            break;
                        case cmStartSelect:
                            startSelect();
                            break;
                        case cmHideSelect:
                            hideSelect();
                            break;
                        case cmIndentMode:
                            autoIndent = Boolean(!autoIndent);
                            break;
                        case cmSelectAll:
                            setCurPtr(0, selectMode);
                            selectMode |= smExtend;
                            setCurPtr(bufLen, selectMode);
                            break;
                        default:
                            unlock();
                            return;
                        }
                    trackCursor(centerCursor);
                    unlock();
                    break;
                }
            break;

        case evBroadcast:
            switch( event.message.command )
                {
                case cmScrollBarChanged:
                    if ((event.message.infoPtr == hScrollBar) ||
                        (event.message.infoPtr == vScrollBar))
                        {
                        checkScrollBar( event, hScrollBar, delta.x );
                        checkScrollBar( event, vScrollBar, delta.y );
                        }
                    else
                        return;
                    break;
                default:
                    return;
                }
            break;
        }
    clearEvent(event);
}


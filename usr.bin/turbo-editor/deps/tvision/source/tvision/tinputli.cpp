/*------------------------------------------------------------*/
/* filename -       tinputli.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TInputLine member functions               */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TGroup
#define Uses_TKeys
#define Uses_TInputLine
#define Uses_TDrawBuffer
#define Uses_TEvent
#define Uses_TValidator
#define Uses_opstream
#define Uses_ipstream
#define Uses_TText
#define Uses_TClipboard
#include <tvision/tv.h>

#if !defined( __CTYPE_H )
#include <ctype.h>
#endif  // __CTYPE_H

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __DOS_H )
#include <dos.h>
#endif  // __DOS_H

const int CONTROL_Y = 25;

char hotKey( const char *s ) noexcept
{
    char *p;

    if( (p = strchr( (char *) s, '~' )) != 0 )
        return toupper(p[1]);
    else
        return 0;
}

static int prevWord(const char *s, int pos) noexcept
{
    for (int i = pos - 1; i >= 1; --i)
        {
        if( s[i] != ' ' && s[i - 1] == ' ' )
            return i;
        }
    return 0;
};

static int nextWord(TStringView s, int pos) noexcept
{
    for (int i = pos; i < int(s.size()) - 1; ++i)
        {
        if( s[i] == ' ' && s[i + 1] != ' ' )
            return i + 1;
        }
    return int(s.size());
};

#define cpInputLine "\x13\x13\x14\x15"

TInputLine::TInputLine( const TRect& bounds, uint limit, TValidator *aValid, ushort limitMode ) noexcept :
    TView(bounds),
    maxLen  ( (limitMode == ilMaxBytes) ? min(max(limit-1, 0), 255) : 255 ),
    maxWidth( (limitMode == ilMaxWidth) ? limit : UINT_MAX ),
    maxChars( (limitMode == ilMaxChars) ? limit : UINT_MAX ),
    curPos( 0 ),
    firstPos( 0 ),
    selStart( 0 ),
    selEnd( 0 ),
    validator( aValid )
{
    data = new char[maxLen + 1];
    oldData = new char[maxLen + 1];

    state |= sfCursorVis;
    options |= ofSelectable | ofFirstClick;
    *data = EOS;
}

TInputLine::~TInputLine()
{
    delete[] data;
    delete[] oldData;
    destroy(validator);
}

Boolean TInputLine::canScroll( int delta )
{
    if( delta < 0 )
        return Boolean( firstPos > 0 );
    else
        if( delta > 0 )
            return Boolean( strwidth(data) - firstPos + 2 > size.x );
        else
            return False;
}

ushort TInputLine::dataSize()
{
    ushort dSize = 0;

    if (validator)
        dSize = validator->transfer(data, NULL, vtDataSize);
    if (dSize == 0)
        dSize = maxLen + 1;
    return dSize;
}

void TInputLine::draw()
{
    int l, r;
    TDrawBuffer b;

    TColorAttr color = getColor( (state & sfFocused) ? 2 : 1 );

    b.moveChar( 0, ' ', color, size.x );
    if( size.x > 1 )
        {
        b.moveStr( 1, data, color, size.x - 1, firstPos);
        }
    if( canScroll(1) )
        b.moveChar( size.x-1, rightArrow, getColor(4), 1 );
    if( canScroll(-1) )
        b.moveChar( 0, leftArrow, getColor(4), 1 );
    if( (state & sfSelected) != 0 )
        {
        l = displayedPos(selStart) - firstPos;
        r = displayedPos(selEnd) - firstPos;
        l = max( 0, l );
        r = min( size.x - 2, r );
        if (l <  r)
            b.moveChar( l+1, 0, getColor(3), r - l );
        }
    writeLine( 0, 0, size.x, size.y, b );
    setCursor( displayedPos(curPos)-firstPos+1, 0);
}

void TInputLine::getData( void *rec )
{
    if ((validator == 0) || (validator->transfer(data, rec, vtGetData) == 0))
        memcpy( rec, data, dataSize() );
}

TPalette& TInputLine::getPalette() const
{
    static TPalette palette( cpInputLine, sizeof( cpInputLine )-1 );
    return palette;
}

int TInputLine::mouseDelta( TEvent& event )
{
    TPoint mouse = makeLocal( event.mouse.where );

    if( mouse.x <= 0 )
        return -1;
    else
        if( mouse.x >= size.x - 1 )
            return 1;
        else
            return 0;
}

int TInputLine::mousePos( TEvent& event )
{
    TPoint mouse = makeLocal( event.mouse.where );
    mouse.x = max( mouse.x, 1 );
    int pos = mouse.x + firstPos - 1;
    pos = max( pos, 0 );
    TStringView text = data;
    return TText::scroll(text, pos, False);
}

int TInputLine::displayedPos( int pos )
{
    return strwidth( TStringView(data, pos) );
}

void TInputLine::deleteSelect()
{
    if( selStart < selEnd )
        {
        int len = strlen(data);
        memmove( data+selStart, data+selEnd, len-selEnd );
        data[len-selEnd+selStart] = EOS;
        curPos = selStart;
        }
}

void TInputLine::deleteCurrent()
{
    TStringView text = data;
    if( curPos < (int) text.size() )
        {
        selStart = curPos;
        selEnd = curPos + TText::next(text.substr(curPos));
        deleteSelect();
        }
}

void TInputLine::adjustSelectBlock()
{
    if (curPos < anchor)
        {
        selStart = curPos;
        selEnd =  anchor;
        }
    else
        {
        selStart = anchor;
        selEnd = curPos;
        }
}

void TInputLine::saveState()
{
    if (validator)
        {
        strcpy(oldData,data);
        oldCurPos = curPos;
        oldFirstPos = firstPos;
        oldSelStart = selStart;
        oldSelEnd = selEnd;
        }
}

void TInputLine::restoreState()
{
    if (validator)
        {
        strcpy(data, oldData);
        curPos = oldCurPos;
        firstPos = oldFirstPos;
        selStart = oldSelStart;
        selEnd = oldSelEnd;
        }
}

Boolean TInputLine::checkValid(Boolean noAutoFill)
{
    int oldLen, newLen;
    char newData[256];

    if (validator)
        {
        oldLen = strlen(data);
        strcpy(newData, data);
        if (!validator->isValidInput(newData, noAutoFill))
            {
            restoreState();
            return False;
            }
        else
            {
            if (strlen(newData) > maxLen)
                newData[maxLen] = 0;
            strcpy(data, newData);
            newLen = strlen(data);
            if ((curPos >= oldLen) && (newLen > oldLen))
                curPos = newLen;
            return True;
            }
        }
    else
        return True;
}


void TInputLine::handleEvent( TEvent& event )
{
    Boolean extendBlock;
    /* Home, Left Arrow, Right Arrow, End, Ctrl-Left Arrow, Ctrl-Right Arrow */
    static const char padKeys[] = {0x47,0x4b,0x4d,0x4f,0x73,0x74};
    TView::handleEvent(event);

    char keyText[ sizeof( event.keyDown.text )+1 ];
    int delta, i, len, curWidth;
    if( (state & sfSelected) != 0 )
        {
        switch( event.what )
            {
            case evMouseDown:
                if( canScroll(delta = mouseDelta(event)) )
                    do  {
                        if( canScroll(delta) )
                            {
                            firstPos += delta;
                            drawView();
                            }
                        } while( mouseEvent( event, evMouseAuto ) );
                else if (event.mouse.eventFlags & meDoubleClick)
                    selectAll(True);
                else
                    {
                    anchor =  mousePos(event);
                    do  {
                        if( event.what == evMouseAuto)
                            {
                            delta = mouseDelta(event);
                            if (canScroll(delta))
                                firstPos += delta;
                            }
                        curPos = mousePos(event);
                        adjustSelectBlock();
                        drawView();
                        }
                        while (mouseEvent(event,evMouseMove | evMouseAuto));
                    }
                clearEvent(event);
                break;
            case evKeyDown:
                saveState();
                event.keyDown.keyCode = ctrlToArrow(event.keyDown.keyCode);
                if( memchr(padKeys, event.keyDown.charScan.scanCode, sizeof(padKeys)) != 0 &&
                    (event.keyDown.controlKeyState & kbShift) != 0
                  )
                    {
                    event.keyDown.charScan.charCode = 0;
                    if (curPos == selEnd)
                        anchor = selStart;
                    else if (selStart == selEnd)
                        anchor = curPos;
                    else
                        anchor = selEnd;
                    extendBlock = True;
                    }
                else
                    extendBlock = False;

                switch( event.keyDown.keyCode )
                    {
                    case kbLeft:
                        curPos -= TText::prev(data, curPos);
                        break;
                    case kbRight:
                        curPos += TText::next(data+curPos);
                        break;
                    case kbCtrlLeft:
                        curPos = prevWord(data, curPos);
                        break;
                    case kbCtrlRight:
                        curPos = nextWord(data, curPos);
                        break;
                    case kbHome:
                        curPos =  0;
                        break;
                    case kbEnd:
                        curPos = strlen(data);
                        break;
                    case kbBack:
                        if( selStart == selEnd )
                            {
                            selStart = curPos - TText::prev(data, curPos);
                            selEnd = curPos;
                            }
                        deleteSelect();
                        checkValid(True);
                        break;
                    case kbCtrlBack:
                    case kbAltBack:
                        if( selStart == selEnd )
                            {
                            selStart = prevWord(data, curPos);
                            selEnd = curPos;
                            }
                        deleteSelect();
                        checkValid(True);
                        break;
                    case kbDel:
                        if( selStart == selEnd )
                            deleteCurrent();
                        else
                            deleteSelect();
                        checkValid(True);
                        break;
                    case kbCtrlDel:
                        if( selStart == selEnd )
                            {
                            selStart = curPos;
                            selEnd = nextWord(data, curPos);
                            }
                        deleteSelect();
                        checkValid(True);
                    case kbIns:
                        setState(sfCursorIns, Boolean(!(state & sfCursorIns)));
                        break;
                    default:
                        // The event text may contain null characters, but 'data' is null-terminated,
                        // so rely on strlen to measure the text length.
                        strnzcpy( keyText, event.keyDown.getText(), sizeof( keyText ) );
                        if( (len = strlen(keyText)) > 0 )
                            {
                            deleteSelect();
                            if( (state & sfCursorIns) != 0 )
                                deleteCurrent();

                            if( checkValid(True) )
                                {
                                if( strchr("\t\r\n", keyText[0]) != 0 )
                                    keyText[0] = ' '; // Replace tabs and newlines into spaces.
                                TTextMetrics dataMts = TText::measure(data);
                                TTextMetrics keyMts = TText::measure(keyText);
                                if( strlen(data) + len <= maxLen &&
                                    dataMts.width + keyMts.width <= maxWidth &&
                                    dataMts.graphemeCount + keyMts.graphemeCount <= maxChars
                                  )
                                    {
                                    if( firstPos > curPos )
                                        firstPos = curPos;
                                    memmove( data+curPos+len, data+curPos, strlen(data+curPos)+1 );
                                    memcpy( data+curPos, keyText, len );
                                    curPos += len;
                                    }
                                checkValid(False);
                                }
                            }
                        else if( event.keyDown.charScan.charCode == CONTROL_Y )
                            {
                            *data = EOS;
                            curPos = 0;
                            }
                        else
                            return;
                    }
                if (extendBlock)
                    adjustSelectBlock();
                else
                    selStart = selEnd = 0;
                curWidth = displayedPos(curPos);
                if( firstPos > curWidth )
                    firstPos = curWidth;
                i = curWidth - size.x + 2;
                if( firstPos < i )
                    firstPos = i;
                drawView();
                clearEvent( event );
                break;
            case evCommand:
                if( event.message.command == cmPaste )
                    {
                    TClipboard::requestText();
                    clearEvent( event );
                    }
                else if( event.message.command == cmCut || event.message.command == cmCopy )
                    {
                    TStringView sel( data + selStart, selEnd - selStart );
                    TClipboard::setText(sel);
                    if( event.message.command == cmCut )
                        {
                        saveState();
                        deleteSelect();
                        checkValid(True);
                        selStart = selEnd = 0;
                        drawView();
                        }
                    clearEvent( event );
                    }
                break;
            }
        if( canUpdateCommands() )
            updateCommands();
        }
}

void TInputLine::selectAll( Boolean enable, Boolean scroll )
{
    selStart = 0;
    if( enable )
        curPos = selEnd = strlen(data);
    else
        curPos = selEnd = 0;
    if( scroll )
        firstPos = max( 0, displayedPos(curPos)-size.x+2 );
    drawView();
    if( canUpdateCommands() )
        updateCommands();
}

void TInputLine::setData( void *rec )
{
    if ((validator == 0) || (validator->transfer(data,rec,vtSetData)==0))
        {
        memcpy( data, rec, dataSize()-1 );
        data[dataSize()-1] = EOS;
        }
    selectAll( True );
}

void TInputLine::setState( ushort aState, Boolean enable )
{
    Boolean updateBefore = canUpdateCommands();
    TView::setState( aState, enable );
    Boolean updateAfter = canUpdateCommands();

    if( aState == sfSelected || (aState == sfActive && (state & sfSelected)) )
        selectAll( enable, False );
    if( updateBefore != updateAfter )
        updateCommands();
}

void TInputLine::setValidator( TValidator* aValid )
{
    if (validator!=0)
      destroy(validator);

    validator = aValid;
}

Boolean TInputLine::canUpdateCommands()
{
    return Boolean( (~state & (sfActive | sfSelected)) == 0 );
}

void TInputLine::setCmdState( ushort command, Boolean enable )
{
    TCommandSet s;
    s += command;
    if( enable && canUpdateCommands() )
        enableCommands(s);
    else
        disableCommands(s);
}

void TInputLine::updateCommands()
{
    setCmdState( cmCut, Boolean( selStart < selEnd ) );
    setCmdState( cmCopy, Boolean( selStart < selEnd ) );
    setCmdState( cmPaste, True );
}

#if !defined(NO_STREAMABLE)

void TInputLine::write( opstream& os )
{
    TView::write( os );
    os << maxLen << maxWidth << maxChars << curPos << firstPos
       << selStart << selEnd;
    os.writeString( data);
    os << validator;
}

void *TInputLine::read( ipstream& is )
{
    TView::read( is );
    is >> maxLen >> maxWidth >> maxChars >> curPos >> firstPos
       >> selStart >> selEnd;
    data = new char[maxLen + 1];
    oldData = new char[maxLen + 1];
    is.readString(data, maxLen+1);
    state |= sfCursorVis;
    is >> (void*&)validator;
    return this;
}

TStreamable *TInputLine::build()
{
    return new TInputLine( streamableInit );
}

TInputLine::TInputLine( StreamableInit ) noexcept : TView( streamableInit )
{
}

#endif

Boolean TInputLine::valid(ushort cmd)
{
    if (validator)
        {
        if (cmd == cmValid)
            return Boolean(validator->status == vsOk);
        else if (cmd != cmCancel)
            if (!validator->validate(data))
                {
                select();
                return False;
                }
        }
    return True;
}



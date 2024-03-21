/*------------------------------------------------------------*/
/* filename -       tevent.cpp                                */
/*                                                            */
/* function(s)                                                */
/*                  TEvent member functions                   */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */
#if defined( __DPMI16__ )
#pragma inline
#endif

#define Uses_TKeys
#define Uses_TEvent
#define Uses_TScreen
#define Uses_TEventQueue
#define Uses_THardwareInfo
#define Uses_TText
#include <tvision/tv.h>

#if !defined (__FLAT__)

#if !defined( __BIOS_H )
#include <bios.h>
#endif  // __BIOS_H

#if !defined( __DOS_H )
#include <dos.h>
#endif  // __DOS_H

#endif

#if !defined( __FLAT__ )
TEvent _NEAR TEventQueue::eventQueue[ eventQSize ] = { {0} };
TEvent * _NEAR TEventQueue::eventQHead = TEventQueue::eventQueue;
TEvent * _NEAR TEventQueue::eventQTail = TEventQueue::eventQueue;
Boolean _NEAR TEventQueue::mouseIntFlag = False;
ushort _NEAR TEventQueue::eventCount = 0;
#endif

ushort _NEAR TEventQueue::downTicks = 0;

Boolean _NEAR TEventQueue::mouseEvents = False;
Boolean _NEAR TEventQueue::mouseReverse = False;
Boolean _NEAR TEventQueue::pendingMouseUp = False;
ushort _NEAR TEventQueue::doubleDelay = 8;
ushort _NEAR TEventQueue::repeatDelay = 8;
ushort _NEAR TEventQueue::autoTicks = 0;
ushort _NEAR TEventQueue::autoDelay = 0;

MouseEventType _NEAR TEventQueue::lastMouse;
MouseEventType _NEAR TEventQueue::curMouse;
MouseEventType _NEAR TEventQueue::downMouse;

char * _FAR TEventQueue::pasteText = 0;
size_t _NEAR TEventQueue::pasteTextLength = 0;
size_t _NEAR TEventQueue::pasteTextIndex = 0;

TEvent _NEAR TEventQueue::keyEventQueue[ keyEventQSize ] = { {0} };
size_t _NEAR TEventQueue::keyEventCount = 0;
size_t _NEAR TEventQueue::keyEventIndex = 0;
Boolean _NEAR TEventQueue::keyPasteState = False;

TEventQueue::TEventQueue() noexcept
{
    resume();
}


void TEventQueue::resume() noexcept
{
    if( TMouse::present() == False )
        TMouse::resume();
    if( TMouse::present() == False )
        return;

    TMouse::getEvent( curMouse );
    lastMouse = curMouse;

#if defined( __FLAT__ )
    THardwareInfo::clearPendingEvent();
#else
    TMouse::registerHandler( 0xFFFF, (void (_FAR *)()) mouseInt );
#endif

    mouseEvents = True;
    TMouse::setRange( TScreen::screenWidth-1, TScreen::screenHeight-1 );
}


void TEventQueue::suspend() noexcept
{
    TMouse::suspend();
}

TEventQueue::~TEventQueue()
{
    suspend();
    delete pasteText;
    pasteText = 0;
}


void TEventQueue::getMouseEvent( TEvent & ev) noexcept
{
    if( mouseEvents == True )
        {
        if( pendingMouseUp == True )
            {
            ev.what = evMouseUp;
            ev.mouse = lastMouse;
            lastMouse.buttons = 0;
            pendingMouseUp = False;
            return;
            }
        if( !getMouseState( ev ) )
            return;

        ev.mouse.eventFlags = 0;

        if( ev.mouse.buttons == 0 && lastMouse.buttons != 0 )
            {
            if( ev.mouse.where == lastMouse.where )
                {
                ev.what = evMouseUp;
                uchar buttons = lastMouse.buttons;
                lastMouse = ev.mouse;
                ev.mouse.buttons = buttons;
                }
            else
                {
                ev.what = evMouseMove;
                MouseEventType up = ev.mouse;
                TPoint where = up.where;
                ev.mouse = lastMouse;
                ev.mouse.where = where;
                ev.mouse.eventFlags |= meMouseMoved;
                up.buttons = lastMouse.buttons;
                lastMouse = up;
                pendingMouseUp = True;
                }
            return;
            }

        if( ev.mouse.buttons != 0 && lastMouse.buttons == 0 )
            {
            if( ev.mouse.buttons == downMouse.buttons &&
                ev.mouse.where == downMouse.where &&
                ev.what - downTicks <= doubleDelay
              )
                {
                if( !(downMouse.eventFlags & (meDoubleClick | meTripleClick)) )
                    ev.mouse.eventFlags |= meDoubleClick;
                else if( downMouse.eventFlags & meDoubleClick )
                    {
                    ev.mouse.eventFlags &= ~meDoubleClick;
                    ev.mouse.eventFlags |= meTripleClick;
                    }
                }

            downMouse = ev.mouse;
            autoTicks = downTicks = ev.what;
            autoDelay = repeatDelay;
            ev.what = evMouseDown;
            lastMouse = ev.mouse;
            return;
            }

        ev.mouse.buttons = lastMouse.buttons;

        if( ev.mouse.wheel != 0 )
            {
            ev.what = evMouseWheel;
#ifdef __BORLANDC__
        // A bug in Borland C++ causes mouse position to be trash in
        // MOUSE_WHEELED events.
            ev.mouse.where = lastMouse.where;
#endif
            lastMouse = ev.mouse;
            return;
            }

        if( ev.mouse.where != lastMouse.where )
            {
            ev.what = evMouseMove;
            ev.mouse.eventFlags |= meMouseMoved;
            lastMouse = ev.mouse;
            return;
            }

        if( ev.mouse.buttons != 0 && ev.what - autoTicks > autoDelay )
            {
            autoTicks = ev.what;
            autoDelay = 1;
            ev.what = evMouseAuto;
            lastMouse = ev.mouse;
            return;
            }
        }

    ev.what = evNothing;
}


Boolean TEventQueue::getMouseState( TEvent & ev ) noexcept
{
#if defined( __FLAT__ )
    ev.what = evNothing;

    if( !THardwareInfo::getMouseEvent( curMouse ) )
        return False;

    if( mouseReverse == True && curMouse.buttons != 0 && curMouse.buttons != 3 )
        curMouse.buttons ^= 3;

    ev.what = THardwareInfo::getTickCount();  // Temporarily save tick count when event was read.
    ev.mouse = curMouse;
    return True;
#else
    disable();

    if( eventCount == 0 )
        {
        ev.what = THardwareInfo::getTickCount();
        ev.mouse = curMouse;
        // 'wheel' represents an event, not a state. So, in order not to process
        // a mouse wheel event more than once, this field must be set back to zero.
        curMouse.wheel = 0;
        }
    else
        {
        ev = *eventQHead;
        if( ++eventQHead >= eventQueue + eventQSize )
            eventQHead = eventQueue;
        eventCount--;
        }
    enable();

    if( mouseReverse && ev.mouse.buttons != 0 && ev.mouse.buttons != 3 )
        ev.mouse.buttons ^= 3;

    return True;
#endif
}


#if !defined( __FLAT__ )
#pragma saveregs
void __MOUSEHUGE TEventQueue::mouseInt()
{
#if defined( __DPMI16__ )
I   PUSH DS          // Cannot use huge anymore, because someone might compile
I   PUSH AX          //  this module with -WX and that generates Smart Callback
I   MOV  AX, DGROUP  //  style prolog code.  This is an asynchronous callback!
I   MOV  DS, AX
I   POP  AX
#endif

    unsigned flag = _AX;
    MouseEventType tempMouse;

    tempMouse.buttons = _BL;
    tempMouse.wheel = _BH == 0 ? 0 : char(_BH) > 0 ? mwDown : mwUp; // CuteMouse
    tempMouse.eventFlags = 0;
    tempMouse.where.x = _CX >> 3;
    tempMouse.where.y = _DX >> 3;
    tempMouse.controlKeyState = THardwareInfo::getShiftState();

    if( (flag & 0x1e) != 0 && eventCount < eventQSize )
        {
        eventQTail->what = THardwareInfo::getTickCount();
        eventQTail->mouse = curMouse;
        if( ++eventQTail >= eventQueue + eventQSize )
            eventQTail = eventQueue;
        eventCount++;
        }

    curMouse = tempMouse;
    mouseIntFlag = True;

#if defined( __DPMI16__ )
I   POP DS
#endif
}
#endif

void TEventQueue::getKeyEvent( TEvent &ev ) noexcept
{
    static TEvent pendingKey = {0};
    if( pendingKey.what != evNothing )
        {
        ev = pendingKey;
        pendingKey.what = evNothing;
        return;
        }

    getKeyOrPasteEvent( ev );

    if( ev.what == evKeyDown && (ev.keyDown.controlKeyState & kbPaste) != 0 )
        {
        if( ev.keyDown.textLength == 0 )
            {
            ev.keyDown.text[0] = (char) ev.keyDown.charScan.charCode;
            ev.keyDown.textLength = 1;
            }
        if( ev.keyDown.text[0] == '\r' ) // Convert CR and CRLF into LF.
            {
            ev.keyDown.text[0] = '\n';

            TEvent next;
            getKeyOrPasteEvent( next );

            if( next.what == evKeyDown &&
                (next.keyDown.controlKeyState & kbPaste) != 0 &&
                next.keyDown.textLength == 1 &&
                next.keyDown.text[0] == '\n'
              )
                ; // Drop event.
            else
                pendingKey = next;
            }
        ev.keyDown.keyCode = 0;
        }
}

void TEventQueue::putPaste( TStringView text ) noexcept
{
    delete[] pasteText;
    // Always initialize the paste event, even if it is empty, so that
    // 'waitForEvent' won't block in the next call.
    if( (pasteText = new char[ text.size() ]) != 0 )
        {
        pasteTextLength = text.size();
        pasteTextIndex = 0;
        memcpy( pasteText, text.data(), text.size() );
        }
}

Boolean TEventQueue::getPasteEvent( TEvent &ev ) noexcept
{
    if( pasteText )
        {
        TSpan<char> text( pasteText + pasteTextIndex,
                          pasteTextLength - pasteTextIndex );
        size_t length = TText::next( text );
        if( length > 0 )
            {
            KeyDownEvent keyDown = { {0x0000}, kbPaste, {0}, (uchar) length };
            ev.what = evKeyDown;
            ev.keyDown = keyDown;
            memcpy( ev.keyDown.text, text.data(), length );
            pasteTextIndex += length;
            return True;
            }
        delete[] pasteText;
        pasteText = 0;
        }
    return False;
}

static int isTextEvent( TEvent &ev ) noexcept
{
    return ev.what == evKeyDown &&
           ( ev.keyDown.textLength != 0 ||
             ev.keyDown.keyCode == kbEnter ||
             ev.keyDown.keyCode == kbTab );
}

void TEventQueue::getKeyOrPasteEvent( TEvent &ev ) noexcept
{
    if( getPasteEvent( ev ) )
        return;
    if( keyEventCount == 0 )
        {
        int firstNonText = keyEventQSize;
        for( int i = 0; i < keyEventQSize; ++i )
            {
            if( !readKeyPress( keyEventQueue[i] ) )
                break;
            ++keyEventCount;
            if( !isTextEvent( keyEventQueue[i] ) )
                {
                firstNonText = i;
                break;
                }
            }
        // If we receive at least X consecutive text events, then this is
        // the beginning of a paste event.
        if( keyEventCount == keyEventQSize && firstNonText == keyEventQSize )
            keyPasteState = True;
        if( keyPasteState )
            for( int i = 0; i < min(keyEventCount, firstNonText); ++i )
                keyEventQueue[i].keyDown.controlKeyState |= kbPaste;
        if( keyEventCount < keyEventQSize || firstNonText < keyEventQSize )
            keyPasteState = False;
        keyEventIndex = 0;
        }
    if( keyEventCount != 0 )
        {
        ev = keyEventQueue[keyEventIndex];
        ++keyEventIndex;
        --keyEventCount;
        }
    else
        ev.what = evNothing;
}

Boolean TEventQueue::readKeyPress( TEvent &ev ) noexcept
{
#if defined( __FLAT__ )
    if( !THardwareInfo::getKeyEvent( ev ) )
        ev.what = evNothing;
#else

I   MOV AH,1;
I   INT 16h;
I   JNZ keyWaiting;

    ev.what = evNothing;
    return False;

keyWaiting:

    ev.what = evKeyDown;

I   MOV AH,0;
I   INT 16h;

    ev.keyDown.keyCode = _AX;
    ev.keyDown.controlKeyState = THardwareInfo::getShiftState();
#endif
#if defined( __BORLANDC__ )
    if( ev.what == evKeyDown )
        {
        if( ' ' <= ev.keyDown.charScan.charCode &&
            ev.keyDown.charScan.charCode != 0x7F &&
            ev.keyDown.charScan.charCode != 0xFF
          )
            {
            ev.keyDown.text[0] = (char) ev.keyDown.charScan.charCode;
            ev.keyDown.textLength = 1;
            }
        else
            ev.keyDown.textLength = 0;
        }
#endif
    return Boolean( ev.what != evNothing );
}

void TEventQueue::waitForEvent(int timeoutMs) noexcept
{
#if defined( __FLAT__ )
    if( !pasteText && keyEventCount == 0 )
        THardwareInfo::waitForEvent(timeoutMs);
#else
    (void) timeoutMs;
#endif
}

void TEvent::getKeyEvent() noexcept
{
    TEventQueue::getKeyEvent( *this );
}

void TEvent::waitForEvent(int timeoutMs) noexcept
{
    TEventQueue::waitForEvent( timeoutMs );
}

void TEvent::putNothing() noexcept
{
#if defined( __FLAT__ )
    THardwareInfo::interruptEventWait();
#endif
}

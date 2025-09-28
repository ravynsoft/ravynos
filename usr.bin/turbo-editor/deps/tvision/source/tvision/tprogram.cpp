/*------------------------------------------------------------*/
/* filename -       tprogram.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TProgram member functions                 */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TKeys
#define Uses_TProgram
#define Uses_TEvent
#define Uses_TScreen
#define Uses_TStatusLine
#define Uses_TMenu
#define Uses_TGroup
#define Uses_TDeskTop
#define Uses_TEventQueue
#define Uses_TMenuBar
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TDialog
#define Uses_TTimerQueue
#include <tvision/tv.h>

// Public variables

TStatusLine * _NEAR TProgram::statusLine = 0;
TMenuBar * _NEAR TProgram::menuBar = 0;
TDeskTop * _NEAR TProgram::deskTop = 0;
TProgram * _NEAR TProgram::application = 0;
int _NEAR TProgram::appPalette = apColor;
int _NEAR TProgram::eventTimeout = 20; // 50 wake-ups per second.
TEvent _NEAR TProgram::pending;
TTimerQueue _NEAR TProgram::timerQueue;

extern TPoint shadowSize;

TProgInit::TProgInit( TStatusLine *(*cStatusLine)( TRect ),
                            TMenuBar *(*cMenuBar)( TRect ),
                            TDeskTop *(*cDeskTop )( TRect )
                          ) noexcept :
    createStatusLine( cStatusLine ),
    createMenuBar( cMenuBar ),
    createDeskTop( cDeskTop )
{
}

TProgram::TProgram() noexcept :
    TProgInit( &TProgram::initStatusLine,
                  &TProgram::initMenuBar,
                  &TProgram::initDeskTop
                ),
    TGroup( TRect( 0,0,TScreen::screenWidth,TScreen::screenHeight ) )
{
    application = this;
    initScreen();
    state = sfVisible | sfSelected | sfFocused | sfModal | sfExposed;
    options = 0;
    buffer = TScreen::screenBuffer;

    if( createDeskTop != 0 &&
        (deskTop = createDeskTop( getExtent() )) != 0
      )
        insert(deskTop);

    if( createStatusLine != 0 &&
        (statusLine = createStatusLine( getExtent() )) != 0
      )
        insert(statusLine);

    if( createMenuBar != 0 &&
        (menuBar = createMenuBar( getExtent() )) != 0
      )
        insert(menuBar);
}

TProgram::~TProgram()
{
    application = 0;
}

void TProgram::shutDown()
{
    statusLine = 0;
    menuBar = 0;
    deskTop = 0;
    TGroup::shutDown();
}

Boolean TProgram::canMoveFocus()
{
    return deskTop->valid(cmReleasedFocus);
}

int TProgram::eventWaitTimeout()
{
    int timerTimeout = min(timerQueue.timeUntilTimeout(), (int32_t) INT_MAX);
    if (timerTimeout < 0)
        return eventTimeout;
    return min(eventTimeout, timerTimeout);
}

ushort TProgram::executeDialog( TDialog* pD, void* data )
{
    ushort c=cmCancel;

    if (validView(pD))
        {
        if (data)
        pD->setData(data);
        c = deskTop->execView(pD);
        if ((c != cmCancel) && (data))
            pD->getData(data);
        destroy(pD);
        }

    return c;
}

static Boolean viewHasMouse( TView *p, void *s )
{
    return Boolean( (p->state & sfVisible) != 0 &&
                     p->mouseInView( ((TEvent *)s)->mouse.where ));
}

void TProgram::getEvent(TEvent& event)
{
    if( pending.what != evNothing )
        {
        event = pending;
        pending.what = evNothing;
        }
    else
        {
        event.waitForEvent(eventWaitTimeout());
        event.getMouseEvent();
        if( event.what == evNothing )
            {
            event.getKeyEvent();
            if( event.what == evNothing )
                idle();
            }
        }

    if( statusLine != 0 )
        {
        if( (event.what & evKeyDown) != 0 ||
            ( (event.what & evMouseDown) != 0 &&
              firstThat( viewHasMouse, &event ) == statusLine
            )
          )
            statusLine->handleEvent( event );
        }
    if( event.what == evCommand && event.message.command == cmScreenChanged )
        {
        setScreenMode( TDisplay::smUpdate );
        clearEvent(event);
        }
}

TPalette& TProgram::getPalette() const
{
    static TPalette color ( cpAppColor, sizeof( cpAppColor )-1 );
    static TPalette blackwhite(cpAppBlackWhite, sizeof( cpAppBlackWhite )-1 );
    static TPalette monochrome(cpAppMonochrome, sizeof( cpAppMonochrome )-1 );
    static TPalette *palettes[] =
        {
        &color,
        &blackwhite,
        &monochrome
        };
    return *(palettes[appPalette]);
}

void TProgram::handleEvent( TEvent& event )
{
    if( event.what == evKeyDown )
        {
        char c = getAltChar( event.keyDown.keyCode );
        if( c >= '1' && c <= '9' )
            {
            if( canMoveFocus() )
                {
                if( message( deskTop,
                             evBroadcast,
                             cmSelectWindowNum,
                             (void *)(size_t)(c - '0')
                           ) != 0 )
                    clearEvent( event );
                }
            else
                clearEvent( event );
            }
        }

    TGroup::handleEvent( event );
    if( event.what == evCommand && event.message.command == cmQuit )
        {
        endModal( cmQuit );
        clearEvent( event );
        }
}

static void doHandleTimeout( TTimerId id, void *self )
{
    message( (TProgram *) self, evBroadcast, cmTimeout, id );
}

void TProgram::idle()
{
    if( statusLine != 0 )
        statusLine->update();

    if( commandSetChanged == True )
        {
        message( this, evBroadcast, cmCommandSetChanged, 0 );
        commandSetChanged = False;
        }

    timerQueue.collectTimeouts(doHandleTimeout, this);
}

TDeskTop *TProgram::initDeskTop( TRect r )
{
    r.a.y++;
    r.b.y--;
    return new TDeskTop( r );
}

TMenuBar *TProgram::initMenuBar( TRect r )
{
    r.b.y = r.a.y + 1;
    return new TMenuBar( r, (TMenu *)0 );
}

void TProgram::initScreen()
{
    if( (TScreen::screenMode & 0x00FF) != TDisplay::smMono )
        {
        if( (TScreen::screenMode & TDisplay::smFont8x8) != 0 )
            shadowSize.x = 1;
        else
            shadowSize.x = 2;
        shadowSize.y = 1;
        showMarkers = False;
        if( (TScreen::screenMode & 0x00FF) == TDisplay::smBW80 )
            appPalette = apBlackWhite;
        else
            appPalette = apColor;
        }
    else
        {

        shadowSize.x = 0;
        shadowSize.y = 0;
        showMarkers = True;
        appPalette = apMonochrome;
        }
}

TStatusLine *TProgram::initStatusLine( TRect r )
{
    r.a.y = r.b.y - 1;
    return new TStatusLine( r,
        *new TStatusDef( 0, 0xFFFF ) +
            *new TStatusItem( exitText, kbAltX, cmQuit ) +
            *new TStatusItem( 0, kbF10, cmMenu ) +
            *new TStatusItem( 0, kbAltF3, cmClose ) +
            *new TStatusItem( 0, kbF5, cmZoom ) +
            *new TStatusItem( 0, kbCtrlF5, cmResize )
            );
}

TWindow* TProgram::insertWindow(TWindow* pWin)
{
    if (validView(pWin))
        {
        if (canMoveFocus())
            {
            deskTop->insert(pWin);
            return pWin;
            }
        else
            destroy(pWin);
        }

   return NULL;
}

void TProgram::killTimer( TTimerId id )
{
    timerQueue.killTimer(id);
}

void TProgram::outOfMemory()
{
}

void TProgram::putEvent( TEvent & event )
{
    pending = event;
}

void TProgram::run()
{
    execute();
}

void TProgram::setScreenMode( ushort mode )
{
    TRect  r;

    TMouse::hide();
    TScreen::setVideoMode( mode );
    initScreen();
    buffer = TScreen::screenBuffer;
    r = TRect( 0, 0, TScreen::screenWidth, TScreen::screenHeight );
    changeBounds( r );
    setState(sfExposed, False);
    setState(sfExposed, True);
    redraw();
    TMouse::show();
}

TTimerId TProgram::setTimer( uint timeoutMs, int periodMs )
{
    return timerQueue.setTimer( timeoutMs, periodMs );
}

TView* TProgram::validView(TView* p) noexcept
{
    if( p == 0 )
        return 0;
    if( lowMemory() )
        {
        destroy( p );
        outOfMemory();
        return 0;
        }
    if( !p->valid( cmValid ) )
        {
        destroy( p );
        return 0;
        }
    return p;
}

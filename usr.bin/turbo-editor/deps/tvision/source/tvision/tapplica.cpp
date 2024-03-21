/*-------------------------------------------------------------------*/
/* filename -       tapplica.cpp                                     */
/*                                                                   */
/* function(s)                                                       */
/*          TApplication member functions (constructor & destructor) */
/*-------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TSystemError
#define Uses_TEventQueue
#define Uses_THardwareInfo
#define Uses_TScreen
#define Uses_TObject
#define Uses_TApplication
#define Uses_TDeskTop
#include <tvision/tv.h>

#include <iostream.h>
#include <stdlib.h>
#include <signal.h>

void initHistory();
void doneHistory();

TSubsystemsInit::TSubsystemsInit() noexcept
{
    static THardwareInfo hwInfoManager;
    static TMouse tms;
    static TScreen tsc;
    static TEventQueue teq;
    static TSystemError sysErr;
}

TApplication::TApplication() noexcept :
    TProgInit( &TApplication::initStatusLine,
                  &TApplication::initMenuBar,
                  &TApplication::initDeskTop
                )
{
    initHistory();
}

TApplication::~TApplication()
{
    doneHistory();
}

void TApplication::suspend()
{
    TSystemError::suspend();
    TEventQueue::suspend();
    TScreen::suspend();
}

void TApplication::resume()
{
    TScreen::resume();
    TEventQueue::resume();
    TSystemError::resume();
}

void TApplication::cascade()
{
    if( deskTop )
        deskTop->cascade( getTileRect() );
}

void TApplication::dosShell()
{
    suspend();
    writeShellMsg();
#if defined( __BORLANDC__ ) || defined( _WIN32 )
    system( getenv( "COMSPEC" ) );
#else
    raise( SIGTSTP );
#endif
    resume();
    redraw();
}

TRect TApplication::getTileRect()
{
    return deskTop->getExtent();
}

void TApplication::handleEvent(TEvent &event)
{
    TProgram::handleEvent(event);

    if( event.what == evCommand )
        {
        switch (event.message.command)
            {
            case cmDosShell:
                dosShell();
                break;
            case cmCascade:
                cascade();
                break;
            case cmTile:
                tile();
                break;
            default:
                return;
            }
        clearEvent(event);
        }
}

void TApplication::tile()
{
    if( deskTop )
        deskTop->tile( getTileRect() );
}

void TApplication::writeShellMsg()
{
#if defined( __BORLANDC__ ) || defined( _WIN32 )
    cout << "Type EXIT to return..." << endl;
#else
    cout << "The application has been stopped. You can return by entering 'fg'." << endl;
#endif
}

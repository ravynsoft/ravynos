/*----------------------------------------------------------*/
/*                                                          */
/*   Turbo Vision TVDEMO source file                        */
/*                                                          */
/*----------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TRect
#define Uses_TMenuBar
#define Uses_TSubMenu
#define Uses_TMenuItem
#define Uses_TKeys
#define Uses_fpstream
#define Uses_TView
#define Uses_TPalette
#define Uses_MsgBox
#define Uses_TFileDialog
#define Uses_TApplication
#define Uses_TDeskTop
#define Uses_TStaticText
#define Uses_TDialog
#define Uses_TEventQueue

#include <tvision/tv.h>

#include "tvdemo.h"
#include "tvcmds.h"
#include "gadgets.h"
#include "mousedlg.h"
#include "demohelp.h"
#include "fileview.h"

#include <tvision/help.h>

//
// Mouse Control Dialog Box function
//

void TVDemo::mouse()
{
    TMouseDialog *mouseCage = (TMouseDialog *) validView( new TMouseDialog() );

    if (mouseCage != 0)
        {
        int32_t mouseReverse = TEventQueue::mouseReverse;
        mouseCage->helpCtx = hcOMMouseDBox;
        mouseCage->setData(&mouseReverse);
        if (deskTop->execView(mouseCage) != cmCancel)
            mouseCage->getData(&mouseReverse);
        TEventQueue::mouseReverse = mouseReverse;
        }
    destroy( mouseCage );
   
}


//
// File Viewer function
//

void TVDemo::openFile( const char *fileSpec )
{
    TFileDialog *d= (TFileDialog *)validView(
    new TFileDialog(fileSpec, "Open a File", "~N~ame", fdOpenButton, 100 ));

    if( d != 0 && deskTop->execView( d ) != cmCancel )
        {
        char fileName[MAXPATH];
        d->getFileName( fileName );
        d->helpCtx = hcFOFileOpenDBox;
        TView *w= validView( new TFileWindow( fileName ) );
        if( w != 0 )
            deskTop->insert(w);
    }
    destroy( d );
}


//
// "Out of Memory" function ( called by validView() )
//

void TVDemo::outOfMemory()
{
    messageBox( "Not enough memory available to complete operation.",
      mfError | mfOKButton );
}

//
// getPalette() function ( returns application's palette )
//
/*
TPalette& TVDemo::getPalette() const
{
    static TPalette newcolor ( cpAppColor cHelpColor, sizeof( cpAppColor cHelpColor )-1 );
    static TPalette newblackwhite( cpAppBlackWhite cHelpBlackWhite, sizeof( cpAppBlackWhite cHelpBlackWhite)-1 );
    static TPalette newmonochrome( cpAppMonochrome cHelpMonochrome, sizeof( cpAppMonochrome cHelpMonochrome)-1 );
    static TPalette *palettes[] =
        {
        &newcolor,
        &newblackwhite,
        &newmonochrome
        };
    return *(palettes[appPalette]);

}
*/

//
// isTileable() function ( checks a view on desktop is tileable or not )
//

static Boolean isTileable(TView *p, void * )
{
   if( (p->options & ofTileable) != 0)
       return True;
   else
       return False;
}

//
// idle() function ( updates heap and clock views for this program. )
//

void TVDemo::idle()
{
    TProgram::idle();
    clock->update();
    heap->update();
    if (deskTop->firstThat(isTileable, 0) != 0 )
        {
        enableCommand(cmTile);
        enableCommand(cmCascade);
        }
    else 
        {
        disableCommand(cmTile);
        disableCommand(cmCascade);
        }
}

//
// closeView() function
//

static void closeView(TView *p, void *p1)
{
    message(p, evCommand, cmClose, p1);
}

//
// loadDesktop() function 
//

void TVDemo::loadDesktop(fpstream &s)
{
    TView  *p;

    if (deskTop->valid(cmClose))
        { 
        deskTop->forEach(::closeView, 0);  // Clear the desktop
        do {
           s >> p;
           deskTop->insertBefore(validView(p), deskTop->last);
           }
           while (p != 0);
        }
}

//
// Menubar initialization.
//

TMenuBar *TVDemo::initMenuBar(TRect r)
{
    TSubMenu& sub1 =
      *new TSubMenu( "~\360~", 0, hcSystem ) +
        *new TMenuItem( "~V~ideo mode", cmVideoMode, kbNoKey, hcNoContext, "" ) +
         newLine() +
        *new TMenuItem( "~A~bout...", cmAboutCmd, kbNoKey, hcSAbout ) +
         newLine() +
        *new TMenuItem( "~P~uzzle", cmPuzzleCmd, kbNoKey, hcSPuzzle ) +
        *new TMenuItem( "Ca~l~endar", cmCalendarCmd, kbNoKey, hcSCalendar ) +
        *new TMenuItem( "Ascii ~T~able", cmAsciiCmd, kbNoKey, hcSAsciiTable ) +
        *new TMenuItem( "~C~alculator", cmCalcCmd, kbNoKey, hcCalculator ) +
        *new TMenuItem( "~E~vent Viewer", cmEventViewCmd, kbAlt0, hcNoContext, "Alt-0" );

    TSubMenu& sub2 =
      *new TSubMenu( "~F~ile", 0, hcFile ) +
        *new TMenuItem( "~O~pen...", cmOpenCmd, kbF3, hcFOpen, "F3" ) +
        *new TMenuItem( "~C~hange Dir...", cmChDirCmd, kbNoKey, hcFChangeDir ) +
         newLine() +
        *new TMenuItem( "~D~OS Shell", cmDosShell, kbNoKey, hcFDosShell ) +
        *new TMenuItem( "E~x~it", cmQuit, kbAltX, hcFExit, "Alt-X" );

    TSubMenu& sub3 =
      *new TSubMenu( "~W~indows", 0, hcWindows ) +
        *new TMenuItem( "~R~esize/move", cmResize, kbCtrlF5, hcWSizeMove, "Ctrl-F5" ) +
        *new TMenuItem( "~Z~oom", cmZoom, kbF5, hcWZoom, "F5" ) +
        *new TMenuItem( "~N~ext", cmNext, kbF6, hcWNext, "F6" ) +
        *new TMenuItem( "~C~lose", cmClose, kbAltF3, hcWClose, "Alt-F3" ) +
        *new TMenuItem( "~T~ile", cmTile, kbNoKey, hcWTile ) +
        *new TMenuItem( "C~a~scade", cmCascade, kbNoKey, hcWCascade );

    TSubMenu& sub4 =
      *new TSubMenu( "~O~ptions", 0, hcOptions ) +
        *new TMenuItem( "~M~ouse...", cmMouseCmd, kbNoKey, hcOMouse ) +
        *new TMenuItem( "~C~olors...", cmColorCmd, kbNoKey, hcOColors ) +
        *new TMenuItem( "~B~ackground...", cmChBackground, kbNoKey ) +
        (TMenuItem&) (
            *new TSubMenu( "~D~esktop", 0 ) +
            *new TMenuItem( "~S~ave desktop", cmSaveCmd, kbNoKey, hcOSaveDesktop ) +
            *new TMenuItem( "~R~etrieve desktop", cmRestoreCmd, kbNoKey, hcORestoreDesktop )
        );

    r.b.y =  r.a.y + 1;
    return (new TMenuBar( r, sub1 + sub2 + sub3 + sub4 ) );
}




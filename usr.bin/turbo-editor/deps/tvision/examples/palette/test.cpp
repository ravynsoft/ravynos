//
// TEST.CPP - Main Test module for Turbo Vision example program.
//

#define Uses_TKeys
#define Uses_TRect
#define Uses_TEvent
#define Uses_TMenuBar
#define Uses_TMenu
#define Uses_TMenuItem
#define Uses_TDialog
#define Uses_TButton
#define Uses_TStaticText
#define Uses_TDeskTop
#define Uses_TApplication
#define Uses_MsgBox
#include <tv.h>
#include "cmds.h"       // User defined command set for this
                        // application
#include "test.h"       // Application class definition
#include "palette.h"    // TTestWindow class definition

//
// TTestApp - Constructor.
//

#define cpTestAppC  "\x3E\x2D\x72\x5F\x68\x4E"
#define cpTestAppBW "\x07\x07\x0F\x70\x78\x7F"
#define cpTestAppM  "\x07\x0F\x70\x09\x0F\x79"

TTestApp::TTestApp() :
    TProgInit( initStatusLine, initMenuBar, initDeskTop )
{
}

//
// initMenuBar - Initialize the menu bar. It will be called by
// the virtual base TProgInit constructor.
//

TMenuBar *TTestApp::initMenuBar( TRect bounds )
{
    bounds.b.y = bounds.a.y + 1;
    TMenuBar *mainMenu = new TMenuBar (bounds, new TMenu(
        *new TMenuItem("~A~bout...", cmAbout, kbAltA, hcNoContext, 0,
         new TMenuItem("~P~alette", cmPaletteView, kbAltP, hcNoContext, 0,
         new TMenuItem("E~x~it", cmQuit, kbAltX)))
        ));
    return( mainMenu );
}

//
// handleEvent - Need to handle the event for the menu and status
// line choices
//

void TTestApp::handleEvent(TEvent& event)
{
    TApplication::handleEvent(event);
    switch(event.what)
    {
    case evCommand:             // handle COMMAND events.
        switch(event.message.command)
        {
        case cmAbout:           // Bring up the dialog box.
            aboutDlg();
            break;
        case cmPaletteView:     // Bring up palette example.
            paletteView();
            break;
        default:                // these events not handled.
            return;
        }
        break;
    default:                    // these events not handled.
        return;
    }
    clearEvent(event);          // Clear the events we did
                                // handle.
}

//
// getPalette: define a new system palette.  Notice that the
// system palette must define palettes for three different types
// of displays: color, BW, and mono.  Also, we are using Borland
// C++ string concatenation in this function to join the system
// default palette to our extension.
//

TPalette& TTestApp::getPalette() const
{
    static TPalette
        newColor( cpAppColor cpTestAppC,
                  sizeof( cpAppColor cpTestAppC )-1 ),
        newBlackWhite( cpAppBlackWhite cpTestAppBW,
                       sizeof( cpAppBlackWhite cpTestAppBW)-1 ),
        newMonochrome( cpAppMonochrome cpTestAppM,
                       sizeof( cpAppMonochrome cpTestAppM)-1 );
    static TPalette *palettes[] =
        {
        &newColor,
        &newBlackWhite,
        &newMonochrome
        };
    return *(palettes[appPalette]); // 'appPalette' is a member
                                    // variable that
                                    // indicates which palette
                                    // (color, BW,
                                    // Mono) is being used.
}

//
// aboutDlg - Creates a about dialog box and execute the dialog
//             box.
//

void TTestApp::aboutDlg()
{
    TDialog *aboutDlgBox = new TDialog(TRect(0, 0, 47, 13), "About");
    if( validView( aboutDlgBox ) )
    {
        aboutDlgBox->insert(
            new TStaticText(
                TRect(2,1,45,9),
                "\n\003PALETTE EXAMPLE\n \n"
                "\003A Turbo Vision Demo\n \n"
                "\003written by\n \n"
                "\003Borland C++ Tech Support\n"
            ));
        aboutDlgBox->insert(
            new TButton(TRect(18,10,29,12), "OK", cmOK,
                         bfDefault)
            );
        aboutDlgBox->options |= ofCentered;     // Centered on
                                                // the screen
        execView( aboutDlgBox );                // Bring up the
                                                // box as modal
        destroy( aboutDlgBox );                 // Destroy the
                                                // box
    }
}

void TTestApp::paletteView()
{
    TView *view = new TTestWindow;
    if( validView( view ) )
        deskTop->insert( view );
}

int main()
{
    TTestApp testApp;
    testApp.run();
    return 0;
}

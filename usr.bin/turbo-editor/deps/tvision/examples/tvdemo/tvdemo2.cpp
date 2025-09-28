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

#define Uses_TDialog
#define Uses_TRect
#define Uses_TStaticText
#define Uses_TButton
#define Uses_TEvent
#define Uses_TWindow
#define Uses_TColorGroup
#define Uses_TColorItem
#define Uses_TColorDialog
#define Uses_TPalette
#define Uses_TDeskTop
#define Uses_TApplication
#define Uses_TChDirDialog
#define Uses_TScreen

#include <tvision/tv.h>

#include "tvdemo.h"
#include "tvcmds.h"
#include "demohelp.h"
#include "ascii.h"
#include "calendar.h"
#include "calc.h"
#include "evntview.h"
#include "backgrnd.h"

#include <stdlib.h>
#include <signal.h>

//
// DemoApp::handleEvent()
//  Event loop to distribute the work.
//

void TVDemo::handleEvent(TEvent &event)
{
    TApplication::handleEvent(event);

    if (event.what == evCommand)
    {
        switch (event.message.command)
            {
            case cmAboutCmd:            //  About Dialog Box
                aboutDlgBox();
                break;

            case cmCalendarCmd:         //  Calendar Window
                calendar();
                break;

            case cmAsciiCmd:            //  Ascii Table
                asciiTable();
                break;

            case cmCalcCmd:             //  Calculator
                calculator();
                break;

            case cmPuzzleCmd:           //  Puzzle
                puzzle();
                break;

            case cmEventViewCmd:        //  Open Event Viewer
                eventViewer();
                break;

            case cmChBackground:        //  Change Background pattern
                chBackground();
                break;

            case cmOpenCmd:             //  View a file
                openFile("*.*");
                break;

            case cmChDirCmd:            //  Change directory
                changeDir();
                break;

            case cmMouseCmd:            //  Mouse control dialog box
                mouse();
                break;

            case cmColorCmd:            //  Color control dialog box
                colors();
                break;

        case cmSaveCmd:             //  Save current desktop
                saveDesktop();
                break;
 
        case cmRestoreCmd:          //  Restore saved desktop
                retrieveDesktop();
                break;

            default:                    //  Unknown command
                return;

            }
        clearEvent (event);
        }
}



//
// About Box function()
//

void TVDemo::aboutDlgBox()
{
    TDialog *aboutBox = new TDialog(TRect(0, 0, 39, 13), "About");

    aboutBox->insert(
      new TStaticText(TRect(9, 2, 30, 9),
        "\003Turbo Vision Demo\n\n"       // These strings will be
        "\003C++ Version\n\n"             // concatenated by the compiler.
        "\003Copyright (c) 1994\n\n"      // The \003 centers the line.
        "\003Borland International"
        )
      );

    aboutBox->insert(
      new TButton(TRect(14, 10, 26, 12), " OK", cmOK, bfDefault)
      );

    aboutBox->options |= ofCentered;

    executeDialog(aboutBox);

}


//
// Ascii Chart function
//

void TVDemo::asciiTable()
{
    TAsciiChart *chart = (TAsciiChart *) validView(new TAsciiChart);

    if(chart != 0)
    {
        chart->helpCtx = hcAsciiTable;
        deskTop->insert(chart);
    }
}

void TVDemo::chBackground()
{
    TChBackground *b = (TChBackground *) validView(new TChBackground(deskTop->background));
    if (b != 0)
    {
        deskTop->execView(b);
        destroy(b);
    }
}


//
// Calendar function()
//

void TVDemo::calendar()
{
    TCalendarWindow *cal = (TCalendarWindow *) validView(new TCalendarWindow);

    if(cal != 0)
    {
        cal->helpCtx = hcCalendar;
        deskTop->insert( cal );
    }
}


//
// Calculator function
//

void TVDemo::calculator()
{
    TCalculator *calc = (TCalculator *) validView(new TCalculator);

    if(calc != 0)
    {
        calc->helpCtx = hcCalculator;
        deskTop->insert(calc);
    }
}

void TVDemo::eventViewer()
{
    TEventViewer *viewer = (TEventViewer *) message(deskTop, evBroadcast, cmFndEventView, 0);
    if( viewer != 0 )
        viewer->toggle();
    else
        deskTop->insert(new TEventViewer(deskTop->getExtent(), 0x0F00));
}

void TVDemo::printEvent(const TEvent &event)
{
    TEventViewer *viewer = (TEventViewer *) message(deskTop, evBroadcast, cmFndEventView, 0);
    if( viewer != 0 )
        viewer->print(event);
}

//
// Change Directory function
//

void TVDemo::changeDir()
{
    TView *d = validView( new TChDirDialog( 0, cmChangeDir ) );

    if( d != 0 )
        {
        d->helpCtx = hcFCChDirDBox;
        deskTop->execView( d );
        destroy( d );
    }
}


//
// Color Control Dialog Box function
//

void TVDemo::colors()
{
    TColorGroup &group1 =
        *new TColorGroup("Desktop") +
            *new TColorItem("Color",             1)+

        *new TColorGroup("Menus") +
            *new TColorItem("Normal",            2)+
            *new TColorItem("Disabled",          3)+
            *new TColorItem("Shortcut",          4)+
            *new TColorItem("Selected",          5)+
            *new TColorItem("Selected disabled", 6)+
            *new TColorItem("Shortcut selected", 7
        );

    TColorGroup &group2 =
        *new TColorGroup("Dialogs/Calc") +
            *new TColorItem("Frame/background",  33)+
            *new TColorItem("Frame icons",       34)+
            *new TColorItem("Scroll bar page",   35)+
            *new TColorItem("Scroll bar icons",  36)+
            *new TColorItem("Static text",       37)+

            *new TColorItem("Label normal",      38)+
            *new TColorItem("Label selected",    39)+
            *new TColorItem("Label shortcut",    40
        );

    TColorItem &item_coll1 =
        *new TColorItem("Button normal",     41)+
        *new TColorItem("Button default",    42)+
        *new TColorItem("Button selected",   43)+
        *new TColorItem("Button disabled",   44)+
        *new TColorItem("Button shortcut",   45)+
        *new TColorItem("Button shadow",     46)+
        *new TColorItem("Cluster normal",    47)+
        *new TColorItem("Cluster selected",  48)+
        *new TColorItem("Cluster shortcut",  49
        );

    TColorItem &item_coll2 =
        *new TColorItem("Input normal",      50)+
        *new TColorItem("Input selected",    51)+
        *new TColorItem("Input arrow",       52)+

        *new TColorItem("History button",    53)+
        *new TColorItem("History sides",     54)+
        *new TColorItem("History bar page",  55)+
        *new TColorItem("History bar icons", 56)+

        *new TColorItem("List normal",       57)+
        *new TColorItem("List focused",      58)+
        *new TColorItem("List selected",     59)+
        *new TColorItem("List divider",      60)+

        *new TColorItem("Information pane",  61
        );

     group2 = group2 + item_coll1 + item_coll2;

     TColorGroup &group3 =
         *new TColorGroup("Viewer") +
             *new TColorItem("Frame passive",      8)+
             *new TColorItem("Frame active",       9)+
             *new TColorItem("Frame icons",       10)+
             *new TColorItem("Scroll bar page",   11)+
             *new TColorItem("Scroll bar icons",  12)+
             *new TColorItem("Text",              13)+
         *new TColorGroup("Puzzle")+
             *new TColorItem("Frame passive",      8)+
             *new TColorItem("Frame active",       9)+
             *new TColorItem("Frame icons",       10)+
             *new TColorItem("Scroll bar page",   11)+
             *new TColorItem("Scroll bar icons",  12)+
             *new TColorItem("Normal text",       13)+
             *new TColorItem("Highlighted text",  14
         );


     TColorGroup &group4 =
         *new TColorGroup("Calendar") +
             *new TColorItem("Frame passive",     16)+
             *new TColorItem("Frame active",      17)+
             *new TColorItem("Frame icons",       18)+
             *new TColorItem("Scroll bar page",   19)+
             *new TColorItem("Scroll bar icons",  20)+
             *new TColorItem("Normal text",       21)+
             *new TColorItem("Current day",       22)+

         *new TColorGroup("Ascii table") +
             *new TColorItem("Frame passive",     24)+
             *new TColorItem("Frame active",      25)+
             *new TColorItem("Frame icons",       26)+
             *new TColorItem("Scroll bar page",   27)+
             *new TColorItem("Scroll bar icons",  28)+
             *new TColorItem("Text",              29
         );


    TColorGroup &group5 = group1 + group2 + group3 + group4;

    TColorDialog *c = new TColorDialog((TPalette*)0, &group5 );

    if( validView( c ) != 0 )
    {
        c->helpCtx = hcOCColorsDBox;  // set context help constant
        c->setData(&getPalette());
        if( deskTop->execView( c ) != cmCancel )
            {
            getPalette() = *(c->pal);
            setScreenMode(TScreen::screenMode);
            }
        destroy( c );
    }
}

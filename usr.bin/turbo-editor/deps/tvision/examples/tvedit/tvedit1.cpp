/*----------------------------------------------------------*/
/*                                                          */
/*   Copyright (c) 1991 by Borland International            */
/*                                                          */
/*----------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TApplication
#define Uses_TEditWindow
#define Uses_TDeskTop
#define Uses_TRect
#define Uses_TEditor
#define Uses_TFileEditor
#define Uses_TFileDialog
#define Uses_TChDirDialog

#include <tvision/tv.h>

#include "tvedit.h"

#include <stdlib.h>
#include <strstrea.h>
#include <iomanip.h>

TEditWindow *TEditorApp::openEditor( const char *fileName, Boolean visible )
{
    TRect r = deskTop->getExtent();
    TView *p = validView( new TEditWindow( r, fileName, wnNoNumber ) );
    if( !visible )
        p->hide();
    deskTop->insert( p );
    return (TEditWindow *)p;
}

TEditorApp::TEditorApp( int argc, char **argv ) :
    TProgInit( TEditorApp::initStatusLine,
               TEditorApp::initMenuBar,
               TEditorApp::initDeskTop
             ),
    TApplication()
{

    TCommandSet ts;
    ts.enableCmd( cmSave );
    ts.enableCmd( cmSaveAs );
    ts.enableCmd( cmCut );
    ts.enableCmd( cmCopy );
    ts.enableCmd( cmPaste );
    ts.enableCmd( cmClear );
    ts.enableCmd( cmUndo );
    ts.enableCmd( cmFind );
    ts.enableCmd( cmReplace );
    ts.enableCmd( cmSearchAgain );
    disableCommands( ts );

    TEditor::editorDialog = doEditDialog;

    while (--argc > 0)                              // Open files specified
        openEditor(*++argv, True);                  // on command line.
    cascade();
}

void TEditorApp::fileOpen()
{
    char fileName[MAXPATH];
    strcpy( fileName, "*.*" );

    if( execDialog( new TFileDialog( "*.*", "Open file",
            "~N~ame", fdOpenButton, 100 ), fileName) != cmCancel )
        openEditor( fileName, True );
}

void TEditorApp::fileNew()
{
    openEditor( 0, True );
}

void TEditorApp::changeDir()
{
    execDialog( new TChDirDialog( cdNormal, 0 ), 0 );
}

void TEditorApp::handleEvent( TEvent& event )
{
    TApplication::handleEvent( event );
    if( event.what != evCommand )
        return;
    else
        switch( event.message.command )
            {
            case cmOpen:
                fileOpen();
                break;

            case cmNew:
                fileNew();
                break;

            case cmChangeDrct:
                changeDir();
                break;

            default:
                return ;
            }
    clearEvent( event );
}
int main( int argc, char **argv )
{
    TEditorApp editorApp( argc, argv );
    editorApp.run();
    editorApp.shutDown();
    return 0;
}


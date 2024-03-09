/*---------------------------------------------------------------------*/
/*                                                                     */
/*   Turbo Vision Forms Demo main source file.                         */
/*                                                                     */
/*---------------------------------------------------------------------*/
/*                                                                     */
/* This Turbo Vision application uses forms to enter and edit data     */
/* in a collection. Two data files, PHONENUM.F16 and PARTS.F16, are    */
/* provided and can be loaded using this application's File|Open menu. */
/* (PHONENUM.F32 and PARTS.F32 are used for the 32 bit version of      */
/*  TVFORMS)                                                           */
/*                                                                     */
/* The .F16 or .F32 files were created by GENFORMS.MAK, which compiles */
/* and runs GENFORM.CPP. You can create additional data files by       */
/* copying and modifying GENPARTS.H or GENPHONE.H and then             */
/* incorporating your new header into GENFORM.CPP.                     */
/*                                                                     */
/*---------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TKeys
#define Uses_TApplication
#define Uses_TEvent
#define Uses_TRect
#define Uses_TDialog
#define Uses_TStaticText
#define Uses_TButton
#define Uses_TMenuBar
#define Uses_TSubMenu
#define Uses_TMenuItem
#define Uses_TStatusLine
#define Uses_TStatusItem
#define Uses_TStatusDef
#define Uses_TDeskTop
#define Uses_TChDirDialog
#define Uses_TFileDialog
#define Uses_MsgBox
#define Uses_TDisplay
#define Uses_TScreen
#define Uses_TEditor
#define Uses_TMemo
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RResourceCollection )
__link( RButton )
__link( RCheckBoxes )
__link( RInputLine )
__link( RLabel )
__link( RMenuBar )
__link( RRadioButtons )
__link( RFrame )
__link( REditor )
__link( RMemo )

#if !defined( __STRING_H )
#include  <string.h>
#endif // __STRING_H

#if !defined( __STDLIB_H )
#include <stdlib.h>
#endif // __STDLIB_H

#if !defined( __FORMCMDS_H )
#include "formcmds.h"
#endif // __FORMCMDS_H

#if !defined( __LISTDLG_H )
#include "listdlg.h"
#endif // __LISTDLG_H

#if defined( __FLAT__ )
#define FORM_WILDCARD "*.f32"
#else
#define FORM_WILDCARD "*.f16"
#endif

const int MAXSIZE = 150;


class TFormApp : public TApplication
{
public:

    TFormApp();

    void handleEvent( TEvent& Event);
    static TMenuBar *initMenuBar( TRect r);
    static TStatusLine *initStatusLine( TRect r);
    void changeDir();
    void openListDialog();
};

// TFormApp
TFormApp::TFormApp() :
    TProgInit(&TFormApp::initStatusLine,
              &TFormApp::initMenuBar,
              &TFormApp::initDeskTop),
    TApplication()
{
    TEvent event;

    // Display about box
    event.what = evCommand;
    event.message.command = cmAboutBox;
    putEvent(event);
}

void TFormApp::changeDir()
{
    TView *d = validView( new TChDirDialog( 0, hlChangeDir ) );

    if( d != 0 ) {
        deskTop->execView( d );
        destroy(d);
    }
}

void TFormApp::openListDialog()
{
    TFileDialog *d;
    char fileName[MAXPATH];
    TDialog *listEditor;
    char errorMsg[MAXSIZE];
    extern Boolean fileExists( char *);
    char name[MAXFILE];
    char drive[MAXDRIVE];
    char dir[MAXDIR];
    char ext[MAXEXT];

    d = new TFileDialog(FORM_WILDCARD, "Open File",
           "~N~ame", fdOpenButton, hlOpenListDlg);
    if (validView(d) != NULL)
        {
        if (deskTop->execView(d) != cmCancel)
            {
            d->getFileName(fileName);
            if (!fileExists(fileName))
                {
                strcpy(errorMsg, "Cannot find file ");
                strcat(errorMsg, fileName);
                messageBox(errorMsg, mfError | mfOKButton);
                }
            else
                {
                // If listEditor exists, select it; otherwise, open new one
                fnsplit(fileName, drive, dir, name, ext);
                listEditor = (TDialog *)message(deskTop, evBroadcast, cmEditingFile, fileName);
                if (listEditor == NULL)
                    deskTop->insert(validView(new TListDialog(fileName, name)));
                else listEditor->select();
                }
            }
        destroy(d);
        }
}

void TFormApp::handleEvent(TEvent& event)
{
    int newMode;
    char aboutMsg[80];

    TApplication::handleEvent(event);
    if (event.what == evCommand)
        {
        switch (event.message.command)
            {
            case cmListOpen:
                openListDialog();
                break;
            case cmChgDir:
                changeDir();
                break;
            case cmAboutBox:
                strcpy(aboutMsg, "\x3Turbo Vision C++ 2.0\n\n\x3Turbo Vision Forms Demo");
                messageBox(aboutMsg, mfInformation | mfOKButton);
                break;
            case cmVideoMode:
                newMode = TScreen::screenMode ^ TDisplay::smFont8x8;
                setScreenMode((ushort)newMode);
                break;

            default:
                return;
            }
        clearEvent(event);
        }
}

TMenuBar *TFormApp::initMenuBar( TRect r)
{

    r.b.y = r.a.y + 1;
    return new TMenuBar(r,
      *new TSubMenu( "~\xF0~", hcNoContext ) +
        *new TMenuItem( "~V~ideo mode", cmVideoMode, kbNoKey, hcNoContext, "" ) +
             newLine() +
        *new TMenuItem( "~A~bout...", cmAboutBox, kbNoKey, hcNoContext ) +
      *new TSubMenu( "~F~ile", hcNoContext) +
        *new TMenuItem( "~O~pen...", cmListOpen, kbF3, hcNoContext, "F3" ) +
        *new TMenuItem( "~S~ave", cmListSave, kbF2, hcNoContext, "F2" ) +
             newLine() +
        *new TMenuItem( "~C~hange directory...", cmChgDir, kbNoKey, hcNoContext ) +
        *new TMenuItem( "~D~OS shell", cmDosShell, kbNoKey, hcNoContext ) +
        *new TMenuItem( "E~x~it", cmQuit, kbAltX, hcNoContext, "Alt-X" ) +
      *new TSubMenu( "~W~indow", hcNoContext ) +
        *new TMenuItem( "~M~ove", cmResize, kbCtrlF5, hcNoContext, "Cntl-F5") +
        *new TMenuItem( "~N~ext", cmNext, kbF6, hcNoContext, "F6") +
        *new TMenuItem( "~P~rev", cmPrev, kbShiftF6, hcNoContext, "Shift-F6") +
        *new TMenuItem( "~C~lose", cmClose, kbAltF3, hcNoContext, "Alt-F3")
      );
}

TStatusLine *TFormApp::initStatusLine( TRect r )
{
    r.a.y = r.b.y - 1;
    return new TStatusLine( r,
      *new TStatusDef( 0, 0xFFFF ) +
        *new TStatusItem( "~F2~ Save", kbF2, cmListSave ) +
        *new TStatusItem( "~F3~ Open", kbF3, cmListOpen ) +
        *new TStatusItem( "~F10~ Menu", kbF10, cmMenu) +
        *new TStatusItem( 0, kbShiftDel, cmCut ) +
        *new TStatusItem( 0, kbCtrlIns, cmCopy ) +
        *new TStatusItem( 0, kbShiftIns, cmPaste ) +
        *new TStatusItem( "", kbCtrlF5, cmResize )
        );
}

int main()
{
    TFormApp *formApp = new TFormApp;

    formApp->run();

    TObject::destroy(formApp);

    return 0;
}

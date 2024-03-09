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
#define Uses_TMenuBar
#define Uses_TRect
#define Uses_TSubMenu
#define Uses_TKeys
#define Uses_TMenuItem
#define Uses_TStatusLine
#define Uses_TStatusItem
#define Uses_TStatusDef
#define Uses_TPoint
#define Uses_TEditor
#define Uses_MsgBox
#define Uses_TFileDialog
#define Uses_TDeskTop

#include <tvision/tv.h>

#include "tvedit.h"

#include <strstrea.h>
#include <iomanip.h>

TMenuBar *TEditorApp::initMenuBar( TRect r )
{

      TSubMenu& sub1 = *new TSubMenu( "~F~ile", kbAltF ) +
        *new TMenuItem( "~O~pen", cmOpen, kbF3, hcNoContext, "F3" ) +
        *new TMenuItem( "~N~ew", cmNew, kbCtrlN, hcNoContext, "Ctrl-N" ) +
        *new TMenuItem( "~S~ave", cmSave, kbF2, hcNoContext, "F2" ) +
        *new TMenuItem( "S~a~ve as...", cmSaveAs, kbNoKey ) +
             newLine() +
        *new TMenuItem( "~C~hange dir...", cmChangeDrct, kbNoKey ) +
        *new TMenuItem( "~D~OS shell", cmDosShell, kbNoKey ) +
        *new TMenuItem( "E~x~it", cmQuit, kbCtrlQ, hcNoContext, "Ctrl-Q" );

      TSubMenu& sub2 = *new TSubMenu( "~E~dit", kbAltE ) +
        *new TMenuItem( "~U~ndo", cmUndo, kbCtrlU, hcNoContext, "Ctrl-U" ) +
             newLine() +
        *new TMenuItem( "Cu~t~", cmCut, kbShiftDel, hcNoContext, "Shift-Del" ) +
        *new TMenuItem( "~C~opy", cmCopy, kbCtrlIns, hcNoContext, "Ctrl-Ins" ) +
        *new TMenuItem( "~P~aste", cmPaste, kbShiftIns, hcNoContext, "Shift-Ins" ) +
             newLine() +
        *new TMenuItem( "~C~lear", cmClear, kbCtrlDel, hcNoContext, "Ctrl-Del" );

      TSubMenu& sub3 = *new TSubMenu( "~S~earch", kbAltS ) +
        *new TMenuItem( "~F~ind...", cmFind, kbNoKey ) +
        *new TMenuItem( "~R~eplace...", cmReplace, kbNoKey ) +
        *new TMenuItem( "~S~earch again", cmSearchAgain, kbNoKey );

      TSubMenu& sub4 = *new TSubMenu( "~W~indows", kbAltW ) +
        *new TMenuItem( "~S~ize/move",cmResize, kbCtrlF5, hcNoContext, "Ctrl-F5" ) +
        *new TMenuItem( "~Z~oom", cmZoom, kbF5, hcNoContext, "F5" ) +
        *new TMenuItem( "~T~ile", cmTile, kbNoKey ) +
        *new TMenuItem( "C~a~scade", cmCascade, kbNoKey ) +
        *new TMenuItem( "~N~ext", cmNext, kbF6, hcNoContext, "F6" ) +
        *new TMenuItem( "~P~revious", cmPrev, kbShiftF6, hcNoContext, "Shift-F6" ) +
        *new TMenuItem( "~C~lose", cmClose, kbCtrlW, hcNoContext, "Ctrl+W" );

    r.b.y = r.a.y+1;
    return new TMenuBar( r, sub1 + sub2 + sub3 + sub4 );
}

TStatusLine *TEditorApp::initStatusLine( TRect r )
{
    r.a.y = r.b.y-1;
    return new TStatusLine( r,
        *new TStatusDef( 0, 0xFFFF ) +
            *new TStatusItem( 0, kbAltX, cmQuit) +
            *new TStatusItem( "~F2~ Save", kbF2, cmSave ) +
            *new TStatusItem( "~F3~ Open", kbF3, cmOpen ) +
            *new TStatusItem( "~Ctrl-W~ Close", kbAltF3, cmClose ) +
            *new TStatusItem( "~F5~ Zoom", kbF5, cmZoom ) +
            *new TStatusItem( "~F6~ Next", kbF6, cmNext ) +
            *new TStatusItem( "~F10~ Menu", kbF10, cmMenu ) +
            *new TStatusItem( 0, kbShiftDel, cmCut ) +
            *new TStatusItem( 0, kbCtrlIns, cmCopy ) +
            *new TStatusItem( 0, kbShiftIns, cmPaste ) +
            *new TStatusItem( 0, kbCtrlF5, cmResize )
            );

}

void TEditorApp::outOfMemory()
{
    messageBox("Not enough memory for this operation.", mfError | mfOKButton );
}

typedef char *_charPtr;
typedef TPoint *PPoint;

#pragma warn -rvl

ushort doEditDialog( int dialog, ... )
{
    va_list arg;

    char buf[256] = {0};
    ostrstream os( buf, sizeof( buf )-1 );
    switch( dialog )
        {
        case edOutOfMemory:
            return messageBox( "Not enough memory for this operation",
                               mfError | mfOKButton );
        case edReadError:
            {
            va_start( arg, dialog );
            os << "Error reading file " << va_arg( arg, _charPtr )
               << "." << ends;
            va_end( arg );
            return messageBox( buf, mfError | mfOKButton );
            }
        case edWriteError:
            {
            va_start( arg, dialog );
            os << "Error writing file " << va_arg( arg,_charPtr )
               << "." << ends;
            va_end( arg );
            return messageBox( buf, mfError | mfOKButton );
            }
        case edCreateError:
            {
            va_start( arg, dialog );
            os << "Error creating file " << va_arg( arg, _charPtr )
               << "." << ends;
            va_end( arg );
            return messageBox( buf, mfError | mfOKButton );
            }
        case edSaveModify:
            {
            va_start( arg, dialog );
            os << va_arg( arg, _charPtr )
               << " has been modified. Save?" << ends;
            va_end( arg );
            return messageBox( buf, mfInformation | mfYesNoCancel );
            }
        case edSaveUntitled:
            return messageBox( "Save untitled file?",
                               mfInformation | mfYesNoCancel );
        case edSaveAs:
            {
            va_start( arg, dialog );
            return execDialog( new TFileDialog( "*.*",
                                                "Save file as",
                                                "~N~ame",
                                                fdOKButton,
                                                101 ), va_arg( arg, _charPtr ) );
            }

        case edFind:
            {
            va_start( arg, dialog );
            return execDialog( createFindDialog(), va_arg( arg, _charPtr ) );
            }

        case edSearchFailed:
            return messageBox( "Search string not found.",
                               mfError | mfOKButton );
        case edReplace:
            {
            va_start( arg, dialog );
            return execDialog( createReplaceDialog(), va_arg( arg, _charPtr ) );
            }

        case edReplacePrompt:
            //  Avoid placing the dialog on the same line as the cursor
            TRect r( 0, 1, 40, 8 );
            r.move( (TProgram::deskTop->size.x-r.b.x)/2, 0 );
            TPoint t = TProgram::deskTop->makeGlobal( r.b );
            t.y++;
            va_start( arg, dialog );
            TPoint *pt = va_arg( arg, PPoint );
            if( pt->y <= t.y )
                r.move( 0, TProgram::deskTop->size.y - r.b.y - 2 );
            va_end( arg );
            return messageBoxRect( r, "Replace this occurence?",
                                   mfYesNoCancel | mfInformation );

        }
    return cmCancel;
}

#pragma warn .rvl



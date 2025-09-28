/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   TV.H                                                                  */
/*                                                                         */
/*   includes other header files based on which Uses_XXXX symbols are      */
/*   defined.                                                              */
/*                                                                         */
/* ------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined( __BORLANDC__ )
#define __FLAT__

#if !defined( _WIN32 )
#define __cdecl
#define __stdcall
#define _TV_UNIX
#endif

#endif

#if defined( __FLAT__ )
#define _NEAR
#define _FAR
#else
#define _NEAR near
#endif

#if defined( __BORLANDC__ )
#pragma option -Vo-
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po-
#endif
#if defined( _MSC_VER )
#pragma warning(push)
#pragma warning(disable: 4250)
#endif
#if defined( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#endif

#if !defined( __FLAT__ )
#if !defined ( __LARGE__ )
#error TV needs the large memory model
#endif
#endif

#if defined( _RTLDLL )
#error TV must use the RTL in static form only
#endif

#define _TV_VERSION 0x0200

#define Uses_EventCodes
#define Uses_ViewCommands
#define __INC_TKEYS_H
#define __INC_STDDLG_H

#if defined( Uses_TApplication )
#define Uses_TProgram
#define __INC_APP_H
#endif

#if defined( Uses_TProgram )
#define Uses_TEvent
#define Uses_TGroup
#define __INC_APP_H
#endif

#if defined( Uses_TDeskTop )
#define Uses_TGroup
#define __INC_APP_H
#endif

#if defined( Uses_TBackground )
#define Uses_TView
#define __INC_APP_H
#endif

#if defined( Uses_TReplaceDialogRec )
#define __INC_EDITORS_H
#endif

#if defined( Uses_TFindDialogRec )
#define __INC_EDITORS_H
#endif

#if defined( Uses_TEditWindow )
#define Uses_TWindow
#define __INC_EDITORS_H
#endif

#if defined( Uses_TFileEditor )
#define Uses_TEditor
#define __INC_EDITORS_H
#endif

#if defined( Uses_TMemo )
#define Uses_TEditor
#define __INC_EDITORS_H
#endif

#if defined( Uses_TEditor )
#define Uses_TView
#define Uses_TPoint
#define __INC_EDITORS_H
#endif

#if defined( Uses_TIndicator )
#define Uses_TView
#define Uses_TPoint
#define __INC_EDITORS_H
#endif

#if defined( Uses_TTerminal )
#define Uses_TTextDevice
#define __INC_TEXTVIEW_H
#endif

#if defined( Uses_TOutline )
#define Uses_TOutlineViewer
#endif

#if defined( Uses_TOutlineViewer )
#define Uses_TScroller
#define Uses_TScrollBar
#define __INC_OUTLINE_H
#endif

#if defined( Uses_TSurfaceView )
#define Uses_TView
#define Uses_TDrawSurface
#endif

#if defined( Uses_TDrawSurface )
#define Uses_TScreenCell
#define Uses_TPoint
#define __INC_SURFACE_H
#endif

#if defined( Uses_TTextDevice )
#define Uses_TScroller
#define __INC_TEXTVIEW_H
#endif

#if defined( Uses_TStatusLine )
#define Uses_TView
#define __INC_MENUS_H
#endif

#if defined( Uses_TStatusDef )
#define __INC_MENUS_H
#endif

#if defined( Uses_TStatusItem )
#define __INC_MENUS_H
#endif

#if defined( Uses_TMenuPopup )
#define Uses_TMenuBox
#define __INC_MENUS_H
#endif

#if defined( Uses_TMenuBox )
#define Uses_TMenuView
#define __INC_MENUS_H
#endif

#if defined( Uses_TMenuBar )
#define Uses_TMenuView
#define __INC_MENUS_H
#endif

#if defined( Uses_TMenuView )
#define Uses_TView
#define __INC_MENUS_H
#endif

#if defined( Uses_TMenu )
#define __INC_MENUS_H
#endif

#if defined( Uses_TSubMenu )
#define Uses_TMenuItem
#define __INC_MENUS_H
#endif

#if defined( Uses_TMenuItem )
#define __INC_MENUS_H
#endif

#if defined( Uses_TColorDialog )
#define Uses_TColorGroup
#define Uses_TDialog
#define __INC_COLORSEL_H
#endif

#if defined( Uses_TColorItemList )
#define Uses_TListViewer
#define __INC_COLORSEL_H
#endif

#if defined( Uses_TColorGroupList )
#define Uses_TListViewer
#define __INC_COLORSEL_H
#endif

#if defined( Uses_TColorDisplay )
#define Uses_TView
#define __INC_COLORSEL_H
#endif

#if defined( Uses_TMonoSelector )
#define __INC_COLORSEL_H
#endif

#if defined( Uses_TMonoSelector )
#define Uses_TCluster
#define __INC_COLORSEL_H
#endif

#if defined( Uses_TColorSelector )
#define Uses_TView
#define __INC_COLORSEL_H
#endif

#if defined( Uses_TColorGroup )
#define __INC_COLORSEL_H
#endif

#if defined( Uses_TColorItem )
#define __INC_COLORSEL_H
#endif

#if defined( Uses_TChDirDialog )
#define Uses_TDialog
#define __INC_STDDLG_H
#endif

#if defined( Uses_TDirListBox )
#define Uses_TListBox
#define __INC_STDDLG_H
#endif

#if defined( Uses_TDirCollection )
#define Uses_TCollection
#define Uses_TDirEntry
#define __INC_STDDLG_H
#endif

#if defined ( Uses_TRangeValidator )
#define Uses_TFilterValidator
#endif

#if defined ( Uses_TFilterValidator )
#define Uses_TValidator
#endif

#if defined ( Uses_TStringLookupValidator )
#define Uses_TLookupValidator
#define Uses_TStringCollection
#endif

#if defined ( Uses_TLookupValidator )
#define Uses_TValidator
#endif

#if defined ( Uses_TPXPictureValidator )
#define Uses_TValidator
#endif

#if defined ( Uses_TValidator )
#define Uses_TObject
#define Uses_TStreamable
#define __INC_VALIDATOR_H
#endif

#if defined( Uses_TDirEntry )
#define __INC_STDDLG_H
#endif

#if defined( Uses_TFileDialog )
#define Uses_TDialog
#define __INC_STDDLG_H
#endif

#if defined( Uses_TFileInfoPane )
#define Uses_TView
#define Uses_TSearchRec
#define __INC_STDDLG_H
#endif

#if defined( Uses_TFileList )
#define Uses_TSortedListBox
#define Uses_TFileCollection
#define Uses_TSearchRec
#define __INC_STDDLG_H
#endif

#if defined( Uses_TSortedListBox )
#define Uses_TListBox
#define Uses_TSortedCollection
#define __INC_STDDLG_H
#endif

#if defined( Uses_TFileCollection )
#define Uses_TSortedCollection
#define Uses_TSearchRec
#define __INC_STDDLG_H
#endif

#if defined( Uses_TFileInputLine )
#define Uses_TInputLine
#define __INC_STDDLG_H
#endif

#if defined( Uses_TSearchRec )
#define __INC_STDDLG_H
#endif

#if defined( Uses_THistory )
#define Uses_TView
#define __INC_DIALOGS_H
#endif

#if defined( Uses_THistoryWindow )
#define Uses_TWindow
#define __INC_DIALOGS_H
#endif

#if defined( Uses_THistoryViewer )
#define Uses_TListViewer
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TLabel )
#define Uses_TStaticText
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TParamText )
#define Uses_TStaticText
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TStaticText )
#define Uses_TView
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TListBox )
#define Uses_TListViewer
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TCheckBoxes )
#define Uses_TCluster
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TMultiCheckBoxes )
#define Uses_TCluster
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TRadioButtons )
#define Uses_TCluster
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TCluster )
#define Uses_TView
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TSItem )
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TButton )
#define Uses_TView
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TInputLine )
#define Uses_TView
#define Uses_TValidator
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TDialog )
#define Uses_TWindow
#define __INC_DIALOGS_H
#endif

#if defined( Uses_TVMemMgr )
#define __INC_BUFFERS_H
#endif

#if defined( Uses_TWindow )
#define Uses_TGroup
#define __INC_VIEWS_H
#endif

#if defined( Uses_TGroup )
#define Uses_TView
#define Uses_TRect
#define __INC_VIEWS_H
#endif

#if defined( Uses_TListViewer )
#define Uses_TView
#define __INC_VIEWS_H
#endif

#if defined( Uses_TScroller )
#define Uses_TView
#define Uses_TPoint
#define __INC_VIEWS_H
#endif

#if defined( Uses_TScrollBar )
#define Uses_TView
#define __INC_VIEWS_H
#endif

#if defined( Uses_TFrame )
#define Uses_TView
#define __INC_VIEWS_H
#endif

#if defined( Uses_TView )
#define Uses_TObject
#define Uses_TStreamable
#define Uses_TCommandSet
#define Uses_TPoint
#define Uses_TDrawBuffer
#define Uses_TPalette
#define __INC_VIEWS_H
#endif

#if defined( Uses_TPalette )
#define Uses_TColorAttr
#define __INC_VIEWS_H
#endif

#if defined( Uses_TCommandSet )
#define __INC_VIEWS_H
#endif

#if defined( Uses_ViewCommands )
#define __INC_VIEWS_H
#endif

#if defined( Uses_TStrListMaker )
#define Uses_TObject
#define Uses_TStreamable
#define Uses_TStrIndexRec
#define __INC_RESOURCE_H
#endif

#if defined( Uses_TStringList )
#define Uses_TObject
#define Uses_TStreamable
#define __INC_RESOURCE_H
#endif

#if defined( Uses_TStrIndexRec )
#define __INC_RESOURCE_H
#endif

#if defined( Uses_TResourceFile )
#define Uses_TObject
#define __INC_RESOURCE_H
#endif

#if defined( Uses_TResourceItem )
#define __INC_RESOURCE_H
#endif

#if defined( Uses_TResourceCollection )
#define Uses_TStringCollection
#define __INC_RESOURCE_H
#endif

#if defined( Uses_TStringCollection )
#define Uses_TSortedCollection
#define __INC_RESOURCE_H
#endif

#if defined( Uses_MsgBox )
#define __INC_MSGBOX_H
#endif

#if defined( Uses_TSystemError )
#define __INC_SYSTEM_H
#endif

#if defined( Uses_TScreen )
#define __INC_SYSTEM_H
#endif

#if defined( Uses_TClipboard )
#define __INC_SYSTEM_H
#endif

#if defined( Uses_TTimerQueue )
#define __INC_SYSTEM_H
#endif

#if defined( Uses_TEventQueue )
#define Uses_TEvent
#define __INC_SYSTEM_H
#endif

#if defined( Uses_TEvent )
#define Uses_TPoint
#define __INC_SYSTEM_H
#endif

#if defined( Uses_THardwareInfo )
#define Uses_TScreenCell
#define __INC_HARDWARE_H
#endif

#if defined( Uses_EventCodes )
#define __INC_SYSTEM_H
#endif

#if defined( Uses_TSortedCollection )
#define Uses_TNSSortedCollection
#define Uses_TCollection
#define __INC_OBJECTS_H
#endif

#if defined( Uses_TCollection )
#define Uses_TNSCollection
#define Uses_TStreamable
#define __INC_OBJECTS_H
#endif

#if defined( Uses_TRect )
#define Uses_TPoint
#define __INC_OBJECTS_H
#endif

#if defined( Uses_TPoint )
#define __INC_OBJECTS_H
#endif

#if defined( Uses_TDrawBuffer )
#define Uses_TScreenCell
#define __INC_DRAWBUF_H
#endif

#if defined( Uses_fpstream )
#define Uses_fpbase
#define Uses_iopstream
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_ofpstream )
#define Uses_fpbase
#define Uses_opstream
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_ifpstream )
#define Uses_fpbase
#define Uses_ipstream
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_fpbase )
#define Uses_pstream
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_iopstream )
#define Uses_ipstream
#define Uses_opstream
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_opstream )
#define Uses_pstream
#define Uses_TPWrittenObjects
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_ipstream )
#define Uses_pstream
#define Uses_TPReadObjects
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_pstream )
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_TPReadObjects )
#define Uses_TNSCollection
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_TPWrittenObjects )
#define Uses_TNSSortedCollection
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_TStreamableTypes )
#define Uses_TNSSortedCollection
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_TStreamableClass )
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_TStreamable )
#define __INC_TOBJSTRM_H
#endif

#if defined( Uses_TNSSortedCollection )
#define Uses_TNSCollection
#define __INC_TVOBJS_H
#endif

#if defined( Uses_TNSCollection )
#define Uses_TObject
#define __INC_TVOBJS_H
#endif

#if defined( Uses_TObject )
#define __INC_TVOBJS_H
#endif

#if defined( Uses_TKeys )
#define __INC_TKEYS_H
#endif

#if defined( Uses_TText )
#define Uses_TScreenCell
#define __INC_TTEXT_H
#endif

#if defined( Uses_TScreenCell )
#define Uses_TColorAttr
#define __INC_SCRNCELL_H
#endif

#if defined( Uses_TColorAttr )
#define __INC_COLORS_H
#endif

#include <tvision/config.h>
#include <tvision/ttypes.h>
#include <tvision/util.h>

#include <tvision/tspan.h>
#include <tvision/tstrview.h>

#if defined( __INC_COLORS_H )
#include <tvision/colors.h>
#endif

#if defined( __INC_SCRNCELL_H )
#include <tvision/scrncell.h>
#endif

#if defined( __INC_HARDWARE_H )
#include <tvision/hardware.h>
#endif

#if defined( __INC_TKEYS_H )
#include <tvision/tkeys.h>
#endif

#if defined( __INC_TTEXT_H )
#include <tvision/ttext.h>
#endif

#if defined( __INC_TVOBJS_H )
#include <tvision/tvobjs.h>
#endif

#if defined( __INC_TOBJSTRM_H )
#include <tvision/tobjstrm.h>
#endif

#if defined( __INC_DRAWBUF_H )
#include <tvision/drawbuf.h>
#endif

#if defined( __INC_OBJECTS_H )
#include <tvision/objects.h>
#endif

#if defined( __INC_SYSTEM_H )
#include <tvision/system.h>
#endif

#if defined( __INC_MSGBOX_H )
#include <tvision/msgbox.h>
#endif

#if defined( __INC_RESOURCE_H )
#include <tvision/resource.h>
#endif

#if defined( __INC_VIEWS_H )
#include <tvision/views.h>
#endif

#if defined( __INC_BUFFERS_H )
#include <tvision/buffers.h>
#endif

#if defined( __INC_DIALOGS_H )
#include <tvision/dialogs.h>
#endif

#if defined( __INC_VALIDATOR_H )
#include <tvision/validate.h>
#endif

#if defined( __INC_STDDLG_H )
#include <tvision/stddlg.h>
#endif

#if defined( __INC_COLORSEL_H )
#include <tvision/colorsel.h>
#endif

#if defined( __INC_MENUS_H )
#include <tvision/menus.h>
#endif

#if defined( __INC_TEXTVIEW_H )
#include <tvision/textview.h>
#endif

#if defined( __INC_EDITORS_H )
#include <tvision/editors.h>
#endif

#if defined( __INC_OUTLINE_H )
#include <tvision/outline.h>
#endif

#if defined( __INC_SURFACE_H )
#include <tvision/surface.h>
#endif

#if defined( __INC_APP_H )
#include <tvision/app.h>
#endif

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif
#if defined( _MSC_VER )
#pragma warning(pop)
#endif
#if defined( __GNUC__ )
#pragma GCC diagnostic pop
#endif

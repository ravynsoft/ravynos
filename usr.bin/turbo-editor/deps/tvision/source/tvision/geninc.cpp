/*------------------------------------------------------------------------*/
/* filename - geninc.cpp                                                  */
/*                                                                        */
/*      generates assembler EQUates for offsets of                        */
/*      class data members                                                */
/*                                                                        */
/*  Used only before build!  Compile to produce GENINC.EXE,               */
/*  then execute GENINC >TVWRITE.INC to produce the .INC                  */
/*  file needed by the assembler files                                    */
/*                                                                        */
/*------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TPoint
#define Uses_TView
#define Uses_TGroup
#define Uses_TEvent
#define Uses_TFrame
#define Uses_TDrawBuffer
#define Uses_TEditor
#define Uses_TTerminal
#include <tvision/tv.h>

#if !defined( __IOSTREAM_H )
#include <iostream.h>
#endif  // __IOSTREAM_H

#if !defined( __IOMANIP_H )
#include <iomanip.h>
#endif  // __IOMANIP_H

#if !defined( __STDDEF_H )
#include <stddef.h>
#endif  // __STDDEF_H

#define genConst( n )  generate( #n, n )

#define gen( n, c, o ) generate( #n, offsetof( c, o ) )

void generate( const char *name, size_t offset )
{
  cout << setw( 19 ) << setiosflags( ios::left )
       << name << " equ " << offset << endl;
}

void genRefs()
{
    gen( TPointX,           TPoint, x );
    gen( TPointY,           TPoint, y );

    gen( TViewSizeX,        TView,  size.x );
    gen( TViewSizeY,        TView,  size.y );
    gen( TViewState,        TView,  state );
    gen( TViewOwner,        TView,  owner );
    gen( TViewOriginY,      TView,  origin.y );
    gen( TViewOriginX,      TView,  origin.x );
    gen( TViewCursorY,      TView,  cursor.y );
    gen( TViewCursorX,      TView,  cursor.x );
    gen( TViewNext,         TView,  next );
    gen( TViewOptions,      TView,  options );

    gen( TGroupClipAY,      TGroup, clip.a.y );
    gen( TGroupClipAX,      TGroup, clip.a.x );
    gen( TGroupClipBY,      TGroup, clip.b.y );
    gen( TGroupClipBX,      TGroup, clip.b.x );
    gen( TGroupLast,        TGroup, last );
    gen( TGroupBuffer,      TGroup, buffer );
    gen( TGroupLockFlag,    TGroup, lockFlag );

    gen( MsEventWhereX,     MouseEventType, where.x );
    gen( MsEventWhereY,     MouseEventType, where.y );

    gen( TFrameSizeX,       TFrame, size.x );
    gen( TFrameOwner,       TFrame, owner );

    gen( TDrawBufferData,   TDrawBuffer, data );
    gen( TEditorCurPtr,     TEditor, curPtr );
    gen( TEditorGapLen,     TEditor, gapLen );
    gen( TEditorBuffer,     TEditor, buffer );
    gen( TEditorSelStart,   TEditor, selStart );
    gen( TEditorSelEnd,     TEditor, selEnd );
    gen( TEditorBufSize,    TEditor, bufSize );
    gen( TEditorBufLen,     TEditor, bufLen );
    gen( TTerminalBuffer,   TTerminal, buffer );
    gen( TTerminalBufSize,  TTerminal, bufSize );
    gen( TTerminalQueBack,  TTerminal, queBack );

    genConst( sfVisible );
    genConst( sfCursorVis );
    genConst( sfCursorIns );
    genConst( sfFocused );
    genConst( sfShadow );
    genConst( sfExposed );
    genConst( ofFramed );
}

int main()
{
    genRefs();
    return 0;
}

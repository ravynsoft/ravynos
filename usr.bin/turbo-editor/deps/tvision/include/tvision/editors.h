/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   EDITORS.H                                                             */
/*                                                                         */
/*   defines the classes TIndicator, TEditor, TMemo, TFileEditor,          */
/*   and TEditWindow                                                       */
/*                                                                         */
/* ------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined( __DIR_H )
#include <tvision/compat/borland/dir.h>
#endif  // __DIR_H

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __LIMITS_H )
#include <limits.h>
#endif  // __LIMITS_H

#if defined( __BORLANDC__ )
#pragma option -Vo-
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po-
#endif

#if !defined( __EDIT_COMMAND_CODES )
#define __EDIT_COMMAND_CODES

const int
  ufUpdate = 0x01,
  ufLine   = 0x02,
  ufView   = 0x04;

const int
  smExtend = 0x01,
  smDouble = 0x02,
  smTriple = 0x04;

const unsigned
  sfSearchFailed = -0x01;

const int
  cmFind        = 82,
  cmReplace     = 83,
  cmSearchAgain = 84;

const int
  cmCharLeft    = 500,
  cmCharRight   = 501,
  cmWordLeft    = 502,
  cmWordRight   = 503,
  cmLineStart   = 504,
  cmLineEnd     = 505,
  cmLineUp      = 506,
  cmLineDown    = 507,
  cmPageUp      = 508,
  cmPageDown    = 509,
  cmTextStart   = 510,
  cmTextEnd     = 511,
  cmNewLine     = 512,
  cmBackSpace   = 513,
  cmDelChar     = 514,
  cmDelWord     = 515,
  cmDelStart    = 516,
  cmDelEnd      = 517,
  cmDelLine     = 518,
  cmInsMode     = 519,
  cmStartSelect = 520,
  cmHideSelect  = 521,
  cmIndentMode  = 522,
  cmUpdateTitle = 523,
  cmSelectAll   = 524,
  cmDelWordLeft = 525,
  cmEncoding    = 526;

const int
  edOutOfMemory   = 0,
  edReadError     = 1,
  edWriteError    = 2,
  edCreateError   = 3,
  edSaveModify    = 4,
  edSaveUntitled  = 5,
  edSaveAs        = 6,
  edFind          = 7,
  edSearchFailed  = 8,
  edReplace       = 9,
  edReplacePrompt = 10;

const int
  efCaseSensitive   = 0x0001,
  efWholeWordsOnly  = 0x0002,
  efPromptOnReplace = 0x0004,
  efReplaceAll      = 0x0008,
  efDoReplace       = 0x0010,
  efBackupFiles     = 0x0100;

const int
  maxLineLength = 256;

#endif  // __EDIT_COMMAND_CODES

typedef ushort (*TEditorDialog)( int, ... );
ushort defEditorDialog( int dialog, ... );

#if defined( Uses_TIndicator ) && !defined( __TIndicator )
#define __TIndicator

class _FAR TRect;

class TIndicator : public TView
{

public:

    TIndicator( const TRect& ) noexcept;

    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void setState( ushort, Boolean );
    void setValue( const TPoint&, Boolean );

protected:

    TPoint location;
    Boolean modified;

private:

    static const char _NEAR dragFrame;
    static const char _NEAR normalFrame;

    virtual const char *streamableName() const
        { return name; }

protected:

    TIndicator( StreamableInit ) noexcept;

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TIndicator& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TIndicator*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TIndicator& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TIndicator* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TIndicator


#if defined( Uses_TEditor ) && !defined( __TEditor )
#define __TEditor

class _FAR TRect;
class _FAR TScrollBar;
class _FAR TIndicator;
struct _FAR TEvent;
class _FAR TMenuItem;

class TEditor : public TView
{

public:

    friend void genRefs();

    TEditor( const TRect&, TScrollBar *, TScrollBar *, TIndicator *, uint ) noexcept;
    virtual ~TEditor();

    virtual void shutDown();

    char bufChar( uint );
    uint bufPtr( uint );
    virtual void changeBounds( const TRect& );
    virtual void convertEvent( TEvent& );
    Boolean cursorVisible();
    void deleteSelect();
    virtual void doneBuffer();
    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& );
    virtual void initBuffer();
    virtual TMenuItem& initContextMenu( TPoint );
    uint insertMultilineText( const char *, uint );
    Boolean insertBuffer( const char *, uint, uint, Boolean, Boolean );
    Boolean insertEOL( Boolean );
    virtual Boolean insertFrom( TEditor * );
    Boolean insertText( const void *, uint, Boolean );
    void scrollTo( int, int );
    Boolean search( const char *, ushort );
    virtual Boolean setBufSize( uint );
    void setCmdState( ushort, Boolean );
    void setSelect( uint, uint, Boolean);
    virtual void setState( ushort, Boolean );
    void trackCursor( Boolean );
    void undo();
    virtual void updateCommands();
    virtual Boolean valid( ushort );

    int charPos( uint, uint );
    uint charPtr( uint, int );
    Boolean clipCopy();
    void clipCut();
    void clipPaste();
    void deleteRange( uint, uint, Boolean );
    void doUpdate();
    void doSearchReplace();
    void drawLines( int, int, uint );
    void formatLine(TScreenCell *, uint, int, TAttrPair );
    void find();
    uint getMousePtr( TPoint );
    Boolean hasSelection();
    void hideSelect();
    Boolean isClipboard();
    uint lineEnd( uint );
    uint lineMove( uint, int );
    uint lineStart( uint );
    uint indentedLineStart( uint );
    void lock();
    void newLine();
    uint nextChar( uint );
    uint nextLine( uint );
    uint nextWord( uint );
    uint prevChar( uint );
    uint prevLine( uint );
    uint prevWord( uint );
    void replace();
    void setBufLen( uint );
    void setCurPtr( uint, uchar );
    void startSelect();
    void toggleEncoding();
    void toggleInsMode();
    void unlock();
    void update( uchar );
    void checkScrollBar( const TEvent&, TScrollBar *, int& );
    void detectEol();

    TScrollBar *hScrollBar;
    TScrollBar *vScrollBar;
    TIndicator *indicator;
    char *buffer;
    uint bufSize;
    uint bufLen;
    uint gapLen;
    uint selStart;
    uint selEnd;
    uint curPtr;
    TPoint curPos;
    TPoint delta;
    TPoint limit;
    int drawLine;
    uint drawPtr;
    uint delCount;
    uint insCount;
    Boolean isValid;
    Boolean canUndo;
    Boolean modified;
    Boolean selecting;
    Boolean overwrite;
    Boolean autoIndent;

    enum EolType { eolCrLf, eolLf, eolCr } eolType;
    enum Encoding { encDefault, encSingleByte } encoding;

    void nextChar( TStringView, uint &P, uint &width );
    Boolean formatCell( TSpan<TScreenCell>, uint&, TStringView, uint& , TColorAttr );
    TStringView bufChars( uint );
    TStringView prevBufChars( uint );

    static TEditorDialog _NEAR editorDialog;
    static ushort _NEAR editorFlags;
    static char _NEAR findStr[maxFindStrLen];
    static char _NEAR replaceStr[maxReplaceStrLen];
    static TEditor * _NEAR clipboard;
    uchar lockCount;
    uchar updateFlags;
    int keyState;

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    TEditor( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TEditor& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TEditor*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TEditor& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TEditor* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TEditor

#if defined( Uses_TMemo ) && !defined( __TMemo )
#define __TMemo

struct _FAR TEvent;

struct TMemoData
{
    ushort length;
    char buffer[1];
};

class TMemo : public TEditor
{

public:

    TMemo( const TRect&, TScrollBar *, TScrollBar *, TIndicator *, ushort ) noexcept;
    virtual void getData( void *rec );
    virtual void setData( void *rec );
    virtual ushort dataSize();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& );

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    TMemo( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TMemo& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TMemo*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TMemo& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TMemo* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TMemo


#if defined( Uses_TFileEditor ) && !defined( __TFileEditor )
#define __TFileEditor

#if !defined( __DIR_H )
#include <tvision/compat/borland/dir.h>
#endif  // __DIR_H

class _FAR TRect;
class _FAR TScrollBar;
class _FAR TIndicator;
struct _FAR TEvent;

class TFileEditor : public TEditor
{

public:

    char fileName[MAXPATH];
    TFileEditor( const TRect&,
                 TScrollBar *,
                 TScrollBar *,
                 TIndicator *,
                 TStringView
               ) noexcept;
    virtual void doneBuffer();
    virtual void handleEvent( TEvent& );
    virtual void initBuffer();
    Boolean loadFile() noexcept;
    Boolean save() noexcept;
    Boolean saveAs() noexcept;
    Boolean saveFile() noexcept;
    virtual Boolean setBufSize( uint );
    virtual void shutDown();
    virtual void updateCommands();
    virtual Boolean valid( ushort );

private:

    static const char * _NEAR backupExt;

    virtual const char *streamableName() const
        { return name; }

protected:

    TFileEditor( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TFileEditor& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TFileEditor*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TFileEditor& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TFileEditor* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TFileEditor


#if defined( Uses_TEditWindow ) && !defined( __TEditWindow )
#define __TEditWindow

class _FAR TFileEditor;

class TEditWindow : public TWindow
{

public:

    TEditWindow( const TRect&, TStringView, int ) noexcept;
    virtual void close();
    virtual const char *getTitle( short );
    virtual void handleEvent( TEvent& );
    virtual void sizeLimits( TPoint& min, TPoint& max );

    TFileEditor *editor;

private:

    static const char * _NEAR clipboardTitle;
    static const char * _NEAR untitled;

    virtual const char *streamableName() const
        { return name; }

protected:

    TEditWindow( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TEditWindow& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TEditWindow*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TEditWindow& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TEditWindow* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TEditWindow


#if defined( Uses_TFindDialogRec ) && !defined( __TFindDialogRec )
#define __TFindDialogRec

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

struct TFindDialogRec
{
    TFindDialogRec( const char *str, ushort flgs ) noexcept
        {
        strnzcpy( find, str, sizeof(find) );
        options = flgs;
        }
    char find[maxFindStrLen];
    ushort options;
};

#endif  // Uses_TFindDialogRec

#if defined( Uses_TReplaceDialogRec ) && !defined( __TReplaceDialogRec )
#define __TReplaceDialogRec

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

struct TReplaceDialogRec
{
    TReplaceDialogRec( const char *str, const char *rep, ushort flgs ) noexcept
        {
        strnzcpy( find, str, sizeof(find) );
        strnzcpy( replace, rep, sizeof(replace) );
        options = flgs;
        }
    char find[maxFindStrLen];
    char replace[maxReplaceStrLen];
    ushort options;
};

#endif  // Uses_TReplaceDialogRec

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif

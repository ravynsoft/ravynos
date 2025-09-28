/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   DIALOGS.H                                                             */
/*                                                                         */
/*   defines the classes TDialog, TInputLine, TButton, TCluster,           */
/*   TRadioButtons, TCheckBoxes, TMultiCheckBoxes, TStaticText,            */
/*   TParamText, TLabel, THistoryViewer, and THistoryWindow.               */
/*                                                                         */
/* ------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if defined( __BORLANDC__ )
#pragma option -Vo-
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po-
#endif

#if !defined( __BUTTON_TYPE )
#define __BUTTON_TYPE

const int
    bfNormal    = 0x00,
    bfDefault   = 0x01,
    bfLeftJust  = 0x02,
    bfBroadcast = 0x04,
    bfGrabFocus = 0x08,

    cmRecordHistory = 60;

#endif  // __BUTTON_TYPE

/* ---------------------------------------------------------------------- */
/*      class TDialog                                                     */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Frame passive                                               */
/*        2 = Frame active                                                */
/*        3 = Frame icon                                                  */
/*        4 = ScrollBar page area                                         */
/*        5 = ScrollBar controls                                          */
/*        6 = StaticText                                                  */
/*        7 = Label normal                                                */
/*        8 = Label selected                                              */
/*        9 = Label shortcut                                              */
/*       10 = Button normal                                               */
/*       11 = Button default                                              */
/*       12 = Button selected                                             */
/*       13 = Button disabled                                             */
/*       14 = Button shortcut                                             */
/*       15 = Button shadow                                               */
/*       16 = Cluster normal                                              */
/*       17 = Cluster selected                                            */
/*       18 = Cluster shortcut                                            */
/*       19 = InputLine normal text                                       */
/*       20 = InputLine selected text                                     */
/*       21 = InputLine arrows                                            */
/*       22 = History arrow                                               */
/*       23 = History sides                                               */
/*       24 = HistoryWindow scrollbar page area                           */
/*       25 = HistoryWindow scrollbar controls                            */
/*       26 = ListViewer normal                                           */
/*       27 = ListViewer focused                                          */
/*       28 = ListViewer selected                                         */
/*       29 = ListViewer divider                                          */
/*       30 = InfoPane                                                    */
/*       31 = Cluster Disabled                                            */
/*       32 = Reserved                                                    */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TDialog ) && !defined( __TDialog )
#define __TDialog

#define  cpGrayDialog \
    "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F"\
    "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F"

#define  cpBlueDialog \
    "\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f"\
    "\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f"

#define  cpCyanDialog \
    "\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f"\
    "\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f"

#define cpDialog cpGrayDialog

const int
      dpBlueDialog = 0,
      dpCyanDialog = 1,
      dpGrayDialog = 2;

class _FAR TRect;
struct _FAR TEvent;
class _FAR TValidator;

class TDialog : public TWindow
{

public:

    TDialog( const TRect& bounds, TStringView aTitle ) noexcept;

    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    virtual Boolean valid( ushort command );

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    TDialog( StreamableInit ) noexcept;

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TDialog& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDialog*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDialog& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDialog* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TDialog

/* ---------------------------------------------------------------------- */
/*      class TInputLine                                                  */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Passive                                                     */
/*        2 = Active                                                      */
/*        3 = Selected                                                    */
/*        4 = Arrows                                                      */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TInputLine ) && !defined( __TInputLine )
#define __TInputLine

const ushort
    ilMaxBytes = 0,
    ilMaxWidth = 1,
    ilMaxChars = 2;

class _FAR TRect;
struct _FAR TEvent;
class _FAR TValidator;

class TInputLine : public TView
{

public:

    TInputLine( const TRect& bounds, uint limit, TValidator *aValid = 0, ushort limitMode = ilMaxBytes ) noexcept;
    ~TInputLine();

    virtual ushort dataSize();
    virtual void draw();
    virtual void getData( void *rec );
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    void selectAll( Boolean enable, Boolean scroll=True );
    virtual void setData( void *rec );
    virtual void setState( ushort aState, Boolean enable );
    virtual Boolean valid( ushort cmd );
    void setValidator( TValidator* aValid );

    char* data;
    uint maxLen;
    uint maxWidth;
    uint maxChars;
    int curPos;
    int firstPos;
    int selStart;
    int selEnd;

private:

    Boolean canScroll( int delta );
    int mouseDelta( TEvent& event );
    int mousePos( TEvent& event );
    int displayedPos( int pos );
    void deleteSelect();
    void deleteCurrent();
    void adjustSelectBlock();
    void saveState();
    void restoreState();
    Boolean checkValid(Boolean);
    Boolean canUpdateCommands();
    void setCmdState( ushort, Boolean );
    void updateCommands();

    static const char _NEAR rightArrow;
    static const char _NEAR leftArrow;

    virtual const char *streamableName() const
        { return name; }

    TValidator* validator;

    int anchor;
    char* oldData;
    int oldCurPos;
    int oldFirstPos;
    int oldSelStart;
    int oldSelEnd;

protected:

    TInputLine( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:
    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TInputLine& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TInputLine*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TInputLine& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TInputLine* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TInputLine


/* ---------------------------------------------------------------------- */
/*      TButton object                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Default text                                                */
/*        3 = Selected text                                               */
/*        4 = Disabled text                                               */
/*        5 = Normal shortcut                                             */
/*        6 = Default shortcut                                            */
/*        7 = Selected shortcut                                           */
/*        8 = Shadow                                                      */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TButton ) && !defined( __TButton )
#define __TButton

class _FAR TRect;
struct _FAR TEvent;
class _FAR TDrawBuffer;

class TButton : public TView
{

public:

    TButton( const TRect& bounds,
             TStringView aTitle,
             ushort aCommand,
             ushort aFlags
           ) noexcept;
    ~TButton();

    virtual void draw();
    void drawState( Boolean down );
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    void makeDefault( Boolean enable );
    virtual void press();
    virtual void setState( ushort aState, Boolean enable );

    const char *title;

protected:

    ushort command;
    uchar flags;
    Boolean amDefault;

private:

    void drawTitle( TDrawBuffer&, int, int, TAttrPair, Boolean );
    void pressButton( TEvent& );
    TRect getActiveRect();

    enum { animationDuration = 100 };
    TTimerId animationTimer;

    static const char * _NEAR shadows;
    static const char * _NEAR markers;

    virtual const char *streamableName() const
        { return name; }

protected:

    TButton( StreamableInit ) noexcept : TView( streamableInit ) {};
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TButton& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TButton*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TButton& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TButton* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TButton


#if defined( Uses_TSItem ) && !defined( __TSItem )
#define __TSItem

class TSItem
{

public:

    TSItem( TStringView aValue, TSItem *aNext ) noexcept
        { value = newStr(aValue); next = aNext; }
    ~TSItem() { delete[] (char *) value; }

    const char *value;
    TSItem *next;
};

#endif  // Uses_TSItem

/* ---------------------------------------------------------------------- */
/*      class TCluster                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Selected text                                               */
/*        3 = Normal shortcut                                             */
/*        4 = Selected shortcut                                           */
/*        5 = Disabled text                                               */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TCluster ) && !defined( __TCluster )
#define __TCluster

class _FAR TRect;
class _FAR TSItem;
struct _FAR TEvent;
class _FAR TPoint;
class _FAR TStringCollection;

class TCluster : public TView
{

public:

    TCluster( const TRect& bounds, TSItem *aStrings ) noexcept;
    ~TCluster();

    virtual ushort dataSize();
    void drawBox( const char *icon, char marker );
    void drawMultiBox(const char *icon, const char* marker);
    virtual void getData( void *rec );
    ushort getHelpCtx();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    virtual Boolean mark( int item );
    virtual uchar multiMark( int item );

    virtual void press( int item );
    virtual void movedTo( int item );
    virtual void setData( void *rec );
    virtual void setState( ushort aState, Boolean enable );
    virtual void setButtonState(uint32_t aMask, Boolean enable);

protected:

    uint32_t value;
    uint32_t enableMask;
    int sel;
    TStringCollection *strings;

private:

    int column( int item );
    int findSel( TPoint p );
    int row( int item );
    void moveSel(int, int);

    virtual const char *streamableName() const
        { return name; }

protected:

    TCluster( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:
    Boolean buttonState(int );

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TCluster& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TCluster*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TCluster& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TCluster* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TCluster


/* ---------------------------------------------------------------------- */
/*      class TRadioButtons                                               */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Selected text                                               */
/*        3 = Normal shortcut                                             */
/*        4 = Selected shortcut                                           */
/* ---------------------------------------------------------------------- */


#if defined( Uses_TRadioButtons ) && !defined( __TRadioButtons )
#define __TRadioButtons

class _FAR TRect;
class _FAR TSItem;

class TRadioButtons : public TCluster
{

public:

    TRadioButtons( const TRect& bounds, TSItem *aStrings ) noexcept;

    virtual void draw();
    virtual Boolean mark( int item );
    virtual void movedTo( int item );
    virtual void press( int item );
    virtual void setData( void *rec );

private:

    static const char * _NEAR button;
    virtual const char *streamableName() const
        { return name; }

protected:

    TRadioButtons( StreamableInit ) noexcept;

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TRadioButtons& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TRadioButtons*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TRadioButtons& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TRadioButtons* cl )
    { return os << (TStreamable *)cl; }

inline TRadioButtons::TRadioButtons( const TRect& bounds, TSItem *aStrings ) noexcept :
    TCluster( bounds, aStrings )
{
}

#endif  // Uses_TRadioButtons


/* ---------------------------------------------------------------------- */
/*      TCheckBoxes                                                       */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Selected text                                               */
/*        3 = Normal shortcut                                             */
/*        4 = Selected shortcut                                           */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TCheckBoxes ) && !defined( __TCheckBoxes )
#define __TCheckBoxes

class _FAR TRect;
class _FAR TSItem;

class TCheckBoxes : public TCluster
{

public:

    TCheckBoxes( const TRect& bounds, TSItem *aStrings) noexcept;

    virtual void draw();

    virtual Boolean mark( int item );
    virtual void press( int item );

private:

    static const char * _NEAR button;

    virtual const char *streamableName() const
        { return name; }

protected:

    TCheckBoxes( StreamableInit ) noexcept;

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TCheckBoxes& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TCheckBoxes*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TCheckBoxes& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TCheckBoxes* cl )
    { return os << (TStreamable *)cl; }

inline TCheckBoxes::TCheckBoxes( const TRect& bounds, TSItem *aStrings) noexcept :
    TCluster( bounds, aStrings )
{
}

#endif  // Uses_TCheckBoxes


#if defined( Uses_TMultiCheckBoxes ) && !defined( __TMultiCheckBoxes )
#define __TMultiCheckBoxes

const unsigned short cfOneBit       = 0x0101,
                     cfTwoBits      = 0x0203,
                     cfFourBits     = 0x040F,
                     cfEightBits    = 0x08FF;

/* ---------------------------------------------------------------------- */
/*      TMultiCheckBoxes                                                  */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Selected text                                               */
/*        3 = Normal shortcut                                             */
/*        4 = Selected shortcut                                           */
/* ---------------------------------------------------------------------- */

class _FAR TRect;
class _FAR TSItem;

class TMultiCheckBoxes : public TCluster
{
public:
    TMultiCheckBoxes(TRect&, TSItem*, uchar, ushort, const char*) noexcept;
    ~TMultiCheckBoxes();
    virtual ushort dataSize();
    virtual void draw();
    virtual void getData(void *);
    virtual uchar multiMark(int item);
    virtual void press( int item );
    virtual void setData(void*);

private:
    uchar selRange;
    ushort flags;
    char* states;

    virtual const char *streamableName() const
        { return name; }

protected:

    TMultiCheckBoxes( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:
    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TMultiCheckBoxes& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TMultiCheckBoxes*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TMultiCheckBoxes& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TMultiCheckBoxes* cl )
    { return os << (TStreamable *)cl; }

#endif


#if defined( Uses_TListBox ) && !defined( __TListBox )
#define __TListBox

class _FAR TRect;
class _FAR TScrollBar;
class _FAR TCollection;

struct TListBoxRec
{
    TCollection *items;
    ushort selection;
};

class TListBox : public TListViewer
{

public:

    TListBox( const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar ) noexcept;
    ~TListBox();

    virtual ushort dataSize();
    virtual void getData( void *rec );
    virtual void getText( char *dest, short item, short maxLen );
    virtual void newList( TCollection *aList );
    virtual void setData( void *rec );

    TCollection *list();

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    TCollection *items;

    TListBox( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TListBox& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TListBox*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TListBox& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TListBox* cl )
    { return os << (TStreamable *)cl; }

inline TCollection *TListBox::list()
{
    return items;
}

#endif  // Uses_TListBox


/* ---------------------------------------------------------------------- */
/*      class TStaticText                                                 */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Text                                                        */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TStaticText ) && !defined( __TStaticText )
#define __TStaticText

class _FAR TRect;

class TStaticText : public TView
{

public:

    TStaticText( const TRect& bounds, TStringView aText ) noexcept;
    ~TStaticText();

    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void getText( char * );

protected:

    const char *text;

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    TStaticText( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TStaticText& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TStaticText*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TStaticText& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TStaticText* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TStaticText


/* ---------------------------------------------------------------------- */
/*      class TParamText                                                  */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Text                                                        */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TParamText ) && !defined( __TParamText )
#define __TParamText

class _FAR TRect;

class TParamText : public TStaticText
{

public:
    TParamText( const TRect& bounds ) noexcept;
    ~TParamText();

    virtual void getText( char *str );
    virtual void setText( const char *fmt, ... );
    virtual int getTextLen();

protected:

    char *str;

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    TParamText( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TParamText& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TParamText*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TParamText& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TParamText* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TParamText


/* ---------------------------------------------------------------------- */
/*      class TLabel                                                      */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Selected text                                               */
/*        3 = Normal shortcut                                             */
/*        4 = Selected shortcut                                           */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TLabel ) && !defined( __TLabel )
#define __TLabel

class _FAR TRect;
struct _FAR TEvent;
class _FAR TView;

class TLabel : public TStaticText
{

public:

    TLabel( const TRect& bounds, TStringView aText, TView *aLink ) noexcept;

    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    virtual void shutDown();

protected:

    TView *link;
    Boolean light;

private:

    virtual const char *streamableName() const
        { return name; }
    void focusLink(TEvent&);

protected:

    TLabel( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TLabel& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TLabel*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TLabel& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TLabel* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TLabel


/* ---------------------------------------------------------------------- */
/*      class THistoryViewer                                              */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Active                                                      */
/*        2 = Inactive                                                    */
/*        3 = Focused                                                     */
/*        4 = Selected                                                    */
/*        5 = Divider                                                     */
/* ---------------------------------------------------------------------- */

#if defined( Uses_THistoryViewer ) && !defined( __THistoryViewer )
#define __THistoryViewer

class _FAR TRect;
class _FAR TScrollBar;

class THistoryViewer : public TListViewer
{

public:

    THistoryViewer( const TRect& bounds,
                    TScrollBar *aHScrollBar,
                    TScrollBar *aVScrollBar,
                    ushort aHistoryId
                  ) noexcept;

    virtual TPalette& getPalette() const;
    virtual void getText( char *dest, short item, short maxLen );
    virtual void handleEvent( TEvent& event );
    int historyWidth() noexcept;

protected:

    ushort historyId;

};

#endif  // Uses_THistoryViewer

#if defined( Uses_THistoryWindow ) && !defined( __THistoryWindow )
#define __THistoryWindow

class _FAR TListViewer;
class _FAR TRect;
class _FAR TWindow;
class _FAR TInputLine;

class THistInit
{

public:

    THistInit( TListViewer *(*cListViewer)( TRect, TWindow *, ushort ) ) noexcept;

protected:

    TListViewer *(*createListViewer)( TRect, TWindow *, ushort );

};

/* ---------------------------------------------------------------------- */
/*      THistoryWindow                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Frame passive                                               */
/*        2 = Frame active                                                */
/*        3 = Frame icon                                                  */
/*        4 = ScrollBar page area                                         */
/*        5 = ScrollBar controls                                          */
/*        6 = HistoryViewer normal text                                   */
/*        7 = HistoryViewer selected text                                 */
/* ---------------------------------------------------------------------- */

class THistoryWindow : public TWindow, public virtual THistInit
{

public:

    THistoryWindow( const TRect& bounds, ushort historyId ) noexcept;

    virtual TPalette& getPalette() const;
    virtual void getSelection( char *dest );
    virtual void handleEvent( TEvent& event );
    static TListViewer *initViewer( TRect, TWindow *, ushort );

protected:

    TListViewer *viewer;
};

#endif  // Uses_THistoryWindow

#if defined( Uses_THistory ) && !defined( __THistory )
#define __THistory

class _FAR TRect;
class _FAR TInputLine;
struct _FAR TEvent;
class _FAR THistoryWindow;

class THistory : public TView
{

public:

    THistory( const TRect& bounds, TInputLine *aLink, ushort aHistoryId ) noexcept;

    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    virtual THistoryWindow *initHistoryWindow( const TRect& bounds );
    virtual void recordHistory(const char *s);
    virtual void shutDown();

protected:

    TInputLine *link;
    ushort historyId;

private:

    static const char * _NEAR icon;

    virtual const char *streamableName() const
        { return name; }

protected:

    THistory( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, THistory& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, THistory*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, THistory& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, THistory* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_THistory

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif

/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   VIEWS.H                                                               */
/*                                                                         */
/*   defines the classes TView, TFrame, TScrollBar, TScroller,             */
/*   TListViewer, TGroup, and TWindow                                      */
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

#if !defined( __COMMAND_CODES )
#define __COMMAND_CODES

const ushort

//  Standard command codes

    cmValid         = 0,
    cmQuit          = 1,
    cmError         = 2,
    cmMenu          = 3,
    cmClose         = 4,
    cmZoom          = 5,
    cmResize        = 6,
    cmNext          = 7,
    cmPrev          = 8,
    cmHelp          = 9,

//  TDialog standard commands

    cmOK            = 10,
    cmCancel        = 11,
    cmYes           = 12,
    cmNo            = 13,
    cmDefault       = 14,

// Standard application commands

    cmNew           = 30,
    cmOpen          = 31,
    cmSave          = 32,
    cmSaveAs        = 33,
    cmSaveAll       = 34,
    cmChDir         = 35,
    cmDosShell      = 36,
    cmCloseAll      = 37,

//  TView State masks

    sfVisible       = 0x001,
    sfCursorVis     = 0x002,
    sfCursorIns     = 0x004,
    sfShadow        = 0x008,
    sfActive        = 0x010,
    sfSelected      = 0x020,
    sfFocused       = 0x040,
    sfDragging      = 0x080,
    sfDisabled      = 0x100,
    sfModal         = 0x200,
    sfDefault       = 0x400,
    sfExposed       = 0x800,

// TView Option masks

    ofSelectable    = 0x001,
    ofTopSelect     = 0x002,
    ofFirstClick    = 0x004,
    ofFramed        = 0x008,
    ofPreProcess    = 0x010,
    ofPostProcess   = 0x020,
    ofBuffered      = 0x040,
    ofTileable      = 0x080,
    ofCenterX       = 0x100,
    ofCenterY       = 0x200,
    ofCentered      = 0x300,
    ofValidate      = 0x400,

// TView GrowMode masks

    gfGrowLoX       = 0x01,
    gfGrowLoY       = 0x02,
    gfGrowHiX       = 0x04,
    gfGrowHiY       = 0x08,
    gfGrowAll       = 0x0f,
    gfGrowRel       = 0x10,
    gfFixed         = 0x20,

// TView DragMode masks

    dmDragMove      = 0x01,
    dmDragGrow      = 0x02,
    dmDragGrowLeft  = 0x04,
    dmLimitLoX      = 0x10,
    dmLimitLoY      = 0x20,
    dmLimitHiX      = 0x40,
    dmLimitHiY      = 0x80,
    dmLimitAll      = dmLimitLoX | dmLimitLoY | dmLimitHiX | dmLimitHiY,

// TView Help context codes

    hcNoContext     = 0,
    hcDragging      = 1,

// TScrollBar part codes

    sbLeftArrow     = 0,
    sbRightArrow    = 1,
    sbPageLeft      = 2,
    sbPageRight     = 3,
    sbUpArrow       = 4,
    sbDownArrow     = 5,
    sbPageUp        = 6,
    sbPageDown      = 7,
    sbIndicator     = 8,

// TScrollBar options for TWindow.StandardScrollBar

    sbHorizontal    = 0x000,
    sbVertical      = 0x001,
    sbHandleKeyboard = 0x002,

// TWindow Flags masks

    wfMove          = 0x01,
    wfGrow          = 0x02,
    wfClose         = 0x04,
    wfZoom          = 0x08,

//  TView inhibit flags

    noMenuBar       = 0x0001,
    noDeskTop       = 0x0002,
    noStatusLine    = 0x0004,
    noBackground    = 0x0008,
    noFrame         = 0x0010,
    noViewer        = 0x0020,
    noHistory       = 0x0040,

// TWindow number constants

    wnNoNumber      = 0,

// TWindow palette entries

    wpBlueWindow    = 0,
    wpCyanWindow    = 1,
    wpGrayWindow    = 2,

//  Application command codes

    cmCut           = 20,
    cmCopy          = 21,
    cmPaste         = 22,
    cmUndo          = 23,
    cmClear         = 24,
    cmTile          = 25,
    cmCascade       = 26,
    cmRedo          = 27,

// Standard messages

    cmReceivedFocus     = 50,
    cmReleasedFocus     = 51,
    cmCommandSetChanged = 52,
    cmTimeout           = 58,

// TScrollBar messages

    cmScrollBarChanged  = 53,
    cmScrollBarClicked  = 54,

// TWindow select messages

    cmSelectWindowNum   = 55,

//  TListViewer messages

    cmListItemSelected  = 56,

//  TProgram messages

    cmScreenChanged     = 57,

//  Event masks

    positionalEvents    = evMouse & ~evMouseWheel,
    focusedEvents       = evKeyboard | evCommand;

#endif  // __COMMAND_CODES

#if defined( Uses_TCommandSet ) && !defined( __TCommandSet )
#define __TCommandSet

class TCommandSet
{

public:

    TCommandSet() noexcept;
    TCommandSet( const TCommandSet& ) noexcept;

    Boolean has( int cmd ) noexcept;

    void disableCmd( int cmd ) noexcept;
    void enableCmd( int cmd ) noexcept;
    void operator += ( int cmd ) noexcept;
    void operator -= ( int cmd ) noexcept;

    void disableCmd( const TCommandSet& ) noexcept;
    void enableCmd( const TCommandSet& ) noexcept;
    void operator += ( const TCommandSet& ) noexcept;
    void operator -= ( const TCommandSet& ) noexcept;

    Boolean isEmpty() noexcept;

    TCommandSet& operator &= ( const TCommandSet& ) noexcept;
    TCommandSet& operator |= ( const TCommandSet& ) noexcept;

    friend TCommandSet operator & ( const TCommandSet&, const TCommandSet& ) noexcept;
    friend TCommandSet operator | ( const TCommandSet&, const TCommandSet& ) noexcept;

    friend int operator == ( const TCommandSet& tc1, const TCommandSet& tc2 ) noexcept;
    friend int operator != ( const TCommandSet& tc1, const TCommandSet& tc2 ) noexcept;

private:

    int loc( int ) noexcept;
    int mask( int ) noexcept;

    static int _NEAR masks[8];

    uchar cmds[32];

};

inline void TCommandSet::operator += ( int cmd ) noexcept
{
    enableCmd( cmd );
}

inline void TCommandSet::operator -= ( int cmd ) noexcept
{
    disableCmd( cmd );
}

inline void TCommandSet::operator += ( const TCommandSet& tc ) noexcept
{
    enableCmd( tc );
}

inline void TCommandSet::operator -= ( const TCommandSet& tc ) noexcept
{
    disableCmd( tc );
}

inline int operator != ( const TCommandSet& tc1, const TCommandSet& tc2 ) noexcept
{
    return !operator == ( tc1, tc2 );
}

inline int TCommandSet::loc( int cmd ) noexcept
{
    return cmd / 8;
}

inline int TCommandSet::mask( int cmd ) noexcept
{
    return masks[ cmd & 0x07 ];
}

#endif  // Uses_TCommandSet

#if defined( Uses_TPalette ) && !defined( __TPalette )
#define __TPalette

class TPalette
{

public:

    TPalette( const char *, ushort ) noexcept;
#ifndef __BORLANDC__
    TPalette( const TColorAttr *, ushort ) noexcept;
    template <size_t N>
    TPalette( const TColorAttr (&array) [N] ) noexcept :
        TPalette(array, (ushort) N)
    {
    }
#endif
    TPalette( const TPalette& ) noexcept;
    ~TPalette();

    TPalette& operator = ( const TPalette& ) noexcept;

    TColorAttr& operator[]( int ) const noexcept;

    TColorAttr *data;

};

inline TColorAttr& TPalette::operator[]( int index ) const noexcept
{
    return data[index];
}

#endif  // Uses_TPalette

#if defined( Uses_TView ) && !defined( __TView )
#define __TView

struct write_args
{
    void _FAR *self;
    void _FAR *target;
    void _FAR *buf;
    ushort offset;
};

class _FAR TRect;
struct _FAR TEvent;
class _FAR TGroup;

class TView : public TObject, public TStreamable
{

public:

    friend void genRefs();

    enum phaseType { phFocused, phPreProcess, phPostProcess };
    enum selectMode{ normalSelect, enterSelect, leaveSelect };

    TView( const TRect& bounds ) noexcept;
    ~TView();

    virtual void sizeLimits( TPoint& min, TPoint& max );
    TRect getBounds() const noexcept;
    TRect getExtent() const noexcept;
    TRect getClipRect() const noexcept;
    Boolean mouseInView( TPoint mouse ) noexcept;
    Boolean containsMouse( TEvent& event ) noexcept;

    void locate( TRect& bounds );
    virtual void dragView( TEvent& event, uchar mode,   //  temporary fix
      TRect& limits, TPoint minSize, TPoint maxSize ); //  for Miller's stuff
    virtual void calcBounds( TRect& bounds, TPoint delta );
    virtual void changeBounds( const TRect& bounds );
    void growTo( short x, short y );
    void moveTo( short x, short y );
    void setBounds( const TRect& bounds ) noexcept;

    virtual ushort getHelpCtx();

    virtual Boolean valid( ushort command );

    void hide();
    void show();
    virtual void draw();
    void drawView() noexcept;
    Boolean exposed() noexcept;
    Boolean focus();
    void hideCursor();
    void drawHide( TView *lastView );
    void drawShow( TView *lastView );
    void drawUnderRect( TRect& r, TView *lastView );
    void drawUnderView( Boolean doShadow, TView *lastView );

    virtual ushort dataSize();
    virtual void getData( void *rec );
    virtual void setData( void *rec );

    virtual void awaken();
    void blockCursor();
    void normalCursor();
    virtual void resetCursor();
    void setCursor( int x, int y ) noexcept;
    void showCursor();
    void drawCursor() noexcept;

    void clearEvent( TEvent& event ) noexcept;
    Boolean eventAvail();
    virtual void getEvent( TEvent& event );
    virtual void handleEvent( TEvent& event );
    virtual void putEvent( TEvent& event );

    static Boolean commandEnabled( ushort command ) noexcept;
    static void disableCommands( TCommandSet& commands ) noexcept;
    static void enableCommands( TCommandSet& commands ) noexcept;
    static void disableCommand( ushort command ) noexcept;
    static void enableCommand( ushort command ) noexcept;
    static void getCommands( TCommandSet& commands ) noexcept;
    static void setCommands( TCommandSet& commands ) noexcept;
    static void setCmdState( TCommandSet& commands, Boolean enable ) noexcept;

    virtual void endModal( ushort command );
    virtual ushort execute();

    TAttrPair getColor( ushort color ) noexcept;
    virtual TPalette& getPalette() const;
    virtual TColorAttr mapColor( uchar ) noexcept;

    Boolean getState( ushort aState ) const noexcept;
    void select();
    virtual void setState( ushort aState, Boolean enable );

    void getEvent( TEvent& event, int timeoutMs );
    void keyEvent( TEvent& event );
    Boolean mouseEvent( TEvent& event, ushort mask );
    Boolean textEvent( TEvent &event, TSpan<char> dest, size_t &length );

    virtual TTimerId setTimer( uint timeoutMs, int periodMs = -1 );
    virtual void killTimer( TTimerId id );

    TPoint makeGlobal( TPoint source ) noexcept;
    TPoint makeLocal( TPoint source ) noexcept;

    TView *nextView() noexcept;
    TView *prevView() noexcept;
    TView *prev() noexcept;
    TView *next;

    void makeFirst();
    void putInFrontOf( TView *Target );
    TView *TopView() noexcept;

    void writeBuf(  short x, short y, short w, short h, const void _FAR* b ) noexcept;
    void writeBuf(  short x, short y, short w, short h, const TDrawBuffer& b ) noexcept;
    void writeChar( short x, short y, char c, uchar color, short count ) noexcept;
    void writeLine( short x, short y, short w, short h, const TDrawBuffer& b ) noexcept;
    void writeLine( short x, short y, short w, short h, const void _FAR *b ) noexcept;
    void writeStr( short x, short y, const char *str, uchar color ) noexcept;
#ifndef __BORLANDC__
    void writeBuf(  short x, short y, short w, short h, const TScreenCell *b ) noexcept;
    void writeLine( short x, short y, short w, short h, const TScreenCell *b ) noexcept;
#endif

    TPoint size;
    ushort options;
    ushort eventMask;
    ushort state;
    TPoint origin;
    TPoint cursor;
    uchar growMode;
    uchar dragMode;
    ushort helpCtx;
    static Boolean _NEAR commandSetChanged;
#ifndef GENINC
    static TCommandSet _NEAR curCommandSet;
#endif
    TGroup *owner;

    static Boolean _NEAR showMarkers;
    static uchar _NEAR errorAttr;

    virtual void shutDown();

private:

    void moveGrow( TPoint p,
                   TPoint s,
                   TRect& limits,
                   TPoint minSize,
                   TPoint maxSize,
                   uchar mode
                 );
    void change( uchar, TPoint delta, TPoint& p, TPoint& s, ushort ctrlState ) noexcept;
    static void writeView( write_args );
    void writeView( short x, short y, short count, const void _FAR* b ) noexcept;
#ifndef __BORLANDC__
    void writeView( short x, short y, short count, const TScreenCell* b ) noexcept;
#endif

    TPoint resizeBalance;

    virtual const char *streamableName() const
        { return name; }

protected:

    TView( StreamableInit ) noexcept;

public:

    static const char * const _NEAR name;
    static TStreamable *build();

protected:

    virtual void write( opstream& );
    virtual void *read( ipstream& );

};

inline ipstream& operator >> ( ipstream& is, TView& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TView*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TView& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TView* cl )
    { return os << (TStreamable *)cl; }

inline void TView::writeBuf( short x, short y, short w, short h,
                             const TDrawBuffer& b ) noexcept
{
    writeBuf( x, y, min(w, short(b.length() - x)), h, &b.data[0] );
}

inline void TView::writeLine( short x, short y, short w, short h,
                              const TDrawBuffer& b ) noexcept
{
    writeLine( x, y, min(w, short(b.length() - x)), h, &b.data[0] );
}

#endif  // Uses_TView

/* ---------------------------------------------------------------------- */
/*      class TFrame                                                      */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Passive frame                                               */
/*        2 = Passive title                                               */
/*        3 = Active frame                                                */
/*        4 = Active title                                                */
/*        5 = Icons                                                       */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TFrame ) && !defined( __TFrame )
#define __TFrame

class _FAR TRect;
struct _FAR TEvent;
class _FAR TDrawBuffer;

class TFrame : public TView
{

public:

    TFrame( const TRect& bounds ) noexcept;

    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    virtual void setState( ushort aState, Boolean enable );

private:

    void frameLine( TDrawBuffer& frameBuf, short y, short n, TColorAttr color );
    void dragWindow( TEvent& event, uchar dragMode );

    friend class TDisplay;
    static const char _NEAR initFrame[19];
    static char _NEAR frameChars[33];
    static const char * _NEAR closeIcon;
    static const char * _NEAR zoomIcon;
    static const char * _NEAR unZoomIcon;
    static const char * _NEAR dragIcon;
    static const char * _NEAR dragLeftIcon;

    virtual const char *streamableName() const
        { return name; }

protected:

    TFrame( StreamableInit ) noexcept;

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TFrame& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TFrame*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TFrame& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TFrame* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TFrame

/* ---------------------------------------------------------------------- */
/*      class TScrollBar                                                  */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Page areas                                                  */
/*        2 = Arrows                                                      */
/*        3 = Indicator                                                   */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TScrollBar ) && !defined( __TScrollBar )
#define __TScrollBar

class _FAR TRect;
struct _FAR TEvent;

typedef char TScrollChars[5];

class TScrollBar : public TView
{

public:

    TScrollBar( const TRect& bounds ) noexcept;

    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    virtual void scrollDraw();
    virtual int scrollStep( int part );
    void setParams( int aValue, int aMin, int aMax,
                    int aPgStep, int aArStep ) noexcept;
    void setRange( int aMin, int aMax ) noexcept;
    void setStep( int aPgStep, int aArStep ) noexcept;
    void setValue( int aValue ) noexcept;

    void drawPos( int pos ) noexcept;
    int getPos() noexcept;
    int getSize() noexcept;

    int value;

    TScrollChars chars;
    int minVal;
    int maxVal;
    int pgStep;
    int arStep;

private:

    int getPartCode(void) noexcept;

    static TScrollChars _NEAR vChars;
    static TScrollChars _NEAR hChars;

    virtual const char *streamableName() const
        { return name; }

protected:

    TScrollBar( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TScrollBar& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TScrollBar*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TScrollBar& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TScrollBar* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TScrollBar

/* ---------------------------------------------------------------------- */
/*      class TScroller                                                   */
/*                                                                        */
/*      Palette layout                                                    */
/*      1 = Normal text                                                   */
/*      2 = Selected text                                                 */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TScroller ) && !defined( __TScroller )
#define __TScroller

class _FAR TRect;
class _FAR TScrollBar;
struct _FAR TEvent;

class TScroller : public TView
{

public:

    TScroller( const TRect& bounds,
               TScrollBar *aHScrollBar,
               TScrollBar *aVScrollBar
             ) noexcept;

    virtual void changeBounds( const TRect& bounds );
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    virtual void scrollDraw();
    void scrollTo( int x, int y ) noexcept;
    void setLimit( int x, int y ) noexcept;
    virtual void setState( ushort aState, Boolean enable );
    void checkDraw() noexcept;
    virtual void shutDown();
    TPoint delta;

protected:

    uchar drawLock;
    Boolean drawFlag;
    TScrollBar *hScrollBar;
    TScrollBar *vScrollBar;
    TPoint limit;

private:

    void showSBar( TScrollBar *sBar );

    virtual const char *streamableName() const
        { return name; }

protected:

    TScroller( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TScroller& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TScroller*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TScroller& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TScroller* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TScroller

#if defined( Uses_TListViewer ) && !defined( __TListViewer )
#define __TListViewer

class _FAR TRect;
class _FAR TScrollBar;
struct _FAR TEvent;

class TListViewer : public TView
{

    static const char * _NEAR emptyText;

public:

    TListViewer( const TRect& bounds,
                 ushort aNumCols,
                 TScrollBar *aHScrollBar,
                 TScrollBar *aVScrollBar
               ) noexcept;

    virtual void changeBounds( const TRect& bounds );
    virtual void draw();
    virtual void focusItem( short item );
    virtual TPalette& getPalette() const;
    virtual void getText( char *dest, short item, short maxLen );
    virtual Boolean isSelected( short item );
    virtual void handleEvent( TEvent& event );
    virtual void selectItem( short item );
    void setRange( short aRange );
    virtual void setState( ushort aState, Boolean enable );

    virtual void focusItemNum( short item );
    virtual void shutDown();

    TScrollBar *hScrollBar;
    TScrollBar *vScrollBar;
    short numCols;
    short topItem;
    short focused;
    short range;

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    TListViewer( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TListViewer& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TListViewer*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TListViewer& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TListViewer* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TListViewer

#if defined( Uses_TGroup ) && !defined( __TGroup )
#define __TGroup

class _FAR TView;

class TGroup : public TView
{

public:

    friend void genRefs();

    TGroup( const TRect& bounds ) noexcept;
    ~TGroup();

    virtual void shutDown();

    ushort execView( TView *p ) noexcept;
    virtual ushort execute();
    virtual void awaken();

    void insertView( TView *p, TView *Target ) noexcept;
    void remove( TView *p );
    void removeView( TView *p ) noexcept;
    void resetCurrent();
    void setCurrent( TView *p, selectMode mode );
    void selectNext( Boolean forwards );
    TView *firstThat( Boolean (*func)( TView *, void * ), void *args );
    Boolean focusNext(Boolean forwards);
    void forEach( void (*func)( TView *, void * ), void *args );
    void insert( TView *p ) noexcept;
    void insertBefore( TView *p, TView *Target );
    TView *current;
    TView *at( short index ) noexcept;
    TView *firstMatch( ushort aState, ushort aOptions ) noexcept;
    short indexOf( TView *p ) noexcept;
    TView *first() noexcept;

    virtual void setState( ushort aState, Boolean enable );

    virtual void handleEvent( TEvent& event );

    void drawSubViews( TView *p, TView *bottom ) noexcept;

    virtual void changeBounds( const TRect& bounds );

    virtual ushort dataSize();
    virtual void getData( void *rec );
    virtual void setData( void *rec );

    virtual void draw();
    void redraw() noexcept;
    void lock() noexcept;
    void unlock() noexcept;
    virtual void resetCursor();

    virtual void endModal( ushort command );

    virtual void eventError( TEvent& event );

    virtual ushort getHelpCtx();

    virtual Boolean valid( ushort command );

    void freeBuffer() noexcept;
    void getBuffer() noexcept;

    TView *last;

    TRect clip;
    phaseType phase;

    TScreenCell *buffer;
    uchar lockFlag;
    ushort endState;

private:

    void focusView( TView *p, Boolean enable );
    void selectView( TView *p, Boolean enable );
    TView* findNext(Boolean forwards) noexcept;

    virtual const char *streamableName() const
        { return name; }

protected:

    TGroup( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TGroup& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TGroup*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TGroup& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TGroup* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TGroup

#if defined( Uses_TWindow ) && !defined( __TWindow )
#define __TWindow

#define cpBlueWindow "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
#define cpCyanWindow "\x10\x11\x12\x13\x14\x15\x16\x17"
#define cpGrayWindow "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"

class _FAR TFrame;
class _FAR TRect;
class _FAR TPoint;
struct _FAR TEvent;
class _FAR TFrame;
class _FAR TScrollBar;

class TWindowInit
{

public:

    TWindowInit( TFrame *(*cFrame)( TRect ) ) noexcept;

protected:

    TFrame *(*createFrame)( TRect );

};

/* ---------------------------------------------------------------------- */
/*      class TWindow                                                     */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Frame passive                                               */
/*        2 = Frame active                                                */
/*        3 = Frame icon                                                  */
/*        4 = ScrollBar page area                                         */
/*        5 = ScrollBar controls                                          */
/*        6 = Scroller normal text                                        */
/*        7 = Scroller selected text                                      */
/*        8 = Reserved                                                    */
/* ---------------------------------------------------------------------- */

class TWindow: public TGroup, public virtual TWindowInit
{

public:

    TWindow( const TRect& bounds,
             TStringView aTitle,
             short aNumber
           ) noexcept;
    ~TWindow();

    virtual void close();
    virtual TPalette& getPalette() const;
    virtual const char *getTitle( short maxSize );
    virtual void handleEvent( TEvent& event );
    static TFrame *initFrame( TRect );
    virtual void setState( ushort aState, Boolean enable );
    virtual void sizeLimits( TPoint& min, TPoint& max );
    TScrollBar *standardScrollBar( ushort aOptions ) noexcept;
    virtual void zoom();
    virtual void shutDown();

    uchar flags;
    TRect zoomRect;
    short number;
    short palette;
    TFrame *frame;
    const char *title;

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    TWindow( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TWindow& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TWindow*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TWindow& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TWindow* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TWindow

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif

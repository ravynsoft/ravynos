/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   APP.H                                                                 */
/*                                                                         */
/*   defines the classes TBackground, TDeskTop, TProgram, and TApplication */
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

#if defined( Uses_TBackground ) && !defined( __TBackground )
#define __TBackground

class _FAR TRect;

class TBackground : public TView
{

public:

    TBackground( const TRect& bounds, char aPattern ) noexcept;
    virtual void draw();
    virtual TPalette& getPalette() const;

    char pattern;

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    TBackground( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TBackground& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TBackground*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TBackground& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TBackground* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TBackground


#if defined( Uses_TDeskTop )  && !defined( __TDeskTop )
#define __TDeskTop

class _FAR TBackground;
class _FAR TRect;
struct _FAR TEvent;

class TDeskInit
{

public:

    TDeskInit( TBackground *(*cBackground)( TRect ) ) noexcept;

protected:

    TBackground *(*createBackground)( TRect );

};

class TDeskTop : public TGroup, public virtual TDeskInit
{

public:

    TDeskTop( const TRect& ) noexcept;

    void cascade( const TRect& );
    virtual void handleEvent( TEvent& );
    static TBackground *initBackground( TRect );
    void tile( const TRect& );
    virtual void tileError();
    virtual void shutDown();

    TBackground *background;

protected:

    Boolean tileColumnsFirst;

private:

    static const char _NEAR defaultBkgrnd;

    virtual const char *streamableName() const
        { return name; }

protected:

    TDeskTop( StreamableInit ) noexcept;

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TDeskTop& cl )
    { return is >> (TStreamable&)(TGroup&)cl; }
inline ipstream& operator >> ( ipstream& is, TDeskTop*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDeskTop& cl )
    { return os << (TStreamable&)(TGroup&)cl; }
inline opstream& operator << ( opstream& os, TDeskTop* cl )
    { return os << (TStreamable *)(TGroup *)cl; }

#endif

// Turbo Vision 2.0 Color Palettes

#define cpAppColor \
       "\x71\x70\x78\x74\x20\x28\x24\x17\x1F\x1A\x31\x31\x1E\x71\x1F" \
    "\x37\x3F\x3A\x13\x13\x3E\x21\x3F\x70\x7F\x7A\x13\x13\x70\x7F\x7E" \
    "\x70\x7F\x7A\x13\x13\x70\x70\x7F\x7E\x20\x2B\x2F\x78\x2E\x70\x30" \
    "\x3F\x3E\x1F\x2F\x1A\x20\x72\x31\x31\x30\x2F\x3E\x31\x13\x38\x00" \
    "\x17\x1F\x1A\x71\x71\x1E\x17\x1F\x1E\x20\x2B\x2F\x78\x2E\x10\x30" \
    "\x3F\x3E\x70\x2F\x7A\x20\x12\x31\x31\x30\x2F\x3E\x31\x13\x38\x00" \
    "\x37\x3F\x3A\x13\x13\x3E\x30\x3F\x3E\x20\x2B\x2F\x78\x2E\x30\x70" \
    "\x7F\x7E\x1F\x2F\x1A\x20\x32\x31\x71\x70\x2F\x7E\x71\x13\x78\x00" \
    "\x37\x3F\x3A\x13\x13\x30\x3E\x1E"    // help colors

#define cpAppBlackWhite \
       "\x70\x70\x78\x7F\x07\x07\x0F\x07\x0F\x07\x70\x70\x07\x70\x0F" \
    "\x07\x0F\x07\x70\x70\x07\x70\x0F\x70\x7F\x7F\x70\x07\x70\x07\x0F" \
    "\x70\x7F\x7F\x70\x07\x70\x70\x7F\x7F\x07\x0F\x0F\x78\x0F\x78\x07" \
    "\x0F\x0F\x0F\x70\x0F\x07\x70\x70\x70\x07\x70\x0F\x07\x07\x08\x00" \
    "\x07\x0F\x0F\x07\x70\x07\x07\x0F\x0F\x70\x78\x7F\x08\x7F\x08\x70" \
    "\x7F\x7F\x7F\x0F\x70\x70\x07\x70\x70\x70\x07\x7F\x70\x07\x78\x00" \
    "\x70\x7F\x7F\x70\x07\x70\x70\x7F\x7F\x07\x0F\x0F\x78\x0F\x78\x07" \
    "\x0F\x0F\x0F\x70\x0F\x07\x70\x70\x70\x07\x70\x0F\x07\x07\x08\x00" \
    "\x07\x0F\x07\x70\x70\x07\x0F\x70"    // help colors

#define cpAppMonochrome \
       "\x70\x07\x07\x0F\x70\x70\x70\x07\x0F\x07\x70\x70\x07\x70\x00" \
    "\x07\x0F\x07\x70\x70\x07\x70\x00\x70\x70\x70\x07\x07\x70\x07\x00" \
    "\x70\x70\x70\x07\x07\x70\x70\x70\x0F\x07\x07\x0F\x70\x0F\x70\x07" \
    "\x0F\x0F\x07\x70\x07\x07\x70\x07\x07\x07\x70\x0F\x07\x07\x70\x00" \
    "\x70\x70\x70\x07\x07\x70\x70\x70\x0F\x07\x07\x0F\x70\x0F\x70\x07" \
    "\x0F\x0F\x07\x70\x07\x07\x70\x07\x07\x07\x70\x0F\x07\x07\x01\x00" \
    "\x70\x70\x70\x07\x07\x70\x70\x70\x0F\x07\x07\x0F\x70\x0F\x70\x07" \
    "\x0F\x0F\x07\x70\x07\x07\x70\x07\x07\x07\x70\x0F\x07\x07\x01\x00" \
    "\x07\x0F\x07\x70\x70\x07\x0F\x70"    // help colors

#if defined( Uses_TProgram ) && !defined( __TProgram )
#define __TProgram

// Standard application help contexts

// Note: range $FF00 - $FFFF of help contexts are reserved by Borland

const unsigned short hcNew          = 0xFF01;
const unsigned short hcOpen         = 0xFF02;
const unsigned short hcSave         = 0xFF03;
const unsigned short hcSaveAs       = 0xFF04;
const unsigned short hcSaveAll      = 0xFF05;
const unsigned short hcChangeDir    = 0xFF06;
const unsigned short hcDosShell     = 0xFF07;
const unsigned short hcExit         = 0xFF08;

const unsigned short hcUndo         = 0xFF10;
const unsigned short hcCut          = 0xFF11;
const unsigned short hcCopy         = 0xFF12;
const unsigned short hcPaste        = 0xFF13;
const unsigned short hcClear        = 0xFF14;

const unsigned short hcTile         = 0xFF20;
const unsigned short hcCascade      = 0xFF21;
const unsigned short hcCloseAll     = 0xFF22;
const unsigned short hcResize       = 0xFF23;
const unsigned short hcZoom         = 0xFF24;
const unsigned short hcNext         = 0xFF25;
const unsigned short hcPrev         = 0xFF26;
const unsigned short hcClose        = 0xFF27;


class _FAR TStatusLine;
class _FAR TMenuBar;
class _FAR TDeskTop;
struct _FAR TEvent;
class _FAR TView;

class TProgInit
{

public:

    TProgInit( TStatusLine *(*cStatusLine)( TRect ),
               TMenuBar *(*cMenuBar)( TRect ),
               TDeskTop *(*cDeskTop )( TRect )
             ) noexcept;

protected:

    TStatusLine *(*createStatusLine)( TRect );
    TMenuBar *(*createMenuBar)( TRect );
    TDeskTop *(*createDeskTop)( TRect );

};

/* ---------------------------------------------------------------------- */
/*      class TProgram                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*          1 = TBackground                                               */
/*       2- 7 = TMenuView and TStatusLine                                 */
/*       8-15 = TWindow(Blue)                                             */
/*      16-23 = TWindow(Cyan)                                             */
/*      24-31 = TWindow(Gray)                                             */
/*      32-63 = TDialog                                                   */
/* ---------------------------------------------------------------------- */

const int

//  TApplication palette entries

    apColor      = 0,
    apBlackWhite = 1,
    apMonochrome = 2;

class _FAR TDialog;
class _FAR TWindow;
class _FAR TTimerQueue;

class TProgram : public TGroup, public virtual TProgInit
{

public:

    TProgram() noexcept;
    virtual ~TProgram();

    virtual Boolean canMoveFocus();
    virtual ushort executeDialog(TDialog*, void*data = 0);
    virtual void getEvent(TEvent& event);
    virtual TPalette& getPalette() const;
    virtual void handleEvent(TEvent& event);
    virtual void idle();
    virtual void initScreen();
    virtual void outOfMemory();
    virtual void putEvent( TEvent& event );
    virtual void run();
    virtual TWindow* insertWindow(TWindow*);
    void setScreenMode( ushort mode );
    TView *validView( TView *p ) noexcept;
    virtual void shutDown();

    virtual TTimerId setTimer( uint timeoutMs, int periodMs = -1 );
    virtual void killTimer( TTimerId id );

    virtual void suspend() {}
    virtual void resume() {}

    static TStatusLine *initStatusLine( TRect );
    static TMenuBar *initMenuBar( TRect );
    static TDeskTop *initDeskTop( TRect );

    static TProgram * _NEAR application;
    static TStatusLine * _NEAR statusLine;
    static TMenuBar * _NEAR menuBar;
    static TDeskTop * _NEAR deskTop;
    static int _NEAR appPalette;
    static int _NEAR eventTimeout;

protected:

    static TEvent _NEAR pending;

private:

    static int eventWaitTimeout();

    static const char * _NEAR exitText;
    static TTimerQueue _NEAR timerQueue;

};

#endif

#if defined( Uses_TApplication ) && !defined( __TApplication )
#define __TApplication

class TSubsystemsInit
{

public:

    TSubsystemsInit() noexcept;

};

class TApplication : public virtual TSubsystemsInit, public TProgram
{

protected:

    TApplication() noexcept;
    virtual ~TApplication();

public:
    virtual void suspend();
    virtual void resume();

    void cascade();
    void dosShell();
    virtual TRect getTileRect();
    virtual void handleEvent(TEvent &event);
    void tile();
    virtual void writeShellMsg();

};

#endif

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif

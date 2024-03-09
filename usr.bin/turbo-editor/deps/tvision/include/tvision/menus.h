/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   MENUS.H                                                               */
/*                                                                         */
/*   defines the classes TMenuItem, TMenu, TMenuView, TSubMenu,            */
/*   TMenuBar, TMenuBox, TStatusItem, TStatusDef, and TStatusLine          */
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

class _FAR TSubMenu;
class _FAR TMenuItem;
class _FAR TStatusDef;
class _FAR TStatusItem;

TSubMenu& operator + ( TSubMenu& s, TMenuItem& i ) noexcept;
TSubMenu& operator + ( TSubMenu& s1, TSubMenu& s2 ) noexcept;
TMenuItem& operator + ( TMenuItem& i1, TMenuItem& i2 ) noexcept;
TStatusDef& operator + ( TStatusDef& s1, TStatusItem& s2 ) noexcept;
TStatusDef& operator + ( TStatusDef& s1, TStatusDef& s2 ) noexcept;

#if defined( Uses_TMenuItem ) && !defined( __TMenuItem )
#define __TMenuItem

class _FAR TMenu;

class TMenuItem
{

public:

    TMenuItem( TStringView aName,
               ushort aCommand,
               TKey aKey,
               ushort aHelpCtx = hcNoContext,
               TStringView p = 0,
               TMenuItem *aNext = 0
             ) noexcept;
    TMenuItem( TStringView aName,
               TKey aKey,
               TMenu *aSubMenu,
               ushort aHelpCtx = hcNoContext,
               TMenuItem *aNext = 0
             ) noexcept;

    ~TMenuItem();

    void append( TMenuItem *aNext ) noexcept;

    TMenuItem *next;
    const char *name;
    ushort command;
    Boolean disabled;
    TKey keyCode;
    ushort helpCtx;
    union
        {
        const char *param;
        TMenu *subMenu;
        };
};

inline void TMenuItem::append( TMenuItem *aNext ) noexcept
{
    next = aNext;
}

inline TMenuItem &newLine() noexcept
{
    return *new TMenuItem( 0, 0, 0, hcNoContext, 0, 0 );
}

#endif  // Uses_TMenuItem

#if defined( Uses_TSubMenu ) && !defined( __TSubMenu )
#define __TSubMenu

class TSubMenu : public TMenuItem
{

public:

    TSubMenu( TStringView nm, TKey key, ushort helpCtx = hcNoContext ) noexcept;

};

#endif  // Uses_TSubMenu

#if defined( Uses_TMenu ) && !defined( __TMenu )
#define __TMenu

class TMenu
{

public:

    TMenu() noexcept : items(0), deflt(0) {};
    TMenu( TMenuItem& itemList ) noexcept
        { items = &itemList; deflt = &itemList; }
    TMenu( TMenuItem& itemList, TMenuItem& TheDefault ) noexcept
        { items = &itemList; deflt = &TheDefault; }
    ~TMenu();

    TMenuItem *items;
    TMenuItem *deflt;

};

#endif  // Uses_TMenu

/* ---------------------------------------------------------------------- */
/*      class TMenuView                                                   */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Disabled text                                               */
/*        3 = Shortcut text                                               */
/*        4 = Normal selection                                            */
/*        5 = Disabled selection                                          */
/*        6 = Shortcut selection                                          */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TMenuView ) && !defined( __TMenuView )
#define __TMenuView

class _FAR TRect;
class _FAR TMenu;
struct _FAR TEvent;

class TMenuView : public TView
{

public:

    TMenuView( const TRect& bounds, TMenu *aMenu, TMenuView *aParent = 0 ) noexcept;
    TMenuView( const TRect& bounds ) noexcept;

    virtual ushort execute();
    TMenuItem *findItem( char ch );
    virtual TRect getItemRect( TMenuItem *item );
    virtual ushort getHelpCtx();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    TMenuItem *hotKey( TKey key );
    TMenuView *newSubView( const TRect& bounds,
                           TMenu *aMenu,
                           TMenuView *aParentMenu
                         );

protected:

    TMenuView *parentMenu;
    TMenu *menu;
    TMenuItem *current;

    Boolean putClickEventOnExit;

private:

    void nextItem();
    void prevItem();
    void trackKey( Boolean findNext );
    Boolean mouseInOwner( TEvent& e );
    Boolean mouseInMenus( TEvent& e );
    void trackMouse( TEvent& e , Boolean& mouseActive);
    TMenuView *topMenu();
    Boolean updateMenu( TMenu *menu );
    void do_a_select( TEvent& );
    TMenuItem *findHotKey( TMenuItem *p, TKey key );

private:

    virtual const char *streamableName() const
        { return name; }
    static void writeMenu( opstream&, TMenu * );
    static TMenu *readMenu( ipstream& );

protected:

    TMenuView( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TMenuView& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TMenuView*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TMenuView& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TMenuView* cl )
    { return os << (TStreamable *)cl; }

inline TMenuView::TMenuView( const TRect& bounds,
                             TMenu *aMenu,
                             TMenuView *aParent
                           ) noexcept :
    TView(bounds), parentMenu( aParent ), menu( aMenu ), current( 0 ),
    putClickEventOnExit( True )
{
    eventMask |= evBroadcast;
}

inline TMenuView::TMenuView( const TRect& bounds ) noexcept :
    TView(bounds), parentMenu(0), menu(0), current(0),
    putClickEventOnExit( True )
{
    eventMask |= evBroadcast;
}

#endif  // Uses_TMenuView

/* ---------------------------------------------------------------------- */
/*      class TMenuBar                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Disabled text                                               */
/*        3 = Shortcut text                                               */
/*        4 = Normal selection                                            */
/*        5 = Disabled selection                                          */
/*        6 = Shortcut selection                                          */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TMenuBar ) && !defined( __TMenuBar )
#define __TMenuBar

class _FAR TRect;
class _FAR TMenu;

class TMenuBar : public TMenuView
{

public:

    TMenuBar( const TRect& bounds, TMenu *aMenu ) noexcept;
    TMenuBar( const TRect& bounds, TSubMenu &aMenu ) noexcept;
    ~TMenuBar();

    virtual void draw();
    virtual TRect getItemRect( TMenuItem *item );

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    TMenuBar( StreamableInit ) noexcept;

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TMenuBar& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TMenuBar*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TMenuBar& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TMenuBar* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TMenuBar

/* ---------------------------------------------------------------------- */
/*      class TMenuBox                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Disabled text                                               */
/*        3 = Shortcut text                                               */
/*        4 = Normal selection                                            */
/*        5 = Disabled selection                                          */
/*        6 = Shortcut selection                                          */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TMenuBox ) && !defined( __TMenuBox )
#define __TMenuBox

class _FAR TRect;
class _FAR TMenu;
class _FAR TMenuView;
class _FAR TDrawBuffer;

class TMenuBox : public TMenuView
{

public:

    TMenuBox( const TRect& bounds, TMenu *aMenu, TMenuView *aParentMenu) noexcept;

    virtual void draw();
    virtual TRect getItemRect( TMenuItem *item );

private:

    void frameLine( TDrawBuffer&, short n );
    void drawLine( TDrawBuffer& );

    static const char * _NEAR frameChars;
    virtual const char *streamableName() const
        { return name; }

protected:

    TMenuBox( StreamableInit ) noexcept;

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};


inline ipstream& operator >> ( ipstream& is, TMenuBox& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TMenuBox*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TMenuBox& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TMenuBox* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TMenuBox


#if defined( Uses_TMenuPopup ) && !defined( __TMenuPopup )
#define __TMenuPopup

/* ---------------------------------------------------------------------- */
/*      class TMenuPopup                                                  */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Disabled text                                               */
/*        3 = Shortcut text                                               */
/*        4 = Normal selection                                            */
/*        5 = Disabled selection                                          */
/*        6 = Shortcut selection                                          */
/* ---------------------------------------------------------------------- */

class TMenuPopup : public TMenuBox
{

public:

    TMenuPopup(const TRect& bounds, TMenu *aMenu, TMenuView *aParent = 0) noexcept;
    virtual ushort execute();
    virtual void handleEvent(TEvent&);

protected:

    TMenuPopup( StreamableInit ) noexcept;

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};


#endif  // Uses_TMenuPopup




#if defined( Uses_TStatusItem ) && !defined( __TStatusItem )
#define __TStatusItem

class TStatusItem
{

public:

    TStatusItem( TStringView aText,
                 TKey aKey,
                 ushort cmd,
                 TStatusItem *aNext = 0
                ) noexcept;
    ~TStatusItem();

    TStatusItem *next;
    char *text;
    TKey keyCode;
    ushort command;

};

inline TStatusItem::TStatusItem( TStringView aText,
                                 TKey aKey,
                                 ushort cmd,
                                 TStatusItem *aNext
                                ) noexcept :
     next( aNext ), text( newStr(aText) ), keyCode( aKey ), command( cmd )
{
}

inline TStatusItem::~TStatusItem()
{
    delete[] text;
}

#endif  // Uses_TStatusItem

#if defined( Uses_TStatusDef ) && !defined( __TStatusDef )
#define __TStatusDef

class TStatusDef
{

public:

    TStatusDef( ushort aMin,
                ushort aMax,
                TStatusItem *someItems = 0,
                TStatusDef *aNext = 0
              ) noexcept;

    TStatusDef *next;
    ushort min;
    ushort max;
    TStatusItem *items;
};

inline TStatusDef::TStatusDef( ushort aMin,
                               ushort aMax,
                               TStatusItem *someItems,
                               TStatusDef *aNext
                             ) noexcept :
    next( aNext ), min( aMin ), max( aMax ), items( someItems )
{
}

#endif  // Uses_TStatusDef

/* ---------------------------------------------------------------------- */
/*      class TStatusLine                                                 */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Disabled text                                               */
/*        3 = Shortcut text                                               */
/*        4 = Normal selection                                            */
/*        5 = Disabled selection                                          */
/*        6 = Shortcut selection                                          */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TStatusLine ) && !defined( __TStatusLine )
#define __TStatusLine

class _FAR TRect;
struct _FAR TEvent;
class _FAR TPoint;

class TStatusLine : public TView
{

public:

    TStatusLine( const TRect& bounds, TStatusDef& aDefs ) noexcept;
    ~TStatusLine();

    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    virtual const char* hint( ushort aHelpCtx );
    void update();

protected:

    TStatusItem *items;
    TStatusDef *defs;

private:

    void drawSelect( TStatusItem *selected );
    void findItems() noexcept;
    TStatusItem *itemMouseIsIn( TPoint );
    void disposeItems( TStatusItem *item );

    static const char * _NEAR hintSeparator;

    virtual const char *streamableName() const
        { return name; }

    static void writeItems( opstream&, TStatusItem * );
    static void writeDefs( opstream&, TStatusDef * );
    static TStatusItem *readItems( ipstream& );
    static TStatusDef *readDefs( ipstream& );


protected:

    TStatusLine( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TStatusLine& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TStatusLine*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TStatusLine& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TStatusLine* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TStatusLine

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif

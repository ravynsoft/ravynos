/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   COLORSEL.H                                                            */
/*                                                                         */
/*   defines the class TColorDialog, used to set application palettes      */
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

#if !defined( __COLOR_COMMAND_CODES )
#define __COLOR_COMMAND_CODES

const int
  cmColorForegroundChanged = 71,
  cmColorBackgroundChanged = 72,
  cmColorSet               = 73,
  cmNewColorItem           = 74,
  cmNewColorIndex          = 75,
  cmSaveColorIndex         = 76;

#endif  // __COLOR_COMMAND_CODES

class _FAR TColorItem;
class _FAR TColorGroup;

TColorItem& operator + ( TColorItem& i1, TColorItem& i2 ) noexcept;
TColorGroup& operator + ( TColorGroup& g, TColorItem& i ) noexcept;
TColorGroup& operator + ( TColorGroup& g1, TColorGroup& g2 ) noexcept;

#if defined( Uses_TColorItem ) && !defined( __TColorItem )
#define __TColorItem

class _FAR TColorGroup;

class TColorItem
{

public:

    TColorItem( const char *nm, uchar idx, TColorItem *nxt = 0 ) noexcept;
    virtual ~TColorItem();
    const char *name;
    uchar index;
    TColorItem *next;
    friend TColorGroup& operator + ( TColorGroup&, TColorItem& ) noexcept;
    friend TColorItem& operator + ( TColorItem& i1, TColorItem& i2 ) noexcept;

};

#endif  // Uses_TColorItem

#if defined( Uses_TColorGroup ) && !defined( __TColorGroup )
#define __TColorGroup

class _FAR TColorItem;

class TColorGroup
{

public:

    TColorGroup( const char *nm, TColorItem *itm = 0, TColorGroup *nxt = 0 ) noexcept;
    virtual ~TColorGroup();
    const char *name;
    uchar index;
    TColorItem *items;
    TColorGroup *next;
    friend TColorGroup& operator + ( TColorGroup&, TColorItem& ) noexcept;
    friend TColorGroup& operator + ( TColorGroup& g1, TColorGroup& g2 ) noexcept;


};

class TColorIndex
{
public:
    uchar groupIndex;
    uchar colorSize;
    uchar colorIndex[256];
};


#endif  // Uses_TColorGroup

#if defined( Uses_TColorSelector ) && !defined( __TColorSelector )
#define __TColorSelector

class _FAR TRect;
struct _FAR TEvent;

class TColorSelector : public TView
{

public:

    enum ColorSel { csBackground, csForeground };

    TColorSelector( const TRect& Bounds, ColorSel ASelType ) noexcept;
    virtual void draw();
    virtual void handleEvent( TEvent& event );

protected:

    uchar color;
    ColorSel selType;

private:

    void colorChanged();

    static const char _NEAR icon;

    virtual const char *streamableName() const
        { return name; }

protected:

    TColorSelector( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TColorSelector& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TColorSelector*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TColorSelector& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TColorSelector* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TColorSelector


#if defined( Uses_TMonoSelector ) && !defined( __TMonoSelector )
#define __TMonoSelector

class _FAR TRect;
struct _FAR TEvent;

class TMonoSelector : public TCluster
{

public:

    TMonoSelector( const TRect& bounds ) noexcept;
    virtual void draw();
    virtual void handleEvent( TEvent& event );
    virtual Boolean mark( int item );
    void newColor();
    virtual void press( int item );
    void movedTo( int item );

private:

    static const char * _NEAR button;
    static const char * _NEAR normal;
    static const char * _NEAR highlight;
    static const char * _NEAR underline;
    static const char * _NEAR inverse;

    virtual const char *streamableName() const
        { return name; }

protected:

    TMonoSelector( StreamableInit ) noexcept;

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TMonoSelector& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TMonoSelector*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TMonoSelector& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TMonoSelector* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TMonoSelector

#if defined( Uses_TColorDisplay ) && !defined( __TColorDisplay )
#define __TColorDisplay

class _FAR TRect;
struct _FAR TEvent;

class TColorDisplay : public TView
{

public:

    TColorDisplay( const TRect& bounds, TStringView aText ) noexcept;
    virtual ~TColorDisplay();
    virtual void draw();
    virtual void handleEvent( TEvent& event );
    virtual void setColor( TColorAttr *aColor );

protected:

    TColorAttr *color;
    const char *text;

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    TColorDisplay( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TColorDisplay& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TColorDisplay*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TColorDisplay& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TColorDisplay* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TColorDisplay


#if defined( Uses_TColorGroupList ) && !defined( __TColorGroupList )
#define __TColorGroupList

class _FAR TRect;
class _FAR TScrollBar;
class _FAR TColorGroup;
class _FAR TColorItem;

class TColorGroupList : public TListViewer
{

public:

    TColorGroupList( const TRect& bounds,
                     TScrollBar *aScrollBar,
                     TColorGroup *aGroups
                   ) noexcept;
    virtual ~TColorGroupList();
    virtual void focusItem( short item );
    virtual void getText( char *dest, short item, short maxLen );

    virtual void handleEvent(TEvent&);


protected:

    TColorGroup *groups;

private:

    virtual const char *streamableName() const
        { return name; }
    static void writeItems( opstream&, TColorItem * );
    static void writeGroups( opstream&, TColorGroup * );
    static TColorItem *readItems( ipstream& );
    static TColorGroup *readGroups( ipstream& );

protected:

    TColorGroupList( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    void setGroupIndex(uchar groupNum, uchar itemNum);
    TColorGroup* getGroup(uchar groupNum);
    uchar getGroupIndex(uchar groupNum);
    uchar getNumGroups();
    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TColorGroupList& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TColorGroupList*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TColorGroupList& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TColorGroupList* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TColorGroupList


#if defined( Uses_TColorItemList ) && !defined( __TColorItemList )
#define __TColorItemList

class _FAR TRect;
class _FAR TScrollBar;
class _FAR TColorItem;
struct _FAR TEvent;

class TColorItemList : public TListViewer
{

public:

    TColorItemList( const TRect& bounds,
                    TScrollBar *aScrollBar,
                    TColorItem *aItems
                  ) noexcept;
    virtual void focusItem( short item );
    virtual void getText( char *dest, short item, short maxLen );
    virtual void handleEvent( TEvent& event );

protected:

    TColorItem *items;

private:

    virtual const char *streamableName() const
        { return name; }

protected:

    TColorItemList( StreamableInit ) noexcept;

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TColorItemList& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TColorItemList*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TColorItemList& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TColorItemList* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TColorItemList


#if defined( Uses_TColorDialog ) && !defined( __TColorDialog )
#define __TColorDialog

class _FAR TColorGroup;
struct _FAR TEvent;
class _FAR TColorDisplay;
class _FAR TColorGroupList;
class _FAR TLabel;
class _FAR TColorSelector;
class _FAR TMonoSelector;
class _FAR TPalette;

class TColorDialog : public TDialog
{

public:

    TColorDialog( TPalette *aPalette, TColorGroup *aGroups ) noexcept;
    ~TColorDialog();
    virtual ushort dataSize();
    virtual void getData( void *rec );
    virtual void handleEvent( TEvent& event );
    virtual void setData( void *rec);

    TPalette *pal;

protected:

    TColorDisplay *display;
    TColorGroupList *groups;
    TLabel *forLabel;
    TColorSelector *forSel;
    TLabel *bakLabel;
    TColorSelector *bakSel;
    TLabel *monoLabel;
    TMonoSelector *monoSel;
    uchar groupIndex;

private:

    static const char * _NEAR colors;
    static const char * _NEAR groupText;
    static const char * _NEAR itemText;
    static const char * _NEAR forText;
    static const char * _NEAR bakText;
    static const char * _NEAR textText;
    static const char * _NEAR colorText;
    static const char * _NEAR okText;
    static const char * _NEAR cancelText;

    virtual const char *streamableName() const
        { return name; }

protected:

    TColorDialog( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    void getIndexes(TColorIndex*&);
    void setIndexes(TColorIndex*&);
    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TColorDialog& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TColorDialog*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TColorDialog& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TColorDialog* cl )
    { return os << (TStreamable *)cl; }

#endif  // TColorDialog

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif

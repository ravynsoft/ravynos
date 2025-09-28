/*------------------------------------------------------------*/
/* filename -       colorsel.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  Member function(s) of following classes   */
/*                      TColorSelector                        */
/*                      TMonoSelector                         */
/*                      TColorDisplay                         */
/*                      TColorItem                            */
/*                      TColorItemList                        */
/*                      TColorGroup                           */
/*                      TColorGroupList                       */
/*                      TColorDialog                          */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TKeys
#define Uses_TColorSelector
#define Uses_TMonoSelector
#define Uses_TColorDisplay
#define Uses_TColorItem
#define Uses_TColorItemList
#define Uses_TColorGroup
#define Uses_TColorGroupList
#define Uses_TColorDialog
#define Uses_TEvent
#define Uses_TDrawBuffer
#define Uses_TGroup
#define Uses_TSItem
#define Uses_TScrollBar
#define Uses_TLabel
#define Uses_TButton
#define Uses_TRect
#define Uses_opstream
#define Uses_ipstream
#include <tvision/tv.h>

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

static TColorIndex* colorIndexes = 0;

TColorItem::TColorItem( const char *nm, uchar idx, TColorItem *nxt ) noexcept
{
    index = idx;
    next = nxt;
    name = newStr( nm );
}

TColorItem::~TColorItem()
{
    delete[] (char *) name;
}

TColorGroup::TColorGroup( const char *nm, TColorItem *itm, TColorGroup *nxt ) noexcept
{
    items = itm;
    next = nxt;
    name = newStr( nm );
}

TColorGroup::~TColorGroup()
{
    delete[] (char *) name;
}

TColorItem& operator + ( TColorItem& i1, TColorItem& i2 ) noexcept
{
    TColorItem *cur = &i1;
    while( cur->next != 0 )
        cur = cur->next;
    cur->next = &i2;
    return i1;
}

TColorGroup& operator + ( TColorGroup& g, TColorItem& i ) noexcept
{
    TColorGroup *grp = &g;
    while( grp->next != 0 )
        grp = grp->next;

    if( grp->items == 0 )
        grp->items = &i;
    else
        {
        TColorItem *cur = grp->items;
        while( cur->next != 0 )
            cur = cur->next;
        cur->next = &i;
        }
    return g;
}

TColorGroup& operator + ( TColorGroup& g1, TColorGroup& g2 ) noexcept
{
    TColorGroup *cur = &g1;
    while( cur->next != 0 )
        cur = cur->next;
    cur->next = &g2;
    return g1;
}


TColorSelector::TColorSelector( const TRect& bounds, ColorSel aSelType ) noexcept :
    TView( bounds )
{
    options |= ofSelectable | ofFirstClick | ofFramed;
    eventMask |= evBroadcast;
    selType = aSelType;
    color = 0;
}

void TColorSelector::draw()
{
    TDrawBuffer b;
    b.moveChar( 0, ' ', 0x70, size.x );
    for(int i = 0; i <= size.y; i++ )
        {
        if( i < 4 )
            {
            for( int j = 0; j < 4; j++ )
                {
                int c = i*4+j;
                b.moveChar( j*3, icon, c, 3 );
                if( c == color )
                    {
                    b.putChar( j*3+1, 8 );
                    if( c == 0 )
                        b.putAttribute( j*3+1, 0x70 );
                    }
                }
            }
        writeLine( 0, i, size.x, 1, b );
        }
}

void TColorSelector::colorChanged()
{
    int msg;
    if( selType == csForeground )
        msg = cmColorForegroundChanged;
    else
        msg = cmColorBackgroundChanged;
    message( owner, evBroadcast, msg, (void *)(size_t)color );
}

void TColorSelector::handleEvent( TEvent& event )
{
    const int width = 4;

    TView::handleEvent( event );

    uchar oldColor = color;
    int maxCol = (selType == csBackground) ? 7 : 15;
    switch( event.what )
        {

        case evMouseDown:
            do  {
                if( mouseInView( event.mouse.where ) )
                    {
                    TPoint mouse = makeLocal( event.mouse.where );
                    color = mouse.y*4 + mouse.x/3;
                    }
                else
                    color = oldColor;
                colorChanged();
                drawView();
                } while( mouseEvent( event, evMouseMove ) );
            break;

        case evKeyDown:
            switch( ctrlToArrow( event.keyDown.keyCode ) )
                {
                case kbLeft:
                    if( color > 0 )
                        color--;
                    else
                        color = maxCol;
                    break;

                case kbRight:
                    if( color < maxCol )
                        color++;
                    else
                        color = 0;
                    break;

                case kbUp:
                    if( color > width-1 )
                        color -= width;
                    else if( color == 0 )
                        color = maxCol;
                    else
                        color += maxCol - width;
                    break;

                case kbDown:
                    if( color < maxCol - (width-1) )
                        color += width;
                    else if( color == maxCol )
                        color = 0;
                    else
                        color -= maxCol - width;
                    break;

                default:
                    return;
                }
            break;

        case evBroadcast:
            if( event.message.command == cmColorSet )
                {
                if( selType == csBackground )
                    color = event.message.infoByte >> 4;
                else
                    color = event.message.infoByte & 0x0F;
                drawView();
                return;
                }
            else
                return;
        default:
            return;
        }
    drawView();
    colorChanged();
    clearEvent( event );
}

#if !defined(NO_STREAMABLE)

void TColorSelector::write( opstream& os )
{
    TView::write( os );
    os << color << (int)selType;
}

void *TColorSelector::read( ipstream& is )
{
    int temp;
    TView::read( is );
    is >> color >> temp;
    selType = ColorSel(temp);
    return this;
}

TStreamable *TColorSelector::build()
{
    return new TColorSelector( streamableInit );
}

TColorSelector::TColorSelector( StreamableInit ) noexcept : TView( streamableInit )
{
}

#endif

const uchar monoColors[] = { 0x07, 0x0F, 0x01, 0x70, 0x09 };

TMonoSelector::TMonoSelector( const TRect& bounds ) noexcept :
    TCluster( bounds, new TSItem( normal,
                      new TSItem( highlight,
                      new TSItem( underline,
                      new TSItem( inverse,  0 )))))
{
    eventMask |= evBroadcast;
}

void TMonoSelector::draw()
{
    drawBox( button, 0x07 );
}

void TMonoSelector::handleEvent( TEvent& event )
{
    TCluster::handleEvent( event );
    if( event.what == evBroadcast && event.message.command == cmColorSet )
        {
        value = event.message.infoByte;
        drawView();
        }
}

Boolean TMonoSelector::mark( int item )
{
    return Boolean(monoColors[item] == value);
}

void TMonoSelector::newColor()
{
    message( owner, evBroadcast, cmColorForegroundChanged,
        (void *)(size_t)(value & 0x0F) );
    message( owner, evBroadcast, cmColorBackgroundChanged,
        (void *)(size_t)((value >> 4) & 0x0F));
}

void TMonoSelector::press( int item )
{
    value = monoColors[item];
    newColor();
}

void TMonoSelector::movedTo( int item )
{
    value = monoColors[item];
    newColor();
}

#if !defined(NO_STREAMABLE)

TStreamable *TMonoSelector::build()
{
    return new TMonoSelector( streamableInit );
}

TMonoSelector::TMonoSelector( StreamableInit ) noexcept : TCluster( streamableInit )
{
}

#endif


TColorDisplay::TColorDisplay( const TRect& bounds, TStringView aText ) noexcept :
    TView( bounds ),
    color( 0 ),
    text( newStr( aText ) )
{
    eventMask |= evBroadcast;
}

TColorDisplay::~TColorDisplay()
{
    delete[] (char *) text;
}

void TColorDisplay::draw()
{
    uchar c = *color;
    if( c == 0 )
        c = errorAttr;
    const int len = strlen( text );
    TDrawBuffer b;
    for( int i = 0; i <= size.x/len; i++ )
        b.moveStr( i*len, text, c );
    writeLine( 0, 0, size.x, size.y, b );
}

void TColorDisplay::handleEvent( TEvent& event )
{
    TView::handleEvent( event );
    if( event.what == evBroadcast )
        switch( event.message.command )
            {
            case cmColorBackgroundChanged:
                *color = (*color & 0x0F) | ((event.message.infoByte << 4) & 0xF0);
                drawView();
                break;

            case cmColorForegroundChanged:
                *color = (*color & 0xF0) | (event.message.infoByte & 0x0F);
                drawView();
            }
}

void TColorDisplay::setColor( TColorAttr *aColor )
{
    color = aColor;
    message( owner, evBroadcast, cmColorSet, (void *)(size_t)(*color) );
    drawView();
}

#if !defined(NO_STREAMABLE)

void TColorDisplay::write( opstream& os )
{
    TView::write( os );
    os.writeString( text );
}

void *TColorDisplay::read( ipstream& is )
{
    TView::read( is );
    text = is.readString();
    color = 0;
    return this;
}

TStreamable *TColorDisplay::build()
{
    return new TColorDisplay( streamableInit );
}

TColorDisplay::TColorDisplay( StreamableInit ) noexcept : TView( streamableInit )
{
}

#endif

TColorGroupList::TColorGroupList( const TRect& bounds,
                                  TScrollBar *aScrollBar,
                                  TColorGroup *aGroups
                                ) noexcept :
    TListViewer( bounds, 1, 0, aScrollBar ),
    groups( aGroups )
{
    int i = 0;
    while( aGroups != 0 )
        {
        aGroups = aGroups->next;
        i++;
        }
    setRange(i);
}

static void freeItems( TColorItem *curItem ) noexcept
{
    while( curItem != 0 )
        {
        TColorItem *p = curItem;
        curItem = curItem->next;
        delete p;
        }
}

static void freeGroups( TColorGroup *curGroup ) noexcept
{
    while( curGroup != 0 )
        {
        TColorGroup *p = curGroup;
        freeItems( curGroup->items );
        curGroup = curGroup->next;
        delete p;
        }
}

TColorGroupList::~TColorGroupList()
{
    freeGroups( groups );
}

void TColorGroupList::focusItem( short item )
{
    TListViewer::focusItem( item );
    TColorGroup *curGroup = groups;
    while( item-- > 0 )
        curGroup = curGroup->next;
    message( owner, evBroadcast, cmNewColorItem, curGroup);
}

void TColorGroupList::getText( char *dest, short item, short maxChars )
{
    TColorGroup *curGroup = groups;
    while( item-- > 0 )
        curGroup = curGroup->next;
    strncpy( dest, curGroup->name, maxChars );
    dest[maxChars] = '\0';
}

#if !defined(NO_STREAMABLE)

void TColorGroupList::writeItems( opstream& os, TColorItem *items )
{
    int count = 0;
    TColorItem *cur;

    for( cur = items; cur != 0; cur = cur->next )
        count++;

    os << count;

    for( cur = items; cur != 0; cur = cur->next )
        {
        os.writeString( cur->name );
        os << cur->index;
        }
}


void TColorGroupList::writeGroups( opstream& os, TColorGroup *groups )
{
    int count = 0;
    TColorGroup *cur;

    for( cur = groups; cur != 0; cur = cur->next )
        count++;

    os << count;

    for( cur = groups; cur != 0; cur = cur->next )
        {
        os.writeString( cur->name );
        writeItems( os, cur->items );
        }
}
#endif

void TColorGroupList::handleEvent(TEvent& ev)
{
    TListViewer::handleEvent(ev);
    if ((ev.what == evBroadcast) &&
        (ev.message.command == cmSaveColorIndex))
        setGroupIndex(focused, ev.message.infoByte);
}

void TColorGroupList::setGroupIndex(uchar groupNum, uchar itemNum)
{
    TColorGroup* g = getGroup(groupNum);
    if (g)
        g->index = itemNum;
}

uchar TColorGroupList::getGroupIndex(uchar groupNum)
{
    TColorGroup* g = getGroup(groupNum);
    if (g)
        return g->index;
    else
        return 0;
}

TColorGroup* TColorGroupList::getGroup(uchar groupNum)
{
    TColorGroup* g = groups;

    while (groupNum--)
        g = g->next;

    return g;
}

uchar TColorGroupList::getNumGroups()
{
    uchar n;
    TColorGroup* g = groups;

    for (n=0; g; n++)
        g = g->next;

    return n;
}

#if !defined(NO_STREAMABLE)

void TColorGroupList::write( opstream& os )
{
    TListViewer::write( os );
    writeGroups( os, groups );
}

TColorItem *TColorGroupList::readItems( ipstream& is )
{
    int count;
    is >> count;
    TColorItem *items = 0;
    TColorItem **cur = &items;
    while( count-- > 0 )
        {
        char *nm = is.readString();
        uchar index;
        is >> index;
        *cur = new TColorItem( nm, index );
        delete nm;
        cur = &((*cur)->next);
        }
    *cur = 0;
    return items;
}

TColorGroup *TColorGroupList::readGroups( ipstream& is )
{
    int count;
    is >> count;
    TColorGroup *groups = 0;
    TColorGroup **cur = &groups;
    while( count-- > 0 )
        {
        char *nm = is.readString();
        TColorItem *grp = readItems( is );
        *cur = new TColorGroup( nm, grp );
        cur = &((*cur)->next);
        delete nm;
        }
    *cur = 0;
    return groups;
}

void *TColorGroupList::read( ipstream& is )
{
    TListViewer::read( is );
    groups = readGroups( is );
    return this;
}

TStreamable *TColorGroupList::build()
{
    return new TColorGroupList( streamableInit );
}

TColorGroupList::TColorGroupList( StreamableInit ) noexcept :
    TListViewer( streamableInit )
{
}

#endif

TColorItemList::TColorItemList( const TRect& bounds,
                                TScrollBar *aScrollBar,
                                TColorItem *aItems
                              ) noexcept :
    TListViewer( bounds, 1, 0, aScrollBar ),
    items( aItems )
{
    eventMask |= evBroadcast;
    int i = 0;
    while( aItems != 0 )
        {
        aItems = aItems->next;
        i++;
        }
    setRange( i );
}

void TColorItemList::focusItem( short item )
{
    TListViewer::focusItem( item );
    message(owner,evBroadcast, cmSaveColorIndex, (void*)(size_t)item);

    TColorItem *curItem = items;
    while( item-- > 0 )
        curItem = curItem->next;
    message( owner, evBroadcast, cmNewColorIndex, (void *)(size_t)(curItem->index));
}

void TColorItemList::getText( char *dest, short item, short maxChars )
{
    TColorItem *curItem = items;
    while( item-- > 0 )
        curItem = curItem->next;
    strncpy( dest, curItem->name, maxChars );
    dest[maxChars] = '\0';
}

void TColorItemList::handleEvent( TEvent& event )
{
    TListViewer::handleEvent( event );
    if( event.what == evBroadcast )
        {
        TColorGroup* g = (TColorGroup*) event.message.infoPtr;
        TColorItem *curItem;
        int i = 0;

        switch(event.message.command)
            {
            case cmNewColorItem:
                curItem = items = g->items;
                while( curItem != 0 )
                    {
                    curItem = curItem->next;
                    i++;
                    }
                setRange( i );
                focusItem( g->index);
                drawView();
                break;
            default:
                break;
            }
        }
}

#if !defined(NO_STREAMABLE)

TStreamable *TColorItemList::build()
{
    return new TColorItemList( streamableInit );
}

TColorItemList::TColorItemList( StreamableInit ) noexcept :
    TListViewer( streamableInit )
{
}

#endif

TColorDialog::TColorDialog( TPalette *aPalette, TColorGroup *aGroups ) noexcept :
    TWindowInit( &TColorDialog::initFrame ),
    TDialog( TRect( 0, 0, 61, 18 ), colors )
{
    options |= ofCentered;
    if( aPalette != 0 )
        {
        pal = new TPalette( "", 0 );
        *pal = *aPalette;
        }
    else
        pal = 0;

    TScrollBar *sb = new TScrollBar( TRect( 18, 3, 19, 14 ) );
    insert( sb );

    groups = new TColorGroupList( TRect( 3, 3, 18, 14 ), sb, aGroups);
    insert( groups );
    insert( new TLabel( TRect( 2, 2, 8, 3 ), groupText, groups ) );

    sb = new TScrollBar( TRect( 41, 3, 42, 14 ) );
    insert( sb );

    TView *p = new TColorItemList( TRect( 21, 3, 41, 14 ), sb, aGroups->items );
    insert( p );
    insert( new TLabel( TRect( 20, 2, 25, 3 ), itemText, p ) );

    forSel = new TColorSelector( TRect( 45, 3, 57, 7 ),
                                 TColorSelector::csForeground );
    insert( forSel );
    forLabel = new TLabel( TRect( 45, 2, 57, 3 ), forText, forSel );
    insert( forLabel );

    bakSel = new TColorSelector( TRect( 45, 9, 57, 11 ),
                                 TColorSelector::csBackground );
    insert( bakSel );
    bakLabel = new TLabel( TRect( 45, 8, 57, 9 ), bakText, bakSel );
    insert( bakLabel );

    display = new TColorDisplay( TRect( 44, 12, 58, 14 ), textText );
    insert( display );

    monoSel = new TMonoSelector( TRect( 44, 3, 59, 7 ) );
    monoSel->hide();
    insert( monoSel );
    monoLabel = new TLabel( TRect( 43, 2, 49, 3 ), colorText, monoSel );
    monoLabel->hide();
    insert( monoLabel );

    insert( new TButton( TRect( 36, 15, 46, 17 ), okText, cmOK, bfDefault ) );
    insert( new TButton( TRect( 48, 15, 58, 17 ), cancelText, cmCancel, bfNormal ) );
    selectNext( False );

    if( pal != 0 )
        setData( pal );
}

TColorDialog::~TColorDialog()
{
    delete pal;
}

void TColorDialog::handleEvent( TEvent& event )
{
    if( event.what==evBroadcast && event.message.command==cmNewColorItem )
        groupIndex = groups->focused;
    TDialog::handleEvent( event );
    if( event.what==evBroadcast && event.message.command==cmNewColorIndex )
        display->setColor( &pal->data[event.message.infoByte] );
}

ushort TColorDialog::dataSize()
{
    return *pal->data + 1;
}

void TColorDialog::getData( void *rec )
{
    getIndexes(colorIndexes);
    *(TPalette *) rec = *pal;
}

void TColorDialog::setData( void *rec)
{
    if( !pal )
        pal = new TPalette("", 0);
    *pal = *(TPalette *) rec;

    setIndexes(colorIndexes);
    display->setColor(&pal->data[groups->getGroupIndex(groupIndex)]);
    groups->focusItem( groupIndex);
    if( showMarkers )
        {
        forLabel->hide();
        forSel->hide();
        bakLabel->hide();
        bakSel->hide();
        monoLabel->show();
        monoSel->show();
        }
    groups->select();
}

void TColorDialog::setIndexes(TColorIndex*& colIdx)
{
    uchar numGroups, index;

    numGroups = groups->getNumGroups();
    if (colIdx && (colIdx->colorSize != numGroups))
    {
        delete[] colIdx;
        colIdx = 0;
    }
    if (!colIdx)
    {
        colIdx = (TColorIndex*) new uchar[numGroups+2];
        colIdx->groupIndex = 0;
        memset(colIdx->colorIndex, 0, numGroups);
        colIdx->colorSize = numGroups;
    }
    for (index = 0; index < numGroups; index++)
        groups->setGroupIndex(index, colIdx->colorIndex[index]);

    groupIndex = colIdx->groupIndex;
}

void TColorDialog::getIndexes(TColorIndex*& colIdx)
{
    uchar n = groups->getNumGroups();
    if (!colIdx)
    {
        colIdx = (TColorIndex*) new uchar[n+2];
        memset(colIdx->colorIndex, 0, n);
        colIdx->colorSize = n;
    }
    colIdx->groupIndex = groupIndex;
    for (uchar index=0; index < n; index++)
        colIdx->colorIndex[index] = groups->getGroupIndex(index);
}

#if !defined(NO_STREAMABLE)

void TColorDialog::write( opstream& os )
{
    TDialog::write( os );
    os << display << groups << forLabel << forSel
       << bakLabel << bakSel << monoLabel << monoSel;
}

void *TColorDialog::read( ipstream& is )
{
    TDialog::read( is );
    is >> display >> groups >> forLabel >> forSel
       >> bakLabel >> bakSel >> monoLabel >> monoSel;
    pal = 0;
    return this;
}

TStreamable *TColorDialog::build()
{
    return new TColorDialog( streamableInit );
}

TColorDialog::TColorDialog( StreamableInit ) noexcept :
    TWindowInit( 0 ),
    TDialog( streamableInit )
{
}


#endif

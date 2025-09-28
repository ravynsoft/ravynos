/*------------------------------------------------------------*/
/* filename -       tlstview.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TListViewer member functions              */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TKeys
#define Uses_TListViewer
#define Uses_TScrollBar
#define Uses_TDrawBuffer
#define Uses_TPoint
#define Uses_TEvent
#define Uses_TGroup
#define Uses_opstream
#define Uses_ipstream
#include <tvision/tv.h>

#if !defined( __MEM_H )
#include <mem.h>
#endif  // __MEM_H

#define cpListViewer "\x1A\x1A\x1B\x1C\x1D"

TListViewer::TListViewer( const TRect& bounds,
                          ushort aNumCols,
                          TScrollBar *aHScrollBar,
                          TScrollBar *aVScrollBar) noexcept :
    TView( bounds ),
    numCols( aNumCols ),
    topItem( 0 ),
    focused( 0 ),
    range( 0 )
{
    short arStep, pgStep;

    options |= ofFirstClick | ofSelectable;
    eventMask |= evBroadcast;
    if( aVScrollBar != 0 )
        {
        if( numCols == 1 )
            {
            pgStep = size.y - 1;
            arStep = 1;
            }
        else
            {
            pgStep = size.y * numCols;
            arStep = size.y;
            }
        aVScrollBar->setStep( pgStep, arStep );
        }

    if( aHScrollBar != 0 )
        aHScrollBar->setStep( size.x / numCols, 1 );

    hScrollBar = aHScrollBar;
    vScrollBar = aVScrollBar;
}

void TListViewer::changeBounds( const TRect& bounds )
{
    TView::changeBounds( bounds );
    if( hScrollBar != 0 )
        hScrollBar->setStep( size.x / numCols, hScrollBar->arStep);
    if( vScrollBar != 0 )
        vScrollBar->setStep( size.y, vScrollBar->arStep);
}

void TListViewer::draw()
{
    short i, j, item;
    TColorAttr normalColor, selectedColor, focusedColor, color;
    short colWidth, curCol, indent;
    TDrawBuffer b;
    uchar scOff;
    Boolean focusedVis;

    if( (state&(sfSelected | sfActive)) == (sfSelected | sfActive))
        {
        normalColor = getColor(1);
        focusedColor = getColor(3);
        selectedColor = getColor(4);
        }
    else
        {
        normalColor = getColor(2);
        selectedColor = getColor(4);
        focusedColor = 0; // Unused, but silence warning.
        }

    if( hScrollBar != 0 )
        indent = hScrollBar->value;
    else
        indent = 0;

    focusedVis = False;
    colWidth = size.x / numCols + 1;
    for( i = 0; i < size.y; i++ )
        {
        for( j = 0; j < numCols; j++ )
            {
            item =  j * size.y + i + topItem;
            curCol = j * colWidth;
            if( (state & (sfSelected | sfActive)) == (sfSelected | sfActive) &&
                focused == item &&
                range > 0)
                {
                color = focusedColor;
                setCursor( curCol + 1, i );
                scOff = 0;
                focusedVis = True;
                }
            else if( item < range && isSelected(item) )
                {
                color = selectedColor;
                scOff = 2;
                }
            else
                {
                color = normalColor;
                scOff = 4;
                }

            b.moveChar( curCol, ' ', color, colWidth );
            if( item < range )
                {
                if (indent < 255)
                    {
                    char text[256];
                    getText( text, item, 255 );
                    b.moveStr( curCol+1, text, color, colWidth, indent );
                    }
                if( showMarkers )
                    {
                    b.putChar( curCol, specialChars[scOff] );
                    b.putChar( curCol+colWidth-2, specialChars[scOff+1] );
                    }
                }
            else if( i == 0 && j == 0 )
                b.moveStr( curCol+1, emptyText, getColor(1) );

            b.moveChar( curCol+colWidth-1, '\xB3', getColor(5), 1 );
            }
        writeLine( 0, i, size.x, 1, b );
        }

    if ( !focusedVis )
        setCursor( -1, -1 );
}

void TListViewer::focusItem( short item )
{
    focused = item;
    if( vScrollBar != 0 )
        vScrollBar->setValue( item );
    else
        drawView();
    if( size.y > 0 )
        {
        if( item < topItem )
            {
            if( numCols == 1 )
                topItem = item;
            else
                topItem = item - item % size.y;
            }
        else if( item >= topItem + size.y*numCols )
            {
            if( numCols == 1 )
                topItem = item - size.y + 1;
            else
                topItem = item - item % size.y - (size.y * (numCols-1));
            }
        }
}

void TListViewer::focusItemNum( short item )
{
    if( item < 0 )
        item = 0;
    else
        if( item >= range && range > 0 )
            item = range - 1;

    if( range !=  0 )
        focusItem( item );
}

TPalette& TListViewer::getPalette() const
{
    static TPalette palette( cpListViewer, sizeof( cpListViewer )-1 );
    return palette;
}

void TListViewer::getText( char *dest, short, short )
{
    *dest = EOS;
}

Boolean TListViewer::isSelected( short item )
{
    return Boolean( item == focused );
}

void TListViewer::handleEvent( TEvent& event )
{
    TPoint mouse;
    ushort colWidth;
    short  oldItem, newItem = 0;
    ushort count;
    int mouseAutosToSkip = 4;

    TView::handleEvent(event);

    if( event.what == evMouseDown )
        {
        colWidth = size.x / numCols + 1;
        oldItem =  focused;
        count = 0;
        do  {
            mouse = makeLocal( event.mouse.where );
            if( mouseInView( event.mouse.where ) )
                newItem = mouse.y + (size.y * (mouse.x / colWidth)) + topItem;
            else
                {
                if( numCols == 1 )
                    {
                    if( event.what == evMouseAuto )
                        count++;
                    if( count == mouseAutosToSkip )
                        {
                        count = 0;
                        if( mouse.y < 0 )
                            newItem = focused - 1;
                        else if( mouse.y >= size.y )
                            newItem = focused + 1;
                        }
                    }
                else
                    {
                    if( event.what == evMouseAuto )
                        count++;
                    if( count == mouseAutosToSkip )
                        {
                        count = 0;
                        if( mouse.x < 0 )
                            newItem = focused - size.y;
                        else if( mouse.x >= size.x )
                            newItem = focused + size.y;
                        else if( mouse.y < 0 )
                            newItem = focused - focused % size.y;
                        else if( mouse.y > size.y )
                            newItem = focused - focused % size.y + size.y - 1;
                        }
                    }
                }
            if( newItem != oldItem )
                {
                focusItemNum( newItem );
                drawView();
                }
            oldItem = newItem;
            if( event.mouse.eventFlags & meDoubleClick )
                break;
            } while( mouseEvent( event, evMouseMove | evMouseAuto ) );
        focusItemNum( newItem );
        drawView();
        if( (event.mouse.eventFlags & meDoubleClick) && range > newItem )
            selectItem( newItem );
        clearEvent( event );
        }
    else if( event.what == evKeyDown )
        {
        if (event.keyDown.charScan.charCode ==  ' ' && focused < range )
            {
            selectItem( focused );
            newItem = focused;
            }
        else
            {
            switch (ctrlToArrow(event.keyDown.keyCode))
                {
                case kbUp:
                    newItem = focused - 1;
                    break;
                case kbDown:
                    newItem = focused + 1;
                    break;
                case kbRight:
                    if( numCols > 1 )
                        newItem = focused + size.y;
                    else
                        return;
                    break;
                case kbLeft:
                    if( numCols > 1 )
                        newItem = focused - size.y;
                    else
                        return;
                    break;
                case kbPgDn:
                    newItem = focused + size.y * numCols;
                    break;
                case  kbPgUp:
                    newItem = focused - size.y * numCols;
                    break;
                case kbHome:
                    newItem = topItem;
                    break;
                case kbEnd:
                    newItem = topItem + (size.y * numCols) - 1;
                    break;
                case kbCtrlPgDn:
                    newItem = range - 1;
                    break;
                case kbCtrlPgUp:
                    newItem = 0;
                    break;
                default:
                    return;
                }
            }
        focusItemNum(newItem);
        drawView();
        clearEvent(event);
        }
    else if( event.what == evBroadcast )
        {
        if( (options & ofSelectable) != 0 )
            {
            if( event.message.command == cmScrollBarClicked &&
                ( event.message.infoPtr == hScrollBar ||
                  event.message.infoPtr == vScrollBar ))
                select();
            else if( event.message.command == cmScrollBarChanged )
                {
                if( vScrollBar == event.message.infoPtr )
                    {
                    focusItemNum( vScrollBar->value );
                    drawView();
                    }
                else if( hScrollBar == event.message.infoPtr )
                    drawView();
                }
            }
        }
}

void TListViewer::selectItem( short )
{
    message( owner, evBroadcast, cmListItemSelected, this );
}

void TListViewer::setRange( short aRange )
{
    range = aRange;
    if( focused >= aRange )
        focused = 0;
    if( vScrollBar != 0 )
        vScrollBar->setParams( focused, 0, aRange - 1, vScrollBar->pgStep,
                               vScrollBar->arStep );
    else
        drawView();
}

void TListViewer::setState( ushort aState, Boolean enable )
{
    TView::setState( aState, enable );
    if( (aState & (sfSelected | sfActive | sfVisible)) != 0 )
        {
        if( hScrollBar != 0 )
            {
            if( getState(sfActive) && getState(sfVisible))
                hScrollBar->show();
            else
                hScrollBar->hide();
            }
        if( vScrollBar != 0 )
            {
            if( getState(sfActive) && getState(sfVisible))
                vScrollBar->show();
            else
                vScrollBar->hide();
            }
        drawView();
        }
}

void TListViewer::shutDown()
{
     hScrollBar = 0;
     vScrollBar = 0;
     TView::shutDown();
}

#if !defined(NO_STREAMABLE)

void TListViewer::write( opstream& os )
{
    TView::write( os );
    os << hScrollBar << vScrollBar << numCols
       << topItem << focused << range;
}

void *TListViewer::read( ipstream& is )
{
    TView::read( is );
    is >> hScrollBar >> vScrollBar >> numCols
       >> topItem >> focused >> range;
    return this;
}

TStreamable *TListViewer::build()
{
    return new TListViewer( streamableInit );
}

TListViewer::TListViewer( StreamableInit ) noexcept : TView( streamableInit )
{
}


#endif

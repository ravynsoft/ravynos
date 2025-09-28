/*------------------------------------------------------------*/
/* filename -       tstatusl.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TStatusLine member functions              */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TStatusLine
#define Uses_TStatusItem
#define Uses_TStatusDef
#define Uses_TDrawBuffer
#define Uses_TEvent
#define Uses_TKeys
#define Uses_opstream
#define Uses_ipstream
#include <tvision/tv.h>

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#define cpStatusLine "\x02\x03\x04\x05\x06\x07"

TStatusLine::TStatusLine( const TRect& bounds, TStatusDef& aDefs ) noexcept :
    TView( bounds ),
    defs( &aDefs )
{
    options |= ofPreProcess;
    eventMask |= evBroadcast;
    growMode = gfGrowLoY | gfGrowHiX | gfGrowHiY;
    findItems();
}

void TStatusLine::disposeItems( TStatusItem *item )
{
    while( item != 0 )
        {
        TStatusItem *T = item;
        item = item->next;
        delete T;
        }
}

TStatusLine::~TStatusLine(void)
{
    while( defs != 0 )
        {
        TStatusDef *T = defs;
        defs = defs->next;
        disposeItems( T->items );
        delete T;
        }
}

void TStatusLine::draw()
{
    drawSelect( 0 );
}

void TStatusLine::drawSelect( TStatusItem *selected )
{
    TDrawBuffer b;
    TAttrPair color;

    TAttrPair cNormal = getColor(0x0301);
    TAttrPair cSelect = getColor(0x0604);
    TAttrPair cNormDisabled = getColor(0x0202);
    TAttrPair cSelDisabled = getColor(0x0505);
    b.moveChar( 0, ' ', cNormal, size.x );
    TStatusItem *T =  items;
    ushort i = 0;

    while( T != 0 )
        {
        if( T->text != 0 )
            {
            ushort l = cstrlen( T->text );
            if( i + l < size.x )
                {
                if( commandEnabled( T->command) )
                    if( T == selected )
                        color = cSelect;
                    else
                        color = cNormal;
                else
                    if( T == selected )
                        color = cSelDisabled;
                    else
                        color = cNormDisabled;

                b.moveChar( i, ' ', color, 1 );
                b.moveCStr( i+1, T->text, color );
                b.moveChar( i+l+1, ' ', color, 1 );
                }
            i += l+2;
            }
        T = T->next;
        }
    if( i < size.x - 2 )
        {
        TStringView hintText = hint( helpCtx );
        if( hintText.size() )
            {
            b.moveStr( i, hintSeparator, cNormal );
            i += 2;
            b.moveStr( i, hintText, cNormal, size.x-i );
            }
        }
    writeLine( 0, 0, size.x, 1, b );
}

void TStatusLine::findItems() noexcept
{
    TStatusDef *p = defs;
    while( p != 0 && ( helpCtx < p->min || helpCtx > p->max ) )
        p = p->next;
    items = ( p == 0 ) ? 0 : p->items;
}

TPalette& TStatusLine::getPalette() const
{
    static TPalette palette( cpStatusLine, sizeof( cpStatusLine )-1 );
    return palette;
}

TStatusItem *TStatusLine::itemMouseIsIn( TPoint mouse )
{
    if( mouse.y !=  0 )
        return 0;

    ushort i;
    TStatusItem *T;

    for( i = 0, T = items; T != 0; T = T->next)
        {
        if( T->text != 0 )
            {
            ushort k = i + cstrlen(T->text) + 2;
            if( mouse.x >= i && mouse. x < k )
                return T;
            i = k;
            }
        }
    return 0;
}

void TStatusLine::handleEvent( TEvent& event )
{
    TView::handleEvent(event);

    switch (event.what)
        {
        case  evMouseDown:
            {
            TStatusItem *T = 0;

            do  {
                TPoint mouse = makeLocal( event.mouse.where );
                if( T != itemMouseIsIn(mouse) )
                    drawSelect( T = itemMouseIsIn(mouse) );
                } while( mouseEvent( event, evMouseMove ) );

            if( T != 0 && commandEnabled(T->command) )
                {
                event.what = evCommand;
                event.message.command = T->command;
                event.message.infoPtr = 0;
                putEvent(event);
                }
            clearEvent(event);
            drawView();
            break;
            }
        case evKeyDown:
            if( event.keyDown.keyCode != kbNoKey )
                {
                for( TStatusItem *T = items; T != 0; T = T->next )
                    {
                    if( TKey(event.keyDown) == T->keyCode &&
                        commandEnabled(T->command) )
                        {
                        event.what = evCommand;
                        event.message.command = T->command;
                        event.message.infoPtr = 0;
                        return;
                        }
                    }
                }
            break;
        case evBroadcast:
            if( event.message.command == cmCommandSetChanged )
                drawView();
            break;
        }
}

const char* TStatusLine::hint( ushort )
{
    return "";
}

void TStatusLine::update()
{
    TView *p = TopView();
    ushort h = ( p != 0 ) ? p->getHelpCtx() : hcNoContext;
    if( helpCtx != h )
        {
        helpCtx = h;
        findItems();
        drawView();
        }
}

#if !defined(NO_STREAMABLE)

void TStatusLine::writeItems( opstream& os, TStatusItem *ts )
{
    int count = 0;
    for( TStatusItem *t = ts; t != 0; t = t->next )
        count++;
    os << count;
    for( ; ts != 0; ts = ts->next )
        {
        os.writeString( ts->text );
        os << ts->keyCode << ts->command;
        }
}

void TStatusLine::writeDefs( opstream& os, TStatusDef *td )
{
    int count = 0;
    for( TStatusDef *t = td; t != 0; t = t->next )
        count++;
    os << count;
    for( ; td != 0; td = td->next )
        {
        os << td->min << td->max;
        writeItems( os, td->items );
        }
}

void TStatusLine::write( opstream& os )
{
    TView::write( os );
    writeDefs( os, defs );
}

TStatusItem *TStatusLine::readItems( ipstream& is )
{
    TStatusItem *cur;
    TStatusItem *first;
    TStatusItem **last = &first;
    int count;
    is >> count;
    while( count-- > 0 )
        {
        char *t = is.readString();
        ushort key, cmd;
        is >> key >> cmd;
        cur = new TStatusItem( t, key, cmd );
        *last = cur;
        last = &(cur->next);
        delete t;
        }
    *last = 0;
    return first;
}

TStatusDef *TStatusLine::readDefs( ipstream& is )
{
    TStatusDef *cur;
    TStatusDef *first;
    TStatusDef **last = &first;
    int count;
    is >> count;
    while( count-- > 0 )
        {
        ushort min, max;
        is >> min >> max;
        cur = new TStatusDef( min, max, readItems( is ) );
        *last = cur;
        last = &(cur->next);
        }
    *last = 0;
    return first;
}


void *TStatusLine::read( ipstream& is )
{
    TView::read( is );
    defs = readDefs( is );
    findItems();
    return this;
}

TStreamable *TStatusLine::build()
{
    return new TStatusLine( streamableInit );
}

TStatusLine::TStatusLine( StreamableInit ) noexcept : TView( streamableInit )
{
}


#endif

/*------------------------------------------------------------*/
/* filename -       tgroup.cpp                                */
/*                                                            */
/* function(s)                                                */
/*                  TGroup member functions                   */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TGroup
#define Uses_TView
#define Uses_TRect
#define Uses_TEvent
#define Uses_opstream
#define Uses_ipstream
#define Uses_TVMemMgr
#include <tvision/tv.h>

TView *TheTopView = 0;
TGroup* ownerGroup = 0;

TGroup::TGroup( const TRect& bounds ) noexcept :
    TView(bounds), current( 0 ), last( 0 ), phase( phFocused ), buffer( 0 ),
    lockFlag( 0 ), endState( 0 )
{
    options |= ofSelectable | ofBuffered;
    clip = getExtent();
    eventMask = 0xFFFF;
}

TGroup::~TGroup()
{
}

void TGroup::shutDown()
{
    TView* p = last;
    if( p != 0 )
    {
        do {
        p->hide();
        p = p->prev();
    } while (p != last);

        do  {
            TView* T = p->prev();
            destroy( p );
            p = T;
        } while( last != 0 );
    }
    freeBuffer();
    current = 0;
    TView::shutDown();
}

void doCalcChange( TView *p, void *d )
{
    TRect  r;
    p->calcBounds(r, *(TPoint*)d);
    p->changeBounds(r);
}

#pragma warn -par
static void doAwaken (TView* v, void* p)
{
    v->awaken();
}

void TGroup::awaken()
{
    forEach(doAwaken, 0);
}

void TGroup::changeBounds( const TRect& bounds )
{
    TPoint d;

    d.x = (bounds.b.x - bounds.a.x) - size.x;
    d.y = (bounds.b.y - bounds.a.y) - size.y;
    if( d.x == 0 && d.y == 0 )
        {
        setBounds(bounds);
        drawView();
        }
    else
        {
        setBounds( bounds );
        clip = getExtent();
        getBuffer();
        lock();
        forEach( doCalcChange, &d );
        unlock();
        }
}

void addSubviewDataSize( TView *p, void *T )
{
   *((ushort *)T) += p->dataSize();
}

ushort TGroup::dataSize()
{
    ushort T = 0;
    forEach( addSubviewDataSize, &T );
    return T;
}

void TGroup::remove(TView* p)
{
    if( p )
        {
        ushort saveState;
        saveState = p->state;
        p->hide();
        removeView(p);
        p->owner = 0;
        p->next= 0;
        if( (saveState & sfVisible) != 0 )
            p->show();
        }
}


void TGroup::draw()
{
    if( buffer == 0 )
        {
        getBuffer();
        if( buffer != 0 )
            {
            lockFlag++;
            redraw();
            lockFlag--;
            }
        }
    if( buffer != 0 )
        writeBuf( 0, 0, size.x, size.y, buffer );
    else
        {
        clip = getClipRect();
        redraw();
        clip = getExtent();
        }
}

void TGroup::drawSubViews( TView* p, TView* bottom ) noexcept
{
    while( p != bottom )
        {
        p->drawView();
        p = p->nextView();
        }
}

void TGroup::endModal( ushort command )
{
    if( (state & sfModal) != 0 )
        endState = command;
    else
        TView::endModal( command );
}

void TGroup::eventError( TEvent& event )
{
    if (owner != 0 )
        owner->eventError( event );
}

ushort TGroup::execute()
{
    do  {
        endState = 0;
        do  {
            TEvent e;
            getEvent( e );
            handleEvent( e );
            if( e.what != evNothing )
                eventError( e );
            } while( endState == 0 );
    } while( !valid(endState) );
    return endState;
}

ushort TGroup::execView( TView* p ) noexcept
{
    if( p == 0 )
        return cmCancel;

    ushort saveOptions = p->options;
    TGroup *saveOwner = p->owner;
    TView *saveTopView = TheTopView;
    TView *saveCurrent= current;
    TCommandSet saveCommands;
    getCommands( saveCommands );
    TheTopView = p;
    p->options = p->options & ~ofSelectable;
    p->setState(sfModal, True);
    setCurrent(p, enterSelect);
    if( saveOwner == 0 )
        insert(p);
    ushort retval = p->execute();
    if( saveOwner == 0 )
        remove(p);
    setCurrent(saveCurrent, leaveSelect);
    p->setState(sfModal, False);
    p->options = saveOptions;
    TheTopView = saveTopView;
    setCommands(saveCommands);
    return retval;
}

TView *TGroup::first() noexcept
{
    if( last == 0 )
        return 0;
    else
        return last->next;
}

TView* TGroup::findNext(Boolean forwards) noexcept
{
    TView* p, *result;

    result = 0;
    if( current )
        {
        p = current;
        do {
            if( forwards )
                p = p->next;
            else
                p = p->prev();

            } while( !( (((p->state & (sfVisible | sfDisabled)) == sfVisible) &&
                        (p->options & ofSelectable)) || (p == current) ) );

        if( p != current )
        result = p;
        }
    return result;
}

Boolean TGroup::focusNext(Boolean forwards)
{
    TView* p;

    p = findNext(forwards);
    if (p)
        return p->focus();
    else
        return True;
}

TView *TGroup::firstMatch( ushort aState, ushort aOptions ) noexcept
{
    if( last == 0 )
        return 0;

    TView* temp = last;
    while(1)
        {
        if( ((temp->state & aState) == aState) &&
            ((temp->options & aOptions) ==  aOptions))
            return temp;

        temp = temp->next;
        if( temp == last )
            return 0;
        }
}

void TGroup::freeBuffer() noexcept
{
    if( (options & ofBuffered) != 0 && buffer != 0 )
        {
        TVMemMgr::freeDiscardable( buffer );
        buffer = 0;
        }
}

void TGroup::getBuffer() noexcept
{
    if( (state & sfExposed) != 0 )
        if( (options & ofBuffered) != 0 )
            {
            int sz = max(size.x * size.y * sizeof(TScreenCell), 0);
            TVMemMgr::reallocateDiscardable( (void *&)buffer, sz );
#ifndef __BORLANDC__
            // An uninitialized screen buffer is harmless in MS-DOS, since the worst
            // you will see are random characters and colors. But in non-Borland mode,
            // it may result in control characters being printed to screen, which will
            // severely mess up the display. Do not allow this to happen.
            if( buffer != 0 )
                memset(buffer, 0, sz);
#endif
            }
}

void TGroup::getData(void *rec)
{
    ushort i = 0;
    if (last != 0 )
        {
        TView* v = last;
        do  {
            v->getData( ((char *)rec) + i );
            i += v->dataSize();
            v = v->prev();
            } while( v != last );
        }
}

struct handleStruct
{
    handleStruct( TEvent& e, TGroup& g ) : event( e ), grp( g ) {}
    TEvent& event;
    TGroup& grp;
};

static void doHandleEvent( TView *p, void *s )
{
    handleStruct *ptr = (handleStruct *)s;

    if( p == 0 ||
        ( (p->state & sfDisabled) != 0 &&
          (ptr->event.what & (positionalEvents | focusedEvents)) != 0
        )
      )
        return;

    switch( ptr->grp.phase )
        {
        case TView::phFocused:
            break;
        case TView::phPreProcess:
            if( (p->options & ofPreProcess) == 0 )
                return;
            break;
        case TView::phPostProcess:
            if( (p->options & ofPostProcess) == 0 )
                return;
            break;
        }
    if( (ptr->event.what & p->eventMask) != 0 )
        p->handleEvent( ptr->event );
}

static Boolean hasMouse( TView *p, void *s )
{
    return p->containsMouse( *(TEvent *)s );
}

void TGroup::handleEvent( TEvent& event )
{
    TView::handleEvent( event );

    handleStruct hs( event, *this );

    if( (event.what & focusedEvents) != 0 )
        {
        phase = phPreProcess;
        forEach( doHandleEvent, &hs );

        phase = phFocused;
        doHandleEvent( current, &hs );

        phase = phPostProcess;
        forEach( doHandleEvent, &hs );
        }
    else if( event.what )
        {
        phase = phFocused;
        if( (event.what & positionalEvents) != 0 )
            {
            doHandleEvent( firstThat( hasMouse, &event ), &hs );
            }
        else
            forEach( doHandleEvent, &hs );
        }
}

void TGroup::insert( TView* p ) noexcept
{
    insertBefore( p, first() );
}

void TGroup::insertBefore( TView *p, TView *Target )
{
    if( p != 0 && p->owner == 0 && (Target == 0 || Target->owner == this) )
        {
        if( (p->options & ofCenterX) != 0 )
            p->origin.x = (size.x - p->size.x)/2;
        if( (p->options & ofCenterY) != 0 )
            p->origin.y = (size.y - p->size.y)/2;
        ushort saveState = p->state;
        p->hide();
        insertView( p, Target );
        if( (saveState & sfVisible) != 0 )
            p->show();
        if( (saveState & sfActive) != 0 )
        p->setState(sfActive, True);
        }
}

void TGroup::insertView( TView* p, TView* Target ) noexcept
{
    p->owner = this;
    if( Target != 0 )
        {
        Target = Target->prev();
        p->next = Target->next;
        Target->next= p;
        }
    else
        {
        if( last== 0 )
            p->next = p;
        else
            {
            p->next = last->next;
            last->next = p;
            }
        last = p;
        }
}

void TGroup::lock() noexcept
{
    if( buffer != 0 || lockFlag != 0 )
        lockFlag++;
}

void TGroup::redraw() noexcept
{
    drawSubViews( first(), 0 );
}

void TGroup::resetCurrent()
{
    setCurrent( firstMatch( sfVisible, ofSelectable ), normalSelect );
}

void TGroup::resetCursor()
{
    if( current != 0 )
        current->resetCursor();
}

void TGroup::selectNext( Boolean forwards )
{
    if( current != 0 )
    {
        TView* p = findNext(forwards);
    if (p) p->select();
    }
}

void TGroup::selectView( TView* p, Boolean enable )
{
    if( p != 0 )
        p->setState( sfSelected, enable );
}

void TGroup::focusView( TView* p, Boolean enable )
{
    if( (state & sfFocused) != 0 && p != 0 )
        p->setState( sfFocused, enable );
}



void TGroup::setCurrent( TView* p, selectMode mode )
{
    if (current!= p)
        {
        lock();
        focusView( current, False );
        if( mode != enterSelect )
            if( current != 0 )
                current->setState( sfSelected, False );
        if( mode != leaveSelect )
            if( p != 0 )
                p->setState( sfSelected, True );
        if( (state & sfFocused) != 0 && p != 0 )
            p->setState( sfFocused, True );
        current = p;
        unlock();
        }
}

void TGroup::setData(void *rec)
{
    ushort i = 0;
    if( last!= 0 )
        {
        TView* v = last;
        do  {
            v->setData( (char *)rec + i );
            i += v->dataSize();
            v = v->prev();
            } while (v != last);
        }
}

static void doExpose( TView *p, void *enable )
{
    if( (p->state & sfVisible) != 0 )
        p->setState( sfExposed, *(Boolean *)enable );
}

struct setBlock
{
    ushort st;
    Boolean en;
};

static void doSetState( TView *p, void *b )
{
    p->setState( ((setBlock *)b)->st, ((setBlock *)b)->en );
}

void TGroup::setState( ushort aState, Boolean enable )
{
    setBlock sb;
    sb.st = aState;
    sb.en = enable;

    TView::setState( aState, enable );

    if( (aState & (sfActive | sfDragging)) != 0 )
        {
        lock();
        forEach( doSetState, &sb );
        unlock();
        }

    if( (aState & sfFocused) != 0 )
        {
        if( current != 0 )
            current->setState( sfFocused, enable );
        }

    if( (aState & sfExposed) != 0 )
        {
        forEach( doExpose, &enable );
        if( enable == False )
            freeBuffer();
        }
}

void TGroup::unlock() noexcept
{
    if( lockFlag != 0 && --lockFlag == 0 )
        drawView();
}

Boolean isInvalid( TView *p, void *command )
{
    return Boolean( !p->valid( *(ushort *) command ) );
}

Boolean TGroup::valid( ushort command )
{
    if (command == cmReleasedFocus)
    {
        if (current && (current->options & ofValidate))
            return current->valid(command);
        else
            return True;
    }

    return Boolean( firstThat( isInvalid, &command ) == 0 );
}

ushort TGroup::getHelpCtx()
{
    ushort h = hcNoContext;
    if( current!= 0 )
        h = current->getHelpCtx();
    if (h == hcNoContext)
        h = TView::getHelpCtx();
    return h;
}

#if !defined(NO_STREAMABLE)

static void doPut( TView *p, void *osp )
{
    *(opstream *)osp << p;
}

void TGroup::write( opstream& os )
{
    ushort index;

    TView::write( os );
    TGroup *ownerSave = owner;
    owner = this;
    int count = indexOf( last );
    os << count;
    forEach( doPut, &os );
    if (current == 0)
       index = 0;
    else
       index = indexOf(current);
    os << index;
    owner = ownerSave;
}

void *TGroup::read( ipstream& is )
{
    ushort index;

    TView::read( is );
    clip = getExtent();
    TGroup *ownerSave = owner;
    owner = this;
    last = 0;
    phase = TView::phFocused;
    current = 0;
    buffer = 0;
    lockFlag = 0;
    endState = 0;
    int count;
    is >> count;
    TView *tv;
    for( int i = 0; i < count; i++ )
        {
        is >> tv;
        if( tv != 0 )
            insertView( tv, 0 );
        }
    owner = ownerSave;
    TView *current;
    is >> index;
    current = at(index);
    setCurrent( current, TView::normalSelect );
    if (ownerGroup == NULL)
        awaken();
    return this;
}

TStreamable *TGroup::build()
{
    return new TGroup( streamableInit );
}

TGroup::TGroup( StreamableInit ) noexcept : TView( streamableInit )
{
}

#endif // NO_STREAMABLE

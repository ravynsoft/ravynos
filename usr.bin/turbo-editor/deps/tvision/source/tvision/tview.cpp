/*------------------------------------------------------------*/
/* filename -       tview.cpp                                 */
/*                                                            */
/* function(s)                                                */
/*                  TView member functions                    */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TKeys
#define Uses_TView
#define Uses_TCommandSet
#define Uses_TPoint
#define Uses_TGroup
#define Uses_TRect
#define Uses_TEvent
#define Uses_TProgram
#define Uses_opstream
#define Uses_ipstream
#include <tvision/tv.h>

#if !defined( __DOS_H )
#include <dos.h>
#endif  // __DOS_H

#if !defined( __LIMITS_H )
#include <limits.h>
#endif  // __LIMITS_H

TPoint shadowSize = {2,1};
uchar shadowAttr = 0x08;
Boolean _NEAR TView::showMarkers = False;
uchar _NEAR TView::errorAttr = 0xCF;
Boolean _NEAR TView::commandSetChanged = False;

extern TView *TheTopView;

static TCommandSet initCommands() noexcept
{
    TCommandSet temp;
    for( int i = 0; i < 256; i++ )
        temp.enableCmd( i );
    temp.disableCmd( cmZoom );
    temp.disableCmd( cmClose );
    temp.disableCmd( cmResize );
    temp.disableCmd( cmNext );
    temp.disableCmd( cmPrev );
    return temp;
}

TCommandSet _NEAR TView::curCommandSet = initCommands();

TView::TView( const TRect& bounds) noexcept :
    next( 0 ), options( 0 ), eventMask( evMouseDown | evKeyDown | evCommand ),
    state( sfVisible ), growMode( 0 ), dragMode( dmLimitLoY ),
    helpCtx( hcNoContext ), owner( 0 )
{
    setBounds( bounds);
    cursor.x = cursor.y = 0;
    resizeBalance.x = resizeBalance.y = 0;
}

TView::~TView()
{
}

void TView::awaken()
{
}

void TView::blockCursor()
{
     setState(sfCursorIns, True);
}

static inline int range( int val, int min, int max )
{
    if( min > max )
        min = max;
    if( val < min )
        return min;
    else if( val > max )
        return max;
    else
        return val;
}

static int balancedRange( int val, int min, int max, int &balance) noexcept
{
    // Fit val into the range, but keep track of the remainders,
    // and apply them back when possible. This allows views to recover
    // their original sizes.
    if( min > max )
        max = min;
    if( val < min )
        {
        balance += val - min;
        return min;
        }
    else if( val > max )
        {
        balance += val - max;
        return max;
        }
    else
        {
        int offset = range( val+balance, min, max ) - val;
        balance -= offset;
        return val + offset;
        }
}

static inline void fitToLimits( int a, int &b, int min, int max, int &balance)
{
    b = a + balancedRange( b - a, min, max, balance );
}

static inline void grow( TView *p, int s, int d, int &i )
{
    if( p->growMode & gfGrowRel )
        {
        if( s != d )
            i = (i * s + ((s - d) >> 1)) / (s - d);
        }
    else
        i += d;
}

void TView::calcBounds( TRect& bounds, TPoint delta )
{
    bounds = getBounds();

    short s = owner->size.x;
    short d = delta.x;

    if( (growMode & gfGrowLoX) != 0 )
        grow(this, s, d, bounds.a.x);

    if( (growMode & gfGrowHiX) != 0 )
        grow(this, s, d, bounds.b.x);

    s = owner->size.y;
    d = delta.y;

    if( (growMode & gfGrowLoY) != 0 )
        grow(this, s, d, bounds.a.y);

    if( (growMode & gfGrowHiY) != 0 )
        grow(this, s, d, bounds.b.y);

    TPoint minLim, maxLim;
    sizeLimits( minLim, maxLim );
    fitToLimits( bounds.a.x, bounds.b.x, minLim.x, maxLim.x, resizeBalance.x );
    fitToLimits( bounds.a.y, bounds.b.y, minLim.y, maxLim.y, resizeBalance.y );
}

void TView::changeBounds( const TRect& bounds )
{
    setBounds(bounds);
    drawView();
}

void TView::clearEvent( TEvent& event ) noexcept
{
    event.what = evNothing;
    event.message.infoPtr = this;
}

Boolean TView::commandEnabled( ushort command ) noexcept
{
    return Boolean((command > 255) || curCommandSet.has(command));
}

ushort TView::dataSize()
{
    return 0;
}

void TView::disableCommands( TCommandSet& commands ) noexcept
{
    commandSetChanged = Boolean( commandSetChanged ||
                                !(curCommandSet & commands).isEmpty());
    curCommandSet.disableCmd(commands);
}

void TView::disableCommand( ushort command ) noexcept
{
    commandSetChanged = Boolean( commandSetChanged ||
                                 curCommandSet.has(command) );
    curCommandSet.disableCmd(command);
}

void TView::moveGrow( TPoint p,
                      TPoint s,
                      TRect& limits,
                      TPoint minSize,
                      TPoint maxSize,
                      uchar mode
                    )
{
    TRect   r;
    s.x = min(max(s.x, minSize.x), maxSize.x);
    s.y = min(max(s.y, minSize.y), maxSize.y);
    p.x = min(max(p.x, limits.a.x - s.x+1), limits.b.x-1);
    p.y = min(max(p.y, limits.a.y - s.y+1), limits.b.y-1);

    if( (mode & dmLimitLoX) != 0 )
        p.x = max(p.x, limits.a.x);
    if( (mode & dmLimitLoY) != 0 )
        p.y = max(p.y, limits.a.y);
    if( (mode & dmLimitHiX) != 0 )
        p.x = min(p.x, limits.b.x-s.x);
    if( (mode & dmLimitHiY) != 0 )
        p.y = min(p.y, limits.b.y-s.y);
    r = TRect(p.x, p.y, p.x +  s.x, p.y +  s.y);
    locate(r);
}

void TView::change( uchar mode, TPoint delta, TPoint& p, TPoint& s, ushort ctrlState ) noexcept
{
    if( (mode & dmDragMove) != 0 && (ctrlState & kbShift) == 0 )
        p += delta;
    else if( (mode & dmDragGrow) != 0 && (ctrlState & kbShift) != 0 )
        s += delta;
}

void TView::dragView( TEvent& event,
                      uchar mode,
                      TRect& limits,
                      TPoint minSize,
                      TPoint maxSize
                    )
{
    TRect saveBounds;

    TPoint p, s;
    setState( sfDragging, True );

    if( event.what == evMouseDown )
        {
        if( (mode & dmDragMove) != 0 )
            {
            p = origin - event.mouse.where;
            do  {
                event.mouse.where += p;
                moveGrow( event.mouse.where,
                          size,
                          limits,
                          minSize,
                          maxSize,
                          mode
                        );
                } while( mouseEvent(event,evMouseMove) );
            }
        else if( mode & dmDragGrow )
            {
            p = size - event.mouse.where;
            do  {
                event.mouse.where += p;
                moveGrow( origin,
                          event.mouse.where,
                          limits,
                          minSize,
                          maxSize,
                          mode
                        );
                } while( mouseEvent(event,evMouseMove) );
            }
        else // dmDragGrowLeft
            {
            TRect bounds = getBounds();
            s = origin;
            s.y += size.y;
            p = s - event.mouse.where;
            do  {
                event.mouse.where += p;
                bounds.a.x = min(max(event.mouse.where.x, bounds.b.x - maxSize.x), bounds.b.x - minSize.x);
                bounds.b.y = event.mouse.where.y;
                moveGrow( bounds.a,
                          bounds.b - bounds.a,
                          limits,
                          minSize,
                          maxSize,
                          mode
                        );
                } while( mouseEvent(event,evMouseMove) );
            }
        }
    else
        {
        static TPoint
            goLeft      =   {-1, 0},
            goRight     =   { 1, 0},
            goUp        =   { 0,-1},
            goDown      =   { 0, 1},
            goCtrlLeft  =   {-8, 0},
            goCtrlRight =   { 8, 0},
            goCtrlUp    =   { 0,-4},
            goCtrlDown  =   { 0, 4};

        saveBounds = getBounds();
        do  {
            p = origin;
            s = size;
            keyEvent(event);
            switch (event.keyDown.keyCode & 0xFF00)
                {
                case kbLeft:
                    change(mode,goLeft,p,s,event.keyDown.controlKeyState);
                    break;
                case kbRight:
                    change(mode,goRight,p,s,event.keyDown.controlKeyState);
                    break;
                case kbUp:
                    change(mode,goUp,p,s,event.keyDown.controlKeyState);
                    break;
                case kbDown:
                    change(mode,goDown,p,s,event.keyDown.controlKeyState);
                    break;
                case kbCtrlLeft:
                    change(mode,goCtrlLeft,p,s,event.keyDown.controlKeyState);
                    break;
                case kbCtrlRight:
                    change(mode,goCtrlRight,p,s,event.keyDown.controlKeyState);
                    break;
                case kbCtrlUp:
                    change(mode,goCtrlUp,p,s,event.keyDown.controlKeyState);
                    break;
                case kbCtrlDown:
                    change(mode,goCtrlDown,p,s,event.keyDown.controlKeyState);
                    break;
                case kbHome:
                    p.x = limits.a.x;
                    break;
                case kbEnd:
                    p.x = limits.b.x - s.x;
                    break;
                case kbPgUp:
                    p.y = limits.a.y;
                    break;
                case kbPgDn:
                    p.y = limits.b.y - s.y;
                    break;
                }
            moveGrow( p, s, limits, minSize, maxSize, mode );
            } while( event.keyDown.keyCode != kbEsc &&
                     event.keyDown.keyCode != kbEnter
                   );
        if( event.keyDown.keyCode == kbEsc )
            locate(saveBounds);
        }
    setState(sfDragging, False);
}

void TView::draw()
{
    TDrawBuffer  b;

    b.moveChar( 0, ' ', getColor(1), size.x );
    writeLine( 0, 0, size.x, size.y, b );
}

void TView::drawCursor() noexcept
{
    if( (state & sfFocused) != 0 )
        resetCursor();
}

void TView::drawHide( TView* lastView )
{
    drawCursor();
    drawUnderView(Boolean(state & sfShadow), lastView);
}

void TView::drawShow( TView* lastView )
{
    drawView();
    if( (state & sfShadow) != 0 )
        drawUnderView( True, lastView );
}

void TView::drawUnderRect( TRect& r, TView* lastView )
{
    owner->clip.intersect(r);
    owner->drawSubViews(nextView(), lastView);
    owner->clip = owner->getExtent();
}

void TView::drawUnderView( Boolean doShadow, TView* lastView )
{
    TRect r = getBounds();
    if( doShadow != False )
        r.b += shadowSize;
    drawUnderRect( r, lastView );
}

void TView::drawView() noexcept
{
    if (exposed())
        {
        draw();
        drawCursor();
        }
}

void TView::enableCommands( TCommandSet& commands ) noexcept
{
    commandSetChanged = Boolean( commandSetChanged ||
                                ((curCommandSet&commands) != commands) );
    curCommandSet += commands;
}

void TView::enableCommand( ushort command ) noexcept
{
    commandSetChanged = Boolean( commandSetChanged ||
                                 !curCommandSet.has( command ) );
    curCommandSet += command;
}

void TView::endModal( ushort command )
{
    if( TopView() != 0 )
        TopView()->endModal(command);
}

Boolean  TView::eventAvail()
{
    TEvent event;
    getEvent(event);
    if( event.what != evNothing )
        putEvent(event);
    return Boolean( event.what != evNothing );
}

TRect TView::getBounds() const noexcept
{
    return TRect( origin, origin+size );
}

ushort  TView::execute()
{
    return cmCancel;
}

Boolean TView::focus()
{
    Boolean result = True;

    if ((state & (sfSelected | sfModal)) == 0)
        {
        if (owner)
            {
            result = owner->focus();
            if (result)
                {
                if (!owner->current ||
                    (!(owner->current->options & ofValidate) ||
                       owner->current->valid(cmReleasedFocus)))
                    select();
                else
                    return False;
                }
            }
        }
    return result;
}

TRect TView::getClipRect() const noexcept
{
    TRect clip = getBounds();
    if( owner != 0 )
        clip.intersect(owner->clip);
    clip.move(-origin.x, -origin.y);
    return clip;
}

TAttrPair TView::getColor( ushort color ) noexcept
{
    TAttrPair colorPair = color >> 8;

    if( colorPair != 0 )
        colorPair = mapColor(colorPair) << 8;

    colorPair |= mapColor( uchar(color) );

    return colorPair;
}

void TView::getCommands( TCommandSet& commands ) noexcept
{
    commands = curCommandSet;
}

void TView::getData( void * )
{
}

void TView::getEvent( TEvent& event )
{
    if( owner != 0 )
        owner->getEvent(event);
}

void TView::getEvent( TEvent& event, int timeoutMs )
{
    int saveTimeout = TProgram::eventTimeout;
    TProgram::eventTimeout = timeoutMs;

    getEvent( event );

    TProgram::eventTimeout = saveTimeout;
}

TRect TView::getExtent() const noexcept
{
    return TRect( 0, 0, size.x, size.y );
}

ushort TView::getHelpCtx()
{
    if( (state & sfDragging) != 0 )
        return hcDragging;
    return helpCtx;
}

TPalette& TView::getPalette() const
{
    static char ch = 0;
    static TPalette palette( &ch, 0 );
    return palette;
}

Boolean TView::getState( ushort aState ) const noexcept
{
    return Boolean( (state & aState) == aState );
}

void TView::growTo( short x, short y )
{
    TRect r = TRect(origin.x, origin.y, origin.x + x, origin.y + y);
    locate(r);
}

void TView::handleEvent(TEvent& event)
{
    if( event.what == evMouseDown )
        {
        if(!(state & (sfSelected | sfDisabled)) && (options & ofSelectable) )
            if( !focus() || !(options & ofFirstClick) )
                clearEvent(event);
        }
}

void TView::hide()
{
    if( (state & sfVisible) != 0 )
        setState( sfVisible, False );
}

void TView::hideCursor()
{
    setState( sfCursorVis, False );
}

void TView::keyEvent( TEvent& event )
{
    do {
       getEvent(event);
       } while( event.what != evKeyDown );
}

void TView::killTimer( TTimerId id )
{
    if( owner != 0 )
        owner->killTimer(id);
}

void TView::locate( TRect& bounds )
{
    TPoint   min, max;
    sizeLimits(min, max);
    bounds.b.x = bounds.a.x + range(bounds.b.x - bounds.a.x, min.x, max.x);
    bounds.b.y = bounds.a.y + range(bounds.b.y - bounds.a.y, min.y, max.y);
    TRect r = getBounds();
    if( bounds != r )
        {
        changeBounds( bounds );
        if( owner != 0 && (state & sfVisible) != 0 )
            {
            if( (state & sfShadow) != 0 )
                {
                r.Union(bounds);
                r.b += shadowSize;
                }
            drawUnderRect( r, 0 );
            }
        }
}

void TView::makeFirst()
{
    putInFrontOf(owner->first());
}

TPoint TView::makeGlobal( TPoint source ) noexcept
{
    TPoint temp = source + origin;
    TView *cur = this;
    while( cur->owner != 0 )
        {
        cur = cur->owner;
        temp += cur->origin;
        }
    return temp;
}

TPoint TView::makeLocal( TPoint source ) noexcept
{
    TPoint temp = source - origin;
    TView* cur = this;
    while( cur->owner != 0 )
        {
        cur = cur->owner;
        temp -= cur->origin;
        }
    return temp;
}

Boolean TView::mouseEvent(TEvent& event, ushort mask)
{
    do {
       getEvent(event);
        } while( !(event.what & (mask | evMouseUp)) );

    return Boolean(event.what != evMouseUp);
}

Boolean TView::mouseInView(TPoint mouse) noexcept
{
     mouse = makeLocal( mouse );
     TRect r = getExtent();
     return r.contains(mouse);
}

void TView::moveTo( short x, short y )
{
     TRect r( x, y, x+size.x, y+size.y );
     locate(r);
}

TView *TView::nextView() noexcept
{
    if( this == owner->last )
        return 0;
    else
        return next;
}

void TView::normalCursor()
{
    setState(sfCursorIns, False);
}

TView *TView::prev() noexcept
{
    TView* res = this;
    while( res->next != this )
        res = res->next;
    return res;
}

TView *TView::prevView() noexcept
{
    if( this == owner->first() )
        return 0;
    else
        return prev();
}

void TView::putEvent( TEvent& event )
{
    if( owner != 0 )
        owner->putEvent(event);
}

void TView::putInFrontOf( TView *Target )
{
    TView *p, *lastView;

    if( owner != 0 && Target != this && Target != nextView() &&
         ( Target == 0 || Target->owner == owner)
      )
        {
        if( (state & sfVisible) == 0 )
            {
            owner->removeView(this);
            owner->insertView(this, Target);
            }
        else
            {
            lastView = nextView();
            p = Target;
            while( p != 0 && p != this )
                p = p->nextView();
            if( p == 0 )
                lastView = Target;
            state &= ~sfVisible;
            if( lastView == Target )
                drawHide(lastView);
            owner->removeView(this);
            owner->insertView(this, Target);
            state |= sfVisible;
            if( lastView != Target )
                drawShow(lastView);
            if( (options & ofSelectable) != 0 )
                owner->resetCurrent();
            }
        }
}

void TView::select()
{
    if( (options & ofSelectable) != 0 && owner != 0 )
        {
        if( (options & ofTopSelect) != 0 )
            makeFirst();
        else
            owner->setCurrent( this, normalSelect );
        }
}

void TView::setBounds( const TRect& bounds ) noexcept
{
    origin = bounds.a;
    size = bounds.b - bounds.a;
}

void TView::setCmdState(TCommandSet& commands, Boolean enable) noexcept
{
    if (enable)
        enableCommands(commands);
    else
        disableCommands(commands);
}

void TView::setCommands( TCommandSet& commands ) noexcept
{
    commandSetChanged = Boolean( commandSetChanged ||
                                (curCommandSet != commands ));
    curCommandSet = commands;
}

void TView::setCursor( int x, int y ) noexcept
{
    cursor.x = x;
    cursor.y = y;
    drawCursor();
}

void TView::setData( void * )
{
}

void TView::setState( ushort aState, Boolean enable )
{
    if( enable == True )
        state |= aState;
    else
        state &= ~aState;

    if( owner == 0 )
        return;

    switch( aState )
        {
        case  sfVisible:
            if( (owner->state & sfExposed) != 0 )
                setState( sfExposed, enable );
            if( enable == True )
                drawShow( 0 );
            else
                drawHide( 0 );
            if( (options & ofSelectable) != 0 )
                owner->resetCurrent();
            break;
        case  sfCursorVis:
        case  sfCursorIns:
            drawCursor();
            break;
        case  sfShadow:
            drawUnderView( True, 0 );
            break;
        case  sfFocused:
            resetCursor();
            message( owner,
                     evBroadcast,
                     (enable == True) ? cmReceivedFocus : cmReleasedFocus,
                     this
                   );
            break;
        }
}

TTimerId TView::setTimer( uint timeoutMs, int periodMs )
{
    if( owner != 0 )
        return owner->setTimer(timeoutMs, periodMs);
    return 0;
}

void TView::show()
{
    if( (state & sfVisible) == 0 )
        setState(sfVisible, True);
}

void TView::showCursor()
{
    setState( sfCursorVis, True );
}

void TView::sizeLimits( TPoint& min, TPoint& max )
{
    min.x = min.y = 0;
    if( !(growMode & gfFixed) && owner != 0 )
        max = owner->size;
    else
        max.x = max.y = INT_MAX;
}

static Boolean getEventText( TEvent &event, TSpan<char> dest, size_t &length )
{
    if( event.what == evKeyDown )
        {
        TStringView text = event.keyDown.textLength         ? event.keyDown.getText()
                         : event.keyDown.keyCode == kbEnter ? TStringView("\n")
                         : event.keyDown.keyCode == kbTab   ? TStringView("\t")
                                                            : TStringView();
        TSpan<char> dst = dest.subspan( length );
        if( !text.empty() && text.size() <= dst.size() )
            {
            memcpy( dst.data(), text.data(), text.size() );
            length += text.size();
            return True;
            }
        }
    return False;
}

Boolean TView::textEvent( TEvent& event, TSpan<char> dest, size_t &length )
// Fill the 'dest' buffer with text from consecutive events.
// If 'event' is an evKeyDown, its text is also included in 'dest'.
// 'length' is set to the number of bytes written into 'dest'.
// Returns whether any bytes were written into 'dest'.
// On exit, 'event.what' is evNothing.
{
    length = 0;

    getEventText( event, dest, length );
    do  {
        int timeoutMs = 0;
        getEvent( event, timeoutMs );
        } while( getEventText( event, dest, length ) );

    if( event.what != evNothing )
        putEvent( event );
    clearEvent( event );

    return Boolean( length != 0 );
}

TView* TView::TopView() noexcept
{
    if( TheTopView != 0 )
        return TheTopView;
    else
        {
        TView* p = this;
        while( p != 0 && !(p->state & sfModal) )
            p = p->owner;
        return p;
        }
}

Boolean TView::valid( ushort )
{
    return True;
}

Boolean TView::containsMouse( TEvent& event ) noexcept
{
    return Boolean( (state & sfVisible) != 0 &&
                    mouseInView( event.mouse.where )
                  );
}

void TView::shutDown()
{
    hide();
    if( owner != 0 )
        owner->remove( this );
    TObject::shutDown();
}


#if !defined(NO_STREAMABLE)

void TView::write( opstream& os )
{
    ushort saveState =
        state & ~( sfActive | sfSelected | sfFocused | sfExposed );

    os << origin << size << cursor
       << growMode << dragMode << helpCtx
       << saveState << options << eventMask;
}

void *TView::read( ipstream& is )
{
    is >> origin >> size >> cursor
       >> growMode >> dragMode >> helpCtx
       >> state >> options >> eventMask;
    owner = 0;
    next = 0;
    return this;
}

TStreamable *TView::build()
{
    return new TView( streamableInit );
}

TView::TView( StreamableInit ) noexcept
{
}


#endif

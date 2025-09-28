/***********************************************************************
 *                                                                     *
 * MMENU.CPP                                                           *
 *   This module contains the code to support the TMultiMenu class.    *
 *                                                                     *
 ***********************************************************************/

#define Uses_TEvent
#define Uses_TMenu
#define Uses_TSubMenu
#define Uses_TMenuItem
#define Uses_TMenuBar
#include <tv.h>

#if !defined( __MMENU_H )
#include "mmenu.h"
#endif


/***********************************************************************
 * global operator +
 *
 * Since the objects will always be in a linked list, and the operator+
 * is processd left-to-right, we will define the function as appending
 * menuItem2 to menuItem1, and then return menuItem1.
 ***********************************************************************/

#if 0 // This function is now already provided by the library.
TMenuItem& operator +( TMenuItem& menuItem1, TMenuItem& menuItem2 )
{
    TMenuItem *p = &menuItem1;
    while( p->next != NULL )
        p = p->next;
    p->next = &menuItem2;
    return menuItem1;
}
#endif


/***********************************************************************
 *                                                                     *
 * class TTestList                                                     *
 *                                                                     *
 ***********************************************************************
 * TMultiMenu::TMultiMenu
 *   Constructor for a TMultiMenu object.  This version takes an array
 *   of TMenu pointers.
 ***********************************************************************/

TMultiMenu::TMultiMenu( const TRect& bounds, TMenu *aMenu[],
            int nMenus ) :    TMenuBar( bounds, aMenu[0] )
{
    if( nMenus == 0)
        for( nMenus = 0; aMenu[nMenus] != NULL; nMenus++ )
            ;
    mList = new TMenu *[nMenus];
    numMenus = nMenus;

    for( int i = 0; i < nMenus; i++ )
        mList[i] = aMenu[i];
}


/***********************************************************************
 * TMultiMenu::TMultiMenu
 *   Constructor for a TMultiMenu object.  This version takes an array
 *   of TSubMenu objects.
 ***********************************************************************/

TMultiMenu::TMultiMenu( const TRect& bounds, TSubMenu aMenu[],
                        int nMenus ) :
    TMenuBar( bounds, aMenu[0] ),
    mList( new TMenu *[nMenus] ),
    numMenus( nMenus )
{
    mList[0] = menu;                  // First menu is already allocated.
    for( int i = 1; i < nMenus; i++ )
        mList[i] = new TMenu( aMenu[i] );
}


/***********************************************************************
 * TMultiMenu::~TMultiMenu
 *   Destructor for a TMultiMenu object.  Destroys any stored menus
 *   except for the current one (which will be destroyed by ~TMenuBar)
 *   and frees the space where the list was stored.
 ***********************************************************************/

TMultiMenu::~TMultiMenu()
{
    for( int i = 0; i < numMenus; i++ )
        if( mList[i] != menu )          // Delete all but current menu.
            delete mList[i];

    delete [] mList;
}


/***********************************************************************
 * TMultiMenu::handleEvent
 *   Code to respond to the cmMMChangeMenu broadcast message.  The
 *   data the arrives with this message specifies which menu to switch
 *   to, passed via the infoInt data member of TEvent.
 ***********************************************************************/

void TMultiMenu::handleEvent( TEvent& event )
{
    if( event.what == evBroadcast &&
        event.message.command == cmMMChangeMenu )
    {
        if( event.message.infoInt >= 0 &&
            event.message.infoInt < numMenus )
        {
            if( menu != mList[ event.message.infoInt ] )
            {
                menu = mList[ event.message.infoInt ];
                drawView();
            }
        }
        clearEvent( event );
    }
    else
        TMenuBar::handleEvent( event );
}

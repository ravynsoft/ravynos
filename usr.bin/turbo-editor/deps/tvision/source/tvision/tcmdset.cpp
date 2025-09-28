/*------------------------------------------------------------*/
/* filename -       tcmdset.cpp                               */
/*                                                            */
/* function(s)                                                */
/*                  TCommandSet member functions              */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TCommandSet
#include <tvision/tv.h>

int _NEAR TCommandSet::masks[8] =
{
    0x0001,
    0x0002,
    0x0004,
    0x0008,
    0x0010,
    0x0020,
    0x0040,
    0x0080
};

TCommandSet::TCommandSet() noexcept
{
    for( int i = 0; i < 32; i++ )
        cmds[i] = 0;
}

TCommandSet::TCommandSet( const TCommandSet& tc ) noexcept
{
    for( int i = 0; i < 32; i++ )
        cmds[i] = tc.cmds[i];
}

Boolean TCommandSet::has( int cmd ) noexcept
{
    return Boolean( (cmds[ loc( cmd ) ] & mask( cmd )) != 0 );
}

void TCommandSet::disableCmd( int cmd ) noexcept
{
    cmds[ loc( cmd ) ] &= ~mask( cmd );
}

void TCommandSet::enableCmd( const TCommandSet& tc ) noexcept
{
    for( int i = 0; i < 32; i++ )
        cmds[i] |= tc.cmds[i];
}

void TCommandSet::disableCmd( const TCommandSet& tc ) noexcept
{
    for( int i = 0; i < 32; i++ )
        cmds[i] &= ~(tc.cmds[i]);
}

void TCommandSet::enableCmd( int cmd ) noexcept
{
    cmds[ loc( cmd ) ] |= mask( cmd );
}

TCommandSet& TCommandSet::operator &= ( const TCommandSet& tc ) noexcept
{
    for( int i = 0; i < 32; i++ )
        cmds[i] &= tc.cmds[i];
    return *this;
}

TCommandSet& TCommandSet::operator |= ( const TCommandSet& tc ) noexcept
{
    for( int i = 0; i < 32; i++ )
        cmds[i] |= tc.cmds[i];
    return *this;
}

TCommandSet operator & ( const TCommandSet& tc1, const TCommandSet& tc2 ) noexcept
{
    TCommandSet temp( tc1 );
    temp &= tc2;
    return temp;
}

TCommandSet operator | ( const TCommandSet& tc1, const TCommandSet& tc2 ) noexcept
{
    TCommandSet temp( tc1 );
    temp |= tc2;
    return temp;
}

Boolean TCommandSet::isEmpty() noexcept
{
    for( int i = 0; i < 32; i++ )
        if( cmds[i] != 0 )
            return False;
    return True;
}

int operator == ( const TCommandSet& tc1, const TCommandSet& tc2 ) noexcept
{
    for( int i = 0; i < 32; i++ )
        if( tc1.cmds[i] != tc2.cmds[i] )
            return 0;
    return 1;
}


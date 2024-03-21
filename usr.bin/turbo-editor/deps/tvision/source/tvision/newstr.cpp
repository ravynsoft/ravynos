/*------------------------------------------------------------*/
/* filename -       newstr.cpp                                */
/*                                                            */
/* function(s)                                                */
/*                  newStr member function                    */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */


#include <tvision/tv.h>

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

char *newStr( TStringView s ) noexcept
{
    if( s.data() == 0 )
        return 0;
    char *temp = new char[ s.size()+1 ];
    memcpy( temp, s.data(), s.size() );
    temp[s.size()] = EOS;
    return temp;
}

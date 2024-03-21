/*------------------------------------------------------------*/
/* filename -       misc.cpp                                  */
/*                                                            */
/* function(s)                                                */
/*          message -- sends a message to an object           */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TView
#define Uses_TEvent
#define Uses_TObject
#define Uses_TVMemMgr
#include <tvision/tv.h>
#include <stddef.h>
#include <string.h>

void *message( TView *receiver, ushort what, ushort command, void *infoPtr)
{
    if( receiver == 0 )
        return 0;

    TEvent event;
    event.what = what;
    event.message.command = command;
    event.message.infoPtr = infoPtr;
    receiver->handleEvent( event );
    if( event.what == evNothing )
        return event.message.infoPtr;
    else
        return 0;
}

Boolean lowMemory() noexcept
{
    return Boolean(TVMemMgr::safetyPoolExhausted());
}

size_t strnzcpy( char *dest, TStringView src, size_t size ) noexcept
{
    // Same as strlcpy. 'size' is the size of the 'dest' buffer,
    // which is always made null-terminated unless 'size' is zero.
    // Returns the number of bytes copied into 'dest'.
    // 'dest' and 'src' must not overlap.
    if (size)
    {
        size_t copy_bytes = src.size();
        if (copy_bytes > size - 1)
            copy_bytes = size - 1;
        memcpy(dest, src.data(), copy_bytes);
        dest[copy_bytes] = '\0';
        return copy_bytes;
    }
    return 0;
}

size_t strnzcat( char *dest, TStringView src, size_t size ) noexcept
{
    // Similar to strlcpy, except that 'dest' is always left null-terminated,
    // and the return value is the length of 'dest'.
    if (size)
    {
        size_t dstLen = 0;
        while (dstLen < size - 1 && dest[dstLen])
            ++dstLen;
        size_t copy_bytes = src.size();
        if (copy_bytes > size - 1 - dstLen)
            copy_bytes = size - 1 - dstLen;
        memcpy(&dest[dstLen], src.data(), copy_bytes);
        dest[dstLen + copy_bytes] = '\0';
        return dstLen + copy_bytes;
    }
    return 0;
}

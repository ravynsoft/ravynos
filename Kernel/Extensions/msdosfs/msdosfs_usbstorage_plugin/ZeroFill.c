/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  ZeroFill.c
 *  livefiles_msdos
 *
 *  Created by Yakov Ben Zaken on 28/11/2017.
 *
 */

#include "ZeroFill.h"

#define ZERO_BUF_SIZE   (1024*1024)

static void* gpvZeroBuf = NULL;


int
ZeroFill_Init()
{
    if ( gpvZeroBuf )
    {
        return 0;
    }

    gpvZeroBuf = malloc( ZERO_BUF_SIZE );
    if ( gpvZeroBuf == NULL )
    {
        return ENOMEM;
    }

    memset( gpvZeroBuf, 0, ZERO_BUF_SIZE );

    return 0;
}

void
ZeroFill_DeInit()
{
    if ( gpvZeroBuf )
    {
        free( gpvZeroBuf );
    }

    gpvZeroBuf = NULL;
}

int
ZeroFill_Fill( int iFd, uint64_t uOffset, uint32_t uLength )
{
    int iErr                    = 0;
    long lWriteSize             = 0;
    uint64_t uCurWriteOffset    = 0;
    uint32_t uCurWriteLen       = 0;
    uint32_t uDataWriten        = 0;

    if ( gpvZeroBuf == NULL )
    {
        iErr = EINVAL;
        goto exit;
    }

    while ( uDataWriten < uLength )
    {
        uCurWriteOffset = uOffset+uDataWriten;
        uCurWriteLen    = MIN( (uLength - uDataWriten), ZERO_BUF_SIZE );

        lWriteSize = pwrite( iFd, gpvZeroBuf, uCurWriteLen, uCurWriteOffset );
        if ( lWriteSize != uCurWriteLen )
        {
            iErr = errno;
            goto exit;
        }

        uDataWriten += uCurWriteLen;
    }

exit:
    return iErr;
}

/*------------------------------------------------------------*/
/* filename -       strmstat.cpp                              */
/*                                                            */
/* defines the static members of class pstream                */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_pstream
#include <tvision/tv.h>

TStreamableTypes * _NEAR pstream::types = (pstream::initTypes(), pstream::types);

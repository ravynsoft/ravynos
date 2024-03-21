/*------------------------------------------------------------*/
/* filename -       svalid.cpp                                */
/*                                                            */
/* Registeration object for the TValidator classes            */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TValidator
#define Uses_TFilterValidator
#define Uses_TRangeValidator
#define Uses_TPXPictureValidator
#define Uses_TLookupValidator
#define Uses_TStringLookupValidator
#define Uses_TStreamableClass

#include <tvision/tv.h>

TStreamableClass RValidator( TValidator::name,
                         TValidator::build,
                         __DELTA(TValidator)
                       );

TStreamableClass RFilterValidator( TFilterValidator::name,
                         TFilterValidator::build,
                         __DELTA(TFilterValidator)
                       );

TStreamableClass RRangeValidator( TRangeValidator::name,
                         TRangeValidator::build,
                         __DELTA(TRangeValidator)
                       );

TStreamableClass RPXPictureValidator( TPXPictureValidator::name,
                         TPXPictureValidator::build,
                         __DELTA(TPXPictureValidator)
                       );

TStreamableClass RLookupValidator( TLookupValidator::name,
                         TLookupValidator::build,
                         __DELTA(TLookupValidator)
                       );

TStreamableClass RStringLookupValidator( TStringLookupValidator::name,
                         TStringLookupValidator::build,
                         __DELTA(TStringLookupValidator)
                       );

#endif

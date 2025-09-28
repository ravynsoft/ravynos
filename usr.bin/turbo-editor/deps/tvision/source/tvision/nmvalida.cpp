/*------------------------------------------------------------*/
/* filename -       nmvalida.cpp                              */
/*                                                            */
/* defines the streamable name for TValidator classes         */
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
#define Uses_TPXPictureValidator
#define Uses_TRangeValidator
#define Uses_TFilterValidator
#define Uses_TLookupValidator
#define Uses_TStringLookupValidator
#include <tvision/tv.h>

const char * const _NEAR TValidator::name = "TValidator";
const char * const _NEAR TPXPictureValidator::name = "TPXPictureValidator";
const char * const _NEAR TRangeValidator::name = "TRangeValidator";
const char * const _NEAR TFilterValidator::name = "TFilterValidator";
const char * const _NEAR TLookupValidator::name = "TLookupValidator";
const char * const _NEAR TStringLookupValidator::name = "TStringLookupValidator";
#endif

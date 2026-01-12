/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
    CFICULogging.h
    Copyright (c) 2008-2014, Apple Inc. All rights reserved.
*/

/*
 This file is for the use of the CoreFoundation project only.
*/

#if !defined(__COREFOUNDATION_CFICULOGGING__)
#define __COREFOUNDATION_CFICULOGGING__ 1

#include <unicode/ucal.h>
#include <unicode/udatpg.h>
#include <unicode/udat.h>
#include <unicode/unum.h>
#include <unicode/ucurr.h>
#include <unicode/ustring.h>

// ucal


// ucal
#define __cficu_ucal_open ucal_open
#define __cficu_ucal_close ucal_close
#define __cficu_ucal_clone ucal_clone
#define __cficu_ucal_setAttribute ucal_setAttribute
#define __cficu_ucal_getAttribute ucal_getAttribute
#define __cficu_ucal_setGregorianChange ucal_setGregorianChange
#define __cficu_ucal_getGregorianChange ucal_getGregorianChange
#define __cficu_ucal_getMillis ucal_getMillis
#define __cficu_ucal_setMillis ucal_setMillis
#define __cficu_ucal_set ucal_set
#define __cficu_ucal_get ucal_get
#define __cficu_ucal_getDayOfWeekType ucal_getDayOfWeekType
#define __cficu_ucal_getWeekendTransition ucal_getWeekendTransition
#define __cficu_ucal_isWeekend ucal_isWeekend
#define __cficu_ucal_clear ucal_clear
#define __cficu_ucal_getLimit ucal_getLimit
#define __cficu_ucal_add ucal_add
#define __cficu_ucal_roll ucal_roll
#define __cficu_ucal_getFieldDifference ucal_getFieldDifference
#define __cficu_ucal_getNow ucal_getNow
#define __cficu_ucal_setTimeZone ucal_setTimeZone
// udatpg
#define __cficu_udatpg_open udatpg_open
#define __cficu_udatpg_close udatpg_close
#define __cficu_udatpg_getSkeleton udatpg_getSkeleton
#define __cficu_udatpg_getBestPattern udatpg_getBestPattern
// udat
#define __cficu_udat_applyPattern udat_applyPattern
#define __cficu_udat_applyPatternRelative udat_applyPatternRelative
#define __cficu_udat_clone udat_clone
#define __cficu_udat_close udat_close
#define __cficu_udat_countSymbols udat_countSymbols
#define __cficu_udat_format udat_format
#define __cficu_udat_get2DigitYearStart udat_get2DigitYearStart
#define __cficu_udat_getCalendar udat_getCalendar
#define __cficu_udat_getSymbols udat_getSymbols
#define __cficu_udat_isLenient udat_isLenient
#define __cficu_udat_open udat_open
#define __cficu_udat_parseCalendar udat_parseCalendar
#define __cficu_udat_set2DigitYearStart udat_set2DigitYearStart
#define __cficu_udat_setCalendar udat_setCalendar
#define __cficu_udat_setLenient udat_setLenient
#define __cficu_udat_setSymbols udat_setSymbols
#define __cficu_udat_toPattern udat_toPattern
#define __cficu_udat_toPatternRelativeDate udat_toPatternRelativeDate
#define __cficu_udat_toPatternRelativeTime udat_toPatternRelativeTime
#define __cficu_unum_applyPattern unum_applyPattern
#define __cficu_unum_close unum_close
#define __cficu_unum_formatDecimal unum_formatDecimal
#define __cficu_unum_formatDouble unum_formatDouble
#define __cficu_unum_getAttribute unum_getAttribute
#define __cficu_unum_getDoubleAttribute unum_getDoubleAttribute
#define __cficu_unum_getSymbol unum_getSymbol
#define __cficu_unum_getTextAttribute unum_getTextAttribute
#define __cficu_unum_open unum_open
#define __cficu_unum_parse unum_parse
#define __cficu_unum_parseDecimal unum_parseDecimal
#define __cficu_unum_setAttribute unum_setAttribute
#define __cficu_unum_setDoubleAttribute unum_setDoubleAttribute
#define __cficu_unum_setSymbol unum_setSymbol
#define __cficu_unum_setTextAttribute unum_setTextAttribute
#define __cficu_unum_toPattern unum_toPattern
#define __cficu_udat_setContext udat_setContext
#define __cficu_udat_getContext udat_getContext
// ucurr
#define __cficu_ucurr_getDefaultFractionDigits ucurr_getDefaultFractionDigits
#define __cficu_ucurr_getRoundingIncrement ucurr_getRoundingIncrement
// unum
#define __cficu_unum_setContext unum_setContext
#define __cficu_unum_getContext unum_getContext

#endif

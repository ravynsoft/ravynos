/*
	CFICULogging.h
	Copyright (c) 2008-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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
#include <unicode/ulistformatter.h>
#include <unicode/ucurr.h>
#include <unicode/ustring.h>
#if __has_include(<unicode/ureldatefmt.h>)
#include <unicode/ureldatefmt.h>
#endif

#if !DEPLOYMENT_RUNTIME_SWIFT && __has_include(<os/log.h>)
#include <os/log.h>
#endif

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
#define __cficu_udat_formatForFields udat_formatForFields
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
// ureldatefmt
#define __cficu_ureldatefmt_open ureldatefmt_open
#define __cficu_ureldatefmt_formatNumeric ureldatefmt_formatNumeric
#define __cficu_ureldatefmt_format ureldatefmt_format
#define __cficu_ureldatefmt_close ureldatefmt_close
// ulistfmt
#define __cficu_ulistfmt_open ulistfmt_open
#define __cficu_ulistfmt_close ulistfmt_close
#define __cficu_ulistfmt_format ulistfmt_format
#endif

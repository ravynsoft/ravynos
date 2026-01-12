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

/*	CFLocaleKeys.c
	Copyright (c) 2008-2014, Apple Inc. All rights reserved.
	Responsibility: Christopher Kane
*/

#include "CFInternal.h"

CONST_STRING_DECL(kCFLocaleAlternateQuotationBeginDelimiterKey, "kCFLocaleAlternateQuotationBeginDelimiterKey");
CONST_STRING_DECL(kCFLocaleAlternateQuotationEndDelimiterKey, "kCFLocaleAlternateQuotationEndDelimiterKey");
CONST_STRING_DECL(kCFLocaleQuotationBeginDelimiterKey, "kCFLocaleQuotationBeginDelimiterKey");
CONST_STRING_DECL(kCFLocaleQuotationEndDelimiterKey, "kCFLocaleQuotationEndDelimiterKey");
CONST_STRING_DECL(kCFLocaleCalendarIdentifierKey, "calendar"); // ***
CONST_STRING_DECL(kCFLocaleCalendarKey, "kCFLocaleCalendarKey");

CONST_STRING_DECL(kCFLocaleCollationIdentifierKey, "collation"); // ***
CONST_STRING_DECL(kCFLocaleCollatorIdentifierKey, "kCFLocaleCollatorIdentifierKey");
CONST_STRING_DECL(kCFLocaleCountryCodeKey, "kCFLocaleCountryCodeKey");
CONST_STRING_DECL(kCFLocaleCurrencyCodeKey, "currency"); // ***
CONST_STRING_DECL(kCFLocaleCurrencySymbolKey, "kCFLocaleCurrencySymbolKey");
CONST_STRING_DECL(kCFLocaleDecimalSeparatorKey, "kCFLocaleDecimalSeparatorKey");
CONST_STRING_DECL(kCFLocaleExemplarCharacterSetKey, "kCFLocaleExemplarCharacterSetKey");
CONST_STRING_DECL(kCFLocaleGroupingSeparatorKey, "kCFLocaleGroupingSeparatorKey");
CONST_STRING_DECL(kCFLocaleIdentifierKey, "kCFLocaleIdentifierKey");
CONST_STRING_DECL(kCFLocaleLanguageCodeKey, "kCFLocaleLanguageCodeKey");
CONST_STRING_DECL(kCFLocaleMeasurementSystemKey, "kCFLocaleMeasurementSystemKey");
CONST_STRING_DECL(kCFLocaleScriptCodeKey, "kCFLocaleScriptCodeKey");
CONST_STRING_DECL(kCFLocaleUsesMetricSystemKey, "kCFLocaleUsesMetricSystemKey");
CONST_STRING_DECL(kCFLocaleVariantCodeKey, "kCFLocaleVariantCodeKey");

CONST_STRING_DECL(kCFDateFormatterAMSymbolKey, "kCFDateFormatterAMSymbolKey");
CONST_STRING_DECL(kCFDateFormatterCalendarKey, "kCFDateFormatterCalendarKey");
CONST_STRING_DECL(kCFDateFormatterCalendarIdentifierKey, "kCFDateFormatterCalendarIdentifierKey");
CONST_STRING_DECL(kCFDateFormatterDefaultDateKey, "kCFDateFormatterDefaultDateKey");
CONST_STRING_DECL(kCFDateFormatterDefaultFormatKey, "kCFDateFormatterDefaultFormatKey");
CONST_STRING_DECL(kCFDateFormatterDoesRelativeDateFormattingKey, "kCFDateFormatterDoesRelativeDateFormattingKey");
CONST_STRING_DECL(kCFDateFormatterEraSymbolsKey, "kCFDateFormatterEraSymbolsKey");
CONST_STRING_DECL(kCFDateFormatterGregorianStartDateKey, "kCFDateFormatterGregorianStartDateKey");
CONST_STRING_DECL(kCFDateFormatterIsLenientKey, "kCFDateFormatterIsLenientKey");
CONST_STRING_DECL(kCFDateFormatterLongEraSymbolsKey, "kCFDateFormatterLongEraSymbolsKey");
CONST_STRING_DECL(kCFDateFormatterMonthSymbolsKey, "kCFDateFormatterMonthSymbolsKey");
CONST_STRING_DECL(kCFDateFormatterPMSymbolKey, "kCFDateFormatterPMSymbolKey");
CONST_STRING_DECL(kCFDateFormatterAmbiguousYearStrategyKey, "kCFDateFormatterAmbiguousYearStrategyKey");
CONST_STRING_DECL(kCFDateFormatterQuarterSymbolsKey, "kCFDateFormatterQuarterSymbolsKey");
CONST_STRING_DECL(kCFDateFormatterShortMonthSymbolsKey, "kCFDateFormatterShortMonthSymbolsKey");
CONST_STRING_DECL(kCFDateFormatterShortQuarterSymbolsKey, "kCFDateFormatterShortQuarterSymbolsKey");
CONST_STRING_DECL(kCFDateFormatterShortStandaloneMonthSymbolsKey, "kCFDateFormatterShortStandaloneMonthSymbolsKey");
CONST_STRING_DECL(kCFDateFormatterShortStandaloneQuarterSymbolsKey, "kCFDateFormatterShortStandaloneQuarterSymbolsKey");
CONST_STRING_DECL(kCFDateFormatterShortStandaloneWeekdaySymbolsKey, "kCFDateFormatterShortStandaloneWeekdaySymbolsKey");
CONST_STRING_DECL(kCFDateFormatterShortWeekdaySymbolsKey, "kCFDateFormatterShortWeekdaySymbolsKey");
CONST_STRING_DECL(kCFDateFormatterStandaloneMonthSymbolsKey, "kCFDateFormatterStandaloneMonthSymbolsKey");
CONST_STRING_DECL(kCFDateFormatterStandaloneQuarterSymbolsKey, "kCFDateFormatterStandaloneQuarterSymbolsKey");
CONST_STRING_DECL(kCFDateFormatterStandaloneWeekdaySymbolsKey, "kCFDateFormatterStandaloneWeekdaySymbolsKey");
CONST_STRING_DECL(kCFDateFormatterTimeZoneKey, "kCFDateFormatterTimeZoneKey");
CONST_STRING_DECL(kCFDateFormatterTwoDigitStartDateKey, "kCFDateFormatterTwoDigitStartDateKey");
CONST_STRING_DECL(kCFDateFormatterVeryShortMonthSymbolsKey, "kCFDateFormatterVeryShortMonthSymbolsKey");
CONST_STRING_DECL(kCFDateFormatterVeryShortStandaloneMonthSymbolsKey, "kCFDateFormatterVeryShortStandaloneMonthSymbolsKey");
CONST_STRING_DECL(kCFDateFormatterVeryShortStandaloneWeekdaySymbolsKey, "kCFDateFormatterVeryShortStandaloneWeekdaySymbolsKey");
CONST_STRING_DECL(kCFDateFormatterVeryShortWeekdaySymbolsKey, "kCFDateFormatterVeryShortWeekdaySymbolsKey");
CONST_STRING_DECL(kCFDateFormatterWeekdaySymbolsKey, "kCFDateFormatterWeekdaySymbolsKey");
CONST_STRING_DECL(kCFDateFormatterUsesCharacterDirectionKey, "kCFDateFormatterUsesCharacterDirectionKey");

CONST_STRING_DECL(kCFNumberFormatterAlwaysShowDecimalSeparatorKey, "kCFNumberFormatterAlwaysShowDecimalSeparatorKey");
CONST_STRING_DECL(kCFNumberFormatterCurrencyCodeKey, "kCFNumberFormatterCurrencyCodeKey");
CONST_STRING_DECL(kCFNumberFormatterCurrencyDecimalSeparatorKey, "kCFNumberFormatterCurrencyDecimalSeparatorKey");
CONST_STRING_DECL(kCFNumberFormatterCurrencyGroupingSeparatorKey, "kCFNumberFormatterCurrencyGroupingSeparatorKey");
CONST_STRING_DECL(kCFNumberFormatterCurrencySymbolKey, "kCFNumberFormatterCurrencySymbolKey");
CONST_STRING_DECL(kCFNumberFormatterDecimalSeparatorKey, "kCFNumberFormatterDecimalSeparatorKey");
CONST_STRING_DECL(kCFNumberFormatterDefaultFormatKey, "kCFNumberFormatterDefaultFormatKey");
CONST_STRING_DECL(kCFNumberFormatterExponentSymbolKey, "kCFNumberFormatterExponentSymbolKey");
CONST_STRING_DECL(kCFNumberFormatterFormatWidthKey, "kCFNumberFormatterFormatWidthKey");
CONST_STRING_DECL(kCFNumberFormatterGroupingSeparatorKey, "kCFNumberFormatterGroupingSeparatorKey");
CONST_STRING_DECL(kCFNumberFormatterGroupingSizeKey, "kCFNumberFormatterGroupingSizeKey");
CONST_STRING_DECL(kCFNumberFormatterInfinitySymbolKey, "kCFNumberFormatterInfinitySymbolKey");
CONST_STRING_DECL(kCFNumberFormatterInternationalCurrencySymbolKey, "kCFNumberFormatterInternationalCurrencySymbolKey");
CONST_STRING_DECL(kCFNumberFormatterIsLenientKey, "kCFNumberFormatterIsLenientKey");
CONST_STRING_DECL(kCFNumberFormatterMaxFractionDigitsKey, "kCFNumberFormatterMaxFractionDigitsKey");
CONST_STRING_DECL(kCFNumberFormatterMaxIntegerDigitsKey, "kCFNumberFormatterMaxIntegerDigitsKey");
CONST_STRING_DECL(kCFNumberFormatterMaxSignificantDigitsKey, "kCFNumberFormatterMaxSignificantDigitsKey");
CONST_STRING_DECL(kCFNumberFormatterMinFractionDigitsKey, "kCFNumberFormatterMinFractionDigitsKey");
CONST_STRING_DECL(kCFNumberFormatterMinIntegerDigitsKey, "kCFNumberFormatterMinIntegerDigitsKey");
CONST_STRING_DECL(kCFNumberFormatterMinSignificantDigitsKey, "kCFNumberFormatterMinSignificantDigitsKey");
CONST_STRING_DECL(kCFNumberFormatterMinusSignKey, "kCFNumberFormatterMinusSignKey");
CONST_STRING_DECL(kCFNumberFormatterMultiplierKey, "kCFNumberFormatterMultiplierKey");
CONST_STRING_DECL(kCFNumberFormatterNaNSymbolKey, "kCFNumberFormatterNaNSymbolKey");
CONST_STRING_DECL(kCFNumberFormatterNegativePrefixKey, "kCFNumberFormatterNegativePrefixKey");
CONST_STRING_DECL(kCFNumberFormatterNegativeSuffixKey, "kCFNumberFormatterNegativeSuffixKey");
CONST_STRING_DECL(kCFNumberFormatterPaddingCharacterKey, "kCFNumberFormatterPaddingCharacterKey");
CONST_STRING_DECL(kCFNumberFormatterPaddingPositionKey, "kCFNumberFormatterPaddingPositionKey");
CONST_STRING_DECL(kCFNumberFormatterPerMillSymbolKey, "kCFNumberFormatterPerMillSymbolKey");
CONST_STRING_DECL(kCFNumberFormatterPercentSymbolKey, "kCFNumberFormatterPercentSymbolKey");
CONST_STRING_DECL(kCFNumberFormatterPlusSignKey, "kCFNumberFormatterPlusSignKey");
CONST_STRING_DECL(kCFNumberFormatterPositivePrefixKey, "kCFNumberFormatterPositivePrefixKey");
CONST_STRING_DECL(kCFNumberFormatterPositiveSuffixKey, "kCFNumberFormatterPositiveSuffixKey");
CONST_STRING_DECL(kCFNumberFormatterRoundingIncrementKey, "kCFNumberFormatterRoundingIncrementKey");
CONST_STRING_DECL(kCFNumberFormatterRoundingModeKey, "kCFNumberFormatterRoundingModeKey");
CONST_STRING_DECL(kCFNumberFormatterSecondaryGroupingSizeKey, "kCFNumberFormatterSecondaryGroupingSizeKey");
CONST_STRING_DECL(kCFNumberFormatterUseGroupingSeparatorKey, "kCFNumberFormatterUseGroupingSeparatorKey");
CONST_STRING_DECL(kCFNumberFormatterUseSignificantDigitsKey, "kCFNumberFormatterUseSignificantDigitsKey");
CONST_STRING_DECL(kCFNumberFormatterZeroSymbolKey, "kCFNumberFormatterZeroSymbolKey");
CONST_STRING_DECL(kCFNumberFormatterUsesCharacterDirectionKey, "kCFNumberFormatterUsesCharacterDirectionKey");

CONST_STRING_DECL(kCFCalendarIdentifierGregorian, "gregorian");
CONST_STRING_DECL(kCFCalendarIdentifierBuddhist, "buddhist");
CONST_STRING_DECL(kCFCalendarIdentifierJapanese, "japanese");
CONST_STRING_DECL(kCFCalendarIdentifierIslamic, "islamic");
CONST_STRING_DECL(kCFCalendarIdentifierIslamicCivil, "islamic-civil");
CONST_STRING_DECL(kCFCalendarIdentifierIslamicUmmAlQura, "islamic-umalqura");
CONST_STRING_DECL(kCFCalendarIdentifierIslamicTabular, "islamic-tbla");
CONST_STRING_DECL(kCFCalendarIdentifierHebrew, "hebrew");
CONST_STRING_DECL(kCFCalendarIdentifierChinese, "chinese");
CONST_STRING_DECL(kCFCalendarIdentifierRepublicOfChina, "roc");
CONST_STRING_DECL(kCFCalendarIdentifierPersian, "persian");
CONST_STRING_DECL(kCFCalendarIdentifierIndian, "indian");
CONST_STRING_DECL(kCFCalendarIdentifierISO8601, "");
CONST_STRING_DECL(kCFCalendarIdentifierCoptic, "coptic");
CONST_STRING_DECL(kCFCalendarIdentifierEthiopicAmeteMihret, "ethiopic");
CONST_STRING_DECL(kCFCalendarIdentifierEthiopicAmeteAlem, "ethiopic-amete-alem");

// Aliases for Linux
#if DEPLOYMENT_TARGET_LINUX

CF_EXPORT CFStringRef const kCFBuddhistCalendar __attribute__((alias ("kCFCalendarIdentifierBuddhist")));
CF_EXPORT CFStringRef const kCFChineseCalendar __attribute__((alias ("kCFCalendarIdentifierChinese")));
CF_EXPORT CFStringRef const kCFGregorianCalendar __attribute__((alias ("kCFCalendarIdentifierGregorian")));
CF_EXPORT CFStringRef const kCFHebrewCalendar __attribute__((alias ("kCFCalendarIdentifierHebrew")));
CF_EXPORT CFStringRef const kCFISO8601Calendar __attribute__((alias ("kCFCalendarIdentifierISO8601")));
CF_EXPORT CFStringRef const kCFIndianCalendar __attribute__((alias ("kCFCalendarIdentifierIndian")));
CF_EXPORT CFStringRef const kCFIslamicCalendar __attribute__((alias ("kCFCalendarIdentifierIslamic")));
CF_EXPORT CFStringRef const kCFIslamicTabularCalendar __attribute__((alias ("kCFCalendarIdentifierIslamicTabular")));
CF_EXPORT CFStringRef const kCFIslamicUmmAlQuraCalendar __attribute__((alias ("kCFCalendarIdentifierIslamicUmmAlQura")));
CF_EXPORT CFStringRef const kCFIslamicCivilCalendar __attribute__((alias ("kCFCalendarIdentifierIslamicCivil")));
CF_EXPORT CFStringRef const kCFJapaneseCalendar __attribute__((alias ("kCFCalendarIdentifierJapanese")));
CF_EXPORT CFStringRef const kCFPersianCalendar __attribute__((alias ("kCFCalendarIdentifierPersian")));
CF_EXPORT CFStringRef const kCFRepublicOfChinaCalendar __attribute__((alias ("kCFCalendarIdentifierRepublicOfChina")));


CF_EXPORT CFStringRef const kCFLocaleCalendarIdentifier __attribute__((alias ("kCFLocaleCalendarIdentifierKey")));
CF_EXPORT CFStringRef const kCFLocaleCalendar __attribute__((alias ("kCFLocaleCalendarKey")));
CF_EXPORT CFStringRef const kCFLocaleCollationIdentifier __attribute__((alias ("kCFLocaleCollationIdentifierKey")));
CF_EXPORT CFStringRef const kCFLocaleCollatorIdentifier __attribute__((alias ("kCFLocaleCollatorIdentifierKey")));
CF_EXPORT CFStringRef const kCFLocaleCountryCode __attribute__((alias ("kCFLocaleCountryCodeKey")));
CF_EXPORT CFStringRef const kCFLocaleCurrencyCode __attribute__((alias ("kCFLocaleCurrencyCodeKey")));
CF_EXPORT CFStringRef const kCFLocaleCurrencySymbol __attribute__((alias ("kCFLocaleCurrencySymbolKey")));
CF_EXPORT CFStringRef const kCFLocaleDecimalSeparator __attribute__((alias ("kCFLocaleDecimalSeparatorKey")));
CF_EXPORT CFStringRef const kCFLocaleExemplarCharacterSet __attribute__((alias ("kCFLocaleExemplarCharacterSetKey")));
CF_EXPORT CFStringRef const kCFLocaleGroupingSeparator __attribute__((alias ("kCFLocaleGroupingSeparatorKey")));
CF_EXPORT CFStringRef const kCFLocaleIdentifier __attribute__((alias ("kCFLocaleIdentifierKey")));
CF_EXPORT CFStringRef const kCFLocaleLanguageCode __attribute__((alias ("kCFLocaleLanguageCodeKey")));
CF_EXPORT CFStringRef const kCFLocaleMeasurementSystem __attribute__((alias ("kCFLocaleMeasurementSystemKey")));
CF_EXPORT CFStringRef const kCFLocaleScriptCode __attribute__((alias ("kCFLocaleScriptCodeKey")));
CF_EXPORT CFStringRef const kCFLocaleUsesMetricSystem __attribute__((alias ("kCFLocaleUsesMetricSystemKey")));
CF_EXPORT CFStringRef const kCFLocaleVariantCode __attribute__((alias ("kCFLocaleVariantCodeKey")));
CF_EXPORT CFStringRef const kCFDateFormatterAMSymbol __attribute__((alias ("kCFDateFormatterAMSymbolKey")));
CF_EXPORT CFStringRef const kCFDateFormatterCalendar __attribute__((alias ("kCFDateFormatterCalendarKey")));
CF_EXPORT CFStringRef const kCFDateFormatterCalendarIdentifier __attribute__((alias ("kCFDateFormatterCalendarIdentifierKey")));
CF_EXPORT CFStringRef const kCFDateFormatterDefaultDate __attribute__((alias ("kCFDateFormatterDefaultDateKey")));
CF_EXPORT CFStringRef const kCFDateFormatterDefaultFormat __attribute__((alias ("kCFDateFormatterDefaultFormatKey")));
CF_EXPORT CFStringRef const kCFDateFormatterEraSymbols __attribute__((alias ("kCFDateFormatterEraSymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterGregorianStartDate __attribute__((alias ("kCFDateFormatterGregorianStartDateKey")));
CF_EXPORT CFStringRef const kCFDateFormatterIsLenient __attribute__((alias ("kCFDateFormatterIsLenientKey")));
CF_EXPORT CFStringRef const kCFDateFormatterLongEraSymbols __attribute__((alias ("kCFDateFormatterLongEraSymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterMonthSymbols __attribute__((alias ("kCFDateFormatterMonthSymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterPMSymbol __attribute__((alias ("kCFDateFormatterPMSymbolKey")));
CF_EXPORT CFStringRef const kCFDateFormatterQuarterSymbols __attribute__((alias ("kCFDateFormatterQuarterSymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterShortMonthSymbols __attribute__((alias ("kCFDateFormatterShortMonthSymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterShortQuarterSymbols __attribute__((alias ("kCFDateFormatterShortQuarterSymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterShortStandaloneMonthSymbols __attribute__((alias ("kCFDateFormatterShortStandaloneMonthSymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterShortStandaloneQuarterSymbols __attribute__((alias ("kCFDateFormatterShortStandaloneQuarterSymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterShortStandaloneWeekdaySymbols __attribute__((alias ("kCFDateFormatterShortStandaloneWeekdaySymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterShortWeekdaySymbols __attribute__((alias ("kCFDateFormatterShortWeekdaySymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterStandaloneMonthSymbols __attribute__((alias ("kCFDateFormatterStandaloneMonthSymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterStandaloneQuarterSymbols __attribute__((alias ("kCFDateFormatterStandaloneQuarterSymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterStandaloneWeekdaySymbols __attribute__((alias ("kCFDateFormatterStandaloneWeekdaySymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterTimeZone __attribute__((alias ("kCFDateFormatterTimeZoneKey")));
CF_EXPORT CFStringRef const kCFDateFormatterTwoDigitStartDate __attribute__((alias ("kCFDateFormatterTwoDigitStartDateKey")));
CF_EXPORT CFStringRef const kCFDateFormatterVeryShortMonthSymbols __attribute__((alias ("kCFDateFormatterVeryShortMonthSymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterVeryShortStandaloneMonthSymbols __attribute__((alias ("kCFDateFormatterVeryShortStandaloneMonthSymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterVeryShortStandaloneWeekdaySymbols __attribute__((alias ("kCFDateFormatterVeryShortStandaloneWeekdaySymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterVeryShortWeekdaySymbols __attribute__((alias ("kCFDateFormatterVeryShortWeekdaySymbolsKey")));
CF_EXPORT CFStringRef const kCFDateFormatterWeekdaySymbols __attribute__((alias ("kCFDateFormatterWeekdaySymbolsKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterAlwaysShowDecimalSeparator __attribute__((alias ("kCFNumberFormatterAlwaysShowDecimalSeparatorKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterCurrencyCode __attribute__((alias ("kCFNumberFormatterCurrencyCodeKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterCurrencyDecimalSeparator __attribute__((alias ("kCFNumberFormatterCurrencyDecimalSeparatorKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterCurrencyGroupingSeparator __attribute__((alias ("kCFNumberFormatterCurrencyGroupingSeparatorKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterCurrencySymbol __attribute__((alias ("kCFNumberFormatterCurrencySymbolKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterDecimalSeparator __attribute__((alias ("kCFNumberFormatterDecimalSeparatorKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterDefaultFormat __attribute__((alias ("kCFNumberFormatterDefaultFormatKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterExponentSymbol __attribute__((alias ("kCFNumberFormatterExponentSymbolKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterFormatWidth __attribute__((alias ("kCFNumberFormatterFormatWidthKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterGroupingSeparator __attribute__((alias ("kCFNumberFormatterGroupingSeparatorKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterGroupingSize __attribute__((alias ("kCFNumberFormatterGroupingSizeKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterInfinitySymbol __attribute__((alias ("kCFNumberFormatterInfinitySymbolKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterInternationalCurrencySymbol __attribute__((alias ("kCFNumberFormatterInternationalCurrencySymbolKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterIsLenient __attribute__((alias ("kCFNumberFormatterIsLenientKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterMaxFractionDigits __attribute__((alias ("kCFNumberFormatterMaxFractionDigitsKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterMaxIntegerDigits __attribute__((alias ("kCFNumberFormatterMaxIntegerDigitsKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterMaxSignificantDigits __attribute__((alias ("kCFNumberFormatterMaxSignificantDigitsKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterMinFractionDigits __attribute__((alias ("kCFNumberFormatterMinFractionDigitsKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterMinIntegerDigits __attribute__((alias ("kCFNumberFormatterMinIntegerDigitsKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterMinSignificantDigits __attribute__((alias ("kCFNumberFormatterMinSignificantDigitsKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterMinusSign __attribute__((alias ("kCFNumberFormatterMinusSignKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterMultiplier __attribute__((alias ("kCFNumberFormatterMultiplierKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterNaNSymbol __attribute__((alias ("kCFNumberFormatterNaNSymbolKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterNegativePrefix __attribute__((alias ("kCFNumberFormatterNegativePrefixKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterNegativeSuffix __attribute__((alias ("kCFNumberFormatterNegativeSuffixKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterPaddingCharacter __attribute__((alias ("kCFNumberFormatterPaddingCharacterKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterPaddingPosition __attribute__((alias ("kCFNumberFormatterPaddingPositionKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterPerMillSymbol __attribute__((alias ("kCFNumberFormatterPerMillSymbolKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterPercentSymbol __attribute__((alias ("kCFNumberFormatterPercentSymbolKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterPlusSign __attribute__((alias ("kCFNumberFormatterPlusSignKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterPositivePrefix __attribute__((alias ("kCFNumberFormatterPositivePrefixKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterPositiveSuffix __attribute__((alias ("kCFNumberFormatterPositiveSuffixKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterRoundingIncrement __attribute__((alias ("kCFNumberFormatterRoundingIncrementKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterRoundingMode __attribute__((alias ("kCFNumberFormatterRoundingModeKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterSecondaryGroupingSize __attribute__((alias ("kCFNumberFormatterSecondaryGroupingSizeKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterUseGroupingSeparator __attribute__((alias ("kCFNumberFormatterUseGroupingSeparatorKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterUseSignificantDigits __attribute__((alias ("kCFNumberFormatterUseSignificantDigitsKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterZeroSymbol __attribute__((alias ("kCFNumberFormatterZeroSymbolKey")));
CF_EXPORT CFStringRef const kCFNumberFormatterUsesCharacterDirection __attribute__((alias ("kCFNumberFormatterUsesCharacterDirectionKey")));
CF_EXPORT CFStringRef const kCFDateFormatterUsesCharacterDirection __attribute__((alias ("kCFDateFormatterUsesCharacterDirectionKey")));
CF_EXPORT CFStringRef const kCFDateFormatterCalendarName __attribute__((alias ("kCFDateFormatterCalendarIdentifierKey")));

#endif

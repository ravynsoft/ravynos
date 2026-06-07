/*
*****************************************************************************************
* Copyright (C) 2014-2017 Apple Inc. All Rights Reserved.
*****************************************************************************************
*/

#ifndef UAMEASUREFORMAT_H
#define UAMEASUREFORMAT_H

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING
#ifndef U_HIDE_DRAFT_API

#include "unicode/localpointer.h"
#include "unicode/unum.h"
#include "unicode/umisc.h"
#include "unicode/ufieldpositer.h"

/**
 * \file
 * \brief C API: Format combinations of measurement units and numeric values.
 *
 * This is a somewhat temporary Apple-specific wrapper for using C++ MeasureFormat
 * to format Measure objects, until the official ICU C API is available.
 */

/**
 * Opaque UAMeasureFormat object for use in C programs.
 * @draft ICU 53
 */
struct UAMeasureFormat;
typedef struct UAMeasureFormat UAMeasureFormat;  /**< C typedef for struct UAMeasureFormat. @draft ICU 53 */

/**
 * Constants for various widths.
 * @draft ICU 53
 */
typedef enum UAMeasureFormatWidth {
    /**
     * Full unit names, e.g. "5 hours, 37 minutes"
     * @draft ICU 53 
     */
    UAMEASFMT_WIDTH_WIDE,
 
    /**
     * Abbreviated unit names, e.g. "5 hr, 37 min"
     * @draft ICU 53
     */
    UAMEASFMT_WIDTH_SHORT,

    /**
     * Use unit symbols if possible, e.g. "5h 37m"
     * @draft ICU 53
     */
    UAMEASFMT_WIDTH_NARROW,

    /**
     * Completely omit unit designatins if possible, e.g. "5:37"
     * @draft ICU 53
     */
    UAMEASFMT_WIDTH_NUMERIC,

    /**
     * Shorter, between SHORT and NARROW, e.g. "5hr 37min"
     * @draft ICU 57
     */
    UAMEASFMT_WIDTH_SHORTER,

    /**
     * Count of values in this enum.
     * @draft ICU 53
     */
    UAMEASFMT_WIDTH_COUNT
} UAMeasureFormatWidth;

/**
 * Measurement units
 * @draft ICU 54
 */
typedef enum UAMeasureUnit {
    UAMEASUNIT_ACCELERATION_G_FORCE = (0 << 8) + 0,
    UAMEASUNIT_ACCELERATION_METER_PER_SECOND_SQUARED = (0 << 8) + 1, // (CLDR 26, ICU-541)
    //
    UAMEASUNIT_ANGLE_DEGREE         = (1 << 8) + 0,
    UAMEASUNIT_ANGLE_ARC_MINUTE     = (1 << 8) + 1,
    UAMEASUNIT_ANGLE_ARC_SECOND     = (1 << 8) + 2,
    UAMEASUNIT_ANGLE_RADIAN         = (1 << 8) + 3,     // (CLDR 26, ICU-541)
    UAMEASUNIT_ANGLE_REVOLUTION     = (1 << 8) + 4,     // (CLDR 28, ICU-561.3)
    //
    UAMEASUNIT_AREA_SQUARE_METER     = (2 << 8) + 0,
    UAMEASUNIT_AREA_SQUARE_KILOMETER = (2 << 8) + 1,
    UAMEASUNIT_AREA_SQUARE_FOOT      = (2 << 8) + 2,
    UAMEASUNIT_AREA_SQUARE_MILE      = (2 << 8) + 3,
    UAMEASUNIT_AREA_ACRE             = (2 << 8) + 4,
    UAMEASUNIT_AREA_HECTARE          = (2 << 8) + 5,
    UAMEASUNIT_AREA_SQUARE_CENTIMETER = (2 << 8) + 6,   // (CLDR 26, ICU-541)
    UAMEASUNIT_AREA_SQUARE_INCH      = (2 << 8) + 7,    // (CLDR 26, ICU-541)
    UAMEASUNIT_AREA_SQUARE_YARD      = (2 << 8) + 8,    // (CLDR 26, ICU-541)
    UAMEASUNIT_AREA_DUNAM            = (2 << 8) + 9,    // (CLDR 35, ICU-641)
    //
    // (3 reserved for currency, handled separately)
    //
    UAMEASUNIT_DURATION_YEAR        = (4 << 8) + 0,
    UAMEASUNIT_DURATION_MONTH       = (4 << 8) + 1,
    UAMEASUNIT_DURATION_WEEK        = (4 << 8) + 2,
    UAMEASUNIT_DURATION_DAY         = (4 << 8) + 3,
    UAMEASUNIT_DURATION_HOUR        = (4 << 8) + 4,
    UAMEASUNIT_DURATION_MINUTE      = (4 << 8) + 5,
    UAMEASUNIT_DURATION_SECOND      = (4 << 8) + 6,
    UAMEASUNIT_DURATION_MILLISECOND = (4 << 8) + 7,
    UAMEASUNIT_DURATION_MICROSECOND = (4 << 8) + 8,     // (CLDR 26, ICU-541)
    UAMEASUNIT_DURATION_NANOSECOND  = (4 << 8) + 9,     // (CLDR 26, ICU-541)
    UAMEASUNIT_DURATION_CENTURY     = (4 << 8) + 10,    // (CLDR 28, ICU-561.3)
    UAMEASUNIT_DURATION_YEAR_PERSON = (4 << 8) + 11,    // (CLDR 35, ICU-641)
    UAMEASUNIT_DURATION_MONTH_PERSON = (4 << 8) + 12,   // (CLDR 35, ICU-641)
    UAMEASUNIT_DURATION_WEEK_PERSON = (4 << 8) + 13,    // (CLDR 35, ICU-641)
    UAMEASUNIT_DURATION_DAY_PERSON  = (4 << 8) + 14,    // (CLDR 35, ICU-641)
    //
    UAMEASUNIT_LENGTH_METER         = (5 << 8) + 0,
    UAMEASUNIT_LENGTH_CENTIMETER    = (5 << 8) + 1,
    UAMEASUNIT_LENGTH_KILOMETER     = (5 << 8) + 2,
    UAMEASUNIT_LENGTH_MILLIMETER    = (5 << 8) + 3,
    UAMEASUNIT_LENGTH_PICOMETER     = (5 << 8) + 4,
    UAMEASUNIT_LENGTH_FOOT          = (5 << 8) + 5,
    UAMEASUNIT_LENGTH_INCH          = (5 << 8) + 6,
    UAMEASUNIT_LENGTH_MILE          = (5 << 8) + 7,
    UAMEASUNIT_LENGTH_YARD          = (5 << 8) + 8,
    UAMEASUNIT_LENGTH_LIGHT_YEAR    = (5 << 8) + 9,
    UAMEASUNIT_LENGTH_DECIMETER     = (5 << 8) + 10,    // (CLDR 26, ICU-541)
    UAMEASUNIT_LENGTH_MICROMETER    = (5 << 8) + 11,    // (CLDR 26, ICU-541)
    UAMEASUNIT_LENGTH_NANOMETER     = (5 << 8) + 12,    // (CLDR 26, ICU-541)
    UAMEASUNIT_LENGTH_NAUTICAL_MILE = (5 << 8) + 13,    // (CLDR 26, ICU-541)
    UAMEASUNIT_LENGTH_FATHOM        = (5 << 8) + 14,    // (CLDR 26, ICU-541)
    UAMEASUNIT_LENGTH_FURLONG       = (5 << 8) + 15,    // (CLDR 26, ICU-541)
    UAMEASUNIT_LENGTH_ASTRONOMICAL_UNIT = (5 << 8) + 16, // (CLDR 26, ICU-541)
    UAMEASUNIT_LENGTH_PARSEC        = (5 << 8) + 17,    // (CLDR 26, ICU-541)
    UAMEASUNIT_LENGTH_MILE_SCANDINAVIAN = (5 << 8) + 18, // (CLDR 28, ICU-561.3)
    UAMEASUNIT_LENGTH_POINT         = (5 << 8) + 19,    // (CLDR 31, ICU-590)
    UAMEASUNIT_LENGTH_SOLAR_RADIUS  = (5 << 8) + 20,    // (CLDR 35, ICU-641)
    //
    UAMEASUNIT_MASS_GRAM            = (6 << 8) + 0,
    UAMEASUNIT_MASS_KILOGRAM        = (6 << 8) + 1,
    UAMEASUNIT_MASS_OUNCE           = (6 << 8) + 2,
    UAMEASUNIT_MASS_POUND           = (6 << 8) + 3,
    UAMEASUNIT_MASS_STONE           = (6 << 8) + 4,     // = 14 pounds / 6.35 kg, abbr "st", used in UK/Ireland for body weight (CLDR 26)
    UAMEASUNIT_MASS_MICROGRAM       = (6 << 8) + 5,     // (CLDR 26, ICU-541)
    UAMEASUNIT_MASS_MILLIGRAM       = (6 << 8) + 6,     // (CLDR 26, ICU-541)
    UAMEASUNIT_MASS_METRIC_TON      = (6 << 8) + 7,     // = "tonne" (CLDR 26, ICU-541)
    UAMEASUNIT_MASS_TON             = (6 << 8) + 8,     // = "short ton", U.S. ton (CLDR 26, ICU-541)
    UAMEASUNIT_MASS_CARAT           = (6 << 8) + 9,     // (CLDR 26, ICU-541)
    UAMEASUNIT_MASS_OUNCE_TROY      = (6 << 8) + 10,    // (CLDR 26, ICU-541)
    UAMEASUNIT_MASS_DALTON          = (6 << 8) + 11,    // (CLDR 35, ICU-641)
    UAMEASUNIT_MASS_EARTH_MASS      = (6 << 8) + 12,    // (CLDR 35, ICU-641)
    UAMEASUNIT_MASS_SOLAR_MASS      = (6 << 8) + 13,    // (CLDR 35, ICU-641)
    //
    UAMEASUNIT_POWER_WATT           = (7 << 8) + 0,
    UAMEASUNIT_POWER_KILOWATT       = (7 << 8) + 1,
    UAMEASUNIT_POWER_HORSEPOWER     = (7 << 8) + 2,
    UAMEASUNIT_POWER_MILLIWATT      = (7 << 8) + 3,     // (CLDR 26, ICU-541)
    UAMEASUNIT_POWER_MEGAWATT       = (7 << 8) + 4,     // (CLDR 26, ICU-541)
    UAMEASUNIT_POWER_GIGAWATT       = (7 << 8) + 5,     // (CLDR 26, ICU-541)
    //
    UAMEASUNIT_PRESSURE_HECTOPASCAL = (8 << 8) + 0,
    UAMEASUNIT_PRESSURE_INCH_HG     = (8 << 8) + 1,
    UAMEASUNIT_PRESSURE_MILLIBAR    = (8 << 8) + 2,
    UAMEASUNIT_PRESSURE_MILLIMETER_OF_MERCURY = (8 << 8) + 3, // (CLDR 26, ICU-541)
    UAMEASUNIT_PRESSURE_POUND_PER_SQUARE_INCH = (8 << 8) + 4, // (CLDR 26, ICU-541)
    UAMEASUNIT_PRESSURE_ATMOSPHERE      = (8 << 8) + 5,     // (CLDR 34, ICU-631)
    UAMEASUNIT_PRESSURE_KILOPASCAL      = (8 << 8) + 6,     // (CLDR 35, ICU-641)
    UAMEASUNIT_PRESSURE_MEGAPASCAL      = (8 << 8) + 7,     // (CLDR 35, ICU-641)
    //
    UAMEASUNIT_SPEED_METER_PER_SECOND   = (9 << 8) + 0,
    UAMEASUNIT_SPEED_KILOMETER_PER_HOUR = (9 << 8) + 1,
    UAMEASUNIT_SPEED_MILE_PER_HOUR      = (9 << 8) + 2,
    UAMEASUNIT_SPEED_KNOT               = (9 << 8) + 3,     // (CLDR 28, ICU-561.3)
    //
    UAMEASUNIT_TEMPERATURE_CELSIUS      = (10 << 8) + 0,
    UAMEASUNIT_TEMPERATURE_FAHRENHEIT   = (10 << 8) + 1,
    UAMEASUNIT_TEMPERATURE_KELVIN       = (10 << 8) + 2,    // (CLDR 26, ICU-541)
    UAMEASUNIT_TEMPERATURE_GENERIC      = (10 << 8) + 3,    // (CLDR 27, ICU-550.2)
    //
    UAMEASUNIT_VOLUME_LITER             = (11 << 8) + 0,
    UAMEASUNIT_VOLUME_CUBIC_KILOMETER   = (11 << 8) + 1,
    UAMEASUNIT_VOLUME_CUBIC_MILE        = (11 << 8) + 2,
    UAMEASUNIT_VOLUME_MILLILITER        = (11 << 8) + 3,    // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_CENTILITER        = (11 << 8) + 4,    // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_DECILITER         = (11 << 8) + 5,    // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_HECTOLITER        = (11 << 8) + 6,    // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_MEGALITER         = (11 << 8) + 7,    // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_CUBIC_CENTIMETER  = (11 << 8) + 8,    // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_CUBIC_METER       = (11 << 8) + 9,    // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_CUBIC_INCH        = (11 << 8) + 10,   // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_CUBIC_FOOT        = (11 << 8) + 11,   // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_CUBIC_YARD        = (11 << 8) + 12,   // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_ACRE_FOOT         = (11 << 8) + 13,   // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_BUSHEL            = (11 << 8) + 14,   // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_TEASPOON          = (11 << 8) + 15,   // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_TABLESPOON        = (11 << 8) + 16,   // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_FLUID_OUNCE       = (11 << 8) + 17,   // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_CUP               = (11 << 8) + 18,   // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_PINT              = (11 << 8) + 19,   // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_QUART             = (11 << 8) + 20,   // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_GALLON            = (11 << 8) + 21,   // (CLDR 26, ICU-541)
    UAMEASUNIT_VOLUME_CUP_METRIC        = (11 << 8) + 22,   // (CLDR 28, ICU-561.3)
    UAMEASUNIT_VOLUME_PINT_METRIC       = (11 << 8) + 23,   // (CLDR 28, ICU-561.3)
    UAMEASUNIT_VOLUME_GALLON_IMPERIAL   = (11 << 8) + 24,   // (CLDR 29, ICU-561.8+)
    UAMEASUNIT_VOLUME_FLUID_OUNCE_IMPERIAL = (11 << 8) + 25, // (CLDR 35, ICU-641)
    UAMEASUNIT_VOLUME_BARREL            = (11 << 8) + 26,   // (CLDR 35, ICU-641)
    //
    // new categories/values in CLDR 26
    //
    UAMEASUNIT_ENERGY_JOULE             = (12 << 8) + 2,
    UAMEASUNIT_ENERGY_KILOJOULE         = (12 << 8) + 4,
    UAMEASUNIT_ENERGY_CALORIE           = (12 << 8) + 0,    // chemistry "calories", abbr "cal"
    UAMEASUNIT_ENERGY_KILOCALORIE       = (12 << 8) + 3,    // kilocalories in general (chemistry, food), abbr "kcal"
    UAMEASUNIT_ENERGY_FOODCALORIE       = (12 << 8) + 1,    // kilocalories specifically for food; in US/UK/? "Calories" abbr "C", elsewhere same as "kcal"
    UAMEASUNIT_ENERGY_KILOWATT_HOUR     = (12 << 8) + 5,    // (ICU-541)
    UAMEASUNIT_ENERGY_ELECTRONVOLT      = (12 << 8) + 6,    // (CLDR 35, ICU-641)
    UAMEASUNIT_ENERGY_BRITISH_THERMAL_UNIT = (12 << 8) + 7, // (CLDR 35, ICU-641)
    //
    // new categories/values in CLDR 26 & ICU-541
    //
    UAMEASUNIT_CONSUMPTION_LITER_PER_KILOMETER = (13 << 8) + 0,
    UAMEASUNIT_CONSUMPTION_MILE_PER_GALLON     = (13 << 8) + 1,
    UAMEASUNIT_CONSUMPTION_LITER_PER_100_KILOMETERs = (13 << 8) + 2, // (CLDR 28, ICU-561.3)
    UAMEASUNIT_CONSUMPTION_MILE_PER_GALLON_IMPERIAL = (13 << 8) + 3, // (CLDR 29, ICU-561.8+)
    //
    UAMEASUNIT_DIGITAL_BIT              = (14 << 8) + 0,
    UAMEASUNIT_DIGITAL_BYTE             = (14 << 8) + 1,
    UAMEASUNIT_DIGITAL_GIGABIT          = (14 << 8) + 2,
    UAMEASUNIT_DIGITAL_GIGABYTE         = (14 << 8) + 3,
    UAMEASUNIT_DIGITAL_KILOBIT          = (14 << 8) + 4,
    UAMEASUNIT_DIGITAL_KILOBYTE         = (14 << 8) + 5,
    UAMEASUNIT_DIGITAL_MEGABIT          = (14 << 8) + 6,
    UAMEASUNIT_DIGITAL_MEGABYTE         = (14 << 8) + 7,
    UAMEASUNIT_DIGITAL_TERABIT          = (14 << 8) + 8,
    UAMEASUNIT_DIGITAL_TERABYTE         = (14 << 8) + 9,
    UAMEASUNIT_DIGITAL_PETABYTE         = (14 << 8) + 10,   // (CLDR 34, ICU-631)
    //
    UAMEASUNIT_ELECTRIC_AMPERE          = (15 << 8) + 0,
    UAMEASUNIT_ELECTRIC_MILLIAMPERE     = (15 << 8) + 1,
    UAMEASUNIT_ELECTRIC_OHM             = (15 << 8) + 2,
    UAMEASUNIT_ELECTRIC_VOLT            = (15 << 8) + 3,
    //
    UAMEASUNIT_FREQUENCY_HERTZ          = (16 << 8) + 0,
    UAMEASUNIT_FREQUENCY_KILOHERTZ      = (16 << 8) + 1,
    UAMEASUNIT_FREQUENCY_MEGAHERTZ      = (16 << 8) + 2,
    UAMEASUNIT_FREQUENCY_GIGAHERTZ      = (16 << 8) + 3,
    //
    UAMEASUNIT_LIGHT_LUX                = (17 << 8) + 0,
    UAMEASUNIT_LIGHT_SOLAR_LUMINOSITY   = (17 << 8) + 1,    // (CLDR 35, ICU-641)
    //
    // new categories/values in CLDR 29, ICU-561.8+
    //
    UAMEASUNIT_CONCENTRATION_KARAT      = (18 << 8) + 0,    // (CLDR 29, ICU-561.8+)
    UAMEASUNIT_CONCENTRATION_MILLIGRAM_PER_DECILITER = (18 << 8) + 1, // (CLDR 29, ICU-561.8+)
    UAMEASUNIT_CONCENTRATION_MILLIMOLE_PER_LITER = (18 << 8) + 2, // (CLDR 29, ICU-561.8+)
    UAMEASUNIT_CONCENTRATION_PART_PER_MILLION = (18 << 8) + 3, // (CLDR 29, ICU-561.8+)
    UAMEASUNIT_CONCENTRATION_PERCENT    = (18 << 8) + 4,    // (CLDR 34, ICU-631nn)
    UAMEASUNIT_CONCENTRATION_PERMILLE   = (18 << 8) + 5,    // (CLDR 34, ICU-631nn)
    UAMEASUNIT_CONCENTRATION_PERMYRIAD  = (18 << 8) + 6,    // (CLDR 35, ICU-641)
    UAMEASUNIT_CONCENTRATION_MOLE       = (18 << 8) + 7,    // (CLDR 35, ICU-641)
    //
    // new categories/values in CLDR 35, ICU-641+
    //
    UAMEASUNIT_FORCE_NEWTON             = (19 << 8) + 0,    // (CLDR 35, ICU-641)
    UAMEASUNIT_FORCE_POUND_FORCE        = (19 << 8) + 1,    // (CLDR 35, ICU-641)
    //
    UAMEASUNIT_TORQUE_NEWTON_METER      = (20 << 8) + 0,    // (CLDR 35, ICU-641)
    UAMEASUNIT_TORQUE_POUND_FOOT        = (20 << 8) + 1,    // (CLDR 35, ICU-641)
    //
} UAMeasureUnit;

enum {
    // Mask bit set in UFieldPosition, in addition to a UAMeasureUnit value,
    // to indicate the numeric portion of the field corresponding to the UAMeasureUnit.
    UAMEASFMT_NUMERIC_FIELD_FLAG = (1 << 30)
};

/**
 * Structure that combines value and UAMeasureUnit,
 * for use with uameasfmt_formatMultiple to specify a
 * list of value/unit combinations to format.
 * @draft ICU 54
 */
typedef struct UAMeasure {
    double          value;
    UAMeasureUnit   unit;
} UAMeasure;


/**
 * Open a new UAMeasureFormat object for a given locale using the specified width,
 * along with a number formatter (if desired) to override the default formatter
 * that would be used for the numeric part of the unit in uameasfmt_format, or the
 * numeric part of the *last unit* (only) in uameasfmt_formatMultiple. The default
 * formatter typically rounds toward 0 and has a minimum of 0 fraction digits and a
 * maximum of 3 fraction digits (i.e. it will show as many decimal places as
 * necessary up to 3, without showing trailing 0s). An alternate number formatter
 * can be used to produce (e.g.) "37.0 mins" instead of "37 mins", or
 * "5 hours, 37.2 minutes" instead of "5 hours, 37.217 minutes".
 *
 * @param locale
 *          The locale
 * @param style
 *          The width - wide, short, narrow, etc.
 * @param nfToAdopt
 *          A number formatter to set for this UAMeasureFormat object (instead of
 *          the default decimal formatter). Ownership of this UNumberFormat object
 *          will pass to the UAMeasureFormat object (the UAMeasureFormat adopts the
 *          UNumberFormat), which becomes responsible for closing it. If the caller
 *          wishes to retain ownership of the UNumberFormat object, the caller must
 *          clone it (with unum_clone) and pass the clone to
 *          uatmufmt_openWithNumberFormat. May be NULL to use the default decimal
 *          formatter.
 * @param status
 *          A pointer to a UErrorCode to receive any errors.
 * @return
 *          A pointer to a UAMeasureFormat object for the specified locale,
 *          or NULL if an error occurred.
 * @draft ICU 54
 */
U_DRAFT UAMeasureFormat* U_EXPORT2
uameasfmt_open( const char*          locale,
                UAMeasureFormatWidth width,
                UNumberFormat*       nfToAdopt,
                UErrorCode*          status );

/**
 * Close a UAMeasureFormat object. Once closed it may no longer be used.
 * @param measfmt
 *            The UATimeUnitFormat object to close.
 * @draft ICU 54
 */
U_DRAFT void U_EXPORT2
uameasfmt_close(UAMeasureFormat *measfmt);

#if U_SHOW_CPLUSPLUS_API

U_NAMESPACE_BEGIN

/**
 * \class LocalUAMeasureFormatPointer
 * "Smart pointer" class, closes a UAMeasureFormat via uameasfmt_close().
 * For most methods see the LocalPointerBase base class.
 *
 * @see LocalPointerBase
 * @see LocalPointer
 * @draft ICU 54
 */
U_DEFINE_LOCAL_OPEN_POINTER(LocalUAMeasureFormatPointer, UAMeasureFormat, uameasfmt_close);

U_NAMESPACE_END

#endif // U_SHOW_CPLUSPLUS_API


/**
 * Format a value like 1.0 and a field like UAMEASUNIT_DURATION_MINUTE to e.g. "1.0 minutes".
 *
 * @param measfmt
 *          The UAMeasureFormat object specifying the format conventions.
 * @param value
 *          The numeric value to format
 * @param unit
 *          The unit to format with the specified numeric value
 * @param result
 *          A pointer to a buffer to receive the formatted result.
 * @param resultCapacity
 *          The maximum size of result.
 * @param status
 *          A pointer to a UErrorCode to receive any errors. In case of error status,
 *          the contents of result are undefined.
 * @return
 *          The length of the formatted result; may be greater than resultCapacity,
 *          in which case an error is returned.
 * @draft ICU 54
 */
U_DRAFT int32_t U_EXPORT2
uameasfmt_format( const UAMeasureFormat* measfmt,
                  double            value,
                  UAMeasureUnit     unit,
                  UChar*            result,
                  int32_t           resultCapacity,
                  UErrorCode*       status );

/**
 * Format a value like 1.0 and a field like UAMEASUNIT_DURATION_MINUTE to e.g. "1.0 minutes",
 * and get the position in the formatted result for certain types for fields.
 *
 * @param measfmt
 *          The UAMeasureFormat object specifying the format conventions.
 * @param value
 *          The numeric value to format
 * @param unit
 *          The unit to format with the specified numeric value
 * @param result
 *          A pointer to a buffer to receive the formatted result.
 * @param resultCapacity
 *          The maximum size of result.
 * @param pos
 *          A pointer to a UFieldPosition. On input, pos->field is read; this should
 *          be a value from the UNumberFormatFields enum in unum.h. On output,
 *          pos->beginIndex and pos->endIndex indicate the beginning and ending offsets
 *          of that field in the formatted output, if relevant. This parameter may be
 *          NULL if no position information is desired.
 * @param status
 *          A pointer to a UErrorCode to receive any errors. In case of error status,
 *          the contents of result are undefined.
 * @return
 *          The length of the formatted result; may be greater than resultCapacity,
 *          in which case an error is returned.
 * @draft ICU 54
 */
U_DRAFT int32_t U_EXPORT2
uameasfmt_formatGetPosition( const UAMeasureFormat* measfmt,
                            double            value,
                            UAMeasureUnit     unit,
                            UChar*            result,
                            int32_t           resultCapacity,
                            UFieldPosition*   pos,
                            UErrorCode*       status );

/**
 * Format a list of value and unit combinations, using locale-appropriate
 * conventions for the list. Each combination is represented by a UAMeasure
 * that combines a value and unit, such as 5.3 + UAMEASUNIT_DURATION_HOUR or
 * 37.2 + UAMEASUNIT_DURATION_MINUTE. For all except the last UAMeasure in the
 * list, the numeric part will be formatted using the default formatter (zero
 * decimal places, rounds toward 0); for the last UAMeasure, the default may
 * be overriden by passing a number formatter in uameasfmt_open. The result
 * can thus be something like "5 hours, 37.2 minutes" or "5 hrs, 37.2 mins".
 *
 * @param measfmt
 *            The UAMeasureFormat object specifying the format conventions.
 * @param measures
 *            A list of UAMeasure structs each specifying a numeric value
 *            and a UAMeasureUnit.
 * @param measureCount
 *            The count of UAMeasureUnits in measures. Currently this has a limit of 8.
 * @param result
 *            A pointer to a buffer to receive the formatted result.
 * @param resultCapacity
 *            The maximum size of result.
 * @param status
 *            A pointer to a UErrorCode to receive any errors. In case of error status,
 *            the contents of result are undefined.
 * @return
 *            The length of the formatted result; may be greater than resultCapacity,
 *            in which case an error is returned.
 * @draft ICU 54
 */
U_DRAFT int32_t U_EXPORT2
uameasfmt_formatMultiple( const UAMeasureFormat* measfmt,
                          const UAMeasure*  measures,
                          int32_t           measureCount,
                          UChar*            result,
                          int32_t           resultCapacity,
                          UErrorCode*       status );

/**
 * Format a list of value and unit combinations, using locale-appropriate
 * conventions for the list. This has the same format behavior as
 * uameasfmt_formatMultiple but adds the fpositer parameter. 
 *
 * @param measfmt
 *            The UAMeasureFormat object specifying the format conventions.
 * @param measures
 *            A list of UAMeasure structs each specifying a numeric value
 *            and a UAMeasureUnit.
 * @param measureCount
 *            The count of UAMeasureUnits in measures. Currently this has a limit of 8.
 * @param result
 *            A pointer to a buffer to receive the formatted result.
 * @param resultCapacity
 *            The maximum size of result.
 * @param fpositer
 *            A pointer to a UFieldPositionIterator created by ufieldpositer_open
 *            (may be NULL if field position information is not needed). Any
 *            iteration information already present in the UFieldPositionIterator
 *            will be deleted, and the iterator will be reset to apply to the
 *            fields in the formatted string created by this function call. In the
 *            the formatted result, each unit field (unit name or symbol plus any
 *            associated numeric value) will correspond to one or two results from
 *            ufieldpositer_next. The first result returns a UAMeasureUnit value and
 *            indicates the begin and end index for the complete field. If there is
 *            a numeric value contained in the field, then a subsequent call to
 *            ufieldpositer_next returns a value with UAMEASFMT_NUMERIC_FIELD_FLAG
 *            set and the same UAMeasureUnit value in the low-order bits, and
 *            indicates the begin and end index for the numeric portion of the field.
 *            For example with the string "3 hours, dualminute" the sequence of
 *            calls to ufieldpositer_next would result in:
 *            (1) return UAMEASUNIT_DURATION_HOUR, begin index 0, end index 7
 *            (2) return UAMEASUNIT_DURATION_HOUR | UAMEASFMT_NUMERIC_FIELD_FLAG, begin index 0, end index 1
 *            (3) return UAMEASUNIT_DURATION_MINUTE, begin index 9, end index 19
 *            (4) return -1 to indicate end of interation
 * @param status
 *            A pointer to a UErrorCode to receive any errors. In case of error status,
 *            the contents of result are undefined.
 * @return
 *            The length of the formatted result; may be greater than resultCapacity,
 *            in which case an error is returned.
 * @draft ICU 58
 */
U_DRAFT int32_t U_EXPORT2
uameasfmt_formatMultipleForFields( const UAMeasureFormat* measfmt,
                                   const UAMeasure*  measures,
                                   int32_t           measureCount,
                                   UChar*            result,
                                   int32_t           resultCapacity,
                                   UFieldPositionIterator* fpositer,
                                   UErrorCode*       status );

/**
 * Get the display name for a unit, such as "minutes" or "kilometers".
 *
 * @param measfmt
 *          The UAMeasureFormat object specifying the format conventions.
 * @param unit
 *          The unit whose localized name to get
 * @param result
 *          A pointer to a buffer to receive the name.
 * @param resultCapacity
 *          The maximum size of result.
 * @param status
 *          A pointer to a UErrorCode to receive any errors. In case of error status,
 *          the contents of result are undefined.
 * @return
 *          The length of the name; may be greater than resultCapacity,
 *          in which case an error is returned.
 * @draft ICU 57
 */
U_DRAFT int32_t U_EXPORT2
uameasfmt_getUnitName( const UAMeasureFormat* measfmt,
                       UAMeasureUnit unit,
                       UChar* result,
                       int32_t resultCapacity,
                       UErrorCode* status );

/**
 * Constants for unit display name list styles
 * @draft ICU 57
 */
typedef enum UAMeasureNameListStyle {
    /**
     * Use standard (linguistic) list style, the same for all unit widths; e.g.
     *   wide:    "hours, minutes, and seconds"
     *   short:   "hours, min, and secs"
     *   narrow:  "hour, min, and sec"
     * @draft ICU 57 
     */
    UAMEASNAME_LIST_STANDARD,
 
    /**
      * Use the same list style as used by the formatted units, depends on width; e.g.
     *   wide:    "hours, minutes, seconds"
     *   short:   "hours, min, secs"
     *   narrow:  "hour min sec"
     * @draft ICU 57
     */
    UAMEASNAME_LIST_MATCHUNITS,
} UAMeasureNameListStyle;

/**
 * Get a list of display names for multiple units
 *
 * @param measfmt
 *          The UAMeasureFormat object specifying the format conventions.
 * @param units
 *          The array of unit types whose names to get.
 * @param unitCount
 *          The number of unit types in the units array.
 * @param listStyle
 *          The list style used for combining the unit names.
 * @param result
 *          A pointer to a buffer to receive the list of names.
 * @param resultCapacity
 *          The maximum size of result.
 * @param status
 *          A pointer to a UErrorCode to receive any errors. In case of error status,
 *          the contents of result are undefined.
 * @return
 *          The length of the list of names; may be greater than resultCapacity,
 *          in which case an error is returned.
 * @draft ICU 57
 */
U_DRAFT int32_t U_EXPORT2
uameasfmt_getMultipleUnitNames( const UAMeasureFormat* measfmt,
                                const UAMeasureUnit* units,
                                int32_t unitCount,
                                UAMeasureNameListStyle listStyle,
                                UChar* result,
                                int32_t resultCapacity,
                                UErrorCode* status );

/**
 * Get the units used for a particular usage. This low-level function depends
 * one some knowledge of the relevant CLDR keys. After more experience with
 * usage, enums for relevant usage values may be created.
 *
 * This is sensitive to two locale keywords.
 * If the "ms" keyword is present, then the measurement system specified by its
 * value is used (except for certain categories like duration and concentr).
 * Else if the "rg" keyword is present, then the region specified by its value
 * determines the unit usage.
 * Else if the locale has a region subtag, it determines the unit usage.
 * Otherwise the likely region for the language determines the usage.
 *
 * @param locale
 *          The locale, which determines the usage as specified above.
 * @param category
 *          A string representing the CLDR category key for the desired usage,
 *          such as "length" or "mass". Must not be NULL.
 * @param usage
 *          A string representing the CLDR usage subkey for the desired usage,
 *          such as "person", "person-small" (for infants), "person-informal"
 *          (for conversational/informal usage), etc. To get the general unit
 *          for the category (not for a specific usage), this may be NULL, or
 *          may be just "large" or "small" to indicate a variant of the general
 *          unit for larger or smaller ranges than normal.
 * @param units
 *          Array to be filled in with UAMeasureUnit values; the size is
 *          specified by unitsCapacity (which in general should be at least 3).
 *          The number of array elements actually filled in is indicated by
 *          the return value; if no error status is set then this will be
 *          non-zero.
 *
 *          If the return value is positive then units represents an ordered
 *          list of one or more units that should be used in combination for
 *          the desired usage (e.g. the values UAMEASUNIT_LENGTH_FOOT,
 *          UAMEASUNIT_LENGTH_INCH to indicate a height expressed as a
 *          combination of feet and inches, or just UAMEASUNIT_LENGTH_CENTIMETER
 *          to indicate height expressed in centimeters alone).
 *
 *          Negative return values may be used for future uses (such as
 *          indicating an X-per-Y relationship among the returned units).
 *
 *          The units parameter may be NULL if unitsCapacity is 0, for
 *          pre-flighting (i.e. to determine the size of the units array that
 *          woud be required for the given category and usage).
 * @param unitsCapacity
 *          The maximum capacity of the passed-in units array.
 * @param status
 *          A pointer to a UErrorCode to receive any errors. In case of error status,
 *          the contents of result are undefined.
 * @return
 *          Positive values indicate the number of units require for the usage;
 *          may be greater than resultCapacity, in which case an error is returned.
 *          If no error, than this number of units are actually provided in the
 *          units array. Negative return values are reserved for future uses.
 * @draft ICU 57
 */
U_DRAFT int32_t U_EXPORT2
uameasfmt_getUnitsForUsage( const char*     locale,
                            const char*     category,
                            const char*     usage,
                            UAMeasureUnit*  units,
                            int32_t         unitsCapacity,
                            UErrorCode*     status );

/**
 * Get the (non-localized) category name for a unit. For example, for
 * UAMEASUNIT_VOLUME_LITER, returns "volume".
 *
 * @param unit
 *          The unit whose category name to get
 * @param status
 *          A pointer to a UErrorCode to receive any errors. In case of error status,
 *          the return value is undefined.
 * @return
 *          Pointer to a zero-terminated string giving the
 *          (non-localized) category name.
 * @draft ICU 58
 */
U_DRAFT const char * U_EXPORT2
uameasfmt_getUnitCategory(UAMeasureUnit unit,
                          UErrorCode* status );


#endif /* U_HIDE_DRAFT_API */
#endif /* #if !UCONFIG_NO_FORMATTING */

#endif /* #ifndef UAMEASUREFORMAT_H */

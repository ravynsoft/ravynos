// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
**********************************************************************
* Copyright (c) 2004-2016, International Business Machines
* Corporation and others.  All Rights Reserved.
**********************************************************************
* Author: Alan Liu
* Created: April 20, 2004
* Since: ICU 3.0
**********************************************************************
*/
#include "utypeinfo.h"  // for 'typeid' to work
#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/measfmt.h"
#include "unicode/numfmt.h"
#include "currfmt.h"
#include "unicode/localpointer.h"
#include "resource.h"
#include "unicode/simpleformatter.h"
#include "quantityformatter.h"
#include "unicode/plurrule.h"
#include "unicode/decimfmt.h"
#include "uresimp.h"
#include "unicode/ures.h"
#include "unicode/ustring.h"
#include "ureslocs.h"
#include "cstring.h"
#include "mutex.h"
#include "ucln_in.h"
#include "unicode/listformatter.h"
#include "charstr.h"
#include "unicode/putil.h"
#include "unicode/smpdtfmt.h"
#include "uassert.h"
#include "unicode/numberformatter.h"
#include "number_longnames.h"
// Apple-specific
#include "unicode/uameasureformat.h"
#include "fphdlimp.h"

#include "sharednumberformat.h"
#include "sharedpluralrules.h"
#include "standardplural.h"
#include "unifiedcache.h"


U_NAMESPACE_BEGIN

static constexpr int32_t WIDTH_INDEX_COUNT = UMEASFMT_WIDTH_NARROW + 1;

UOBJECT_DEFINE_RTTI_IMPLEMENTATION(MeasureFormat)

// Used to format durations like 5:47 or 21:35:42.
class NumericDateFormatters : public UMemory {
public:
    // Formats like H:mm
    SimpleDateFormat hourMinute;

    // formats like M:ss
    SimpleDateFormat minuteSecond;

    // formats like H:mm:ss
    SimpleDateFormat hourMinuteSecond;

    // Constructor that takes the actual patterns for hour-minute,
    // minute-second, and hour-minute-second respectively.
    NumericDateFormatters(
            const UnicodeString &hm,
            const UnicodeString &ms,
            const UnicodeString &hms,
            UErrorCode &status) : 
            hourMinute(hm, status),
            minuteSecond(ms, status), 
            hourMinuteSecond(hms, status) {
        const TimeZone *gmt = TimeZone::getGMT();
        hourMinute.setTimeZone(*gmt);
        minuteSecond.setTimeZone(*gmt);
        hourMinuteSecond.setTimeZone(*gmt);
    }
private:
    NumericDateFormatters(const NumericDateFormatters &other);
    NumericDateFormatters &operator=(const NumericDateFormatters &other);
};

static UMeasureFormatWidth getRegularWidth(UMeasureFormatWidth width) {
    if (width >= WIDTH_INDEX_COUNT) {
        return UMEASFMT_WIDTH_NARROW;
    }
    return width;
}

static UNumberUnitWidth getUnitWidth(UMeasureFormatWidth width) {
    switch (width) {
    case UMEASFMT_WIDTH_WIDE:
        return UNUM_UNIT_WIDTH_FULL_NAME;
    case UMEASFMT_WIDTH_NARROW:
    case UMEASFMT_WIDTH_NUMERIC:
        return UNUM_UNIT_WIDTH_NARROW;
    case UMEASFMT_WIDTH_SHORT:
    default:
        return UNUM_UNIT_WIDTH_SHORT;
    }
}

/**
 * Instances contain all MeasureFormat specific data for a particular locale.
 * This data is cached. It is never copied, but is shared via shared pointers.
 *
 * Note: We might change the cache data to have an array[WIDTH_INDEX_COUNT] of
 * complete sets of unit & per patterns,
 * to correspond to the resource data and its aliases.
 *
 * TODO: Maybe store more sparsely in general, with pointers rather than potentially-empty objects.
 */
class MeasureFormatCacheData : public SharedObject {
public:

    /**
     * Redirection data from root-bundle, top-level sideways aliases.
     * - UMEASFMT_WIDTH_COUNT: initial value, just fall back to root
     * - UMEASFMT_WIDTH_WIDE/SHORT/NARROW: sideways alias for missing data
     */
    UMeasureFormatWidth widthFallback[WIDTH_INDEX_COUNT];

    MeasureFormatCacheData();
    virtual ~MeasureFormatCacheData();

    void adoptCurrencyFormat(int32_t widthIndex, NumberFormat *nfToAdopt) {
        delete currencyFormats[widthIndex];
        currencyFormats[widthIndex] = nfToAdopt;
    }
    const NumberFormat *getCurrencyFormat(UMeasureFormatWidth width) const {
        return currencyFormats[getRegularWidth(width)];
    }
    void adoptIntegerFormat(NumberFormat *nfToAdopt) {
        delete integerFormat;
        integerFormat = nfToAdopt;
    }
    const NumberFormat *getIntegerFormat() const {
        return integerFormat;
    }
    void adoptNumericDateFormatters(NumericDateFormatters *formattersToAdopt) {
        delete numericDateFormatters;
        numericDateFormatters = formattersToAdopt;
    }
    const NumericDateFormatters *getNumericDateFormatters() const {
        return numericDateFormatters;
    }

private:
    NumberFormat* currencyFormats[WIDTH_INDEX_COUNT];
    NumberFormat* integerFormat;
    NumericDateFormatters* numericDateFormatters;

    MeasureFormatCacheData(const MeasureFormatCacheData &other);
    MeasureFormatCacheData &operator=(const MeasureFormatCacheData &other);
};

MeasureFormatCacheData::MeasureFormatCacheData()
        : integerFormat(nullptr), numericDateFormatters(nullptr) {
    for (int32_t i = 0; i < WIDTH_INDEX_COUNT; ++i) {
        widthFallback[i] = UMEASFMT_WIDTH_COUNT;
    }
    memset(currencyFormats, 0, sizeof(currencyFormats));
}

MeasureFormatCacheData::~MeasureFormatCacheData() {
    for (int32_t i = 0; i < UPRV_LENGTHOF(currencyFormats); ++i) {
        delete currencyFormats[i];
    }
    // Note: the contents of 'dnams' are pointers into the resource bundle
    delete integerFormat;
    delete numericDateFormatters;
}

static UBool isCurrency(const MeasureUnit &unit) {
    return (uprv_strcmp(unit.getType(), "currency") == 0);
}

static UBool getString(
        const UResourceBundle *resource,
        UnicodeString &result,
        UErrorCode &status) {
    int32_t len = 0;
    const UChar *resStr = ures_getString(resource, &len, &status);
    if (U_FAILURE(status)) {
        return FALSE;
    }
    result.setTo(TRUE, resStr, len);
    return TRUE;
}

static const UAMeasureUnit indexToUAMsasUnit[] = {
    // UAMeasureUnit                                  // UAMeasUnit vals # MeasUnit.getIndex()
    //                                                                   # --- acceleration (0)
    UAMEASUNIT_ACCELERATION_G_FORCE,                  // (0 << 8) + 0,   # 0   g-force
    UAMEASUNIT_ACCELERATION_METER_PER_SECOND_SQUARED, // (0 << 8) + 1,   # 1   meter-per-second-squared
    //                                                                   # --- angle (2)
    UAMEASUNIT_ANGLE_ARC_MINUTE,                      // (1 << 8) + 1,   # 2   arc-minute
    UAMEASUNIT_ANGLE_ARC_SECOND,                      // (1 << 8) + 2,   # 3   arc-second
    UAMEASUNIT_ANGLE_DEGREE,                          // (1 << 8) + 0,   # 4   degree
    UAMEASUNIT_ANGLE_RADIAN,                          // (1 << 8) + 3,   # 5   radian
    UAMEASUNIT_ANGLE_REVOLUTION,                      // (1 << 8) + 4,   # 6   revolution
    //                                                                   # --- area (7)
    UAMEASUNIT_AREA_ACRE,                             // (2 << 8) + 4,   # 7   acre
    UAMEASUNIT_AREA_DUNAM,                            // (2 << 8) + 9,   # 8   dunam
    UAMEASUNIT_AREA_HECTARE,                          // (2 << 8) + 5,   # 9   hectare
    UAMEASUNIT_AREA_SQUARE_CENTIMETER,                // (2 << 8) + 6,   # 10  square-centimeter
    UAMEASUNIT_AREA_SQUARE_FOOT,                      // (2 << 8) + 2,   # 11  square-foot
    UAMEASUNIT_AREA_SQUARE_INCH,                      // (2 << 8) + 7,   # 12  square-inch
    UAMEASUNIT_AREA_SQUARE_KILOMETER,                 // (2 << 8) + 1,   # 13  square-kilometer
    UAMEASUNIT_AREA_SQUARE_METER,                     // (2 << 8) + 0,   # 14  square-meter
    UAMEASUNIT_AREA_SQUARE_MILE,                      // (2 << 8) + 3,   # 15  square-mile
    UAMEASUNIT_AREA_SQUARE_YARD,                      // (2 << 8) + 8,   # 16  square-yard
    //                                                                   # --- concentr (17)
    UAMEASUNIT_CONCENTRATION_KARAT,                   // (18 << 8) + 0,  # 17  karat
    UAMEASUNIT_CONCENTRATION_MILLIGRAM_PER_DECILITER, // (18 << 8) + 1,  # 18  milligram-per-deciliter
    UAMEASUNIT_CONCENTRATION_MILLIMOLE_PER_LITER,     // (18 << 8) + 2,  # 19  millimole-per-liter
    UAMEASUNIT_CONCENTRATION_MOLE,                    // (18 << 8) + 7,  # 20  mole
    UAMEASUNIT_CONCENTRATION_PART_PER_MILLION,        // (18 << 8) + 3,  # 21  part-per-million
    UAMEASUNIT_CONCENTRATION_PERCENT,                 // (18 << 8) + 4,  # 22  percent
    UAMEASUNIT_CONCENTRATION_PERMILLE,                // (18 << 8) + 5,  # 23  permille
    UAMEASUNIT_CONCENTRATION_PERMYRIAD,               // (18 << 8) + 6,  # 24  permyriad
    //                                                                   # --- consumption (25)
    UAMEASUNIT_CONSUMPTION_LITER_PER_100_KILOMETERs,  // (13 << 8) + 2,  # 25  liter-per-100kilometers
    UAMEASUNIT_CONSUMPTION_LITER_PER_KILOMETER,       // (13 << 8) + 0,  # 26  liter-per-kilometer
    UAMEASUNIT_CONSUMPTION_MILE_PER_GALLON,           // (13 << 8) + 1,  # 27  mile-per-gallon
    UAMEASUNIT_CONSUMPTION_MILE_PER_GALLON_IMPERIAL,  // (13 << 8) + 3,  # 28  mile-per-gallon-imperial
    //                                                                   # --- currency (29)
    //                                                                   # --- digital (29)
    UAMEASUNIT_DIGITAL_BIT,                           // (14 << 8) + 0,  # 29  bit
    UAMEASUNIT_DIGITAL_BYTE,                          // (14 << 8) + 1,  # 30  byte
    UAMEASUNIT_DIGITAL_GIGABIT,                       // (14 << 8) + 2,  # 31  gigabit
    UAMEASUNIT_DIGITAL_GIGABYTE,                      // (14 << 8) + 3,  # 32  gigabyte
    UAMEASUNIT_DIGITAL_KILOBIT,                       // (14 << 8) + 4,  # 33  kilobit
    UAMEASUNIT_DIGITAL_KILOBYTE,                      // (14 << 8) + 5,  # 34  kilobyte
    UAMEASUNIT_DIGITAL_MEGABIT,                       // (14 << 8) + 6,  # 35  megabit
    UAMEASUNIT_DIGITAL_MEGABYTE,                      // (14 << 8) + 7,  # 36  megabyte
    UAMEASUNIT_DIGITAL_PETABYTE,                      // (14 << 8) + 10, # 37  petabyte
    UAMEASUNIT_DIGITAL_TERABIT,                       // (14 << 8) + 8,  # 38  terabit
    UAMEASUNIT_DIGITAL_TERABYTE,                      // (14 << 8) + 9,  # 39  terabyte
    //                                                                   # --- duration (40)
    UAMEASUNIT_DURATION_CENTURY,                      // (4 << 8) + 10,  # 40  century
    UAMEASUNIT_DURATION_DAY,                          // (4 << 8) + 3,   # 41  day
    UAMEASUNIT_DURATION_DAY_PERSON,                   // (4 << 8) + 14,  # 42  day-person
    UAMEASUNIT_DURATION_HOUR,                         // (4 << 8) + 4,   # 43  hour
    UAMEASUNIT_DURATION_MICROSECOND,                  // (4 << 8) + 8,   # 44  microsecond
    UAMEASUNIT_DURATION_MILLISECOND,                  // (4 << 8) + 7,   # 45  millisecond
    UAMEASUNIT_DURATION_MINUTE,                       // (4 << 8) + 5,   # 46  minute
    UAMEASUNIT_DURATION_MONTH,                        // (4 << 8) + 1,   # 47  month
    UAMEASUNIT_DURATION_MONTH_PERSON,                 // (4 << 8) + 12,  # 48  month-person
    UAMEASUNIT_DURATION_NANOSECOND,                   // (4 << 8) + 9,   # 49  nanosecond
    UAMEASUNIT_DURATION_SECOND,                       // (4 << 8) + 6,   # 50  second
    UAMEASUNIT_DURATION_WEEK,                         // (4 << 8) + 2,   # 51  week
    UAMEASUNIT_DURATION_WEEK_PERSON,                  // (4 << 8) + 13,  # 52  week-person
    UAMEASUNIT_DURATION_YEAR,                         // (4 << 8) + 0,   # 53  year
    UAMEASUNIT_DURATION_YEAR_PERSON,                  // (4 << 8) + 11,  # 54  year-person
    //                                                                   # --- electric (55)
    UAMEASUNIT_ELECTRIC_AMPERE,                       // (15 << 8) + 0,  # 55  ampere
    UAMEASUNIT_ELECTRIC_MILLIAMPERE,                  // (15 << 8) + 1,  # 56  milliampere
    UAMEASUNIT_ELECTRIC_OHM,                          // (15 << 8) + 2,  # 57  ohm
    UAMEASUNIT_ELECTRIC_VOLT,                         // (15 << 8) + 3,  # 58  volt
    //                                                                   # --- energy (59)
    UAMEASUNIT_ENERGY_BRITISH_THERMAL_UNIT,           // (12 << 8) + 7,  # 59  british-thermal-unit
    UAMEASUNIT_ENERGY_CALORIE,                        // (12 << 8) + 0,  # 60  calorie
    UAMEASUNIT_ENERGY_ELECTRONVOLT,                   // (12 << 8) + 6,  # 61  electronvolt
    UAMEASUNIT_ENERGY_FOODCALORIE,                    // (12 << 8) + 1,  # 62  foodcalorie
    UAMEASUNIT_ENERGY_JOULE,                          // (12 << 8) + 2,  # 63  joule
    UAMEASUNIT_ENERGY_KILOCALORIE,                    // (12 << 8) + 3,  # 64  kilocalorie
    UAMEASUNIT_ENERGY_KILOJOULE,                      // (12 << 8) + 4,  # 65  kilojoule
    UAMEASUNIT_ENERGY_KILOWATT_HOUR,                  // (12 << 8) + 5,  # 66  kilowatt-hour
    //                                                                   # --- force (67)
    UAMEASUNIT_FORCE_NEWTON,                          // (19 << 8) + 0,  # 67  newton
    UAMEASUNIT_FORCE_POUND_FORCE,                     // (19 << 8) + 1,  # 68  pound-force
    //                                                                   # --- frequency (69)
    UAMEASUNIT_FREQUENCY_GIGAHERTZ,                   // (16 << 8) + 3,  # 69  gigahertz
    UAMEASUNIT_FREQUENCY_HERTZ,                       // (16 << 8) + 0,  # 70  hertz
    UAMEASUNIT_FREQUENCY_KILOHERTZ,                   // (16 << 8) + 1,  # 71  kilohertz
    UAMEASUNIT_FREQUENCY_MEGAHERTZ,                   // (16 << 8) + 2,  # 72  megahertz
    //                                                                   # --- length (73)
    UAMEASUNIT_LENGTH_ASTRONOMICAL_UNIT,              // (5 << 8) + 16,  # 73  astronomical-unit
    UAMEASUNIT_LENGTH_CENTIMETER,                     // (5 << 8) + 1,   # 74  centimeter
    UAMEASUNIT_LENGTH_DECIMETER,                      // (5 << 8) + 10,  # 75  decimeter
    UAMEASUNIT_LENGTH_FATHOM,                         // (5 << 8) + 14,  # 76  fathom
    UAMEASUNIT_LENGTH_FOOT,                           // (5 << 8) + 5,   # 77  foot
    UAMEASUNIT_LENGTH_FURLONG,                        // (5 << 8) + 15,  # 78  furlong
    UAMEASUNIT_LENGTH_INCH,                           // (5 << 8) + 6,   # 79  inch
    UAMEASUNIT_LENGTH_KILOMETER,                      // (5 << 8) + 2,   # 80  kilometer
    UAMEASUNIT_LENGTH_LIGHT_YEAR,                     // (5 << 8) + 9,   # 81  light-year
    UAMEASUNIT_LENGTH_METER,                          // (5 << 8) + 0,   # 82  meter
    UAMEASUNIT_LENGTH_MICROMETER,                     // (5 << 8) + 11,  # 83  micrometer
    UAMEASUNIT_LENGTH_MILE,                           // (5 << 8) + 7,   # 84  mile
    UAMEASUNIT_LENGTH_MILE_SCANDINAVIAN,              // (5 << 8) + 18,  # 85  mile-scandinavian
    UAMEASUNIT_LENGTH_MILLIMETER,                     // (5 << 8) + 3,   # 86  millimeter
    UAMEASUNIT_LENGTH_NANOMETER,                      // (5 << 8) + 12,  # 87  nanometer
    UAMEASUNIT_LENGTH_NAUTICAL_MILE,                  // (5 << 8) + 13,  # 88  nautical-mile
    UAMEASUNIT_LENGTH_PARSEC,                         // (5 << 8) + 17,  # 89  parsec
    UAMEASUNIT_LENGTH_PICOMETER,                      // (5 << 8) + 4,   # 90  picometer
    UAMEASUNIT_LENGTH_POINT,                          // (5 << 8) + 19,  # 91  point
    UAMEASUNIT_LENGTH_SOLAR_RADIUS,                   // (5 << 8) + 20,  # 92  solar-radius
    UAMEASUNIT_LENGTH_YARD,                           // (5 << 8) + 8,   # 93  yard
    //                                                                   # --- light (94)
    UAMEASUNIT_LIGHT_LUX,                             // (17 << 8) + 0,  # 94  lux
    UAMEASUNIT_LIGHT_SOLAR_LUMINOSITY,                // (17 << 8) + 1,  # 95  solar-luminosity
    //                                                                   # --- mass (96)
    UAMEASUNIT_MASS_CARAT,                            // (6 << 8) + 9,   # 96  carat
    UAMEASUNIT_MASS_DALTON,                           // (6 << 8) + 11,  # 97  dalton
    UAMEASUNIT_MASS_EARTH_MASS,                       // (6 << 8) + 12,  # 98  earth-mass
    UAMEASUNIT_MASS_GRAM,                             // (6 << 8) + 0,   # 99  gram
    UAMEASUNIT_MASS_KILOGRAM,                         // (6 << 8) + 1,   # 100 kilogram
    UAMEASUNIT_MASS_METRIC_TON,                       // (6 << 8) + 7,   # 101 metric-ton
    UAMEASUNIT_MASS_MICROGRAM,                        // (6 << 8) + 5,   # 102 microgram
    UAMEASUNIT_MASS_MILLIGRAM,                        // (6 << 8) + 6,   # 103 milligram
    UAMEASUNIT_MASS_OUNCE,                            // (6 << 8) + 2,   # 104 ounce
    UAMEASUNIT_MASS_OUNCE_TROY,                       // (6 << 8) + 10,  # 105 ounce-troy
    UAMEASUNIT_MASS_POUND,                            // (6 << 8) + 3,   # 106 pound
    UAMEASUNIT_MASS_SOLAR_MASS,                       // (6 << 8) + 13,  # 107 solar-mass
    UAMEASUNIT_MASS_STONE,                            // (6 << 8) + 4,   # 108 stone
    UAMEASUNIT_MASS_TON,                              // (6 << 8) + 8,   # 109 ton
    //                                                                   # --- none (110)
    UAMEASUNIT_CONCENTRATION_PERCENT,                 // BOGUS           # 110 base
    UAMEASUNIT_CONCENTRATION_PERCENT,                 // BOGUS           # 111 percent
    UAMEASUNIT_CONCENTRATION_PERMILLE,                // BOGUS           # 112 permille
    //                                                                   # --- power (113)
    UAMEASUNIT_POWER_GIGAWATT,                        // (7 << 8) + 5,   # 113 gigawatt
    UAMEASUNIT_POWER_HORSEPOWER,                      // (7 << 8) + 2,   # 114 horsepower
    UAMEASUNIT_POWER_KILOWATT,                        // (7 << 8) + 1,   # 115 kilowatt
    UAMEASUNIT_POWER_MEGAWATT,                        // (7 << 8) + 4,   # 116 megawatt
    UAMEASUNIT_POWER_MILLIWATT,                       // (7 << 8) + 3,   # 117 milliwatt
    UAMEASUNIT_POWER_WATT,                            // (7 << 8) + 0,   # 118 watt
    //                                                                   # --- pressure (119)
    UAMEASUNIT_PRESSURE_ATMOSPHERE,                   // (8 << 8) + 5,   # 119 atmosphere
    UAMEASUNIT_PRESSURE_HECTOPASCAL,                  // (8 << 8) + 0,   # 120 hectopascal
    UAMEASUNIT_PRESSURE_INCH_HG,                      // (8 << 8) + 1,   # 121 inch-hg
    UAMEASUNIT_PRESSURE_KILOPASCAL,                   // (8 << 8) + 6,   # 122 kilopascal
    UAMEASUNIT_PRESSURE_MEGAPASCAL,                   // (8 << 8) + 7,   # 123 megapascal
    UAMEASUNIT_PRESSURE_MILLIBAR,                     // (8 << 8) + 2,   # 124 millibar
    UAMEASUNIT_PRESSURE_MILLIMETER_OF_MERCURY,        // (8 << 8) + 3,   # 125 millimeter-of-mercury
    UAMEASUNIT_PRESSURE_POUND_PER_SQUARE_INCH,        // (8 << 8) + 4,   # 126 pound-per-square-inch
    //                                                                   # --- speed (127)
    UAMEASUNIT_SPEED_KILOMETER_PER_HOUR,              // (9 << 8) + 1,   # 127 kilometer-per-hour
    UAMEASUNIT_SPEED_KNOT,                            // (9 << 8) + 3,   # 128 knot
    UAMEASUNIT_SPEED_METER_PER_SECOND,                // (9 << 8) + 0,   # 129 meter-per-second
    UAMEASUNIT_SPEED_MILE_PER_HOUR,                   // (9 << 8) + 2,   # 130 mile-per-hour
    //                                                                   # --- temperature (131)
    UAMEASUNIT_TEMPERATURE_CELSIUS,                   // (10 << 8) + 0,  # 131 celsius
    UAMEASUNIT_TEMPERATURE_FAHRENHEIT,                // (10 << 8) + 1,  # 132 fahrenheit
    UAMEASUNIT_TEMPERATURE_GENERIC,                   // (10 << 8) + 3,  # 133 generic
    UAMEASUNIT_TEMPERATURE_KELVIN,                    // (10 << 8) + 2,  # 134 kelvin
    //                                                                   # --- torque (135)
    UAMEASUNIT_TORQUE_NEWTON_METER,                   // (20 << 8) + 0,  # 135 newton-meter
    UAMEASUNIT_TORQUE_POUND_FOOT,                     // (20 << 8) + 1,  # 136 pound-foot
    //                                                                   # --- volume (137)
    UAMEASUNIT_VOLUME_ACRE_FOOT,                      // (11 << 8) + 13, # 137 acre-foot
    UAMEASUNIT_VOLUME_BARREL,                         // (11 << 8) + 26, # 138 barrel
    UAMEASUNIT_VOLUME_BUSHEL,                         // (11 << 8) + 14, # 139 bushel
    UAMEASUNIT_VOLUME_CENTILITER,                     // (11 << 8) + 4,  # 140 centiliter
    UAMEASUNIT_VOLUME_CUBIC_CENTIMETER,               // (11 << 8) + 8,  # 141 cubic-centimeter
    UAMEASUNIT_VOLUME_CUBIC_FOOT,                     // (11 << 8) + 11, # 142 cubic-foot
    UAMEASUNIT_VOLUME_CUBIC_INCH,                     // (11 << 8) + 10, # 143 cubic-inch
    UAMEASUNIT_VOLUME_CUBIC_KILOMETER,                // (11 << 8) + 1,  # 144 cubic-kilometer
    UAMEASUNIT_VOLUME_CUBIC_METER,                    // (11 << 8) + 9,  # 145 cubic-meter
    UAMEASUNIT_VOLUME_CUBIC_MILE,                     // (11 << 8) + 2,  # 146 cubic-mile
    UAMEASUNIT_VOLUME_CUBIC_YARD,                     // (11 << 8) + 12, # 147 cubic-yard
    UAMEASUNIT_VOLUME_CUP,                            // (11 << 8) + 18, # 148 cup
    UAMEASUNIT_VOLUME_CUP_METRIC,                     // (11 << 8) + 22, # 149 cup-metric
    UAMEASUNIT_VOLUME_DECILITER,                      // (11 << 8) + 5,  # 150 deciliter
    UAMEASUNIT_VOLUME_FLUID_OUNCE,                    // (11 << 8) + 17, # 151 fluid-ounce
    UAMEASUNIT_VOLUME_FLUID_OUNCE_IMPERIAL,           // (11 << 8) + 25, # 152 fluid-ounce-imperial
    UAMEASUNIT_VOLUME_GALLON,                         // (11 << 8) + 21, # 153 gallon
    UAMEASUNIT_VOLUME_GALLON_IMPERIAL,                // (11 << 8) + 24, # 154 gallon-imperial
    UAMEASUNIT_VOLUME_HECTOLITER,                     // (11 << 8) + 6,  # 155 hectoliter
    UAMEASUNIT_VOLUME_LITER,                          // (11 << 8) + 0,  # 156 liter
    UAMEASUNIT_VOLUME_MEGALITER,                      // (11 << 8) + 7,  # 157 megaliter
    UAMEASUNIT_VOLUME_MILLILITER,                     // (11 << 8) + 3,  # 158 milliliter
    UAMEASUNIT_VOLUME_PINT,                           // (11 << 8) + 19, # 159 pint
    UAMEASUNIT_VOLUME_PINT_METRIC,                    // (11 << 8) + 23, # 160 pint-metric
    UAMEASUNIT_VOLUME_QUART,                          // (11 << 8) + 20, # 161 quart
    UAMEASUNIT_VOLUME_TABLESPOON,                     // (11 << 8) + 16, # 162 tablespoon
    UAMEASUNIT_VOLUME_TEASPOON,                       // (11 << 8) + 15, # 163 teaspoon
};

static UnicodeString loadNumericDateFormatterPattern(
        const UResourceBundle *resource,
        const char *pattern,
        UErrorCode &status) {
    UnicodeString result;
    if (U_FAILURE(status)) {
        return result;
    }
    CharString chs;
    chs.append("durationUnits", status)
            .append("/", status).append(pattern, status);
    LocalUResourceBundlePointer patternBundle(
            ures_getByKeyWithFallback(
                resource,
                chs.data(),
                NULL,
                &status));
    if (U_FAILURE(status)) {
        return result;
    }
    getString(patternBundle.getAlias(), result, status);
    // Replace 'h' with 'H'
    int32_t len = result.length();
    UChar *buffer = result.getBuffer(len);
    for (int32_t i = 0; i < len; ++i) {
        if (buffer[i] == 0x68) { // 'h'
            buffer[i] = 0x48; // 'H'
        }
    }
    result.releaseBuffer(len);
    return result;
}

static NumericDateFormatters *loadNumericDateFormatters(
        const UResourceBundle *resource,
        UErrorCode &status) {
    if (U_FAILURE(status)) {
        return NULL;
    }
    NumericDateFormatters *result = new NumericDateFormatters(
        loadNumericDateFormatterPattern(resource, "hm", status),
        loadNumericDateFormatterPattern(resource, "ms", status),
        loadNumericDateFormatterPattern(resource, "hms", status),
        status);
    if (U_FAILURE(status)) {
        delete result;
        return NULL;
    }
    return result;
}

template<> U_I18N_API
const MeasureFormatCacheData *LocaleCacheKey<MeasureFormatCacheData>::createObject(
        const void * /*unused*/, UErrorCode &status) const {
    const char *localeId = fLoc.getName();
    LocalUResourceBundlePointer unitsBundle(ures_open(U_ICUDATA_UNIT, localeId, &status));
    static UNumberFormatStyle currencyStyles[] = {
            UNUM_CURRENCY_PLURAL, UNUM_CURRENCY_ISO, UNUM_CURRENCY};
    LocalPointer<MeasureFormatCacheData> result(new MeasureFormatCacheData(), status);
    if (U_FAILURE(status)) {
        return NULL;
    }
    result->adoptNumericDateFormatters(loadNumericDateFormatters(
            unitsBundle.getAlias(), status));
    if (U_FAILURE(status)) {
        return NULL;
    }

    for (int32_t i = 0; i < WIDTH_INDEX_COUNT; ++i) {
        // NumberFormat::createInstance can erase warning codes from status, so pass it
        // a separate status instance
        UErrorCode localStatus = U_ZERO_ERROR;
        result->adoptCurrencyFormat(i, NumberFormat::createInstance(
                localeId, currencyStyles[i], localStatus));
        if (localStatus != U_ZERO_ERROR) {
            status = localStatus;
        }
        if (U_FAILURE(status)) {
            return NULL;
        }
    }
    NumberFormat *inf = NumberFormat::createInstance(
            localeId, UNUM_DECIMAL, status);
    if (U_FAILURE(status)) {
        return NULL;
    }
    inf->setMaximumFractionDigits(0);
    DecimalFormat *decfmt = dynamic_cast<DecimalFormat *>(inf);
    if (decfmt != NULL) {
        decfmt->setRoundingMode(DecimalFormat::kRoundDown);
    }
    result->adoptIntegerFormat(inf);
    result->addRef();
    return result.orphan();
}

static UBool isTimeUnit(const MeasureUnit &mu, const char *tu) {
    return uprv_strcmp(mu.getType(), "duration") == 0 &&
            uprv_strcmp(mu.getSubtype(), tu) == 0;
}

// Converts a composite measure into hours-minutes-seconds and stores at hms
// array. [0] is hours; [1] is minutes; [2] is seconds. Returns a bit map of
// units found: 1=hours, 2=minutes, 4=seconds. For example, if measures
// contains hours-minutes, this function would return 3.
//
// If measures cannot be converted into hours, minutes, seconds or if amounts
// are negative, or if hours, minutes, seconds are out of order, returns 0.
static int32_t toHMS(
        const Measure *measures,
        int32_t measureCount,
        Formattable *hms,
        UErrorCode &status) {
    if (U_FAILURE(status)) {
        return 0;
    }
    int32_t result = 0;
    if (U_FAILURE(status)) {
        return 0;
    }
    // We use copy constructor to ensure that both sides of equality operator
    // are instances of MeasureUnit base class and not a subclass. Otherwise,
    // operator== will immediately return false.
    for (int32_t i = 0; i < measureCount; ++i) {
        if (isTimeUnit(measures[i].getUnit(), "hour")) {
            // hour must come first
            if (result >= 1) {
                return 0;
            }
            hms[0] = measures[i].getNumber();
            if (hms[0].getDouble() < 0.0) {
                return 0;
            }
            result |= 1;
        } else if (isTimeUnit(measures[i].getUnit(), "minute")) {
            // minute must come after hour
            if (result >= 2) {
                return 0;
            }
            hms[1] = measures[i].getNumber();
            if (hms[1].getDouble() < 0.0) {
                return 0;
            }
            result |= 2;
        } else if (isTimeUnit(measures[i].getUnit(), "second")) {
            // second must come after hour and minute
            if (result >= 4) {
                return 0;
            }
            hms[2] = measures[i].getNumber();
            if (hms[2].getDouble() < 0.0) {
                return 0;
            }
            result |= 4;
        } else {
            return 0;
        }
    }
    return result;
}


MeasureFormat::MeasureFormat(
        const Locale &locale, UMeasureFormatWidth w, UErrorCode &status)
        : cache(NULL),
          numberFormat(NULL),
          pluralRules(NULL),
          fWidth((w==UMEASFMT_WIDTH_SHORTER)? UMEASFMT_WIDTH_SHORT: w),
          stripPatternSpaces(w==UMEASFMT_WIDTH_SHORTER),
          listFormatter(NULL),
          listFormatterStd(NULL) {
    initMeasureFormat(locale, (w==UMEASFMT_WIDTH_SHORTER)? UMEASFMT_WIDTH_SHORT: w, NULL, status);
}

MeasureFormat::MeasureFormat(
        const Locale &locale,
        UMeasureFormatWidth w,
        NumberFormat *nfToAdopt,
        UErrorCode &status) 
        : cache(NULL),
          numberFormat(NULL),
          pluralRules(NULL),
          fWidth((w==UMEASFMT_WIDTH_SHORTER)? UMEASFMT_WIDTH_SHORT: w),
          stripPatternSpaces(w==UMEASFMT_WIDTH_SHORTER),
          listFormatter(NULL),
          listFormatterStd(NULL) {
    initMeasureFormat(locale, (w==UMEASFMT_WIDTH_SHORTER)? UMEASFMT_WIDTH_SHORT: w, nfToAdopt, status);
}

MeasureFormat::MeasureFormat(const MeasureFormat &other) :
        Format(other),
        cache(other.cache),
        numberFormat(other.numberFormat),
        pluralRules(other.pluralRules),
        fWidth(other.fWidth),
        stripPatternSpaces(other.stripPatternSpaces),
        listFormatter(NULL),
        listFormatterStd(NULL) {
    cache->addRef();
    numberFormat->addRef();
    pluralRules->addRef();
    if (other.listFormatter != NULL) {
        listFormatter = new ListFormatter(*other.listFormatter);
    }
    if (other.listFormatterStd != NULL) {
        listFormatterStd = new ListFormatter(*other.listFormatterStd);
    }
}

MeasureFormat &MeasureFormat::operator=(const MeasureFormat &other) {
    if (this == &other) {
        return *this;
    }
    Format::operator=(other);
    SharedObject::copyPtr(other.cache, cache);
    SharedObject::copyPtr(other.numberFormat, numberFormat);
    SharedObject::copyPtr(other.pluralRules, pluralRules);
    fWidth = other.fWidth;
    stripPatternSpaces = other.stripPatternSpaces;
    delete listFormatter;
    if (other.listFormatter != NULL) {
        listFormatter = new ListFormatter(*other.listFormatter);
    } else {
        listFormatter = NULL;
    }
    delete listFormatterStd;
    if (other.listFormatterStd != NULL) {
        listFormatterStd = new ListFormatter(*other.listFormatterStd);
    } else {
        listFormatterStd = NULL;
    }
    return *this;
}

MeasureFormat::MeasureFormat() :
        cache(NULL),
        numberFormat(NULL),
        pluralRules(NULL),
        fWidth(UMEASFMT_WIDTH_SHORT),
        stripPatternSpaces(FALSE),
        listFormatter(NULL),
        listFormatterStd(NULL) {
}

MeasureFormat::~MeasureFormat() {
    if (cache != NULL) {
        cache->removeRef();
    }
    if (numberFormat != NULL) {
        numberFormat->removeRef();
    }
    if (pluralRules != NULL) {
        pluralRules->removeRef();
    }
    delete listFormatter;
    delete listFormatterStd;
}

UBool MeasureFormat::operator==(const Format &other) const {
    if (this == &other) { // Same object, equal
        return TRUE;
    }
    if (!Format::operator==(other)) {
        return FALSE;
    }
    const MeasureFormat &rhs = static_cast<const MeasureFormat &>(other);

    // Note: Since the ListFormatter depends only on Locale and width, we
    // don't have to check it here.

    // differing widths aren't equivalent
    if (fWidth != rhs.fWidth || stripPatternSpaces != rhs.stripPatternSpaces) {
        return FALSE;
    }
    // Width the same check locales.
    // We don't need to check locales if both objects have same cache.
    if (cache != rhs.cache) {
        UErrorCode status = U_ZERO_ERROR;
        const char *localeId = getLocaleID(status);
        const char *rhsLocaleId = rhs.getLocaleID(status);
        if (U_FAILURE(status)) {
            // On failure, assume not equal
            return FALSE;
        }
        if (uprv_strcmp(localeId, rhsLocaleId) != 0) {
            return FALSE;
        }
    }
    // Locales same, check NumberFormat if shared data differs.
    return (
            numberFormat == rhs.numberFormat ||
            **numberFormat == **rhs.numberFormat);
}

Format *MeasureFormat::clone() const {
    return new MeasureFormat(*this);
}

UnicodeString &MeasureFormat::format(
        const Formattable &obj,
        UnicodeString &appendTo,
        FieldPosition &pos,
        UErrorCode &status) const {
    if (U_FAILURE(status)) return appendTo;
    if (obj.getType() == Formattable::kObject) {
        const UObject* formatObj = obj.getObject();
        const Measure* amount = dynamic_cast<const Measure*>(formatObj);
        if (amount != NULL) {
            return formatMeasure(
                    *amount, **numberFormat, appendTo, pos, status);
        }
    }
    status = U_ILLEGAL_ARGUMENT_ERROR;
    return appendTo;
}

void MeasureFormat::parseObject(
        const UnicodeString & /*source*/,
        Formattable & /*result*/,
        ParsePosition& /*pos*/) const {
    return;
}

UnicodeString &MeasureFormat::formatMeasurePerUnit(
        const Measure &measure,
        const MeasureUnit &perUnit,
        UnicodeString &appendTo,
        FieldPosition &pos,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    auto* df = dynamic_cast<const DecimalFormat*>(&getNumberFormatInternal());
    if (df == nullptr) {
        // Don't know how to handle other types of NumberFormat
        status = U_UNSUPPORTED_ERROR;
        return appendTo;
    }
    number::FormattedNumber result;
    if (auto* lnf = df->toNumberFormatter(status)) {
        result = lnf->unit(measure.getUnit())
            .perUnit(perUnit)
            .unitWidth(getUnitWidth(fWidth))
            .formatDouble(measure.getNumber().getDouble(status), status);
    }
    DecimalFormat::fieldPositionHelper(result, pos, appendTo.length(), status);
    appendTo.append(result.toTempString(status));
    return appendTo;
}

UnicodeString &MeasureFormat::formatMeasures(
        const Measure *measures,
        int32_t measureCount,
        UnicodeString &appendTo,
        FieldPosition &pos,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    if (measureCount == 0) {
        return appendTo;
    }
    if (measureCount == 1) {
        return formatMeasure(measures[0], **numberFormat, appendTo, pos, status);
    }
    if (fWidth == UMEASFMT_WIDTH_NUMERIC) {
        Formattable hms[3];
        int32_t bitMap = toHMS(measures, measureCount, hms, status);
        if (bitMap > 0) {
            FieldPositionIteratorHandler handler((FieldPositionIterator*)NULL, status);
            return formatNumeric(hms, bitMap, appendTo, handler, status);
        }
    }
    if (pos.getField() != FieldPosition::DONT_CARE) {
        return formatMeasuresSlowTrack(
                measures, measureCount, appendTo, pos, status);
    }
    UnicodeString *results = new UnicodeString[measureCount];
    if (results == NULL) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return appendTo;
    }
    for (int32_t i = 0; i < measureCount; ++i) {
        const NumberFormat *nf = cache->getIntegerFormat();
        if (i == measureCount - 1) {
            nf = numberFormat->get();
        }
        formatMeasure(
                measures[i],
                *nf,
                results[i],
                pos,
                status);
    }
    listFormatter->format(results, measureCount, appendTo, status);
    delete [] results;
    return appendTo;
}

// Apple-specific version for now;
// uses FieldPositionIterator* instead of FieldPosition&
UnicodeString &MeasureFormat::formatMeasures(
        const Measure *measures,
        int32_t measureCount,
        UnicodeString &appendTo,
        FieldPositionIterator* posIter,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    FieldPositionIteratorHandler handler(posIter, status);
    if (measureCount == 0) {
        return appendTo;
    }
    if (measureCount == 1) {
        int32_t start = appendTo.length();
        int32_t field = indexToUAMsasUnit[measures[0].getUnit().getIndex()];
        FieldPosition pos(UAMEASFMT_NUMERIC_FIELD_FLAG); // special field value to request range of entire numeric part
        formatMeasure(measures[0], **numberFormat, appendTo, pos, status);
        handler.addAttribute(field, start, appendTo.length());
        handler.addAttribute(field | UAMEASFMT_NUMERIC_FIELD_FLAG, pos.getBeginIndex(), pos.getEndIndex());
        return appendTo;
    }
    if (fWidth == UMEASFMT_WIDTH_NUMERIC) {
        Formattable hms[3];
        int32_t bitMap = toHMS(measures, measureCount, hms, status);
        if (bitMap > 0) {
            return formatNumeric(hms, bitMap, appendTo, handler, status);
        }
    }
    UnicodeString *results = new UnicodeString[measureCount];
    if (results == NULL) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return appendTo;
    }
    FieldPosition *numPositions = new FieldPosition[measureCount];
    if (results == NULL) {
        delete [] results;
        status = U_MEMORY_ALLOCATION_ERROR;
        return appendTo;
    }
    
    for (int32_t i = 0; i < measureCount; ++i) {
        const NumberFormat *nf = cache->getIntegerFormat();
        if (i == measureCount - 1) {
            nf = numberFormat->get();
        }
        numPositions[i].setField(UAMEASFMT_NUMERIC_FIELD_FLAG);
        formatMeasure(
                measures[i],
                *nf,
                results[i],
                numPositions[i],
                status);
    }
    listFormatter->format(results, measureCount, appendTo, status);
    for (int32_t i = 0; i < measureCount; ++i) {
        int32_t begin = appendTo.indexOf(results[i]);
        if (begin >= 0) {
            int32_t field = indexToUAMsasUnit[measures[i].getUnit().getIndex()];
            handler.addAttribute(field, begin, begin + results[i].length());
            int32_t numPosBegin = numPositions[i].getBeginIndex();
            int32_t numPosEnd   = numPositions[i].getEndIndex();
            if (numPosBegin >= 0 && numPosEnd > numPosBegin) {
                handler.addAttribute(field | UAMEASFMT_NUMERIC_FIELD_FLAG, begin + numPosBegin, begin + numPosEnd);
            }
        }
    }
    delete [] results;
    delete [] numPositions;
    return appendTo;
}

UnicodeString MeasureFormat::getUnitDisplayName(const MeasureUnit& unit, UErrorCode& status) const {
    return number::impl::LongNameHandler::getUnitDisplayName(
        getLocale(status),
        unit,
        getUnitWidth(fWidth),
        status);
}

void MeasureFormat::initMeasureFormat(
        const Locale &locale,
        UMeasureFormatWidth w,
        NumberFormat *nfToAdopt,
        UErrorCode &status) {
    static const char *listStyles[] = {"unit", "unit-short", "unit-narrow"};
    LocalPointer<NumberFormat> nf(nfToAdopt);
    if (U_FAILURE(status)) {
        return;
    }
    const char *name = locale.getName();
    setLocaleIDs(name, name);

    UnifiedCache::getByLocale(locale, cache, status);
    if (U_FAILURE(status)) {
        return;
    }

    const SharedPluralRules *pr = PluralRules::createSharedInstance(
            locale, UPLURAL_TYPE_CARDINAL, status);
    if (U_FAILURE(status)) {
        return;
    }
    SharedObject::copyPtr(pr, pluralRules);
    pr->removeRef();
    if (nf.isNull()) {
        // TODO: Clean this up
        const SharedNumberFormat *shared = NumberFormat::createSharedInstance(
                locale, UNUM_DECIMAL, status);
        if (U_FAILURE(status)) {
            return;
        }
        SharedObject::copyPtr(shared, numberFormat);
        shared->removeRef();
    } else {
        adoptNumberFormat(nf.orphan(), status);
        if (U_FAILURE(status)) {
            return;
        }
    }
    fWidth = w;
    if (stripPatternSpaces) {
        w = UMEASFMT_WIDTH_NARROW;
    }
    delete listFormatter;
    listFormatter = ListFormatter::createInstance(
            locale,
            listStyles[getRegularWidth(w)],
            status);
    delete listFormatterStd;
    listFormatterStd = ListFormatter::createInstance(
            locale,
            "standard",
            status);
}

void MeasureFormat::adoptNumberFormat(
        NumberFormat *nfToAdopt, UErrorCode &status) {
    LocalPointer<NumberFormat> nf(nfToAdopt);
    if (U_FAILURE(status)) {
        return;
    }
    SharedNumberFormat *shared = new SharedNumberFormat(nf.getAlias());
    if (shared == NULL) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return;
    }
    nf.orphan();
    SharedObject::copyPtr(shared, numberFormat);
}

UBool MeasureFormat::setMeasureFormatLocale(const Locale &locale, UErrorCode &status) {
    if (U_FAILURE(status) || locale == getLocale(status)) {
        return FALSE;
    }
    initMeasureFormat(locale, fWidth, NULL, status);
    return U_SUCCESS(status);
} 

// Apple-specific for now
UMeasureFormatWidth MeasureFormat::getWidth() const {
    return fWidth;
}

const NumberFormat &MeasureFormat::getNumberFormatInternal() const {
    return **numberFormat;
}

const NumberFormat &MeasureFormat::getCurrencyFormatInternal() const {
    return *cache->getCurrencyFormat(UMEASFMT_WIDTH_NARROW);
}

const PluralRules &MeasureFormat::getPluralRules() const {
    return **pluralRules;
}

Locale MeasureFormat::getLocale(UErrorCode &status) const {
    return Format::getLocale(ULOC_VALID_LOCALE, status);
}

const char *MeasureFormat::getLocaleID(UErrorCode &status) const {
    return Format::getLocaleID(ULOC_VALID_LOCALE, status);
}

// Apple=specific
// now just re-implement using standard getUnitDisplayName
// so we no longer use cache->getDisplayName
UnicodeString &MeasureFormat::getUnitName(
        const MeasureUnit* unit,
        UnicodeString &result ) const {
    UErrorCode status = U_ZERO_ERROR;
    result = getUnitDisplayName(*unit, status); // does not use or set status
    return result;
}

// Apple=specific
UnicodeString &MeasureFormat::getMultipleUnitNames(
        const MeasureUnit** units,
        int32_t unitCount,
        UAMeasureNameListStyle listStyle,
        UnicodeString &result ) const {
    if (unitCount == 0) {
        return result.remove();
    }
    if (unitCount == 1) {
        return getUnitName(units[0], result);
    }
    UnicodeString *results = new UnicodeString[unitCount];
    if (results != NULL) {
        for (int32_t i = 0; i < unitCount; ++i) {
            getUnitName(units[i], results[i]);
        }
        UErrorCode status = U_ZERO_ERROR;
        if (listStyle == UAMEASNAME_LIST_STANDARD) {
            listFormatterStd->format(results, unitCount, result, status);
        } else {
            listFormatter->format(results, unitCount, result, status);
        }
        delete [] results;
        if (U_SUCCESS(status)) {
            return result;
        }
    }
    result.setToBogus();
    return result;
}

UnicodeString &MeasureFormat::formatMeasure(
        const Measure &measure,
        const NumberFormat &nf,
        UnicodeString &appendTo,
        FieldPosition &pos,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    const Formattable& amtNumber = measure.getNumber();
    const MeasureUnit& amtUnit = measure.getUnit();
    if (isCurrency(amtUnit)) {
        UChar isoCode[4];
        u_charsToUChars(amtUnit.getSubtype(), isoCode, 4);
        return cache->getCurrencyFormat(fWidth)->format(
                new CurrencyAmount(amtNumber, isoCode, status),
                appendTo,
                pos,
                status);
    }
    int32_t cur = appendTo.length();
    UBool posForFullNumericPart = (pos.getField() == UAMEASFMT_NUMERIC_FIELD_FLAG);
    if (posForFullNumericPart) {
        pos.setField(FieldPosition::DONT_CARE);
    }

    auto* df = dynamic_cast<const DecimalFormat*>(&nf);
    if (df == nullptr) {
        // Handle other types of NumberFormat using the old code, Apple restore for <rdar://problem/49131922>
        UnicodeString formattedNumber;
        StandardPlural::Form pluralForm = QuantityFormatter::selectPlural(
                amtNumber, nf, **pluralRules, formattedNumber, pos, status);
        if (posForFullNumericPart) {
            pos.setField(UAMEASFMT_NUMERIC_FIELD_FLAG);
            pos.setBeginIndex(0);
            pos.setEndIndex(formattedNumber.length());
        }
        UnicodeString pattern = number::impl::LongNameHandler::getUnitPattern(getLocale(status),
                amtUnit, getUnitWidth(fWidth), pluralForm, status);
        // The above already handles fallback from other widths to short
        if (pattern.isBogus() && pluralForm != StandardPlural::Form::OTHER) {
            pattern = number::impl::LongNameHandler::getUnitPattern(getLocale(status),
                amtUnit, getUnitWidth(fWidth), StandardPlural::Form::OTHER, status);
        }
        if (U_FAILURE(status)) {
            return appendTo;
        }
        SimpleFormatter formatter(pattern, 0, 1, status);
        QuantityFormatter::format(formatter, formattedNumber, appendTo, pos, status);
    } else {
        // This is the current code
        number::FormattedNumber result;
        if (auto* lnf = df->toNumberFormatter(status)) {
            result = lnf->unit(amtUnit)
                .unitWidth(getUnitWidth(fWidth))
                .formatDouble(amtNumber.getDouble(status), status);
        }
        DecimalFormat::fieldPositionHelper(result, pos, appendTo.length(), status);
        if (posForFullNumericPart) {
            pos.setField(UNUM_INTEGER_FIELD);
            DecimalFormat::fieldPositionHelper(result, pos, appendTo.length(), status);
            int32_t intBegin = pos.getBeginIndex();
            int32_t intEnd = pos.getEndIndex();
            pos.setField(UNUM_FRACTION_FIELD);
            DecimalFormat::fieldPositionHelper(result, pos, appendTo.length(), status);
            int32_t fracBegin = pos.getBeginIndex();
            int32_t fracEnd = pos.getEndIndex();
            if (intBegin >= 0 && intEnd > intBegin) {
                // we have an integer part
                pos.setBeginIndex(intBegin);
                if (fracBegin >= intEnd && fracEnd > fracBegin) {
                    // we have a fraction part, include it too
                    pos.setEndIndex(fracEnd);
                } else {
                    pos.setEndIndex(intEnd);
                }
            } else if (fracBegin >= 0 && fracEnd > fracBegin) {
                // only a fract part, use it
                pos.setBeginIndex(fracBegin);
                pos.setEndIndex(fracEnd);
            } else {
                // no numeric part
                pos.setBeginIndex(0);
                pos.setEndIndex(0);
            }
            pos.setField(UAMEASFMT_NUMERIC_FIELD_FLAG);
        }
        appendTo.append(result.toTempString(status));
    }

    if (stripPatternSpaces) {
        // Get the narrow pattern for OTHER.
        // If there are spaces in that, then do not continue to strip spaces
        // (i.e. even in the narrowest form this locale keeps spaces).
        UnicodeString narrowPattern = number::impl::LongNameHandler::getUnitPattern(getLocale(status),
                amtUnit, UNUM_UNIT_WIDTH_NARROW, StandardPlural::Form::OTHER, status); 
        if (U_SUCCESS(status)) {
            if (narrowPattern.indexOf((UChar)0x0020) == -1 && narrowPattern.indexOf((UChar)0x00A0) == -1) {
                int32_t end = appendTo.length();
                for (; cur < end; cur++) {
                    if (appendTo.charAt(cur) == 0x0020) {
                        appendTo.remove(cur, 1);
                        if (pos.getBeginIndex() > cur) {
                            pos.setBeginIndex(pos.getBeginIndex() - 1);
                            pos.setEndIndex(pos.getEndIndex() - 1);
                        }
                    }
                }
            }
        }
    }
    return appendTo;
}

// Formats hours-minutes-seconds as 5:37:23 or similar.
UnicodeString &MeasureFormat::formatNumeric(
        const Formattable *hms,  // always length 3
        int32_t bitMap,   // 1=hourset, 2=minuteset, 4=secondset
        UnicodeString &appendTo,
        FieldPositionHandler& handler,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    UDate millis = 
        (UDate) (((uprv_trunc(hms[0].getDouble(status)) * 60.0
             + uprv_trunc(hms[1].getDouble(status))) * 60.0
                  + uprv_trunc(hms[2].getDouble(status))) * 1000.0);
    switch (bitMap) {
    case 5: // hs
    case 7: // hms
        return formatNumeric(
                millis,
                cache->getNumericDateFormatters()->hourMinuteSecond,
                UDAT_SECOND_FIELD,
                hms[2],
                appendTo,
                handler,
                status);
        break;
    case 6: // ms
        return formatNumeric(
                millis,
                cache->getNumericDateFormatters()->minuteSecond,
                UDAT_SECOND_FIELD,
                hms[2],
                appendTo,
                handler,
                status);
        break;
    case 3: // hm
        return formatNumeric(
                millis,
                cache->getNumericDateFormatters()->hourMinute,
                UDAT_MINUTE_FIELD,
                hms[1],
                appendTo,
                handler,
                status);
        break;
    default:
        status = U_INTERNAL_PROGRAM_ERROR;
        return appendTo;
        break;
    }
}

static void appendRange(
        const UnicodeString &src,
        int32_t start,
        int32_t end,
        UnicodeString &dest) {
    dest.append(src, start, end - start);
}

static void appendRange(
        const UnicodeString &src,
        int32_t end,
        UnicodeString &dest) {
    dest.append(src, end, src.length() - end);
}

// Formats time like 5:37:23
UnicodeString &MeasureFormat::formatNumeric(
        UDate date, // Time since epoch 1:30:00 would be 5400000
        const DateFormat &dateFmt, // h:mm, m:ss, or h:mm:ss
        UDateFormatField smallestField, // seconds in 5:37:23.5
        const Formattable &smallestAmount, // 23.5 for 5:37:23.5
        UnicodeString &appendTo,
        FieldPositionHandler& handler,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    // Format the smallest amount with this object's NumberFormat
    UnicodeString smallestAmountFormatted;

    // We keep track of the integer part of smallest amount so that
    // we can replace it later so that we get '0:00:09.3' instead of
    // '0:00:9.3'
    FieldPosition intFieldPosition(UNUM_INTEGER_FIELD);
    (*numberFormat)->format(
            smallestAmount, smallestAmountFormatted, intFieldPosition, status);
    if (
            intFieldPosition.getBeginIndex() == 0 &&
            intFieldPosition.getEndIndex() == 0) {
        status = U_INTERNAL_PROGRAM_ERROR;
        return appendTo;
    }

    // Format time. draft becomes something like '5:30:45'
    // #13606: DateFormat is not thread-safe, but MeasureFormat advertises itself as thread-safe.
    FieldPositionIterator posIter;
    UnicodeString draft;
    static UMutex *dateFmtMutex = STATIC_NEW(UMutex);
    umtx_lock(dateFmtMutex);
    dateFmt.format(date, draft, &posIter, status);
    umtx_unlock(dateFmtMutex);

    int32_t start = appendTo.length();
    FieldPosition smallestFieldPosition(smallestField);
    FieldPosition fp;
    int32_t measField = -1;
    while (posIter.next(fp)) {
        int32_t dateField = fp.getField();
        switch (dateField) {
            case UDAT_HOUR_OF_DAY1_FIELD:
            case UDAT_HOUR_OF_DAY0_FIELD:
            case UDAT_HOUR1_FIELD:
            case UDAT_HOUR0_FIELD:
                measField = UAMEASUNIT_DURATION_HOUR; break;
            case UDAT_MINUTE_FIELD:
                measField = UAMEASUNIT_DURATION_MINUTE; break;
            case UDAT_SECOND_FIELD:
                measField = UAMEASUNIT_DURATION_SECOND; break;
            default:
                measField = -1; break;
        }
        if (dateField != smallestField) {
            if (measField >= 0) {
                handler.addAttribute(measField, start + fp.getBeginIndex(), start + fp.getEndIndex());
                handler.addAttribute(measField | UAMEASFMT_NUMERIC_FIELD_FLAG, start + fp.getBeginIndex(), start + fp.getEndIndex());
            }
        } else {
            smallestFieldPosition.setBeginIndex(fp.getBeginIndex());
            smallestFieldPosition.setEndIndex(fp.getEndIndex());
            break;
        }
    }

    // If we find field for smallest amount replace it with the formatted
    // smallest amount from above taking care to replace the integer part
    // with what is in original time. For example, If smallest amount
    // is 9.35s and the formatted time is 0:00:09 then 9.35 becomes 09.35
    // and replacing yields 0:00:09.35
    if (smallestFieldPosition.getBeginIndex() != 0 ||
            smallestFieldPosition.getEndIndex() != 0) {
        appendRange(draft, 0, smallestFieldPosition.getBeginIndex(), appendTo);
        appendRange(
                smallestAmountFormatted,
                0,
                intFieldPosition.getBeginIndex(),
                appendTo);
        appendRange(
                draft,
                smallestFieldPosition.getBeginIndex(),
                smallestFieldPosition.getEndIndex(),
                appendTo);
        appendRange(
                smallestAmountFormatted,
                intFieldPosition.getEndIndex(),
                appendTo);
        appendRange(
                draft,
                smallestFieldPosition.getEndIndex(),
                appendTo);
        handler.addAttribute(measField, start + smallestFieldPosition.getBeginIndex(), appendTo.length());
        handler.addAttribute(measField | UAMEASFMT_NUMERIC_FIELD_FLAG, start + smallestFieldPosition.getBeginIndex(), appendTo.length());
    } else {
        appendTo.append(draft);
    }
    return appendTo;
}

UnicodeString &MeasureFormat::formatMeasuresSlowTrack(
        const Measure *measures,
        int32_t measureCount,
        UnicodeString& appendTo,
        FieldPosition& pos,
        UErrorCode& status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    FieldPosition dontCare(FieldPosition::DONT_CARE);
    FieldPosition fpos(pos.getField());
    LocalArray<UnicodeString> results(new UnicodeString[measureCount], status);
    int32_t fieldPositionFoundIndex = -1;
    for (int32_t i = 0; i < measureCount; ++i) {
        const NumberFormat *nf = cache->getIntegerFormat();
        if (i == measureCount - 1) {
            nf = numberFormat->get();
        }
        if (fieldPositionFoundIndex == -1) {
            formatMeasure(measures[i], *nf, results[i], fpos, status);
            if (U_FAILURE(status)) {
                return appendTo;
            }
            if (fpos.getBeginIndex() != 0 || fpos.getEndIndex() != 0) {
                fieldPositionFoundIndex = i;
            }
        } else {
            formatMeasure(measures[i], *nf, results[i], dontCare, status);
        }
    }
    int32_t offset;
    listFormatter->format(
            results.getAlias(),
            measureCount,
            appendTo,
            fieldPositionFoundIndex,
            offset,
            status);
    if (U_FAILURE(status)) {
        return appendTo;
    }
    // Fix up FieldPosition indexes if our field is found.
    if (offset != -1) {
        pos.setBeginIndex(fpos.getBeginIndex() + offset);
        pos.setEndIndex(fpos.getEndIndex() + offset);
    }
    return appendTo;
}

MeasureFormat* U_EXPORT2 MeasureFormat::createCurrencyFormat(const Locale& locale,
                                                   UErrorCode& ec) {
    if (U_FAILURE(ec)) {
        return nullptr;
    }
    LocalPointer<CurrencyFormat> fmt(new CurrencyFormat(locale, ec), ec);
    return fmt.orphan();
}

MeasureFormat* U_EXPORT2 MeasureFormat::createCurrencyFormat(UErrorCode& ec) {
    if (U_FAILURE(ec)) {
        return nullptr;
    }
    return MeasureFormat::createCurrencyFormat(Locale::getDefault(), ec);
}

U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_FORMATTING */

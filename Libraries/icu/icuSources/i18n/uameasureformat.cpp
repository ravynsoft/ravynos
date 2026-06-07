/*
*****************************************************************************************
* Copyright (C) 2014-2016 Apple Inc. All Rights Reserved.
*****************************************************************************************
*/

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include <stdlib.h>
#include "unicode/uameasureformat.h"
#include "unicode/fieldpos.h"
#include "unicode/localpointer.h"
#include "unicode/numfmt.h"
#include "unicode/measunit.h"
#include "unicode/measure.h"
#include "unicode/measfmt.h"
#include "unicode/unistr.h"
#include "unicode/unum.h"
#include "unicode/umisc.h"
#include "unicode/ures.h"
#include "uresimp.h"
#include "ustr_imp.h"
#include "cstring.h"
#include "ulocimp.h"

U_NAMESPACE_USE


U_CAPI UAMeasureFormat* U_EXPORT2
uameasfmt_open( const char*          locale,
                UAMeasureFormatWidth width,
                UNumberFormat*       nfToAdopt,
                UErrorCode*          status )
{
    if (U_FAILURE(*status)) {
        return NULL;
    }
    UMeasureFormatWidth mfWidth = UMEASFMT_WIDTH_WIDE;
    switch (width) {
        case UAMEASFMT_WIDTH_WIDE:
            break;
        case UAMEASFMT_WIDTH_SHORT:
            mfWidth = UMEASFMT_WIDTH_SHORT; break;
        case UAMEASFMT_WIDTH_NARROW:
            mfWidth = UMEASFMT_WIDTH_NARROW; break;
        case UAMEASFMT_WIDTH_NUMERIC:
            mfWidth = UMEASFMT_WIDTH_NUMERIC; break;
        case UAMEASFMT_WIDTH_SHORTER:
            mfWidth = UMEASFMT_WIDTH_SHORTER; break;
        default:
            *status = U_ILLEGAL_ARGUMENT_ERROR; return NULL;
    }
    LocalPointer<MeasureFormat> measfmt( new MeasureFormat(Locale(locale), mfWidth, (NumberFormat*)nfToAdopt, *status) );
    if (U_FAILURE(*status)) {
        return NULL;
    }
    return (UAMeasureFormat*)measfmt.orphan();
}


U_CAPI void U_EXPORT2
uameasfmt_close(UAMeasureFormat *measfmt)
{
    delete (MeasureFormat*)measfmt;
}

static MeasureUnit * createObjectForMeasureUnit(UAMeasureUnit unit, UErrorCode* status )
{
    MeasureUnit * munit = NULL;
    switch (unit) {
        case UAMEASUNIT_ACCELERATION_G_FORCE:   munit = MeasureUnit::createGForce(*status);      break;
        case UAMEASUNIT_ACCELERATION_METER_PER_SECOND_SQUARED:  munit = MeasureUnit::createMeterPerSecondSquared(*status); break;

        case UAMEASUNIT_ANGLE_DEGREE:           munit = MeasureUnit::createDegree(*status);      break;
        case UAMEASUNIT_ANGLE_ARC_MINUTE:       munit = MeasureUnit::createArcMinute(*status);   break;
        case UAMEASUNIT_ANGLE_ARC_SECOND:       munit = MeasureUnit::createArcSecond(*status);   break;
        case UAMEASUNIT_ANGLE_RADIAN:           munit = MeasureUnit::createRadian(*status);      break;
        case UAMEASUNIT_ANGLE_REVOLUTION:       munit = MeasureUnit::createRevolutionAngle(*status); break;

        case UAMEASUNIT_AREA_SQUARE_METER:      munit = MeasureUnit::createSquareMeter(*status);     break;
        case UAMEASUNIT_AREA_SQUARE_KILOMETER:  munit = MeasureUnit::createSquareKilometer(*status); break;
        case UAMEASUNIT_AREA_SQUARE_FOOT:       munit = MeasureUnit::createSquareFoot(*status);      break;
        case UAMEASUNIT_AREA_SQUARE_MILE:       munit = MeasureUnit::createSquareMile(*status);      break;
        case UAMEASUNIT_AREA_ACRE:              munit = MeasureUnit::createAcre(*status);            break;
        case UAMEASUNIT_AREA_HECTARE:           munit = MeasureUnit::createHectare(*status);         break;
        case UAMEASUNIT_AREA_SQUARE_CENTIMETER: munit = MeasureUnit::createSquareCentimeter(*status); break;
        case UAMEASUNIT_AREA_SQUARE_INCH:       munit = MeasureUnit::createSquareInch(*status);      break;
        case UAMEASUNIT_AREA_SQUARE_YARD:       munit = MeasureUnit::createSquareYard(*status);      break;
        case UAMEASUNIT_AREA_DUNAM:             munit = MeasureUnit::createDunam(*status);           break;

        case UAMEASUNIT_DURATION_YEAR:          munit = MeasureUnit::createYear(*status);        break;
        case UAMEASUNIT_DURATION_MONTH:         munit = MeasureUnit::createMonth(*status);       break;
        case UAMEASUNIT_DURATION_WEEK:          munit = MeasureUnit::createWeek(*status);        break;
        case UAMEASUNIT_DURATION_DAY:           munit = MeasureUnit::createDay(*status);         break;
        case UAMEASUNIT_DURATION_HOUR:          munit = MeasureUnit::createHour(*status);        break;
        case UAMEASUNIT_DURATION_MINUTE:        munit = MeasureUnit::createMinute(*status);      break;
        case UAMEASUNIT_DURATION_SECOND:        munit = MeasureUnit::createSecond(*status);      break;
        case UAMEASUNIT_DURATION_MILLISECOND:   munit = MeasureUnit::createMillisecond(*status); break;
        case UAMEASUNIT_DURATION_MICROSECOND:   munit = MeasureUnit::createMicrosecond(*status); break;
        case UAMEASUNIT_DURATION_NANOSECOND:    munit = MeasureUnit::createNanosecond(*status);  break;
        case UAMEASUNIT_DURATION_CENTURY:       munit = MeasureUnit::createCentury(*status);     break;
        case UAMEASUNIT_DURATION_YEAR_PERSON:   munit = MeasureUnit::createYearPerson(*status);  break;
        case UAMEASUNIT_DURATION_MONTH_PERSON:  munit = MeasureUnit::createMonthPerson(*status); break;
        case UAMEASUNIT_DURATION_WEEK_PERSON:   munit = MeasureUnit::createWeekPerson(*status);  break;
        case UAMEASUNIT_DURATION_DAY_PERSON:    munit = MeasureUnit::createDayPerson(*status);   break;

        case UAMEASUNIT_LENGTH_METER:           munit = MeasureUnit::createMeter(*status);       break;
        case UAMEASUNIT_LENGTH_CENTIMETER:      munit = MeasureUnit::createCentimeter(*status);  break;
        case UAMEASUNIT_LENGTH_KILOMETER:       munit = MeasureUnit::createKilometer(*status);   break;
        case UAMEASUNIT_LENGTH_MILLIMETER:      munit = MeasureUnit::createMillimeter(*status);  break;
        case UAMEASUNIT_LENGTH_PICOMETER:       munit = MeasureUnit::createPicometer(*status);   break;
        case UAMEASUNIT_LENGTH_FOOT:            munit = MeasureUnit::createFoot(*status);        break;
        case UAMEASUNIT_LENGTH_INCH:            munit = MeasureUnit::createInch(*status);        break;
        case UAMEASUNIT_LENGTH_MILE:            munit = MeasureUnit::createMile(*status);        break;
        case UAMEASUNIT_LENGTH_YARD:            munit = MeasureUnit::createYard(*status);        break;
        case UAMEASUNIT_LENGTH_LIGHT_YEAR:      munit = MeasureUnit::createLightYear(*status);   break;
        case UAMEASUNIT_LENGTH_DECIMETER:       munit = MeasureUnit::createDecimeter(*status);   break;
        case UAMEASUNIT_LENGTH_MICROMETER:      munit = MeasureUnit::createMicrometer(*status);  break;
        case UAMEASUNIT_LENGTH_NANOMETER:       munit = MeasureUnit::createNanometer(*status);   break;
        case UAMEASUNIT_LENGTH_NAUTICAL_MILE:   munit = MeasureUnit::createNauticalMile(*status); break;
        case UAMEASUNIT_LENGTH_FATHOM:          munit = MeasureUnit::createFathom(*status);      break;
        case UAMEASUNIT_LENGTH_FURLONG:         munit = MeasureUnit::createFurlong(*status);     break;
        case UAMEASUNIT_LENGTH_ASTRONOMICAL_UNIT: munit = MeasureUnit::createAstronomicalUnit(*status); break;
        case UAMEASUNIT_LENGTH_PARSEC:          munit = MeasureUnit::createParsec(*status);      break;
        case UAMEASUNIT_LENGTH_MILE_SCANDINAVIAN: munit = MeasureUnit::createMileScandinavian(*status); break;
        case UAMEASUNIT_LENGTH_POINT:           munit = MeasureUnit::createPoint(*status);       break;
        case UAMEASUNIT_LENGTH_SOLAR_RADIUS:    munit = MeasureUnit::createSolarRadius(*status); break;

        case UAMEASUNIT_MASS_GRAM:              munit = MeasureUnit::createGram(*status);        break;
        case UAMEASUNIT_MASS_KILOGRAM:          munit = MeasureUnit::createKilogram(*status);    break;
        case UAMEASUNIT_MASS_OUNCE:             munit = MeasureUnit::createOunce(*status);       break;
        case UAMEASUNIT_MASS_POUND:             munit = MeasureUnit::createPound(*status);       break;
        case UAMEASUNIT_MASS_STONE:             munit = MeasureUnit::createStone(*status);       break;
        case UAMEASUNIT_MASS_MICROGRAM:         munit = MeasureUnit::createMicrogram(*status);   break;
        case UAMEASUNIT_MASS_MILLIGRAM:         munit = MeasureUnit::createMilligram(*status);   break;
        case UAMEASUNIT_MASS_METRIC_TON:        munit = MeasureUnit::createMetricTon(*status);   break;
        case UAMEASUNIT_MASS_TON:               munit = MeasureUnit::createTon(*status);         break;
        case UAMEASUNIT_MASS_CARAT:             munit = MeasureUnit::createCarat(*status);       break;
        case UAMEASUNIT_MASS_OUNCE_TROY:        munit = MeasureUnit::createOunceTroy(*status);   break;
        case UAMEASUNIT_MASS_DALTON:            munit = MeasureUnit::createDalton(*status);      break;
        case UAMEASUNIT_MASS_EARTH_MASS:        munit = MeasureUnit::createEarthMass(*status);   break;
        case UAMEASUNIT_MASS_SOLAR_MASS:        munit = MeasureUnit::createSolarMass(*status);   break;

        case UAMEASUNIT_POWER_WATT:             munit = MeasureUnit::createWatt(*status);        break;
        case UAMEASUNIT_POWER_KILOWATT:         munit = MeasureUnit::createKilowatt(*status);    break;
        case UAMEASUNIT_POWER_HORSEPOWER:       munit = MeasureUnit::createHorsepower(*status);  break;
        case UAMEASUNIT_POWER_MILLIWATT:        munit = MeasureUnit::createMilliwatt(*status);   break;
        case UAMEASUNIT_POWER_MEGAWATT:         munit = MeasureUnit::createMegawatt(*status);    break;
        case UAMEASUNIT_POWER_GIGAWATT:         munit = MeasureUnit::createGigawatt(*status);    break;

        case UAMEASUNIT_PRESSURE_HECTOPASCAL:   munit = MeasureUnit::createHectopascal(*status); break;
        case UAMEASUNIT_PRESSURE_INCH_HG:       munit = MeasureUnit::createInchHg(*status);      break;
        case UAMEASUNIT_PRESSURE_MILLIBAR:      munit = MeasureUnit::createMillibar(*status);    break;
        case UAMEASUNIT_PRESSURE_MILLIMETER_OF_MERCURY:  munit = MeasureUnit::createMillimeterOfMercury(*status); break;
        case UAMEASUNIT_PRESSURE_POUND_PER_SQUARE_INCH: munit = MeasureUnit::createPoundPerSquareInch(*status);  break;
        case UAMEASUNIT_PRESSURE_ATMOSPHERE:    munit = MeasureUnit::createAtmosphere(*status);  break;
        case UAMEASUNIT_PRESSURE_KILOPASCAL:    munit = MeasureUnit::createKilopascal(*status);  break;
        case UAMEASUNIT_PRESSURE_MEGAPASCAL:    munit = MeasureUnit::createMegapascal(*status);  break;

        case UAMEASUNIT_SPEED_METER_PER_SECOND:   munit = MeasureUnit::createMeterPerSecond(*status);   break;
        case UAMEASUNIT_SPEED_KILOMETER_PER_HOUR: munit = MeasureUnit::createKilometerPerHour(*status); break;
        case UAMEASUNIT_SPEED_MILE_PER_HOUR:      munit = MeasureUnit::createMilePerHour(*status);      break;
        case UAMEASUNIT_SPEED_KNOT:               munit = MeasureUnit::createKnot(*status);      break;

        case UAMEASUNIT_TEMPERATURE_CELSIUS:    munit = MeasureUnit::createCelsius(*status);     break;
        case UAMEASUNIT_TEMPERATURE_FAHRENHEIT: munit = MeasureUnit::createFahrenheit(*status);  break;
        case UAMEASUNIT_TEMPERATURE_KELVIN:     munit = MeasureUnit::createKelvin(*status);      break;
        case UAMEASUNIT_TEMPERATURE_GENERIC:    munit = MeasureUnit::createGenericTemperature(*status); break;

        case UAMEASUNIT_VOLUME_LITER:           munit = MeasureUnit::createLiter(*status);          break;
        case UAMEASUNIT_VOLUME_CUBIC_KILOMETER: munit = MeasureUnit::createCubicKilometer(*status); break;
        case UAMEASUNIT_VOLUME_CUBIC_MILE:      munit = MeasureUnit::createCubicMile(*status);      break;
        case UAMEASUNIT_VOLUME_MILLILITER:      munit = MeasureUnit::createMilliliter(*status);     break;
        case UAMEASUNIT_VOLUME_CENTILITER:      munit = MeasureUnit::createCentiliter(*status);     break;
        case UAMEASUNIT_VOLUME_DECILITER:       munit = MeasureUnit::createDeciliter(*status);      break;
        case UAMEASUNIT_VOLUME_HECTOLITER:      munit = MeasureUnit::createHectoliter(*status);     break;
        case UAMEASUNIT_VOLUME_MEGALITER:       munit = MeasureUnit::createMegaliter(*status);      break;
        case UAMEASUNIT_VOLUME_CUBIC_CENTIMETER: munit = MeasureUnit::createCubicCentimeter(*status); break;
        case UAMEASUNIT_VOLUME_CUBIC_METER:     munit = MeasureUnit::createCubicMeter(*status);     break;
        case UAMEASUNIT_VOLUME_CUBIC_INCH:      munit = MeasureUnit::createCubicInch(*status);      break;
        case UAMEASUNIT_VOLUME_CUBIC_FOOT:      munit = MeasureUnit::createCubicFoot(*status);      break;
        case UAMEASUNIT_VOLUME_CUBIC_YARD:      munit = MeasureUnit::createCubicYard(*status);      break;
        case UAMEASUNIT_VOLUME_ACRE_FOOT:       munit = MeasureUnit::createAcreFoot(*status);       break;
        case UAMEASUNIT_VOLUME_BUSHEL:          munit = MeasureUnit::createBushel(*status);         break;
        case UAMEASUNIT_VOLUME_TEASPOON:        munit = MeasureUnit::createTeaspoon(*status);       break;
        case UAMEASUNIT_VOLUME_TABLESPOON:      munit = MeasureUnit::createTablespoon(*status);     break;
        case UAMEASUNIT_VOLUME_FLUID_OUNCE:     munit = MeasureUnit::createFluidOunce(*status);     break;
        case UAMEASUNIT_VOLUME_CUP:             munit = MeasureUnit::createCup(*status);            break;
        case UAMEASUNIT_VOLUME_PINT:            munit = MeasureUnit::createPint(*status);           break;
        case UAMEASUNIT_VOLUME_QUART:           munit = MeasureUnit::createQuart(*status);          break;
        case UAMEASUNIT_VOLUME_GALLON:          munit = MeasureUnit::createGallon(*status);         break;
        case UAMEASUNIT_VOLUME_CUP_METRIC:      munit = MeasureUnit::createCupMetric(*status);      break;
        case UAMEASUNIT_VOLUME_PINT_METRIC:     munit = MeasureUnit::createPintMetric(*status);     break;
        case UAMEASUNIT_VOLUME_GALLON_IMPERIAL: munit = MeasureUnit::createGallonImperial(*status); break;
        case UAMEASUNIT_VOLUME_FLUID_OUNCE_IMPERIAL: munit = MeasureUnit::createFluidOunceImperial(*status); break;
        case UAMEASUNIT_VOLUME_BARREL:          munit = MeasureUnit::createBarrel(*status);         break;

        case UAMEASUNIT_ENERGY_JOULE:           munit = MeasureUnit::createJoule(*status);          break;
        case UAMEASUNIT_ENERGY_KILOJOULE:       munit = MeasureUnit::createKilojoule(*status);      break;
        case UAMEASUNIT_ENERGY_CALORIE:         munit = MeasureUnit::createCalorie(*status);        break;
        case UAMEASUNIT_ENERGY_KILOCALORIE:     munit = MeasureUnit::createKilocalorie(*status);    break;
        case UAMEASUNIT_ENERGY_FOODCALORIE:     munit = MeasureUnit::createFoodcalorie(*status);    break;
        case UAMEASUNIT_ENERGY_KILOWATT_HOUR:   munit = MeasureUnit::createKilowattHour(*status);   break;
        case UAMEASUNIT_ENERGY_ELECTRONVOLT:    munit = MeasureUnit::createElectronvolt(*status);   break;
        case UAMEASUNIT_ENERGY_BRITISH_THERMAL_UNIT: munit = MeasureUnit::createBritishThermalUnit(*status); break;

        case UAMEASUNIT_CONSUMPTION_LITER_PER_KILOMETER: munit = MeasureUnit::createLiterPerKilometer(*status); break;
        case UAMEASUNIT_CONSUMPTION_MILE_PER_GALLON:     munit = MeasureUnit::createMilePerGallon(*status);     break;
        case UAMEASUNIT_CONSUMPTION_LITER_PER_100_KILOMETERs: munit = MeasureUnit::createLiterPer100Kilometers(*status); break;
        case UAMEASUNIT_CONSUMPTION_MILE_PER_GALLON_IMPERIAL: munit = MeasureUnit::createMilePerGallonImperial(*status); break;

        case UAMEASUNIT_DIGITAL_BIT:            munit = MeasureUnit::createBit(*status);         break;
        case UAMEASUNIT_DIGITAL_BYTE:           munit = MeasureUnit::createByte(*status);        break;
        case UAMEASUNIT_DIGITAL_GIGABIT:        munit = MeasureUnit::createGigabit(*status);     break;
        case UAMEASUNIT_DIGITAL_GIGABYTE:       munit = MeasureUnit::createGigabyte(*status);    break;
        case UAMEASUNIT_DIGITAL_KILOBIT:        munit = MeasureUnit::createKilobit(*status);     break;
        case UAMEASUNIT_DIGITAL_KILOBYTE:       munit = MeasureUnit::createKilobyte(*status);    break;
        case UAMEASUNIT_DIGITAL_MEGABIT:        munit = MeasureUnit::createMegabit(*status);     break;
        case UAMEASUNIT_DIGITAL_MEGABYTE:       munit = MeasureUnit::createMegabyte(*status);    break;
        case UAMEASUNIT_DIGITAL_TERABIT:        munit = MeasureUnit::createTerabit(*status);     break;
        case UAMEASUNIT_DIGITAL_TERABYTE:       munit = MeasureUnit::createTerabyte(*status);    break;
        case UAMEASUNIT_DIGITAL_PETABYTE:       munit = MeasureUnit::createPetabyte(*status);    break;

        case UAMEASUNIT_ELECTRIC_AMPERE:        munit = MeasureUnit::createAmpere(*status);      break;
        case UAMEASUNIT_ELECTRIC_MILLIAMPERE:   munit = MeasureUnit::createMilliampere(*status); break;
        case UAMEASUNIT_ELECTRIC_OHM:           munit = MeasureUnit::createOhm(*status);         break;
        case UAMEASUNIT_ELECTRIC_VOLT:          munit = MeasureUnit::createVolt(*status);        break;

        case UAMEASUNIT_FREQUENCY_HERTZ:        munit = MeasureUnit::createHertz(*status);       break;
        case UAMEASUNIT_FREQUENCY_KILOHERTZ:    munit = MeasureUnit::createKilohertz(*status);   break;
        case UAMEASUNIT_FREQUENCY_MEGAHERTZ:    munit = MeasureUnit::createMegahertz(*status);   break;
        case UAMEASUNIT_FREQUENCY_GIGAHERTZ:    munit = MeasureUnit::createGigahertz(*status);   break;

        case UAMEASUNIT_LIGHT_LUX:              munit = MeasureUnit::createLux(*status);         break;
        case UAMEASUNIT_LIGHT_SOLAR_LUMINOSITY: munit = MeasureUnit::createSolarLuminosity(*status); break;

        case UAMEASUNIT_CONCENTRATION_KARAT:    munit = MeasureUnit::createKarat(*status);       break;
        case UAMEASUNIT_CONCENTRATION_MILLIGRAM_PER_DECILITER: munit = MeasureUnit::createMilligramPerDeciliter(*status); break;
        case UAMEASUNIT_CONCENTRATION_MILLIMOLE_PER_LITER:     munit = MeasureUnit::createMillimolePerLiter(*status);     break;
        case UAMEASUNIT_CONCENTRATION_PART_PER_MILLION:        munit = MeasureUnit::createPartPerMillion(*status);        break;
        case UAMEASUNIT_CONCENTRATION_PERCENT:  munit = MeasureUnit::createPercent(*status);     break;
        case UAMEASUNIT_CONCENTRATION_PERMILLE: munit = MeasureUnit::createPermille(*status);    break;
        case UAMEASUNIT_CONCENTRATION_PERMYRIAD: munit = MeasureUnit::createPermyriad(*status);  break;
        case UAMEASUNIT_CONCENTRATION_MOLE:     munit = MeasureUnit::createMole(*status);        break;

        case UAMEASUNIT_FORCE_NEWTON:           munit = MeasureUnit::createNewton(*status);      break;
        case UAMEASUNIT_FORCE_POUND_FORCE:      munit = MeasureUnit::createPoundForce(*status);  break;

        case UAMEASUNIT_TORQUE_NEWTON_METER:    munit = MeasureUnit::createNewtonMeter(*status); break;
        case UAMEASUNIT_TORQUE_POUND_FOOT:      munit = MeasureUnit::createPoundFoot(*status);   break;

        default: *status = U_ILLEGAL_ARGUMENT_ERROR; break;
    }
    return munit;
}


U_CAPI int32_t U_EXPORT2
uameasfmt_format( const UAMeasureFormat* measfmt,
                  double            value,
                  UAMeasureUnit     unit,
                  UChar*            result,
                  int32_t           resultCapacity,
                  UErrorCode*       status )
{
    return uameasfmt_formatGetPosition(measfmt, value, unit, result,
                                        resultCapacity, NULL, status);
}

U_CAPI int32_t U_EXPORT2
uameasfmt_formatGetPosition( const UAMeasureFormat* measfmt,
                            double            value,
                            UAMeasureUnit     unit,
                            UChar*            result,
                            int32_t           resultCapacity,
                            UFieldPosition*   pos,
                            UErrorCode*       status )
{
    if (U_FAILURE(*status)) {
        return 0;
    }
    if ( ((result==NULL)? resultCapacity!=0: resultCapacity<0) ) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    if ( ((MeasureFormat*)measfmt)->getWidth() == UMEASFMT_WIDTH_NUMERIC &&
            (unit == UAMEASUNIT_TEMPERATURE_CELSIUS || unit == UAMEASUNIT_TEMPERATURE_FAHRENHEIT) ) {
        // Fix here until http://bugs.icu-project.org/trac/ticket/11593 is addressed
        unit = UAMEASUNIT_TEMPERATURE_GENERIC;
    }
    MeasureUnit * munit = createObjectForMeasureUnit(unit, status);
    if (U_FAILURE(*status)) {
        return 0;
    }
    LocalPointer<Measure> meas(new Measure(value, munit, *status));
    if (U_FAILURE(*status)) {
        return 0;
    }
    FieldPosition fp;
    if (pos != NULL) {
        int32_t field = pos->field;
        if (field < 0 || field >= UNUM_FIELD_COUNT) {
            *status = U_ILLEGAL_ARGUMENT_ERROR;
            return 0;
        }
        fp.setField(field);
    } else {
        fp.setField(FieldPosition::DONT_CARE);
    }
    Formattable fmt;
    fmt.adoptObject(meas.orphan());
    UnicodeString res;
    res.setTo(result, 0, resultCapacity);
    ((MeasureFormat*)measfmt)->format(fmt, res, fp, *status);
    if (pos != NULL) {
        pos->beginIndex = fp.getBeginIndex();
        pos->endIndex = fp.getEndIndex();
    }
    return res.extract(result, resultCapacity, *status);
}

enum { kMeasuresMax = 8 }; // temporary limit, will add allocation later as necessary

U_CAPI int32_t U_EXPORT2
uameasfmt_formatMultiple( const UAMeasureFormat* measfmt,
                          const UAMeasure*  measures,
                          int32_t           measureCount,
                          UChar*            result,
                          int32_t           resultCapacity,
                          UErrorCode*       status )
{
    return uameasfmt_formatMultipleForFields(measfmt, measures, measureCount,
                                             result, resultCapacity, NULL, status);
}

U_CAPI int32_t U_EXPORT2
uameasfmt_formatMultipleForFields( const UAMeasureFormat* measfmt,
                                   const UAMeasure*  measures,
                                   int32_t           measureCount,
                                   UChar*            result,
                                   int32_t           resultCapacity,
                                   UFieldPositionIterator* fpositer,
                                   UErrorCode*       status )
{
    if (U_FAILURE(*status)) {
        return 0;
    }
    if ( ((result==NULL)? resultCapacity!=0: resultCapacity<0) || measureCount <= 0 || measureCount > kMeasuresMax ) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    int32_t i;
    Measure * measurePtrs[kMeasuresMax];
    for (i = 0; i < kMeasuresMax && U_SUCCESS(*status); i++) {
        if (i < measureCount) {
            UAMeasureUnit unit = measures[i].unit;
            if ( ((MeasureFormat*)measfmt)->getWidth() == UMEASFMT_WIDTH_NUMERIC &&
                    (unit == UAMEASUNIT_TEMPERATURE_CELSIUS || unit == UAMEASUNIT_TEMPERATURE_FAHRENHEIT) ) {
                // Fix here until http://bugs.icu-project.org/trac/ticket/11593 is addressed
                unit = UAMEASUNIT_TEMPERATURE_GENERIC;
            }
            MeasureUnit * munit = createObjectForMeasureUnit(unit, status);
            measurePtrs[i] = new Measure(measures[i].value, munit, *status);
        } else {
            MeasureUnit * munit = MeasureUnit::createGForce(*status); // any unit will do
            measurePtrs[i] = new Measure(0, munit, *status);
        }
    }
    if (U_FAILURE(*status)) {
        while (i-- > 0) {
            delete measurePtrs[i];
        }
        return 0;
    }
    Measure measureObjs[kMeasuresMax] = { *measurePtrs[0], *measurePtrs[1], *measurePtrs[2], *measurePtrs[3],
                                          *measurePtrs[4], *measurePtrs[5], *measurePtrs[6], *measurePtrs[7] };
    UnicodeString res;
    res.setTo(result, 0, resultCapacity);
    ((MeasureFormat*)measfmt)->formatMeasures(measureObjs, measureCount, res, (FieldPositionIterator*)fpositer, *status);
    for (i = 0; i < kMeasuresMax; i++) {
        delete measurePtrs[i];
    }
    return res.extract(result, resultCapacity, *status);
}

U_CAPI int32_t U_EXPORT2
uameasfmt_getUnitName( const UAMeasureFormat* measfmt,
                       UAMeasureUnit unit,
                       UChar* result,
                       int32_t resultCapacity,
                       UErrorCode* status )
{
    if (U_FAILURE(*status)) {
        return 0;
    }
    if ( ((result==NULL)? resultCapacity!=0: resultCapacity<0) ) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    LocalPointer<const MeasureUnit> munit(createObjectForMeasureUnit(unit, status));
    if (U_FAILURE(*status)) {
        return 0;
    }
    UnicodeString res;
    res.setTo(result, 0, resultCapacity);
    ((MeasureFormat*)measfmt)->getUnitName(munit.getAlias(), res);
    if (res.isBogus()) {
        *status = U_MISSING_RESOURCE_ERROR;
        return 0;
    }
    return res.extract(result, resultCapacity, *status);
}

U_CAPI int32_t U_EXPORT2
uameasfmt_getMultipleUnitNames( const UAMeasureFormat* measfmt,
                                const UAMeasureUnit* units,
                                int32_t unitCount,
                                UAMeasureNameListStyle listStyle,
                                UChar* result,
                                int32_t resultCapacity,
                                UErrorCode* status )
{
    if (U_FAILURE(*status)) {
        return 0;
    }
    if ( ((result==NULL)? resultCapacity!=0: resultCapacity<0) || unitCount <= 0 || unitCount > kMeasuresMax ) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    int32_t i;
    const MeasureUnit * unitPtrs[kMeasuresMax];
    for (i = 0; i < unitCount && U_SUCCESS(*status); i++) {
        unitPtrs[i] = createObjectForMeasureUnit(units[i], status);
    }
    if (U_FAILURE(*status)) {
        while (i-- > 0) {
            delete unitPtrs[i];
        }
        return 0;
    }
    UnicodeString res;
    res.setTo(result, 0, resultCapacity);
    ((MeasureFormat*)measfmt)->getMultipleUnitNames(unitPtrs, unitCount, listStyle, res);
    for (i = 0; i < unitCount; i++) {
        delete unitPtrs[i];
    }
    if (res.isBogus()) {
        *status = U_MISSING_RESOURCE_ERROR;
        return 0;
    }
    return res.extract(result, resultCapacity, *status);
}

// Temporary hack until we can use the forthcoming C++ MeasureUnitPreferences class for this
typedef struct {
    const char*     key;
    int32_t         count;
    UAMeasureUnit   units[2];
} KeyToUnits;
static const KeyToUnits keyToUnits[] = {
    {   "acre",                     1,  { UAMEASUNIT_AREA_ACRE }   },
    {   "celsius",                  1,  { UAMEASUNIT_TEMPERATURE_CELSIUS }   },
    {   "centimeter",               1,  { UAMEASUNIT_LENGTH_CENTIMETER }   },
    {   "fahrenheit",               1,  { UAMEASUNIT_TEMPERATURE_FAHRENHEIT }   },
    {   "foodcalorie",              1,  { UAMEASUNIT_ENERGY_FOODCALORIE }   },
    {   "foot",                     1,  { UAMEASUNIT_LENGTH_FOOT }   },
    {   "foot inch",                2,  { UAMEASUNIT_LENGTH_FOOT, UAMEASUNIT_LENGTH_INCH }   },
    {   "gallon",                   1,  { UAMEASUNIT_VOLUME_GALLON }   },
    {   "gram",                     1,  { UAMEASUNIT_MASS_GRAM }   },
    {   "hectare",                  1,  { UAMEASUNIT_AREA_HECTARE }   },
    {   "hectopascal",              1,  { UAMEASUNIT_PRESSURE_HECTOPASCAL }   },
    {   "inch",                     1,  { UAMEASUNIT_LENGTH_INCH }   },
    {   "inch-hg",                  1,  { UAMEASUNIT_PRESSURE_INCH_HG }   },
    {   "kilocalorie",              1,  { UAMEASUNIT_ENERGY_KILOCALORIE }   },
    {   "kilogram",                 1,  { UAMEASUNIT_MASS_KILOGRAM }   },
    {   "kilogram gram",            2,  { UAMEASUNIT_MASS_KILOGRAM, UAMEASUNIT_MASS_GRAM }   },
    {   "kilometer",                1,  { UAMEASUNIT_LENGTH_KILOMETER }   },
    {   "kilometer-per-hour",       1,  { UAMEASUNIT_SPEED_KILOMETER_PER_HOUR }   },
    {   "liter-per-100kilometers",  1,  { UAMEASUNIT_CONSUMPTION_LITER_PER_100_KILOMETERs }   },
    {   "liter-per-kilometer",      1,  { UAMEASUNIT_CONSUMPTION_LITER_PER_KILOMETER }   },
    {   "liter",                    1,  { UAMEASUNIT_VOLUME_LITER }   },
    {   "meter",                    1,  { UAMEASUNIT_LENGTH_METER }   },
    {   "meter centimeter",         2,  { UAMEASUNIT_LENGTH_METER, UAMEASUNIT_LENGTH_CENTIMETER }   },
    {   "meter-per-second",         1,  { UAMEASUNIT_SPEED_METER_PER_SECOND }   },
    {   "mile",                     1,  { UAMEASUNIT_LENGTH_MILE }   },
    {   "mile-per-gallon",          1,  { UAMEASUNIT_CONSUMPTION_MILE_PER_GALLON }   },
    {   "mile-per-gallon-imperial", 1,  { UAMEASUNIT_CONSUMPTION_MILE_PER_GALLON_IMPERIAL }   },
    {   "mile-per-hour",            1,  { UAMEASUNIT_SPEED_MILE_PER_HOUR }   },
    {   "mile-scandinavian",        1,  { UAMEASUNIT_LENGTH_MILE_SCANDINAVIAN }   },
    {   "millibar",                 1,  { UAMEASUNIT_PRESSURE_MILLIBAR }   },
    {   "milligram-per-deciliter",  1,  { UAMEASUNIT_CONCENTRATION_MILLIGRAM_PER_DECILITER }   },
    {   "millimeter",               1,  { UAMEASUNIT_LENGTH_MILLIMETER }   },
    {   "millimeter-of-mercury",    1,  { UAMEASUNIT_PRESSURE_MILLIMETER_OF_MERCURY }   },
    {   "millimole-per-liter",      1,  { UAMEASUNIT_CONCENTRATION_MILLIMOLE_PER_LITER }   },
    {   "minute second",            2,  { UAMEASUNIT_DURATION_MINUTE, UAMEASUNIT_DURATION_SECOND }   },
    {   "pound",                    1,  { UAMEASUNIT_MASS_POUND }   },
    {   "pound ounce",              2,  { UAMEASUNIT_MASS_POUND, UAMEASUNIT_MASS_OUNCE }   },
    {   "stone pound",              2,  { UAMEASUNIT_MASS_STONE, UAMEASUNIT_MASS_POUND }   },
    {   "yard",                     1,  { UAMEASUNIT_LENGTH_YARD }   },
    {   "year-person month-person", 2,  { UAMEASUNIT_DURATION_YEAR, UAMEASUNIT_DURATION_MONTH }   },
};
enum { kKeyToUnitsCount = UPRV_LENGTHOF(keyToUnits) };

enum { kCombinedKeyMax = 64, kKeyValueMax = 15 };

static int compareKeyToUnits(const void* searchKey, const void* tableEntry) {
    return uprv_strncmp(((const KeyToUnits*)searchKey)->key, ((const KeyToUnits*)tableEntry)->key, kCombinedKeyMax);
}

U_CAPI int32_t U_EXPORT2
uameasfmt_getUnitsForUsage( const char*     locale,
                            const char*     category,
                            const char*     usage,
                            UAMeasureUnit*  units,
                            int32_t         unitsCapacity,
                            UErrorCode*     status )
{
    if (U_FAILURE(*status)) {
        return 0;
    }
    if ( category==NULL || ((units==NULL)? unitsCapacity!=0: unitsCapacity<0) ) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    char combinedKey[kCombinedKeyMax+1] = "";
    char* combinedKeyPtr;
    uprv_strncat(combinedKey, category, kCombinedKeyMax);
    if (usage != NULL) {
        uprv_strncat(combinedKey, "-", kCombinedKeyMax-uprv_strlen(combinedKey));
        uprv_strncat(combinedKey, usage, kCombinedKeyMax-uprv_strlen(combinedKey));
    }

    // get unitPreferenceData bundle
    UResourceBundle *prefb = ures_openDirect(NULL, "supplementalData", status);
    ures_getByKey(prefb, "unitPreferenceData", prefb, status);
    if (U_FAILURE(*status)) {
        return 0;
    }
    
    // Get region to use
    char region[ULOC_COUNTRY_CAPACITY];
    UErrorCode localStatus;
    UBool usedOverride = FALSE;
    // First check for ms overrides, except in certain categories
    if (uprv_strcmp(category, "concentr") != 0 && uprv_strcmp(category, "duration") != 0) {
        char msValue[kKeyValueMax + 1];
        localStatus = U_ZERO_ERROR;
        int32_t msValueLen = uloc_getKeywordValue(locale, "ms", msValue, kKeyValueMax, &localStatus);
        if (U_SUCCESS(localStatus) && msValueLen> 2) {
            msValue[kKeyValueMax] = 0; // ensure termination
            if (uprv_strcmp(msValue, "metric") == 0) {
                uprv_strcpy(region, "001");
                usedOverride = TRUE;
            } else if (uprv_strcmp(msValue, "ussystem") == 0) {
                uprv_strcpy(region, "US");
                usedOverride = TRUE;
            } else if (uprv_strcmp(msValue, "uksystem") == 0) {
                uprv_strcpy(region, "GB");
                usedOverride = TRUE;
            }
        }
    }
    if (!usedOverride) {
        (void)ulocimp_getRegionForSupplementalData(locale, TRUE, region, sizeof(region), status);
        if (U_FAILURE(*status)) {
            return 0;
        }
    }

    UResourceBundle *unitb = NULL;
    localStatus = U_ZERO_ERROR;
    int32_t retval = 0;
    UResourceBundle *regb = ures_getByKey(prefb, region, NULL, &localStatus);
    if (U_SUCCESS(localStatus)) {
        unitb = ures_getByKey(regb, combinedKey, unitb, &localStatus);
        if (U_FAILURE(localStatus)) {
            combinedKeyPtr = uprv_strstr(combinedKey, "-informal");
            if (combinedKeyPtr != NULL) {
                *combinedKeyPtr = 0;
                localStatus = U_ZERO_ERROR;
                unitb = ures_getByKey(regb, combinedKey, unitb, &localStatus);
            }
        }
    } else {
        combinedKeyPtr = uprv_strstr(combinedKey, "-informal");
        if (combinedKeyPtr != NULL) {
            *combinedKeyPtr = 0;
        }
    }
    if (U_FAILURE(localStatus)) {
        localStatus = U_ZERO_ERROR;
        regb = ures_getByKey(prefb, "001", regb, &localStatus);
        if (U_SUCCESS(localStatus)) {
            unitb = ures_getByKey(regb, combinedKey, unitb, &localStatus);
            if (U_FAILURE(localStatus)) {
                combinedKeyPtr = uprv_strstr(combinedKey, "-small");
                if (combinedKeyPtr == NULL) {
                    combinedKeyPtr = uprv_strstr(combinedKey, "-large");
                }
                if (combinedKeyPtr != NULL) {
                    *combinedKeyPtr = 0;
                    localStatus = U_ZERO_ERROR;
                    unitb = ures_getByKey(regb, combinedKey, unitb, &localStatus);
                }
            }
        }
    }
    if (U_FAILURE(localStatus)) {
        *status = localStatus;
    } else {
        int32_t keyLen = kCombinedKeyMax;
        const char* unitsKey = ures_getUTF8String(unitb, combinedKey, &keyLen, FALSE, status);
        if (U_SUCCESS(*status)) {
            KeyToUnits  searchKey = { unitsKey, 0, { (UAMeasureUnit)0 } };
            const KeyToUnits* keyToUnitsPtr = (const KeyToUnits*)bsearch(&searchKey, keyToUnits, kKeyToUnitsCount,
                                                                        sizeof(KeyToUnits), compareKeyToUnits);
            if (keyToUnitsPtr == NULL) {
                *status = U_MISSING_RESOURCE_ERROR;
            } else {
                retval = keyToUnitsPtr->count;
                if (units != NULL) {
                    if (retval > unitsCapacity) {
                        *status = U_BUFFER_OVERFLOW_ERROR;
                    } else {
                        units[0] = keyToUnitsPtr->units[0];
                        if (retval > 1) {
                            units[1] = keyToUnitsPtr->units[1];
                        }
                    }
                }
            }
        }
    }

    ures_close(unitb);
    ures_close(regb);
    ures_close(prefb);

    return retval;
}

U_CAPI const char * U_EXPORT2
uameasfmt_getUnitCategory(UAMeasureUnit unit,
                          UErrorCode* status )
{
    if (U_FAILURE(*status)) {
        return NULL;
    }
    LocalPointer<const MeasureUnit> munit(createObjectForMeasureUnit(unit, status));
    if (U_FAILURE(*status)) {
        return NULL;
    }
    return munit->getType();
}

#endif /* #if !UCONFIG_NO_FORMATTING */

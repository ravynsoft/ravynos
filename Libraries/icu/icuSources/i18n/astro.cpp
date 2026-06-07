// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/************************************************************************
 * Copyright (C) 1996-2014, International Business Machines Corporation
 * and others. All Rights Reserved.
 ************************************************************************
 *  2003-nov-07   srl       Port from Java
 */

#include "astro.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/calendar.h"
#include <math.h>
#include <float.h>
#include "unicode/putil.h"
#include "uhash.h"
#include "umutex.h"
#include "ucln_in.h"
#include "putilimp.h"
#include <stdio.h>  // for toString()

#if defined (PI) 
#undef PI
#endif

#ifdef U_DEBUG_ASTRO
# include "uresimp.h" // for debugging

static void debug_astro_loc(const char *f, int32_t l)
{
  fprintf(stderr, "%s:%d: ", f, l);
}

static void debug_astro_msg(const char *pat, ...)
{
  va_list ap;
  va_start(ap, pat);
  vfprintf(stderr, pat, ap);
  fflush(stderr);
}
#include "unicode/datefmt.h"
#include "unicode/ustring.h"
static const char * debug_astro_date(UDate d) {
  static char gStrBuf[1024];
  static DateFormat *df = NULL;
  if(df == NULL) {
    df = DateFormat::createDateTimeInstance(DateFormat::MEDIUM, DateFormat::MEDIUM, Locale::getUS());
    df->adoptTimeZone(TimeZone::getGMT()->clone());
  }
  UnicodeString str;
  df->format(d,str);
  u_austrncpy(gStrBuf,str.getTerminatedBuffer(),sizeof(gStrBuf)-1);
  return gStrBuf;
}

// must use double parens, i.e.:  U_DEBUG_ASTRO_MSG(("four is: %d",4));
#define U_DEBUG_ASTRO_MSG(x) {debug_astro_loc(__FILE__,__LINE__);debug_astro_msg x;}
#else
#define U_DEBUG_ASTRO_MSG(x)
#endif

static inline UBool isINVALID(double d) {
  return(uprv_isNaN(d));
}

static icu::UMutex *ccLock() {
    static icu::UMutex *m = STATIC_NEW(icu::UMutex);
    return m;
}

U_CDECL_BEGIN
static UBool calendar_astro_cleanup(void) {
  return TRUE;
}
U_CDECL_END

U_NAMESPACE_BEGIN

/**
 * The number of standard hours in one sidereal day.
 * Approximately 24.93.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
#define SIDEREAL_DAY (23.93446960027)

/**
 * The number of sidereal hours in one mean solar day.
 * Approximately 24.07.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
#define SOLAR_DAY  (24.065709816)

/**
 * The average number of solar days from one new moon to the next.  This is the time
 * it takes for the moon to return the same ecliptic longitude as the sun.
 * It is longer than the sidereal month because the sun's longitude increases
 * during the year due to the revolution of the earth around the sun.
 * Approximately 29.53.
 *
 * @see #SIDEREAL_MONTH
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
const double CalendarAstronomer::SYNODIC_MONTH  = 29.530588853;

/**
 * The average number of days it takes
 * for the moon to return to the same ecliptic longitude relative to the
 * stellar background.  This is referred to as the sidereal month.
 * It is shorter than the synodic month due to
 * the revolution of the earth around the sun.
 * Approximately 27.32.
 *
 * @see #SYNODIC_MONTH
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
#define SIDEREAL_MONTH  27.32166

/**
 * The average number number of days between successive vernal equinoxes.
 * Due to the precession of the earth's
 * axis, this is not precisely the same as the sidereal year.
 * Approximately 365.24
 *
 * @see #SIDEREAL_YEAR
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
#define TROPICAL_YEAR  365.242191

/**
 * The average number of days it takes
 * for the sun to return to the same position against the fixed stellar
 * background.  This is the duration of one orbit of the earth about the sun
 * as it would appear to an outside observer.
 * Due to the precession of the earth's
 * axis, this is not precisely the same as the tropical year.
 * Approximately 365.25.
 *
 * @see #TROPICAL_YEAR
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
#define SIDEREAL_YEAR  365.25636

//-------------------------------------------------------------------------
// Time-related constants
//-------------------------------------------------------------------------

/**
 * The number of milliseconds in one second.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
#define SECOND_MS  U_MILLIS_PER_SECOND

/**
 * The number of milliseconds in one minute.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
#define MINUTE_MS  U_MILLIS_PER_MINUTE

/**
 * The number of milliseconds in one hour.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
#define HOUR_MS   U_MILLIS_PER_HOUR

/**
 * The number of milliseconds in one day.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
#define DAY_MS U_MILLIS_PER_DAY

/**
 * The start of the julian day numbering scheme used by astronomers, which
 * is 1/1/4713 BC (Julian), 12:00 GMT.  This is given as the number of milliseconds
 * since 1/1/1970 AD (Gregorian), a negative number.
 * Note that julian day numbers and
 * the Julian calendar are <em>not</em> the same thing.  Also note that
 * julian days start at <em>noon</em>, not midnight.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
#define JULIAN_EPOCH_MS  -210866760000000.0


/**
 * Milliseconds value for 0.0 January 2000 AD.
 */
#define EPOCH_2000_MS  946598400000.0

//-------------------------------------------------------------------------
// Assorted private data used for conversions
//-------------------------------------------------------------------------

// My own copies of these so compilers are more likely to optimize them away
const double CalendarAstronomer::PI = 3.14159265358979323846;

#define CalendarAstronomer_PI2  (CalendarAstronomer::PI*2.0)
#define RAD_HOUR  ( 12 / CalendarAstronomer::PI )     // radians -> hours
#define DEG_RAD ( CalendarAstronomer::PI / 180 )      // degrees -> radians
#define RAD_DEG  ( 180 / CalendarAstronomer::PI )     // radians -> degrees

/***
 * Given 'value', add or subtract 'range' until 0 <= 'value' < range.
 * The modulus operator.
 */
inline static double normalize(double value, double range)  {
    return value - range * ClockMath::floorDivide(value, range);
}

/**
 * Normalize an angle so that it's in the range 0 - 2pi.
 * For positive angles this is just (angle % 2pi), but the Java
 * mod operator doesn't work that way for negative numbers....
 */
inline static double norm2PI(double angle)  {
    return normalize(angle, CalendarAstronomer::PI * 2.0);
}

/**
 * Normalize an angle into the range -PI - PI
 */
inline static  double normPI(double angle)  {
    return normalize(angle + CalendarAstronomer::PI, CalendarAstronomer::PI * 2.0) - CalendarAstronomer::PI;
}

//-------------------------------------------------------------------------
// Constructors
//-------------------------------------------------------------------------

/**
 * Construct a new <code>CalendarAstronomer</code> object that is initialized to
 * the current date and time.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
CalendarAstronomer::CalendarAstronomer():
  fTime(Calendar::getNow()), fLongitude(0.0), fLatitude(0.0), fGmtOffset(0.0), moonPosition(0,0), moonPositionSet(FALSE) {
  clearCache();
}

/**
 * Construct a new <code>CalendarAstronomer</code> object that is initialized to
 * the specified date and time.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
CalendarAstronomer::CalendarAstronomer(UDate d): fTime(d), fLongitude(0.0), fLatitude(0.0), fGmtOffset(0.0), moonPosition(0,0), moonPositionSet(FALSE) {
  clearCache();
}

/**
 * Construct a new <code>CalendarAstronomer</code> object with the given
 * latitude and longitude.  The object's time is set to the current
 * date and time.
 * <p>
 * @param longitude The desired longitude, in <em>degrees</em> east of
 *                  the Greenwich meridian.
 *
 * @param latitude  The desired latitude, in <em>degrees</em>.  Positive
 *                  values signify North, negative South.
 *
 * @see java.util.Date#getTime()
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
CalendarAstronomer::CalendarAstronomer(double longitude, double latitude) :
  fTime(Calendar::getNow()), moonPosition(0,0), moonPositionSet(FALSE) {
  fLongitude = normPI(longitude * (double)DEG_RAD);
  fLatitude  = normPI(latitude  * (double)DEG_RAD);
  fGmtOffset = (double)(fLongitude * 24. * (double)HOUR_MS / (double)CalendarAstronomer_PI2);
  clearCache();
}

CalendarAstronomer::~CalendarAstronomer()
{
}

//-------------------------------------------------------------------------
// Time and date getters and setters
//-------------------------------------------------------------------------

/**
 * Set the current date and time of this <code>CalendarAstronomer</code> object.  All
 * astronomical calculations are performed based on this time setting.
 *
 * @param aTime the date and time, expressed as the number of milliseconds since
 *              1/1/1970 0:00 GMT (Gregorian).
 *
 * @see #setDate
 * @see #getTime
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
void CalendarAstronomer::setTime(UDate aTime) {
    fTime = aTime;
    U_DEBUG_ASTRO_MSG(("setTime(%.1lf, %sL)\n", aTime, debug_astro_date(aTime+fGmtOffset)));
    clearCache();
}

/**
 * Set the current date and time of this <code>CalendarAstronomer</code> object.  All
 * astronomical calculations are performed based on this time setting.
 *
 * @param jdn   the desired time, expressed as a "julian day number",
 *              which is the number of elapsed days since
 *              1/1/4713 BC (Julian), 12:00 GMT.  Note that julian day
 *              numbers start at <em>noon</em>.  To get the jdn for
 *              the corresponding midnight, subtract 0.5.
 *
 * @see #getJulianDay
 * @see #JULIAN_EPOCH_MS
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
void CalendarAstronomer::setJulianDay(double jdn) {
    fTime = (double)(jdn * DAY_MS) + JULIAN_EPOCH_MS;
    clearCache();
    julianDay = jdn;
}

/**
 * Get the current time of this <code>CalendarAstronomer</code> object,
 * represented as the number of milliseconds since
 * 1/1/1970 AD 0:00 GMT (Gregorian).
 *
 * @see #setTime
 * @see #getDate
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
UDate CalendarAstronomer::getTime() {
    return fTime;
}

/**
 * Get the current time of this <code>CalendarAstronomer</code> object,
 * expressed as a "julian day number", which is the number of elapsed
 * days since 1/1/4713 BC (Julian), 12:00 GMT.
 *
 * @see #setJulianDay
 * @see #JULIAN_EPOCH_MS
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
double CalendarAstronomer::getJulianDay() {
    if (isINVALID(julianDay)) {
        julianDay = (fTime - (double)JULIAN_EPOCH_MS) / (double)DAY_MS;
    }
    return julianDay;
}

/**
 * Return this object's time expressed in julian centuries:
 * the number of centuries after 1/1/1900 AD, 12:00 GMT
 *
 * @see #getJulianDay
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
double CalendarAstronomer::getJulianCentury() {
    if (isINVALID(julianCentury)) {
        julianCentury = (getJulianDay() - 2415020.0) / 36525.0;
    }
    return julianCentury;
}

/**
 * Returns the current Greenwich sidereal time, measured in hours
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
double CalendarAstronomer::getGreenwichSidereal() {
    if (isINVALID(siderealTime)) {
        // See page 86 of "Practical Astronomy with your Calculator",
        // by Peter Duffet-Smith, for details on the algorithm.

        double UT = normalize(fTime/(double)HOUR_MS, 24.);

        siderealTime = normalize(getSiderealOffset() + UT*1.002737909, 24.);
    }
    return siderealTime;
}

double CalendarAstronomer::getSiderealOffset() {
    if (isINVALID(siderealT0)) {
        double JD  = uprv_floor(getJulianDay() - 0.5) + 0.5;
        double S   = JD - 2451545.0;
        double T   = S / 36525.0;
        siderealT0 = normalize(6.697374558 + 2400.051336*T + 0.000025862*T*T, 24);
    }
    return siderealT0;
}

/**
 * Returns the current local sidereal time, measured in hours
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
double CalendarAstronomer::getLocalSidereal() {
    return normalize(getGreenwichSidereal() + (fGmtOffset/(double)HOUR_MS), 24.);
}

/**
 * Converts local sidereal time to Universal Time.
 *
 * @param lst   The Local Sidereal Time, in hours since sidereal midnight
 *              on this object's current date.
 *
 * @return      The corresponding Universal Time, in milliseconds since
 *              1 Jan 1970, GMT.
 */
double CalendarAstronomer::lstToUT(double lst) {
    // Convert to local mean time
    double lt = normalize((lst - getSiderealOffset()) * 0.9972695663, 24);

    // Then find local midnight on this day
    double base = (DAY_MS * ClockMath::floorDivide(fTime + fGmtOffset,(double)DAY_MS)) - fGmtOffset;

    //out("    lt  =" + lt + " hours");
    //out("    base=" + new Date(base));

    return base + (long)(lt * HOUR_MS);
}


//-------------------------------------------------------------------------
// Coordinate transformations, all based on the current time of this object
//-------------------------------------------------------------------------

/**
 * Convert from ecliptic to equatorial coordinates.
 *
 * @param ecliptic  A point in the sky in ecliptic coordinates.
 * @return          The corresponding point in equatorial coordinates.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
CalendarAstronomer::Equatorial& CalendarAstronomer::eclipticToEquatorial(CalendarAstronomer::Equatorial& result, const CalendarAstronomer::Ecliptic& ecliptic)
{
    return eclipticToEquatorial(result, ecliptic.longitude, ecliptic.latitude);
}

/**
 * Convert from ecliptic to equatorial coordinates.
 *
 * @param eclipLong     The ecliptic longitude
 * @param eclipLat      The ecliptic latitude
 *
 * @return              The corresponding point in equatorial coordinates.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
CalendarAstronomer::Equatorial& CalendarAstronomer::eclipticToEquatorial(CalendarAstronomer::Equatorial& result, double eclipLong, double eclipLat)
{
    // See page 42 of "Practical Astronomy with your Calculator",
    // by Peter Duffet-Smith, for details on the algorithm.

    double obliq = eclipticObliquity();
    double sinE = ::sin(obliq);
    double cosE = cos(obliq);

    double sinL = ::sin(eclipLong);
    double cosL = cos(eclipLong);

    double sinB = ::sin(eclipLat);
    double cosB = cos(eclipLat);
    double tanB = tan(eclipLat);

    result.set(atan2(sinL*cosE - tanB*sinE, cosL),
        asin(sinB*cosE + cosB*sinE*sinL) );
    return result;
}

/**
 * Convert from ecliptic longitude to equatorial coordinates.
 *
 * @param eclipLong     The ecliptic longitude
 *
 * @return              The corresponding point in equatorial coordinates.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
CalendarAstronomer::Equatorial& CalendarAstronomer::eclipticToEquatorial(CalendarAstronomer::Equatorial& result, double eclipLong)
{
    return eclipticToEquatorial(result, eclipLong, 0);  // TODO: optimize
}

/**
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
CalendarAstronomer::Horizon& CalendarAstronomer::eclipticToHorizon(CalendarAstronomer::Horizon& result, double eclipLong)
{
    Equatorial equatorial;
    eclipticToEquatorial(equatorial, eclipLong);

    double H = getLocalSidereal()*CalendarAstronomer::PI/12 - equatorial.ascension;     // Hour-angle

    double sinH = ::sin(H);
    double cosH = cos(H);
    double sinD = ::sin(equatorial.declination);
    double cosD = cos(equatorial.declination);
    double sinL = ::sin(fLatitude);
    double cosL = cos(fLatitude);

    double altitude = asin(sinD*sinL + cosD*cosL*cosH);
    double azimuth  = atan2(-cosD*cosL*sinH, sinD - sinL * ::sin(altitude));

    result.set(azimuth, altitude);
    return result;
}


//-------------------------------------------------------------------------
// The Sun
//-------------------------------------------------------------------------

//
// Parameters of the Sun's orbit as of the epoch Jan 0.0 1990
// Angles are in radians (after multiplying by CalendarAstronomer::PI/180)
//
#define JD_EPOCH  2447891.5 // Julian day of epoch

#define SUN_ETA_G    (279.403303 * CalendarAstronomer::PI/180) // Ecliptic longitude at epoch
#define SUN_OMEGA_G  (282.768422 * CalendarAstronomer::PI/180) // Ecliptic longitude of perigee
#define SUN_E         0.016713          // Eccentricity of orbit
//double sunR0        1.495585e8        // Semi-major axis in KM
//double sunTheta0    (0.533128 * CalendarAstronomer::PI/180) // Angular diameter at R0

// winter solstice moon date/times 1900-2100 (in UTC)
// These are in UDate/10000.0 (i.e. in units of 10 seconds) to fit into 32 bits.
// sources from e.g.
// http://www.timeanddate.com/calendar/seasons.html?year=1900&n=0
// http://astropixels.com/ephemeris/soleq2001.html
// These 2 tables are just 808 bytes each but but greatly improve both the
// accuracy (and speed for one) of the relevant methods for the relevant time range.
// For getSunTime:
// before fix, errors of up to +73 / -101 min or more; after, errors always less than 1 min.
// about 17 times faster with the fix.
// For getSunLongitude:
// before fix, only accurate to about 0.07 degree; this was enough so Chinese calendar 
// calculations were off by a month in some cases.
// after fix, about 100 times more accurate.
// speed is about the same.
static const int32_t winterSolsticeDates[] = {
//  millis/10K     date
    -220984944, // 1899 Dec 22, 00:56
    -217829274, // 1900 Dec 22, 06:41
    -214673538, // 1901 Dec 22, 12:37
    -211517790, // 1902 Dec 22, 18:35
    -208362120, // 1903 Dec 23, 00:20
    -205206396, // 1904 Dec 22, 06:14
    -202050696, // 1905 Dec 22, 12:04
    -198895002, // 1906 Dec 22, 17:53
    -195739254, // 1907 Dec 22, 23:51
    -192583602, // 1908 Dec 22, 05:33
    -189427920, // 1909 Dec 22, 11:20
    -186272208, // 1910 Dec 22, 17:12
    -183116562, // 1911 Dec 22, 22:53
    -179960850, // 1912 Dec 22, 04:45
    -176805150, // 1913 Dec 22, 10:35
    -173649468, // 1914 Dec 22, 16:22
    -170493744, // 1915 Dec 22, 22:16
    -167338086, // 1916 Dec 22, 03:59
    -164182404, // 1917 Dec 22, 09:46
    -161026674, // 1918 Dec 22, 15:41
    -157870998, // 1919 Dec 22, 21:27
    -154715298, // 1920 Dec 22, 03:17
    -151559592, // 1921 Dec 22, 09:08
    -148403898, // 1922 Dec 22, 14:57
    -145248162, // 1923 Dec 22, 20:53
    -142092450, // 1924 Dec 22, 02:45
    -138936738, // 1925 Dec 22, 08:37
    -135781002, // 1926 Dec 22, 14:33
    -132625332, // 1927 Dec 22, 20:18
    -129469656, // 1928 Dec 22, 02:04
    -126313962, // 1929 Dec 22, 07:53
    -123158286, // 1930 Dec 22, 13:39
    -120002586, // 1931 Dec 22, 19:29
    -116846916, // 1932 Dec 22, 01:14
    -113691252, // 1933 Dec 22, 06:58
    -110535546, // 1934 Dec 22, 12:49
    -107379858, // 1935 Dec 22, 18:37
    -104224158, // 1936 Dec 22, 00:27
    -101068428, // 1937 Dec 22, 06:22
     -97912722, // 1938 Dec 22, 12:13
     -94757004, // 1939 Dec 22, 18:06
     -91601310, // 1940 Dec 21, 23:55
     -88445616, // 1941 Dec 22, 05:44
     -85289886, // 1942 Dec 22, 11:39
     -82134186, // 1943 Dec 22, 17:29
     -78978510, // 1944 Dec 21, 23:15
     -75822822, // 1945 Dec 22, 05:03
     -72667122, // 1946 Dec 22, 10:53
     -69511422, // 1947 Dec 22, 16:43
     -66355722, // 1948 Dec 21, 22:33
     -63200022, // 1949 Dec 22, 04:23
     -60044322, // 1950 Dec 22, 10:13
     -56888640, // 1951 Dec 22, 16:00
     -53732982, // 1952 Dec 21, 21:43
     -50577294, // 1953 Dec 22, 03:31
     -47421576, // 1954 Dec 22, 09:24
     -44265894, // 1955 Dec 22, 15:11
     -41110200, // 1956 Dec 21, 21:00
     -37954506, // 1957 Dec 22, 02:49
     -34798800, // 1958 Dec 22, 08:40
     -31643076, // 1959 Dec 22, 14:34
     -28487364, // 1960 Dec 21, 20:26
     -25331646, // 1961 Dec 22, 02:19
     -22175910, // 1962 Dec 22, 08:15
     -19020228, // 1963 Dec 22, 14:02
     -15864546, // 1964 Dec 21, 19:49
     -12708840, // 1965 Dec 22, 01:40
      -9553152, // 1966 Dec 22, 07:28
      -6397464, // 1967 Dec 22, 13:16
      -3241800, // 1968 Dec 21, 19:00
        -86136, // 1969 Dec 22, 00:44
       3069576, // 1970 Dec 22, 06:36
       6225264, // 1971 Dec 22, 12:24
       9380958, // 1972 Dec 21, 18:13
      12536688, // 1973 Dec 22, 00:08
      15692376, // 1974 Dec 22, 05:56
      18848070, // 1975 Dec 22, 11:45
      22003770, // 1976 Dec 21, 17:35
      25159458, // 1977 Dec 21, 23:23
      28315206, // 1978 Dec 22, 05:21
      31470900, // 1979 Dec 22, 11:10
      34626576, // 1980 Dec 21, 16:56
      37782306, // 1981 Dec 21, 22:51
      40937988, // 1982 Dec 22, 04:38
      44093700, // 1983 Dec 22, 10:30
      47249418, // 1984 Dec 21, 16:23
      50405088, // 1985 Dec 21, 22:08
      53560812, // 1986 Dec 22, 04:02
      56716476, // 1987 Dec 22, 09:46
      59872128, // 1988 Dec 21, 15:28
      63027852, // 1989 Dec 21, 21:22
      66183522, // 1990 Dec 22, 03:07
      69339198, // 1991 Dec 22, 08:53
      72494898, // 1992 Dec 21, 14:43
      75650556, // 1993 Dec 21, 20:26
      78806298, // 1994 Dec 22, 02:23
      81962022, // 1995 Dec 22, 08:17
      85117716, // 1996 Dec 21, 14:06
      88273482, // 1997 Dec 21, 20:07
      91429176, // 1998 Dec 22, 01:56
      94584864, // 1999 Dec 22, 07:44
      97740588, // 2000 Dec 21, 13:38
     100896246, // 2001 Dec 21, 19:21
     104051964, // 2002 Dec 22, 01:14
     107207664, // 2003 Dec 22, 07:04
     110363292, // 2004 Dec 21, 12:42
     113519010, // 2005 Dec 21, 18:35
     116674692, // 2006 Dec 22, 00:22
     119830362, // 2007 Dec 22, 06:07
     122986104, // 2008 Dec 21, 12:04
     126141762, // 2009 Dec 21, 17:47
     129297468, // 2010 Dec 21, 23:38
     132453180, // 2011 Dec 22, 05:30
     135608832, // 2012 Dec 21, 11:12
     138764586, // 2013 Dec 21, 17:11
     141920298, // 2014 Dec 21, 23:03
     145075968, // 2015 Dec 22, 04:48
     148231704, // 2016 Dec 21, 10:44
     151387368, // 2017 Dec 21, 16:28
     154543092, // 2018 Dec 21, 22:22
     157698834, // 2019 Dec 22, 04:19
     160854492, // 2020 Dec 21, 10:02
     164010234, // 2021 Dec 21, 15:59
     167165928, // 2022 Dec 21, 21:48
     170321562, // 2023 Dec 22, 03:27
     173477280, // 2024 Dec 21, 09:20
     176632938, // 2025 Dec 21, 15:03
     179788620, // 2026 Dec 21, 20:50
     182944332, // 2027 Dec 22, 02:42
     186099960, // 2028 Dec 21, 08:20
     189255684, // 2029 Dec 21, 14:14
     192411414, // 2030 Dec 21, 20:09
     195567090, // 2031 Dec 22, 01:55
     198722856, // 2032 Dec 21, 07:56
     201878550, // 2033 Dec 21, 13:45
     205034244, // 2034 Dec 21, 19:34
     208189986, // 2035 Dec 22, 01:31
     211345638, // 2036 Dec 21, 07:13
     214501362, // 2037 Dec 21, 13:07
     217657092, // 2038 Dec 21, 19:02
     220812720, // 2039 Dec 22, 00:40
     223968438, // 2040 Dec 21, 06:33
     227124108, // 2041 Dec 21, 12:18
     230279784, // 2042 Dec 21, 18:04
     233435526, // 2043 Dec 22, 00:01
     236591184, // 2044 Dec 21, 05:44
     239746890, // 2045 Dec 21, 11:35
     242902608, // 2046 Dec 21, 17:28
     246058242, // 2047 Dec 21, 23:07
     249213972, // 2048 Dec 21, 05:02
     252369672, // 2049 Dec 21, 10:52
     255525348, // 2050 Dec 21, 16:38
     258681078, // 2051 Dec 21, 22:33
     261836742, // 2052 Dec 21, 04:17
     264992454, // 2053 Dec 21, 10:09
     268148214, // 2054 Dec 21, 16:09
     271303890, // 2055 Dec 21, 21:55
     274459626, // 2056 Dec 21, 03:51
     277615332, // 2057 Dec 21, 09:42
     280770990, // 2058 Dec 21, 15:25
     283926708, // 2059 Dec 21, 21:18
     287082366, // 2060 Dec 21, 03:01
     290238048, // 2061 Dec 21, 08:48
     293393772, // 2062 Dec 21, 14:42
     296549406, // 2063 Dec 21, 20:21
     299705088, // 2064 Dec 21, 02:08
     302860800, // 2065 Dec 21, 08:00
     306016470, // 2066 Dec 21, 13:45
     309172218, // 2067 Dec 21, 19:43
     312327912, // 2068 Dec 21, 01:32
     315483612, // 2069 Dec 21, 07:22
     318639354, // 2070 Dec 21, 13:19
     321795018, // 2071 Dec 21, 19:03
     324950736, // 2072 Dec 21, 00:56
     328106460, // 2073 Dec 21, 06:50
     331262130, // 2074 Dec 21, 12:35
     334417842, // 2075 Dec 21, 18:27
     337573518, // 2076 Dec 21, 00:13
     340729200, // 2077 Dec 21, 06:00
     343884942, // 2078 Dec 21, 11:57
     347040624, // 2079 Dec 21, 17:44
     350196312, // 2080 Dec 20, 23:32
     353352012, // 2081 Dec 21, 05:22
     356507664, // 2082 Dec 21, 11:04
     359663358, // 2083 Dec 21, 16:53
     362819046, // 2084 Dec 20, 22:41
     365974728, // 2085 Dec 21, 04:28
     369130452, // 2086 Dec 21, 10:22
     372286128, // 2087 Dec 21, 16:08
     375441816, // 2088 Dec 20, 21:56
     378597552, // 2089 Dec 21, 03:52
     381753258, // 2090 Dec 21, 09:43
     384908988, // 2091 Dec 21, 15:38
     388064706, // 2092 Dec 20, 21:31
     391220400, // 2093 Dec 21, 03:20
     394376118, // 2094 Dec 21, 09:13
     397531800, // 2095 Dec 21, 15:00
     400687476, // 2096 Dec 20, 20:46
     403843176, // 2097 Dec 21, 02:36
     406998840, // 2098 Dec 21, 08:20
     410154504, // 2099 Dec 21, 14:04
     413310186, // 2100 Dec 21, 19:51
};
enum { kWinterSolsticeDatesCount = sizeof(winterSolsticeDates)/sizeof(winterSolsticeDates[0]) };

static const UDate winterSolsticeDatesFirst = 10000.0 * -220984944; // winterSolsticeDates[0];
static const UDate winterSolsticeDatesLast  = 10000.0 *  413310186; // winterSolsticeDates[kWinterSolsticeDatesCount-1];
static const UDate winterSolsticeDatesRange = 10000.0 * (413310186 + 220984944); // winterSolsticeDatesLast - winterSolsticeDatesFirst;

static const int8_t sunLongitudeAdjustmts[][4] = {
//  adjustments x 100000 for
//  computed solar longitudes
//  (in radians) at times
//  corresponding to actual
//  longitudes (degrees) of
//     270     0    90   180       for 12 months from
//     ---   ---   ---   ---       ------------------
    {   85,   25,  -89,  -32 }, // 1899 Dec 22, 00:56
    {   90,   30,  -88,  -33 }, // 1900 Dec 22, 06:41
    {   81,   26,  -86,  -29 }, // 1901 Dec 22, 12:37
    {   69,   14,  -87,  -30 }, // 1902 Dec 22, 18:35
    {   74,   21,  -84,  -38 }, // 1903 Dec 23, 00:20
    {   68,    7,  -98,  -40 }, // 1904 Dec 22, 06:14
    {   66,    0, -100,  -35 }, // 1905 Dec 22, 12:04
    {   66,   10,  -91,  -41 }, // 1906 Dec 22, 17:53
    {   54,    3, -100,  -42 }, // 1907 Dec 22, 23:51
    {   63,    7,  -97,  -40 }, // 1908 Dec 22, 05:33
    {   65,    6,  -90,  -36 }, // 1909 Dec 22, 11:20
    {   61,    3,  -88,  -33 }, // 1910 Dec 22, 17:12
    {   70,   20,  -79,  -36 }, // 1911 Dec 22, 22:53
    {   66,   19,  -84,  -31 }, // 1912 Dec 22, 04:45
    {   65,   14,  -80,  -22 }, // 1913 Dec 22, 10:35
    {   67,   25,  -64,  -24 }, // 1914 Dec 22, 16:22
    {   61,   16,  -71,  -26 }, // 1915 Dec 22, 22:16
    {   68,   14,  -72,  -22 }, // 1916 Dec 22, 03:59
    {   70,   15,  -68,  -19 }, // 1917 Dec 22, 09:46
    {   62,    9,  -74,  -20 }, // 1918 Dec 22, 15:41
    {   66,   20,  -71,  -24 }, // 1919 Dec 22, 21:27
    {   64,   16,  -80,  -28 }, // 1920 Dec 22, 03:17
    {   61,    6,  -82,  -29 }, // 1921 Dec 22, 09:08
    {   61,   15,  -67,  -34 }, // 1922 Dec 22, 14:57
    {   52,   12,  -76,  -43 }, // 1923 Dec 22, 20:53
    {   48,    8,  -78,  -37 }, // 1924 Dec 22, 02:45
    {   44,    8,  -68,  -32 }, // 1925 Dec 22, 08:37
    {   35,   -2,  -72,  -33 }, // 1926 Dec 22, 14:33
    {   40,    2,  -67,  -32 }, // 1927 Dec 22, 20:18
    {   43,    0,  -74,  -30 }, // 1928 Dec 22, 02:04
    {   43,   -6,  -78,  -24 }, // 1929 Dec 22, 07:53
    {   46,    7,  -62,  -22 }, // 1930 Dec 22, 13:39
    {   45,    8,  -69,  -27 }, // 1931 Dec 22, 19:29
    {   49,    7,  -69,  -23 }, // 1932 Dec 22, 01:14
    {   55,   13,  -54,  -17 }, // 1933 Dec 22, 06:58
    {   52,   10,  -56,  -22 }, // 1934 Dec 22, 12:49
    {   53,   22,  -49,  -21 }, // 1935 Dec 22, 18:37
    {   52,   23,  -52,  -19 }, // 1936 Dec 22, 00:27
    {   44,   12,  -54,  -17 }, // 1937 Dec 22, 06:22
    {   41,   16,  -40,  -18 }, // 1938 Dec 22, 12:13
    {   36,    8,  -49,  -26 }, // 1939 Dec 22, 18:06
    {   36,    0,  -59,  -25 }, // 1940 Dec 21, 23:55
    {   35,   -2,  -52,  -20 }, // 1941 Dec 22, 05:44
    {   28,   -7,  -60,  -26 }, // 1942 Dec 22, 11:39
    {   26,   -2,  -62,  -27 }, // 1943 Dec 22, 17:29
    {   30,   -2,  -63,  -28 }, // 1944 Dec 21, 23:15
    {   31,  -11,  -68,  -30 }, // 1945 Dec 22, 05:03
    {   29,   -1,  -51,  -29 }, // 1946 Dec 22, 10:53
    {   27,    4,  -55,  -34 }, // 1947 Dec 22, 16:43
    {   26,    1,  -59,  -29 }, // 1948 Dec 21, 22:33
    {   24,    3,  -40,  -16 }, // 1949 Dec 22, 04:23
    {   23,    1,  -41,  -21 }, // 1950 Dec 22, 10:13
    {   25,    3,  -40,  -19 }, // 1951 Dec 22, 16:00
    {   32,    4,  -38,  -11 }, // 1952 Dec 21, 21:43
    {   33,    0,  -44,  -12 }, // 1953 Dec 22, 03:31
    {   28,    8,  -31,   -8 }, // 1954 Dec 22, 09:24
    {   30,   11,  -35,  -14 }, // 1955 Dec 22, 15:11
    {   30,    4,  -45,  -17 }, // 1956 Dec 21, 21:00
    {   29,    4,  -30,  -10 }, // 1957 Dec 22, 02:49
    {   27,    2,  -35,  -22 }, // 1958 Dec 22, 08:40
    {   20,    3,  -39,  -25 }, // 1959 Dec 22, 14:34
    {   16,    3,  -38,  -18 }, // 1960 Dec 21, 20:26
    {   11,   -6,  -44,  -24 }, // 1961 Dec 22, 02:19
    {    2,   -9,  -34,  -23 }, // 1962 Dec 22, 08:15
    {    4,  -10,  -39,  -28 }, // 1963 Dec 22, 14:02
    {    6,  -18,  -50,  -29 }, // 1964 Dec 21, 19:49
    {    4,  -16,  -37,  -15 }, // 1965 Dec 22, 01:40
    {    4,  -11,  -38,  -22 }, // 1966 Dec 22, 07:28
    {    5,   -7,  -40,  -21 }, // 1967 Dec 22, 13:16
    {   11,   -4,  -32,  -12 }, // 1968 Dec 21, 19:00
    {   17,   -3,  -31,  -16 }, // 1969 Dec 22, 00:44
    {   13,    5,  -16,  -13 }, // 1970 Dec 22, 06:36
    {   14,   11,  -14,  -12 }, // 1971 Dec 22, 12:24
    {   14,    9,  -21,  -11 }, // 1972 Dec 21, 18:13
    {    6,    2,   -7,    1 }, // 1973 Dec 22, 00:08
    {    7,    0,   -7,   -7 }, // 1974 Dec 22, 05:56
    {    7,   -3,  -18,  -12 }, // 1975 Dec 22, 11:45
    {    5,   -8,  -19,   -2 }, // 1976 Dec 21, 17:35
    {    6,  -11,  -28,  -12 }, // 1977 Dec 21, 23:23
    {   -4,  -11,  -24,  -14 }, // 1978 Dec 22, 05:21
    {   -5,  -10,  -27,  -18 }, // 1979 Dec 22, 11:10
    {   -1,  -15,  -38,  -27 }, // 1980 Dec 21, 16:56
    {   -9,  -19,  -25,  -18 }, // 1981 Dec 21, 22:51
    {   -7,  -14,  -21,  -26 }, // 1982 Dec 22, 04:38
    {  -11,   -9,  -27,  -29 }, // 1983 Dec 22, 10:30
    {  -16,  -11,  -19,  -12 }, // 1984 Dec 21, 16:23
    {  -11,  -11,  -16,  -16 }, // 1985 Dec 21, 22:08
    {  -18,  -11,   -7,  -12 }, // 1986 Dec 22, 04:02
    {  -12,   -9,   -3,   -7 }, // 1987 Dec 22, 09:46
    {   -4,   -9,  -12,  -10 }, // 1988 Dec 21, 15:28
    {  -10,  -12,   -2,    6 }, // 1989 Dec 21, 21:22
    {   -6,   -5,    0,    1 }, // 1990 Dec 22, 03:07
    {   -2,   -2,   -6,   -6 }, // 1991 Dec 22, 08:53
    {   -4,   -7,   -3,    5 }, // 1992 Dec 21, 14:43
    {    2,   -5,   -2,   -4 }, // 1993 Dec 21, 20:26
    {   -7,   -3,    0,  -10 }, // 1994 Dec 22, 02:23
    {  -13,   -2,    0,   -8 }, // 1995 Dec 22, 08:17
    {  -13,   -6,   -9,  -17 }, // 1996 Dec 21, 14:06
    {  -29,  -18,   -2,   -9 }, // 1997 Dec 21, 20:07
    {  -29,  -22,    0,  -14 }, // 1998 Dec 22, 01:56
    {  -28,  -22,  -11,  -23 }, // 1999 Dec 22, 07:44
    {  -34,  -31,  -12,   -9 }, // 2000 Dec 21, 13:38
    {  -27,  -26,  -10,  -11 }, // 2001 Dec 21, 19:21
    {  -33,  -21,   -7,  -15 }, // 2002 Dec 22, 01:14
    {  -34,  -20,   -4,   -8 }, // 2003 Dec 22, 07:04
    {  -21,  -15,   -4,  -13 }, // 2004 Dec 21, 12:42
    {  -26,  -19,    5,   -4 }, // 2005 Dec 21, 18:35
    {  -24,  -11,   15,   -2 }, // 2006 Dec 22, 00:22
    {  -19,   -2,   10,   -7 }, // 2007 Dec 22, 06:07
    {  -29,  -10,   12,    9 }, // 2008 Dec 21, 12:04
    {  -22,  -10,   19,    7 }, // 2009 Dec 21, 17:47
    {  -25,  -10,   21,    0 }, // 2010 Dec 21, 23:38
    {  -29,  -15,   17,    4 }, // 2011 Dec 22, 05:30
    {  -21,  -14,    9,   -2 }, // 2012 Dec 21, 11:12
    {  -33,  -22,   11,    1 }, // 2013 Dec 21, 17:11
    {  -37,  -21,   13,    0 }, // 2014 Dec 21, 23:03
    {  -33,  -16,    5,  -15 }, // 2015 Dec 22, 04:48
    {  -42,  -29,    3,   -6 }, // 2016 Dec 21, 10:44
    {  -36,  -25,   10,  -10 }, // 2017 Dec 21, 16:28
    {  -42,  -18,   12,  -18 }, // 2018 Dec 21, 22:22
    {  -53,  -22,   12,   -9 }, // 2019 Dec 22, 04:19
    {  -45,  -20,   11,  -10 }, // 2020 Dec 21, 10:02
    {  -56,  -29,   19,   -4 }, // 2021 Dec 21, 15:59
    {  -56,  -31,   26,    0 }, // 2022 Dec 21, 21:48
    {  -44,  -23,   20,   -7 }, // 2023 Dec 22, 03:27
    {  -49,  -31,   17,    8 }, // 2024 Dec 21, 09:20
    {  -42,  -25,   24,   12 }, // 2025 Dec 21, 15:03
    {  -40,  -15,   27,    3 }, // 2026 Dec 21, 20:50
    {  -44,  -19,   24,    9 }, // 2027 Dec 22, 02:42
    {  -31,  -13,   28,    4 }, // 2028 Dec 21, 08:20
    {  -37,  -15,   34,    4 }, // 2029 Dec 21, 14:14
    {  -45,  -16,   37,    5 }, // 2030 Dec 21, 20:09
    {  -41,   -6,   34,   -3 }, // 2031 Dec 22, 01:55
    {  -56,  -21,   30,    5 }, // 2032 Dec 21, 07:56
    {  -57,  -27,   37,    7 }, // 2033 Dec 21, 13:45
    {  -57,  -24,   36,   -5 }, // 2034 Dec 21, 19:34
    {  -67,  -37,   24,   -1 }, // 2035 Dec 22, 01:31
    {  -59,  -36,   23,   -1 }, // 2036 Dec 21, 07:13
    {  -65,  -37,   25,   -1 }, // 2037 Dec 21, 13:07
    {  -73,  -41,   26,    0 }, // 2038 Dec 21, 19:02
    {  -60,  -29,   26,   -8 }, // 2039 Dec 22, 00:40
    {  -65,  -37,   24,    0 }, // 2040 Dec 21, 06:33
    {  -60,  -35,   34,    5 }, // 2041 Dec 21, 12:18
    {  -57,  -17,   42,   -1 }, // 2042 Dec 21, 18:04
    {  -67,  -22,   37,    6 }, // 2043 Dec 22, 00:01
    {  -60,  -20,   45,   10 }, // 2044 Dec 21, 05:44
    {  -63,  -23,   53,   10 }, // 2045 Dec 21, 11:35
    {  -68,  -29,   54,   13 }, // 2046 Dec 21, 17:28
    {  -56,  -20,   51,    9 }, // 2047 Dec 21, 23:07
    {  -64,  -27,   46,   17 }, // 2048 Dec 21, 05:02
    {  -65,  -30,   49,   20 }, // 2049 Dec 21, 10:52
    {  -62,  -19,   54,    8 }, // 2050 Dec 21, 16:38
    {  -70,  -29,   43,    9 }, // 2051 Dec 21, 22:33
    {  -64,  -31,   44,    6 }, // 2052 Dec 21, 04:17
    {  -68,  -30,   51,    1 }, // 2053 Dec 21, 10:09
    {  -82,  -36,   47,    0 }, // 2054 Dec 21, 16:09
    {  -78,  -28,   47,   -1 }, // 2055 Dec 21, 21:55
    {  -87,  -39,   44,    4 }, // 2056 Dec 21, 03:51
    {  -90,  -48,   48,    9 }, // 2057 Dec 21, 09:42
    {  -83,  -37,   56,    1 }, // 2058 Dec 21, 15:25
    {  -88,  -44,   44,    6 }, // 2059 Dec 21, 21:18
    {  -81,  -41,   46,   12 }, // 2060 Dec 21, 03:01
    {  -79,  -33,   58,   12 }, // 2061 Dec 21, 08:48
    {  -85,  -37,   55,   13 }, // 2062 Dec 21, 14:42
    {  -73,  -26,   62,   14 }, // 2063 Dec 21, 20:21
    {  -71,  -27,   64,   17 }, // 2064 Dec 21, 02:08
    {  -75,  -30,   69,   22 }, // 2065 Dec 21, 08:00
    {  -70,  -13,   80,   18 }, // 2066 Dec 21, 13:45
    {  -82,  -21,   70,   19 }, // 2067 Dec 21, 19:43
    {  -82,  -28,   71,   24 }, // 2068 Dec 21, 01:32
    {  -84,  -30,   80,   19 }, // 2069 Dec 21, 07:22
    {  -94,  -43,   69,   13 }, // 2070 Dec 21, 13:19
    {  -88,  -40,   63,   12 }, // 2071 Dec 21, 19:03
    {  -93,  -45,   58,   14 }, // 2072 Dec 21, 00:56
    { -100,  -53,   55,   15 }, // 2073 Dec 21, 06:50
    {  -95,  -40,   63,    7 }, // 2074 Dec 21, 12:35
    {  -99,  -43,   55,    3 }, // 2075 Dec 21, 18:27
    {  -96,  -47,   57,    8 }, // 2076 Dec 21, 00:13
    {  -94,  -37,   73,    8 }, // 2077 Dec 21, 06:00
    { -104,  -38,   70,    7 }, // 2078 Dec 21, 11:57
    { -102,  -33,   74,   14 }, // 2079 Dec 21, 17:44
    { -101,  -34,   82,   22 }, // 2080 Dec 20, 23:32
    { -102,  -43,   84,   27 }, // 2081 Dec 21, 05:22
    {  -94,  -32,   94,   27 }, // 2082 Dec 21, 11:04
    {  -94,  -33,   85,   28 }, // 2083 Dec 21, 16:53
    {  -93,  -39,   81,   34 }, // 2084 Dec 20, 22:41
    {  -91,  -31,   95,   34 }, // 2085 Dec 21, 04:28
    {  -97,  -36,   85,   25 }, // 2086 Dec 21, 10:22
    {  -94,  -35,   83,   24 }, // 2087 Dec 21, 16:08
    {  -93,  -36,   86,   23 }, // 2088 Dec 20, 21:56
    { -102,  -44,   81,   19 }, // 2089 Dec 21, 03:52
    { -105,  -33,   89,   17 }, // 2090 Dec 21, 09:43
    { -113,  -37,   80,   14 }, // 2091 Dec 21, 15:38
    { -118,  -52,   75,   15 }, // 2092 Dec 20, 21:31
    { -118,  -50,   91,   17 }, // 2093 Dec 21, 03:20
    { -123,  -56,   83,   10 }, // 2094 Dec 21, 09:13
    { -121,  -54,   79,   17 }, // 2095 Dec 21, 15:00
    { -118,  -51,   86,   26 }, // 2096 Dec 20, 20:46
    { -119,  -55,   84,   25 }, // 2097 Dec 21, 02:36
    { -113,  -41,   97,   28 }, // 2098 Dec 21, 08:20
    { -108,  -39,   94,   27 }, // 2099 Dec 21, 14:04
    { -105,    0,    0,    0 }, // 2100 Dec 21, 19:51        
};

static const int32_t timeDeltaToSprEquin =  768903; // avg delta in millis/10000 from winter solstice to spring equinox, within 1 hr
static const int32_t timeDeltaToSumSolst = 1570332; // avg delta in millis/10000 from winter solstice to summer solstice, within 2.7 hrs
static const int32_t timeDeltaToAutEquin = 2379459; // avg delta in millis/10000 from winter solstice to autumn equinox, within 2 hrs

// The following three methods, which compute the sun parameters
// given above for an arbitrary epoch (whatever time the object is
// set to), make only a small difference as compared to using the
// above constants.  E.g., Sunset times might differ by ~12
// seconds.  Furthermore, the eta-g computation is befuddled by
// Duffet-Smith's incorrect coefficients (p.86).  I've corrected
// the first-order coefficient but the others may be off too - no
// way of knowing without consulting another source.

//  /**
//   * Return the sun's ecliptic longitude at perigee for the current time.
//   * See Duffett-Smith, p. 86.
//   * @return radians
//   */
//  private double getSunOmegaG() {
//      double T = getJulianCentury();
//      return (281.2208444 + (1.719175 + 0.000452778*T)*T) * DEG_RAD;
//  }

//  /**
//   * Return the sun's ecliptic longitude for the current time.
//   * See Duffett-Smith, p. 86.
//   * @return radians
//   */
//  private double getSunEtaG() {
//      double T = getJulianCentury();
//      //return (279.6966778 + (36000.76892 + 0.0003025*T)*T) * DEG_RAD;
//      //
//      // The above line is from Duffett-Smith, and yields manifestly wrong
//      // results.  The below constant is derived empirically to match the
//      // constant he gives for the 1990 EPOCH.
//      //
//      return (279.6966778 + (-0.3262541582718024 + 0.0003025*T)*T) * DEG_RAD;
//  }

//  /**
//   * Return the sun's eccentricity of orbit for the current time.
//   * See Duffett-Smith, p. 86.
//   * @return double
//   */
//  private double getSunE() {
//      double T = getJulianCentury();
//      return 0.01675104 - (0.0000418 + 0.000000126*T)*T;
//  }

/**
 * Find the "true anomaly" (longitude) of an object from
 * its mean anomaly and the eccentricity of its orbit.  This uses
 * an iterative solution to Kepler's equation.
 *
 * @param meanAnomaly   The object's longitude calculated as if it were in
 *                      a regular, circular orbit, measured in radians
 *                      from the point of perigee.
 *
 * @param eccentricity  The eccentricity of the orbit
 *
 * @return The true anomaly (longitude) measured in radians
 */
static double trueAnomaly(double meanAnomaly, double eccentricity)
{
    // First, solve Kepler's equation iteratively
    // Duffett-Smith, p.90
    double delta;
    double E = meanAnomaly;
    do {
        delta = E - eccentricity * ::sin(E) - meanAnomaly;
        E = E - delta / (1 - eccentricity * ::cos(E));
    }
    while (uprv_fabs(delta) > 1e-5); // epsilon = 1e-5 rad

    return 2.0 * ::atan( ::tan(E/2) * ::sqrt( (1+eccentricity)
                                             /(1-eccentricity) ) );
}


/**
 * Returns sunLongitude which may be adjusted for correctness
 * based on the time, using a table which only has data covering
 * gregorian years 1900-2100.
 * <p>
 * @param theSunLongitude the sunLongitude to be adjusted if necessary
 * @param theTime         the time for which the sunLongitude is to be adjusted
 * @internal
 */
double CalendarAstronomer::adjustSunLongitude(double &theSunLongitude, UDate theTime)
{
    // apply piecewise linear corrections in the range 1900-2100
    if (theTime >= winterSolsticeDatesFirst && theTime < winterSolsticeDatesLast) {
        int32_t offset = (int32_t)(((double)kWinterSolsticeDatesCount)*(theTime - winterSolsticeDatesFirst)/winterSolsticeDatesRange);
        const int32_t * winterSolsticeDatesPtr = winterSolsticeDates + offset; // approximate starting position
        int32_t curTime = (int32_t)(theTime/10000.0);
        while (curTime < *winterSolsticeDatesPtr) {
            winterSolsticeDatesPtr--;
        }
        while (curTime >= *(winterSolsticeDatesPtr+1)) {
            winterSolsticeDatesPtr++;
        }
        // curTime is in the 12-month period beginning with *winterSolsticeDatesPtr
        offset = winterSolsticeDatesPtr - winterSolsticeDates;
        curTime -= *winterSolsticeDatesPtr;
        double factor = 0.0;
        int32_t adjustForStart = 0, adjustForEnd = 0;
        if (curTime < timeDeltaToSumSolst) {
            if (curTime < timeDeltaToSprEquin) {
                // curTime from winter solstice to before spring equinox
                factor = (double)curTime/(double)timeDeltaToSprEquin;
                adjustForStart = sunLongitudeAdjustmts[offset][0];
                adjustForEnd = sunLongitudeAdjustmts[offset][1];
            } else {
                // curTime from spring equinox to before summer solstice
                factor = (double)(curTime - timeDeltaToSprEquin)/(double)(timeDeltaToSumSolst - timeDeltaToSprEquin);
                adjustForStart = sunLongitudeAdjustmts[offset][1];
                adjustForEnd = sunLongitudeAdjustmts[offset][2];
            }
        } else {
            if (curTime < timeDeltaToAutEquin) {
                // curTime from summer solstice to before autumn equinox
                factor = (double)(curTime - timeDeltaToSumSolst)/(double)(timeDeltaToAutEquin - timeDeltaToSumSolst);
                adjustForStart = sunLongitudeAdjustmts[offset][2];
                adjustForEnd = sunLongitudeAdjustmts[offset][3];
            } else {
                // curTime from autumn equinox to before next winter solstice
                factor = (double)(curTime - timeDeltaToAutEquin)/(double)(*(winterSolsticeDatesPtr+1) - *winterSolsticeDatesPtr - timeDeltaToAutEquin);
                adjustForStart = sunLongitudeAdjustmts[offset][3];
                adjustForEnd = sunLongitudeAdjustmts[offset+1][0];
            }
        }
        double adjustmt = ((double)adjustForStart + factor*((double)(adjustForEnd - adjustForStart)))/100000.0;
        theSunLongitude += adjustmt;
        if (theSunLongitude >= 2*PI) {
            theSunLongitude -= 2*PI;
        } else if (theSunLongitude < 0) {
            theSunLongitude += 2*PI;
        }
    }
    return theSunLongitude;
}

/**
 * The longitude of the sun at the time specified by theTime.
 * This does not result in caching of any of the intermediate computations.
 * @internal
 */
double CalendarAstronomer::getSunLongitudeForTime(UDate theTime)
{    
    double jd = (theTime - (double)JULIAN_EPOCH_MS) / (double)DAY_MS;
    double theSunLongitude;
    double theMeanAnomalySun;
 
    getSunLongitude(jd, theSunLongitude, theMeanAnomalySun);
    return CalendarAstronomer::adjustSunLongitude(theSunLongitude, theTime);
}

/**
 * The longitude of the sun at the time specified by this object.
 * The longitude is measured in radians along the ecliptic
 * from the "first point of Aries," the point at which the ecliptic
 * crosses the earth's equatorial plane at the vernal equinox.
 * <p>
 * Currently, this method uses an approximation of the two-body Kepler's
 * equation for the earth and the sun.  It does not take into account the
 * perturbations caused by the other planets, the moon, etc.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
double CalendarAstronomer::getSunLongitude()
{
    // See page 86 of "Practical Astronomy with your Calculator",
    // by Peter Duffet-Smith, for details on the algorithm.

    // Currently this is called externally by ChineseCalendar,
    // and internally by getMoonPosition and getSunTime.
    
    if (isINVALID(sunLongitude)) {
        // this sets instance variables julianDay (from fTime), sunLongitude, meanAnomalySun
        getSunLongitude(getJulianDay(), sunLongitude, meanAnomalySun);
    }
    
    // apply piecewise linear corrections in the range 1900-2100,
    // update sunLongitude as necessary
    return CalendarAstronomer::adjustSunLongitude(sunLongitude, fTime);
}

/**
 * TODO Make this public when the entire class is package-private.
 */
/*public*/ void CalendarAstronomer::getSunLongitude(double jDay, double &longitude, double &meanAnomaly)
{
    // See page 86 of "Practical Astronomy with your Calculator",
    // by Peter Duffet-Smith, for details on the algorithm.

    double day = jDay - JD_EPOCH;       // Days since epoch

    // Find the angular distance the sun in a fictitious
    // circular orbit has travelled since the epoch.
    double epochAngle = norm2PI(CalendarAstronomer_PI2/TROPICAL_YEAR*day);

    // The epoch wasn't at the sun's perigee; find the angular distance
    // since perigee, which is called the "mean anomaly"
    meanAnomaly = norm2PI(epochAngle + SUN_ETA_G - SUN_OMEGA_G);

    // Now find the "true anomaly", e.g. the real solar longitude
    // by solving Kepler's equation for an elliptical orbit
    // NOTE: The 3rd ed. of the book lists omega_g and eta_g in different
    // equations; omega_g is to be correct.
    longitude =  norm2PI(trueAnomaly(meanAnomaly, SUN_E) + SUN_OMEGA_G);
}

/**
 * The position of the sun at this object's current date and time,
 * in equatorial coordinates.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
CalendarAstronomer::Equatorial& CalendarAstronomer::getSunPosition(CalendarAstronomer::Equatorial& result) {
    return eclipticToEquatorial(result, getSunLongitude(), 0);
}


/**
 * Constant representing the vernal equinox.
 * For use with {@link #getSunTime getSunTime}.
 * Note: In this case, "vernal" refers to the northern hemisphere's seasons.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
/*double CalendarAstronomer::VERNAL_EQUINOX() {
  return 0;
}*/

/**
 * Constant representing the summer solstice.
 * For use with {@link #getSunTime getSunTime}.
 * Note: In this case, "summer" refers to the northern hemisphere's seasons.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
double CalendarAstronomer::SUMMER_SOLSTICE() {
    return  (CalendarAstronomer::PI/2);
}

/**
 * Constant representing the autumnal equinox.
 * For use with {@link #getSunTime getSunTime}.
 * Note: In this case, "autumn" refers to the northern hemisphere's seasons.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
/*double CalendarAstronomer::AUTUMN_EQUINOX() {
  return  (CalendarAstronomer::PI);
}*/

/**
 * Constant representing the winter solstice.
 * For use with {@link #getSunTime getSunTime}.
 * Note: In this case, "winter" refers to the northern hemisphere's seasons.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
double CalendarAstronomer::WINTER_SOLSTICE() {
    return  ((CalendarAstronomer::PI*3)/2);
}

CalendarAstronomer::AngleFunc::~AngleFunc() {}

/**
 * Find the next time at which the sun's ecliptic longitude will have
 * the desired value.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
class SunTimeAngleFunc : public CalendarAstronomer::AngleFunc {
public:
    virtual ~SunTimeAngleFunc();
    virtual double eval(CalendarAstronomer& a) { return a.getSunLongitude(); }
};

SunTimeAngleFunc::~SunTimeAngleFunc() {}

UDate CalendarAstronomer::getSunTime(double desired, UBool next)
{
    // Currently, the only client is ChineseCalendar, which calls
    // this with desired == CalendarAstronomer::WINTER_SOLSTICE()
    if (desired == CalendarAstronomer::WINTER_SOLSTICE() && fTime >= winterSolsticeDatesFirst && fTime < winterSolsticeDatesLast) {
        int32_t offset = (int32_t)(((double)kWinterSolsticeDatesCount)*(fTime - winterSolsticeDatesFirst)/winterSolsticeDatesRange);
        const int32_t * winterSolsticeDatesPtr = winterSolsticeDates + offset; // approximate starting position
        int32_t curTime = (int32_t)(fTime/10000.0);
        while (curTime < *winterSolsticeDatesPtr) {
            winterSolsticeDatesPtr--;
        }
        while (curTime >= *(winterSolsticeDatesPtr+1)) {
            winterSolsticeDatesPtr++;
        }
        if (next) {
            winterSolsticeDatesPtr++;
        }
        return  10000.0 * (UDate)(*winterSolsticeDatesPtr);
    }

    SunTimeAngleFunc func;
    return timeOfAngle( func,
                        desired,
                        TROPICAL_YEAR,
                        MINUTE_MS,
                        next);
}

CalendarAstronomer::CoordFunc::~CoordFunc() {}

class RiseSetCoordFunc : public CalendarAstronomer::CoordFunc {
public:
    virtual ~RiseSetCoordFunc();
    virtual void eval(CalendarAstronomer::Equatorial& result, CalendarAstronomer&a) {  a.getSunPosition(result); }
};

RiseSetCoordFunc::~RiseSetCoordFunc() {}

UDate CalendarAstronomer::getSunRiseSet(UBool rise)
{
    UDate t0 = fTime;

    // Make a rough guess: 6am or 6pm local time on the current day
    double noon = ClockMath::floorDivide(fTime + fGmtOffset, (double)DAY_MS)*DAY_MS - fGmtOffset + (12*HOUR_MS);

    U_DEBUG_ASTRO_MSG(("Noon=%.2lf, %sL, gmtoff %.2lf\n", noon, debug_astro_date(noon+fGmtOffset), fGmtOffset));
    setTime(noon +  ((rise ? -6 : 6) * HOUR_MS));
    U_DEBUG_ASTRO_MSG(("added %.2lf ms as a guess,\n", ((rise ? -6. : 6.) * HOUR_MS)));

    RiseSetCoordFunc func;
    double t = riseOrSet(func,
                         rise,
                         .533 * DEG_RAD,        // Angular Diameter
                         34. /60.0 * DEG_RAD,    // Refraction correction
                         MINUTE_MS / 12.);       // Desired accuracy

    setTime(t0);
    return t;
}

// Commented out - currently unused. ICU 2.6, Alan
//    //-------------------------------------------------------------------------
//    // Alternate Sun Rise/Set
//    // See Duffett-Smith p.93
//    //-------------------------------------------------------------------------
//
//    // This yields worse results (as compared to USNO data) than getSunRiseSet().
//    /**
//     * TODO Make this when the entire class is package-private.
//     */
//    /*public*/ long getSunRiseSet2(boolean rise) {
//        // 1. Calculate coordinates of the sun's center for midnight
//        double jd = uprv_floor(getJulianDay() - 0.5) + 0.5;
//        double[] sl = getSunLongitude(jd);//        double lambda1 = sl[0];
//        Equatorial pos1 = eclipticToEquatorial(lambda1, 0);
//
//        // 2. Add ... to lambda to get position 24 hours later
//        double lambda2 = lambda1 + 0.985647*DEG_RAD;
//        Equatorial pos2 = eclipticToEquatorial(lambda2, 0);
//
//        // 3. Calculate LSTs of rising and setting for these two positions
//        double tanL = ::tan(fLatitude);
//        double H = ::acos(-tanL * ::tan(pos1.declination));
//        double lst1r = (CalendarAstronomer_PI2 + pos1.ascension - H) * 24 / CalendarAstronomer_PI2;
//        double lst1s = (pos1.ascension + H) * 24 / CalendarAstronomer_PI2;
//               H = ::acos(-tanL * ::tan(pos2.declination));
//        double lst2r = (CalendarAstronomer_PI2-H + pos2.ascension ) * 24 / CalendarAstronomer_PI2;
//        double lst2s = (H + pos2.ascension ) * 24 / CalendarAstronomer_PI2;
//        if (lst1r > 24) lst1r -= 24;
//        if (lst1s > 24) lst1s -= 24;
//        if (lst2r > 24) lst2r -= 24;
//        if (lst2s > 24) lst2s -= 24;
//
//        // 4. Convert LSTs to GSTs.  If GST1 > GST2, add 24 to GST2.
//        double gst1r = lstToGst(lst1r);
//        double gst1s = lstToGst(lst1s);
//        double gst2r = lstToGst(lst2r);
//        double gst2s = lstToGst(lst2s);
//        if (gst1r > gst2r) gst2r += 24;
//        if (gst1s > gst2s) gst2s += 24;
//
//        // 5. Calculate GST at 0h UT of this date
//        double t00 = utToGst(0);
//
//        // 6. Calculate GST at 0h on the observer's longitude
//        double offset = ::round(fLongitude*12/PI); // p.95 step 6; he _rounds_ to nearest 15 deg.
//        double t00p = t00 - offset*1.002737909;
//        if (t00p < 0) t00p += 24; // do NOT normalize
//
//        // 7. Adjust
//        if (gst1r < t00p) {
//            gst1r += 24;
//            gst2r += 24;
//        }
//        if (gst1s < t00p) {
//            gst1s += 24;
//            gst2s += 24;
//        }
//
//        // 8.
//        double gstr = (24.07*gst1r-t00*(gst2r-gst1r))/(24.07+gst1r-gst2r);
//        double gsts = (24.07*gst1s-t00*(gst2s-gst1s))/(24.07+gst1s-gst2s);
//
//        // 9. Correct for parallax, refraction, and sun's diameter
//        double dec = (pos1.declination + pos2.declination) / 2;
//        double psi = ::acos(sin(fLatitude) / cos(dec));
//        double x = 0.830725 * DEG_RAD; // parallax+refraction+diameter
//        double y = ::asin(sin(x) / ::sin(psi)) * RAD_DEG;
//        double delta_t = 240 * y / cos(dec) / 3600; // hours
//
//        // 10. Add correction to GSTs, subtract from GSTr
//        gstr -= delta_t;
//        gsts += delta_t;
//
//        // 11. Convert GST to UT and then to local civil time
//        double ut = gstToUt(rise ? gstr : gsts);
//        //System.out.println((rise?"rise=":"set=") + ut + ", delta_t=" + delta_t);
//        long midnight = DAY_MS * (time / DAY_MS); // Find UT midnight on this day
//        return midnight + (long) (ut * 3600000);
//    }

// Commented out - currently unused. ICU 2.6, Alan
//    /**
//     * Convert local sidereal time to Greenwich sidereal time.
//     * Section 15.  Duffett-Smith p.21
//     * @param lst in hours (0..24)
//     * @return GST in hours (0..24)
//     */
//    double lstToGst(double lst) {
//        double delta = fLongitude * 24 / CalendarAstronomer_PI2;
//        return normalize(lst - delta, 24);
//    }

// Commented out - currently unused. ICU 2.6, Alan
//    /**
//     * Convert UT to GST on this date.
//     * Section 12.  Duffett-Smith p.17
//     * @param ut in hours
//     * @return GST in hours
//     */
//    double utToGst(double ut) {
//        return normalize(getT0() + ut*1.002737909, 24);
//    }

// Commented out - currently unused. ICU 2.6, Alan
//    /**
//     * Convert GST to UT on this date.
//     * Section 13.  Duffett-Smith p.18
//     * @param gst in hours
//     * @return UT in hours
//     */
//    double gstToUt(double gst) {
//        return normalize(gst - getT0(), 24) * 0.9972695663;
//    }

// Commented out - currently unused. ICU 2.6, Alan
//    double getT0() {
//        // Common computation for UT <=> GST
//
//        // Find JD for 0h UT
//        double jd = uprv_floor(getJulianDay() - 0.5) + 0.5;
//
//        double s = jd - 2451545.0;
//        double t = s / 36525.0;
//        double t0 = 6.697374558 + (2400.051336 + 0.000025862*t)*t;
//        return t0;
//    }

// Commented out - currently unused. ICU 2.6, Alan
//    //-------------------------------------------------------------------------
//    // Alternate Sun Rise/Set
//    // See sci.astro FAQ
//    // http://www.faqs.org/faqs/astronomy/faq/part3/section-5.html
//    //-------------------------------------------------------------------------
//
//    // Note: This method appears to produce inferior accuracy as
//    // compared to getSunRiseSet().
//
//    /**
//     * TODO Make this when the entire class is package-private.
//     */
//    /*public*/ long getSunRiseSet3(boolean rise) {
//
//        // Compute day number for 0.0 Jan 2000 epoch
//        double d = (double)(time - EPOCH_2000_MS) / DAY_MS;
//
//        // Now compute the Local Sidereal Time, LST:
//        //
//        double LST  =  98.9818  +  0.985647352 * d  +  /*UT*15  +  long*/
//            fLongitude*RAD_DEG;
//        //
//        // (east long. positive).  Note that LST is here expressed in degrees,
//        // where 15 degrees corresponds to one hour.  Since LST really is an angle,
//        // it's convenient to use one unit---degrees---throughout.
//
//        //    COMPUTING THE SUN'S POSITION
//        //    ----------------------------
//        //
//        // To be able to compute the Sun's rise/set times, you need to be able to
//        // compute the Sun's position at any time.  First compute the "day
//        // number" d as outlined above, for the desired moment.  Next compute:
//        //
//        double oblecl = 23.4393 - 3.563E-7 * d;
//        //
//        double w  =  282.9404  +  4.70935E-5   * d;
//        double M  =  356.0470  +  0.9856002585 * d;
//        double e  =  0.016709  -  1.151E-9     * d;
//        //
//        // This is the obliquity of the ecliptic, plus some of the elements of
//        // the Sun's apparent orbit (i.e., really the Earth's orbit): w =
//        // argument of perihelion, M = mean anomaly, e = eccentricity.
//        // Semi-major axis is here assumed to be exactly 1.0 (while not strictly
//        // true, this is still an accurate approximation).  Next compute E, the
//        // eccentric anomaly:
//        //
//        double E = M + e*(180/PI) * ::sin(M*DEG_RAD) * ( 1.0 + e*cos(M*DEG_RAD) );
//        //
//        // where E and M are in degrees.  This is it---no further iterations are
//        // needed because we know e has a sufficiently small value.  Next compute
//        // the true anomaly, v, and the distance, r:
//        //
//        /*      r * cos(v)  =  */ double A  =  cos(E*DEG_RAD) - e;
//        /*      r * ::sin(v)  =  */ double B  =  ::sqrt(1 - e*e) * ::sin(E*DEG_RAD);
//        //
//        // and
//        //
//        //      r  =  sqrt( A*A + B*B )
//        double v  =  ::atan2( B, A )*RAD_DEG;
//        //
//        // The Sun's true longitude, slon, can now be computed:
//        //
//        double slon  =  v + w;
//        //
//        // Since the Sun is always at the ecliptic (or at least very very close to
//        // it), we can use simplified formulae to convert slon (the Sun's ecliptic
//        // longitude) to sRA and sDec (the Sun's RA and Dec):
//        //
//        //                   ::sin(slon) * cos(oblecl)
//        //     tan(sRA)  =  -------------------------
//        //            cos(slon)
//        //
//        //     ::sin(sDec) =  ::sin(oblecl) * ::sin(slon)
//        //
//        // As was the case when computing az, the Azimuth, if possible use an
//        // atan2() function to compute sRA.
//
//        double sRA = ::atan2(sin(slon*DEG_RAD) * cos(oblecl*DEG_RAD), cos(slon*DEG_RAD))*RAD_DEG;
//
//        double sin_sDec = ::sin(oblecl*DEG_RAD) * ::sin(slon*DEG_RAD);
//        double sDec = ::asin(sin_sDec)*RAD_DEG;
//
//        //    COMPUTING RISE AND SET TIMES
//        //    ----------------------------
//        //
//        // To compute when an object rises or sets, you must compute when it
//        // passes the meridian and the HA of rise/set.  Then the rise time is
//        // the meridian time minus HA for rise/set, and the set time is the
//        // meridian time plus the HA for rise/set.
//        //
//        // To find the meridian time, compute the Local Sidereal Time at 0h local
//        // time (or 0h UT if you prefer to work in UT) as outlined above---name
//        // that quantity LST0.  The Meridian Time, MT, will now be:
//        //
//        //     MT  =  RA - LST0
//        double MT = normalize(sRA - LST, 360);
//        //
//        // where "RA" is the object's Right Ascension (in degrees!).  If negative,
//        // add 360 deg to MT.  If the object is the Sun, leave the time as it is,
//        // but if it's stellar, multiply MT by 365.2422/366.2422, to convert from
//        // sidereal to solar time.  Now, compute HA for rise/set, name that
//        // quantity HA0:
//        //
//        //                 ::sin(h0)  -  ::sin(lat) * ::sin(Dec)
//        // cos(HA0)  =  ---------------------------------
//        //                      cos(lat) * cos(Dec)
//        //
//        // where h0 is the altitude selected to represent rise/set.  For a purely
//        // mathematical horizon, set h0 = 0 and simplify to:
//        //
//        //    cos(HA0)  =  - tan(lat) * tan(Dec)
//        //
//        // If you want to account for refraction on the atmosphere, set h0 = -35/60
//        // degrees (-35 arc minutes), and if you want to compute the rise/set times
//        // for the Sun's upper limb, set h0 = -50/60 (-50 arc minutes).
//        //
//        double h0 = -50/60 * DEG_RAD;
//
//        double HA0 = ::acos(
//          (sin(h0) - ::sin(fLatitude) * sin_sDec) /
//          (cos(fLatitude) * cos(sDec*DEG_RAD)))*RAD_DEG;
//
//        // When HA0 has been computed, leave it as it is for the Sun but multiply
//        // by 365.2422/366.2422 for stellar objects, to convert from sidereal to
//        // solar time.  Finally compute:
//        //
//        //    Rise time  =  MT - HA0
//        //    Set  time  =  MT + HA0
//        //
//        // convert the times from degrees to hours by dividing by 15.
//        //
//        // If you'd like to check that your calculations are accurate or just
//        // need a quick result, check the USNO's Sun or Moon Rise/Set Table,
//        // <URL:http://aa.usno.navy.mil/AA/data/docs/RS_OneYear.html>.
//
//        double result = MT + (rise ? -HA0 : HA0); // in degrees
//
//        // Find UT midnight on this day
//        long midnight = DAY_MS * (time / DAY_MS);
//
//        return midnight + (long) (result * 3600000 / 15);
//    }

//-------------------------------------------------------------------------
// The Moon
//-------------------------------------------------------------------------

#define moonL0  (318.351648 * CalendarAstronomer::PI/180 )   // Mean long. at epoch
#define moonP0 ( 36.340410 * CalendarAstronomer::PI/180 )   // Mean long. of perigee
#define moonN0 ( 318.510107 * CalendarAstronomer::PI/180 )   // Mean long. of node
#define moonI  (   5.145366 * CalendarAstronomer::PI/180 )   // Inclination of orbit
#define moonE  (   0.054900 )            // Eccentricity of orbit

// These aren't used right now
#define moonA  (   3.84401e5 )           // semi-major axis (km)
#define moonT0 (   0.5181 * CalendarAstronomer::PI/180 )     // Angular size at distance A
#define moonPi (   0.9507 * CalendarAstronomer::PI/180 )     // Parallax at distance A

// new moon date/times 1900-2100 (in UTC)
// These are in UDate/10000.0 (i.e. in units of 10 seconds) to fit into 32 bits.
// sources from e.g.
// http://eclipse.gsfc.nasa.gov/phase/phases2001.html
// http://www.timeanddate.com/calendar/moonphases.html?year=1900&n=0
// From the latter: "The lunation number represents the number of times the Moon has
// cycled the Earth since January 1923 (based on a series described by Ernest W. Brown
// in _Planetary Theory_, 1933). One cycle, or lunation, starts at new moon and lasts
// until the next new moon."
// The mean synodic month (interval from one new moon to the next) is 29.530588853 days,
// but the deviation from the mean is significant and difficult to model. I tried based
// on the description in http://individual.utoronto.ca/kalendis/lunar/index.htm
// using the product of two sine waves (with periods of 109.65 days and 13.944 days and
// different phase shifts and amplitudes) but could not get anywhere near enough accuracy.
// Hence these tables. -Peter E
// These table are 9965 and 4974 bytes but greatly improve both the
// accuracy and speed of the relevant methods for the relevant time range.
// For getMoonTime:
// before fix, errors of up to +23 min / -18 min for next = true and much more for
// next = false; after, errors always less than 1 min).
// 40 to 80 times faster with the fix, depending on starting point.
// For getMoonAge:
// more accurate with the fix (and never returns values >= 2*PI), also more than 11 times faster.
static const int32_t newMoonDates[] = {
//  millis/10K     date                 lunation number
    -221149158, // 1899 Dec  3, 00:47    -285
    -220893888, // 1900 Jan  1, 13:52    -284
    -220639188, // 1900 Jan 31, 01:22    -283
    -220385010, // 1900 Mar  1, 11:25    -282
    -220131180, // 1900 Mar 30, 20:30    -281
    -219877422, // 1900 Apr 29, 05:23    -280
    -219623460, // 1900 May 28, 14:50    -279
    -219369078, // 1900 Jun 27, 01:27    -278
    -219114102, // 1900 Jul 26, 13:43    -277
    -218858442, // 1900 Aug 25, 03:53    -276
    -218602098, // 1900 Sep 23, 19:57    -275
    -218345238, // 1900 Oct 23, 13:27    -274
    -218088258, // 1900 Nov 22, 07:17    -273
    -217831674, // 1900 Dec 22, 00:01    -272
    -217575864, // 1901 Jan 20, 14:36    -271
    -217320930, // 1901 Feb 19, 02:45    -270
    -217066722, // 1901 Mar 20, 12:53    -269
    -216813018, // 1901 Apr 18, 21:37    -268
    -216559572, // 1901 May 18, 05:38    -267
    -216306162, // 1901 Jun 16, 13:33    -266
    -216052500, // 1901 Jul 15, 22:10    -265
    -215798238, // 1901 Aug 14, 08:27    -264
    -215543046, // 1901 Sep 12, 21:19    -263
    -215286768, // 1901 Oct 12, 13:12    -262
    -215029590, // 1901 Nov 11, 07:35    -261
    -214772076, // 1901 Dec 11, 02:54    -260
    -214514910, // 1902 Jan  9, 21:15    -259
    -214258554, // 1902 Feb  8, 13:21    -258
    -214003140, // 1902 Mar 10, 02:50    -257
    -213748620, // 1902 Apr  8, 13:50    -256
    -213494850, // 1902 May  7, 22:45    -255
    -213241614, // 1902 Jun  6, 06:11    -254
    -212988606, // 1902 Jul  5, 12:59    -253
    -212735418, // 1902 Aug  3, 20:17    -252
    -212481606, // 1902 Sep  2, 05:19    -251
    -212226786, // 1902 Oct  1, 17:09    -250
    -211970796, // 1902 Oct 31, 08:14    -249
    -211713810, // 1902 Nov 30, 02:05    -248
    -211456290, // 1902 Dec 29, 21:25    -247
    -211198806, // 1903 Jan 28, 16:39    -246
    -210941880, // 1903 Feb 27, 10:20    -245
    -210685884, // 1903 Mar 29, 01:26    -244
    -210430974, // 1903 Apr 27, 13:31    -243
    -210177060, // 1903 May 26, 22:50    -242
    -209923854, // 1903 Jun 25, 06:11    -241
    -209670924, // 1903 Jul 24, 12:46    -240
    -209417814, // 1903 Aug 22, 19:51    -239
    -209164140, // 1903 Sep 21, 04:30    -238
    -208909620, // 1903 Oct 20, 15:30    -237
    -208654140, // 1903 Nov 19, 05:10    -236
    -208397724, // 1903 Dec 18, 21:26    -235
    -208140558, // 1904 Jan 17, 15:47    -234
    -207883050, // 1904 Feb 16, 11:05    -233
    -207625806, // 1904 Mar 17, 05:39    -232
    -207369402, // 1904 Apr 15, 21:53    -231
    -207114132, // 1904 May 15, 10:58    -230
    -206859900, // 1904 Jun 13, 21:10    -229
    -206606358, // 1904 Jul 13, 05:27    -228
    -206353092, // 1904 Aug 11, 12:58    -227
    -206099742, // 1904 Sep  9, 20:43    -226
    -205846050, // 1904 Oct  9, 05:25    -225
    -205591818, // 1904 Nov  7, 15:37    -224
    -205336884, // 1904 Dec  7, 03:46    -223
    -205081098, // 1905 Jan  5, 18:17    -222
    -204824484, // 1905 Feb  4, 11:06    -221
    -204567366, // 1905 Mar  6, 05:19    -220
    -204310296, // 1905 Apr  4, 23:24    -219
    -204053820, // 1905 May  4, 15:50    -218
    -203798178, // 1905 Jun  3, 05:57    -217
    -203543340, // 1905 Jul  2, 17:50    -216
    -203289102, // 1905 Aug  1, 04:03    -215
    -203035242, // 1905 Aug 30, 13:13    -214
    -202781520, // 1905 Sep 28, 22:00    -213
    -202527732, // 1905 Oct 28, 06:58    -212
    -202273638, // 1905 Nov 26, 16:47    -211
    -202019016, // 1905 Dec 26, 04:04    -210
    -201763746, // 1906 Jan 24, 17:09    -209
    -201507858, // 1906 Feb 23, 07:57    -208
    -201251568, // 1906 Mar 24, 23:52    -207
    -200995158, // 1906 Apr 23, 16:07    -206
    -200738874, // 1906 May 23, 08:01    -205
    -200482884, // 1906 Jun 21, 23:06    -204
    -200227326, // 1906 Jul 21, 12:59    -203
    -199972272, // 1906 Aug 20, 01:28    -202
    -199717722, // 1906 Sep 18, 12:33    -201
    -199463502, // 1906 Oct 17, 22:43    -200
    -199209378, // 1906 Nov 16, 08:37    -199
    -198955116, // 1906 Dec 15, 18:54    -198
    -198700578, // 1907 Jan 14, 05:57    -197
    -198445782, // 1907 Feb 12, 17:43    -196
    -198190770, // 1907 Mar 14, 06:05    -195
    -197935524, // 1907 Apr 12, 19:06    -194
    -197679966, // 1907 May 12, 08:59    -193
    -197424060, // 1907 Jun 10, 23:50    -192
    -197167938, // 1907 Jul 10, 15:17    -191
    -196911858, // 1907 Aug  9, 06:37    -190
    -196656096, // 1907 Sep  7, 21:04    -189
    -196400754, // 1907 Oct  7, 10:21    -188
    -196145766, // 1907 Nov  5, 22:39    -187
    -195890982, // 1907 Dec  5, 10:23    -186
    -195636336, // 1908 Jan  3, 21:44    -185
    -195381864, // 1908 Feb  2, 08:36    -184
    -195127578, // 1908 Mar  2, 18:57    -183
    -194873388, // 1908 Apr  1, 05:02    -182
    -194619042, // 1908 Apr 30, 15:33    -181
    -194364276, // 1908 May 30, 03:14    -180
    -194108934, // 1908 Jun 28, 16:31    -179
    -193853058, // 1908 Jul 28, 07:17    -178
    -193596846, // 1908 Aug 26, 22:59    -177
    -193340526, // 1908 Sep 25, 14:59    -176
    -193084278, // 1908 Oct 25, 06:47    -175
    -192828282, // 1908 Nov 23, 21:53    -174
    -192572700, // 1908 Dec 23, 11:50    -173
    -192317688, // 1909 Jan 22, 00:12    -172
    -192063288, // 1909 Feb 20, 10:52    -171
    -191809374, // 1909 Mar 21, 20:11    -170
    -191555694, // 1909 Apr 20, 04:51    -169
    -191301948, // 1909 May 19, 13:42    -168
    -191047872, // 1909 Jun 17, 23:28    -167
    -190793256, // 1909 Jul 17, 10:44    -166
    -190537950, // 1909 Aug 15, 23:55    -165
    -190281906, // 1909 Sep 14, 15:09    -164
    -190025202, // 1909 Oct 14, 08:13    -163
    -189768132, // 1909 Nov 13, 02:18    -162
    -189511206, // 1909 Dec 12, 19:59    -161
    -189254934, // 1910 Jan 11, 11:51    -160
    -188999562, // 1910 Feb 10, 01:13    -159
    -188745048, // 1910 Mar 11, 12:12    -158
    -188491170, // 1910 Apr  9, 21:25    -157
    -188237688, // 1910 May  9, 05:32    -156
    -187984344, // 1910 Jun  7, 13:16    -155
    -187730880, // 1910 Jul  6, 21:20    -154
    -187476984, // 1910 Aug  5, 06:36    -153
    -187222290, // 1910 Sep  3, 18:05    -152
    -186966528, // 1910 Oct  3, 08:32    -151
    -186709704, // 1910 Nov  2, 01:56    -150
    -186452214, // 1910 Dec  1, 21:11    -149
    -186194754, // 1910 Dec 31, 16:21    -148
    -185937930, // 1911 Jan 30, 09:45    -147
    -185682054, // 1911 Mar  1, 00:31    -146
    -185427132, // 1911 Mar 30, 12:38    -145
    -185173050, // 1911 Apr 28, 22:25    -144
    -184919616, // 1911 May 28, 06:24    -143
    -184666566, // 1911 Jun 26, 13:19    -142
    -184413534, // 1911 Jul 25, 20:11    -141
    -184160076, // 1911 Aug 24, 04:14    -140
    -183905778, // 1911 Sep 22, 14:37    -139
    -183650346, // 1911 Oct 22, 04:09    -138
    -183393786, // 1911 Nov 20, 20:49    -137
    -183136440, // 1911 Dec 20, 15:40    -136
    -182878866, // 1912 Jan 19, 11:09    -135
    -182621616, // 1912 Feb 18, 05:44    -134
    -182365152, // 1912 Mar 18, 22:08    -133
    -182109720, // 1912 Apr 17, 11:40    -132
    -181855362, // 1912 May 16, 22:13    -131
    -181601862, // 1912 Jun 15, 06:23    -130
    -181348842, // 1912 Jul 14, 13:13    -129
    -181095858, // 1912 Aug 12, 19:57    -128
    -180842472, // 1912 Sep 11, 03:48    -127
    -180588360, // 1912 Oct 10, 13:40    -126
    -180333336, // 1912 Nov  9, 02:04    -125
    -180077364, // 1912 Dec  8, 17:06    -124
    -179820552, // 1913 Jan  7, 10:28    -123
    -179563194, // 1913 Feb  6, 05:21    -122
    -179305788, // 1913 Mar  8, 00:22    -121
    -179048952, // 1913 Apr  6, 17:48    -120
    -178793136, // 1913 May  6, 08:24    -119
    -178538418, // 1913 Jun  4, 19:57    -118
    -178284564, // 1913 Jul  4, 05:06    -117
    -178031172, // 1913 Aug  2, 12:58    -116
    -177777858, // 1913 Aug 31, 20:37    -115
    -177524304, // 1913 Sep 30, 04:56    -114
    -177270306, // 1913 Oct 29, 14:29    -113
    -177015714, // 1913 Nov 28, 01:41    -112
    -176760372, // 1913 Dec 27, 14:58    -111
    -176504196, // 1914 Jan 26, 06:34    -110
    -176247348, // 1914 Feb 25, 00:02    -109
    -175990266, // 1914 Mar 26, 18:09    -108
    -175733514, // 1914 Apr 25, 11:21    -107
    -175477476, // 1914 May 25, 02:34    -106
    -175222242, // 1914 Jun 23, 15:33    -105
    -174967692, // 1914 Jul 23, 02:38    -104
    -174713604, // 1914 Aug 21, 12:26    -103
    -174459762, // 1914 Sep 19, 21:33    -102
    -174205962, // 1914 Oct 19, 06:33    -101
    -173951988, // 1914 Nov 17, 16:02    -100
    -173697630, // 1914 Dec 17, 02:35     -99
    -173442708, // 1915 Jan 15, 14:42     -98
    -173187174, // 1915 Feb 14, 04:31     -97
    -172931148, // 1915 Mar 15, 19:42     -96
    -172674870, // 1915 Apr 14, 11:35     -95
    -172418574, // 1915 May 14, 03:31     -94
    -172162458, // 1915 Jun 12, 18:57     -93
    -171906660, // 1915 Jul 12, 09:30     -92
    -171651288, // 1915 Aug 10, 22:52     -91
    -171396408, // 1915 Sep  9, 10:52     -90
    -171141954, // 1915 Oct  8, 21:41     -89
    -170887728, // 1915 Nov  7, 07:52     -88
    -170633502, // 1915 Dec  6, 18:03     -87
    -170379090, // 1916 Jan  5, 04:45     -86
    -170124450, // 1916 Feb  3, 16:05     -85
    -169869612, // 1916 Mar  4, 03:58     -84
    -169614594, // 1916 Apr  2, 16:21     -83
    -169359306, // 1916 May  2, 05:29     -82
    -169103658, // 1916 May 31, 19:37     -81
    -168847662, // 1916 Jun 30, 10:43     -80
    -168591510, // 1916 Jul 30, 02:15     -79
    -168335490, // 1916 Aug 28, 17:25     -78
    -168079836, // 1916 Sep 27, 07:34     -77
    -167824578, // 1916 Oct 26, 20:37     -76
    -167569620, // 1916 Nov 25, 08:50     -75
    -167314854, // 1916 Dec 24, 20:31     -74
    -167060280, // 1917 Jan 23, 07:40     -73
    -166805946, // 1917 Feb 21, 18:09     -72
    -166551810, // 1917 Mar 23, 04:05     -71
    -166297674, // 1917 Apr 21, 14:01     -70
    -166043238, // 1917 May 21, 00:47     -69
    -165788268, // 1917 Jun 19, 13:02     -68
    -165532680, // 1917 Jul 19, 03:00     -67
    -165276594, // 1917 Aug 17, 18:21     -66
    -165020232, // 1917 Sep 16, 10:28     -65
    -164763834, // 1917 Oct 16, 02:41     -64
    -164507592, // 1917 Nov 14, 18:28     -63
    -164251698, // 1917 Dec 14, 09:17     -62
    -163996350, // 1918 Jan 12, 22:35     -61
    -163741656, // 1918 Feb 11, 10:04     -60
    -163487568, // 1918 Mar 12, 19:52     -59
    -163233876, // 1918 Apr 11, 04:34     -58
    -162980274, // 1918 May 10, 13:01     -57
    -162726462, // 1918 Jun  8, 22:03     -56
    -162472188, // 1918 Jul  8, 08:22     -55
    -162217266, // 1918 Aug  6, 20:29     -54
    -161961576, // 1918 Sep  5, 10:44     -53
    -161705130, // 1918 Oct  5, 03:05     -52
    -161448114, // 1918 Nov  3, 21:01     -51
    -161190966, // 1918 Dec  3, 15:19     -50
    -160934256, // 1919 Jan  2, 08:24     -49
    -160678398, // 1919 Jan 31, 23:07     -48
    -160423494, // 1919 Mar  2, 11:11     -47
    -160169370, // 1919 Mar 31, 21:05     -46
    -159915780, // 1919 Apr 30, 05:30     -45
    -159662448, // 1919 May 29, 13:12     -44
    -159409128, // 1919 Jun 27, 20:52     -43
    -159155514, // 1919 Jul 27, 05:21     -42
    -158901258, // 1919 Aug 25, 15:37     -41
    -158646036, // 1919 Sep 24, 04:34     -40
    -158389686, // 1919 Oct 23, 20:39     -39
    -158132400, // 1919 Nov 22, 15:20     -38
    -157874790, // 1919 Dec 22, 10:55     -37
    -157617558, // 1920 Jan 21, 05:27     -36
    -157361196, // 1920 Feb 19, 21:34     -35
    -157105824, // 1920 Mar 20, 10:56     -34
    -156851382, // 1920 Apr 18, 21:43     -33
    -156597690, // 1920 May 18, 06:25     -32
    -156344514, // 1920 Jun 16, 13:41     -31
    -156091530, // 1920 Jul 15, 20:25     -30
    -155838336, // 1920 Aug 14, 03:44     -29
    -155584494, // 1920 Sep 12, 12:51     -28
    -155329620, // 1920 Oct 12, 00:50     -27
    -155073570, // 1920 Nov 10, 16:05     -26
    -154816536, // 1920 Dec 10, 10:04     -25
    -154558998, // 1921 Jan  9, 05:27     -24
    -154301538, // 1921 Feb  8, 00:37     -23
    -154044666, // 1921 Mar  9, 18:09     -22
    -153788730, // 1921 Apr  8, 09:05     -21
    -153533874, // 1921 May  7, 21:01     -20
    -153279990, // 1921 Jun  6, 06:15     -19
    -153026784, // 1921 Jul  5, 13:36     -18
    -152773818, // 1921 Aug  3, 20:17     -17
    -152520642, // 1921 Sep  2, 03:33     -16
    -152266884, // 1921 Oct  1, 12:26     -15
    -152012292, // 1921 Oct 30, 23:38     -14
    -151756770, // 1921 Nov 29, 13:25     -13
    -151500366, // 1921 Dec 29, 05:39     -12
    -151243272, // 1922 Jan 27, 23:48     -11
    -150985878, // 1922 Feb 26, 18:47     -10
    -150728742, // 1922 Mar 28, 13:03      -9
    -150472416, // 1922 Apr 27, 05:04      -8
    -150217176, // 1922 May 26, 18:04      -7
    -149962920, // 1922 Jun 25, 04:20      -6
    -149709318, // 1922 Jul 24, 12:47      -5
    -149455956, // 1922 Aug 22, 20:34      -4
    -149202492, // 1922 Sep 21, 04:38      -3
    -148948680, // 1922 Oct 20, 13:40      -2
    -148694364, // 1922 Nov 19, 00:06      -1
    -148439400, // 1922 Dec 18, 12:20       0
    -148183674, // 1923 Jan 17, 02:41       1
    -147927198, // 1923 Feb 15, 19:07       2
    -147670254, // 1923 Mar 17, 12:51       3
    -147413352, // 1923 Apr 16, 06:28       4
    -147156972, // 1923 May 15, 22:38       5
    -146901348, // 1923 Jun 14, 12:42       6
    -146646450, // 1923 Jul 14, 00:45       7
    -146392098, // 1923 Aug 12, 11:17       8
    -146138088, // 1923 Sep 10, 20:52       9
    -145884210, // 1923 Oct 10, 06:05      10
    -145630278, // 1923 Nov  8, 15:27      11
    -145376100, // 1923 Dec  8, 01:30      12
    -145121478, // 1924 Jan  6, 12:47      13
    -144866292, // 1924 Feb  5, 01:38      14
    -144610578, // 1924 Mar  5, 15:57      15
    -144354498, // 1924 Apr  4, 07:17      16
    -144098280, // 1924 May  3, 23:00      17
    -143842116, // 1924 Jun  2, 14:34      18
    -143586150, // 1924 Jul  2, 05:35      19
    -143330508, // 1924 Jul 31, 19:42      20
    -143075298, // 1924 Aug 30, 08:37      21
    -142820544, // 1924 Sep 28, 20:16      22
    -142566138, // 1924 Oct 28, 06:57      23
    -142311864, // 1924 Nov 26, 17:16      24
    -142057524, // 1924 Dec 26, 03:46      25
    -141803010, // 1925 Jan 24, 14:45      26
    -141548328, // 1925 Feb 23, 02:12      27
    -141293502, // 1925 Mar 24, 14:03      28
    -141038472, // 1925 Apr 23, 02:28      29
    -140783112, // 1925 May 22, 15:48      30
    -140527338, // 1925 Jun 21, 06:17      31
    -140271240, // 1925 Jul 20, 21:40      32
    -140015070, // 1925 Aug 19, 13:15      33
    -139759122, // 1925 Sep 18, 04:13      34
    -139503564, // 1925 Oct 17, 18:06      35
    -139248372, // 1925 Nov 16, 06:58      36
    -138993450, // 1925 Dec 15, 19:05      37
    -138738750, // 1926 Jan 14, 06:35      38
    -138484320, // 1926 Feb 12, 17:20      39
    -138230160, // 1926 Mar 14, 03:20      40
    -137976144, // 1926 Apr 12, 12:56      41
    -137721990, // 1926 May 11, 22:55      42
    -137467392, // 1926 Jun 10, 10:08      43
    -137212164, // 1926 Jul  9, 23:06      44
    -136956306, // 1926 Aug  8, 13:49      45
    -136700010, // 1926 Sep  7, 05:45      46
    -136443522, // 1926 Oct  6, 22:13      47
    -136187076, // 1926 Nov  5, 14:34      48
    -135930888, // 1926 Dec  5, 06:12      49
    -135675192, // 1927 Jan  3, 20:28      50
    -135420156, // 1927 Feb  2, 08:54      51
    -135165816, // 1927 Mar  3, 19:24      52
    -134912016, // 1927 Apr  2, 04:24      53
    -134658486, // 1927 May  1, 12:39      54
    -134404890, // 1927 May 30, 21:05      55
    -134150934, // 1927 Jun 29, 06:31      56
    -133896384, // 1927 Jul 28, 17:36      57
    -133641090, // 1927 Aug 27, 06:45      58
    -133384974, // 1927 Sep 25, 22:11      59
    -133128138, // 1927 Oct 25, 15:37      60
    -132870906, // 1927 Nov 24, 10:09      61
    -132613842, // 1927 Dec 24, 04:13      62
    -132357486, // 1928 Jan 22, 20:19      63
    -132102114, // 1928 Feb 21, 09:41      64
    -131847666, // 1928 Mar 21, 20:29      65
    -131593890, // 1928 Apr 20, 05:25      66
    -131340516, // 1928 May 19, 13:14      67
    -131087268, // 1928 Jun 17, 20:42      68
    -130833870, // 1928 Jul 17, 04:35      69
    -130579992, // 1928 Aug 15, 13:48      70
    -130325280, // 1928 Sep 14, 01:20      71
    -130069464, // 1928 Oct 13, 15:56      72
    -129812550, // 1928 Nov 12, 09:35      73
    -129554964, // 1928 Dec 12, 05:06      74
    -129297432, // 1929 Jan 11, 00:28      75
    -129040590, // 1929 Feb  9, 17:55      76
    -128784738, // 1929 Mar 11, 08:37      77
    -128529882, // 1929 Apr  9, 20:33      78
    -128275872, // 1929 May  9, 06:08      79
    -128022498, // 1929 Jun  7, 13:57      80
    -127769478, // 1929 Jul  6, 20:47      81
    -127516440, // 1929 Aug  5, 03:40      82
    -127262958, // 1929 Sep  3, 11:47      83
    -127008606, // 1929 Oct  2, 22:19      84
    -126753114, // 1929 Nov  1, 12:01      85
    -126496512, // 1929 Dec  1, 04:48      86
    -126239148, // 1929 Dec 30, 23:42      87
    -125981598, // 1930 Jan 29, 19:07      88
    -125724402, // 1930 Feb 28, 13:33      89
    -125467998, // 1930 Mar 30, 05:47      90
    -125212626, // 1930 Apr 28, 19:09      91
    -124958298, // 1930 May 28, 05:37      92
    -124704798, // 1930 Jun 26, 13:47      93
    -124451748, // 1930 Jul 25, 20:42      94
    -124198698, // 1930 Aug 24, 03:37      95
    -123945234, // 1930 Sep 22, 11:41      96
    -123691038, // 1930 Oct 21, 21:47      97
    -123435954, // 1930 Nov 20, 10:21      98
    -123179976, // 1930 Dec 20, 01:24      99
    -122923230, // 1931 Jan 18, 18:35     100
    -122665980, // 1931 Feb 17, 13:10     101
    -122408700, // 1931 Mar 19, 07:50     102
    -122151960, // 1931 Apr 18, 01:00     103
    -121896192, // 1931 May 17, 15:28     104
    -121641468, // 1931 Jun 16, 03:02     105
    -121387560, // 1931 Jul 15, 12:20     106
    -121134078, // 1931 Aug 13, 20:27     107
    -120880644, // 1931 Sep 12, 04:26     108
    -120626964, // 1931 Oct 11, 13:06     109
    -120372870, // 1931 Nov  9, 22:55     110
    -120118224, // 1931 Dec  9, 10:16     111
    -119862906, // 1932 Jan  7, 23:29     112
    -119606850, // 1932 Feb  6, 14:45     113
    -119350176, // 1932 Mar  7, 07:44     114
    -119093274, // 1932 Apr  6, 01:21     115
    -118836654, // 1932 May  5, 18:11     116
    -118580664, // 1932 Jun  4, 09:16     117
    -118325400, // 1932 Jul  3, 22:20     118
    -118070748, // 1932 Aug  2, 09:42     119
    -117816510, // 1932 Aug 31, 19:55     120
    -117562500, // 1932 Sep 30, 05:30     121
    -117308544, // 1932 Oct 29, 14:56     122
    -117054462, // 1932 Nov 28, 00:43     123
    -116800068, // 1932 Dec 27, 11:22     124
    -116545200, // 1933 Jan 25, 23:20     125
    -116289816, // 1933 Feb 24, 12:44     126
    -116034000, // 1933 Mar 26, 03:20     127
    -115777932, // 1933 Apr 24, 18:38     128
    -115521798, // 1933 May 24, 10:07     129
    -115265748, // 1933 Jun 23, 01:22     130
    -115009902, // 1933 Jul 22, 16:03     131
    -114754392, // 1933 Aug 21, 05:48     132
    -114499314, // 1933 Sep 19, 18:21     133
    -114244650, // 1933 Oct 19, 05:45     134
    -113990256, // 1933 Nov 17, 16:24     135
    -113735922, // 1933 Dec 17, 02:53     136
    -113481498, // 1934 Jan 15, 13:37     137
    -113226936, // 1934 Feb 14, 00:44     138
    -112972266, // 1934 Mar 15, 12:09     139
    -112717458, // 1934 Apr 13, 23:57     140
    -112462380, // 1934 May 13, 12:30     141
    -112206894, // 1934 Jun 12, 02:11     142
    -111950964, // 1934 Jul 11, 17:06     143
    -111694764, // 1934 Aug 10, 08:46     144
    -111438600, // 1934 Sep  9, 00:20     145
    -111182730, // 1934 Oct  8, 15:05     146
    -110927262, // 1934 Nov  7, 04:43     147
    -110672130, // 1934 Dec  6, 17:25     148
    -110417280, // 1935 Jan  5, 05:20     149
    -110162718, // 1935 Feb  3, 16:27     150
    -109908480, // 1935 Mar  5, 02:40     151
    -109654494, // 1935 Apr  3, 12:11     152
    -109400538, // 1935 May  2, 21:37     153
    -109146288, // 1935 Jun  1, 07:52     154
    -108891456, // 1935 Jun 30, 19:44     155
    -108635928, // 1935 Jul 30, 09:32     156
    -108379794, // 1935 Aug 29, 01:01     157
    -108123300, // 1935 Sep 27, 17:30     158
    -107866710, // 1935 Oct 27, 10:15     159
    -107610264, // 1935 Nov 26, 02:36     160
    -107354226, // 1935 Dec 25, 17:49     161
    -107098812, // 1936 Jan 24, 07:18     162
    -106844148, // 1936 Feb 22, 18:42     163
    -106590162, // 1936 Mar 23, 04:13     164
    -106336602, // 1936 Apr 21, 12:33     165
    -106083150, // 1936 May 20, 20:35     166
    -105829476, // 1936 Jun 19, 05:14     167
    -105575292, // 1936 Jul 18, 15:18     168
    -105320394, // 1936 Aug 17, 03:21     169
    -105064674, // 1936 Sep 15, 17:41     170
    -104808114, // 1936 Oct 15, 10:21     171
    -104550948, // 1936 Nov 14, 04:42     172
    -104293650, // 1936 Dec 13, 23:25     173
    -104036838, // 1937 Jan 12, 16:47     174
    -103780956, // 1937 Feb 11, 07:34     175
    -103526094, // 1937 Mar 12, 19:31     176
    -103272060, // 1937 Apr 11, 05:10     177
    -103018572, // 1937 May 10, 13:18     178
    -102765342, // 1937 Jun  8, 20:43     179
    -102512082, // 1937 Jul  8, 04:13     180
    -102258498, // 1937 Aug  6, 12:37     181
    -102004242, // 1937 Sep  4, 22:53     182
    -101748972, // 1937 Oct  4, 11:58     183
    -101492544, // 1937 Nov  3, 04:16     184
    -101235174, // 1937 Dec  2, 23:11     185
    -100977486, // 1938 Jan  1, 18:59     186
    -100720230, // 1938 Jan 31, 13:35     187
    -100463880, // 1938 Mar  2, 05:40     188
    -100208568, // 1938 Mar 31, 18:52     189
     -99954192, // 1938 Apr 30, 05:28     190
     -99700560, // 1938 May 29, 14:00     191
     -99447420, // 1938 Jun 27, 21:10     192
     -99194442, // 1938 Jul 27, 03:53     193
     -98941218, // 1938 Aug 25, 11:17     194
     -98687322, // 1938 Sep 23, 20:33     195
     -98432388, // 1938 Oct 23, 08:42     196
     -98176290, // 1938 Nov 22, 00:05     197
     -97919238, // 1938 Dec 21, 18:07     198
     -97661718, // 1939 Jan 20, 13:27     199
     -97404312, // 1939 Feb 19, 08:28     200
     -97147506, // 1939 Mar 21, 01:49     201
     -96891630, // 1939 Apr 19, 16:35     202
     -96636810, // 1939 May 19, 04:25     203
     -96382938, // 1939 Jun 17, 13:37     204
     -96129702, // 1939 Jul 16, 21:03     205
     -95876682, // 1939 Aug 15, 03:53     206
     -95623428, // 1939 Sep 13, 11:22     207
     -95369580, // 1939 Oct 12, 20:30     208
     -95114916, // 1939 Nov 11, 07:54     209
     -94859370, // 1939 Dec 10, 21:45     210
     -94603002, // 1940 Jan  9, 13:53     211
     -94346010, // 1940 Feb  8, 07:45     212
     -94088742, // 1940 Mar  9, 02:23     213
     -93831732, // 1940 Apr  7, 20:18     214
     -93575478, // 1940 May  7, 12:07     215
     -93320250, // 1940 Jun  6, 01:05     216
     -93065952, // 1940 Jul  5, 11:28     217
     -92812266, // 1940 Aug  3, 20:09     218
     -92558790, // 1940 Sep  2, 04:15     219
     -92305194, // 1940 Oct  1, 12:41     220
     -92051262, // 1940 Oct 30, 22:03     221
     -91796868, // 1940 Nov 29, 08:42     222
     -91541904, // 1940 Dec 28, 20:56     223
     -91286262, // 1941 Jan 27, 11:03     224
     -91029948, // 1941 Feb 26, 03:02     225
     -90773196, // 1941 Mar 27, 20:14     226
     -90516456, // 1941 Apr 26, 13:24     227
     -90260172, // 1941 May 26, 05:18     228
     -90004548, // 1941 Jun 24, 19:22     229
     -89749566, // 1941 Jul 24, 07:39     230
     -89495076, // 1941 Aug 22, 18:34     231
     -89240886, // 1941 Sep 21, 04:39     232
     -88986840, // 1941 Oct 20, 14:20     233
     -88732776, // 1941 Nov 19, 00:04     234
     -88478532, // 1941 Dec 18, 10:18     235
     -88223928, // 1942 Jan 16, 21:32     236
     -87968862, // 1942 Feb 15, 10:03     237
     -87713340, // 1942 Mar 16, 23:50     238
     -87457482, // 1942 Apr 15, 14:33     239
     -87201450, // 1942 May 15, 05:45     240
     -86945388, // 1942 Jun 13, 21:02     241
     -86689422, // 1942 Jul 13, 12:03     242
     -86433672, // 1942 Aug 12, 02:28     243
     -86178282, // 1942 Sep 10, 15:53     244
     -85923324, // 1942 Oct 10, 04:06     245
     -85668726, // 1942 Nov  8, 15:19     246
     -85414320, // 1942 Dec  8, 02:00     247
     -85159932, // 1943 Jan  6, 12:38     248
     -84905466, // 1943 Feb  4, 23:29     249
     -84650916, // 1943 Mar  6, 10:34     250
     -84396282, // 1943 Apr  4, 21:53     251
     -84141462, // 1943 May  4, 09:43     252
     -83886282, // 1943 Jun  2, 22:33     253
     -83630616, // 1943 Jul  2, 12:44     254
     -83374524, // 1943 Aug  1, 04:06     255
     -83118240, // 1943 Aug 30, 20:00     256
     -82862106, // 1943 Sep 29, 11:29     257
     -82606326, // 1943 Oct 29, 01:59     258
     -82350942, // 1943 Nov 27, 15:23     259
     -82095900, // 1943 Dec 27, 03:50     260
     -81841176, // 1944 Jan 25, 15:24     261
     -81586806, // 1944 Feb 24, 01:59     262
     -81332784, // 1944 Mar 24, 11:36     263
     -81078936, // 1944 Apr 22, 20:44     264
     -80824962, // 1944 May 22, 06:13     265
     -80570520, // 1944 Jun 20, 17:00     266
     -80315388, // 1944 Jul 20, 05:42     267
     -80059530, // 1944 Aug 18, 20:25     268
     -79803138, // 1944 Sep 17, 12:37     269
     -79546470, // 1944 Oct 17, 05:35     270
     -79289826, // 1944 Nov 15, 22:29     271
     -79033470, // 1944 Dec 15, 14:35     272
     -78777678, // 1945 Jan 14, 05:07     273
     -78522642, // 1945 Feb 12, 17:33     274
     -78268374, // 1945 Mar 14, 03:51     275
     -78014700, // 1945 Apr 12, 12:30     276
     -77761308, // 1945 May 11, 20:22     277
     -77507844, // 1945 Jun 10, 04:26     278
     -77253990, // 1945 Jul  9, 13:35     279
     -76999488, // 1945 Aug  8, 00:32     280
     -76744176, // 1945 Sep  6, 13:44     281
     -76487988, // 1945 Oct  6, 05:22     282
     -76231020, // 1945 Nov  4, 23:10     283
     -75973638, // 1945 Dec  4, 18:07     284
     -75716460, // 1946 Jan  3, 12:30     285
     -75460062, // 1946 Feb  2, 04:43     286
     -75204714, // 1946 Mar  3, 18:01     287
     -74950338, // 1946 Apr  2, 04:37     288
     -74696664, // 1946 May  1, 13:16     289
     -74443386, // 1946 May 30, 20:49     290
     -74190204, // 1946 Jun 29, 04:06     291
     -73936842, // 1946 Jul 28, 11:53     292
     -73682958, // 1946 Aug 26, 21:07     293
     -73428210, // 1946 Sep 25, 08:45     294
     -73172328, // 1946 Oct 24, 23:32     295
     -72915336, // 1946 Nov 23, 17:24     296
     -72657684, // 1946 Dec 23, 13:06     297
     -72400116, // 1947 Jan 22, 08:34     298
     -72143280, // 1947 Feb 21, 02:00     299
     -71887476, // 1947 Mar 22, 16:34     300
     -71632686, // 1947 Apr 21, 04:19     301
     -71378736, // 1947 May 20, 13:44     302
     -71125404, // 1947 Jun 18, 21:26     303
     -70872390, // 1947 Jul 18, 04:15     304
     -70619328, // 1947 Aug 16, 11:12     305
     -70365792, // 1947 Sep 14, 19:28     306
     -70111380, // 1947 Oct 14, 06:10     307
     -69855834, // 1947 Nov 12, 20:01     308
     -69599202, // 1947 Dec 12, 12:53     309
     -69341850, // 1948 Jan 11, 07:45     310
     -69084348, // 1948 Feb 10, 03:02     311
     -68827230, // 1948 Mar 10, 21:15     312
     -68570898, // 1948 Apr  9, 13:17     313
     -68315580, // 1948 May  9, 02:30     314
     -68061264, // 1948 Jun  7, 12:56     315
     -67807746, // 1948 Jul  6, 21:09     316
     -67554642, // 1948 Aug  5, 04:13     317
     -67301514, // 1948 Sep  3, 11:21     318
     -67047948, // 1948 Oct  2, 19:42     319
     -66793662, // 1948 Nov  1, 06:03     320
     -66538530, // 1948 Nov 30, 18:45     321
     -66282570, // 1948 Dec 30, 09:45     322
     -66025908, // 1949 Jan 29, 02:42     323
     -65768790, // 1949 Feb 27, 20:55     324
     -65511654, // 1949 Mar 29, 15:11     325
     -65255028, // 1949 Apr 28, 08:02     326
     -64999296, // 1949 May 27, 22:24     327
     -64744548, // 1949 Jun 26, 10:02     328
     -64490562, // 1949 Jul 25, 19:33     329
     -64236966, // 1949 Aug 24, 03:59     330
     -63983394, // 1949 Sep 22, 12:21     331
     -63729582, // 1949 Oct 21, 21:23     332
     -63475386, // 1949 Nov 20, 07:29     333
     -63220704, // 1949 Dec 19, 18:56     334
     -62965440, // 1950 Jan 18, 08:00     335
     -62709522, // 1950 Feb 16, 22:53     336
     -62453040, // 1950 Mar 18, 15:20     337
     -62196330, // 1950 Apr 17, 08:25     338
     -61939830, // 1950 May 17, 00:55     339
     -61683882, // 1950 Jun 15, 15:53     340
     -61428564, // 1950 Jul 15, 05:06     341
     -61173792, // 1950 Aug 13, 16:48     342
     -60919386, // 1950 Sep 12, 03:29     343
     -60665202, // 1950 Oct 11, 13:33     344
     -60411090, // 1950 Nov  9, 23:25     345
     -60156906, // 1950 Dec  9, 09:29     346
     -59902500, // 1951 Jan  7, 20:10     347
     -59647716, // 1951 Feb  6, 07:54     348
     -59392494, // 1951 Mar  7, 20:51     349
     -59136888, // 1951 Apr  6, 10:52     350
     -58881024, // 1951 May  6, 01:36     351
     -58625040, // 1951 Jun  4, 16:40     352
     -58369032, // 1951 Jul  4, 07:48     353
     -58113126, // 1951 Aug  2, 22:39     354
     -57857460, // 1951 Sep  1, 12:50     355
     -57602178, // 1951 Oct  1, 01:57     356
     -57347316, // 1951 Oct 30, 13:54     357
     -57092760, // 1951 Nov 29, 01:00     358
     -56838342, // 1951 Dec 28, 11:43     359
     -56583924, // 1952 Jan 26, 22:26     360
     -56329464, // 1952 Feb 25, 09:16     361
     -56074962, // 1952 Mar 25, 20:13     362
     -55820352, // 1952 Apr 24, 07:28     363
     -55565472, // 1952 May 23, 19:28     364
     -55310130, // 1952 Jun 22, 08:45     365
     -55054254, // 1952 Jul 21, 23:31     366
     -54797994, // 1952 Aug 20, 15:21     367
     -54541668, // 1952 Sep 19, 07:22     368
     -54285582, // 1952 Oct 18, 22:43     369
     -54029904, // 1952 Nov 17, 12:56     370
     -53774628, // 1952 Dec 17, 02:02     371
     -53519712, // 1953 Jan 15, 14:08     372
     -53265180, // 1953 Feb 14, 01:10     373
     -53011050, // 1953 Mar 15, 11:05     374
     -52757226, // 1953 Apr 13, 20:09     375
     -52503444, // 1953 May 13, 05:06     376
     -52249350, // 1953 Jun 11, 14:55     377
     -51994632, // 1953 Jul 11, 02:28     378
     -51739140, // 1953 Aug  9, 16:10     379
     -51482952, // 1953 Sep  8, 07:48     380
     -51226314, // 1953 Oct  8, 00:41     381
     -50969532, // 1953 Nov  6, 17:58     382
     -50712912, // 1953 Dec  6, 10:48     383
     -50456754, // 1954 Jan  5, 02:21     384
     -50201310, // 1954 Feb  3, 15:55     385
     -49946694, // 1954 Mar  5, 03:11     386
     -49692810, // 1954 Apr  3, 12:25     387
     -49439382, // 1954 May  2, 20:23     388
     -49186062, // 1954 Jun  1, 04:03     389
     -48932484, // 1954 Jun 30, 12:26     390
     -48678360, // 1954 Jul 29, 22:20     391
     -48423474, // 1954 Aug 28, 10:21     392
     -48167694, // 1954 Sep 27, 00:51     393
     -47911038, // 1954 Oct 26, 17:47     394
     -47653734, // 1954 Nov 25, 12:31     395
     -47396316, // 1954 Dec 25, 07:34     396
     -47139438, // 1955 Jan 24, 01:07     397
     -46883556, // 1955 Feb 22, 15:54     398
     -46628748, // 1955 Mar 24, 03:42     399
     -46374804, // 1955 Apr 22, 13:06     400
     -46121406, // 1955 May 21, 20:59     401
     -45868248, // 1955 Jun 20, 04:12     402
     -45615036, // 1955 Jul 19, 11:34     403
     -45361452, // 1955 Aug 17, 19:58     404
     -45107166, // 1955 Sep 16, 06:19     405
     -44851848, // 1955 Oct 15, 19:32     406
     -44595348, // 1955 Nov 14, 12:02     407
     -44337918, // 1955 Dec 14, 07:07     408
     -44080194, // 1956 Jan 13, 03:01     409
     -43822932, // 1956 Feb 11, 21:38     410
     -43566618, // 1956 Mar 12, 13:37     411
     -43311366, // 1956 Apr 11, 02:39     412
     -43057056, // 1956 May 10, 13:04     413
     -42803466, // 1956 Jun  8, 21:29     414
     -42550332, // 1956 Jul  8, 04:38     415
     -42297330, // 1956 Aug  6, 11:25     416
     -42044058, // 1956 Sep  4, 18:57     417
     -41790090, // 1956 Oct  4, 04:25     418
     -41535096, // 1956 Nov  2, 16:44     419
     -41278962, // 1956 Dec  2, 08:13     420
     -41021916, // 1957 Jan  1, 02:14     421
     -40764450, // 1957 Jan 30, 21:25     422
     -40507122, // 1957 Mar  1, 16:13     423
     -40250406, // 1957 Mar 31, 09:19     424
     -39994596, // 1957 Apr 29, 23:54     425
     -39739806, // 1957 May 29, 11:39     426
     -39485916, // 1957 Jun 27, 20:54     427
     -39232632, // 1957 Jul 27, 04:28     428
     -38979522, // 1957 Aug 25, 11:33     429
     -38726166, // 1957 Sep 23, 19:19     430
     -38472222, // 1957 Oct 23, 04:43     431
     -38217486, // 1957 Nov 21, 16:19     432
     -37961928, // 1957 Dec 21, 06:12     433
     -37705632, // 1958 Jan 19, 22:08     434
     -37448772, // 1958 Feb 18, 15:38     435
     -37191660, // 1958 Mar 20, 09:50     436
     -36934782, // 1958 Apr 19, 03:23     437
     -36678600, // 1958 May 18, 19:00     438
     -36423366, // 1958 Jun 17, 07:59     439
     -36169002, // 1958 Jul 16, 18:33     440
     -35915202, // 1958 Aug 15, 03:33     441
     -35661588, // 1958 Sep 13, 12:02     442
     -35407848, // 1958 Oct 12, 20:52     443
     -35153796, // 1958 Nov 11, 06:34     444
     -34899342, // 1958 Dec 10, 17:23     445
     -34644396, // 1959 Jan  9, 05:34     446
     -34388868, // 1959 Feb  7, 19:22     447
     -34132734, // 1959 Mar  9, 10:51     448
     -33876186, // 1959 Apr  8, 03:29     449
     -33619608, // 1959 May  7, 20:12     450
     -33363402, // 1959 Jun  6, 11:53     451
     -33107760, // 1959 Jul  6, 02:00     452
     -32852676, // 1959 Aug  4, 14:34     453
     -32598030, // 1959 Sep  3, 01:55     454
     -32343654, // 1959 Oct  2, 12:31     455
     -32089434, // 1959 Oct 31, 22:41     456
     -31835244, // 1959 Nov 30, 08:46     457
     -31580946, // 1959 Dec 29, 19:09     458
     -31326390, // 1960 Jan 28, 06:15     459
     -31071462, // 1960 Feb 26, 18:23     460
     -30816138, // 1960 Mar 27, 07:37     461
     -30560496, // 1960 Apr 25, 21:44     462
     -30304644, // 1960 May 25, 12:26     463
     -30048678, // 1960 Jun 24, 03:27     464
     -29792694, // 1960 Jul 23, 18:31     465
     -29536830, // 1960 Aug 22, 09:15     466
     -29281242, // 1960 Sep 20, 23:13     467
     -29026068, // 1960 Oct 20, 12:02     468
     -28771284, // 1960 Nov 18, 23:46     469
     -28516758, // 1960 Dec 18, 10:47     470
     -28262340, // 1961 Jan 16, 21:30     471
     -28007940, // 1961 Feb 15, 08:10     472
     -27753534, // 1961 Mar 16, 18:51     473
     -27499092, // 1961 Apr 15, 05:38     474
     -27244476, // 1961 May 14, 16:54     475
     -26989464, // 1961 Jun 13, 05:16     476
     -26733894, // 1961 Jul 12, 19:11     477
     -26477784, // 1961 Aug 11, 10:36     478
     -26221380, // 1961 Sep 10, 02:50     479
     -25965042, // 1961 Oct  9, 18:53     480
     -25709052, // 1961 Nov  8, 09:58     481
     -25453488, // 1961 Dec  7, 23:52     482
     -25198350, // 1962 Jan  6, 12:35     483
     -24943620, // 1962 Feb  5, 00:10     484
     -24689334, // 1962 Mar  6, 10:31     485
     -24435450, // 1962 Apr  4, 19:45     486
     -24181770, // 1962 May  4, 04:25     487
     -23927958, // 1962 Jun  2, 13:27     488
     -23673648, // 1962 Jul  1, 23:52     489
     -23418576, // 1962 Jul 31, 12:24     490
     -23162706, // 1962 Aug 30, 03:09     491
     -22906206, // 1962 Sep 28, 19:39     492
     -22649370, // 1962 Oct 28, 13:05     493
     -22392546, // 1962 Nov 27, 06:29     494
     -22136046, // 1962 Dec 26, 22:59     495
     -21880188, // 1963 Jan 25, 13:42     496
     -21625164, // 1963 Feb 24, 02:06     497
     -21370986, // 1963 Mar 25, 12:09     498
     -21117432, // 1963 Apr 23, 20:28     499
     -20864160, // 1963 May 23, 04:00     500
     -20610804, // 1963 Jun 21, 11:46     501
     -20357022, // 1963 Jul 20, 20:43     502
     -20102556, // 1963 Aug 19, 07:34     503
     -19847214, // 1963 Sep 17, 20:51     504
     -19590942, // 1963 Oct 17, 12:43     505
     -19333860, // 1963 Nov 16, 06:50     506
     -19076364, // 1963 Dec 16, 02:06     507
     -18819096, // 1964 Jan 14, 20:44     508
     -18562674, // 1964 Feb 13, 13:01     509
     -18307356, // 1964 Mar 14, 02:14     510
     -18053052, // 1964 Apr 12, 12:38     511
     -17799468, // 1964 May 11, 21:02     512
     -17546268, // 1964 Jun 10, 04:22     513
     -17293134, // 1964 Jul  9, 11:31     514
     -17039778, // 1964 Aug  7, 19:17     515
     -16785876, // 1964 Sep  6, 04:34     516
     -16531080, // 1964 Oct  5, 16:20     517
     -16275144, // 1964 Nov  4, 07:16     518
     -16018092, // 1964 Dec  4, 01:18     519
     -15760398, // 1965 Jan  2, 21:07     520
     -15502824, // 1965 Feb  1, 16:36     521
     -15246024, // 1965 Mar  3, 09:56     522
     -14990274, // 1965 Apr  2, 00:21     523
     -14735544, // 1965 May  1, 11:56     524
     -14481642, // 1965 May 30, 21:13     525
     -14228322, // 1965 Jun 29, 04:53     526
     -13975290, // 1965 Jul 28, 11:45     527
     -13722174, // 1965 Aug 26, 18:51     528
     -13468572, // 1965 Sep 25, 03:18     529
     -13214088, // 1965 Oct 24, 14:12     530
     -12958500, // 1965 Nov 23, 04:10     531
     -12701862, // 1965 Dec 22, 21:03     532
     -12444558, // 1966 Jan 21, 15:47     533
     -12187146, // 1966 Feb 20, 10:49     534
     -11930124, // 1966 Mar 22, 04:46     535
     -11673870, // 1966 Apr 20, 20:35     536
     -11418582, // 1966 May 20, 09:43     537
     -11164266, // 1966 Jun 18, 20:09     538
     -10910700, // 1966 Jul 18, 04:30     539
     -10657512, // 1966 Aug 16, 11:48     540
     -10404282, // 1966 Sep 14, 19:13     541
     -10150608, // 1966 Oct 14, 03:52     542
      -9896238, // 1966 Nov 12, 14:27     543
      -9641076, // 1966 Dec 12, 03:14     544
      -9385164, // 1967 Jan 10, 18:06     545
      -9128616, // 1967 Feb  9, 10:44     546
      -8871660, // 1967 Mar 11, 04:30     547
      -8614680, // 1967 Apr  9, 22:20     548
      -8358150, // 1967 May  9, 14:55     549
      -8102436, // 1967 Jun  8, 05:14     550
      -7847640, // 1967 Jul  7, 17:00     551
      -7593552, // 1967 Aug  6, 02:48     552
      -7339818, // 1967 Sep  4, 11:37     553
      -7086096, // 1967 Oct  3, 20:24     554
      -6832152, // 1967 Nov  2, 05:48     555
      -6577860, // 1967 Dec  1, 16:10     556
      -6323166, // 1967 Dec 31, 03:39     557
      -6067980, // 1968 Jan 29, 16:30     558
      -5812224, // 1968 Feb 28, 06:56     559
      -5555952, // 1968 Mar 28, 22:48     560
      -5299434, // 1968 Apr 27, 15:21     561
      -5043060, // 1968 May 27, 07:30     562
      -4787130, // 1968 Jun 25, 22:25     563
      -4531740, // 1968 Jul 25, 11:50     564
      -4276818, // 1968 Aug 23, 23:57     565
      -4022232, // 1968 Sep 22, 11:08     566
      -3767856, // 1968 Oct 21, 21:44     567
      -3513588, // 1968 Nov 20, 08:02     568
      -3259326, // 1968 Dec 19, 18:19     569
      -3004926, // 1969 Jan 18, 04:59     570
      -2750250, // 1969 Feb 16, 16:25     571
      -2495208, // 1969 Mar 18, 04:52     572
      -2239824, // 1969 Apr 16, 18:16     573
      -1984164, // 1969 May 16, 08:26     574
      -1728306, // 1969 Jun 14, 23:09     575
      -1472328, // 1969 Jul 14, 14:12     576
      -1216338, // 1969 Aug 13, 05:17     577
       -960504, // 1969 Sep 11, 19:56     578
       -705006, // 1969 Oct 11, 09:39     579
       -449934, // 1969 Nov  9, 22:11     580
       -195228, // 1969 Dec  9, 09:42     581
         59250, // 1970 Jan  7, 20:35     582
        313638, // 1970 Feb  6, 07:13     583
        567978, // 1970 Mar  7, 17:43     584
        822300, // 1970 Apr  6, 04:10     585
       1076706, // 1970 May  5, 14:51     586
       1331406, // 1970 Jun  4, 02:21     587
       1586628, // 1970 Jul  3, 15:18     588
       1842468, // 1970 Aug  2, 05:58     589
       2098812, // 1970 Aug 31, 22:02     590
       2355312, // 1970 Sep 30, 14:32     591
       2611608, // 1970 Oct 30, 06:28     592
       2867484, // 1970 Nov 28, 21:14     593
       3122892, // 1970 Dec 28, 10:42     594
       3377850, // 1971 Jan 26, 22:55     595
       3632328, // 1971 Feb 25, 09:48     596
       3886338, // 1971 Mar 26, 19:23     597
       4140012, // 1971 Apr 25, 04:02     598
       4393632, // 1971 May 24, 12:32     599
       4647582, // 1971 Jun 22, 21:57     600
       4902210, // 1971 Jul 22, 09:15     601
       5157678, // 1971 Aug 20, 22:53     602
       5413938, // 1971 Sep 19, 14:43     603
       5670714, // 1971 Oct 19, 07:59     604
       5927676, // 1971 Nov 18, 01:46     605
       6184458, // 1971 Dec 17, 19:03     606
       6440712, // 1972 Jan 16, 10:52     607
       6696174, // 1972 Feb 15, 00:29     608
       6950730, // 1972 Mar 15, 11:35     609
       7204506, // 1972 Apr 13, 20:31     610
       7457808, // 1972 May 13, 04:08     611
       7711020, // 1972 Jun 11, 11:30     612
       7964514, // 1972 Jul 10, 19:39     613
       8218596, // 1972 Aug  9, 05:26     614
       8473488, // 1972 Sep  7, 17:28     615
       8729328, // 1972 Oct  7, 08:08     616
       8986086, // 1972 Nov  6, 01:21     617
       9243504, // 1972 Dec  5, 20:24     618
       9501018, // 1973 Jan  4, 15:43     619
       9757938, // 1973 Feb  3, 09:23     620
      10013802, // 1973 Mar  5, 00:07     621
      10268550, // 1973 Apr  3, 11:45     622
      10522410, // 1973 May  2, 20:55     623
      10775724, // 1973 Jun  1, 04:34     624
      11028834, // 1973 Jun 30, 11:39     625
      11282034, // 1973 Jul 29, 18:59     626
      11535630, // 1973 Aug 28, 03:25     627
      11789964, // 1973 Sep 26, 13:54     628
      12045342, // 1973 Oct 26, 03:17     629
      12301890, // 1973 Nov 24, 19:55     630
      12559362, // 1973 Dec 24, 15:07     631
      12817092, // 1974 Jan 23, 11:02     632
      13074324, // 1974 Feb 22, 05:34     633
      13330584, // 1974 Mar 23, 21:24     634
      13585782, // 1974 Apr 22, 10:17     635
      13840044, // 1974 May 21, 20:34     636
      14093616, // 1974 Jun 20, 04:56     637
      14346756, // 1974 Jul 19, 12:06     638
      14599806, // 1974 Aug 17, 19:01     639
      14853150, // 1974 Sep 16, 02:45     640
      15107190, // 1974 Oct 15, 12:25     641
      15362238, // 1974 Nov 14, 00:53     642
      15618390, // 1974 Dec 13, 16:25     643
      15875400, // 1975 Jan 12, 10:20     644
      16132782, // 1975 Feb 11, 05:17     645
      16390008, // 1975 Mar 12, 23:48     646
      16646634, // 1975 Apr 11, 16:39     647
      16902390, // 1975 May 11, 07:05     648
      17157174, // 1975 Jun  9, 18:49     649
      17411100, // 1975 Jul  9, 04:10     650
      17664462, // 1975 Aug  7, 11:57     651
      17917668, // 1975 Sep  5, 19:18     652
      18171138, // 1975 Oct  5, 03:23     653
      18425184, // 1975 Nov  3, 13:04     654
      18679980, // 1975 Dec  3, 00:50     655
      18935520, // 1976 Jan  1, 14:40     656
      19191720, // 1976 Jan 31, 06:20     657
      19448430, // 1976 Feb 29, 23:25     658
      19705368, // 1976 Mar 30, 17:08     659
      19962120, // 1976 Apr 29, 10:20     660
      20218242, // 1976 May 29, 01:47     661
      20473500, // 1976 Jun 27, 14:50     662
      20727954, // 1976 Jul 27, 01:39     663
      20981880, // 1976 Aug 25, 11:00     664
      21235650, // 1976 Sep 23, 19:55     665
      21489540, // 1976 Oct 23, 05:10     666
      21743706, // 1976 Nov 21, 15:11     667
      21998208, // 1976 Dec 21, 02:08     668
      22253106, // 1977 Jan 19, 14:11     669
      22508502, // 1977 Feb 18, 03:37     670
      22764438, // 1977 Mar 19, 18:33     671
      23020776, // 1977 Apr 18, 10:36     672
      23277192, // 1977 May 18, 02:52     673
      23533338, // 1977 Jun 16, 18:23     674
      23789022, // 1977 Jul 16, 08:37     675
      24044226, // 1977 Aug 14, 21:31     676
      24299058, // 1977 Sep 13, 09:23     677
      24553626, // 1977 Oct 12, 20:31     678
      24808020, // 1977 Nov 11, 07:10     679
      25062318, // 1977 Dec 10, 17:33     680
      25316640, // 1978 Jan  9, 04:00     681
      25571124, // 1978 Feb  7, 14:54     682
      25825896, // 1978 Mar  9, 02:36     683
      26081010, // 1978 Apr  7, 15:15     684
      26336442, // 1978 May  7, 04:47     685
      26592132, // 1978 Jun  5, 19:02     686
      26848026, // 1978 Jul  5, 09:51     687
      27104046, // 1978 Aug  4, 01:01     688
      27360054, // 1978 Sep  2, 16:09     689
      27615846, // 1978 Oct  2, 06:41     690
      27871236, // 1978 Oct 31, 20:06     691
      28126194, // 1978 Nov 30, 08:19     692
      28380816, // 1978 Dec 29, 19:36     693
      28635234, // 1979 Jan 28, 06:19     694
      28889550, // 1979 Feb 26, 16:45     695
      29143794, // 1979 Mar 28, 02:59     696
      29398050, // 1979 Apr 26, 13:15     697
      29652480, // 1979 May 26, 00:00     698
      29907348, // 1979 Jun 24, 11:58     699
      30162846, // 1979 Jul 24, 01:41     700
      30418986, // 1979 Aug 22, 17:11     701
      30675522, // 1979 Sep 21, 09:47     702
      30932058, // 1979 Oct 21, 02:23     703
      31188264, // 1979 Nov 19, 18:04     704
      31443978, // 1979 Dec 19, 08:23     705
      31699200, // 1980 Jan 17, 21:20     706
      31953906, // 1980 Feb 16, 08:51     707
      32208096, // 1980 Mar 16, 18:56     708
      32461836, // 1980 Apr 15, 03:46     709
      32715360, // 1980 May 14, 12:00     710
      32969034, // 1980 Jun 12, 20:39     711
      33223236, // 1980 Jul 12, 06:46     712
      33478260, // 1980 Aug 10, 19:10     713
      33734166, // 1980 Sep  9, 10:01     714
      33990780, // 1980 Oct  9, 02:50     715
      34247778, // 1980 Nov  7, 20:43     716
      34504770, // 1980 Dec  7, 14:35     717
      34761384, // 1981 Jan  6, 07:24     718
      35017284, // 1981 Feb  4, 22:14     719
      35272266, // 1981 Mar  6, 10:31     720
      35526360, // 1981 Apr  4, 20:20     721
      35779794, // 1981 May  4, 04:19     722
      36032952, // 1981 Jun  2, 11:32     723
      36286218, // 1981 Jul  1, 19:03     724
      36539952, // 1981 Jul 31, 03:52     725
      36794424, // 1981 Aug 29, 14:44     726
      37049808, // 1981 Sep 28, 04:08     727
      37306164, // 1981 Oct 27, 20:14     728
      37563348, // 1981 Nov 26, 14:38     729
      37820940, // 1981 Dec 26, 10:10     730
      38078256, // 1982 Jan 25, 04:56     731
      38334684, // 1982 Feb 23, 21:14     732
      38589948, // 1982 Mar 25, 10:18     733
      38844174, // 1982 Apr 23, 20:29     734
      39097686, // 1982 May 23, 04:41     735
      39350832, // 1982 Jun 21, 11:52     736
      39603942, // 1982 Jul 20, 18:57     737
      39857310, // 1982 Aug 19, 02:45     738
      40111254, // 1982 Sep 17, 12:09     739
      40366104, // 1982 Oct 17, 00:04     740
      40622100, // 1982 Nov 15, 15:10     741
      40879188, // 1982 Dec 15, 09:18     742
      41136888, // 1983 Jan 14, 05:08     743
      41394432, // 1983 Feb 13, 00:32     744
      41651184, // 1983 Mar 14, 17:44     745
      41906874, // 1983 Apr 13, 07:59     746
      42161550, // 1983 May 12, 19:25     747
      42415428, // 1983 Jun 11, 04:38     748
      42668754, // 1983 Jul 10, 12:19     749
      42921828, // 1983 Aug  8, 19:18     750
      43175010, // 1983 Sep  7, 02:35     751
      43428696, // 1983 Oct  6, 11:16     752
      43683246, // 1983 Nov  4, 22:21     753
      43938876, // 1983 Dec  4, 12:26     754
      44195496, // 1984 Jan  3, 05:16     755
      44452722, // 1984 Feb  1, 23:47     756
      44710026, // 1984 Mar  2, 18:31     757
      44966940, // 1984 Apr  1, 12:10     758
      45223116, // 1984 May  1, 03:46     759
      45478368, // 1984 May 30, 16:48     760
      45732714, // 1984 Jun 29, 03:19     761
      45986346, // 1984 Jul 28, 11:51     762
      46239636, // 1984 Aug 26, 19:26     763
      46492986, // 1984 Sep 25, 03:11     764
      46746768, // 1984 Oct 24, 12:08     765
      47001222, // 1984 Nov 22, 22:57     766
      47256402, // 1984 Dec 22, 11:47     767
      47512248, // 1985 Jan 21, 02:28     768
      47768658, // 1985 Feb 19, 18:43     769
      48025434, // 1985 Mar 21, 11:59     770
      48282252, // 1985 Apr 20, 05:22     771
      48538686, // 1985 May 19, 21:41     772
      48794388, // 1985 Jun 18, 11:58     773
      49049262, // 1985 Jul 17, 23:57     774
      49303476, // 1985 Aug 16, 10:06     775
      49557360, // 1985 Sep 14, 19:20     776
      49811238, // 1985 Oct 14, 04:33     777
      50065326, // 1985 Nov 12, 14:21     778
      50319690, // 1985 Dec 12, 00:55     779
      50574372, // 1986 Jan 10, 12:22     780
      50829456, // 1986 Feb  9, 00:56     781
      51085032, // 1986 Mar 10, 14:52     782
      51341088, // 1986 Apr  9, 06:08     783
      51597420, // 1986 May  8, 22:10     784
      51853686, // 1986 Jun  7, 14:01     785
      52109610, // 1986 Jul  7, 04:55     786
      52365096, // 1986 Aug  5, 18:36     787
      52620186, // 1986 Sep  4, 07:11     788
      52874970, // 1986 Oct  3, 18:55     789
      53129532, // 1986 Nov  2, 06:02     790
      53383938, // 1986 Dec  1, 16:43     791
      53638260, // 1986 Dec 31, 03:10     792
      53892630, // 1987 Jan 29, 13:45     793
      54147186, // 1987 Feb 28, 00:51     794
      54402036, // 1987 Mar 29, 12:46     795
      54657204, // 1987 Apr 28, 01:34     796
      54912678, // 1987 May 27, 15:13     797
      55168422, // 1987 Jun 26, 05:37     798
      55424388, // 1987 Jul 25, 20:38     799
      55680474, // 1987 Aug 24, 11:59     800
      55936494, // 1987 Sep 23, 03:09     801
      56192208, // 1987 Oct 22, 17:28     802
      56447478, // 1987 Nov 21, 06:33     803
      56702310, // 1987 Dec 20, 18:25     804
      56956836, // 1988 Jan 19, 05:26     805
      57211164, // 1988 Feb 17, 15:54     806
      57465372, // 1988 Mar 18, 02:02     807
      57719520, // 1988 Apr 16, 12:00     808
      57973746, // 1988 May 15, 22:11     809
      58228284, // 1988 Jun 14, 09:14     810
      58483398, // 1988 Jul 13, 21:53     811
      58739226, // 1988 Aug 12, 12:31     812
      58995660, // 1988 Sep 11, 04:50     813
      59252334, // 1988 Oct 10, 21:49     814
      59508840, // 1988 Nov  9, 14:20     815
      59764896, // 1988 Dec  9, 05:36     816
      60020412, // 1989 Jan  7, 19:22     817
      60275382, // 1989 Feb  6, 07:37     818
      60529794, // 1989 Mar  7, 18:19     819
      60783678, // 1989 Apr  6, 03:33     820
      61037202, // 1989 May  5, 11:47     821
      61290678, // 1989 Jun  3, 19:53     822
      61544514, // 1989 Jul  3, 04:59     823
      61799076, // 1989 Aug  1, 16:06     824
      62054550, // 1989 Aug 31, 05:45     825
      62310888, // 1989 Sep 29, 21:48     826
      62567808, // 1989 Oct 29, 15:28     827
      62824926, // 1989 Nov 28, 09:41     828
      63081840, // 1989 Dec 28, 03:20     829
      63338160, // 1990 Jan 26, 19:20     830
      63593610, // 1990 Feb 25, 08:55     831
      63848088, // 1990 Mar 26, 19:48     832
      64101768, // 1990 Apr 25, 04:28     833
      64354962, // 1990 May 24, 11:47     834
      64608090, // 1990 Jun 22, 18:55     835
      64861524, // 1990 Jul 22, 02:54     836
      65115594, // 1990 Aug 20, 12:39     837
      65370516, // 1990 Sep 19, 00:46     838
      65626422, // 1990 Oct 18, 15:37     839
      65883270, // 1990 Nov 17, 09:05     840
      66140772, // 1990 Dec 17, 04:22     841
      66398340, // 1991 Jan 15, 23:50     842
      66655272, // 1991 Feb 14, 17:32     843
      66911106, // 1991 Mar 16, 08:11     844
      67165788, // 1991 Apr 14, 19:38     845
      67419576, // 1991 May 14, 04:36     846
      67672836, // 1991 Jun 12, 12:06     847
      67925916, // 1991 Jul 11, 19:06     848
      68179122, // 1991 Aug 10, 02:27     849
      68432766, // 1991 Sep  8, 11:01     850
      68687154, // 1991 Oct  7, 21:39     851
      68942586, // 1991 Nov  6, 11:11     852
      69199176, // 1991 Dec  6, 03:56     853
      69456660, // 1992 Jan  4, 23:10     854
      69714360, // 1992 Feb  3, 19:00     855
      69971538, // 1992 Mar  4, 13:23     856
      70227732, // 1992 Apr  3, 05:02     857
      70482870, // 1992 May  2, 17:45     858
      70737102, // 1992 Jun  1, 03:57     859
      70990668, // 1992 Jun 30, 12:18     860
      71243850, // 1992 Jul 29, 19:35     861
      71496972, // 1992 Aug 28, 02:42     862
      71750400, // 1992 Sep 26, 10:40     863
      72004524, // 1992 Oct 25, 20:34     864
      72259626, // 1992 Nov 24, 09:11     865
      72515778, // 1992 Dec 24, 00:43     866
      72772722, // 1993 Jan 22, 18:27     867
      73029996, // 1993 Feb 21, 13:06     868
      73287090, // 1993 Mar 23, 07:15     869
      73543614, // 1993 Apr 21, 23:49     870
      73799322, // 1993 May 21, 14:07     871
      74054118, // 1993 Jun 20, 01:53     872
      74308104, // 1993 Jul 19, 11:24     873
      74561568, // 1993 Aug 17, 19:28     874
      74814900, // 1993 Sep 16, 03:10     875
      75068496, // 1993 Oct 15, 11:36     876
      75322644, // 1993 Nov 13, 21:34     877
      75577482, // 1993 Dec 13, 09:27     878
      75832980, // 1994 Jan 11, 23:10     879
      76089060, // 1994 Feb 10, 14:30     880
      76345590, // 1994 Mar 12, 07:05     881
      76602342, // 1994 Apr 11, 00:17     882
      76858962, // 1994 May 10, 17:07     883
      77115042, // 1994 Jun  9, 08:27     884
      77370342, // 1994 Jul  8, 21:37     885
      77624910, // 1994 Aug  7, 08:45     886
      77878998, // 1994 Sep  5, 18:33     887
      78132930, // 1994 Oct  5, 03:55     888
      78386970, // 1994 Nov  3, 13:35     889
      78641244, // 1994 Dec  2, 23:54     890
      78895770, // 1995 Jan  1, 10:55     891
      79150608, // 1995 Jan 30, 22:48     892
      79405848, // 1995 Mar  1, 11:48     893
      79661568, // 1995 Mar 31, 02:08     894
      79917696, // 1995 Apr 29, 17:36     895
      80173962, // 1995 May 29, 09:27     896
      80430060, // 1995 Jun 28, 00:50     897
      80685798, // 1995 Jul 27, 15:13     898
      80941146, // 1995 Aug 26, 04:31     899
      81196170, // 1995 Sep 24, 16:55     900
      81450936, // 1995 Oct 24, 04:36     901
      81705498, // 1995 Nov 22, 15:43     902
      81959892, // 1995 Dec 22, 02:22     903
      82214220, // 1996 Jan 20, 12:50     904
      82468620, // 1996 Feb 18, 23:30     905
      82723230, // 1996 Mar 19, 10:45     906
      82978134, // 1996 Apr 17, 22:49     907
      83233356, // 1996 May 17, 11:46     908
      83488896, // 1996 Jun 16, 01:36     909
      83744730, // 1996 Jul 15, 16:15     910
      84000804, // 1996 Aug 14, 07:34     911
      84256968, // 1996 Sep 12, 23:08     912
      84512970, // 1996 Oct 12, 14:15     913
      84768576, // 1996 Nov 11, 04:16     914
      85023696, // 1996 Dec 10, 16:56     915
      85278396, // 1997 Jan  9, 04:26     916
      85532796, // 1997 Feb  7, 15:06     917
      85787004, // 1997 Mar  9, 01:14     918
      86041092, // 1997 Apr  7, 11:02     919
      86295162, // 1997 May  6, 20:47     920
      86549424, // 1997 Jun  5, 07:04     921
      86804160, // 1997 Jul  4, 18:40     922
      87059604, // 1997 Aug  3, 08:14     923
      87315792, // 1997 Sep  1, 23:52     924
      87572472, // 1997 Oct  1, 16:52     925
      87829212, // 1997 Oct 31, 10:02     926
      88085604, // 1997 Nov 30, 02:14     927
      88341462, // 1997 Dec 29, 16:57     928
      88596726, // 1998 Jan 28, 06:01     929
      88851396, // 1998 Feb 26, 17:26     930
      89105484, // 1998 Mar 28, 03:14     931
      89359086, // 1998 Apr 26, 11:41     932
      89612472, // 1998 May 25, 19:32     933
      89866020, // 1998 Jun 24, 03:50     934
      90120144, // 1998 Jul 23, 13:44     935
      90375138, // 1998 Aug 22, 02:03     936
      90631092, // 1998 Sep 20, 17:02     937
      90887820, // 1998 Oct 20, 10:10     938
      91144962, // 1998 Nov 19, 04:27     939
      91402098, // 1998 Dec 18, 22:43     940
      91658796, // 1999 Jan 17, 15:46     941
      91914714, // 1999 Feb 16, 06:39     942
      92169648, // 1999 Mar 17, 18:48     943
      92423652, // 1999 Apr 16, 04:22     944
      92676990, // 1999 May 15, 12:05     945
      92930058, // 1999 Jun 13, 19:03     946
      93183264, // 1999 Jul 13, 02:24     947
      93436974, // 1999 Aug 11, 11:09     948
      93691452, // 1999 Sep  9, 22:02     949
      93946890, // 1999 Oct  9, 11:35     950
      94203318, // 1999 Nov  8, 03:53     951
      94460592, // 1999 Dec  7, 22:32     952
      94718244, // 2000 Jan  6, 18:14     953
      94975584, // 2000 Feb  5, 13:04     954
      95231982, // 2000 Mar  6, 05:17     955
      95487192, // 2000 Apr  4, 18:12     956
      95741352, // 2000 May  4, 04:12     957
      95994804, // 2000 Jun  2, 12:14     958
      96247920, // 2000 Jul  1, 19:20     959
      96501030, // 2000 Jul 31, 02:25     960
      96754434, // 2000 Aug 29, 10:19     961
      97008438, // 2000 Sep 27, 19:53     962
      97263348, // 2000 Oct 27, 07:58     963
      97519392, // 2000 Nov 25, 23:12     964
      97776492, // 2000 Dec 25, 17:22     965
      98034162, // 2001 Jan 24, 13:07     966
      98291652, // 2001 Feb 23, 08:22     967
      98548332, // 2001 Mar 25, 01:22     968
      98803956, // 2001 Apr 23, 15:26     969
      99058602, // 2001 May 23, 02:47     970
      99312468, // 2001 Jun 21, 11:58     971
      99565830, // 2001 Jul 20, 19:45     972
      99818970, // 2001 Aug 19, 02:55     973
     100072248, // 2001 Sep 17, 10:28     974
     100326024, // 2001 Oct 16, 19:24     975
     100580640, // 2001 Nov 15, 06:40     976
     100836288, // 2001 Dec 14, 20:48     977
     101092854, // 2002 Jan 13, 13:29     978
     101349966, // 2002 Feb 12, 07:41     979
     101607138, // 2002 Mar 14, 02:03     980
     101863926, // 2002 Apr 12, 19:21     981
     102120030, // 2002 May 12, 10:45     982
     102375282, // 2002 Jun 10, 23:47     983
     102629676, // 2002 Jul 10, 10:26     984
     102883410, // 2002 Aug  8, 19:15     985
     103136820, // 2002 Sep  7, 03:10     986
     103390308, // 2002 Oct  6, 11:18     987
     103644210, // 2002 Nov  4, 20:35     988
     103898730, // 2002 Dec  4, 07:35     989
     104153898, // 2003 Jan  2, 20:23     990
     104409654, // 2003 Feb  1, 10:49     991
     104665890, // 2003 Mar  3, 02:35     992
     104922468, // 2003 Apr  1, 19:18     993
     105179130, // 2003 May  1, 12:15     994
     105435480, // 2003 May 31, 04:20     995
     105691194, // 2003 Jun 29, 18:39     996
     105946158, // 2003 Jul 29, 06:53     997
     106200516, // 2003 Aug 27, 17:26     998
     106454574, // 2003 Sep 26, 03:09     999
     106708620, // 2003 Oct 25, 12:50    1000
     106962834, // 2003 Nov 23, 22:59    1001
     107217258, // 2003 Dec 23, 09:43    1002
     107471910, // 2004 Jan 21, 21:05    1003
     107726868, // 2004 Feb 20, 09:18    1004
     107982246, // 2004 Mar 20, 22:41    1005
     108238086, // 2004 Apr 19, 13:21    1006
     108494232, // 2004 May 19, 04:52    1007
     108750402, // 2004 Jun 17, 20:27    1008
     109006344, // 2004 Jul 17, 11:24    1009
     109261944, // 2004 Aug 16, 01:24    1010
     109517214, // 2004 Sep 14, 14:29    1011
     109772208, // 2004 Oct 14, 02:48    1012
     110026962, // 2004 Nov 12, 14:27    1013
     110281494, // 2004 Dec 12, 01:29    1014
     110535858, // 2005 Jan 10, 12:03    1015
     110790168, // 2005 Feb  8, 22:28    1016
     111044586, // 2005 Mar 10, 09:11    1017
     111299232, // 2005 Apr  8, 20:32    1018
     111554190, // 2005 May  8, 08:45    1019
     111809490, // 2005 Jun  6, 21:55    1020
     112065138, // 2005 Jul  6, 12:03    1021
     112321110, // 2005 Aug  5, 03:05    1022
     112577316, // 2005 Sep  3, 18:46    1023
     112833528, // 2005 Oct  3, 10:28    1024
     113089470, // 2005 Nov  2, 01:25    1025
     113344926, // 2005 Dec  1, 15:01    1026
     113599872, // 2005 Dec 31, 03:12    1027
     113854410, // 2006 Jan 29, 14:15    1028
     114108666, // 2006 Feb 28, 00:31    1029
     114362730, // 2006 Mar 29, 10:15    1030
     114616704, // 2006 Apr 27, 19:44    1031
     114870756, // 2006 May 27, 05:26    1032
     115125150, // 2006 Jun 25, 16:05    1033
     115380186, // 2006 Jul 25, 04:31    1034
     115636020, // 2006 Aug 23, 19:10    1035
     115892550, // 2006 Sep 22, 11:45    1036
     116149404, // 2006 Oct 22, 05:14    1037
     116406108, // 2006 Nov 20, 22:18    1038
     116662326, // 2006 Dec 20, 14:01    1039
     116917926, // 2007 Jan 19, 04:01    1040
     117172884, // 2007 Feb 17, 16:14    1041
     117427218, // 2007 Mar 19, 02:43    1042
     117680976, // 2007 Apr 17, 11:36    1043
     117934362, // 2007 May 16, 19:27    1044
     118187718, // 2007 Jun 15, 03:13    1045
     118441464, // 2007 Jul 14, 12:04    1046
     118695972, // 2007 Aug 12, 23:02    1047
     118951464, // 2007 Sep 11, 12:44    1048
     119207886, // 2007 Oct 11, 05:01    1049
     119464938, // 2007 Nov  9, 23:03    1050
     119722200, // 2007 Dec  9, 17:40    1051
     119979222, // 2008 Jan  8, 11:37    1052
     120235584, // 2008 Feb  7, 03:44    1053
     120491004, // 2008 Mar  7, 17:14    1054
     120745410, // 2008 Apr  6, 03:55    1055
     120998988, // 2008 May  5, 12:18    1056
     121252098, // 2008 Jun  3, 19:23    1057
     121505154, // 2008 Jul  3, 02:19    1058
     121758552, // 2008 Aug  1, 10:12    1059
     122012628, // 2008 Aug 30, 19:58    1060
     122267592, // 2008 Sep 29, 08:12    1061
     122523564, // 2008 Oct 28, 23:14    1062
     122780490, // 2008 Nov 27, 16:55    1063
     123038058, // 2008 Dec 27, 12:23    1064
     123295656, // 2009 Jan 26, 07:56    1065
     123552570, // 2009 Feb 25, 01:35    1066
     123808356, // 2009 Mar 26, 16:06    1067
     124062978, // 2009 Apr 25, 03:23    1068
     124316706, // 2009 May 24, 12:11    1069
     124569930, // 2009 Jun 22, 19:35    1070
     124823010, // 2009 Jul 22, 02:35    1071
     125076246, // 2009 Aug 20, 10:01    1072
     125329944, // 2009 Sep 18, 18:44    1073
     125584398, // 2009 Oct 18, 05:33    1074
     125839884, // 2009 Nov 16, 19:14    1075
     126096492, // 2009 Dec 16, 12:02    1076
     126353952, // 2010 Jan 15, 07:12    1077
     126611592, // 2010 Feb 14, 02:52    1078
     126868686, // 2010 Mar 15, 21:01    1079
     127124814, // 2010 Apr 14, 12:29    1080
     127379910, // 2010 May 14, 01:05    1081
     127634130, // 2010 Jun 12, 11:15    1082
     127887726, // 2010 Jul 11, 19:41    1083
     128140968, // 2010 Aug 10, 03:08    1084
     128394180, // 2010 Sep  8, 10:30    1085
     128647704, // 2010 Oct  7, 18:44    1086
     128901912, // 2010 Nov  6, 04:52    1087
     129157056, // 2010 Dec  5, 17:36    1088
     129413178, // 2011 Jan  4, 09:03    1089
     129670026, // 2011 Feb  3, 02:31    1090
     129927156, // 2011 Mar  4, 20:46    1091
     130184112, // 2011 Apr  3, 14:32    1092
     130440546, // 2011 May  3, 06:51    1093
     130696218, // 2011 Jun  1, 21:03    1094
     130951044, // 2011 Jul  1, 08:54    1095
     131205120, // 2011 Jul 30, 18:40    1096
     131458704, // 2011 Aug 29, 03:04    1097
     131712174, // 2011 Sep 27, 11:09    1098
     131965896, // 2011 Oct 26, 19:56    1099
     132220140, // 2011 Nov 25, 06:10    1100
     132474996, // 2011 Dec 24, 18:06    1101
     132730434, // 2012 Jan 23, 07:39    1102
     132986370, // 2012 Feb 21, 22:35    1103
     133242702, // 2012 Mar 22, 14:37    1104
     133499274, // 2012 Apr 21, 07:19    1105
     133755762, // 2012 May 20, 23:47    1106
     134011818, // 2012 Jun 19, 15:03    1107
     134267190, // 2012 Jul 19, 04:25    1108
     134521890, // 2012 Aug 17, 15:55    1109
     134776146, // 2012 Sep 16, 02:11    1110
     135030258, // 2012 Oct 15, 12:03    1111
     135284448, // 2012 Nov 13, 22:08    1112
     135538812, // 2012 Dec 13, 08:42    1113
     135793344, // 2013 Jan 11, 19:44    1114
     136048080, // 2013 Feb 10, 07:20    1115
     136303146, // 2013 Mar 11, 19:51    1116
     136558656, // 2013 Apr 10, 09:36    1117
     136814574, // 2013 May 10, 00:29    1118
     137070702, // 2013 Jun  8, 15:57    1119
     137326770, // 2013 Jul  8, 07:15    1120
     137582586, // 2013 Aug  6, 21:51    1121
     137838102, // 2013 Sep  5, 11:37    1122
     138093330, // 2013 Oct  5, 00:35    1123
     138348300, // 2013 Nov  3, 12:50    1124
     138603018, // 2013 Dec  3, 00:23    1125
     138857484, // 2014 Jan  1, 11:14    1126
     139111794, // 2014 Jan 30, 21:39    1127
     139366080, // 2014 Mar  1, 08:00    1128
     139620510, // 2014 Mar 30, 18:45    1129
     139875210, // 2014 Apr 29, 06:15    1130
     140130240, // 2014 May 28, 18:40    1131
     140385654, // 2014 Jun 27, 08:09    1132
     140641452, // 2014 Jul 26, 22:42    1133
     140897598, // 2014 Aug 25, 14:13    1134
     141153924, // 2014 Sep 24, 06:14    1135
     141410142, // 2014 Oct 23, 21:57    1136
     141665958, // 2014 Nov 22, 12:33    1137
     141921216, // 2014 Dec 22, 01:36    1138
     142175964, // 2015 Jan 20, 13:14    1139
     142430322, // 2015 Feb 18, 23:47    1140
     142684416, // 2015 Mar 20, 09:36    1141
     142938342, // 2015 Apr 18, 18:57    1142
     143192238, // 2015 May 18, 04:13    1143
     143446350, // 2015 Jun 16, 14:05    1144
     143700990, // 2015 Jul 16, 01:25    1145
     143956404, // 2015 Aug 14, 14:54    1146
     144212652, // 2015 Sep 13, 06:42    1147
     144469476, // 2015 Oct 13, 00:06    1148
     144726408, // 2015 Nov 11, 17:48    1149
     144982980, // 2015 Dec 11, 10:30    1150
     145238946, // 2016 Jan 10, 01:31    1151
     145494234, // 2016 Feb  8, 14:39    1152
     145748850, // 2016 Mar  9, 01:55    1153
     146002824, // 2016 Apr  7, 11:24    1154
     146256300, // 2016 May  6, 19:30    1155
     146509560, // 2016 Jun  5, 03:00    1156
     146763006, // 2016 Jul  4, 11:01    1157
     147017070, // 2016 Aug  2, 20:45    1158
     147272064, // 2016 Sep  1, 09:04    1159
     147528072, // 2016 Oct  1, 00:12    1160
     147784914, // 2016 Oct 30, 17:39    1161
     148042194, // 2016 Nov 29, 12:19    1162
     148299444, // 2016 Dec 29, 06:54    1163
     148556202, // 2017 Jan 28, 00:07    1164
     148812114, // 2017 Feb 26, 14:59    1165
     149066988, // 2017 Mar 28, 02:58    1166
     149320896, // 2017 Apr 26, 12:16    1167
     149574150, // 2017 May 25, 19:45    1168
     149827146, // 2017 Jun 24, 02:31    1169
     150080316, // 2017 Jul 23, 09:46    1170
     150334020, // 2017 Aug 21, 18:30    1171
     150588540, // 2017 Sep 20, 05:30    1172
     150844032, // 2017 Oct 19, 19:12    1173
     151100532, // 2017 Nov 18, 11:42    1174
     151357860, // 2017 Dec 18, 06:30    1175
     151615542, // 2018 Jan 17, 02:17    1176
     151872870, // 2018 Feb 15, 21:05    1177
     152129232, // 2018 Mar 17, 13:12    1178
     152384382, // 2018 Apr 16, 01:57    1179
     152638488, // 2018 May 15, 11:48    1180
     152891898, // 2018 Jun 13, 19:43    1181
     153145008, // 2018 Jul 13, 02:48    1182
     153398148, // 2018 Aug 11, 09:58    1183
     153651606, // 2018 Sep  9, 18:01    1184
     153905682, // 2018 Oct  9, 03:47    1185
     154160652, // 2018 Nov  7, 16:02    1186
     154416720, // 2018 Dec  7, 07:20    1187
     154673808, // 2019 Jan  6, 01:28    1188
     154931424, // 2019 Feb  4, 21:04    1189
     155188824, // 2019 Mar  6, 16:04    1190
     155445420, // 2019 Apr  5, 08:50    1191
     155700996, // 2019 May  4, 22:46    1192
     155955612, // 2019 Jun  3, 10:02    1193
     156209496, // 2019 Jul  2, 19:16    1194
     156462912, // 2019 Aug  1, 03:12    1195
     156716142, // 2019 Aug 30, 10:37    1196
     156969516, // 2019 Sep 28, 18:26    1197
     157223394, // 2019 Oct 28, 03:39    1198
     157478076, // 2019 Nov 26, 15:06    1199
     157733718, // 2019 Dec 26, 05:13    1200
     157990212, // 2020 Jan 24, 21:42    1201
     158247192, // 2020 Feb 23, 15:32    1202
     158504208, // 2020 Mar 24, 09:28    1203
     158760876, // 2020 Apr 23, 02:26    1204
     159016914, // 2020 May 22, 17:39    1205
     159272172, // 2020 Jun 21, 06:42    1206
     159526638, // 2020 Jul 20, 17:33    1207
     159780486, // 2020 Aug 19, 02:41    1208
     160034040, // 2020 Sep 17, 11:00    1209
     160287666, // 2020 Oct 16, 19:31    1210
     160541682, // 2020 Nov 15, 05:07    1211
     160796262, // 2020 Dec 14, 16:17    1212
     161051400, // 2021 Jan 13, 05:00    1213
     161307036, // 2021 Feb 11, 19:06    1214
     161563086, // 2021 Mar 13, 10:21    1215
     161819466, // 2021 Apr 12, 02:31    1216
     162075960, // 2021 May 11, 19:00    1217
     162332238, // 2021 Jun 10, 10:53    1218
     162587982, // 2021 Jul 10, 01:17    1219
     162843060, // 2021 Aug  8, 13:50    1220
     163097592, // 2021 Sep  7, 00:52    1221
     163351830, // 2021 Oct  6, 11:05    1222
     163606050, // 2021 Nov  4, 21:15    1223
     163860378, // 2021 Dec  4, 07:43    1224
     164114844, // 2022 Jan  2, 18:34    1225
     164369436, // 2022 Feb  1, 05:46    1226
     164624250, // 2022 Mar  2, 17:35    1227
     164879424, // 2022 Apr  1, 06:24    1228
     165135048, // 2022 Apr 30, 20:28    1229
     165391020, // 2022 May 30, 11:30    1230
     165647112, // 2022 Jun 29, 02:52    1231
     165903090, // 2022 Jul 28, 17:55    1232
     166158822, // 2022 Aug 27, 08:17    1233
     166414284, // 2022 Sep 25, 21:54    1234
     166669488, // 2022 Oct 25, 10:48    1235
     166924422, // 2022 Nov 23, 22:57    1236
     167179062, // 2022 Dec 23, 10:17    1237
     167433438, // 2023 Jan 21, 20:53    1238
     167687676, // 2023 Feb 20, 07:06    1239
     167941938, // 2023 Mar 21, 17:23    1240
     168196398, // 2023 Apr 20, 04:13    1241
     168451158, // 2023 May 19, 15:53    1242
     168706302, // 2023 Jun 18, 04:37    1243
     168961872, // 2023 Jul 17, 18:32    1244
     169217868, // 2023 Aug 16, 09:38    1245
     169474200, // 2023 Sep 15, 01:40    1246
     169730610, // 2023 Oct 14, 17:55    1247
     169986762, // 2023 Nov 13, 09:27    1248
     170242392, // 2023 Dec 12, 23:32    1249
     170497422, // 2024 Jan 11, 11:57    1250
     170751954, // 2024 Feb  9, 22:59    1251
     171006126, // 2024 Mar 10, 09:01    1252
     171260046, // 2024 Apr  8, 18:21    1253
     171513852, // 2024 May  8, 03:22    1254
     171767748, // 2024 Jun  6, 12:38    1255
     172022022, // 2024 Jul  5, 22:57    1256
     172276998, // 2024 Aug  4, 11:13    1257
     172532856, // 2024 Sep  3, 01:56    1258
     172789500, // 2024 Oct  2, 18:50    1259
     173046528, // 2024 Nov  1, 12:48    1260
     173303412, // 2024 Dec  1, 06:22    1261
     173559762, // 2024 Dec 30, 22:27    1262
     173815416, // 2025 Jan 29, 12:36    1263
     174070350, // 2025 Feb 28, 00:45    1264
     174324588, // 2025 Mar 29, 10:58    1265
     174578232, // 2025 Apr 27, 19:32    1266
     174831498, // 2025 May 27, 03:03    1267
     175084752, // 2025 Jun 25, 10:32    1268
     175338426, // 2025 Jul 24, 19:11    1269
     175592916, // 2025 Aug 23, 06:06    1270
     175848444, // 2025 Sep 21, 19:54    1271
     176104956, // 2025 Oct 21, 12:26    1272
     176362128, // 2025 Nov 20, 06:48    1273
     176619504, // 2025 Dec 20, 01:44    1274
     176876592, // 2026 Jan 18, 19:52    1275
     177132966, // 2026 Feb 17, 12:01    1276
     177388344, // 2026 Mar 19, 01:24    1277
     177642672, // 2026 Apr 17, 11:52    1278
     177896166, // 2026 May 16, 20:01    1279
     178149204, // 2026 Jun 15, 02:54    1280
     178402224, // 2026 Jul 14, 09:44    1281
     178655616, // 2026 Aug 12, 17:36    1282
     178909722, // 2026 Sep 11, 03:27    1283
     179164740, // 2026 Oct 10, 15:50    1284
     179420772, // 2026 Nov  9, 07:02    1285
     179677752, // 2026 Dec  9, 00:52    1286
     179935344, // 2027 Jan  7, 20:24    1287
     180192936, // 2027 Feb  6, 15:56    1288
     180449820, // 2027 Mar  8, 09:30    1289
     180705546, // 2027 Apr  6, 23:51    1290
     180960114, // 2027 May  6, 10:59    1291
     181213806, // 2027 Jun  4, 19:41    1292
     181467012, // 2027 Jul  4, 03:02    1293
     181720110, // 2027 Aug  2, 10:05    1294
     181973406, // 2027 Aug 31, 17:41    1295
     182227176, // 2027 Sep 30, 02:36    1296
     182481696, // 2027 Oct 29, 13:36    1297
     182737224, // 2027 Nov 28, 03:24    1298
     182993832, // 2027 Dec 27, 20:12    1299
     183251238, // 2028 Jan 26, 15:13    1300
     183508788, // 2028 Feb 25, 10:38    1301
     183765792, // 2028 Mar 26, 04:32    1302
     184021842, // 2028 Apr 24, 19:47    1303
     184276902, // 2028 May 24, 08:17    1304
     184531128, // 2028 Jun 22, 18:28    1305
     184784772, // 2028 Jul 22, 03:02    1306
     185038104, // 2028 Aug 20, 10:44    1307
     185291424, // 2028 Sep 18, 18:24    1308
     185545062, // 2028 Oct 18, 02:57    1309
     185799348, // 2028 Nov 16, 13:18    1310
     186054516, // 2028 Dec 16, 02:06    1311
     186310590, // 2029 Jan 14, 17:25    1312
     186567312, // 2029 Feb 13, 10:32    1313
     186824280, // 2029 Mar 15, 04:20    1314
     187081080, // 2029 Apr 13, 21:40    1315
     187337412, // 2029 May 13, 13:42    1316
     187593066, // 2029 Jun 12, 03:51    1317
     187847946, // 2029 Jul 11, 15:51    1318
     188102136, // 2029 Aug 10, 01:56    1319
     188355870, // 2029 Sep  8, 10:45    1320
     188609490, // 2029 Oct  7, 19:15    1321
     188863344, // 2029 Nov  6, 04:24    1322
     189117672, // 2029 Dec  5, 14:52    1323
     189372534, // 2030 Jan  4, 02:49    1324
     189627888, // 2030 Feb  2, 16:08    1325
     189883650, // 2030 Mar  4, 06:35    1326
     190139778, // 2030 Apr  2, 22:03    1327
     190396152, // 2030 May  2, 14:12    1328
     190652526, // 2030 Jun  1, 06:21    1329
     190908570, // 2030 Jun 30, 21:35    1330
     191164026, // 2030 Jul 30, 11:11    1331
     191418882, // 2030 Aug 28, 23:07    1332
     191673330, // 2030 Sep 27, 09:55    1333
     191927622, // 2030 Oct 26, 20:17    1334
     192181962, // 2030 Nov 25, 06:47    1335
     192436392, // 2030 Dec 24, 17:32    1336
     192690906, // 2031 Jan 23, 04:31    1337
     192945534, // 2031 Feb 21, 15:49    1338
     193200414, // 2031 Mar 23, 03:49    1339
     193455702, // 2031 Apr 21, 16:57    1340
     193711422, // 2031 May 21, 07:17    1341
     193967430, // 2031 Jun 19, 22:25    1342
     194223480, // 2031 Jul 19, 13:40    1343
     194479392, // 2031 Aug 18, 04:32    1344
     194735082, // 2031 Sep 16, 18:47    1345
     194990526, // 2031 Oct 16, 08:21    1346
     195245700, // 2031 Nov 14, 21:10    1347
     195500556, // 2031 Dec 14, 09:06    1348
     195755082, // 2032 Jan 12, 20:07    1349
     196009344, // 2032 Feb 11, 06:24    1350
     196263504, // 2032 Mar 11, 16:24    1351
     196517760, // 2032 Apr 10, 02:40    1352
     196772256, // 2032 May  9, 13:36    1353
     197027112, // 2032 Jun  8, 01:32    1354
     197282412, // 2032 Jul  7, 14:42    1355
     197538192, // 2032 Aug  6, 05:12    1356
     197794422, // 2032 Sep  4, 20:57    1357
     198050922, // 2032 Oct  4, 13:27    1358
     198307350, // 2032 Nov  3, 05:45    1359
     198563358, // 2032 Dec  2, 20:53    1360
     198818742, // 2033 Jan  1, 10:17    1361
     199073520, // 2033 Jan 30, 22:00    1362
     199327818, // 2033 Mar  1, 08:23    1363
     199581786, // 2033 Mar 30, 17:51    1364
     199835556, // 2033 Apr 29, 02:46    1365
     200089302, // 2033 May 28, 11:37    1366
     200343282, // 2033 Jun 26, 21:07    1367
     200597838, // 2033 Jul 26, 08:13    1368
     200853240, // 2033 Aug 24, 21:40    1369
     201109560, // 2033 Sep 23, 13:40    1370
     201366534, // 2033 Oct 23, 07:29    1371
     201623640, // 2033 Nov 22, 01:40    1372
     201880362, // 2033 Dec 21, 18:47    1373
     202136412, // 2034 Jan 20, 10:02    1374
     202391700, // 2034 Feb 18, 23:10    1375
     202646250, // 2034 Mar 20, 10:15    1376
     202900116, // 2034 Apr 18, 19:26    1377
     203153472, // 2034 May 18, 03:12    1378
     203406636, // 2034 Jun 16, 10:26    1379
     203660010, // 2034 Jul 15, 18:15    1380
     203914038, // 2034 Aug 14, 03:53    1381
     204169044, // 2034 Sep 12, 16:14    1382
     204425118, // 2034 Oct 12, 07:33    1383
     204682056, // 2034 Nov 11, 01:16    1384
     204939444, // 2034 Dec 10, 20:14    1385
     205196778, // 2035 Jan  9, 15:03    1386
     205453572, // 2035 Feb  8, 08:22    1387
     205709460, // 2035 Mar  9, 23:10    1388
     205964268, // 2035 Apr  8, 10:58    1389
     206218104, // 2035 May  7, 20:04    1390
     206471280, // 2035 Jun  6, 03:20    1391
     206724234, // 2035 Jul  5, 09:59    1392
     206977386, // 2035 Aug  3, 17:11    1393
     207231114, // 2035 Sep  2, 01:59    1394
     207485682, // 2035 Oct  1, 13:07    1395
     207741234, // 2035 Oct 31, 02:59    1396
     207997788, // 2035 Nov 29, 19:38    1397
     208255146, // 2035 Dec 29, 14:31    1398
     208512822, // 2036 Jan 28, 10:17    1399
     208770120, // 2036 Feb 27, 05:00    1400
     209026422, // 2036 Mar 27, 20:57    1401
     209281518, // 2036 Apr 26, 09:33    1402
     209535582, // 2036 May 25, 19:17    1403
     209788980, // 2036 Jun 24, 03:10    1404
     210042102, // 2036 Jul 23, 10:17    1405
     210295290, // 2036 Aug 21, 17:35    1406
     210548826, // 2036 Sep 20, 01:51    1407
     210802980, // 2036 Oct 19, 11:50    1408
     211058010, // 2036 Nov 18, 00:15    1409
     211314090, // 2036 Dec 17, 15:35    1410
     211571124, // 2037 Jan 16, 09:34    1411
     211828644, // 2037 Feb 15, 04:54    1412
     212085942, // 2037 Mar 16, 23:37    1413
     212342448, // 2037 Apr 15, 16:08    1414
     212597970, // 2037 May 15, 05:55    1415
     212852586, // 2037 Jun 13, 17:11    1416
     213106512, // 2037 Jul 13, 02:32    1417
     213360012, // 2037 Aug 11, 10:42    1418
     213613350, // 2037 Sep  9, 18:25    1419
     213866850, // 2037 Oct  9, 02:35    1420
     214120818, // 2037 Nov  7, 12:03    1421
     214375554, // 2037 Dec  6, 23:39    1422
     214631172, // 2038 Jan  5, 13:42    1423
     214887552, // 2038 Feb  4, 05:52    1424
     215144370, // 2038 Mar  5, 23:15    1425
     215401218, // 2038 Apr  4, 16:43    1426
     215657760, // 2038 May  4, 09:20    1427
     215913744, // 2038 Jun  3, 00:24    1428
     216169032, // 2038 Jul  2, 13:32    1429
     216423600, // 2038 Aug  1, 00:40    1430
     216677592, // 2038 Aug 30, 10:12    1431
     216931302, // 2038 Sep 28, 18:57    1432
     217185078, // 2038 Oct 28, 03:53    1433
     217439202, // 2038 Nov 26, 13:47    1434
     217693812, // 2038 Dec 26, 01:02    1435
     217948896, // 2039 Jan 24, 13:36    1436
     218204388, // 2039 Feb 23, 03:18    1437
     218460240, // 2039 Mar 24, 18:00    1438
     218716410, // 2039 Apr 23, 09:35    1439
     218972748, // 2039 May 23, 01:38    1440
     219228966, // 2039 Jun 21, 17:21    1441
     219484764, // 2039 Jul 21, 07:54    1442
     219739980, // 2039 Aug 19, 20:50    1443
     219994698, // 2039 Sep 18, 08:23    1444
     220249134, // 2039 Oct 17, 19:09    1445
     220503516, // 2039 Nov 16, 05:46    1446
     220757952, // 2039 Dec 15, 16:32    1447
     221012430, // 2040 Jan 14, 03:25    1448
     221266950, // 2040 Feb 12, 14:25    1449
     221521602, // 2040 Mar 13, 01:47    1450
     221776560, // 2040 Apr 11, 14:00    1451
     222031968, // 2040 May 11, 03:28    1452
     222287778, // 2040 Jun  9, 18:03    1453
     222543810, // 2040 Jul  9, 09:15    1454
     222799836, // 2040 Aug  8, 00:26    1455
     223055724, // 2040 Sep  6, 15:14    1456
     223311396, // 2040 Oct  6, 05:26    1457
     223566816, // 2040 Nov  4, 18:56    1458
     223821918, // 2040 Dec  4, 07:33    1459
     224076648, // 2041 Jan  2, 19:08    1460
     224331018, // 2041 Feb  1, 05:43    1461
     224585154, // 2041 Mar  2, 15:39    1462
     224839260, // 2041 Apr  1, 01:30    1463
     225093522, // 2041 Apr 30, 11:47    1464
     225348096, // 2041 May 29, 22:56    1465
     225603102, // 2041 Jun 28, 11:17    1466
     225858612, // 2041 Jul 28, 01:02    1467
     226114656, // 2041 Aug 26, 16:16    1468
     226371126, // 2041 Sep 25, 08:41    1469
     226627740, // 2041 Oct 25, 01:30    1470
     226884096, // 2041 Nov 23, 17:36    1471
     227139876, // 2041 Dec 23, 08:06    1472
     227394972, // 2042 Jan 21, 20:42    1473
     227649474, // 2042 Feb 20, 07:39    1474
     227903538, // 2042 Mar 21, 17:23    1475
     228157314, // 2042 Apr 20, 02:19    1476
     228410970, // 2042 May 19, 10:55    1477
     228664728, // 2042 Jun 17, 19:48    1478
     228918912, // 2042 Jul 17, 05:52    1479
     229173846, // 2042 Aug 15, 18:01    1480
     229429740, // 2042 Sep 14, 08:50    1481
     229686498, // 2042 Oct 14, 02:03    1482
     229943688, // 2042 Nov 12, 20:28    1483
     230200734, // 2042 Dec 12, 14:29    1484
     230457198, // 2043 Jan 11, 06:53    1485
     230712882, // 2043 Feb  9, 21:07    1486
     230967774, // 2043 Mar 11, 09:09    1487
     231221922, // 2043 Apr  9, 19:07    1488
     231475446, // 2043 May  9, 03:21    1489
     231728610, // 2043 Jun  7, 10:35    1490
     231981786, // 2043 Jul  6, 17:51    1491
     232235418, // 2043 Aug  5, 02:23    1492
     232489902, // 2043 Sep  3, 13:17    1493
     232745472, // 2043 Oct  3, 03:12    1494
     233002068, // 2043 Nov  1, 19:58    1495
     233259342, // 2043 Dec  1, 14:37    1496
     233516808, // 2043 Dec 31, 09:48    1497
     233773944, // 2044 Jan 30, 04:04    1498
     234030312, // 2044 Feb 28, 20:12    1499
     234285636, // 2044 Mar 29, 09:26    1500
     234539892, // 2044 Apr 27, 19:42    1501
     234793320, // 2044 May 27, 03:40    1502
     235046304, // 2044 Jun 25, 10:24    1503
     235299300, // 2044 Jul 24, 17:10    1504
     235552716, // 2044 Aug 23, 01:06    1505
     235806858, // 2044 Sep 21, 11:03    1506
     236061936, // 2044 Oct 20, 23:36    1507
     236318028, // 2044 Nov 19, 14:58    1508
     236575038, // 2044 Dec 19, 08:53    1509
     236832630, // 2045 Jan 18, 04:25    1510
     237090186, // 2045 Feb 16, 23:51    1511
     237347010, // 2045 Mar 18, 17:15    1512
     237602682, // 2045 Apr 17, 07:27    1513
     237857202, // 2045 May 16, 18:27    1514
     238110870, // 2045 Jun 15, 03:05    1515
     238364094, // 2045 Jul 14, 10:29    1516
     238617234, // 2045 Aug 12, 17:39    1517
     238870608, // 2045 Sep 11, 01:28    1518
     239124462, // 2045 Oct 10, 10:37    1519
     239379054, // 2045 Nov  8, 21:49    1520
     239634606, // 2045 Dec  8, 11:41    1521
     239891184, // 2046 Jan  7, 04:24    1522
     240148500, // 2046 Feb  5, 23:10    1523
     240405936, // 2046 Mar  7, 18:16    1524
     240662832, // 2046 Apr  6, 11:52    1525
     240918816, // 2046 May  6, 02:56    1526
     241173852, // 2046 Jun  4, 15:22    1527
     241428114, // 2046 Jul  4, 01:39    1528
     241681830, // 2046 Aug  2, 10:25    1529
     241935270, // 2046 Aug 31, 18:25    1530
     242188710, // 2046 Sep 30, 02:25    1531
     242442462, // 2046 Oct 29, 11:17    1532
     242696820, // 2046 Nov 27, 21:50    1533
     242951994, // 2046 Dec 27, 10:39    1534
     243207984, // 2047 Jan 26, 01:44    1535
     243464556, // 2047 Feb 24, 18:26    1536
     243721344, // 2047 Mar 26, 11:44    1537
     243978000, // 2047 Apr 25, 04:40    1538
     244234242, // 2047 May 24, 20:27    1539
     244489896, // 2047 Jun 23, 10:36    1540
     244744854, // 2047 Jul 22, 22:49    1541
     244999176, // 2047 Aug 21, 09:16    1542
     245253066, // 2047 Sep 19, 18:31    1543
     245506848, // 2047 Oct 19, 03:28    1544
     245760834, // 2047 Nov 17, 12:59    1545
     246015228, // 2047 Dec 16, 23:38    1546
     246270072, // 2048 Jan 15, 11:32    1547
     246525312, // 2048 Feb 14, 00:32    1548
     246780888, // 2048 Mar 14, 14:28    1549
     247036800, // 2048 Apr 13, 05:20    1550
     247292988, // 2048 May 12, 20:58    1551
     247549260, // 2048 Jun 11, 12:50    1552
     247805304, // 2048 Jul 11, 04:04    1553
     248060874, // 2048 Aug  9, 17:59    1554
     248315910, // 2048 Sep  8, 06:25    1555
     248570550, // 2048 Oct  7, 17:45    1556
     248825034, // 2048 Nov  6, 04:39    1557
     249079500, // 2048 Dec  5, 15:30    1558
     249333990, // 2049 Jan  4, 02:25    1559
     249588456, // 2049 Feb  2, 13:16    1560
     249842952, // 2049 Mar  4, 00:12    1561
     250097634, // 2049 Apr  2, 11:39    1562
     250352706, // 2049 May  2, 00:11    1563
     250608240, // 2049 May 31, 14:00    1564
     250864146, // 2049 Jun 30, 04:51    1565
     251120202, // 2049 Jul 29, 20:07    1566
     251376234, // 2049 Aug 28, 11:19    1567
     251632110, // 2049 Sep 27, 02:05    1568
     251887770, // 2049 Oct 26, 16:15    1569
     252143136, // 2049 Nov 25, 05:36    1570
     252398112, // 2049 Dec 24, 17:52    1571
     252652662, // 2050 Jan 23, 04:57    1572
     252906858, // 2050 Feb 21, 15:03    1573
     253160886, // 2050 Mar 23, 00:41    1574
     253414956, // 2050 Apr 21, 10:26    1575
     253669266, // 2050 May 20, 20:51    1576
     253923972, // 2050 Jun 19, 08:22    1577
     254179182, // 2050 Jul 18, 21:17    1578
     254434962, // 2050 Aug 17, 11:47    1579
     254691294, // 2050 Sep 16, 03:49    1580
     254947974, // 2050 Oct 15, 20:49    1581
     255204612, // 2050 Nov 14, 13:42    1582
     255460788, // 2050 Dec 14, 05:18    1583
     255716268, // 2051 Jan 12, 18:58    1584
     255971046, // 2051 Feb 11, 06:41    1585
     256225272, // 2051 Mar 12, 16:52    1586
     256479114, // 2051 Apr 11, 01:59    1587
     256732734, // 2051 May 10, 10:29    1588
     256986336, // 2051 Jun  8, 18:56    1589
     257240214, // 2051 Jul  8, 04:09    1590
     257494710, // 2051 Aug  6, 15:05    1591
     257750118, // 2051 Sep  5, 04:33    1592
     258006522, // 2051 Oct  4, 20:47    1593
     258263634, // 2051 Nov  3, 14:59    1594
     258520902, // 2051 Dec  3, 09:37    1595
     258777756, // 2052 Jan  2, 03:06    1596
     259033860, // 2052 Jan 31, 18:30    1597
     259289136, // 2052 Mar  1, 07:36    1598
     259543602, // 2052 Mar 30, 18:27    1599
     259797366, // 2052 Apr 29, 03:21    1600
     260050620, // 2052 May 28, 10:50    1601
     260303700, // 2052 Jun 26, 17:50    1602
     260557026, // 2052 Jul 26, 01:31    1603
     260811042, // 2052 Aug 24, 11:07    1604
     261066078, // 2052 Sep 22, 23:33    1605
     261322218, // 2052 Oct 22, 15:03    1606
     261579252, // 2052 Nov 21, 09:02    1607
     261836730, // 2052 Dec 21, 04:15    1608
     262094112, // 2053 Jan 19, 23:12    1609
     262350912, // 2053 Feb 18, 16:32    1610
     262606752, // 2053 Mar 20, 07:12    1611
     262861488, // 2053 Apr 18, 18:48    1612
     263115258, // 2053 May 18, 03:43    1613
     263368386, // 2053 Jun 16, 10:51    1614
     263621316, // 2053 Jul 15, 17:26    1615
     263874486, // 2053 Aug 14, 00:41    1616
     264128256, // 2053 Sep 12, 09:36    1617
     264382878, // 2053 Oct 11, 20:53    1618
     264638490, // 2053 Nov 10, 10:55    1619
     264895080, // 2053 Dec 10, 03:40    1620
     265152444, // 2054 Jan  8, 22:34    1621
     265410084, // 2054 Feb  7, 18:14    1622
     265667316, // 2054 Mar  9, 12:46    1623
     265923552, // 2054 Apr  8, 04:32    1624
     266178600, // 2054 May  7, 17:00    1625
     266432640, // 2054 Jun  6, 02:40    1626
     266686044, // 2054 Jul  5, 10:34    1627
     266939208, // 2054 Aug  3, 17:48    1628
     267192468, // 2054 Sep  2, 01:18    1629
     267446094, // 2054 Oct  1, 09:49    1630
     267700326, // 2054 Oct 30, 20:01    1631
     267955404, // 2054 Nov 29, 08:34    1632
     268211466, // 2054 Dec 28, 23:51    1633
     268468434, // 2055 Jan 27, 17:39    1634
     268725834, // 2055 Feb 26, 12:39    1635
     268983006, // 2055 Mar 28, 07:01    1636
     269239422, // 2055 Apr 26, 23:17    1637
     269494902, // 2055 May 26, 12:57    1638
     269749530, // 2055 Jun 25, 00:15    1639
     270003528, // 2055 Jul 24, 09:48    1640
     270257124, // 2055 Aug 22, 18:14    1641
     270510594, // 2055 Sep 21, 02:19    1642
     270764214, // 2055 Oct 20, 10:49    1643
     271018284, // 2055 Nov 18, 20:34    1644
     271273050, // 2055 Dec 18, 08:15    1645
     271528620, // 2056 Jan 16, 22:10    1646
     271784880, // 2056 Feb 15, 14:00    1647
     272041512, // 2056 Mar 16, 06:52    1648
     272298186, // 2056 Apr 14, 23:51    1649
     272554596, // 2056 May 14, 16:06    1650
     272810544, // 2056 Jun 13, 07:04    1651
     273065880, // 2056 Jul 12, 20:20    1652
     273320568, // 2056 Aug 11, 07:48    1653
     273574722, // 2056 Sep  9, 17:47    1654
     273828600, // 2056 Oct  9, 03:00    1655
     274082526, // 2056 Nov  7, 12:21    1656
     274336746, // 2056 Dec  6, 22:31    1657
     274591374, // 2057 Jan  5, 09:49    1658
     274846386, // 2057 Feb  3, 22:11    1659
     275101710, // 2057 Mar  5, 11:25    1660
     275357346, // 2057 Apr  4, 01:31    1661
     275613312, // 2057 May  3, 16:32    1662
     275869506, // 2057 Jun  2, 08:11    1663
     276125682, // 2057 Jul  1, 23:47    1664
     276381552, // 2057 Jul 31, 14:32    1665
     276636924, // 2057 Aug 30, 03:54    1666
     276891840, // 2057 Sep 28, 16:00    1667
     277146474, // 2057 Oct 28, 03:19    1668
     277401012, // 2057 Nov 26, 14:22    1669
     277655532, // 2057 Dec 26, 01:22    1670
     277910004, // 2058 Jan 24, 12:14    1671
     278164422, // 2058 Feb 22, 22:57    1672
     278418900, // 2058 Mar 24, 09:50    1673
     278673654, // 2058 Apr 22, 21:29    1674
     278928858, // 2058 May 22, 10:23    1675
     279184530, // 2058 Jun 21, 00:35    1676
     279440520, // 2058 Jul 20, 15:40    1677
     279696618, // 2058 Aug 19, 07:03    1678
     279952662, // 2058 Sep 17, 22:17    1679
     280208550, // 2058 Oct 17, 13:05    1680
     280464174, // 2058 Nov 16, 03:09    1681
     280719426, // 2058 Dec 15, 16:11    1682
     280974222, // 2059 Jan 14, 03:57    1683
     281228562, // 2059 Feb 12, 14:27    1684
     281482590, // 2059 Mar 14, 00:05    1685
     281736534, // 2059 Apr 12, 09:29    1686
     281990610, // 2059 May 11, 19:15    1687
     282245022, // 2059 Jun 10, 05:57    1688
     282499908, // 2059 Jul  9, 17:58    1689
     282755388, // 2059 Aug  8, 07:38    1690
     283011486, // 2059 Sep  6, 23:01    1691
     283268100, // 2059 Oct  6, 15:50    1692
     283524906, // 2059 Nov  5, 09:11    1693
     283781454, // 2059 Dec  5, 01:49    1694
     284037360, // 2060 Jan  3, 16:40    1695
     284292492, // 2060 Feb  2, 05:22    1696
     284546952, // 2060 Mar  2, 16:12    1697
     284800908, // 2060 Apr  1, 01:38    1698
     285054546, // 2060 Apr 30, 10:11    1699
     285308064, // 2060 May 29, 18:24    1700
     285561708, // 2060 Jun 28, 02:58    1701
     285815820, // 2060 Jul 27, 12:50    1702
     286070736, // 2060 Aug 26, 00:56    1703
     286326684, // 2060 Sep 24, 15:54    1704
     286583556, // 2060 Oct 24, 09:26    1705
     286840896, // 2060 Nov 23, 04:16    1706
     287098080, // 2060 Dec 22, 22:40    1707
     287354616, // 2061 Jan 21, 15:16    1708
     287610306, // 2061 Feb 20, 05:31    1709
     287865138, // 2061 Mar 21, 17:23    1710
     288119190, // 2061 Apr 20, 03:05    1711
     288372618, // 2061 May 19, 11:03    1712
     288625698, // 2061 Jun 17, 18:03    1713
     288878826, // 2061 Jul 17, 01:11    1714
     289132440, // 2061 Aug 15, 09:40    1715
     289386942, // 2061 Sep 13, 20:37    1716
     289642572, // 2061 Oct 13, 10:42    1717
     289899240, // 2061 Nov 12, 03:40    1718
     290156598, // 2061 Dec 11, 22:33    1719
     290414118, // 2062 Jan 10, 17:53    1720
     290671266, // 2062 Feb  9, 12:11    1721
     290927598, // 2062 Mar 11, 04:13    1722
     291182862, // 2062 Apr  9, 17:17    1723
     291437058, // 2062 May  9, 03:23    1724
     291690432, // 2062 Jun  7, 11:12    1725
     291943398, // 2062 Jul  6, 17:53    1726
     292196400, // 2062 Aug  5, 00:40    1727
     292449852, // 2062 Sep  3, 08:42    1728
     292704054, // 2062 Oct  2, 18:49    1729
     292959198, // 2062 Nov  1, 07:33    1730
     293215326, // 2062 Nov 30, 23:01    1731
     293472342, // 2062 Dec 30, 16:57    1732
     293729898, // 2063 Jan 29, 12:23    1733
     293987388, // 2063 Feb 28, 07:38    1734
     294244140, // 2063 Mar 30, 00:50    1735
     294499752, // 2063 Apr 28, 14:52    1736
     294754242, // 2063 May 28, 01:47    1737
     295007916, // 2063 Jun 26, 10:26    1738
     295261170, // 2063 Jul 25, 17:55    1739
     295514382, // 2063 Aug 24, 01:17    1740
     295767846, // 2063 Sep 22, 09:21    1741
     296021796, // 2063 Oct 21, 18:46    1742
     296276454, // 2063 Nov 20, 06:09    1743
     296532024, // 2063 Dec 19, 20:04    1744
     296788542, // 2064 Jan 18, 12:37    1745
     297045738, // 2064 Feb 17, 07:03    1746
     297303030, // 2064 Mar 18, 01:45    1747
     297559812, // 2064 Apr 16, 19:02    1748
     297815730, // 2064 May 16, 09:55    1749
     298070766, // 2064 Jun 14, 22:21    1750
     298325076, // 2064 Jul 14, 08:46    1751
     298578894, // 2064 Aug 12, 17:49    1752
     298832466, // 2064 Sep 11, 02:11    1753
     299086044, // 2064 Oct 10, 10:34    1754
     299339910, // 2064 Nov  8, 19:45    1755
     299594334, // 2064 Dec  8, 06:29    1756
     299849490, // 2065 Jan  6, 19:15    1757
     300105372, // 2065 Feb  5, 10:02    1758
     300361770, // 2065 Mar  7, 02:15    1759
     300618366, // 2065 Apr  5, 19:01    1760
     300874860, // 2065 May  5, 11:30    1761
     301131030, // 2065 Jun  4, 03:05    1762
     301386696, // 2065 Jul  3, 17:16    1763
     301641756, // 2065 Aug  2, 05:46    1764
     301896234, // 2065 Aug 31, 16:39    1765
     302150304, // 2065 Sep 30, 02:24    1766
     302404248, // 2065 Oct 29, 11:48    1767
     302658360, // 2065 Nov 27, 21:40    1768
     302912802, // 2065 Dec 27, 08:27    1769
     303167604, // 2066 Jan 25, 20:14    1770
     303422706, // 2066 Feb 24, 08:51    1771
     303678084, // 2066 Mar 25, 22:14    1772
     303933774, // 2066 Apr 24, 12:29    1773
     304189788, // 2066 May 24, 03:38    1774
     304445970, // 2066 Jun 22, 19:15    1775
     304702044, // 2066 Jul 22, 10:34    1776
     304957740, // 2066 Aug 21, 00:50    1777
     305212962, // 2066 Sep 19, 13:47    1778
     305467812, // 2066 Oct 19, 01:42    1779
     305722476, // 2066 Nov 17, 13:06    1780
     305977062, // 2066 Dec 17, 00:17    1781
     306231576, // 2067 Jan 15, 11:16    1782
     306485982, // 2067 Feb 13, 21:57    1783
     306740334, // 2067 Mar 15, 08:29    1784
     306994824, // 2067 Apr 13, 19:24    1785
     307249680, // 2067 May 13, 07:20    1786
     307505046, // 2067 Jun 11, 20:41    1787
     307760856, // 2067 Jul 11, 11:16    1788
     308016936, // 2067 Aug 10, 02:36    1789
     308273094, // 2067 Sep  8, 18:09    1790
     308529168, // 2067 Oct  8, 09:28    1791
     308785044, // 2067 Nov  7, 00:14    1792
     309040590, // 2067 Dec  6, 14:05    1793
     309295668, // 2068 Jan  5, 02:38    1794
     309550224, // 2068 Feb  3, 13:44    1795
     309804342, // 2068 Mar  3, 23:37    1796
     310058226, // 2068 Apr  2, 08:51    1797
     310312122, // 2068 May  1, 18:07    1798
     310566258, // 2068 May 31, 04:03    1799
     310820826, // 2068 Jun 29, 15:11    1800
     311075970, // 2068 Jul 29, 03:55    1801
     311331768, // 2068 Aug 27, 18:28    1802
     311588208, // 2068 Sep 26, 10:48    1803
     311845062, // 2068 Oct 26, 04:17    1804
     312101892, // 2068 Nov 24, 21:42    1805
     312358224, // 2068 Dec 24, 13:44    1806
     312613776, // 2069 Jan 23, 03:36    1807
     312868542, // 2069 Feb 21, 15:17    1808
     313122678, // 2069 Mar 23, 01:13    1809
     313376388, // 2069 Apr 21, 09:58    1810
     313629876, // 2069 May 20, 18:06    1811
     313883364, // 2069 Jun 19, 02:14    1812
     314137158, // 2069 Jul 18, 11:13    1813
     314391618, // 2069 Aug 16, 22:03    1814
     314647050, // 2069 Sep 15, 11:35    1815
     314903538, // 2069 Oct 15, 04:03    1816
     315160788, // 2069 Nov 13, 22:38    1817
     315418188, // 2069 Dec 13, 17:38    1818
     315675138, // 2070 Jan 12, 11:23    1819
     315931278, // 2070 Feb 11, 02:53    1820
     316186512, // 2070 Mar 12, 15:52    1821
     316440900, // 2070 Apr 11, 02:30    1822
     316694568, // 2070 May 10, 11:08    1823
     316947744, // 2070 Jun  8, 18:24    1824
     317200764, // 2070 Jul  8, 01:14    1825
     317454066, // 2070 Aug  6, 08:51    1826
     317708094, // 2070 Sep  4, 18:29    1827
     317963166, // 2070 Oct  4, 07:01    1828
     318219378, // 2070 Nov  2, 22:43    1829
     318476484, // 2070 Dec  2, 16:54    1830
     318734010, // 2071 Jan  1, 12:15    1831
     318991416, // 2071 Jan 31, 07:16    1832
     319248192, // 2071 Mar  2, 00:32    1833
     319503978, // 2071 Mar 31, 15:03    1834
     319758660, // 2071 Apr 30, 02:30    1835
     320012382, // 2071 May 29, 11:17    1836
     320265480, // 2071 Jun 27, 18:20    1837
     320518416, // 2071 Jul 27, 00:56    1838
     320771616, // 2071 Aug 25, 08:16    1839
     321025446, // 2071 Sep 23, 17:21    1840
     321280134, // 2071 Oct 23, 04:49    1841
     321535794, // 2071 Nov 21, 18:59    1842
     321792402, // 2071 Dec 21, 11:47    1843
     322049730, // 2072 Jan 20, 06:35    1844
     322307298, // 2072 Feb 19, 02:03    1845
     322564452, // 2072 Mar 19, 20:22    1846
     322820622, // 2072 Apr 18, 11:57    1847
     323075634, // 2072 May 18, 00:19    1848
     323329662, // 2072 Jun 16, 09:57    1849
     323583096, // 2072 Jul 15, 17:56    1850
     323836326, // 2072 Aug 14, 01:21    1851
     324089682, // 2072 Sep 12, 09:07    1852
     324343410, // 2072 Oct 11, 17:55    1853
     324597726, // 2072 Nov 10, 04:21    1854
     324852834, // 2072 Dec  9, 16:59    1855
     325108866, // 2073 Jan  8, 08:11    1856
     325365720, // 2073 Feb  7, 01:40    1857
     325622976, // 2073 Mar  8, 20:16    1858
     325880004, // 2073 Apr  7, 14:14    1859
     326136330, // 2073 May  7, 06:15    1860
     326391792, // 2073 Jun  5, 19:52    1861
     326646462, // 2073 Jul  5, 07:17    1862
     326900544, // 2073 Aug  3, 17:04    1863
     327154272, // 2073 Sep  2, 01:52    1864
     327407886, // 2073 Oct  1, 10:21    1865
     327661638, // 2073 Oct 30, 19:13    1866
     327915792, // 2073 Nov 29, 05:12    1867
     328170570, // 2073 Dec 28, 16:55    1868
     328426062, // 2074 Jan 27, 06:37    1869
     328682160, // 2074 Feb 25, 22:00    1870
     328938600, // 2074 Mar 27, 14:20    1871
     329195088, // 2074 Apr 26, 06:48    1872
     329451384, // 2074 May 25, 22:44    1873
     329707314, // 2074 Jun 24, 13:39    1874
     329962722, // 2074 Jul 24, 03:07    1875
     330217554, // 2074 Aug 22, 14:59    1876
     330471888, // 2074 Sep 21, 01:28    1877
     330725946, // 2074 Oct 20, 11:11    1878
     330980016, // 2074 Nov 18, 20:56    1879
     331234320, // 2074 Dec 18, 07:20    1880
     331488942, // 2075 Jan 16, 18:37    1881
     331743846, // 2075 Feb 15, 06:41    1882
     331998990, // 2075 Mar 16, 19:25    1883
     332254410, // 2075 Apr 15, 08:55    1884
     332510172, // 2075 May 14, 23:22    1885
     332766234, // 2075 Jun 13, 14:39    1886
     333022386, // 2075 Jul 13, 06:11    1887
     333278346, // 2075 Aug 11, 21:11    1888
     333533892, // 2075 Sep 10, 11:02    1889
     333789018, // 2075 Oct  9, 23:43    1890
     334043850, // 2075 Nov  8, 11:35    1891
     334298538, // 2075 Dec  7, 23:03    1892
     334553130, // 2076 Jan  6, 10:15    1893
     334807566, // 2076 Feb  4, 21:01    1894
     335061864, // 2076 Mar  5, 07:24    1895
     335316162, // 2076 Apr  3, 17:47    1896
     335570712, // 2076 May  3, 04:52    1897
     335825724, // 2076 Jun  1, 17:14    1898
     336081270, // 2076 Jul  1, 07:05    1899
     336337236, // 2076 Jul 30, 22:06    1900
     336593424, // 2076 Aug 29, 13:44    1901
     336849642, // 2076 Sep 28, 05:27    1902
     337105740, // 2076 Oct 27, 20:50    1903
     337361568, // 2076 Nov 26, 11:28    1904
     337616958, // 2076 Dec 26, 00:53    1905
     337871790, // 2077 Jan 24, 12:45    1906
     338126082, // 2077 Feb 22, 23:07    1907
     338379984, // 2077 Mar 24, 08:24    1908
     338633760, // 2077 Apr 22, 17:20    1909
     338887668, // 2077 May 22, 02:38    1910
     339141930, // 2077 Jun 20, 12:55    1911
     339396726, // 2077 Jul 20, 00:41    1912
     339652188, // 2077 Aug 18, 14:18    1913
     339908358, // 2077 Sep 17, 05:53    1914
     340165122, // 2077 Oct 16, 23:07    1915
     340422120, // 2077 Nov 15, 17:00    1916
     340678836, // 2077 Dec 15, 10:06    1917
     340934844, // 2078 Jan 14, 01:14    1918
     341189988, // 2078 Feb 12, 13:58    1919
     341444382, // 2078 Mar 14, 00:37    1920
     341698230, // 2078 Apr 12, 09:45    1921
     341951736, // 2078 May 11, 17:56    1922
     342205134, // 2078 Jun 10, 01:49    1923
     342458688, // 2078 Jul  9, 10:08    1924
     342712752, // 2078 Aug  7, 19:52    1925
     342967674, // 2078 Sep  6, 07:59    1926
     343223676, // 2078 Oct  5, 23:06    1927
     343480656, // 2078 Nov  4, 16:56    1928
     343738128, // 2078 Dec  4, 12:08    1929
     343995420, // 2079 Jan  3, 06:50    1930
     344252010, // 2079 Feb  1, 23:35    1931
     344507688, // 2079 Mar  3, 13:48    1932
     344762460, // 2079 Apr  2, 01:30    1933
     345016422, // 2079 May  1, 10:57    1934
     345269766, // 2079 May 30, 18:41    1935
     345522786, // 2079 Jun 29, 01:31    1936
     345775878, // 2079 Jul 28, 08:33    1937
     346029492, // 2079 Aug 26, 17:02    1938
     346284036, // 2079 Sep 25, 04:06    1939
     346539720, // 2079 Oct 24, 18:20    1940
     346796460, // 2079 Nov 23, 11:30    1941
     347053872, // 2079 Dec 23, 06:32    1942
     347311410, // 2080 Jan 22, 01:55    1943
     347568546, // 2080 Feb 20, 20:11    1944
     347824836, // 2080 Mar 21, 12:06    1945
     348080040, // 2080 Apr 20, 01:00    1946
     348334176, // 2080 May 19, 10:56    1947
     348587520, // 2080 Jun 17, 18:40    1948
     348840486, // 2080 Jul 17, 01:21    1949
     349093518, // 2080 Aug 15, 08:13    1950
     349347030, // 2080 Sep 13, 16:25    1951
     349601304, // 2080 Oct 13, 02:44    1952
     349856502, // 2080 Nov 11, 15:37    1953
     350112660, // 2080 Dec 11, 07:10    1954
     350369652, // 2081 Jan 10, 01:02    1955
     350627142, // 2081 Feb  8, 20:17    1956
     350884542, // 2081 Mar 10, 15:17    1957
     351141210, // 2081 Apr  9, 08:15    1958
     351396774, // 2081 May  8, 22:09    1959
     351651246, // 2081 Jun  7, 09:01    1960
     351904944, // 2081 Jul  6, 17:44    1961
     352158264, // 2081 Aug  5, 01:24    1962
     352411566, // 2081 Sep  3, 09:01    1963
     352665138, // 2081 Oct  2, 17:23    1964
     352919184, // 2081 Nov  1, 03:04    1965
     353173896, // 2081 Nov 30, 14:36    1966
     353429448, // 2081 Dec 30, 04:28    1967
     353685876, // 2082 Jan 28, 20:46    1968
     353942928, // 2082 Feb 27, 14:48    1969
     354200070, // 2082 Mar 29, 09:05    1970
     354456732, // 2082 Apr 28, 02:02    1971
     354712602, // 2082 May 27, 16:47    1972
     354967656, // 2082 Jun 26, 05:16    1973
     355222044, // 2082 Jul 25, 15:54    1974
     355475988, // 2082 Aug 24, 01:18    1975
     355729704, // 2082 Sep 22, 10:04    1976
     355983420, // 2082 Oct 21, 18:50    1977
     356237394, // 2082 Nov 20, 04:19    1978
     356491860, // 2082 Dec 19, 15:10    1979
     356746980, // 2083 Jan 18, 03:50    1980
     357002730, // 2083 Feb 16, 18:15    1981
     357258942, // 2083 Mar 18, 09:57    1982
     357515340, // 2083 Apr 17, 02:10    1983
     357771684, // 2083 May 16, 18:14    1984
     358027782, // 2083 Jun 15, 09:37    1985
     358283484, // 2083 Jul 14, 23:54    1986
     358538670, // 2083 Aug 13, 12:45    1987
     358793322, // 2083 Sep 12, 00:07    1988
     359047578, // 2083 Oct 11, 10:23    1989
     359301690, // 2083 Nov  9, 20:15    1990
     359555910, // 2083 Dec  9, 06:25    1991
     359810382, // 2084 Jan  7, 17:17    1992
     360065118, // 2084 Feb  6, 04:53    1993
     360320064, // 2084 Mar  6, 17:04    1994
     360575232, // 2084 Apr  5, 05:52    1995
     360830718, // 2084 May  4, 19:33    1996
     361086558, // 2084 Jun  3, 10:13    1997
     361342668, // 2084 Jul  3, 01:38    1998
     361598784, // 2084 Aug  1, 17:04    1999
     361854630, // 2084 Aug 31, 07:45    2000
     362110056, // 2084 Sep 29, 21:16    2001
     362365122, // 2084 Oct 29, 09:47    2002
     362619954, // 2084 Nov 27, 21:39    2003
     362874642, // 2084 Dec 27, 09:07    2004
     363129162, // 2085 Jan 25, 20:07    2005
     363383472, // 2085 Feb 24, 06:32    2006
     363637662, // 2085 Mar 25, 16:37    2007
     363891954, // 2085 Apr 24, 02:59    2008
     364146618, // 2085 May 23, 14:23    2009
     364401828, // 2085 Jun 22, 03:18    2010
     364657578, // 2085 Jul 21, 17:43    2011
     364913706, // 2085 Aug 20, 09:11    2012
     365170002, // 2085 Sep 19, 01:07    2013
     365426280, // 2085 Oct 18, 17:00    2014
     365682366, // 2085 Nov 17, 08:21    2015
     365938068, // 2085 Dec 16, 22:38    2016
     366193224, // 2086 Jan 15, 11:24    2017
     366447762, // 2086 Feb 13, 22:27    2018
     366701784, // 2086 Mar 15, 08:04    2019
     366955518, // 2086 Apr 13, 16:53    2020
     367209246, // 2086 May 13, 01:41    2021
     367463232, // 2086 Jun 11, 11:12    2022
     367717692, // 2086 Jul 10, 22:02    2023
     367972788, // 2086 Aug  9, 10:38    2024
     368228622, // 2086 Sep  8, 01:17    2025
     368485176, // 2086 Oct  7, 17:56    2026
     368742198, // 2086 Nov  6, 11:53    2027
     368999208, // 2086 Dec  6, 05:48    2028
     369255666, // 2087 Jan  4, 22:11    2029
     369511260, // 2087 Feb  3, 12:10    2030
     369765990, // 2087 Mar  4, 23:45    2031
     370020036, // 2087 Apr  3, 09:26    2032
     370273626, // 2087 May  2, 17:51    2033
     370526988, // 2087 Jun  1, 01:38    2034
     370780386, // 2087 Jun 30, 09:31    2035
     371034120, // 2087 Jul 29, 18:20    2036
     371288568, // 2087 Aug 28, 05:08    2037
     371544042, // 2087 Sep 26, 18:47    2038
     371800614, // 2087 Oct 26, 11:29    2039
     372057984, // 2087 Nov 25, 06:24    2040
     372315498, // 2087 Dec 25, 01:43    2041
     372572514, // 2088 Jan 23, 19:39    2042
     372828654, // 2088 Feb 22, 11:09    2043
     373083846, // 2088 Mar 23, 00:01    2044
     373338150, // 2088 Apr 21, 10:25    2045
     373591734, // 2088 May 20, 18:49    2046
     373844844, // 2088 Jun 19, 01:54    2047
     374097828, // 2088 Jul 18, 08:38    2048
     374351130, // 2088 Aug 16, 16:15    2049
     374605188, // 2088 Sep 15, 01:58    2050
     374860314, // 2088 Oct 14, 14:39    2051
     375116592, // 2088 Nov 13, 06:32    2052
     375373752, // 2088 Dec 13, 00:52    2053
     375631308, // 2089 Jan 11, 20:18    2054
     375888696, // 2089 Feb 10, 15:16    2055
     376145424, // 2089 Mar 12, 08:24    2056
     376401156, // 2089 Apr 10, 22:46    2057
     376655784, // 2089 May 10, 10:04    2058
     376909464, // 2089 Jun  8, 18:44    2059
     377162562, // 2089 Jul  8, 01:47    2060
     377415528, // 2089 Aug  6, 08:28    2061
     377668782, // 2089 Sep  4, 15:57    2062
     377922690, // 2089 Oct  4, 01:15    2063
     378177450, // 2089 Nov  2, 12:55    2064
     378433146, // 2089 Dec  2, 03:11    2065
     378689736, // 2089 Dec 31, 19:56    2066
     378947004, // 2090 Jan 30, 14:34    2067
     379204476, // 2090 Mar  1, 09:46    2068
     379461528, // 2090 Mar 31, 03:48    2069
     379717632, // 2090 Apr 29, 19:12    2070
     379972614, // 2090 May 29, 07:29    2071
     380226666, // 2090 Jun 27, 17:11    2072
     380480154, // 2090 Jul 27, 01:19    2073
     380733468, // 2090 Aug 25, 08:58    2074
     380986938, // 2090 Sep 23, 17:03    2075
     381240774, // 2090 Oct 23, 02:09    2076
     381495168, // 2090 Nov 21, 12:48    2077
     381750294, // 2090 Dec 21, 01:29    2078
     382006260, // 2091 Jan 19, 16:30    2079
     382262988, // 2091 Feb 18, 09:38    2080
     382520076, // 2091 Mar 20, 03:46    2081
     382776960, // 2091 Apr 18, 21:20    2082
     383033202, // 2091 May 18, 13:07    2083
     383288646, // 2091 Jun 17, 02:41    2084
     383543376, // 2091 Jul 16, 14:16    2085
     383797572, // 2091 Aug 15, 00:22    2086
     384051444, // 2091 Sep 13, 09:34    2087
     384305214, // 2091 Oct 12, 18:29    2088
     384559092, // 2091 Nov 11, 03:42    2089
     384813324, // 2091 Dec 10, 13:54    2090
     385068102, // 2092 Jan  9, 01:37    2091
     385323492, // 2092 Feb  7, 15:02    2092
     385579416, // 2092 Mar  8, 05:56    2093
     385835646, // 2092 Apr  6, 21:41    2094
     386091954, // 2092 May  6, 13:39    2095
     386348142, // 2092 Jun  5, 05:17    2096
     386604066, // 2092 Jul  4, 20:11    2097
     386859570, // 2092 Aug  3, 09:55    2098
     387114564, // 2092 Sep  1, 22:14    2099
     387369090, // 2092 Oct  1, 09:15    2100
     387623328, // 2092 Oct 30, 19:28    2101
     387877536, // 2092 Nov 29, 05:36    2102
     388131900, // 2092 Dec 28, 16:10    2103
     388386492, // 2093 Jan 27, 03:22    2104
     388641276, // 2093 Feb 25, 15:06    2105
     388896228, // 2093 Mar 27, 03:18    2106
     389151432, // 2093 Apr 25, 16:12    2107
     389407002, // 2093 May 25, 06:07    2108
     389662950, // 2093 Jun 23, 21:05    2109
     389919096, // 2093 Jul 23, 12:36    2110
     390175164, // 2093 Aug 22, 03:54    2111
     390430902, // 2093 Sep 20, 18:17    2112
     390686238, // 2093 Oct 20, 07:33    2113
     390941262, // 2093 Nov 18, 19:57    2114
     391196082, // 2093 Dec 18, 07:47    2115
     391450710, // 2094 Jan 16, 19:05    2116
     391705098, // 2094 Feb 15, 05:43    2117
     391959264, // 2094 Mar 16, 15:44    2118
     392213388, // 2094 Apr 15, 01:38    2119
     392467734, // 2094 May 14, 12:09    2120
     392722578, // 2094 Jun 13, 00:03    2121
     392978022, // 2094 Jul 12, 13:37    2122
     393233982, // 2094 Aug 11, 04:37    2123
     393490266, // 2094 Sep  9, 20:31    2124
     393746664, // 2094 Oct  9, 12:44    2125
     394002972, // 2094 Nov  8, 04:42    2126
     394258980, // 2094 Dec  7, 19:50    2127
     394514478, // 2095 Jan  6, 09:33    2128
     394769334, // 2095 Feb  4, 21:29    2129
     395023554, // 2095 Mar  6, 07:39    2130
     395277336, // 2095 Apr  4, 16:36    2131
     395530956, // 2095 May  4, 01:06    2132
     395784708, // 2095 Jun  2, 09:58    2133
     396038844, // 2095 Jul  1, 19:54    2134
     396293574, // 2095 Jul 31, 07:29    2135
     396549036, // 2095 Aug 29, 21:06    2136
     396805284, // 2095 Sep 28, 12:54    2137
     397062198, // 2095 Oct 28, 06:33    2138
     397319364, // 2095 Nov 27, 00:54    2139
     397576230, // 2095 Dec 26, 18:25    2140
     397832310, // 2096 Jan 25, 09:45    2141
     398087448, // 2096 Feb 23, 22:28    2142
     398341770, // 2096 Mar 24, 08:55    2143
     398595504, // 2096 Apr 22, 17:44    2144
     398848896, // 2096 May 22, 01:36    2145
     399102192, // 2096 Jun 20, 09:12    2146
     399355686, // 2096 Jul 19, 17:21    2147
     399609720, // 2096 Aug 18, 03:00    2148
     399864660, // 2096 Sep 16, 15:10    2149
     400120734, // 2096 Oct 16, 06:29    2150
     400377816, // 2096 Nov 15, 00:36    2151
     400635396, // 2096 Dec 14, 20:06    2152
     400892760, // 2097 Jan 13, 15:00    2153
     401149374, // 2097 Feb 12, 07:49    2154
     401405022, // 2097 Mar 13, 21:57    2155
     401659716, // 2097 Apr 12, 09:26    2156
     401913600, // 2097 May 11, 18:40    2157
     402166884, // 2097 Jun 10, 02:14    2158
     402419868, // 2097 Jul  9, 08:58    2159
     402672960, // 2097 Aug  7, 16:00    2160
     402926598, // 2097 Sep  6, 00:33    2161
     403181190, // 2097 Oct  5, 11:45    2162
     403436934, // 2097 Nov  4, 02:09    2163
     403693722, // 2097 Dec  3, 19:27    2164
     403951158, // 2098 Jan  2, 14:33    2165
     404208684, // 2098 Feb  1, 09:54    2166
     404465772, // 2098 Mar  3, 04:02    2167
     404722002, // 2098 Apr  1, 19:47    2168
     404977152, // 2098 May  1, 08:32    2169
     405231258, // 2098 May 30, 18:23    2170
     405484596, // 2098 Jun 29, 02:06    2171
     405737586, // 2098 Jul 28, 08:51    2172
     405990672, // 2098 Aug 26, 15:52    2173
     406244256, // 2098 Sep 25, 00:16    2174
     406498614, // 2098 Oct 24, 10:49    2175
     406753866, // 2098 Nov 22, 23:51    2176
     407010024, // 2098 Dec 22, 15:24    2177
     407266962, // 2099 Jan 21, 09:07    2178
     407524350, // 2099 Feb 20, 04:05    2179
     407781636, // 2099 Mar 21, 22:46    2180
     408038220, // 2099 Apr 20, 15:30    2181
     408293736, // 2099 May 20, 05:16    2182
     408548220, // 2099 Jun 18, 16:10    2183
     408801966, // 2099 Jul 18, 01:01    2184
     409055364, // 2099 Aug 16, 08:54    2185
     409308780, // 2099 Sep 14, 16:50    2186
     409562472, // 2099 Oct 14, 01:32    2187
     409816614, // 2099 Nov 12, 11:29    2188
     410071374, // 2099 Dec 11, 23:09    2189
     410326896, // 2100 Jan 10, 12:56    2190
     410583210, // 2100 Feb  9, 04:55    2191
     410840094, // 2100 Mar 10, 22:29    2192
     411097062, // 2100 Apr  9, 16:17    2193
     411353604, // 2100 May  9, 08:54    2194
     411609426, // 2100 Jun  7, 23:31    2195
     411864516, // 2100 Jul  7, 12:06    2196
     412119012, // 2100 Aug  5, 23:02    2197
     412373094, // 2100 Sep  4, 08:49    2198
     412626972, // 2100 Oct  3, 18:02    2199
     412880844, // 2100 Nov  2, 03:14    2200
     413134920, // 2100 Dec  1, 13:00    2201
     413389416, // 2100 Dec 30, 23:56    2202
     413644464, // 2101 Jan 29, 12:24    2203
};
enum { kNewMoonDatesCount = sizeof(newMoonDates)/sizeof(newMoonDates[0]) };

static const UDate newMoonDatesFirst = 10000.0 * -221149158; // newMoonDates[0];
static const UDate newMoonDatesLast  = 10000.0 *  413644464; // newMoonDates[kNewMoonDatesCount-1];
static const UDate newMoonDatesRange = 10000.0 * (413644464 + 221149158); // newMoonDatesLast - newMoonDatesFirst;

// To get the full moon date/time in millis,
// first we use the newMoonDates data to estimate the full moon time for a given lunation
// as halfway between the new moon for the current lunation and the new moon for the next,
// then we add the correction from the table below.
// These adjustment values are in millis/10000.0 (i.e. in units of 10 seconds) to fit
// into 16 bits.
// This fullMoonAdjustmts array has one fewer entry than the newMoonDates array.
static const int16_t fullMoonAdjustmts[] = {
//  adj/10K   lunation number
    -6411, // -285
    -4500, // -284
    -1641, // -283
     1527, // -282
     4353, // -281
     6303, // -280
     7017, // -279
     6402, // -278
     4572, // -277
     1866, // -276
    -1224, // -275
    -4092, // -274
    -6126, // -273
    -6873, // -272
    -6183, // -271
    -4230, // -270
    -1410, // -269
     1689, // -268
     4425, // -267
     6279, // -266
     6933, // -265
     6288, // -264
     4443, // -263
     1701, // -262
    -1419, // -261
    -4251, // -260
    -6192, // -259
    -6849, // -258
    -6114, // -257
    -4125, // -256
    -1332, // -255
     1692, // -254
     4362, // -253
     6210, // -252
     6894, // -251
     6237, // -250
     4305, // -249
     1458, // -248
    -1710, // -247
    -4509, // -246
    -6360, // -245
    -6903, // -244
    -6075, // -243
    -4095, // -242
    -1353, // -241
     1653, // -240
     4377, // -239
     6258, // -238
     6882, // -237
     6084, // -236
     4023, // -235
     1122, // -234
    -2004, // -233
    -4692, // -232
    -6417, // -231
    -6894, // -230
    -6093, // -229
    -4143, // -228
    -1371, // -227
     1716, // -226
     4470, // -225
     6303, // -224
     6837, // -223
     5955, // -222
     3837, // -221
      921, // -220
    -2154, // -219
    -4785, // -218
    -6489, // -217
    -6987, // -216
    -6156, // -215
    -4119, // -214
    -1236, // -213
     1911, // -212
     4677, // -211
     6477, // -210
     6918, // -209
     5895, // -208
     3675, // -207
      756, // -206
    -2289, // -205
    -4893, // -204
    -6561, // -203
    -6987, // -202
    -6054, // -201
    -3924, // -200
     -951, // -199
     2271, // -198
     5010, // -197
     6654, // -196
     6891, // -195
     5769, // -194
     3561, // -193
      681, // -192
    -2328, // -191
    -4893, // -190
    -6531, // -189
    -6918, // -188
    -5916, // -187
    -3651, // -186
     -558, // -185
     2637, // -184
     5211, // -183
     6705, // -182
     6891, // -181
     5775, // -180
     3564, // -179
      666, // -178
    -2376, // -177
    -4980, // -176
    -6612, // -175
    -6885, // -174
    -5688, // -173
    -3282, // -172
     -213, // -171
     2862, // -170
     5343, // -169
     6774, // -168
     6906, // -167
     5727, // -166
     3456, // -165
      510, // -164
    -2571, // -163
    -5139, // -162
    -6630, // -161
    -6726, // -160
    -5439, // -159
    -3045, // -158
      -39, // -157
     2970, // -156
     5358, // -155
     6708, // -154
     6801, // -153
     5601, // -152
     3300, // -151
      309, // -150
    -2766, // -149
    -5262, // -148
    -6666, // -147
    -6699, // -146
    -5373, // -145
    -2973, // -144
       -9, // -143
     2928, // -142
     5289, // -141
     6663, // -140
     6768, // -139
     5514, // -138
     3105, // -137
       27, // -136
    -3057, // -135
    -5490, // -134
    -6780, // -133
    -6705, // -132
    -5334, // -131
    -2970, // -130
      -42, // -129
     2913, // -128
     5340, // -127
     6708, // -126
     6702, // -125
     5298, // -124
     2787, // -123
     -291, // -122
    -3300, // -121
    -5604, // -120
    -6795, // -119
    -6711, // -118
    -5376, // -117
    -3003, // -116
       -9, // -115
     3021, // -114
     5436, // -113
     6723, // -112
     6618, // -111
     5136, // -110
     2595, // -109
     -468, // -108
    -3405, // -107
    -5673, // -106
    -6873, // -105
    -6792, // -104
    -5391, // -103
    -2910, // -102
      183, // -101
     3249, // -100
     5649, //  -99
     6867, //  -98
     6633, //  -97
     5031, //  -96
     2436, //  -95
     -612, //  -94
    -3519, //  -93
    -5760, //  -92
    -6912, //  -91
    -6729, //  -90
    -5229, //  -89
    -2649, //  -88
      528, //  -87
     3624, //  -86
     5925, //  -85
     6939, //  -84
     6552, //  -83
     4908, //  -82
     2346, //  -81
     -654, //  -80
    -3540, //  -79
    -5757, //  -78
    -6867, //  -77
    -6633, //  -76
    -5025, //  -75
    -2301, //  -74
      927, //  -73
     3906, //  -72
     6036, //  -71
     6954, //  -70
     6555, //  -69
     4914, //  -68
     2343, //  -67
     -699, //  -66
    -3621, //  -65
    -5853, //  -64
    -6909, //  -63
    -6510, //  -62
    -4713, //  -61
    -1938, //  -60
     1200, //  -59
     4071, //  -58
     6126, //  -57
     6993, //  -56
     6531, //  -55
     4833, //  -54
     2199, //  -53
     -888, //  -52
    -3822, //  -51
    -5967, //  -50
    -6849, //  -49
    -6306, //  -48
    -4482, //  -47
    -1755, //  -46
     1320, //  -45
     4116, //  -44
     6093, //  -43
     6900, //  -42
     6411, //  -41
     4689, //  -40
     2013, //  -39
    -1107, //  -38
    -3996, //  -37
    -6051, //  -36
    -6858, //  -35
    -6267, //  -34
    -4422, //  -33
    -1710, //  -32
     1308, //  -31
     4047, //  -30
     6033, //  -29
     6873, //  -28
     6369, //  -27
     4545, //  -26
     1755, //  -25
    -1410, //  -24
    -4266, //  -23
    -6228, //  -22
    -6924, //  -21
    -6258, //  -20
    -4407, //  -19
    -1731, //  -18
     1278, //  -17
     4083, //  -16
     6102, //  -15
     6885, //  -14
     6228, //  -13
     4275, //  -12
     1437, //  -11
    -1686, //  -10
    -4443, //   -9
    -6288, //   -8
    -6924, //   -7
    -6279, //   -6
    -4449, //   -5
    -1734, //   -4
     1374, //   -3
     4218, //   -2
     6180, //   -1
     6855, //    0
     6114, //    1
     4104, //    2
     1257, //    3
    -1818, //    4
    -4518, //    5
    -6357, //    6
    -7008, //    7
    -6333, //    8
    -4395, //    9
    -1560, //   10
     1617, //   11
     4467, //   12
     6381, //   13
     6957, //   14
     6078, //   15
     3969, //   16
     1110, //   17
    -1941, //   18
    -4617, //   19
    -6423, //   20
    -6999, //   21
    -6213, //   22
    -4173, //   23
    -1248, //   24
     1989, //   25
     4803, //   26
     6561, //   27
     6939, //   28
     5964, //   29
     3873, //   30
     1053, //   31
    -1971, //   32
    -4626, //   33
    -6399, //   34
    -6930, //   35
    -6063, //   36
    -3894, //   37
     -855, //   38
     2346, //   39
     4992, //   40
     6603, //   41
     6939, //   42
     5976, //   43
     3873, //   44
     1020, //   45
    -2040, //   46
    -4731, //   47
    -6492, //   48
    -6906, //   49
    -5844, //   50
    -3546, //   51
     -540, //   52
     2541, //   53
     5100, //   54
     6666, //   55
     6951, //   56
     5919, //   57
     3756, //   58
      840, //   59
    -2262, //   60
    -4914, //   61
    -6528, //   62
    -6774, //   63
    -5628, //   64
    -3348, //   65
     -405, //   66
     2610, //   67
     5097, //   68
     6591, //   69
     6840, //   70
     5784, //   71
     3585, //   72
      633, //   73
    -2472, //   74
    -5055, //   75
    -6582, //   76
    -6768, //   77
    -5595, //   78
    -3315, //   79
     -402, //   80
     2565, //   81
     5031, //   82
     6558, //   83
     6816, //   84
     5697, //   85
     3378, //   86
      339, //   87
    -2772, //   88
    -5292, //   89
    -6714, //   90
    -6804, //   91
    -5580, //   92
    -3321, //   93
     -429, //   94
     2574, //   95
     5112, //   96
     6624, //   97
     6765, //   98
     5493, //   99
     3075, //  100
       30, //  101
    -3000, //  102
    -5400, //  103
    -6732, //  104
    -6804, //  105
    -5613, //  106
    -3339, //  107
     -366, //  108
     2721, //  109
     5247, //  110
     6669, //  111
     6702, //  112
     5355, //  113
     2907, //  114
     -114, //  115
    -3093, //  116
    -5460, //  117
    -6804, //  118
    -6879, //  119
    -5619, //  120
    -3210, //  121
     -129, //  122
     2991, //  123
     5490, //  124
     6834, //  125
     6744, //  126
     5268, //  127
     2769, //  128
     -237, //  129
    -3189, //  130
    -5541, //  131
    -6837, //  132
    -6810, //  133
    -5433, //  134
    -2925, //  135
      234, //  136
     3369, //  137
     5757, //  138
     6912, //  139
     6669, //  140
     5163, //  141
     2697, //  142
     -282, //  143
    -3216, //  144
    -5541, //  145
    -6798, //  146
    -6708, //  147
    -5217, //  148
    -2577, //  149
      621, //  150
     3639, //  151
     5856, //  152
     6915, //  153
     6672, //  154
     5172, //  155
     2685, //  156
     -345, //  157
    -3321, //  158
    -5661, //  159
    -6855, //  160
    -6597, //  161
    -4926, //  162
    -2241, //  163
      858, //  164
     3762, //  165
     5925, //  166
     6948, //  167
     6645, //  168
     5076, //  169
     2520, //  170
     -561, //  171
    -3549, //  172
    -5796, //  173
    -6813, //  174
    -6417, //  175
    -4731, //  176
    -2100, //  177
      945, //  178
     3792, //  179
     5886, //  180
     6852, //  181
     6519, //  182
     4926, //  183
     2319, //  184
     -792, //  185
    -3744, //  186
    -5901, //  187
    -6846, //  188
    -6414, //  189
    -4710, //  190
    -2088, //  191
      921, //  192
     3732, //  193
     5838, //  194
     6837, //  195
     6477, //  196
     4776, //  197
     2058, //  198
    -1101, //  199
    -4011, //  200
    -6084, //  201
    -6930, //  202
    -6420, //  203
    -4704, //  204
    -2106, //  205
      909, //  206
     3786, //  207
     5940, //  208
     6867, //  209
     6354, //  210
     4518, //  211
     1746, //  212
    -1365, //  213
    -4173, //  214
    -6138, //  215
    -6933, //  216
    -6441, //  217
    -4734, //  218
    -2082, //  219
     1038, //  220
     3963, //  221
     6054, //  222
     6867, //  223
     6261, //  224
     4374, //  225
     1596, //  226
    -1476, //  227
    -4236, //  228
    -6201, //  229
    -7005, //  230
    -6483, //  231
    -4659, //  232
    -1872, //  233
     1320, //  234
     4242, //  235
     6273, //  236
     6981, //  237
     6243, //  238
     4266, //  239
     1473, //  240
    -1581, //  241
    -4335, //  242
    -6267, //  243
    -6993, //  244
    -6345, //  245
    -4413, //  246
    -1536, //  247
     1707, //  248
     4581, //  249
     6447, //  250
     6972, //  251
     6150, //  252
     4173, //  253
     1416, //  254
    -1614, //  255
    -4353, //  256
    -6252, //  257
    -6924, //  258
    -6189, //  259
    -4128, //  260
    -1149, //  261
     2043, //  262
     4752, //  263
     6477, //  264
     6969, //  265
     6156, //  266
     4173, //  267
     1380, //  268
    -1704, //  269
    -4482, //  270
    -6360, //  271
    -6918, //  272
    -5994, //  273
    -3810, //  274
     -873, //  275
     2202, //  276
     4830, //  277
     6525, //  278
     6975, //  279
     6090, //  280
     4038, //  281
     1176, //  282
    -1953, //  283
    -4689, //  284
    -6417, //  285
    -6804, //  286
    -5808, //  287
    -3657, //  288
     -777, //  289
     2247, //  290
     4821, //  291
     6456, //  292
     6858, //  293
     5949, //  294
     3852, //  295
      942, //  296
    -2184, //  297
    -4842, //  298
    -6492, //  299
    -6825, //  300
    -5811, //  301
    -3648, //  302
     -789, //  303
     2199, //  304
     4764, //  305
     6432, //  306
     6849, //  307
     5868, //  308
     3648, //  309
      645, //  310
    -2481, //  311
    -5076, //  312
    -6633, //  313
    -6876, //  314
    -5811, //  315
    -3660, //  316
     -810, //  317
     2229, //  318
     4869, //  319
     6522, //  320
     6816, //  321
     5673, //  322
     3357, //  323
      354, //  324
    -2691, //  325
    -5172, //  326
    -6648, //  327
    -6879, //  328
    -5832, //  329
    -3660, //  330
     -714, //  331
     2418, //  332
     5049, //  333
     6600, //  334
     6777, //  335
     5565, //  336
     3219, //  337
      234, //  338
    -2766, //  339
    -5229, //  340
    -6714, //  341
    -6945, //  342
    -5820, //  343
    -3498, //  344
     -438, //  345
     2721, //  346
     5310, //  347
     6777, //  348
     6831, //  349
     5496, //  350
     3096, //  351
      132, //  352
    -2859, //  353
    -5313, //  354
    -6753, //  355
    -6867, //  356
    -5610, //  357
    -3189, //  358
      -57, //  359
     3102, //  360
     5577, //  361
     6855, //  362
     6768, //  363
     5403, //  364
     3036, //  365
       84, //  366
    -2895, //  367
    -5325, //  368
    -6717, //  369
    -6762, //  370
    -5394, //  371
    -2850, //  372
      309, //  373
     3348, //  374
     5661, //  375
     6855, //  376
     6771, //  377
     5412, //  378
     3012, //  379
        9, //  380
    -3021, //  381
    -5466, //  382
    -6783, //  383
    -6666, //  384
    -5136, //  385
    -2550, //  386
      510, //  387
     3444, //  388
     5709, //  389
     6876, //  390
     6735, //  391
     5304, //  392
     2826, //  393
     -234, //  394
    -3273, //  395
    -5619, //  396
    -6765, //  397
    -6522, //  398
    -4974, //  399
    -2451, //  400
      555, //  401
     3456, //  402
     5664, //  403
     6783, //  404
     6609, //  405
     5142, //  406
     2613, //  407
     -480, //  408
    -3477, //  409
    -5733, //  410
    -6822, //  411
    -6549, //  412
    -4983, //  413
    -2457, //  414
      525, //  415
     3402, //  416
     5634, //  417
     6783, //  418
     6579, //  419
     5001, //  420
     2355, //  421
     -786, //  422
    -3744, //  423
    -5925, //  424
    -6915, //  425
    -6567, //  426
    -4986, //  427
    -2469, //  428
      534, //  429
     3486, //  430
     5766, //  431
     6843, //  432
     6474, //  433
     4758, //  434
     2064, //  435
    -1029, //  436
    -3891, //  437
    -5967, //  438
    -6912, //  439
    -6576, //  440
    -5007, //  441
    -2418, //  442
      708, //  443
     3711, //  444
     5913, //  445
     6864, //  446
     6405, //  447
     4632, //  448
     1935, //  449
    -1125, //  450
    -3945, //  451
    -6024, //  452
    -6987, //  453
    -6606, //  454
    -4908, //  455
    -2169, //  456
     1029, //  457
     4008, //  458
     6150, //  459
     6996, //  460
     6399, //  461
     4542, //  462
     1833, //  463
    -1212, //  464
    -4032, //  465
    -6090, //  466
    -6969, //  467
    -6456, //  468
    -4635, //  469
    -1815, //  470
     1422, //  471
     4347, //  472
     6315, //  473
     6984, //  474
     6312, //  475
     4467, //  476
     1779, //  477
    -1260, //  478
    -4071, //  479
    -6093, //  480
    -6906, //  481
    -6309, //  482
    -4359, //  483
    -1455, //  484
     1728, //  485
     4488, //  486
     6336, //  487
     6975, //  488
     6318, //  489
     4455, //  490
     1722, //  491
    -1374, //  492
    -4224, //  493
    -6216, //  494
    -6915, //  495
    -6132, //  496
    -4071, //  497
    -1209, //  498
     1854, //  499
     4548, //  500
     6369, //  501
     6975, //  502
     6243, //  503
     4302, //  504
     1491, //  505
    -1644, //  506
    -4446, //  507
    -6297, //  508
    -6825, //  509
    -5982, //  510
    -3960, //  511
    -1158, //  512
     1869, //  513
     4524, //  514
     6297, //  515
     6864, //  516
     6102, //  517
     4116, //  518
     1251, //  519
    -1887, //  520
    -4614, //  521
    -6387, //  522
    -6873, //  523
    -6015, //  524
    -3978, //  525
    -1182, //  526
     1830, //  527
     4485, //  528
     6294, //  529
     6864, //  530
     6027, //  531
     3906, //  532
      960, //  533
    -2175, //  534
    -4845, //  535
    -6528, //  536
    -6936, //  537
    -6015, //  538
    -3978, //  539
    -1179, //  540
     1887, //  541
     4623, //  542
     6423, //  543
     6858, //  544
     5850, //  545
     3636, //  546
      696, //  547
    -2367, //  548
    -4935, //  549
    -6540, //  550
    -6930, //  551
    -6033, //  552
    -3963, //  553
    -1050, //  554
     2124, //  555
     4845, //  556
     6519, //  557
     6840, //  558
     5760, //  559
     3525, //  560
      597, //  561
    -2427, //  562
    -4977, //  563
    -6603, //  564
    -6987, //  565
    -6000, //  566
    -3768, //  567
     -735, //  568
     2454, //  569
     5124, //  570
     6711, //  571
     6906, //  572
     5712, //  573
     3423, //  574
      501, //  575
    -2517, //  576
    -5061, //  577
    -6639, //  578
    -6906, //  579
    -5775, //  580
    -3441, //  581
     -354, //  582
     2826, //  583
     5379, //  584
     6783, //  585
     6846, //  586
     5631, //  587
     3366, //  588
      450, //  589
    -2568, //  590
    -5094, //  591
    -6618, //  592
    -6810, //  593
    -5571, //  594
    -3123, //  595
       -9, //  596
     3045, //  597
     5436, //  598
     6777, //  599
     6846, //  600
     5628, //  601
     3330, //  602
      354, //  603
    -2715, //  604
    -5259, //  605
    -6705, //  606
    -6735, //  607
    -5340, //  608
    -2862, //  609
      153, //  610
     3108, //  611
     5469, //  612
     6789, //  613
     6804, //  614
     5514, //  615
     3123, //  616
       87, //  617
    -2991, //  618
    -5430, //  619
    -6708, //  620
    -6618, //  621
    -5214, //  622
    -2799, //  623
      171, //  624
     3102, //  625
     5424, //  626
     6699, //  627
     6681, //  628
     5346, //  629
     2904, //  630
     -165, //  631
    -3204, //  632
    -5556, //  633
    -6783, //  634
    -6669, //  635
    -5256, //  636
    -2826, //  637
      141, //  638
     3072, //  639
     5418, //  640
     6720, //  641
     6666, //  642
     5211, //  643
     2649, //  644
     -471, //  645
    -3465, //  646
    -5742, //  647
    -6882, //  648
    -6693, //  649
    -5253, //  650
    -2817, //  651
      177, //  652
     3195, //  653
     5586, //  654
     6804, //  655
     6582, //  656
     4983, //  657
     2379, //  658
     -690, //  659
    -3597, //  660
    -5781, //  661
    -6873, //  662
    -6693, //  663
    -5253, //  664
    -2739, //  665
      387, //  666
     3453, //  667
     5769, //  668
     6852, //  669
     6534, //  670
     4887, //  671
     2280, //  672
     -759, //  673
    -3636, //  674
    -5832, //  675
    -6942, //  676
    -6714, //  677
    -5127, //  678
    -2457, //  679
      735, //  680
     3774, //  681
     6012, //  682
     6987, //  683
     6540, //  684
     4815, //  685
     2187, //  686
     -846, //  687
    -3726, //  688
    -5904, //  689
    -6927, //  690
    -6555, //  691
    -4839, //  692
    -2091, //  693
     1122, //  694
     4092, //  695
     6168, //  696
     6981, //  697
     6456, //  698
     4743, //  699
     2130, //  700
     -900, //  701
    -3780, //  702
    -5919, //  703
    -6873, //  704
    -6417, //  705
    -4587, //  706
    -1761, //  707
     1398, //  708
     4212, //  709
     6171, //  710
     6963, //  711
     6456, //  712
     4725, //  713
     2055, //  714
    -1047, //  715
    -3960, //  716
    -6069, //  717
    -6900, //  718
    -6261, //  719
    -4335, //  720
    -1563, //  721
     1491, //  722
     4245, //  723
     6189, //  724
     6954, //  725
     6378, //  726
     4554, //  727
     1806, //  728
    -1338, //  729
    -4200, //  730
    -6168, //  731
    -6840, //  732
    -6147, //  733
    -4260, //  734
    -1539, //  735
     1485, //  736
     4218, //  737
     6126, //  738
     6855, //  739
     6240, //  740
     4362, //  741
     1560, //  742
    -1584, //  743
    -4380, //  744
    -6267, //  745
    -6906, //  746
    -6201, //  747
    -4299, //  748
    -1569, //  749
     1455, //  750
     4203, //  751
     6147, //  752
     6873, //  753
     6174, //  754
     4161, //  755
     1272, //  756
    -1869, //  757
    -4602, //  758
    -6408, //  759
    -6969, //  760
    -6210, //  761
    -4293, //  762
    -1545, //  763
     1551, //  764
     4383, //  765
     6312, //  766
     6891, //  767
     6021, //  768
     3912, //  769
     1029, //  770
    -2031, //  771
    -4677, //  772
    -6417, //  773
    -6963, //  774
    -6216, //  775
    -4245, //  776
    -1374, //  777
     1824, //  778
     4635, //  779
     6432, //  780
     6888, //  781
     5952, //  782
     3822, //  783
      957, //  784
    -2076, //  785
    -4713, //  786
    -6477, //  787
    -7014, //  788
    -6159, //  789
    -4023, //  790
    -1029, //  791
     2181, //  792
     4920, //  793
     6627, //  794
     6966, //  795
     5919, //  796
     3744, //  797
      873, //  798
    -2169, //  799
    -4806, //  800
    -6519, //  801
    -6927, //  802
    -5928, //  803
    -3693, //  804
     -648, //  805
     2538, //  806
     5160, //  807
     6693, //  808
     6903, //  809
     5835, //  810
     3678, //  811
      813, //  812
    -2235, //  813
    -4851, //  814
    -6510, //  815
    -6840, //  816
    -5733, //  817
    -3396, //  818
     -348, //  819
     2724, //  820
     5196, //  821
     6672, //  822
     6897, //  823
     5829, //  824
     3627, //  825
      684, //  826
    -2415, //  827
    -5043, //  828
    -6618, //  829
    -6789, //  830
    -5535, //  831
    -3174, //  832
     -219, //  833
     2760, //  834
     5217, //  835
     6675, //  836
     6855, //  837
     5703, //  838
     3408, //  839
      399, //  840
    -2706, //  841
    -5226, //  842
    -6639, //  843
    -6699, //  844
    -5448, //  845
    -3144, //  846
     -228, //  847
     2745, //  848
     5178, //  849
     6600, //  850
     6738, //  851
     5535, //  852
     3186, //  853
      144, //  854
    -2925, //  855
    -5367, //  856
    -6723, //  857
    -6768, //  858
    -5505, //  859
    -3183, //  860
     -249, //  861
     2736, //  862
     5196, //  863
     6645, //  864
     6744, //  865
     5412, //  866
     2931, //  867
     -147, //  868
    -3174, //  869
    -5544, //  870
    -6828, //  871
    -6801, //  872
    -5496, //  873
    -3156, //  874
     -174, //  875
     2892, //  876
     5403, //  877
     6759, //  878
     6678, //  879
     5205, //  880
     2694, //  881
     -342, //  882
    -3288, //  883
    -5574, //  884
    -6810, //  885
    -6792, //  886
    -5478, //  887
    -3042, //  888
       75, //  889
     3195, //  890
     5607, //  891
     6822, //  892
     6648, //  893
     5136, //  894
     2619, //  895
     -393, //  896
    -3315, //  897
    -5616, //  898
    -6876, //  899
    -6801, //  900
    -5331, //  901
    -2733, //  902
      450, //  903
     3528, //  904
     5853, //  905
     6960, //  906
     6663, //  907
     5076, //  908
     2535, //  909
     -477, //  910
    -3414, //  911
    -5703, //  912
    -6867, //  913
    -6636, //  914
    -5040, //  915
    -2370, //  916
      822, //  917
     3822, //  918
     5997, //  919
     6945, //  920
     6582, //  921
     5004, //  922
     2478, //  923
     -546, //  924
    -3486, //  925
    -5736, //  926
    -6825, //  927
    -6510, //  928
    -4803, //  929
    -2070, //  930
     1059, //  931
     3915, //  932
     5982, //  933
     6924, //  934
     6579, //  935
     4971, //  936
     2376, //  937
     -723, //  938
    -3696, //  939
    -5907, //  940
    -6873, //  941
    -6387, //  942
    -4596, //  943
    -1911, //  944
     1116, //  945
     3927, //  946
     5991, //  947
     6915, //  948
     6495, //  949
     4794, //  950
     2109, //  951
    -1032, //  952
    -3954, //  953
    -6021, //  954
    -6837, //  955
    -6300, //  956
    -4548, //  957
    -1920, //  958
     1101, //  959
     3906, //  960
     5946, //  961
     6825, //  962
     6360, //  963
     4596, //  964
     1863, //  965
    -1275, //  966
    -4134, //  967
    -6132, //  968
    -6921, //  969
    -6375, //  970
    -4605, //  971
    -1944, //  972
     1089, //  973
     3918, //  974
     5994, //  975
     6870, //  976
     6315, //  977
     4410, //  978
     1584, //  979
    -1542, //  980
    -4338, //  981
    -6264, //  982
    -6981, //  983
    -6381, //  984
    -4581, //  985
    -1890, //  986
     1221, //  987
     4134, //  988
     6186, //  989
     6912, //  990
     6174, //  991
     4185, //  992
     1371, //  993
    -1689, //  994
    -4401, //  995
    -6270, //  996
    -6969, //  997
    -6369, //  998
    -4509, //  999
    -1683, // 1000
     1536, // 1001
     4422, // 1002
     6333, // 1003
     6927, // 1004
     6132, // 1005
     4119, // 1006
     1323, // 1007
    -1719, // 1008
    -4434, // 1009
    -6321, // 1010
    -7011, // 1011
    -6297, // 1012
    -4266, // 1013
    -1314, // 1014
     1905, // 1015
     4707, // 1016
     6525, // 1017
     7011, // 1018
     6108, // 1019
     4050, // 1020
     1236, // 1021
    -1815, // 1022
    -4536, // 1023
    -6375, // 1024
    -6930, // 1025
    -6063, // 1026
    -3933, // 1027
     -948, // 1028
     2238, // 1029
     4923, // 1030
     6576, // 1031
     6945, // 1032
     6024, // 1033
     3981, // 1034
     1167, // 1035
    -1899, // 1036
    -4608, // 1037
    -6387, // 1038
    -6864, // 1039
    -5895, // 1040
    -3669, // 1041
     -687, // 1042
     2391, // 1043
     4944, // 1044
     6543, // 1045
     6930, // 1046
     6012, // 1047
     3915, // 1048
     1020, // 1049
    -2109, // 1050
    -4815, // 1051
    -6513, // 1052
    -6828, // 1053
    -5727, // 1054
    -3483, // 1055
     -591, // 1056
     2400, // 1057
     4941, // 1058
     6546, // 1059
     6888, // 1060
     5874, // 1061
     3681, // 1062
      714, // 1063
    -2415, // 1064
    -5019, // 1065
    -6555, // 1066
    -6771, // 1067
    -5670, // 1068
    -3486, // 1069
     -618, // 1070
     2382, // 1071
     4917, // 1072
     6489, // 1073
     6783, // 1074
     5718, // 1075
     3456, // 1076
      456, // 1077
    -2631, // 1078
    -5160, // 1079
    -6648, // 1080
    -6852, // 1081
    -5742, // 1082
    -3525, // 1083
     -624, // 1084
     2400, // 1085
     4968, // 1086
     6558, // 1087
     6801, // 1088
     5604, // 1089
     3225, // 1090
      186, // 1091
    -2865, // 1092
    -5328, // 1093
    -6747, // 1094
    -6882, // 1095
    -5724, // 1096
    -3477, // 1097
     -519, // 1098
     2598, // 1099
     5208, // 1100
     6705, // 1101
     6762, // 1102
     5424, // 1103
     3006, // 1104
       12, // 1105
    -2958, // 1106
    -5352, // 1107
    -6732, // 1108
    -6864, // 1109
    -5688, // 1110
    -3333, // 1111
     -234, // 1112
     2934, // 1113
     5442, // 1114
     6783, // 1115
     6747, // 1116
     5373, // 1117
     2952, // 1118
      -18, // 1119
    -2982, // 1120
    -5394, // 1121
    -6798, // 1122
    -6867, // 1123
    -5523, // 1124
    -2997, // 1125
      159, // 1126
     3261, // 1127
     5679, // 1128
     6918, // 1129
     6771, // 1130
     5325, // 1131
     2877, // 1132
     -105, // 1133
    -3093, // 1134
    -5487, // 1135
    -6792, // 1136
    -6705, // 1137
    -5226, // 1138
    -2649, // 1139
      507, // 1140
     3537, // 1141
     5802, // 1142
     6900, // 1143
     6690, // 1144
     5241, // 1145
     2808, // 1146
     -198, // 1147
    -3192, // 1148
    -5550, // 1149
    -6771, // 1150
    -6594, // 1151
    -5022, // 1152
    -2391, // 1153
      702, // 1154
     3600, // 1155
     5775, // 1156
     6864, // 1157
     6675, // 1158
     5202, // 1159
     2691, // 1160
     -402, // 1161
    -3423, // 1162
    -5739, // 1163
    -6840, // 1164
    -6507, // 1165
    -4848, // 1166
    -2265, // 1167
      732, // 1168
     3591, // 1169
     5778, // 1170
     6858, // 1171
     6594, // 1172
     5016, // 1173
     2406, // 1174
     -717, // 1175
    -3690, // 1176
    -5865, // 1177
    -6825, // 1178
    -6447, // 1179
    -4833, // 1180
    -2295, // 1181
      708, // 1182
     3579, // 1183
     5748, // 1184
     6783, // 1185
     6468, // 1186
     4830, // 1187
     2160, // 1188
     -966, // 1189
    -3870, // 1190
    -5976, // 1191
    -6918, // 1192
    -6528, // 1193
    -4896, // 1194
    -2313, // 1195
      729, // 1196
     3633, // 1197
     5835, // 1198
     6855, // 1199
     6441, // 1200
     4656, // 1201
     1902, // 1202
    -1212, // 1203
    -4065, // 1204
    -6111, // 1205
    -6981, // 1206
    -6528, // 1207
    -4851, // 1208
    -2223, // 1209
      900, // 1210
     3888, // 1211
     6063, // 1212
     6918, // 1213
     6321, // 1214
     4452, // 1215
     1713, // 1216
    -1335, // 1217
    -4110, // 1218
    -6099, // 1219
    -6954, // 1220
    -6501, // 1221
    -4758, // 1222
    -1986, // 1223
     1245, // 1224
     4194, // 1225
     6219, // 1226
     6951, // 1227
     6294, // 1228
     4410, // 1229
     1680, // 1230
    -1359, // 1231
    -4140, // 1232
    -6159, // 1233
    -6996, // 1234
    -6423, // 1235
    -4494, // 1236
    -1602, // 1237
     1617, // 1238
     4479, // 1239
     6402, // 1240
     7026, // 1241
     6282, // 1242
     4341, // 1243
     1602, // 1244
    -1458, // 1245
    -4257, // 1246
    -6222, // 1247
    -6921, // 1248
    -6189, // 1249
    -4164, // 1250
    -1254, // 1251
     1920, // 1252
     4665, // 1253
     6438, // 1254
     6963, // 1255
     6192, // 1256
     4269, // 1257
     1512, // 1258
    -1572, // 1259
    -4356, // 1260
    -6255, // 1261
    -6867, // 1262
    -6039, // 1263
    -3939, // 1264
    -1032, // 1265
     2031, // 1266
     4659, // 1267
     6393, // 1268
     6939, // 1269
     6174, // 1270
     4188, // 1271
     1338, // 1272
    -1812, // 1273
    -4590, // 1274
    -6405, // 1275
    -6867, // 1276
    -5916, // 1277
    -3795, // 1278
     -969, // 1279
     2028, // 1280
     4656, // 1281
     6405, // 1282
     6903, // 1283
     6036, // 1284
     3942, // 1285
     1020, // 1286
    -2118, // 1287
    -4800, // 1288
    -6459, // 1289
    -6828, // 1290
    -5886, // 1291
    -3819, // 1292
    -1011, // 1293
     2016, // 1294
     4647, // 1295
     6366, // 1296
     6816, // 1297
     5886, // 1298
     3723, // 1299
      771, // 1300
    -2334, // 1301
    -4935, // 1302
    -6558, // 1303
    -6921, // 1304
    -5964, // 1305
    -3858, // 1306
     -996, // 1307
     2067, // 1308
     4737, // 1309
     6468, // 1310
     6861, // 1311
     5793, // 1312
     3504, // 1313
      516, // 1314
    -2544, // 1315
    -5091, // 1316
    -6654, // 1317
    -6945, // 1318
    -5931, // 1319
    -3780, // 1320
     -849, // 1321
     2310, // 1322
     5019, // 1323
     6633, // 1324
     6831, // 1325
     5622, // 1326
     3315, // 1327
      375, // 1328
    -2622, // 1329
    -5106, // 1330
    -6630, // 1331
    -6918, // 1332
    -5874, // 1333
    -3612, // 1334
     -537, // 1335
     2667, // 1336
     5256, // 1337
     6726, // 1338
     6828, // 1339
     5598, // 1340
     3288, // 1341
      351, // 1342
    -2646, // 1343
    -5151, // 1344
    -6696, // 1345
    -6915, // 1346
    -5694, // 1347
    -3261, // 1348
     -135, // 1349
     2994, // 1350
     5484, // 1351
     6852, // 1352
     6858, // 1353
     5556, // 1354
     3210, // 1355
      255, // 1356
    -2772, // 1357
    -5268, // 1358
    -6702, // 1359
    -6756, // 1360
    -5409, // 1361
    -2925, // 1362
      180, // 1363
     3231, // 1364
     5589, // 1365
     6822, // 1366
     6774, // 1367
     5469, // 1368
     3126, // 1369
      141, // 1370
    -2895, // 1371
    -5349, // 1372
    -6699, // 1373
    -6666, // 1374
    -5235, // 1375
    -2709, // 1376
      342, // 1377
     3270, // 1378
     5547, // 1379
     6786, // 1380
     6753, // 1381
     5421, // 1382
     2985, // 1383
      -78, // 1384
    -3147, // 1385
    -5553, // 1386
    -6792, // 1387
    -6612, // 1388
    -5100, // 1389
    -2616, // 1390
      345, // 1391
     3252, // 1392
     5550, // 1393
     6780, // 1394
     6672, // 1395
     5223, // 1396
     2691, // 1397
     -408, // 1398
    -3423, // 1399
    -5691, // 1400
    -6792, // 1401
    -6570, // 1402
    -5109, // 1403
    -2661, // 1404
      318, // 1405
     3258, // 1406
     5547, // 1407
     6729, // 1408
     6564, // 1409
     5043, // 1410
     2460, // 1411
     -645, // 1412
    -3597, // 1413
    -5805, // 1414
    -6888, // 1415
    -6669, // 1416
    -5172, // 1417
    -2661, // 1418
      372, // 1419
     3348, // 1420
     5664, // 1421
     6831, // 1422
     6558, // 1423
     4893, // 1424
     2220, // 1425
     -873, // 1426
    -3774, // 1427
    -5928, // 1428
    -6948, // 1429
    -6654, // 1430
    -5103, // 1431
    -2538, // 1432
      582, // 1433
     3639, // 1434
     5922, // 1435
     6912, // 1436
     6456, // 1437
     4713, // 1438
     2061, // 1439
     -975, // 1440
    -3807, // 1441
    -5916, // 1442
    -6921, // 1443
    -6618, // 1444
    -4989, // 1445
    -2274, // 1446
      957, // 1447
     3960, // 1448
     6084, // 1449
     6951, // 1450
     6444, // 1451
     4689, // 1452
     2040, // 1453
     -987, // 1454
    -3840, // 1455
    -5982, // 1456
    -6966, // 1457
    -6531, // 1458
    -4707, // 1459
    -1881, // 1460
     1326, // 1461
     4227, // 1462
     6255, // 1463
     7023, // 1464
     6435, // 1465
     4629, // 1466
     1956, // 1467
    -1107, // 1468
    -3975, // 1469
    -6060, // 1470
    -6894, // 1471
    -6300, // 1472
    -4395, // 1473
    -1566, // 1474
     1590, // 1475
     4392, // 1476
     6279, // 1477
     6954, // 1478
     6339, // 1479
     4539, // 1480
     1845, // 1481
    -1245, // 1482
    -4095, // 1483
    -6114, // 1484
    -6864, // 1485
    -6180, // 1486
    -4212, // 1487
    -1386, // 1488
     1674, // 1489
     4368, // 1490
     6222, // 1491
     6924, // 1492
     6315, // 1493
     4446, // 1494
     1653, // 1495
    -1503, // 1496
    -4350, // 1497
    -6282, // 1498
    -6888, // 1499
    -6090, // 1500
    -4104, // 1501
    -1356, // 1502
     1650, // 1503
     4356, // 1504
     6237, // 1505
     6903, // 1506
     6180, // 1507
     4191, // 1508
     1326, // 1509
    -1818, // 1510
    -4566, // 1511
    -6348, // 1512
    -6870, // 1513
    -6078, // 1514
    -4146, // 1515
    -1398, // 1516
     1647, // 1517
     4377, // 1518
     6228, // 1519
     6828, // 1520
     6039, // 1521
     3984, // 1522
     1086, // 1523
    -2028, // 1524
    -4698, // 1525
    -6444, // 1526
    -6963, // 1527
    -6162, // 1528
    -4170, // 1529
    -1350, // 1530
     1740, // 1531
     4503, // 1532
     6363, // 1533
     6897, // 1534
     5970, // 1535
     3792, // 1536
      858, // 1537
    -2217, // 1538
    -4839, // 1539
    -6531, // 1540
    -6987, // 1541
    -6117, // 1542
    -4065, // 1543
    -1173, // 1544
     2019, // 1545
     4812, // 1546
     6552, // 1547
     6888, // 1548
     5820, // 1549
     3624, // 1550
      738, // 1551
    -2274, // 1552
    -4845, // 1553
    -6510, // 1554
    -6948, // 1555
    -6042, // 1556
    -3867, // 1557
     -825, // 1558
     2391, // 1559
     5064, // 1560
     6645, // 1561
     6900, // 1562
     5811, // 1563
     3609, // 1564
      720, // 1565
    -2304, // 1566
    -4908, // 1567
    -6582, // 1568
    -6945, // 1569
    -5856, // 1570
    -3513, // 1571
     -432, // 1572
     2706, // 1573
     5271, // 1574
     6765, // 1575
     6927, // 1576
     5769, // 1577
     3528, // 1578
      618, // 1579
    -2442, // 1580
    -5037, // 1581
    -6600, // 1582
    -6792, // 1583
    -5577, // 1584
    -3201, // 1585
     -153, // 1586
     2910, // 1587
     5355, // 1588
     6729, // 1589
     6834, // 1590
     5676, // 1591
     3432, // 1592
      480, // 1593
    -2592, // 1594
    -5139, // 1595
    -6618, // 1596
    -6732, // 1597
    -5439, // 1598
    -3030, // 1599
      -33, // 1600
     2922, // 1601
     5295, // 1602
     6678, // 1603
     6810, // 1604
     5616, // 1605
     3279, // 1606
      237, // 1607
    -2865, // 1608
    -5364, // 1609
    -6732, // 1610
    -6702, // 1611
    -5343, // 1612
    -2970, // 1613
      -45, // 1614
     2895, // 1615
     5307, // 1616
     6693, // 1617
     6744, // 1618
     5421, // 1619
     2976, // 1620
     -102, // 1621
    -3144, // 1622
    -5508, // 1623
    -6744, // 1624
    -6684, // 1625
    -5364, // 1626
    -3024, // 1627
      -66, // 1628
     2925, // 1629
     5334, // 1630
     6669, // 1631
     6651, // 1632
     5256, // 1633
     2754, // 1634
     -324, // 1635
    -3306, // 1636
    -5610, // 1637
    -6840, // 1638
    -6783, // 1639
    -5424, // 1640
    -3003, // 1641
       24, // 1642
     3057, // 1643
     5493, // 1644
     6801, // 1645
     6666, // 1646
     5124, // 1647
     2535, // 1648
     -525, // 1649
    -3462, // 1650
    -5724, // 1651
    -6900, // 1652
    -6765, // 1653
    -5337, // 1654
    -2847, // 1655
      270, // 1656
     3384, // 1657
     5766, // 1658
     6894, // 1659
     6582, // 1660
     4965, // 1661
     2403, // 1662
     -606, // 1663
    -3489, // 1664
    -5712, // 1665
    -6864, // 1666
    -6711, // 1667
    -5199, // 1668
    -2556, // 1669
      666, // 1670
     3717, // 1671
     5937, // 1672
     6933, // 1673
     6576, // 1674
     4956, // 1675
     2397, // 1676
     -621, // 1677
    -3534, // 1678
    -5790, // 1679
    -6918, // 1680
    -6618, // 1681
    -4914, // 1682
    -2166, // 1683
     1020, // 1684
     3966, // 1685
     6090, // 1686
     7008, // 1687
     6567, // 1688
     4896, // 1689
     2295, // 1690
     -759, // 1691
    -3693, // 1692
    -5886, // 1693
    -6861, // 1694
    -6402, // 1695
    -4626, // 1696
    -1884, // 1697
     1245, // 1698
     4089, // 1699
     6096, // 1700
     6930, // 1701
     6468, // 1702
     4794, // 1703
     2166, // 1704
     -918, // 1705
    -3840, // 1706
    -5958, // 1707
    -6849, // 1708
    -6318, // 1709
    -4482, // 1710
    -1746, // 1711
     1302, // 1712
     4056, // 1713
     6033, // 1714
     6897, // 1715
     6441, // 1716
     4686, // 1717
     1959, // 1718
    -1200, // 1719
    -4110, // 1720
    -6144, // 1721
    -6900, // 1722
    -6258, // 1723
    -4407, // 1724
    -1731, // 1725
     1263, // 1726
     4050, // 1727
     6069, // 1728
     6882, // 1729
     6306, // 1730
     4434, // 1731
     1632, // 1732
    -1515, // 1733
    -4320, // 1734
    -6222, // 1735
    -6891, // 1736
    -6261, // 1737
    -4455, // 1738
    -1776, // 1739
     1284, // 1740
     4101, // 1741
     6087, // 1742
     6837, // 1743
     6183, // 1744
     4242, // 1745
     1410, // 1746
    -1701, // 1747
    -4443, // 1748
    -6312, // 1749
    -6987, // 1750
    -6345, // 1751
    -4470, // 1752
    -1701, // 1753
     1419, // 1754
     4266, // 1755
     6252, // 1756
     6927, // 1757
     6135, // 1758
     4068, // 1759
     1203, // 1760
    -1875, // 1761
    -4575, // 1762
    -6396, // 1763
    -7005, // 1764
    -6279, // 1765
    -4332, // 1766
    -1482, // 1767
     1731, // 1768
     4599, // 1769
     6459, // 1770
     6933, // 1771
     6009, // 1772
     3927, // 1773
     1107, // 1774
    -1923, // 1775
    -4578, // 1776
    -6369, // 1777
    -6957, // 1778
    -6186, // 1779
    -4113, // 1780
    -1113, // 1781
     2121, // 1782
     4854, // 1783
     6549, // 1784
     6948, // 1785
     6009, // 1786
     3921, // 1787
     1092, // 1788
    -1953, // 1789
    -4647, // 1790
    -6450, // 1791
    -6957, // 1792
    -6003, // 1793
    -3756, // 1794
     -735, // 1795
     2412, // 1796
     5040, // 1797
     6660, // 1798
     6978, // 1799
     5964, // 1800
     3837, // 1801
      966, // 1802
    -2115, // 1803
    -4797, // 1804
    -6486, // 1805
    -6822, // 1806
    -5745, // 1807
    -3480, // 1808
     -495, // 1809
     2574, // 1810
     5094, // 1811
     6609, // 1812
     6876, // 1813
     5862, // 1814
     3720, // 1815
      807, // 1816
    -2292, // 1817
    -4923, // 1818
    -6534, // 1819
    -6789, // 1820
    -5640, // 1821
    -3348, // 1822
     -414, // 1823
     2568, // 1824
     5037, // 1825
     6564, // 1826
     6852, // 1827
     5802, // 1828
     3549, // 1829
      549, // 1830
    -2583, // 1831
    -5166, // 1832
    -6657, // 1833
    -6783, // 1834
    -5577, // 1835
    -3315, // 1836
     -438, // 1837
     2538, // 1838
     5055, // 1839
     6588, // 1840
     6798, // 1841
     5604, // 1842
     3252, // 1843
      216, // 1844
    -2853, // 1845
    -5307, // 1846
    -6684, // 1847
    -6780, // 1848
    -5607, // 1849
    -3369, // 1850
     -450, // 1851
     2598, // 1852
     5118, // 1853
     6594, // 1854
     6720, // 1855
     5457, // 1856
     3048, // 1857
       12, // 1858
    -2997, // 1859
    -5403, // 1860
    -6777, // 1861
    -6873, // 1862
    -5658, // 1863
    -3321, // 1864
     -318, // 1865
     2775, // 1866
     5313, // 1867
     6750, // 1868
     6759, // 1869
     5346, // 1870
     2856, // 1871
     -168, // 1872
    -3141, // 1873
    -5508, // 1874
    -6822, // 1875
    -6843, // 1876
    -5547, // 1877
    -3135, // 1878
      -30, // 1879
     3123, // 1880
     5604, // 1881
     6858, // 1882
     6690, // 1883
     5211, // 1884
     2745, // 1885
     -234, // 1886
    -3162, // 1887
    -5493, // 1888
    -6789, // 1889
    -6780, // 1890
    -5394, // 1891
    -2826, // 1892
      372, // 1893
     3459, // 1894
     5769, // 1895
     6903, // 1896
     6690, // 1897
     5211, // 1898
     2739, // 1899
     -258, // 1900
    -3225, // 1901
    -5583, // 1902
    -6852, // 1903
    -6693, // 1904
    -5106, // 1905
    -2442, // 1906
      705, // 1907
     3684, // 1908
     5904, // 1909
     6963, // 1910
     6684, // 1911
     5145, // 1912
     2625, // 1913
     -414, // 1914
    -3405, // 1915
    -5700, // 1916
    -6810, // 1917
    -6498, // 1918
    -4845, // 1919
    -2202, // 1920
      891, // 1921
     3783, // 1922
     5901, // 1923
     6882, // 1924
     6579, // 1925
     5031, // 1926
     2484, // 1927
     -594, // 1928
    -3570, // 1929
    -5799, // 1930
    -6819, // 1931
    -6444, // 1932
    -4743, // 1933
    -2106, // 1934
      918, // 1935
     3732, // 1936
     5835, // 1937
     6852, // 1938
     6546, // 1939
     4914, // 1940
     2250, // 1941
     -891, // 1942
    -3852, // 1943
    -5997, // 1944
    -6894, // 1945
    -6408, // 1946
    -4692, // 1947
    -2109, // 1948
      882, // 1949
     3732, // 1950
     5877, // 1951
     6855, // 1952
     6423, // 1953
     4668, // 1954
     1935, // 1955
    -1200, // 1956
    -4062, // 1957
    -6078, // 1958
    -6894, // 1959
    -6423, // 1960
    -4746, // 1961
    -2145, // 1962
      924, // 1963
     3819, // 1964
     5934, // 1965
     6834, // 1966
     6324, // 1967
     4494, // 1968
     1731, // 1969
    -1371, // 1970
    -4173, // 1971
    -6159, // 1972
    -6990, // 1973
    -6498, // 1974
    -4746, // 1975
    -2034, // 1976
     1101, // 1977
     4029, // 1978
     6132, // 1979
     6945, // 1980
     6288, // 1981
     4341, // 1982
     1542, // 1983
    -1521, // 1984
    -4287, // 1985
    -6237, // 1986
    -6996, // 1987
    -6420, // 1988
    -4578, // 1989
    -1782, // 1990
     1440, // 1991
     4380, // 1992
     6345, // 1993
     6960, // 1994
     6183, // 1995
     4218, // 1996
     1473, // 1997
    -1560, // 1998
    -4293, // 1999
    -6213, // 2000
    -6951, // 2001
    -6318, // 2002
    -4344, // 2003
    -1410, // 2004
     1827, // 2005
     4629, // 2006
     6438, // 2007
     6978, // 2008
     6189, // 2009
     4227, // 2010
     1452, // 2011
    -1608, // 2012
    -4383, // 2013
    -6309, // 2014
    -6957, // 2015
    -6132, // 2016
    -3999, // 2017
    -1047, // 2018
     2091, // 2019
     4788, // 2020
     6531, // 2021
     7002, // 2022
     6144, // 2023
     4125, // 2024
     1311, // 2025
    -1791, // 2026
    -4551, // 2027
    -6363, // 2028
    -6837, // 2029
    -5901, // 2030
    -3759, // 2031
     -849, // 2032
     2223, // 2033
     4821, // 2034
     6471, // 2035
     6900, // 2036
     6033, // 2037
     3996, // 2038
     1131, // 2039
    -1995, // 2040
    -4704, // 2041
    -6426, // 2042
    -6828, // 2043
    -5838, // 2044
    -3672, // 2045
     -801, // 2046
     2196, // 2047
     4755, // 2048
     6429, // 2049
     6879, // 2050
     5967, // 2051
     3816, // 2052
      852, // 2053
    -2292, // 2054
    -4950, // 2055
    -6570, // 2056
    -6852, // 2057
    -5796, // 2058
    -3651, // 2059
     -825, // 2060
     2175, // 2061
     4794, // 2062
     6474, // 2063
     6834, // 2064
     5781, // 2065
     3522, // 2066
      534, // 2067
    -2550, // 2068
    -5088, // 2069
    -6597, // 2070
    -6852, // 2071
    -5832, // 2072
    -3705, // 2073
     -819, // 2074
     2268, // 2075
     4899, // 2076
     6513, // 2077
     6783, // 2078
     5652, // 2079
     3336, // 2080
      348, // 2081
    -2691, // 2082
    -5178, // 2083
    -6687, // 2084
    -6948, // 2085
    -5874, // 2086
    -3633, // 2087
     -645, // 2088
     2490, // 2089
     5121, // 2090
     6693, // 2091
     6840, // 2092
     5559, // 2093
     3174, // 2094
      192, // 2095
    -2808, // 2096
    -5274, // 2097
    -6735, // 2098
    -6909, // 2099
    -5739, // 2100
    -3408, // 2101
     -324, // 2102
     2862, // 2103
     5430, // 2104
     6804, // 2105
     6780, // 2106
     5445, // 2107
     3078, // 2108
      141, // 2109
    -2832, // 2110
    -5265, // 2111
    -6702, // 2112
    -6834, // 2113
    -5568, // 2114
    -3090, // 2115
       78, // 2116
     3189, // 2117
     5580, // 2118
     6843, // 2119
     6792, // 2120
     5454, // 2121
     3078, // 2122
      108, // 2123
    -2901, // 2124
    -5370, // 2125
    -6774, // 2126
    -6753, // 2127
    -5298, // 2128
    -2730, // 2129
      381, // 2130
     3378, // 2131
     5694, // 2132
     6900, // 2133
     6777, // 2134
     5373, // 2135
     2946, // 2136
      -75, // 2137
    -3111, // 2138
    -5511, // 2139
    -6750, // 2140
    -6579, // 2141
    -5067, // 2142
    -2529, // 2143
      522, // 2144
     3450, // 2145
     5673, // 2146
     6807, // 2147
     6666, // 2148
     5253, // 2149
     2787, // 2150
     -282, // 2151
    -3300, // 2152
    -5625, // 2153
    -6786, // 2154
    -6555, // 2155
    -5004, // 2156
    -2466, // 2157
      528, // 2158
     3390, // 2159
     5619, // 2160
     6786, // 2161
     6636, // 2162
     5130, // 2163
     2544, // 2164
     -585, // 2165
    -3588, // 2166
    -5829, // 2167
    -6873, // 2168
    -6549, // 2169
    -4977, // 2170
    -2481, // 2171
      495, // 2172
     3414, // 2173
     5685, // 2174
     6810, // 2175
     6525, // 2176
     4887, // 2177
     2232, // 2178
     -879, // 2179
    -3786, // 2180
    -5910, // 2181
    -6876, // 2182
    -6567, // 2183
    -5025, // 2184
    -2496, // 2185
      564, // 2186
     3537, // 2187
     5778, // 2188
     6813, // 2189
     6447, // 2190
     4734, // 2191
     2052, // 2192
    -1035, // 2193
    -3885, // 2194
    -5985, // 2195
    -6966, // 2196
    -6633, // 2197
    -5001, // 2198
    -2352, // 2199
      792, // 2200
     3792, // 2201
     6006, // 2202
};

/**
 * The position of the moon at the time set on this
 * object, in equatorial coordinates.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
const CalendarAstronomer::Equatorial& CalendarAstronomer::getMoonPosition()
{
    //
    // See page 142 of "Practical Astronomy with your Calculator",
    // by Peter Duffet-Smith, for details on the algorithm.
    //
    if (moonPositionSet == FALSE) {
        // Calculate the solar longitude.  Has the side effect of
        // filling in "meanAnomalySun" as well.
        getSunLongitude();

        //
        // Find the # of days since the epoch of our orbital parameters.
        // TODO: Convert the time of day portion into ephemeris time
        //
        double day = getJulianDay() - JD_EPOCH;       // Days since epoch

        // Calculate the mean longitude and anomaly of the moon, based on
        // a circular orbit.  Similar to the corresponding solar calculation.
        double meanLongitude = norm2PI(13.1763966*PI/180*day + moonL0);
        meanAnomalyMoon = norm2PI(meanLongitude - 0.1114041*PI/180 * day - moonP0);

        //
        // Calculate the following corrections:
        //  Evection:   the sun's gravity affects the moon's eccentricity
        //  Annual Eqn: variation in the effect due to earth-sun distance
        //  A3:         correction factor (for ???)
        //
        double evection = 1.2739*PI/180 * ::sin(2 * (meanLongitude - sunLongitude)
            - meanAnomalyMoon);
        double annual   = 0.1858*PI/180 * ::sin(meanAnomalySun);
        double a3       = 0.3700*PI/180 * ::sin(meanAnomalySun);

        meanAnomalyMoon += evection - annual - a3;

        //
        // More correction factors:
        //  center  equation of the center correction
        //  a4      yet another error correction (???)
        //
        // TODO: Skip the equation of the center correction and solve Kepler's eqn?
        //
        double center = 6.2886*PI/180 * ::sin(meanAnomalyMoon);
        double a4 =     0.2140*PI/180 * ::sin(2 * meanAnomalyMoon);

        // Now find the moon's corrected longitude
        moonLongitude = meanLongitude + evection + center - annual + a4;

        //
        // And finally, find the variation, caused by the fact that the sun's
        // gravitational pull on the moon varies depending on which side of
        // the earth the moon is on
        //
        double variation = 0.6583*CalendarAstronomer::PI/180 * ::sin(2*(moonLongitude - sunLongitude));

        moonLongitude += variation;

        //
        // What we've calculated so far is the moon's longitude in the plane
        // of its own orbit.  Now map to the ecliptic to get the latitude
        // and longitude.  First we need to find the longitude of the ascending
        // node, the position on the ecliptic where it is crossed by the moon's
        // orbit as it crosses from the southern to the northern hemisphere.
        //
        double nodeLongitude = norm2PI(moonN0 - 0.0529539*PI/180 * day);

        nodeLongitude -= 0.16*PI/180 * ::sin(meanAnomalySun);

        double y = ::sin(moonLongitude - nodeLongitude);
        double x = cos(moonLongitude - nodeLongitude);

        moonEclipLong = ::atan2(y*cos(moonI), x) + nodeLongitude;
        double moonEclipLat = ::asin(y * ::sin(moonI));

        eclipticToEquatorial(moonPosition, moonEclipLong, moonEclipLat);
        moonPositionSet = TRUE;
    }
    return moonPosition;
}

/**
 * The "age" of the moon at the time specified in this object.
 * This is really the angle between the
 * current ecliptic longitudes of the sun and the moon,
 * measured in radians.
 *
 * @see #getMoonPhase
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
double CalendarAstronomer::getMoonAge() {
    // See page 147 of "Practical Astronomy with your Calculator",
    // by Peter Duffet-Smith, for details on the algorithm.
    //
    // Force the moon's position to be calculated.  We're going to use
    // some the intermediate results cached during that calculation.
    //
    // Currently, the only client is IslamicCalendar. All it cares
    // about is that the method returns new moon (0) and full moon (PI)
    // at the correct date & time, and otherwise that the returned value
    // is monotonically increasing from 0 to PI for the range new moon date
    // to full moon date, and monotonically increasing from PI to just under
    // 2*PI for the range full moon date to just before next new moon date.

    if (fTime >= newMoonDatesFirst && fTime < newMoonDatesLast) {
        int32_t offset = (int32_t)(((double)kNewMoonDatesCount)*(fTime - newMoonDatesFirst)/newMoonDatesRange);
        const int32_t * newMoonDatesPtr = newMoonDates + offset; // approximate starting position
        int32_t curTime = (int32_t)(fTime/10000.0);
        while (curTime < *newMoonDatesPtr) {
            newMoonDatesPtr--;
        }
        while (curTime >= *(newMoonDatesPtr+1)) {
            newMoonDatesPtr++;
        }
        offset = newMoonDatesPtr - newMoonDates;
        int32_t fullMoonDate = (*newMoonDatesPtr + *(newMoonDatesPtr+1))/2 + fullMoonAdjustmts[offset];
        if (curTime < fullMoonDate) {
            return PI*((double)(curTime - *newMoonDatesPtr))/((double)(fullMoonDate - *newMoonDatesPtr));
        }
        return PI + PI*((double)(curTime - fullMoonDate))/((double)(*(newMoonDatesPtr+1) - fullMoonDate));
    }

    getMoonPosition();

    return norm2PI(moonEclipLong - sunLongitude);
}

/**
 * Calculate the phase of the moon at the time set in this object.
 * The returned phase is a <code>double</code> in the range
 * <code>0 <= phase < 1</code>, interpreted as follows:
 * <ul>
 * <li>0.00: New moon
 * <li>0.25: First quarter
 * <li>0.50: Full moon
 * <li>0.75: Last quarter
 * </ul>
 *
 * @see #getMoonAge
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
double CalendarAstronomer::getMoonPhase() {
    // See page 147 of "Practical Astronomy with your Calculator",
    // by Peter Duffet-Smith, for details on the algorithm.
    return 0.5 * (1 - cos(getMoonAge()));
}

/**
 * Constant representing a new moon.
 * For use with {@link #getMoonTime getMoonTime}
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
const CalendarAstronomer::MoonAge CalendarAstronomer::NEW_MOON() {
    return  CalendarAstronomer::MoonAge(0);
}

/**
 * Constant representing the moon's first quarter.
 * For use with {@link #getMoonTime getMoonTime}
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
/*const CalendarAstronomer::MoonAge CalendarAstronomer::FIRST_QUARTER() {
  return   CalendarAstronomer::MoonAge(CalendarAstronomer::PI/2);
}*/

/**
 * Constant representing a full moon.
 * For use with {@link #getMoonTime getMoonTime}
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
const CalendarAstronomer::MoonAge CalendarAstronomer::FULL_MOON() {
    return   CalendarAstronomer::MoonAge(CalendarAstronomer::PI);
}
/**
 * Constant representing the moon's last quarter.
 * For use with {@link #getMoonTime getMoonTime}
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */

class MoonTimeAngleFunc : public CalendarAstronomer::AngleFunc {
public:
    virtual ~MoonTimeAngleFunc();
    virtual double eval(CalendarAstronomer&a) { return a.getMoonAge(); }
};

MoonTimeAngleFunc::~MoonTimeAngleFunc() {}

/*const CalendarAstronomer::MoonAge CalendarAstronomer::LAST_QUARTER() {
  return  CalendarAstronomer::MoonAge((CalendarAstronomer::PI*3)/2);
}*/

/**
 * Find the next or previous time of a new moon if date is in the
 * range handled by this function (approx gregorian 1900-2100),
 * else return 0.
 * <p>
 * @param theTime   the time relative to which the function should find
 *                  the next or previous new moon
 * @param next      <tt>true</tt> if the next occurrance of the new moon
 *                  is desired, <tt>false</tt> for the previous occurrance.
 * @internal
 */
UDate CalendarAstronomer::getNewMoonTimeInRange(UDate theTime, UBool next)
{
    if (theTime < newMoonDatesFirst || theTime >= newMoonDatesLast) {
        return 0.0;
    }
    int32_t offset = (int32_t)(((double)kNewMoonDatesCount)*(theTime - newMoonDatesFirst)/newMoonDatesRange);
    const int32_t * newMoonDatesPtr = newMoonDates + offset; // approximate starting position
    int32_t curTime = (int32_t)(theTime/10000.0);
    while (curTime < *newMoonDatesPtr) {
        newMoonDatesPtr--;
    }
    while (curTime >= *(newMoonDatesPtr+1)) {
        newMoonDatesPtr++;
    }
    if (next) {
        newMoonDatesPtr++;
    }
    return 10000.0 * (UDate)(*newMoonDatesPtr);
}


/**
 * Find the next or previous time at which the Moon's ecliptic
 * longitude will have the desired value.
 * <p>
 * @param desired   The desired longitude.
 * @param next      <tt>true</tt> if the next occurrance of the phase
 *                  is desired, <tt>false</tt> for the previous occurrance.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
UDate CalendarAstronomer::getMoonTime(double desired, UBool next)
{
    // Currently, we only get here via a call from ChineseCalendar,
    // with desired == CalendarAstronomer::NEW_MOON().value
    if (desired == CalendarAstronomer::NEW_MOON().value) {
        UDate newMoonTime = CalendarAstronomer::getNewMoonTimeInRange(fTime, next);
        if (newMoonTime != 0.0) {
            return newMoonTime;
        }
        // else fall through to the full calculation
    }
    
    MoonTimeAngleFunc func;
    return timeOfAngle( func,
                        desired,
                        SYNODIC_MONTH,
                        MINUTE_MS,
                        next);
}

/**
 * Find the next or previous time at which the moon will be in the
 * desired phase.
 * <p>
 * @param desired   The desired phase of the moon.
 * @param next      <tt>true</tt> if the next occurrance of the phase
 *                  is desired, <tt>false</tt> for the previous occurrance.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
UDate CalendarAstronomer::getMoonTime(const CalendarAstronomer::MoonAge& desired, UBool next) {
    // Currently, the only client is ChineseCalendar, which calls
    // this with desired == CalendarAstronomer::NEW_MOON()
    return getMoonTime(desired.value, next);
}

class MoonRiseSetCoordFunc : public CalendarAstronomer::CoordFunc {
public:
    virtual ~MoonRiseSetCoordFunc();
    virtual void eval(CalendarAstronomer::Equatorial& result, CalendarAstronomer&a) { result = a.getMoonPosition(); }
};

MoonRiseSetCoordFunc::~MoonRiseSetCoordFunc() {}

/**
 * Returns the time (GMT) of sunrise or sunset on the local date to which
 * this calendar is currently set.
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
UDate CalendarAstronomer::getMoonRiseSet(UBool rise)
{
    MoonRiseSetCoordFunc func;
    return riseOrSet(func,
                     rise,
                     .533 * DEG_RAD,        // Angular Diameter
                     34 /60.0 * DEG_RAD,    // Refraction correction
                     MINUTE_MS);            // Desired accuracy
}

//-------------------------------------------------------------------------
// Interpolation methods for finding the time at which a given event occurs
//-------------------------------------------------------------------------

UDate CalendarAstronomer::timeOfAngle(AngleFunc& func, double desired,
                                      double periodDays, double epsilon, UBool next)
{
    // Find the value of the function at the current time
    double lastAngle = func.eval(*this);

    // Find out how far we are from the desired angle
    double deltaAngle = norm2PI(desired - lastAngle) ;

    // Using the average period, estimate the next (or previous) time at
    // which the desired angle occurs.
    double deltaT =  (deltaAngle + (next ? 0.0 : - CalendarAstronomer_PI2 )) * (periodDays*DAY_MS) / CalendarAstronomer_PI2;

    double lastDeltaT = deltaT; // Liu
    UDate startTime = fTime; // Liu

    setTime(fTime + uprv_ceil(deltaT));

    // Now iterate until we get the error below epsilon.  Throughout
    // this loop we use normPI to get values in the range -Pi to Pi,
    // since we're using them as correction factors rather than absolute angles.
    do {
        // Evaluate the function at the time we've estimated
        double angle = func.eval(*this);

        // Find the # of milliseconds per radian at this point on the curve
        double factor = uprv_fabs(deltaT / normPI(angle-lastAngle));

        // Correct the time estimate based on how far off the angle is
        deltaT = normPI(desired - angle) * factor;

        // HACK:
        //
        // If abs(deltaT) begins to diverge we need to quit this loop.
        // This only appears to happen when attempting to locate, for
        // example, a new moon on the day of the new moon.  E.g.:
        //
        // This result is correct:
        // newMoon(7508(Mon Jul 23 00:00:00 CST 1990,false))=
        //   Sun Jul 22 10:57:41 CST 1990
        //
        // But attempting to make the same call a day earlier causes deltaT
        // to diverge:
        // CalendarAstronomer.timeOfAngle() diverging: 1.348508727575625E9 ->
        //   1.3649828540224032E9
        // newMoon(7507(Sun Jul 22 00:00:00 CST 1990,false))=
        //   Sun Jul 08 13:56:15 CST 1990
        //
        // As a temporary solution, we catch this specific condition and
        // adjust our start time by one eighth period days (either forward
        // or backward) and try again.
        // Liu 11/9/00
        if (uprv_fabs(deltaT) > uprv_fabs(lastDeltaT)) {
            double delta = uprv_ceil (periodDays * DAY_MS / 8.0);
            setTime(startTime + (next ? delta : -delta));
            return timeOfAngle(func, desired, periodDays, epsilon, next);
        }

        lastDeltaT = deltaT;
        lastAngle = angle;

        setTime(fTime + uprv_ceil(deltaT));
    }
    while (uprv_fabs(deltaT) > epsilon);

    return fTime;
}

UDate CalendarAstronomer::riseOrSet(CoordFunc& func, UBool rise,
                                    double diameter, double refraction,
                                    double epsilon)
{
    Equatorial pos;
    double      tanL   = ::tan(fLatitude);
    double     deltaT = 0;
    int32_t         count = 0;

    //
    // Calculate the object's position at the current time, then use that
    // position to calculate the time of rising or setting.  The position
    // will be different at that time, so iterate until the error is allowable.
    //
    U_DEBUG_ASTRO_MSG(("setup rise=%s, dia=%.3lf, ref=%.3lf, eps=%.3lf\n",
        rise?"T":"F", diameter, refraction, epsilon));
    do {
        // See "Practical Astronomy With Your Calculator, section 33.
        func.eval(pos, *this);
        double angle = ::acos(-tanL * ::tan(pos.declination));
        double lst = ((rise ? CalendarAstronomer_PI2-angle : angle) + pos.ascension ) * 24 / CalendarAstronomer_PI2;

        // Convert from LST to Universal Time.
        UDate newTime = lstToUT( lst );

        deltaT = newTime - fTime;
        setTime(newTime);
        U_DEBUG_ASTRO_MSG(("%d] dT=%.3lf, angle=%.3lf, lst=%.3lf,   A=%.3lf/D=%.3lf\n",
            count, deltaT, angle, lst, pos.ascension, pos.declination));
    }
    while (++ count < 5 && uprv_fabs(deltaT) > epsilon);

    // Calculate the correction due to refraction and the object's angular diameter
    double cosD  = ::cos(pos.declination);
    double psi   = ::acos(sin(fLatitude) / cosD);
    double x     = diameter / 2 + refraction;
    double y     = ::asin(sin(x) / ::sin(psi));
    long  delta  = (long)((240 * y * RAD_DEG / cosD)*SECOND_MS);

    return fTime + (rise ? -delta : delta);
}
/**
 * Return the obliquity of the ecliptic (the angle between the ecliptic
 * and the earth's equator) at the current time.  This varies due to
 * the precession of the earth's axis.
 *
 * @return  the obliquity of the ecliptic relative to the equator,
 *          measured in radians.
 */
double CalendarAstronomer::eclipticObliquity() {
    if (isINVALID(eclipObliquity)) {
        const double epoch = 2451545.0;     // 2000 AD, January 1.5

        double T = (getJulianDay() - epoch) / 36525;

        eclipObliquity = 23.439292
            - 46.815/3600 * T
            - 0.0006/3600 * T*T
            + 0.00181/3600 * T*T*T;

        eclipObliquity *= DEG_RAD;
    }
    return eclipObliquity;
}


//-------------------------------------------------------------------------
// Private data
//-------------------------------------------------------------------------
void CalendarAstronomer::clearCache() {
    const double INVALID = uprv_getNaN();

    julianDay       = INVALID;
    julianCentury   = INVALID;
    sunLongitude    = INVALID;
    meanAnomalySun  = INVALID;
    moonLongitude   = INVALID;
    moonEclipLong   = INVALID;
    meanAnomalyMoon = INVALID;
    eclipObliquity  = INVALID;
    siderealTime    = INVALID;
    siderealT0      = INVALID;
    moonPositionSet = FALSE;
}

//private static void out(String s) {
//    System.out.println(s);
//}

//private static String deg(double rad) {
//    return Double.toString(rad * RAD_DEG);
//}

//private static String hours(long ms) {
//    return Double.toString((double)ms / HOUR_MS) + " hours";
//}

/**
 * @internal
 * @deprecated ICU 2.4. This class may be removed or modified.
 */
/*UDate CalendarAstronomer::local(UDate localMillis) {
  // TODO - srl ?
  TimeZone *tz = TimeZone::createDefault();
  int32_t rawOffset;
  int32_t dstOffset;
  UErrorCode status = U_ZERO_ERROR;
  tz->getOffset(localMillis, TRUE, rawOffset, dstOffset, status);
  delete tz;
  return localMillis - rawOffset;
}*/

// Debugging functions
UnicodeString CalendarAstronomer::Ecliptic::toString() const
{
#ifdef U_DEBUG_ASTRO
    char tmp[800];
    sprintf(tmp, "[%.5f,%.5f]", longitude*RAD_DEG, latitude*RAD_DEG);
    return UnicodeString(tmp, "");
#else
    return UnicodeString();
#endif
}

UnicodeString CalendarAstronomer::Equatorial::toString() const
{
#ifdef U_DEBUG_ASTRO
    char tmp[400];
    sprintf(tmp, "%f,%f",
        (ascension*RAD_DEG), (declination*RAD_DEG));
    return UnicodeString(tmp, "");
#else
    return UnicodeString();
#endif
}

UnicodeString CalendarAstronomer::Horizon::toString() const
{
#ifdef U_DEBUG_ASTRO
    char tmp[800];
    sprintf(tmp, "[%.5f,%.5f]", altitude*RAD_DEG, azimuth*RAD_DEG);
    return UnicodeString(tmp, "");
#else
    return UnicodeString();
#endif
}


//  static private String radToHms(double angle) {
//    int hrs = (int) (angle*RAD_HOUR);
//    int min = (int)((angle*RAD_HOUR - hrs) * 60);
//    int sec = (int)((angle*RAD_HOUR - hrs - min/60.0) * 3600);

//    return Integer.toString(hrs) + "h" + min + "m" + sec + "s";
//  }

//  static private String radToDms(double angle) {
//    int deg = (int) (angle*RAD_DEG);
//    int min = (int)((angle*RAD_DEG - deg) * 60);
//    int sec = (int)((angle*RAD_DEG - deg - min/60.0) * 3600);

//    return Integer.toString(deg) + "\u00b0" + min + "'" + sec + "\"";
//  }

// =============== Calendar Cache ================

void CalendarCache::createCache(CalendarCache** cache, UErrorCode& status) {
    ucln_i18n_registerCleanup(UCLN_I18N_ASTRO_CALENDAR, calendar_astro_cleanup);
    if(cache == NULL) {
        status = U_MEMORY_ALLOCATION_ERROR;
    } else {
        *cache = new CalendarCache(32, status);
        if(U_FAILURE(status)) {
            delete *cache;
            *cache = NULL;
        }
    }
}

int32_t CalendarCache::get(CalendarCache** cache, int32_t key, UErrorCode &status) {
    int32_t res;

    if(U_FAILURE(status)) {
        return 0;
    }
    umtx_lock(ccLock());

    if(*cache == NULL) {
        createCache(cache, status);
        if(U_FAILURE(status)) {
            umtx_unlock(ccLock());
            return 0;
        }
    }

    res = uhash_igeti((*cache)->fTable, key);
    U_DEBUG_ASTRO_MSG(("%p: GET: [%d] == %d\n", (*cache)->fTable, key, res));

    umtx_unlock(ccLock());
    return res;
}

void CalendarCache::put(CalendarCache** cache, int32_t key, int32_t value, UErrorCode &status) {
    if(U_FAILURE(status)) {
        return;
    }
    umtx_lock(ccLock());

    if(*cache == NULL) {
        createCache(cache, status);
        if(U_FAILURE(status)) {
            umtx_unlock(ccLock());
            return;
        }
    }

    uhash_iputi((*cache)->fTable, key, value, &status);
    U_DEBUG_ASTRO_MSG(("%p: PUT: [%d] := %d\n", (*cache)->fTable, key, value));

    umtx_unlock(ccLock());
}

CalendarCache::CalendarCache(int32_t size, UErrorCode &status) {
    fTable = uhash_openSize(uhash_hashLong, uhash_compareLong, NULL, size, &status);
    U_DEBUG_ASTRO_MSG(("%p: Opening.\n", fTable));
}

CalendarCache::~CalendarCache() {
    if(fTable != NULL) {
        U_DEBUG_ASTRO_MSG(("%p: Closing.\n", fTable));
        uhash_close(fTable);
    }
}

U_NAMESPACE_END

#endif //  !UCONFIG_NO_FORMATTING

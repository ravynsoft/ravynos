/*

Copyright (c) 2007-2008  Michael G Schwern

This software originally derived from Paul Sheer's pivotal_gmtime_r.c.

The MIT License:

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/


/*
 *   This thing all things devours:
 *   Birds, beasts, trees, flowers;
 *   Gnaws iron, bites steel;
 *   Grinds hard stones to meal;
 *   Slays king, ruins town,
 *   And beats high mountain down."
 *
 * Poor Bilbo sat in the dark thinking of all the horrible names of all the
 * giants and ogres he had ever heard told of in tales, but not one of them had
 * done all these things. He had a feeling that the answer was quite different
 * and that he ought to know it, but he could not think of it. He began to get
 * frightened, and that is bad for thinking. Gollum began to get out of his
 * boat. He flapped into the water and paddled to the bank; Bilbo could see his
 * eyes coming towards him. His tongue seemed to stick in his mouth; he wanted
 * to shout out: "Give me more time! Give me time!" But all that came out with
 * a sudden squeal was:
 *
 * "Time! Time!"
 *
 * Bilbo was saved by pure luck. For that of course was the answer.
 *
 *     [p.84 of _The Hobbit_: "Riddles in the Dark"]
 *
*/

/*

Programmers who have available to them 64-bit time values as a 'long
long' type can use localtime64_r() and gmtime64_r() which correctly
converts the time even on 32-bit systems. Whether you have 64-bit time
values will depend on the operating system.

Perl_localtime64_r() is a 64-bit equivalent of localtime_r().

Perl_gmtime64_r() is a 64-bit equivalent of gmtime_r().

*/

#include "EXTERN.h"
#define PERL_IN_TIME64_C
#include "perl.h"
#include "time64.h"

static const char days_in_month[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

static const short julian_days_by_month[2][12] = {
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};

static const short length_of_year[2] = { 365, 366 };

/* Number of days in a 400 year Gregorian cycle */
static const Year years_in_gregorian_cycle = 400;
static const int days_in_gregorian_cycle  = (365 * 400) + 100 - 4 + 1;

/* 28 year calendar cycle between 2010 and 2037 */
#define SOLAR_CYCLE_LENGTH 28
static const short safe_years[SOLAR_CYCLE_LENGTH] = {
    2016, 2017, 2018, 2019,
    2020, 2021, 2022, 2023,
    2024, 2025, 2026, 2027,
    2028, 2029, 2030, 2031,
    2032, 2033, 2034, 2035,
    2036, 2037, 2010, 2011,
    2012, 2013, 2014, 2015
};

/* Let's assume people are going to be looking for dates in the future.
   Let's provide some cheats so you can skip ahead.
   This has a 4x speed boost when near 2008.
*/
/* Number of days since epoch on Jan 1st, 2008 GMT */
#define CHEAT_DAYS  (1199145600 / 24 / 60 / 60)
#define CHEAT_YEARS 108

#define IS_LEAP(n)	((!(((n) + 1900) % 400) || (!(((n) + 1900) % 4) && (((n) + 1900) % 100))) != 0)
#undef WRAP /* some <termios.h> define this */
#define WRAP(a,b,m)	((a) = ((a) <  0  ) ? ((b)--, (a) + (m)) : (a))

#ifdef USE_SYSTEM_LOCALTIME
#    define SHOULD_USE_SYSTEM_LOCALTIME(a)  (       \
    (a) <= SYSTEM_LOCALTIME_MAX &&              \
    (a) >= SYSTEM_LOCALTIME_MIN                 \
)
#else
#    define SHOULD_USE_SYSTEM_LOCALTIME(a)      (0)
#endif

#ifdef USE_SYSTEM_GMTIME
#    define SHOULD_USE_SYSTEM_GMTIME(a)     (       \
    (a) <= SYSTEM_GMTIME_MAX    &&              \
    (a) >= SYSTEM_GMTIME_MIN                    \
)
#else
#    define SHOULD_USE_SYSTEM_GMTIME(a)         (0)
#endif

/* Multi varadic macros are a C99 thing, alas */
#ifdef TIME_64_DEBUG
#    define TIME64_TRACE(format) (fprintf(stderr, format))
#    define TIME64_TRACE1(format, var1)    (fprintf(stderr, format, var1))
#    define TIME64_TRACE2(format, var1, var2)    (fprintf(stderr, format, var1, var2))
#    define TIME64_TRACE3(format, var1, var2, var3)    (fprintf(stderr, format, var1, var2, var3))
#else
#    define TIME64_TRACE(format) ((void)0)
#    define TIME64_TRACE1(format, var1) ((void)0)
#    define TIME64_TRACE2(format, var1, var2) ((void)0)
#    define TIME64_TRACE3(format, var1, var2, var3) ((void)0)
#endif

static int S_is_exception_century(Year year)
{
    const int is_exception = ((year % 100 == 0) && !(year % 400 == 0));
    TIME64_TRACE1("# is_exception_century: %s\n", is_exception ? "yes" : "no");

    return(is_exception);
}


static Time64_T S_timegm64(const struct TM *date) {
    int      days    = 0;
    Time64_T seconds = 0;

    if( date->tm_year > 70 ) {
        Year year = 70;
        while( year < date->tm_year ) {
            days += length_of_year[IS_LEAP(year)];
            year++;
        }
    }
    else if ( date->tm_year < 70 ) {
        Year year = 69;
        do {
            days -= length_of_year[IS_LEAP(year)];
            year--;
        } while( year >= date->tm_year );
    }

    days += julian_days_by_month[IS_LEAP(date->tm_year)][date->tm_mon];
    days += date->tm_mday - 1;

    /* Avoid overflowing the days integer */
    seconds = days;
    seconds = seconds * 60 * 60 * 24;

    seconds += date->tm_hour * 60 * 60;
    seconds += date->tm_min * 60;
    seconds += date->tm_sec;

    return(seconds);
}


#ifdef DEBUGGING
static int S_check_tm(const struct TM *tm)
{
    /* Don't forget leap seconds */
    assert(tm->tm_sec >= 0);
    assert(tm->tm_sec <= 61);

    assert(tm->tm_min >= 0);
    assert(tm->tm_min <= 59);

    assert(tm->tm_hour >= 0);
    assert(tm->tm_hour <= 23);

    assert(tm->tm_mday >= 1);
    assert(tm->tm_mday <= days_in_month[IS_LEAP(tm->tm_year)][tm->tm_mon]);

    assert(tm->tm_mon  >= 0);
    assert(tm->tm_mon  <= 11);

    assert(tm->tm_wday >= 0);
    assert(tm->tm_wday <= 6);

    assert(tm->tm_yday >= 0);
    assert(tm->tm_yday <= length_of_year[IS_LEAP(tm->tm_year)]);

#ifdef HAS_TM_TM_GMTOFF
    assert(tm->tm_gmtoff >= -24 * 60 * 60);
    assert(tm->tm_gmtoff <=  24 * 60 * 60);
#endif

    return 1;
}
#endif


/* The exceptional centuries without leap years cause the cycle to
   shift by 16
*/
static Year S_cycle_offset(Year year)
{
    const Year start_year = 2000;
    Year year_diff  = year - start_year;
    Year exceptions;

    if( year > start_year )
        year_diff--;

    exceptions  = year_diff / 100;
    exceptions -= year_diff / 400;

    TIME64_TRACE3("# year: %lld, exceptions: %lld, year_diff: %lld\n",
          year, exceptions, year_diff);

    return exceptions * 16;
}

/* For a given year after 2038, pick the latest possible matching
   year in the 28 year calendar cycle.

   A matching year...
   1) Starts on the same day of the week.
   2) Has the same leap year status.

   This is so the calendars match up.

   Also the previous year must match.  When doing Jan 1st you might
   wind up on Dec 31st the previous year when doing a -UTC time zone.

   Finally, the next year must have the same start day of week.  This
   is for Dec 31st with a +UTC time zone.
   It doesn't need the same leap year status since we only care about
   January 1st.
*/
static int S_safe_year(Year year)
{
    int safe_year;
    Year year_cycle = year + S_cycle_offset(year);

    /* Change non-leap xx00 years to an equivalent */
    if( S_is_exception_century(year) )
        year_cycle += 11;

    /* Also xx01 years, since the previous year will be wrong */
    if( S_is_exception_century(year - 1) )
        year_cycle += 17;

    year_cycle %= SOLAR_CYCLE_LENGTH;
    if( year_cycle < 0 )
        year_cycle = SOLAR_CYCLE_LENGTH + year_cycle;

    assert( year_cycle >= 0 );
    assert( year_cycle < SOLAR_CYCLE_LENGTH );
    safe_year = safe_years[year_cycle];

    assert(safe_year <= 2037 && safe_year >= 2010);

    TIME64_TRACE3("# year: %lld, year_cycle: %lld, safe_year: %d\n",
          year, year_cycle, safe_year);

    return safe_year;
}


static void S_copy_little_tm_to_big_TM(const struct tm *src, struct TM *dest) {
    assert(src);
    assert(dest);
#ifdef USE_TM64
    dest->tm_sec        = src->tm_sec;
    dest->tm_min        = src->tm_min;
    dest->tm_hour       = src->tm_hour;
    dest->tm_mday       = src->tm_mday;
    dest->tm_mon        = src->tm_mon;
    dest->tm_year       = (Year)src->tm_year;
    dest->tm_wday       = src->tm_wday;
    dest->tm_yday       = src->tm_yday;
    dest->tm_isdst      = src->tm_isdst;

#  ifdef HAS_TM_TM_GMTOFF
    dest->tm_gmtoff     = src->tm_gmtoff;
#  endif

#  ifdef HAS_TM_TM_ZONE
    dest->tm_zone       = src->tm_zone;
#  endif

#else
    /* They're the same type */
    memcpy(dest, src, sizeof(*dest));
#endif
}

struct TM *Perl_gmtime64_r (const Time64_T *in_time, struct TM *p)
{
    int v_tm_sec, v_tm_min, v_tm_hour, v_tm_mon, v_tm_wday;
    Time64_T v_tm_tday;
    int leap;
    Time64_T m;
    Time64_T time = *in_time;
    Year year = 70;
    dTHX;

    assert(p != NULL);

    /* Use the system gmtime() if time_t is small enough */
    if( SHOULD_USE_SYSTEM_GMTIME(*in_time) ) {
        time_t safe_time = (time_t)*in_time;
        struct tm safe_date;
        struct tm * result;

        GMTIME_LOCK;

        /* reentr.h will automatically replace this with a call to gmtime_r()
         * when appropriate */
        result = gmtime(&safe_time);

        assert(result != NULL);

#if defined(HAS_GMTIME_R) && defined(USE_REENTRANT_API)

        PERL_UNUSED_VAR(safe_date);
#else
        /* Here, no gmtime_r() and is a threaded perl where the result can be
         * overwritten by a call in another thread.  Copy to a safe place,
         * hopefully before another gmtime that isn't using the mutexes can
         * jump in and trash this result. */
        memcpy(&safe_date, result, sizeof(safe_date));
        result = &safe_date;
#endif
        GMTIME_UNLOCK;

        S_copy_little_tm_to_big_TM(result, p);
        assert(S_check_tm(p));

        return p;
    }

#ifdef HAS_TM_TM_GMTOFF
    p->tm_gmtoff = 0;
#endif
    p->tm_isdst  = 0;

#ifdef HAS_TM_TM_ZONE
    p->tm_zone   = "UTC";
#endif

    v_tm_sec  = (int)Perl_fmod(time, 60.0);
    time      = time >= 0 ? Perl_floor(time / 60.0) : Perl_ceil(time / 60.0);
    v_tm_min  = (int)Perl_fmod(time, 60.0);
    time      = time >= 0 ? Perl_floor(time / 60.0) : Perl_ceil(time / 60.0);
    v_tm_hour = (int)Perl_fmod(time, 24.0);
    time      = time >= 0 ? Perl_floor(time / 24.0) : Perl_ceil(time / 24.0);
    v_tm_tday = time;

    WRAP (v_tm_sec, v_tm_min, 60);
    WRAP (v_tm_min, v_tm_hour, 60);
    WRAP (v_tm_hour, v_tm_tday, 24);

    v_tm_wday = (int)Perl_fmod((v_tm_tday + 4.0), 7.0);
    if (v_tm_wday < 0)
        v_tm_wday += 7;
    m = v_tm_tday;

    if (m >= CHEAT_DAYS) {
        year = CHEAT_YEARS;
        m -= CHEAT_DAYS;
    }

    if (m >= 0) {
        /* Gregorian cycles, this is huge optimization for distant times */
        const int cycles = (int)Perl_floor(m / (Time64_T) days_in_gregorian_cycle);
        if( cycles ) {
            m -= (cycles * (Time64_T) days_in_gregorian_cycle);
            year += (cycles * years_in_gregorian_cycle);
        }

        /* Years */
        leap = IS_LEAP (year);
        while (m >= (Time64_T) length_of_year[leap]) {
            m -= (Time64_T) length_of_year[leap];
            year++;
            leap = IS_LEAP (year);
        }

        /* Months */
        v_tm_mon = 0;
        while (m >= (Time64_T) days_in_month[leap][v_tm_mon]) {
            m -= (Time64_T) days_in_month[leap][v_tm_mon];
            v_tm_mon++;
        }
    } else {
        int cycles;

        year--;

        /* Gregorian cycles */
        cycles = (int)Perl_ceil((m / (Time64_T) days_in_gregorian_cycle) + 1);
        if( cycles ) {
            m -= (cycles * (Time64_T) days_in_gregorian_cycle);
            year += (cycles * years_in_gregorian_cycle);
        }

        /* Years */
        leap = IS_LEAP (year);
        while (m < (Time64_T) -length_of_year[leap]) {
            m += (Time64_T) length_of_year[leap];
            year--;
            leap = IS_LEAP (year);
        }

        /* Months */
        v_tm_mon = 11;
        while (m < (Time64_T) -days_in_month[leap][v_tm_mon]) {
            m += (Time64_T) days_in_month[leap][v_tm_mon];
            v_tm_mon--;
        }
        m += (Time64_T) days_in_month[leap][v_tm_mon];
    }

    p->tm_year = year;
    if( p->tm_year != year ) {
#ifdef EOVERFLOW
        errno = EOVERFLOW;
#endif
        return NULL;
    }

    /* At this point m is less than a year so casting to an int is safe */
    p->tm_mday = (int) m + 1;
    p->tm_yday = julian_days_by_month[leap][v_tm_mon] + (int)m;
    p->tm_sec  = v_tm_sec;
    p->tm_min  = v_tm_min;
    p->tm_hour = v_tm_hour;
    p->tm_mon  = v_tm_mon;
    p->tm_wday = v_tm_wday;

    assert(S_check_tm(p));

    return p;
}


struct TM *Perl_localtime64_r (const Time64_T *time, struct TM *local_tm)
{
    time_t safe_time;
    struct tm safe_date;
    const struct tm * result;
    struct TM gm_tm;
    Year orig_year = 0; /* initialise to avoid spurious compiler warning */
    int month_diff;
    const bool use_system = SHOULD_USE_SYSTEM_LOCALTIME(*time);
    dTHX;

    assert(local_tm != NULL);

    /* Use the system localtime() if time_t is small enough */
    if (use_system) {
        safe_time = (time_t)*time;

        TIME64_TRACE1("Using system localtime for %lld\n", *time);
    }
    else {
        if (Perl_gmtime64_r(time, &gm_tm) == NULL) {
            TIME64_TRACE1("gmtime64_r returned null for %lld\n", *time);
            return NULL;
        }

        orig_year = gm_tm.tm_year;

        if (gm_tm.tm_year > (2037 - 1900) ||
            gm_tm.tm_year < (1970 - 1900)
           )
        {
            TIME64_TRACE1("Mapping tm_year %lld to safe_year\n",
                                                        (Year)gm_tm.tm_year);
            gm_tm.tm_year = S_safe_year((Year)(gm_tm.tm_year + 1900)) - 1900;
        }

        safe_time = (time_t)S_timegm64(&gm_tm);
    }

    LOCALTIME_LOCK;

    /* reentr.h will automatically replace this with a call to localtime_r()
     * when appropriate */
    result = localtime(&safe_time);

    if(UNLIKELY(result == NULL)) {
        LOCALTIME_UNLOCK;
        TIME64_TRACE1("localtime(%d) returned NULL\n", (int)safe_time);
        return NULL;
    }

#if ! defined(USE_REENTRANT_API) || defined(PERL_REENTR_USING_LOCALTIME_R)

    PERL_UNUSED_VAR(safe_date);

#else

    /* Here, would be using localtime_r() if it could, meaning there isn't one,
     * and is a threaded perl where the result can be overwritten by a call in
     * another thread.  Copy to a safe place, hopefully before another
     * localtime that isn't using the mutexes can jump in and trash this
     * result. */
    memcpy(&safe_date, result, sizeof(safe_date));
    result = &safe_date;

#endif

    LOCALTIME_UNLOCK;

    S_copy_little_tm_to_big_TM(result, local_tm);

    if (! use_system) {

        local_tm->tm_year = orig_year;
        if( local_tm->tm_year != orig_year ) {
            TIME64_TRACE2("tm_year overflow: tm_year %lld, orig_year %lld\n",
                  (Year)local_tm->tm_year, (Year)orig_year);

#ifdef EOVERFLOW
            errno = EOVERFLOW;
#endif
            return NULL;
        }

        month_diff = local_tm->tm_mon - gm_tm.tm_mon;

        /*  When localtime is Dec 31st previous year and
            gmtime is Jan 1st next year.
        */
        if( month_diff == 11 ) {
            local_tm->tm_year--;
        }

        /*  When localtime is Jan 1st, next year and
            gmtime is Dec 31st, previous year.
        */
        if( month_diff == -11 ) {
            local_tm->tm_year++;
        }

        /* GMT is Jan 1st, xx01 year, but localtime is still Dec 31st
           in a non-leap xx00.  There is one point in the cycle
           we can't account for which the safe xx00 year is a leap
           year.  So we need to correct for Dec 31st coming out as
           the 366th day of the year.
        */
        if( !IS_LEAP(local_tm->tm_year) && local_tm->tm_yday == 365 )
            local_tm->tm_yday--;

    }

    assert(S_check_tm(local_tm));

    return local_tm;
}

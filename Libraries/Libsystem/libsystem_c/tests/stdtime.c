#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <err.h>
#include <TargetConditionals.h>

#include <darwintest.h>

T_DECL(strptime_PR_24428248, "strptime parse day of year %j does not work correctly")
{
	struct tm percent_j = {0};
	struct tm standard = {0};
	strptime("2007 80 0 0 15", "%Y %j %H %M %S", &percent_j);
	strptime("2007-03-21 0:0:15", "%Y-%m-%d %H:%M:%S", &standard);
	time_t percent_j_out = mktime(&percent_j);
	time_t standard_out = mktime(&standard);
	T_EXPECT_EQ(percent_j_out, standard_out, NULL);
}

#if !TARGET_OS_BRIDGE
T_DECL(strptime_PR_5879606, "alloca(strlen(input)) in strptime(\"%Z\")")
{
    struct tm tm;
    time_t t = time(NULL);
    size_t s = 100000000;
    char *buf;

	localtime_r(&t, &tm);
	T_LOG("%s", asctime(&tm));
	T_ASSERT_NOTNULL(strptime("GMT", "%Z", &tm), "strptime GMT");
	T_LOG("%s", asctime(&tm));

	localtime_r(&t, &tm);
	T_ASSERT_NOTNULL(strptime("PST", "%Z", &tm), "strptime PST");
	T_LOG("%s", asctime(&tm));

	localtime_r(&t, &tm);
	T_ASSERT_NOTNULL(strptime("PDT", "%Z", &tm), "strptime PDT");
	T_LOG("%s", asctime(&tm));

	T_QUIET; T_ASSERT_NOTNULL((buf = malloc(s)), NULL);
	memset(buf, 'Z', s);
	buf[s - 1] = 0;
	T_ASSERT_NULL(strptime(buf, "%Z", &tm), NULL);
    free(buf);
}

T_DECL(strptime_PR_6882179, "date command fails with 'illegal time format'")
{
    struct tm tm;
    char buf[] = "Tue May 12 18:19:41 PDT 2009";

    T_ASSERT_NOTNULL(strptime(buf, "%a %b %d %T %Z %Y", &tm), NULL);

    T_EXPECT_EQ(tm.tm_sec, 0x29, NULL);
    T_EXPECT_EQ(tm.tm_min, 0x13, NULL);
    T_EXPECT_EQ(tm.tm_hour, 0x12, NULL);
    T_EXPECT_EQ(tm.tm_mday, 0xc, NULL);
    T_EXPECT_EQ(tm.tm_mon, 0x4, NULL);
    T_EXPECT_EQ(tm.tm_year, 0x6d, NULL);
    T_EXPECT_EQ(tm.tm_wday, 0x2, NULL);
    T_EXPECT_EQ(tm.tm_yday, 0x83, NULL);
}
#endif

T_DECL(strptime_lukemftp, "year parsing"){
	struct tm tm;
	setlocale(LC_ALL, "C");
	T_ASSERT_NOTNULL(strptime("20090505223446", "%Y%m%d%H%M%S", &tm), NULL);
}

T_DECL(strptime_five_digit_year, "strptime(%Y) with a 5 digit year")
{
	// POSIX conformance requires that %Y only use 4 characters, so use the
	// field width to change that for this test.
    char *timestr = "20080922T020000";
    struct tm tm;
    bzero(&tm, sizeof(tm));
    T_ASSERT_NOTNULL(strptime("10001", "%5Y", &tm), NULL);
    T_EXPECT_EQ(tm.tm_year, 10001 - 1900, NULL);
    T_ASSERT_NOTNULL(strptime(timestr, "%Y%m%dT%H%M%S", &tm), NULL);
}

T_DECL(strptime_PR_10842560, "strptime() with %W and %U")
{
    const struct test {
        const char *fmt;
        const char *str;
        const char *result;
    } test[] = {
        {"%Y:%U:%w:%H", "2012:6:0:23",	"Sun Feb 05 2012 23:00"},
        {"%Y:%w:%U:%H", "2012:0:6:23",	"Sun Feb 05 2012 23:00"},
        {"%U:%w:%Y:%H", "6:0:2012:23",	"Sun Feb 05 2012 23:00"},
        {"%U:%Y:%w:%H", "6:2012:0:23",	"Sun Feb 05 2012 23:00"},
        {"%w:%Y:%U:%H", "0:2012:6:23",	"Sun Feb 05 2012 23:00"},
        {"%w:%U:%Y:%H", "0:6:2012:23",	"Sun Feb 05 2012 23:00"},
        {"%Y:%V:%w:%H", "2012:6:0:23",	"Sun Feb 12 2012 23:00"},
        {"%Y:%w:%V:%H", "2012:0:6:23",	"Sun Feb 12 2012 23:00"},
        {"%V:%w:%Y:%H", "6:0:2012:23",	"Sun Feb 12 2012 23:00"},
        {"%V:%Y:%w:%H", "6:2012:0:23",	"Sun Feb 12 2012 23:00"},
        {"%w:%Y:%V:%H", "0:2012:6:23",	"Sun Feb 12 2012 23:00"},
        {"%w:%V:%Y:%H", "0:6:2012:23",	"Sun Feb 12 2012 23:00"},
        {"%Y:%W:%w:%H", "2012:6:0:23",	"Sun Feb 12 2012 23:00"},
        {"%Y:%w:%W:%H", "2012:0:6:23",	"Sun Feb 12 2012 23:00"},
        {"%W:%w:%Y:%H", "6:0:2012:23",	"Sun Feb 12 2012 23:00"},
        {"%W:%Y:%w:%H", "6:2012:0:23",	"Sun Feb 12 2012 23:00"},
        {"%w:%Y:%W:%H", "0:2012:6:23",	"Sun Feb 12 2012 23:00"},
        {"%w:%W:%Y:%H", "0:6:2012:23",	"Sun Feb 12 2012 23:00"},
        {"%Y:%U:%w:%H", "2011:6:0:23",	"Sun Feb 06 2011 23:00"},
        {"%Y:%U:%w:%H", "2010:6:0:23",	"Sun Feb 07 2010 23:00"},
        {"%Y:%U:%w:%H", "2009:6:0:23",	"Sun Feb 08 2009 23:00"},
        {"%Y:%U:%w:%H", "2008:6:0:23",	"Sun Feb 10 2008 23:00"},
        {"%Y:%U:%w:%H", "2007:6:0:23",	"Sun Feb 11 2007 23:00"},
        {"%Y:%U:%w:%H", "2006:6:0:23",	"Sun Feb 05 2006 23:00"},
        {"%Y:%V:%w:%H", "2011:6:0:23",	"Sun Feb 13 2011 23:00"},
        {"%Y:%V:%w:%H", "2010:6:0:23",	"Sun Feb 14 2010 23:00"},
        {"%Y:%V:%w:%H", "2009:6:0:23",	"Sun Feb 08 2009 23:00"},
        {"%Y:%V:%w:%H", "2008:6:0:23",	"Sun Feb 10 2008 23:00"},
        {"%Y:%V:%w:%H", "2007:6:0:23",	"Sun Feb 11 2007 23:00"},
        {"%Y:%V:%w:%H", "2006:6:0:23",	"Sun Feb 12 2006 23:00"},
        {"%Y:%W:%w:%H", "2011:6:0:23",	"Sun Feb 13 2011 23:00"},
        {"%Y:%W:%w:%H", "2010:6:0:23",	"Sun Feb 14 2010 23:00"},
        {"%Y:%W:%w:%H", "2009:6:0:23",	"Sun Feb 15 2009 23:00"},
        {"%Y:%W:%w:%H", "2008:6:0:23",	"Sun Feb 17 2008 23:00"},
        {"%Y:%W:%w:%H", "2007:6:0:23",	"Sun Feb 11 2007 23:00"},
        {"%Y:%W:%w:%H", "2006:6:0:23",	"Sun Feb 12 2006 23:00"},
        {NULL, NULL, NULL}
    };
    const struct test *tp;

    for(tp = test; tp->fmt; tp++){
        struct tm      Tm;
        char          *s;
        char Buf[100];

        memset(&Tm,0,sizeof(Tm));
        s = strptime(tp->str, tp->fmt, &Tm);
        T_QUIET; T_EXPECT_NOTNULL(s, "strptime() should return non-NULL");
        if (s) {
            strftime(Buf, sizeof(Buf), "%a %b %d %Y %R", &Tm);
            T_EXPECT_EQ_STR(Buf, tp->result, "%s | %s", tp->fmt, tp->str);
        }
    }
}

#if !TARGET_OS_BRIDGE
T_DECL(strptime_asctime, "strptime->asctime")
{
    char *test[] = {
        "Sun,  6 Apr 2003 03:30:00 -0500",
        "Sun,  6 Apr 2003 04:30:00 -0500",
        "Sun,  6 Apr 2003 05:30:00 -0500",
        "Sun,  6 Apr 2003 06:30:00 -0500",
        "Wed, 17 Sep 2003 13:30:00 -0500",
        "Sun, 26 Oct 2003 03:30:00 -0500",
        "Sun, 26 Oct 2003 04:30:00 -0500",
        "Sun, 26 Oct 2003 05:30:00 -0500",
        "Sun, 26 Oct 2003 06:30:00 -0500",
        NULL
    };

    char *result[] = {
        "Sun Apr  6 00:30:00 2003\n",
        "Sun Apr  6 01:30:00 2003\n",
        "Sun Apr  6 03:30:00 2003\n",
        "Sun Apr  6 04:30:00 2003\n",
        "Wed Sep 17 11:30:00 2003\n",
        "Sun Oct 26 01:30:00 2003\n",
        "Sun Oct 26 01:30:00 2003\n",
        "Sun Oct 26 02:30:00 2003\n",
        "Sun Oct 26 03:30:00 2003\n",
        NULL
    };

    int i = 0;
    while (test[i]){
        struct tm tm = {0};
        strptime(test[i], "%a, %d %b %Y %H:%M:%S %z", &tm);
        T_EXPECT_EQ_STR(result[i], asctime(&tm), "%s", test[i]);
        i++;
    }
}
#endif

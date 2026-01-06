#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <darwintest.h>
#include <darwintest_utils.h>

//#include "../stdtime/getdate.c"

extern int getdate_err;

__unused static void
log_tm(struct tm *tm) {
	T_LOG("tm = {\n\t.tm_sec = %d, tm_min = %d, tm_hour = %d,\n\t.tm_mday = %d, .tm_mon = %d, .tm_year = %d,\n\t.tm_wday = %d, tm_yday = %d\n};",
			tm->tm_sec, tm->tm_min, tm->tm_hour, tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday, tm->tm_yday);
}

static char *
create_template_file(const char *name, const char *contents)
{
	T_SETUPBEGIN;
	char *template_path;
	asprintf(&template_path, "%s/%s", dt_tmpdir(), name);
	T_QUIET; T_WITH_ERRNO; T_ASSERT_NOTNULL(template_path, NULL);

	FILE *template_file = fopen(template_path, "w");
	T_QUIET; T_WITH_ERRNO; T_ASSERT_NOTNULL(template_file, NULL);
	fprintf(template_file, "%s", contents);
	fclose(template_file);
	T_SETUPEND;

	return template_path;
}

T_DECL(getdate3, "getdate() using \"%a\" with match")
{
	struct tm tm = {};
	char *remainder = strptime("Fri", "%a", &tm);
	T_EXPECT_NOTNULL(remainder, "strptime(\"Fri\", \"%%a\", &tm)");
	T_EXPECT_EQ(tm.tm_wday, 5, "Returned time should be on Friday");

	char *template_path = create_template_file("a3", "%a\n");
	setenv("DATEMSK", template_path, 1);
	free(template_path);

	getdate_err = 0;
	struct tm *ptr_tm = getdate("Fri");
	T_LOG("getdate(\"Fri\") -> Should indicate Friday");
	T_ASSERT_NOTNULL(ptr_tm, "getdate() returned NULL, error is %d", getdate_err);
	T_PASS("getdate returned properly, with the time as: %s", asctime (ptr_tm));
	T_EXPECT_EQ(ptr_tm->tm_wday, 5, "Returned time should be on Friday");
}

T_DECL(getdate4, "getdate() using \"%A\" with match")
{
	struct tm tm = {};
	char *remainder = strptime("FriDay", "%A", &tm);
	T_EXPECT_NOTNULL(remainder, "strptime(\"FriDay\", \"%%A\", &tm)");
	T_EXPECT_EQ(tm.tm_wday, 5, "Returned time should be on Friday");

	char *template_path = create_template_file("a4", "%A\n");
	setenv("DATEMSK", template_path, 1);
	free(template_path);

	getdate_err = 0;
	struct tm *ptr_tm = getdate("FriDay");
	T_LOG("getdate(\"FriDay\") -> Should indicate Friday");
	T_ASSERT_NOTNULL(ptr_tm, "getdate() returned NULL, error is %d", getdate_err);
	T_PASS("getdate returned properly, with the time as: %s", asctime (ptr_tm));
	T_EXPECT_EQ(ptr_tm->tm_wday, 5, "Returned time should be on Friday");
}

T_DECL(getdate15, "getdate() using \"%w\" with match")
{
	char *template_path = create_template_file("a4", "%w\n");
	setenv("DATEMSK", template_path, 1);
	free(template_path);

	getdate_err = 0;
	struct tm *ptr_tm = getdate("0");
	T_LOG("getdate(\"0\") -> Should indicate weekday 0");
	T_ASSERT_NOTNULL(ptr_tm, "getdate() returned NULL, error is %d", getdate_err);
	T_PASS("getdate returned properly, with the time as: %s", asctime (ptr_tm));
	T_EXPECT_EQ(ptr_tm->tm_wday, 0, "Returned time should be on weekday 0");

	getdate_err = 0;
	ptr_tm = getdate("6");
	T_LOG("getdate(\"6\") -> Should indicate weekday 6");
	T_ASSERT_NOTNULL(ptr_tm, "getdate() returned NULL, error is %d", getdate_err);
	T_PASS("getdate returned properly, with the time as: %s", asctime (ptr_tm));
	T_EXPECT_EQ(ptr_tm->tm_wday, 6, "Returned time should be on weekday 6");
}

T_DECL(getdate46, "getdate() using \"%%t\" without match")
{
	char *template_path = create_template_file("a46", "%t\n");
	setenv("DATEMSK", template_path, 1);
	free(template_path);

	getdate_err = 0;
	struct tm *ptr_tm = getdate("there'snotemplateforthis.");
	if (ptr_tm) {
		T_FAIL("getdate returned: %s", asctime (ptr_tm));
	}
	/* Now check for the getdate_err */
	T_EXPECT_EQ(getdate_err, 7, NULL);
}

T_DECL(getdate21, "getdate() using \"%D\" with match")
{
	struct tm tm = {};
	char *remainder = strptime("1/1/38", "%D", &tm);
	T_EXPECT_NOTNULL(remainder, "strptime(\"1/1/38\", \"%%D\", &tm)");

	char *template_path = create_template_file("a21", "%D\n");
	setenv("DATEMSK", template_path, 1);
	free(template_path);

	getdate_err = 0;
	struct tm *ptr_tm = getdate ("1/1/38");
	T_LOG("getdate(\"1/1/38\") -> Should indicate 2038!");
	T_ASSERT_NOTNULL(ptr_tm, "getdate() returned NULL, error is %d", getdate_err);
	T_PASS("getdate returned properly, with the time as: %s", asctime (ptr_tm));

	T_EXPECT_EQ(ptr_tm->tm_year, 2038 - 1900, NULL);
	T_EXPECT_EQ(ptr_tm->tm_mon, 0, NULL);
	T_EXPECT_EQ(ptr_tm->tm_mday, 1, NULL);
}

T_DECL(getdate53, "getdate() using \"%d+%m+%y %C\" with match")
{
	struct tm tm = {};
	char *remainder = strptime("15+05+20 20", "%d+%m+%y %C", &tm);
	T_EXPECT_NOTNULL(remainder, "strptime(\"15+05+20 20\", \"%%d+%%m+%%y %%C\", &tm)");

	char *template_path = create_template_file("a53", "%d+%m+%y %C\n");
	setenv("DATEMSK", template_path, 1);
	free(template_path);

	getdate_err = 0;
	struct tm *ptr_tm = getdate ("15+05+20 20");
	T_LOG("getdate(\"15+05+20 20\")");
	T_ASSERT_NOTNULL(ptr_tm, "getdate() returned NULL, error is %d", getdate_err);
	T_PASS("getdate returned properly, with the time as: %s", asctime (ptr_tm));

	T_EXPECT_EQ(ptr_tm->tm_year, 2020 - 1900, NULL);
	T_EXPECT_EQ(ptr_tm->tm_mon, 4, NULL);
	T_EXPECT_EQ(ptr_tm->tm_mday, 15, NULL);
}

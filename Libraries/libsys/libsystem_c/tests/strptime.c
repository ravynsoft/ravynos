#include <time.h>

#include <darwintest.h>
#include <darwintest_utils.h>

T_DECL(strptime_PR_27004626, "strptime() should fail when a %t doesn't match anything")
{
	struct tm tm;
	T_ASSERT_NULL(strptime("there'snotemplateforthis", "%t", &tm), NULL);
}

T_DECL(strptime_PR_29381762, "strptime() sets the tm_wday field incorrectly")
{
	time_t epoch = 0;
	struct tm t = *localtime(&epoch);

	T_LOG("2015-01-01 12:00:00 -> Thursday");
	(void)strptime("2015-01-01 12:00:00", "%F %T", &t);
	T_EXPECT_EQ(t.tm_wday, 4, NULL);

	T_LOG("2015-04-19 12:00:00 -> Sunday");
	(void)strptime("2015-04-19 12:00:00", "%F %T", &t);
	T_EXPECT_EQ(t.tm_wday, 0, NULL);

	T_LOG("2009-03-03 12:00:00 -> Tuesday");
	(void)strptime("2009-03-03 12:00:00", "%F %T", &t);
	T_EXPECT_EQ(t.tm_wday, 2, NULL);

	T_LOG("1990-02-15 12:00:00 -> Thursday");
	(void)strptime("1990-02-15 12:00:00", "%F %T", &t);
	T_EXPECT_EQ(t.tm_wday, 4, NULL);

	T_LOG("1993-03-02 12:00:00 -> Sunday");
	(void)strptime("1993-03-02 12:00:00", "%F %T", &t);
	T_EXPECT_EQ(t.tm_wday, 2, NULL);
}


T_DECL(strptime_PR_42669744_1, "strptime() with %%C, %%y and %%Y")
{
	struct tm tm;
	char *result;

	// %C%y combinations
	T_LOG("201, %%C%%y");
	result = strptime("201", "%C%y", &tm);
	T_QUIET; T_EXPECT_NOTNULL(result, "201, %%C%%y");
	T_EXPECT_EQ(tm.tm_year, 2001 - 1900, NULL);

	T_LOG("2010, %%C%%y");
	result = strptime("2010", "%C%y", &tm);
	T_QUIET; T_EXPECT_NOTNULL(result, "2010, %%C%%y");
	T_EXPECT_EQ(tm.tm_year, 2010 - 1900, NULL);

	T_LOG("20010, %%C%%y");
	result = strptime("20010", "%C%y", &tm);
	T_QUIET; T_EXPECT_NOTNULL(result, "20010, %%C%%y");
	T_EXPECT_EQ(tm.tm_year, 2001 - 1900, NULL);

	T_LOG("+2010, %%C%%y");
	result = strptime("+2010", "%C%y", &tm);
	T_QUIET; T_EXPECT_NOTNULL(result, "2010, %%C%%y");
	T_EXPECT_EQ(tm.tm_year, 201 - 1900, NULL);

	T_LOG("-20100, %%C%%y");
	result = strptime("-20100", "%C%y", &tm);
	T_QUIET; T_EXPECT_NOTNULL(result, "-20100, %%C%%y");
	T_EXPECT_EQ(tm.tm_year, -200 + 1 - 1900, NULL);

	T_LOG("-2-1, %%C%%y");
	result = strptime("-2-1", "%C%y", &tm);
	T_QUIET; T_EXPECT_NOTNULL(result, "-2-1, %%C%%y");
	T_EXPECT_EQ(tm.tm_year, -200 - 1 - 1900, NULL);

	T_LOG("-2+1, %%C%%y");
	result = strptime("-2+1", "%C%y", &tm);
	T_QUIET; T_EXPECT_NOTNULL(result, "-2+1, %%C%%y");
	T_EXPECT_EQ(tm.tm_year, -200 + 1 - 1900, NULL);

	// %Y combinations
	T_LOG("201, %%Y");
	result = strptime("201", "%Y", &tm);
	T_QUIET; T_EXPECT_NOTNULL(result, "201, %%Y");
	T_EXPECT_EQ(tm.tm_year, 201 - 1900, NULL);

	T_LOG("2001, %%Y");
	result = strptime("2001", "%Y", &tm);
	T_QUIET; T_EXPECT_NOTNULL(result, "2001, %%Y");
	T_EXPECT_EQ(tm.tm_year, 2001 - 1900, NULL);

	T_LOG("20010, %%Y");
	result = strptime("20010", "%Y", &tm);
	T_QUIET; T_EXPECT_NOTNULL(result, "20010, %%Y");
	T_EXPECT_EQ(tm.tm_year, 2001 - 1900, NULL);

	T_LOG("+2010, %%Y");
	result = strptime("+2010", "%Y", &tm);
	T_QUIET; T_EXPECT_NOTNULL(result, "+2010, %%Y");
	T_EXPECT_EQ(tm.tm_year, 201 - 1900, NULL);

	T_LOG("-2010, %%Y");
	result = strptime("-2010", "%Y", &tm);
	T_QUIET; T_EXPECT_NOTNULL(result, "-2010, %%Y");
	T_EXPECT_EQ(tm.tm_year, -201 - 1900, NULL);
}


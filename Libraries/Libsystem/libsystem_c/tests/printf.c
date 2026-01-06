#include <sys/types.h>
#include <sys/resource.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <os/assumes.h>

#include "darwintest.h"
#include "darwintest_utils.h"

static void crash_callback(const char *str) {
	T_PASS("Crashed with \"%s\"", str);
	T_END;
}

T_DECL(sprintf_percent_n, "Test of %n")
{
	char str[1024];
	int len, ret;

	char *fmt = "%010d%n";

	T_EXPECT_POSIX_SUCCESS((ret = snprintf(str, sizeof(str), fmt, 0, &len)), NULL);
	T_EXPECT_EQ(len, ret, NULL);

	char fmt_buf[32];
	strlcpy(fmt_buf, fmt, sizeof(fmt_buf));

	os_set_crash_callback(crash_callback);
	snprintf(str, sizeof(str), fmt_buf, 0, &len);
	T_ASSERT_FAIL("Should have crashed on dynamic %%n");
}

#if !TARGET_OS_IPHONE
#define STRSIZE (1024 * 1024 * 256)

T_DECL(printf_PR_30663523, "Test for PR-30663523",
		T_META_CHECK_LEAKS(NO))
{
	char *temp_path;
	asprintf(&temp_path, "%s/%s", dt_tmpdir(), "big_file");

	{
		char *x = calloc(1, 0x80000001);
		memset(x, 0x41, 0x80000001);

		FILE *f = fopen(temp_path, "w");
		int len = fprintf(f, "%s", x);
		T_EXPECT_EQ(len, EOF, "fprintf should return EOF when string is longer than INT_MAX");
		fclose(f);
	}

	{
		char *x = calloc(1, STRSIZE);
		memset(x, 0x41, STRSIZE - 1);

		FILE *f = fopen(temp_path, "w");
		int len = fprintf(f, "%s%s%s%s%s%s%s%s%s%s", x,x,x,x,x,x,x,x,x,x);
		T_EXPECT_EQ(len, EOF, "fprintf should return EOF when output string is longer than INT_MAX");
		fclose(f);
	}
}
#endif // !TARGET_OS_IPHONE

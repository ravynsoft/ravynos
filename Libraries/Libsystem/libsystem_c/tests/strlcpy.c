#include <string.h>

#include <darwintest.h>

T_DECL(strlcpy_PR_30745460, "Test return value of strlcpy(3)",
		T_META_CHECK_LEAKS(NO))
{
	char buf[1];
	T_EXPECT_EQ(strlcpy(buf, "text", 1), 4UL, NULL);
	T_EXPECT_EQ(strlcpy(NULL, "text", 0), 4UL, NULL);
}

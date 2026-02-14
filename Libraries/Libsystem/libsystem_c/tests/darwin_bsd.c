#include <darwintest.h>

#if !defined(DARWIN_API_AVAILABLE_20190830)
#define DARWIN_API_AVAILABLE_20190830
#endif

#include "../libdarwin/bsd.c"

static struct test_case {
	const char *args;
	const char *argname;
	const char *argvalue;
} test_cases[] = {
	{"-x -a b=3 y=42", "-a", ""},
	{"-x -a b=3 y=42", "b", "3"},
	{"-x -a b=2 ba=3 y=42", "b", "2"},
	{"-x -a ba=3 b=2 y=42", "b", "2"},
	{"-x -a b=2 ba=3 y=42", "ba", "3"},
	{"-x -a ba=3 b=2 y=42", "ba", "3"},
	{"-x -ab -aa y=42", "-a", NULL},
	{"-x b=96 y=42", "bx", NULL},
	{"-x ab=96 y=42", "a", NULL},
};

T_DECL(parse_boot_arg_value, "Parsing boot args")
{
	for (int i = 0; i < (int)(sizeof(test_cases)/sizeof(test_cases[0])); i++) {
		struct test_case *test_case = &test_cases[i];
		T_LOG("\"%s\": Looking for \"%s\", expecting \"%s\"",
				test_case->args, test_case->argname, test_case->argvalue);

		char *argbuff = strdup(test_case->args);

		char result[256] = "NOT_FOUND";
		bool found = _parse_boot_arg_value(argbuff, test_case->argname,
				result,sizeof(result));

		if (test_case->argvalue) {
			T_EXPECT_EQ(found, true, "Should find argument");
			T_EXPECT_EQ_STR(result, test_case->argvalue, "Should find correct result");
		} else {
			T_EXPECT_EQ(found, false, "Should not find argument");
		}

		free(argbuff);
	}
}

T_DECL(os_parse_boot_arg, "Getting boot args")
{
	int64_t value = 0;
	T_EXPECT_EQ(os_parse_boot_arg_int("notarealthing", &value), false, NULL);

	T_MAYFAIL;
	T_EXPECT_EQ(os_parse_boot_arg_int("debug", &value), true, NULL);
	T_EXPECT_GT(value, 0LL, "non-zero debug= value");

	char buf[64] = {};

	T_EXPECT_EQ(os_parse_boot_arg_string("notarealthing", buf, sizeof(buf)), false, NULL);

	T_MAYFAIL;
	T_EXPECT_EQ(os_parse_boot_arg_string("debug", buf, sizeof(buf)), true, NULL);
	T_EXPECT_GT(strlen(buf), 0UL, "non-empty debug= value");
}

#include <stdlib.h>

#include <darwintest.h>

T_DECL(setenv_getenv, "getenv returns value set by setenv")
{
	char *name = "foo";
	char *value = "bar";
	int setenv_rc = setenv(name, value, 0);
	T_EXPECT_EQ(0, setenv_rc, "setenv must succeed with 0 return code");
	char *getenv_result = getenv(name);
	T_EXPECT_EQ_STR(value, getenv_result, "getenv must return setenv argument");
}

T_DECL(setenv_overwrite, "getenv returns the latest setenv argument")
{
	char *name = "foo";
	char *first_value = "bar";
	char *second_value = "baz";
	int setenv_rc = 0;
	setenv_rc = setenv(name, first_value, 0);
	T_EXPECT_EQ(0, setenv_rc, "setenv must succeed with 0 return code");
	setenv_rc = setenv(name, second_value, 1);
	T_EXPECT_EQ(0, setenv_rc, "setenv must succeed with 0 return code");
	char *getenv_result = getenv(name);
	T_EXPECT_EQ_STR(second_value, getenv_result, "getenv must return the latest setenv argument");
}

T_DECL(setenv_dont_overwrite, "setenv respects overwrite")
{
	char *name = "foo";
	char *first_value = "bar";
	char *second_value = "baz";
	int setenv_rc = 0;
	setenv_rc = setenv(name, first_value, 0);
	T_EXPECT_EQ(0, setenv_rc, "setenv must succeed with 0 return code");
	setenv_rc = setenv(name, second_value, 0);
	T_EXPECT_EQ(0, setenv_rc, "setenv must succeed with 0 return code");
	char *getenv_result = getenv(name);
	T_EXPECT_EQ_STR(first_value, getenv_result, "the second setenv must not overwrite the first one");
}

/* There are tests for leading '=' in values because BSDs used to strip them off: rdar://problem/19342460 */

T_DECL(setenv_accepts_leading_eq_sign, "setenv accepts values starting with '='")
{
	char *name = "foo";
	char *value = "=bar";
	int setenv_rc = setenv(name, value, 0);
	T_EXPECT_EQ(0, setenv_rc, "setenv must succeed with 0 return code");
	char *getenv_result = getenv(name);
	T_EXPECT_EQ_STR(value, getenv_result, "getenv must return setenv argument");
}

T_DECL(setenv_accepts_leading_eq_sign_overwrite, "setenv accepts values starting with '=' when overwriting an existing value")
{
	char *name = "foo";
	char *first_value = "bar";
	char *second_value = "=baz";
	int setenv_rc = 0;
	setenv_rc = setenv(name, first_value, 0);
	T_EXPECT_EQ(0, setenv_rc, "setenv must succeed with 0 return code");
	setenv_rc = setenv(name, second_value, 1);
	T_EXPECT_EQ(0, setenv_rc, "setenv must succeed with 0 return code");
	char *getenv_result = getenv(name);
	T_EXPECT_EQ_STR(second_value, getenv_result, "getenv must return the latest setenv argument");
}

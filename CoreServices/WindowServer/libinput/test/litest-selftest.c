#include <config.h>

#include <sys/resource.h>
#include <sys/time.h>
#include <check.h>
#include <signal.h>

#include <valgrind/valgrind.h>

#include "litest.h"

START_TEST(litest_assert_trigger)
{
	litest_assert(1 == 2);
}
END_TEST

START_TEST(litest_assert_notrigger)
{
	litest_assert(1 == 1);
}
END_TEST

START_TEST(litest_assert_msg_trigger)
{
	litest_assert_msg(1 == 2, "1 is not 2\n");
}
END_TEST

START_TEST(litest_assert_msg_NULL_trigger)
{
	litest_assert_msg(1 == 2, NULL);
}
END_TEST

START_TEST(litest_assert_msg_notrigger)
{
	litest_assert_msg(1 == 1, "1 is not 2\n");
	litest_assert_msg(1 == 1, NULL);
}
END_TEST

START_TEST(litest_abort_msg_trigger)
{
	litest_abort_msg("message\n");
}
END_TEST

START_TEST(litest_abort_msg_NULL_trigger)
{
	litest_abort_msg(NULL);
}
END_TEST

START_TEST(litest_int_eq_trigger)
{
	int a = 10;
	int b = 20;
	litest_assert_int_eq(a, b);
}
END_TEST

START_TEST(litest_int_eq_notrigger)
{
	int a = 10;
	int b = 10;
	litest_assert_int_eq(a, b);
}
END_TEST

START_TEST(litest_int_ne_trigger)
{
	int a = 10;
	int b = 10;
	litest_assert_int_ne(a, b);
}
END_TEST

START_TEST(litest_int_ne_notrigger)
{
	int a = 10;
	int b = 20;
	litest_assert_int_ne(a, b);
}
END_TEST

START_TEST(litest_int_lt_trigger_eq)
{
	int a = 10;
	int b = 10;
	litest_assert_int_lt(a, b);
}
END_TEST

START_TEST(litest_int_lt_trigger_gt)
{
	int a = 11;
	int b = 10;
	litest_assert_int_lt(a, b);
}
END_TEST

START_TEST(litest_int_lt_notrigger)
{
	int a = 10;
	int b = 11;
	litest_assert_int_lt(a, b);
}
END_TEST

START_TEST(litest_int_le_trigger)
{
	int a = 11;
	int b = 10;
	litest_assert_int_le(a, b);
}
END_TEST

START_TEST(litest_int_le_notrigger)
{
	int a = 10;
	int b = 11;
	int c = 10;
	litest_assert_int_le(a, b);
	litest_assert_int_le(a, c);
}
END_TEST

START_TEST(litest_int_gt_trigger_eq)
{
	int a = 10;
	int b = 10;
	litest_assert_int_gt(a, b);
}
END_TEST

START_TEST(litest_int_gt_trigger_lt)
{
	int a = 9;
	int b = 10;
	litest_assert_int_gt(a, b);
}
END_TEST

START_TEST(litest_int_gt_notrigger)
{
	int a = 10;
	int b = 9;
	litest_assert_int_gt(a, b);
}
END_TEST

START_TEST(litest_int_ge_trigger)
{
	int a = 9;
	int b = 10;
	litest_assert_int_ge(a, b);
}
END_TEST

START_TEST(litest_int_ge_notrigger)
{
	int a = 10;
	int b = 9;
	int c = 10;
	litest_assert_int_ge(a, b);
	litest_assert_int_ge(a, c);
}
END_TEST

START_TEST(litest_ptr_eq_notrigger)
{
	int v = 10;
	int *a = &v;
	int *b = &v;
	int *c = NULL;
	int *d = NULL;

	litest_assert_ptr_eq(a, b);
	litest_assert_ptr_eq(c, d);
}
END_TEST

START_TEST(litest_ptr_eq_trigger)
{
	int v = 10;
	int v2 = 11;
	int *a = &v;
	int *b = &v2;

	litest_assert_ptr_eq(a, b);
}
END_TEST

START_TEST(litest_ptr_eq_trigger_NULL)
{
	int v = 10;
	int *a = &v;
	int *b = NULL;

	litest_assert_ptr_eq(a, b);
}
END_TEST

START_TEST(litest_ptr_eq_trigger_NULL2)
{
	int v = 10;
	int *a = &v;
	int *b = NULL;

	litest_assert_ptr_eq(b, a);
}
END_TEST

START_TEST(litest_ptr_ne_trigger)
{
	int v = 10;
	int *a = &v;
	int *b = &v;

	litest_assert_ptr_ne(a, b);
}
END_TEST

START_TEST(litest_ptr_ne_trigger_NULL)
{
	int *a = NULL;

	litest_assert_ptr_ne(a, NULL);
}
END_TEST

START_TEST(litest_ptr_ne_trigger_NULL2)
{
	int *a = NULL;

	litest_assert_ptr_ne(NULL, a);
}
END_TEST

START_TEST(litest_ptr_ne_notrigger)
{
	int v1 = 10;
	int v2 = 10;
	int *a = &v1;
	int *b = &v2;
	int *c = NULL;

	litest_assert_ptr_ne(a, b);
	litest_assert_ptr_ne(a, c);
	litest_assert_ptr_ne(c, b);
}
END_TEST

START_TEST(litest_ptr_null_notrigger)
{
	int *a = NULL;

	litest_assert_ptr_null(a);
	litest_assert_ptr_null(NULL);
}
END_TEST

START_TEST(litest_ptr_null_trigger)
{
	int v;
	int *a = &v;

	litest_assert_ptr_null(a);
}
END_TEST

START_TEST(litest_ptr_notnull_notrigger)
{
	int v;
	int *a = &v;

	litest_assert_ptr_notnull(a);
}
END_TEST

START_TEST(litest_ptr_notnull_trigger)
{
	int *a = NULL;

	litest_assert_ptr_notnull(a);
}
END_TEST

START_TEST(litest_ptr_notnull_trigger_NULL)
{
	litest_assert_ptr_notnull(NULL);
}
END_TEST

START_TEST(ck_double_eq_and_ne)
{
	ck_assert_double_eq(0.4,0.4);
	ck_assert_double_eq(0.4,0.4 + 1E-6);
	ck_assert_double_ne(0.4,0.4 + 1E-3);
}
END_TEST

START_TEST(ck_double_lt_gt)
{
	ck_assert_double_lt(12.0,13.0);
	ck_assert_double_gt(15.4,13.0);
	ck_assert_double_le(12.0,12.0);
	ck_assert_double_le(12.0,20.0);
	ck_assert_double_ge(12.0,12.0);
	ck_assert_double_ge(20.0,12.0);
}
END_TEST

START_TEST(ck_double_eq_fails)
{
	ck_assert_double_eq(0.41,0.4);
}
END_TEST

START_TEST(ck_double_ne_fails)
{
	ck_assert_double_ne(0.4 + 1E-7,0.4);
}
END_TEST

START_TEST(ck_double_lt_fails)
{
	ck_assert_double_lt(6,5);
}
END_TEST

START_TEST(ck_double_gt_fails)
{
	ck_assert_double_gt(5,6);
}
END_TEST

START_TEST(ck_double_le_fails)
{
	ck_assert_double_le(6,5);
}
END_TEST

START_TEST(ck_double_ge_fails)
{
	ck_assert_double_ge(5,6);
}
END_TEST

START_TEST(zalloc_overflow)
{
	zalloc((size_t)-1);
}
END_TEST

START_TEST(zalloc_max_size)
{
	/* Built-in alloc maximum */
	free(zalloc(1536 * 1024));
}
END_TEST

START_TEST(zalloc_too_large)
{
	zalloc(1536 * 1024 + 1);
}
END_TEST

static Suite *
litest_assert_macros_suite(void)
{
	TCase *tc;
	Suite *s;

	s = suite_create("litest:assert macros");
	tc = tcase_create("assert");
	tcase_add_test_raise_signal(tc, litest_assert_trigger, SIGABRT);
	tcase_add_test(tc, litest_assert_notrigger);
	tcase_add_test_raise_signal(tc, litest_assert_msg_trigger, SIGABRT);
	tcase_add_test_raise_signal(tc, litest_assert_msg_NULL_trigger, SIGABRT);
	tcase_add_test(tc, litest_assert_msg_notrigger);
	suite_add_tcase(s, tc);

	tc = tcase_create("abort");
	tcase_add_test_raise_signal(tc, litest_abort_msg_trigger, SIGABRT);
	tcase_add_test_raise_signal(tc, litest_abort_msg_NULL_trigger, SIGABRT);
	suite_add_tcase(s, tc);

	tc = tcase_create("int comparison ");
	tcase_add_test_raise_signal(tc, litest_int_eq_trigger, SIGABRT);
	tcase_add_test(tc, litest_int_eq_notrigger);
	tcase_add_test_raise_signal(tc, litest_int_ne_trigger, SIGABRT);
	tcase_add_test(tc, litest_int_ne_notrigger);
	tcase_add_test_raise_signal(tc, litest_int_le_trigger, SIGABRT);
	tcase_add_test(tc, litest_int_le_notrigger);
	tcase_add_test_raise_signal(tc, litest_int_lt_trigger_gt, SIGABRT);
	tcase_add_test_raise_signal(tc, litest_int_lt_trigger_eq, SIGABRT);
	tcase_add_test(tc, litest_int_lt_notrigger);
	tcase_add_test_raise_signal(tc, litest_int_ge_trigger, SIGABRT);
	tcase_add_test(tc, litest_int_ge_notrigger);
	tcase_add_test_raise_signal(tc, litest_int_gt_trigger_eq, SIGABRT);
	tcase_add_test_raise_signal(tc, litest_int_gt_trigger_lt, SIGABRT);
	tcase_add_test(tc, litest_int_gt_notrigger);
	suite_add_tcase(s, tc);

	tc = tcase_create("pointer comparison ");
	tcase_add_test_raise_signal(tc, litest_ptr_eq_trigger, SIGABRT);
	tcase_add_test_raise_signal(tc, litest_ptr_eq_trigger_NULL, SIGABRT);
	tcase_add_test_raise_signal(tc, litest_ptr_eq_trigger_NULL2, SIGABRT);
	tcase_add_test(tc, litest_ptr_eq_notrigger);
	tcase_add_test_raise_signal(tc, litest_ptr_ne_trigger, SIGABRT);
	tcase_add_test_raise_signal(tc, litest_ptr_ne_trigger_NULL, SIGABRT);
	tcase_add_test_raise_signal(tc, litest_ptr_ne_trigger_NULL2, SIGABRT);
	tcase_add_test(tc, litest_ptr_ne_notrigger);
	tcase_add_test_raise_signal(tc, litest_ptr_null_trigger, SIGABRT);
	tcase_add_test(tc, litest_ptr_null_notrigger);
	tcase_add_test_raise_signal(tc, litest_ptr_notnull_trigger, SIGABRT);
	tcase_add_test_raise_signal(tc, litest_ptr_notnull_trigger_NULL, SIGABRT);
	tcase_add_test(tc, litest_ptr_notnull_notrigger);
	suite_add_tcase(s, tc);

	tc = tcase_create("double comparison ");
	tcase_add_test(tc, ck_double_eq_and_ne);
	tcase_add_test(tc, ck_double_lt_gt);
	tcase_add_exit_test(tc, ck_double_eq_fails, 1);
	tcase_add_exit_test(tc, ck_double_ne_fails, 1);
	tcase_add_exit_test(tc, ck_double_lt_fails, 1);
	tcase_add_exit_test(tc, ck_double_gt_fails, 1);
	tcase_add_exit_test(tc, ck_double_le_fails, 1);
	tcase_add_exit_test(tc, ck_double_ge_fails, 1);
	suite_add_tcase(s, tc);

	tc = tcase_create("zalloc ");
	tcase_add_test(tc, zalloc_max_size);
	tcase_add_test_raise_signal(tc, zalloc_overflow, SIGABRT);
	tcase_add_test_raise_signal(tc, zalloc_too_large, SIGABRT);
	suite_add_tcase(s, tc);

	return s;
}

int
main (int argc, char **argv)
{
	const struct rlimit corelimit = { 0, 0 };
	int nfailed;
	Suite *s;
	SRunner *sr;

        /* when running under valgrind we're using nofork mode, so a signal
         * raised by a test will fail in valgrind. There's nothing to
         * memcheck here anyway, so just skip the valgrind test */
        if (RUNNING_ON_VALGRIND)
            return 77;

	if (setrlimit(RLIMIT_CORE, &corelimit) != 0)
		perror("WARNING: Core dumps not disabled");

	s = litest_assert_macros_suite();
        sr = srunner_create(s);

	srunner_run_all(sr, CK_ENV);
	nfailed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (nfailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

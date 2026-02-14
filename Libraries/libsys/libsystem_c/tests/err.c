#include <Block.h>
#include <darwintest.h>
#include <err.h>

#define ITERATIONS 100

T_DECL(err_multiple_exit_b, "Repeated set exit blocks doesn't leak copied blocks")
{
	int __block num = 0;
	for (int i = 0; i < ITERATIONS; ++i) {
		err_set_exit_b(^(int j) { num += j; });
	}
	err_set_exit_b(NULL);
	// Dummy expect is necessary to run leaks on this test.
	T_EXPECT_NULL(NULL, "DUMMY EXPECT");
}

T_DECL(err_multiple_exit, "Setting exit w/o block after setting exit with block doesn't leak copied block")
{
	int __block num = 0;
	err_set_exit_b(^(int j) { num += j; });
	err_set_exit(NULL);
	// Dummy expect is necessary to run leaks on this test.
	T_EXPECT_NULL(NULL, "DUMMY EXPECT");
}

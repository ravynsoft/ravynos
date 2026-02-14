#include <string.h>
#include <stdlib.h>

#include <darwintest.h>

T_DECL(timingsafe_bcmp, "tests for timingsafe_bcmp(3)")
{
	// empty
	T_ASSERT_EQ(0, timingsafe_bcmp(NULL, NULL, 0), NULL);
	T_ASSERT_EQ(0, timingsafe_bcmp("foo", "foo", 0), NULL);
	T_ASSERT_EQ(0, timingsafe_bcmp("foo", "bar", 0), NULL);

	// equal
	T_ASSERT_EQ(0, timingsafe_bcmp("foo", "foo", strlen("foo")), NULL);

	// unequal
	T_ASSERT_EQ(1, timingsafe_bcmp("foo", "bar", strlen("foo")), NULL);
	T_ASSERT_EQ(1, timingsafe_bcmp("foo", "goo", strlen("foo")), NULL);
	T_ASSERT_EQ(1, timingsafe_bcmp("foo", "fpo", strlen("foo")), NULL);
	T_ASSERT_EQ(1, timingsafe_bcmp("foo", "fop", strlen("foo")), NULL);

	// all possible bitwise differences
	int i;
	for (i = 1; i < 256; i += 1) {
		unsigned char a = 0;
		unsigned char b = (unsigned char)i;

		T_ASSERT_EQ(1, timingsafe_bcmp(&a, &b, sizeof(a)), NULL);
	}

	// large
	char buf[1024 * 16];
	arc4random_buf(buf, sizeof(buf));
	T_ASSERT_EQ(0, timingsafe_bcmp(buf, buf, sizeof(buf)), NULL);
	T_ASSERT_EQ(1, timingsafe_bcmp(buf, buf + 1, sizeof(buf) - 1), NULL);
	T_ASSERT_EQ(1, timingsafe_bcmp(buf, buf + 128, 128), NULL);

	memcpy(buf+128, buf, 128);
	T_ASSERT_EQ(0, timingsafe_bcmp(buf, buf + 128, 128), NULL);
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <darwintest.h>
#include "darwintest_utils.h"

#define IS_FLAG_SET(fp, flag) ((fp)->_flags & (flag))
#define BUFFER_SIZE(fp) ((fp)->_bf._size)

static FILE* test_setup(void)
{
	char* temp_dir;
	temp_dir = dt_tmpdir();
	T_LOG("Temp Dir: %s\n", temp_dir);

	char temp_path[128];
	snprintf(temp_path, sizeof(temp_path), "%s/%s", temp_dir, "test_file");
	T_LOG("Using Temporary File: %s\n", temp_path);
	FILE* fp = fopen(temp_path, "w");
	T_LOG("File opened: %d\n", fileno(fp));
	T_ASSERT_EQ(fileno(fp), 3, "file descriptor matches");
	// Need to prime the buffer
	fprintf(fp, "Test\n");

	return fp;
}

T_DECL(envbuf_PR_38637477_unbuf_all, "Forcing unbuffered through environment variable", T_META_ENVVAR("STDBUF=U"))
{
	FILE* fp = test_setup();

	T_EXPECT_TRUE(IS_FLAG_SET(fp, __SNBF), "unbuffered");
	T_EXPECT_FALSE(IS_FLAG_SET(fp, __SLBF), "not line buffered");
	T_EXPECT_EQ(BUFFER_SIZE(fp), 1, "buffer size 1");
}


T_DECL(envbuf_PR_38637477_line_all, "Forcing line buffering through environment variable", T_META_ENVVAR("STDBUF=L16"))
{
	FILE* fp = test_setup();

	T_EXPECT_FALSE(IS_FLAG_SET(fp, __SNBF), "not unbuffered");
	T_EXPECT_TRUE(IS_FLAG_SET(fp, __SLBF), "line buffered");
	T_EXPECT_EQ(BUFFER_SIZE(fp), 16, "buffer size 16");
}

T_DECL(envbuf_PR_38637477_full_all, "Forcing full buffering through environment variable", T_META_ENVVAR("STDBUF=F16"))
{
	FILE* fp = test_setup();

	T_EXPECT_FALSE(IS_FLAG_SET(fp, __SNBF), "not unbuffered");
	T_EXPECT_FALSE(IS_FLAG_SET(fp, __SLBF), "not line buffered");
	T_EXPECT_EQ(BUFFER_SIZE(fp), 16, "buffer size 16");
}

T_DECL(envbuf_PR_38637477_unbuf_stdout, "Forcing unbuffered through environment variable for stdout", T_META_ENVVAR("STDBUF1=U"))
{

	fprintf(stdout, "Test\n");

	T_EXPECT_TRUE(IS_FLAG_SET(stdout, __SNBF), "unbuffered");
	T_EXPECT_FALSE(IS_FLAG_SET(stdout, __SLBF), "not linebuffered");
	T_EXPECT_EQ(BUFFER_SIZE(stdout), 1, "buffer size 1");
}

T_DECL(envbuf_PR_38637477_line_stdout, "Forcing line buffering through environment variable for stdout", T_META_ENVVAR("STDBUF1=L32"))
{

	fprintf(stdout, "Test\n");

	T_EXPECT_FALSE(IS_FLAG_SET(stdout, __SNBF), "not unbuffered");
	T_EXPECT_TRUE(IS_FLAG_SET(stdout, __SLBF), "line buffered");
	T_EXPECT_EQ(BUFFER_SIZE(stdout), 32, "buffer size 32");
}

T_DECL(envbuf_PR_38637477_full_stdout, "Forcing full buffering through environment variable for stdout", T_META_ENVVAR("STDBUF1=F16"))
{

	fprintf(stdout, "Test\n");

	T_EXPECT_FALSE(IS_FLAG_SET(stdout, __SNBF), "not unbuffered");
	T_EXPECT_FALSE(IS_FLAG_SET(stdout, __SLBF), "not line buffered");
	T_EXPECT_EQ(BUFFER_SIZE(stdout), 16, "buffer size 16");
}

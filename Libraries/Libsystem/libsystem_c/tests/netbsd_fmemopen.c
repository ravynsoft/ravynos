/* $NetBSD: t_fmemopen.c,v 1.4 2013/10/19 17:45:00 christos Exp $ */

/*-
 * Copyright (c)2010 Takehiko NOZAKI,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <darwintest.h>

static const char *mode_rwa[] = {
    "r", "rb", "r+", "rb+", "r+b",
    "w", "wb", "w+", "wb+", "w+b",
    "a", "ab", "a+", "ab+", "a+b",
    NULL
};

static const char *mode_r[] = { "r", "rb", "r+", "rb+", "r+b", NULL };
static const char *mode_w[] = { "w", "wb", "w+", "wb+", "w+b", NULL };
static const char *mode_a[] = { "a", "ab", "a+", "ab+", "a+b", NULL };

static struct testcase {
	const char *s;
	off_t n;
} testcases[] = {
#define TESTSTR(s)	{ s, sizeof(s)-1 }
	TESTSTR("\0he quick brown fox jumps over the lazy dog"),
	TESTSTR("T\0e quick brown fox jumps over the lazy dog"),
	TESTSTR("Th\0 quick brown fox jumps over the lazy dog"),
	TESTSTR("The\0quick brown fox jumps over the lazy dog"),
	TESTSTR("The \0uick brown fox jumps over the lazy dog"),
	TESTSTR("The q\0ick brown fox jumps over the lazy dog"),
	TESTSTR("The qu\0ck brown fox jumps over the lazy dog"),
	TESTSTR("The qui\0k brown fox jumps over the lazy dog"),
	TESTSTR("The quic\0 brown fox jumps over the lazy dog"),
	TESTSTR("The quick\0brown fox jumps over the lazy dog"),
	TESTSTR("The quick \0rown fox jumps over the lazy dog"),
	TESTSTR("The quick b\0own fox jumps over the lazy dog"),
	TESTSTR("The quick br\0wn fox jumps over the lazy dog"),
	TESTSTR("The quick bro\0n fox jumps over the lazy dog"),
	TESTSTR("The quick brow\0 fox jumps over the lazy dog"),
	TESTSTR("The quick brown\0fox jumps over the lazy dog"),
	TESTSTR("The quick brown \0ox jumps over the lazy dog"),
	TESTSTR("The quick brown f\0x jumps over the lazy dog"),
	TESTSTR("The quick brown fo\0 jumps over the lazy dog"),
	TESTSTR("The quick brown fox\0jumps over the lazy dog"),
	TESTSTR("The quick brown fox \0umps over the lazy dog"),
	TESTSTR("The quick brown fox j\0mps over the lazy dog"),
	TESTSTR("The quick brown fox ju\0ps over the lazy dog"),
	TESTSTR("The quick brown fox jum\0s over the lazy dog"),
	TESTSTR("The quick brown fox jump\0 over the lazy dog"),
	TESTSTR("The quick brown fox jumps\0over the lazy dog"),
	TESTSTR("The quick brown fox jumps \0ver the lazy dog"),
	TESTSTR("The quick brown fox jumps o\0er the lazy dog"),
	TESTSTR("The quick brown fox jumps ov\0r the lazy dog"),
	TESTSTR("The quick brown fox jumps ove\0 the lazy dog"),
	TESTSTR("The quick brown fox jumps over\0the lazy dog"),
	TESTSTR("The quick brown fox jumps over \0he lazy dog"),
	TESTSTR("The quick brown fox jumps over t\0e lazy dog"),
	TESTSTR("The quick brown fox jumps over th\0 lazy dog"),
	TESTSTR("The quick brown fox jumps over the\0lazy dog"),
	TESTSTR("The quick brown fox jumps over the \0azy dog"),
	TESTSTR("The quick brown fox jumps over the l\0zy dog"),
	TESTSTR("The quick brown fox jumps over the la\0y dog"),
	TESTSTR("The quick brown fox jumps over the laz\0 dog"),
	TESTSTR("The quick brown fox jumps over the lazy\0dog"),
	TESTSTR("The quick brown fox jumps over the lazy \0og"),
	TESTSTR("The quick brown fox jumps over the lazy d\0g"),
	TESTSTR("The quick brown fox jumps over the lazy do\0"),
	TESTSTR("The quick brown fox jumps over the lazy dog"),
	{ NULL, 0 },
};

T_DECL(netbsd_fmemopen_test00, "")
{
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

	for (p = &mode_rwa[0]; *p != NULL; ++p) {
		fp = fmemopen(&buf[0], sizeof(buf), *p);
/*
 * Upon successful completion, fmemopen() shall return a pointer to the
 * object controlling the stream.
 */
		T_EXPECT_NOTNULL(fp, NULL);

		T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
	}
}

T_DECL(netbsd_fmemopen_test01, "")
{
	const char **p;
	const char *mode[] = {
	    "r+", "rb+", "r+b",
	    "w+", "wb+", "w+b",
	    "a+", "ab+", "a+b",
	    NULL
	};
	FILE *fp;

	for (p = &mode[0]; *p != NULL; ++p) {
/*
 * If a null pointer is specified as the buf argument, fmemopen() shall
 * allocate size bytes of memory as if by a call to malloc().
 */
		fp = fmemopen(NULL, BUFSIZ, *p);
		T_EXPECT_NOTNULL(fp, NULL);

/*
 * If buf is a null pointer, the initial position shall always be set
 * to the beginning of the buffer.
 */
		T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);

		T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
	}
}

T_DECL(netbsd_fmemopen_test02, "")
{
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

	for (p = &mode_r[0]; *p != NULL; ++p) {

		memset(&buf[0], 0x1, sizeof(buf));
		fp = fmemopen(&buf[0], sizeof(buf), *p);
		T_EXPECT_NOTNULL(fp, NULL);

/*
 * This position is initially set to either the beginning of the buffer
 * (for r and w modes)
 */
		T_EXPECT_EQ((unsigned char)buf[0], 0x1, NULL);
		T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);

/*
 * The stream also maintains the size of the current buffer contents.
 * For modes r and r+ the size is set to the value given by the size argument.
 */
#if !defined(__GLIBC__)
		T_EXPECT_POSIX_ZERO(fseeko(fp, (off_t)0, SEEK_END), NULL);
		T_EXPECT_EQ(ftello(fp), (off_t)sizeof(buf), NULL);
#endif
		T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
	}
}

T_DECL(netbsd_fmemopen_test03, "")
{
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;
 
	for (p = &mode_w[0]; *p != NULL; ++p) {

		memset(&buf[0], 0x1, sizeof(buf));
		fp = fmemopen(&buf[0], sizeof(buf), *p);
		T_EXPECT_NOTNULL(fp, NULL);

/*
 * This position is initially set to either the beginning of the buffer
 * (for r and w modes)
 */
		T_EXPECT_EQ(buf[0], '\0', NULL);
		T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);

/*
 * For modes w and w+ the initial size is zero
 */
		T_EXPECT_POSIX_ZERO(fseeko(fp, (off_t)0, SEEK_END), NULL);
		T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);

		T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
	}
}

T_DECL(netbsd_fmemopen_test04, "")
{
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

/*
 * or to the first null byte in the buffer (for a modes)
 */
	for (p = &mode_a[0]; *p != NULL; ++p) {

		memset(&buf[0], 0x1, sizeof(buf));
		fp = fmemopen(&buf[0], sizeof(buf), *p);
		T_EXPECT_NOTNULL(fp, NULL);

		T_EXPECT_EQ((unsigned char)buf[0], 0x1, NULL);

/* If no null byte is found in append mode,
 * the initial position is set to one byte after the end of the buffer.
 */
#if !defined(__GLIBC__)
		T_EXPECT_EQ(ftello(fp), (off_t)sizeof(buf), NULL);
#endif

/*
 * and for modes a and a+ the initial size is either the position of the
 * first null byte in the buffer or the value of the size argument
 * if no null byte is found.
 */
#if !defined(__GLIBC__)
		T_EXPECT_POSIX_ZERO(fseeko(fp, (off_t)0, SEEK_END), NULL);
		T_EXPECT_EQ(ftello(fp), (off_t)sizeof(buf), NULL);
#endif

		T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
	}
}

T_DECL(netbsd_fmemopen_test05, "")
{
	const char **p;
	FILE *fp;
	char buf[BUFSIZ];

	for (p = &mode_rwa[0]; *p != NULL; ++p) {
/*
 * Otherwise, a null pointer shall be returned, and errno shall be set
 * to indicate the error.
 */
		errno = 0;
		fp = fmemopen(NULL, (size_t)0, *p);
		T_EXPECT_NULL(fp, NULL);
		T_EXPECT_EQ(errno, EINVAL, NULL);

		errno = 0;
		fp = fmemopen((void *)&buf[0], 0, *p);
		T_EXPECT_NULL(fp, NULL);
		T_EXPECT_EQ(errno, EINVAL, NULL);
	}
}

T_DECL(netbsd_fmemopen_test06, "")
{
	const char **p;
	const char *mode[] = { "", " ", "???", NULL };
	FILE *fp;

	for (p = &mode[0]; *p != NULL; ++p) {
/*
 * The value of the mode argument is not valid.
 */
		fp = fmemopen(NULL, 1, *p);
		T_EXPECT_NULL(fp, NULL);
		T_EXPECT_EQ(errno, EINVAL, NULL);
	}
}

T_DECL(netbsd_fmemopen_test07, "")
{
#if !defined(__GLIBC__)
	const char **p;
	const char *mode[] = {
	    "r", "rb",
	    "w", "wb",
	    "a", "ab",
	    NULL
	};
	FILE *fp;

	for (p = &mode[0]; *p != NULL; ++p) {
/*
 * Because this feature is only useful when the stream is opened for updating
 * (because there is no way to get a pointer to the buffer) the fmemopen()
 * call may fail if the mode argument does not include a '+' . 
 */
		errno = 0;
		fp = fmemopen(NULL, 1, *p);
		T_EXPECT_NULL(fp, NULL);
		T_EXPECT_EQ(errno, EINVAL, NULL);
	}
#endif
}

T_DECL(netbsd_fmemopen_test08, "")
{
#if !defined(__GLIBC__)
	const char **p;
	const char *mode[] = {
	    "r+", "rb+", "r+b",
	    "w+", "wb+", "w+b",
	    "a+", "ab+", "a+b",
	    NULL
	};
	FILE *fp;

	for (p = &mode[0]; *p != NULL; ++p) {
/*
 * The buf argument is a null pointer and the allocation of a buffer of
 * length size has failed.
 */
		fp = fmemopen(NULL, SIZE_MAX, *p);
		T_EXPECT_NULL(fp, NULL);
		T_EXPECT_EQ(errno, ENOMEM, NULL);
	}
#endif
}

/*
 * test09 - test14:
 * An attempt to seek a memory buffer stream to a negative position or to a
 * position larger than the buffer size given in the size argument shall fail.
 */

T_DECL(netbsd_fmemopen_test09, "")
{
	struct testcase *t;
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;
	off_t i;

	for (t = &testcases[0]; t->s != NULL; ++t) {
		for (p = &mode_rwa[0]; *p != NULL; ++p) {

			memcpy(&buf[0], t->s, t->n);
			fp = fmemopen(&buf[0], (size_t)t->n, *p);
			T_EXPECT_NOTNULL(fp, NULL);

/*
 * test fmemopen_seek(SEEK_SET)
 */
			/* zero */
			T_EXPECT_POSIX_ZERO(fseeko(fp, (off_t)0, SEEK_SET), NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);

			/* positive */
			for (i = (off_t)1; i <= (off_t)t->n; ++i) {
				T_EXPECT_POSIX_ZERO(fseeko(fp, i, SEEK_SET), NULL);
				T_EXPECT_EQ(ftello(fp), i, NULL);
			}
			/* positive + OOB */
			T_EXPECT_EQ(fseeko(fp, t->n + 1, SEEK_SET), -1, NULL);
			T_EXPECT_EQ(ftello(fp), t->n, NULL);

			/* negative + OOB */
			T_EXPECT_EQ(fseeko(fp, (off_t)-1, SEEK_SET), -1, NULL);
			T_EXPECT_EQ(ftello(fp), t->n, NULL);

			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}

static const char *mode_rw[] = {
    "r", "rb", "r+", "rb+", "r+b",
    "w", "wb", "w+", "wb+", "w+b",
    NULL
};

T_DECL(netbsd_fmemopen_test10, "")
{
	struct testcase *t;
	off_t i;
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

	for (t = &testcases[0]; t->s != NULL; ++t) {
		for (p = &mode_rw[0]; *p != NULL; ++p) {

			memcpy(&buf[0], t->s, t->n);
			fp = fmemopen(&buf[0], (size_t)t->n, *p);
			T_EXPECT_NOTNULL(fp, NULL);

/*
 * test fmemopen_seek(SEEK_CUR)
 */
			T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);

			/* zero */
			T_EXPECT_POSIX_ZERO(fseeko(fp, (off_t)0, SEEK_CUR), NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);

			/* negative & OOB */
			T_EXPECT_EQ(fseeko(fp, (off_t)-1, SEEK_CUR), -1, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);

			/* positive */
			for (i = 0; i < (off_t)t->n; ++i) {
				T_EXPECT_POSIX_ZERO(fseeko(fp, (off_t)1, SEEK_CUR), NULL);
				T_EXPECT_EQ(ftello(fp), i + 1, NULL);
			}

			/* positive & OOB */
			T_EXPECT_EQ(fseeko(fp, (off_t)1, SEEK_CUR), -1, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)t->n, NULL);

			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}

T_DECL(netbsd_fmemopen_test11, "")
{
	struct testcase *t;
	off_t len, rest, i;
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

	/* test fmemopen_seek(SEEK_CUR) */
	for (t = &testcases[0]; t->s != NULL; ++t) {
		len = (off_t)strnlen(t->s, (size_t)t->n);
		rest = (off_t)t->n - len;
		for (p = &mode_a[0]; *p != NULL; ++p) {

			memcpy(&buf[0], t->s, t->n);
			fp = fmemopen(&buf[0], (size_t)t->n, *p);
			T_EXPECT_NOTNULL(fp, NULL);
/*
 * test fmemopen_seek(SEEK_CUR)
 */
#if defined(__GLIBC__)
			if (i < (off_t)t->n) {
#endif
			/* zero */
			T_EXPECT_POSIX_ZERO(fseeko(fp, (off_t)0, SEEK_CUR), NULL);
			T_EXPECT_EQ(ftello(fp), len, NULL);

			/* posive */
			for (i = (off_t)1; i <= rest; ++i) {
				T_EXPECT_POSIX_ZERO(fseeko(fp, (off_t)1, SEEK_CUR), NULL);
				T_EXPECT_EQ(ftello(fp), len + i, NULL);
			}

			/* positive + OOB */
			T_EXPECT_EQ(fseeko(fp, (off_t)1, SEEK_CUR), -1, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)t->n, NULL);

			/* negative */
			for (i = (off_t)1; i <= (off_t)t->n; ++i) {
				T_EXPECT_POSIX_ZERO(fseeko(fp, (off_t)-1, SEEK_CUR), NULL);
				T_EXPECT_EQ(ftello(fp), (off_t)t->n - i, NULL);
			}

			/* negative + OOB */
			T_EXPECT_EQ(fseeko(fp, (off_t)-1, SEEK_CUR), -1, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);

#if defined(__GLIBC__)
			}
#endif
			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}

#if 0
T_DECL(netbsd_fmemopen_test12, "")
{
	struct testcase *t;
	off_t len, rest, i;
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

	/* test fmemopen_seek(SEEK_END) */
	for (t = &testcases[0]; t->s != NULL; ++t) {
		len = (off_t)strnlen(t->s, t->n);
		rest = t->n - len;
		for (p = &mode_r[0]; *p != NULL; ++p) {

			memcpy(buf, t->s, t->n);
			fp = fmemopen(&buf[0], t->n, *p);
			T_EXPECT_NOTNULL(fp, NULL);

/*
 * test fmemopen_seek(SEEK_END)
 */
#if !defined(__GLIBC__)
			T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);

			/* zero */
			T_EXPECT_POSIX_ZERO(fseeko(fp, (off_t)0, SEEK_END), NULL);
			T_EXPECT_EQ(ftello(fp), len, NULL);

			/* positive + OOB */
			T_EXPECT_EQ(fseeko(fp, rest + 1, SEEK_END), -1, NULL);
			T_EXPECT_EQ(ftello(fp), len, NULL);

			/* negative + OOB */
			T_EXPECT_EQ(fseeko(fp, -(len + 1), SEEK_END), -1, NULL);
			T_EXPECT_EQ(ftello(fp), len, NULL);

			/* positive */
			for (i = 1; i <= rest; ++i) {
				T_EXPECT_POSIX_ZERO(fseeko(fp, i, SEEK_END), NULL);
				T_EXPECT_EQ(ftello(fp), len + i, NULL);
			}

			/* negative */
			for (i = 1; i < len; ++i) {
				T_EXPECT_POSIX_ZERO(fseeko(fp, -i, SEEK_END), NULL);
				T_EXPECT_EQ(ftello(fp), len - i, NULL);
			}
#endif
			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}
#endif // 0

T_DECL(netbsd_fmemopen_test13, "")
{
	struct testcase *t;
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

	/* test fmemopen_seek(SEEK_END) */
	for (t = &testcases[0]; t->s != NULL; ++t) {
		for (p = &mode_w[0]; *p != NULL; ++p) {

			memcpy(buf, t->s, t->n);
			fp = fmemopen(&buf[0], (size_t)t->n, *p);
			T_EXPECT_NOTNULL(fp, NULL);
/*
 * test fmemopen_seek(SEEK_END)
 */
#if !defined(__GLIBC__)
			T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);
			T_EXPECT_EQ(buf[0], '\0', NULL);

			/* zero */
			T_EXPECT_POSIX_ZERO(fseeko(fp, (off_t)0, SEEK_END), NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);

			/* positive + OOB */
			T_EXPECT_EQ(fseeko(fp, (off_t)t->n + 1, SEEK_END), -1, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);

			/* negative + OOB */
			T_EXPECT_EQ(fseeko(fp, -1, SEEK_END), -1, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);
#endif

#if 0
			/* positive */
			for (int i = 1; i <= t->n; ++i) {
				T_EXPECT_POSIX_ZERO(fseeko(fp, i, SEEK_END), NULL);
				T_EXPECT_EQ(ftello(fp), i, NULL);
			}
#endif
			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}

T_DECL(netbsd_fmemopen_test14, "")
{
	struct testcase *t;
	off_t len, rest, i;
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

	/* test fmemopen_seek(SEEK_END) */
	for (t = &testcases[0]; t->s != NULL; ++t) {
		len = (off_t)strnlen(t->s, (size_t)t->n);
		rest = (off_t)t->n - len;
		for (p = &mode_a[0]; *p != NULL; ++p) {

			memcpy(buf, t->s, t->n);
			fp = fmemopen(&buf[0], (size_t)t->n, *p);
			T_EXPECT_NOTNULL(fp, NULL);
/*
 * test fmemopen_seek(SEEK_END)
 */
#if !defined(__GLIBC__)
			T_EXPECT_EQ(ftello(fp), len, NULL);

			/* zero */
			T_EXPECT_POSIX_ZERO(fseeko(fp, 0, SEEK_END), NULL);
			T_EXPECT_EQ(ftello(fp), len, NULL);

			/* positive + OOB */
			T_EXPECT_EQ(fseeko(fp, rest + 1, SEEK_END), -1, NULL);
			T_EXPECT_EQ(ftello(fp), len, NULL);

			/* negative + OOB */
			T_EXPECT_EQ(fseeko(fp, -(len + 1), SEEK_END), -1, NULL);
			T_EXPECT_EQ(ftello(fp), len, NULL);

#if 0
			/* positive */
			for (i = 1; i <= rest; ++i) {
				T_EXPECT_POSIX_ZERO(fseeko(fp, i, SEEK_END), NULL);
				T_EXPECT_EQ(ftello(fp), len + i, NULL);
			}
#endif

			/* negative */
			for (i = 1; i < len; ++i) {
				T_EXPECT_POSIX_ZERO(fseeko(fp, -i, SEEK_END), NULL);
				T_EXPECT_EQ(ftello(fp), len - i, NULL);
			}
#endif
			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}

#if 0

static const char *mode_rw1[] = {
    "r", "rb", "r+", "rb+", "r+b",
    "w+", "wb+",
    NULL
};

/* test15 - 18:
 * When a stream open for writing is flushed or closed, a null byte is written
 * at the current position or at the end of the buffer, depending on the size
 * of the contents.
 */

T_DECL(netbsd_fmemopen_test15, "")
{
	struct testcase *t;
	const char **p;
	char buf0[BUFSIZ];
	FILE *fp;
	int i;

	for (t = &testcases[0]; t->s != NULL; ++t) {
		for (p = &mode_rw1[0]; *p != NULL; ++p) {

			memcpy(&buf0[0], t->s, t->n);
			fp = fmemopen(&buf0[0], t->n, *p);
			T_EXPECT_NOTNULL(fp, NULL);
/*
 * test fmemopen_read + fgetc(3)
 */
			for (i = 0; i < t->n; ++i) {
				T_EXPECT_EQ(ftello(fp), (off_t)i, NULL);
				T_EXPECT_EQ(fgetc(fp), buf0[i], NULL);
				T_EXPECT_EQ(feof(fp), 0, NULL);
				T_EXPECT_EQ(ftello(fp), (off_t)i + 1, NULL);
			}
			T_EXPECT_EQ(fgetc(fp), EOF, NULL);
			T_EXPECT_NE(feof(fp), 0, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)t->n, NULL);
			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}

T_DECL(netbsd_fmemopen_test16, "")
{
	struct testcase *t;
	const char **p;
	char buf0[BUFSIZ], buf1[BUFSIZ];
	FILE *fp;

	for (t = &testcases[0]; t->s != NULL; ++t) {
		for (p = &mode_rw1[0]; *p != NULL; ++p) {

			memcpy(&buf0[0], t->s, t->n);
			buf1[t->n] = 0x1;
			fp = fmemopen(&buf0[0], t->n, *p);
			T_EXPECT_NOTNULL(fp, NULL);
/*
 * test fmemopen_read + fread(4)
 */
			T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);
			T_EXPECT_EQ(fread(&buf1[0], 1, sizeof(buf1), fp), (size_t)t->n, NULL);
			T_EXPECT_NE(feof(fp), 0, NULL);
			T_EXPECT_EQ(memcmp(&buf0[0], &buf1[0], t->n), 0, NULL);
			T_EXPECT_EQ((unsigned char)buf1[t->n], 0x1, NULL);

			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}

const char *mode_a1[] = { "a+", "ab+", NULL };

T_DECL(netbsd_fmemopen_test17, "")
{
	struct testcase *t;
	size_t len;
	int i;
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

	for (t = &testcases[0]; t->s != NULL; ++t) {
		len = strnlen(t->s, t->n);
		for (p = &mode_a1[0]; *p != NULL; ++p) {

			memcpy(&buf[0], t->s, t->n);
			fp = fmemopen(&buf[0], t->n, *p);
			T_EXPECT_NOTNULL(fp, NULL);
/*
 * test fmemopen_read + fgetc(3)
 */
#if defined(__GLIBC__)
			if (i < t->n) {
#endif
			for (i = len; i < t->n; ++i) {
				T_EXPECT_EQ(ftello(fp), (off_t)i, NULL);
				T_EXPECT_EQ(fgetc(fp), buf[i], NULL);
				T_EXPECT_EQ(feof(fp), 0, NULL);
				T_EXPECT_EQ(ftello(fp), (off_t)i + 1, NULL);
			}
			T_EXPECT_EQ(fgetc(fp), EOF, NULL);
			T_EXPECT_NE(feof(fp), 0, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)t->n, NULL);
			rewind(fp);
			for (i = 0; i < t->n; ++i) {
				T_EXPECT_EQ(ftello(fp), (off_t)i, NULL);
				T_EXPECT_EQ(fgetc(fp), buf[i], NULL);
				T_EXPECT_EQ(feof(fp), 0, NULL);
				T_EXPECT_EQ(ftello(fp), (off_t)i + 1, NULL);
			}
			T_EXPECT_EQ(fgetc(fp), EOF, NULL);
			T_EXPECT_NE(feof(fp), 0, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)t->n, NULL);
#if defined(__GLIBC__)
			}
#endif
			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}

T_DECL(netbsd_fmemopen_test18, "")
{
	struct testcase *t;
	size_t len;
	const char **p;
	char buf0[BUFSIZ], buf1[BUFSIZ];
	FILE *fp;

	for (t = &testcases[0]; t->s != NULL; ++t) {
		len = strnlen(t->s, t->n);
		for (p = &mode_a1[0]; *p != NULL; ++p) {

			memcpy(&buf0[0], t->s, t->n);
			buf1[t->n - len] = 0x1;
			fp = fmemopen(&buf0[0], t->n, *p);
			T_EXPECT_NOTNULL(fp, NULL);
/*
 * test fmemopen_read + fread(3)
 */
#if defined(__GLIBC__)
			if (i < t->n) {
#endif
			T_EXPECT_EQ(ftello(fp), (off_t)len, NULL);
			T_EXPECT_EQ(fread(&buf1[0], 1, sizeof(buf1), fp)
			   , t->n - len, NULL);
			T_EXPECT_NE(feof(fp), 0, NULL);
			T_EXPECT_FALSE(memcmp(&buf0[len], &buf1[0], t->n - len));
			T_EXPECT_EQ((unsigned char)buf1[t->n - len], 0x1, NULL);
			rewind(fp);
			buf1[t->n] = 0x1;
			T_EXPECT_EQ(ftello(fp), (off_t)0, NULL);
			T_EXPECT_EQ(fread(&buf1[0], 1, sizeof(buf1), fp)
			   , (size_t)t->n, NULL);
			T_EXPECT_NE(feof(fp), 0, NULL);
			T_EXPECT_FALSE(memcmp(&buf0[0], &buf1[0], t->n));
			T_EXPECT_EQ((unsigned char)buf1[t->n], 0x1, NULL);
#if defined(__GLIBC__)
			}
#endif
			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}

/*
 * test19 - test22:
 * If a stream open for update is flushed or closed and the last write has
 * advanced the current buffer size, a null byte is written at the end of the
 * buffer if it fits.
 */

const char *mode_rw2[] = {
    "r+", "rb+", "r+b",
    "w", "wb", "w+", "wb+", "w+b",
    NULL
};

T_DECL(netbsd_fmemopen_test19, "")
{
	struct testcase *t;
	int i;
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

	for (t = &testcases[0]; t->s != NULL; ++t) {
		for (p = &mode_rw2[0]; *p != NULL; ++p) {

			memcpy(&buf[0], t->s, t->n);
			buf[t->n] = 0x1;
			fp = fmemopen(&buf[0], t->n + 1, *p);
			T_EXPECT_NOTNULL(fp, NULL);
			setbuf(fp, NULL);
/*
 * test fmemopen_write + fputc(3)
 */
			for (i = 0; i < t->n; ++i) {
				T_EXPECT_EQ(ftello(fp), (off_t)i, NULL);
				T_EXPECT_EQ(fputc(t->s[i], fp), t->s[i], NULL);
				T_EXPECT_EQ(buf[i], t->s[i], NULL);
				T_EXPECT_EQ(ftello(fp), (off_t)i + 1, NULL);
				T_EXPECT_EQ(buf[i], t->s[i], NULL);
#if !defined(__GLIBC__)
				T_EXPECT_EQ(buf[i + 1], '\0', NULL);
#endif
			}

/* don't accept non nul character at end of buffer */
			T_EXPECT_EQ(fputc(0x1, fp), EOF, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)t->n, NULL);
			T_EXPECT_EQ(feof(fp), 0, NULL);

/* accept nul character at end of buffer */
			T_EXPECT_EQ(fputc('\0', fp), '\0', NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)t->n + 1, NULL);
			T_EXPECT_EQ(feof(fp), 0, NULL);

/* reach EOF */
			T_EXPECT_EQ(fputc('\0', fp), EOF, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)t->n + 1, NULL);

			/* compare */
			T_EXPECT_EQ(memcmp(&buf[0], t->s, t->n), 0, NULL);
			T_EXPECT_EQ(buf[t->n], '\0', NULL);

			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}

T_DECL(netbsd_fmemopen_test20, "")
{
	struct testcase *t;
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

	for (t = &testcases[0]; t->s != NULL; ++t) {
		for (p = &mode_rw2[0]; *p != NULL; ++p) {

			memcpy(&buf[0], t->s, t->n);
			buf[t->n] = 0x1;
			fp = fmemopen(&buf[0], t->n + 1, *p);
			T_EXPECT_NOTNULL(fp, NULL);
			setbuf(fp, NULL);
			T_EXPECT_EQ(fwrite(t->s, 1, t->n, fp), (size_t)t->n, NULL);
/*
 * test fmemopen_write + fwrite(3)
 */
#if !defined(__GLIBC__)
			T_EXPECT_EQ(buf[t->n], '\0', NULL);

/* don't accept non nul character at end of buffer */
			T_EXPECT_EQ(fwrite("\x1", 1, 1, fp), 0, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)t->n, NULL);
			T_EXPECT_EQ(feof(fp), 0, NULL);
#endif

/* accept nul character at end of buffer */
			T_EXPECT_EQ(fwrite("\x0", 1, 1, fp), 1, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)t->n + 1, NULL);
			T_EXPECT_EQ(feof(fp), 0, NULL);

/* reach EOF */
			T_EXPECT_EQ(fputc('\0', fp), EOF, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)t->n + 1, NULL);

/* compare */
			T_EXPECT_EQ(memcmp(&buf[0], t->s, t->n), 0, NULL);
			T_EXPECT_EQ(buf[t->n], '\0', NULL);

			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}

T_DECL(netbsd_fmemopen_test21, "")
{
	struct testcase *t;
	int len, i;
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

	for (t = &testcases[0]; t->s != NULL; ++t) {
		len = strnlen(t->s, t->n);
		for (p = &mode_a[0]; *p != NULL; ++p) {
			memcpy(&buf[0], t->s, t->n);
			fp = fmemopen(&buf[0], t->n, *p);
			T_EXPECT_NOTNULL(fp, NULL);
			setbuf(fp, NULL);
/*
 * test fmemopen_write + fputc(3)
 */
			if (len < t->n) {
				for (i = len; i < t->n - 1; ++i) {
					T_EXPECT_EQ(ftello(fp), (off_t)i, NULL);
					T_EXPECT_EQ(fputc(t->s[i - len], fp)
					   , t->s[i - len], NULL);
					T_EXPECT_EQ(buf[i], t->s[i - len], NULL);
					T_EXPECT_EQ(ftello(fp), (off_t)i + 1, NULL);
#if !defined(__GLIBC__)
					T_EXPECT_EQ(buf[i + 1], '\0', NULL);
#endif
				}

/* don't accept non nul character at end of buffer */
				T_EXPECT_EQ(ftello(fp), (off_t)t->n - 1, NULL);
				T_EXPECT_EQ(fputc(0x1, fp), EOF, NULL);
				T_EXPECT_EQ(ftello(fp), (off_t)t->n - 1, NULL);

/* accept nul character at end of buffer */
				T_EXPECT_EQ(ftello(fp), (off_t)t->n - 1, NULL);
				T_EXPECT_EQ(fputc('\0', fp), '\0', NULL);
				T_EXPECT_EQ(ftello(fp), (off_t)t->n, NULL);
			}

/* reach EOF */
			T_EXPECT_EQ(ftello(fp), (off_t)t->n, NULL);
			T_EXPECT_EQ(fputc('\0', fp), EOF, NULL);
			T_EXPECT_EQ(ftello(fp), (off_t)t->n, NULL);

			T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
		}
	}
}

T_DECL(netbsd_fmemopen_test22, "")
{
	struct testcase *t0, *t1;
	size_t len0, len1, nleft;
	const char **p;
	char buf[BUFSIZ];
	FILE *fp;

	for (t0 = &testcases[0]; t0->s != NULL; ++t0) {
		len0 = strnlen(t0->s, t0->n);
		for (t1 = &testcases[0]; t1->s != NULL; ++t1) {
			len1 = strnlen(t1->s, t1->n);
			for (p = &mode_a[0]; *p != NULL; ++p) {

				memcpy(&buf[0], t0->s, t0->n);
				fp = fmemopen(&buf[0], t0->n, *p);
				T_EXPECT_NOTNULL(fp, NULL);
				setbuf(fp, NULL);
/*
 * test fmemopen_write + fwrite(3)
 */
				nleft = t0->n - len0;
#if !defined(__GLIBC__)
				if (nleft == 0 || len1 == nleft - 1) {
					T_EXPECT_EQ(fwrite(t1->s, 1, t1->n, fp)
					   , nleft, NULL);
					T_EXPECT_EQ(ftell(fp), t1->n, NULL);
				} else {
					T_EXPECT_EQ(fwrite(t1->s, 1, t1->n, fp)
					   , nleft - 1, NULL);
					T_EXPECT_EQ(ftell(fp), t1->n - 1, NULL);
				}
#endif
				T_EXPECT_POSIX_ZERO(fclose(fp), NULL);
			}
		}
	}
}
#endif //0

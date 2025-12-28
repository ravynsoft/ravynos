/*
 * Copyright (c) 2012 Apple, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <err.h>
#include <sysexits.h>

#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/time.h>

#ifdef __linux__
#include <bsd/unistd.h> /* for getmode, setmode */
#define COPYFILE_DATA 0
int fcopyfile(int, int, void *, int);
#else
#include <copyfile.h>
#endif

void usage(void);

int
main(int argc, char * argv[])
{
	struct stat sb;
	void *mset;
	mode_t mode;
	bool gotmode = false;
	int ch;
	int ret;
	int srcfd, dstfd;
	const char *src = NULL;
	const char *dst = NULL;
	char dsttmpname[MAXPATHLEN];

	while ((ch = getopt(argc, argv, "cSm:")) != -1) {
		switch (ch) {
		case 'c':
		case 'S':
			/* ignored for compatibility */
			break;
		case 'm':
			gotmode = true;
			mset = setmode(optarg);
			if (!mset) {
				errx(EX_USAGE, "Unrecognized mode %s", optarg);
			} else {
				mode = getmode(mset, 0);
				free(mset);
			}
			break;
		case '?':
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 2) {
		usage();
	}

	src = argv[0];
	dst = argv[1];

#ifdef __linux__
	srcfd = open(src, O_RDONLY | O_NOFOLLOW, 0);
#else
	srcfd = open(src, O_RDONLY | O_SYMLINK, 0);
#endif
	if (srcfd < 0) {
		err(EX_NOINPUT, "open(%s)", src);
	}

	ret = fstat(srcfd, &sb);
	if (ret < 0) {
		err(EX_NOINPUT, "fstat(%s)", src);
	}

	if (!S_ISREG(sb.st_mode)) {
		err(EX_USAGE, "%s is not a regular file", src);
	}

	snprintf(dsttmpname, sizeof(dsttmpname), "%s.XXXXXX", dst);

	dstfd = mkstemp(dsttmpname);
	if (dstfd < 0) {
		err(EX_UNAVAILABLE, "mkstemp(%s)", dsttmpname);
	}

	ret = fcopyfile(srcfd, dstfd, NULL,
	    COPYFILE_DATA);
	if (ret < 0) {
		err(EX_UNAVAILABLE, "fcopyfile(%s, %s)", src, dsttmpname);
	}

	ret = futimes(dstfd, NULL);
	if (ret < 0) {
		err(EX_UNAVAILABLE, "futimes(%s)", dsttmpname);
	}

	if (gotmode) {
		ret = fchmod(dstfd, mode);
		if (ret < 0) {
			err(EX_NOINPUT, "fchmod(%s, %ho)", dsttmpname, mode);
		}
	}

	ret = rename(dsttmpname, dst);
	if (ret < 0) {
		err(EX_NOINPUT, "rename(%s, %s)", dsttmpname, dst);
	}

	ret = close(dstfd);
	if (ret < 0) {
		err(EX_NOINPUT, "close(dst)");
	}

	ret = close(srcfd);
	if (ret < 0) {
		err(EX_NOINPUT, "close(src)");
	}

	return 0;
}

void
usage(void)
{
	fprintf(stderr, "Usage: %s [-c] [-S] [-m <mode>] <src> <dst>\n",
#ifdef __linux__
		"installfile");
#else
	    getprogname());
#endif
	exit(EX_USAGE);
}

#ifdef __linux__
/*
 * fcopyfile() is used to copy a source file descriptor to a destination file
 * descriptor.  This allows an application to figure out how it wants to open
 * the files (doing various security checks, perhaps), and then just pass in
 * the file descriptors.
 */
int fcopyfile(int src_fd, int dst_fd, void *state, int flags)
{
	char buffer[512*1024];
	int ret = 0, size = 0;
	struct stat src_sb, dst_sb;

	if (src_fd < 0 || dst_fd < 0)
	{
		errno = EINVAL;
		return -1;
	}

	fstat(src_fd, &src_sb);
	switch (src_sb.st_mode & S_IFMT)
	{
		case S_IFLNK:
		case S_IFDIR:
		case S_IFREG:
			break;
		default:
			errno = ENOTSUP;
			return -1;
	}

	(void)fstat(dst_fd, &dst_sb);
	(void)fchmod(dst_fd, (dst_sb.st_mode & ~S_IFMT) | (S_IRUSR | S_IWUSR));

	while((size = read(src_fd, buffer, sizeof(buffer))) > 0)
	{
		write(dst_fd, buffer, size);
	}

	/*
	 * copy the non-meta data portion of the file.  We attempt to
	 * remove (via unlink) the destination file if we fail.
	 */
	if ((src_sb.st_mode & S_IFMT) != S_IFREG)
		return 0;

	fchmod(dst_fd, src_sb.st_mode & ~S_IFMT);
	return ret;
}

#endif /* __linux__ */
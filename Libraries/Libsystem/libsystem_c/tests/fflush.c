#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <darwintest.h>
#include <darwintest_utils.h>

static char tmpfile_template[] = "/tmp/libc_test_fflushXXXXX";
#define BUFSZ 128
static char wrbuf[BUFSZ] = "";
static const size_t filesz = BUFSZ * 120;

static void
cleanup_tmp_file(void)
{
	(void)unlink(tmpfile_template);
}

static const char *
assert_empty_tmp_file(void)
{
	T_SETUPBEGIN;

	int tmpfd = mkstemp(tmpfile_template);
	T_ASSERT_POSIX_SUCCESS(tmpfd, "created tmp file at %s", tmpfile_template);
	T_ATEND(cleanup_tmp_file);
	close(tmpfd);

	T_SETUPEND;

	return tmpfile_template;
}

static const char *
assert_full_tmp_file(void)
{
	T_SETUPBEGIN;

	int tmpfd = mkstemp(tmpfile_template);
	T_ASSERT_POSIX_SUCCESS(tmpfd, "created tmp file at %s", tmpfile_template);
	T_ATEND(cleanup_tmp_file);

	/*
	 * Write a pattern of bytes into the file -- the lowercase alphabet,
	 * separated by newlines.
	 */
	for (size_t i = 0; i < BUFSZ; i++) {
		wrbuf[i] = 'a' + (i % 27);
		if (i % 27 == 26) {
			wrbuf[i] = '\n';
		}
	}
	for (size_t i = 0; i < filesz; i++) {
		ssize_t byteswr = 0;
		do {
			byteswr = write(tmpfd, wrbuf, BUFSZ);
		} while (byteswr == -1 && errno == EAGAIN);

		T_QUIET; T_ASSERT_POSIX_SUCCESS(byteswr, "wrote %d bytes to tmp file",
				BUFSZ);
		T_QUIET; T_ASSERT_EQ(byteswr, (ssize_t)BUFSZ,
				"wrote correct amount of bytes to tmp file");
	}

	close(tmpfd);

	T_SETUPEND;

	return tmpfile_template;
}

/*
 * Ensure that fflush on an input stream conforms to the SUSv3 definition, which
 * requires synchronizing the FILE position with the underlying file descriptor.
 */
T_DECL(fflush_input, "fflush on a read-only FILE resets fd offset")
{
	const char *tmpfile = assert_full_tmp_file();

	T_SETUPBEGIN;

	FILE *tmpf = fopen(tmpfile, "r");
	T_QUIET; T_WITH_ERRNO;
	T_ASSERT_NOTNULL(tmpf, "opened tmp file for reading");

	/*
	 * Move some way into the file.
	 */
	char buf[100] = "";
	size_t nread = fread(buf, sizeof(buf), 1, tmpf);
	T_ASSERT_EQ(nread, (size_t)1, "read correct number of items from FILE");
	char last_read_char = buf[sizeof(buf) - 1];

	off_t curoff = lseek(fileno(tmpf), 0, SEEK_CUR);
	T_ASSERT_GT(curoff, (off_t)0, "file offset should be non-zero");

	T_SETUPEND;

	/*
	 * fflush(3) to reset the fd back to the FILE offset.
	 */
	int ret = fflush(tmpf);
	T_ASSERT_POSIX_SUCCESS(ret, "fflush on read-only FILE");

	off_t flushoff = lseek(fileno(tmpf), 0, SEEK_CUR);
	T_ASSERT_EQ(flushoff, (off_t)sizeof(buf),
			"offset of file should be bytes read on FILE after fflush");

	/*
	 * Make sure the FILE is reading the right thing -- the next character
	 * should be one letter after the last byte read, from the last call to
	 * fread(3).
	 */
	char c = '\0';
	nread = fread(&c, sizeof(c), 1, tmpf);
	T_QUIET;
	T_ASSERT_EQ(nread, (size_t)1, "read correct number of items from FILE");

	/*
	 * The pattern in the file is the alphabet -- and this doesn't land on
	 * a newline.
	 */
	T_QUIET;
	T_ASSERT_NE((flushoff) % 27, (off_t)0,
			"previous offset shouldn't land on newline");
	T_QUIET;
	T_ASSERT_NE((flushoff + 1) % 27, (off_t)0,
			"current offset shouldn't land on newline");

	T_ASSERT_EQ(c, last_read_char + 1, "read correct byte after fflush");

	ret = fflush(tmpf);
	T_QUIET; T_ASSERT_POSIX_SUCCESS(ret, "fflush on read-only FILE");

	flushoff = lseek(fileno(tmpf), 0, SEEK_CUR);
	T_ASSERT_EQ(flushoff, (off_t)(sizeof(buf) + sizeof(c)),
			"offset of file should be incremented after subsequent read");

	/*
	 * Use ungetc(3) to induce the optimized ungetc behavior in the FILE.
	 */
	int ugret = ungetc(c, tmpf);
	T_QUIET; T_ASSERT_NE(ugret, EOF, "ungetc after fflush");
	T_QUIET; T_ASSERT_EQ((char)ugret, c, "ungetc un-got the correct char");

	ret = fflush(tmpf);
	T_ASSERT_POSIX_SUCCESS(ret, "fflush after ungetc");
	flushoff = lseek(fileno(tmpf), 0, SEEK_CUR);
	T_ASSERT_EQ(flushoff, (off_t)sizeof(buf),
			"offset of file should be correct after ungetc and fflush");

	nread = fread(&c, sizeof(c), 1, tmpf);
	T_QUIET;
	T_ASSERT_EQ(nread, (size_t)1, "read correct number of items from FILE");
	T_ASSERT_EQ(c, last_read_char + 1,
			"read correct byte after ungetc and fflush");
}

/*
 * Try to trick fclose into not reporting an ENOSPC error from the underlying
 * descriptor in update mode.  Previous versions of Libc only flushed the FILE
 * if it was write-only.
 */

#if TARGET_OS_OSX
/*
 * Only macOS contains a version of hdiutil that can create disk images.
 */

#define DMGFILE "/tmp/test_fclose_enospc.dmg"
#define VOLNAME "test_fclose_enospc"
static const char *small_file = "/Volumes/" VOLNAME "/test.txt";

static void
cleanup_dmg(void)
{
	char *hdiutil_detach_argv[] = {
		"/usr/bin/hdiutil", "detach", "/Volumes/" VOLNAME, NULL,
	};
	pid_t hdiutil_detach = -1;
	int ret = dt_launch_tool(&hdiutil_detach, hdiutil_detach_argv, false, NULL,
			NULL);
	if (ret != -1) {
		int status = 0;
		(void)waitpid(hdiutil_detach, &status, 0);
	}
	(void)unlink(DMGFILE);
}

T_DECL(fclose_enospc, "ensure ENOSPC is preserved on fclose")
{
	T_SETUPBEGIN;

	/*
	 * Ensure a disk is available that will fill up and start returning ENOSPC.
	 *
	 * system(3) would be easier...
	 */
	char *hdiutil_argv[] = {
		"/usr/bin/hdiutil", "create", "-size", "5m", "-type", "UDIF",
		"-volname", VOLNAME, "-nospotlight", "-fs", "HFS+", DMGFILE, "-attach",
		NULL,
	};
	pid_t hdiutil_create = -1;
	int ret = dt_launch_tool(&hdiutil_create, hdiutil_argv, false, NULL, NULL);
	T_ASSERT_POSIX_SUCCESS(ret, "created and attached 5MB DMG");
	int status = 0;
	pid_t waited = waitpid(hdiutil_create, &status, 0);
	T_QUIET; T_ASSERT_EQ(waited, hdiutil_create,
			"should have waited for the process that was launched");
	T_QUIET;
	T_ASSERT_TRUE(WIFEXITED(status), "hdiutil should have exited");
	T_QUIET;
	T_ASSERT_EQ(WEXITSTATUS(status), 0,
			"hdiutil should have exited successfully");

	T_ATEND(cleanup_dmg);

	/*
	 * Open for updating, as previously only write-only files would be flushed
	 * on fclose.
	 */
	FILE *fp = fopen(small_file, "a+");
	T_WITH_ERRNO;
	T_ASSERT_NOTNULL(fp, "opened file at %s for append-updating", small_file);

	char *buf = malloc(BUFSIZ);
	T_QUIET; T_WITH_ERRNO;
	T_ASSERT_NOTNULL(buf, "should allocate BUFSIZ bytes");

	for (int i = 0; i < BUFSIZ; i++) {
		buf[i] = (char)(i % 256);
	}

	/*
	 * Fill up the disk -- induce ENOSPC.
	 */
	size_t wrsize = BUFSIZ;
	for (int i = 0; i < 2; i++) {
		for (;;) {
			errno = 0;
			if (write(fileno(fp), buf, wrsize) < 0) {
				if (errno == ENOSPC) {
					break;
				}
				T_WITH_ERRNO; T_ASSERT_FAIL("write(2) failed");
			}
		}
		wrsize = 1;
	}
	T_PASS("filled up the file until ENOSPC");
	free(buf);

	/*
	 * Make sure the FILE is at the end, so any writes it does hit ENOSPC.
	 */
	ret = fseek(fp, 0, SEEK_END);
	T_ASSERT_POSIX_SUCCESS(ret, "fseek to the end of a complete file");

	/*
	 * Try to push a character into the file; since this is buffered, it should
	 * succeed.
	 */
	ret = fputc('a', fp);
	T_ASSERT_POSIX_SUCCESS(ret,
			"fputc to put an additional character in the FILE");

	T_SETUPEND;

	/*
	 * fclose should catch the ENOSPC error when it flushes the file, before it
	 * closes the underlying descriptor.
	 */
	errno = 0;
	ret = fclose(fp);
	if (ret != EOF) {
		T_ASSERT_FAIL("fclose should fail when the FILE is full");
	}
	if (errno != ENOSPC) {
		T_WITH_ERRNO; T_ASSERT_FAIL("fclose should fail with ENOSPC");
	}

	T_PASS("fclose returned ENOSPC");
}

#endif // TARGET_OS_OSX

/*
 * Ensure no errors are returned when flushing a read-only, unseekable input
 * stream.
 */
T_DECL(fflush_unseekable_input,
		"ensure sanity when an unseekable input stream is flushed")
{
	T_SETUPBEGIN;

	/*
	 * Use a pipe for the unseekable streams.
	 */
	int pipes[2];
	int ret = pipe(pipes);
	T_ASSERT_POSIX_SUCCESS(ret, "create a pipe");
	FILE *in = fdopen(pipes[0], "r");
	T_QUIET; T_WITH_ERRNO; T_ASSERT_NOTNULL(in,
			"open input stream to read end of pipe");
	FILE *out = fdopen(pipes[1], "w");
	T_QUIET; T_WITH_ERRNO; T_ASSERT_NOTNULL(out,
			"open output stream to write end of pipe");

	/*
	 * Fill the pipe with some text (but not too much that the write would
	 * block!).
	 */
	fprintf(out, "this is a test and has some more text");
	ret = fflush(out);
	T_ASSERT_POSIX_SUCCESS(ret, "flushed the output stream");

	/*
	 * Protect stdio from delving too deep into the pipe.
	 */
	char inbuf[8] = {};
	setbuffer(in, inbuf, sizeof(inbuf));

	/*
	 * Just read a teensy bit to get the FILE offset different from the
	 * descriptor "offset."
	 */
	char rdbuf[2] = {};
	size_t nitems = fread(rdbuf, sizeof(rdbuf), 1, in);
	T_QUIET; T_ASSERT_GT(nitems, (size_t)0,
			"read from the read end of the pipe");

	T_SETUPEND;

	ret = fflush(in);
	T_ASSERT_POSIX_SUCCESS(ret,
			"should successfully flush unseekable input stream after reading");
}

/*
 * Ensure that reading to the end of a file and then calling ftell() still
 * causes EOF.
 */
T_DECL(ftell_feof,
		"ensure ftell does not reset feof when actually at end of file") {
	T_SETUPBEGIN;
	FILE *fp = fopen("/System/Library/CoreServices/SystemVersion.plist", "rb");
	T_WITH_ERRNO;
	T_ASSERT_NOTNULL(fp, "opened SystemVersion.plist");
	struct stat sb;
	T_ASSERT_POSIX_SUCCESS(fstat(fileno(fp), &sb), "fstat SystemVersion.plist");
	void *buf = malloc(sb.st_size * 2);
	T_ASSERT_NOTNULL(buf, "allocating buffer for size of SystemVersion.plist");
	T_SETUPEND;

	T_ASSERT_POSIX_SUCCESS(fseek(fp, 0, SEEK_SET), "seek to beginning");
	// fread can return short *or* zero, according to manpage
	fread(buf, sb.st_size * 2, 1, fp);
	T_ASSERT_EQ(ftell(fp), sb.st_size, "tfell() == file size");
	T_ASSERT_TRUE(feof(fp), "feof() reports end-of-file");
	free(buf);
}

T_DECL(putc_flush, "ensure putc flushes to file on close") {
	const char *fname = assert_empty_tmp_file();
	FILE *fp = fopen(fname, "w");
	T_WITH_ERRNO;
	T_ASSERT_NOTNULL(fp, "opened temporary file read/write");
	T_WITH_ERRNO;
	T_ASSERT_EQ(fwrite("testing", 1, 7, fp), 7, "write temp contents");
	(void)fclose(fp);

	fp = fopen(fname, "r+");
	T_WITH_ERRNO;
	T_ASSERT_NOTNULL(fp, "opened temporary file read/write");

	T_ASSERT_POSIX_SUCCESS(fseek(fp, -1, SEEK_END), "seek to end - 1");
	T_ASSERT_EQ(fgetc(fp), 'g', "fgetc should read 'g'");
	T_ASSERT_EQ(fgetc(fp), EOF, "fgetc should read EOF");
	T_ASSERT_EQ(ftell(fp), 7, "tfell should report position 7");

	int ret = fputc('!', fp);
	T_ASSERT_POSIX_SUCCESS(ret,
			"fputc to put an additional character in the FILE");
	T_ASSERT_EQ(ftell(fp), 8, "tfell should report position 8");

	T_QUIET;
	T_ASSERT_POSIX_SUCCESS(fclose(fp), "close temp file");

	fp = fopen(fname, "r");
	T_WITH_ERRNO;
	T_ASSERT_NOTNULL(fp, "opened temporary file read/write");

	char buf[9];
	T_WITH_ERRNO;
	T_ASSERT_NOTNULL(fgets(buf, sizeof(buf), fp), "read file data");
	T_ASSERT_EQ_STR(buf, "testing!", "read all the new data");

	(void)fclose(fp);
}

T_DECL(putc_writedrop, "ensure writes are flushed with a pending read buffer") {
	const char *fname = assert_empty_tmp_file();
	FILE *fp = fopen(fname, "w");
	T_WITH_ERRNO;
	T_ASSERT_NOTNULL(fp, "opened temporary file read/write");
	T_WITH_ERRNO;
	T_ASSERT_EQ(fwrite("testing", 1, 7, fp), 7, "write temp contents");
	(void)fclose(fp);

	fp = fopen(fname, "r+");
	T_WITH_ERRNO;
	T_ASSERT_NOTNULL(fp, "opened temporary file read/write");

	T_ASSERT_POSIX_SUCCESS(fseek(fp, -1, SEEK_END), "seek to end - 1");

	int ret = fputc('!', fp);
	T_ASSERT_POSIX_SUCCESS(ret,
			"fputc to put an additional character in the FILE");
	// flush the write buffer by reading a byte from the stream to put the
	// FILE* into read mode
	T_ASSERT_EQ(fgetc(fp), EOF, "fgetc should read EOF");

	T_QUIET;
	T_ASSERT_POSIX_SUCCESS(fclose(fp), "close temp file");

	fp = fopen(fname, "r");
	T_WITH_ERRNO;
	T_ASSERT_NOTNULL(fp, "opened temporary file read/write");

	char buf[9];
	T_WITH_ERRNO;
	T_ASSERT_NOTNULL(fgets(buf, sizeof(buf), fp), "read file data");
	T_ASSERT_EQ_STR(buf, "testin!", "read all the new data");

	(void)fclose(fp);
}

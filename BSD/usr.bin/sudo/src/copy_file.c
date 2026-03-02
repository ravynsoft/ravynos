/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020-2021 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/stat.h>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sudo.h>
#include <sudo_edit.h>

/*
 * Extend the given fd to the specified size in bytes.
 * We do this to allocate disk space up-front before overwriting
 * the original file with the temporary.  Otherwise, we could
 * run out of disk space after truncating the original file.
 */
static int
sudo_extend_file(int fd, const char *name, off_t new_size)
{
    off_t old_size, size;
    ssize_t nwritten;
    char zeroes[BUFSIZ] = { '\0' };
    debug_decl(sudo_extend_file, SUDO_DEBUG_UTIL);

    if ((old_size = lseek(fd, 0, SEEK_END)) == -1) {
	sudo_warn("lseek");
	debug_return_int(-1);
    }
    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: extending %s from %lld to %lld",
	__func__, name, (long long)old_size, (long long)new_size);

    for (size = old_size; size < new_size; size += nwritten) {
	off_t len = new_size - size;
	if (len > ssizeof(zeroes))
	    len = ssizeof(zeroes);
	nwritten = write(fd, zeroes, (size_t)len);
	if (nwritten == -1) {
	    int serrno = errno;
	    if (ftruncate(fd, old_size) == -1) {
		sudo_debug_printf(
		    SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		    "unable to truncate %s to %lld", name, (long long)old_size);
	    }
	    errno = serrno;
	    debug_return_int(-1);
	}
    }
    if (lseek(fd, 0, SEEK_SET) == -1) {
	sudo_warn("lseek");
	debug_return_int(-1);
    }

    debug_return_int(0);
}

/*
 * Copy the contents of src_fd into dst_fd.
 * Returns 0 on success or -1 on error.
 */
int
sudo_copy_file(const char *src, int src_fd, off_t src_len, const char *dst,
    int dst_fd, off_t dst_len)
{
    char buf[BUFSIZ];
    ssize_t nwritten, nread;
    debug_decl(sudo_copy_file, SUDO_DEBUG_UTIL);

    /* Prompt the user before zeroing out an existing file. */
    if (dst_len > 0 && src_len == 0) {
	fprintf(stderr, U_("%s: truncate %s to zero bytes? (y/n) [n] "),
	    getprogname(), dst);
	if (fgets(buf, sizeof(buf), stdin) == NULL ||
		(buf[0] != 'y' && buf[0] != 'Y')) {
	    sudo_warnx(U_("not overwriting %s"), dst);
	    debug_return_int(0);
	}
    }

    /* Extend the file to the new size if larger before copying. */
    if (dst_len > 0 && src_len > dst_len) {
	if (sudo_extend_file(dst_fd, dst, src_len) == -1)
	    goto write_error;
    }

    /* Overwrite the old file with the new contents. */
    while ((nread = read(src_fd, buf, sizeof(buf))) > 0) {
	ssize_t off = 0;
	do {
	    nwritten = write(dst_fd, buf + off, (size_t)(nread - off));
	    if (nwritten == -1)
		goto write_error;
	    off += nwritten;
	} while (nread > off);
    }
    if (nread == -1) {
	sudo_warn(U_("unable to read from %s"), src);
	debug_return_int(-1);
    }

    /* Did the file shrink? */
    if (src_len < dst_len) {
	/* We don't open with O_TRUNC so must truncate manually. */
	if (ftruncate(dst_fd, src_len) == -1) {
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"unable to truncate %s to %lld", dst, (long long)src_len);
	    goto write_error;
	}
    }

    debug_return_int(0);
write_error:
    sudo_warn(U_("unable to write to %s"), dst);
    debug_return_int(-1);
}

bool
sudo_check_temp_file(int tfd, const char *tfile, uid_t uid, struct stat *sb)
{
    struct stat sbuf;
    debug_decl(sudo_check_temp_file, SUDO_DEBUG_UTIL);

    if (sb == NULL)
	sb = &sbuf;

    if (fstat(tfd, sb) == -1) {
	sudo_warn(U_("unable to stat %s"), tfile);
	debug_return_bool(false);
    }
    if (!S_ISREG(sb->st_mode)) {
	sudo_warnx(U_("%s: not a regular file"), tfile);
	debug_return_bool(false);
    }
    if ((sb->st_mode & ALLPERMS) != (S_IRUSR|S_IWUSR)) {
	sudo_warnx(U_("%s: bad file mode: 0%o"), tfile,
	    (unsigned int)(sb->st_mode & ALLPERMS));
	debug_return_bool(false);
    }
    if (sb->st_uid != uid) {
	sudo_warnx(U_("%s is owned by uid %u, should be %u"),
	    tfile, (unsigned int)sb->st_uid, (unsigned int)uid);
	debug_return_bool(false);
    }
    debug_return_bool(true);
}

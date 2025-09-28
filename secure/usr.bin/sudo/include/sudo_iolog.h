/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_IOLOG_H
#define SUDO_IOLOG_H

#include <sys/types.h>	/* for gid_t, mode_t, size_t, ssize_t, uid_t */

#ifdef HAVE_ZLIB_H
# include <zlib.h>	/* for gzFile */
#endif

/* Default maximum session ID */
#define SESSID_MAX	2176782336U

/* Default value for "iolog_file" */
#define IOLOG_FILE	"%{seq}"

/*
 * I/O log event types as stored as the first field in the timing file.
 * Changing existing values will result in incompatible I/O log files.
 */
#define IO_EVENT_STDIN		0
#define IO_EVENT_STDOUT		1
#define IO_EVENT_STDERR		2
#define IO_EVENT_TTYIN		3
#define IO_EVENT_TTYOUT		4
#define IO_EVENT_WINSIZE	5
#define IO_EVENT_TTYOUT_1_8_7	6
#define IO_EVENT_SUSPEND	7
#define IO_EVENT_COUNT		8

/*
 * Indexes into iolog_files[] array.
 * These must match the IO_EVENT_ defines above.
 * TODO: eliminate use of IOFD_* and IO_EVENT_* as indexes in favor of
 *       a struct containing iolog_file *s for each (and names too?).
 */
#define IOFD_STDIN	0
#define IOFD_STDOUT	1
#define IOFD_STDERR	2
#define IOFD_TTYIN	3
#define IOFD_TTYOUT	4
#define IOFD_TIMING	5
#define IOFD_MAX	6

/*
 * Default password prompt regex.
 */
#define PASSPROMPT_REGEX	"[Pp]assword[: ]*"

struct timing_closure {
    struct timespec delay;
    const char *decimal;
    struct iolog_file *iol;
    int event;
    union {
	struct {
	    int lines;
	    int cols;
	} winsize;
	size_t nbytes;
	int signo;
    } u;
};

struct iolog_file {
    bool enabled;
    bool compressed;
    bool writable;
    union {
	FILE *f;
#ifdef HAVE_ZLIB_H
	gzFile g;
#endif
	void *v;
    } fd;
};

struct iolog_path_escape {
    const char *name;
    size_t (*copy_fn)(char *, size_t, void *);
};

/* host_port.c */
bool iolog_parse_host_port(char *str, char **hostp, char **portp, bool *tlsp, const char *defport, const char *defport_tls);

/* iolog_path.c */
bool expand_iolog_path(const char *inpath, char *path, size_t pathlen, const struct iolog_path_escape *escapes, void *closure);

/* iolog_util.c */
bool iolog_parse_timing(const char *line, struct timing_closure *timing);
char *iolog_parse_delay(const char *cp, struct timespec *delay, const char *decimal_point);
int iolog_read_timing_record(struct iolog_file *iol, struct timing_closure *timing);
struct eventlog *iolog_parse_loginfo(int dfd, const char *iolog_dir);
bool iolog_parse_loginfo_json(FILE *fp, const char *iolog_dir, struct eventlog *evlog);
bool iolog_parse_loginfo_legacy(FILE *fp, const char *iolog_dir, struct eventlog *evlog);
void iolog_adjust_delay(struct timespec *delay, struct timespec *max_delay, double scale_factor);

/* iolog_fileio.c */
struct passwd;
struct group;
bool iolog_close(struct iolog_file *iol, const char **errstr);
bool iolog_eof(struct iolog_file *iol);
bool iolog_mkdtemp(char *path);
bool iolog_mkpath(char *path);
bool iolog_nextid(const char *iolog_dir, char sessid[7]);
bool iolog_open(struct iolog_file *iol, int dfd, int iofd, const char *mode);
bool iolog_write_info_file(int dfd, struct eventlog *evlog);
char *iolog_gets(struct iolog_file *iol, char *buf, int bufsize, const char **errsttr);
const char *iolog_fd_to_name(int iofd);
int iolog_openat(int fdf, const char *path, int flags);
off_t iolog_seek(struct iolog_file *iol, off_t offset, int whence);
ssize_t iolog_read(struct iolog_file *iol, void *buf, size_t nbytes, const char **errstr);
ssize_t iolog_write(struct iolog_file *iol, const void *buf, size_t len, const char **errstr);
void iolog_clearerr(struct iolog_file *iol);
bool iolog_flush(struct iolog_file *iol, const char **errstr);
void iolog_rewind(struct iolog_file *iol);
unsigned int iolog_get_maxseq(void);
uid_t iolog_get_uid(void);
gid_t iolog_get_gid(void);
mode_t iolog_get_file_mode(void);
mode_t iolog_get_dir_mode(void);
bool iolog_get_compress(void);
bool iolog_get_flush(void);
void iolog_set_compress(bool);
void iolog_set_defaults(void);
void iolog_set_flush(bool);
void iolog_set_gid(gid_t gid);
void iolog_set_maxseq(unsigned int maxval);
void iolog_set_mode(mode_t mode);
void iolog_set_owner(uid_t uid, uid_t gid);
bool iolog_swapids(bool restore);
bool iolog_mkdirs(const char *path);

/* iolog_filter.c */
void *iolog_pwfilt_alloc(void);
bool iolog_pwfilt_add(void *handle, const char *pattern);
void iolog_pwfilt_free(void *handle);
bool iolog_pwfilt_remove(void *handle, const char *pattern);
bool iolog_pwfilt_run(void *handle, int event, const char *buf, size_t len, char **newbuf);

#endif /* SUDO_IOLOG_H */

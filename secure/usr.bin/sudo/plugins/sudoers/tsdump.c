/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2018-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>

#include <sudoers.h>
#include <timestamp.h>

struct timestamp_entry_common {
    unsigned short version;	/* version number */
    unsigned short size;	/* entry size */
    unsigned short type;	/* TS_GLOBAL, TS_TTY, TS_PPID */
    unsigned short flags;	/* TS_DISABLED, TS_ANYUID */
};

union timestamp_entry_storage {
    struct timestamp_entry_common common;
    struct timestamp_entry_v1 v1;
    struct timestamp_entry v2;
};

sudo_dso_public int main(int argc, char *argv[]);

static void dump_entry(struct timestamp_entry *entry, off_t pos);
static bool valid_entry(union timestamp_entry_storage *u, off_t pos);
static bool convert_entry(union timestamp_entry_storage *record, struct timespec *off);
sudo_noreturn static void usage(void);

/*
 * tsdump: a simple utility to dump the contents of a time stamp file.
 * Unlock sudo, does not perform any locking of the time stamp file.
 */

int
main(int argc, char *argv[])
{
    int ch, fd;
    const char *user = NULL;
    char *fname = NULL;
    union timestamp_entry_storage cur;
    struct timespec now, timediff;
    debug_decl(main, SUDOERS_DEBUG_MAIN);

#if defined(SUDO_DEVEL) && defined(__OpenBSD__)
    malloc_options = "S";
#endif

    initprogname(argc > 0 ? argv[0] : "tsdump");

    bindtextdomain("sudoers", LOCALEDIR);
    textdomain("sudoers");

    /* Initialize the debug subsystem. */
    if (sudo_conf_read(NULL, SUDO_CONF_DEBUG) == -1)
	return EXIT_FAILURE;
    sudoers_debug_register(getprogname(), sudo_conf_debug_files(getprogname()));

    while ((ch = getopt(argc, argv, "f:u:")) != -1) {
	switch (ch) {
	    case 'f':
		fname = optarg;
		break;
	    case 'u':
		user = optarg;
		break;
	    default:
		usage();
	}
    }
    argc -= optind;
    argv += optind;

    if (fname != NULL && user != NULL) {
	sudo_warnx("the -f and -u flags are mutually exclusive");
	usage();
    }

    /* Calculate the difference between real time and mono time. */
    if (sudo_gettime_real(&now) == -1)
	sudo_fatal("unable to get current time");
    if (sudo_gettime_mono(&timediff) == -1)
	sudo_fatal("unable to read the clock");
    sudo_timespecsub(&now, &timediff, &timediff);

    if (fname == NULL) {
	struct passwd *pw;

	if (user == NULL) {
	    if ((pw = getpwuid(geteuid())) == NULL)
		sudo_fatalx(U_("unknown uid %u"), (unsigned int)geteuid());
	    user = pw->pw_name;
	}
	if (asprintf(&fname, "%s/%s", _PATH_SUDO_TIMEDIR, user) == -1)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    }

    fd = open(fname, O_RDONLY);
    if (fd == -1)
	sudo_fatal(U_("unable to open %s"), fname);

    for (;;) {
	off_t pos = lseek(fd, 0, SEEK_CUR);
	ssize_t nread;
	bool valid;

	if ((nread = read(fd, &cur, sizeof(cur))) == 0)
	    break;
	if (nread == -1)
	    sudo_fatal(U_("unable to read %s"), fname);

	valid = valid_entry(&cur, pos);
	if (cur.common.size != 0 && cur.common.size != sizeof(cur)) {
	    off_t offset = (off_t)cur.common.size - (off_t)sizeof(cur);
	    if (lseek(fd, offset, SEEK_CUR) == -1)
		sudo_fatal("unable to seek %d bytes", (int)offset);
	}
	if (valid) {
	    /* Convert entry to latest version as needed. */
	    if (!convert_entry(&cur, &timediff))
		continue;
	    dump_entry(&cur.v2, pos);
	}
    }

    return 0;
}

static bool
valid_entry(union timestamp_entry_storage *u, off_t pos)
{
    struct timestamp_entry *entry = (struct timestamp_entry *)u;
    debug_decl(valid_entry, SUDOERS_DEBUG_UTIL);

    switch (entry->version) {
    case 1:
	if (entry->size != sizeof(struct timestamp_entry_v1)) {
	    sudo_warn("wrong sized v1 record @ %lld, got %hu, expected %zu",
		(long long)pos, entry->size, sizeof(struct timestamp_entry_v1));
	    debug_return_bool(false);
	}
	break;
    case 2:
	if (entry->size != sizeof(struct timestamp_entry)) {
	    sudo_warn("wrong sized v2 record @ %lld, got %hu, expected %zu",
		(long long)pos, entry->size, sizeof(struct timestamp_entry));
	    debug_return_bool(false);
	}
	break;
    default:
	sudo_warn("unknown time stamp entry version %d @ %lld",
	    (int)entry->version, (long long)pos);
	debug_return_bool(false);
	break;
    }
    debug_return_bool(true);
}

static const char *
type2string(int type)
{
    static char name[64];
    debug_decl(type2string, SUDOERS_DEBUG_UTIL);

    switch (type) {
    case TS_LOCKEXCL:
	debug_return_const_str("TS_LOCKEXCL");
    case TS_GLOBAL:
	debug_return_const_str("TS_GLOBAL");
    case TS_TTY:
	debug_return_const_str("TS_TTY");
    case TS_PPID:
	debug_return_const_str("TS_PPID");
    }
    (void)snprintf(name, sizeof(name), "UNKNOWN (0x%x)", type);
    debug_return_const_str(name);
}

static void
print_flags(unsigned int flags)
{
    bool first = true;
    debug_decl(print_flags, SUDOERS_DEBUG_UTIL);

    printf("flags: ");
    if (ISSET(flags, TS_DISABLED)) {
	printf("%sTS_DISABLED", first ? "" : ", ");
	CLR(flags, TS_DISABLED);
	first = false;
    }
    if (ISSET(flags, TS_ANYUID)) {
	/* TS_ANYUID should never appear on disk. */
	printf("%sTS_ANYUID", first ? "" : ", ");
	CLR(flags, TS_ANYUID);
	first = false;
    }
    if (flags != 0)
	printf("%s0x%x", first ? "" : ", ", flags);
    putchar('\n');

    debug_return;
}

/*
 * Convert an older entry to current.
 * Also adjusts time stamps on Linux to be wallclock time.
 */
static bool
convert_entry(union timestamp_entry_storage *record, struct timespec *off)
{
    union timestamp_entry_storage orig;
    debug_decl(convert_entry, SUDOERS_DEBUG_UTIL);

    if (record->common.version != TS_VERSION) {
	if (record->common.version != 1) {
	    sudo_warnx("unexpected record version %hu", record->common.version);
	    debug_return_bool(false);
	}

	/* The first four fields are the same regardless of version. */
	memcpy(&orig, record, sizeof(union timestamp_entry_storage));
	record->v2.auth_uid = orig.v1.auth_uid;
	record->v2.sid = orig.v1.sid;
	sudo_timespecclear(&record->v2.start_time);
	record->v2.ts = orig.v1.ts;
	if (record->common.type == TS_TTY)
	    record->v2.u.ttydev = orig.v1.u.ttydev;
	else if (record->common.type == TS_PPID)
	    record->v2.u.ppid = orig.v1.u.ppid;
	else
	    memset(&record->v2.u, 0, sizeof(record->v2.u));
    }

    /* On Linux, start time is relative to boot time, adjust to real time. */
#ifdef __linux__
    if (sudo_timespecisset(&record->v2.start_time))
	sudo_timespecadd(&record->v2.start_time, off, &record->v2.start_time);
#endif

    /* Adjust time stamp from mono time to real time. */
    if (sudo_timespecisset(&record->v2.ts))
	sudo_timespecadd(&record->v2.ts, off, &record->v2.ts);

    debug_return_bool(true);
}

static void
dump_entry(struct timestamp_entry *entry, off_t pos)
{
    debug_decl(dump_entry, SUDOERS_DEBUG_UTIL);

    printf("position: %lld\n", (long long)pos);
    printf("version: %hu\n", entry->version);
    printf("size: %hu\n", entry->size);
    printf("type: %s\n", type2string(entry->type));
    print_flags(entry->flags);
    printf("auth uid: %d\n", (int)entry->auth_uid);
    printf("session ID: %d\n", (int)entry->sid);
    if (sudo_timespecisset(&entry->start_time))
	printf("start time: %s", ctime(&entry->start_time.tv_sec));
    if (sudo_timespecisset(&entry->ts))
	printf("time stamp: %s", ctime(&entry->ts.tv_sec));
    if (entry->type == TS_TTY) {
	char tty[PATH_MAX];
	if (sudo_ttyname_dev(entry->u.ttydev, tty, sizeof(tty)) == NULL)
	    printf("terminal: %d\n", (int)entry->u.ttydev);
	else
	    printf("terminal: %s\n", tty);
    } else if (entry->type == TS_PPID) {
	printf("parent pid: %d\n", (int)entry->u.ppid);
    }
    fputc('\n', stdout);

    debug_return;
}

sudo_noreturn static void
usage(void)
{
    fprintf(stderr, "usage: %s [-f timestamp_file] | [-u username]\n",
	getprogname());
    exit(EXIT_FAILURE);
}

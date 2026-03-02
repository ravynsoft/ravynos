/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif

#include <pathnames.h>
#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>
#include <sudo_iolog.h>

static unsigned int sessid_max = SESSID_MAX;
static mode_t iolog_filemode = S_IRUSR|S_IWUSR;
static mode_t iolog_dirmode = S_IRWXU;
static uid_t iolog_uid = ROOT_UID;
static gid_t iolog_gid = ROOT_GID;
static bool iolog_gid_set;
static bool iolog_docompress;
static bool iolog_doflush;

/*
 * Reset I/O log settings to default values.
 */
void
iolog_set_defaults(void)
{
    sessid_max = SESSID_MAX;
    iolog_filemode = S_IRUSR|S_IWUSR;
    iolog_dirmode = S_IRWXU;
    iolog_uid = ROOT_UID;
    iolog_gid = ROOT_GID;
    iolog_gid_set = false;
    iolog_docompress = false;
    iolog_doflush = false;
}

/*
 * Set max sequence number (aka session ID)
 */
void
iolog_set_maxseq(unsigned int newval)
{
    debug_decl(iolog_set_maxseq, SUDO_DEBUG_UTIL);

    /* Clamp to SESSID_MAX as documented. */
    if (newval > SESSID_MAX)
	newval = SESSID_MAX;
    sessid_max = newval;

    debug_return;
}

/*
 * Set iolog_uid (and iolog_gid if gid not explicitly set).
 */
void
iolog_set_owner(uid_t uid, gid_t gid)
{
    debug_decl(iolog_set_owner, SUDO_DEBUG_UTIL);

    iolog_uid = uid;
    if (!iolog_gid_set)
	iolog_gid = gid;

    debug_return;
}

/*
 * Set iolog_gid.
 */
void
iolog_set_gid(gid_t gid)
{
    debug_decl(iolog_set_gid, SUDO_DEBUG_UTIL);

    iolog_gid = gid;
    iolog_gid_set = true;

    debug_return;
}

/*
 * Set iolog_filemode and iolog_dirmode.
 */
void
iolog_set_mode(mode_t mode)
{
    debug_decl(iolog_set_mode, SUDO_DEBUG_UTIL);

    /* I/O log files must be readable and writable by owner. */
    iolog_filemode = S_IRUSR|S_IWUSR;

    /* Add in group and other read/write if specified. */
    iolog_filemode |= mode & (S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

    /* For directory mode, add execute bits as needed. */
    iolog_dirmode = iolog_filemode | S_IXUSR;
    if (iolog_dirmode & (S_IRGRP|S_IWGRP))
	iolog_dirmode |= S_IXGRP;
    if (iolog_dirmode & (S_IROTH|S_IWOTH))
	iolog_dirmode |= S_IXOTH;

    debug_return;
}

/*
 * Set iolog_docompress
 */
void
iolog_set_compress(bool newval)
{
    debug_decl(iolog_set_compress, SUDO_DEBUG_UTIL);
    iolog_docompress = newval;
    debug_return;
}

/*
 * Set iolog_doflush
 */
void
iolog_set_flush(bool newval)
{
    debug_decl(iolog_set_flush, SUDO_DEBUG_UTIL);
    iolog_doflush = newval;
    debug_return;
}

/*
 * Getters.
 */

unsigned int
iolog_get_maxseq(void)
{
    return sessid_max;
}

uid_t
iolog_get_uid(void)
{
    return iolog_uid;
}

gid_t
iolog_get_gid(void)
{
    return iolog_gid;
}

mode_t
iolog_get_file_mode(void)
{
    return iolog_filemode;
}

mode_t
iolog_get_dir_mode(void)
{
    return iolog_dirmode;
}

bool
iolog_get_compress(void)
{
    return iolog_docompress;
}

bool
iolog_get_flush(void)
{
    return iolog_doflush;
}

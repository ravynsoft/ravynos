/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_EDIT_H
#define SUDO_EDIT_H

/*
 * Directory open flags for use with openat(2).
 * Use O_SEARCH/O_PATH and/or O_DIRECTORY where possible.
 */
#if defined(O_SEARCH)
# if defined(O_DIRECTORY)
#  define DIR_OPEN_FLAGS	(O_SEARCH|O_DIRECTORY)
# else
#  define DIR_OPEN_FLAGS	(O_SEARCH)
# endif
#elif defined(O_PATH)
# if defined(O_DIRECTORY)
#  define DIR_OPEN_FLAGS	(O_PATH|O_DIRECTORY)
# else
#  define DIR_OPEN_FLAGS	(O_PATH)
# endif
#elif defined(O_DIRECTORY)
# define DIR_OPEN_FLAGS		(O_RDONLY|O_DIRECTORY)
#else
# define DIR_OPEN_FLAGS		(O_RDONLY|O_NONBLOCK)
#endif

/* copy_file.c */
int sudo_copy_file(const char *src, int src_fd, off_t src_len, const char *dst, int dst_fd, off_t dst_len);
bool sudo_check_temp_file(int tfd, const char *tname, uid_t uid, struct stat *sb);

/* edit_open.c */
struct sudo_cred;
void switch_user(uid_t euid, gid_t egid, int ngroups, GETGROUPS_T *groups);
int sudo_edit_open(char *path, int oflags, mode_t mode, unsigned int sflags, const struct sudo_cred *user_cred, const struct sudo_cred *cur_cred);
int dir_is_writable(int dfd, const struct sudo_cred *user_cred, const struct sudo_cred *cur_cred);
bool sudo_edit_parent_valid(char *path, unsigned int sflags, const struct sudo_cred *user_cred, const struct sudo_cred *cur_cred);

#endif /* SUDO_EDIT_H */

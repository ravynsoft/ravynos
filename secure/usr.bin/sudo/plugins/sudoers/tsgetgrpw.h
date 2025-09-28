/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010, 2021 Todd C. Miller <Todd.Miller@sudo.ws>
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
 * Trivial replacements for the libc getgrent() and getpwent() family
 * of functions for use by testsudoers in the sudo test harness.
 * We need our own since many platforms don't provide set{pw,gr}file().
 */

#include <config.h>

#include <pwd.h>
#include <grp.h>

void testsudoers_setgrfile(const char *);
void testsudoers_setgrent(void);
void testsudoers_endgrent(void);
int testsudoers_setgroupent(int);
struct group *testsudoers_getgrent(void);
struct group *testsudoers_getgrnam(const char *);
struct group *testsudoers_getgrgid(gid_t);

void testsudoers_setpwfile(const char *);
void testsudoers_setpwent(void);
void testsudoers_endpwent(void);
int testsudoers_setpassent(int);
struct passwd *testsudoers_getpwent(void);
struct passwd *testsudoers_getpwnam(const char *);
struct passwd *testsudoers_getpwuid(uid_t);

char *testsudoers_getusershell(void);
void testsudoers_setusershell(void);
void testsudoers_endusershell(void);
void testsudoers_setshellfile(const char *file);

int testsudoers_getgrouplist2(const char *name, GETGROUPS_T basegid,
    GETGROUPS_T **groupsp, int *ngroupsp);

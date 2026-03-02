/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2015 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifdef HAVE_PROJECT_H
# include <project.h>
# include <sys/task.h>
# include <errno.h>
# include <pwd.h>
#endif

#include <sudo.h>
#include <sudo_dso.h>

int
os_init(int argc, char *argv[], char *envp[])
{
    /*
     * Solaris 11 is unable to load the per-locale shared objects
     * without this.  We must keep the handle open for it to work.
     * This bug was fixed in Solaris 11 Update 1.
     */
    void *handle = sudo_dso_load("/usr/lib/locale/common/methods_unicode.so.3",
	SUDO_DSO_LAZY|SUDO_DSO_GLOBAL);
    (void)&handle;

    return os_init_common(argc, argv, envp);
}

#ifdef HAVE_PROJECT_H
void
set_project(struct passwd *pw)
{
    struct project proj;
    char buf[PROJECT_BUFSZ];
    int errval;
    debug_decl(set_project, SUDO_DEBUG_UTIL);

    /*
     * Collect the default project for the user and settaskid
     */
    setprojent();
    if (getdefaultproj(pw->pw_name, &proj, buf, sizeof(buf)) != NULL) {
	errval = setproject(proj.pj_name, pw->pw_name, TASK_NORMAL);
	switch(errval) {
	case 0:
	    break;
	case SETPROJ_ERR_TASK:
	    switch (errno) {
	    case EAGAIN:
		sudo_warnx("%s", U_("resource control limit has been reached"));
		break;
	    case ESRCH:
		sudo_warnx(U_("user \"%s\" is not a member of project \"%s\""),
		    pw->pw_name, proj.pj_name);
		break;
	    case EACCES:
		sudo_warnx("%s", U_("the invoking task is final"));
		break;
	    default:
		sudo_warnx(U_("could not join project \"%s\""), proj.pj_name);
		break;
	    }
	    break;
	case SETPROJ_ERR_POOL:
	    switch (errno) {
	    case EACCES:
		sudo_warnx(U_("no resource pool accepting default bindings "
		    "exists for project \"%s\""), proj.pj_name);
		break;
	    case ESRCH:
		sudo_warnx(U_("specified resource pool does not exist for "
		    "project \"%s\""), proj.pj_name);
		break;
	    default:
		sudo_warnx(U_("could not bind to default resource pool for "
		    "project \"%s\""), proj.pj_name);
		break;
	    }
	    break;
	default:
	    if (errval <= 0) {
		sudo_warnx(U_("setproject failed for project \"%s\""), proj.pj_name);
	    } else {
		sudo_warnx(U_("warning, resource control assignment failed for "
		    "project \"%s\""), proj.pj_name);
	    }
	    break;
	}
    } else {
	sudo_warn("getdefaultproj");
    }
    endprojent();
    debug_return;
}
#endif /* HAVE_PROJECT_H */

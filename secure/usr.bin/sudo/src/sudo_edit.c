/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004-2008, 2010-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <fcntl.h>

#include <sudo.h>
#include <sudo_edit.h>
#include <sudo_exec.h>

#if defined(HAVE_SETRESUID) || defined(HAVE_SETREUID) || defined(HAVE_SETEUID)

/*
 * Editor temporary file name along with original name, mtime and size.
 */
struct tempfile {
    char *tfile;
    char *ofile;
    off_t osize;
    struct timespec omtim;
};

static char edit_tmpdir[MAX(sizeof(_PATH_VARTMP), sizeof(_PATH_TMP))];

/*
 * Find our temporary directory, one of /var/tmp, /usr/tmp, or /tmp
 * Returns true on success, else false;
 */
static bool
set_tmpdir(const struct sudo_cred *user_cred)
{
    const char *tdir = NULL;
    const char *tmpdirs[] = {
	_PATH_VARTMP,
#ifdef _PATH_USRTMP
	_PATH_USRTMP,
#endif
	_PATH_TMP
    };
    struct sudo_cred saved_cred;
    size_t i;
    size_t len;
    int dfd;
    debug_decl(set_tmpdir, SUDO_DEBUG_EDIT);

    /* Stash old credentials. */
    saved_cred.uid = getuid();
    saved_cred.euid = geteuid();
    saved_cred.gid = getgid();
    saved_cred.egid = getegid();
    saved_cred.ngroups = getgroups(0, NULL); // -V575
    if (saved_cred.ngroups > 0) {
	saved_cred.groups =
	    reallocarray(NULL, (size_t)saved_cred.ngroups, sizeof(GETGROUPS_T));
	if (saved_cred.groups == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_bool(false);
	}
	saved_cred.ngroups = getgroups(saved_cred.ngroups, saved_cred.groups);
	if (saved_cred.ngroups < 0) {
	    sudo_warn("%s", U_("unable to get group list"));
	    free(saved_cred.groups);
	    debug_return_bool(false);
	}
    } else {
	saved_cred.ngroups = 0;
	saved_cred.groups = NULL;
    }

    for (i = 0; tdir == NULL && i < nitems(tmpdirs); i++) {
	if ((dfd = open(tmpdirs[i], O_RDONLY)) != -1) {
	    if (dir_is_writable(dfd, user_cred, &saved_cred) == true)
		tdir = tmpdirs[i];
	    close(dfd);
	}
    }
    free(saved_cred.groups);

    if (tdir == NULL) {
	sudo_warnx("%s", U_("no writable temporary directory found"));
	debug_return_bool(false);
    }

    len = strlcpy(edit_tmpdir, tdir, sizeof(edit_tmpdir));
    if (len >= sizeof(edit_tmpdir)) {
	errno = ENAMETOOLONG;
	sudo_warn("%s", tdir);
	debug_return_bool(false);
    }
    while (len > 0 && edit_tmpdir[--len] == '/')
	edit_tmpdir[len] = '\0';
    debug_return_bool(true);
}

/*
 * Construct a temporary file name for file and return an
 * open file descriptor.  The temporary file name is stored
 * in tfile which the caller is responsible for freeing.
 */
static int
sudo_edit_mktemp(const char *ofile, char **tfile)
{
    const char *base, *suff;
    int len, tfd;
    debug_decl(sudo_edit_mktemp, SUDO_DEBUG_EDIT);

    base = sudo_basename(ofile);
    suff = strrchr(base, '.');
    if (suff != NULL) {
	len = asprintf(tfile, "%s/%.*sXXXXXXXX%s", edit_tmpdir,
	    (int)(size_t)(suff - base), base, suff);
    } else {
	len = asprintf(tfile, "%s/%s.XXXXXXXX", edit_tmpdir, base);
    }
    if (len == -1) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_int(-1);
    }
    tfd = mkstemps(*tfile, suff ? (int)strlen(suff) : 0);
    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"%s -> %s, fd %d", ofile, *tfile, tfd);
    debug_return_int(tfd);
}

/*
 * Create temporary copies of files[] and store the temporary path name
 * along with the original name, size and mtime in tf.
 * Returns the number of files copied (which may be less than nfiles)
 * or -1 if a fatal error occurred.
 */
static int
sudo_edit_create_tfiles(const struct command_details *command_details,
    const struct sudo_cred *user_cred, struct tempfile *tf, char *files[],
    int nfiles)
{
    int i, j, tfd, ofd, rc;
    struct timespec times[2];
    struct stat sb;
    debug_decl(sudo_edit_create_tfiles, SUDO_DEBUG_EDIT);

    /*
     * For each file specified by the user, make a temporary version
     * and copy the contents of the original to it.
     */
    for (i = 0, j = 0; i < nfiles; i++) {
	rc = -1;
	switch_user(command_details->cred.euid, command_details->cred.egid,
	    command_details->cred.ngroups, command_details->cred.groups);
	ofd = sudo_edit_open(files[i], O_RDONLY,
	    S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, command_details->flags,
	    user_cred, &command_details->cred);
	if (ofd != -1 || errno == ENOENT) {
	    if (ofd != -1) {
		rc = fstat(ofd, &sb);
	    } else {
		/* New file, verify parent dir exists and is not writable. */
		memset(&sb, 0, sizeof(sb));
		if (sudo_edit_parent_valid(files[i], command_details->flags, user_cred, &command_details->cred))
		    rc = 0;
	    }
	}
	switch_user(ROOT_UID, user_cred->egid, user_cred->ngroups, user_cred->groups);
	if (ofd != -1 && !S_ISREG(sb.st_mode)) {
	    sudo_warnx(U_("%s: not a regular file"), files[i]);
	    close(ofd);
	    continue;
	}
	if (rc == -1) {
	    /* open() or fstat() error. */
	    if (ofd == -1 && errno == ELOOP) {
		sudo_warnx(U_("%s: editing symbolic links is not permitted"),
		    files[i]);
	    } else if (ofd == -1 && errno == EISDIR) {
		sudo_warnx(U_("%s: editing files in a writable directory is not permitted"),
		    files[i]);
	    } else {
		sudo_warn("%s", files[i]);
	    }
	    if (ofd != -1)
		close(ofd);
	    continue;
	}
	tf[j].ofile = files[i];
	tf[j].osize = sb.st_size; // -V614
	mtim_get(&sb, tf[j].omtim);
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "seteuid(%u)", (unsigned int)user_cred->uid);
	if (seteuid(user_cred->uid) != 0)
	    sudo_fatal("seteuid(%u)", (unsigned int)user_cred->uid);
	tfd = sudo_edit_mktemp(tf[j].ofile, &tf[j].tfile);
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "seteuid(%u)", ROOT_UID);
	if (seteuid(ROOT_UID) != 0)
	    sudo_fatal("seteuid(ROOT_UID)");
	if (tfd == -1) {
	    sudo_warn("mkstemps");
	    if (ofd != -1)
		close(ofd);
	    debug_return_int(-1);
	}
	if (ofd != -1) {
	    if (sudo_copy_file(tf[j].ofile, ofd, tf[j].osize, tf[j].tfile, tfd, -1) == -1) {
		close(ofd);
		close(tfd);
		debug_return_int(-1);
	    }
	    close(ofd);
	}
	/*
	 * We always update the stashed mtime because the time
	 * resolution of the filesystem the temporary file is on may
	 * not match that of the filesystem where the file to be edited
	 * resides.  It is OK if futimens() fails since we only use the
	 * info to determine whether or not a file has been modified.
	 */
	times[0].tv_sec = times[1].tv_sec = tf[j].omtim.tv_sec;
	times[0].tv_nsec = times[1].tv_nsec = tf[j].omtim.tv_nsec;
	if (futimens(tfd, times) == -1) {
	    if (utimensat(AT_FDCWD, tf[j].tfile, times, 0) == -1)
		sudo_warn("%s", tf[j].tfile);
	}
	rc = fstat(tfd, &sb);
	if (!rc)
	    mtim_get(&sb, tf[j].omtim);
	close(tfd);
	j++;
    }
    debug_return_int(j);
}

/*
 * Copy the temporary files specified in tf to the originals.
 * Returns the number of copy errors or 0 if completely successful.
 */
static int
sudo_edit_copy_tfiles(const struct command_details *command_details,
    const struct sudo_cred *user_cred, struct tempfile *tf,
    int nfiles, struct timespec *times)
{
    int i, tfd, ofd, errors = 0;
    struct timespec ts;
    struct stat sb;
    mode_t oldmask;
    debug_decl(sudo_edit_copy_tfiles, SUDO_DEBUG_EDIT);

    /* Copy contents of temp files to real ones. */
    for (i = 0; i < nfiles; i++) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "seteuid(%u)", (unsigned int)user_cred->uid);
	if (seteuid(user_cred->uid) != 0)
	    sudo_fatal("seteuid(%u)", (unsigned int)user_cred->uid);
	tfd = sudo_edit_open(tf[i].tfile, O_RDONLY,
	    S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, 0, user_cred, NULL);
	if (seteuid(ROOT_UID) != 0)
	    sudo_fatal("seteuid(ROOT_UID)");
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "seteuid(%u)", ROOT_UID);
	if (tfd == -1 || !sudo_check_temp_file(tfd, tf[i].tfile, user_cred->uid, &sb)) {
	    sudo_warnx(U_("%s left unmodified"), tf[i].ofile);
	    if (tfd != -1)
		close(tfd);
	    errors++;
	    continue;
	}
	mtim_get(&sb, ts);
	if (tf[i].osize == sb.st_size && sudo_timespeccmp(&tf[i].omtim, &ts, ==)) {
	    /*
	     * If mtime and size match but the user spent no measurable
	     * time in the editor we can't tell if the file was changed.
	     */
	    if (sudo_timespeccmp(&times[0], &times[1], !=)) {
		sudo_warnx(U_("%s unchanged"), tf[i].ofile);
		unlink(tf[i].tfile);
		close(tfd);
		continue;
	    }
	}
	switch_user(command_details->cred.euid, command_details->cred.egid,
	    command_details->cred.ngroups, command_details->cred.groups);
	oldmask = umask(command_details->umask);
	ofd = sudo_edit_open(tf[i].ofile, O_WRONLY|O_CREAT,
	    S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, command_details->flags,
	    user_cred, &command_details->cred);
	umask(oldmask);
	switch_user(ROOT_UID, user_cred->egid, user_cred->ngroups, user_cred->groups);
	if (ofd == -1) {
	    sudo_warn(U_("unable to write to %s"), tf[i].ofile);
	    goto bad;
	}

	/* Overwrite the old file with the new contents. */
	if (sudo_copy_file(tf[i].tfile, tfd, sb.st_size, tf[i].ofile, ofd,
		tf[i].osize) == 0) {
	    /* success, remove temporary file. */
	    unlink(tf[i].tfile);
	} else {
bad:
	    sudo_warnx(U_("contents of edit session left in %s"), tf[i].tfile);
	    errors++;
	}

	if (ofd != -1)
	    close(ofd);
	close(tfd);
    }
    debug_return_int(errors);
}

#ifdef HAVE_SELINUX
static int
selinux_run_helper(uid_t uid, gid_t gid, int ngroups, GETGROUPS_T *groups,
    char *const argv[], char *const envp[])
{
    int status, ret = SESH_ERR_FAILURE;
    const char *sesh;
    pid_t child, pid;
    debug_decl(selinux_run_helper, SUDO_DEBUG_EDIT);

    sesh = sudo_conf_sesh_path();
    if (sesh == NULL) {
	sudo_warnx("internal error: sesh path not set");
	debug_return_int(-1);
    }

    child = sudo_debug_fork();
    switch (child) {
    case -1:
	sudo_warn("%s", U_("unable to fork"));
	break;
    case 0:
	/* child runs sesh in new context */
	if (selinux_setexeccon() == 0) {
	    switch_user(uid, gid, ngroups, groups);
	    execve(sesh, argv, envp);
	}
	_exit(SESH_ERR_FAILURE);
    default:
	/* parent waits */
	do {
	    pid = waitpid(child, &status, 0);
	} while (pid == -1 && errno == EINTR);

	ret = WIFSIGNALED(status) ? SESH_ERR_KILLED : WEXITSTATUS(status);
    }

    debug_return_int(ret);
}

static char *
selinux_fmt_sudo_user(const struct sudo_cred *user_cred)
{
    char *cp, *user_str;
    size_t user_size;
    int i, len;
    debug_decl(selinux_fmt_sudo_user, SUDO_DEBUG_EDIT);

    user_size = (STRLEN_MAX_UNSIGNED(uid_t) + 1) * (2 + user_cred->ngroups);
    if ((user_str = malloc(user_size)) == NULL)
	debug_return_ptr(NULL);

    /* UID:GID: */
    len = snprintf(user_str, user_size, "%u:%u:",
	(unsigned int)user_cred->uid, (unsigned int)user_cred->gid);
    if (len < 0 || (size_t)len >= user_size)
	sudo_fatalx(U_("internal error, %s overflow"), __func__);

    /* Supplementary GIDs */
    cp = user_str + len;
    for (i = 0; i < user_cred->ngroups; i++) {
	len = snprintf(cp, user_size - (cp - user_str), "%s%u",
	    i ? "," : "", (unsigned int)user_cred->groups[i]);
	if (len < 0 || (size_t)len >= user_size - (cp - user_str))
	    sudo_fatalx(U_("internal error, %s overflow"), __func__);
	cp += len;
    }

    debug_return_ptr(user_str);
}

static int
selinux_edit_create_tfiles(const struct command_details *command_details,
    const struct sudo_cred *user_cred, struct tempfile *tf,
    char *files[], int nfiles)
{
    const char **sesh_args, **sesh_ap;
    char *user_str = NULL;
    int i, error, sesh_nargs, ret = -1;
    struct stat sb;
    debug_decl(selinux_edit_create_tfiles, SUDO_DEBUG_EDIT);
    
    if (nfiles < 1)
	debug_return_int(0);

    sesh_nargs = 6 + (nfiles * 2) + 1;
    sesh_args = sesh_ap = reallocarray(NULL, sesh_nargs, sizeof(char *));
    if (sesh_args == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    *sesh_ap++ = "sesh";
    *sesh_ap++ = "--edit-create";
    if (!ISSET(command_details->flags, CD_SUDOEDIT_FOLLOW))
	*sesh_ap++ = "--no-dereference";
    if (ISSET(command_details->flags, CD_SUDOEDIT_CHECKDIR)) {
	if ((user_str = selinux_fmt_sudo_user(user_cred)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
	*sesh_ap++ = "--edit-checkdir";
	*sesh_ap++ = user_str;
    }
    *sesh_ap++ = "--";

    for (i = 0; i < nfiles; i++) {
	char *tfile, *ofile = files[i];
	int tfd;
	*sesh_ap++  = ofile;
	tf[i].ofile = ofile;
	if (stat(ofile, &sb) == -1)
	    memset(&sb, 0, sizeof(sb));		/* new file */
	tf[i].osize = sb.st_size;
	mtim_get(&sb, tf[i].omtim);
	/*
	 * The temp file must be created by the sesh helper,
	 * which uses O_EXCL | O_NOFOLLOW to make this safe.
	 */
	tfd = sudo_edit_mktemp(ofile, &tfile);
	if (tfd == -1) {
	    sudo_warn("mkstemps");
	    free(tfile);
	    goto done;
	}
	/* Helper will re-create temp file with proper security context. */
	close(tfd);
	unlink(tfile);
	*sesh_ap++  = tfile;
	tf[i].tfile = tfile;
    }
    *sesh_ap = NULL;

    /* Run sesh -c [-h] [-w userstr] <o1> <t1> ... <on> <tn> */
    error = selinux_run_helper(command_details->cred.uid,
	command_details->cred.gid, command_details->cred.ngroups,
	command_details->cred.groups, (char **)sesh_args, command_details->envp);
    switch (error) {
    case SESH_SUCCESS:
	break;
    case SESH_ERR_BAD_PATHS:
	sudo_fatalx("%s", U_("sesh: internal error: odd number of paths"));
    case SESH_ERR_NO_FILES:
	sudo_fatalx("%s", U_("sesh: unable to create temporary files"));
    case SESH_ERR_KILLED:
	sudo_fatalx("%s", U_("sesh: killed by a signal"));
    default:
	sudo_warnx(U_("sesh: unknown error %d"), error);
	goto done;
    }

    for (i = 0; i < nfiles; i++) {
	int tfd = open(tf[i].tfile, O_RDONLY|O_NONBLOCK|O_NOFOLLOW);
	if (tfd == -1) {
	    sudo_warn(U_("unable to open %s"), tf[i].tfile);
	    goto done;
	}
	if (!sudo_check_temp_file(tfd, tf[i].tfile, command_details->cred.uid, NULL)) {
	    close(tfd);
	    goto done;
	}
	if (fchown(tfd, user_cred->uid, user_cred->gid) != 0) {
	    sudo_warn("unable to chown(%s) to %d:%d for editing",
		tf[i].tfile, user_cred->uid, user_cred->gid);
	    close(tfd);
	    goto done;
	}
	close(tfd);
    }
    ret = nfiles;

done:
    /* Contents of tf will be freed by caller. */
    free(sesh_args);
    free(user_str);

    debug_return_int(ret);
}

static int
selinux_edit_copy_tfiles(const struct command_details *command_details,
    const struct sudo_cred *user_cred, struct tempfile *tf,
    int nfiles, struct timespec *times)
{
    const char **sesh_args, **sesh_ap;
    char *user_str = NULL;
    bool run_helper = false;
    int i, error, sesh_nargs, ret = 1;
    int tfd = -1;
    struct timespec ts;
    struct stat sb;
    debug_decl(selinux_edit_copy_tfiles, SUDO_DEBUG_EDIT);
    
    if (nfiles < 1)
	debug_return_int(0);

    sesh_nargs = 5 + (nfiles * 2) + 1;
    sesh_args = sesh_ap = reallocarray(NULL, sesh_nargs, sizeof(char *));
    if (sesh_args == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    *sesh_ap++ = "sesh";
    *sesh_ap++ = "--edit-install";
    if (ISSET(command_details->flags, CD_SUDOEDIT_CHECKDIR)) {
	if ((user_str = selinux_fmt_sudo_user(user_cred)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
	*sesh_ap++ = "--edit-checkdir";
	*sesh_ap++ = user_str;
    }
    *sesh_ap++ = "--";

    for (i = 0; i < nfiles; i++) {
	if (tfd != -1)
	    close(tfd);
	if ((tfd = open(tf[i].tfile, O_RDONLY|O_NONBLOCK|O_NOFOLLOW)) == -1) {
	    sudo_warn(U_("unable to open %s"), tf[i].tfile);
	    continue;
	}
	if (!sudo_check_temp_file(tfd, tf[i].tfile, user_cred->uid, &sb))
	    continue;
	mtim_get(&sb, ts);
	if (tf[i].osize == sb.st_size && sudo_timespeccmp(&tf[i].omtim, &ts, ==)) {
	    /*
	     * If mtime and size match but the user spent no measurable
	     * time in the editor we can't tell if the file was changed.
	     */
	    if (sudo_timespeccmp(&times[0], &times[1], !=)) {
		sudo_warnx(U_("%s unchanged"), tf[i].ofile);
		unlink(tf[i].tfile);
		continue;
	    }
	}
	run_helper = true;
	*sesh_ap++ = tf[i].tfile;
	*sesh_ap++ = tf[i].ofile;
	if (fchown(tfd, command_details->cred.uid, command_details->cred.gid) != 0) {
	    sudo_warn("unable to chown(%s) back to %d:%d", tf[i].tfile,
		command_details->cred.uid, command_details->cred.gid);
	}
    }
    *sesh_ap = NULL;

    if (!run_helper)
	goto done;

    /* Run sesh -i <t1> <o1> ... <tn> <on> */
    error = selinux_run_helper(command_details->cred.uid,
	command_details->cred.gid, command_details->cred.ngroups,
	command_details->cred.groups, (char **)sesh_args, command_details->envp);
    switch (error) {
    case SESH_SUCCESS:
	ret = 0;
	break;
    case SESH_ERR_NO_FILES:
	sudo_warnx("%s",
	    U_("unable to copy temporary files back to their original location"));
	break;
    case SESH_ERR_SOME_FILES:
	sudo_warnx("%s",
	    U_("unable to copy some of the temporary files back to their original location"));
	break;
    case SESH_ERR_KILLED:
	sudo_warnx("%s", U_("sesh: killed by a signal"));
	break;
    default:
	sudo_warnx(U_("sesh: unknown error %d"), error);
	break;
    }

done:
    if (tfd != -1)
	close(tfd);
    /* Contents of tf will be freed by caller. */
    free(sesh_args);
    free(user_str);

    debug_return_int(ret);
}
#endif /* HAVE_SELINUX */

/*
 * Wrapper to allow users to edit privileged files with their own uid.
 * Returns the wait status of the command on success and a wait status
 * of 1 on failure.
 */
int
sudo_edit(struct command_details *command_details,
    const struct user_details *user_details)
{
    struct command_details saved_command_details;
    const struct sudo_cred *user_cred = &user_details->cred;
    char **nargv = NULL, **files = NULL;
    int nfiles = command_details->nfiles;
    int errors, i, ac, nargc, ret;
    int editor_argc = 0;
    struct timespec times[2];
    struct tempfile *tf = NULL;
    debug_decl(sudo_edit, SUDO_DEBUG_EDIT);

    /*
     * Set real, effective and saved uids to root.
     * We will change the euid as needed below.
     */
    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"setuid(%u)", ROOT_UID);
    if (setuid(ROOT_UID) != 0) {
	sudo_warn(U_("unable to change uid to root (%u)"), ROOT_UID);
	goto cleanup;
    }

    /* Find a temporary directory writable by the user.  */
    if (!set_tmpdir(user_cred))
	goto cleanup;

    if (nfiles > 0) {
	/*
	 * The plugin specified the number of files to edit, use it.
	 */
	editor_argc = command_details->argc - nfiles;
	if (editor_argc < 2 || strcmp(command_details->argv[editor_argc - 1], "--") != 0) {
	    sudo_warnx("%s", U_("plugin error: invalid file list for sudoedit"));
	    goto cleanup;
	}

	/* We don't include the "--" when running the user's editor. */
	files = &command_details->argv[editor_argc--];
    } else {
	/*
	 * Compute the number of files to edit by looking for the "--"
	 * option which separate the editor from the files.
	 */
	for (i = 0; command_details->argv[i] != NULL; i++) {
	    if (strcmp(command_details->argv[i], "--") == 0) {
		editor_argc = i;
		files = &command_details->argv[i + 1];
		nfiles = command_details->argc - (i + 1);
		break;
	    }
	}
    }
    if (nfiles == 0) {
	sudo_warnx("%s", U_("plugin error: missing file list for sudoedit"));
	goto cleanup;
    }

    /* Copy editor files to temporaries. */
    tf = calloc((size_t)nfiles, sizeof(*tf));
    if (tf == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto cleanup;
    }
#ifdef HAVE_SELINUX
    if (ISSET(command_details->flags, CD_RBAC_ENABLED))
	nfiles = selinux_edit_create_tfiles(command_details, user_cred, tf, files, nfiles);
    else 
#endif
	nfiles = sudo_edit_create_tfiles(command_details, user_cred, tf, files, nfiles);
    if (nfiles <= 0)
	goto cleanup;

    /*
     * Allocate space for the new argument vector and fill it in.
     * We concatenate the editor with its args and the file list
     * to create a new argv.
     */
    nargc = editor_argc + nfiles;
    nargv = reallocarray(NULL, (size_t)nargc + 1, sizeof(char *));
    if (nargv == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto cleanup;
    }
    for (ac = 0; ac < editor_argc; ac++)
	nargv[ac] = command_details->argv[ac];
    for (i = 0; i < nfiles && ac < nargc; )
	nargv[ac++] = tf[i++].tfile;
    nargv[ac] = NULL;

    /*
     * Run the editor with the invoking user's creds and drop setuid.
     * Keep track of the time spent in the editor to distinguish between
     * a user editing a file and a program doing it.
     * XXX - should run editor with user's context
     */
    if (sudo_gettime_real(&times[0]) == -1) {
	sudo_warn("%s", U_("unable to read the clock"));
	goto cleanup;
    }
#ifdef HAVE_SELINUX
    if (ISSET(command_details->flags, CD_RBAC_ENABLED))
	selinux_audit_role_change();
#endif
    memcpy(&saved_command_details, command_details, sizeof(struct command_details));
    command_details->cred = *user_cred;
    command_details->cred.euid = user_cred->uid;
    command_details->cred.egid = user_cred->gid;
    command_details->argc = nargc;
    command_details->argv = nargv;
    ret = run_command(command_details, user_details);
    if (sudo_gettime_real(&times[1]) == -1) {
	sudo_warn("%s", U_("unable to read the clock"));
	goto cleanup;
    }

    /* Restore saved command_details. */
    command_details->cred = saved_command_details.cred;
    command_details->argc = saved_command_details.argc;
    command_details->argv = saved_command_details.argv;

    /* Copy contents of temp files to real ones. */
#ifdef HAVE_SELINUX
    if (ISSET(command_details->flags, CD_RBAC_ENABLED))
	errors = selinux_edit_copy_tfiles(command_details, user_cred, tf, nfiles, times);
    else
#endif
	errors = sudo_edit_copy_tfiles(command_details, user_cred, tf, nfiles, times);
    if (errors) {
	/* Preserve the edited temporary files. */
	ret = W_EXITCODE(1, 0);
    }

    for (i = 0; i < nfiles; i++)
	free(tf[i].tfile);
    free(tf);
    free(nargv);
    debug_return_int(ret);

cleanup:
    /* Clean up temp files and return. */
    if (tf != NULL) {
	for (i = 0; i < nfiles; i++) {
	    if (tf[i].tfile != NULL)
		unlink(tf[i].tfile);
	    free(tf[i].tfile);
	}
    }
    free(tf);
    free(nargv);
    debug_return_int(W_EXITCODE(1, 0));
}

#else /* HAVE_SETRESUID || HAVE_SETREUID || HAVE_SETEUID */

/*
 * Must have the ability to change the effective uid to use sudoedit.
 */
int
sudo_edit(const struct command_details *command_details, const struct sudo_cred *user_cred)
{
    debug_decl(sudo_edit, SUDO_DEBUG_EDIT);
    debug_return_int(W_EXITCODE(1, 0));
}

#endif /* HAVE_SETRESUID || HAVE_SETREUID || HAVE_SETEUID */

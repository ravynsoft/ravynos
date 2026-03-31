/*
 * cap.c - POSIX.1e (POSIX.6) capability set manipulation
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1997 Andrew Main
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Andrew Main or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Andrew Main and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Andrew Main and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Andrew Main and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "cap.mdh"
#include "cap.pro"

#ifdef HAVE_CAP_GET_PROC

static int
bin_cap(char *nam, char **argv, UNUSED(Options ops), UNUSED(int func))
{
    int ret = 0;
    cap_t caps;
    if(*argv) {
	unmetafy(*argv, NULL);
	caps = cap_from_text(*argv);
	if(!caps) {
	    zwarnnam(nam, "invalid capability string");
	    return 1;
	}
	if(cap_set_proc(caps)) {
	    zwarnnam(nam, "can't change capabilities: %e", errno);
	    ret = 1;
	}
    } else {
	char *result = NULL;
	ssize_t length;
	caps = cap_get_proc();
	if(caps)
	    result = cap_to_text(caps, &length);
	if(!caps || !result) {
	    zwarnnam(nam, "can't get capabilities: %e", errno);
	    ret = 1;
	} else
	    puts(result);
    }
    cap_free(caps);
    return ret;
}

static int
bin_getcap(char *nam, char **argv, UNUSED(Options ops), UNUSED(int func))
{
    int ret = 0;

    do {
	char *result = NULL;
	ssize_t length;
	cap_t caps;

	caps = cap_get_file(unmetafy(dupstring(*argv), NULL));
	if(caps)
	    result = cap_to_text(caps, &length);
	if (!caps || !result) {
	    zwarnnam(nam, "%s: %e", *argv, errno);
	    ret = 1;
	} else
	    printf("%s %s\n", *argv, result);
	cap_free(caps);
    } while(*++argv);
    return ret;
}

static int
bin_setcap(char *nam, char **argv, UNUSED(Options ops), UNUSED(int func))
{
    cap_t caps;
    int ret = 0;

    unmetafy(*argv, NULL);
    caps = cap_from_text(*argv++);
    if(!caps) {
	zwarnnam(nam, "invalid capability string");
	return 1;
    }

    do {
	if(cap_set_file(unmetafy(dupstring(*argv), NULL), caps)) {
	    zwarnnam(nam, "%s: %e", *argv, errno);
	    ret = 1;
	}
    } while(*++argv);
    cap_free(caps);
    return ret;
}

#else /* !HAVE_CAP_GET_PROC */

# define bin_cap    bin_notavail
# define bin_getcap bin_notavail
# define bin_setcap bin_notavail

#endif /* !HAVE_CAP_GET_PROC */

/* module paraphernalia */

static struct builtin bintab[] = {
    BUILTIN("cap",    0, bin_cap,    0,  1, 0, NULL, NULL),
    BUILTIN("getcap", 0, bin_getcap, 1, -1, 0, NULL, NULL),
    BUILTIN("setcap", 0, bin_setcap, 2, -1, 0, NULL, NULL),
};

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    NULL, 0,
    NULL, 0,
    NULL, 0,
    0
};

/**/
int
setup_(UNUSED(Module m))
{
    return 0;
}

/**/
int
features_(Module m, char ***features)
{
    *features = featuresarray(m, &module_features);
    return 0;
}

/**/
int
enables_(Module m, int **enables)
{
    return handlefeatures(m, &module_features, enables);
}

/**/
int
boot_(UNUSED(Module m))
{
    return 0;
}

/**/
int
cleanup_(Module m)
{
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}

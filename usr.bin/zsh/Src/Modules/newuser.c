/*
 * newuser.c - handler for easy setup for new zsh users
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 2005 Peter Stephenson
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Peter Stephenson or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Peter Stephenson and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Peter Stephenson and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Peter Stephenson and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "newuser.mdh"
#include "newuser.pro"

#include "../zshpaths.h"

/**/
int
setup_(UNUSED(Module m))
{
    return 0;
}

/**/
int
features_(UNUSED(Module m), UNUSED(char ***features))
{
    return 1;
}

/**/
int
enables_(UNUSED(Module m), UNUSED(int **enables))
{
    return 0;
}

/**/
static int
check_dotfile(const char *dotdir, const char *fname)
{
    VARARR(char, buf, strlen(dotdir) + strlen(fname) + 2);
    sprintf(buf, "%s/%s", dotdir, fname);

    return access(buf, F_OK);
}

/**/
int
boot_(UNUSED(Module m))
{
    const char *dotdir = getsparam_u("ZDOTDIR");
    const char *spaths[] = {
#ifdef SITESCRIPT_DIR
	SITESCRIPT_DIR,
#endif
#ifdef SCRIPT_DIR
	SCRIPT_DIR,
#endif
	0 };
    const char **sp;

    if (!EMULATION(EMULATE_ZSH))
	return 0;

    if (!dotdir) {
	dotdir = home;
	if (!dotdir)
	    return 0;
    }

    if (check_dotfile(dotdir, ".zshenv") == 0 ||
	check_dotfile(dotdir, ".zprofile") == 0 ||
	check_dotfile(dotdir, ".zshrc") == 0 ||
	check_dotfile(dotdir, ".zlogin") == 0)
	return 0;

    for (sp = spaths; *sp; sp++) {
	VARARR(char, buf, strlen(*sp) + 9);
	sprintf(buf, "%s/newuser", *sp);

	if (source(buf) != SOURCE_NOT_FOUND)
	    break;
    }

    return 0;
}

/**/
int
cleanup_(UNUSED(Module m))
{
    return 0;
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}

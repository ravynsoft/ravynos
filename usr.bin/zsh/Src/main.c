/*
 * main.c - the main() function
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1992-1997 Paul Falstad
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Paul Falstad or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Paul Falstad and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Paul Falstad and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Paul Falstad and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "zsh.mdh"
#include "main.pro"

/*
 * Support for Cygwin binary/text mode filesystems.
 * Peter A. Castro <doctor@fruitbat.org>
 *
 * This deserves some explanation, because it uses Cygwin specific
 * runtime functions.
 *
 * Cygwin supports the notion of binary or text mode access to files
 * based on the mount attributes of the filesystem.  If a file is on
 * a binary mounted filesystem, you get exactly what's in the file, CRLF's
 * and all.  If it's on a text mounted filesystem, Cygwin will strip out
 * the CRs.  This presents a problem because zsh code doesn't allow for
 * CRLF's as line terminators.  So, we must force all open files to be
 * in text mode regardless of the underlying filesystem attributes.
 * However, we only want to do this for reading, not writing as we still
 * want to write files in the mode of the filesystem.  To do this,
 * we have two options: augment all {f}open() calls to have O_TEXT added to
 * the list of file mode options, or have the Cygwin runtime do it for us.
 * I choose the latter. :)
 *
 * Cygwin's runtime provides pre-execution hooks which allow you to set
 * various attributes for the process which effect how the process functions.
 * One of these attributes controls how files are opened.  I've set
 * it up so that all files opened RDONLY will have the O_TEXT option set,
 * thus forcing line termination manipulation.  This seems to solve the
 * problem (at least the Test suite runs clean :).
 *
 * Note: this may not work in later implementations.  This will override
 * all mode options passed into open().  Cygwin (really Windows) doesn't
 * support all that much in options, so for now this is OK, but later on
 * it may not, in which case O_TEXT will have to be added to all opens calls
 * appropriately.
 *
 * This function is actually a hook in the Cygwin runtime which
 * is called before the main of a program.  Because it's part of the program
 * pre-startup, it must be located in the program main and not in a DLL.
 * It must also be made an export so the linker resolves this function to
 * our code instead of the default Cygwin stub routine.
 */

/**/
#ifdef __CYGWIN__
/**/
mod_export void
cygwin_premain0 (int argc, char **argv, void *myself)
{
    static struct __cygwin_perfile pf[] =
    {
        {"", O_RDONLY | O_TEXT},
        {NULL, 0}
    };
    cygwin_internal (CW_PERFILE, pf);
}
/**/
#endif /* __CYGWIN__ */

/**/
int
main(int argc, char **argv)
{
    return (zsh_main(argc, argv));
}

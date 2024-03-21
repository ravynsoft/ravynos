/* Information about terminal capabilities.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2006.  */

#ifndef _TERMINFO_H
#define _TERMINFO_H

/* This file uses HAVE_TERMINFO, HAVE_TERMCAP, HAVE_TPARAM.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* Including <curses.h> or <term.h> is dangerous, because it also declares
   a lot of junk, such as variables PC, UP, and other.  */

#ifdef __cplusplus
extern "C" {
#endif

#if HAVE_TERMINFO

/* Gets the capability information for terminal type TYPE and prepares FD.
   Returns 0 if successful, -1 upon error.  If ERRP is non-NULL, also returns
   an error indicator in *ERRP; otherwise an error is signalled.  */
extern int setupterm (const char *type, int fd, int *errp);

/* Retrieves the value of a numerical capability.
   Returns -1 if it is not available, -2 if ID is invalid.  */
extern int tigetnum (const char *id);

/* Retrieves the value of a boolean capability.
   Returns 1 if it available, 0 if not available, -1 if ID is invalid.  */
extern int tigetflag (const char *id);

/* Retrieves the value of a string capability.
   Returns NULL if it is not available, (char *)(-1) if ID is invalid.  */
extern const char * tigetstr (const char *id);

#elif HAVE_TERMCAP

/* Gets the capability information for terminal type TYPE.
   BP must point to a buffer, at least 2048 bytes large.
   Returns 1 if successful, 0 if TYPE is unknown, -1 on other error.  */
extern int tgetent (char *bp, const char *type);

/* Retrieves the value of a numerical capability.
   Returns -1 if it is not available.  */
extern int tgetnum (const char *id);

/* Retrieves the value of a boolean capability.
   Returns 1 if it available, 0 otherwise.  */
extern int tgetflag (const char *id);

/* Retrieves the value of a string capability.
   Returns NULL if it is not available.
   Also, if AREA != NULL, stores it at *AREA and advances *AREA.  */
extern const char * tgetstr (const char *id, char **area);

#endif

#if HAVE_TPARAM

/* API provided by GNU termcap in <termcap.h>.  */

/* Instantiates a string capability with format strings.
   BUF must be a buffer having room for BUFSIZE bytes.
   The return value is either equal to BUF or freshly malloc()ed.  */
extern char * tparam (const char *str, void *buf, int bufsize, ...);

#else

/* API provided by
     - GNU ncurses in <term.h>, <curses.h>, <ncurses.h>,
     - OSF/1 curses in <term.h>, <curses.h>,
     - Solaris, AIX, HP-UX, IRIX curses in <term.h>,
     - gnulib's replacement.  */

/* Instantiates a string capability with format strings.
   The return value is statically allocated and must not be freed.  */
extern char * tparm (const char *str, ...);

#endif

#if HAVE_TERMINFO || HAVE_TERMCAP

/* Retrieves a string that causes cursor positioning to (column, row).
   This function is necessary because the string returned by tgetstr ("cm")
   is in a special format.  */
extern const char * tgoto (const char *cm, int column, int row);

#endif

/* Retrieves the value of a string capability.
   OUTCHARFUN is called in turn for each 'char' of the result.
   This function is necessary because string capabilities can contain
   padding commands.  */
extern void tputs (const char *cp, int affcnt, int (*outcharfun) (int));

/* The ncurses functions for color handling (see ncurses/base/lib_color.c)
   are overkill: Most terminal emulators support only a fixed, small number
   of colors.  */

#ifdef __cplusplus
}
#endif

#endif /* _TERMINFO_H */

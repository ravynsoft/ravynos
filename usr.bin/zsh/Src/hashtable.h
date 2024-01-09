/*
 * hashtable.h - header file for hash table handling code
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

/* Builtin function numbers; used by handler functions that handle more *
 * than one builtin.  Note that builtins such as compctl, that are not  *
 * overloaded, don't get a number.                                      */

#define BIN_TYPESET   0
#define BIN_BG        1
#define BIN_FG        2
#define BIN_JOBS      3
#define BIN_WAIT      4
#define BIN_DISOWN    5
#define BIN_BREAK     6
#define BIN_CONTINUE  7
#define BIN_EXIT      8
#define BIN_RETURN    9
#define BIN_CD       10
#define BIN_POPD     11
#define BIN_PUSHD    12
#define BIN_PRINT    13
#define BIN_EVAL     14
#define BIN_SCHED    15
#define BIN_FC       16
#define BIN_R	     17
#define BIN_PUSHLINE 18
#define BIN_LOGOUT   19
#define BIN_TEST     20
#define BIN_BRACKET  21
#define BIN_READONLY 22
#define BIN_ECHO     23
#define BIN_DISABLE  24
#define BIN_ENABLE   25
#define BIN_PRINTF   26
#define BIN_COMMAND  27
#define BIN_UNHASH   28
#define BIN_UNALIAS  29
#define BIN_UNFUNCTION  30
#define BIN_UNSET    31
#define BIN_EXPORT   32

/* These currently depend on being 0 and 1. */
#define BIN_SETOPT    0
#define BIN_UNSETOPT  1

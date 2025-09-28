/*
 * hashtable.c - hash tables
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

#include "../config.h"

/*
 * On Solaris 8 there's a clash between "bool" in curses and RPC.
 * We don't need curses here, so ensure it doesn't get included.
 */
#define ZSH_NO_TERM_HANDLING

#include "zsh.mdh"
#include "hashnameddir.pro"

/****************************************/
/* Named Directory Hash Table Functions */
/****************************************/

/* hash table containing named directories */

/**/
mod_export HashTable nameddirtab;

/* != 0 if all the usernames have already been *
 * added to the named directory hash table.    */

static int allusersadded;

/* Create new hash table for named directories */

/**/
void
createnameddirtable(void)
{
    nameddirtab = newhashtable(201, "nameddirtab", NULL);

    nameddirtab->hash        = hasher;
    nameddirtab->emptytable  = emptynameddirtable;
    nameddirtab->filltable   = fillnameddirtable;
    nameddirtab->cmpnodes    = strcmp;
    nameddirtab->addnode     = addnameddirnode;
    nameddirtab->getnode     = gethashnode;
    nameddirtab->getnode2    = gethashnode2;
    nameddirtab->removenode  = removenameddirnode;
    nameddirtab->disablenode = NULL;
    nameddirtab->enablenode  = NULL;
    nameddirtab->freenode    = freenameddirnode;
    nameddirtab->printnode   = printnameddirnode;

    allusersadded = 0;
    finddir(NULL);		/* clear the finddir cache */
}

/* Empty the named directories table */

/**/
static void
emptynameddirtable(HashTable ht)
{
    emptyhashtable(ht);
    allusersadded = 0;
    finddir(NULL);		/* clear the finddir cache */
}

/* Add all the usernames in the password file/database *
 * to the named directories table.                     */

/**/
static void
fillnameddirtable(UNUSED(HashTable ht))
{
    if (!allusersadded) {
#ifdef USE_GETPWENT
	struct passwd *pw;

	setpwent();

	/* loop through the password file/database *
	 * and add all entries returned.           */
	while ((pw = getpwent()) && !errflag)
	    adduserdir(pw->pw_name, pw->pw_dir, ND_USERNAME, 1);

	endpwent();
#endif /* USE_GETPWENT */
	allusersadded = 1;
    }
}

/* Add an entry to the named directory hash *
 * table, clearing the finddir() cache and  *
 * initialising the `diff' member.          */

/**/
static void
addnameddirnode(HashTable ht, char *nam, void *nodeptr)
{
    Nameddir nd = (Nameddir) nodeptr;

    nd->diff = strlen(nd->dir) - strlen(nam);
    finddir(NULL);		/* clear the finddir cache */
    addhashnode(ht, nam, nodeptr);
}

/* Remove an entry from the named directory  *
 * hash table, clearing the finddir() cache. */

/**/
static HashNode
removenameddirnode(HashTable ht, const char *nam)
{
    HashNode hn = removehashnode(ht, nam);

    if(hn)
	finddir(NULL);		/* clear the finddir cache */
    return hn;
}

/* Free up the memory used by a named directory hash node. */

/**/
static void
freenameddirnode(HashNode hn)
{
    Nameddir nd = (Nameddir) hn;

    zsfree(nd->node.nam);
    zsfree(nd->dir);
    zfree(nd, sizeof(struct nameddir));
}

/* Print a named directory */

/**/
static void
printnameddirnode(HashNode hn, int printflags)
{
    Nameddir nd = (Nameddir) hn;

    if (printflags & PRINT_NAMEONLY) {
	zputs(nd->node.nam, stdout);
	putchar('\n');
	return;
    }

    if (printflags & PRINT_LIST) {
      printf("hash -d ");

      if(nd->node.nam[0] == '-')
	    printf("-- ");
    }

    quotedzputs(nd->node.nam, stdout);
    putchar('=');
    quotedzputs(nd->dir, stdout);
    putchar('\n');
}

#include "../config.h"

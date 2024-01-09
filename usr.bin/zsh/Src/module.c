/*
 * module.c - deal with dynamic modules
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1996-1997 Zoltán Hidvégi
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Zoltán Hidvégi or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Zoltán Hidvégi and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Zoltán Hidvégi and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Zoltán Hidvégi and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 */

#include "zsh.mdh"
#include "module.pro"

/*
 * List of linked-in modules.
 * This is set up at boot and remains for the life of the shell;
 * entries do not appear in "zmodload" listings.
 */

/**/
LinkList linkedmodules;

/* $module_path ($MODULE_PATH) */

/**/
char **module_path;

/* Hash of modules */

/**/
mod_export HashTable modulestab;

/*
 * Bit flags passed as the "flags" argument of a autofeaturefn_t.
 * Used in other places, such as the final argument to
 * do_module_features().
 */
enum {
    /*
     * `-i' option: ignore errors pertaining to redefinitions,
     * or indicate to do_module_features() that it should be
     * silent.
     */
    FEAT_IGNORE = 0x0001,
    /* If a condition, condition is infix rather than prefix */
    FEAT_INFIX = 0x0002,
    /*
     * Enable all features in the module when autoloading.
     * This is the traditional zmodload -a behaviour;
     * zmodload -Fa only enables features explicitly marked for
     * autoloading.
     */
    FEAT_AUTOALL = 0x0004,
    /*
     * Remove feature:  alternative to "-X:NAME" used if
     * X is passed separately from NAME.
     */
    FEAT_REMOVE = 0x0008,
    /*
     * For do_module_features().  Check that any autoloads
     * for the module are actually provided.
     */
    FEAT_CHECKAUTO = 0x0010
};

/*
 * All functions to add or remove autoloadable features fit
 * the following prototype.
 *
 * "module" is the name of the module.
 *
 * "feature" is the name of the feature, minus any type prefix.
 *
 * "flags" is a set of the bits above.
 *
 * The return value is 0 for success, -1 for failure with no
 * message needed, and one of the following to indicate the calling
 * function should print a message:
 *
 * 1:  failed to add [type] `[feature]'
 * 2:  [feature]: no such [type]
 * 3:  [feature]: [type] is already defined
 */
typedef int (*autofeaturefn_t)(const char *module, const char *feature,
			       int flags);

/* Bits in the second argument to find_module. */
enum {
    /*
     * Resolve any aliases to the underlying module.
     */
    FINDMOD_ALIASP = 0x0001,
    /*
     * Create an element for the module in the list if
     * it is not found.
     */
    FINDMOD_CREATE = 0x0002,
};

static void
freemodulenode(HashNode hn)
{
    Module m = (Module) hn;

    if (m->node.flags & MOD_ALIAS)
	zsfree(m->u.alias);
    zsfree(m->node.nam);
    if (m->autoloads)
	freelinklist(m->autoloads, freestr);
    if (m->deps)
	freelinklist(m->deps, freestr);
    zfree(m, sizeof(*m));
}

/* flags argument to printmodulenode */
enum {
    /* -L flag, output zmodload commands */
    PRINTMOD_LIST = 0x0001,
    /* -e flag */
    PRINTMOD_EXIST = 0x0002,
    /* -A flag */
    PRINTMOD_ALIAS = 0x0004,
    /* -d flag */
    PRINTMOD_DEPS = 0x0008,
    /* -F flag */
    PRINTMOD_FEATURES = 0x0010,
    /* -l flag in combination with -L flag */
    PRINTMOD_LISTALL = 0x0020,
    /* -a flag */
    PRINTMOD_AUTO = 0x0040
};

/* Scan function for printing module details */

static void
printmodulenode(HashNode hn, int flags)
{
    Module m = (Module)hn;
    /*
     * If we check for a module loaded under an alias, we
     * need the name of the alias.  We can use it in other
     * cases, too.
     */
    const char *modname = m->node.nam;

    if (flags & PRINTMOD_DEPS) {
	/*
	 * Print the module's dependencies.
	 */
	LinkNode n;

	if (!m->deps)
	    return;

	if (flags & PRINTMOD_LIST) {
	    printf("zmodload -d ");
	    if (modname[0] == '-')
		fputs("-- ", stdout);
	    quotedzputs(modname, stdout);
	} else {
	    nicezputs(modname, stdout);
	    putchar(':');
	}
	for (n = firstnode(m->deps); n; incnode(n)) {
	    putchar(' ');
	    if (flags & PRINTMOD_LIST)
		quotedzputs((char *) getdata(n), stdout);
	    else
		nicezputs((char *) getdata(n), stdout);
	}
    } else if (flags & PRINTMOD_EXIST) {
	/*
	 * Just print the module name, provided the module is
	 * present under an alias or otherwise.
	 */
	if (m->node.flags & MOD_ALIAS) {
	    if (!(flags & PRINTMOD_ALIAS) ||
		!(m = find_module(m->u.alias, FINDMOD_ALIASP, NULL)))
		return;
	}
	if (!m->u.handle || (m->node.flags & MOD_UNLOAD))
	    return;
	nicezputs(modname, stdout);
   } else if (m->node.flags & MOD_ALIAS) {
	/*
	 * Normal listing, but for aliases.
	 */
	if (flags & PRINTMOD_LIST) {
	    printf("zmodload -A ");
	    if (modname[0] == '-')
		fputs("-- ", stdout);
	    quotedzputs(modname, stdout);
	    putchar('=');
	    quotedzputs(m->u.alias, stdout);
	} else {
	    nicezputs(modname, stdout);
	    fputs(" -> ", stdout);
	    nicezputs(m->u.alias, stdout);
	}
    } else if (m->u.handle || (flags & PRINTMOD_AUTO)) {
	/*
	 * Loaded module.
	 */
	if (flags & PRINTMOD_LIST) {
	    /*
	     * List with -L format.  Possibly we are printing
	     * features, either enables or autoloads.
	     */
	    char **features = NULL;
	    int *enables = NULL;
	    if (flags & PRINTMOD_AUTO) {
		if (!m->autoloads || !firstnode(m->autoloads))
		    return;
	    } else if (flags & PRINTMOD_FEATURES) {
		if (features_module(m, &features) ||
		    enables_module(m, &enables) ||
		    !*features)
		    return;
	    }
	    printf("zmodload ");
	    if (flags & PRINTMOD_AUTO) {
		fputs("-Fa ", stdout);
	    } else if (features)
		fputs("-F ", stdout);
	    if(modname[0] == '-')
		fputs("-- ", stdout);
	    quotedzputs(modname, stdout);
	    if (flags & PRINTMOD_AUTO) {
		LinkNode an;
		for (an = firstnode(m->autoloads); an; incnode(an)) {
		    putchar(' ');
		    quotedzputs((char *)getdata(an), stdout);
		}
	    } else if (features) {
		const char *f;
		while ((f = *features++)) {
		    int on = *enables++;
		    if (flags & PRINTMOD_LISTALL)
			printf(" %s", on ? "+" : "-");
		    else if (!on)
			continue;
		    else
			putchar(' ');
		    quotedzputs(f, stdout);
		}
	    }
	} else /* -l */
	    nicezputs(modname, stdout);
    } else
	return;
    putchar('\n');
}

/**/
HashTable
newmoduletable(int size, char const *name)
{
    HashTable ht;
    ht = newhashtable(size, name, NULL);

    ht->hash        = hasher;
    ht->emptytable  = emptyhashtable;
    ht->filltable   = NULL;
    ht->cmpnodes    = strcmp;
    ht->addnode     = addhashnode;
    /* DISABLED is not supported */
    ht->getnode     = gethashnode2;
    ht->getnode2    = gethashnode2;
    ht->removenode  = removehashnode;
    ht->disablenode = NULL;
    ht->enablenode  = NULL;
    ht->freenode    = freemodulenode;
    ht->printnode   = printmodulenode;

    return ht;
}

/************************************************************************
 * zsh/main standard module functions
 ************************************************************************/

/* The `zsh/main' module contains all the base code that can't actually be *
 * built as a separate module.  It is initialised by main(), so there's    *
 * nothing for the boot function to do.                                    */

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
    /*
     * There are lots and lots of features, but they're not
     * handled here.
     */
    return 1;
}

/**/
int
enables_(UNUSED(Module m), UNUSED(int **enables))
{
    return 1;
}

/**/
int
boot_(UNUSED(Module m))
{
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


/************************************************************************
 * Module utility functions
 ************************************************************************/

/* This registers a builtin module.                                   */

/**/
void
register_module(char *n, Module_void_func setup,
		Module_features_func features,
		Module_enables_func enables,
		Module_void_func boot,
		Module_void_func cleanup,
		Module_void_func finish)
{
    Linkedmod m;

    m = (Linkedmod) zalloc(sizeof(*m));

    m->name = ztrdup(n);
    m->setup = setup;
    m->features = features;
    m->enables = enables;
    m->boot = boot;
    m->cleanup = cleanup;
    m->finish = finish;

    zaddlinknode(linkedmodules, m);
}

/* Check if a module is linked in. */

/**/
Linkedmod
module_linked(char const *name)
{
    LinkNode node;

    for (node = firstnode(linkedmodules); node; incnode(node))
	if (!strcmp(((Linkedmod) getdata(node))->name, name))
	    return (Linkedmod) getdata(node);

    return NULL;
}


/************************************************************************
 * Support for the various feature types.
 * First, builtins.
 ************************************************************************/

/* addbuiltin() can be used to add a new builtin.  It returns zero on *
 * success, 1 on failure.  The only possible type of failure is that  *
 * a builtin with the specified name already exists.  An autoloaded   *
 * builtin can be replaced using this function.                       */

/**/
static int
addbuiltin(Builtin b)
{
    Builtin bn = (Builtin) builtintab->getnode2(builtintab, b->node.nam);
    if (bn && (bn->node.flags & BINF_ADDED))
	return 1;
    if (bn)
	builtintab->freenode(builtintab->removenode(builtintab, b->node.nam));
    builtintab->addnode(builtintab, b->node.nam, b);
    return 0;
}

/* Define an autoloadable builtin.  It returns 0 on success, or 1 on *
 * failure.  The only possible cause of failure is that a builtin    *
 * with the specified name already exists.                           */

/**/
static int
add_autobin(const char *module, const char *bnam, int flags)
{
    Builtin bn;
    int ret;

    bn = zshcalloc(sizeof(*bn));
    bn->node.nam = ztrdup(bnam);
    bn->optstr = ztrdup(module);
    if (flags & FEAT_AUTOALL)
	bn->node.flags |= BINF_AUTOALL;
    if ((ret = addbuiltin(bn))) {
	builtintab->freenode(&bn->node);
	if (!(flags & FEAT_IGNORE))
	    return 1;
    }
    return 0;
}

/* Remove the builtin added previously by addbuiltin().  Returns *
 * zero on success and -1 if there is no builtin with that name. */

/**/
int
deletebuiltin(const char *nam)
{
    Builtin bn;

    bn = (Builtin) builtintab->removenode(builtintab, nam);
    if (!bn)
	return -1;
    builtintab->freenode(&bn->node);
    return 0;
}

/* Remove an autoloaded added by add_autobin */

/**/
static int
del_autobin(UNUSED(const char *module), const char *bnam, int flags)
{
    Builtin bn = (Builtin) builtintab->getnode2(builtintab, bnam);
    if (!bn) {
	if(!(flags & FEAT_IGNORE))
	    return 2;
    } else if (bn->node.flags & BINF_ADDED) {
	if (!(flags & FEAT_IGNORE))
	    return 3;
    } else
	deletebuiltin(bnam);

    return 0;
}

/*
 * Manipulate a set of builtins.  This should be called
 * via setfeatureenables() (or, usually, via the next level up,
 * handlefeatures()).
 *
 * "nam" is the name of the calling code builtin, probably "zmodload".
 *
 * "binl" is the builtin table containing an array of "size" builtins.
 *
 * "e" is either NULL, in which case all builtins in the
 * table are removed, or else an array corresponding to "binl"
 * with a 1 for builtins that are to be added and a 0 for builtins
 * that are to be removed.  Any builtin already in the appropriate
 * state is left alone.
 *
 * Returns 1 on any error, 0 for success.  The recommended way
 * of handling errors is to compare the enables passed down
 * with the set retrieved after the error to find what failed.
 */

/**/
static int
setbuiltins(char const *nam, Builtin binl, int size, int *e)
{
    int ret = 0, n;

    for(n = 0; n < size; n++) {
	Builtin b = &binl[n];
	if (e && *e++) {
	    if (b->node.flags & BINF_ADDED)
		continue;
	    if (addbuiltin(b)) {
		zwarnnam(nam,
			 "name clash when adding builtin `%s'", b->node.nam);
		ret = 1;
	    } else {
		b->node.flags |= BINF_ADDED;
	    }
	} else {
	    if (!(b->node.flags & BINF_ADDED))
		continue;
	    if (deletebuiltin(b->node.nam)) {
		zwarnnam(nam, "builtin `%s' already deleted", b->node.nam);
		ret = 1;
	    } else {
		b->node.flags &= ~BINF_ADDED;
	    }
	}
    }
    return ret;
}

/*
 * Add multiple builtins.  binl points to a table of `size' builtin
 * structures.  Those for which (.flags & BINF_ADDED) is false are to be
 * added; that flag is set if they succeed.
 *
 * If any fail, an error message is printed, using nam as the leading name.
 * Returns 0 on success, 1 for any failure.
 *
 * This should not be used from a module; instead, use handlefeatures().
 */

/**/
mod_export int
addbuiltins(char const *nam, Builtin binl, int size)
{
    int ret = 0, n;

    for(n = 0; n < size; n++) {
	Builtin b = &binl[n];
	if(b->node.flags & BINF_ADDED)
	    continue;
	if(addbuiltin(b)) {
	    zwarnnam(nam, "name clash when adding builtin `%s'", b->node.nam);
	    ret = 1;
	} else {
	    b->node.flags |= BINF_ADDED;
	}
    }
    return ret;
}


/************************************************************************
 * Function wrappers.
 ************************************************************************/

/* The list of function wrappers defined. */

/**/
FuncWrap wrappers;

/* This adds a definition for a wrapper. Return value is one in case of *
 * error and zero if all went fine. */

/**/
mod_export int
addwrapper(Module m, FuncWrap w)
{
    FuncWrap p, q;

    /*
     * We can't add a wrapper to an alias, since it's supposed
     * to behave identically to the resolved module.  This shouldn't
     * happen since we usually add wrappers when a real module is
     * loaded.
     */
    if (m->node.flags & MOD_ALIAS)
	return 1;

    if (w->flags & WRAPF_ADDED)
	return 1;
    for (p = wrappers, q = NULL; p; q = p, p = p->next);
    if (q)
	q->next = w;
    else
	wrappers = w;
    w->next = NULL;
    w->flags |= WRAPF_ADDED;
    w->module = m;

    return 0;
}

/* This removes the given wrapper definition from the list. Returned is *
 * one in case of error and zero otherwise. */

/**/
mod_export int
deletewrapper(Module m, FuncWrap w)
{
    FuncWrap p, q;

    if (m->node.flags & MOD_ALIAS)
	return 1;

    if (w->flags & WRAPF_ADDED) {
	for (p = wrappers, q = NULL; p && p != w; q = p, p = p->next);

	if (p) {
	    if (q)
		q->next = p->next;
	    else
		wrappers = p->next;
	    p->flags &= ~WRAPF_ADDED;

	    return 0;
	}
    }
    return 1;
}


/************************************************************************
 * Conditions.
 ************************************************************************/

/* The list of module-defined conditions. */

/**/
mod_export Conddef condtab;

/* This gets a condition definition with the given name. The first        *
 * argument says if we have to look for an infix condition. The last      *
 * argument is non-zero if we should autoload modules if needed. */

/**/
Conddef
getconddef(int inf, const char *name, int autol)
{
    Conddef p;
    int f = 1;
    char *lookup, *s;

    /* detokenize the Dash to the form encoded in lookup tables */
    lookup = dupstring(name);
    if (!lookup)
	return NULL;
    for (s = lookup; *s != '\0'; s++) {
	if (*s == Dash)
	    *s = '-';
    }

    do {
	for (p = condtab; p; p = p->next) {
	    if ((!!inf == !!(p->flags & CONDF_INFIX)) &&
		!strcmp(lookup, p->name))
		break;
	}
	if (autol && p && p->module) {
	    /*
	     * This is a definition for an autoloaded condition; load the
	     * module if we haven't tried that already.
	     */
	    if (f) {
		(void)ensurefeature(p->module,
				    (p->flags & CONDF_INFIX) ? "C:" : "c:",
				    (p->flags & CONDF_AUTOALL) ? NULL : lookup);
		f = 0;
		p = NULL;
	    } else {
		deleteconddef(p);
		return NULL;
	    }
	} else
	    break;
    } while (!p);

    return p;
}

/*
 * This adds the given condition definition. The return value is zero on *
 * success and 1 on failure. If there is a matching definition for an    *
 * autoloaded condition, it is removed.
 *
 * This is used for adding both an autoload definition or
 * a real condition.  In the latter case the caller is responsible
 * for setting the CONDF_ADDED flag.
 */

/**/
static int
addconddef(Conddef c)
{
    Conddef p = getconddef((c->flags & CONDF_INFIX), c->name, 0);

    if (p) {
	if (!p->module || (p->flags & CONDF_ADDED))
	    return 1;
	/* There is an autoload definition. */

	deleteconddef(p);
    }
    c->next = condtab;
    condtab = c;
    return 0;
}

/* This removes the given condition definition from the list(s). If this *
 * is a definition for a autoloaded condition, the memory is freed. */

/**/
int
deleteconddef(Conddef c)
{
    Conddef p, q;

    for (p = condtab, q = NULL; p && p != c; q = p, p = p->next);

    if (p) {
	if (q)
	    q->next = p->next;
	else
	    condtab = p->next;

	if (p->module) {
	    /* autoloaded, free it */
	    zsfree(p->name);
	    zsfree(p->module);
	    zfree(p, sizeof(*p));
	}
	return 0;
    }
    return -1;
}

/*
 * Add or remove sets of conditions.  The interface is
 * identical to setbuiltins().
 */

/**/
static int
setconddefs(char const *nam, Conddef c, int size, int *e)
{
    int ret = 0;

    while (size--) {
	if (e && *e++) {
	    if (c->flags & CONDF_ADDED) {
		c++;
		continue;
	    }
	    if (addconddef(c)) {
		zwarnnam(nam, "name clash when adding condition `%s'",
			 c->name);
		ret = 1;
	    } else {
		c->flags |= CONDF_ADDED;
	    }
	} else {
	    if (!(c->flags & CONDF_ADDED)) {
		c++;
		continue;
	    }
	    if (deleteconddef(c)) {
		zwarnnam(nam, "condition `%s' already deleted", c->name);
		ret = 1;
	    } else {
		c->flags &= ~CONDF_ADDED;
	    }
	}
	c++;
    }
    return ret;
}

/* This adds a definition for autoloading a module for a condition. */

/**/
static int
add_autocond(const char *module, const char *cnam, int flags)
{
    Conddef c;

    c = (Conddef) zalloc(sizeof(*c));

    c->name = ztrdup(cnam);
    c->flags = ((flags & FEAT_INFIX) ? CONDF_INFIX : 0);
    if (flags & FEAT_AUTOALL)
	c->flags |= CONDF_AUTOALL;
    c->module = ztrdup(module);

    if (addconddef(c)) {
	zsfree(c->name);
	zsfree(c->module);
	zfree(c, sizeof(*c));

	if (!(flags & FEAT_IGNORE))
	    return 1;
    }
    return 0;
}

/* Remove a condition added with add_autocond */

/**/
static int
del_autocond(UNUSED(const char *modnam), const char *cnam, int flags)
{
    Conddef cd = getconddef((flags & FEAT_INFIX) ? 1 : 0, cnam, 0);

    if (!cd) {
	if (!(flags & FEAT_IGNORE)) {
	    return 2;
	}
    } else if (cd->flags & CONDF_ADDED) {
	if (!(flags & FEAT_IGNORE))
	    return 3;
    } else
	deleteconddef(cd);

    return 0;
}

/************************************************************************
 * Hook functions.
 ************************************************************************/

/* This list of hook functions defined. */

/**/
Hookdef hooktab;

/* Find a hook definition given the name. */

/**/
Hookdef
gethookdef(char *n)
{
    Hookdef p;

    for (p = hooktab; p; p = p->next)
	if (!strcmp(n, p->name))
	    return p;
    return NULL;
}

/* This adds the given hook definition. The return value is zero on      *
 * success and 1 on failure.                                             */

/**/
int
addhookdef(Hookdef h)
{
    if (gethookdef(h->name))
	return 1;

    h->next = hooktab;
    hooktab = h;
    h->funcs = znewlinklist();

    return 0;
}

/*
 * This adds multiple hook definitions. This is like addbuiltins().
 * This allows a NULL module because we call it from init.c.
 */

/**/
mod_export int
addhookdefs(Module m, Hookdef h, int size)
{
    int ret = 0;

    while (size--) {
	if (addhookdef(h)) {
	    zwarnnam(m ? m->node.nam : NULL,
		     "name clash when adding hook `%s'", h->name);
	    ret = 1;
	}
	h++;
    }
    return ret;
}

/* Delete hook definitions. */

/**/
int
deletehookdef(Hookdef h)
{
    Hookdef p, q;

    for (p = hooktab, q = NULL; p && p != h; q = p, p = p->next);

    if (!p)
	return 1;

    if (q)
	q->next = p->next;
    else
	hooktab = p->next;
    freelinklist(p->funcs, NULL);
    return 0;
}

/* Remove multiple hook definitions. */

/**/
mod_export int
deletehookdefs(UNUSED(Module m), Hookdef h, int size)
{
    int ret = 0;

    while (size--) {
	if (deletehookdef(h))
	    ret = 1;
	h++;
    }
    return ret;
}

/* Add a function to a hook. */

/**/
int
addhookdeffunc(Hookdef h, Hookfn f)
{
    zaddlinknode(h->funcs, (void *) f);

    return 0;
}

/**/
mod_export int
addhookfunc(char *n, Hookfn f)
{
    Hookdef h = gethookdef(n);

    if (h)
	return addhookdeffunc(h, f);
    return 1;
}

/* Delete a function from a hook. */

/**/
int
deletehookdeffunc(Hookdef h, Hookfn f)
{
    LinkNode p;

    for (p = firstnode(h->funcs); p; incnode(p))
	if (f == (Hookfn) getdata(p)) {
	    remnode(h->funcs, p);
	    return 0;
	}
    return 1;
}

/* Delete a hook. */

/**/
mod_export int
deletehookfunc(char *n, Hookfn f)
{
    Hookdef h = gethookdef(n);

    if (h)
	return deletehookdeffunc(h, f);
    return 1;
}

/* Run the function(s) for a hook. */

/**/
mod_export int
runhookdef(Hookdef h, void *d)
{
    if (empty(h->funcs)) {
	if (h->def)
	    return h->def(h, d);
	return 0;
    } else if (h->flags & HOOKF_ALL) {
	LinkNode p;
	int r;

	for (p = firstnode(h->funcs); p; incnode(p))
	    if ((r = ((Hookfn) getdata(p))(h, d)))
		return r;
	if (h->def)
	    return h->def(h, d);
	return 0;
    } else
	return ((Hookfn) getdata(lastnode(h->funcs)))(h, d);
}



/************************************************************************
 * Shell parameters.
 ************************************************************************/

/*
 * Check that it's possible to add a parameter.  This
 * requires that either there's no parameter already present,
 * or it's a global parameter marked for autoloading.
 *
 * The special status 2 is to indicate it didn't work but
 * -i was in use so we didn't print a warning.
 */

static int
checkaddparam(const char *nam, int opt_i)
{
    Param pm;

    if (!(pm = (Param) gethashnode2(paramtab, nam)))
	return 0;

    if (pm->level || !(pm->node.flags & PM_AUTOLOAD)) {
	/*
	 * -i suppresses "it's already that way" warnings,
	 * but not "this can't possibly work" warnings, so we print
	 * the message anyway if there's a local parameter blocking
	 * the parameter we want to add, not if there's a
	 * non-autoloadable parameter already there.  This
	 * is consistent with the way add_auto* functions work.
	 */
	if (!opt_i || pm->level) {
	    zwarn("Can't add module parameter `%s': %s",
		  nam, pm->level ?
		  "local parameter exists" :
		  "parameter already exists");
	    return 1;
	}
	return 2;
    }

    unsetparam_pm(pm, 0, 1);
    return 0;
}

/* This adds the given parameter definition. The return value is zero on *
 * success and 1 on failure. */

/**/
int
addparamdef(Paramdef d)
{
    Param pm;

    if (checkaddparam(d->name, 0))
	return 1;

    if (d->getnfn) {
	if (!(pm = createspecialhash(d->name, d->getnfn,
				     d->scantfn, d->flags)))
	    return 1;
    }
    else if (!(pm = createparam(d->name, d->flags)) &&
	!(pm = (Param) paramtab->getnode(paramtab, d->name)))
	return 1;

    d->pm = pm;
    pm->level = 0;
    if (d->var)
	pm->u.data = d->var;
    if (d->var || d->gsu) {
	/*
	 * If no get/set/unset class, use the appropriate
	 * variable type, else use the one supplied.
	 */
	switch (PM_TYPE(pm->node.flags)) {
	case PM_SCALAR:
	    pm->gsu.s = d->gsu ? (GsuScalar)d->gsu : &varscalar_gsu;
	    break;

	case PM_INTEGER:
	    pm->gsu.i = d->gsu ? (GsuInteger)d->gsu : &varinteger_gsu;
	    break;

	case PM_FFLOAT:
	case PM_EFLOAT:
	    pm->gsu.f = d->gsu;
	    break;

	case PM_ARRAY:
	    pm->gsu.a = d->gsu ? (GsuArray)d->gsu : &vararray_gsu;
	    break;

	case PM_HASHED:
	    /* hashes may behave like standard hashes */
	    if (d->gsu)
		pm->gsu.h = (GsuHash)d->gsu;
	    break;

	default:
	    unsetparam_pm(pm, 0, 1);
	    return 1;
	}
    }

    return 0;
}

/* Delete parameters defined. No error checking yet. */

/**/
int
deleteparamdef(Paramdef d)
{
    Param pm = (Param) paramtab->getnode(paramtab, d->name);

    if (!pm)
	return 1;
    if (pm != d->pm) {
	/*
	 * See if the parameter has been hidden.  If so,
	 * bring it to the front to unset it.
	 */
	Param prevpm, searchpm;
	for (prevpm = pm, searchpm = pm->old;
	     searchpm;
	     prevpm = searchpm, searchpm = searchpm->old)
	    if (searchpm == d->pm)
		break;

	if (!searchpm)
	    return 1;

	paramtab->removenode(paramtab, pm->node.nam);
	prevpm->old = searchpm->old;
	searchpm->old = pm;
	paramtab->addnode(paramtab, searchpm->node.nam, searchpm);

	pm = searchpm;
    }
    pm->node.flags = (pm->node.flags & ~PM_READONLY) | PM_REMOVABLE;
    unsetparam_pm(pm, 0, 1);
    d->pm = NULL;
    return 0;
}

/*
 * Add or remove sets of parameters.  The interface is
 * identical to setbuiltins().
 */

/**/
static int
setparamdefs(char const *nam, Paramdef d, int size, int *e)
{
    int ret = 0;

    while (size--) {
	if (e && *e++) {
	    if (d->pm) {
		d++;
		continue;
	    }
	    if (addparamdef(d)) {
		zwarnnam(nam, "error when adding parameter `%s'", d->name);
		ret = 1;
	    }
	} else {
	    if (!d->pm) {
		d++;
		continue;
	    }
	    if (deleteparamdef(d)) {
		zwarnnam(nam, "parameter `%s' already deleted", d->name);
		ret = 1;
	    }
	}
	d++;
    }
    return ret;
}

/* This adds a definition for autoloading a module for a parameter. */

/**/
static int
add_autoparam(const char *module, const char *pnam, int flags)
{
    Param pm;
    int ret;

    queue_signals();
    if ((ret = checkaddparam(pnam, (flags & FEAT_IGNORE)))) {
	unqueue_signals();
	/*
	 * checkaddparam() has already printed a message if one was
	 * needed.  If it wasn't owing to the presence of -i, ret is 2;
	 * for consistency with other add_auto* functions we return
	 * status 0 to indicate there's already such a parameter and
	 * we've been told not to worry if so.
	 */
	return ret == 2 ? 0 : -1;
    }

    pm = setsparam(dupstring(pnam), ztrdup(module));

    pm->node.flags |= PM_AUTOLOAD;
    if (flags & FEAT_AUTOALL)
	pm->node.flags |= PM_AUTOALL;
    unqueue_signals();

    return 0;
}

/* Remove a parameter added with add_autoparam() */

/**/
static int
del_autoparam(UNUSED(const char *modnam), const char *pnam, int flags)
{
    Param pm = (Param) gethashnode2(paramtab, pnam);

    if (!pm) {
	if (!(flags & FEAT_IGNORE))
	    return 2;
    } else if (!(pm->node.flags & PM_AUTOLOAD)) {
	if (!(flags & FEAT_IGNORE))
	    return 3;
    } else
	unsetparam_pm(pm, 0, 1);

    return 0;
}

/************************************************************************
 * Math functions.
 ************************************************************************/

/* List of math functions. */

/**/
MathFunc mathfuncs;

/*
 * Remove a single math function form the list (utility function).
 * This does not delete a module math function, that's deletemathfunc().
 */

/**/
void
removemathfunc(MathFunc previous, MathFunc current)
{
    if (previous)
	previous->next = current->next;
    else
	mathfuncs = current->next;

    zsfree(current->name);
    zsfree(current->module);
    zfree(current, sizeof(*current));
}

/* Find a math function in the list, handling autoload if necessary. */

/**/
MathFunc
getmathfunc(const char *name, int autol)
{
    MathFunc p, q = NULL;

    for (p = mathfuncs; p; q = p, p = p->next)
	if (!strcmp(name, p->name)) {
	    if (autol && p->module && !(p->flags & MFF_USERFUNC)) {
		char *n = dupstring(p->module);
		int flags = p->flags;

		removemathfunc(q, p);

		(void)ensurefeature(n, "f:", (flags & MFF_AUTOALL) ? NULL :
				    name);

	       p = getmathfunc(name, 0);
	       if (!p) {
		   zerr("autoloading module %s failed to define math function: %s", n, name);
	       }
	    }
	    return p;
	}

    return NULL;
}

/* Add a single math function */

/**/
static int
addmathfunc(MathFunc f)
{
    MathFunc p, q = NULL;

    if (f->flags & MFF_ADDED)
	return 1;

    for (p = mathfuncs; p; q = p, p = p->next)
	if (!strcmp(f->name, p->name)) {
	    if (p->module && !(p->flags & MFF_USERFUNC)) {
		/*
		 * Autoloadable, replace.
		 */
		removemathfunc(q, p);
		break;
	    }
	    return 1;
	}

    f->next = mathfuncs;
    mathfuncs = f;

    return 0;
}

/* Delete a single math function */

/**/
mod_export int
deletemathfunc(MathFunc f)
{
    MathFunc p, q;

    for (p = mathfuncs, q = NULL; p && p != f; q = p, p = p->next);

    if (p) {
	if (q)
	    q->next = f->next;
	else
	    mathfuncs = f->next;

	/* the following applies to both unloaded and user-defined functions */
	if (f->module) {
	    zsfree(f->name);
	    zsfree(f->module);
	    zfree(f, sizeof(*f));
	} else
	    f->flags &= ~MFF_ADDED;

	return 0;
    }
    return -1;
}

/*
 * Add or remove sets of math functions.  The interface is
 * identical to setbuiltins().
 */

/**/
static int
setmathfuncs(char const *nam, MathFunc f, int size, int *e)
{
    int ret = 0;

    while (size--) {
	if (e && *e++) {
	    if (f->flags & MFF_ADDED) {
		f++;
		continue;
	    }
	    if (addmathfunc(f)) {
		zwarnnam(nam, "name clash when adding math function `%s'",
			 f->name);
		ret = 1;
	    } else {
		f->flags |= MFF_ADDED;
	    }
	} else {
	    if (!(f->flags & MFF_ADDED)) {
		f++;
		continue;
	    }
	    if (deletemathfunc(f)) {
		zwarnnam(nam, "math function `%s' already deleted", f->name);
		ret = 1;
	    }
	}
	f++;
    }
    return ret;
}

/* Add an autoload definition for a math function. */

/**/
static int
add_automathfunc(const char *module, const char *fnam, int flags)
{
    MathFunc f;

    f = (MathFunc) zalloc(sizeof(*f));

    f->name = ztrdup(fnam);
    f->module = ztrdup(module);
    f->flags = 0;

    if (addmathfunc(f)) {
	zsfree(f->name);
	zsfree(f->module);
	zfree(f, sizeof(*f));

	if (!(flags & FEAT_IGNORE))
	    return 1;
    }

    return 0;
}

/* Remove a math function added with add_automathfunc() */

/**/
static int
del_automathfunc(UNUSED(const char *modnam), const char *fnam, int flags)
{
    MathFunc f = getmathfunc(fnam, 0);
    
    if (!f) {
	if (!(flags & FEAT_IGNORE))
	    return 2;
    } else if (f->flags & MFF_ADDED) {
	if (!(flags & FEAT_IGNORE))
	    return 3;
    } else
	deletemathfunc(f);

    return 0;
}

/************************************************************************
 * Now support for dynamical loading and the fallback functions
 * we use for loading if dynamical loading is not available.
 ************************************************************************/

/**/
#ifdef DYNAMIC

/**/
#ifdef AIXDYNAMIC

#include <sys/ldr.h>

static char *dlerrstr[256];

static void *
load_and_bind(const char *fn)
{
    void *ret = (void *) load((char *) fn, L_NOAUTODEFER, NULL);

    if (ret) {
	Module m;
	int i, err = loadbind(0, (void *) addbuiltin, ret);
	for (i = 0; i < modulestab->hsize && !err; i++) {
	    for (m = (Module)modulestab->nodes[i]; m && !err;
		 m = (Module)m->node.next) {
		if (!(m->node.flags & MOD_ALIAS) &&
		    m->u.handle && !(m->node.flags & MOD_LINKED))
		    err |= loadbind(0, m->u.handle, ret);
	    }
	}

	if (err) {
	    loadquery(L_GETMESSAGES, dlerrstr, sizeof(dlerrstr));
	    unload(ret);
	    ret = NULL;
	}
    } else
	loadquery(L_GETMESSAGES, dlerrstr, sizeof(dlerrstr));

    return ret;
}

#define dlopen(X,Y) load_and_bind(X)
#define dlclose(X)  unload(X)
#define dlerror()   (dlerrstr[0])
#ifndef HAVE_DLERROR
# define HAVE_DLERROR 1
#endif

/**/
#else

#ifdef HAVE_DLFCN_H
# if defined(HAVE_DL_H) && defined(HPUX10DYNAMIC)
#  include <dl.h>
# else
#  include <dlfcn.h>
# endif
#else
# ifdef HAVE_DL_H
#  include <dl.h>
#  define RTLD_LAZY BIND_DEFERRED
#  define RTLD_GLOBAL DYNAMIC_PATH
# else
#  include <sys/types.h>
#  include <nlist.h>
#  include <link.h>
# endif
#endif

/**/
#ifdef HPUX10DYNAMIC
# define dlopen(file,mode) (void *)shl_load((file), (mode), (long) 0)
# define dlclose(handle) shl_unload((shl_t)(handle))

static
void *
hpux_dlsym(void *handle, char *name)
{
    void *sym_addr;
    if (!shl_findsym((shl_t *)&handle, name, TYPE_UNDEFINED, &sym_addr))
	return sym_addr;
    return NULL;
}

# define dlsym(handle,name) hpux_dlsym(handle,name)
# ifdef HAVE_DLERROR		/* paranoia */
#  undef HAVE_DLERROR
# endif
#else
# ifndef HAVE_DLCLOSE
#  define dlclose(X) ((X), 0)
# endif
/**/
#endif

#ifdef DLSYM_NEEDS_UNDERSCORE
# define STR_SETUP     "_setup_"
# define STR_FEATURES  "_features_"
# define STR_ENABLES   "_enables_"
# define STR_BOOT      "_boot_"
# define STR_CLEANUP   "_cleanup_"
# define STR_FINISH    "_finish_"
#else /* !DLSYM_NEEDS_UNDERSCORE */
# define STR_SETUP     "setup_"
# define STR_FEATURES  "features_"
# define STR_ENABLES   "enables_"
# define STR_BOOT      "boot_"
# define STR_CLEANUP   "cleanup_"
# define STR_FINISH    "finish_"
#endif /* !DLSYM_NEEDS_UNDERSCORE */

/**/
#endif /* !AIXDYNAMIC */

#ifndef RTLD_LAZY
# define RTLD_LAZY 1
#endif
#ifndef RTLD_GLOBAL
# define RTLD_GLOBAL 0
#endif

/*
 * Attempt to load a module.  This is the lowest level of
 * zsh function for dynamical modules.  Returns the handle
 * from the dynamic loader.
 */

/**/
static void *
try_load_module(char const *name)
{
    char buf[PATH_MAX + 1];
    char **pp;
    void *ret = NULL;
    int l;

    l = 1 + strlen(name) + 1 + strlen(DL_EXT);
    for (pp = module_path; !ret && *pp; pp++) {
	if (l + (**pp ? strlen(*pp) : 1) > PATH_MAX)
	    continue;
	sprintf(buf, "%s/%s.%s", **pp ? *pp : ".", name, DL_EXT);
	unmetafy(buf, NULL);
	if (*buf) /* dlopen(NULL) returns a handle to the main binary */
	    ret = dlopen(buf, RTLD_LAZY | RTLD_GLOBAL);
    }

    return ret;
}

/*
 * Load a module, with option to complain or not.
 * Returns the handle from the dynamic loader.
 */

/**/
static void *
do_load_module(char const *name, int silent)
{
    void *ret;

    ret = try_load_module(name);
    if (!ret && !silent) {
#ifdef HAVE_DLERROR
	char *errstr = dlerror();
	zwarn("failed to load module `%s': %s", name,
	      errstr ? metafy(errstr, -1, META_HEAPDUP) : "empty module path");
#else
	zwarn("failed to load module: %s", name);
#endif
    }
    return ret;
}

/**/
#else /* !DYNAMIC */

/*
 * Dummy loader when no dynamic loading available; always fails.
 */

/**/
static void *
do_load_module(char const *name, int silent)
{
    if (!silent)
	zwarn("failed to load module: %s", name);

    return NULL;
}

/**/
#endif /* !DYNAMIC */

/*
 * Find a module in the list.
 * flags is a set of bits defined in the enum above.
 * If namep is set, this is set to point to the last alias value resolved,
 *   even if that module was not loaded. or the module name if no aliases.
 *   Hence this is always the physical module to load in a chain of aliases.
 * Return NULL if the module named is not stored as a structure, or if we were
 * resolving aliases and the final module named is not stored as a
 * structure.
 */
/**/
static Module
find_module(const char *name, int flags, const char **namep)
{
    Module m;

    m = (Module)modulestab->getnode2(modulestab, name);
    if (m) {
	if ((flags & FINDMOD_ALIASP) && (m->node.flags & MOD_ALIAS)) {
	    if (namep)
		*namep = m->u.alias;
	    return find_module(m->u.alias, flags, namep);
	}
	if (namep)
	    *namep = m->node.nam;
	return m;
    }
    if (!(flags & FINDMOD_CREATE))
	return NULL;
    m = zshcalloc(sizeof(*m));
    modulestab->addnode(modulestab, ztrdup(name), m);
    return m;
}

/*
 * Unlink and free a module node from the linked list.
 */

/**/
static void
delete_module(Module m)
{
    modulestab->removenode(modulestab, m->node.nam);

    modulestab->freenode(&m->node);
}

/*
 * Return 1 if a module is fully loaded else zero.
 * A linked module may be marked as unloaded even though
 * we can't fully unload it; this returns 0 to try to
 * make that state transparently like an unloaded module.
 */

/**/
mod_export int
module_loaded(const char *name)
{
    Module m;

    return ((m = find_module(name, FINDMOD_ALIASP, NULL)) &&
	    m->u.handle &&
	    !(m->node.flags & MOD_UNLOAD));
}

/*
 * Setup and cleanup functions:  we don't search for aliases here,
 * since they should have been resolved before we try to load or unload
 * the module.
 */

/**/
#ifdef DYNAMIC

/**/
#ifdef AIXDYNAMIC

/**/
static int
dyn_setup_module(Module m)
{
    return ((int (*)_((int,Module, void*))) m->u.handle)(0, m, NULL);
}

/**/
static int
dyn_features_module(Module m, char ***features)
{
    return ((int (*)_((int,Module, void*))) m->u.handle)(4, m, features);
}

/**/
static int
dyn_enables_module(Module m, int **enables)
{
    return ((int (*)_((int,Module, void*))) m->u.handle)(5, m, enables);
}

/**/
static int
dyn_boot_module(Module m)
{
    return ((int (*)_((int,Module, void*))) m->u.handle)(1, m, NULL);
}

/**/
static int
dyn_cleanup_module(Module m)
{
    return ((int (*)_((int,Module, void*))) m->u.handle)(2, m, NULL);
}

/**/
static int
dyn_finish_module(Module m)
{
    return ((int (*)_((int,Module,void *))) m->u.handle)(3, m, NULL);
}

/**/
#else

static Module_generic_func
module_func(Module m, char *name)
{
#ifdef DYNAMIC_NAME_CLASH_OK
    return (Module_generic_func) dlsym(m->u.handle, name);
#else /* !DYNAMIC_NAME_CLASH_OK */
    VARARR(char, buf, strlen(name) + strlen(m->node.nam)*2 + 1);
    char const *p;
    char *q;
    strcpy(buf, name);
    q = strchr(buf, 0);
    for(p = m->node.nam; *p; p++) {
	if(*p == '/') {
	    *q++ = 'Q';
	    *q++ = 's';
	} else if(*p == '_') {
	    *q++ = 'Q';
	    *q++ = 'u';
	} else if(*p == 'Q') {
	    *q++ = 'Q';
	    *q++ = 'q';
	} else
	    *q++ = *p;
    }
    *q = 0;
    return (Module_generic_func) dlsym(m->u.handle, buf);
#endif /* !DYNAMIC_NAME_CLASH_OK */
}

/**/
static int
dyn_setup_module(Module m)
{
    Module_void_func fn = (Module_void_func)module_func(m, STR_SETUP);

    if (fn)
	return fn(m);
    zwarnnam(m->node.nam, "no setup function");
    return 1;
}

/**/
static int
dyn_features_module(Module m, char ***features)
{
    Module_features_func fn =
	(Module_features_func)module_func(m, STR_FEATURES);

    if (fn)
	return fn(m, features);
    /* not a user-visible error if no features function */
    return 1;
}

/**/
static int
dyn_enables_module(Module m, int **enables)
{
    Module_enables_func fn = (Module_enables_func)module_func(m, STR_ENABLES);

    if (fn)
	return fn(m, enables);
    /* not a user-visible error if no enables function */
    return 1;
}

/**/
static int
dyn_boot_module(Module m)
{
    Module_void_func fn = (Module_void_func)module_func(m, STR_BOOT);

    if(fn)
	return fn(m);
    zwarnnam(m->node.nam, "no boot function");
    return 1;
}

/**/
static int
dyn_cleanup_module(Module m)
{
    Module_void_func fn = (Module_void_func)module_func(m, STR_CLEANUP);

    if(fn)
	return fn(m);
    zwarnnam(m->node.nam, "no cleanup function");
    return 1;
}

/* Note that this function does more than just calling finish_foo(), *
 * it really unloads the module. */

/**/
static int
dyn_finish_module(Module m)
{
    Module_void_func fn = (Module_void_func)module_func(m, STR_FINISH);
    int r;

    if (fn)
	r = fn(m);
    else {
	zwarnnam(m->node.nam, "no finish function");
	r = 1;
    }
    dlclose(m->u.handle);
    return r;
}

/**/
#endif /* !AIXDYNAMIC */

/**/
static int
setup_module(Module m)
{
    return ((m->node.flags & MOD_LINKED) ?
	    (m->u.linked->setup)(m) : dyn_setup_module(m));
}

/**/
static int
features_module(Module m, char ***features)
{
    return ((m->node.flags & MOD_LINKED) ?
	    (m->u.linked->features)(m, features) :
	    dyn_features_module(m, features));
}

/**/
static int
enables_module(Module m, int **enables)
{
    return ((m->node.flags & MOD_LINKED) ?
	    (m->u.linked->enables)(m, enables) :
	    dyn_enables_module(m, enables));
}

/**/
static int
boot_module(Module m)
{
    return ((m->node.flags & MOD_LINKED) ?
	    (m->u.linked->boot)(m) : dyn_boot_module(m));
}

/**/
static int
cleanup_module(Module m)
{
    return ((m->node.flags & MOD_LINKED) ?
	    (m->u.linked->cleanup)(m) : dyn_cleanup_module(m));
}

/**/
static int
finish_module(Module m)
{
    return ((m->node.flags & MOD_LINKED) ?
	    (m->u.linked->finish)(m) : dyn_finish_module(m));
}

/**/
#else /* !DYNAMIC */

/**/
static int
setup_module(Module m)
{
    return ((m->node.flags & MOD_LINKED) ? (m->u.linked->setup)(m) : 1);
}

/**/
static int
features_module(Module m, char ***features)
{
    return ((m->node.flags & MOD_LINKED) ? (m->u.linked->features)(m, features)
	    : 1);
}

/**/
static int
enables_module(Module m, int **enables)
{
    return ((m->node.flags & MOD_LINKED) ? (m->u.linked->enables)(m, enables)
	    : 1);
}

/**/
static int
boot_module(Module m)
{
    return ((m->node.flags & MOD_LINKED) ? (m->u.linked->boot)(m) : 1);
}

/**/
static int
cleanup_module(Module m)
{
    return ((m->node.flags & MOD_LINKED) ? (m->u.linked->cleanup)(m) : 1);
}

/**/
static int
finish_module(Module m)
{
    return ((m->node.flags & MOD_LINKED) ? (m->u.linked->finish)(m) : 1);
}

/**/
#endif /* !DYNAMIC */


/************************************************************************
 * Functions called when manipulating modules
 ************************************************************************/

/*
 * Set the features for the module, which must be loaded
 * by now (though may not be fully set up).
 *
 * Return 0 for success, 1 for failure, 2 if some features
 * couldn't be set by the module itself (non-existent features
 * are tested here and cause 1 to be returned).
 */

/**/
static int
do_module_features(Module m, Feature_enables enablesarr, int flags)
{
    char **features;
    int ret = 0;

    if (features_module(m, &features) == 0) {
	/*
	 * Features are supported.  If we were passed
	 * a NULL array, enable all features, else
	 * enable only the features listed.
	 * (This may in principle be an empty array,
	 * although that's not very pointful.)
	 */
	int *enables = NULL;
	if (enables_module(m, &enables)) {
	    /* If features are supported, enables should be, too */
	    if (!(flags & FEAT_IGNORE))
		zwarn("error getting enabled features for module `%s'",
		      m->node.nam);
	    return 1;
	}

	if ((flags & FEAT_CHECKAUTO) && m->autoloads) {
	    /*
	     * Check autoloads are available.  Since these
	     * have been requested at some other point, they
	     * don't affect the return status unless something
	     * in enablesstr doesn't work.
	     */
	    LinkNode an, nextn;
	    for (an = firstnode(m->autoloads); an; an = nextn) {
		char *al = (char *)getdata(an), **ptr;
		/* careful, we can delete the current node */
		nextn = nextnode(an);
		for (ptr = features; *ptr; ptr++)
		    if (!strcmp(al, *ptr))
			break;
		if (!*ptr) {
		    char *arg[2];
		    if (!(flags & FEAT_IGNORE))
			zwarn(
		    "module `%s' has no such feature: `%s': autoload cancelled",
		    m->node.nam, al);
		    /*
		     * This shouldn't happen, so it's not worth optimising
		     * the call to autofeatures...
		     */
		    arg[0] = al = dupstring(al);
		    arg[1] = NULL;
		    (void)autofeatures(NULL, m->node.nam, arg, 0,
				       FEAT_IGNORE|FEAT_REMOVE);
		    /*
		     * don't want to try to enable *that*...
		     * expunge it from the enable string.
		     */
		    if (enablesarr) {
			Feature_enables fep;
			for (fep = enablesarr; fep->str; fep++) {
			    char *str = fep->str;
			    if (*str == '+' || *str == '-')
				str++;
			    if (fep->pat ? pattry(fep->pat, al) :
				!strcmp(al, str)) {
				/* can't enable it after all, so return 1 */
				ret = 1;
				while (fep->str) {
				    fep->str = fep[1].str;
				    fep->pat = fep[1].pat;
				    fep++;
				}
				if (!fep->pat)
				    break;
			    }
			}
		    }
		}
	    }
	}

	if (enablesarr) {
	    Feature_enables fep;
	    for (fep = enablesarr; fep->str; fep++) {
		char **fp, *esp = fep->str;
		int on = 1, found = 0;
		if (*esp == '+')
		    esp++;
		else if (*esp == '-') {
		    on = 0;
		    esp++;
		}
		for (fp = features; *fp; fp++)
		    if (fep->pat ? pattry(fep->pat, *fp) : !strcmp(*fp, esp)) {
			enables[fp - features] = on;
			found++;
			if (!fep->pat)
			    break;
		    }
		if (!found) {
		    if (!(flags & FEAT_IGNORE))
			zwarn(fep->pat ?
			      "module `%s' has no feature matching: `%s'" :
			      "module `%s' has no such feature: `%s'",
			      m->node.nam, esp);
		    return 1;
		}
	    }
	} else {
	    /*
	     * Enable all features.  This is used when loading
	     * without using zmodload -F.
	     */
	    int n_features = arrlen(features);
	    int *ep;
	    for (ep = enables; n_features--; ep++)
		*ep = 1;
	}

	if (enables_module(m, &enables))
	    return 2;
    } else if (enablesarr) {
	if (!(flags & FEAT_IGNORE))
	    zwarn("module `%s' does not support features", m->node.nam);
	return 1;
    }
    /* Else it doesn't support features but we don't care. */

    return ret;
}

/*
 * Boot the module, including setting up features.
 * As we've only just loaded the module, we don't yet
 * know what features it supports, so we get them passed
 * as a string.
 *
 * Returns 0 if OK, 1 if completely failed, 2 if some features
 * couldn't be set up.
 */

/**/
static int
do_boot_module(Module m, Feature_enables enablesarr, int silent)
{
    int ret = do_module_features(m, enablesarr,
				 silent ? FEAT_IGNORE|FEAT_CHECKAUTO :
				 FEAT_CHECKAUTO);

    if (ret == 1)
	return 1;

    if (boot_module(m))
	return 1;
    return ret;
}

/*
 * Cleanup the module.
 */

/**/
static int
do_cleanup_module(Module m)
{
    return (m->node.flags & MOD_LINKED) ?
	(m->u.linked && m->u.linked->cleanup(m)) :
	(m->u.handle && cleanup_module(m));
}

/*
 * Test a module name contains only valid characters: those
 * allowed in a shell identifier plus slash.  Return 1 if so.
 */

/**/
static int
modname_ok(char const *p)
{
    do {
	p = itype_end(p, IIDENT, 0);
	if (!*p)
	    return 1;
    } while(*p++ == '/');
    return 0;
}

/*
 * High level function to load a module, encapsulating
 * all the handling of module functions.
 *
 * "*enablesstr" is NULL if the caller is not feature-aware;
 * then the module should turn on all features.  If it
 * is not NULL it points to an array of features to be
 * turned on.  This function is responsible for testing whether
 * the module supports those features.
 *
 * If "silent" is 1, don't issue warnings for errors.
 *
 * Now returns 0 for success (changed post-4.3.4),
 * 1 for complete failure, 2 if some features couldn't be set.
 */

/**/
mod_export int
load_module(char const *name, Feature_enables enablesarr, int silent)
{
    Module m;
    void *handle = NULL;
    Linkedmod linked;
    int set, bootret;

    if (!modname_ok(name)) {
	if (!silent)
	    zerr("invalid module name `%s'", name);
	return 1;
    }
    /*
     * The following function call may alter name to the final name in a
     * chain of aliases.  This makes sure the actual module loaded
     * is the right one.
     */
    queue_signals();
    if (!(m = find_module(name, FINDMOD_ALIASP, &name))) {
	if (!(linked = module_linked(name)) &&
	    !(handle = do_load_module(name, silent))) {
	    unqueue_signals();
	    return 1;
	}
	m = zshcalloc(sizeof(*m));
	if (handle) {
	    m->u.handle = handle;
	    m->node.flags |= MOD_SETUP;
	} else {
	    m->u.linked = linked;
	    m->node.flags |= MOD_SETUP | MOD_LINKED;
	}
	modulestab->addnode(modulestab, ztrdup(name), m);

	if ((set = setup_module(m)) ||
	    (bootret = do_boot_module(m, enablesarr, silent)) == 1) {
	    if (!set)
		do_cleanup_module(m);
	    finish_module(m);
	    delete_module(m);
	    unqueue_signals();
	    return 1;
	}
	m->node.flags |= MOD_INIT_S | MOD_INIT_B;
	m->node.flags &= ~MOD_SETUP;
	unqueue_signals();
	return bootret;
    }
    if (m->node.flags & MOD_SETUP) {
	unqueue_signals();
	return 0;
    }
    if (m->node.flags & MOD_UNLOAD)
	m->node.flags &= ~MOD_UNLOAD;
    else if ((m->node.flags & MOD_LINKED) ? m->u.linked : m->u.handle) {
	unqueue_signals();
	return 0;
    }
    if (m->node.flags & MOD_BUSY) {
	unqueue_signals();
	zerr("circular dependencies for module ;%s", name);
	return 1;
    }
    m->node.flags |= MOD_BUSY;
    /*
     * TODO: shouldn't we unload the module if one of
     * its dependencies fails?
     */
    if (m->deps) {
	LinkNode n;
	for (n = firstnode(m->deps); n; incnode(n))
	    if (load_module((char *) getdata(n), NULL, silent) == 1) {
		m->node.flags &= ~MOD_BUSY;
		unqueue_signals();
		return 1;
	    }
    }
    m->node.flags &= ~MOD_BUSY;
    if (!m->u.handle) {
	handle = NULL;
	if (!(linked = module_linked(name)) &&
	    !(handle = do_load_module(name, silent))) {
	    unqueue_signals();
	    return 1;
	}
	if (handle) {
	    m->u.handle = handle;
	    m->node.flags |= MOD_SETUP;
	} else {
	    m->u.linked = linked;
	    m->node.flags |= MOD_SETUP | MOD_LINKED;
	}
	if (setup_module(m)) {
	    finish_module(m);
	    if (handle)
		m->u.handle = NULL;
	    else
		m->u.linked = NULL;
	    m->node.flags &= ~MOD_SETUP;
	    unqueue_signals();
	    return 1;
	}
	m->node.flags |= MOD_INIT_S;
    }
    m->node.flags |= MOD_SETUP;
    if ((bootret = do_boot_module(m, enablesarr, silent)) == 1) {
	do_cleanup_module(m);
	finish_module(m);
	if (m->node.flags & MOD_LINKED)
	    m->u.linked = NULL;
	else
	    m->u.handle = NULL;
	m->node.flags &= ~MOD_SETUP;
	unqueue_signals();
	return 1;
    }
    m->node.flags |= MOD_INIT_B;
    m->node.flags &= ~MOD_SETUP;
    unqueue_signals();
    return bootret;
}

/* This ensures that the module with the name given as the first argument
 * is loaded.
 * The other argument is the array of features to set.  If this is NULL
 * all features are enabled (even if the module was already loaded).
 *
 * If this is non-NULL the module features are set accordingly
 * whether or not the module is loaded; it is an error if the
 * module does not support the features passed (even if the feature
 * is to be turned off) or if the module does not support features
 * at all.
 * The return value is 0 if the module was found or loaded
 * (this changed post-4.3.4, because I got so confused---pws),
 * 1 if loading failed completely, 2 if some features couldn't be set.
 *
 * This function behaves like load_module() except that it
 * handles the case where the module was already loaded, and
 * sets features accordingly.
 */

/**/
mod_export int
require_module(const char *module, Feature_enables features, int silent)
{
    Module m = NULL;
    int ret = 0;

    /* Resolve aliases and actual loadable module as for load_module */
    queue_signals();
    m = find_module(module, FINDMOD_ALIASP, &module);
    if (!m || !m->u.handle ||
	(m->node.flags & MOD_UNLOAD))
	ret = load_module(module, features, silent);
    else
	ret = do_module_features(m, features, 0);
    unqueue_signals();

    return ret;
}

/*
 * Indicate that the module named "name" depends on the module
 * named "from".
 */

/**/
void
add_dep(const char *name, char *from)
{
    LinkNode node;
    Module m;

    /*
     * If we were passed an alias, we must resolve it to a final
     * module name (and maybe add the corresponding struct), since otherwise
     * we would need to check all modules to see if they happen
     * to be aliased to the same thing to implement dependencies properly.
     *
     * This should mean that an attempt to add an alias which would
     * have the same name as a module which has dependencies is correctly
     * rejected, because then the module named already exists as a non-alias.
     * Better make sure.  (There's no problem making a an alias which
     * *points* to a module with dependencies, of course.)
     */
    m = find_module(name, FINDMOD_ALIASP|FINDMOD_CREATE, &name);
    if (!m->deps)
	m->deps = znewlinklist();
    for (node = firstnode(m->deps);
	 node && strcmp((char *) getdata(node), from);
	 incnode(node));
    if (!node)
	zaddlinknode(m->deps, ztrdup(from));
}

/*
 * Function to be used when scanning the builtins table to
 * find and print autoloadable builtins.
 */

/**/
static void
autoloadscan(HashNode hn, int printflags)
{
    Builtin bn = (Builtin) hn;

    if(bn->node.flags & BINF_ADDED)
	return;
    if(printflags & PRINT_LIST) {
	fputs("zmodload -ab ", stdout);
	if(bn->optstr[0] == '-')
	    fputs("-- ", stdout);
	quotedzputs(bn->optstr, stdout);
	if(strcmp(bn->node.nam, bn->optstr)) {
	    putchar(' ');
	    quotedzputs(bn->node.nam, stdout);
	}
    } else {
	nicezputs(bn->node.nam, stdout);
	if(strcmp(bn->node.nam, bn->optstr)) {
	    fputs(" (", stdout);
	    nicezputs(bn->optstr, stdout);
	    putchar(')');
	}
    }
    putchar('\n');
}


/************************************************************************
 * Handling for the zmodload builtin and its various options.
 ************************************************************************/

/*
 * Main builtin entry point for zmodload.
 */

/**/
int
bin_zmodload(char *nam, char **args, Options ops, UNUSED(int func))
{
    int ops_bcpf = OPT_ISSET(ops,'b') || OPT_ISSET(ops,'c') || 
	OPT_ISSET(ops,'p') || OPT_ISSET(ops,'f');
    int ops_au = OPT_ISSET(ops,'a') || OPT_ISSET(ops,'u');
    int ret = 1, autoopts;
    /* options only allowed with -F */
    char *fonly = "lP", *fp;

    if (ops_bcpf && !ops_au) {
	zwarnnam(nam, "-b, -c, -f, and -p must be combined with -a or -u");
	return 1;
    }
    if (OPT_ISSET(ops,'F') && (ops_bcpf || OPT_ISSET(ops,'u'))) {
	zwarnnam(nam, "-b, -c, -f, -p and -u cannot be combined with -F");
	return 1;
    }
    if (OPT_ISSET(ops,'A') || OPT_ISSET(ops,'R')) {
	if (ops_bcpf || ops_au || OPT_ISSET(ops,'d') || 
	    (OPT_ISSET(ops,'R') && OPT_ISSET(ops,'e'))) {
	    zwarnnam(nam, "illegal flags combined with -A or -R");
	    return 1;
	}
	if (!OPT_ISSET(ops,'e'))
	    return bin_zmodload_alias(nam, args, ops);
    }
    if (OPT_ISSET(ops,'d') && OPT_ISSET(ops,'a')) {
	zwarnnam(nam, "-d cannot be combined with -a");
	return 1;
    }
    if (OPT_ISSET(ops,'u') && !*args) {
	zwarnnam(nam, "what do you want to unload?");
	return 1;
    }
    if (OPT_ISSET(ops,'e') && (OPT_ISSET(ops,'I') || OPT_ISSET(ops,'L') || 
			       (OPT_ISSET(ops,'a') && !OPT_ISSET(ops,'F'))
			       || OPT_ISSET(ops,'d') ||
			       OPT_ISSET(ops,'i') || OPT_ISSET(ops,'u'))) {
	zwarnnam(nam, "-e cannot be combined with other options");
	/* except -F ... */
	return 1;
    }
    for (fp = fonly; *fp; fp++) {
	if (OPT_ISSET(ops,STOUC(*fp)) && !OPT_ISSET(ops,'F')) {
	    zwarnnam(nam, "-%c is only allowed with -F", *fp);
	    return 1;
	}
    }
    queue_signals();
    if (OPT_ISSET(ops, 'F'))
	ret = bin_zmodload_features(nam, args, ops);
    else if (OPT_ISSET(ops,'e'))
	ret = bin_zmodload_exist(nam, args, ops);
    else if (OPT_ISSET(ops,'d'))
	ret = bin_zmodload_dep(nam, args, ops);
    else if ((autoopts = OPT_ISSET(ops, 'b') + OPT_ISSET(ops, 'c') +
	      OPT_ISSET(ops, 'p') + OPT_ISSET(ops, 'f')) ||
	     /* zmodload -a is equivalent to zmodload -ab, annoyingly */
	     OPT_ISSET(ops, 'a')) {
	if (autoopts > 1) {
	    zwarnnam(nam, "use only one of -b, -c, or -p");
	    ret = 1;
	} else
	    ret = bin_zmodload_auto(nam, args, ops);
    } else
	ret = bin_zmodload_load(nam, args, ops);
    unqueue_signals();

    return ret;
}

/* zmodload -A */

/**/
static int
bin_zmodload_alias(char *nam, char **args, Options ops)
{
    /*
     * TODO: while it would be too nasty to have aliases, as opposed
     * to real loadable modules, with dependencies --- just what would
     * we need to load when, exactly? --- there is in principle no objection
     * to making it possible to force an alias onto an existing unloaded
     * module which has dependencies.  This would simply transfer
     * the dependencies down the line to the aliased-to module name.
     * This is actually useful, since then you can alias zsh/zle=mytestzle
     * to load another version of zle.  But then what happens when the
     * alias is removed?  Do you transfer the dependencies back? And
     * suppose other names are aliased to the same file?  It might be
     * kettle of fish best left unwormed.
     */
    Module m;

    if (!*args) {
	if (OPT_ISSET(ops,'R')) {
	    zwarnnam(nam, "no module alias to remove");
	    return 1;
	}
	scanhashtable(modulestab, 1, MOD_ALIAS, 0,
		      modulestab->printnode,
		      OPT_ISSET(ops,'L') ? PRINTMOD_LIST : 0);
	return 0;
    }

    for (; *args; args++) {
	char *eqpos = strchr(*args, '=');
	char *aliasname = eqpos ? eqpos+1 : NULL;
	if (eqpos)
	    *eqpos = '\0';
	if (!modname_ok(*args)) {
	    zwarnnam(nam, "invalid module name `%s'", *args);
	    return 1;
	}
	if (OPT_ISSET(ops,'R')) {
	    if (aliasname) {
		zwarnnam(nam, "bad syntax for removing module alias: %s",
			 *args);
		return 1;
	    }
	    m = find_module(*args, 0, NULL);
	    if (m) {
		if (!(m->node.flags & MOD_ALIAS)) {
		    zwarnnam(nam, "module is not an alias: %s", *args);
		    return 1;
		}
		delete_module(m);
	    } else {
		zwarnnam(nam, "no such module alias: %s", *args);
		return 1;
	    }
	} else {
	    if (aliasname) {
		const char *mname = aliasname;
		if (!modname_ok(aliasname)) {
		    zwarnnam(nam, "invalid module name `%s'", aliasname);
		    return 1;
		}
		do {
		    if (!strcmp(mname, *args)) {
			zwarnnam(nam, "module alias would refer to itself: %s",
				 *args);
			return 1;
		    }
		} while ((m = find_module(mname, 0, NULL))
			 && (m->node.flags & MOD_ALIAS)
			 && (mname = m->u.alias));
		m = find_module(*args, 0, NULL);
		if (m) {
		    if (!(m->node.flags & MOD_ALIAS)) {
			zwarnnam(nam, "module is not an alias: %s", *args);
			return 1;
		    }
		    zsfree(m->u.alias);
		} else {
		    m = (Module) zshcalloc(sizeof(*m));
		    m->node.flags = MOD_ALIAS;
		    modulestab->addnode(modulestab, ztrdup(*args), m);
		}
		m->u.alias = ztrdup(aliasname);
	    } else {
		if ((m = find_module(*args, 0, NULL))) {
		    if (m->node.flags & MOD_ALIAS)
			modulestab->printnode(&m->node,
					      OPT_ISSET(ops,'L') ?
					      PRINTMOD_LIST : 0);
		    else {
			zwarnnam(nam, "module is not an alias: %s", *args);
			return 1;
		    }
		} else {
		    zwarnnam(nam, "no such module alias: %s", *args);
		    return 1;
		}
	    }
	}
    }

    return 0;
}

/* zmodload -e (without -F) */

/**/
static int
bin_zmodload_exist(UNUSED(char *nam), char **args, Options ops)
{
    Module m;

    if (!*args) {
	scanhashtable(modulestab, 1, 0, 0, modulestab->printnode,
		      OPT_ISSET(ops,'A') ? PRINTMOD_EXIST|PRINTMOD_ALIAS :
		      PRINTMOD_EXIST);
	return 0;
    } else {
	int ret = 0;

	for (; !ret && *args; args++) {
	    if (!(m = find_module(*args, FINDMOD_ALIASP, NULL))
		|| !m->u.handle
		|| (m->node.flags & MOD_UNLOAD))
		ret = 1;
	}
	return ret;
    }
}

/* zmodload -d */

/**/
static int
bin_zmodload_dep(UNUSED(char *nam), char **args, Options ops)
{
    Module m;
    if (OPT_ISSET(ops,'u')) {
	/* remove dependencies, which can't pertain to aliases */
	const char *tnam = *args++;
	m = find_module(tnam, FINDMOD_ALIASP, &tnam);
	if (!m)
	    return 0;
	if (*args && m->deps) {
	    do {
		LinkNode dnode;
		for (dnode = firstnode(m->deps); dnode; incnode(dnode))
		    if (!strcmp(*args, getdata(dnode))) {
			zsfree(getdata(dnode));
			remnode(m->deps, dnode);
			break;
		    }
	    } while(*++args);
	    if (empty(m->deps)) {
		freelinklist(m->deps, freestr);
		m->deps = NULL;
	    }
	} else {
	    if (m->deps) {
		freelinklist(m->deps, freestr);
		m->deps = NULL;
	    }
	}
	if (!m->deps && !m->u.handle)
	    delete_module(m);
	return 0;
    } else if (!args[0] || !args[1]) {
	/* list dependencies */
	int depflags = OPT_ISSET(ops,'L') ?
	    PRINTMOD_DEPS|PRINTMOD_LIST : PRINTMOD_DEPS;
	if (args[0]) {
	    if ((m = (Module)modulestab->getnode2(modulestab, args[0])))
		modulestab->printnode(&m->node, depflags);
	} else {
	    scanhashtable(modulestab, 1, 0, 0, modulestab->printnode,
			  depflags);
	}
	return 0;
    } else {
	/* add dependencies */
	int ret = 0;
	char *tnam = *args++;

	for (; *args; args++)
	    add_dep(tnam, *args);
	return ret;
    }
}

/*
 * Function for scanning the parameter table to find and print
 * out autoloadable parameters.
 */

static void
printautoparams(HashNode hn, int lon)
{
    Param pm = (Param) hn;

    if (pm->node.flags & PM_AUTOLOAD) {
	if (lon)
	    printf("zmodload -ap %s %s\n", pm->u.str, pm->node.nam);
	else
	    printf("%s (%s)\n", pm->node.nam, pm->u.str);
    }
}

/* zmodload -a/u [bcpf] */

/**/
static int
bin_zmodload_auto(char *nam, char **args, Options ops)
{
    int fchar, flags;
    char *modnam;

    if (OPT_ISSET(ops,'c')) {
	if (!*args) {
	    /* list autoloaded conditions */
	    Conddef p;

	    for (p = condtab; p; p = p->next) {
		if (p->module) {
		    if (OPT_ISSET(ops,'L')) {
			fputs("zmodload -ac", stdout);
			if (p->flags & CONDF_INFIX)
			    putchar('I');
			printf(" %s %s\n", p->module, p->name);
		    } else {
			if (p->flags & CONDF_INFIX)
			    fputs("infix ", stdout);
			else
			    fputs("post ", stdout);
			printf("%s (%s)\n",p->name, p->module);
		    }
		}
	    }
	    return 0;
	}
	fchar = OPT_ISSET(ops,'I') ? 'C' : 'c';
    } else if (OPT_ISSET(ops,'p')) {
	if (!*args) {
	    /* list autoloaded parameters */
	    scanhashtable(paramtab, 1, 0, 0, printautoparams,
			  OPT_ISSET(ops,'L'));
	    return 0;
	}
	fchar = 'p';
    } else if (OPT_ISSET(ops,'f')) {
	if (!*args) {
	    /* list autoloaded math functions */
	    MathFunc p;

	    for (p = mathfuncs; p; p = p->next) {
		if (!(p->flags & MFF_USERFUNC) && p->module) {
		    if (OPT_ISSET(ops,'L')) {
			fputs("zmodload -af", stdout);
			printf(" %s %s\n", p->module, p->name);
		    } else
			printf("%s (%s)\n",p->name, p->module);
		}
	    }
	    return 0;
	}
	fchar = 'f';
    } else {
	/* builtins are the default; zmodload -ab or just zmodload -a */
	if (!*args) {
	    /* list autoloaded builtins */
	    scanhashtable(builtintab, 1, 0, 0,
			  autoloadscan, OPT_ISSET(ops,'L') ? PRINT_LIST : 0);
	    return 0;
	}
	fchar = 'b';
    }

    flags = FEAT_AUTOALL;
    if (OPT_ISSET(ops,'i'))
	flags |= FEAT_IGNORE;
    if (OPT_ISSET(ops,'u')) {
	/* remove autoload */
	flags |= FEAT_REMOVE;
	modnam = NULL;
    } else {
	/* add autoload */
	modnam = *args;

	if (args[1])
	    args++;
    }
    return autofeatures(nam, modnam, args, fchar, flags);
}

/* Backend handler for zmodload -u */

/**/
int
unload_module(Module m)
{
    int del;

    /*
     * Only unload the real module, so resolve aliases.
     */
    if (m->node.flags & MOD_ALIAS) {
	m = find_module(m->u.alias, FINDMOD_ALIASP, NULL);
	if (!m)
	    return 1;
    }
    /*
     * We may need to clean up the module any time setup_ has been
     * called.  After cleanup_ is successful we are no longer in the
     * booted state (because features etc. are deregistered), so remove
     * MOD_INIT_B, and also MOD_INIT_S since we won't need to cleanup
     * again if this succeeded.
     */
    if ((m->node.flags & MOD_INIT_S) &&
	!(m->node.flags & MOD_UNLOAD) &&
	do_cleanup_module(m))
	return 1;
    m->node.flags &= ~(MOD_INIT_B|MOD_INIT_S);

    del = (m->node.flags & MOD_UNLOAD);

    if (m->wrapper) {
	m->node.flags |= MOD_UNLOAD;
	return 0;
    }
    m->node.flags &= ~MOD_UNLOAD;

    /*
     * We always need to finish the module (and unload it)
     * if it is present.
     */
    if (m->node.flags & MOD_LINKED) {
	if (m->u.linked) {
	    m->u.linked->finish(m);
	    m->u.linked = NULL;
	}
    } else {
	if (m->u.handle) {
	    finish_module(m);
	    m->u.handle = NULL;
	}
    }

    if (del && m->deps) {
	/* The module was unloaded delayed, unload all modules *
	 * on which it depended. */
	LinkNode n;

	for (n = firstnode(m->deps); n; incnode(n)) {
	    Module dm = find_module((char *) getdata(n),
				    FINDMOD_ALIASP, NULL);

	    if (dm &&
		(dm->node.flags & MOD_UNLOAD)) {
		/* See if this is the only module depending on it. */
		Module am;
		int du = 1, i;
		/* Scan hash table the hard way */
		for (i = 0; du && i < modulestab->hsize; i++) {
		    for (am = (Module)modulestab->nodes[i]; du && am;
			 am = (Module)am->node.next) {
			LinkNode sn;
			/*
			 * Don't scan the module we're unloading;
			 * ignore if no dependencies.
			 */
			if (am == m || !am->deps)
			    continue;
			/* Don't scan if not loaded nor linked */
			if ((am->node.flags & MOD_LINKED) ?
			    !am->u.linked : !am->u.handle)
			    continue;
			for (sn = firstnode(am->deps); du && sn;
			     incnode(sn)) {
			    if (!strcmp((char *) getdata(sn),
					dm->node.nam))
				du = 0;
			}
		    }
		}
		if (du)
		    unload_module(dm);
	    }
	}
    }
    if (m->autoloads && firstnode(m->autoloads)) {
	/*
	 * Module has autoloadable features.  Restore them
	 * so that the module will be reloaded when needed.
	 */
	autofeatures("zsh", m->node.nam,
		     hlinklist2array(m->autoloads, 0), 0, FEAT_IGNORE);
    } else if (!m->deps) {
	delete_module(m);
    }
    return 0;
}

/*
 * Unload a module by name (modname); nam is the command name.
 * Optionally don't print some error messages (always print
 * dependency errors).
 */

/**/
int
unload_named_module(char *modname, char *nam, int silent)
{
    const char *mname;
    Module m;
    int ret = 0;

    m = find_module(modname, FINDMOD_ALIASP, &mname);
    if (m) {
	int i, del = 0;
	Module dm;

	for (i = 0; i < modulestab->hsize; i++) {
	    for (dm = (Module)modulestab->nodes[i]; dm;
		 dm = (Module)dm->node.next) {
		LinkNode dn;
		if (!dm->deps || !dm->u.handle)
		    continue;
		for (dn = firstnode(dm->deps); dn; incnode(dn)) {
		    if (!strcmp((char *) getdata(dn), mname)) {
			if (dm->node.flags & MOD_UNLOAD)
			    del = 1;
			else {
			    zwarnnam(nam, "module %s is in use by another module and cannot be unloaded", mname);
			    return 1;
			}
		    }
		}
	    }
	}
	if (del)
	    m->wrapper++;
	if (unload_module(m))
	    ret = 1;
	if (del)
	    m->wrapper--;
    } else if (!silent) {
	zwarnnam(nam, "no such module %s", modname);
	ret = 1;
    }

    return ret;
}

/* zmodload -u without -d */

/**/
static int
bin_zmodload_load(char *nam, char **args, Options ops)
{
    int ret = 0;
    if(OPT_ISSET(ops,'u')) {
	/* unload modules */
	for(; *args; args++) {
	    if (unload_named_module(*args, nam, OPT_ISSET(ops,'i')))
		ret = 1;
	}
	return ret;
    } else if(!*args) {
	/* list modules */
	scanhashtable(modulestab, 1, 0, MOD_UNLOAD|MOD_ALIAS,
		      modulestab->printnode,
		      OPT_ISSET(ops,'L') ? PRINTMOD_LIST : 0);
	return 0;
    } else {
	/* load modules */
	for (; *args; args++) {
	    int tmpret = require_module(*args, NULL, OPT_ISSET(ops,'s'));
	    if (tmpret && ret != 1)
		ret = tmpret;
	}

	return ret;
    }
}

/* zmodload -F */

/**/
static int
bin_zmodload_features(const char *nam, char **args, Options ops)
{
    int iarg;
    char *modname = *args;
    Patprog *patprogs;
    Feature_enables features, fep;

    if (modname)
	args++;
    else if (OPT_ISSET(ops,'L')) {
	int printflags = PRINTMOD_LIST|PRINTMOD_FEATURES;
	if (OPT_ISSET(ops,'P')) {
	    zwarnnam(nam, "-P is only allowed with a module name");
	    return 1;
	}
	if (OPT_ISSET(ops,'l'))
	    printflags |= PRINTMOD_LISTALL;
	if (OPT_ISSET(ops,'a'))
	    printflags |= PRINTMOD_AUTO;
	scanhashtable(modulestab, 1, 0, MOD_ALIAS,
		      modulestab->printnode, printflags);
	return 0;
    }

    if (!modname) {
	zwarnnam(nam, "-F requires a module name");
	return 1;
    }

    if (OPT_ISSET(ops,'m')) {
	char **argp;
	Patprog *patprogp;

	/* not NULL terminated */
	patprogp = patprogs =
	    (Patprog *)zhalloc(arrlen(args)*sizeof(Patprog));
	for (argp = args; *argp; argp++, patprogp++) {
	    char *arg = *argp;
	    if (*arg == '+' || *arg == '-')
		arg++;
	    tokenize(arg);
	    *patprogp = patcompile(arg, 0, 0);
	}
    } else
	patprogs = NULL;

    if (OPT_ISSET(ops,'l') || OPT_ISSET(ops,'L') || OPT_ISSET(ops,'e')) {
	/*
	 * With option 'l', list all features one per line with + or -.
	 * With option 'L', list as zmodload statement showing
	 * only options turned on.
	 * With both options, list as zmodload showing options
	 * to be turned both on and off.
	 */
	Module m;
	char **features, **fp, **arrset = NULL, **arrp = NULL;
	int *enables = NULL, *ep;
	char *param = OPT_ARG_SAFE(ops,'P');

	m = find_module(modname, FINDMOD_ALIASP, NULL);
	if (OPT_ISSET(ops,'a')) {
	    LinkNode ln;
	    /*
	     * If there are no autoloads defined, return status 1.
	     */
	    if (!m || !m->autoloads)
		return 1;
	    if (OPT_ISSET(ops,'e')) {
		for (fp = args; *fp; fp++) {
		    char *fstr = *fp;
		    int sense = 1;
		    if (*fstr == '+')
			fstr++;
		    else if (*fstr == '-') {
			fstr++;
			sense = 0;
		    }
		    if ((linknodebystring(m->autoloads, fstr) != NULL) !=
			sense)
			return 1;
		}
		return 0;
	    }
	    if (param) {
		arrp = arrset = (char **)zalloc(sizeof(char*) *
				 (countlinknodes(m->autoloads)+1));
	    } else if (OPT_ISSET(ops,'L')) {
		printf("zmodload -aF %s%c", m->node.nam,
		       m->autoloads && firstnode(m->autoloads) ? ' ' : '\n');
		arrp = NULL;
	    }
	    for (ln = firstnode(m->autoloads); ln; incnode(ln)) {
		char *al = (char *)getdata(ln);
		if (param)
		    *arrp++ = ztrdup(al);
		else
		    printf("%s%c", al,
			   OPT_ISSET(ops,'L') && nextnode(ln) ? ' ' : '\n');
	    }
	    if (param) {
		*arrp = NULL;
		if (!setaparam(param, arrset))
		    return 1;
	    }
	    return 0;
	}
	if (!m || !m->u.handle || (m->node.flags & MOD_UNLOAD)) {
	    if (!OPT_ISSET(ops,'e'))
		zwarnnam(nam, "module `%s' is not yet loaded", modname);
	    return 1;
	}
	if (features_module(m, &features)) {
	    if (!OPT_ISSET(ops,'e'))
		zwarnnam(nam, "module `%s' does not support features",
			 m->node.nam);
	    return 1;
	}
	if (enables_module(m, &enables)) {
	    /* this shouldn't ever happen, so don't silence this error */
	    zwarnnam(nam, "error getting enabled features for module `%s'",
		     m->node.nam);
	    return 1;
	}
	for (arrp = args, iarg = 0; *arrp; arrp++, iarg++) {
	    char *arg = *arrp;
	    int on, found = 0;
	    if (*arg == '-') {
		on = 0;
		arg++;
	    } else if (*arg == '+') {
		on = 1;
		arg++;
	    } else
		on = -1;
	    for (fp = features, ep = enables; *fp; fp++, ep++) {
		if (patprogs ? pattry(patprogs[iarg], *fp) :
		    !strcmp(arg, *fp)) {
		    /* for -e, check given state, if any */
		    if (OPT_ISSET(ops,'e') && on != -1 &&
			on != (*ep & 1))
			return 1;
		    found++;
		    if (!patprogs)
			break;
		}
	    }
	    if (!found) {
		if (!OPT_ISSET(ops,'e'))
		    zwarnnam(nam, patprogs ?
			     "module `%s' has no feature matching: `%s'" :
			     "module `%s' has no such feature: `%s'",
			     modname, *arrp);
		return 1;
	    }
	}
	if (OPT_ISSET(ops,'e'))		/* yep, everything we want exists */
	    return 0;
	if (param) {
	    int arrlen = 0;
	    for (fp = features, ep = enables; *fp; fp++, ep++) {
		if (OPT_ISSET(ops, 'L') && !OPT_ISSET(ops, 'l') &&
		    !*ep)
		    continue;
		if (*args) {
		    char **argp;
		    for (argp = args, iarg = 0; *argp; argp++, iarg++) {
			char *arg = *argp;
			/* ignore +/- for consistency */
			if (*arg == '+' || *arg == '-')
			    arg++;
			if (patprogs ? pattry(patprogs[iarg], *fp) :
			    !strcmp(*fp, arg))
			    break;
		    }
		    if (!*argp)
			continue;
		}
		arrlen++;
	    }
	    arrp = arrset = zalloc(sizeof(char *) * (arrlen+1));
	} else if (OPT_ISSET(ops, 'L'))
	    printf("zmodload -F %s ", m->node.nam);
	for (fp = features, ep = enables; *fp; fp++, ep++) {
	    char *onoff;
	    int term;
	    if (*args) {
		char **argp;
		for (argp = args, iarg = 0; *argp; argp++, iarg++) {
		    char *arg = *argp;
		    if (*arg == '+' || *arg == '-')
			arg++;
		    if (patprogs ? pattry(patprogs[iarg], *fp) :
			!strcmp(*fp, *argp))
			break;
		}
		if (!*argp)
		    continue;
	    }
	    if (OPT_ISSET(ops, 'L') && !OPT_ISSET(ops, 'l')) {
		if (!*ep)
		    continue;
		onoff = "";
	    } else if (*ep) {
		onoff = "+";
	    } else {
		onoff = "-";
	    }
	    if (param) {
		*arrp++ = bicat(onoff, *fp);
	    } else {
		if (OPT_ISSET(ops, 'L') && fp[1]) {
		    term = ' ';
		} else {
		    term = '\n';
		}
		printf("%s%s%c", onoff, *fp, term);
	    }
	}
	if (param) {
	    *arrp = NULL;
	    if (!setaparam(param, arrset))
		return 1;
	}
	return 0;
    } else if (OPT_ISSET(ops,'P')) {
	zwarnnam(nam, "-P can only be used with -l or -L");
	return 1;
    } else if (OPT_ISSET(ops,'a')) {
	if (OPT_ISSET(ops,'m')) {
	    zwarnnam(nam, "-m cannot be used with -a");
	    return 1;
	}
	/*
	 * With zmodload -aF, we always use the effect of -i.
	 * The thinking is that marking a feature for
	 * autoload is separate from enabling or disabling it.
	 * Arguably we could do this with the zmodload -ab method
	 * but I've kept it there for old time's sake.
	 * The decoupling has meant FEAT_IGNORE/-i also
	 * suppresses an error for attempting to remove an
	 * autoload when the feature is enabled, which used
	 * to be a hard error before.
	 */
	return autofeatures(nam, modname, args, 0, FEAT_IGNORE);
    }

    fep = features =
	(Feature_enables)zhalloc((arrlen(args)+1)*sizeof(*fep));

    while (*args) {
	fep->str = *args++;
	fep->pat = patprogs ? *patprogs++ : NULL;
	fep++;
    }
    fep->str = NULL;
    fep->pat = NULL;

    return require_module(modname, features, OPT_ISSET(ops,'s'));
}


/************************************************************************
 * Generic feature support.
 * These functions are designed to be called by modules.
 ************************************************************************/

/*
 * Construct a features array out of the list of concrete
 * features given, leaving space for any abstract features
 * to be added by the module itself.
 *
 * Note the memory is from the heap.
 */

/**/
mod_export char **
featuresarray(UNUSED(Module m), Features f)
{
    int bn_size = f->bn_size, cd_size = f->cd_size;
    int mf_size = f->mf_size, pd_size = f->pd_size;
    int features_size = bn_size + cd_size + pd_size + mf_size + f->n_abstract;
    Builtin bnp = f->bn_list;
    Conddef cdp = f->cd_list;
    MathFunc mfp = f->mf_list;
    Paramdef pdp = f->pd_list;
    char **features = (char **)zhalloc((features_size + 1) * sizeof(char *));
    char **featurep = features;

    while (bn_size--)
	*featurep++ = dyncat("b:", (bnp++)->node.nam);
    while (cd_size--) {
	*featurep++ = dyncat((cdp->flags & CONDF_INFIX) ? "C:" : "c:",
			     cdp->name);
	cdp++;
    }
    while (mf_size--)
	*featurep++ = dyncat("f:", (mfp++)->name);
    while (pd_size--)
	*featurep++ = dyncat("p:", (pdp++)->name);

    features[features_size] = NULL;
    return features;
}

/*
 * Return the current set of enables for the features in a
 * module using heap memory.  Leave space for abstract
 * features.  The array is not zero terminated.
 */
/**/
mod_export int *
getfeatureenables(UNUSED(Module m), Features f)
{
    int bn_size = f->bn_size, cd_size = f->cd_size;
    int mf_size = f->mf_size, pd_size = f->pd_size;
    int features_size = bn_size + cd_size + mf_size + pd_size + f->n_abstract;
    Builtin bnp = f->bn_list;
    Conddef cdp = f->cd_list;
    MathFunc mfp = f->mf_list;
    Paramdef pdp = f->pd_list;
    int *enables = zhalloc(sizeof(int) * features_size);
    int *enablep = enables;

    while (bn_size--)
	*enablep++ = ((bnp++)->node.flags & BINF_ADDED) ? 1 : 0;
    while (cd_size--)
	*enablep++ = ((cdp++)->flags & CONDF_ADDED) ? 1 : 0;
    while (mf_size--)
	*enablep++ = ((mfp++)->flags & MFF_ADDED) ? 1 : 0;
    while (pd_size--)
	*enablep++ = (pdp++)->pm ? 1 : 0;

    return enables;
}

/*
 * Add or remove the concrete features passed in arguments,
 * depending on the corresponding element of the array e.
 * If e is NULL, disable everything.
 * Return 0 for success, 1 for failure; does not attempt
 * to imitate the return values of addbuiltins() etc.
 * Any failure in adding a requested feature is an
 * error.
 */

/**/
mod_export int
setfeatureenables(Module m, Features f, int *e)
{
    int ret = 0;

    if (f->bn_size) {
	if (setbuiltins(m->node.nam, f->bn_list, f->bn_size, e))
	    ret = 1;
	if (e)
	    e += f->bn_size;
    }
    if (f->cd_size) {
	if (setconddefs(m->node.nam, f->cd_list, f->cd_size, e))
	    ret = 1;
	if (e)
	    e += f->cd_size;
    }
    if (f->mf_size) {
	if (setmathfuncs(m->node.nam, f->mf_list, f->mf_size, e))
	    ret = 1;
	if (e)
	    e += f->mf_size;
    }
    if (f->pd_size) {
	if (setparamdefs(m->node.nam, f->pd_list, f->pd_size, e))
	    ret = 1;
	if (e)
	    e += f->pd_size;
    }
    return ret;
}

/*
 * Convenient front-end to get or set features which
 * can be used in a module enables_() function.
 */

/**/
mod_export int
handlefeatures(Module m, Features f, int **enables)
{
    if (!enables || *enables)
	return setfeatureenables(m, f, enables ? *enables : NULL);
    *enables = getfeatureenables(m, f);
    return 0;
}

/*
 * Ensure module "modname" is providing feature with "prefix"
 * and "feature" (e.g. "b:", "limit").  If feature is NULL,
 * ensure all features are loaded (used for compatibility
 * with the pre-feature autoloading behaviour).
 *
 * This will usually be called from the main shell to handle
 * loading of an autoloadable feature.
 *
 * Returns 0 on success, 1 for error in module, 2 for error
 * setting the feature.  However, this isn't actually all
 * that useful for testing immediately on an autoload since
 * it could be a failure to autoload a different feature
 * from the one we want.  We could fix this but it's
 * possible to test other ways.
 */

/**/
mod_export int
ensurefeature(const char *modname, const char *prefix, const char *feature)
{
    char *f;
    struct feature_enables features[2];

    if (!feature)
	return require_module(modname, NULL, 0);
    f = dyncat(prefix, feature);

    features[0].str = f;
    features[0].pat = NULL;
    features[1].str = NULL;
    features[1].pat = NULL;
    return require_module(modname, features, 0);
}

/*
 * Add autoloadable features for a given module.
 */

/**/
int
autofeatures(const char *cmdnam, const char *module, char **features,
	     int prefchar, int defflags)
{
    int ret = 0, subret;
    Module defm, m;
    char **modfeatures = NULL;
    int *modenables = NULL;
    if (module) {
	defm = (Module)find_module(module,
				   FINDMOD_ALIASP|FINDMOD_CREATE, NULL);
	if ((defm->node.flags & MOD_LINKED) ? defm->u.linked :
	    defm->u.handle) {
	    (void)features_module(defm, &modfeatures);
	    (void)enables_module(defm, &modenables);
	}
    } else
	defm = NULL;

    for (; *features; features++) {
	char *fnam, *typnam, *feature;
	int add, fchar, flags = defflags;
	autofeaturefn_t fn;

	if (prefchar) {
	    /*
	     * "features" is list of bare features with no
	     * type prefix; prefchar gives type character.
	     */
	    add = 1; 		/* unless overridden by flag */
	    fchar = prefchar;
	    fnam = *features;
	    feature = zhalloc(strlen(fnam) + 3);
	    sprintf(feature, "%c:%s", fchar, fnam);
	} else {
	    feature = *features;
	    if (*feature == '-') {
		add = 0;
		feature++;
	    } else {
		add = 1;
		if (*feature == '+')
		    feature++;
	    }

	    if (!*feature || feature[1] != ':') {
		zwarnnam(cmdnam, "bad format for autoloadable feature: `%s'",
			 feature);
		ret = 1;
		continue;
	    }
	    fnam = feature + 2;
	    fchar = feature[0];
	}
	if (flags & FEAT_REMOVE)
	    add = 0;

	switch (fchar) {
	case 'b':
	    fn = add ? add_autobin : del_autobin;
	    typnam = "builtin";
	    break;

	case 'C':
	    flags |= FEAT_INFIX;
	    /* FALLTHROUGH */
	case 'c':
	    fn = add ? add_autocond : del_autocond;
	    typnam = "condition";
	    break;

	case 'f':
	    fn = add ? add_automathfunc : del_automathfunc;
	    typnam = "math function";
	    break;

	case 'p':
	    fn = add ? add_autoparam : del_autoparam;
	    typnam = "parameter";
	    break;

	default:
	    zwarnnam(cmdnam, "bad autoloadable feature type: `%c'",
		     fchar);
	    ret = 1;
	    continue;
	}

	if (strchr(fnam, '/')) {
	    zwarnnam(cmdnam, "%s: `/' is illegal in a %s", fnam, typnam);
	    ret = 1;
	    continue;
	}

	if (!module) {
	    /*
	     * Traditional un-autoload syntax doesn't tell us
	     * which module this came from.
	     */
	    int i;
	    for (i = 0, m = NULL; !m && i < modulestab->hsize; i++) {
		for (m = (Module)modulestab->nodes[i]; m;
		     m = (Module)m->node.next) {
		    if (m->autoloads &&
			linknodebystring(m->autoloads, feature))
			break;
		}
	    }
	    if (!m) {
		if (!(flags & FEAT_IGNORE)) {
		    ret = 1;
		    zwarnnam(cmdnam, "%s: no such %s", fnam, typnam);
		}
		continue;
	    }
	} else
	    m = defm;

	subret = 0;
	if (add) {
	    char **ptr;
	    if (modfeatures) {
		/*
		 * If the module is already available, check that
		 * it does in fact provide the necessary feature.
		 */
		for (ptr = modfeatures; *ptr; ptr++)
		    if (!strcmp(*ptr, feature))
			break;
		if (!*ptr) {
		    zwarnnam(cmdnam, "module `%s' has no such feature: `%s'",
			     m->node.nam, feature);
		    ret = 1;
		    continue;
		}
		/*
		 * If the feature is already provided by the module, there's
		 * nothing more to do.
		 */
		if (modenables[ptr-modfeatures])
		    continue;
		/*
		 * Otherwise, marking it for autoload will do the
		 * right thing when the feature is eventually used.
		 */
	    }
	    if (!m->autoloads) {
		m->autoloads = znewlinklist();
		zaddlinknode(m->autoloads, ztrdup(feature));
	    } else {
		/* Insert in lexical order */
		LinkNode ln, prev = (LinkNode)m->autoloads;
		while ((ln = nextnode(prev))) {
		    int cmp = strcmp(feature, (char *)getdata(ln));
		    if (cmp == 0) {
			/* Already there.  Never an error. */
			break;
		    }
		    if (cmp < 0) {
			zinsertlinknode(m->autoloads, prev,
					ztrdup(feature));
			break;
		    }
		    prev = ln;
		}
		if (!ln)
		    zaddlinknode(m->autoloads, ztrdup(feature));
	    }
	} else if (m->autoloads) {
	    LinkNode ln;
	    if ((ln = linknodebystring(m->autoloads, feature)))
		zsfree((char *)remnode(m->autoloads, ln));
	    else {
		/*
		 * With -i (or zmodload -Fa), removing an autoload
		 * that's not there is not an error.
		 */
		subret = (flags & FEAT_IGNORE) ? -2 : 2;
	    }
	}

	if (subret == 0)
	    subret = fn(module, fnam, flags);

	if (subret != 0) {
	    /* -2 indicates not an error, just skip running fn() */
	    if (subret != -2)
		ret = 1;
	    switch (subret) {
	    case 1:
		zwarnnam(cmdnam, "failed to add %s `%s'", typnam, fnam);
		break;

	    case 2:
		zwarnnam(cmdnam, "%s: no such %s", fnam, typnam);
		break;

	    case 3:
		zwarnnam(cmdnam, "%s: %s is already defined", fnam, typnam);
		break;

	    default:
		/* no (further) message needed */
		break;
	    }
	}
    }

    return ret;
}

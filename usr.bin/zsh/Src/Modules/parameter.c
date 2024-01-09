/*
 * parameter.c - parameter interface to zsh internals
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1999 Sven Wischnowsky
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Sven Wischnowsky or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Sven Wischnowsky and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Sven Wischnowsky and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Sven Wischnowsky and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "parameter.mdh"
#include "parameter.pro"

/* This says if we are cleaning up when the module is unloaded. */

static int incleanup;

/* Functions for the parameters special parameter. */

/* Return a string describing the type of a parameter. */

/**/
static char *
paramtypestr(Param pm)
{
    char *val = NULL;
    int f = pm->node.flags;

    if (!(f & PM_UNSET)) {
	if (pm->node.flags & PM_AUTOLOAD)
	    return dupstring("undefined");

	switch (PM_TYPE(f)) {
	case PM_SCALAR:  val = "scalar"; break;
	case PM_ARRAY:   val = "array"; break;
	case PM_INTEGER: val = "integer"; break;
	case PM_EFLOAT:
	case PM_FFLOAT:  val = "float"; break;
	case PM_HASHED:  val = "association"; break;
	}
	DPUTS(!val, "BUG: type not handled in parameter");
	val = dupstring(val);
	if (pm->level)
	    val = dyncat(val, "-local");
	if (f & PM_LEFT)
	    val = dyncat(val, "-left");
	if (f & PM_RIGHT_B)
	    val = dyncat(val, "-right_blanks");
	if (f & PM_RIGHT_Z)
	    val = dyncat(val, "-right_zeros");
	if (f & PM_LOWER)
	    val = dyncat(val, "-lower");
	if (f & PM_UPPER)
	    val = dyncat(val, "-upper");
	if (f & PM_READONLY)
	    val = dyncat(val, "-readonly");
	if (f & PM_TAGGED)
	    val = dyncat(val, "-tag");
	if (f & PM_TIED)
	    val = dyncat(val, "-tied");
	if (f & PM_EXPORTED)
	    val = dyncat(val, "-export");
	if (f & PM_UNIQUE)
	    val = dyncat(val, "-unique");
	if (f & PM_HIDE)
	    val = dyncat(val, "-hide");
	if (f & PM_HIDEVAL)
	    val = dyncat(val, "-hideval");
	if (f & PM_SPECIAL)
	    val = dyncat(val, "-special");
    } else
	val = dupstring("");

    return val;
}

/**/
static HashNode
getpmparameter(UNUSED(HashTable ht), const char *name)
{
    Param rpm, pm = NULL;

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR | PM_READONLY;
    pm->gsu.s = &nullsetscalar_gsu;
    if ((rpm = (Param) realparamtab->getnode(realparamtab, name)) &&
	!(rpm->node.flags & PM_UNSET))
	pm->u.str = paramtypestr(rpm);
    else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static void
scanpmparameters(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    struct param pm;
    int i;
    HashNode hn;

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR | PM_READONLY;
    pm.gsu.s = &nullsetscalar_gsu;

    for (i = 0; i < realparamtab->hsize; i++)
	for (hn = realparamtab->nodes[i]; hn; hn = hn->next) {
	    if (((Param)hn)->node.flags & PM_UNSET)
		continue;
	    pm.node.nam = hn->nam;
	    if (func != scancountparams &&
		((flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
		 !(flags & SCANPM_WANTKEYS)))
		pm.u.str = paramtypestr((Param) hn);
	    func(&pm.node, flags);
	}
}

/* Functions for the commands special parameter. */

/**/
static void
setpmcommand(Param pm, char *value)
{
    if (isset(RESTRICTED)) {
	zwarn("restricted: %s", value);
	zsfree(value);
    } else {
	Cmdnam cn = zshcalloc(sizeof(*cn));

	cn->node.flags = HASHED;
	cn->u.cmd = value;

	cmdnamtab->addnode(cmdnamtab, ztrdup(pm->node.nam), &cn->node);
    }
}

/**/
static void
unsetpmcommand(Param pm, UNUSED(int exp))
{
    HashNode hn = cmdnamtab->removenode(cmdnamtab, pm->node.nam);

    if (hn)
	cmdnamtab->freenode(hn);
}

/**/
static void
setpmcommands(Param pm, HashTable ht)
{
    int i;
    HashNode hn;

    if (!ht)
	return;

    for (i = 0; i < ht->hsize; i++)
	for (hn = ht->nodes[i]; hn; hn = hn->next) {
	    Cmdnam cn = zshcalloc(sizeof(*cn));
	    struct value v;

	    v.isarr = v.flags = v.start = 0;
	    v.end = -1;
	    v.arr = NULL;
	    v.pm = (Param) hn;

	    cn->node.flags = HASHED;
	    cn->u.cmd = ztrdup(getstrvalue(&v));

	    cmdnamtab->addnode(cmdnamtab, ztrdup(hn->nam), &cn->node);
	}
    /*
     * On full-array assignment ht is a temporary hash with the default
     * get/set functions, whereas pm->u.hash has the special $commands
     * get/set functions.  Do not assign ht to pm, just delete it.
     *
     * On append, ht and pm->u.hash are the same table, don't delete.
     */
    if (ht != pm->u.hash)
	deleteparamtable(ht);
}

static const struct gsu_scalar pmcommand_gsu =
{ strgetfn, setpmcommand, unsetpmcommand };


/**/
static HashNode
getpmcommand(UNUSED(HashTable ht), const char *name)
{
    Cmdnam cmd;
    Param pm = NULL;

    if (!(cmd = (Cmdnam) cmdnamtab->getnode(cmdnamtab, name)) &&
	isset(HASHLISTALL)) {
	cmdnamtab->filltable(cmdnamtab);
	cmd = (Cmdnam) cmdnamtab->getnode(cmdnamtab, name);
    }
    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR;
    pm->gsu.s = &pmcommand_gsu;
    if (cmd) {
	if (cmd->node.flags & HASHED)
	    pm->u.str = cmd->u.cmd;
	else {
	    pm->u.str = zhalloc(strlen(*(cmd->u.name)) + strlen(name) + 2);
	    strcpy(pm->u.str, *(cmd->u.name));
	    strcat(pm->u.str, "/");
	    strcat(pm->u.str, name);
	}
    } else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static void
scanpmcommands(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    struct param pm;
    int i;
    HashNode hn;
    Cmdnam cmd;

    if (isset(HASHLISTALL))
	cmdnamtab->filltable(cmdnamtab);

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR;
    pm.gsu.s = &pmcommand_gsu;

    for (i = 0; i < cmdnamtab->hsize; i++)
	for (hn = cmdnamtab->nodes[i]; hn; hn = hn->next) {
	    pm.node.nam = hn->nam;
	    cmd = (Cmdnam) hn;
	    if (func != scancountparams &&
		((flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
		 !(flags & SCANPM_WANTKEYS))) {
		if (cmd->node.flags & HASHED)
		    pm.u.str = cmd->u.cmd;
		else {
		    pm.u.str = zhalloc(strlen(*(cmd->u.name)) +
				       strlen(cmd->node.nam) + 2);
		    strcpy(pm.u.str, *(cmd->u.name));
		    strcat(pm.u.str, "/");
		    strcat(pm.u.str, cmd->node.nam);
		}
	    }
	    func(&pm.node, flags);
	}
}

/* Functions for the functions special parameter. */

/**/
static void
setfunction(char *name, char *val, int dis)
{
    char *value = dupstring(val);
    Shfunc shf;
    Eprog prog;
    int sn;

    val = metafy(val, strlen(val), META_REALLOC);

    prog = parse_string(val, 1);

    if (!prog || prog == &dummy_eprog) {
	zwarn("invalid function definition", value);
	zsfree(val);
	return;
    }
    shf = (Shfunc) zshcalloc(sizeof(*shf));
    shf->funcdef = dupeprog(prog, 0);
    shf->node.flags = dis;
    shfunc_set_sticky(shf);

    if (!strncmp(name, "TRAP", 4) &&
	(sn = getsignum(name + 4)) != -1) {
	if (settrap(sn, NULL, ZSIG_FUNC)) {
	    freeeprog(shf->funcdef);
	    zfree(shf, sizeof(*shf));
	    zsfree(val);
	    return;
	}
    }
    shfunctab->addnode(shfunctab, ztrdup(name), shf);
    zsfree(val);
}

/**/
static void
setpmfunction(Param pm, char *value)
{
    setfunction(pm->node.nam, value, 0);
}

/**/
static void
setpmdisfunction(Param pm, char *value)
{
    setfunction(pm->node.nam, value, DISABLED);
}

/**/
static void
unsetpmfunction(Param pm, UNUSED(int exp))
{
    HashNode hn = shfunctab->removenode(shfunctab, pm->node.nam);

    if (hn)
	shfunctab->freenode(hn);
}

/**/
static void
setfunctions(Param pm, HashTable ht, int dis)
{
    int i;
    HashNode hn;

    if (!ht)
	return;

    for (i = 0; i < ht->hsize; i++)
	for (hn = ht->nodes[i]; hn; hn = hn->next) {
	    struct value v;

	    v.isarr = v.flags = v.start = 0;
	    v.end = -1;
	    v.arr = NULL;
	    v.pm = (Param) hn;

	    setfunction(hn->nam, ztrdup(getstrvalue(&v)), dis);
	}
    /* See setpmcommands() above */
    if (ht != pm->u.hash)
	deleteparamtable(ht);
}

/**/
static void
setpmfunctions(Param pm, HashTable ht)
{
    setfunctions(pm, ht, 0);
}

/**/
static void
setpmdisfunctions(Param pm, HashTable ht)
{
    setfunctions(pm, ht, DISABLED);
}

static const struct gsu_scalar pmfunction_gsu =
{ strgetfn, setpmfunction, unsetpmfunction };
static const struct gsu_scalar pmdisfunction_gsu =
{ strgetfn, setpmdisfunction, unsetpmfunction };

/**/
static HashNode
getfunction(UNUSED(HashTable ht), const char *name, int dis)
{
    Shfunc shf;
    Param pm = NULL;

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR;
    pm->gsu.s = dis ? &pmdisfunction_gsu :  &pmfunction_gsu;

    if ((shf = (Shfunc) shfunctab->getnode2(shfunctab, name)) &&
	(dis ? (shf->node.flags & DISABLED) : !(shf->node.flags & DISABLED))) {
	if (shf->node.flags & PM_UNDEFINED) {
	    pm->u.str = dyncat("builtin autoload -X",
			       ((shf->node.flags & PM_UNALIASED) ?
				((shf->node.flags & PM_TAGGED) ? "Ut" : "U") :
				((shf->node.flags & PM_TAGGED) ? "t" : "")));
	} else {
	    char *t = getpermtext(shf->funcdef, NULL, 1), *n, *h;
	    char *start;

	    if (shf->redir)
		start = "{\n\t";
	    else
		start = "\t";

	    if (shf->funcdef->flags & EF_RUN) {
		n = nicedupstring(name);
		h = (char *) zhalloc(strlen(start) + strlen(t) + strlen(n) + 8);
		strcpy(h, start);
		strcat(h, t);
		strcat(h, "\n\t");
		strcat(h, n);
		strcat(h, " \"$@\"");
	    } else
		h = dyncat(start, t);
	    zsfree(t);

	    if (shf->redir) {
		t = getpermtext(shf->redir, NULL, 1);
		h = zhtricat(h, "\n}", t);
		zsfree(t);
	    }

	    pm->u.str = h;
	}
    } else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static HashNode
getpmfunction(HashTable ht, const char *name)
{
    return getfunction(ht, name, 0);
}

/**/
static HashNode
getpmdisfunction(HashTable ht, const char *name)
{
    return getfunction(ht, name, DISABLED);
}

/**/
static void
scanfunctions(UNUSED(HashTable ht), ScanFunc func, int flags, int dis)
{
    struct param pm;
    int i;
    HashNode hn;

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR;
    pm.gsu.s = dis ? &pmdisfunction_gsu : &pmfunction_gsu;

    for (i = 0; i < shfunctab->hsize; i++)
	for (hn = shfunctab->nodes[i]; hn; hn = hn->next) {
	    if (dis ? (hn->flags & DISABLED) : !(hn->flags & DISABLED)) {
		pm.node.nam = hn->nam;
		if (func != scancountparams &&
		    ((flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
		     !(flags & SCANPM_WANTKEYS))) {
		    if (((Shfunc) hn)->node.flags & PM_UNDEFINED) {
			Shfunc shf = (Shfunc) hn;
			pm.u.str =
			    dyncat("builtin autoload -X",
				   ((shf->node.flags & PM_UNALIASED) ?
				    ((shf->node.flags & PM_TAGGED) ? "Ut" : "U") :
				    ((shf->node.flags & PM_TAGGED) ? "t" : "")));
		    } else {
			Shfunc shf = (Shfunc)hn;
			char *t = getpermtext(shf->funcdef, NULL, 1);
			char *n, *start;

			if (shf->redir)
			    start = "{\n\t";
			else
			    start = "\t";

			if (shf->funcdef->flags & EF_RUN) {
			    n = nicedupstring(hn->nam);
			    pm.u.str = (char *) zhalloc(
				strlen(start) + strlen(t) + strlen(n) + 8);
			    strcpy(pm.u.str, start);
			    strcat(pm.u.str, t);
			    strcat(pm.u.str, "\n\t");
			    strcat(pm.u.str, n);
			    strcat(pm.u.str, " \"$@\"");
			} else
			    pm.u.str = dyncat(start, t);
			zsfree(t);

			if (shf->redir) {
			    t = getpermtext(shf->redir, NULL, 1);
			    pm.u.str = zhtricat(pm.u.str, "\n}", t);
			    zsfree(t);
			}
		    }
		}
		func(&pm.node, flags);
	    }
	}
}

/**/
static void
scanpmfunctions(HashTable ht, ScanFunc func, int flags)
{
    scanfunctions(ht, func, flags, 0);
}

/**/
static void
scanpmdisfunctions(HashTable ht, ScanFunc func, int flags)
{
    scanfunctions(ht, func, flags, DISABLED);
}

/* Functions for the functions_source special parameter. */

/* Retrieve the source file for a function by explicit name */

/**/
static HashNode
getfunction_source(UNUSED(HashTable ht), const char *name, int dis)
{
    Shfunc shf;
    Param pm = NULL;

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR|PM_READONLY;
    pm->gsu.s = dis ? &pmdisfunction_gsu :  &pmfunction_gsu;

    if ((shf = (Shfunc) shfunctab->getnode2(shfunctab, name)) &&
	(dis ? (shf->node.flags & DISABLED) : !(shf->node.flags & DISABLED))) {
	pm->u.str = getshfuncfile(shf);
	if (!pm->u.str)
	    pm->u.str = dupstring("");
    }
    return &pm->node;
}

/* Retrieve the source file for functions by scanning the table */

/**/
static void
scanfunctions_source(UNUSED(HashTable ht), ScanFunc func, int flags, int dis)
{
    struct param pm;
    int i;
    HashNode hn;

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR|PM_READONLY;
    pm.gsu.s = dis ? &pmdisfunction_gsu : &pmfunction_gsu;

    for (i = 0; i < shfunctab->hsize; i++) {
	for (hn = shfunctab->nodes[i]; hn; hn = hn->next) {
	    if (dis ? (hn->flags & DISABLED) : !(hn->flags & DISABLED)) {
		pm.node.nam = hn->nam;
		if (func != scancountparams &&
		    ((flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
		     !(flags & SCANPM_WANTKEYS))) {
		    pm.u.str = getshfuncfile((Shfunc)hn);
		    if (!pm.u.str)
			pm.u.str = dupstring("");
		}
		func(&pm.node, flags);
	    }
	}
    }
}

/* Param table entry for retrieving functions_source element */

/**/
static HashNode
getpmfunction_source(HashTable ht, const char *name)
{
    return getfunction_source(ht, name, 0);
}

/* Param table entry for retrieving dis_functions_source element */

/**/
static HashNode
getpmdisfunction_source(HashTable ht, const char *name)
{
    return getfunction_source(ht, name, 1);
}

/* Param table entry for scanning functions_source table */

/**/
static void
scanpmfunction_source(HashTable ht, ScanFunc func, int flags)
{
    scanfunctions_source(ht, func, flags, 0);
}

/* Param table entry for scanning dis_functions_source table */

/**/
static void
scanpmdisfunction_source(HashTable ht, ScanFunc func, int flags)
{
    scanfunctions_source(ht, func, flags, 1);
}

/* Functions for the funcstack special parameter. */

/**/
static char **
funcstackgetfn(UNUSED(Param pm))
{
    Funcstack f;
    int num;
    char **ret, **p;

    for (f = funcstack, num = 0; f; f = f->prev, num++);

    ret = (char **) zhalloc((num + 1) * sizeof(char *));

    for (f = funcstack, p = ret; f; f = f->prev, p++)
	*p = f->name;
    *p = NULL;

    return ret;
}

/* Functions for the functrace special parameter. */

/**/
static char **
functracegetfn(UNUSED(Param pm))
{
    Funcstack f;
    int num;
    char **ret, **p;

    for (f = funcstack, num = 0; f; f = f->prev, num++);

    ret = (char **) zhalloc((num + 1) * sizeof(char *));

    for (f = funcstack, p = ret; f; f = f->prev, p++) {
	char *colonpair;

	colonpair = zhalloc(strlen(f->caller) + (f->lineno > 9999 ? 24 : 6));
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
	sprintf(colonpair, "%s:%lld", f->caller, f->lineno);
#else
	sprintf(colonpair, "%s:%ld", f->caller, (long)f->lineno);
#endif

	*p = colonpair;
    }
    *p = NULL;

    return ret;
}

/* Functions for the funcsourcetrace special parameter. */

/**/
static char **
funcsourcetracegetfn(UNUSED(Param pm))
{
    Funcstack f;
    int num;
    char **ret, **p;

    for (f = funcstack, num = 0; f; f = f->prev, num++);

    ret = (char **) zhalloc((num + 1) * sizeof(char *));

    for (f = funcstack, p = ret; f; f = f->prev, p++) {
	char *colonpair;
	char *fname = f->filename ? f->filename : "";

	colonpair = zhalloc(strlen(fname) + (f->flineno > 9999 ? 24 : 6));
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
	sprintf(colonpair, "%s:%lld", fname, f->flineno);
#else
	sprintf(colonpair, "%s:%ld", fname, (long)f->flineno);
#endif

	*p = colonpair;
    }
    *p = NULL;

    return ret;
}

/* Functions for the funcfiletrace special parameter. */

/**/
static char **
funcfiletracegetfn(UNUSED(Param pm))
{
    Funcstack f;
    int num;
    char **ret, **p;

    for (f = funcstack, num = 0; f; f = f->prev, num++);

    ret = (char **) zhalloc((num + 1) * sizeof(char *));

    for (f = funcstack, p = ret; f; f = f->prev, p++) {
	char *colonpair, *fname;

	if (!f->prev || f->prev->tp == FS_SOURCE) {
	    /*
	     * Calling context is a file---either the parent
	     * script or interactive shell, or a sourced
	     * script.  Just print the file information for the caller
	     * (same as $functrace)
	     */
	    colonpair = zhalloc(strlen(f->caller) +
				(f->lineno > 9999 ? 24 : 6));
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
	    sprintf(colonpair, "%s:%lld", f->caller, f->lineno);
#else
	    sprintf(colonpair, "%s:%ld", f->caller, (long)f->lineno);
#endif
	} else {
	    /*
	     * Calling context is a function or eval; we need to find
	     * the line number in the file where that function was
	     * defined or the eval was called.  For this we need the
	     * $funcsourcetrace information for the context above,
	     * together with the $functrace line number for the current
	     * context.
	     */
	    zlong flineno = f->prev->flineno + f->lineno;
	    /*
	     * Line numbers in eval start from 1, not zero,
	     * so offset by one to get line in file.
	     */
	    if (f->prev->tp == FS_EVAL)
		flineno--;
	    fname = f->prev->filename ? f->prev->filename : "";

	    colonpair = zhalloc(strlen(fname) + (flineno > 9999 ? 24 : 6));
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
	    sprintf(colonpair, "%s:%lld", fname, flineno);
#else
	    sprintf(colonpair, "%s:%ld", fname, (long)flineno);
#endif
	}

	*p = colonpair;
    }
    *p = NULL;

    return ret;
}

/* Functions for the builtins special parameter. */

/**/
static HashNode
getbuiltin(UNUSED(HashTable ht), const char *name, int dis)
{
    Param pm = NULL;
    Builtin bn;

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR | PM_READONLY;
    pm->gsu.s = &nullsetscalar_gsu;
    if ((bn = (Builtin) builtintab->getnode2(builtintab, name)) &&
	(dis ? (bn->node.flags & DISABLED) : !(bn->node.flags & DISABLED))) {
	char *t = ((bn->handlerfunc || (bn->node.flags & BINF_PREFIX)) ?
		   "defined" : "undefined");

	pm->u.str = dupstring(t);
    } else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static HashNode
getpmbuiltin(HashTable ht, const char *name)
{
    return getbuiltin(ht, name, 0);
}

/**/
static HashNode
getpmdisbuiltin(HashTable ht, const char *name)
{
    return getbuiltin(ht, name, DISABLED);
}

/**/
static void
scanbuiltins(UNUSED(HashTable ht), ScanFunc func, int flags, int dis)
{
    struct param pm;
    int i;
    HashNode hn;

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR | PM_READONLY;
    pm.gsu.s = &nullsetscalar_gsu;

    for (i = 0; i < builtintab->hsize; i++)
	for (hn = builtintab->nodes[i]; hn; hn = hn->next) {
	    if (dis ? (hn->flags & DISABLED) : !(hn->flags & DISABLED)) {
		pm.node.nam = hn->nam;
		if (func != scancountparams &&
		    ((flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
		     !(flags & SCANPM_WANTKEYS))) {
		    char *t = ((((Builtin) hn)->handlerfunc ||
				(hn->flags & BINF_PREFIX)) ?
			       "defined" : "undefined");

		    pm.u.str = dupstring(t);
		}
		func(&pm.node, flags);
	    }
	}
}

/**/
static void
scanpmbuiltins(HashTable ht, ScanFunc func, int flags)
{
    scanbuiltins(ht, func, flags, 0);
}

/**/
static void
scanpmdisbuiltins(HashTable ht, ScanFunc func, int flags)
{
    scanbuiltins(ht, func, flags, DISABLED);
}

/* Functions for the reswords special parameter. */

/**/
static char **
getreswords(int dis)
{
    int i;
    HashNode hn;
    char **ret, **p;

    p = ret = (char **) zhalloc((reswdtab->ct + 1) * sizeof(char *));

    for (i = 0; i < reswdtab->hsize; i++)
	for (hn = reswdtab->nodes[i]; hn; hn = hn->next)
	    if (dis ? (hn->flags & DISABLED) : !(hn->flags & DISABLED))
		*p++ = dupstring(hn->nam);
    *p = NULL;

    return ret;
}

/**/
static char **
reswordsgetfn(UNUSED(Param pm))
{
    return getreswords(0);
}

/**/
static char **
disreswordsgetfn(UNUSED(Param pm))
{
    return getreswords(DISABLED);
}

/* Functions for the patchars special parameter. */

/**/
static char **
getpatchars(int dis)
{
    int i;
    char **ret, **p;

    p = ret = (char **) zhalloc(ZPC_COUNT * sizeof(char *));

    for (i = 0; i < ZPC_COUNT; i++)
	if (zpc_strings[i] && !dis == !zpc_disables[i])
	    *p++ = dupstring(zpc_strings[i]);

    *p = NULL;

    return ret;
}

static char **
patcharsgetfn(UNUSED(Param pm))
{
    return getpatchars(0);
}

static char **
dispatcharsgetfn(UNUSED(Param pm))
{
    return getpatchars(1);
}

/* Functions for the options special parameter. */

/**/
static void
setpmoption(Param pm, char *value)
{
    int n;

    if (!value || (strcmp(value, "on") && strcmp(value, "off")))
	zwarn("invalid value: %s", value);
    else if (!(n = optlookup(pm->node.nam)))
	zwarn("no such option: %s", pm->node.nam);
    else if (dosetopt(n, (value && strcmp(value, "off")), 0, opts))
	zwarn("can't change option: %s", pm->node.nam);
    zsfree(value);
}

/**/
static void
unsetpmoption(Param pm, UNUSED(int exp))
{
    int n;

    if (!(n = optlookup(pm->node.nam)))
	zwarn("no such option: %s", pm->node.nam);
    else if (dosetopt(n, 0, 0, opts))
	zwarn("can't change option: %s", pm->node.nam);
}

/**/
static void
setpmoptions(Param pm, HashTable ht)
{
    int i;
    HashNode hn;

    if (!ht)
	return;

    for (i = 0; i < ht->hsize; i++)
	for (hn = ht->nodes[i]; hn; hn = hn->next) {
	    struct value v;
	    char *val;

	    v.isarr = v.flags = v.start = 0;
	    v.end = -1;
	    v.arr = NULL;
	    v.pm = (Param) hn;

	    val = getstrvalue(&v);
	    if (!val || (strcmp(val, "on") && strcmp(val, "off")))
		zwarn("invalid value: %s", val);
	    else if (dosetopt(optlookup(hn->nam),
			      (val && strcmp(val, "off")), 0, opts))
		zwarn("can't change option: %s", hn->nam);
	}
    /* See setpmcommands() above */
    if (ht != pm->u.hash)
	deleteparamtable(ht);
}

static const struct gsu_scalar pmoption_gsu =
{ strgetfn, setpmoption, unsetpmoption };

/**/
static HashNode
getpmoption(UNUSED(HashTable ht), const char *name)
{
    Param pm = NULL;
    int n;

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR;
    pm->gsu.s = &pmoption_gsu;

    if ((n = optlookup(name)))
    {
	int ison;
	if (n > 0)
	    ison = opts[n];
	else
	    ison = !opts[-n];
	pm->u.str = dupstring(ison ? "on" : "off");
    }
    else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static void
scanpmoptions(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    struct param pm;
    int i;
    HashNode hn;

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR;
    pm.gsu.s = &pmoption_gsu;

    for (i = 0; i < optiontab->hsize; i++)
	for (hn = optiontab->nodes[i]; hn; hn = hn->next) {
	    int optno = ((Optname) hn)->optno, ison;
	    pm.node.nam = hn->nam;
	    ison = optno < 0 ? !opts[-optno] : opts[optno];
	    pm.u.str = dupstring(ison ? "on" : "off");
	    func(&pm.node, flags);
	}
}

/* Functions for the modules special parameter. */

/**/
static HashNode
getpmmodule(UNUSED(HashTable ht), const char *name)
{
    Param pm = NULL;
    char *type = NULL;
    Module m;

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR | PM_READONLY;
    pm->gsu.s = &nullsetscalar_gsu;

    m = (Module)modulestab->getnode2(modulestab, name);

    if (!m)
	return NULL;
    if (m->u.handle && !(m->node.flags & MOD_UNLOAD)) {
	type = ((m->node.flags & MOD_ALIAS) ?
		dyncat("alias:", m->u.alias) : "loaded");
    }
    if (!type) {
	if (m->autoloads && firstnode(m->autoloads))
	    type = "autoloaded";
    }
    if (type)
	pm->u.str = dupstring(type);
    else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static void
scanpmmodules(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    struct param pm;
    int i;
    HashNode hn;
    LinkList done = newlinklist();
    Module m;
    Conddef p;
    char *loaded = dupstring("loaded");

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR | PM_READONLY;
    pm.gsu.s = &nullsetscalar_gsu;

    for (i = 0; i < modulestab->hsize; i++) {
	for (hn = modulestab->nodes[i]; hn; hn = hn->next) {
	    m = (Module) hn;
	    if (m->u.handle && !(m->node.flags & MOD_UNLOAD)) {
		pm.node.nam = m->node.nam;
		pm.u.str = ((m->node.flags & MOD_ALIAS) ?
			    dyncat("alias:", m->u.alias) : loaded);
		addlinknode(done, pm.node.nam);
		func(&pm.node, flags);
	    }
	}
    }
    pm.u.str = dupstring("autoloaded");
    for (i = 0; i < builtintab->hsize; i++)
	for (hn = builtintab->nodes[i]; hn; hn = hn->next) {
	    if (!(((Builtin) hn)->node.flags & BINF_ADDED) &&
		!linknodebystring(done, ((Builtin) hn)->optstr)) {
		pm.node.nam = ((Builtin) hn)->optstr;
		addlinknode(done, pm.node.nam);
		func(&pm.node, flags);
	    }
	}
    for (p = condtab; p; p = p->next)
	if (p->module && !linknodebystring(done, p->module)) {
	    pm.node.nam = p->module;
	    addlinknode(done, pm.node.nam);
	    func(&pm.node, flags);
	}
    for (i = 0; i < realparamtab->hsize; i++)
	for (hn = realparamtab->nodes[i]; hn; hn = hn->next) {
	    if ((((Param) hn)->node.flags & PM_AUTOLOAD) &&
		!linknodebystring(done, ((Param) hn)->u.str)) {
		pm.node.nam = ((Param) hn)->u.str;
		addlinknode(done, pm.node.nam);
		func(&pm.node, flags);
	    }
	}
}

/* Functions for the dirstack special parameter. */

/**/
static void
dirssetfn(UNUSED(Param pm), char **x)
{
    char **ox = x;

    if (!incleanup) {
	freelinklist(dirstack, freestr);
	dirstack = znewlinklist();
	while (x && *x)
	    zaddlinknode(dirstack, ztrdup(*x++));
    }
    if (ox)
	freearray(ox);
}

/**/
static char **
dirsgetfn(UNUSED(Param pm))
{
    return hlinklist2array(dirstack, 1);
}

/* Functions for the history special parameter. */

/**/
static HashNode
getpmhistory(UNUSED(HashTable ht), const char *name)
{
    Param pm = NULL;
    Histent he;
    const char *p;
    int ok = 1;

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR | PM_READONLY;
    pm->gsu.s = &nullsetscalar_gsu;

    if (*name != '0' || name[1]) {
	if (*name == '0')
	    ok = 0;
	else {
	    for (p = name; *p && idigit(*p); p++);
	    if (*p)
		ok = 0;
	}
    }
    if (ok && (he = quietgethist(atoi(name))))
	pm->u.str = dupstring(he->node.nam);
    else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static void
scanpmhistory(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    struct param pm;
    int i = addhistnum(curhist, -1, HIST_FOREIGN);
    Histent he = gethistent(i, GETHIST_UPWARD);
    char buf[40];

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR | PM_READONLY;
    pm.gsu.s = &nullsetscalar_gsu;

    while (he) {
	if (func != scancountparams) {
	    convbase(buf, he->histnum, 10);
	    pm.node.nam = dupstring(buf);
	    if ((flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
		!(flags & SCANPM_WANTKEYS))
		pm.u.str = dupstring(he->node.nam);
	}
	func(&pm.node, flags);

	he = up_histent(he);
    }
}

/* Function for the historywords special parameter. */

/**/
static char **
histwgetfn(UNUSED(Param pm))
{
    char *h, *e, sav;
    LinkList l = newlinklist(), ll;
    LinkNode n;
    int i = addhistnum(curhist, -1, HIST_FOREIGN), iw;
    Histent he = gethistent(i, GETHIST_UPWARD);

    if ((ll = bufferwords(NULL, NULL, NULL, 0)))
        for (n = firstnode(ll); n; incnode(n))
            pushnode(l, getdata(n));

    while (he) {
	for (iw = he->nwords - 1; iw >= 0; iw--) {
	    h = he->node.nam + he->words[iw * 2];
	    e = he->node.nam + he->words[iw * 2 + 1];
	    sav = *e;
	    *e = '\0';
	    addlinknode(l, dupstring(h));
	    *e = sav;
	}
	he = up_histent(he);
    }

    return hlinklist2array(l, 0);
}

/* Functions for the jobtexts special parameter. */

/**/
static char *
pmjobtext(Job jtab, int job)
{
    Process pn;
    int len = 1;
    char *ret;

    for (pn = jtab[job].procs; pn; pn = pn->next)
	len += strlen(pn->text) + 3;

    ret = (char *) zhalloc(len);
    ret[0] = '\0';

    for (pn = jtab[job].procs; pn; pn = pn->next) {
	strcat(ret, pn->text);
	if (pn->next)
	    strcat(ret, " | ");
    }
    return ret;
}

/**/
static HashNode
getpmjobtext(UNUSED(HashTable ht), const char *name)
{
    Param pm = NULL;
    int job, jmax;
    char *pend;
    Job jtab;

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR | PM_READONLY;
    pm->gsu.s = &nullsetscalar_gsu;

    selectjobtab(&jtab, &jmax);

    job = strtod(name, &pend);
    /* Non-numeric keys are looked up by job name */
    if (*pend)
	job = getjob(name, NULL);
    if (job >= 1 && job <= jmax &&
	jtab[job].stat && jtab[job].procs &&
	!(jtab[job].stat & STAT_NOPRINT))
	pm->u.str = pmjobtext(jtab, job);
    else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static void
scanpmjobtexts(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    struct param pm;
    int job, jmax;
    char buf[40];
    Job jtab;

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR | PM_READONLY;
    pm.gsu.s = &nullsetscalar_gsu;

    selectjobtab(&jtab, &jmax);

    for (job = 1; job <= jmax; job++) {
	if (jtab[job].stat && jtab[job].procs &&
	    !(jtab[job].stat & STAT_NOPRINT)) {
	    if (func != scancountparams) {
		sprintf(buf, "%d", job);
		pm.node.nam = dupstring(buf);
		if ((flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
		    !(flags & SCANPM_WANTKEYS))
		    pm.u.str = pmjobtext(jtab, job);
	    }
	    func(&pm.node, flags);
	}
    }
}

/* Functions for the jobstates special parameter. */

/**/
static char *
pmjobstate(Job jtab, int job)
{
    Process pn;
    char buf[256], buf2[128], *ret, *state, *cp;

    if (job == curjob)
	cp = ":+";
    else if (job == prevjob)
	cp = ":-";
    else
	cp = ":";

    if (jtab[job].stat & STAT_DONE)
	ret = dyncat("done", cp);
    else if (jtab[job].stat & STAT_STOPPED)
	ret = dyncat("suspended", cp);
    else
	ret = dyncat("running", cp);

    for (pn = jtab[job].procs; pn; pn = pn->next) {

	if (pn->status == SP_RUNNING)
	    state = "running";
	else if (WIFEXITED(pn->status)) {
	    if (WEXITSTATUS(pn->status))
		sprintf((state = buf2), "exit %d", (pn->status));
	    else
		state = "done";
	} else if (WIFSTOPPED(pn->status))
	    state = sigmsg(WSTOPSIG(pn->status));
	else if (WCOREDUMP(pn->status))
	    sprintf((state = buf2), "%s (core dumped)",
		    sigmsg(WTERMSIG(pn->status)));
	else
	    state = sigmsg(WTERMSIG(pn->status));

	sprintf(buf, ":%d=%s", (int)pn->pid, state);

	ret = dyncat(ret, buf);
    }
    return ret;
}

/**/
static HashNode
getpmjobstate(UNUSED(HashTable ht), const char *name)
{
    Param pm = NULL;
    int job, jmax;
    char *pend;
    Job jtab;

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR | PM_READONLY;
    pm->gsu.s = &nullsetscalar_gsu;

    selectjobtab(&jtab, &jmax);

    job = strtod(name, &pend);
    if (*pend)
	job = getjob(name, NULL);
    if (job >= 1 && job <= jmax &&
	jtab[job].stat && jtab[job].procs &&
	!(jtab[job].stat & STAT_NOPRINT))
	pm->u.str = pmjobstate(jtab, job);
    else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static void
scanpmjobstates(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    struct param pm;
    int job, jmax;
    Job jtab;
    char buf[40];

    selectjobtab(&jtab, &jmax);

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR | PM_READONLY;
    pm.gsu.s = &nullsetscalar_gsu;

    for (job = 1; job <= jmax; job++) {
	if (jtab[job].stat && jtab[job].procs &&
	    !(jtab[job].stat & STAT_NOPRINT)) {
	    if (func != scancountparams) {
		sprintf(buf, "%d", job);
		pm.node.nam = dupstring(buf);
		if ((flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
		    !(flags & SCANPM_WANTKEYS))
		    pm.u.str = pmjobstate(jtab, job);
	    }
	    func(&pm.node, flags);
	}
    }
}

/* Functions for the jobdirs special parameter. */

/**/
static char *
pmjobdir(Job jtab, int job)
{
    char *ret;

    ret = dupstring(jtab[job].pwd ? jtab[job].pwd : pwd);
    return ret;
}

/**/
static HashNode
getpmjobdir(UNUSED(HashTable ht), const char *name)
{
    Param pm = NULL;
    int job, jmax;
    char *pend;
    Job jtab;

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR | PM_READONLY;
    pm->gsu.s = &nullsetscalar_gsu;

    selectjobtab(&jtab, &jmax);

    job = strtod(name, &pend);
    if (*pend)
	job = getjob(name, NULL);
    if (job >= 1 && job <= jmax &&
	jtab[job].stat && jtab[job].procs &&
	!(jtab[job].stat & STAT_NOPRINT))
	pm->u.str = pmjobdir(jtab, job);
    else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static void
scanpmjobdirs(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    struct param pm;
    int job, jmax;
    char buf[40];
    Job jtab;

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR | PM_READONLY;
    pm.gsu.s = &nullsetscalar_gsu;

    selectjobtab(&jtab, &jmax);

    for (job = 1; job <= jmax; job++) {
       if (jtab[job].stat && jtab[job].procs &&
           !(jtab[job].stat & STAT_NOPRINT)) {
           if (func != scancountparams) {
	       sprintf(buf, "%d", job);
	       pm.node.nam = dupstring(buf);
               if ((flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
		   !(flags & SCANPM_WANTKEYS))
		   pm.u.str = pmjobdir(jtab, job);
	   }
           func(&pm.node, flags);
       }
    }
}

/* Functions for the nameddirs special parameter. */

/**/
static void
setpmnameddir(Param pm, char *value)
{
    if (!value)
	zwarn("invalid value: ''");
    else {
	Nameddir nd = (Nameddir) zshcalloc(sizeof(*nd));

	nd->node.flags = 0;
	nd->dir = value;
	nameddirtab->addnode(nameddirtab, ztrdup(pm->node.nam), nd);
    }
}

/**/
static void
unsetpmnameddir(Param pm, UNUSED(int exp))
{
    HashNode hd = nameddirtab->removenode(nameddirtab, pm->node.nam);

    if (hd)
	nameddirtab->freenode(hd);
}

/**/
static void
setpmnameddirs(Param pm, HashTable ht)
{
    int i;
    HashNode hn, next, hd;

    if (!ht)
	return;

    for (i = 0; i < nameddirtab->hsize; i++)
	for (hn = nameddirtab->nodes[i]; hn; hn = next) {
	    next = hn->next;
	    if (!(((Nameddir) hn)->node.flags & ND_USERNAME) &&
		(hd = nameddirtab->removenode(nameddirtab, hn->nam)))
		nameddirtab->freenode(hd);
	}

    for (i = 0; i < ht->hsize; i++)
	for (hn = ht->nodes[i]; hn; hn = hn->next) {
	    struct value v;
	    char *val;

	    v.isarr = v.flags = v.start = 0;
	    v.end = -1;
	    v.arr = NULL;
	    v.pm = (Param) hn;

	    if (!(val = getstrvalue(&v)))
		zwarn("invalid value: ''");
	    else {
		Nameddir nd = (Nameddir) zshcalloc(sizeof(*nd));

		nd->node.flags = 0;
		nd->dir = ztrdup(val);
		nameddirtab->addnode(nameddirtab, ztrdup(hn->nam), nd);
	    }
	}

    /* The INTERACTIVE stuff ensures that the dirs are not immediately removed
     * when the sub-pms are deleted. */

    i = opts[INTERACTIVE];
    opts[INTERACTIVE] = 0;
    /* See setpmcommands() above */
    if (ht != pm->u.hash)
	deleteparamtable(ht);
    opts[INTERACTIVE] = i;
}

static const struct gsu_scalar pmnamedir_gsu =
{ strgetfn, setpmnameddir, unsetpmnameddir };

/**/
static HashNode
getpmnameddir(UNUSED(HashTable ht), const char *name)
{
    Param pm = NULL;
    Nameddir nd;

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR;
    pm->gsu.s = &pmnamedir_gsu;
    if ((nd = (Nameddir) nameddirtab->getnode(nameddirtab, name)) &&
	!(nd->node.flags & ND_USERNAME))
	pm->u.str = dupstring(nd->dir);
    else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static void
scanpmnameddirs(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    struct param pm;
    int i;
    HashNode hn;
    Nameddir nd;

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR;
    pm.gsu.s = &pmnamedir_gsu;

    for (i = 0; i < nameddirtab->hsize; i++)
	for (hn = nameddirtab->nodes[i]; hn; hn = hn->next) {
	    if (!((nd = (Nameddir) hn)->node.flags & ND_USERNAME)) {
		pm.node.nam = hn->nam;
		if (func != scancountparams &&
		    ((flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
		     !(flags & SCANPM_WANTKEYS)))
		    pm.u.str = dupstring(nd->dir);
		func(&pm.node, flags);
	    }
	}
}

/* Functions for the userdirs special parameter. */

/**/
static HashNode
getpmuserdir(UNUSED(HashTable ht), const char *name)
{
    Param pm = NULL;
    Nameddir nd;

    nameddirtab->filltable(nameddirtab);

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR | PM_READONLY;
    pm->gsu.s = &nullsetscalar_gsu;
    if ((nd = (Nameddir) nameddirtab->getnode(nameddirtab, name)) &&
	(nd->node.flags & ND_USERNAME))
	pm->u.str = dupstring(nd->dir);
    else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static void
scanpmuserdirs(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    struct param pm;
    int i;
    HashNode hn;
    Nameddir nd;

    nameddirtab->filltable(nameddirtab);

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR | PM_READONLY;
    pm.gsu.s = &nullsetscalar_gsu;

    for (i = 0; i < nameddirtab->hsize; i++)
	for (hn = nameddirtab->nodes[i]; hn; hn = hn->next) {
	    if ((nd = (Nameddir) hn)->node.flags & ND_USERNAME) {
		pm.node.nam = hn->nam;
		if (func != scancountparams &&
		    ((flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
		     !(flags & SCANPM_WANTKEYS)))
		    pm.u.str = dupstring(nd->dir);
		func(&pm.node, flags);
	    }
	}
}

/* Functions for the raliases, galiases and saliases special parameters. */

/**/
static void
setalias(HashTable ht, Param pm, char *value, int flags)
{
    ht->addnode(ht, ztrdup(pm->node.nam),
		createaliasnode(value, flags));
}

/**/
static void
setpmralias(Param pm, char *value)
{
    setalias(aliastab, pm, value, 0);
}

/**/
static void
setpmdisralias(Param pm, char *value)
{
    setalias(aliastab, pm, value, DISABLED);
}

/**/
static void
setpmgalias(Param pm, char *value)
{
    setalias(aliastab, pm, value, ALIAS_GLOBAL);
}

/**/
static void
setpmdisgalias(Param pm, char *value)
{
    setalias(aliastab, pm, value, ALIAS_GLOBAL|DISABLED);
}

/**/
static void
setpmsalias(Param pm, char *value)
{
    setalias(sufaliastab, pm, value, ALIAS_SUFFIX);
}

/**/
static void
setpmdissalias(Param pm, char *value)
{
    setalias(sufaliastab, pm, value, ALIAS_SUFFIX|DISABLED);
}

/**/
static void
unsetpmalias(Param pm, UNUSED(int exp))
{
    HashNode hd = aliastab->removenode(aliastab, pm->node.nam);

    if (hd)
	aliastab->freenode(hd);
}

/**/
static void
unsetpmsalias(Param pm, UNUSED(int exp))
{
    HashNode hd = sufaliastab->removenode(sufaliastab, pm->node.nam);

    if (hd)
	sufaliastab->freenode(hd);
}

/**/
static void
setaliases(HashTable alht, Param pm, HashTable ht, int flags)
{
    int i;
    HashNode hn, next, hd;

    if (!ht)
	return;

    for (i = 0; i < alht->hsize; i++)
	for (hn = alht->nodes[i]; hn; hn = next) {
	    next = hn->next;
	    /*
	     * The following respects the DISABLED flag, e.g.
	     * we get a different behaviour for raliases and dis_raliases.
	     * The predecessor to this code didn't do that; presumably
	     * that was a bug.
	     */
	    if (flags == ((Alias)hn)->node.flags &&
		(hd = alht->removenode(alht, hn->nam)))
		alht->freenode(hd);
	}

    for (i = 0; i < ht->hsize; i++)
	for (hn = ht->nodes[i]; hn; hn = hn->next) {
	    struct value v;
	    char *val;

	    v.isarr = v.flags = v.start = 0;
	    v.end = -1;
	    v.arr = NULL;
	    v.pm = (Param) hn;

	    if ((val = getstrvalue(&v)))
		alht->addnode(alht, ztrdup(hn->nam),
			      createaliasnode(ztrdup(val), flags));
	}
    /* See setpmcommands() above */
    if (ht != pm->u.hash)
	deleteparamtable(ht);
}

/**/
static void
setpmraliases(Param pm, HashTable ht)
{
    setaliases(aliastab, pm, ht, 0);
}

/**/
static void
setpmdisraliases(Param pm, HashTable ht)
{
    setaliases(aliastab, pm, ht, DISABLED);
}

/**/
static void
setpmgaliases(Param pm, HashTable ht)
{
    setaliases(aliastab, pm, ht, ALIAS_GLOBAL);
}

/**/
static void
setpmdisgaliases(Param pm, HashTable ht)
{
    setaliases(aliastab, pm, ht, ALIAS_GLOBAL|DISABLED);
}

/**/
static void
setpmsaliases(Param pm, HashTable ht)
{
    setaliases(sufaliastab, pm, ht, ALIAS_SUFFIX);
}

/**/
static void
setpmdissaliases(Param pm, HashTable ht)
{
    setaliases(sufaliastab, pm, ht, ALIAS_SUFFIX|DISABLED);
}

static const struct gsu_scalar pmralias_gsu =
{ strgetfn, setpmralias, unsetpmalias };
static const struct gsu_scalar pmgalias_gsu =
{ strgetfn, setpmgalias, unsetpmalias };
static const struct gsu_scalar pmsalias_gsu =
{ strgetfn, setpmsalias, unsetpmsalias };
static const struct gsu_scalar pmdisralias_gsu =
{ strgetfn, setpmdisralias, unsetpmalias };
static const struct gsu_scalar pmdisgalias_gsu =
{ strgetfn, setpmdisgalias, unsetpmalias };
static const struct gsu_scalar pmdissalias_gsu =
{ strgetfn, setpmdissalias, unsetpmsalias };

/**/
static void
assignaliasdefs(Param pm, int flags)
{
    pm->node.flags = PM_SCALAR;

    /* we really need to squirrel the flags away somewhere... */
    switch (flags) {
    case 0:
	pm->gsu.s = &pmralias_gsu;
	break;

    case ALIAS_GLOBAL:
	pm->gsu.s = &pmgalias_gsu;
	break;

    case ALIAS_SUFFIX:
	pm->gsu.s = &pmsalias_gsu;
	break;

    case DISABLED:
	pm->gsu.s = &pmdisralias_gsu;
	break;

    case ALIAS_GLOBAL|DISABLED:
	pm->gsu.s = &pmdisgalias_gsu;
	break;

    case ALIAS_SUFFIX|DISABLED:
	pm->gsu.s = &pmdissalias_gsu;
	break;
    }
}

/**/
static HashNode
getalias(HashTable alht, UNUSED(HashTable ht), const char *name, int flags)
{
    Param pm = NULL;
    Alias al;

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);

    assignaliasdefs(pm, flags);

    if ((al = (Alias) alht->getnode2(alht, name)) &&
	flags == al->node.flags)
	pm->u.str = dupstring(al->text);
    else {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    }
    return &pm->node;
}

/**/
static HashNode
getpmralias(HashTable ht, const char *name)
{
    return getalias(aliastab, ht, name, 0);
}

/**/
static HashNode
getpmdisralias(HashTable ht, const char *name)
{
    return getalias(aliastab, ht, name, DISABLED);
}

/**/
static HashNode
getpmgalias(HashTable ht, const char *name)
{
    return getalias(aliastab, ht, name, ALIAS_GLOBAL);
}

/**/
static HashNode
getpmdisgalias(HashTable ht, const char *name)
{
    return getalias(aliastab, ht, name, ALIAS_GLOBAL|DISABLED);
}

/**/
static HashNode
getpmsalias(HashTable ht, const char *name)
{
    return getalias(sufaliastab, ht, name, ALIAS_SUFFIX);
}

/**/
static HashNode
getpmdissalias(HashTable ht, const char *name)
{
    return getalias(sufaliastab, ht, name, ALIAS_SUFFIX|DISABLED);
} 

/**/
static void
scanaliases(HashTable alht, UNUSED(HashTable ht), ScanFunc func,
	    int pmflags, int alflags)
{
    struct param pm;
    int i;
    Alias al;

    memset((void *)&pm, 0, sizeof(struct param));
    assignaliasdefs(&pm, alflags);

    for (i = 0; i < alht->hsize; i++)
	for (al = (Alias) alht->nodes[i]; al; al = (Alias) al->node.next) {
	    if (alflags == al->node.flags) {
		pm.node.nam = al->node.nam;
		if (func != scancountparams &&
		    ((pmflags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
		     !(pmflags & SCANPM_WANTKEYS)))
		    pm.u.str = dupstring(al->text);
		func(&pm.node, pmflags);
	    }
	}
}

/**/
static void
scanpmraliases(HashTable ht, ScanFunc func, int flags)
{
    scanaliases(aliastab, ht, func, flags, 0);
}

/**/
static void
scanpmdisraliases(HashTable ht, ScanFunc func, int flags)
{
    scanaliases(aliastab, ht, func, flags, DISABLED);
}

/**/
static void
scanpmgaliases(HashTable ht, ScanFunc func, int flags)
{
    scanaliases(aliastab, ht, func, flags, ALIAS_GLOBAL);
}

/**/
static void
scanpmdisgaliases(HashTable ht, ScanFunc func, int flags)
{
    scanaliases(aliastab, ht, func, flags, ALIAS_GLOBAL|DISABLED);
}

/**/
static void
scanpmsaliases(HashTable ht, ScanFunc func, int flags)
{
    scanaliases(sufaliastab, ht, func, flags, ALIAS_SUFFIX);
}

/**/
static void
scanpmdissaliases(HashTable ht, ScanFunc func, int flags)
{
    scanaliases(sufaliastab, ht, func, flags, ALIAS_SUFFIX|DISABLED);
}


/* Functions for the usergroups special parameter */

/*
 * Get GID and names for groups of which the current user is a member.
 */

/**/
static Groupset get_all_groups(void)
{
#ifdef DISABLE_DYNAMIC_NSS
    return NULL;
#else
    Groupset gs = zhalloc(sizeof(*gs));
    Groupmap gaptr;
    gid_t *list, *lptr, egid;
    int add_egid;
    struct group *grptr;

    egid = getegid();
    add_egid = 1;
    gs->num = getgroups(0, NULL);
    if (gs->num > 0) {
	list = zhalloc(gs->num * sizeof(*list));
	if (getgroups(gs->num, list) < 0) {
	    return NULL;
	}

	/*
	 * It's unspecified whether $EGID is included in the
	 * group set, so check.
	 */
	for (lptr = list; lptr < list + gs->num; lptr++) {
	    if (*lptr == egid) {
		add_egid = 0;
		break;
	    }
	}
	gs->array = zhalloc((gs->num + add_egid) * sizeof(*gs->array));
	/* Put EGID if needed first */
	gaptr = gs->array + add_egid;
	for (lptr = list; lptr < list + gs->num; lptr++) {
	    gaptr->gid = *lptr;
	    gaptr++;
	}
	gs->num += add_egid;
    } else {
	/* Just use effective GID */
	gs->num = 1;
	gs->array = zhalloc(sizeof(*gs->array));
    }
    if (add_egid) {
	gs->array->gid = egid;
    }

    /* Get group names */
    for (gaptr = gs->array; gaptr < gs->array + gs->num; gaptr++) {
	grptr = getgrgid(gaptr->gid);
	if (!grptr) {
	    return NULL;
	}
	gaptr->name = dupstring(grptr->gr_name);
    }

    return gs;
#endif /* DISABLE_DYNAMIC_NSS */
}

/* Standard hash element lookup. */

/**/
static HashNode
getpmusergroups(UNUSED(HashTable ht), const char *name)
{
    Param pm = NULL;
    Groupset gs = get_all_groups();
    Groupmap gaptr;

    pm = (Param)hcalloc(sizeof(struct param));
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR | PM_READONLY;
    pm->gsu.s = &nullsetscalar_gsu;

    if (!gs) {
#ifdef DISABLE_DYNAMIC_NSS
	zerr("parameter 'usergroups' not available: NSS is disabled");
#else
	zerr("failed to retrieve groups for user: %e", errno);
#endif
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
	return &pm->node;
    }

    for (gaptr = gs->array; gaptr < gs->array + gs->num; gaptr++) {
	if (!strcmp(name, gaptr->name)) {
	    char buf[DIGBUFSIZE];

	    sprintf(buf, "%d", (int)gaptr->gid);
	    pm->u.str = dupstring(buf);
	    return &pm->node;
	}
    }

    pm->u.str = dupstring("");
    pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    return &pm->node;
}

/* Standard hash scan. */

/**/
static void
scanpmusergroups(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    struct param pm;
    Groupset gs = get_all_groups();
    Groupmap gaptr;

    if (!gs) {
#ifdef DISABLE_DYNAMIC_NSS
	zerr("parameter 'usergroups' not available: NSS is disabled");
#else
	zerr("failed to retrieve groups for user: %e", errno);
#endif
	return;
    }

    memset((void *)&pm, 0, sizeof(pm));
    pm.node.flags = PM_SCALAR | PM_READONLY;
    pm.gsu.s = &nullsetscalar_gsu;

    for (gaptr = gs->array; gaptr < gs->array + gs->num; gaptr++) {
	pm.node.nam = gaptr->name;
	if (func != scancountparams &&
	    ((flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)) ||
	     !(flags & SCANPM_WANTKEYS))) {
	    char buf[DIGBUFSIZE];

	    sprintf(buf, "%d", (int)gaptr->gid);
	    pm.u.str = dupstring(buf);
	}
	func(&pm.node, flags);
    }
}


/* Table for defined parameters. */

struct pardef {
    char *name;
    int flags;
    GetNodeFunc getnfn;
    ScanTabFunc scantfn;
    GsuHash hash_gsu;
    GsuArray array_gsu;
    Param pm;
};

static const struct gsu_hash pmcommands_gsu =
{ hashgetfn, setpmcommands, stdunsetfn };
static const struct gsu_hash pmfunctions_gsu =
{ hashgetfn, setpmfunctions, stdunsetfn };
static const struct gsu_hash pmdisfunctions_gsu =
{ hashgetfn, setpmdisfunctions, stdunsetfn };
static const struct gsu_hash pmoptions_gsu =
{ hashgetfn, setpmoptions, stdunsetfn };
static const struct gsu_hash pmnameddirs_gsu =
{ hashgetfn, setpmnameddirs, stdunsetfn };
static const struct gsu_hash pmraliases_gsu =
{ hashgetfn, setpmraliases, stdunsetfn };
static const struct gsu_hash pmgaliases_gsu =
{ hashgetfn, setpmgaliases, stdunsetfn };
static const struct gsu_hash pmsaliases_gsu =
{ hashgetfn, setpmsaliases, stdunsetfn };
static const struct gsu_hash pmdisraliases_gsu =
{ hashgetfn, setpmdisraliases, stdunsetfn };
static const struct gsu_hash pmdisgaliases_gsu =
{ hashgetfn, setpmdisgaliases, stdunsetfn };
static const struct gsu_hash pmdissaliases_gsu =
{ hashgetfn, setpmdissaliases, stdunsetfn };

static const struct gsu_array funcstack_gsu =
{ funcstackgetfn, arrsetfn, stdunsetfn };
static const struct gsu_array functrace_gsu =
{ functracegetfn, arrsetfn, stdunsetfn };
static const struct gsu_array funcsourcetrace_gsu =
{ funcsourcetracegetfn, arrsetfn, stdunsetfn };
static const struct gsu_array funcfiletrace_gsu =
{ funcfiletracegetfn, arrsetfn, stdunsetfn };
static const struct gsu_array reswords_gsu =
{ reswordsgetfn, arrsetfn, stdunsetfn };
static const struct gsu_array disreswords_gsu =
{ disreswordsgetfn, arrsetfn, stdunsetfn };
static const struct gsu_array patchars_gsu =
{ patcharsgetfn, arrsetfn, stdunsetfn };
static const struct gsu_array dispatchars_gsu =
{ dispatcharsgetfn, arrsetfn, stdunsetfn };
static const struct gsu_array dirs_gsu =
{ dirsgetfn, dirssetfn, stdunsetfn };
static const struct gsu_array historywords_gsu =
{ histwgetfn, arrsetfn, stdunsetfn };

/* Make sure to update autofeatures in parameter.mdd if necessary */
static struct paramdef partab[] = {
    SPECIALPMDEF("aliases", 0,
	    &pmraliases_gsu, getpmralias, scanpmraliases),
    SPECIALPMDEF("builtins", PM_READONLY_SPECIAL, NULL, getpmbuiltin, scanpmbuiltins),
    SPECIALPMDEF("commands", 0, &pmcommands_gsu, getpmcommand, scanpmcommands),
    SPECIALPMDEF("dirstack", PM_ARRAY,
	    &dirs_gsu, NULL, NULL),
    SPECIALPMDEF("dis_aliases", 0,
	    &pmdisraliases_gsu, getpmdisralias, scanpmdisraliases),
    SPECIALPMDEF("dis_builtins", PM_READONLY_SPECIAL,
	    NULL, getpmdisbuiltin, scanpmdisbuiltins),
    SPECIALPMDEF("dis_functions", 0, 
	    &pmdisfunctions_gsu, getpmdisfunction, scanpmdisfunctions),
    SPECIALPMDEF("dis_functions_source", PM_READONLY_SPECIAL, NULL,
		 getpmdisfunction_source, scanpmdisfunction_source),
    SPECIALPMDEF("dis_galiases", 0,
	    &pmdisgaliases_gsu, getpmdisgalias, scanpmdisgaliases),
    SPECIALPMDEF("dis_patchars", PM_ARRAY|PM_READONLY_SPECIAL,
	    &dispatchars_gsu, NULL, NULL),
    SPECIALPMDEF("dis_reswords", PM_ARRAY|PM_READONLY_SPECIAL,
	    &disreswords_gsu, NULL, NULL),
    SPECIALPMDEF("dis_saliases", 0,
	    &pmdissaliases_gsu, getpmdissalias, scanpmdissaliases),
    SPECIALPMDEF("funcfiletrace", PM_ARRAY|PM_READONLY_SPECIAL,
	    &funcfiletrace_gsu, NULL, NULL),
    SPECIALPMDEF("funcsourcetrace", PM_ARRAY|PM_READONLY_SPECIAL,
	    &funcsourcetrace_gsu, NULL, NULL),
    SPECIALPMDEF("funcstack", PM_ARRAY|PM_READONLY_SPECIAL,
	    &funcstack_gsu, NULL, NULL),
    SPECIALPMDEF("functions", 0, &pmfunctions_gsu, getpmfunction,
		 scanpmfunctions),
    SPECIALPMDEF("functions_source", PM_READONLY_SPECIAL, NULL,
		 getpmfunction_source, scanpmfunction_source),
    SPECIALPMDEF("functrace", PM_ARRAY|PM_READONLY_SPECIAL,
	    &functrace_gsu, NULL, NULL),
    SPECIALPMDEF("galiases", 0,
	    &pmgaliases_gsu, getpmgalias, scanpmgaliases),
    SPECIALPMDEF("history", PM_READONLY_SPECIAL,
	    NULL, getpmhistory, scanpmhistory),
    SPECIALPMDEF("historywords", PM_ARRAY|PM_READONLY_SPECIAL,
	    &historywords_gsu, NULL, NULL),
    SPECIALPMDEF("jobdirs", PM_READONLY_SPECIAL,
	    NULL, getpmjobdir, scanpmjobdirs),
    SPECIALPMDEF("jobstates", PM_READONLY_SPECIAL,
	    NULL, getpmjobstate, scanpmjobstates),
    SPECIALPMDEF("jobtexts", PM_READONLY_SPECIAL,
	    NULL, getpmjobtext, scanpmjobtexts),
    SPECIALPMDEF("modules", PM_READONLY_SPECIAL,
	    NULL, getpmmodule, scanpmmodules),
    SPECIALPMDEF("nameddirs", 0,
	    &pmnameddirs_gsu, getpmnameddir, scanpmnameddirs),
    SPECIALPMDEF("options", 0,
	    &pmoptions_gsu, getpmoption, scanpmoptions),
    SPECIALPMDEF("parameters", PM_READONLY_SPECIAL,
	    NULL, getpmparameter, scanpmparameters),
    SPECIALPMDEF("patchars", PM_ARRAY|PM_READONLY_SPECIAL,
	    &patchars_gsu, NULL, NULL),
    SPECIALPMDEF("reswords", PM_ARRAY|PM_READONLY_SPECIAL,
	    &reswords_gsu, NULL, NULL),
    SPECIALPMDEF("saliases", 0,
	    &pmsaliases_gsu, getpmsalias, scanpmsaliases),
    SPECIALPMDEF("userdirs", PM_READONLY_SPECIAL,
	    NULL, getpmuserdir, scanpmuserdirs),
    SPECIALPMDEF("usergroups", PM_READONLY_SPECIAL,
	    NULL, getpmusergroups, scanpmusergroups)
};

static struct features module_features = {
    NULL, 0,
    NULL, 0,
    NULL, 0,
    partab, sizeof(partab)/sizeof(*partab),
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
    int ret;
    /*
     * If we remove features, we shouldn't have an effect
     * on the main shell, so set the flag to indicate.
     */
    incleanup = 1;
    ret = handlefeatures(m, &module_features, enables);
    incleanup = 0;
    return ret;
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
    int ret;
    incleanup = 1;
    ret = setfeatureenables(m, &module_features, NULL);
    incleanup = 0;
    return ret;
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}

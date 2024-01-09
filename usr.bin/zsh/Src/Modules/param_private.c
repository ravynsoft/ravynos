/*
 * param_private.c - bindings for private parameter scopes
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 2015 Barton E. Schaefer
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Barton E. Schaefer or the Zsh Development
 * Group be liable to any party for direct, indirect, special, incidental, or
 * consequential damages arising out of the use of this software and its
 * documentation, even if Barton E. Schaefer and the Zsh
 * Development Group have been advised of the possibility of such damage.
 *
 * Barton E. Schaefer and the Zsh Development Group
 * specifically disclaim any warranties, including, but not limited to, the
 * implied warranties of merchantability and fitness for a particular purpose.
 * The software provided hereunder is on an "as is" basis, and
 * Barton E. Schaefer and the Zsh Development Group have no
 * obligation to provide maintenance, support, updates, enhancements, or
 * modifications.
 *
 */

#include "param_private.mdh"
#include "param_private.pro"

struct gsu_closure {
    union {
	struct gsu_scalar s;
	struct gsu_integer i;
	struct gsu_float f;
	struct gsu_array a;
	struct gsu_hash h;
    } u;
    void *g;
};

static const struct gsu_scalar scalar_private_gsu =
{ pps_getfn, pps_setfn, pps_unsetfn };

static const struct gsu_integer integer_private_gsu =
{ ppi_getfn, ppi_setfn, ppi_unsetfn };

static const struct gsu_float float_private_gsu =
{ ppf_getfn, ppf_setfn, ppf_unsetfn };

static const struct gsu_array array_private_gsu =
{ ppa_getfn, ppa_setfn, ppa_unsetfn };

static const struct gsu_hash hash_private_gsu =
{ pph_getfn, pph_setfn, pph_unsetfn };

/*
 * The trick here is:
 *
 * bin_private() opens a new parameter scope, then calls bin_typeset().
 *
 * bin_typeset() handles the usual parameter creation and error checks.
 *
 * makeprivate() then finds all parameters created in the new scope and
 * rejects them if they can't be "promoted" to the surrounding scope.
 * Otherwise it swaps out their GSU structure and promotes them so they
 * will be removed when the surrounding scope ends.
 *
 * bin_private() then ends the current scope, which discards any of the
 * parameters rejected by makeprivate().
 *
 */

static int makeprivate_error = 0;

static void
makeprivate(HashNode hn, UNUSED(int flags))
{
    Param pm = (Param)hn;
    if (pm->level == locallevel) {
	if (pm->ename || (pm->node.flags & PM_NORESTORE) ||
	    (pm->old &&
	     (pm->old->level == locallevel - 1 ||
	      ((pm->node.flags & (PM_SPECIAL|PM_REMOVABLE)) == PM_SPECIAL &&
	       /* typeset_single() line 2300 discards PM_REMOVABLE -- why? */
	       !is_private(pm->old))))) {
	    zwarnnam("private", "can't change scope of existing param: %s",
		     pm->node.nam);
	    makeprivate_error = 1;
	    return;
	}
	struct gsu_closure *gsu = zhalloc(sizeof(struct gsu_closure));
	switch (PM_TYPE(pm->node.flags)) {
	case PM_SCALAR:
	    gsu->g = (void *)(pm->gsu.s);
	    gsu->u.s = scalar_private_gsu;
	    pm->gsu.s = (GsuScalar)gsu;
	    break;
	case PM_INTEGER:
	    gsu->g = (void *)(pm->gsu.i);
	    gsu->u.i = integer_private_gsu;
	    pm->gsu.i = (GsuInteger)gsu;
	    break;
	case PM_EFLOAT:
	case PM_FFLOAT:
	    gsu->g = (void *)(pm->gsu.f);
	    gsu->u.f = float_private_gsu;
	    pm->gsu.f = (GsuFloat)gsu;
	    break;
	case PM_ARRAY:
	    gsu->g = (void *)(pm->gsu.a);
	    gsu->u.a = array_private_gsu;
	    pm->gsu.a = (GsuArray)gsu;
	    break;
	case PM_HASHED:
	    gsu->g = (void *)(pm->gsu.h);
	    gsu->u.h = hash_private_gsu;
	    pm->gsu.h = (GsuHash)gsu;
	    break;
	default:
	    makeprivate_error = 1;
	    break;
	}
	/* PM_HIDE so new parameters in deeper scopes do not shadow */
	pm->node.flags |= (PM_HIDE|PM_SPECIAL|PM_REMOVABLE|PM_RO_BY_DESIGN);
	pm->level -= 1;
    }
}

/**/
static int
is_private(Param pm)
{
    switch (PM_TYPE(pm->node.flags)) {
    case PM_SCALAR:
	if (!pm->gsu.s || pm->gsu.s->unsetfn != pps_unsetfn)
	    return 0;
	break;
    case PM_INTEGER:
	if (!pm->gsu.i || pm->gsu.i->unsetfn != ppi_unsetfn)
	    return 0;
	break;
    case PM_EFLOAT:
    case PM_FFLOAT:
	if (!pm->gsu.f || pm->gsu.f->unsetfn != ppf_unsetfn)
	    return 0;
	break;
    case PM_ARRAY:
	if (!pm->gsu.a || pm->gsu.a->unsetfn != ppa_unsetfn)
	    return 0;
	break;
    case PM_HASHED:
	if (!pm->gsu.h || pm->gsu.h->unsetfn != pph_unsetfn)
	    return 0;
	break;
    default:
	/* error */
	return 0;
    }
    return 1;
}

static int fakelevel;

/**/
static int
bin_private(char *nam, char **args, LinkList assigns, Options ops, int func)
{
    int from_typeset = 1;
    int ofake = fakelevel;	/* paranoia in case of recursive call */
    int hasargs = /* *args != NULL || */ (assigns && firstnode(assigns));
    makeprivate_error = 0;

    if (!OPT_ISSET(ops, 'P')) {
	fakelevel = 0;
	from_typeset = bin_typeset(nam, args, assigns, ops, func);
	fakelevel = ofake;
	return from_typeset;
    } else if (OPT_ISSET(ops, 'T')) {
	zwarn("bad option: -T");
	return 1;
    }

    if (locallevel == 0) {
	if (isset(WARNCREATEGLOBAL))
	    zwarnnam(nam, "invalid local scope, using globals");
	return bin_typeset("private", args, assigns, ops, func);
    }

    if (!(OPT_ISSET(ops, 'm') || OPT_ISSET(ops, '+')))
	ops->ind['g'] = 2;	/* force bin_typeset() to behave as "local" */
    if (OPT_ISSET(ops, 'p') || OPT_ISSET(ops, 'm') ||
	(!hasargs && OPT_ISSET(ops, '+'))) {
	return bin_typeset("private", args, assigns, ops, func);
    }

    queue_signals();
    fakelevel = locallevel;
    startparamscope();
    from_typeset = bin_typeset("private", args, assigns, ops, func);
    scanhashtable(paramtab, 0, 0, 0, makeprivate, 0);
    endparamscope();
    fakelevel = ofake;
    unqueue_signals();

    return makeprivate_error | from_typeset;
}

static void
setfn_error(Param pm)
{
    pm->node.flags |= PM_UNSET;
    zerr("%s: attempt to assign private in nested scope", pm->node.nam);
}

/*
 * How the GSU functions work:
 *
 * The getfn and setfn family compare to locallevel and then call through
 * to the original getfn or setfn.  This means you can't assign at a
 * deeper scope to any parameter declared private unless you first declare
 * it local again at the new scope.  Testing locallevel in getfn is most
 * likely unnecessary given the scopeprivate() wrapper installed below.
 *
 * The unsetfn family compare locallevel and restore the old GSU before
 * calling the original unsetfn.  This assures that if the old unsetfn
 * wants to use its getfn or setfn, they're unconditionally present.
 * The "explicit" flag indicates that "unset" was called, if zero the
 * parameter is going out of scope (see params.c).
 *
 */

/**/
static char *
pps_getfn(Param pm)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.s);
    GsuScalar gsu = (GsuScalar)(c->g);

    if (locallevel >= pm->level)
	return gsu->getfn(pm);
    else
	return (char *) hcalloc(1);
}

/**/
static void
pps_setfn(Param pm, char *x)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.s);
    GsuScalar gsu = (GsuScalar)(c->g);
    if (locallevel == pm->level)
	gsu->setfn(pm, x);
    else
	setfn_error(pm);
}

/**/
static void
pps_unsetfn(Param pm, int explicit)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.s);
    GsuScalar gsu = (GsuScalar)(c->g);
    pm->gsu.s = gsu;
    if (locallevel <= pm->level)
	gsu->unsetfn(pm, explicit);
    if (explicit)
	pm->gsu.s = (GsuScalar)c;
}

/**/
static zlong
ppi_getfn(Param pm)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.i);
    GsuInteger gsu = (GsuInteger)(c->g);
    if (locallevel >= pm->level)
	return gsu->getfn(pm);
    else
	return 0;
}

/**/
static void
ppi_setfn(Param pm, zlong x)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.i);
    GsuInteger gsu = (GsuInteger)(c->g);
    if (locallevel == pm->level)
	gsu->setfn(pm, x);
    else
	setfn_error(pm);
}

/**/
static void
ppi_unsetfn(Param pm, int explicit)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.i);
    GsuInteger gsu = (GsuInteger)(c->g);
    pm->gsu.i = gsu;
    if (locallevel <= pm->level)
	gsu->unsetfn(pm, explicit);
    if (explicit)
	pm->gsu.i = (GsuInteger)c;
}

/**/
static double
ppf_getfn(Param pm)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.f);
    GsuFloat gsu = (GsuFloat)(c->g);
    if (locallevel >= pm->level)
	return gsu->getfn(pm);
    else
	return 0;
}

/**/
static void
ppf_setfn(Param pm, double x)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.f);
    GsuFloat gsu = (GsuFloat)(c->g);
    if (locallevel == pm->level)
	gsu->setfn(pm, x);
    else
	setfn_error(pm);
}

/**/
static void
ppf_unsetfn(Param pm, int explicit)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.f);
    GsuFloat gsu = (GsuFloat)(c->g);
    pm->gsu.f = gsu;
    if (locallevel <= pm->level)
	gsu->unsetfn(pm, explicit);
    if (explicit)
	pm->gsu.f = (GsuFloat)c;
}

/**/
static char **
ppa_getfn(Param pm)
{
    static char *nullarray = NULL;
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.a);
    GsuArray gsu = (GsuArray)(c->g);
    if (locallevel >= pm->level)
	return gsu->getfn(pm);
    else
	return &nullarray;
}

/**/
static void
ppa_setfn(Param pm, char **x)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.a);
    GsuArray gsu = (GsuArray)(c->g);
    if (locallevel == pm->level)
	gsu->setfn(pm, x);
    else
	setfn_error(pm);
}

/**/
static void
ppa_unsetfn(Param pm, int explicit)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.a);
    GsuArray gsu = (GsuArray)(c->g);
    pm->gsu.a = gsu;
    if (locallevel <= pm->level)
	gsu->unsetfn(pm, explicit);
    if (explicit)
	pm->gsu.a = (GsuArray)c;
}

static HashTable emptytable;

/**/
static HashTable
pph_getfn(Param pm)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.h);
    GsuHash gsu = (GsuHash)(c->g);
    if (locallevel >= pm->level)
	return gsu->getfn(pm);
    else
	return emptytable;
}

/**/
static void
pph_setfn(Param pm, HashTable x)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.h);
    GsuHash gsu = (GsuHash)(c->g);
    if (locallevel == pm->level)
	gsu->setfn(pm, x);
    else
	setfn_error(pm);
}

/**/
static void
pph_unsetfn(Param pm, int explicit)
{
    struct gsu_closure *c = (struct gsu_closure *)(pm->gsu.h);
    GsuHash gsu = (GsuHash)(c->g);
    pm->gsu.h = gsu;
    if (locallevel <= pm->level)
	gsu->unsetfn(pm, explicit);
    if (explicit)
	pm->gsu.h = (GsuHash)c;
}

/*
 * How visibility works:
 *
 * Upon entering a new function scope, we find all the private parameters
 * at this locallevel.  Any that we find are marked PM_UNSET.  If they are
 * already unset, they are also marked as PM_NORESTORE.
 *
 * The same game is played with PM_READONLY and PM_RESTRICTED, so private
 * parameters are always read-only at deeper locallevel.  This is possible
 * because there is no builtin-command interface to set PM_RESTRICTED, so
 * only parameters "known to the shell" can otherwise be restricted.
 *
 * On exit from the scope, we find the same parameters again and reset
 * the PM_UNSET and PM_NORESTORE flags as appropriate.  We're guaraneed
 * by makeprivate() that PM_NORESTORE won't conflict with anything here.
 * Similarly, PM_READONLY and PM_RESTRICTED are also reset.
 *
 */

#define PM_WAS_UNSET PM_NORESTORE
#define PM_WAS_RONLY PM_RESTRICTED

static void
scopeprivate(HashNode hn, int onoff)
{
    Param pm = (Param)hn;
    if (pm->level == locallevel) {
	if (!is_private(pm))
	    return;
	if (onoff == PM_UNSET) {
	    if (pm->node.flags & PM_UNSET)
		pm->node.flags |= PM_WAS_UNSET;
	    else
		pm->node.flags |= PM_UNSET;
	    if (pm->node.flags & PM_READONLY)
		pm->node.flags |= PM_WAS_RONLY;
	    else
		pm->node.flags |= PM_READONLY;
	} else {
	    if (pm->node.flags & PM_WAS_UNSET)
		pm->node.flags |= PM_UNSET;	/* createparam() may frob */
	    else
		pm->node.flags &= ~PM_UNSET;
	    if (pm->node.flags & PM_WAS_RONLY)
		pm->node.flags |= PM_READONLY;	/* paranoia / symmetry */
	    else
		pm->node.flags &= ~PM_READONLY;
	    pm->node.flags &= ~(PM_WAS_UNSET|PM_WAS_RONLY);
	}
    }
}

static struct funcwrap wrapper[] = {
    WRAPDEF(wrap_private)
};

/**/
static int
wrap_private(Eprog prog, FuncWrap w, char *name)
{
    static int wraplevel = 0;

    if (wraplevel < locallevel /* && strcmp(name, "(anon)") != 0 */) {
	int owl = wraplevel;
	wraplevel = locallevel;
	scanhashtable(paramtab, 0, 0, 0, scopeprivate, PM_UNSET);
	runshfunc(prog, w, name);
	scanhashtable(paramtab, 0, 0, 0, scopeprivate, 0);
	wraplevel = owl;
	return 0;
    }
    return 1;
}

static GetNodeFunc getparamnode;

/**/
static HashNode
getprivatenode(HashTable ht, const char *nam)
{
    HashNode hn = getparamnode(ht, nam);
    Param pm = (Param) hn;

    while (!fakelevel && pm && locallevel > pm->level && is_private(pm)) {
	if (!(pm->node.flags & PM_UNSET)) {
	    /*
	     * private parameters are always marked PM_UNSET before we
	     * increment locallevel, so the only way we get here is
	     * when createparam() wants a new parameter that is not at
	     * the current locallevel and it has therefore cleared the
	     * PM_UNSET flag.
	     */
	    DPUTS(pm->old, "BUG: PM_UNSET cleared in wrong scope");
	    setfn_error(pm);
	    /*
	     * TODO: instead of throwing an error here, create a global
	     * parameter, insert as pm->old, handle WARN_CREATE_GLOBAL.
	     */
	}
	pm = pm->old;
    }
    return (HashNode)pm;
}

/**/
static HashNode
getprivatenode2(HashTable ht, const char *nam)
{
    /* getparamnode() would follow autoloads, we must not do that here */
    HashNode hn = gethashnode2(ht, nam);
    Param pm = (Param) hn;

    while (!fakelevel && pm && locallevel > pm->level && is_private(pm))
	pm = pm->old;
    return (HashNode)pm;
}

/**/
static void
printprivatenode(HashNode hn, int printflags)
{
    Param pm = (Param) hn;
    while (pm && (!fakelevel ||
		  (fakelevel > pm->level && (pm->node.flags & PM_UNSET))) &&
	   locallevel > pm->level && is_private(pm))
	pm = pm->old;
    /* Ideally, we'd print the word "private" here instead of "typeset"
     * when the parameter is in fact a private, but that would require
     * re-implementing the entirety of printparamnode(). */
    if (pm)
	printparamnode((HashNode)pm, printflags);
}

/*
 * Standard module configuration/linkage
 */

static struct builtin bintab[] = {
    /* Copied from BUILTIN("local"), "P" added */
    BUILTIN("private", BINF_PLUSOPTS | BINF_MAGICEQUALS | BINF_PSPECIAL | BINF_ASSIGN, (HandlerFunc)bin_private, 0, -1, 0, "AE:%F:%HL:%PR:%TUZ:%ahi:%lmprtux", "P")
};

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    NULL, 0,
    NULL, 0,
    NULL, 0,
    0
};

static struct builtin save_local;
static GetNodeFunc save_getnode2;
static ScanFunc save_printnode;
static struct reswd reswd_private = {{NULL, "private", 0}, TYPESET};

/**/
int
setup_(UNUSED(Module m))
{
    HashNode hn = builtintab->getnode(builtintab, "local");

    /* Horrible, horrible hack */
    getparamnode = realparamtab->getnode;
    save_getnode2 = realparamtab->getnode2;
    save_printnode = realparamtab->printnode;
    realparamtab->getnode = getprivatenode;
    realparamtab->getnode2 = getprivatenode2;
    realparamtab->printnode = printprivatenode;

    /* Even more horrible hack */
    save_local = *(Builtin)hn;
    ((Builtin)hn)->handlerfunc = bintab[0].handlerfunc;
    ((Builtin)hn)->optstr = bintab[0].optstr;

    reswdtab->addnode(reswdtab, reswd_private.node.nam, &reswd_private);

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
boot_(Module m)
{
    emptytable = newparamtable(1, "private");
    return addwrapper(m, wrapper);
}

/**/
int
cleanup_(Module m)
{
    HashNode hn = builtintab->getnode(builtintab, "local");
    *(Builtin)hn = save_local;

    removehashnode(reswdtab, "private");
    
    realparamtab->getnode = getparamnode;
    realparamtab->getnode2 = save_getnode2;
    realparamtab->printnode = save_printnode;

    deletewrapper(m, wrapper);
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    deletehashtable(emptytable);
    return 0;
}

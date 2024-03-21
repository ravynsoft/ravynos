/*
 * zle_params.c - ZLE special parameters
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

#include "zle.mdh"

#include "zle_params.pro"

/*
 * ZLE SPECIAL PARAMETERS:
 *
 * These special parameters are created, with a local scope, when
 * running user-defined widget functions.  Reading and writing them
 * reads and writes bits of ZLE state.  The parameters are:
 *
 * BUFFER   (scalar)   entire buffer contents
 * CURSOR   (integer)  cursor position; 0 <= $CURSOR <= $#BUFFER
 * LBUFFER  (scalar)   portion of buffer to the left of the cursor
 * RBUFFER  (scalar)   portion of buffer to the right of the cursor
 */

static const struct gsu_scalar buffer_gsu =
{ get_buffer, set_buffer, zleunsetfn };
static const struct gsu_scalar context_gsu =
{ get_context, nullstrsetfn, zleunsetfn };
static const struct gsu_scalar cutbuffer_gsu =
{ get_cutbuffer, set_cutbuffer, unset_cutbuffer };
static const struct gsu_scalar keymap_gsu =
{ get_keymap, nullstrsetfn, zleunsetfn };
static const struct gsu_scalar keys_gsu =
{ get_keys, nullstrsetfn, zleunsetfn };
static const struct gsu_scalar lastabortedsearch_gsu =
{ get_lasearch, nullstrsetfn, zleunsetfn };
static const struct gsu_scalar lastsearch_gsu =
{ get_lsearch, nullstrsetfn, zleunsetfn };
static const struct gsu_scalar lastwidget_gsu =
{ get_lwidget, nullstrsetfn, zleunsetfn };
static const struct gsu_scalar lbuffer_gsu =
{ get_lbuffer, set_lbuffer, zleunsetfn };
static const struct gsu_scalar postdisplay_gsu =
{ get_postdisplay, set_postdisplay, zleunsetfn };
static const struct gsu_scalar prebuffer_gsu =
{ get_prebuffer, nullstrsetfn, zleunsetfn };
static const struct gsu_scalar predisplay_gsu =
{ get_predisplay, set_predisplay, zleunsetfn };
static const struct gsu_scalar rbuffer_gsu =
{ get_rbuffer, set_rbuffer, zleunsetfn };
static const struct gsu_scalar widget_gsu =
{ get_widget, nullstrsetfn, zleunsetfn };
static const struct gsu_scalar widgetfunc_gsu =
{ get_widgetfunc, nullstrsetfn, zleunsetfn };
static const struct gsu_scalar widgetstyle_gsu =
{ get_widgetstyle, nullstrsetfn, zleunsetfn };
static const struct gsu_scalar zle_state_gsu =
{ get_zle_state, nullstrsetfn, zleunsetfn };

static const struct gsu_integer bufferlines_gsu =
{ get_bufferlines, NULL, zleunsetfn };
static const struct gsu_integer cursor_gsu =
{ get_cursor, set_cursor, zleunsetfn };
static const struct gsu_integer histno_gsu =
{ get_histno, set_histno, zleunsetfn };
static const struct gsu_integer keys_queued_count_gsu =
{ get_keys_queued_count, NULL, zleunsetfn };
static const struct gsu_integer mark_gsu =
{ get_mark, set_mark, zleunsetfn };
static const struct gsu_integer numeric_gsu =
{ get_numeric, set_numeric, unset_numeric };
static const struct gsu_integer pending_gsu =
{ get_pending, NULL, zleunsetfn };
static const struct gsu_integer recursive_gsu =
{ get_recursive, NULL, zleunsetfn };
static const struct gsu_integer region_active_gsu =
{ get_region_active, set_region_active, zleunsetfn };
static const struct gsu_integer undo_change_no_gsu =
{ get_undo_current_change, NULL, zleunsetfn };
static const struct gsu_integer undo_limit_no_gsu =
{ get_undo_limit_change, set_undo_limit_change, zleunsetfn };
static const struct gsu_integer yankstart_gsu =
{ get_yankstart, set_yankstart, zleunsetfn };
static const struct gsu_integer yankend_gsu =
{ get_yankend, set_yankend, zleunsetfn };
static const struct gsu_integer yankactive_gsu =
{ get_yankactive, NULL, zleunsetfn };
static const struct gsu_integer isearchmatchstart_gsu =
{ get_isearchmatchstart, NULL, zleunsetfn };
static const struct gsu_integer isearchmatchend_gsu =
{ get_isearchmatchend, NULL, zleunsetfn };
static const struct gsu_integer isearchmatchactive_gsu =
{ get_isearchmatchactive, NULL, zleunsetfn };
static const struct gsu_integer suffixstart_gsu =
{ get_suffixstart, NULL, zleunsetfn };
static const struct gsu_integer suffixend_gsu =
{ get_suffixend, NULL, zleunsetfn };
static const struct gsu_integer suffixactive_gsu =
{ get_suffixactive, NULL, zleunsetfn };

static const struct gsu_array killring_gsu =
{ get_killring, set_killring, unset_killring };

static const struct gsu_scalar register_gsu =
{ strgetfn, set_register, unset_register };
static const struct gsu_hash registers_gsu =
{ hashgetfn, set_registers, unset_registers };

/* implementation is in zle_refresh.c */
static const struct gsu_array region_highlight_gsu =
{ get_region_highlight, set_region_highlight, unset_region_highlight };

#define GSU(X) ( (GsuScalar)(void*)(&(X)) )
static struct zleparam {
    char *name;
    int type;
    GsuScalar gsu;
    void *data;
} zleparams[] = {
    { "BUFFER",  PM_SCALAR,  GSU(buffer_gsu), NULL },
    { "BUFFERLINES", PM_INTEGER | PM_READONLY, GSU(bufferlines_gsu),
        NULL },
    { "CONTEXT", PM_SCALAR | PM_READONLY, GSU(context_gsu),
	NULL },
    { "CURSOR",  PM_INTEGER, GSU(cursor_gsu),
	NULL },
    { "CUTBUFFER", PM_SCALAR, GSU(cutbuffer_gsu), NULL },
    { "HISTNO", PM_INTEGER, GSU(histno_gsu), NULL },
    { "KEYMAP", PM_SCALAR | PM_READONLY, GSU(keymap_gsu), NULL },
    { "KEYS", PM_SCALAR | PM_READONLY, GSU(keys_gsu), NULL },
    { "KEYS_QUEUED_COUNT", PM_INTEGER | PM_READONLY, GSU(keys_queued_count_gsu),
      NULL},
    { "killring", PM_ARRAY, GSU(killring_gsu), NULL },
    { "LASTABORTEDSEARCH", PM_SCALAR | PM_READONLY, GSU(lastabortedsearch_gsu),
      NULL },
    { "LASTSEARCH", PM_SCALAR | PM_READONLY, GSU(lastsearch_gsu), NULL },
    { "LASTWIDGET", PM_SCALAR | PM_READONLY, GSU(lastwidget_gsu), NULL },
    { "LBUFFER", PM_SCALAR,  GSU(lbuffer_gsu), NULL },
    { "MARK",  PM_INTEGER, GSU(mark_gsu), NULL },
    { "NUMERIC", PM_INTEGER | PM_UNSET, GSU(numeric_gsu), NULL },
    { "PENDING", PM_INTEGER | PM_READONLY, GSU(pending_gsu), NULL },
    { "POSTDISPLAY", PM_SCALAR, GSU(postdisplay_gsu), NULL },
    { "PREBUFFER",  PM_SCALAR | PM_READONLY,  GSU(prebuffer_gsu), NULL },
    { "PREDISPLAY", PM_SCALAR, GSU(predisplay_gsu), NULL },
    { "RBUFFER", PM_SCALAR,  GSU(rbuffer_gsu), NULL },
    { "REGION_ACTIVE", PM_INTEGER, GSU(region_active_gsu), NULL},
    { "region_highlight", PM_ARRAY, GSU(region_highlight_gsu), NULL },
    { "UNDO_CHANGE_NO", PM_INTEGER | PM_READONLY, GSU(undo_change_no_gsu),
      NULL },
    { "UNDO_LIMIT_NO", PM_INTEGER, GSU(undo_limit_no_gsu), NULL },
    { "WIDGET", PM_SCALAR | PM_READONLY, GSU(widget_gsu), NULL },
    { "WIDGETFUNC", PM_SCALAR | PM_READONLY, GSU(widgetfunc_gsu), NULL },
    { "WIDGETSTYLE", PM_SCALAR | PM_READONLY, GSU(widgetstyle_gsu), NULL },
    { "YANK_START", PM_INTEGER, GSU(yankstart_gsu), NULL },
    { "YANK_END", PM_INTEGER, GSU(yankend_gsu), NULL },
    { "YANK_ACTIVE", PM_INTEGER | PM_READONLY, GSU(yankactive_gsu), NULL },
    { "ISEARCHMATCH_START", PM_INTEGER, GSU(isearchmatchstart_gsu), NULL },
    { "ISEARCHMATCH_END", PM_INTEGER, GSU(isearchmatchend_gsu), NULL },
    { "ISEARCHMATCH_ACTIVE", PM_INTEGER | PM_READONLY, GSU(isearchmatchactive_gsu), NULL },
    { "SUFFIX_START", PM_INTEGER, GSU(suffixstart_gsu), NULL },
    { "SUFFIX_END", PM_INTEGER, GSU(suffixend_gsu), NULL },
    { "SUFFIX_ACTIVE", PM_INTEGER | PM_READONLY, GSU(suffixactive_gsu), NULL },
    { "ZLE_RECURSIVE", PM_INTEGER | PM_READONLY, GSU(recursive_gsu), NULL },
    { "ZLE_STATE", PM_SCALAR | PM_READONLY, GSU(zle_state_gsu), NULL },
    { NULL, 0, NULL, NULL }
};

/* ro means parameters are readonly, used from completion */

/**/
mod_export void
makezleparams(int ro)
{
    struct zleparam *zp;
    Param reg_param;

    for(zp = zleparams; zp->name; zp++) {
	Param pm = createparam(zp->name, (zp->type |PM_SPECIAL|PM_REMOVABLE|
					  PM_LOCAL|(ro ? PM_READONLY : 0)));
	if (!pm)
	    pm = (Param) paramtab->getnode(paramtab, zp->name);
	DPUTS(!pm, "param not set in makezleparams");

	pm->level = locallevel + 1;
	pm->u.data = zp->data;
	switch(PM_TYPE(zp->type)) {
	    case PM_SCALAR:
		pm->gsu.s = zp->gsu;
		break;
	    case PM_ARRAY:
		pm->gsu.a = (GsuArray)zp->gsu;
		break;
	    case PM_INTEGER:
		pm->gsu.i = (GsuInteger)zp->gsu;
		pm->base = 10;
		break;
	}
	if ((zp->type & PM_UNSET) && (zmod.flags & (MOD_MULT|MOD_TMULT)))
	    pm->node.flags &= ~PM_UNSET;
    }

    reg_param = createspecialhash("registers", get_registers, &scan_registers,
	    PM_LOCAL|PM_REMOVABLE);
    reg_param->gsu.h = &registers_gsu;
    reg_param->level = locallevel + 1;
}

/* Special unset function for ZLE special parameters: act like the standard *
 * unset function if this is a user-initiated unset, but nothing is done if *
 * the parameter is merely going out of scope (which it will do).           */

/**/
static void
zleunsetfn(Param pm, int exp)
{
    if(exp)
	stdunsetfn(pm, exp);
}

/**/
static void
set_buffer(UNUSED(Param pm), char *x)
{
    if(x) {
	setline(x, 0);
	zsfree(x);
    } else
	viinsbegin = zlecs = zlell = 0;
    fixsuffix();
    menucmp = 0;
}

/**/
static char *
get_buffer(UNUSED(Param pm))
{
    if (zlemetaline != 0)
	return dupstring(zlemetaline);
    return zlelineasstring(zleline, zlell, 0, NULL, NULL, 1);
}

/**/
static void
set_cursor(UNUSED(Param pm), zlong x)
{
    if(x < 0)
	zlecs = 0;
    else if(x > zlell)
	zlecs = zlell;
    else
	zlecs = x;
    fixsuffix();
    menucmp = 0;
}

/**/
static zlong
get_cursor(UNUSED(Param pm))
{
    if (zlemetaline != NULL) {
	/* A lot of work for one number, but still... */
	ZLE_STRING_T tmpline;
	int tmpcs, tmpll, tmpsz;
	char *tmpmetaline = ztrdup(zlemetaline);
	tmpline = stringaszleline(tmpmetaline, zlemetacs,
				  &tmpll, &tmpsz, &tmpcs);
	free(tmpmetaline);
	free(tmpline);
	return tmpcs;
    }
    return zlecs;
}

/**/
static void
set_mark(UNUSED(Param pm), zlong x)
{
    if (x < 0)
	mark = 0;
    else if (x > zlell)
	mark = zlell;
    else
	mark = x;
}

/**/
static zlong
get_mark(UNUSED(Param pm))
{
    return mark;
}

/**/
static void
set_region_active(UNUSED(Param pm), zlong x)
{
    region_active = (int)!!x;
}

/**/
static zlong
get_region_active(UNUSED(Param pm))
{
    return region_active;
}

/**/
static void
set_lbuffer(UNUSED(Param pm), char *x)
{
    ZLE_STRING_T y;
    int len;

    if (x && *x != ZWC('\0'))
	y = stringaszleline(x, 0, &len, NULL, NULL);
    else
	y = ZWS(""), len = 0;
    sizeline(zlell - zlecs + len);
    ZS_memmove(zleline + len, zleline + zlecs, zlell - zlecs);
    ZS_memcpy(zleline, y, len);
    zlell = zlell - zlecs + len;
    zlecs = len;
    zsfree(x);
    if (len)
	free(y);
    fixsuffix();
    menucmp = 0;
}

/**/
static char *
get_lbuffer(UNUSED(Param pm))
{
    if (zlemetaline != NULL)
	return dupstrpfx(zlemetaline, zlemetacs);
    return zlelineasstring(zleline, zlecs, 0, NULL, NULL, 1);
}

/**/
static void
set_rbuffer(UNUSED(Param pm), char *x)
{
    ZLE_STRING_T y;
    int len;

    if (x && *x != ZWC('\0'))
	y = stringaszleline(x, 0, &len, NULL, NULL);
    else
	y = ZWS(""), len = 0;
    sizeline(zlell = zlecs + len);
    ZS_memcpy(zleline + zlecs, y, len);
    zsfree(x);
    if (len)
	free(y);
    fixsuffix();
    menucmp = 0;
}

/**/
static char *
get_rbuffer(UNUSED(Param pm))
{
    if (zlemetaline != NULL)
	return dupstrpfx((char *)zlemetaline + zlemetacs,
			 zlemetall - zlemetacs);
    return zlelineasstring(zleline + zlecs, zlell - zlecs, 0, NULL, NULL, 1);
}

/**/
static char *
get_prebuffer(UNUSED(Param pm))
{
    /*
     * Use the editing current history line, not necessarily the
     * history line that's currently in the history mechanism
     * since our line may have been stacked.
     */
    if (zle_chline) {
	/* zle_chline was NULL terminated when pushed onto the stack */
	return dupstring(zle_chline);
    }
    if (chline) {
	/* hptr is valid */
	return dupstrpfx(chline, hptr - chline);
    }
    return dupstring("");
}

/**/
static char *
get_widget(UNUSED(Param pm))
{
    return bindk ? bindk->nam : "";
}

/**/
static char *
get_widgetfunc(UNUSED(Param pm))
{
    Widget widget = bindk->widget;
    int flags = widget->flags;

    if (flags & WIDGET_INT)
	return ".internal";	/* Don't see how this can ever be returned... */
    if (flags & WIDGET_NCOMP)
	return widget->u.comp.func;
    return widget->u.fnnam;
}

/**/
static char *
get_widgetstyle(UNUSED(Param pm))
{
    Widget widget = bindk->widget;
    int flags = widget->flags;

    if (flags & WIDGET_INT)
	return ".internal";	/* Don't see how this can ever be returned... */
    if (flags & WIDGET_NCOMP)
	return widget->u.comp.wid;
    return "";
}

/**/
static char *
get_lwidget(UNUSED(Param pm))
{
    return (lbindk ? lbindk->nam : "");
}

/**/
static char *
get_keymap(UNUSED(Param pm))
{
    return dupstring(curkeymapname);
}

/**/
static char *
get_keys(UNUSED(Param pm))
{
    return keybuf;
}

/**/
static zlong
get_keys_queued_count(UNUSED(Param pm))
{
    return kungetct;
}

/**/
static void
set_numeric(UNUSED(Param pm), zlong x)
{
    zmult = x;
    zmod.flags = MOD_MULT;
}

/**/
static zlong
get_numeric(UNUSED(Param pm))
{
    return zmult;
}

/**/
static void
unset_numeric(Param pm, int exp)
{
    if (exp) {
	stdunsetfn(pm, exp);
	zmod.flags = 0;
	zmult = 1;
    }
}

/**/
static void
set_histno(UNUSED(Param pm), zlong x)
{
    Histent he;

    if (!(he = quietgethist((int)x)))
	return;
    zle_setline(he);
}

/**/
static zlong
get_histno(UNUSED(Param pm))
{
    return histline;
}

/**/
static zlong
get_bufferlines(UNUSED(Param pm))
{
    return nlnct;
}

/**/
static zlong
get_pending(UNUSED(Param pm))
{
    return noquery(0);
}

/**/
static zlong
get_recursive(UNUSED(Param pm))
{
    return zle_recursive;
}

/**/
static zlong
get_yankstart(UNUSED(Param pm))
{
    return yankb;
}

/**/
static zlong
get_yankend(UNUSED(Param pm))
{
    return yanke;
}

/**/
static zlong
get_yankactive(UNUSED(Param pm))
{
    return !!(lastcmd & ZLE_YANK) + !!(lastcmd & ZLE_YANKAFTER);
}

/**/
static void
set_yankstart(UNUSED(Param pm), zlong i)
{
    yankb = i;
}

/**/
static void
set_yankend(UNUSED(Param pm), zlong i)
{
    yanke = i;
}

/**/
static zlong
get_isearchmatchstart(UNUSED(Param pm))
{
    return isearch_startpos;
}

/**/
static zlong
get_isearchmatchend(UNUSED(Param pm))
{
    return isearch_endpos;
}

/**/
static zlong
get_isearchmatchactive(UNUSED(Param pm))
{
    return isearch_active;
}

/**/
static zlong
get_suffixstart(UNUSED(Param pm))
{
    return zlecs - suffixlen;
}

/**/
static zlong
get_suffixend(UNUSED(Param pm))
{
    return zlecs;
}

/**/
static zlong
get_suffixactive(UNUSED(Param pm))
{
    return suffixlen;
}

/**/
static char *
get_cutbuffer(UNUSED(Param pm))
{
    if (cutbuf.buf)
	return zlelineasstring(cutbuf.buf, cutbuf.len, 0, NULL, NULL, 1);
    return "";
}


/**/
static void
set_cutbuffer(UNUSED(Param pm), char *x)
{
    if (cutbuf.buf)
	free(cutbuf.buf);
    cutbuf.flags = 0;
    if (x) {
	int n;
	cutbuf.buf = stringaszleline(x, 0, &n, NULL, NULL);
	cutbuf.len = n;
	free(x);
    } else {
	cutbuf.buf = NULL;
	cutbuf.len = 0;
    }
}

/**/
static void
unset_cutbuffer(Param pm, int exp)
{
    if (exp) {
	stdunsetfn(pm, exp);
	if (cutbuf.buf) {
	    free(cutbuf.buf);
	    cutbuf.buf = NULL;
	    cutbuf.len = 0;
	}
    }
}

/**/
static void
set_killring(UNUSED(Param pm), char **x)
{
    int kcnt;
    Cutbuffer kptr;
    char **p;

    if (kring) {
	for (kptr = kring, kcnt = 0; kcnt < kringsize; kcnt++, kptr++)
	    if (kptr->buf)
		free(kptr->buf);
	zfree(kring, kringsize * sizeof(struct cutbuffer));
	kring = NULL;
	kringsize = kringnum = 0;
    }
    if (x) {
	/*
	 * Insert the elements into the kill ring.
	 * Regardless of the old order, we number it with the current
	 * entry first.
	 *
	 * Be careful to add elements by looping backwards; this
	 * fits in with how we cycle the ring.
	 */
	int kpos = 0;
	kringsize = arrlen(x);
	if (kringsize != 0) {
	    kring = (Cutbuffer)zshcalloc(kringsize * sizeof(struct cutbuffer));
	    for (p = x; *p; p++) {
		int n, len = strlen(*p);
		kptr = kring + kpos;

		kptr->buf = stringaszleline(*p, 0, &n, NULL, NULL);
		kptr->len = n;

		zfree(*p, len+1);
		kpos = (kpos + kringsize -1 ) % kringsize;
	    }
	}
	free(x);
    }
}

/**/
static char **
get_killring(UNUSED(Param pm))
{
    /*
     * Return the kill ring with the most recently killed first.
     * Since the kill ring is no longer a fixed length, we return
     * all entries even if empty.
     */
    int kpos, kcnt;
    char **ret, **p;

    /* Supposed to work even if kring is NULL */
    if (!kring) {
	kringsize = KRINGCTDEF;
	kring = (Cutbuffer)zshcalloc(kringsize * sizeof(struct cutbuffer));
    }

    p = ret = (char **)zhalloc((kringsize+1) * sizeof(char *));

    for (kpos = kringnum, kcnt = 0; kcnt < kringsize; kcnt++) {
	Cutbuffer kptr = kring + kpos;
	if (kptr->buf)
	{
	    /* Allocate on heap. */
	    *p++ = zlelineasstring(kptr->buf, kptr->len, 0, NULL, NULL, 1);
	}
	else
	    *p++ = dupstring("");
	kpos = (kpos + kringsize - 1) % kringsize;
    }
    *p = NULL;

    return ret;
}

/**/
static void
unset_killring(Param pm, int exp)
{
    if (exp) {
	set_killring(pm, NULL);
	stdunsetfn(pm, exp);
    }
}

/**/
static void
set_register(Param pm, char *value)
{
    int n = 0;
    int offset = -1;
    Cutbuffer vbuf;

    if (!pm->node.nam || pm->node.nam[1])
	;
    else if (*pm->node.nam >= '0' && *pm->node.nam <= '9')
	offset = '0' - 26;
    else if (*pm->node.nam >= 'a' && *pm->node.nam <= 'z')
	offset = 'a';

    if (offset == -1) {
	zerr("invalid zle register: %s", pm->node.nam);
	return;
    }

    vbuf = &vibuf[*pm->node.nam - offset];
    if (*value)
	vbuf->buf = stringaszleline(value, 0, &n, NULL, NULL);
    vbuf->len = n;
}

/**/
static void
unset_register(Param pm, UNUSED(int exp))
{
    set_register(pm, "");
}

/**/
static void
scan_registers(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    int i;
    char ch;
    struct param pm;

    memset((void *)&pm, 0, sizeof(struct param));
    pm.node.flags = PM_SCALAR | PM_READONLY;
    pm.gsu.s = &nullsetscalar_gsu;

    for (i = 0, ch = 'a'; i < 36; i++) {
	pm.node.nam = zhalloc(2);
	*pm.node.nam = ch;
	pm.node.nam[1] = '\0';
	pm.u.str = zlelineasstring(vibuf[i].buf, vibuf[i].len, 0, NULL, NULL, 1);
	func(&pm.node, flags);
	if (ch++ == 'z')
	    ch = '0';
    }
}

/**/
static HashNode
get_registers(UNUSED(HashTable ht), const char *name)
{
    Param pm = (Param) hcalloc(sizeof(struct param));
    int vbuf = -1;
    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR;
    pm->gsu.s = &register_gsu;

    if (name[1])
       ;
    else if (*name >= '0' && *name <= '9')
	vbuf = *name - '0' + 26;
    else if (*name >= 'a' && *name <= 'z')
	vbuf = *name - 'a';

    if (vbuf == -1) {
	pm->u.str = dupstring("");
	pm->node.flags |= (PM_UNSET|PM_SPECIAL);
    } else
	pm->u.str = zlelineasstring(vibuf[vbuf].buf, vibuf[vbuf].len, 0, NULL, NULL, 1);

    return &pm->node;
}

/**/
static void
set_registers(Param pm, HashTable ht)
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

	    set_register(v.pm, getstrvalue(&v));
        }
    if (ht != pm->u.hash)
	deleteparamtable(ht);
}

/**/
static void
unset_registers(Param pm, int exp)
{
    stdunsetfn(pm, exp);
    deletehashtable(pm->u.hash);
    pm->u.hash = NULL;
}

static void
set_prepost(ZLE_STRING_T *textvar, int *lenvar, char *x)
{
    if (*lenvar) {
	free(*textvar);
	*textvar = NULL;
	*lenvar = 0;
    }
    if (x) {
	*textvar = stringaszleline(x, 0, lenvar, NULL, NULL);
	free(x);
    }
}

static char *
get_prepost(ZLE_STRING_T text, int len)
{
    return zlelineasstring(text, len, 0, NULL, NULL, 1);
}

/**/
static void
set_predisplay(UNUSED(Param pm), char *x)
{
    set_prepost(&predisplay, &predisplaylen, x);
}

/**/
static char *
get_predisplay(UNUSED(Param pm))
{
    return get_prepost(predisplay, predisplaylen);
}

/**/
static void
set_postdisplay(UNUSED(Param pm), char *x)
{
    set_prepost(&postdisplay, &postdisplaylen, x);
}

/**/
static char *
get_postdisplay(UNUSED(Param pm))
{
    return get_prepost(postdisplay, postdisplaylen);
}

/**/
void
free_prepostdisplay(void)
{
    if (predisplaylen)
	set_prepost(&predisplay, &predisplaylen, NULL);
    if (postdisplaylen)
	set_prepost(&postdisplay, &postdisplaylen, NULL);
}

/**/
static char *
get_lasearch(UNUSED(Param pm))
{
    if (previous_aborted_search)
	return previous_aborted_search;
    return "";
}

/**/
static char *
get_lsearch(UNUSED(Param pm))
{
    if (previous_search)
	return previous_search;
    return "";
}

/**/
static char *
get_context(UNUSED(Param pm))
{
    switch (zlecontext) {
    case ZLCON_LINE_CONT:
	return "cont";
	break;

    case ZLCON_SELECT:
	return "select";
	break;

    case ZLCON_VARED:
	return "vared";
	break;

    case ZLCON_LINE_START:
    default:
	return "start";
	break;
    }
}

/**/
static char *
get_zle_state(UNUSED(Param pm))
{
    char *zle_state = NULL, *ptr = NULL, **arr = NULL;
    int itp, istate, len = 0;

    /*
     * Substrings are sorted at the end, so the user can
     * easily match against this parameter:
     * if [[ $ZLE_STATE == *bar*foo*zonk* ]]; then ...; fi
     */
    for (itp = 0; itp < 2; itp++) {
	char *str;
	for (istate = 0; istate < 2; istate++) {
	    int slen;
	    switch (istate) {
	    case 0:
		if (insmode) {
		    str = "insert";
		} else {
		    str = "overwrite";
		}
		break;
	    case 1:
		if (hist_skip_flags & HIST_FOREIGN) {
		    str = "localhistory";
		} else {
		    str = "globalhistory";
		}
		break;

	    default:
		str = "";
	    }
	    slen = strlen(str);
	    if (itp == 0) {
		/* Accumulating length */
		if (istate)
		    len++;	/* for space */
		len += slen;
	    } else {
		/* Accumulating string */
		if (istate)
		    *ptr++ = ':';
		memcpy(ptr, str, slen);
		ptr += slen;
	    }
	}
	if (itp == 0) {
	    len++;		/* terminating NULL */
	    ptr = zle_state = (char *)zhalloc(len);
	} else {
	    *ptr = '\0';
	}
    }

    arr = colonsplit(zle_state, 0);
    strmetasort(arr, SORTIT_ANYOLDHOW, NULL);
    zle_state = zjoin(arr, ' ', 1);
    freearray(arr);

    return zle_state;
}

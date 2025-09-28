/*
 * computil.c - completion utilities
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

#include "computil.mdh"
#include "computil.pro"


/* Help for `_describe'. */

typedef struct cdset *Cdset;
typedef struct cdstr *Cdstr;
typedef struct cdrun *Cdrun;

struct cdstate {
    int showd;			/* != 0 if descriptions should be shown */
    char *sep;			/* the separator string */
    int slen;			/* its metafied length */
    int swidth;			/* its screen width */
    int maxmlen;                /* maximum length to allow for the matches */
    Cdset sets;			/* the sets of matches */
    int pre;                    /* longest prefix length (before description) */
    int premaxw;		/* ... and its screen width */
    int suf;                    /* longest suffix (description) */
    int maxg;                   /* size of largest group */
    int maxglen;                /* columns for matches of largest group */
    int groups;                 /* number of groups */
    int descs;                  /* number of non-group matches with desc */
    int gprew;                   /* prefix screen width for group display */
    Cdrun runs;                 /* runs to report to shell code */
};

struct cdstr {
    Cdstr next;                 /* the next one in this set */
    char *str;                  /* the string to display */
    char *desc;                 /* the description or NULL */
    char *match;                /* the match to add */
    char *sortstr;		/* unmetafied string used to sort matches */
    int len;                    /* length of str or match */
    int width;			/* ... and its screen width */
    Cdstr other;                /* next string with the same description */
    int kind;                   /* 0: not in a group, 1: the first, 2: other */
    Cdset set;                  /* the set this string is in */
    Cdstr run;                  /* next in this run */
};

struct cdrun {
    Cdrun next;                 /* ... */
    int type;                   /* see CRT_* below */
    Cdstr strs;                 /* strings in this run */
    int count;                  /* number of strings in this run */
};

#define CRT_SIMPLE 0
#define CRT_DESC   1
#define CRT_SPEC   2
#define CRT_DUMMY  3
#define CRT_EXPL   4

struct cdset {
    Cdset next;			/* guess what */
    char **opts;		/* the compadd-options */
    Cdstr strs;                 /* the strings/matches */
    int count;                  /* number of matches in this set */
    int desc;                   /* number of matches with description */
};

static struct cdstate cd_state;
static int cd_parsed = 0;

static void
freecdsets(Cdset p)
{
    Cdset n;
    Cdstr s, sn;
    Cdrun r, rn;

    for (; p; p = n) {
	n = p->next;
	if (p->opts)
	    freearray(p->opts);
        for (s = p->strs; s; s = sn) {
            sn = s->next;
	    zfree(s->sortstr, strlen(s->str) + 1);
            zsfree(s->str);
            zsfree(s->desc);
            if (s->match != s->str)
                zsfree(s->match);
            zfree(s, sizeof(*s));
        }
        for (r = cd_state.runs; r; r = rn) {
            rn = r->next;
            zfree(r, sizeof(*r));
        }
	zfree(p, sizeof(*p));
    }
}

/* Find matches with same descriptions and group them. */

static void
cd_group(int maxg)
{
    Cdset set1, set2;
    Cdstr str1, str2, *strp;
    int num, width;

    cd_state.groups = cd_state.descs = cd_state.maxglen = 0;
    cd_state.maxg = 0;

    for (set1 = cd_state.sets; set1; set1 = set1->next)
        for (str1 = set1->strs; str1; str1 = str1->next) {
            str1->kind = 0;
            str1->other = NULL;
        }

    for (set1 = cd_state.sets; set1; set1 = set1->next) {
        for (str1 = set1->strs; str1; str1 = str1->next) {
            if (!str1->desc || str1->kind != 0)
                continue;

            num = 1;
            width = str1->width + cd_state.swidth;
            if (width > cd_state.maxglen)
                cd_state.maxglen = width;
            strp = &(str1->other);

            for (set2 = set1; set2; set2 = set2->next) {
                for (str2 = (set2 == set1 ? str1->next : set2->strs);
                     str2; str2 = str2->next)
                    if (str2->desc && !strcmp(str1->desc, str2->desc)) {
                        width += CM_SPACE + str2->width;
                        if (width > cd_state.maxmlen || num == maxg)
                            break;
                        if (width > cd_state.maxglen)
                            cd_state.maxglen = width;
                        str1->kind = 1;
                        str2->kind = 2;
                        num++;
                        *strp = str2;
                        strp = &(str2->other);
                    }
                if (str2)
                    break;
            }
            *strp = NULL;

            if (num > 1)
                cd_state.groups++;
            else
                cd_state.descs++;

            if (num > cd_state.maxg)
                cd_state.maxg = num;
        }
    }
}

/* Calculate longest prefix and suffix and count the strings with
 * descriptions. */

static void
cd_calc(void)
{
    Cdset set;
    Cdstr str;
    int l;

    cd_state.pre = cd_state.suf = 0;

    for (set = cd_state.sets; set; set = set->next) {
        set->count = set->desc = 0;
        for (str = set->strs; str; str = str->next) {
            set->count++;
            if ((l = strlen(str->str)) > cd_state.pre)
                cd_state.pre = l;
            if ((l = ZMB_nicewidth(str->str)) > cd_state.premaxw)
                cd_state.premaxw = l;
            if (str->desc) {
                set->desc++;
                if ((l = strlen(str->desc)) > cd_state.suf) /* ### strlen() assumes no \n */
                    cd_state.suf = l;
            }
        }
    }
}

/* Return 1 if cd_state specifies unsorted groups, 0 otherwise. */
static int
cd_groups_want_sorting(void)
{
    Cdset set;
    char *const *i;

    for (set = cd_state.sets; set; set = set->next)
        for (i = set->opts; *i; i++) {
            if (!strncmp(*i, "-V", 2))
                return 0;
            else if (!strncmp(*i, "-J", 2))
                return 1;
        }

    /* Sorted by default */
    return 1;
}

static int
cd_sort(const void *a, const void *b)
{
    return zstrcmp((*((Cdstr *) a))->sortstr, (*((Cdstr *) b))->sortstr, 0);
}

static int
cd_prep(void)
{
    Cdrun run, *runp;
    Cdset set;
    Cdstr str, *strp;

    runp = &(cd_state.runs);

    if (cd_state.groups) {
        int preplines = cd_state.groups + cd_state.descs;
        VARARR(Cdstr, grps, preplines);
        VARARR(int, wids, cd_state.maxg);
        Cdstr gs, gp, gn, *gpp;
        int i, j, d;
        Cdrun expl;
        Cdstr *strp2;

        memset(wids, 0, cd_state.maxg * sizeof(int));
        strp = grps;

        for (set = cd_state.sets; set; set = set->next)
            for (str = set->strs; str; str = str->next) {
                if (str->kind != 1) {
                    if (!str->kind && str->desc) {
                        if (str->width > wids[0])
                            wids[0] = str->width;
                        str->other = NULL;
                        *strp++ = str;
                    }
                    continue;
                }
                gs = str;
                gs->kind = 2;
                gp = str->other;
                gs->other = NULL;
                for (; gp; gp = gn) {
                    gn = gp->other;
                    gp->other = NULL;
                    for (gpp = &gs; *gpp && (*gpp)->width > gp->width;
                         gpp = &((*gpp)->other));
                    gp->other = *gpp;
                    *gpp = gp;
                }
                for (gp = gs, i = 0; gp; gp = gp->other, i++)
                    if (gp->width > wids[i])
                        wids[i] = gp->width;

                *strp++ = gs;
            }

        cd_state.gprew = 0;
        for (i = 0; i < cd_state.maxg; i++) {
            cd_state.gprew += wids[i] + CM_SPACE;
	}

        if (cd_state.gprew > cd_state.maxmlen && cd_state.maxglen > 1)
            return 1;

	for (i = 0; i < preplines; i++) {
	    Cdstr s = grps[i];
	    int dummy;

	    s->sortstr = ztrdup(s->str);
	    unmetafy(s->sortstr, &dummy);
	}

        if (cd_groups_want_sorting())
            qsort(grps, preplines, sizeof(Cdstr), cd_sort);

        for (i = preplines, strp = grps; i > 1; i--, strp++) {
            strp2 = strp + 1;
            if (!strcmp((*strp)->desc, (*strp2)->desc))
                continue;
            for (j = i - 2, strp2++; j > 0; j--, strp2++)
                if (!strcmp((*strp)->desc, (*strp2)->desc)) {
                    Cdstr tmp = *strp2;

                    memmove(strp + 2, strp + 1,
                            (strp2 - strp - 1) * sizeof(Cdstr));

                    *++strp = tmp;
                    i--;
                }
        }
        expl =  (Cdrun) zalloc(sizeof(*run));
        expl->type = CRT_EXPL;
        expl->strs = grps[0];
        expl->count = preplines;

        for (i = preplines, strp = grps, strp2 = NULL; i; i--, strp++) {
            str = *strp;
            *strp = str->other;
            if (strp2)
                *strp2 = str;
            strp2 = &(str->run);

            *runp = run = (Cdrun) zalloc(sizeof(*run));
            runp = &(run->next);
            run->type = CRT_SPEC;
            run->strs = str;
            run->count = 1;
        }
        *strp2 = NULL;

        for (i = cd_state.maxg - 1; i; i--) {
            for (d = 0, j = preplines, strp = grps; j; j--, strp++) {
                if ((str = *strp)) {
                    if (d) {
                        *runp = run = (Cdrun) zalloc(sizeof(*run));
                        runp = &(run->next);
                        run->type = CRT_DUMMY;
                        run->strs = expl->strs;
                        run->count = d;
                        d = 0;
                    }
                    *runp = run = (Cdrun) zalloc(sizeof(*run));
                    runp = &(run->next);
                    run->type = CRT_SPEC;
                    run->strs = str;
                    run->strs->run = NULL;
                    run->count = 1;

                    *strp = str->other;
                } else
                    d++;
            }
            if (d) {
                *runp = run = (Cdrun) zalloc(sizeof(*run));
                runp = &(run->next);
                run->type = CRT_DUMMY;
                run->strs = expl->strs;
                run->count = d;
            }
        }
        *runp = expl;
        runp = &(expl->next);

        for (set = cd_state.sets; set; set = set->next) {
            for (i = 0, gs = NULL, gpp = &gs, str = set->strs;
                 str; str = str->next) {
                if (str->kind || str->desc)
                    continue;

                i++;
                *gpp = str;
                gpp = &(str->run);
            }
            *gpp = NULL;
            if (i) {
                *runp = run = (Cdrun) zalloc(sizeof(*run));
                runp = &(run->next);
                run->type = CRT_SIMPLE;
                run->strs = gs;
                run->count = i;
            }
        }
    } else if (cd_state.showd) {
        for (set = cd_state.sets; set; set = set->next) {
            if (set->desc) {
                *runp = run = (Cdrun) zalloc(sizeof(*run));
                runp = &(run->next);
                run->type = CRT_DESC;
                strp = &(run->strs);
                for (str = set->strs; str; str = str->next)
                    if (str->desc) {
                        *strp = str;
                        strp = &(str->run);
                    }
                *strp = NULL;
                run->count = set->desc;
            }
            if (set->desc != set->count) {
                *runp = run = (Cdrun) zalloc(sizeof(*run));
                runp = &(run->next);
                run->type = CRT_SIMPLE;
                strp = &(run->strs);
                for (str = set->strs; str; str = str->next)
                    if (!str->desc) {
                        *strp = str;
                        strp = &(str->run);
                    }
                *strp = NULL;
                run->count = set->count - set->desc;
            }
        }
    } else {
        for (set = cd_state.sets; set; set = set->next)
            if (set->count) {
                *runp = run = (Cdrun) zalloc(sizeof(*run));
                runp = &(run->next);
                run->type = CRT_SIMPLE;
                run->strs = set->strs;
                for (str = set->strs; str; str = str->next)
                    str->run = str->next;
                run->count = set->count;
            }
    }
    *runp = NULL;

    return 0;
}

/* Duplicate and concatenate two arrays.  Return the result. */

static char **
cd_arrcat(char **a, char **b)
{
    if (!b)
        return zarrdup(a);
    else {
        char **r = (char **) zalloc((arrlen(a) + arrlen(b) + 1) *
                                    sizeof(char *));
        char **p = r;

        for (; *a; a++)
            *p++ = ztrdup(*a);
        for (; *b; b++)
            *p++ = ztrdup(*b);

        *p = NULL;

        return r;
    }
}

/* Initialisation. Store and calculate the string and matches and so on.
 *
 * nam: argv[0] of the builtin
 * hide: ???
 * mlen: see max-matches-width style
 * sep: see list-seperator style
 * opts: options to (eventually) pass to compadd.
 *       Returned via 2nd return parameter of 'compdescribe -g'.
 * args: ??? (the positional arguments to 'compdescribe')
 * disp: 1 if descriptions should be shown, 0 otherwise
 */

static int
cd_init(char *nam, char *hide, char *mlen, char *sep,
        char **opts, char **args, int disp)
{
    Cdset *setp, set;
    Cdstr *strp, str;
    char **ap, *tmp;
    int grp = 0, itmp;

    if (cd_parsed) {
	zsfree(cd_state.sep);
	freecdsets(cd_state.sets);
	cd_parsed = 0;
    }
    setp = &(cd_state.sets);
    cd_state.sep = ztrdup(sep);
    cd_state.slen = strlen(sep);
    cd_state.swidth = ZMB_nicewidth(sep);
    cd_state.sets = NULL;
    cd_state.showd = disp;
    cd_state.maxg = cd_state.groups = cd_state.descs = 0;
    cd_state.maxmlen = atoi(mlen);
    cd_state.premaxw = 0;
    itmp = zterm_columns - cd_state.swidth - 4;
    if (cd_state.maxmlen > itmp)
        cd_state.maxmlen = itmp;
    if (cd_state.maxmlen < 4)
        cd_state.maxmlen = 4;
    if (*args && !strcmp(*args, "-g")) {
        args++;
        grp = 1;
    }
    while (*args) {
	*setp = set = (Cdset) zshcalloc(sizeof(*set));
	setp = &(set->next);
        *setp = NULL;
        set->opts = NULL;
        set->strs = NULL;

	if (!(ap = get_user_var(*args))) {
	    zwarnnam(nam, "invalid argument: %s", *args);
            zsfree(cd_state.sep);
            freecdsets(cd_state.sets);
	    return 1;
	}
        for (str = NULL, strp = &(set->strs); *ap; ap++) {
            *strp = str = (Cdstr) zalloc(sizeof(*str));
            strp = &(str->next);

            str->kind = 0;
            str->other = NULL;
            str->set = set;

	    /* Advance tmp to the first unescaped colon. */
	    for (tmp = *ap; *tmp && *tmp != ':'; tmp++)
                if (*tmp == '\\' && tmp[1])
                    tmp++;

            if (*tmp)
                str->desc = ztrdup(rembslash(tmp + 1));
            else
                str->desc = NULL;
            *tmp = '\0';
            str->str = str->match = ztrdup(rembslash(*ap));
            str->len = strlen(str->str);
            str->width = ZMB_nicewidth(str->str);
	    str->sortstr = NULL;
        }
        if (str)
            str->next = NULL;

	if (*++args && **args != '-') {
	    if (!(ap = get_user_var(*args))) {
		zwarnnam(nam, "invalid argument: %s", *args);
                zsfree(cd_state.sep);
                freecdsets(cd_state.sets);
		return 1;
	    }
            for (str = set->strs; str && *ap; str = str->next, ap++)
                str->match = ztrdup(*ap);

	    args++;
	}
        if (hide && *hide) {
            for (str = set->strs; str; str = str->next) {
                if (str->str == str->match)
                    str->str = ztrdup(str->str);
                if (hide[1] && str->str[0] == '-' && str->str[1] == '-')
                    memmove(str->str, str->str + 2, strlen(str->str) - 1);
                else if (str->str[0] == '-' || str->str[0] == '+')
                    memmove(str->str, str->str + 1, strlen(str->str));
            }
        }
	for (ap = args; *args &&
		 (args[0][0] != '-' || args[0][1] != '-' || args[0][2]);
	     args++);

	tmp = *args;
	*args = NULL;
	set->opts = cd_arrcat(ap, opts);
	if ((*args = tmp))
	    args++;
    }
    if (disp && grp) {
        int mg = zterm_columns;

        do {
            cd_group(mg);
            mg = cd_state.maxg - 1;
            cd_calc();
        } while (cd_prep());

    } else {
        cd_calc();
        cd_prep();
    }
    cd_parsed = 1;
    return 0;
}

/* Copy an array with one element in reserve (at the beginning). */

static char **
cd_arrdup(char **a)
{
    char **r = (char **) zalloc((arrlen(a) + 2) * sizeof(char *));
    char **p = r + 1;

    while (*a)
        *p++ = ztrdup(*a++);
    *p = NULL;

    return r;
}

/* Get the next set. */

static int
cd_get(char **params)
{
    Cdrun run;

    if ((run = cd_state.runs)) {
        Cdstr str;
        char **mats, **mp, **dpys, **dp, **opts, *csl = "";

        cd_state.runs = run->next;

        switch (run->type) {
        case CRT_SIMPLE:
            mats = mp = (char **) zalloc((run->count + 1) * sizeof(char *));
            dpys = dp = (char **) zalloc((run->count + 1) * sizeof(char *));

            for (str = run->strs; str; str = str->run) {
                *mp++ = ztrdup(str->match);
                *dp++ = ztrdup(str->str ? str->str : str->match);
            }
            *mp = *dp = NULL;
            opts = zarrdup(run->strs->set->opts);
            if (cd_state.groups) {
                /* We are building a columnised list with dummy matches
                 * but there are also matches without descriptions.
                 * Those end up in a different group, so make sure that
                 * group doesn't have an explanation. */

                for (mp = dp = opts; *mp; mp++) {
                    if (dp[0][0] == '-' && dp[0][1] == 'X') {
                        if (!dp[0][2] && dp[1])
                            mp++;
                    } else
                        *dp++ = *mp;
                }
                *dp = NULL;
            }
            break;

        case CRT_DESC:
            {
		/*
		 * The buffer size:
		 *     max prefix length (cd_state.pre) +
		 *     max padding (cd_state.premaxw generously :) +
		 *     separator length (cd_state.slen) +
		 *     inter matches gap (CM_SPACE) +
		 *     max description length (cd_state.suf) +
		 *     trailing \0
		 */
                VARARR(char, buf,
                       cd_state.pre + cd_state.suf +
		       cd_state.premaxw + cd_state.slen + 3);
                mats = mp = (char **) zalloc((run->count + 1) * sizeof(char *));
                dpys = dp = (char **) zalloc((run->count + 1) * sizeof(char *));

                for (str = run->strs; str; str = str->run) {
		    char *p = buf, *pp, *d;
		    int l, remw, w;

                    *mp++ = ztrdup(str->match);
		    strcpy(p, str->str);
		    p += str->len;
                    memset(p, ' ', (l = (cd_state.premaxw - str->width + CM_SPACE)));
		    p += l;

		    remw = zterm_columns - cd_state.premaxw -
			cd_state.swidth - 3;
		    while (remw < 0 && zterm_columns) {
			/* line wrapped, use remainder of the extra line */
			remw += zterm_columns;
		    }
		    if (cd_state.slen < remw) {
			strcpy(p, cd_state.sep);
			p += cd_state.slen;
			remw -= cd_state.slen;

			/*
			 * copy a character at once until no more screen
			 * width is available. Leave 1 character at the
			 * end of screen as safety margin
			 */
			d = str->desc;
			w = ZMB_nicewidth(d);
			if (w <= remw)
			    strcpy(p, d);
			else {
			    pp = p;
			    while (remw > 0 && *d) {
				l = MB_METACHARLEN(d);
				memcpy(pp, d, l);
				pp[l] = '\0';
				w = ZMB_nicewidth(pp);
				if (w > remw) {
				    *pp = '\0';
				    break;
				}

				pp += l;
				d += l;
				remw -= w;
			    }
			}
		    }

                    *dp++ = ztrdup(buf);
                }
                *mp = *dp = NULL;
                opts = cd_arrdup(run->strs->set->opts);
                opts[0] = ztrdup("-l");
                break;
            }

        case CRT_SPEC:
            mats = (char **) zalloc(2 * sizeof(char *));
            dpys = (char **) zalloc(2 * sizeof(char *));
            mats[0] = ztrdup(run->strs->match);
            dpys[0] = ztrdup(run->strs->str);
            mats[1] = dpys[1] = NULL;
            opts = cd_arrdup(run->strs->set->opts);

            /* Set -2V, possibly reusing the group name from an existing -J/-V
             * flag. */
            for (dp = opts + 1; *dp; dp++)
                if ((dp[0][0] == '-' && dp[0][1] == 'J') ||
		    (dp[0][0] == '-' && dp[0][1] == 'V'))
                    break;
            if (*dp) {
                char *s = tricat("-2V", "", dp[0] + 2);

                zsfree(*dp);
                *dp = s;

                memmove(opts, opts + 1,
                        (arrlen(opts + 1) + 1) * sizeof(char *));
                
            } else
                opts[0] = ztrdup("-2V-default-");
            csl = "packed";
            break;
  
        case CRT_DUMMY:
            {
                char buf[20];

                sprintf(buf, "-E%d", run->count);

                mats = (char **) zalloc(sizeof(char *));
                dpys = (char **) zalloc(sizeof(char *));
                mats[0] = dpys[0] = NULL;

                opts = cd_arrdup(run->strs->set->opts);
                opts[0] = ztrdup(buf);

                csl = "packed";
            }
            break;

	default: /* This silences the "might be used uninitialized" warnings */
        case CRT_EXPL:
            {
		/* add columns as safety margin */
                VARARR(char, dbuf, cd_state.suf + cd_state.slen +
		       zterm_columns);
                char buf[20], *p, *pp, *d;
                int i = run->count, remw, w, l;

                sprintf(buf, "-E%d", i);

                mats = (char **) zalloc(sizeof(char *));
                dpys = (char **) zalloc((i + 1) * sizeof(char *));

                for (dp = dpys, str = run->strs; str; str = str->run) {
                    if (str->run && !strcmp(str->desc, str->run->desc)) {
                        *dp++ = ztrdup("");
                        continue;
                    }

                    strcpy(dbuf, cd_state.sep);
		    remw = zterm_columns - cd_state.gprew -
			cd_state.swidth - CM_SPACE;
		    p = pp = dbuf + cd_state.slen;
		    d = str->desc;
		    w = ZMB_nicewidth(d);
		    if (w <= remw) {
			strcpy(p, d);
			remw -= w;
			pp += strlen(d);
		    } else
			while (remw > 0 && *d) {
			    l = MB_METACHARLEN(d);
			    memcpy(pp, d, l);
			    pp[l] = '\0';
			    w = ZMB_nicewidth(pp);
			    if (w > remw) {
				*pp = '\0';
				break;
			    }

			    pp += l;
			    d += l;
			    remw -= w;
			}

		    while (remw-- > 0)
			*pp++ = ' ';
		    *pp = '\0';

                    *dp++ = ztrdup(dbuf);
                }
                mats[0] = *dp = NULL;

                opts = cd_arrdup(run->strs->set->opts);
                opts[0] = ztrdup(buf);

                csl = "packed";
            }
            break;
        }
        setsparam(params[0], ztrdup(csl));
        setaparam(params[1], opts);
        setaparam(params[2], mats);
        setaparam(params[3], dpys);

        zfree(run, sizeof(*run));

        return 0;
    }
    return 1;
}

/**/
static int
bin_compdescribe(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    int n = arrlen(args);

    if (incompfunc != 1) {
	zwarnnam(nam, "can only be called from completion function");
	return 1;
    }
    if (!args[0][0] || !args[0][1] || args[0][2]) {
	zwarnnam(nam, "invalid argument: %s", args[0]);
	return 1;
    }
    switch (args[0][1]) {
    case 'i':
        if (n < 3) {
            zwarnnam(nam, "not enough arguments");

            return 1;
        }
	return cd_init(nam, args[1], args[2], "", NULL, args + 3, 0);
    case 'I':
        if (n < 6) {
            zwarnnam(nam, "not enough arguments");

            return 1;
        } else {
            char **opts;

            if (!(opts = getaparam(args[4]))) {
		zwarnnam(nam, "unknown parameter: %s", args[4]);
		return 1;
            }
            return cd_init(nam, args[1], args[2], args[3], opts, args + 5, 1);
        }
    case 'g':
	if (cd_parsed) {
	    if (n != 5) {
		zwarnnam(nam, (n < 5 ? "not enough arguments" :
			      "too many arguments"));
		return 1;
	    }
	    return cd_get(args + 1);
	} else {
	    zwarnnam(nam, "no parsed state");
	    return 1;
	}
    }
    zwarnnam(nam, "invalid option: %s", args[0]);
    return 1;
}

/* Help for `_arguments'. */

typedef struct cadef *Cadef;
typedef struct caopt *Caopt;
typedef struct caarg *Caarg;

/* Cache for a set of _arguments-definitions. */

struct cadef {
    Cadef next;			/* next in cache */
    Cadef snext;		/* next set */
    Caopt opts;			/* the options */
    int nopts, ndopts, nodopts;	/* number of options/direct/optional direct */
    Caarg args;			/* the normal arguments */
    Caarg rest;			/* the rest-argument */
    char **defs;		/* the original strings */
    int ndefs;			/* number of ... */
    int lastt;			/* last time this was used */
    Caopt *single;		/* array of single-letter options */
    char *match;		/* -M spec to use */
    int argsactive;		/* if normal arguments are still allowed */
				/* used while parsing a command line */
    char *set;			/* set name prefix (<name>-), shared */
    int flags;			/* see CDF_* below */
    char *nonarg;		/* pattern for non-args (-A argument) */
};

#define CDF_SEP 1		/* -S was specified: -- terminates options */

/* Description for an option. */

struct caopt {
    Caopt next;
    char *name;			/* option name */
    char *descr;		/* the description */
    char **xor;			/* if this, then not ... */
    int type;			/* type, CAO_* */
    Caarg args;			/* option arguments */
    int active;			/* still allowed on command line */
    int num;			/* it's the num'th option */
    char *gsname;		/* group or set name, shared */
    int not;			/* don't complete this option (`!...') */
};

#define CAO_NEXT    1		/* argument follows in next argument (`-opt:...') */
#define CAO_DIRECT  2		/* argument follows option directly (`-opt-:...') */
#define CAO_ODIRECT 3		/* argument may follow option directly (`-opt+:...') */
#define CAO_EQUAL   4		/* argument follows mandatory equals (`-opt=-:...') */
#define CAO_OEQUAL  5		/* argument follows optional equals (`-opt=:...') */

/* Description for an argument */

struct caarg {
    Caarg next;
    char *descr;		/* description */
    char **xor;			/* if this, then not ... */
    char *action;		/* what to do for it */
    int type;			/* CAA_* below */
    char *end;			/* end-pattern for ::<pat>:... */
    char *opt;			/* option name if for an option */
    int num;			/* it's the num'th argument */
    int min;			/* earliest possible arg pos, given optional args */
    int direct;			/* true if argument number was given explicitly */
    int active;			/* still allowed on command line */
    char *gsname;		/* group or set name, shared */
};

#define CAA_NORMAL 1
#define CAA_OPT    2
#define CAA_REST   3
#define CAA_RARGS  4
#define CAA_RREST  5

/* The cache of parsed descriptions. */

#define MAX_CACACHE 8
static Cadef cadef_cache[MAX_CACACHE];

/* Compare two arrays of strings for equality. */

static int
arrcmp(char **a, char **b)
{
    if (!a && !b)
	return 1;
    else if (!a || !b)
	return 0;
    else {
	while (*a && *b)
	    if (strcmp(*a++, *b++))
		return 0;

	return (!*a && !*b);
    }
}

/* Memory stuff. Obviously. */

static void
freecaargs(Caarg a)
{
    Caarg n;

    for (; a; a = n) {
	n = a->next;
	zsfree(a->descr);
	if (a->xor)
	    freearray(a->xor);
	zsfree(a->action);
	zsfree(a->end);
	zsfree(a->opt);
	zfree(a, sizeof(*a));
    }
}

static void
freecadef(Cadef d)
{
    Cadef s;
    Caopt p, n;

    while (d) {
	s = d->snext;
	zsfree(d->match);
	zsfree(d->set);
	if (d->defs)
	    freearray(d->defs);

	for (p = d->opts; p; p = n) {
	    n = p->next;
	    zsfree(p->name);
	    zsfree(p->descr);
	    if (p->xor)
		freearray(p->xor);
	    freecaargs(p->args);
	    zfree(p, sizeof(*p));
	}
	freecaargs(d->args);
	freecaargs(d->rest);
	zsfree(d->nonarg);
	if (d->single)
	    zfree(d->single, 188 * sizeof(Caopt));
	zfree(d, sizeof(*d));
	d = s;
    }
}

/* Remove backslashes before colons. */

static char *
rembslashcolon(char *s)
{
    char *p, *r;

    r = p = s = dupstring(s);

    while (*s) {
	if (s[0] != '\\' || s[1] != ':')
	    *p++ = *s;
	s++;
    }
    *p = '\0';

    return r;
}

/* Add backslashes before colons. */

static char *
bslashcolon(char *s)
{
    char *p, *r;

    r = p = zhalloc((2 * strlen(s)) + 1);

    while (*s) {
	if (*s == ':')
	    *p++ = '\\';
	*p++ = *s++;
    }
    *p = '\0';

    return r;
}

/* Get an index into the single array used in struct cadef
 * opt is the option letter and pre is either - or +
 * we only keep an array for the 94 ASCII characters from ! to ~ so
 * with - and + prefixes, the array is double that at 188 elements
 * returns -1 if opt is out-of-range
 */
static int
single_index(char pre, char opt)
{
    if (opt <= 0x20 || opt > 0x7e)
	return -1;

    return opt + (pre == '-' ? -0x21 : 94 - 0x21);
}

/* Parse an argument definition. */

static Caarg
parse_caarg(int mult, int type, int num, int opt, char *oname, char **def,
	    char *set)
{
    Caarg ret = (Caarg) zalloc(sizeof(*ret));
    char *p = *def, *d, sav;

    ret->next = NULL;
    ret->descr = ret->action = ret->end = NULL;
    ret->xor = NULL;
    ret->num = num;
    ret->min = num - opt;
    ret->type = type;
    ret->opt = ztrdup(oname);
    ret->direct = 0;
    ret->gsname = set;

    /* Get the description. */

    for (d = p; *p && *p != ':'; p++)
	if (*p == '\\' && p[1])
	    p++;
    sav = *p;
    *p = '\0';
    ret->descr = ztrdup(rembslashcolon(d));

    /* Get the action if there is one. */

    if (sav) {
	if (mult) {
	    for (d = ++p; *p && *p != ':'; p++)
		if (*p == '\\' && p[1])
		    p++;
	    sav = *p;
	    *p = '\0';
	    ret->action = ztrdup(rembslashcolon(d));
	    if (sav)
		*p = ':';
	} else
	    ret->action = ztrdup(rembslashcolon(p + 1));
    } else
	ret->action = ztrdup("");
    *def = p;

    return ret;
}

static Cadef
alloc_cadef(char **args, int single, char *match, char *nonarg, int flags)
{
    Cadef ret;

    ret = (Cadef) zalloc(sizeof(*ret));
    ret->next = ret->snext = NULL;
    ret->opts = NULL;
    ret->args = ret->rest = NULL;
    ret->nonarg = ztrdup(nonarg);
    if (args) {
	ret->defs = zarrdup(args);
	ret->ndefs = arrlen(args);
    } else {
	ret->defs = NULL;
	ret->ndefs = 0;
    }
    ret->nopts = 0;
    ret->ndopts = 0;
    ret->nodopts = 0;
    ret->lastt = time(0);
    ret->set = NULL;
    if (single) {
	ret->single = (Caopt *) zalloc(188 * sizeof(Caopt));
	memset(ret->single, 0, 188 * sizeof(Caopt));
    } else
	ret->single = NULL;
    ret->match = ztrdup(match);
    ret->flags = flags;

    return ret;
}

static void
set_cadef_opts(Cadef def)
{
    Caarg argp;
    int xnum;

    for (argp = def->args, xnum = 0; argp; argp = argp->next) {
	if (!argp->direct)
	    argp->min = argp->num - xnum;
	if (argp->type == CAA_OPT)
	    xnum++;
    }
}

/* Parse an array of definitions. */

static Cadef
parse_cadef(char *nam, char **args)
{
    Cadef all, ret;
    Caopt *optp;
    char **orig_args = args, *p, *q, *match = "r:|[_-]=* r:|=*", **xor, **sargs;
    char *adpre, *adsuf, *axor = NULL, *doset = NULL, **pendset = NULL, **curset = NULL;
    char *nonarg = NULL;
    int single = 0, anum = 1, xnum, flags = 0;
    int foreignset = 0, not = 0;

    /* First string is the auto-description definition. */

    for (p = args[0]; *p && (p[0] != '%' || p[1] != 'd'); p++);

    if (*p) {
	*p = '\0';
	adpre = dupstring(args[0]);
	*p = '%';
	adsuf = dupstring(p + 2);
    } else
	adpre = adsuf = NULL;

    /* Now get the -s, -A, -S and -M options. */

    args++;
    while ((p = *args) && *p == '-' && p[1]) {
	for (q = ++p; *q; q++)
	    if (*q == 'M' || *q == 'A') {
		q = "";
		break;
	    } else if (*q != 's' && *q != 'S')
		break;

	if (*q)
	    break;

	for (; *p; p++) {
	    if (*p == 's')
		single = 1;
	    else if (*p == 'S')
		flags |= CDF_SEP;
	    else if (*p == 'A') {
		if (p[1]) {
		    nonarg = p + 1;
		    p += strlen(p+1);
		} else if (args[1])
		    nonarg = *++args;
		else
		    break;
	    } else if (*p == 'M') {
		if (p[1]) {
		    match = p + 1;
		    p += strlen(p+1);
		} else if (args[1])
		    match = *++args;
		else
		    break;
	    }
	}
	if (*p)
	    break;

	args++;
    }
    if (*args && !strcmp(*args, ":"))
        args++;
    if (!*args)
	return NULL;

    if (nonarg)
	tokenize(nonarg = dupstring(nonarg));
    /* Looks good. Optimistically allocate the cadef structure. */

    all = ret = alloc_cadef(orig_args, single, match, nonarg, flags);
    optp = &(ret->opts);
    sargs = args;

    /* Get the definitions. */

    for (; *args || pendset; args++) {
	if (!*args) {
	    /* start new set */
	    args = sargs; /* go back and repeat parse of common options */
	    doset = NULL;
	    set_cadef_opts(ret);
	    ret = ret->snext = alloc_cadef(NULL, single, match, nonarg, flags);
	    optp = &(ret->opts);
	    anum = 1;
	    foreignset = 0;
	    curset = pendset;
	    pendset = 0;
        }
        if (args[0][0] == '-' && !args[0][1] && args[1]) {
	    if ((foreignset = curset && args != curset)) {
		if (!pendset && args > curset)
		    pendset = args; /* mark pointer to next pending set */
		++args;
	    } else {
		/* Carrying on: this is the current set */
		char *p = *++args;
		int l = strlen(p) - 1;

		if (*p == '(' && p[l] == ')') {
		    axor = p = dupstring(p + 1);
		    p[l - 1] = '\0';
		} else
		    axor = NULL;
		if (!*p) {
		    freecadef(all);
		    zwarnnam(nam, "empty set name");
		    return NULL;
		}
		ret->set = doset = tricat(p, "-", "");
		curset = args; /* needed for the first set */
	    }
	    continue;
	} else if (args[0][0] == '+' && !args[0][1] && args[1]) {
	    char *p;
	    int l;

	    foreignset = 0; /* group not in any set, don't want to skip it */
	    p = *++args;
	    l = strlen(p) - 1;
	    if (*p == '(' && p[l] == ')') {
		axor = p = dupstring(p + 1);
		p[l - 1] = '\0';
	    } else
		axor = NULL;
	    if (!*p) {
		freecadef(all);
		zwarnnam(nam, "empty group name");
		return NULL;
	    }
	    doset = tricat(p, "-", "");
	    continue;
	} else if (foreignset) /* skipping over a different set */
	    continue;
	p = dupstring(*args);
	xnum = 0;
	if ((not = (*p == '!')))
	    p++;
	if (*p == '(') {
	    /* There is a xor list, get it. */

	    LinkList list = newlinklist();
	    LinkNode node;
	    char **xp, sav;

	    while (*p && *p != ')') {
		for (p++; inblank(*p); p++);

		if (*p == ')')
		    break;
		for (q = p++; *p && *p != ')' && !inblank(*p); p++);

		if (!*p)
		    break;

		sav = *p;
		*p = '\0';
		addlinknode(list, dupstring(q));
		xnum++;
		*p = sav;
	    }
	    /* Oops, end-of-string. */
	    if (*p != ')') {
		freecadef(all);
		zwarnnam(nam, "invalid argument: %s", *args);
		return NULL;
	    }
	    if (doset && axor)
		xnum++;
	    xor = (char **) zalloc((xnum + 2) * sizeof(char *));
	    for (node = firstnode(list), xp = xor; node; incnode(node), xp++)
		*xp = ztrdup((char *) getdata(node));
	    if (doset && axor)
		*xp++ = ztrdup(axor);
	    xp[0] = xp[1] = NULL;

	    p++;
	} else if (doset && axor) {
	    xnum = 1;
	    xor = (char **) zalloc(3 * sizeof(char *));
	    xor[0] = ztrdup(axor);
	    xor[1] = xor[2] = NULL;
	} else
	    xor = NULL;

	if (*p == '-' || *p == '+' ||
	    (*p == '*' && (p[1] == '-' || p[1] == '+'))) {
	    /* It's an option. */
	    Caopt opt;
	    Caarg oargs = NULL;
	    int multi, otype = CAO_NEXT, again = 0;
	    char *name, *descr, c, *againp = NULL;

	    rec:

	    /* Allowed more than once? */
	    if ((multi = (*p == '*')))
		p++;

	    if (((p[0] == '-' && p[1] == '+') ||
		 (p[0] == '+' && p[1] == '-')) &&
		p[2] && p[2] != ':' && p[2] != '[' &&
		p[2] != '=' && p[2] != '-' && p[2] != '+') {
		/* It's a -+ or +- definition. We just execute the whole
		 * stuff twice for such things. */
		againp = dupstring(p);
		name = ++p;
		*p = (again ? '-' : '+');
		again++;
	    } else {
		name = p;
		/* If it's a long option skip over the first `-'. */
		if (p[0] == '-' && p[1] == '-')
		    p++;
	    }
	    if (!p[1]) {
		freecadef(all);
		zwarnnam(nam, "invalid argument: %s", *args);
		return NULL;
	    }

	    /* Skip over the name. */
	    for (p++; *p && *p != ':' && *p != '[' &&
		     ((*p != '-' && *p != '+') ||
		      (p[1] != ':' && p[1] != '[')) &&
		     (*p != '=' ||
		      (p[1] != ':' && p[1] != '[' && p[1] != '-')); p++)
		if (*p == '\\' && p[1])
		    p++;

	    /* The character after the option name specifies the type. */
	    c = *p;
	    *p = '\0';
	    if (c == '-') {
		otype = CAO_DIRECT;
		c = *++p;
	    } else if (c == '+') {
		otype = CAO_ODIRECT;
		c = *++p;
	    } else if (c == '=') {
		otype = CAO_OEQUAL;
		if ((c = *++p) == '-') {
		    otype = CAO_EQUAL;
		    c = *++p;
		}
	    }
	    /* Get the optional description, if any. */
	    if (c == '[') {
		for (descr = ++p; *p && *p != ']'; p++)
		    if (*p == '\\' && p[1])
			p++;

		if (!*p) {
		    freecadef(all);
		    zwarnnam(nam, "invalid option definition: %s", *args);
		    return NULL;
		}
		*p++ = '\0';
		c = *p;
	    } else
		descr = NULL;

	    if (c && c != ':') {
		freecadef(all);
		zwarnnam(nam, "invalid option definition: %s", *args);
		return NULL;
	    }
	    /* Add the option name to the xor list if not `*-...'. */
	    if (!multi) {
		if (!xor) {
		    xor = (char **) zalloc(2 * sizeof(char *));
		    xor[0] = xor[1] = NULL;
		}
                zsfree(xor[xnum]);
		xor[xnum] = ztrdup(rembslashcolon(name));
	    }
	    if (c == ':') {
		/* There's at least one argument. */

		Caarg *oargp = &oargs;
		int atype, rest, oanum = 1, onum = 0;
		char *end;

		/* Loop over the arguments. */

		while (c == ':') {
		    rest = 0;
		    end = NULL;

		    /* Get the argument type. */
		    if (*++p == ':') {
			atype = CAA_OPT;
			p++;
		    } else if (*p == '*') {
			if (*++p != ':') {
			    char sav;

			    for (end = p++; *p && *p != ':'; p++)
				if (*p == '\\' && p[1])
				    p++;
			    sav = *p;
			    *p = '\0';
			    end = dupstring(end);
			    tokenize(end);
			    *p = sav;
			}
			if (*p != ':') {
			    freecadef(all);
			    freecaargs(oargs);
			    zwarnnam(nam, "invalid option definition: %s",
				    *args);
			    return NULL;
			}
			if (*++p == ':') {
			    if (*++p == ':') {
				atype = CAA_RREST;
				p++;
			    } else
				atype = CAA_RARGS;
			} else
			    atype = CAA_REST;
			rest = 1;
		    } else
			atype = CAA_NORMAL;

		    /* And the definition. */

		    *oargp = parse_caarg(!rest, atype, oanum++, onum,
					 name, &p, doset);
		    if (atype == CAA_OPT)
			onum++;
		    if (end)
			(*oargp)->end = ztrdup(end);
		    oargp = &((*oargp)->next);
		    if (rest)
			break;
		    c = *p;
		}
	    }
	    /* Store the option definition. */

	    *optp = opt = (Caopt) zalloc(sizeof(*opt));
	    optp = &((*optp)->next);

	    opt->next = NULL;
	    opt->gsname = doset;
	    opt->name = ztrdup(rembslashcolon(name));
	    if (descr)
		opt->descr = ztrdup(descr);
	    else if (adpre && oargs && !oargs->next) {
		char *d;

		for (d = oargs->descr; *d; d++)
		    if (!iblank(*d))
			break;

		if (*d)
		    opt->descr = tricat(adpre, oargs->descr, adsuf);
		else
		    opt->descr = NULL;
	    } else
		opt->descr = NULL;
	    opt->xor = (again == 1 && xor ? zarrdup(xor) : xor);
	    opt->type = otype;
	    opt->args = oargs;
	    opt->num = ret->nopts++;
	    opt->not = not;

	    if (otype == CAO_DIRECT || otype == CAO_EQUAL)
		ret->ndopts++;
	    else if (otype == CAO_ODIRECT || otype == CAO_OEQUAL)
		ret->nodopts++;

	    /* If this is for single-letter option we also store a
	     * pointer for the definition in the array for fast lookup.
	     * But don't treat '--' as a single option called '-' */

	    if (single && name[1] && !name[2] && name[1] != '-') {
		int sidx = single_index(*name, name[1]);
		if (sidx >= 0) ret->single[sidx] = opt;
	    }

	    if (again == 1) {
		/* Do it all again for `*-...'. */
		p = againp;
		goto rec;
	    }
	} else if (*p == '*') {
	    /* It's a rest-argument definition. */

	    int type = CAA_REST;

	    if (not)
		continue;

	    if (*++p != ':') {
		freecadef(all);
		zwarnnam(nam, "invalid rest argument definition: %s", *args);
		return NULL;
	    }
	    if (ret->rest) {
		freecadef(all);
		zwarnnam(nam, "doubled rest argument definition: %s", *args);
		return NULL;
	    }
	    if (*++p == ':') {
		if (*++p == ':') {
		    type = CAA_RREST;
		    p++;
		} else
		    type = CAA_RARGS;
	    }
	    ret->rest = parse_caarg(0, type, -1, 0, NULL, &p, doset);
	    ret->rest->xor = xor;
	} else {
	    /* It's a normal argument definition. */

	    int type = CAA_NORMAL, direct;
	    Caarg arg, tmp, pre;

	    if (not)
		continue;

	    if ((direct = idigit(*p))) {
		/* Argument number is given. */
		int num = 0;

		while (*p && idigit(*p))
		    num = (num * 10) + (((int) *p++) - '0');

		anum = num + 1;
	    } else
		/* Default number. */
		anum++;

	    if (*p != ':') {
		freecadef(all);
		zwarnnam(nam, "invalid argument: %s", *args);
		if (xor)
		    free(xor);
		return NULL;
	    }
	    if (*++p == ':') {
		/* Optional argument. */
		type = CAA_OPT;
		p++;
	    }
	    arg = parse_caarg(0, type, anum - 1, 0, NULL, &p, doset);
	    arg->xor = xor;
	    arg->direct = direct;

	    /* Sort the new definition into the existing list. */

	    for (tmp = ret->args, pre = NULL;
		 tmp && tmp->num < anum - 1;
		 pre = tmp, tmp = tmp->next);

	    if (tmp && tmp->num == anum - 1) {
		freecadef(all);
		freecaargs(arg);
		zwarnnam(nam, "doubled argument definition: %s", *args);
		return NULL;
	    }
	    arg->next = tmp;
	    if (pre)
		pre->next = arg;
	    else
		ret->args = arg;
	}
    }
    set_cadef_opts(ret);

    return all;
}

/* Given an array of definitions, return the cadef for it. From the cache
 * are newly built. */

static Cadef
get_cadef(char *nam, char **args)
{
    Cadef *p, *min, new;
    int i, na = arrlen(args);

    for (i = MAX_CACACHE, p = cadef_cache, min = NULL; i && *p; p++, i--)
	if (*p && na == (*p)->ndefs && arrcmp(args, (*p)->defs)) {
	    (*p)->lastt = time(0);

	    return *p;
	} else if (!min || !*p || (*p)->lastt < (*min)->lastt)
	    min = p;
    if (i > 0)
	min = p;
    if ((new = parse_cadef(nam, args))) {
	freecadef(*min);
	*min = new;
    }
    return new;
}

/*
 * Get the option used in a word from the line, if any.
 *
 * "d" is a complete set of argument/option definitions to scan.
 * "line" is the word we are scanning.
 * "full" indicates that the option must match a full word; otherwise
 *   we look for "=" arguments or prefixes.
 * *"end" is set to point to the end of the option, in some cases
 *   leaving an option argument after it.
 */

static Caopt
ca_get_opt(Cadef d, char *line, int full, char **end)
{
    Caopt p;

    /* The full string may be an option. */

    for (p = d->opts; p; p = p->next)
	if (p->active && !strcmp(p->name, line)) {
	    if (end)
		*end = line + strlen(line);

	    return p;
	}

    if (!full) {
	/* The string from the line probably only begins with an option. */
	for (p = d->opts; p; p = p->next)
	    if (p->active && ((!p->args || p->type == CAO_NEXT) ?
			      !strcmp(p->name, line) : strpfx(p->name, line))) {
		int l = strlen(p->name);
		if ((p->type == CAO_OEQUAL || p->type == CAO_EQUAL) &&
		    line[l] && line[l] != '=')
		    continue;

		if (end) {
		    /* Return a pointer to the end of the option. */
		    if ((p->type == CAO_OEQUAL || p->type == CAO_EQUAL) &&
			line[l] == '=')
			l++;

		    *end = line + l;
		}
		return p;
	    }
    }
    return NULL;
}

/* Same as above, only for single-letter-style. */

static Caopt
ca_get_sopt(Cadef d, char *line, char **end, LinkList *lp)
{
    Caopt p, pp = NULL;
    char pre = *line++;
    LinkList l = NULL;
    int sidx;

    *lp = NULL;
    for (p = NULL; *line; line++) {
	if ((sidx = single_index(pre, *line)) != -1 &&
	    (p = d->single[sidx]) && p->active && p->args) {
	    if (p->type == CAO_NEXT) {
		if (!l)
		    *lp = l = newlinklist();
		addlinknode(l, p);
	    } else {
		if (end) {
		    line++;
		    if ((p->type == CAO_OEQUAL || p->type == CAO_EQUAL) &&
			*line == '=')
			line++;
		    *end = line;
		}
		pp = p;
		break;
	    }
	} else if (!p || (p && !p->active))
	    return NULL;
	pp = (p->name[0] == pre ? p : NULL);
	p = NULL;
    }
    if (pp && end)
	*end = line;
    return pp;
}

/* Search for an option in all sets except the current one.
 * Return true if found */

static int
ca_foreign_opt(Cadef curset, Cadef all, char *option)
{
    Cadef d;
    Caopt p;

    for (d = all; d; d = d->snext) {
	if (d == curset)
	    continue;

	for (p = d->opts; p; p = p->next) {
	    if (!strcmp(p->name, option))
		return 1;
	}
    }
    return 0;
}

/* Return the n'th argument definition. */

static Caarg
ca_get_arg(Cadef d, int n)
{
    if (d->argsactive) {
	Caarg a = d->args;

	while (a && (!a->active || n < a->min || n > a->num)) {
            if (!a->active)
                n++;
	    a = a->next;
        }
	if (a && a->min <= n && a->num >= n && a->active)
	    return a;

	return (d->rest && d->rest->active ? d->rest : NULL);
    }
    return NULL;
}

/* Mark options as inactive.
 *   d: option definitions for a set
 *   pass either:
 *     xor: a list of exclusions
 *     opts: if set, all options excluded leaving only nornal/rest arguments */

static void
ca_inactive(Cadef d, char **xor, int cur, int opts)
{
    if ((xor || opts) && cur <= compcurrent) {
	Caopt opt;
	char *x;
        /* current word could be a prefix of a longer one so only do
	 * exclusions for single-letter options (for option clumping) */
	int single = !opts && (cur == compcurrent);

	for (; (x = (opts ? "-" : *xor)); xor++) {
	    int excludeall = 0;
	    char *grp = NULL;
	    size_t grplen;
	    char *next, *sep = x;

	    while (*sep != '+' && *sep != '-' && *sep != ':' && *sep != '*' && !idigit(*sep)) {
		if (!(next = strchr(sep, '-')) || !*++next) {
		    /* exclusion is just the name of a set or group */
		    excludeall = 1; /* excluding options and args */
		    sep += strlen(sep);
		    /* A trailing '-' is included in the various gsname fields but is not
		     * there for this branch. This is why we add excludeall to grplen
		     * when checking for the null in a few places below */
		    break;
		}
		sep = next;
	    }
	    if (sep > x) { /* exclusion included a set or group name */
		grp = x;
		grplen = sep - grp;
		x = sep;
	    }

	    if (excludeall || (x[0] == ':' && !x[1])) {
		if (grp) {
		    Caarg a;

		    for (a = d->args; a; a = a->next)
			if (a->gsname && !strncmp(a->gsname, grp, grplen) &&
				!a->gsname[grplen + excludeall])
			    a->active = 0;
		    if (d->rest && d->rest->gsname &&
			    !strncmp(d->rest->gsname, grp, grplen) &&
			    !d->rest->gsname[grplen + excludeall])
			d->rest->active = 0;
		} else
		    d->argsactive = 0;
	    }

	    if (excludeall || (x[0] == '-' && !x[1])) {
		Caopt p;

		for (p = d->opts; p; p = p->next)
		    if ((!grp || (p->gsname && !strncmp(p->gsname, grp, grplen) &&
			    !p->gsname[grplen + excludeall])) &&
			    !(single && *p->name && p->name[1] && p->name[2]))
			p->active = 0;
	    }

	    if (excludeall || (x[0] == '*' && !x[1])) {
		if (d->rest && (!grp || (d->rest->gsname &&
			!strncmp(d->rest->gsname, grp, grplen) &&
			!d->rest->gsname[grplen + excludeall])))
		    d->rest->active = 0;
            }

	    if (!excludeall) {
		if (idigit(x[0])) {
		    int n = atoi(x);
		    Caarg a = d->args;

		    while (a && a->num < n)
			a = a->next;

		    if (a && a->num == n && (!grp || (a->gsname &&
			    !strncmp(a->gsname, grp, grplen))))
			a->active = 0;
		} else if ((opt = ca_get_opt(d, x, 1, NULL)) &&
			(!grp || (opt->gsname && !strncmp(opt->gsname, grp, grplen))) &&
			!(single && *opt->name && opt->name[1] && opt->name[2]))
		    opt->active = 0;
		if (opts)
		    break;
	    }
	}
    }
}

/* State when parsing a command line. */

typedef struct castate *Castate;

/* Encapsulates details from parsing the current line against a particular set,
 * Covers positions of options and normal arguments. Used as a linked list
 * with one state for each set. */

struct castate {
    Castate snext;	/* state for next set */
    Cadef d;		/* parsed _arguments specs for the set */
    int nopts;		/* number of specified options (size of oargs) */
    Caarg def;		/* definition for the current set */
    Caarg ddef;
    Caopt curopt;	/* option description corresponding to option found on the command-line */
    Caopt dopt;
    int opt;		/* the length of the option up to a maximum of 2 */
    int arg;		/* completing arguments to an option or rest args */
    int argbeg;         /* position of first rest argument (+1) */
    int optbeg;		/* first word after the last option to the left of the cursor:
			 * in effect the start of any arguments to the current option */
    int nargbeg;	/* same as optbeg but used during parse */
    int restbeg;	/* same as argbeg but used during parse */
    int curpos;		/* current word position */
    int argend;         /* total number of words */
    int inopt;		/* set to current word pos if word is a recognised option */
    int inarg;          /* in a normal argument */
    int nth;		/* number of current normal arg */
    int singles;	/* argument consists of clumped options */
    int oopt;
    int actopts;	/* count of active options */
    LinkList args;	/* list of non-option args used for populating $line */
    LinkList *oargs;	/* list of lists used for populating $opt_args */
};

static struct castate ca_laststate;
static int ca_parsed = 0, ca_alloced = 0;
static int ca_doff; /* no. of chars of ignored prefix (for clumped options or arg to an option) */

static void
freecastate(Castate s)
{
    int i;
    LinkList *p;

    freelinklist(s->args, freestr);
    for (i = s->nopts, p = s->oargs; i--; p++)
	if (*p)
	    freelinklist(*p, freestr);
    zfree(s->oargs, s->d->nopts * sizeof(LinkList));
}

/* Return a copy of an option's argument, ignoring possible quoting
 * in the option name. */

static char *
ca_opt_arg(Caopt opt, char *line)
{
    char *o = opt->name;

    while (1) {
        if (*o == '\\')
            o++;
        if (*line == '\\' || *line == '\'' || *line == '"')
            line++;
        if (!*o || *o != *line)
            break;
        o++;
        line++;
    }
    if (*line && (opt->type == CAO_EQUAL || opt->type == CAO_OEQUAL)) {
        if (*line == '\\')
            line++;
        if (*line == '=')
            line++;
    }
    return ztrdup(line);
}

/* Parse the command line for a particular argument set (d).
 * Returns 1 if the set should be skipped because it doesn't match
 * existing options on the line. */

static int
ca_parse_line(Cadef d, Cadef all, int multi, int first)
{
    Caarg adef, ddef;
    Caopt ptr, wasopt = NULL, dopt;
    struct castate state;
    char *line, *oline, *pe, **argxor = NULL;
    int cur, doff, argend, arglast;
    Patprog endpat = NULL, napat = NULL;
    LinkList sopts = NULL;
#if 0
    int ne;
#endif

    /* Free old state. */

    if (first && ca_alloced) {
	Castate s = &ca_laststate, ss;

	while (s) {
	    ss = s->snext;
	    freecastate(s);
	    s = ss;
	}
    }
    /* Mark everything as active. */

    for (ptr = d->opts; ptr; ptr = ptr->next)
	ptr->active = 1;
    d->argsactive = 1;
    if (d->rest)
	d->rest->active = 1;
    for (adef = d->args; adef; adef = adef->next)
	adef->active = 1;

    /* Default values for the state. */

    state.snext = NULL;
    state.d = d;
    state.nopts = d->nopts;
    state.def = state.ddef = NULL;
    state.curopt = state.dopt = NULL;
    state.argbeg = state.optbeg = state.nargbeg = state.restbeg = state.actopts =
	state.nth = state.inarg = state.opt = state.arg = 1;
    state.argend = argend = arrlen(compwords) - 1;
    state.inopt = state.singles = state.oopt = 0;
    state.curpos = compcurrent;
    state.args = znewlinklist();
    state.oargs = (LinkList *) zalloc(d->nopts * sizeof(LinkList));
    memset(state.oargs, 0, d->nopts * sizeof(LinkList));

    ca_alloced = 1;

    memcpy(&ca_laststate, &state, sizeof(state));

    if (!compwords[1]) {
	ca_laststate.opt = ca_laststate.arg = 0;

	goto end;
    }
    if (d->nonarg) /* argument to -A */
	napat = patcompile(d->nonarg, 0, NULL);

    /* Loop over the words from the line. */

    for (line = compwords[1], cur = 2, state.curopt = NULL, state.def = NULL;
	 line; line = compwords[cur++]) {
	ddef = adef = NULL;
	dopt = NULL;
	state.singles = arglast = 0;

        oline = line;
#if 0
        /*
	 * remove quotes.
	 * This is commented out:  it doesn't allow you to discriminate
	 * between command line values that can be expanded and those
	 * that can't, and in some cases this generates inconsistency;
	 * for example, ~/foo\[bar unqotes to ~/foo[bar which doesn't
	 * work either way---it's wrong if the ~ is quoted, and
	 * wrong if the [ isn't quoted..  So it's now up to the caller to
	 * unquote.
	 */
        line = dupstring(line);
        ne = noerrs;
        noerrs = 2;
        parse_subst_string(line);
        noerrs = ne;
#endif
        remnulargs(line);
        untokenize(line);

	if (argxor) {
	    ca_inactive(d, argxor, cur - 1, 0);
	    argxor = NULL;
	}
	if ((d->flags & CDF_SEP) && cur != compcurrent && state.actopts &&
		!strcmp(line, "--")) {
	    ca_inactive(d, NULL, cur, 1);
	    state.actopts = 0;
	    continue;
	}

	/* We've got a definition for an option/rest argument. For an option,
	 * this means that we're completing arguments to that option. */
	if (state.def) {
	    state.arg = 0;
	    if (state.curopt)
		zaddlinknode(state.oargs[state.curopt->num], ztrdup(oline));

	    if ((state.opt = (state.def->type == CAA_OPT)) && state.def->opt)
		state.oopt++;

	    if (state.def->type == CAA_REST || state.def->type == CAA_RARGS ||
		state.def->type == CAA_RREST) {
		if (state.def->end && pattry(endpat, line)) {
		    state.def = NULL;
		    state.curopt = NULL;
		    state.opt = state.arg = 1;
		    state.argend = ca_laststate.argend = cur - 1;
		    goto cont;
		}
	    } else if ((state.def = state.def->next)) {
		state.argbeg = cur;
		state.argend = argend;
	    } else if (sopts && nonempty(sopts)) {
		state.curopt = (Caopt) uremnode(sopts, firstnode(sopts));
		state.def = state.curopt->args;
		state.opt = 0;
		state.argbeg = state.optbeg = state.inopt = cur;
		state.argend = argend;
		doff = 0;
		state.singles = 1;
		if (!state.oargs[state.curopt->num])
		    state.oargs[state.curopt->num] = znewlinklist();
		goto cont;
	    } else {
		state.curopt = NULL;
		state.opt = 1;
	    }
	} else {
	    state.opt = state.arg = 1;
	    state.curopt = NULL;
	}
	if (state.opt)
	    state.opt = (line[0] ? (line[1] ? 2 : 1) : 0);

	pe = NULL;

	wasopt = NULL;

	/* See if it's an option. */

	if (state.opt == 2 && (*line == '-' || *line == '+') &&
	    (state.curopt = ca_get_opt(d, line, 0, &pe)) &&
	    (state.curopt->type == CAO_OEQUAL ?
	     (compwords[cur] || pe[-1] == '=') :
	     (state.curopt->type == CAO_EQUAL ?
	      (pe[-1] == '=' || !pe[0]) : 1))) {

	    if ((ddef = state.def = ((state.curopt->type != CAO_EQUAL ||
				      pe[-1] == '=') ?
				     state.curopt->args : NULL)))
		dopt = state.curopt;

	    doff = pe - line;
	    state.optbeg = state.argbeg = state.inopt = cur;
	    state.argend = argend;
	    state.singles = (d->single && (!pe || !*pe) &&
			     state.curopt->name[1] && !state.curopt->name[2] &&
			     /* Don't treat '--' as a single option called '-' */
			     state.curopt->name[1] != '-');

	    if (!state.oargs[state.curopt->num])
		state.oargs[state.curopt->num] = znewlinklist();

	    ca_inactive(d, state.curopt->xor, cur, 0);

	    /* Collect the argument strings. Maybe. */

	    if (state.def &&
		(state.curopt->type == CAO_DIRECT ||
		 state.curopt->type == CAO_EQUAL ||
		 (state.curopt->type == CAO_ODIRECT && pe[0]) ||
		 (state.curopt->type == CAO_OEQUAL &&
		  (pe[0] || pe[-1] == '=')))) {
		if (state.def->type != CAA_REST &&
		    state.def->type != CAA_RARGS &&
		    state.def->type != CAA_RREST)
		    state.def = state.def->next;

		zaddlinknode(state.oargs[state.curopt->num],
                             ca_opt_arg(state.curopt, oline));
	    }
	    if (state.def)
		state.opt = 0;
	    else {
		if (!d->single || (state.curopt->name[1] && state.curopt->name[2]))
		    wasopt = state.curopt;
		state.curopt = NULL;
	    }
	} else if (state.opt == 2 && d->single &&
		   (*line == '-' || *line == '+') &&
		   ((state.curopt = ca_get_sopt(d, line, &pe, &sopts)) ||
		    (cur != compcurrent && sopts && nonempty(sopts)))) {
	    /* Or maybe it's a single-letter option? */

	    char *p;
	    Caopt tmpopt;
	    int sidx;

	    if (cur != compcurrent && sopts && nonempty(sopts))
		state.curopt = (Caopt) uremnode(sopts, firstnode(sopts));

	    if (!state.oargs[state.curopt->num])
		state.oargs[state.curopt->num] = znewlinklist();

	    state.def = state.curopt->args;
	    ddef = (state.curopt->type == CAO_NEXT && cur == compcurrent ?
		    NULL : state.def);
	    dopt = state.curopt;
	    doff = pe - line;
	    state.optbeg = state.argbeg = state.inopt = cur;
	    state.argend = argend;
	    state.singles = (!pe || !*pe);

	    for (p = line + 1; p < pe; p++) {
		if ((sidx = single_index(*line, *p)) != -1 &&
		    (tmpopt = d->single[sidx])) {
		    if (!state.oargs[tmpopt->num])
			state.oargs[tmpopt->num] = znewlinklist();

		    ca_inactive(d, tmpopt->xor, cur, 0);
		}
	    }
	    if (state.def &&
		(state.curopt->type == CAO_DIRECT ||
		 state.curopt->type == CAO_EQUAL ||
		 (state.curopt->type == CAO_ODIRECT && pe[0]) ||
		 (state.curopt->type == CAO_OEQUAL &&
		  (pe[0] || pe[-1] == '=')))) {
		if (state.def->type != CAA_REST &&
		    state.def->type != CAA_RARGS &&
		    state.def->type != CAA_RREST)
		    state.def = state.def->next;

		zaddlinknode(state.oargs[state.curopt->num],
                             ca_opt_arg(state.curopt, line));
	    }
	    if (state.def)
		state.opt = 0;
	    else
		state.curopt = NULL;
	} else if (multi && (*line == '-' || *line == '+') && cur != compcurrent
		&& (ca_foreign_opt(d, all, line)))
	    return 1;
	else if (state.arg && cur <= compcurrent) {
	    /* Otherwise it's a normal argument. */

	    /* test pattern passed to -A. if it matches treat this as an unknown
	     * option and continue to the next word */
	    if (napat && cur < compcurrent && state.actopts) {
		if (pattry(napat, line))
		    continue;
		ca_inactive(d, NULL, cur + 1, 1);
		state.actopts = 0;
	    }

	    arglast = 1;
	    /* if this is the first normal arg after an option, may have been
	     * earlier normal arguments if they're intermixed with options */
	    if (state.inopt) {
		state.inopt = 0;
		state.nargbeg = cur - 1;
		state.argend = argend;
	    }
	    if (!d->args && !d->rest && *line && *line != '-' && *line != '+') {
		if (!multi && cur > compcurrent)
		    break;
		return 1;
	    }
	    if ((adef = state.def = ca_get_arg(d, state.nth)) &&
		(state.def->type == CAA_RREST ||
		 state.def->type == CAA_RARGS)) {

		/* Bart 2009/11/17:
		 * We've reached the "rest" definition.  If at this point
		 * we already found another definition that describes the
		 * current word, use that instead.  If not, prep for the
		 * "narrowing" of scope to only the remaining words.
		 *
		 * We can't test ca_laststate.def in the loop conditions
		 * at the top because this same loop also handles the
		 * ':*PATTERN:MESSAGE:ACTION' form for multiple arguments
		 * after an option, which may need to continue scanning.
		 * There might be an earlier point at which this test can
		 * be made but tracking it down is not worth the effort.
		 */
		if (ca_laststate.def)
		    break;

		state.opt = (cur == state.nargbeg + 1 &&
			     (!multi || !*line || 
			      *line == '-' || *line == '+'));
		state.optbeg = state.nargbeg;
		state.argbeg = cur - 1;
		state.argend = argend;

		for (; line; line = compwords[cur++])
		    zaddlinknode(state.args, ztrdup(line));

		memcpy(&ca_laststate, &state, sizeof(state));
		ca_laststate.ddef = NULL;
		ca_laststate.dopt = NULL;
		break;
	    }
	    zaddlinknode(state.args, ztrdup(line));
            if (adef)
                state.oopt = adef->num - state.nth;

	    if (state.def && cur != compcurrent)
		argxor = state.def->xor;

	    if (state.def && state.def->type != CAA_NORMAL &&
		state.def->type != CAA_OPT && state.inarg) {
		state.restbeg = cur;
		state.inarg = 0;
	    } else if (!state.def || state.def->type == CAA_NORMAL ||
		       state.def->type == CAA_OPT)
		state.inarg = 1;
	    state.nth++;
	    state.def = NULL;
	}
	/* Do the end-pattern test if needed. */

	if (state.def && state.curopt &&
	    (state.def->type == CAA_RREST || state.def->type == CAA_RARGS)) {
	    if (state.def->end)
		endpat = patcompile(state.def->end, 0, NULL);
	    else {
		LinkList l = state.oargs[state.curopt->num];

		if (cur < compcurrent)
		    memcpy(&ca_laststate, &state, sizeof(state));

		for (; line; line = compwords[cur++])
		    zaddlinknode(l, ztrdup(line));

		ca_laststate.ddef = NULL;
		ca_laststate.dopt = NULL;
		break;
	    }
	} else if (state.def && state.def->end)
	    endpat = patcompile(state.def->end, 0, NULL);

	/* Copy the state into the global one. */

    cont:

	if (cur + 1 == compcurrent) {
	    memcpy(&ca_laststate, &state, sizeof(state));
	    ca_laststate.ddef = NULL;
	    ca_laststate.dopt = NULL;
	} else if (cur == compcurrent && !ca_laststate.def) {
	    if ((ca_laststate.def = ddef)) {
		ca_laststate.singles = state.singles;
		if (state.curopt && state.curopt->type == CAO_NEXT) {
		    ca_laststate.ddef = ddef;
		    ca_laststate.dopt = dopt;
		    ca_laststate.def = NULL;
		    ca_laststate.opt = 1;
		    state.curopt->active = 1;
		} else {
		    ca_doff = doff;
		    ca_laststate.opt = 0;
		}
	    } else {
		ca_laststate.def = adef;
		ca_laststate.opt = (!arglast || !multi || !*line || 
				    *line == '-' || *line == '+');
		ca_laststate.ddef = NULL;
		ca_laststate.dopt = NULL;
		ca_laststate.optbeg = state.nargbeg;
		ca_laststate.argbeg = state.restbeg;
		ca_laststate.argend = state.argend;
		ca_laststate.singles = state.singles;
		ca_laststate.oopt = state.oopt;
		if (wasopt)
		    wasopt->active = 1;
	    }
	}
    }
 end:

    ca_laststate.actopts = 0;
    for (ptr = d->opts; ptr; ptr = ptr->next)
	if (ptr->active)
	    ca_laststate.actopts++;

    return 0;
}

/* Build a NUL-separated from a list.
 *
 * This is only used to populate values of $opt_args.
 */

static char *
ca_nullist(LinkList l)
{
    if (l) {
	char **array = zlinklist2array(l, 0 /* don't dup elements */);
	char *ret = zjoin(array, '\0', 0 /* permanent allocation */);
	free(array); /* the elements are owned by the list */
	return ret;
    } else
	return ztrdup("");
}

/* Build a colon-list from a list.
 *
 * This is only used to populate values of $opt_args.
 */

static char *
ca_colonlist(LinkList l)
{
    if (l) {
	LinkNode n;
	int len = 0;
	char *p, *ret, *q;

	/* Compute the length to be allocated. */
	for (n = firstnode(l); n; incnode(n)) {
	    len++;
	    for (p = (char *) getdata(n); *p; p++)
		len += (*p == ':' || *p == '\\') ? 2 : 1;
	}
	ret = q = (char *) zalloc(len);

	/* Join L into RET, joining with colons and escaping colons and
	 * backslashes. */
	for (n = firstnode(l); n;) {
	    for (p = (char *) getdata(n); *p; p++) {
		if (*p == ':' || *p == '\\')
		    *q++ = '\\';
		*q++ = *p;
	    }
	    incnode(n);
	    if (n)
		*q++ = ':';
	}
	*q = '\0';

	return ret;
    } else
	return ztrdup("");
}

/*
 * This function adds the current set of descriptions, actions,
 * and subcontext descriptions to the given linked list for passing
 * up in comparguments -D and comparguments -L.  opt is the
 * option string (may be NULL if this isn't an option argument) and arg the
 * argument structure (either an option argument or a normal argument
 * as determined by arg->type).
 */

static void
ca_set_data(LinkList descr, LinkList act, LinkList subc,
	    char *opt, Caarg arg, Caopt optdef, int single)
{
    LinkNode dnode, anode;
    char nbuf[40], *buf;
    int restr = 0, onum, miss = 0, rest, oopt = 1, lopt = 0, addopt;

 rec:

    addopt = (opt ? 0 : ca_laststate.oopt);

    for (; arg && (opt || (arg->num < 0 ||
			   (arg->min <= ca_laststate.nth + addopt &&
			    arg->num >= ca_laststate.nth)));) {
	lopt = (arg->type == CAA_OPT);
	if (!opt && !lopt && oopt > 0)
	    oopt = 0;

	for (dnode = firstnode(descr), anode = firstnode(act);
	     dnode; incnode(dnode), incnode(anode))
	    if (!strcmp((char *) getdata(dnode), arg->descr) &&
		!strcmp((char *) getdata(anode), arg->action))
		break;

	/* with an ignored prefix, we're not completing any normal arguments */
	if (single && !arg->opt)
	    return;

	if (!dnode) {
	    addlinknode(descr, arg->descr);
	    addlinknode(act, arg->action);

	    if (!restr) {

		if ((restr = (arg->type == CAA_RARGS)))
		    restrict_range(ca_laststate.optbeg, ca_laststate.argend);
		else if ((restr = (arg->type == CAA_RREST)))
		    restrict_range(ca_laststate.argbeg, ca_laststate.argend);
	    }
	    if (arg->opt) {
		buf = (char *) zhalloc((arg->gsname ? strlen(arg->gsname) : 0) +
				       strlen(arg->opt) + 40);
		if (arg->num > 0 && arg->type < CAA_REST)
		    sprintf(buf, "%soption%s-%d",
			    (arg->gsname ? arg->gsname : ""), arg->opt, arg->num);
		else
		    sprintf(buf, "%soption%s-rest",
			    (arg->gsname ? arg->gsname : ""), arg->opt);
	    } else if (arg->num > 0) {
		sprintf(nbuf, "argument-%d", arg->num);
		buf = (arg->gsname ? dyncat(arg->gsname, nbuf) : dupstring(nbuf));
	    } else
		buf = (arg->gsname ? dyncat(arg->gsname, "argument-rest") :
		       dupstring("argument-rest"));

	    addlinknode(subc, buf);
	}
	/*
	 * If this is an argument to an option, and the option definition says
	 * the argument to the option is required and in the following
	 * (i.e. this) word, then it must match what we've just told it to
	 * match---don't try to match normal arguments.
	 *
	 * This test may be too stringent for what we need, or it
	 * may be too loose; I've simply tweaked it until it gets
	 * the case above right.
	 */
	if (arg->type == CAA_NORMAL &&
	    opt && optdef &&
	    (optdef->type == CAO_NEXT || optdef->type == CAO_ODIRECT ||
	     optdef->type == CAO_OEQUAL))
	    return;

	if (single)
	    break;

	if (!opt) {
	    if (arg->num >= 0 && !arg->next && miss)
		arg = (ca_laststate.d->rest && ca_laststate.d->rest->active ?
		       ca_laststate.d->rest : NULL);
	    else {
		onum = arg->num;
		rest = (onum != arg->min && onum == ca_laststate.nth);
		if ((arg = arg->next)) {
		    if (arg->num != onum + 1)
			miss = 1;
		} else if (rest || (oopt > 0 && !opt)) {
		    arg = (ca_laststate.d->rest && ca_laststate.d->rest->active ?
			   ca_laststate.d->rest : NULL);
		    oopt = -1;
		}
	    }
	} else {
	    if (!lopt)
		break;
	    arg = arg->next;
	}
    }
    if (!single && opt && (lopt || ca_laststate.oopt)) {
	opt = NULL;
	arg = ca_get_arg(ca_laststate.d, ca_laststate.nth);
	goto rec;
    }
    if (!opt && oopt > 0) {
	oopt = -1;
	arg = (ca_laststate.d->rest && ca_laststate.d->rest->active ?
	       ca_laststate.d->rest : NULL);

	goto rec;
    }
}

static int
bin_comparguments(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    int min, max, n;
    Castate lstate = &ca_laststate;

    if (incompfunc != 1) {
	zwarnnam(nam, "can only be called from completion function");
	return 1;
    }
    if (args[0][0] != '-' || !args[0][1] || args[0][2]) {
	zwarnnam(nam, "invalid argument: %s", args[0]);
	return 1;
    }
    if (args[0][1] != 'i' && args[0][1] != 'I' && !ca_parsed) {
	zwarnnam(nam, "no parsed state");
	return 1;
    }
    switch (args[0][1]) {
    case 'i': min = 2; max = -1; break;
    case 'D': min = 3; max =  3; break;
    case 'O': min = 4; max =  4; break;
    case 'L': min = 3; max =  4; break;
    case 's': min = 1; max =  1; break;
    case 'M': min = 1; max =  1; break;
    case 'a': min = 0; max =  0; break;
    case 'W': min = 3; max =  3; break;
    case 'n': min = 1; max =  1; break;
    default:
	zwarnnam(nam, "invalid option: %s", args[0]);
	return 1;
    }
    n = arrlen(args) - 1;
    if (n < min) {
	zwarnnam(nam, "not enough arguments");
	return 1;
    } else if (max >= 0 && n > max) {
	zwarnnam(nam, "too many arguments");
	return 1;
    }
    switch (args[0][1]) {
    case 'i':
        /* This initialises the internal data structures. Arguments are the
         * auto-description string, the optional -s, -S, -A and -M options
         * given to _arguments and the specs. */
	if (compcurrent > 1 && compwords[0]) {
	    Cadef def, all;
	    int cap = ca_parsed, multi, first = 1, use, ret = 0;
	    Castate states = NULL, sp;

	    ca_parsed = 0;

	    if (!(def = all = get_cadef(nam, args + 1)))
		return 1;

	    multi = !!def->snext; /* if we have sets */
	    ca_parsed = cap;
	    ca_doff = 0;

	    while (def) { /* for each set */
		use = !ca_parse_line(def, all, multi, first);
		def = def->snext;
		if (use && def) {
		    /* entry needed so save it into list */
		    sp = (Castate) zalloc(sizeof(*sp));
		    memcpy(sp, &ca_laststate, sizeof(*sp));
		    sp->snext = states;
		    states = sp;
		} else if (!use && !def) {
		    /* final entry not needed */
		    if (states) {
			freecastate(&ca_laststate);
			memcpy(&ca_laststate, states, sizeof(*sp));
			sp = states->snext;
			zfree(states, sizeof(*states));
			states = sp;
		    } else
			ret = 1;
		}
		first = 0;
	    }
	    ca_parsed = 1;
	    ca_laststate.snext = states;

	    return ret;
	}
	return 1;

    case 'D':
        /* This returns the descriptions, actions and sub-contexts for the
         * things _arguments has to execute at this place on the line (the
         * sub-contexts are used as tags).
         * The return value is particularly important here, it says if 
         * there are arguments to complete at all. */
	{
	    LinkList descr, act, subc;
	    Caarg arg;
	    int ret = 1;

	    descr = newlinklist();
	    act = newlinklist();
	    subc = newlinklist();

	    ignore_prefix(ca_doff);
	    while (lstate) {
		arg = lstate->def;

		if (arg) {
		    ret = 0;
		    ca_set_data(descr, act, subc, arg->opt, arg,
				lstate->curopt, (ca_doff > 0));
		}
		lstate = lstate->snext;
	    }
	    if (!ret) {
		set_list_array(args[1], descr);
		set_list_array(args[2], act);
		set_list_array(args[3], subc);
	    }
	    return ret;
	}
    case 'O':
        /* This returns the descriptions for the options in the arrays whose
         * names are given as arguments.  The descriptions are strings in a
         * form usable by _describe.  The return value says if there are any
         * options to be completed. */
	{
	    LinkList next = newlinklist();
	    LinkList direct = newlinklist();
	    LinkList odirect = newlinklist();
	    LinkList equal = newlinklist(), l;
            LinkNode node;
	    Caopt p;
	    char *str;
	    int ret = 1;

	    for (; lstate; lstate = lstate->snext) {
		if (lstate->actopts &&
		    (lstate->opt || (ca_doff && lstate->def) ||
		     (lstate->def && lstate->def->opt &&
		      (lstate->def->type == CAA_OPT ||
		       (lstate->def->type >= CAA_RARGS &&
			lstate->def->num < 0)))) &&
		    (!lstate->def || lstate->def->type < CAA_RARGS ||
		     (lstate->def->type == CAA_RARGS ?
		      (lstate->curpos == lstate->argbeg + 1) :
		      (compcurrent == 1)))) {
		    ret = 0;
		    for (p = lstate->d->opts; p; p = p->next) {
			if (p->active && !p->not) {
			    switch (p->type) {
			    case CAO_NEXT:    l = next;    break;
			    case CAO_DIRECT:  l = direct;  break;
			    case CAO_ODIRECT: l = odirect; break;
			    default:          l = equal;   break;
			    }
			    if (p->descr) {
				char *n = bslashcolon(p->name);
				int len = strlen(n) + strlen(p->descr) + 2;

				str = (char *) zhalloc(len);
				strcpy(str, n);
				strcat(str, ":");
				strcat(str, p->descr);
			    } else
				str = bslashcolon(p->name);

                            for (node = firstnode(l); node; incnode(node))
                                if (!strcmp(str, (char *) getdata(node)))
                                    break;

                            if (!node)
                                addlinknode(l, str);
			}
		    }
		}
	    }
	    if (!ret) {
		set_list_array(args[1], next);
		set_list_array(args[2], direct);
		set_list_array(args[3], odirect);
		set_list_array(args[4], equal);

		return 0;
	    }
	    return (ca_laststate.singles ? 2 : 1);
	}
    case 'L':
        /* This tests if the beginning of the current word matches an option.
         * It is for cases like `./configure --pre=/<TAB>' which should
         * complete to `--prefix=/...'.  The options name isn't fully typed
         * and _arguments finds out that there is no option `--pre' and that
         * it should complete some argument to an option.  It then uses -L
         * to find the option the argument is for. */
	{
	    LinkList descr, act, subc;
	    Caopt opt;
	    int ret = 1;

	    descr = newlinklist();
	    act = newlinklist();
	    subc = newlinklist();

	    while (lstate) {
		opt = ca_get_opt(lstate->d, args[1], 1, NULL);

		if (opt && opt->args) {
		    ret = 0;
		    ca_set_data(descr, act, subc, opt->name, opt->args, opt, 1);
		}
		lstate = lstate->snext;
	    }
	    if (!ret) {
		set_list_array(args[2], descr);
		set_list_array(args[3], act);
		set_list_array(args[4], subc);
	    }
	    return ret;
	}
    case 's':
        /* This returns zero if we are completing single letter options.
         * It also uses its argument as the name of a parameter and sets
         * that to a string describing the argument behaviour of the last
         * option in the current word so that we can get the auto-suffix
         * right. */
	for (; lstate; lstate = lstate->snext)
	    if (lstate->d->single && lstate->singles &&
		lstate->actopts
#if 0
                /* let's try without, for the -W option of _arguments */
                && lstate->opt
#endif
                ) {
		setsparam(args[1],
			  ztrdup((lstate->ddef && lstate->dopt) ?
				 (lstate->dopt->type == CAO_DIRECT ?
				  "direct" :
				  ((lstate->dopt->type == CAO_OEQUAL ||
				    lstate->dopt->type == CAO_EQUAL) ?
				   "equal" : "next")) : ""));
		return 0;
	    }
	return 1;
    case 'M':
        /* This returns the match specs defined for the set of specs we are
         * using.  Returned, as usual in a parameter whose name is given as
         * the argument. */
	setsparam(args[1], ztrdup(ca_laststate.d->match));
	return 0;
    case 'a':
        /* This just sets the return value.  To zero if there would be or
         * were any normal arguments to be completed.  Used to decide if
         * _arguments should say `no arguments' or `no more arguments'. */
	for (; lstate; lstate = lstate->snext)
	    if (lstate->d->args || lstate->d->rest)
		return 0;
	return 1;
    case 'W':
        /* This gets two parameter names and one integer as arguments.
         *
         * The first parameter is set to the current word sans any option
         * prefixes handled by comparguments.
         *
         * The second parameter is set to an array containing the options on
         * the line and their arguments.  I.e. the stuff _arguments returns
         * to its caller in the `line' and `opt_args' parameters.
         *
         * The integer is one if the second parameter (which is just $opt_args,
         * you know) should encode multiple values by joining them with NULs
         * and zero if it should encode multiple values by joining them with
         * colons after backslash-escaping colons and backslashes.
         */
	{
	    Castate s;
	    char **ret, **p;
	    LinkNode n;
	    LinkList *a;
	    Caopt o;
	    int num;
	    int opt_args_use_NUL_separators = (args[3][0] != '0');

	    for (num = 0, s = lstate; s; s = s->snext)
		num += countlinknodes(s->args);

	    ret = p = zalloc((num + 1) * sizeof(char *));

	    for (s = lstate; s; s = s->snext)
		for (n = firstnode(s->args); n; incnode(n))
		    *p++ = ztrdup((char *) getdata(n));
	    *p = NULL;

	    setaparam(args[1], ret);

	    for (num = 0, s = lstate; s; s = s->snext)
		for (o = s->d->opts, a = s->oargs; o; o = o->next, a++)
		    if (*a)
			num += 2;

	    ret = p = zalloc((num + 1) * sizeof(char *));

	    for (s = lstate; s; s = s->snext)
		for (o = s->d->opts, a = s->oargs; o; o = o->next, a++)
		    if (*a) {
			*p++ = (o->gsname ? tricat(o->gsname, o->name, "") :
				ztrdup(o->name));
			if (opt_args_use_NUL_separators)
			    *p++ = ca_nullist(*a);
			else
			    *p++ = ca_colonlist(*a);
		    }
	    *p = NULL;

	    sethparam(args[2], ret);
	}
	return 0;
    case 'n':
	/*
	 * This returns the array index of the word where normal
	 * arguments began.  It uses optbeg rather than nargbeg
	 * (the value used when parsing) because nargbeg is assigned
	 * to optbeg in the returned value and nargbeg isn't
	 * used.
	 *
	 * -->PLEASE DON'T ASK<--
	 *
	 * Thank you.
	 */
	setiparam(args[1], (zlong)ca_laststate.optbeg + !isset(KSHARRAYS));
	return 0;
    }
    return 1;
}

/* Help for `_values'. */

typedef struct cvdef *Cvdef;
typedef struct cvval *Cvval;

/* Definitions for _values. */

struct cvdef {
    char *descr;		/* global description */
    int hassep;			/* multiple values allowed */
    char sep;			/* separator character */
    char argsep;                /* argument separator */
    Cvdef next;			/* next in cache */
    Cvval vals;			/* value definitions */
    char **defs;		/* original strings */
    int ndefs;			/* number of ... */
    int lastt;			/* last time used */
    int words;                  /* if to look at other words */
};

/* One value definition. */

struct cvval {
    Cvval next;
    char *name;			/* value name */
    char *descr;		/* description */
    char **xor;			/* xor-list */
    int type;			/* CVV_* below */
    Caarg arg;			/* argument definition */
    int active;			/* still allowed */
};

#define CVV_NOARG 0
#define CVV_ARG   1
#define CVV_OPT   2

/* Cache. */

#define MAX_CVCACHE 8
static Cvdef cvdef_cache[MAX_CVCACHE];

/* Memory stuff. */

static void
freecvdef(Cvdef d)
{
    if (d) {
	Cvval p, n;

	zsfree(d->descr);
	if (d->defs)
	    freearray(d->defs);

	for (p = d->vals; p; p = n) {
	    n = p->next;
	    zsfree(p->name);
	    zsfree(p->descr);
	    if (p->xor)
		freearray(p->xor);
	    freecaargs(p->arg);
	    zfree(p, sizeof(*p));
	}
	zfree(d, sizeof(*d));
    }
}

/* Parse option definitions. */

static Cvdef
parse_cvdef(char *nam, char **args)
{
    Cvdef ret;
    Cvval val, *valp;
    Caarg arg;
    char **oargs = args, sep = '\0', asep = '=', *name, *descr, *p, *q, **xor, c;
    int xnum, multi, vtype, hassep = 0, words = 0;

    while (args && args[0] && args[1] &&
           args[0][0] == '-' &&
           (args[0][1] == 's' || args[0][1] == 'S' || args[0][1] == 'w') &&
           !args[0][2]) {

        if (args[0][1] == 's') {
            hassep = 1;
            sep = args[1][0];
            args += 2;
        } else if (args[0][1] == 'S') {
            asep = args[1][0];
            args += 2;
        } else {
            words = 1;
            args++;
        }
    }
    if (!args[0] || !args[1]) {
	zwarnnam(nam, "not enough arguments");
	return NULL;
    }
    descr = *args++;

    ret = (Cvdef) zalloc(sizeof(*ret));
    ret->descr = ztrdup(descr);
    ret->hassep = hassep;
    ret->sep = sep;
    ret->argsep = asep;
    ret->next = NULL;
    ret->vals = NULL;
    ret->defs = zarrdup(oargs);
    ret->ndefs = arrlen(oargs);
    ret->lastt = time(0);
    ret->words = words;

    for (valp = &(ret->vals); *args; args++) {
	int bs = 0;
	p = dupstring(*args);
	xnum = 0;

	/* xor list? */
	if (*p == '(') {
	    LinkList list = newlinklist();
	    LinkNode node;
	    char **xp, sav;

	    while (*p && *p != ')') {
		for (p++; inblank(*p); p++);

		if (*p == ')')
		    break;
		for (q = p++; *p && *p != ')' && !inblank(*p); p++);

		if (!*p)
		    break;

		sav = *p;
		*p = '\0';
		addlinknode(list, dupstring(q));
		xnum++;
		*p = sav;
	    }
	    if (*p != ')') {
		freecvdef(ret);
		zwarnnam(nam, "invalid argument: %s", *args);
		return NULL;
	    }
	    xor = (char **) zalloc((xnum + 2) * sizeof(char *));
	    for (node = firstnode(list), xp = xor; node; incnode(node), xp++)
		*xp = ztrdup((char *) getdata(node));
	    xp[0] = xp[1] = NULL;

	    p++;
	} else
	    xor = NULL;

	/* More than once allowed? */
	if ((multi = (*p == '*')))
	    p++;

	/* Skip option name. */

	for (name = p; *p && *p != ':' && *p != '['; p++)
	    if (*p == '\\' && p[1])
		p++, bs = 1;

	if (hassep && !sep && name + bs + 1 < p) {
	    freecvdef(ret);
	    if (xor) freearray(xor);
	    zwarnnam(nam, "no multi-letter values with empty separator allowed");
	    return NULL;
	}
	/* Optional description? */

	if ((c = *p) == '[') {
	    *p = '\0';
	    for (descr = ++p; *p && *p != ']'; p++)
		if (*p == '\\' && p[1])
		    p++;

	    if (!*p) {
		freecvdef(ret);
		if (xor) freearray(xor);
		zwarnnam(nam, "invalid value definition: %s", *args);
		return NULL;
	    }
	    *p++ = '\0';
	    c = *p;
	} else {
	    *p = '\0';
	    descr = NULL;
	}
	if (c && c != ':') {
	    freecvdef(ret);
	    if (xor) freearray(xor);
	    zwarnnam(nam, "invalid value definition: %s", *args);
	    return NULL;
	}
	/* Get argument? */

	if (c == ':') {
	    if (hassep && !sep) {
		freecvdef(ret);
		if (xor) freearray(xor);
		zwarnnam(nam, "no value with argument with empty separator allowed");
		return NULL;
	    }
	    if (*++p == ':') {
		p++;
		vtype = CVV_OPT;
	    } else
		vtype = CVV_ARG;
	    arg = parse_caarg(0, 0, 0, 0, name, &p, NULL);
	} else {
	    vtype = CVV_NOARG;
	    arg = NULL;
	}
	if (!multi) {
	    if (!xor) {
		xor = (char **) zalloc(2 * sizeof(char *));
		xor[1] = NULL;
	    }
	    xor[xnum] = ztrdup(name);
	}
	*valp = val = (Cvval) zalloc(sizeof(*val));
	valp = &((*valp)->next);

	val->next = NULL;
	val->name = ztrdup(name);
	val->descr = ztrdup(descr);
	val->xor = xor;
	val->type = vtype;
	val->arg = arg;
    }
    return ret;
}

/* Get the definition from the cache or newly built. */

static Cvdef
get_cvdef(char *nam, char **args)
{
    Cvdef *p, *min, new;
    int i, na = arrlen(args);

    for (i = MAX_CVCACHE, p = cvdef_cache, min = NULL; *p && i--; p++)
	if (*p && na == (*p)->ndefs && arrcmp(args, (*p)->defs)) {
	    (*p)->lastt = time(0);

	    return *p;
	} else if (!min || !*p || (*p)->lastt < (*min)->lastt)
	    min = p;
    if (i > 0)
	min = p;
    if ((new = parse_cvdef(nam, args))) {
	freecvdef(*min);
	*min = new;
    }
    return new;
}

/* Get the definition for a value. */

static Cvval
cv_get_val(Cvdef d, char *name)
{
    Cvval p;

    for (p = d->vals; p; p = p->next)
	if (!strcmp(name, p->name))
	    return p;

    return NULL;
}

static Cvval
cv_quote_get_val(Cvdef d, char *name)
{
    int ne;

    /* remove quotes */
    name = dupstring(name);
    ne = noerrs;
    noerrs = 2;
    parse_subst_string(name);
    noerrs = ne;
    remnulargs(name);
    untokenize(name);

    return cv_get_val(d, name);
}

/* Handle a xor list. */

static void
cv_inactive(Cvdef d, char **xor)
{
    if (xor) {
	Cvval val;

	for (; *xor; xor++)
	    if ((val = cv_get_val(d, *xor)))
		val->active = 0;
    }
}

/* Parse state. */

struct cvstate {
    Cvdef d;
    Caarg def;
    Cvval val;
    LinkList vals;
};

static struct cvstate cv_laststate;
static int cv_parsed = 0, cv_alloced = 0;

/* Get the next value in the string.  Return it's definition and update the
 * sp pointer to point to the end of the value (plus argument, if any).
 * If there is no next value, the string pointer is set to null.  In any
 * case ap will point to the beginning of the argument or will be a null
 * pointer if there is no argument.
 */

static Cvval
cv_next(Cvdef d, char **sp, char **ap)
{
    Cvval r = NULL;
    char *s = *sp;

    if (!*s) {
        *sp = *ap = NULL;

        return NULL;
    }
    if ((d->hassep && !d->sep) || !d->argsep) {
        char sav, ec, *v = s, *os;

        ec = ((d->hassep && d->sep) ? d->sep : d->argsep);

        do {
            sav = *++s;
            *s = '\0';
            if ((r = cv_quote_get_val(d, v))) {
                *s = sav;

                break;
            }
            *s = sav;
        } while (*s && *s != ec);

        os = s;

        if (d->hassep && d->sep) {
            if ((s = strchr(s, d->sep)))
                *sp = s + 1;
            else
                *sp = NULL;
        } else
            *sp = s;
        if (d->argsep && *os == d->argsep) {
            *ap = os + 1;
            *sp = NULL;
        } else if (r && r->type != CVV_NOARG)
            *ap = os;
        else
            *ap = NULL;

        return r;

    } else if (d->hassep) {
        char *ns = strchr(s, d->sep), *as = 0, *sap, sav = 0;
        int skip = 0;

        if (d->argsep && (as = strchr(s, d->argsep)) && (!ns || as <= ns)) {
            *ap = as + 1;
            ns = strchr(as + 1, d->sep);
            skip = 1;
            sap = as;
        } else {
            *ap = NULL;
            sap = ns;
        }
        if (sap) {
            sav = *sap;
            *sap = '\0';
        }
        if ((!(r = cv_quote_get_val(d, s)) || r->type == CVV_NOARG) && skip)
            ns = as;

        if (sap)
            *sap = sav;

        *sp = ((!ns || (ns == as && r && r->type != CVV_NOARG)) ? NULL : ns + 1);

        return r;
    } else {
        char *as = strchr(s, d->argsep), *sap, sav = 0;

        *sp = NULL;

        if (as) {
            *ap = as + 1;
            sap = as;
            sav = *as;
            *sap = '\0';
        } else
            *ap = sap = NULL;

        r = cv_quote_get_val(d, s);

        if (sap)
            *sap = sav;

        return r;
    }
}

/* Parse the current word. */

static void
cv_parse_word(Cvdef d)
{
    Cvval val;
    struct cvstate state;
    char *str, *arg = NULL, *pign = compprefix;
    int nosfx = 0;

    if (cv_alloced)
	freelinklist(cv_laststate.vals, freestr);

    for (val = d->vals; val; val = val->next)
	val->active = 1;

    state.d = d;
    state.def = NULL;
    state.val = NULL;
    state.vals = (LinkList) znewlinklist();

    cv_alloced = 1;

    if (d->words && compwords[0]) {
        int i;

        for (i = 1; compwords[i]; i++)
            if (i != compcurrent - 1)
                for (str = compwords[i]; str && *str; ) {
                    if ((val = cv_next(d, &str, &arg))) {
                        zaddlinknode(state.vals, ztrdup(val->name));
                        if (arg) {
                            char sav = '\0';

                            if (str) {
                                sav = str[-1];
                                str[-1] = '\0';
                            }
                            zaddlinknode(state.vals, ztrdup(arg));
                            if (str)
                                str[-1] = sav;
                        } else
                            zaddlinknode(state.vals, ztrdup(""));

                        if (i + 1 < compcurrent)
                            cv_inactive(d, val->xor);
                    }
                }

        val = NULL;
        arg = NULL;
    }
    for (str = compprefix; str && *str; ) {
        if ((val = cv_next(d, &str, &arg))) {
            zaddlinknode(state.vals, ztrdup(val->name));
            if (arg) {
                if (str) {
                    char sav = str[-1];

                    str[-1] = '\0';
                    zaddlinknode(state.vals, ztrdup(arg));
                    str[-1] = sav;
                } else {
                    zaddlinknode(state.vals, tricat(arg, compsuffix, ""));
                    nosfx = 1;
                }
            } else
                zaddlinknode(state.vals, ztrdup(""));

            cv_inactive(d, val->xor);

            if (str)
                pign = str;
            else
                val->active = 1;
        }
    }
    state.val = val;
    if (val && arg && !str)
        state.def = val->arg;

    if (!nosfx && d->hassep) {
        int ign = 0;
        char *more = NULL;

        ignore_prefix(pign - compprefix);

        if (!d->sep && (!val || val->type == CVV_NOARG)) {
            ign = strlen(compsuffix);
            more = compsuffix;
        } else {
            if (d->sep) {
                char *ns = strchr(compsuffix, d->sep), *as;

                if (d->argsep && (as = strchr(compsuffix, d->argsep)) &&
                    (!ns || as <= ns)) {
                    ign = strlen(as);
                } else
                    ign = (ns ? strlen(ns) : 0);

                more = (ns ? ns + 1 : NULL);
            } else if (d->argsep) {
                char *as;

                if ((as = strchr(compsuffix, d->argsep)))
                    ign = strlen(as);
            }
        }
        more = dupstring(more);

        if (ign)
            ignore_suffix(ign);

        while (more && *more) {
            if ((val = cv_next(d, &more, &arg))) {
                zaddlinknode(state.vals, ztrdup(val->name));
                if (arg) {
                    if (more) {
                        char sav = more[-1];

                        more[-1] = '\0';
                        zaddlinknode(state.vals, ztrdup(arg));
                        more[-1] = sav;
                    } else {
                        zaddlinknode(state.vals, tricat(arg, compsuffix, ""));
                        nosfx = 1;
                    }
                } else
                    zaddlinknode(state.vals, ztrdup(""));

                cv_inactive(d, val->xor);
            }
        }
    } else if (arg)
        ignore_prefix(arg - compprefix);
    else
        ignore_prefix(pign - compprefix);

    memcpy(&cv_laststate, &state, sizeof(state));
}

static int
bin_compvalues(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    int min, max, n;

    if (incompfunc != 1) {
	zwarnnam(nam, "can only be called from completion function");
	return 1;
    }
    if (args[0][0] != '-' || !args[0][1] || args[0][2]) {
	zwarnnam(nam, "invalid argument: %s", args[0]);
	return 1;
    }
    if (args[0][1] != 'i' && !cv_parsed) {
	zwarnnam(nam, "no parsed state");
	return 1;
    }
    switch (args[0][1]) {
    case 'i': min = 2; max = -1; break;
    case 'D': min = 2; max =  2; break;
    case 'C': min = 1; max =  1; break;
    case 'V': min = 3; max =  3; break;
    case 's': min = 1; max =  1; break;
    case 'S': min = 1; max =  1; break;
    case 'd': min = 1; max =  1; break;
    case 'L': min = 3; max =  4; break;
    case 'v': min = 1; max =  1; break;
    default:
	zwarnnam(nam, "invalid option: %s", args[0]);
	return 1;
    }
    n = arrlen(args) - 1;
    if (n < min) {
	zwarnnam(nam, "not enough arguments");
	return 1;
    } else if (max >= 0 && n > max) {
	zwarnnam(nam, "too many arguments");
	return 1;
    }
    switch (args[0][1]) {
    case 'i':
        /* This initialises the internal data structures.  The arguments are
         * just the arguments that were given to _values itself. */
	{
	    Cvdef def = get_cvdef(nam, args + 1);
	    int cvp = cv_parsed;

	    cv_parsed = 0;

	    if (!def)
		return 1;

	    cv_parsed = cvp;
	    cv_parse_word(def);
	    cv_parsed = 1;

	    return 0;
	}

    case 'D':
        /* This returns the description and action to use if we are at
         * a place where some action has to be used at all.  In that case
         * zero is returned and non-zero otherwise. */
	{
	    Caarg arg = cv_laststate.def;

	    if (arg) {
		setsparam(args[1], ztrdup(arg->descr));
		setsparam(args[2], ztrdup(arg->action));

		return 0;
	    }
	    return 1;
	}
    case 'C':
        /* This returns the sub-context (i.e.: the tag) to use when executing
         * an action. */
	{
	    Caarg arg = cv_laststate.def;

	    if (arg) {
		setsparam(args[1], ztrdup(arg->opt));

		return 0;
	    }
	    return 1;
	}
    case 'V':
        /* This is what -O is for comparguments: it returns (in three arrays)
         * the values for values without arguments, with arguments and with
         * optional arguments (so that we can get the auto-suffixes right).
         * As for comparguments, the strings returned are usable for _describe. */
	{
	    LinkList noarg = newlinklist();
	    LinkList arg = newlinklist();
	    LinkList opt = newlinklist(), l;
	    Cvval p;
	    char *str;

	    for (p = cv_laststate.d->vals; p; p = p->next) {
		if (p->active) {
		    switch (p->type) {
		    case CVV_NOARG: l = noarg; break;
		    case CVV_ARG:   l = arg;   break;
		    default:        l = opt;   break;
		    }
		    if (p->descr) {
			int len = strlen(p->name) + strlen(p->descr) + 2;

			str = (char *) zhalloc(len);
			strcpy(str, p->name);
			strcat(str, ":");
			strcat(str, p->descr);
		    } else
			str = p->name;
		    addlinknode(l, str);
		}
	    }
	    set_list_array(args[1], noarg);
	    set_list_array(args[2], arg);
	    set_list_array(args[3], opt);

	    return 0;
	}
    case 's':
        /* This returns the value separator, if any, and sets the return
         * value to say if there is such a separator. */
	if (cv_laststate.d->hassep) {
	    char tmp[2];

	    tmp[0] = cv_laststate.d->sep;
	    tmp[1] = '\0';
	    setsparam(args[1], ztrdup(tmp));

	    return 0;
	}
	return 1;
    case 'S':
        /* Like -s, but for the separator between values and their arguments. */
	{
	    char tmp[2];

	    tmp[0] = cv_laststate.d->argsep;
	    tmp[1] = '\0';
	    setsparam(args[1], ztrdup(tmp));
	}
	return 0;
    case 'd':
        /* This returns the description string (first argument to _values)
         * which is passed down to _describe. */
	setsparam(args[1], ztrdup(cv_laststate.d->descr));
	return 0;
    case 'L':
        /* Almost the same as for comparguments.  This gets a value name
         * and returns the description and action of its first argument, if
         * any.  The rest (prefix matching) is in _values.  Return non-zero
         * if there is no such option. */
	{
	    Cvval val = cv_get_val(cv_laststate.d, args[1]);

	    if (val && val->arg) {
		setsparam(args[2], ztrdup(val->arg->descr));
		setsparam(args[3], ztrdup(val->arg->action));

		if (args[4])
		    setsparam(args[4], ztrdup(val->name));

		return 0;
	    }
	    return 1;
	}
    case 'v':
        /* Again, as for comparguments.  This returns the values and their
         * arguments as an array which will be stored in val_args in _values. */
	if (cv_laststate.vals) {
	    char **ret;

	    ret = zlinklist2array(cv_laststate.vals, 1);
	    sethparam(args[1], ret);

	    return 0;
	}
	return 1;
    }
    return 1;
}

static char *
comp_quote(char *str, int prefix)
{
    int x;
    char *ret;

    if ((x = (prefix && *str == '=')))
	*str = 'x';

    ret = quotestring(str, *compqstack);

    if (x)
	*str = *ret = '=';

    return ret;
}

static int
bin_compquote(char *nam, char **args, Options ops, UNUSED(int func))
{
    char *name;
    struct value vbuf;
    Value v;

    if (incompfunc != 1) {
	zwarnnam(nam, "can only be called from completion function");
	return 1;
    }
    /* Anything to do? */

    if (!compqstack || !*compqstack)
	return 0;

    /* For all parameters given... */

    while ((name = *args++)) {
	name = dupstring(name);
	queue_signals();
	if ((v = getvalue(&vbuf, &name, 0))) {
	    switch (PM_TYPE(v->pm->node.flags)) {
	    case PM_SCALAR:
		setstrvalue(v, ztrdup(comp_quote(getstrvalue(v), 
						 OPT_ISSET(ops,'p'))));
		break;
	    case PM_ARRAY:
		{
		    char **val = v->pm->gsu.a->getfn(v->pm);
		    char **new = (char **) zalloc((arrlen(val) + 1) *
						  sizeof(char *));
		    char **p = new;

		    for (; *val; val++, p++)
			*p = ztrdup(comp_quote(*val, OPT_ISSET(ops,'p')));
		    *p = NULL;

		    setarrvalue(v, new);
		}
		break;
	    default:
		zwarnnam(nam, "invalid parameter type: %s", args[-1]);
	    }
	} else
	    zwarnnam(nam, "unknown parameter: %s", args[-1]);
	unqueue_signals();
    }
    return 0;
}

/* Tags stuff. */

typedef struct ctags *Ctags;
typedef struct ctset *Ctset;

/* A bunch of tag sets. */

struct ctags {
    char **all;			/* all tags offered */
    char *context;		/* the current context */
    int init;			/* not yet used */
    Ctset sets;			/* the tag sets */
};

/* A tag set. */

struct ctset {
    Ctset next;
    char **tags;		/* the tags */
    char *tag;			/* last tag checked for -A */
    char **ptr;			/* ptr into tags for -A */
};

/* Array of tag-set infos. Index is the locallevel. */

#define MAX_TAGS 256
static Ctags comptags[MAX_TAGS];

/* locallevel at last comptags -i */

static int lasttaglevel;

static void
freectset(Ctset s)
{
    Ctset n;

    while (s) {
	n = s->next;

	if (s->tags)
	    freearray(s->tags);
	zsfree(s->tag);
	zfree(s, sizeof(*s));

	s = n;
    }
}

static void
freectags(Ctags t)
{
    if (t) {
	if (t->all)
	    freearray(t->all);
	zsfree(t->context);
	freectset(t->sets);
	zfree(t, sizeof(*t));
    }
}

/* Set the tags for the current local level. */

static void
settags(int level, char **tags)
{
    Ctags t;

    if (comptags[level])
	freectags(comptags[level]);

    comptags[level] = t = (Ctags) zalloc(sizeof(*t));

    t->all = zarrdup(tags + 1);
    t->context = ztrdup(*tags);
    t->sets = NULL;
    t->init = 1;
}

/* Check if an array contains a string. */

/**/
static int
arrcontains(char **a, char *s, int colon)
{
    char *p, *q;

    while (*a) {
	if (colon) {
	    for (p = s, q = *a++; *p && *q && *p != ':' && *q != ':'; p++, q++)
		if (*p != *q)
		    break;
	    if ((!*p || *p == ':') && (!*q || *q == ':'))
		return 1;
	} else if (!strcmp(*a++, s))
	    return 1;
    }
    return 0;
}

static int
bin_comptags(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    int min, max, n, level;

    if (incompfunc != 1) {
	zwarnnam(nam, "can only be called from completion function");
	return 1;
    }
    if (args[0][0] != '-' || !args[0][1] ||
	(args[0][2] && (args[0][2] != '-' || args[0][3]))) {
	zwarnnam(nam, "invalid argument: %s", args[0]);
	return 1;
    }
    level = locallevel - (args[0][2] ? 1 : 0);
    if (level >= MAX_TAGS) {
	zwarnnam(nam, "nesting level too deep");
	return 1;
    }
    if (args[0][1] != 'i' && args[0][1] != 'I' && !comptags[level]) {
	zwarnnam(nam, "no tags registered");
	return 1;
    }
    switch (args[0][1]) {
    case 'i': min = 2; max = -1; break;
    case 'C': min = 1; max =  1; break;
    case 'T': min = 0; max =  0; break;
    case 'N': min = 0; max =  0; break;
    case 'R': min = 1; max =  1; break;
    case 'S': min = 1; max =  1; break;
    case 'A': min = 2; max =  3; break;
    default:
	zwarnnam(nam, "invalid option: %s", args[0]);
	return 1;
    }
    n = arrlen(args) - 1;
    if (n < min) {
	zwarnnam(nam, "not enough arguments");
	return 1;
    } else if (max >= 0 && n > max) {
	zwarnnam(nam, "too many arguments");
	return 1;
    }
    switch (args[0][1]) {
    case 'i':
	settags(level, args + 1);
	lasttaglevel = level;
	break;
    case 'C':
	setsparam(args[1], ztrdup(comptags[level]->context));
	break;
    case 'T':
	return !comptags[level]->sets;
    case 'N':
	{
	    Ctset s;

	    if (comptags[level]->init)
		comptags[level]->init = 0;
	    else if ((s = comptags[level]->sets)) {
		comptags[level]->sets = s->next;
		s->next = NULL;
		freectset(s);
	    }
	    return !comptags[level]->sets;
	}
    case 'R':
	{
	    Ctset s;

	    return !((s = comptags[level]->sets) &&
		     arrcontains(s->tags, args[1], 1));
	}
    case 'A':
	{
	    Ctset s;

	    if (comptags[level] && (s = comptags[level]->sets)) {
		char **q, *v = NULL;
		int l = strlen(args[1]);

		if (!s->tag || strcmp(s->tag, args[1])) {
		    zsfree(s->tag);
		    s->tag = ztrdup(args[1]);
		    s->ptr = s->tags;
		}
		for (q = s->ptr; *q; q++) {
		    if (strpfx(args[1], *q)) {
			if (!(*q)[l]) {
			    v = *q;
			    break;
			} else if ((*q)[l] == ':') {
			    v = (*q) + l + 1;
			    break;
			}
		    }
		}
		if (!v) {
		    zsfree(s->tag);
		    s->tag = NULL;
		    return 1;
		}
		s->ptr = q + 1;
		setsparam(args[2], ztrdup(*v == '-' ? dyncat(args[1], v) : v));
		if (args[3]) {
		    char *r = dupstring(*q), *p;

		    for (p = r + (v - *q); *p && *p != ':'; p++);
		    *p = '\0';

		    setsparam(args[3], ztrdup(r));
		}
		return 0;
	    }
	    return 1;
	}
    case 'S':
	if (comptags[level]->sets) {
	    char **ret;

	    ret = zarrdup(comptags[level]->sets->tags);
	    setaparam(args[1], ret);
	} else
	    return 1;

	break;
    }
    return 0;
}

static int
bin_comptry(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    if (incompfunc != 1) {
	zwarnnam(nam, "can only be called from completion function");
	return 1;
    }
    if (!lasttaglevel || !comptags[lasttaglevel]) {
	zwarnnam(nam, "no tags registered");
	return 1;
    }
    if (*args) {
	if (!strcmp(*args, "-m")) {
	    char *s, *p, *q, *c, **all = comptags[lasttaglevel]->all;
	    LinkList list = newlinklist();
	    int num = 0;
	    Ctset set;

	    while ((s = *++args)) {
		while (*s) {
		    while (*s && iblank(*s))
			s++;
		    for (p = q = s, c = NULL; *s && !inblank(*s); s++) {
			if (!c && *s == ':')
			    c = p;
			if (*s == '\\' && s[1])
			    s++;
			*p++ = *s;
		    }
		    if (*s)
			s++;
		    *p = '\0';
		    if (*q) {
			char *qq, *qqq;

			queue_signals();

			if (c)
			    *c = '\0';

			qqq = qq = dupstring(q);
			while (*qqq) {
			    if (*qqq == '\\' && qqq[1])
				qqq++;
			    else if (*qqq == '{')
				*qqq = Inbrace;
			    else if (*qqq == '}')
				*qqq = Outbrace;
			    else if (*qqq == ',')
				*qqq = Comma;
			    qqq++;
			}
			tokenize(qq);
			if (haswilds(qq) || hasbraces(qq)) {
			    Patprog prog;
			    LinkNode bnode, node;
			    LinkList blist = newlinklist();

			    addlinknode(blist, qq);
			    for (bnode = firstnode(blist); bnode; incnode(bnode))
				while (hasbraces(getdata(bnode)))
				    xpandbraces(blist, &bnode);

			    for (bnode = firstnode(blist); bnode; incnode(bnode)) {
				qq = (char *) getdata(bnode);
				if ((prog = patcompile(qq, PAT_STATIC, NULL))) {
				    char **a, *n;
				    int l = (c ? strlen(c + 1) + 2 : 1), al;

				    for (a = all; *a; a++) {
					for (node = firstnode(list); node;
					     incnode(node)) {
					    char *as, *ls;

					    for (as = *a, ls = (char *) getdata(node);
						 *as && *ls && *ls != ':'; as++, ls++)
						if (*as != *ls)
						    break;
					    if (!*as && (!*ls || *ls == ':'))
						break;
					}
					if (node)
					    continue;
					if (pattry(prog, *a)) {
					    n = (char *) zhalloc((al = strlen(*a)) + l);
					    strcpy(n, *a);
					    if (c) {
						n[al] = ':';
						strcpy(n + al + 1, c + 1);
					    }
					    addlinknode(list, n);
					    num++;
					}
				    }
				}
			    }
			} else if (arrcontains(all, q, 0)) {
			    for (set = comptags[lasttaglevel]->sets; set;
				 set = set->next)
				if (arrcontains(set->tags, q, 0))
				    break;
			    if (!set) {
				addlinknode(list, q);
				num++;
			    }
			}
			if (c)
			    *c = ':';

			unqueue_signals();
		    }
		}
		if (num) {
		    Ctset l;

		    set = (Ctset) zalloc(sizeof(*set));

		    set->tags = zlinklist2array(list, 1);
		    set->next = NULL;
		    set->ptr = NULL;
		    set->tag = NULL;

		    if ((l = comptags[lasttaglevel]->sets)) {
			while (l->next)
			    l = l->next;

			l->next = set;
		    } else
			comptags[lasttaglevel]->sets = set;
		}
	    }
	} else {
	    char **p, **q, **all;
	    int sep = 0;

	    if ((sep = !strcmp(*args, "-s")))
		args++;

	    for (p = q = args, all = comptags[lasttaglevel]->all; *p; p++)
		if (arrcontains(all, *p, 1)) {
		    Ctset s;

		    for (s = comptags[lasttaglevel]->sets; s; s = s->next)
			if (arrcontains(s->tags, *p, 0))
			    break;

		    if (!s)
			*q++ = *p;
		}
	    *q = NULL;

	    if (*args) {
		char *dummy[2];

		do {
		    Ctset s = (Ctset) zalloc(sizeof(*s)), l;

		    if (sep) {
			dummy[0] = *args++;
			dummy[1] = NULL;
			s->tags = zarrdup(dummy);
		    } else
			s->tags = zarrdup(args);
		    s->next = NULL;
		    s->ptr = NULL;
		    s->tag = NULL;

		    if ((l = comptags[lasttaglevel]->sets)) {
			while (l->next)
			    l = l->next;

			l->next = s;
		    } else
			comptags[lasttaglevel]->sets = s;
		} while (sep && *args);
	    }
	}
    }
    return 0;
}

#define PATH_MAX2 (PATH_MAX * 2)

/*
 * Return a list of files we should accept exactly, without
 * trying pattern matching.
 *
 * This is based on the accept-exact style, which may be
 * an array so is passed in via "accept".  The trial files
 * are input in "names".  "skipped" is passed down straight
 * from the file completion function:  it's got something to
 * do with other components in the path but it's hard to work out
 * quite what.
 *
 * There is one extra trick here for Cygwin.  Regardless of the style,
 * if the file ends in a colon it has to be a drive or a special device
 * file and we always accept it exactly because treating it as a pattern
 * won't work.
 */
static LinkList
cfp_test_exact(LinkList names, char **accept, char *skipped)
{
    char buf[PATH_MAX2 + 1], *suf, *p;
    int l, sl, found = 0;
    struct stat st;
    LinkNode node;
    LinkList ret = newlinklist(), alist = NULL;
#ifdef __CYGWIN__
    int accept_off = 0;
#endif

    /*
     * Don't do this unless completion has provided either a
     * prefix or suffix from the command line.
     */
    if (!(compprefix && *compprefix) && !(compsuffix && *compsuffix))
	return NULL;

    /*
     * See if accept-exact is off, implicitly or explicitly.
     */
    if (!accept || !*accept ||
	((!strcmp(*accept, "false") || !strcmp(*accept, "no") ||
	  !strcmp(*accept, "off") || !strcmp(*accept, "0")) && !accept[1])) {
#ifdef __CYGWIN__
	accept_off = 1;
#else
	/* If not Cygwin, nothing to do here. */
	return NULL;
#endif
    }

    /*
     * See if the style is something other than just a boolean.
     */
    if (
#ifdef __CYGWIN__
	!accept_off &&
#endif
	(accept[1] ||
	 (strcmp(*accept, "true") && strcmp(*accept, "yes") &&
	  strcmp(*accept, "on") && strcmp(*accept, "1")))) {
	Patprog prog;

	alist = newlinklist();

	for (; (p = *accept); accept++) {
	    if (*p == '*' && !p[1]) {
		alist = NULL;
		break;
	    }
	    tokenize(p = dupstring(p));
	    if ((prog = patcompile(p, 0, NULL)))
		addlinknode(alist, prog);
	}
    }
    /*
     * Assemble the bits other than the set of file names:
     * the other components, and the prefix and suffix.
     */
    sl = strlen(skipped) + (compprefix ? strlen(compprefix) : 0) +
	(compsuffix ? strlen(compsuffix) : 0);

    if (sl > PATH_MAX2)
	return NULL;

    suf = dyncat(skipped, rembslash(dyncat(compprefix ? compprefix : "",
		                           compsuffix ? compsuffix : "")));

    for (node = firstnode(names); node; incnode(node)) {
	l = strlen(p = (char *) getdata(node));
	if (l + sl < PATH_MAX2) {
#ifdef __CYGWIN__
	    char *testbuf;
#define TESTBUF testbuf
#else
#define TESTBUF buf
#endif
	    strcpy(buf, p);
	    strcpy(buf + l, suf);
#ifdef __CYGWIN__
	    if (accept_off) {
		int sl = strlen(buf);
		/*
		 * If accept-exact is not set, accept this only if
		 * it looks like a special file such as a drive.
		 * We still test if it exists.
		 */
		if (!sl || strchr(buf, '/') || buf[sl-1] != ':')
		    continue;
		if (sl == 2) {
		    /*
		     * Recent versions of Cygwin only recognise "c:/",
		     * but not "c:", as special directories.  So
		     * we have to append the slash for the purpose of
		     * the test.
		     */
		    testbuf = zhalloc(sl + 2);
		    strcpy(testbuf, buf);
		    testbuf[sl] = '/';
		    testbuf[sl+1] = '\0';
		} else {
		    /* Don't do this with stuff like PRN: */
		    testbuf = buf;
		}
	    } else {
		testbuf = buf;
	    }
#endif
	    if (!ztat(TESTBUF, &st, 0)) {
		/*
		 * File exists; if accept-exact contained non-boolean
		 * values it must match those, too.
		 */
		if (alist) {
		    LinkNode anode;

		    for (anode = firstnode(alist); anode; incnode(anode))
			if (pattry((Patprog) getdata(anode), buf))
			    break;

		    if (!anode)
			continue;
		}
		found = 1;
		addlinknode(ret, dupstring(buf));
	    }
	}
    }
    return (found ? ret : NULL);
}


/*
 * This code constructs (from heap) and returns a string that
 * corresponds to a series of matches; when compiled as a pattern, at
 * each position it matches either the character from the string "add"
 * or the corresponding single-character match from the set of matchers.
 * To take a simple case, if add is "a" and the single matcher for the
 * character position matches "[0-9]", the pattern returned is "[0-9a]".
 * We take account of equivalences between the word and line, too.
 *
 * As there are virtually no comments in this file, I don't really
 * know why we're doing this, but it's to do with a matcher which
 * is passed as an argument to the utility compfiles -p/-P.
 */
static char *
cfp_matcher_range(Cmatcher *ms, char *add)
{
    Cmatcher *mp, m;
    int len = 0, mt;
    char *ret = NULL, *p = NULL, *adds = add;

    /*
     * Do this twice:  once to work out the length of the
     * string in len, the second time to build it in ret.
     * This is probably worthwhile because otherwise memory
     * management is difficult.
     */
    for (;;) {
	MB_METACHARINIT();
	for (mp = ms; *add; ) {
	    convchar_t addc;
	    int addlen;

	    addlen = MB_METACHARLENCONV(add, &addc);
#ifdef MULTIBYTE_SUPPORT
	    if (addc == WEOF)
		addc = (wchar_t)(*add == Meta ? add[1] ^ 32 : *add);
#endif

	    if (!(m = *mp)) {
		/*
		 * No matcher, so just match the character
		 * itself.
		 *
		 * TODO: surely this needs quoting if it's a
		 * metacharacter?
		 */
		if (ret) {
		    memcpy(p, add, addlen);
		    p += addlen;
		} else
		    len += addlen;
	    } else if (m->flags & CMF_RIGHT) {
		/*
		 * Right-anchored:  match anything followed
		 * by the character itself.
		 */
		if (ret) {
		    *p++ = '*';
		    /* TODO: quote again? */
		    memcpy(p, add, addlen);
		    p += addlen;
		} else
		    len += addlen + 1;
	    } else {
		/* The usual set of matcher possibilities. */
		convchar_t ind;
		if (m->line->tp == CPAT_EQUIV &&
		    m->word->tp == CPAT_EQUIV) {
		    /*
		     * Genuine equivalence.  Add the character to match
		     * and the equivalent character from the word
		     * pattern.
		     *
		     * TODO: we could be more careful here with special
		     * cases as we are in the basic character class
		     * code below.
		     */
		    if (ret) {
			*p++ = '[';
			memcpy(p, add, addlen);
			p += addlen;
		    } else
			len += addlen + 1;
		    if (PATMATCHRANGE(m->line->u.str, addc, &ind, &mt)) {
			/*
			 * Find the equivalent match for ind in the
			 * word pattern.
			 */
			if ((ind = pattern_match_equivalence
			     (m->word, ind, mt, addc)) != CHR_INVALID) {
			    if (ret) {
				if (imeta(ind)) {
				    *p++ = Meta;
				    *p++ = ind ^ 32;
				} else
				    *p++ = ind;
			    } else
				len += imeta(ind) ? 2 : 1;
			}
		    }
		    if (ret)
			*p++ = ']';
		    else
			len++;
		} else {
		    int newlen, addadd;

		    switch (m->word->tp) {
		    case CPAT_NCLASS:
			/*
			 * TODO: the old logic implies that we need to
			 * match *add, i.e. it should be deleted from
			 * the set of character's we're not allowed to
			 * match.  That's too much like hard work for
			 * now.  Indeed, in general it's impossible
			 * without trickery.  Consider *add == 'A',
			 * range == "[^[:upper:]]": we would have to
			 * resort to something like "(A|[^[:upper:]])";
			 * and in an expression like that *add may or
			 * may not need backslashing.  So we're deep
			 * into see-if-we-can-get-away-without
			 * territory.
			 */
			if (ret) {
			    *p++ = '[';
			    *p++ = '^';
			} else
			    len += 2;
			/*
			 * Convert the compiled range string back
			 * to an ordinary string.
			 */
			newlen =
			    pattern_range_to_string(m->word->u.str, p);
			DPUTS(!newlen, "empty character range");
			if (ret) {
			    p += newlen;
			    *p++ = ']';
			} else
			    len += newlen + 1;
			break;
			    
		    case CPAT_CCLASS:
			/*
			 * If there is an equivalence only on one
			 * side it's not equivalent to anything.
			 * Treat it as an ordinary character class.
			 */ 
		    case CPAT_EQUIV:
		    case CPAT_CHAR:
			if (ret)
			    *p++ = '[';
			else
			    len++;
			/*
			 * We needed to add *add specially only if
			 * it is not covered by the range.  This
			 * is necessary for correct syntax---consider
			 * if *add is ] and ] is also the first
			 * character in the range.
			 */
			addadd = !pattern_match1(m->word, addc, &mt);
			if (addadd && *add == ']') {
			    if (ret)
				*p++ = *add;
			    else
				len++;
			}
			if (m->word->tp == CPAT_CHAR) {
			    /*
			     * The matcher just matches a single
			     * character, but we need to be able
			     * to match *add, too, hence we do
			     * this as a [...].
			     */
			    if (ret) {
				if (imeta(m->word->u.chr)) {
				    *p++ = Meta;
				    *p++ = m->word->u.chr ^ 32;
				} else
				    *p++ = m->word->u.chr;
			    } else
				len += imeta(m->word->u.chr) ? 2 : 1;
			} else {
			    /*
			     * Convert the compiled range string back
			     * to an ordinary string.
			     */
			    newlen =
				pattern_range_to_string(m->word->u.str, p);
			    DPUTS(!newlen, "empty character range");
			    if (ret)
				p += newlen;
			    else
				len += newlen;
			}
			if (addadd && *add != ']') {
			    if (ret) {
				memcpy(p, add, addlen);
				p += addlen;
			    } else
				len += addlen;
			}
			if (ret)
			    *p++ = ']';
			else
			    len++;
			break;

		    case CPAT_ANY:
			if (ret)
			    *p++ = '?';
			else
			    len++;
			break;
		    }
		}
	    }
	    add += addlen;
	    mp++;
	}
	if (ret) {
	    *p = '\0';
	    return ret;
	}
	p = ret = zhalloc(len + 1);
	add = adds;
    }
}


static char *
cfp_matcher_pats(char *matcher, char *add)
{
    Cmatcher m = parse_cmatcher(NULL, matcher);

    if (m && m != pcm_err) {
	char *tmp;
	int al = strlen(add), zl = ztrlen(add), tl, cl;
	VARARR(Cmatcher, ms, zl);	/* One Cmatcher per character */
	Cmatcher *mp;
	Cpattern stopp;
	int stopl = 0;

	/* zl >= (number of wide characters) is guaranteed */
	memset(ms, 0, zl * sizeof(Cmatcher));

	for (; m && *add; m = m->next) {
	    stopp = NULL;
	    if (!(m->flags & (CMF_LEFT|CMF_RIGHT))) {
		if (m->llen == 1 && m->wlen == 1) {
		    /*
		     * In this loop and similar loops below we step
		     * through tmp one (possibly wide) character at a
		     * time.  pattern_match() compares only the first
		     * character using unmeta_one() so keep in step.
		     */
		    for (tmp = add, tl = al, mp = ms; tl; ) {
			if (pattern_match(m->line, tmp, NULL, NULL)) {
			    if (*mp) {
				*tmp = '\0';
				al = tmp - add;
				break;
			    } else
				*mp = m;
			}
			(void) unmeta_one(tmp, &cl);
			tl -= cl;
			tmp += cl;
			mp++;
		    }
		} else {
		    stopp = m->line;
		    stopl = m->llen;
		}
	    } else if (m->flags & CMF_RIGHT) {
		if (m->wlen < 0 && !m->llen && m->ralen == 1) {
		    for (tmp = add, tl = al, mp = ms; tl; ) {
			if (pattern_match(m->right, tmp, NULL, NULL)) {
			    if (*mp || (tmp == add && *tmp == '.')) {
				*tmp = '\0';
				al = tmp - add;
				break;
			    } else
				*mp = m;
			}
			(void) unmeta_one(tmp, &cl);
			tl -= cl;
			tmp += cl;
			mp++;
		    }
		} else if (m->llen) {
		    stopp = m->line;
		    stopl = m->llen;
		} else {
		    stopp = m->right;
		    stopl = m->ralen;
		}
	    } else {
		if (!m->lalen)
		    return "";

		stopp = m->left;
		stopl = m->lalen;
	    }
	    if (stopp)
		for (tmp = add, tl = al; tl >= stopl; ) {
		    if (pattern_match(stopp, tmp, NULL, NULL)) {
			*tmp = '\0';
			al = tmp - add;
			break;
		    }
		    (void) unmeta_one(tmp, &cl);
		    tl -= cl;
		    tmp += cl;
		}
	}
	if (*add)
	    return cfp_matcher_range(ms, add);
    }
    return add;
}

/*
 * ### This function call is skipped by _approximate, so "opt" probably means "optimize".
 */

static void
cfp_opt_pats(char **pats, char *matcher)
{
    char *add, **p, *q, *t, *s;

    if (!compprefix || !*compprefix)
	return;

    if (comppatmatch && *comppatmatch) {
	tokenize(t = rembslash(dyncat(compprefix, compsuffix)));
	remnulargs(t);
	if (haswilds(t))
	    return;
    }
    add = (char *) zhalloc(strlen(compprefix) * 2 + 1);
    for (s = compprefix, t = add; *s; s++) {
	if (*s != '\\' || !s[1] || s[1] == '*' || s[1] == '?' ||
	    s[1] == '<' || s[1] == '>' || s[1] == '(' || s[1] == ')' ||
	    s[1] == '[' || s[1] == ']' || s[1] == '|' || s[1] == '#' ||
	    s[1] == '^' || s[1] == '~' || s[1] == '=') {
	    if ((s == compprefix || s[-1] != '\\') &&
		(*s == '*' || *s == '?' || *s == '<' || *s == '>' ||
		 *s == '(' || *s == ')' || *s == '[' || *s == ']' ||
		 *s == '|' || *s == '#' || *s == '^' || *s == '~' ||
		 *s == '='))
		*t++ = '\\';
	    *t++ = *s;
	}
    }
    *t = '\0';
    for (p = pats; *add && (q = *p); p++) {
	if (*q) {
	    q = dupstring(q);
	    t = q + strlen(q) - 1;
	    if (*t == ')') {
		for (s = t--; t > q; t--)
		    if (*t == ')' || *t == '|' || *t == '~' || *t == '(')
			break;
		if (t != q && *t == '(')
		    *t = '\0';
	    }
	    for (; *q && *add; q++) {
		if (*q == '\\' && q[1]) {
		    for (s = add, q++; *s && *s != *q; s++);
		    *s = '\0';
		} else if (*q == '<') {
		    for (s = add; *s && !idigit(*s); s++);
		    *s = '\0';
		} else if (*q == '[') {
		    int not;
		    char *x = ++q;

		    if ((not = (*x == '!' || *x == '^')))
			x++;
		    for (; *x; x++) {
			if (x[1] == '-' && x[2]) {
			    char c1 = *x, c2 = x[2];

			    for (s = add; *s && (*x < c1 || *x > c2); s++);
			    *s = '\0';
			} else {
			    for (s = add; *s && *s != *x; s++);
			    *s = '\0';
			}
		    }
		} else if (*q != '?' && *q != '*' && *q != '(' && *q != ')' &&
			   *q != '|' && *q != '~' && *q != '#') {
		    for (s = add; *s && *s != *q; s++);
		    *s = '\0';
		}
	    }
	}
    }
    if (*add) {
	if (*matcher && !(add = cfp_matcher_pats(matcher, add)))
	    return;

	for (p = pats; *p; p++)
	    if (**p == '*')
		*p = dyncat(add, *p);
    }
}

static LinkList
cfp_bld_pats(UNUSED(int dirs), LinkList names, char *skipped, char **pats)
{
    LinkList ret = newlinklist();
    LinkNode node;
    int ol, sl = strlen(skipped), pl, dot;
    char **p, *o, *str;

    dot = (unset(GLOBDOTS) && compprefix && *compprefix == '.');
    for (node = firstnode(names); node; incnode(node)) {
	ol = strlen(o = (char *) getdata(node));
	for (p = pats; *p; p++) {
	    pl = strlen(*p);
	    str = (char *) zhalloc(ol + sl + pl + 1);
	    strcpy(str, o);
	    strcpy(str + ol, skipped);
	    strcpy(str + ol + sl, *p);
	    addlinknode(ret, str);
	    if (dot && **p != '.') {
		str = (char *) zhalloc(ol + sl + pl + 2);
		strcpy(str, o);
		strcpy(str + ol, skipped);
		str[ol + sl] = '.';
		strcpy(str + ol + sl + 1, *p);
		addlinknode(ret, str);
	    }
	}
    }
    return ret;
}

static LinkList
cfp_add_sdirs(LinkList final, LinkList orig, char *skipped,
	      char *sdirs, char **fake)
{
    int add = 0;

    if (*sdirs && (isset(GLOBDOTS) || (compprefix && *compprefix == '.'))) {
	if (!strcmp(sdirs, "yes") || !strcmp(sdirs, "true") ||
	    !strcmp(sdirs, "on") || !strcmp(sdirs, "1"))
	    add = 2;
	else if (!strcmp(sdirs, ".."))
	    add = 1;
    }
    if (add) {
	LinkNode node;
	char *s1 = dyncat(skipped, "..");
	char *s2 = (add == 2 ? dyncat(skipped, ".") : NULL), *m;

	for (node = firstnode(orig); node; incnode(node)) {
	    if ((m = (char *) getdata(node))) {
		addlinknode(final, dyncat(m, s1));
		if (s2)
		    addlinknode(final, dyncat(m, s2));
	    }
	}
    }
    if (fake && *fake) {
	LinkNode node;
	char *m, *f, *p, *t, *a, c;
	int sl = strlen(skipped) + 1;
	struct stat st1, st2;
	Patprog pprog;

	for (; (f = *fake); fake++) {
	    f = dupstring(f);
	    for (p = t = f; *p; p++) {
		if (*p == ':')
		    break;
		else if (*p == '\\' && p[1] == ':') {
		    /*
		     * strip quoted colons here; rely
		     * on tokenization to strip other backslashes
		     */
		    p++;
		}
		*t++ = *p;
	    }
	    if (*p) {
		*t = *p++ = '\0';
		if (!*p)
		    continue;

		queue_signals();	/* Protect PAT_STATIC */

		tokenize(f);
		pprog = patcompile(f, PAT_STATIC, NULL);
		untokenize(f);
		for (node = firstnode(orig); node; incnode(node)) {
		    if ((m = (char *) getdata(node)) &&
			((pprog ? pattry(pprog, m) : !strcmp(f, m)) ||
			 (!stat(f, &st1) && !stat((*m ? m : "."), &st2) &&
			  st1.st_dev == st2.st_dev &&
			  st1.st_ino == st2.st_ino))) {
			while (*p) {
			    while (*p && inblank(*p))
				p++;
			    if (!*p)
				break;
			    for (f = t = p; *p; p++) {
				if (inblank(*p))
				    break;
				else if (*p == '\\' && p[1])
				    p++;
				*t++ = *p;
			    }
			    c = *t;
			    *t = '\0';
			    a = (char *) zhalloc(strlen(m) + sl + strlen(f));
			    strcpy(a, m);
			    strcat(a, skipped);
			    strcat(a, f);
			    addlinknode(final, a);
			    *t = c;
			}
		    }
		}

		unqueue_signals();
	    }
	}
    }
    return final;
}

static LinkList
cf_pats(int dirs, int noopt, LinkList names, char **accept, char *skipped,
	char *matcher, char *sdirs, char **fake, char **pats)
{
    LinkList ret;
    char *dpats[2];

    if ((ret = cfp_test_exact(names, accept, skipped)))
	return cfp_add_sdirs(ret, names, skipped, sdirs, fake);

    if (dirs) {
	dpats[0] = "*(-/)";
	dpats[1] = NULL;
	pats = dpats;
    }
    if (!noopt)
	cfp_opt_pats(pats, matcher);

    return cfp_add_sdirs(cfp_bld_pats(dirs, names, skipped, pats),
			 names, skipped, sdirs, fake);
}

/*
 * This function looks at device/inode pairs to determine if
 * a file is one we should ignore because of its relationship
 * to the current or parent directory.
 *
 * We don't follow symbolic links here, because typically
 * a user will not want an explicit link to the current or parent
 * directory ignored.
 */
static void
cf_ignore(char **names, LinkList ign, char *style, char *path)
{
    int pl = strlen(path), tpar, tpwd, found;
    struct stat nst, est, st;
    char *n, *c, *e;

    tpar = !!strstr(style, "parent");
    if ((tpwd = !!strstr(style, "pwd")) && lstat(pwd, &est))
	tpwd = 0;

    if (!tpar && !tpwd)
	return;

    for (; (n = *names); names++) {
	if (!ztat(n, &nst, 1) && S_ISDIR(nst.st_mode)) {
	    if (tpwd && nst.st_dev == est.st_dev && nst.st_ino == est.st_ino) {
		addlinknode(ign, quotestring(n, QT_BACKSLASH));
		continue;
	    }
	    if (tpar && !strncmp((c = dupstring(n)), path, pl)) {
		found = 0;
		while ((e = strrchr(c, '/')) && e > c + pl) {
		    *e = '\0';
		    if (!ztat(c, &st, 0) &&
			st.st_dev == nst.st_dev && st.st_ino == nst.st_ino) {
			found = 1;
			break;
		    }
		}
		if (found || ((e = strrchr(c, '/')) && e > c + pl &&
			      !ztat(c, &st, 1) && st.st_dev == nst.st_dev &&
			      st.st_ino == nst.st_ino))
		    addlinknode(ign, quotestring(n, QT_BACKSLASH));
	    }
	}
    }
}

static LinkList
cf_remove_other(char **names, char *pre, int *amb)
{
    char *p;

    if ((p = strchr(pre, '/'))) {
	char **n;

	*p = '\0';
	pre = dyncat(pre, "/");
	*p = '/';

	for (n = names; *n; n++)
	    if (strpfx(pre, *n))
		break;

	if (*n) {
	    LinkList ret = newlinklist();

	    for (; *names; names++)
		if (strpfx(pre, *names))
		    addlinknode(ret, dupstring(*names));

	    *amb = 0;

	    return ret;
	} else {
	    if (!(p = *names++))
		*amb = 0;
	    else {
		char *q;

		if ((q = strchr((p = dupstring(p)), '/')))
		    *q = '\0';

                p = dyncat(p, "/");

		for (; *names; names++)
		    if (!strpfx(p, *names)) {
			*amb = 1;
			return NULL;
		    }
	    }
	}
    } else {
	if (!(p = *names++))
	    *amb = 0;
	else
	    for (; *names; names++)
		if (strcmp(p, *names)) {
		    *amb = 1;
		    return NULL;
		}
    }
    return NULL;
}

/*
 * SYNOPSIS:
 *     1. compfiles -p  parnam1 parnam2 skipped matcher sdirs parnam3 varargs [..varargs]
 *     2. compfiles -p- parnam1 parnam2 skipped matcher sdirs parnam3 varargs [..varargs]
 *     3. compfiles -P  parnam1 parnam2 skipped matcher sdirs parnam3 
 *
 *     1. Set parnam1 to an array of patterns....
 *        ${(P)parnam1} is an in/out parameter.
 *     2. Like #1 but without calling cfp_opt_pats().  (This is only used by _approximate.)
 *     3. Like #1 but varargs is implicitly set to  char *varargs[2] = { "*(-/)", NULL };.
 *
 *     parnam2 has to do with the accept-exact style (see cfp_test_exact()).
 */

static int
bin_compfiles(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    if (incompfunc != 1) {
	zwarnnam(nam, "can only be called from completion function");
	return 1;
    }
    if (**args != '-') {
	zwarnnam(nam, "missing option: %s", *args);
	return 1;
    }
    switch (args[0][1]) {
    case 'p':
    case 'P':
	if (args[0][2] && (args[0][2] != '-' || args[0][3])) {
	    zwarnnam(nam, "invalid option: %s", *args);
	    return 1;
	} else {
	    char **tmp;
	    LinkList l;

	    if (!args[1] || !args[2] || !args[3] || !args[4] || !args[5] ||
		!args[6] || (args[0][1] == 'p' && !args[7])) {
		zwarnnam(nam, "too few arguments");
		return 1;
	    }
	    queue_signals();
	    if (!(tmp = getaparam(args[1]))) {
		unqueue_signals();
		zwarnnam(nam, "unknown parameter: %s", args[1]);
		return 0;
	    }
	    for (l = newlinklist(); *tmp; tmp++)
		addlinknode(l, quotestring(*tmp, QT_BACKSLASH_PATTERN));
	    set_list_array(args[1], cf_pats((args[0][1] == 'P'), !!args[0][2],
					    l, getaparam(args[2]), args[3],
					    args[4], args[5],
					    getaparam(args[6]), args + 7));
	    unqueue_signals();
	    return 0;
	}
    case 'i':
	if (args[0][2]) {
	    zwarnnam(nam, "invalid option: %s", *args);
	    return 1;
	} else {
	    char **tmp;
	    LinkList l;

	    if (!args[1] || !args[2] || !args[3] || !args[4]) {
		zwarnnam(nam, "too few arguments");
		return 1;
	    }
	    if (args[5]) {
		zwarnnam(nam, "too many arguments");
		return 1;
	    }
	    queue_signals();
	    tmp = getaparam(args[2]);
	    l = newlinklist();
	    if (tmp)
		for (; *tmp; tmp++)
		    addlinknode(l, *tmp);
	    if (!(tmp = getaparam(args[1]))) {
		unqueue_signals();
		zwarnnam(nam, "unknown parameter: %s", args[1]);
		return 0;
	    }
	    cf_ignore(tmp, l, args[3], args[4]);
	    unqueue_signals();
	    set_list_array(args[2], l);
	    return 0;
	}
    case 'r':
	{
	    char **tmp;
	    LinkList l;
	    int ret = 0;

	    if (!args[1] || !args[2]) {
		zwarnnam(nam, "too few arguments");
		return 1;
	    }
	    if (args[3]) {
		zwarnnam(nam, "too many arguments");
		return 1;
	    }
	    queue_signals();
	    if (!(tmp = getaparam(args[1]))) {
		unqueue_signals();
		zwarnnam(nam, "unknown parameter: %s", args[1]);
		return 0;
	    }
	    if ((l = cf_remove_other(tmp, args[2], &ret)))
		set_list_array(args[1], l);
	    unqueue_signals();
	    return ret;
	}
    }
    zwarnnam(nam, "invalid option: %s", *args);
    return 1;
}

static int
bin_compgroups(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    Heap oldheap;
    char *n;

    if (incompfunc != 1) {
	zwarnnam(nam, "can only be called from completion function");
	return 1;
    }
    SWITCHHEAPS(oldheap, compheap) {
	while ((n = *args++)) {
	    endcmgroup(NULL);
	    begcmgroup(n, CGF_NOSORT|CGF_UNIQCON);
	    endcmgroup(NULL);
	    begcmgroup(n, CGF_UNIQALL);
	    endcmgroup(NULL);
	    begcmgroup(n, CGF_NOSORT|CGF_UNIQCON);
	    endcmgroup(NULL);
	    begcmgroup(n, CGF_UNIQALL);
	    endcmgroup(NULL);
	    begcmgroup(n, CGF_NOSORT);
	    endcmgroup(NULL);
	    begcmgroup(n, 0);
	}
    } SWITCHBACKHEAPS(oldheap);

    return 0;
}

static struct builtin bintab[] = {
    BUILTIN("comparguments", 0, bin_comparguments, 1, -1, 0, NULL, NULL),
    BUILTIN("compdescribe", 0, bin_compdescribe, 3, -1, 0, NULL, NULL),
    BUILTIN("compfiles", 0, bin_compfiles, 1, -1, 0, NULL, NULL),
    BUILTIN("compgroups", 0, bin_compgroups, 1, -1, 0, NULL, NULL),
    BUILTIN("compquote", 0, bin_compquote, 1, -1, 0, "p", NULL),
    BUILTIN("comptags", 0, bin_comptags, 1, -1, 0, NULL, NULL),
    BUILTIN("comptry", 0, bin_comptry, 0, -1, 0, NULL, NULL),
    BUILTIN("compvalues", 0, bin_compvalues, 1, -1, 0, NULL, NULL)
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
    memset(cadef_cache, 0, sizeof(cadef_cache));
    memset(cvdef_cache, 0, sizeof(cvdef_cache));

    memset(comptags, 0, sizeof(comptags));

    lasttaglevel = 0;

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
    int i;

    for (i = 0; i < MAX_CACACHE; i++)
	freecadef(cadef_cache[i]);
    for (i = 0; i < MAX_CVCACHE; i++)
	freecvdef(cvdef_cache[i]);

    for (i = 0; i < MAX_TAGS; i++)
	freectags(comptags[i]);

    return 0;
}

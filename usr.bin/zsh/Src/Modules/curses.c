/*
 * curses.c - curses windowing module for zsh
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 2007  Clint Adams
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Clint Adams or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Clint Adams and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Clint Adams and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Clint Adams and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#define ZSH_CURSES_SOURCE 1

#include "curses.mdh"
#include "curses.pro"

#ifndef MULTIBYTE_SUPPORT
# undef HAVE_GETCCHAR
# undef HAVE_SETCCHAR
# undef HAVE_WADDWSTR
# undef HAVE_WGET_WCH
# undef HAVE_WIN_WCH
# undef HAVE_NCURSESW_NCURSES_H
#endif

#ifdef ZSH_HAVE_CURSES_H
# include "../zshcurses.h"
#endif

#ifdef HAVE_SETCCHAR
# include <wchar.h>
#endif

#include <stdio.h>

enum zc_win_flags {
    /* Window is permanent (probably "stdscr") */
    ZCWF_PERMANENT = 0x0001,
    /* Scrolling enabled */
    ZCWF_SCROLL = 0x0002
};

typedef struct zc_win *ZCWin;

struct zc_win {
    WINDOW *win;
    char *name;
    int flags;
    LinkList children;
    ZCWin parent;
};

struct zcurses_namenumberpair {
    char *name;
    int number;
};

struct colorpairnode {
    struct hashnode node;
    short colorpair;
};
typedef struct colorpairnode *Colorpairnode;

typedef int (*zccmd_t)(const char *nam, char **args);
struct zcurses_subcommand {
    const char *name;
    zccmd_t cmd;
    int minargs;
    int maxargs;
};

static struct ttyinfo saved_tty_state;
static struct ttyinfo curses_tty_state;
static LinkList zcurses_windows;
static HashTable zcurses_colorpairs = NULL;
#ifdef NCURSES_MOUSE_VERSION
/*
 * The following is in principle a general set of flags, but
 * is currently only needed for mouse status.
 */
static int zcurses_flags;
#endif

#define ZCURSES_EINVALID 1
#define ZCURSES_EDEFINED 2
#define ZCURSES_EUNDEFINED 3

#define ZCURSES_UNUSED 1
#define ZCURSES_USED 2

#define ZCURSES_ATTRON 1
#define ZCURSES_ATTROFF 2

static int zc_errno, zc_color_phase=0;
static short next_cp=0;

enum {
    ZCF_MOUSE_ACTIVE,
    ZCF_MOUSE_MASK_CHANGED
};

static const struct zcurses_namenumberpair zcurses_attributes[] = {
    {"blink", A_BLINK},
    {"bold", A_BOLD},
    {"dim", A_DIM},
    {"reverse", A_REVERSE},
    {"standout", A_STANDOUT},
    {"underline", A_UNDERLINE},
    {NULL, 0}
};

static const struct zcurses_namenumberpair zcurses_colors[] = {
    {"black", COLOR_BLACK},
    {"red", COLOR_RED},
    {"green", COLOR_GREEN},
    {"yellow", COLOR_YELLOW},
    {"blue", COLOR_BLUE},
    {"magenta", COLOR_MAGENTA},
    {"cyan", COLOR_CYAN},
    {"white", COLOR_WHITE},
#ifdef HAVE_USE_DEFAULT_COLORS
    {"default", -1},
#endif
    {NULL, 0}
};

#ifdef NCURSES_MOUSE_VERSION
enum zcurses_mouse_event_types {
    ZCME_PRESSED,
    ZCME_RELEASED,
    ZCME_CLICKED,
    ZCME_DOUBLE_CLICKED,
    ZCME_TRIPLE_CLICKED
};

static const struct zcurses_namenumberpair zcurses_mouse_event_list[] = {
    {"PRESSED", ZCME_PRESSED},
    {"RELEASED", ZCME_RELEASED},
    {"CLICKED", ZCME_CLICKED},
    {"DOUBLE_CLICKED", ZCME_DOUBLE_CLICKED},
    {"TRIPLE_CLICKED", ZCME_TRIPLE_CLICKED},
    {NULL, 0}
};

struct zcurses_mouse_event {
    int button;
    int what;
    mmask_t event;
};

static const struct zcurses_mouse_event zcurses_mouse_map[] = {
    { 1, ZCME_PRESSED, BUTTON1_PRESSED },
    { 1, ZCME_RELEASED, BUTTON1_RELEASED },
    { 1, ZCME_CLICKED, BUTTON1_CLICKED },
    { 1, ZCME_DOUBLE_CLICKED, BUTTON1_DOUBLE_CLICKED },
    { 1, ZCME_TRIPLE_CLICKED, BUTTON1_TRIPLE_CLICKED },

    { 2, ZCME_PRESSED, BUTTON2_PRESSED },
    { 2, ZCME_RELEASED, BUTTON2_RELEASED },
    { 2, ZCME_CLICKED, BUTTON2_CLICKED },
    { 2, ZCME_DOUBLE_CLICKED, BUTTON2_DOUBLE_CLICKED },
    { 2, ZCME_TRIPLE_CLICKED, BUTTON2_TRIPLE_CLICKED },

    { 3, ZCME_PRESSED, BUTTON3_PRESSED },
    { 3, ZCME_RELEASED, BUTTON3_RELEASED },
    { 3, ZCME_CLICKED, BUTTON3_CLICKED },
    { 3, ZCME_DOUBLE_CLICKED, BUTTON3_DOUBLE_CLICKED },
    { 3, ZCME_TRIPLE_CLICKED, BUTTON3_TRIPLE_CLICKED },

    { 4, ZCME_PRESSED, BUTTON4_PRESSED },
    { 4, ZCME_RELEASED, BUTTON4_RELEASED },
    { 4, ZCME_CLICKED, BUTTON4_CLICKED },
    { 4, ZCME_DOUBLE_CLICKED, BUTTON4_DOUBLE_CLICKED },
    { 4, ZCME_TRIPLE_CLICKED, BUTTON4_TRIPLE_CLICKED },

#ifdef BUTTON5_PRESSED
    /* Not defined if only 32 bits available */
    { 5, ZCME_PRESSED, BUTTON5_PRESSED },
    { 5, ZCME_RELEASED, BUTTON5_RELEASED },
    { 5, ZCME_CLICKED, BUTTON5_CLICKED },
    { 5, ZCME_DOUBLE_CLICKED, BUTTON5_DOUBLE_CLICKED },
    { 5, ZCME_TRIPLE_CLICKED, BUTTON5_TRIPLE_CLICKED },
#endif
    { 0, 0, 0 }
};

static mmask_t zcurses_mouse_mask = ALL_MOUSE_EVENTS;

#endif

/* Autogenerated keypad string/number mapping*/
#include "curses_keys.h"

static char **
zcurses_pairs_to_array(const struct zcurses_namenumberpair *nnps)
{
    char **arr, **arrptr;
    int count;
    const struct zcurses_namenumberpair *nnptr;

    for (nnptr = nnps; nnptr->name; nnptr++)
	;
    count = nnptr - nnps;

    arrptr = arr = (char **)zhalloc((count+1) * sizeof(char *));

    for (nnptr = nnps; nnptr->name; nnptr++)
	*arrptr++ = dupstring(nnptr->name);
    *arrptr = NULL;

    return arr;
}

static const char *
zcurses_strerror(int err)
{
    static const char *errs[] = {
	"unknown error",
	"window name invalid",
	"window already defined",
	"window undefined",
	NULL };

    return errs[(err < 1 || err > 3) ? 0 : err];
}

static LinkNode
zcurses_getwindowbyname(const char *name)
{
    LinkNode node;
    ZCWin w;

    for (node = firstnode(zcurses_windows); node; incnode(node))
	if (w = (ZCWin)getdata(node), !strcmp(w->name, name))
	    return node;

    return NULL;
}

static LinkNode
zcurses_validate_window(char *win, int criteria)
{
    LinkNode target;

    if (win==NULL || strlen(win) < 1) {
	zc_errno = ZCURSES_EINVALID;
	return NULL;
    }

    target = zcurses_getwindowbyname(win);

    if (target && (criteria & ZCURSES_UNUSED)) {
	zc_errno = ZCURSES_EDEFINED;
	return NULL;
    }

    if (!target && (criteria & ZCURSES_USED)) {
	zc_errno = ZCURSES_EUNDEFINED;
	return NULL;
    }

    zc_errno = 0;
    return target;
}

static int
zcurses_free_window(ZCWin w)
{
    if (!(w->flags & ZCWF_PERMANENT) && delwin(w->win)!=OK)
	return 1;

    if (w->name)
	zsfree(w->name);

    if (w->children)
	freelinklist(w->children, (FreeFunc)NULL);

    zfree(w, sizeof(struct zc_win));

    return 0;
}

static struct zcurses_namenumberpair *
zcurses_attrget(UNUSED(WINDOW *w), char *attr)
{
    struct zcurses_namenumberpair *zca;

    if (!attr)
	return NULL;

    for(zca=(struct zcurses_namenumberpair *)zcurses_attributes;zca->name;zca++)
	if (!strcmp(attr, zca->name)) {
	    return zca;
	}

    return NULL;
}

static short
zcurses_color(const char *color)
{
    struct zcurses_namenumberpair *zc;

    for(zc=(struct zcurses_namenumberpair *)zcurses_colors;zc->name;zc++)
	if (!strcmp(color, zc->name)) {
	    return (short)zc->number;
	}

    return (short)-2;
}

static Colorpairnode
zcurses_colorget(const char *nam, char *colorpair)
{
    char *bg, *cp;
    short f, b;
    Colorpairnode cpn;

    /* zcurses_colorpairs is only initialised if color is supported */
    if (!zcurses_colorpairs)
	return NULL;

    if (zc_color_phase==1 ||
	!(cpn = (Colorpairnode) gethashnode2(zcurses_colorpairs, colorpair))) {
	zc_color_phase = 2;
	cp = ztrdup(colorpair);

	bg = strchr(cp, '/');
	if (bg==NULL) {
	    zsfree(cp);
	    return NULL;
	}

	*bg = '\0';        

        // cp/bg can be {number}/{number} or {name}/{name}

        if( cp[0] >= '0' && cp[0] <= '9' ) {
            f = atoi(cp);
        } else {
            f = zcurses_color(cp);
        }

        if( (bg+1)[0] >= '0' && (bg+1)[0] <= '9' ) {
            b = atoi(bg+1);
        } else {
            b = zcurses_color(bg+1);
        }

	if (f==-2 || b==-2) {
	    if (f == -2)
		zwarnnam(nam, "foreground color `%s' not known", cp);
	    if (b == -2)
		zwarnnam(nam, "background color `%s' not known", bg+1);
	    *bg = '/';
	    zsfree(cp);
	    return NULL;
	}
	*bg = '/';

	++next_cp;
	if (next_cp >= COLOR_PAIRS || init_pair(next_cp, f, b) == ERR)  {
	    zsfree(cp);
	    return NULL;
	}

	cpn = (Colorpairnode)zshcalloc(sizeof(struct colorpairnode));
	
	if (!cpn) {
	    zsfree(cp);
	    return NULL;
	}

	cpn->colorpair = next_cp;
	addhashnode(zcurses_colorpairs, cp, (void *)cpn);
    }

    return cpn;
}

static Colorpairnode cpn_match;

static void
zcurses_colornode(HashNode hn, int cp)
{
    Colorpairnode cpn = (Colorpairnode)hn;
    if (cpn->colorpair == (short)cp)
	cpn_match = cpn;
}

static Colorpairnode
zcurses_colorget_reverse(short cp)
{
    if (!zcurses_colorpairs)
	return NULL;

    cpn_match = NULL;
    scanhashtable(zcurses_colorpairs, 0, 0, 0,
		  zcurses_colornode, cp);
    return cpn_match;
}

static void
freecolorpairnode(HashNode hn)
{
    zsfree(hn->nam);
    zfree(hn, sizeof(struct colorpairnode));
}


/*************
 * Subcommands
 *************/

static int
zccmd_init(UNUSED(const char *nam), UNUSED(char **args))
{
    LinkNode stdscr_win = zcurses_getwindowbyname("stdscr");

    if (!stdscr_win) {
	ZCWin w = (ZCWin)zshcalloc(sizeof(struct zc_win));
	if (!w)
	    return 1;

	gettyinfo(&saved_tty_state);
	w->name = ztrdup("stdscr");
	w->win = initscr();
	if (w->win == NULL) {
	    zsfree(w->name);
	    zfree(w, sizeof(struct zc_win));
	    return 1;
	}
	w->flags = ZCWF_PERMANENT;
	zinsertlinknode(zcurses_windows, lastnode(zcurses_windows), (void *)w);
	if (start_color() != ERR) {
	    Colorpairnode cpn;

	    if(!zc_color_phase)
		zc_color_phase = 1;
	    zcurses_colorpairs = newhashtable(8, "zc_colorpairs", NULL);

	    zcurses_colorpairs->hash        = hasher;
	    zcurses_colorpairs->emptytable  = emptyhashtable;
	    zcurses_colorpairs->filltable   = NULL;
	    zcurses_colorpairs->cmpnodes    = strcmp;
	    zcurses_colorpairs->addnode     = addhashnode;
	    zcurses_colorpairs->getnode     = gethashnode2;
	    zcurses_colorpairs->getnode2    = gethashnode2;
	    zcurses_colorpairs->removenode  = removehashnode;
	    zcurses_colorpairs->disablenode = NULL;
	    zcurses_colorpairs->enablenode  = NULL;
	    zcurses_colorpairs->freenode    = freecolorpairnode;
	    zcurses_colorpairs->printnode   = NULL;

#ifdef HAVE_USE_DEFAULT_COLORS
	    use_default_colors();
#endif
	    /* Initialise the default color pair, always 0 */
	    cpn = (Colorpairnode)zshcalloc(sizeof(struct colorpairnode));
	    if (cpn) {
		cpn->colorpair = 0;
		addhashnode(zcurses_colorpairs,
			    ztrdup("default/default"), (void *)cpn);
	    }
	}
	/*
	 * We use cbreak mode because we don't want line buffering
	 * on input since we'd just need to loop over characters.
	 * We use noecho since the manual says that's the right
	 * thing to do with cbreak.
	 *
	 * Turn these on immediately to catch typeahead.
	 */
	cbreak();
	noecho();
	gettyinfo(&curses_tty_state);
    } else {
	settyinfo(&curses_tty_state);
    }
    return 0;
}


static int
zccmd_addwin(const char *nam, char **args)
{
    int nlines, ncols, begin_y, begin_x;
    ZCWin w;

    if (zcurses_validate_window(args[0], ZCURSES_UNUSED) == NULL &&
	zc_errno) {
	zerrnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0], 0);
	return 1;
    }

    nlines = atoi(args[1]);
    ncols = atoi(args[2]);
    begin_y = atoi(args[3]);
    begin_x = atoi(args[4]);

    w = (ZCWin)zshcalloc(sizeof(struct zc_win));
    if (!w)
	return 1;

    w->name = ztrdup(args[0]);
    if (args[5]) {
	LinkNode node;
	ZCWin worig;

	node = zcurses_validate_window(args[5], ZCURSES_USED);
	if (node == NULL) {
	    zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0],
		     0);
	    zsfree(w->name);
	    zfree(w, sizeof(struct zc_win));
	    return 1;
	}

	worig = (ZCWin)getdata(node);

	w->win = subwin(worig->win, nlines, ncols, begin_y, begin_x);
	if (w->win) {
	    w->parent = worig;
	    if (!worig->children)
		worig->children = znewlinklist();
	    zinsertlinknode(worig->children, lastnode(worig->children),
			    (void *)w);
	}
    } else {
	w->win = newwin(nlines, ncols, begin_y, begin_x);
    }

    if (w->win == NULL) {
	zwarnnam(nam, "failed to create window `%s'", w->name);
	zsfree(w->name);
	zfree(w, sizeof(struct zc_win));
	return 1;
    }

    zinsertlinknode(zcurses_windows, lastnode(zcurses_windows), (void *)w);

    return 0;
}

static int
zccmd_delwin(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;
    int ret = 0;

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    w = (ZCWin)getdata(node);

    if (w == NULL) {
	zwarnnam(nam, "record for window `%s' is corrupt", args[0]);
	return 1;
    }
    if (w->flags & ZCWF_PERMANENT) {
	zwarnnam(nam, "window `%s' can't be deleted", args[0]);
	return 1;
    }

    if (w->children && firstnode(w->children)) {
	zwarnnam(nam, "window `%s' has subwindows, delete those first",
		 w->name);
	return 1;
    }

    if (delwin(w->win)!=OK) {
	/*
	 * Not sure what to do here, but we are probably stuffed,
	 * so delete the window locally anyway.
	 */
	ret = 1;
    }

    if (w->parent) {
	/* Remove from parent's list of children */
	LinkList wpc = w->parent->children;
	LinkNode pcnode;
	for (pcnode = firstnode(wpc); pcnode; incnode(pcnode)) {
	    ZCWin child = (ZCWin)getdata(pcnode);
	    if (child == w) {
		remnode(wpc, pcnode);
		break;
	    }
	}
	DPUTS(pcnode == NULL, "BUG: child node not found in parent's children");
	/*
	 * We need to touch the parent to get the parent to refresh
	 * properly.
	 */
	touchwin(w->parent->win);
    }
    else
	touchwin(stdscr);

    if (w->name)
	zsfree(w->name);

    zfree((ZCWin)remnode(zcurses_windows, node), sizeof(struct zc_win));

    return ret;
}


static int
zccmd_refresh(const char *nam, char **args)
{
    WINDOW *win;
    int ret = 0;

    if (args[0]) {
	for (; *args; args++) {
	    LinkNode node;
	    ZCWin w;

	    node = zcurses_validate_window(args[0], ZCURSES_USED);
	    if (node == NULL) {
		zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0],
			 0);
		return 1;
	    }

	    w = (ZCWin)getdata(node);

	    if (w->parent) {
		/* This is what the manual says you have to do. */
		touchwin(w->parent->win);
	    }
	    win = w->win;
	    if (wnoutrefresh(win) != OK)
		ret = 1;
	}
	return (doupdate() != OK || ret);
    }
    else
    {
	return (wrefresh(stdscr) != OK) ? 1 : 0;
    }
}


static int
zccmd_move(const char *nam,  char **args)
{
    int y, x;
    LinkNode node;
    ZCWin w;

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    y = atoi(args[1]);
    x = atoi(args[2]);

    w = (ZCWin)getdata(node);

    if (wmove(w->win, y, x)!=OK)
	return 1;

    return 0;
}


static int
zccmd_clear(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    w = (ZCWin)getdata(node);

    if (!args[1]) {
	return werase(w->win) != OK;
    } else if (!strcmp(args[1], "redraw")) {
	return wclear(w->win) != OK;
    } else if (!strcmp(args[1], "eol")) {
	return wclrtoeol(w->win) != OK;
    } else if (!strcmp(args[1], "bot")) {
	return wclrtobot(w->win) != OK;
    } else {
	zwarnnam(nam, "`clear' expects `redraw', `eol' or `bot'");
	return 1;
    }
}


static int
zccmd_char(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;
#ifdef HAVE_SETCCHAR
    wchar_t c;
    cchar_t cc;
#endif

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    w = (ZCWin)getdata(node);

#ifdef HAVE_SETCCHAR
    if (mbrtowc(&c, args[1], MB_CUR_MAX, NULL) < 1)
	return 1;

    if (setcchar(&cc, &c, A_NORMAL, 0, NULL)==ERR)
	return 1;

    if (wadd_wch(w->win, &cc)!=OK)
	return 1;
#else
    if (waddch(w->win, (chtype)args[1][0])!=OK)
	return 1;
#endif

    return 0;
}


static int
zccmd_string(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;

#ifdef HAVE_WADDWSTR
    int clen;
    wint_t wc;
    wchar_t *wstr, *wptr;
    char *str = args[1];
#endif

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    w = (ZCWin)getdata(node);

#ifdef HAVE_WADDWSTR
    mb_charinit();
    wptr = wstr = zhalloc((strlen(str)+1) * sizeof(wchar_t));

    while (*str && (clen = mb_metacharlenconv(str, &wc))) {
	str += clen;
	if (wc == WEOF) /* TODO: replace with space? nicen? */
	    continue;
	*wptr++ = wc;
    }
    *wptr++ = L'\0';
    if (waddwstr(w->win, wstr)!=OK) {
	return 1;
    }
#else
    if (waddstr(w->win, args[1])!=OK)
	return 1;
#endif
    return 0;
}


static int
zccmd_border(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    w = (ZCWin)getdata(node);

    if (wborder(w->win, 0, 0, 0, 0, 0, 0, 0, 0)!=OK)
	return 1;

    return 0;
}


static int
zccmd_endwin(UNUSED(const char *nam), UNUSED(char **args))
{
    LinkNode stdscr_win = zcurses_getwindowbyname("stdscr");

    if (stdscr_win) {
	endwin();
	/* Restore TTY as it was before zcurses -i */
	settyinfo(&saved_tty_state);
	/*
	 * TODO: should I need the following?  Without it
	 * the screen stays messed up.  Presumably we are
	 * doing stuff with shttyinfo when we shouldn't really be.
	 */
	gettyinfo(&shttyinfo);
    }
    return 0;
}


static int
zccmd_attr(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;
    char **attrs;
    int ret = 0;

    if (!args[0])
	return 1;

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    w = (ZCWin)getdata(node);

    for(attrs = args+1; *attrs; attrs++) {
	if (strchr(*attrs, '/')) {
	    Colorpairnode cpn;
	    if ((cpn = zcurses_colorget(nam, *attrs)) == NULL ||
		wcolor_set(w->win, cpn->colorpair, NULL) == ERR)
		ret = 1;
	} else {
	    char *ptr;
	    int onoff;
	    struct zcurses_namenumberpair *zca;

	    switch(*attrs[0]) {
	    case '-':
		onoff = ZCURSES_ATTROFF;
		ptr = (*attrs) + 1;
		break;
	    case '+':
		onoff = ZCURSES_ATTRON;
		ptr = (*attrs) + 1;
		break;
	    default:
		onoff = ZCURSES_ATTRON;
		ptr = *attrs;
		break;
	    }
	    if ((zca = zcurses_attrget(w->win, ptr)) == NULL) {
		zwarnnam(nam, "attribute `%s' not known", ptr);
		ret = 1;
	    } else {
		switch(onoff) {
		    case ZCURSES_ATTRON:
			if (wattron(w->win, zca->number) == ERR)
			    ret = 1;
			break;
		    case ZCURSES_ATTROFF:
			if (wattroff(w->win, zca->number) == ERR)
			    ret = 1;
			break;
		}
	    }
	}
    }
    return ret;
}


static int
zccmd_bg(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;
    char **attrs;
    int ret = 0;
    chtype ch = 0;

    if (!args[0])
	return 1;

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    w = (ZCWin)getdata(node);

    for(attrs = args+1; *attrs; attrs++) {
	if (strchr(*attrs, '/')) {
	    Colorpairnode cpn;
	    if ((cpn = zcurses_colorget(nam, *attrs)) == NULL)
		ret = 1;
	    else if (cpn->colorpair >= 256) {
		/* pretty unlikely, but... */
		zwarnnam(nam, "bg color pair %s has index (%d) too large (max 255)",
			 cpn->node.nam, cpn->colorpair);
		ret = 1;
	    } else {
		ch |= COLOR_PAIR(cpn->colorpair);
	    }
	} else if (**attrs == '@') {
	    ch |= (*attrs)[1] == Meta ? (*attrs)[2] ^ 32 : (*attrs)[1];
	} else {
	    char *ptr;
	    int onoff;
	    struct zcurses_namenumberpair *zca;

	    switch(*attrs[0]) {
	    case '-':
		onoff = ZCURSES_ATTROFF;
		ptr = (*attrs) + 1;
		break;
	    case '+':
		onoff = ZCURSES_ATTRON;
		ptr = (*attrs) + 1;
		break;
	    default:
		onoff = ZCURSES_ATTRON;
		ptr = *attrs;
		break;
	    }
	    if ((zca = zcurses_attrget(w->win, ptr)) == NULL) {
		zwarnnam(nam, "attribute `%s' not known", ptr);
		ret = 1;
	    } else {
		switch(onoff) {
		    case ZCURSES_ATTRON:
			if (wattron(w->win, zca->number) == ERR)
			    ret = 1;
			break;
		    case ZCURSES_ATTROFF:
			if (wattroff(w->win, zca->number) == ERR)
			    ret = 1;
			break;
		}
	    }
	}
    }

    if (ret == 0)
	return wbkgd(w->win, ch) != OK;
    return ret;
}


static int
zccmd_scroll(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;
    int ret = 0;

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    w = (ZCWin)getdata(node);

    if (!strcmp(args[1], "on")) {
	if (scrollok(w->win, TRUE) == ERR)
	    return 1;
	w->flags |= ZCWF_SCROLL;
    } else if (!strcmp(args[1], "off")) {
	if (scrollok(w->win, FALSE) == ERR)
	    return 1;
	w->flags &= ~ZCWF_SCROLL;
    } else {
	char *endptr;
	zlong sl = zstrtol(args[1], &endptr, 10);
	if (*endptr) {
	    zwarnnam(nam, "scroll requires `on', `off' or integer: %s",
		     args[1]);
	    return 1;
	}
	if (!(w->flags & ZCWF_SCROLL))
	    scrollok(w->win, TRUE);
	if (wscrl(w->win, (int)sl) == ERR)
	    ret = 1;
	if (!(w->flags & ZCWF_SCROLL))
	    scrollok(w->win, FALSE);
    }

    return ret;
}


static int
zccmd_input(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;
    char *var;
    int keypadnum = -1;
    int nargs = arrlen(args);
#ifdef HAVE_WGET_WCH
    int ret;
    wint_t wi;
    VARARR(char, instr, 2*MB_CUR_MAX+1);
#else
    int ci;
    char instr[3];
#endif

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    w = (ZCWin)getdata(node);

    if (nargs >= 3) {
	keypad(w->win, TRUE);
    } else {
	keypad(w->win, FALSE);
    }

    if (nargs >= 4) {
#ifdef NCURSES_MOUSE_VERSION
	if (!(zcurses_flags & ZCF_MOUSE_ACTIVE) ||
	    (zcurses_flags & ZCF_MOUSE_MASK_CHANGED)) {
	    if (mousemask(zcurses_mouse_mask, NULL) == (mmask_t)ERR) {
		zwarnnam(nam, "current mouse mode is not supported");
		return 1;
	    }
	    zcurses_flags = (zcurses_flags & ~ZCF_MOUSE_MASK_CHANGED) |
		ZCF_MOUSE_ACTIVE;
	}
#else
	zwarnnam(nam, "mouse events are not supported");
	return 1;
#endif
    }
#ifdef NCURSES_MOUSE_VERSION
    else {
	if (zcurses_flags & ZCF_MOUSE_ACTIVE) {
	    mousemask((mmask_t)0, NULL);
	    zcurses_flags &= ~ZCF_MOUSE_ACTIVE;
	}
    }
#endif

    /*
     * Linux, OS X, FreeBSD documentation for wgetch() mentions:

       Programmers concerned about portability should be prepared for  either
       of  two cases: (a) signal receipt does not interrupt getch; (b) signal
       receipt interrupts getch and causes it to return ERR with errno set to
       EINTR.  Under the ncurses implementation, handled signals never inter-
       rupt getch.

     * Some observed behavior: wgetch() returns ERR with EINTR when a signal is
     * handled by the shell "trap" command mechanism. Observed that it returns
     * ERR twice, the second time without even attempting to repeat the
     * interrupted read. Third call will then begin reading again.
     *
     * Because of widespread of previous implementation that called wget*ch
     * possibly indefinitely many times after ERR/EINTR, and because of the
     * above observation, wget_wch call is repeated after each ERR/EINTR, but
     * errno is being reset (it wasn't) and the loop to all means should break.
     * Problem: the timeout may be waited twice.
     */
    errno = 0;

#ifdef HAVE_WGET_WCH
    while ((ret = wget_wch(w->win, &wi)) == ERR) {
	if (errno != EINTR || errflag || retflag || breaks || exit_pending)
	    break;
        errno = 0;
    }
    switch (ret) {
    case OK:
	ret = wctomb(instr, (wchar_t)wi);
	if (ret == 0) {
	    instr[0] = Meta;
	    instr[1] = '\0' ^ 32;
	    instr[2] = '\0';
	} else {
	    (void)metafy(instr, ret, META_NOALLOC);
	}
	break;

    case KEY_CODE_YES:
	*instr = '\0';
	keypadnum = (int)wi;
	break;

    case ERR:
    default:
	return 1;
    }
#else
    while ((ci = wgetch(w->win)) == ERR) {
	if (errno != EINTR || errflag || retflag || breaks || exit_pending)
	    return 1;
        errno = 0;
    }
    if (ci >= 256) {
	keypadnum = ci;
	*instr = '\0';
    } else {
	if (imeta(ci)) {
	    instr[0] = Meta;
	    instr[1] = (char)ci ^ 32;
	    instr[2] = '\0';
	} else {
	    instr[0] = (char)ci;
	    instr[1] = '\0';
	}
    }
#endif
    if (args[1])
	var = args[1];
    else
	var = "REPLY";
    if (!setsparam(var, ztrdup(instr)))
	return 1;
    if (nargs >= 3) {
	if (keypadnum > 0) {
#ifdef NCURSES_MOUSE_VERSION
	    if (nargs >= 4 && keypadnum == KEY_MOUSE) {
		MEVENT mevent;
		char digits[DIGBUFSIZE];
		LinkList margs;
		const struct zcurses_mouse_event *zcmmp = zcurses_mouse_map;

		if (!setsparam(args[2], ztrdup("MOUSE")))
		    return 1;
		if (getmouse(&mevent) == ERR) {
		    /*
		     * This may happen if the mouse wasn't in
		     * the window, so set the array to empty
		     * but return success unless the set itself
		     * failed.
		     */
		    return !setaparam(args[3], mkarray(NULL));
		}
		margs = newlinklist();
		sprintf(digits, "%d", (int)mevent.id);
		addlinknode(margs, dupstring(digits));
		sprintf(digits, "%d", mevent.x);
		addlinknode(margs, dupstring(digits));
		sprintf(digits, "%d", mevent.y);
		addlinknode(margs, dupstring(digits));
		sprintf(digits, "%d", mevent.z);
		addlinknode(margs, dupstring(digits));

		/*
		 * We only expect one event, but it doesn't hurt
		 * to keep testing.
		 */
		for (; zcmmp->button; zcmmp++) {
		    if (mevent.bstate & zcmmp->event) {
			const struct zcurses_namenumberpair *zcmelp =
			    zcurses_mouse_event_list;
			for (; zcmelp->name; zcmelp++) {
			    if (zcmelp->number == zcmmp->what) {
				char *evstr = zhalloc(strlen(zcmelp->name)+2);
				sprintf(evstr, "%s%d", zcmelp->name,
					zcmmp->button);
				addlinknode(margs, evstr);

				break;
			    }
			}
		    }
		}
		if (mevent.bstate & BUTTON_SHIFT)
		    addlinknode(margs, "SHIFT");
		if (mevent.bstate & BUTTON_CTRL)
		    addlinknode(margs, "CTRL");
		if (mevent.bstate & BUTTON_ALT)
		    addlinknode(margs, "ALT");
		if (!setaparam(args[3], zlinklist2array(margs, 1)))
		    return 1;
	    } else {
#endif
		const struct zcurses_namenumberpair *nnptr;
		char fbuf[DIGBUFSIZE+1];

		for (nnptr = keypad_names; nnptr->name; nnptr++) {
		    if (keypadnum == nnptr->number) {
			if (!setsparam(args[2], ztrdup(nnptr->name)))
			    return 1;
			return 0;
		    }
		}
		if (keypadnum > KEY_F0) {
		    /* assume it's a function key */
		    sprintf(fbuf, "F%d", keypadnum - KEY_F0);
		} else {
		    /* print raw number */
		    sprintf(fbuf, "%d", keypadnum);
		}
		if (!setsparam(args[2], ztrdup(fbuf)))
		    return 1;
#ifdef NCURSES_MOUSE_VERSION
	    }
#endif
	} else {
	    if (!setsparam(args[2], ztrdup("")))
		return 1;
	}
    }
#ifdef NCURSES_MOUSE_VERSION
    if (keypadnum != KEY_MOUSE && nargs >= 4)
	return !setaparam(args[3], mkarray(NULL));
#endif
    return 0;
}


static int
zccmd_timeout(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;
    int to;
    char *eptr;

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    w = (ZCWin)getdata(node);

    to = (int)zstrtol(args[1], &eptr, 10);
    if (*eptr) {
	zwarnnam(nam, "timeout requires an integer: %s", args[1]);
	return 1;
    }

#if defined(__sun__) && defined(__SVR4) && !defined(HAVE_USE_DEFAULT_COLORS)
    /*
     * On Solaris turning a timeout off seems to be problematic.
     * The following fixes it.  We test for Solaris without ncurses
     * (the last test) to be specific; this may turn up in other older
     * versions of curses, but it's difficult to test for.
     */
    if (to < 0) {
	nocbreak();
	cbreak();
    }
#endif
    wtimeout(w->win, to);
    return 0;
}


static int
zccmd_mouse(const char *nam, char **args)
{
#ifdef NCURSES_MOUSE_VERSION
    int ret = 0;

    for (; *args; args++) {
	if (!strcmp(*args, "delay")) {
	    char *eptr;
	    zlong delay;

	    if (!*++args ||
		((delay = zstrtol(*args, &eptr, 10)), eptr != NULL)) {
		zwarnnam(nam, "mouse delay requires an integer argument");
		return 1;
	    }
	    if (mouseinterval((int)delay) != OK)
		ret = 1;
	} else {
	    char *arg = *args;
	    int onoff = 1;
	    if (*arg == '+')
		arg++;
	    else if (*arg == '-') {
		arg++;
		onoff = 0;
	    }
	    if (!strcmp(arg, "motion")) {
		mmask_t old_mask = zcurses_mouse_mask;
		if (onoff)
		    zcurses_mouse_mask |= REPORT_MOUSE_POSITION;
		else
		    zcurses_mouse_mask &= ~REPORT_MOUSE_POSITION;
		if (old_mask != zcurses_mouse_mask)
		    zcurses_flags |= ZCF_MOUSE_MASK_CHANGED;
	    } else {
		zwarnnam(nam, "unrecognised mouse command: %s", *arg);
		return 1;
	    }
	}
    }

    return ret;
#else
    return 1;
#endif
}


static int
zccmd_position(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;
    int i, intarr[6];
    char **array, dbuf[DIGBUFSIZE];

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    w = (ZCWin)getdata(node);

    /* Look no pointers:  these are macros. */
    getyx(w->win, intarr[0], intarr[1]);
    if (intarr[0] == -1)
	return 1;
    getbegyx(w->win, intarr[2], intarr[3]);
    if (intarr[2] == -1)
	return 1;
    getmaxyx(w->win, intarr[4], intarr[5]);
    if (intarr[4] == -1)
	return 1;

    array = (char **)zalloc(7*sizeof(char *));
    for (i = 0; i < 6; i++) {
	sprintf(dbuf, "%d", intarr[i]);
	array[i] = ztrdup(dbuf);
    }
    array[6] = NULL;

    setaparam(args[1], array);
    return 0;
}


static int
zccmd_querychar(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;
    short cp;
    Colorpairnode cpn;
    const struct zcurses_namenumberpair *zattrp;
    LinkList clist;
#if defined(HAVE_WIN_WCH) && defined(HAVE_GETCCHAR)
    attr_t attrs;
    wchar_t c;
    cchar_t cc;
    int count;
    VARARR(char, instr, 2*MB_CUR_MAX+1);
#else
    chtype inc, attrs;
    char instr[3];
#endif

    node = zcurses_validate_window(args[0], ZCURSES_USED);
    if (node == NULL) {
	zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	return 1;
    }

    w = (ZCWin)getdata(node);

#if defined(HAVE_WIN_WCH) && defined(HAVE_GETCCHAR)
    if (win_wch(w->win, &cc) == ERR)
	return 1;

    if (getcchar(&cc, &c, &attrs, &cp, NULL) == ERR)
	return 1;
    /* Hmmm... I always get 0 for cp, whereas the following works... */
    cp = PAIR_NUMBER(winch(w->win));

    count = wctomb(instr, c);
    if (count == -1)
	return 1;
    (void)metafy(instr, count, META_NOALLOC);
#else
    inc = winch(w->win);
    /* I think the following is correct, the manual is a little terse */
    cp = PAIR_NUMBER(inc);
    inc &= A_CHARTEXT;
    if (imeta(inc)) {
	instr[0] = Meta;
	instr[1] = STOUC(inc ^ 32);
	instr[2] = '\0';
    } else {
	instr[0] = STOUC(inc);
	instr[1] = '\0';
    }
    attrs = inc;
#endif

    /*
     * Attribute numbers vary, so make a linked list.
     * This also saves us from doing the permanent allocation till
     * the end.
     */
    clist = newlinklist();
    /* First the (possibly multibyte) character itself. */
    addlinknode(clist, instr);
    /*
     * Next the colo[u]r.
     * We should be able to match it in the colorpair list, but
     * if some reason we can't, fail safe and output the number.
     */
    cpn = zcurses_colorget_reverse(cp);
    if (cpn) {
	addlinknode(clist, cpn->node.nam);
    } else {
	/* report color pair number */
	char digits[DIGBUFSIZE];
	sprintf(digits, "%d", (int)cp);
	addlinknode(clist, digits);
    }
    /* Now see what attributes are present. */
    for (zattrp = zcurses_attributes; zattrp->name; zattrp++) {
	if (attrs & zattrp->number)
	    addlinknode(clist, zattrp->name);
    }

    /* Turn this into an array and store it. */
    return !setaparam(args[1] ? args[1] : "reply", zlinklist2array(clist, 1));
}


static int
zccmd_touch(const char *nam, char **args)
{
    LinkNode node;
    ZCWin w;
    int ret = 0;

    for (; *args; args++) {
	node = zcurses_validate_window(args[0], ZCURSES_USED);
	if (node == NULL) {
	    zwarnnam(nam, "%s: %s", zcurses_strerror(zc_errno), args[0]);
	    return 1;
	}

	w = (ZCWin)getdata(node);
	if (touchwin(w->win) != OK)
	    ret = 1;
    }

    return ret;
}

static int
zccmd_resize(const char *nam, char **args)
{
#ifdef HAVE_RESIZE_TERM
    int y, x, do_endwin=0, do_save=1;
    LinkNode stdscr_win = zcurses_getwindowbyname("stdscr");

    if (stdscr_win) {
        y = atoi(args[0]);
        x = atoi(args[1]);
        if (args[2]) {
            if (0 == strcmp(args[2], "endwin")) {
                do_endwin=1;
            } else if (0 == strcmp(args[2], "endwin_nosave")) {
                do_endwin=1;
                do_save=0;
            } else if (0 == strcmp(args[2], "nosave")) {
                do_save=0;
            } else {
                zwarnnam(nam, "`resize' expects `endwin', `nosave' or `endwin_nosave' for third argument, if given");
            }
        }

        if (y == 0 && x == 0 && args[2] == NULL) {
            // Special case to just test that curses has resize_term. #ifdef
            // HAVE_RESIZE_TERM will result in return value 2 if resize_term
            // is not available.
            return 0;
        } else {
            // Without this call some window moves are inaccurate. Tested on
            // OS X ncurses 5.4, Homebrew ncursesw 6.0-2, Arch Linux ncursesw
            // 6.0, Ubuntu 14.04 ncurses 5.9, FreeBSD ncursesw.so.8
            //
            // On the other hand, the whole resize goal can be (from tests)
            // accomplished by calling endwin and refresh. But to secure any
            // future problems, resize_term is provided, and it is featured
            // with endwin, so that users have multiple options.
            if (do_endwin) {
                endwin();
            }

            if( resize_term( y, x ) == OK ) {
                // Things work without this, but we need to get out from
                // endwin (i.e. call refresh), and in theory store new
                // curses state (the resize might have changed it), which
                // should be presented to terminal only after refresh.
                if (do_endwin || do_save) {
                    ZCWin w;
                    w = (ZCWin)getdata(stdscr_win);
                    wnoutrefresh(w->win);
                    doupdate();
                }

                if (do_save) {
                    gettyinfo(&curses_tty_state);
                }
                return 0;
            } else {
                return 1;
            }
        }
    } else {
        return 1;
    }
#else
    return 2;
#endif
}

/*********************
  Main builtin handler
 *********************/

/**/
static int
bin_zcurses(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    char **saargs;
    struct zcurses_subcommand *zcsc;
    int num_args;

    struct zcurses_subcommand scs[] = {
	{"init", zccmd_init, 0, 0},
	{"addwin", zccmd_addwin, 5, 6},
	{"delwin", zccmd_delwin, 1, 1},
	{"refresh", zccmd_refresh, 0, -1},
	{"move", zccmd_move, 3, 3},
	{"clear", zccmd_clear, 1, 2},
	{"position", zccmd_position, 2, 2},
	{"char", zccmd_char, 2, 2},
	{"string", zccmd_string, 2, 2},
	{"border", zccmd_border, 1, 1},
	{"end", zccmd_endwin, 0, 0},
	{"attr", zccmd_attr, 2, -1},
	{"bg", zccmd_bg, 2, -1},
	{"scroll", zccmd_scroll, 2, 2},
	{"input", zccmd_input, 1, 4},
	{"timeout", zccmd_timeout, 2, 2},
	{"mouse", zccmd_mouse, 0, -1},
	{"querychar", zccmd_querychar, 1, 2},
	{"touch", zccmd_touch, 1, -1},
	{"resize", zccmd_resize, 2, 3},
	{NULL, (zccmd_t)0, 0, 0}
    };

    for(zcsc = scs; zcsc->name; zcsc++) {
	if(!strcmp(args[0], zcsc->name))
	    break;
    }

    if (zcsc->name == NULL) {
	zwarnnam(nam, "unknown subcommand: %s", args[0]);
	return 1;
    }

    saargs = args;
    while (*saargs++);
    num_args = saargs - (args + 2);

    if (num_args < zcsc->minargs) {
	zwarnnam(nam, "too few arguments for subcommand: %s", args[0]);
	return 1;
    } else if (zcsc->maxargs >= 0 && num_args > zcsc->maxargs) {
	zwarnnam(nam, "too many arguments for subcommand: %s", args[0]);
	return 1;
    }

    if (zcsc->cmd != zccmd_init && zcsc->cmd != zccmd_endwin &&
	!zcurses_getwindowbyname("stdscr")) {
	zwarnnam(nam, "command `%s' can't be used before `zcurses init'",
		 zcsc->name);
	return 1;
    }

    return zcsc->cmd(nam, args+1);
}


static struct builtin bintab[] = {
    BUILTIN("zcurses", 0, bin_zcurses, 1, -1, 0, "", NULL),
};


/*******************
 * Special variables
 *******************/

static char **
zcurses_colorsarrgetfn(UNUSED(Param pm))
{
    return zcurses_pairs_to_array(zcurses_colors);
}

static const struct gsu_array zcurses_colorsarr_gsu =
{ zcurses_colorsarrgetfn, arrsetfn, stdunsetfn };


static char **
zcurses_attrgetfn(UNUSED(Param pm))
{
    return zcurses_pairs_to_array(zcurses_attributes);
}

static const struct gsu_array zcurses_attrs_gsu =
{ zcurses_attrgetfn, arrsetfn, stdunsetfn };


static char **
zcurses_keycodesgetfn(UNUSED(Param pm))
{
    return zcurses_pairs_to_array(keypad_names);
}

static const struct gsu_array zcurses_keycodes_gsu =
{ zcurses_keycodesgetfn, arrsetfn, stdunsetfn };


static char **
zcurses_windowsgetfn(UNUSED(Param pm))
{
    LinkNode node;
    char **arr, **arrptr;
    int count = countlinknodes(zcurses_windows);

    arrptr = arr = (char **)zhalloc((count+1) * sizeof(char *));

    for (node = firstnode(zcurses_windows); node; incnode(node))
	*arrptr++ = dupstring(((ZCWin)getdata(node))->name);
    *arrptr = NULL;

    return arr;
}

static const struct gsu_array zcurses_windows_gsu =
{ zcurses_windowsgetfn, arrsetfn, stdunsetfn };


static zlong
zcurses_colorsintgetfn(UNUSED(Param pm))
{
    return COLORS;
}

static const struct gsu_integer zcurses_colorsint_gsu =
{ zcurses_colorsintgetfn, nullintsetfn, stdunsetfn };


static zlong
zcurses_colorpairsintgetfn(UNUSED(Param pm))
{
    return COLOR_PAIRS;
}

static const struct gsu_integer zcurses_colorpairsint_gsu =
{ zcurses_colorpairsintgetfn, nullintsetfn, stdunsetfn };


static struct paramdef partab[] = {
    SPECIALPMDEF("zcurses_colors", PM_ARRAY|PM_READONLY,
		 &zcurses_colorsarr_gsu, NULL, NULL),
    SPECIALPMDEF("zcurses_attrs", PM_ARRAY|PM_READONLY,
		 &zcurses_attrs_gsu, NULL, NULL),
    SPECIALPMDEF("zcurses_keycodes", PM_ARRAY|PM_READONLY,
		 &zcurses_keycodes_gsu, NULL, NULL),
    SPECIALPMDEF("zcurses_windows", PM_ARRAY|PM_READONLY,
		 &zcurses_windows_gsu, NULL, NULL),
    SPECIALPMDEF("ZCURSES_COLORS", PM_INTEGER|PM_READONLY,
		 &zcurses_colorsint_gsu, NULL, NULL),
    SPECIALPMDEF("ZCURSES_COLOR_PAIRS", PM_INTEGER|PM_READONLY,
		 &zcurses_colorpairsint_gsu, NULL, NULL)
};

/***************************
 * Standard module interface
 ***************************/


/*
 * boot_ is executed when the module is loaded.
 */

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
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
    return handlefeatures(m, &module_features, enables);
}

/**/
int
boot_(UNUSED(Module m))
{
    zcurses_windows = znewlinklist();

    return 0;
}

/**/
int
cleanup_(Module m)
{
    freelinklist(zcurses_windows, (FreeFunc) zcurses_free_window);
    if (zcurses_colorpairs)
	deletehashtable(zcurses_colorpairs);
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}

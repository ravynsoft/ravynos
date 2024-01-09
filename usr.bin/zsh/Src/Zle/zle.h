/*
 * zle.h - header file for line editor
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

#ifdef MULTIBYTE_SUPPORT
typedef wchar_t ZLE_CHAR_T;
typedef wchar_t *ZLE_STRING_T;
typedef wint_t   ZLE_INT_T;
#define ZLE_CHAR_SIZE	sizeof(wchar_t)


#define ZLEEOF	WEOF

/* Functions that operate on a ZLE_STRING_T. */
#define ZS_memcpy wmemcpy
#define ZS_memmove wmemmove
#define ZS_memset wmemset
#define ZS_memcmp wmemcmp
#define ZS_strlen wcslen
#define ZS_strcpy wcscpy
#define ZS_strncpy wcsncpy
#define ZS_strncmp wcsncmp
#define ZS_zarrdup wcs_zarrdup
#define ZS_width wcslen
#define ZS_strchr wcschr
#define ZS_memchr wmemchr

/*
 * Functions that operate on a metafied string.
 * These versions handle multibyte characters.
 */
#define ZMB_nicewidth(s)	mb_niceformat(s, NULL, NULL, 0)

/* Functions that operate on ZLE_CHAR_T. */
#define ZC_ialpha iswalpha
#define ZC_ialnum iswalnum
#define ZC_iblank wcsiblank
#define ZC_icntrl iswcntrl
#define ZC_idigit iswdigit
#define ZC_iident(x) wcsitype((x), IIDENT)
#define ZC_ilower iswlower
#define ZC_inblank iswspace
#define ZC_iupper iswupper
#define ZC_iword(x) wcsitype((x), IWORD)
#define ZC_ipunct iswpunct

#define ZC_tolower towlower
#define ZC_toupper towupper

#define LASTFULLCHAR	lastchar_wide
#define LASTFULLCHAR_T  ZLE_INT_T

/*
 * We may need to handle combining character alignment.
 * The following fix up the position of the cursor so that it
 * never ends up over a zero-width punctuation character following
 * an alphanumeric character.  The first is used if we were
 * moving the cursor left, the second if we were moving right or
 * if something under the cursor may have changed.
 */
#define CCLEFT()	alignmultiwordleft(&zlecs, 1)
#define CCRIGHT()	alignmultiwordright(&zlecs, 1)
/*
 * Same for any other position
 */
#define CCLEFTPOS(pos)	alignmultiwordleft(&pos, 1)
#define CCRIGHTPOS(pos)	alignmultiwordright(&pos, 1)
/*
 * Increment or decrement the cursor position, skipping over
 * combining characters.
 */
#define INCCS()		inccs()
#define DECCS()		deccs()
/*
 * Same for any other position.
 */
#define INCPOS(pos)	incpos(&pos)
#define DECPOS(pos)	decpos(&pos)

#else  /* Not MULTIBYTE_SUPPORT: old single-byte code */

typedef char ZLE_CHAR_T;
typedef char *ZLE_STRING_T;
typedef int ZLE_INT_T;
#define ZLE_CHAR_SIZE	sizeof(ZLE_CHAR_T)

#define ZLEEOF	EOF

/* Functions that operate on a ZLE_STRING_T. */
#define ZS_memcpy memcpy
#define ZS_memmove memmove
#define ZS_memset memset
#define ZS_memcmp memcmp
#define ZS_zarrdup zarrdup
#define ZS_width ztrlen
#define ZS_strchr strchr
#define ZS_memchr memchr

/*
 * Functions that operate on a metafied string.
 * These versions don't handle multibyte characters.
 */
#define ZMB_nicewidth	niceztrlen

#ifdef __GNUC__
static inline size_t ZS_strlen(ZLE_STRING_T s)
{ return strlen((char*)s); }
static inline ZLE_STRING_T ZS_strcpy(ZLE_STRING_T t, ZLE_STRING_T f)
{ return (ZLE_STRING_T)strcpy((char*)t, (char*)f); }
static inline ZLE_STRING_T ZS_strncpy(ZLE_STRING_T t, ZLE_STRING_T f, size_t l)
{ return (ZLE_STRING_T)strncpy((char*)t, (char*)f, l); }
static inline int ZS_strncmp(ZLE_STRING_T s1, ZLE_STRING_T s2, size_t l)
{ return strncmp((char*)s1, (char*)s2, l); }
#else
#define ZS_strlen(s) strlen((char*)(s))
#define ZS_strcpy(t,f) strcpy((char*)(t),(char*)(f))
#define ZS_strncpy(t,f,l) strncpy((char*)(t),(char*)(f),(l))
#define ZS_strncmp(s1,s2,l) strncmp((char*)(s1),(char*)(s2),(l))
#endif

/* Functions that operate on ZLE_CHAR_T. */
#define ZC_ialpha ialpha
#define ZC_ialnum ialnum
#define ZC_iblank iblank
#define ZC_icntrl icntrl
#define ZC_idigit idigit
#define ZC_iident iident
#define ZC_ilower islower
#define ZC_inblank inblank
#define ZC_iupper isupper
#define ZC_iword iword
#define ZC_ipunct ispunct

#define ZC_tolower tulower
#define ZC_toupper tuupper

#define LASTFULLCHAR	lastchar
#define LASTFULLCHAR_T	int

/* Combining character alignment: none in this mode */
#define CCLEFT()
#define CCRIGHT()
#define CCLEFTPOS(pos)
#define CCRIGHTPOS(pos)
/*
 * Increment or decrement the cursor position: simple in this case.
 */
#define INCCS()		((void)(zlecs++))
#define DECCS()		((void)(zlecs--))
/*
 * Same for any other position.
 */
#define INCPOS(pos)	((void)(pos++))
#define DECPOS(pos)	((void)(pos--))

#endif


typedef struct widget *Widget;
typedef struct thingy *Thingy;

/* widgets (ZLE functions) */

typedef int (*ZleIntFunc) _((char **));

struct widget {
    int flags;		/* flags (see below) */
    Thingy first;	/* `first' thingy that names this widget */
    union {
	ZleIntFunc fn;	/* pointer to internally implemented widget */
	char *fnnam;	/* name of the shell function for user-defined widget */
	struct {
	    ZleIntFunc fn; /* internal widget function to call */
	    char *wid;     /* name of widget to call */
	    char *func;    /* name of shell function to call */
	} comp;
    } u;
};

#define WIDGET_INT	(1<<0)	/* widget is internally implemented */
#define WIDGET_NCOMP    (1<<1)	/* new style completion widget */
#define ZLE_MENUCMP	(1<<2)	/* DON'T invalidate completion list */
#define ZLE_YANKAFTER	(1<<3)
#define ZLE_YANKBEFORE	(1<<4)
#define ZLE_YANK        (ZLE_YANKAFTER | ZLE_YANKBEFORE)
#define ZLE_LINEMOVE	(1<<5)	/* command is a line-oriented movement */
#define ZLE_VIOPER	(1<<6)  /* widget reads further keys so wait if prefix */
#define ZLE_LASTCOL     (1<<7)	/* command maintains lastcol correctly */
#define ZLE_KILL	(1<<8)
#define ZLE_KEEPSUFFIX	(1<<9)	/* DON'T remove added suffix */
#define ZLE_NOTCOMMAND  (1<<10)	/* widget should not alter lastcmd */
#define ZLE_ISCOMP      (1<<11)	/* usable for new style completion */
#define WIDGET_INUSE    (1<<12) /* widget is in use */
#define WIDGET_FREE     (1<<13) /* request to free when no longer in use */
#define ZLE_NOLAST	(1<<14)	/* widget should not alter lbindk */

/* thingies */

struct thingy {
    HashNode next;	/* next node in the hash chain */
    char *nam;		/* name of the thingy */
    int flags;		/* TH_* flags (see below) */
    int rc;		/* reference count */
    Widget widget;	/* widget named by this thingy */
    Thingy samew;	/* `next' thingy (circularly) naming the same widget */
};

/* DISABLED is (1<<0) */
#define TH_IMMORTAL	(1<<1)    /* can't refer to a different widget */

/*
 * Check if bindk refers to named thingy (a set of bare characters),
 * also checking the special .thingy widget.
 */
#define IS_THINGY(bindk, name)				\
    ((bindk) == t_ ## name || (bindk) == t_D ## name)

/* command modifier prefixes */

struct modifier {
    int flags;		/* MOD_* flags (see below) */
    int mult;		/* repeat count */
    int tmult;		/* repeat count actually being edited */
    int vibuf;		/* vi cut buffer */
    int base;		/* numeric base for digit arguments (usually 10) */
};

#define MOD_MULT  (1<<0)   /* a repeat count has been selected */
#define MOD_TMULT (1<<1)   /* a repeat count is being entered */
#define MOD_VIBUF (1<<2)   /* a vi cut buffer has been selected */
#define MOD_VIAPP (1<<3)   /* appending to the vi cut buffer */
#define MOD_NEG   (1<<4)   /* last command was negate argument */
#define MOD_NULL  (1<<5)   /* throw away text for the vi cut buffer */
#define MOD_CHAR  (1<<6)   /* force character-wise movement */
#define MOD_LINE  (1<<7)   /* force line-wise movement */

/* current modifier status */

#define zmult (zmod.mult)

/* flags to cut() and cuttext() and other front-ends */

#define CUT_FRONT   (1<<0)   /* Text goes in front of cut buffer */
#define CUT_REPLACE (1<<1)   /* Text replaces cut buffer */
#define CUT_RAW     (1<<2)   /*
			      * Raw character counts (not used in cut itself).
			      * This is used when the values are offsets
			      * into the zleline array rather than numbers
			      * of visible characters directly input by
			      * the user.
			      */
#define CUT_YANK    (1<<3)   /* vi yank: use register 0 instead of 1-9 */

/* undo system */

struct change {
    struct change *prev, *next;	/* adjacent changes */
    int flags;			/* see below */
    int hist;			/* history line being changed */
    int off;			/* offset of the text changes */
    ZLE_STRING_T del;		/* characters to delete */
    int dell;			/* no. of characters in del */
    ZLE_STRING_T ins;		/* characters to insert */
    int insl;			/* no. of characters in ins */
    int old_cs, new_cs;		/* old and new cursor positions */
    zlong changeno;             /* unique number of this change */
};

#define CH_NEXT (1<<0)   /* next structure is also part of this change */
#define CH_PREV (1<<1)   /* previous structure is also part of this change */

/* vi change handling for vi-repeat-change */

/*
 * Examination of the code suggests vichgbuf is consistently tied
 * to raw byte input, so it is left as a character array rather
 * than turned into wide characters.  In particular, when we replay
 * it we use ungetbytes().
 */
struct vichange {
    struct modifier mod; /* value of zmod associated with vi change */
    char *buf;           /* bytes for keys that make up the vi command */
    int bufsz, bufptr;   /* allocated and in use sizes of buf */
};

/* known thingies */

#define Th(X) (&thingies[X])

/* opaque keymap type */

typedef struct keymap *Keymap;

typedef void (*KeyScanFunc) _((char *, Thingy, char *, void *));

#define invicmdmode() (!strcmp(curkeymapname, "vicmd"))

/* Standard type of suffix removal. */

#ifdef MULTIBYTE_SUPPORT
#define NO_INSERT_CHAR	WEOF
#else
#define NO_INSERT_CHAR  256
#endif
#define removesuffix() iremovesuffix(NO_INSERT_CHAR, 0)

/*
 * Cut/kill buffer type.  The buffer itself is purely binary data, not
 * NUL-terminated.  len is a length count (N.B. number of characters,
 * not size in bytes).  flags uses the CUTBUFFER_* constants defined
 * below.
 */

struct cutbuffer {
    ZLE_STRING_T buf;
    size_t len;
    char flags;
};

typedef struct cutbuffer *Cutbuffer;

#define CUTBUFFER_LINE 1   /* for vi: buffer contains whole lines of data */

#define KRINGCTDEF 8   /* default number of buffers in the kill ring */

/* Types of completion. */

#define COMP_COMPLETE        0
#define COMP_LIST_COMPLETE   1
#define COMP_SPELL           2
#define COMP_EXPAND          3
#define COMP_EXPAND_COMPLETE 4
#define COMP_LIST_EXPAND     5
#define COMP_ISEXPAND(X) ((X) >= COMP_EXPAND)

/* Information about one brace run. */

typedef struct brinfo *Brinfo;

struct brinfo {
    Brinfo next;		/* next in list */
    Brinfo prev;		/* previous (only for closing braces) */
    char *str;			/* the string to insert */
    int pos;			/* original position */
    int qpos;			/* original position, with quoting */
    int curpos;			/* position for current match */
};

/* Convenience macros for the hooks */

#define LISTMATCHESHOOK    (zlehooks + 0)
#define COMPLETEHOOK       (zlehooks + 1)
#define BEFORECOMPLETEHOOK (zlehooks + 2)
#define AFTERCOMPLETEHOOK  (zlehooks + 3)
#define ACCEPTCOMPHOOK     (zlehooks + 4)
#define INVALIDATELISTHOOK (zlehooks + 5)

/* complete hook data struct */

typedef struct compldat *Compldat;

struct compldat {
    char *s;
    int lst;
    int incmd;
};

/* List completion matches. */

#define listmatches() runhookdef(LISTMATCHESHOOK, NULL)

/* Invalidate the completion list. */

#define invalidatelist() runhookdef(INVALIDATELISTHOOK, NULL)

/* Bit flags to setline */
enum {
    ZSL_COPY = 1,		/* Copy the argument, don't modify it */
    ZSL_TOEND = 2,		/* Go to the end of the new line */
};


/* Type arguments to addsuffix() */
enum suffixtype {
    SUFTYP_POSSTR,		/* String of characters to match */
    SUFTYP_NEGSTR,		/* String of characters not to match */
    SUFTYP_POSRNG,		/* Range of characters to match */
    SUFTYP_NEGRNG		/* Range of characters not to match */
};

/* Additional flags to suffixes */
enum suffixflags {
    SUFFLAGS_SPACE = 0x0001	/* Add a space when removing suffix */
};


/* Flags for the region_highlight structure */
enum {
    /* Offsets include predisplay */
    ZRH_PREDISPLAY = 1
};

/*
 * Attributes used for highlighting regions.
 * and mark.
 */
struct region_highlight {
    /* Attributes turned on in the region */
    zattr atr;
    /* Start of the region */
    int start;
    /* Start of the region in metafied ZLE line */
    int start_meta;
    /*
     * End of the region:  position of the first character not highlighted
     * (the same system as for point and mark).
     */
    int end;
    /* End of the region in metafied ZLE line */
    int end_meta;
    /*
     * Any of the flags defined above.
     */
    int flags;
    /*
     * User-settable "memo" key.  Metafied.
     */
    const char *memo;
};

/*
 * Count of special uses of region highlighting, which account
 * for the first few elements of region_highlights.
 * 0: region between point and mark
 * 1: isearch region
 * 2: suffix
 * 3: pasted text
 */
/* If you change this, update the documentation of zle_highlight/region_highlight
 * interaction in Doc/Zsh/zle.yo. */
#define N_SPECIAL_HIGHLIGHTS	(4)


#ifdef MULTIBYTE_SUPPORT
/*
 * We use a wint_t here, since we need an invalid character as a
 * placeholder and wint_t guarantees that we can use WEOF to do this.
 */
typedef wint_t REFRESH_CHAR;
#else
typedef char REFRESH_CHAR;
#endif

/*
 * Description of one screen cell in zle_refresh.c
 */
typedef struct {
    /*
     * The (possibly wide) character.
     * If atr contains TXT_MULTIWORD_MASK, an index into the set of multiword
     * symbols (only if MULTIBYTE_SUPPORT is present).
     */
    REFRESH_CHAR chr;
    /*
     * Its attributes.  'On' attributes (TXT_ATTR_ON_MASK) are
     * applied before the character, 'off' attributes (TXT_ATTR_OFF_MASK)
     * after it.  'On' attributes are present for all characters that
     * need the effect; 'off' attributes are only present for the
     * last character in the sequence.
     */
    zattr atr;
} REFRESH_ELEMENT;

/* A string of screen cells */
typedef REFRESH_ELEMENT *REFRESH_STRING;


#if defined(MULTIBYTE_SUPPORT) && defined(__STDC_ISO_10646__)
/*
 * With ISO 10646 there is a private range defined within
 * the encoding.  We use this for storing single-byte
 * characters in sections of strings that wouldn't convert to wide
 * characters.  This allows to preserve the string when transformed
 * back to multibyte strings.
 */

/* The start of the private range we use, for 256 characters */
#define ZSH_INVALID_WCHAR_BASE	(0xe000U)
/* Detect a wide character within our range */
#define ZSH_INVALID_WCHAR_TEST(x)			\
    ((unsigned)(x) >= ZSH_INVALID_WCHAR_BASE &&		\
     (unsigned)(x) <= (ZSH_INVALID_WCHAR_BASE + 255u))
/* Turn a wide character in that range back to single byte */
#define ZSH_INVALID_WCHAR_TO_CHAR(x)			\
    ((char)((unsigned)(x) - ZSH_INVALID_WCHAR_BASE))
/* Turn a wide character in that range to an integer */
#define ZSH_INVALID_WCHAR_TO_INT(x)			\
    ((int)((unsigned)(x) - ZSH_INVALID_WCHAR_BASE))
/* Turn a single byte character into a private wide character */
#define ZSH_CHAR_TO_INVALID_WCHAR(x)			\
    ((wchar_t)(STOUC(x) + ZSH_INVALID_WCHAR_BASE))
#endif


#ifdef DEBUG
#define METACHECK()		\
	DPUTS(zlemetaline == NULL, "line not metafied")
#define UNMETACHECK()		\
	DPUTS(zlemetaline != NULL, "line metafied")
#else
#define METACHECK()
#define UNMETACHECK()
#endif


typedef struct watch_fd *Watch_fd;

struct watch_fd {
    /* Function to call */
    char *func;
    /* Watched fd */
    int fd;
    /* 1 if func is called as a widget */
    int widget;
};

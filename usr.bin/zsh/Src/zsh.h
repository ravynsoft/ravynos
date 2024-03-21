/*
 * zsh.h - standard header file
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

/* A few typical macros */
#define minimum(a,b)  ((a) < (b) ? (a) : (b))

/*
 * Our longest integer type:  will be a 64 bit either if long already is,
 * or if we found some alternative such as long long.
 */
#ifdef ZSH_64_BIT_TYPE
typedef ZSH_64_BIT_TYPE zlong;
#if defined(ZLONG_IS_LONG_LONG) && defined(LLONG_MAX)
#define ZLONG_MAX LLONG_MAX
#else
#ifdef ZLONG_IS_LONG_64
#define ZLONG_MAX LONG_MAX
#else
/* umm... */
#define  ZLONG_MAX ((zlong)9223372036854775807)
#endif
#endif
#ifdef ZSH_64_BIT_UTYPE
typedef ZSH_64_BIT_UTYPE zulong;
#else
typedef unsigned zlong zulong;
#endif
#else
typedef long zlong;
typedef unsigned long zulong;
#define ZLONG_MAX LONG_MAX
#endif

/*
 * Work out how to define large integer constants that will fit
 * in a zlong.
 */
#if defined(ZSH_64_BIT_TYPE) || defined(LONG_IS_64_BIT)
/* We have some 64-bit type */
#ifdef LONG_IS_64_BIT
/* It's long */
#define ZLONG_CONST(x)  x ## l
#else
/* It's long long */
#ifdef ZLONG_IS_LONG_LONG
#define ZLONG_CONST(x)  x ## ll
#else
/*
 * There's some 64-bit type, but we don't know what it is.
 * We'll just cast it and hope the compiler does the right thing.
 */
#define ZLONG_CONST(x) ((zlong)x)
#endif
#endif
#else
/* We're stuck with long */
#define ZLONG_CONST(x) (x ## l)
#endif

/*
 * Double float support requires 64-bit alignment, so if longs and
 * pointers are less we need to pad out.
 */
#ifndef LONG_IS_64_BIT
# define PAD_64_BIT 1
#endif

/* math.c */
typedef struct {
    union {
	zlong l;
	double d;
    } u;
    int type;
} mnumber;

#define MN_INTEGER 1		/* mnumber is integer */
#define MN_FLOAT   2		/* mnumber is floating point */
#define MN_UNSET   4		/* mnumber not yet retrieved */

typedef struct mathfunc *MathFunc;
typedef mnumber (*NumMathFunc)(char *, int, mnumber *, int);
typedef mnumber (*StrMathFunc)(char *, char *, int);

struct mathfunc {
    MathFunc next;
    char *name;
    int flags;			/* MFF_* flags defined below */
    NumMathFunc nfunc;
    StrMathFunc sfunc;
    char *module;
    int minargs;
    int maxargs;
    int funcid;
};

/* Math function takes a string argument */
#define MFF_STR      1
/* Math function has been loaded from library */
#define MFF_ADDED    2
/* Math function is implemented by a shell function */
#define MFF_USERFUNC 4
/* When autoloading, enable all features in module */
#define MFF_AUTOALL  8


#define NUMMATHFUNC(name, func, min, max, id) \
    { NULL, name, 0, func, NULL, NULL, min, max, id }
#define STRMATHFUNC(name, func, id) \
    { NULL, name, MFF_STR, NULL, func, NULL, 0, 0, id }

/* Character tokens are sometimes casted to (unsigned char)'s.         * 
 * Unfortunately, some compilers don't correctly cast signed to        * 
 * unsigned promotions; i.e. (int)(unsigned char)((char) -1) evaluates * 
 * to -1, instead of 255 like it should.  We circumvent the troubles   * 
 * of such shameful delinquency by casting to a larger unsigned type   * 
 * then back down to unsigned char.                                    */

#ifdef BROKEN_SIGNED_TO_UNSIGNED_CASTING
# define STOUC(X)	((unsigned char)(unsigned short)(X))
#else
# define STOUC(X)	((unsigned char)(X))
#endif

/* Meta together with the character following Meta denotes the character *
 * which is the exclusive or of 32 and the character following Meta.     *
 * This is used to represent characters which otherwise has special      *
 * meaning for zsh.  These are the characters for which the imeta() test *
 * is true: the null character, and the characters from Meta to Marker.  */

#define Meta		((char) 0x83)

/* Note that the fourth character in DEFAULT_IFS is Meta *
 * followed by a space which denotes the null character. */

#define DEFAULT_IFS	" \t\n\203 "

/* As specified in the standard (POSIX 2008) */

#define DEFAULT_IFS_SH	" \t\n"

/*
 * Character tokens.
 * These should match the characters in ztokens, defined in lex.c
 */
#define Pound		((char) 0x84)
#define String		((char) 0x85)
#define Hat		((char) 0x86)
#define Star		((char) 0x87)
#define Inpar		((char) 0x88)
#define Inparmath	((char) 0x89)
#define Outpar		((char) 0x8a)
#define Outparmath	((char) 0x8b)
#define Qstring	        ((char) 0x8c)
#define Equals		((char) 0x8d)
#define Bar	      	((char) 0x8e)
#define Inbrace	        ((char) 0x8f)
#define Outbrace	((char) 0x90)
#define Inbrack	        ((char) 0x91)
#define Outbrack	((char) 0x92)
#define Tick		((char) 0x93)
#define Inang		((char) 0x94)
#define Outang		((char) 0x95)
#define OutangProc	((char) 0x96)
#define Quest		((char) 0x97)
#define Tilde		((char) 0x98)
#define Qtick		((char) 0x99)
#define Comma		((char) 0x9a)
#define Dash            ((char) 0x9b) /* Only in patterns */
#define Bang            ((char) 0x9c) /* Only in patterns */
/*
 * Marks the last of the group above.
 * Remaining tokens are even more special.
 */
#define LAST_NORMAL_TOK Bang
/*
 * Null arguments: placeholders for single and double quotes
 * and backslashes.
 */
#define Snull		((char) 0x9d)
#define Dnull		((char) 0x9e)
#define Bnull		((char) 0x9f)
/*
 * Backslash which will be returned to "\" instead of being stripped
 * when we turn the string into a printable format.
 */
#define Bnullkeep       ((char) 0xa0)
/*
 * Null argument that does not correspond to any character.
 * This should be last as it does not appear in ztokens and
 * is used to initialise the IMETA type in inittyptab().
 */
#define Nularg		((char) 0xa1)

/*
 * Take care to update the use of IMETA appropriately when adding
 * tokens here.
 */
/*
 * Marker is used in the following special circumstances:
 * - In paramsubst for rc_expand_param.
 * - In pattern character arrays as guaranteed not to mark a character in
 *   a string.
 * - In assignments with the ASSPM_KEY_VALUE flag set in order to
 *   mark that there is a key / value pair following.  If this
 *   comes from [key]=value the Marker is followed by a null;
 *   if from [key]+=value the Marker is followed by a '+' then a null.
 * All the above are local uses --- any case where the Marker has
 * escaped beyond the context in question is an error.
 */
#define Marker		((char) 0xa2)

/* chars that need to be quoted if meant literally */

#define SPECCHARS "#$^*()=|{}[]`<>?~;&\n\t \\\'\""

/* chars that need to be quoted for pattern matching */

#define PATCHARS "#^*()|[]<>?~\\"

/*
 * Check for a possibly tokenized dash.
 *
 * A dash only needs to be a token in a character range, [a-z], but
 * it's difficult in general to ensure that.  So it's turned into
 * a token at the usual point in the lexer.  However, we need
 * to check for a literal dash at many points.
 */
#define IS_DASH(x) ((x) == '-' || (x) == Dash)

/*
 * Types of quote.  This is used in various places, so care needs
 * to be taken when changing them.  (Oooh, don't you look surprised.)
 * - Passed to quotestring() to indicate style.  This is the ultimate
 *   destiny of most of the other uses of members of the enum.
 * - In paramsubst(), to count q's in parameter substitution.
 * - In the completion code, where we maintain a stack of quotation types.
 */
enum {
    /*
     * No quote.  Not a valid quote, but useful in the substitution
     * and completion code to indicate we're not doing any quoting.
     */
    QT_NONE,
    /* Backslash: \ */
    QT_BACKSLASH,
    /* Single quote: ' */
    QT_SINGLE,
    /* Double quote: " */
    QT_DOUBLE,
    /* Print-style quote: $' */
    QT_DOLLARS,
    /*
     * Backtick: `
     * Not understood by many parts of the code; here for a convenience
     * in those cases where we need to represent a complete set.
     */
    QT_BACKTICK,
    /*
     * Single quotes, but the default is not to quote unless necessary.
     * This is only useful as an argument to quotestring().
     */
    QT_SINGLE_OPTIONAL,
    /*
     * Only quote pattern characters.
     * ${(b)foo} guarantees that ${~foo} matches the string
     * contained in foo.
     */
    QT_BACKSLASH_PATTERN,
    /*
     * As QT_BACKSLASH, but a NULL string is shown as ''.
     */
    QT_BACKSLASH_SHOWNULL,
    /*
     * Quoting as produced by quotedzputs(), used for human
     * readability of parameter values.
     */
    QT_QUOTEDZPUTS
};

#define QT_IS_SINGLE(x)	((x) == QT_SINGLE || (x) == QT_SINGLE_OPTIONAL)

/*
 * Lexical tokens: unlike the character tokens above, these never
 * appear in strings and don't necessarily represent a single character.
 */

enum lextok {
    NULLTOK,		/* 0  */
    SEPER,
    NEWLIN,
    SEMI,
    DSEMI,
    AMPER,		/* 5  */
    INPAR,
    OUTPAR,
    DBAR,
    DAMPER,
    OUTANG,		/* 10 */
    OUTANGBANG,
    DOUTANG,
    DOUTANGBANG,
    INANG,
    INOUTANG,		/* 15 */
    DINANG,
    DINANGDASH,
    INANGAMP,
    OUTANGAMP,
    AMPOUTANG,		/* 20 */
    OUTANGAMPBANG,
    DOUTANGAMP,
    DOUTANGAMPBANG,
    TRINANG,
    BAR,		/* 25 */
    BARAMP,
    INOUTPAR,
    DINPAR,
    DOUTPAR,
    AMPERBANG,		/* 30 */
    SEMIAMP,
    SEMIBAR,
    DOUTBRACK,
    STRING,
    ENVSTRING,		/* 35 */
    ENVARRAY,
    ENDINPUT,
    LEXERR,

    /* Tokens for reserved words */
    BANG,	/* !         */
    DINBRACK,	/* [[        */	/* 40 */
    INBRACE,    /* {         */
    OUTBRACE,   /* }         */
    CASE,	/* case      */
    COPROC,	/* coproc    */
    DOLOOP,	/* do        */ /* 45 */
    DONE,	/* done      */
    ELIF,	/* elif      */
    ELSE,	/* else      */
    ZEND,	/* end       */
    ESAC,	/* esac      */ /* 50 */
    FI,		/* fi        */
    FOR,	/* for       */
    FOREACH,	/* foreach   */
    FUNC,	/* function  */
    IF,		/* if        */ /* 55 */
    NOCORRECT,	/* nocorrect */
    REPEAT,	/* repeat    */
    SELECT,	/* select    */
    THEN,	/* then      */
    TIME,	/* time      */ /* 60 */
    UNTIL,	/* until     */
    WHILE,	/* while     */
    TYPESET     /* typeset or similar */
};

/* Redirection types.  If you modify this, you may also have to modify *
 * redirtab in parse.c and getredirs() in text.c and the IS_* macros   *
 * below.                                                              */

enum {
    REDIR_WRITE,		/* > */
    REDIR_WRITENOW,		/* >| */
    REDIR_APP,		/* >> */
    REDIR_APPNOW,		/* >>| */
    REDIR_ERRWRITE,		/* &>, >& */
    REDIR_ERRWRITENOW,	/* >&| */
    REDIR_ERRAPP,		/* >>& */
    REDIR_ERRAPPNOW,		/* >>&| */
    REDIR_READWRITE,		/* <> */
    REDIR_READ,		/* < */
    REDIR_HEREDOC,		/* << */
    REDIR_HEREDOCDASH,	/* <<- */
    REDIR_HERESTR,		/* <<< */
    REDIR_MERGEIN,		/* <&n */
    REDIR_MERGEOUT,		/* >&n */
    REDIR_CLOSE,		/* >&-, <&- */
    REDIR_INPIPE,		/* < <(...) */
    REDIR_OUTPIPE		/* > >(...) */
};
#define REDIR_TYPE_MASK	(0x1f)
/* Redir using {var} syntax */
#define REDIR_VARID_MASK (0x20)
/* Mark here-string that came from a here-document */
#define REDIR_FROM_HEREDOC_MASK (0x40)

#define IS_WRITE_FILE(X)      ((X)>=REDIR_WRITE && (X)<=REDIR_READWRITE)
#define IS_APPEND_REDIR(X)    (IS_WRITE_FILE(X) && ((X) & 2))
#define IS_CLOBBER_REDIR(X)   (IS_WRITE_FILE(X) && ((X) & 1))
#define IS_ERROR_REDIR(X)     ((X)>=REDIR_ERRWRITE && (X)<=REDIR_ERRAPPNOW)
#define IS_READFD(X)          (((X)>=REDIR_READWRITE && (X)<=REDIR_MERGEIN) || (X)==REDIR_INPIPE)
#define IS_REDIROP(X)         ((X)>=OUTANG && (X)<=TRINANG)

/*
 * Values for the fdtable array.  They say under what circumstances
 * the fd will be close.  The fdtable is an unsigned char, so these are
 * #define's rather than an enum.
 */
/* Entry not used. */
#define FDT_UNUSED		0
/*
 * Entry used internally by the shell, should not be visible to other
 * processes.
 */
#define FDT_INTERNAL		1
/*
 * Entry visible to other processes, for example created using
 * the {varid}> file syntax.
 */
#define FDT_EXTERNAL		2
/*
 * Entry visible to other processes but controlled by a module.
 * The difference from FDT_EXTERNAL is that closing this using
 * standard fd syntax will fail as there is some tidying up that
 * needs to be done by the module's own mechanism.
 */
#define FDT_MODULE		3
/*
 * Entry used by output from the XTRACE option.
 */
#define FDT_XTRACE		4
/*
 * Entry used for file locking.
 */
#define FDT_FLOCK		5
/*
 * As above, but the fd is not marked for closing on exec,
 * so the shell can still exec the last process.
 */
#define FDT_FLOCK_EXEC		6
/*
 * Entry used by a process substitution.
 * This marker is not tested internally as we associated the file
 * descriptor with a job for closing.
 *
 * This is not used unless PATH_DEV_FD is defined.
 */
#define FDT_PROC_SUBST		7
/*
 * Mask to get the basic FDT type.
 */
#define FDT_TYPE_MASK		15

/*
 * Bit flag that fd is saved for later restoration.
 * Currently this is only use with FDT_INTERNAL.  We use this fact so as
 * not to have to mask checks against other types.
 */
#define FDT_SAVED_MASK		16

/* Flags for input stack */
#define INP_FREE      (1<<0)	/* current buffer can be free'd            */
#define INP_ALIAS     (1<<1)	/* expanding alias or history              */
#define INP_HIST      (1<<2)	/* expanding history                       */
#define INP_CONT      (1<<3)	/* continue onto previously stacked input  */
#define INP_ALCONT    (1<<4)	/* stack is continued from alias expn.     */
#define INP_HISTCONT  (1<<5)	/* stack is continued from history expn.   */
#define INP_LINENO    (1<<6)    /* update line number                      */
#define INP_APPEND    (1<<7)    /* Append new lines to allow backup        */
#define INP_RAW_KEEP  (1<<8)    /* Input needed in raw mode even if alias  */

/* Flags for metafy */
#define META_REALLOC	0
#define META_USEHEAP	1
#define META_STATIC	2
#define META_DUP	3
#define META_ALLOC	4
#define META_NOALLOC	5
#define META_HEAPDUP	6
#define META_HREALLOC	7

/* Context to save and restore (bit fields) */
enum {
    /* History mechanism */
    ZCONTEXT_HIST       = (1<<0),
    /* Lexical analyser */
    ZCONTEXT_LEX        = (1<<1),
    /* Parser */
    ZCONTEXT_PARSE      = (1<<2)
};

/* Report from entersubsh() to pass subshell info to addproc */
struct entersubsh_ret {
    /* Process group leader chosen by subshell, else -1 */
    int gleader;
    /* list_pipe_job setting used by subshell, else -1 */
    int list_pipe_job;
};

/**************************/
/* Abstract types for zsh */
/**************************/

typedef struct alias     *Alias;
typedef struct asgment   *Asgment;
typedef struct builtin   *Builtin;
typedef struct cmdnam    *Cmdnam;
typedef struct complist  *Complist;
typedef struct conddef   *Conddef;
typedef struct dirsav    *Dirsav;
typedef struct emulation_options *Emulation_options;
typedef struct execcmd_params *Execcmd_params;
typedef struct features  *Features;
typedef struct feature_enables  *Feature_enables;
typedef struct funcstack *Funcstack;
typedef struct funcwrap  *FuncWrap;
typedef struct hashnode  *HashNode;
typedef struct hashtable *HashTable;
typedef struct heap      *Heap;
typedef struct heapstack *Heapstack;
typedef struct histent   *Histent;
typedef struct hookdef   *Hookdef;
typedef struct imatchdata *Imatchdata;
typedef struct jobfile   *Jobfile;
typedef struct job       *Job;
typedef struct linkedmod *Linkedmod;
typedef struct linknode  *LinkNode;
typedef union  linkroot  *LinkList;
typedef struct module    *Module;
typedef struct nameddir  *Nameddir;
typedef struct options	 *Options;
typedef struct optname   *Optname;
typedef struct param     *Param;
typedef struct paramdef  *Paramdef;
typedef struct patstralloc  *Patstralloc;
typedef struct patprog   *Patprog;
typedef struct prepromptfn *Prepromptfn;
typedef struct process   *Process;
typedef struct redir     *Redir;
typedef struct reswd     *Reswd;
typedef struct shfunc    *Shfunc;
typedef struct timedfn   *Timedfn;
typedef struct value     *Value;

/********************************/
/* Definitions for linked lists */
/********************************/

/* linked list abstract data type */

struct linknode {
    LinkNode next;
    LinkNode prev;
    void *dat;
};

struct linklist {
    LinkNode first;
    LinkNode last;
    int flags;
};

union linkroot {
    struct linklist list;
    struct linknode node;
};

/* Macros for manipulating link lists */

#define firstnode(X)        ((X)->list.first)
#define lastnode(X)         ((X)->list.last)
#define peekfirst(X)        (firstnode(X)->dat)
#define peeklast(X)         (lastnode(X)->dat)
#define addlinknode(X,Y)    insertlinknode(X,lastnode(X),Y)
#define zaddlinknode(X,Y)   zinsertlinknode(X,lastnode(X),Y)
#define uaddlinknode(X,Y)   uinsertlinknode(X,lastnode(X),Y)
#define empty(X)            (firstnode(X) == NULL)
#define nonempty(X)         (firstnode(X) != NULL)
#define getaddrdata(X)      (&((X)->dat))
#define getdata(X)          ((X)->dat)
#define setdata(X,Y)        ((X)->dat = (Y))
#define nextnode(X)         ((X)->next)
#define prevnode(X)         ((X)->prev)
#define pushnode(X,Y)       insertlinknode(X,&(X)->node,Y)
#define zpushnode(X,Y)      zinsertlinknode(X,&(X)->node,Y)
#define incnode(X)          (X = nextnode(X))
#define decnode(X)          (X = prevnode(X))
#define firsthist()         (hist_ring? hist_ring->down->histnum : curhist)
#define setsizednode(X,Y,Z) (firstnode(X)[(Y)].dat = (void *) (Z))

/* stack allocated linked lists */

#define local_list0(N) union linkroot N
#define init_list0(N) \
    do { \
        (N).list.first = NULL; \
        (N).list.last = &(N).node; \
        (N).list.flags = 0; \
    } while (0)
#define local_list1(N) union linkroot N; struct linknode __n0
#define init_list1(N,V0) \
    do { \
        (N).list.first = &__n0; \
        (N).list.last = &__n0; \
        (N).list.flags = 0; \
        __n0.next = NULL; \
        __n0.prev = &(N).node; \
        __n0.dat = (void *) (V0); \
    } while (0)

/*************************************/
/* Specific elements of linked lists */
/*************************************/

typedef void (*voidvoidfnptr_t) _((void));

/*
 * Element of the prepromptfns list.
 */
struct prepromptfn {
    voidvoidfnptr_t func;
};


/*
 * Element of the timedfns list.
 */
struct timedfn {
    voidvoidfnptr_t func;
    time_t when;
};

/********************************/
/* Definitions for syntax trees */
/********************************/

/* These are control flags that are passed *
 * down the execution pipeline.            */
#define Z_TIMED	 (1<<0)	/* pipeline is being timed                   */
#define Z_SYNC	 (1<<1)	/* run this sublist synchronously       (;)  */
#define Z_ASYNC  (1<<2)	/* run this sublist asynchronously      (&)  */
#define Z_DISOWN (1<<3)	/* run this sublist without job control (&|) */
/* (1<<4) is used for Z_END, see the wordcode definitions */
/* (1<<5) is used for Z_SIMPLE, see the wordcode definitions */

/*
 * Condition types.
 *
 * Careful when changing these: both cond_binary_ops in text.c and
 * condstr in cond.c depend on these.  (The zsh motto is "two instances
 * are better than one".  Or something.)
 */

#define COND_NOT    0
#define COND_AND    1
#define COND_OR     2
#define COND_STREQ  3
#define COND_STRDEQ 4
#define COND_STRNEQ 5
#define COND_STRLT  6
#define COND_STRGTR 7
#define COND_NT     8
#define COND_OT     9
#define COND_EF    10
#define COND_EQ    11
#define COND_NE    12
#define COND_LT    13
#define COND_GT    14
#define COND_LE    15
#define COND_GE    16
#define COND_REGEX 17
#define COND_MOD   18
#define COND_MODI  19

typedef int (*CondHandler) _((char **, int));

struct conddef {
    Conddef next;		/* next in list                       */
    char *name;			/* the condition name                 */
    int flags;			/* see CONDF_* below                  */
    CondHandler handler;	/* handler function                   */
    int min;			/* minimum number of strings          */
    int max;			/* maximum number of strings          */
    int condid;			/* for overloading handler functions  */
    char *module;		/* module to autoload                 */
};

/* Condition is an infix */
#define CONDF_INFIX   1
/* Condition has been loaded from library */
#define CONDF_ADDED   2
/* When autoloading, enable all features in library */
#define CONDF_AUTOALL 4

#define CONDDEF(name, flags, handler, min, max, condid) \
    { NULL, name, flags, handler, min, max, condid, NULL }

/* Flags for redirections */

enum {
    /* Mark a here-string that came from a here-document */
    REDIRF_FROM_HEREDOC = 1
};

/* tree element for redirection lists */

struct redir {
    int type;
    int flags;
    int fd1, fd2;
    char *name;
    char *varid;
    char *here_terminator;
    char *munged_here_terminator;
};

/* The number of fds space is allocated for  *
 * each time a multio must increase in size. */
#define MULTIOUNIT 8

/* A multio is a list of fds associated with a certain fd.       *
 * Thus if you do "foo >bar >ble", the multio for fd 1 will have *
 * two fds, the result of open("bar",...), and the result of     *
 * open("ble",....).                                             */

/* structure used for multiple i/o redirection */
/* one for each fd open                        */

struct multio {
    int ct;			/* # of redirections on this fd                 */
    int rflag;			/* 0 if open for reading, 1 if open for writing */
    int pipe;			/* fd of pipe if ct > 1                         */
    int fds[MULTIOUNIT];	/* list of src/dests redirected to/from this fd */
};

/* lvalue for variable assignment/expansion */

struct value {
    int isarr;
    Param pm;		/* parameter node                      */
    int flags;		/* flags defined below                 */
    int start;		/* first element of array slice, or -1 */
    int end;		/* 1-rel last element of array slice, or -1 */
    char **arr;		/* cache for hash turned into array */
};

enum {
    VALFLAG_INV =	0x0001,	/* We are performing inverse subscripting */
    VALFLAG_EMPTY =	0x0002,	/* Subscripted range is empty */
    VALFLAG_SUBST =     0x0004  /* Substitution, so apply padding, case flags */
};

#define MAX_ARRLEN    262144

/********************************************/
/* Definitions for word code                 */
/********************************************/

typedef unsigned int wordcode;
typedef wordcode *Wordcode;

typedef struct funcdump *FuncDump;
typedef struct eprog *Eprog;

struct funcdump {
    FuncDump next;		/* next in list */
    dev_t dev;			/* device */
    ino_t ino;			/* indoe number */
    int fd;			/* file descriptor */
    Wordcode map;		/* pointer to header */
    Wordcode addr;		/* mapped region */
    int len;			/* length */
    int count;			/* reference count */
    char *filename;
};

/*
 * A note on the use of reference counts in Eprogs.
 *
 * When an Eprog is created, nref is set to -1 if the Eprog is on the
 * heap; then no attempt is ever made to free it.  (This information is
 * already present in EF_HEAP; we use the redundancy for debugging
 * checks.)
 *
 * Otherwise, nref is initialised to 1.  Calling freeprog() decrements
 * nref and frees the Eprog if the count is now zero.  When the Eprog
 * is in use, we call useeprog() at the start and freeprog() at the
 * end to increment and decrement the reference counts.  If an attempt
 * is made to free the Eprog from within, this will then take place
 * when execution is finished, typically in the call to freeeprog()
 * in execode().  If the Eprog was on the heap, neither useeprog()
 * nor freeeprog() has any effect.
 */
struct eprog {
    int flags;			/* EF_* below */
    int len;			/* total block length */
    int npats;			/* Patprog cache size */
    int nref;			/* number of references: delete when zero */
    Patprog *pats;		/* the memory block, the patterns */
    Wordcode prog;		/* memory block ctd, the code */
    char *strs;			/* memory block ctd, the strings */
    Shfunc shf;			/* shell function for autoload */
    FuncDump dump;		/* dump file this is in */
};

#define EF_REAL 1
#define EF_HEAP 2
#define EF_MAP  4
#define EF_RUN  8

typedef struct estate *Estate;

struct estate {
    Eprog prog;			/* the eprog executed */
    Wordcode pc;		/* program counter, current pos */
    char *strs;			/* strings from prog */
};

/* 
 * A binary tree of strings.
 *
 * Refer to the "Word code." comment at the top of Src/parse.c for details.
 */
typedef struct eccstr *Eccstr;
struct eccstr {
    /* Child pointers. */
    Eccstr left, right;

    /* String; pointer into to estate::strs. */
    char *str;

    /* Wordcode of a long string, as described in the Src/parse.c comment. */
    wordcode offs;

    /* Raw memory offset of str in estate::strs. */
    wordcode aoffs;

    /* 
     * ### The number of starts and ends of function definitions up to this point.
     *
     * String reuse may only happen between strings that have the same "nfunc" value.
     */
    int nfunc;

    /* Hash of str. */
    int hashval;
};

/*
 * Values for the "dup" parameter to ecgetstr().
 */
enum ec_dup_t {
    /* 
     * Make no promises about how the return value is allocated, except that
     * the caller does not need to explicitly free it.  It might be heap allocated,
     * a static string, or anything in between.
     */
    EC_NODUP = 0,

    /* Allocate the return value from the heap. */
    EC_DUP = 1,

    /* 
     * If the string contains tokens (as indicated by the least significant bit
     * of the wordcode), behave as EC_DUP; otherwise, as EC_NODUP.
     */
    EC_DUPTOK = 2
};

/* See comment at the top of Src/parse.c for details. */
#define WC_CODEBITS 5
#define wc_code(C)   ((C) & ((wordcode) ((1 << WC_CODEBITS) - 1)))
#define wc_data(C)   ((C) >> WC_CODEBITS)
#define wc_bdata(D)  ((D) << WC_CODEBITS)
#define wc_bld(C,D)  (((wordcode) (C)) | (((wordcode) (D)) << WC_CODEBITS))

#define WC_END      0
#define WC_LIST     1
#define WC_SUBLIST  2
#define WC_PIPE     3
#define WC_REDIR    4
#define WC_ASSIGN   5
#define WC_SIMPLE   6
#define WC_TYPESET  7
#define WC_SUBSH    8
#define WC_CURSH    9
#define WC_TIMED   10
#define WC_FUNCDEF 11
#define WC_FOR     12
#define WC_SELECT  13
#define WC_WHILE   14
#define WC_REPEAT  15
#define WC_CASE    16
#define WC_IF      17
#define WC_COND    18
#define WC_ARITH   19
#define WC_AUTOFN  20
#define WC_TRY     21

/* 
 * Increment as necessary.
 * 
 * If this exceeds 31, increment WC_CODEBITS.
 */
#define WC_COUNT   22

#define WCB_END()           wc_bld(WC_END, 0)

#define WC_LIST_TYPE(C)     wc_data(C)
#define Z_END               (1<<4) 
#define Z_SIMPLE            (1<<5)
#define WC_LIST_FREE        (6)	/* Next bit available in integer */
#define WC_LIST_SKIP(C)     (wc_data(C) >> WC_LIST_FREE)
#define WCB_LIST(T,O)       wc_bld(WC_LIST, ((T) | ((O) << WC_LIST_FREE)))

#define WC_SUBLIST_TYPE(C)  (wc_data(C) & ((wordcode) 3))
#define WC_SUBLIST_END      0
#define WC_SUBLIST_AND      1
#define WC_SUBLIST_OR       2
#define WC_SUBLIST_FLAGS(C) (wc_data(C) & ((wordcode) 0x1c))
#define WC_SUBLIST_COPROC   4
#define WC_SUBLIST_NOT      8
#define WC_SUBLIST_SIMPLE  16
#define WC_SUBLIST_FREE    (5)	/* Next bit available in integer */
#define WC_SUBLIST_SKIP(C)  (wc_data(C) >> WC_SUBLIST_FREE)
#define WCB_SUBLIST(T,F,O)  wc_bld(WC_SUBLIST, \
				   ((T) | (F) | ((O) << WC_SUBLIST_FREE)))

#define WC_PIPE_TYPE(C)     (wc_data(C) & ((wordcode) 1))
#define WC_PIPE_END         0
#define WC_PIPE_MID         1
#define WC_PIPE_LINENO(C)   (wc_data(C) >> 1)
#define WCB_PIPE(T,L)       wc_bld(WC_PIPE, ((T) | ((L) << 1)))

#define WC_REDIR_TYPE(C)    ((int)(wc_data(C) & REDIR_TYPE_MASK))
#define WC_REDIR_VARID(C)   ((int)(wc_data(C) & REDIR_VARID_MASK))
#define WC_REDIR_FROM_HEREDOC(C) ((int)(wc_data(C) & REDIR_FROM_HEREDOC_MASK))
#define WCB_REDIR(T)        wc_bld(WC_REDIR, (T))
/* Size of redir is 4 words if REDIR_VARID_MASK is set, else 3 */
#define WC_REDIR_WORDS(C)			\
    ((WC_REDIR_VARID(C) ? 4 : 3) +		\
     (WC_REDIR_FROM_HEREDOC(C) ? 2 : 0))

#define WC_ASSIGN_TYPE(C)   (wc_data(C) & ((wordcode) 1))
#define WC_ASSIGN_TYPE2(C)  ((wc_data(C) & ((wordcode) 2)) >> 1)
#define WC_ASSIGN_SCALAR    0
#define WC_ASSIGN_ARRAY     1
#define WC_ASSIGN_NEW       0
/*
 * In normal assignment, this indicate += to append.
 * In assignment following a typeset, where that's not allowed,
 * we overload this to indicate a variable without an
 * assignment.
 */
#define WC_ASSIGN_INC       1
#define WC_ASSIGN_NUM(C)    (wc_data(C) >> 2)
#define WCB_ASSIGN(T,A,N)   wc_bld(WC_ASSIGN, ((T) | ((A) << 1) | ((N) << 2)))

#define WC_SIMPLE_ARGC(C)   wc_data(C)
#define WCB_SIMPLE(N)       wc_bld(WC_SIMPLE, (N))

#define WC_TYPESET_ARGC(C)  wc_data(C)
#define WCB_TYPESET(N)      wc_bld(WC_TYPESET, (N))

#define WC_SUBSH_SKIP(C)    wc_data(C)
#define WCB_SUBSH(O)        wc_bld(WC_SUBSH, (O))

#define WC_CURSH_SKIP(C)    wc_data(C)
#define WCB_CURSH(O)        wc_bld(WC_CURSH, (O))

#define WC_TIMED_TYPE(C)    wc_data(C)
#define WC_TIMED_EMPTY      0
#define WC_TIMED_PIPE       1
#define WCB_TIMED(T)        wc_bld(WC_TIMED, (T))

#define WC_FUNCDEF_SKIP(C)  wc_data(C)
#define WCB_FUNCDEF(O)      wc_bld(WC_FUNCDEF, (O))

#define WC_FOR_TYPE(C)      (wc_data(C) & 3)
#define WC_FOR_PPARAM       0
#define WC_FOR_LIST         1
#define WC_FOR_COND         2
#define WC_FOR_SKIP(C)      (wc_data(C) >> 2)
#define WCB_FOR(T,O)        wc_bld(WC_FOR, ((T) | ((O) << 2)))

#define WC_SELECT_TYPE(C)   (wc_data(C) & 1)
#define WC_SELECT_PPARAM    0
#define WC_SELECT_LIST      1
#define WC_SELECT_SKIP(C)   (wc_data(C) >> 1)
#define WCB_SELECT(T,O)     wc_bld(WC_SELECT, ((T) | ((O) << 1)))

#define WC_WHILE_TYPE(C)    (wc_data(C) & 1)
#define WC_WHILE_WHILE      0
#define WC_WHILE_UNTIL      1
#define WC_WHILE_SKIP(C)    (wc_data(C) >> 1)
#define WCB_WHILE(T,O)      wc_bld(WC_WHILE, ((T) | ((O) << 1)))

#define WC_REPEAT_SKIP(C)   wc_data(C)
#define WCB_REPEAT(O)       wc_bld(WC_REPEAT, (O))

#define WC_TRY_SKIP(C)	    wc_data(C)
#define WCB_TRY(O)	    wc_bld(WC_TRY, (O))

#define WC_CASE_TYPE(C)     (wc_data(C) & 7)
#define WC_CASE_HEAD        0
#define WC_CASE_OR          1
#define WC_CASE_AND         2
#define WC_CASE_TESTAND     3
#define WC_CASE_FREE	    (3) /* Next bit available in integer */
#define WC_CASE_SKIP(C)     (wc_data(C) >> WC_CASE_FREE)
#define WCB_CASE(T,O)       wc_bld(WC_CASE, ((T) | ((O) << WC_CASE_FREE)))

#define WC_IF_TYPE(C)       (wc_data(C) & 3)
#define WC_IF_HEAD          0
#define WC_IF_IF            1
#define WC_IF_ELIF          2
#define WC_IF_ELSE          3
#define WC_IF_SKIP(C)       (wc_data(C) >> 2)
#define WCB_IF(T,O)         wc_bld(WC_IF, ((T) | ((O) << 2)))

#define WC_COND_TYPE(C)     (wc_data(C) & 127)
#define WC_COND_SKIP(C)     (wc_data(C) >> 7)
#define WCB_COND(T,O)       wc_bld(WC_COND, ((T) | ((O) << 7)))

#define WCB_ARITH()         wc_bld(WC_ARITH, 0)

#define WCB_AUTOFN()        wc_bld(WC_AUTOFN, 0)

/********************************************/
/* Definitions for job table and job control */
/********************************************/

/* Entry in filelist linked list in job table */

struct jobfile {
    /* Record to be deleted or closed */
    union {
	char *name; /* Name of file to delete */
	int fd;	    /* File descriptor to close */
    } u;
    /* Discriminant */
    int is_fd;
};

/* entry in the job table */

struct job {
    pid_t gleader;		/* process group leader of this job  */
    pid_t other;		/* subjob id (SUPERJOB)
				 * or subshell pid (SUBJOB) */
    int stat;                   /* see STATs below                   */
    char *pwd;			/* current working dir of shell when *
				 * this job was spawned              */
    struct process *procs;	/* list of processes                 */
    struct process *auxprocs;	/* auxiliary processes e.g multios   */
    LinkList filelist;		/* list of files to delete when done */
				/* elements are struct jobfile */
    int stty_in_env;		/* if STTY=... is present            */
    struct ttyinfo *ty;		/* the modes specified by STTY       */
};

#define STAT_CHANGED	(0x0001) /* status changed and not reported      */
#define STAT_STOPPED	(0x0002) /* all procs stopped or exited          */
#define STAT_TIMED	(0x0004) /* job is being timed                   */
#define STAT_DONE	(0x0008) /* job is done                          */
#define STAT_LOCKED	(0x0010) /* shell is finished creating this job, */
                                 /*   may be deleted from job table      */
#define STAT_NOPRINT	(0x0020) /* job was killed internally,           */
                                 /*   we don't want to show that         */
#define STAT_INUSE	(0x0040) /* this job entry is in use             */
#define STAT_SUPERJOB	(0x0080) /* job has a subjob                     */
#define STAT_SUBJOB	(0x0100) /* job is a subjob                      */
#define STAT_WASSUPER   (0x0200) /* was a super-job, sub-job needs to be */
				 /* deleted */
#define STAT_CURSH	(0x0400) /* last command is in current shell     */
#define STAT_NOSTTY	(0x0800) /* the tty settings are not inherited   */
				 /* from this job when it exits.         */
#define STAT_ATTACH	(0x1000) /* delay reattaching shell to tty       */
#define STAT_SUBLEADER  (0x2000) /* is super-job, but leader is sub-shell */

#define STAT_BUILTIN    (0x4000) /* job at tail of pipeline is a builtin */
#define STAT_SUBJOB_ORPHANED (0x8000)
                                 /* STAT_SUBJOB with STAT_SUPERJOB exited */
#define STAT_DISOWN     (0x10000) /* STAT_SUPERJOB with disown pending */

#define SP_RUNNING -1		/* fake status for jobs currently running */

struct timeinfo {
    long ut;                    /* user space time   */
    long st;                    /* system space time */
};

#define JOBTEXTSIZE 80

/* Size to initialise the job table to, and to increment it by when needed. */
#define MAXJOBS_ALLOC	(50)

/* node in job process lists */

#ifdef HAVE_GETRUSAGE
typedef struct rusage child_times_t;
#else
typedef struct timeinfo child_times_t;
#endif

struct process {
    struct process *next;
    pid_t pid;                  /* process id                       */
    char text[JOBTEXTSIZE];	/* text to print when 'jobs' is run */
    int status;			/* return code from waitpid/wait3() */
    child_times_t ti;
    struct timeval bgtime;	/* time job was spawned             */
    struct timeval endtime;	/* time job exited                  */
};

struct execstack {
    struct execstack *next;

    pid_t list_pipe_pid;
    int nowait;
    int pline_level;
    int list_pipe_child;
    int list_pipe_job;
    char list_pipe_text[JOBTEXTSIZE];
    int lastval;
    int noeval;
    int badcshglob;
    pid_t cmdoutpid;
    int cmdoutval;
    int use_cmdoutval;
    pid_t procsubstpid;
    int trap_return;
    int trap_state;
    int trapisfunc;
    int traplocallevel;
    int noerrs;
    int this_noerrexit;
    char *underscore;
};

struct heredocs {
    struct heredocs *next;
    int type;
    int pc;
    char *str;
};

struct dirsav {
    int dirfd, level;
    char *dirname;
    dev_t dev;
    ino_t ino;
};

#define MAX_PIPESTATS 256

/*******************************/
/* Definitions for Hash Tables */
/*******************************/

typedef void *(*VFunc) _((void *));
typedef void (*FreeFunc) _((void *));

typedef unsigned (*HashFunc)       _((const char *));
typedef void     (*TableFunc)      _((HashTable));
/*
 * Note that this is deliberately "char *", not "const char *",
 * since the AddNodeFunc is passed a pointer to a string that
 * will be stored and later freed.
 */
typedef void     (*AddNodeFunc)    _((HashTable, char *, void *));
typedef HashNode (*GetNodeFunc)    _((HashTable, const char *));
typedef HashNode (*RemoveNodeFunc) _((HashTable, const char *));
typedef void     (*FreeNodeFunc)   _((HashNode));
typedef int      (*CompareFunc)    _((const char *, const char *));

/* type of function that is passed to *
 * scanhashtable or scanmatchtable    */
typedef void     (*ScanFunc)       _((HashNode, int));
typedef void     (*ScanTabFunc)    _((HashTable, ScanFunc, int));

typedef void (*PrintTableStats) _((HashTable));

/* Hash table for standard open hashing. Instances of struct hashtable can be *
 * created only by newhashtable(). In fact, this function creates an instance *
 * of struct hashtableimpl, which is made of struct hashtable (public part)   *
 * and additional data members that are only accessible from hashtable.c.     */

struct hashtable {
    /* HASHTABLE DATA */
    int hsize;			/* size of nodes[]  (number of hash values)   */
    int ct;			/* number of elements                         */
    HashNode *nodes;		/* array of size hsize                        */
    void *tmpdata;

    /* HASHTABLE METHODS */
    HashFunc hash;		/* pointer to hash function for this table    */
    TableFunc emptytable;	/* pointer to function to empty table         */
    TableFunc filltable;	/* pointer to function to fill table          */
    CompareFunc cmpnodes;	/* pointer to function to compare two nodes     */
    AddNodeFunc addnode;	/* pointer to function to add new node        */
    GetNodeFunc getnode;	/* pointer to function to get an enabled node */
    GetNodeFunc getnode2;	/* pointer to function to get node            */
				/* (getnode2 will ignore DISABLED flag)       */
    RemoveNodeFunc removenode;	/* pointer to function to delete a node       */
    ScanFunc disablenode;	/* pointer to function to disable a node      */
    ScanFunc enablenode;	/* pointer to function to enable a node       */
    FreeNodeFunc freenode;	/* pointer to function to free a node         */
    ScanFunc printnode;		/* pointer to function to print a node        */
    ScanTabFunc scantab;	/* pointer to function to scan table          */
};

/* generic hash table node */

struct hashnode {
    HashNode next;		/* next in hash chain */
    char *nam;			/* hash key           */
    int flags;			/* various flags      */
};

/* The flag to disable nodes in a hash table.  Currently  *
 * you can disable builtins, shell functions, aliases and *
 * reserved words.                                        */
#define DISABLED	(1<<0)

/* node in shell option table */

struct optname {
    struct hashnode node;
    int optno;			/* option number */
};

/* node in shell reserved word hash table (reswdtab) */

struct reswd {
    struct hashnode node;
    int token;			/* corresponding lexer token */
};

/* node in alias hash table (aliastab) */

struct alias {
    struct hashnode node;
    char *text;			/* expansion of alias       */
    int inuse;			/* alias is being expanded  */
};

/* bit 0 of flags is the DISABLED flag */
/* is this alias global? */
#define ALIAS_GLOBAL	(1<<1)
/* is this an alias for suffix handling? */
#define ALIAS_SUFFIX	(1<<2)

/* structure for foo=bar assignments */

struct asgment {
    struct linknode node;
    char *name;
    int flags;
    union {
	char *scalar;
	LinkList array;
    } value;
};

/* Flags for flags element of asgment */
enum {
    /* Array value */
    ASG_ARRAY = 1,
    /* Key / value array pair */
    ASG_KEY_VALUE = 2
};

/*
 * Assignment is array?
 */
#define ASG_ARRAYP(asg) ((asg)->flags & ASG_ARRAY)

/*
 * Assignment has value?
 * If the assignment is an array, then it certainly has a value --- we
 * can only tell if there's an explicit assignment.
 */

#define ASG_VALUEP(asg) (ASG_ARRAYP(asg) ||			\
			 ((asg)->value.scalar != (char *)0))

/* node in command path hash table (cmdnamtab) */

struct cmdnam {
    struct hashnode node;
    union {
	char **name;		/* full pathname for external commands */
	char *cmd;		/* file name for hashed commands       */
    }
    u;
};

/* flag for nodes explicitly added to *
 * cmdnamtab with hash builtin        */
#define HASHED		(1<<1)

/* node in shell function hash table (shfunctab) */

struct shfunc {
    struct hashnode node;
    char *filename;             /* Name of file located in.
				   For not yet autoloaded file, name
				   of explicit directory, if not NULL. */
    zlong lineno;		/* line number in above file */
    Eprog funcdef;		/* function definition    */
    Eprog redir;                /* redirections to apply */
    Emulation_options sticky;   /* sticky emulation definitions, if any */
};

/* Shell function context types. */

#define SFC_NONE     0		/* no function running */
#define SFC_DIRECT   1		/* called directly from the user */
#define SFC_SIGNAL   2		/* signal handler */
#define SFC_HOOK     3		/* one of the special functions */
#define SFC_WIDGET   4		/* user defined widget */
#define SFC_COMPLETE 5		/* called from completion code */
#define SFC_CWIDGET  6		/* new style completion widget */
#define SFC_SUBST    7          /* used to perform substitution task */

/* tp in funcstack */

enum {
    FS_SOURCE,
    FS_FUNC,
    FS_EVAL
};

/* node in function stack */

struct funcstack {
    Funcstack prev;		/* previous in stack */
    char *name;			/* name of function/sourced file called */
    char *filename;		/* file function resides in */
    char *caller;		/* name of caller */
    zlong flineno;		/* line number in file */
    zlong lineno;		/* line offset from beginning of function */
    int tp;     		/* type of entry: sourced file, func, eval */
};

/* node in list of function call wrappers */

typedef int (*WrapFunc) _((Eprog, FuncWrap, char *));

struct funcwrap {
    FuncWrap next;
    int flags;
    WrapFunc handler;
    Module module;
};

#define WRAPF_ADDED 1

#define WRAPDEF(func) \
    { NULL, 0, func, NULL }

/*
 * User-defined hook arrays
 */

/* Name appended to function name to get hook array */
#define HOOK_SUFFIX	"_functions"
/* Length of that including NUL byte */
#define HOOK_SUFFIX_LEN	11

/* node in builtin command hash table (builtintab) */

/*
 * Handling of options.
 *
 * Option strings are standard in that a trailing `:' indicates
 * a mandatory argument.  In addition, `::' indicates an optional
 * argument which must immediately follow the option letter if it is present.
 * `:%' indicates an optional numeric argument which may follow
 * the option letter or be in the next word; the only test is
 * that the next character is a digit, and no actual conversion is done.
 */

#define MAX_OPS 128

/* Macros taking struct option * and char argument */
/* Option was set as -X */
#define OPT_MINUS(ops,c)	((ops)->ind[c] & 1)
/* Option was set as +X */
#define OPT_PLUS(ops,c)		((ops)->ind[c] & 2)
/*
 * Option was set any old how, maybe including an argument
 * (cheap test when we don't care).  Some bits of code
 * expect this to be 1 or 0.
 */
#define OPT_ISSET(ops,c)	((ops)->ind[c] != 0)
/* Option has an argument */
#define OPT_HASARG(ops,c)	((ops)->ind[c] > 3)
/* The argument for the option; not safe if it doesn't have one */
#define OPT_ARG(ops,c)		((ops)->args[((ops)->ind[c] >> 2) - 1])
/* Ditto, but safely returns NULL if there is no argument. */
#define OPT_ARG_SAFE(ops,c)	(OPT_HASARG(ops,c) ? OPT_ARG(ops,c) : NULL)

struct options {
    unsigned char ind[MAX_OPS];
    char **args;
    int argscount, argsalloc;
};

/* Flags to parseargs() */

enum {
    PARSEARGS_TOPLEVEL = 0x1,	/* Call to initialise shell */
    PARSEARGS_LOGIN    = 0x2	/* Shell is login shell */
};


/*
 * Handler arguments are: builtin name, null-terminated argument
 * list excluding command name, option structure, the funcid element from the
 * builtin structure.
 */

typedef int (*HandlerFunc) _((char *, char **, Options, int));
typedef int (*HandlerFuncAssign) _((char *, char **, LinkList, Options, int));
#define NULLBINCMD ((HandlerFunc) 0)

struct builtin {
    struct hashnode node;
    HandlerFunc handlerfunc;	/* pointer to function that executes this builtin     */
    int minargs;		/* minimum number of arguments                        */
    int maxargs;		/* maximum number of arguments, or -1 for no limit    */
    int funcid;			/* xbins (see above) for overloaded handlerfuncs      */
    char *optstr;		/* string of legal options (see execbuiltin())        */
    char *defopts;		/* options set by default for overloaded handlerfuncs */
};

#define BUILTIN(name, flags, handler, min, max, funcid, optstr, defopts) \
    { { NULL, name, flags }, handler, min, max, funcid, optstr, defopts }
#define BIN_PREFIX(name, flags) \
    BUILTIN(name, flags | BINF_PREFIX, NULLBINCMD, 0, 0, 0, NULL, NULL)

/* builtin flags */
/* DISABLE IS DEFINED AS (1<<0) */
#define BINF_PLUSOPTS		(1<<1)	/* +xyz legal */
#define BINF_PRINTOPTS		(1<<2)
#define BINF_ADDED		(1<<3)	/* is in the builtins hash table */
#define BINF_MAGICEQUALS	(1<<4)  /* needs automatic MAGIC_EQUAL_SUBST substitution */
#define BINF_PREFIX		(1<<5)
#define BINF_DASH		(1<<6)
#define BINF_BUILTIN		(1<<7)
#define BINF_COMMAND		(1<<8)
#define BINF_EXEC		(1<<9)
#define BINF_NOGLOB		(1<<10)
#define BINF_PSPECIAL		(1<<11)
/* Builtin option handling */
#define BINF_SKIPINVALID	(1<<12)	/* Treat invalid option as argument */
#define BINF_KEEPNUM		(1<<13) /* `[-+]NUM' can be an option */
#define BINF_SKIPDASH		(1<<14) /* Treat `-' as argument (maybe `+') */
#define BINF_DASHDASHVALID	(1<<15) /* Handle `--' even if SKIPINVALD */
#define BINF_CLEARENV		(1<<16) /* new process started with cleared env */
#define BINF_AUTOALL		(1<<17) /* autoload all features at once */
 /*
  * Handles options itself.  This is only useful if the option string for a
  * builtin with an empty option string.  It is used to indicate that "--"
  * does not terminate options.
  */
#define BINF_HANDLES_OPTS	(1<<18)
/*
 * Handles the assignment interface.  The argv list actually contains
 * two nested lists, the first of normal arguments, and the second of
 * assignment structures.
 */
#define BINF_ASSIGN		(1<<19)

/**
 * Parameters passed to execcmd().
 * These are not opaque --- they are also used by the pipeline manager.
 */
struct execcmd_params {
    LinkList args;		/* All command prefixes, arguments & options */
    LinkList redir;		/* Redirections */
    Wordcode beg;		/* The code at the start of the command */
    Wordcode varspc;		/* The code for assignment parsed as such */
    Wordcode assignspc;		/* The code for assignment parsed as typeset */
    int type;			/* The WC_* type of the command */
    int postassigns;		/* The number of assignspc assiguments */
    int htok;			/* tokens in parameter list */
};

struct module {
    struct hashnode node;
    union {
	void *handle;
	Linkedmod linked;
	char *alias;
    } u;
    LinkList autoloads;
    LinkList deps;
    int wrapper;
};

/* We are in the process of loading the module */
#define MOD_BUSY    (1<<0)
/*
 * We are in the process of unloading the module.
 * Note this is not needed to indicate a module is actually
 * unloaded: for that, the handle (or linked pointer) is set to NULL.
 */
#define MOD_UNLOAD  (1<<1)
/* We are in the process of setting up the module */
#define MOD_SETUP   (1<<2)
/* Module is statically linked into the main binary */
#define MOD_LINKED  (1<<3)
/* Module setup has been carried out (and module has not been finished) */
#define MOD_INIT_S  (1<<4)
/* Module boot has been carried out (and module has not been finished) */
#define MOD_INIT_B  (1<<5)
/* Module record is an alias */
#define MOD_ALIAS   (1<<6)

typedef int (*Module_generic_func) _((void));
typedef int (*Module_void_func) _((Module));
typedef int (*Module_features_func) _((Module, char ***));
typedef int (*Module_enables_func) _((Module, int **));

struct linkedmod {
    char *name;
    Module_void_func setup;
    Module_features_func features;
    Module_enables_func enables;
    Module_void_func boot;
    Module_void_func cleanup;
    Module_void_func finish;
};

/*
 * Structure combining all the concrete features available in
 * a module and with space for information about abstract features.
 */
struct features {
    /* List of builtins provided by the module and the size thereof */
    Builtin bn_list;
    int bn_size;
    /* List of conditions provided by the module and the size thereof */
    Conddef cd_list;
    int cd_size;
    /* List of math functions provided by the module and the size thereof */
    MathFunc mf_list;
    int mf_size;
    /* List of parameters provided by the module and the size thereof */
    Paramdef pd_list;
    int pd_size;
    /* Number of abstract features */
    int n_abstract;
};

/*
 * Structure describing enables for one feature.
 */
struct feature_enables {
    /* String feature to enable (N.B. no leading +/- allowed) */
    char *str;
    /* Optional compiled pattern for str sans +/-, NULL for string match */
    Patprog pat;
};

/* C-function hooks */

typedef int (*Hookfn) _((Hookdef, void *));

struct hookdef {
    Hookdef next;
    char *name;
    Hookfn def;
    int flags;
    LinkList funcs;
};

#define HOOKF_ALL 1

#define HOOKDEF(name, func, flags) { NULL, name, (Hookfn) func, flags, NULL }

/*
 * Types used in pattern matching.  Most of these longs could probably
 * happily be ints.
 */

struct patprog {
    long		startoff;  /* length before start of programme */
    long		size;	   /* total size from start of struct */
    long		mustoff;   /* offset to string that must be present */
    long		patmlen;   /* length of pure string or longest match */
    int			globflags; /* globbing flags to set at start */
    int			globend;   /* globbing flags set after finish */
    int			flags;	   /* PAT_* flags */
    int			patnpar;   /* number of active parentheses */
    char		patstartch;
};

struct patstralloc {
    int unmetalen;		/* Unmetafied length of trial string */
    int unmetalenp;		/* Unmetafied length of path prefix.
				   If 0, no path prefix. */
    char *alloced;		/* Allocated string, may be NULL */
    char *progstrunmeta;	/* Unmetafied pure string in pattern, cached */
    int progstrunmetalen;	/* Length of the foregoing */
};

/* Flags used in pattern matchers (Patprog) and passed down to patcompile */

#define PAT_HEAPDUP	0x0000	/* Dummy flag for default behavior */
#define PAT_FILE	0x0001	/* Pattern is a file name */
#define PAT_FILET	0x0002	/* Pattern is top level file, affects ~ */
#define PAT_ANY		0x0004	/* Match anything (cheap "*") */
#define PAT_NOANCH	0x0008	/* Not anchored at end */
#define PAT_NOGLD	0x0010	/* Don't glob dots */
#define PAT_PURES	0x0020	/* Pattern is a pure string: set internally */
#define PAT_STATIC	0x0040	/* Don't copy pattern to heap as per default */
#define PAT_SCAN	0x0080	/* Scanning, so don't try must-match test */
#define PAT_ZDUP        0x0100  /* Copy pattern in real memory */
#define PAT_NOTSTART	0x0200	/* Start of string is not real start */
#define PAT_NOTEND	0x0400	/* End of string is not real end */
#define PAT_HAS_EXCLUDP	0x0800	/* (internal): top-level path1~path2. */
#define PAT_LCMATCHUC   0x1000  /* equivalent to setting (#l) */

/**
 * Indexes into the array of active pattern characters.
 * This must match the array zpc_chars in pattern.c.
 */
enum zpc_chars {
    /*
     * These characters both terminate a pattern segment and
     * a pure string segment.
     */
    ZPC_SLASH,			/* / active as file separator */
    ZPC_NULL,			/* \0 as string terminator */
    ZPC_BAR,			/* | for "or" */
    ZPC_OUTPAR,			/* ) for grouping */
    ZPC_TILDE,			/* ~ for exclusion (extended glob) */
    ZPC_SEG_COUNT,              /* No. of the above characters */
    /*
     * These characters terminate a pure string segment.
     */
    ZPC_INPAR = ZPC_SEG_COUNT,  /* ( for grouping */
    ZPC_QUEST,			/* ? as wildcard */
    ZPC_STAR,			/* * as wildcard */
    ZPC_INBRACK,		/* [ for character class */
    ZPC_INANG,			/* < for numeric glob */
    ZPC_HAT,			/* ^ for exclusion (extended glob) */
    ZPC_HASH,			/* # for repetition (extended glob) */
    ZPC_BNULLKEEP,		/* Special backslashed null not removed */
    /*
     * These characters are only valid before a parenthesis
     */
    ZPC_NO_KSH_GLOB,
    ZPC_KSH_QUEST = ZPC_NO_KSH_GLOB, /* ? for ?(...) in KSH_GLOB */
    ZPC_KSH_STAR,               /* * for *(...) in KSH_GLOB */
    ZPC_KSH_PLUS,               /* + for +(...) in KSH_GLOB */
    ZPC_KSH_BANG,               /* ! for !(...) in KSH_GLOB */
    ZPC_KSH_BANG2,              /* ! for !(...) in KSH_GLOB, untokenised */
    ZPC_KSH_AT,                 /* @ for @(...) in KSH_GLOB */
    ZPC_COUNT			/* Number of special chararacters */
};

/*
 * Structure to save disables special characters for function scope.
 */
struct zpc_disables_save {
    struct zpc_disables_save *next;
    /*
     * Bit vector of ZPC_COUNT disabled characters.
     * We'll live dangerously and assume ZPC_COUNT is no greater
     * than the number of bits in an unsigned int.
     */
    unsigned int disables;
};

typedef struct zpc_disables_save *Zpc_disables_save;

/*
 * Special match types used in character classes.  These
 * are represented as tokens, with Meta added.  The character
 * class is represented as a metafied string, with only these
 * tokens special.  Note that an active leading "!" or "^" for
 * negation is not part of the string but is flagged in the
 * surrounding context.
 *
 * These types are also used in character and equivalence classes
 * in completion matching.
 *
 * This must be kept ordered by the array colon_stuffs in pattern.c.
 */
/* Special value for first definition */
#define PP_FIRST  1
/* POSIX-defined types:  [:alpha:] etc. */
#define PP_ALPHA  1
#define PP_ALNUM  2
#define PP_ASCII  3
#define PP_BLANK  4
#define PP_CNTRL  5
#define PP_DIGIT  6
#define PP_GRAPH  7
#define PP_LOWER  8
#define PP_PRINT  9
#define PP_PUNCT  10
#define PP_SPACE  11
#define PP_UPPER  12
#define PP_XDIGIT 13
/* Zsh additions:  [:IDENT:] etc. */
#define PP_IDENT  14
#define PP_IFS    15
#define PP_IFSSPACE   16
#define PP_WORD   17
#define PP_INCOMPLETE 18
#define PP_INVALID 19
/* Special value for last definition */
#define PP_LAST   19

/* Unknown type.  Not used in a valid token. */
#define PP_UNKWN  20
/* Range: token followed by the (possibly multibyte) start and end */
#define PP_RANGE  21

/*
 * Argument to get_match_ret() in glob.c
 */
struct imatchdata {
    /* Metafied trial string */
    char *mstr;
    /* Its length */
    int mlen;
    /* Unmetafied string */
    char *ustr;
    /* Its length */
    int ulen;
    /* Flags (SUB_*) */
    int flags;
    /* Replacement string (metafied) */
    char *replstr;
    /*
     * List of bits of matches to concatenate with replacement string.
     * The data is a struct repldata.  It is not used in cases like
     * ${...//#foo/bar} even though SUB_GLOBAL is set, since the match
     * is anchored.  It goes on the heap.
     */
    LinkList repllist;
};

/* Globbing flags: lower 8 bits gives approx count */
#define GF_LCMATCHUC	0x0100
#define GF_IGNCASE	0x0200
#define GF_BACKREF	0x0400
#define GF_MATCHREF	0x0800
#define GF_MULTIBYTE	0x1000	/* Use multibyte if supported by build */

enum {
    /* Valid multibyte character from charref */
    ZMB_VALID,
    /* Incomplete multibyte character from charref */
    ZMB_INCOMPLETE,
    /* Invalid multibyte character charref */
    ZMB_INVALID
};

/* Dummy Patprog pointers. Used mainly in executable code, but the
 * pattern code needs to know about it, too. */

#define dummy_patprog1 ((Patprog) 1)
#define dummy_patprog2 ((Patprog) 2)

/* standard node types for get/set/unset union in parameter */

/*
 * note non-standard const in pointer declaration: structures are
 * assumed to be read-only.
 */
typedef const struct gsu_scalar *GsuScalar;
typedef const struct gsu_integer *GsuInteger;
typedef const struct gsu_float *GsuFloat;
typedef const struct gsu_array *GsuArray;
typedef const struct gsu_hash *GsuHash;

struct gsu_scalar {
    char *(*getfn) _((Param));
    void (*setfn) _((Param, char  *));
    void (*unsetfn) _((Param, int));
};

struct gsu_integer {
    zlong (*getfn) _((Param));
    void (*setfn) _((Param, zlong));
    void (*unsetfn) _((Param, int));
};

struct gsu_float {
    double (*getfn) _((Param));
    void (*setfn) _((Param, double));
    void (*unsetfn) _((Param, int));
};

struct gsu_array {
    char **(*getfn) _((Param));
    void (*setfn) _((Param, char **));
    void (*unsetfn) _((Param, int));
};

struct gsu_hash {
    HashTable (*getfn) _((Param));
    void (*setfn) _((Param, HashTable));
    void (*unsetfn) _((Param, int));
};


/* node used in parameter hash table (paramtab) */

struct param {
    struct hashnode node;

    /* the value of this parameter */
    union {
	void *data;		/* used by special parameter functions    */
	char **arr;		/* value if declared array   (PM_ARRAY)   */
	char *str;		/* value if declared string  (PM_SCALAR)  */
	zlong val;		/* value if declared integer (PM_INTEGER) */
	zlong *valptr;		/* value if special pointer to integer */
	double dval;		/* value if declared float
				                    (PM_EFLOAT|PM_FFLOAT) */
        HashTable hash;		/* value if declared assoc   (PM_HASHED)  */
    } u;

    /*
     * get/set/unset methods.
     *
     * Unlike the data union, this points to a single instance
     * for every type (although there are special types, e.g.
     * tied arrays have a different gsu_scalar struct from the
     * normal one).  It's really a poor man's vtable.
     */
    union {
	GsuScalar s;
	GsuInteger i;
	GsuFloat f;
	GsuArray a;
	GsuHash h;
    } gsu;

    int base;			/* output base or floating point prec    */
    int width;			/* field width                           */
    char *env;			/* location in environment, if exported  */
    char *ename;		/* name of corresponding environment var */
    Param old;			/* old struct for use with local         */
    int level;			/* if (old != NULL), level of localness  */
};

/* structure stored in struct param's u.data by tied arrays */
struct tieddata {
    char ***arrptr;		/* pointer to corresponding array */
    int joinchar;		/* character used to join arrays */
};

/* flags for parameters */

/* parameter types */
#define PM_SCALAR	0	/* scalar                                   */
#define PM_ARRAY	(1<<0)	/* array                                    */
#define PM_INTEGER	(1<<1)	/* integer                                  */
#define PM_EFLOAT	(1<<2)	/* double with %e output		    */
#define PM_FFLOAT	(1<<3)	/* double with %f output		    */
#define PM_HASHED	(1<<4)	/* association                              */

#define PM_TYPE(X) \
  (X & (PM_SCALAR|PM_INTEGER|PM_EFLOAT|PM_FFLOAT|PM_ARRAY|PM_HASHED))

#define PM_LEFT		(1<<5)	/* left justify, remove leading blanks      */
#define PM_RIGHT_B	(1<<6)	/* right justify, fill with leading blanks  */
#define PM_RIGHT_Z	(1<<7)	/* right justify, fill with leading zeros   */
#define PM_LOWER	(1<<8)	/* all lower case                           */

/* The following are the same since they *
 * both represent -u option to typeset   */
#define PM_UPPER	(1<<9)	/* all upper case                           */
#define PM_UNDEFINED	(1<<9)	/* undefined (autoloaded) shell function    */

#define PM_READONLY	(1<<10)	/* readonly                                 */
#define PM_TAGGED	(1<<11)	/* tagged                                   */
#define PM_EXPORTED	(1<<12)	/* exported                                 */
#define PM_ABSPATH_USED (1<<12) /* (function): loaded using absolute path   */

/* The following are the same since they *
 * both represent -U option to typeset   */
#define PM_UNIQUE	(1<<13)	/* remove duplicates                        */
#define PM_UNALIASED	(1<<13)	/* (function) do not expand aliases when autoloading   */

#define PM_HIDE		(1<<14)	/* Special behaviour hidden by local        */
#define PM_CUR_FPATH    (1<<14) /* (function): can use $fpath with filename */
#define PM_HIDEVAL	(1<<15)	/* Value not shown in `typeset' commands    */
#define PM_WARNNESTED   (1<<15) /* (function): non-recursive WARNNESTEDVAR  */
#define PM_TIED 	(1<<16)	/* array tied to colon-path or v.v.         */
#define PM_TAGGED_LOCAL (1<<16) /* (function): non-recursive PM_TAGGED      */

/* Remaining flags do not correspond directly to command line arguments */
#define PM_DONTIMPORT_SUID (1<<17) /* do not import if running setuid */
#define PM_LOADDIR      (1<<17) /* (function) filename gives load directory */
#define PM_SINGLE       (1<<18) /* special can only have a single instance  */
#define PM_ANONYMOUS    (1<<18) /* (function) anonymous function            */
#define PM_LOCAL	(1<<19) /* this parameter will be made local        */
#define PM_KSHSTORED	(1<<19) /* (function) stored in ksh form              */
#define PM_SPECIAL	(1<<20) /* special builtin parameter                */
#define PM_ZSHSTORED	(1<<20) /* (function) stored in zsh form              */
#define PM_RO_BY_DESIGN (1<<21) /* to distinguish from specials that can be
				   made read-only by the user               */
#define PM_READONLY_SPECIAL (PM_SPECIAL|PM_READONLY|PM_RO_BY_DESIGN)
#define PM_DONTIMPORT	(1<<22)	/* do not import this variable              */
#define PM_DECLARED	(1<<22) /* explicitly named with typeset            */
#define PM_RESTRICTED	(1<<23) /* cannot be changed in restricted mode     */
#define PM_UNSET	(1<<24)	/* has null value                           */
#define PM_DEFAULTED	(PM_DECLARED|PM_UNSET)
#define PM_REMOVABLE	(1<<25)	/* special can be removed from paramtab     */
#define PM_AUTOLOAD	(1<<26) /* autoloaded from module                   */
#define PM_NORESTORE	(1<<27)	/* do not restore value of local special    */
#define PM_AUTOALL	(1<<27) /* autoload all features in module
				 * when loading: valid only if PM_AUTOLOAD
				 * is also present.
				 */
#define PM_HASHELEM     (1<<28) /* is a hash-element */
#define PM_NAMEDDIR     (1<<29) /* has a corresponding nameddirtab entry    */

/* The option string corresponds to the first of the variables above */
#define TYPESET_OPTSTR "aiEFALRZlurtxUhHTkz"

/* These typeset options take an optional numeric argument */
#define TYPESET_OPTNUM "LRZiEF"

/* Flags for extracting elements of arrays and associative arrays */
#define SCANPM_WANTVALS   (1<<0) /* Return value includes hash values */
#define SCANPM_WANTKEYS   (1<<1) /* Return value includes hash keys */
#define SCANPM_WANTINDEX  (1<<2) /* Return value includes array index */
#define SCANPM_MATCHKEY   (1<<3) /* Subscript matched against key */
#define SCANPM_MATCHVAL   (1<<4) /* Subscript matched against value */
#define SCANPM_MATCHMANY  (1<<5) /* Subscript matched repeatedly, return all */
#define SCANPM_ASSIGNING  (1<<6) /* Assigning whole array/hash */
#define SCANPM_KEYMATCH   (1<<7) /* keys of hash treated as patterns */
#define SCANPM_DQUOTED    (1<<8) /* substitution was double-quoted
				  * (only used for testing early end of
				  * subscript)
				  */
#define SCANPM_ARRONLY    (1<<9) /* value is array but we don't
				  * necessarily want to match multiple
				  * elements
				  */
#define SCANPM_CHECKING   (1<<10) /* Check if set, no need to create */
/* "$foo[@]"-style substitution
 * Only sign bit is significant
 */
#define SCANPM_ISVAR_AT   ((int)(((unsigned int)-1)<<15))

/*
 * Flags for doing matches inside parameter substitutions, i.e.
 * ${...#...} and friends.  This could be an enum, but so
 * could a lot of other things.
 */

#define SUB_END		0x0001	/* match end instead of beginning, % or %%  */
#define SUB_LONG	0x0002	/* % or # doubled, get longest match */
#define SUB_SUBSTR	0x0004	/* match a substring */
#define SUB_MATCH	0x0008	/* include the matched portion */
#define SUB_REST	0x0010	/* include the unmatched portion */
#define SUB_BIND	0x0020	/* index of beginning of string */
#define SUB_EIND	0x0040	/* index of end of string */
#define SUB_LEN		0x0080	/* length of match */
#define SUB_ALL		0x0100	/* match complete string */
#define SUB_GLOBAL	0x0200	/* global substitution ${..//all/these} */
#define SUB_DOSUBST	0x0400	/* replacement string needs substituting */
#define SUB_RETFAIL	0x0800  /* return status 0 if no match */
#define SUB_START	0x1000  /* force match at start with SUB_END
				 * and no SUB_SUBSTR */
#define SUB_LIST	0x2000  /* no substitution, return list of matches */
#define SUB_EGLOB	0x4000	/* use extended globbing in patterns */

/*
 * Structure recording multiple matches inside a test string.
 * b and e are the beginning and end of the match.
 * replstr is the replacement string, if any.
 */
struct repldata {
    int b, e;			/* beginning and end of chunk to replace */
    char *replstr;		/* replacement string to use */
};
typedef struct repldata *Repldata;

/*
 * Flags to zshtokenize.
 */
enum {
    /* Do glob substitution */
    ZSHTOK_SUBST = 0x0001,
    /* Use sh-style globbing */
    ZSHTOK_SHGLOB = 0x0002
};

/* Flags as the second argument to prefork */
enum {
    /* argument handled like typeset foo=bar */
    PREFORK_TYPESET       = 0x01,
    /* argument handled like the RHS of foo=bar */
    PREFORK_ASSIGN        = 0x02,
    /* single word substitution */
    PREFORK_SINGLE        = 0x04,
    /* explicitly split nested substitution */
    PREFORK_SPLIT         = 0x08,
    /* SHWORDSPLIT in parameter expn */
    PREFORK_SHWORDSPLIT   = 0x10,
    /* SHWORDSPLIT forced off in nested subst */
    PREFORK_NOSHWORDSPLIT = 0x20,
    /* Prefork is part of a parameter subexpression */
    PREFORK_SUBEXP        = 0x40,
    /* Prefork detected an assignment list with [key]=value syntax,
     * Only used on return from prefork, not meaningful passed down.
     * Also used as flag to globlist.
     */
    PREFORK_KEY_VALUE     = 0x80,
    /* No untokenise: used only as flag to globlist */
    PREFORK_NO_UNTOK      = 0x100
};

/*
 * Bit flags passed back from multsub() to paramsubst().
 * Some flags go from a nested parmsubst() through the enclosing
 * stringsubst() and prefork().
 */
enum {
    /*
     * Set if the string had whitespace at the start
     * that should cause word splitting against any preceding string.
     */
    MULTSUB_WS_AT_START = 1,
    /*
     * Set if the string had whitespace at the end
     * that should cause word splitting against any following string.
     */
    MULTSUB_WS_AT_END   = 2,
    /*
     * Set by nested paramsubst() to indicate the return
     * value is a parameter name, rather than a value.
     */
    MULTSUB_PARAM_NAME  = 4
};

/*
 * Structure for adding parameters in a module.
 * The flags should declare the type; note PM_SCALAR is zero.
 *
 * Special hashes are recognized by getnfn so the PM_HASHED
 * is optional.  These get slightly non-standard attention:
 * the function createspecialhash is used to create them.
 *
 * The get/set/unset attribute may be NULL; in that case the
 * parameter is assigned methods suitable for handling the
 * tie variable var, if that is not NULL, else standard methods.
 *
 * pm is set when the parameter is added to the parameter table
 * and serves as a flag that the parameter has been added.
 */
struct paramdef {
    char *name;
    int flags;
    void *var;			/* tied internal variable, if any */
    const void *gsu;		/* get/set/unset structure, if special */
    GetNodeFunc getnfn;		/* function to get node, if special hash */
    ScanTabFunc scantfn;	/* function to scan table, if special hash */
    Param pm;			/* structure inserted into param table */
};

/*
 * Shorthand for common uses of adding parameters, with no special
 * hash properties.
 */
#define PARAMDEF(name, flags, var, gsu) \
    { name, flags, (void *) var, (void *) gsu, \
	    NULL, NULL, NULL \
    }
/*
 * Note that the following definitions are appropriate for defining
 * parameters that reference a variable (var).  Hence the get/set/unset
 * methods used will assume var needs dereferencing to get the value.
 */
#define INTPARAMDEF(name, var) \
    { name, PM_INTEGER, (void *) var, NULL,  NULL, NULL, NULL }
#define STRPARAMDEF(name, var) \
    { name, PM_SCALAR, (void *) var, NULL, NULL, NULL, NULL }
#define ARRPARAMDEF(name, var) \
    { name, PM_ARRAY, (void *) var, NULL, NULL, NULL, NULL }
/*
 * The following is appropriate for a module function that behaves
 * in a special fashion.  Parameters used in a module that don't
 * have special behaviour shouldn't be declared in a table but
 * should just be added with the standard parameter functions.
 *
 * These parameters are not marked as removable, since they
 * shouldn't be loaded as local parameters, unlike the special
 * Zle parameters that are added and removed on each call to Zle.
 * We add the PM_REMOVABLE flag when removing the feature corresponding
 * to the parameter.
 */
#define SPECIALPMDEF(name, flags, gsufn, getfn, scanfn) \
    { name, flags | PM_SPECIAL | PM_HIDE | PM_HIDEVAL, \
	    NULL, gsufn, getfn, scanfn, NULL }

/*
 * Flags for assignsparam and assignaparam.
 */
enum {
    /* Add to rather than override value */
    ASSPM_AUGMENT = 1 << 0,
    /* Test for warning if creating global variable in function */
    ASSPM_WARN_CREATE = 1 << 1,
    /* Test for warning if using nested variable in function */
    ASSPM_WARN_NESTED = 1 << 2,
    ASSPM_WARN = (ASSPM_WARN_CREATE|ASSPM_WARN_NESTED),
    /* Import from environment, so exercise care evaluating value */
    ASSPM_ENV_IMPORT = 1 << 3,
    /* Array is key / value pairs.
     * This is normal for associative arrays but variant behaviour for
     * normal arrays.
     */
    ASSPM_KEY_VALUE = 1 << 4
};

/* node for named directory hash table (nameddirtab) */

struct nameddir {
    struct hashnode node;
    char *dir;			/* the directory in full            */
    int diff;			/* strlen(.dir) - strlen(.nam)      */
};

/* flags for named directories */
/* DISABLED is defined (1<<0) */
#define ND_USERNAME	(1<<1)	/* nam is actually a username       */
#define ND_NOABBREV	(1<<2)	/* never print as abbrev (PWD or OLDPWD) */

/* Storage for single group/name mapping */
typedef struct {
    /* Name of group */
    char *name;
    /* Group identifier */
    gid_t gid;
} groupmap;
typedef groupmap *Groupmap;

/* Storage for a set of group/name mappings */
typedef struct {
    /* The set of name to gid mappings */
    Groupmap array;
    /* A count of the valid entries in groupmap. */
    int num;
} groupset;
typedef groupset *Groupset;

/* flags for controlling printing of hash table nodes */
#define PRINT_NAMEONLY		(1<<0)
#define PRINT_TYPE		(1<<1)
#define PRINT_LIST		(1<<2)
#define PRINT_KV_PAIR		(1<<3)
#define PRINT_INCLUDEVALUE	(1<<4)
#define PRINT_TYPESET		(1<<5)
#define PRINT_LINE	        (1<<6)
#define PRINT_POSIX_EXPORT	(1<<7)
#define PRINT_POSIX_READONLY	(1<<8)

/* flags for printing for the whence builtin */
#define PRINT_WHENCE_CSH	(1<<7)
#define PRINT_WHENCE_VERBOSE	(1<<8)
#define PRINT_WHENCE_SIMPLE	(1<<9)
#define PRINT_WHENCE_FUNCDEF	(1<<10)
#define PRINT_WHENCE_WORD	(1<<11)

/* Return values from loop() */

enum loop_return {
    /* Loop executed OK */
    LOOP_OK,
    /* Loop executed no code */
    LOOP_EMPTY,
    /* Loop encountered an error */
    LOOP_ERROR
};

/* Return values from source() */

enum source_return {
    /* Source ran OK */
    SOURCE_OK = 0,
    /* File not found */
    SOURCE_NOT_FOUND = 1,
    /* Internal error sourcing file */
    SOURCE_ERROR = 2
};

enum noerrexit_bits {
    /* Suppress ERR_EXIT and traps: global */
    NOERREXIT_EXIT = 1,
    /* Suppress ERR_RETURN: per function call */
    NOERREXIT_RETURN = 2,
    /* NOERREXIT only needed on way down */
    NOERREXIT_UNTIL_EXEC = 4,
    /* Force exit on SIGINT */
    NOERREXIT_SIGNAL = 8
};

/***********************************/
/* Definitions for history control */
/***********************************/

/* history entry */

struct histent {
    struct hashnode node;

    Histent up;			/* previous line (moving upward)    */
    Histent down;		/* next line (moving downward)      */
    char *zle_text;		/* the edited history line,
				 * a metafied, NULL-terminated string,
				 * i.e the same format as the original
				 * entry
				 */
    time_t stim;		/* command started time (datestamp) */
    time_t ftim;		/* command finished time            */
    short *words;		/* Position of words in history     */
				/*   line:  as pairs of start, end  */
    int nwords;			/* Number of words in history line  */
    zlong histnum;		/* A sequential history number      */
};

#define HIST_MAKEUNIQUE	0x00000001	/* Kill this new entry if not unique */
#define HIST_OLD	0x00000002	/* Command is already written to disk*/
#define HIST_READ	0x00000004	/* Command was read back from disk*/
#define HIST_DUP	0x00000008	/* Command duplicates a later line */
#define HIST_FOREIGN	0x00000010	/* Command came from another shell */
#define HIST_TMPSTORE	0x00000020	/* Kill when user enters another cmd */
#define HIST_NOWRITE	0x00000040	/* Keep internally but don't write */

#define GETHIST_UPWARD  (-1)
#define GETHIST_DOWNWARD  1
#define GETHIST_EXACT     0

/* Parts of the code where history expansion is disabled *
 * should be within a pair of STOPHIST ... ALLOWHIST     */

#define STOPHIST (stophist += 4);
#define ALLOWHIST (stophist -= 4);

#define HISTFLAG_DONE   1
#define HISTFLAG_NOEXEC 2
#define HISTFLAG_RECALL 4
#define HISTFLAG_SETTY  8

#define HFILE_APPEND		0x0001
#define HFILE_SKIPOLD		0x0002
#define HFILE_SKIPDUPS		0x0004
#define HFILE_SKIPFOREIGN	0x0008
#define HFILE_FAST		0x0010
#define HFILE_NO_REWRITE	0x0020
#define HFILE_USE_OPTIONS	0x8000

/*
 * Flags argument to bufferwords() used
 * also by lexflags variable.
 */
/*
 * Kick the lexer into special string-analysis
 * mode without parsing.  Any bit set in
 * the flags has this effect, but this
 * has otherwise all the default effects.
 */
#define LEXFLAGS_ACTIVE		0x0001
/*
 * Being used from zle.  This is slightly more intrusive
 * (=> grotesquely non-modular) than use from within
 * the main shell, so it's a separate flag.
 */
#define LEXFLAGS_ZLE		0x0002
/*
 * Parse comments and treat each comment as a single string
 */
#define LEXFLAGS_COMMENTS_KEEP	0x0004
/*
 * Parse comments and strip them.
 */
#define LEXFLAGS_COMMENTS_STRIP	0x0008
/*
 * Either of the above
 */
#define LEXFLAGS_COMMENTS (LEXFLAGS_COMMENTS_KEEP|LEXFLAGS_COMMENTS_STRIP)
/*
 * Treat newlines as whitespace
 */
#define LEXFLAGS_NEWLINE	0x0010

/*******************************************/
/* Definitions for programmable completion */
/*******************************************/

/* Nothing special. */
#define IN_NOTHING 0
/* In command position. */
#define IN_CMD     1
/* In a mathematical environment. */
#define IN_MATH    2
/* In a condition. */
#define IN_COND    3
/* In a parameter assignment (e.g. `foo=bar'). */
#define IN_ENV     4
/* In a parameter name in an assignment. */
#define IN_PAR     5


/******************************/
/* Definition for zsh options */
/******************************/

/* Possible values of emulation */

#define EMULATE_CSH  (1<<1) /* C shell */
#define EMULATE_KSH  (1<<2) /* Korn shell */
#define EMULATE_SH   (1<<3) /* Bourne shell */
#define EMULATE_ZSH  (1<<4) /* `native' mode */

/* Test for a shell emulation.  Use this rather than emulation directly. */
#define EMULATION(X)	(emulation & (X))

/* Return only base shell emulation field. */
#define SHELL_EMULATION()	(emulation & ((1<<5)-1))

/* Additional flags */

#define EMULATE_FULLY (1<<5) /* "emulate -R" in effect */
/*
 * Higher bits are used in options.c, record lowest unused bit...
 */
#define EMULATE_UNUSED (1<<6)

/* option indices */

enum {
    OPT_INVALID,
    ALIASESOPT,
    ALIASFUNCDEF,
    ALLEXPORT,
    ALWAYSLASTPROMPT,
    ALWAYSTOEND,
    APPENDHISTORY,
    AUTOCD,
    AUTOCONTINUE,
    AUTOLIST,
    AUTOMENU,
    AUTONAMEDIRS,
    AUTOPARAMKEYS,
    AUTOPARAMSLASH,
    AUTOPUSHD,
    AUTOREMOVESLASH,
    AUTORESUME,
    BADPATTERN,
    BANGHIST,
    BAREGLOBQUAL,
    BASHAUTOLIST,
    BASHREMATCH,
    BEEP,
    BGNICE,
    BRACECCL,
    BSDECHO,
    CASEGLOB,
    CASEMATCH,
    CASEPATHS,
    CBASES,
    CDABLEVARS,
    CDSILENT,
    CHASEDOTS,
    CHASELINKS,
    CHECKJOBS,
    CHECKRUNNINGJOBS,
    CLOBBER,
    CLOBBEREMPTY,
    APPENDCREATE,
    COMBININGCHARS,
    COMPLETEALIASES,
    COMPLETEINWORD,
    CORRECT,
    CORRECTALL,
    CONTINUEONERROR,
    CPRECEDENCES,
    CSHJUNKIEHISTORY,
    CSHJUNKIELOOPS,
    CSHJUNKIEQUOTES,
    CSHNULLCMD,
    CSHNULLGLOB,
    DEBUGBEFORECMD,
    EMACSMODE,
    EQUALS,
    ERREXIT,
    ERRRETURN,
    EXECOPT,
    EXTENDEDGLOB,
    EXTENDEDHISTORY,
    EVALLINENO,
    FLOWCONTROL,
    FORCEFLOAT,
    FUNCTIONARGZERO,
    GLOBOPT,
    GLOBALEXPORT,
    GLOBALRCS,
    GLOBASSIGN,
    GLOBCOMPLETE,
    GLOBDOTS,
    GLOBSTARSHORT,
    GLOBSUBST,
    HASHCMDS,
    HASHDIRS,
    HASHEXECUTABLESONLY,
    HASHLISTALL,
    HISTALLOWCLOBBER,
    HISTBEEP,
    HISTEXPIREDUPSFIRST,
    HISTFCNTLLOCK,
    HISTFINDNODUPS,
    HISTIGNOREALLDUPS,
    HISTIGNOREDUPS,
    HISTIGNORESPACE,
    HISTLEXWORDS,
    HISTNOFUNCTIONS,
    HISTNOSTORE,
    HISTREDUCEBLANKS,
    HISTSAVEBYCOPY,
    HISTSAVENODUPS,
    HISTSUBSTPATTERN,
    HISTVERIFY,
    HUP,
    IGNOREBRACES,
    IGNORECLOSEBRACES,
    IGNOREEOF,
    INCAPPENDHISTORY,
    INCAPPENDHISTORYTIME,
    INTERACTIVE,
    INTERACTIVECOMMENTS,
    KSHARRAYS,
    KSHAUTOLOAD,
    KSHGLOB,
    KSHOPTIONPRINT,
    KSHTYPESET,
    KSHZEROSUBSCRIPT,
    LISTAMBIGUOUS,
    LISTBEEP,
    LISTPACKED,
    LISTROWSFIRST,
    LISTTYPES,
    LOCALLOOPS,
    LOCALOPTIONS,
    LOCALPATTERNS,
    LOCALTRAPS,
    LOGINSHELL,
    LONGLISTJOBS,
    MAGICEQUALSUBST,
    MAILWARNING,
    MARKDIRS,
    MENUCOMPLETE,
    MONITOR,
    MULTIBYTE,
    MULTIFUNCDEF,
    MULTIOS,
    NOMATCH,
    NOTIFY,
    NULLGLOB,
    NUMERICGLOBSORT,
    OCTALZEROES,
    OVERSTRIKE,
    PATHDIRS,
    PATHSCRIPT,
    PIPEFAIL,
    POSIXALIASES,
    POSIXARGZERO,
    POSIXBUILTINS,
    POSIXCD,
    POSIXIDENTIFIERS,
    POSIXJOBS,
    POSIXSTRINGS,
    POSIXTRAPS,
    PRINTEIGHTBIT,
    PRINTEXITVALUE,
    PRIVILEGED,
    PROMPTBANG,
    PROMPTCR,
    PROMPTPERCENT,
    PROMPTSP,
    PROMPTSUBST,
    PUSHDIGNOREDUPS,
    PUSHDMINUS,
    PUSHDSILENT,
    PUSHDTOHOME,
    RCEXPANDPARAM,
    RCQUOTES,
    RCS,
    RECEXACT,
    REMATCHPCRE,
    RESTRICTED,
    RMSTARSILENT,
    RMSTARWAIT,
    SHAREHISTORY,
    SHFILEEXPANSION,
    SHGLOB,
    SHINSTDIN,
    SHNULLCMD,
    SHOPTIONLETTERS,
    SHORTLOOPS,
    SHORTREPEAT,
    SHWORDSPLIT,
    SINGLECOMMAND,
    SINGLELINEZLE,
    SOURCETRACE,
    SUNKEYBOARDHACK,
    TRANSIENTRPROMPT,
    TRAPSASYNC,
    TYPESETSILENT,
    TYPESETTOUNSET,
    UNSET,
    VERBOSE,
    VIMODE,
    WARNCREATEGLOBAL,
    WARNNESTEDVAR,
    XTRACE,
    USEZLE,
    DVORAK,
    OPT_SIZE
};

/*
 * Size required to fit an option number.
 * If OPT_SIZE goes above 256 this will need to expand.
 */
typedef unsigned char OptIndex;

#undef isset
#define isset(X) (opts[X])
#define unset(X) (!opts[X])

#define interact (isset(INTERACTIVE))
#define jobbing  (isset(MONITOR))
#define islogin  (isset(LOGINSHELL))

/*
 * Record of emulation and options that need to be set
 * for a full "emulate".
 */
struct emulation_options {
    /* The emulation itself */
    int emulation;
    /* The number of options in on_opts. */
    int n_on_opts;
    /* The number of options in off_opts. */
    int n_off_opts;
    /*
     * Array of options to be turned on.
     * Only options specified explicitly in the emulate command
     * are recorded.  Null if n_on_opts is zero.
     */
    OptIndex *on_opts;
    /* Array of options to be turned off, similar. */
    OptIndex *off_opts;
};

/***********************************************/
/* Definitions for terminal and display control */
/***********************************************/

/* tty state structure */

struct ttyinfo {
#ifdef HAVE_TERMIOS_H
    struct termios tio;
#else
# ifdef HAVE_TERMIO_H
    struct termio tio;
# else
    struct sgttyb sgttyb;
    int lmodes;
    struct tchars tchars;
    struct ltchars ltchars;
# endif
#endif
#ifdef TIOCGWINSZ
    struct winsize winsize;
#endif
};

#ifndef __INTERIX
/* defines for whether tabs expand to spaces */
#if defined(HAVE_TERMIOS_H) || defined(HAVE_TERMIO_H)
#define SGTTYFLAG       shttyinfo.tio.c_oflag
#else   /* we're using sgtty */
#define SGTTYFLAG       shttyinfo.sgttyb.sg_flags
#endif
# ifdef TAB3
#define SGTABTYPE       TAB3
# else
#  ifdef OXTABS
#define SGTABTYPE       OXTABS
#  else
#   ifdef XTABS
#define SGTABTYPE       XTABS
#   endif
#  endif
# endif
#endif

/* flags for termflags */

#define TERM_BAD	0x01	/* terminal has extremely basic capabilities */
#define TERM_UNKNOWN	0x02	/* unknown terminal type */
#define TERM_NOUP	0x04	/* terminal has no up capability */
#define TERM_SHORT	0x08	/* terminal is < 3 lines high */
#define TERM_NARROW	0x10	/* terminal is < 3 columns wide */

/* interesting termcap strings */

#define TCCLEARSCREEN   0
#define TCLEFT          1
#define TCMULTLEFT      2
#define TCRIGHT         3
#define TCMULTRIGHT     4
#define TCUP            5
#define TCMULTUP        6
#define TCDOWN          7
#define TCMULTDOWN      8
#define TCDEL           9
#define TCMULTDEL      10
#define TCINS          11
#define TCMULTINS      12
#define TCCLEAREOD     13
#define TCCLEAREOL     14
#define TCINSLINE      15
#define TCDELLINE      16
#define TCNEXTTAB      17
#define TCBOLDFACEBEG  18
#define TCSTANDOUTBEG  19
#define TCUNDERLINEBEG 20
#define TCALLATTRSOFF  21
#define TCSTANDOUTEND  22
#define TCUNDERLINEEND 23
#define TCHORIZPOS     24
#define TCUPCURSOR     25
#define TCDOWNCURSOR   26
#define TCLEFTCURSOR   27
#define TCRIGHTCURSOR  28
#define TCSAVECURSOR   29
#define TCRESTRCURSOR  30
#define TCBACKSPACE    31
#define TCFGCOLOUR     32
#define TCBGCOLOUR     33
#define TC_COUNT       34

#define tccan(X) (tclen[X])

/*
 * Text attributes for displaying in ZLE
 */

#ifdef HAVE_STDINT_H
  typedef uint64_t zattr;
#else
  typedef zulong zattr;
#endif

#define TXTBOLDFACE   0x0001
#define TXTSTANDOUT   0x0002
#define TXTUNDERLINE  0x0004
#define TXTFGCOLOUR   0x0008
#define TXTBGCOLOUR   0x0010

#define TXT_ATTR_ON_MASK   0x001F

#define txtisset(X)  (txtattrmask & (X))
#define txtset(X)    (txtattrmask |= (X))
#define txtunset(X)  (txtattrmask &= ~(X))

#define TXTNOBOLDFACE	0x0020
#define TXTNOSTANDOUT	0x0040
#define TXTNOUNDERLINE	0x0080
#define TXTNOFGCOLOUR	0x0100
#define TXTNOBGCOLOUR	0x0200

#define TXT_ATTR_OFF_MASK  0x03E0
/* Bits to shift off right to get on */
#define TXT_ATTR_OFF_ON_SHIFT 5
#define TXT_ATTR_OFF_FROM_ON(attr)	\
    (((attr) & TXT_ATTR_ON_MASK) << TXT_ATTR_OFF_ON_SHIFT)
#define TXT_ATTR_ON_FROM_OFF(attr)	\
    (((attr) & TXT_ATTR_OFF_MASK) >> TXT_ATTR_OFF_ON_SHIFT)
/*
 * Indicates to zle_refresh.c that the character entry is an
 * index into the list of multiword symbols.
 */
#define TXT_MULTIWORD_MASK  0x0400

/* used when, e.g an invalid colour is specified */
#define TXT_ERROR 0x0800

/* Mask for colour to use in foreground */
#define TXT_ATTR_FG_COL_MASK     0x000000FFFFFF0000
/* Bits to shift the foreground colour */
#define TXT_ATTR_FG_COL_SHIFT    (16)
/* Mask for colour to use in background */
#define TXT_ATTR_BG_COL_MASK     0xFFFFFF0000000000
/* Bits to shift the background colour */
#define TXT_ATTR_BG_COL_SHIFT    (40)

/* Flag to indicate that foreground is a 24-bit colour */
#define TXT_ATTR_FG_24BIT        0x4000
/* Flag to indicate that background is a 24-bit colour */
#define TXT_ATTR_BG_24BIT        0x8000

/* Things to turn on, including values for the colour elements */
#define TXT_ATTR_ON_VALUES_MASK	\
    (TXT_ATTR_ON_MASK|TXT_ATTR_FG_COL_MASK|TXT_ATTR_BG_COL_MASK|\
     TXT_ATTR_FG_24BIT|TXT_ATTR_BG_24BIT)

/* Mask out everything to do with setting a foreground colour */
#define TXT_ATTR_FG_ON_MASK \
    (TXTFGCOLOUR|TXT_ATTR_FG_COL_MASK|TXT_ATTR_FG_24BIT)

/* Mask out everything to do with setting a background colour */
#define TXT_ATTR_BG_ON_MASK \
    (TXTBGCOLOUR|TXT_ATTR_BG_COL_MASK|TXT_ATTR_BG_24BIT)

/* Mask out everything to do with activating colours */
#define TXT_ATTR_COLOUR_ON_MASK			\
    (TXT_ATTR_FG_ON_MASK|TXT_ATTR_BG_ON_MASK)

#define txtchangeisset(T,X)	((T) & (X))
#define txtchangeget(T,A)	(((T) & A ## _MASK) >> A ## _SHIFT)
#define txtchangeset(T, X, Y)	((void)(T && (*T &= ~(Y), *T |= (X))))

/*
 * For outputting sequences to change colour: specify foreground
 * or background.
 */
#define COL_SEQ_FG	(0)
#define COL_SEQ_BG	(1)
#define COL_SEQ_COUNT	(2)

struct color_rgb {
    unsigned int red, green, blue;
};

typedef struct color_rgb *Color_rgb;

/*
 * Flags to testcap() and set_colour_attribute (which currently only
 * handles TSC_PROMPT).
 */
enum {
    /* Raw output: use stdout rather than shout */
    TSC_RAW = 0x0001,
    /* Output to current prompt buffer: only used when assembling prompt */
    TSC_PROMPT = 0x0002,
    /* Mask to get the output mode */
    TSC_OUTPUT_MASK = 0x0003,
    /* Change needs reset of other attributes */
    TSC_DIRTY = 0x0004
};

/****************************************/
/* Definitions for the %_ prompt escape */
/****************************************/

#define CMDSTACKSZ 256

#define CS_FOR          0
#define CS_WHILE        1
#define CS_REPEAT       2
#define CS_SELECT       3
#define CS_UNTIL        4
#define CS_IF           5
#define CS_IFTHEN       6
#define CS_ELSE         7
#define CS_ELIF         8
#define CS_MATH         9
#define CS_COND        10
#define CS_CMDOR       11
#define CS_CMDAND      12
#define CS_PIPE        13
#define CS_ERRPIPE     14
#define CS_FOREACH     15
#define CS_CASE        16
#define CS_FUNCDEF     17
#define CS_SUBSH       18
#define CS_CURSH       19
#define CS_ARRAY       20
#define CS_QUOTE       21
#define CS_DQUOTE      22
#define CS_BQUOTE      23
#define CS_CMDSUBST    24
#define CS_MATHSUBST   25
#define CS_ELIFTHEN    26
#define CS_HEREDOC     27
#define CS_HEREDOCD    28
#define CS_BRACE       29
#define CS_BRACEPAR    30
#define CS_ALWAYS      31

/* Increment as necessary */
#define CS_COUNT       32

/*********************
 * Memory management *
 *********************/

/*
 * A Heapid is a type for identifying, uniquely up to the point where
 * the count of new identifiers wraps. all heaps that are or
 * (importantly) have been valid.  Each valid heap is given an
 * identifier, and every time we push a heap we save the old identifier
 * and give the heap a new identifier so that when the heap is popped
 * or freed we can spot anything using invalid memory from the popped
 * heap.
 *
 * We could make this unsigned long long if we wanted a big range.
 */
typedef unsigned int Heapid;

#ifdef ZSH_HEAP_DEBUG

/* printf format specifier corresponding to Heapid */
#define HEAPID_FMT	"%x"

/* Marker that memory is permanently allocated */
#define HEAPID_PERMANENT (UINT_MAX)

/*
 * Heap debug verbosity.
 * Bits to be 'or'ed into the variable also called heap_debug_verbosity.
 */
enum heap_debug_verbosity {
    /* Report when we push a heap */
    HDV_PUSH = 0x01,
    /* Report when we pop a heap */
    HDV_POP = 0x02,
    /* Report when we create a new heap from which to allocate */
    HDV_CREATE = 0x04,
    /* Report every time we free a complete heap */
    HDV_FREE = 0x08,
    /* Report when we temporarily install a new set of heaps */
    HDV_NEW = 0x10,
    /* Report when we restore an old set of heaps */
    HDV_OLD = 0x20,
    /* Report when we temporarily switch heaps */
    HDV_SWITCH = 0x40,
    /*
     * Report every time we allocate memory from the heap.
     * This is very verbose, and arguably not very useful: we
     * would expect to allocate memory from a heap we create.
     * For much debugging heap_debug_verbosity = 0x7f should be sufficient.
     */
    HDV_ALLOC = 0x80
};

#define HEAP_ERROR(heap_id)			\
    fprintf(stderr, "%s:%d: HEAP DEBUG: invalid heap: " HEAPID_FMT ".\n", \
	    __FILE__, __LINE__, heap_id)
#endif

/* heappush saves the current heap state using this structure */

struct heapstack {
    struct heapstack *next;	/* next one in list for this heap */
    size_t used;
#ifdef ZSH_HEAP_DEBUG
    Heapid heap_id;
#endif
};

/* A zsh heap. */

struct heap {
    struct heap *next;		/* next one                                  */
    size_t size;		/* size of heap                              */
    size_t used;		/* bytes used from the heap                  */
    struct heapstack *sp;	/* used by pushheap() to save the value used */

#ifdef ZSH_HEAP_DEBUG
    unsigned int heap_id;
#endif

/* Uncomment the following if the struct needs padding to 64-bit size. */
/* Make sure sizeof(heap) is a multiple of 8 
#if defined(PAD_64_BIT) && !defined(__GNUC__)
    size_t dummy;		
#endif
*/
#define arena(X)	((char *) (X) + sizeof(struct heap))
}
#if defined(PAD_64_BIT) && defined(__GNUC__)
  __attribute__ ((aligned (8)))
#endif
;

# define NEWHEAPS(h)    do { Heap _switch_oldheaps = h = new_heaps(); do
# define OLDHEAPS       while (0); old_heaps(_switch_oldheaps); } while (0);

# define SWITCHHEAPS(o, h)  do { o = switch_heaps(h); do
# define SWITCHBACKHEAPS(o) while (0); switch_heaps(o); } while (0);

/****************/
/* Debug macros */
/****************/

#ifdef DEBUG
#define STRINGIFY_LITERAL(x)	# x
#define STRINGIFY(x)		STRINGIFY_LITERAL(x)
#define ERRMSG(x)		(__FILE__ ":" STRINGIFY(__LINE__) ": " x)
# define DPUTS(X,Y) if (!(X)) {;} else dputs(ERRMSG(Y))
# define DPUTS1(X,Y,Z1) if (!(X)) {;} else dputs(ERRMSG(Y), Z1)
# define DPUTS2(X,Y,Z1,Z2) if (!(X)) {;} else dputs(ERRMSG(Y), Z1, Z2)
# define DPUTS3(X,Y,Z1,Z2,Z3) if (!(X)) {;} else dputs(ERRMSG(Y), Z1, Z2, Z3)
#else
# define DPUTS(X,Y)
# define DPUTS1(X,Y,Z1)
# define DPUTS2(X,Y,Z1,Z2)
# define DPUTS3(X,Y,Z1,Z2,Z3)
#endif

/**************************/
/* Signal handling macros */
/**************************/

/* These used in the sigtrapped[] array */

#define ZSIG_TRAPPED	(1<<0)	/* Signal is trapped */
#define ZSIG_IGNORED	(1<<1)	/* Signal is ignored */
#define ZSIG_FUNC	(1<<2)	/* Trap is a function, not an eval list */
/* Mask to get the above flags */
#define ZSIG_MASK	(ZSIG_TRAPPED|ZSIG_IGNORED|ZSIG_FUNC)
/* No. of bits to shift local level when storing in sigtrapped */
#define ZSIG_ALIAS	(1<<3)  /* Trap is stored under an alias */
#define ZSIG_SHIFT	4

/*
 * State of traps, stored in trap_state.
 */
enum trap_state {
    /* Traps are not active; trap_return is not useful. */
    TRAP_STATE_INACTIVE,
    /*
     * Traps are set but haven't triggered; trap_return gives
     * minus function depth.
     */
    TRAP_STATE_PRIMED,
    /*
     * Trap has triggered to force a return; trap_return givens
     * return value.
     */
    TRAP_STATE_FORCE_RETURN
};

#define IN_EVAL_TRAP() \
    (intrap && !trapisfunc && traplocallevel == locallevel)

/*
 * Bits in the errflag variable.
 */
enum errflag_bits {
    /*
     * Standard internal error bit.
     */
    ERRFLAG_ERROR = 1,
    /*
     * User interrupt.
     */
    ERRFLAG_INT = 2,
    /*
     * Hard error --- return to top-level prompt in interactive
     * shell.  In non-interactive shell we'll typically already
     * have exited.  This is reset by "errflag = 0" in
     * loop(toplevel = 1, ...).
     */
    ERRFLAG_HARD = 4
};

/***********/
/* Sorting */
/***********/

typedef int (*CompareFn) _((const void *, const void *));

enum {
    SORTIT_ANYOLDHOW = 0,	/* Defaults */
    SORTIT_IGNORING_CASE = 1,
    SORTIT_NUMERICALLY = 2,
    SORTIT_NUMERICALLY_SIGNED = 4,
    SORTIT_BACKWARDS = 8,
    /*
     * Ignore backslashes that quote another character---which may
     * be another backslash; the second backslash is active.
     */
    SORTIT_IGNORING_BACKSLASHES = 16,
    /*
     * Ignored by strmetasort(); used by paramsubst() to indicate
     * there is some sorting to do.
     */
    SORTIT_SOMEHOW = 32,
};

/*
 * Element of array passed to qsort().
 */
struct sortelt {
    /* The original string. */
    char *orig;
    /* The string used for comparison. */
    const char *cmp;
    /*
     * The length of the string if passed down to the sort algorithm.
     * Used to sort the lengths together with the strings.
     */
    int origlen;
    /*
     * The length of the string, if needed, else -1.
     * The length is only needed if there are embedded nulls.
     */
    int len;
};

typedef struct sortelt *SortElt;

/*********************************************************/
/* Structures to save and restore for individual modules */
/*********************************************************/

/* History */
struct hist_stack {
    int histactive;
    int histdone;
    int stophist;
    int hlinesz;
    zlong defev;
    char *hline;
    char *hptr;
    short *chwords;
    int chwordlen;
    int chwordpos;
    int (*hgetc) _((void));
    void (*hungetc) _((int));
    void (*hwaddc) _((int));
    void (*hwbegin) _((int));
    void (*hwabort) _((void));
    void (*hwend) _((void));
    void (*addtoline) _((int));
    unsigned char *cstack;
    int csp;
    int hist_keep_comment;
};

/*
 * State of a lexical token buffer.
 *
 * It would be neater to include the pointer to the start of the buffer,
 * however the current code structure means that the standard instance
 * of this, tokstr, is visible in lots of places, so that's not
 * convenient.
 */

struct lexbufstate {
    /*
     * Next character to be added.
     * Set to NULL when the buffer is to be visible from elsewhere.
     */
    char *ptr;
    /* Allocated buffer size */
    int siz;
    /* Length in use */
    int len;
};

/* Lexical analyser */
struct lex_stack {
    int dbparens;
    int isfirstln;
    int isfirstch;
    int lexflags;
    enum lextok tok;
    char *tokstr;
    char *zshlextext;
    struct lexbufstate lexbuf;
    int lex_add_raw;
    char *tokstr_raw;
    struct lexbufstate lexbuf_raw;
    int lexstop;
    zlong toklineno;
};

/* Parser */
struct parse_stack {
    struct heredocs *hdocs;

    int incmdpos;
    int aliasspaceflag;
    int incond;
    int inredir;
    int incasepat;
    int isnewlin;
    int infor;
    int inrepeat_;
    int intypeset;

    int eclen, ecused, ecnpats;
    Wordcode ecbuf;
    Eccstr ecstrs;
    int ecsoffs, ecssub, ecnfunc;
};

/************************/
/* Flags to casemodifiy */
/************************/

enum {
    CASMOD_NONE,		/* dummy for tests */
    CASMOD_UPPER,
    CASMOD_LOWER,
    CASMOD_CAPS
};

/*******************************************/
/* Flags to third argument of getkeystring */
/*******************************************/

/*
 * By default handles some subset of \-escapes.  The following bits
 * turn on extra features.
 */
enum {
    /*
     * Handle octal where the first digit is non-zero e.g. \3, \33, \333
     * Otherwise \0333 etc. is handled, i.e. one of \0123 or \123 will
     * work, but not both.
     */
    GETKEY_OCTAL_ESC = (1 << 0),
    /*
     * Handle Emacs-like key sequences \C-x etc.
     * Also treat \E like \e and use backslashes to escape the
     * next character if not special, i.e. do all the things we
     * don't do with the echo builtin.
     */
    GETKEY_EMACS = (1 << 1),
    /* Handle ^X etc. */
    GETKEY_CTRL = (1 << 2),
    /* Handle \c (uses misc arg to getkeystring()) */
    GETKEY_BACKSLASH_C = (1 << 3),
    /* Do $'...' quoting (len arg to getkeystring() not used) */
    GETKEY_DOLLAR_QUOTE = (1 << 4),
    /* Handle \- (uses misc arg to getkeystring()) */
    GETKEY_BACKSLASH_MINUS = (1 << 5),
    /* Parse only one character (len arg to getkeystring() not used) */
    GETKEY_SINGLE_CHAR = (1 << 6),
    /*
     * If beyond offset in misc arg, add 1 to it for each character removed.
     * Yes, I know that doesn't seem to make much sense.
     * It's for use in completion, comprenez?
     */
    GETKEY_UPDATE_OFFSET = (1 << 7),
    /*
     * When replacing numeric escapes for printf format strings, % -> %%
     */
    GETKEY_PRINTF_PERCENT = (1 << 8)
};

/*
 * Standard combinations used within the shell.
 * Note GETKEYS_... instead of GETKEY_...: this is important in some cases.
 */
/* echo builtin */
#define GETKEYS_ECHO	(GETKEY_BACKSLASH_C)
/* printf format string:  \123 -> S, \0123 -> NL 3, \045 -> %% */
#define GETKEYS_PRINTF_FMT	\
        (GETKEY_OCTAL_ESC|GETKEY_BACKSLASH_C|GETKEY_PRINTF_PERCENT)
/* printf argument:  \123 -> \123, \0123 -> S */
#define GETKEYS_PRINTF_ARG	(GETKEY_BACKSLASH_C)
/* Full print without -e */
#define GETKEYS_PRINT	(GETKEY_OCTAL_ESC|GETKEY_BACKSLASH_C|GETKEY_EMACS)
/* bindkey */
#define GETKEYS_BINDKEY	(GETKEY_OCTAL_ESC|GETKEY_EMACS|GETKEY_CTRL)
/* $'...' */
#define GETKEYS_DOLLARS_QUOTE (GETKEY_OCTAL_ESC|GETKEY_EMACS|GETKEY_DOLLAR_QUOTE)
/* Single character for math processing */
#define GETKEYS_MATH	\
	(GETKEY_OCTAL_ESC|GETKEY_EMACS|GETKEY_CTRL|GETKEY_SINGLE_CHAR)
/* Used to process separators etc. with print-style escapes */
#define GETKEYS_SEP	(GETKEY_OCTAL_ESC|GETKEY_EMACS)
/* Used for suffix removal */
#define GETKEYS_SUFFIX		\
	(GETKEY_OCTAL_ESC|GETKEY_EMACS|GETKEY_CTRL|GETKEY_BACKSLASH_MINUS)

/**********************************/
/* Flags to third argument of zle */
/**********************************/

#define ZLRF_HISTORY	0x01	/* OK to access the history list */
#define ZLRF_NOSETTY	0x02	/* Don't set tty before return */
#define ZLRF_IGNOREEOF  0x04	/* Ignore an EOF from the keyboard */

/***************************/
/* Context of zleread call */
/***************************/

enum {
    ZLCON_LINE_START,		/* Command line at PS1 */
    ZLCON_LINE_CONT,		/* Command line at PS2 */
    ZLCON_SELECT,		/* Select loop */
    ZLCON_VARED			/* Vared command */
};

/****************/
/* Entry points */
/****************/

/* compctl entry point pointers */

typedef int (*CompctlReadFn) _((char *, char **, Options, char *));

/* ZLE entry point pointer */

typedef char * (*ZleEntryPoint)(int cmd, va_list ap);

/* Commands to pass to entry point */

enum {
    ZLE_CMD_GET_LINE,
    ZLE_CMD_READ,
    ZLE_CMD_ADD_TO_LINE,
    ZLE_CMD_TRASH,
    ZLE_CMD_RESET_PROMPT,
    ZLE_CMD_REFRESH,
    ZLE_CMD_SET_KEYMAP,
    ZLE_CMD_GET_KEY,
    ZLE_CMD_SET_HIST_LINE
};

/***************************************/
/* Hooks in core.                      */
/***************************************/

/* The type of zexit()'s second parameter, which see. */
enum zexit_t {
    /* This isn't a bitfield. The values are here just for explicitness. */
    ZEXIT_NORMAL = 0,
    ZEXIT_SIGNAL = 1,
    ZEXIT_DEFERRED = 2
};

#define EXITHOOK       (zshhooks + 0)
#define BEFORETRAPHOOK (zshhooks + 1)
#define AFTERTRAPHOOK  (zshhooks + 2)
#define GETCOLORATTR   (zshhooks + 3)

/* Final argument to [ms]b_niceformat() */
enum {
    NICEFLAG_HEAP = 1,		/* Heap allocation where needed */
    NICEFLAG_QUOTE = 2,		/* Result will appear in $'...' */
    NICEFLAG_NODUP = 4,         /* Leave allocated */
};

#ifdef MULTIBYTE_SUPPORT

/* Metafied input */
#define nicezputs(str, outs)	(void)mb_niceformat((str), (outs), NULL, 0)
#define MB_METACHARINIT()	mb_charinit()
typedef wint_t convchar_t;
#define MB_METACHARLENCONV(str, cp)	mb_metacharlenconv((str), (cp))
#define MB_METACHARLEN(str)	mb_metacharlenconv(str, NULL)
#define MB_METASTRLEN(str)	mb_metastrlenend(str, 0, NULL)
#define MB_METASTRWIDTH(str)	mb_metastrlenend(str, 1, NULL)
#define MB_METASTRLEN2(str, widthp)	mb_metastrlenend(str, widthp, NULL)
#define MB_METASTRLEN2END(str, widthp, eptr)	\
    mb_metastrlenend(str, widthp, eptr)

/* Unmetafined input */
#define MB_CHARINIT()		mb_charinit()
#define MB_CHARLENCONV(str, len, cp)	mb_charlenconv((str), (len), (cp))
#define MB_CHARLEN(str, len)	mb_charlenconv((str), (len), NULL)

/*
 * We replace broken implementations with one that uses Unicode
 * characters directly as wide characters.  In principle this is only
 * likely to work if __STDC_ISO_10646__ is defined, since that's pretty
 * much what the definition tells us.  However, we happen to know this
 * works on MacOS which doesn't define that.
 */
#ifdef ENABLE_UNICODE9
#define WCWIDTH(wc)	u9_wcwidth(wc)
#else
#define WCWIDTH(wc)	wcwidth(wc)
#endif
/*
 * Note WCWIDTH_WINT() takes wint_t, typically as a convchar_t.
 * It's written to use the wint_t from mb_metacharlenconv() without
 * further tests.
 *
 * This version has a non-multibyte definition that simply returns
 * 1.  We never expose WCWIDTH() in the non-multibyte world since
 * it's just a proxy for wcwidth() itself.
 */
#define WCWIDTH_WINT(wc)	zwcwidth(wc)

#define MB_INCOMPLETE	((size_t)-2)
#define MB_INVALID	((size_t)-1)

/*
 * MB_CUR_MAX is the maximum number of bytes that a single wide
 * character will convert into.  We use it to keep strings
 * sufficiently long.  It should always be defined, but if it isn't
 * just assume we are using Unicode which requires 6 characters.
 * (Note that it's not necessarily defined to a constant.)
 */
#ifndef MB_CUR_MAX
#define MB_CUR_MAX 6
#endif

/* Convert character or string to wide character or string */
#define ZWC(c)	L ## c
#define ZWS(s)	L ## s

/*
 * Test for a combining character.
 *
 * wc is assumed to be a wchar_t (i.e. we don't need zwcwidth).
 *
 * Pedantic note: in Unicode, a combining character need not be
 * zero length.  However, we are concerned here about display;
 * we simply need to know whether the character will be displayed
 * on top of another one.  We use "combining character" in this
 * sense throughout the shell.  I am not aware of a way of
 * detecting the Unicode trait in standard libraries.
 */
#define IS_COMBINING(wc)	(wc != 0 && WCWIDTH(wc) == 0)
/*
 * Test for the base of a combining character.
 *
 * We assume a combining character can be successfully displayed with
 * any non-space printable character, which is what a graphic character
 * is, as long as it has non-zero width.  We need to avoid all forms of
 * space because the shell will split words on any whitespace.
 */
#define IS_BASECHAR(wc)		(iswgraph(wc) && WCWIDTH(wc) > 0)

#else /* not MULTIBYTE_SUPPORT */

#define MB_METACHARINIT()
typedef int convchar_t;
#define MB_METACHARLENCONV(str, cp)	metacharlenconv((str), (cp))
#define MB_METACHARLEN(str)	(*(str) == Meta ? 2 : 1)
#define MB_METASTRLEN(str)	ztrlen(str)
#define MB_METASTRWIDTH(str)	ztrlen(str)
#define MB_METASTRLEN2(str, widthp)	ztrlen(str)
#define MB_METASTRLEN2END(str, widthp, eptr)	ztrlenend(str, eptr)

#define MB_CHARINIT()
#define MB_CHARLENCONV(str, len, cp) charlenconv((str), (len), (cp))
#define MB_CHARLEN(str, len) ((len) ? 1 : 0)

#define WCWIDTH_WINT(c)	(1)

/* Leave character or string as is. */
#define ZWC(c)	c
#define ZWS(s)	s

#endif /* MULTIBYTE_SUPPORT */

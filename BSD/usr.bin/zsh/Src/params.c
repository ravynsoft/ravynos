/*
 * params.c - parameters
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

#include "zsh.mdh"
#include "params.pro"

#include "version.h"
#ifdef CUSTOM_PATCHLEVEL
#define ZSH_PATCHLEVEL	CUSTOM_PATCHLEVEL
#else
#include "patchlevel.h"

#include <math.h>

/* If removed from the ChangeLog for some reason */
#ifndef ZSH_PATCHLEVEL
#define ZSH_PATCHLEVEL "unknown"
#endif
#endif

/* What level of localness we are at.
 *
 * Hand-wavingly, this is incremented at every function call and decremented
 * at every function return.  See startparamscope().
 */
 
/**/
mod_export int locallevel;

/* Variables holding values of special parameters */
 
/**/
mod_export
char **pparams,		/* $argv        */
     **cdpath,		/* $cdpath      */
     **fpath,		/* $fpath       */
     **mailpath,	/* $mailpath    */
     **manpath,		/* $manpath     */
     **psvar,		/* $psvar       */
     **zsh_eval_context; /* $zsh_eval_context */
/**/
mod_export
char **path,		/* $path        */
     **fignore;		/* $fignore     */
 
/**/
mod_export
char *argzero,		/* $0           */
     *posixzero,	/* $0           */
     *home,		/* $HOME        */
     *nullcmd,		/* $NULLCMD     */
     *oldpwd,		/* $OLDPWD      */
     *zoptarg,		/* $OPTARG      */
     *prompt,		/* $PROMPT      */
     *prompt2,		/* $PROMPT2     */
     *prompt3,		/* $PROMPT3     */
     *prompt4,		/* $PROMPT4     */
     *readnullcmd,	/* $READNULLCMD */
     *rprompt,		/* $RPROMPT     */
     *rprompt2,		/* $RPROMPT2    */
     *sprompt,		/* $SPROMPT     */
     *wordchars;	/* $WORDCHARS   */
/**/
mod_export
char *ifs,		/* $IFS         */
     *postedit,		/* $POSTEDIT    */
     *term,		/* $TERM        */
     *zsh_terminfo,     /* $TERMINFO    */
     *zsh_terminfodirs, /* $TERMINFO_DIRS */
     *ttystrname,	/* $TTY         */
     *pwd;		/* $PWD         */

/**/
mod_export volatile zlong
     lastval;		/* $?           */
/**/
mod_export zlong
     mypid,		/* $$           */
     lastpid,		/* $!           */
     zterm_columns,	/* $COLUMNS     */
     zterm_lines,	/* $LINES       */
     rprompt_indent,	/* $ZLE_RPROMPT_INDENT */
     ppid,		/* $PPID        */
     zsh_subshell;	/* $ZSH_SUBSHELL */

/* $FUNCNEST    */
/**/
mod_export
zlong zsh_funcnest =
#ifdef MAX_FUNCTION_DEPTH
    MAX_FUNCTION_DEPTH
#else
    /* Disabled by default but can be enabled at run time */
    -1
#endif
    ;

/**/
zlong lineno,		/* $LINENO      */
     zoptind,		/* $OPTIND      */
     shlvl;		/* $SHLVL       */

/* $histchars */
 
/**/
mod_export unsigned char bangchar;
/**/
unsigned char hatchar, hashchar;

/**/
unsigned char keyboardhackchar = '\0';
 
/* $SECONDS = now.tv_sec - shtimer.tv_sec
 *          + (now.tv_usec - shtimer.tv_usec) / 1000000.0
 * (rounded to an integer if the parameter is not set to float) */
 
/**/
struct timeval shtimer;
 
/* 0 if this $TERM setup is usable, otherwise it contains TERM_* flags */

/**/
mod_export int termflags;

/* Forward declaration */

static void
rprompt_indent_unsetfn(Param pm, int exp);

/* Standard methods for get/set/unset pointers in parameters */

/**/
mod_export const struct gsu_scalar stdscalar_gsu =
{ strgetfn, strsetfn, stdunsetfn };
/**/
mod_export const struct gsu_scalar varscalar_gsu =
{ strvargetfn, strvarsetfn, stdunsetfn };
/**/
mod_export const struct gsu_scalar nullsetscalar_gsu =
{ strgetfn, nullstrsetfn, NULL };

/**/
mod_export const struct gsu_integer stdinteger_gsu =
{ intgetfn, intsetfn, stdunsetfn };
/**/
mod_export const struct gsu_integer varinteger_gsu =
{ intvargetfn, intvarsetfn, stdunsetfn };
/**/
mod_export const struct gsu_integer nullsetinteger_gsu =
{ intgetfn, NULL, NULL };

/**/
mod_export const struct gsu_float stdfloat_gsu =
{ floatgetfn, floatsetfn, stdunsetfn };

/**/
mod_export const struct gsu_array stdarray_gsu =
{ arrgetfn, arrsetfn, stdunsetfn };
/**/
mod_export const struct gsu_array vararray_gsu =
{ arrvargetfn, arrvarsetfn, stdunsetfn };

/**/
mod_export const struct gsu_hash stdhash_gsu =
{ hashgetfn, hashsetfn, stdunsetfn };
/**/
mod_export const struct gsu_hash nullsethash_gsu =
{ hashgetfn, nullsethashfn, nullunsetfn };

/**/
mod_export const struct gsu_scalar colonarr_gsu =
{ colonarrgetfn, colonarrsetfn, stdunsetfn };


/* Non standard methods (not exported) */
static const struct gsu_integer pound_gsu =
{ poundgetfn, nullintsetfn, stdunsetfn };
static const struct gsu_integer errno_gsu =
{ errnogetfn, errnosetfn, stdunsetfn };
static const struct gsu_integer gid_gsu =
{ gidgetfn, gidsetfn, stdunsetfn };
static const struct gsu_integer egid_gsu =
{ egidgetfn, egidsetfn, stdunsetfn };
static const struct gsu_integer histsize_gsu =
{ histsizegetfn, histsizesetfn, stdunsetfn };
static const struct gsu_integer random_gsu =
{ randomgetfn, randomsetfn, stdunsetfn };
static const struct gsu_integer savehist_gsu =
{ savehistsizegetfn, savehistsizesetfn, stdunsetfn };
static const struct gsu_integer intseconds_gsu =
{ intsecondsgetfn, intsecondssetfn, stdunsetfn };
static const struct gsu_float floatseconds_gsu =
{ floatsecondsgetfn, floatsecondssetfn, stdunsetfn };
static const struct gsu_integer uid_gsu =
{ uidgetfn, uidsetfn, stdunsetfn };
static const struct gsu_integer euid_gsu =
{ euidgetfn, euidsetfn, stdunsetfn };
static const struct gsu_integer ttyidle_gsu =
{ ttyidlegetfn, nullintsetfn, stdunsetfn };

static const struct gsu_scalar argzero_gsu =
{ argzerogetfn, argzerosetfn, nullunsetfn };
static const struct gsu_scalar username_gsu =
{ usernamegetfn, usernamesetfn, stdunsetfn };
static const struct gsu_scalar dash_gsu =
{ dashgetfn, nullstrsetfn, stdunsetfn };
static const struct gsu_scalar histchars_gsu =
{ histcharsgetfn, histcharssetfn, stdunsetfn };
static const struct gsu_scalar home_gsu =
{ homegetfn, homesetfn, stdunsetfn };
static const struct gsu_scalar term_gsu =
{ termgetfn, termsetfn, stdunsetfn };
static const struct gsu_scalar terminfo_gsu =
{ terminfogetfn, terminfosetfn, stdunsetfn };
static const struct gsu_scalar terminfodirs_gsu =
{ terminfodirsgetfn, terminfodirssetfn, stdunsetfn };
static const struct gsu_scalar wordchars_gsu =
{ wordcharsgetfn, wordcharssetfn, stdunsetfn };
static const struct gsu_scalar ifs_gsu =
{ ifsgetfn, ifssetfn, stdunsetfn };
static const struct gsu_scalar underscore_gsu =
{ underscoregetfn, nullstrsetfn, stdunsetfn };
static const struct gsu_scalar keyboard_hack_gsu =
{ keyboardhackgetfn, keyboardhacksetfn, stdunsetfn };
#ifdef USE_LOCALE
static const struct gsu_scalar lc_blah_gsu =
{ strgetfn, lcsetfn, stdunsetfn };
static const struct gsu_scalar lang_gsu =
{ strgetfn, langsetfn, stdunsetfn };
static const struct gsu_scalar lc_all_gsu =
{ strgetfn, lc_allsetfn, stdunsetfn };
#endif

static const struct gsu_integer varint_readonly_gsu =
{ intvargetfn, nullintsetfn, stdunsetfn };
static const struct gsu_integer zlevar_gsu =
{ intvargetfn, zlevarsetfn, stdunsetfn };

static const struct gsu_integer argc_gsu =
{ poundgetfn, nullintsetfn, stdunsetfn };
static const struct gsu_array pipestatus_gsu =
{ pipestatgetfn, pipestatsetfn, stdunsetfn };

static const struct gsu_integer rprompt_indent_gsu =
{ intvargetfn, zlevarsetfn, rprompt_indent_unsetfn };

/* Nodes for special parameters for parameter hash table */

#ifdef HAVE_UNION_INIT
# define BR(X) {X}
typedef struct param initparam;
#else
# define BR(X) X
typedef struct iparam {
    struct hashnode *next;
    char *nam;			/* hash data                             */
    int flags;			/* PM_* flags (defined in zsh.h)         */
    void *value;
    void *gsu;			/* get/set/unset methods */
    int base;			/* output base                           */
    int width;			/* output field width                    */
    char *env;			/* location in environment, if exported  */
    char *ename;		/* name of corresponding environment var */
    Param old;			/* old struct for use with local         */
    int level;			/* if (old != NULL), level of localness  */
} initparam;
#endif

static initparam special_params[] ={
#define GSU(X) BR((GsuScalar)(void *)(&(X)))
#define NULL_GSU BR((GsuScalar)(void *)NULL)
#define IPDEF1(A,B,C) {{NULL,A,PM_INTEGER|PM_SPECIAL|C},BR(NULL),GSU(B),10,0,NULL,NULL,NULL,0}
IPDEF1("#", pound_gsu, PM_READONLY_SPECIAL),
IPDEF1("ERRNO", errno_gsu, PM_UNSET),
IPDEF1("GID", gid_gsu, PM_DONTIMPORT | PM_RESTRICTED),
IPDEF1("EGID", egid_gsu, PM_DONTIMPORT | PM_RESTRICTED),
IPDEF1("HISTSIZE", histsize_gsu, PM_RESTRICTED),
IPDEF1("RANDOM", random_gsu, 0),
IPDEF1("SAVEHIST", savehist_gsu, PM_RESTRICTED),
IPDEF1("SECONDS", intseconds_gsu, 0),
IPDEF1("UID", uid_gsu, PM_DONTIMPORT | PM_RESTRICTED),
IPDEF1("EUID", euid_gsu, PM_DONTIMPORT | PM_RESTRICTED),
IPDEF1("TTYIDLE", ttyidle_gsu, PM_READONLY_SPECIAL),

#define IPDEF2(A,B,C) {{NULL,A,PM_SCALAR|PM_SPECIAL|C},BR(NULL),GSU(B),0,0,NULL,NULL,NULL,0}
IPDEF2("USERNAME", username_gsu, PM_DONTIMPORT|PM_RESTRICTED),
IPDEF2("-", dash_gsu, PM_READONLY_SPECIAL),
IPDEF2("histchars", histchars_gsu, PM_DONTIMPORT),
IPDEF2("HOME", home_gsu, PM_UNSET),
IPDEF2("TERM", term_gsu, PM_UNSET),
IPDEF2("TERMINFO", terminfo_gsu, PM_UNSET),
IPDEF2("TERMINFO_DIRS", terminfodirs_gsu, PM_UNSET),
IPDEF2("WORDCHARS", wordchars_gsu, 0),
IPDEF2("IFS", ifs_gsu, PM_DONTIMPORT | PM_RESTRICTED),
IPDEF2("_", underscore_gsu, PM_DONTIMPORT),
IPDEF2("KEYBOARD_HACK", keyboard_hack_gsu, PM_DONTIMPORT),
IPDEF2("0", argzero_gsu, 0),

#ifdef USE_LOCALE
# define LCIPDEF(name) IPDEF2(name, lc_blah_gsu, PM_UNSET)
IPDEF2("LANG", lang_gsu, PM_UNSET),
IPDEF2("LC_ALL", lc_all_gsu, PM_UNSET),
# ifdef LC_COLLATE
LCIPDEF("LC_COLLATE"),
# endif
# ifdef LC_CTYPE
LCIPDEF("LC_CTYPE"),
# endif
# ifdef LC_MESSAGES
LCIPDEF("LC_MESSAGES"),
# endif
# ifdef LC_NUMERIC
LCIPDEF("LC_NUMERIC"),
# endif
# ifdef LC_TIME
LCIPDEF("LC_TIME"),
# endif
#endif /* USE_LOCALE */

#define IPDEF4(A,B) {{NULL,A,PM_INTEGER|PM_READONLY_SPECIAL},BR((void *)B),GSU(varint_readonly_gsu),10,0,NULL,NULL,NULL,0}
IPDEF4("!", &lastpid),
IPDEF4("$", &mypid),
IPDEF4("?", &lastval),
IPDEF4("HISTCMD", &curhist),
IPDEF4("LINENO", &lineno),
IPDEF4("PPID", &ppid),
IPDEF4("ZSH_SUBSHELL", &zsh_subshell),

#define IPDEF5(A,B,F) {{NULL,A,PM_INTEGER|PM_SPECIAL},BR((void *)B),GSU(F),10,0,NULL,NULL,NULL,0}
#define IPDEF5U(A,B,F) {{NULL,A,PM_INTEGER|PM_SPECIAL|PM_UNSET},BR((void *)B),GSU(F),10,0,NULL,NULL,NULL,0}
IPDEF5("COLUMNS", &zterm_columns, zlevar_gsu),
IPDEF5("LINES", &zterm_lines, zlevar_gsu),
IPDEF5U("ZLE_RPROMPT_INDENT", &rprompt_indent, rprompt_indent_gsu),
IPDEF5("SHLVL", &shlvl, varinteger_gsu),
IPDEF5("FUNCNEST", &zsh_funcnest, varinteger_gsu),

/* Don't import internal integer status variables. */
#define IPDEF6(A,B,F) {{NULL,A,PM_INTEGER|PM_SPECIAL|PM_DONTIMPORT},BR((void *)B),GSU(F),10,0,NULL,NULL,NULL,0}
IPDEF6("OPTIND", &zoptind, varinteger_gsu),
IPDEF6("TRY_BLOCK_ERROR", &try_errflag, varinteger_gsu),
IPDEF6("TRY_BLOCK_INTERRUPT", &try_interrupt, varinteger_gsu),

#define IPDEF7(A,B) {{NULL,A,PM_SCALAR|PM_SPECIAL},BR((void *)B),GSU(varscalar_gsu),0,0,NULL,NULL,NULL,0}
#define IPDEF7R(A,B) {{NULL,A,PM_SCALAR|PM_SPECIAL|PM_DONTIMPORT_SUID},BR((void *)B),GSU(varscalar_gsu),0,0,NULL,NULL,NULL,0}
#define IPDEF7U(A,B) {{NULL,A,PM_SCALAR|PM_SPECIAL|PM_UNSET},BR((void *)B),GSU(varscalar_gsu),0,0,NULL,NULL,NULL,0}
IPDEF7("OPTARG", &zoptarg),
IPDEF7("NULLCMD", &nullcmd),
IPDEF7U("POSTEDIT", &postedit),
IPDEF7("READNULLCMD", &readnullcmd),
IPDEF7("PS1", &prompt),
IPDEF7U("RPS1", &rprompt),
IPDEF7U("RPROMPT", &rprompt),
IPDEF7("PS2", &prompt2),
IPDEF7U("RPS2", &rprompt2),
IPDEF7U("RPROMPT2", &rprompt2),
IPDEF7("PS3", &prompt3),
IPDEF7R("PS4", &prompt4),
IPDEF7("SPROMPT", &sprompt),

#define IPDEF9(A,B,C,D) {{NULL,A,D|PM_ARRAY|PM_SPECIAL|PM_DONTIMPORT},BR((void *)B),GSU(vararray_gsu),0,0,NULL,C,NULL,0}
IPDEF9("*", &pparams, NULL, PM_ARRAY|PM_READONLY_SPECIAL|PM_DONTIMPORT),
IPDEF9("@", &pparams, NULL, PM_ARRAY|PM_READONLY_SPECIAL|PM_DONTIMPORT),

/*
 * This empty row indicates the end of parameters available in
 * all emulations.
 */
{{NULL,NULL,0},BR(NULL),NULL_GSU,0,0,NULL,NULL,NULL,0},

#define IPDEF8(A,B,C,D) {{NULL,A,D|PM_SCALAR|PM_SPECIAL},BR((void *)B),GSU(colonarr_gsu),0,0,NULL,C,NULL,0}
IPDEF8("CDPATH", &cdpath, "cdpath", PM_TIED),
IPDEF8("FIGNORE", &fignore, "fignore", PM_TIED),
IPDEF8("FPATH", &fpath, "fpath", PM_TIED),
IPDEF8("MAILPATH", &mailpath, "mailpath", PM_TIED),
IPDEF8("PATH", &path, "path", PM_RESTRICTED|PM_TIED),
IPDEF8("PSVAR", &psvar, "psvar", PM_TIED),
IPDEF8("ZSH_EVAL_CONTEXT", &zsh_eval_context, "zsh_eval_context", PM_READONLY_SPECIAL|PM_TIED),

/* MODULE_PATH is not imported for security reasons */
IPDEF8("MODULE_PATH", &module_path, "module_path", PM_DONTIMPORT|PM_RESTRICTED|PM_TIED),

#define IPDEF10(A,B) {{NULL,A,PM_ARRAY|PM_SPECIAL},BR(NULL),GSU(B),10,0,NULL,NULL,NULL,0}

/*
 * The following parameters are not available in sh/ksh compatibility *
 * mode.
 */

/* All of these have sh compatible equivalents.                */
IPDEF1("ARGC", argc_gsu, PM_READONLY_SPECIAL),
IPDEF2("HISTCHARS", histchars_gsu, PM_DONTIMPORT),
IPDEF4("status", &lastval),
IPDEF7("prompt", &prompt),
IPDEF7("PROMPT", &prompt),
IPDEF7("PROMPT2", &prompt2),
IPDEF7("PROMPT3", &prompt3),
IPDEF7("PROMPT4", &prompt4),
IPDEF8("MANPATH", &manpath, "manpath", PM_TIED),
IPDEF9("argv", &pparams, NULL, 0),
IPDEF9("fignore", &fignore, "FIGNORE", PM_TIED),
IPDEF9("cdpath", &cdpath, "CDPATH", PM_TIED),
IPDEF9("fpath", &fpath, "FPATH", PM_TIED),
IPDEF9("mailpath", &mailpath, "MAILPATH", PM_TIED),
IPDEF9("manpath", &manpath, "MANPATH", PM_TIED),
IPDEF9("psvar", &psvar, "PSVAR", PM_TIED),

IPDEF9("zsh_eval_context", &zsh_eval_context, "ZSH_EVAL_CONTEXT", PM_TIED|PM_READONLY_SPECIAL),

IPDEF9("module_path", &module_path, "MODULE_PATH", PM_TIED|PM_RESTRICTED),
IPDEF9("path", &path, "PATH", PM_TIED|PM_RESTRICTED),

/* These are known to zsh alone. */

IPDEF10("pipestatus", pipestatus_gsu),

{{NULL,NULL,0},BR(NULL),NULL_GSU,0,0,NULL,NULL,NULL,0},
};

/*
 * Alternative versions of colon-separated path parameters for
 * sh emulation.  These don't link to the array versions.
 */
static initparam special_params_sh[] = {
IPDEF8("CDPATH", &cdpath, NULL, 0),
IPDEF8("FIGNORE", &fignore, NULL, 0),
IPDEF8("FPATH", &fpath, NULL, 0),
IPDEF8("MAILPATH", &mailpath, NULL, 0),
IPDEF8("PATH", &path, NULL, PM_RESTRICTED),
IPDEF8("PSVAR", &psvar, NULL, 0),
IPDEF8("ZSH_EVAL_CONTEXT", &zsh_eval_context, NULL, PM_READONLY_SPECIAL),

/* MODULE_PATH is not imported for security reasons */
IPDEF8("MODULE_PATH", &module_path, NULL, PM_DONTIMPORT|PM_RESTRICTED),

{{NULL,NULL,0},BR(NULL),NULL_GSU,0,0,NULL,NULL,NULL,0},
};

/*
 * Special way of referring to the positional parameters.  Unlike $*
 * and $@, this is not readonly.  This parameter is not directly
 * visible in user space.
 */
static initparam argvparam_pm = IPDEF9("", &pparams, NULL, \
				 PM_ARRAY|PM_SPECIAL|PM_DONTIMPORT);

#undef BR

#define IS_UNSET_VALUE(V) \
	((V) && (!(V)->pm || ((V)->pm->node.flags & PM_UNSET) || \
		 !(V)->pm->node.nam || !*(V)->pm->node.nam))

static Param argvparam;

/* "parameter table" - hash table containing the parameters
 *
 * realparamtab always points to the shell's global table.  paramtab is sometimes
 * temporarily changed to point at another table, while dealing with the keys
 * of an associative array (for example, see makecompparams() which initializes
 * the associative array ${compstate}).
 */
 
/**/
mod_export HashTable paramtab, realparamtab;

/**/
mod_export HashTable
newparamtable(int size, char const *name)
{
    HashTable ht;
    if (!size)
	size = 17;
    ht = newhashtable(size, name, NULL);

    ht->hash        = hasher;
    ht->emptytable  = emptyhashtable;
    ht->filltable   = NULL;
    ht->cmpnodes    = strcmp;
    ht->addnode     = addhashnode;
    ht->getnode     = getparamnode;
    ht->getnode2    = gethashnode2;
    ht->removenode  = removehashnode;
    ht->disablenode = NULL;
    ht->enablenode  = NULL;
    ht->freenode    = freeparamnode;
    ht->printnode   = printparamnode;

    return ht;
}

/**/
static HashNode
getparamnode(HashTable ht, const char *nam)
{
    HashNode hn = gethashnode2(ht, nam);
    Param pm = (Param) hn;

    if (pm && pm->u.str && (pm->node.flags & PM_AUTOLOAD)) {
	char *mn = dupstring(pm->u.str);

	(void)ensurefeature(mn, "p:", (pm->node.flags & PM_AUTOALL) ? NULL :
			    nam);
	hn = gethashnode2(ht, nam);
	if (!hn) {
	    /*
	     * This used to be a warning, but surely if we allow
	     * stuff to go ahead with the autoload stub with
	     * no error status we're in for all sorts of mayhem?
	     */
	    zerr("autoloading module %s failed to define parameter: %s", mn,
		 nam);
	}
    }
    return hn;
}

/* Copy a parameter hash table */

static HashTable outtable;

/**/
static void
scancopyparams(HashNode hn, UNUSED(int flags))
{
    /* Going into a real parameter, so always use permanent storage */
    Param pm = (Param)hn;
    Param tpm = (Param) zshcalloc(sizeof *tpm);
    tpm->node.nam = ztrdup(pm->node.nam);
    copyparam(tpm, pm, 0);
    addhashnode(outtable, tpm->node.nam, tpm);
}

/**/
HashTable
copyparamtable(HashTable ht, char *name)
{
    HashTable nht = 0;
    if (ht) {
	nht = newparamtable(ht->hsize, name);
	outtable = nht;
	scanhashtable(ht, 0, 0, 0, scancopyparams, 0);
	outtable = NULL;
    }
    return nht;
}

/* Flag to freeparamnode to unset the struct */

static int delunset;

/* Function to delete a parameter table. */

/**/
mod_export void
deleteparamtable(HashTable t)
{
    /* The parameters in the hash table need to be unset *
     * before being deleted.                             */
    int odelunset = delunset;
    delunset = 1;
    deletehashtable(t);
    delunset = odelunset;
}

static unsigned numparamvals;

/**/
mod_export void
scancountparams(UNUSED(HashNode hn), int flags)
{
    ++numparamvals;
    if ((flags & SCANPM_WANTKEYS) && (flags & SCANPM_WANTVALS))
	++numparamvals;
}

static Patprog scanprog;
static char *scanstr;
static char **paramvals;
static Param foundparam;

/**/
static void
scanparamvals(HashNode hn, int flags)
{
    struct value v;
    Patprog prog;

    if (numparamvals && !(flags & SCANPM_MATCHMANY) &&
	(flags & (SCANPM_MATCHVAL|SCANPM_MATCHKEY|SCANPM_KEYMATCH)))
	return;
    v.pm = (Param)hn;
    if ((flags & SCANPM_KEYMATCH)) {
	char *tmp = dupstring(v.pm->node.nam);

	tokenize(tmp);
	remnulargs(tmp);

	if (!(prog = patcompile(tmp, 0, NULL)) || !pattry(prog, scanstr))
	    return;
    } else if ((flags & SCANPM_MATCHKEY) && !pattry(scanprog, v.pm->node.nam)) {
	return;
    }
    foundparam = v.pm;
    if (flags & SCANPM_WANTKEYS) {
	paramvals[numparamvals++] = v.pm->node.nam;
	if (!(flags & (SCANPM_WANTVALS|SCANPM_MATCHVAL)))
	    return;
    }
    v.isarr = (PM_TYPE(v.pm->node.flags) & (PM_ARRAY|PM_HASHED));
    v.flags = 0;
    v.start = 0;
    v.end = -1;
    paramvals[numparamvals] = getstrvalue(&v);
    if (flags & SCANPM_MATCHVAL) {
	if (pattry(scanprog, paramvals[numparamvals])) {
	    numparamvals += ((flags & SCANPM_WANTVALS) ? 1 :
			     !(flags & SCANPM_WANTKEYS));
	} else if (flags & SCANPM_WANTKEYS)
	    --numparamvals;	/* Value didn't match, discard key */
    } else
	++numparamvals;
    foundparam = NULL;
}

/**/
char **
paramvalarr(HashTable ht, int flags)
{
    DPUTS((flags & (SCANPM_MATCHKEY|SCANPM_MATCHVAL)) && !scanprog,
	  "BUG: scanning hash without scanprog set");
    numparamvals = 0;
    if (ht)
	scanhashtable(ht, 0, 0, PM_UNSET, scancountparams, flags);
    paramvals = (char **) zhalloc((numparamvals + 1) * sizeof(char *));
    if (ht) {
	numparamvals = 0;
	scanhashtable(ht, 0, 0, PM_UNSET, scanparamvals, flags);
    }
    paramvals[numparamvals] = 0;
    return paramvals;
}

/* Return the full array (no indexing) referred to by a Value. *
 * The array value is cached for the lifetime of the Value.    */

/**/
static char **
getvaluearr(Value v)
{
    if (v->arr)
	return v->arr;
    else if (PM_TYPE(v->pm->node.flags) == PM_ARRAY)
	return v->arr = v->pm->gsu.a->getfn(v->pm);
    else if (PM_TYPE(v->pm->node.flags) == PM_HASHED) {
	v->arr = paramvalarr(v->pm->gsu.h->getfn(v->pm), v->isarr);
	/* Can't take numeric slices of associative arrays */
	v->start = 0;
	v->end = numparamvals + 1;
	return v->arr;
    } else
	return NULL;
}

/* Return whether the variable is set         *
 * checks that array slices are within range  *
 * used for [[ -v ... ]] condition test       */

/**/
int
issetvar(char *name)
{
    struct value vbuf;
    Value v;
    int slice;
    char **arr;

    if (!(v = getvalue(&vbuf, &name, 1)) || *name)
	return 0; /* no value or more chars after the variable name */
    if (v->isarr & ~SCANPM_ARRONLY)
	return v->end > 1; /* for extracted elements, end gives us a count */

    slice = v->start != 0 || v->end != -1;
    if (PM_TYPE(v->pm->node.flags) != PM_ARRAY || !slice)
	return !slice && !(v->pm->node.flags & PM_UNSET);

    if (!v->end) /* empty array slice */
	return 0;
    /* get the array and check end is within range */
    if (!(arr = getvaluearr(v)))
	return 0;
    return arrlen_ge(arr, v->end < 0 ? - v->end : v->end);
}

/*
 * Split environment string into (name, value) pair.
 * this is used to avoid in-place editing of environment table
 * that results in core dump on some systems
 */

static int
split_env_string(char *env, char **name, char **value)
{
    char *str, *tenv;

    if (!env || !name || !value)
	return 0;

    tenv = strcpy(zhalloc(strlen(env) + 1), env);
    for (str = tenv; *str && *str != '='; str++) {
	if (STOUC(*str) >= 128) {
	    /*
	     * We'll ignore environment variables with names not
	     * from the portable character set since we don't
	     * know of a good reason to accept them.
	     */
	    return 0;
	}
    }
    if (str != tenv && *str == '=') {
	*str = '\0';
	*name = tenv;
	*value = str + 1;
	return 1;
    } else
	return 0;
}

/**
 * Check parameter flags to see if parameter shouldn't be imported
 * from environment at start.
 *
 * return 1: don't import: 0: ok to import.
 */
static int dontimport(int flags)
{
    /* If explicitly marked as don't import */
    if (flags & PM_DONTIMPORT)
	return 1;
    /* If value already exported */
    if (flags & PM_EXPORTED)
	return 1;
    /* If security issue when importing and running with some privilege */
    if ((flags & PM_DONTIMPORT_SUID) && isset(PRIVILEGED))
	return 1;
    /* OK to import */
    return 0;
}

/* Set up parameter hash table.  This will add predefined  *
 * parameter entries as well as setting up parameter table *
 * entries for environment variables we inherit.           */

/**/
void
createparamtable(void)
{
    Param ip, pm;
#if !defined(HAVE_PUTENV) && !defined(USE_SET_UNSET_ENV)
    char **new_environ;
    int  envsize;
#endif
#ifndef USE_SET_UNSET_ENV
    char **envp;
#endif
    char **envp2, **sigptr, **t;
    char buf[50], *str, *iname, *ivalue, *hostnam;
    int  oae = opts[ALLEXPORT];
#ifdef HAVE_UNAME
    struct utsname unamebuf;
    char *machinebuf;
#endif

    paramtab = realparamtab = newparamtable(151, "paramtab");

    /* Add the special parameters to the hash table */
    for (ip = special_params; ip->node.nam; ip++)
	paramtab->addnode(paramtab, ztrdup(ip->node.nam), ip);
    if (EMULATION(EMULATE_SH|EMULATE_KSH)) {
	for (ip = special_params_sh; ip->node.nam; ip++)
	    paramtab->addnode(paramtab, ztrdup(ip->node.nam), ip);
    } else {
	while ((++ip)->node.nam)
	    paramtab->addnode(paramtab, ztrdup(ip->node.nam), ip);
    }

    argvparam = (Param) &argvparam_pm;

    noerrs = 2;

    /* Add the standard non-special parameters which have to    *
     * be initialized before we copy the environment variables. *
     * We don't want to override whatever values the user has   *
     * given them in the environment.                           */
    opts[ALLEXPORT] = 0;
    setiparam("MAILCHECK", 60);
    setiparam("KEYTIMEOUT", 40);
    setiparam("LISTMAX", 100);
    /*
     * We used to get the output baud rate here.  However, that's
     * pretty irrelevant to a terminal on an X display and can lead
     * to unnecessary delays if it's wrong (which it probably is).
     * Furthermore, even if the output is slow it's very likely
     * to be because of WAN delays, not covered by the output
     * baud rate.
     * So allow the user to set it in the special cases where it's
     * useful.
     */
    setsparam("TMPPREFIX", ztrdup_metafy(DEFAULT_TMPPREFIX));
    setsparam("TIMEFMT", ztrdup_metafy(DEFAULT_TIMEFMT));

    hostnam = (char *)zalloc(256);
    gethostname(hostnam, 256);
    setsparam("HOST", ztrdup_metafy(hostnam));
    zfree(hostnam, 256);

    setsparam("LOGNAME", ztrdup_metafy(
#ifndef DISABLE_DYNAMIC_NSS
			(str = getlogin()) && *str ?  str :
#endif
				cached_username
			));

#if !defined(HAVE_PUTENV) && !defined(USE_SET_UNSET_ENV)
    /* Copy the environment variables we are inheriting to dynamic *
     * memory, so we can do mallocs and frees on it.               */
    envsize = sizeof(char *)*(1 + arrlen(environ));
    new_environ = (char **) zalloc(envsize);
    memcpy(new_environ, environ, envsize);
    environ = new_environ;
#endif

    /* Use heap allocation to avoid many small alloc/free calls */
    pushheap();

    /* Now incorporate environment variables we are inheriting *
     * into the parameter hash table. Copy them into dynamic   *
     * memory so that we can free them if needed               */
    for (
#ifndef USE_SET_UNSET_ENV
	envp = 
#endif
	    envp2 = environ; *envp2; envp2++) {
	if (split_env_string(*envp2, &iname, &ivalue)) {
	    if (!idigit(*iname) && isident(iname) && !strchr(iname, '[')) {
		/*
		 * Parameters that aren't already in the parameter table
		 * aren't special to the shell, so it's always OK to
		 * import.  Otherwise, check parameter flags.
		 */
		if ((!(pm = (Param) paramtab->getnode(paramtab, iname)) ||
		     !dontimport(pm->node.flags)) &&
		    (pm = assignsparam(iname, metafy(ivalue, -1, META_DUP),
				       ASSPM_ENV_IMPORT))) {
		    pm->node.flags |= PM_EXPORTED;
		    if (pm->node.flags & PM_SPECIAL)
			pm->env = mkenvstr (pm->node.nam,
					    getsparam(pm->node.nam), pm->node.flags);
		    else
			pm->env = ztrdup(*envp2);
#ifndef USE_SET_UNSET_ENV
		    *envp++ = pm->env;
#endif
		}
	    }
	}
    }
    popheap();
#ifndef USE_SET_UNSET_ENV
    *envp = NULL;
#endif
    opts[ALLEXPORT] = oae;

    /*
     * For native emulation we always set the variable home
     * (see setupvals()).
     */
    pm = (Param) paramtab->getnode(paramtab, "HOME");
    if (EMULATION(EMULATE_ZSH))
    {
	pm->node.flags &= ~PM_UNSET;
	if (!(pm->node.flags & PM_EXPORTED))
	    addenv(pm, home);
    } else if (!home)
	pm->node.flags |= PM_UNSET;
    pm = (Param) paramtab->getnode(paramtab, "LOGNAME");
    if (!(pm->node.flags & PM_EXPORTED))
	addenv(pm, pm->u.str);
    pm = (Param) paramtab->getnode(paramtab, "SHLVL");
    sprintf(buf, "%d", (int)++shlvl);
    /* shlvl value in environment needs updating unconditionally */
    addenv(pm, buf);

    /* Add the standard non-special parameters */
    set_pwd_env();
#ifdef HAVE_UNAME
    if(uname(&unamebuf)) setsparam("CPUTYPE", ztrdup("unknown"));
    else
    {
       machinebuf = ztrdup_metafy(unamebuf.machine);
       setsparam("CPUTYPE", machinebuf);
    }

#else
    setsparam("CPUTYPE", ztrdup_metafy("unknown"));
#endif
    setsparam("MACHTYPE", ztrdup_metafy(MACHTYPE));
    setsparam("OSTYPE", ztrdup_metafy(OSTYPE));
    setsparam("TTY", ztrdup_metafy(ttystrname));
    setsparam("VENDOR", ztrdup_metafy(VENDOR));
    setsparam("ZSH_ARGZERO", ztrdup(posixzero));
    setsparam("ZSH_VERSION", ztrdup_metafy(ZSH_VERSION));
    setsparam("ZSH_PATCHLEVEL", ztrdup_metafy(ZSH_PATCHLEVEL));
    setaparam("signals", sigptr = zalloc((SIGCOUNT+4) * sizeof(char *)));
    for (t = sigs; (*sigptr++ = ztrdup_metafy(*t++)); );

    noerrs = 0;
}

/* assign various functions used for non-special parameters */

/**/
mod_export void
assigngetset(Param pm)
{
    switch (PM_TYPE(pm->node.flags)) {
    case PM_SCALAR:
	pm->gsu.s = &stdscalar_gsu;
	break;
    case PM_INTEGER:
	pm->gsu.i = &stdinteger_gsu;
	break;
    case PM_EFLOAT:
    case PM_FFLOAT:
	pm->gsu.f = &stdfloat_gsu;
	break;
    case PM_ARRAY:
	pm->gsu.a = &stdarray_gsu;
	break;
    case PM_HASHED:
	pm->gsu.h = &stdhash_gsu;
	break;
    default:
	DPUTS(1, "BUG: tried to create param node without valid flag");
	break;
    }
}

/* Create a parameter, so that it can be assigned to.  Returns NULL if the *
 * parameter already exists or can't be created, otherwise returns the     *
 * parameter node.  If a parameter of the same name exists in an outer     *
 * scope, it is hidden by a newly created parameter.  An already existing  *
 * parameter node at the current level may be `created' and returned       *
 * provided it is unset and not special.  If the parameter can't be        *
 * created because it already exists, the PM_UNSET flag is cleared.        */

/**/
mod_export Param
createparam(char *name, int flags)
{
    Param pm, oldpm;

    if (paramtab != realparamtab)
	flags = (flags & ~PM_EXPORTED) | PM_HASHELEM;

    if (name != nulstring) {
	oldpm = (Param) (paramtab == realparamtab ?
			 /* gethashnode2() for direct table read */
			 gethashnode2(paramtab, name) :
			 paramtab->getnode(paramtab, name));

	DPUTS(oldpm && oldpm->level > locallevel,
	      "BUG: old local parameter not deleted");
	if (oldpm && (oldpm->level == locallevel || !(flags & PM_LOCAL))) {
	    if (isset(POSIXBUILTINS) && (oldpm->node.flags & PM_READONLY)) {
		zerr("read-only variable: %s", name);
		return NULL;
	    }
	    if ((oldpm->node.flags & PM_RESTRICTED) && isset(RESTRICTED)) {
		zerr("%s: restricted", name);
		return NULL;
	    }
	    if (!(oldpm->node.flags & PM_UNSET) ||
		(oldpm->node.flags & PM_SPECIAL) ||
		/* POSIXBUILTINS horror: we need to retain 'export' flags */
		(isset(POSIXBUILTINS) && (oldpm->node.flags & PM_EXPORTED))) {
		if (oldpm->node.flags & PM_RO_BY_DESIGN) {
		    zerr("%s: can't change parameter attribute",
			 name);
		    return NULL;
		}
		oldpm->node.flags &= ~PM_UNSET;
		if ((oldpm->node.flags & PM_SPECIAL) && oldpm->ename) {
		    Param altpm =
			(Param) paramtab->getnode(paramtab, oldpm->ename);
		    if (altpm)
			altpm->node.flags &= ~PM_UNSET;
		}
		return NULL;
	    }

	    pm = oldpm;
	    pm->base = pm->width = 0;
	    oldpm = pm->old;
	} else {
	    pm = (Param) zshcalloc(sizeof *pm);
	    if ((pm->old = oldpm)) {
		/*
		 * needed to avoid freeing oldpm, but we do take it
		 * out of the environment when it's hidden.
		 */
		if (oldpm->env)
		    delenv(oldpm);
		paramtab->removenode(paramtab, name);
	    }
	    paramtab->addnode(paramtab, ztrdup(name), pm);
	}

	if (isset(ALLEXPORT) && !(flags & PM_HASHELEM))
	    flags |= PM_EXPORTED;
    } else {
	pm = (Param) hcalloc(sizeof *pm);
	pm->node.nam = nulstring;
    }
    pm->node.flags = flags & ~PM_LOCAL;

    if(!(pm->node.flags & PM_SPECIAL))
	assigngetset(pm);
    return pm;
}

/* Empty dummy function for special hash parameters. */

/**/
static void
shempty(void)
{
}

/*
 * Create a simple special hash parameter.
 *
 * This is for hashes added internally --- it's not possible to add
 * special hashes from shell commands.  It's currently used
 * - by addparamdef() for special parameters in the zsh/parameter
 *   module
 * - by ztie for special parameters tied to databases.
 */

/**/
mod_export Param
createspecialhash(char *name, GetNodeFunc get, ScanTabFunc scan, int flags)
{
    Param pm;
    HashTable ht;

    if (!(pm = createparam(name, PM_SPECIAL|PM_HASHED|flags)))
	return NULL;

    /*
     * If there's an old parameter, we'll put the new one at
     * the current locallevel, so that the old parameter is
     * exposed again after leaving the function.  Otherwise,
     * we'll leave it alone.  Usually this means the parameter
     * will stay in place until explicitly unloaded, however
     * if the parameter was previously unset within a function
     * we'll inherit the level of that function and follow the
     * standard convention that the parameter remains local
     * even if unset.
     *
     * These semantics are similar to those of a normal parameter set
     * within a function without a local definition.
     */
    if (pm->old)
	pm->level = locallevel;
    pm->gsu.h = (flags & PM_READONLY) ? &stdhash_gsu :
	&nullsethash_gsu;
    pm->u.hash = ht = newhashtable(0, name, NULL);

    ht->hash        = hasher;
    ht->emptytable  = (TableFunc) shempty;
    ht->filltable   = NULL;
    ht->addnode     = (AddNodeFunc) shempty;
    ht->getnode     = ht->getnode2 = get;
    ht->removenode  = (RemoveNodeFunc) shempty;
    ht->disablenode = NULL;
    ht->enablenode  = NULL;
    ht->freenode    = (FreeNodeFunc) shempty;
    ht->printnode   = printparamnode;
    ht->scantab     = scan;

    return pm;
}


/*
 * Copy a parameter
 *
 * If fakecopy is set, we are just saving the details of a special
 * parameter.  Otherwise, the result will be used as a real parameter
 * and we need to do more work.
 */

/**/
void
copyparam(Param tpm, Param pm, int fakecopy)
{
    /*
     * Note that tpm, into which we're copying, may not be in permanent
     * storage.  However, the values themselves are later used directly
     * to set the parameter, so must be permanently allocated (in accordance
     * with sets.?fn() usage).
     */
    tpm->node.flags = pm->node.flags;
    tpm->base = pm->base;
    tpm->width = pm->width;
    tpm->level = pm->level;
    if (!fakecopy) {
	tpm->old = pm->old;
	tpm->node.flags &= ~PM_SPECIAL;
    }
    switch (PM_TYPE(pm->node.flags)) {
    case PM_SCALAR:
	tpm->u.str = ztrdup(pm->gsu.s->getfn(pm));
	break;
    case PM_INTEGER:
	tpm->u.val = pm->gsu.i->getfn(pm);
	break;
    case PM_EFLOAT:
    case PM_FFLOAT:
	tpm->u.dval = pm->gsu.f->getfn(pm);
	break;
    case PM_ARRAY:
	tpm->u.arr = zarrdup(pm->gsu.a->getfn(pm));
	break;
    case PM_HASHED:
	tpm->u.hash = copyparamtable(pm->gsu.h->getfn(pm), pm->node.nam);
	break;
    }
    /*
     * If the value is going to be passed as a real parameter (e.g. this is
     * called from inside an associative array), we need the gets and sets
     * functions to be useful.
     *
     * In this case we assume the saved parameter is not itself special,
     * so we just use the standard functions.  This is also why we switch off
     * PM_SPECIAL.
     */
    if (!fakecopy)
	assigngetset(tpm);
}

/* Return 1 if the string s is a valid identifier, else return 0. */

/**/
mod_export int
isident(char *s)
{
    char *ss;

    if (!*s)			/* empty string is definitely not valid */
	return 0;

    if (idigit(*s)) {
	/* If the first character is `s' is a digit, then all must be */
	for (ss = ++s; *ss; ss++)
	    if (!idigit(*ss))
		break;
    } else {
	/* Find the first character in `s' not in the iident type table */
	ss = itype_end(s, IIDENT, 0);
    }

    /* If the next character is not [, then it is *
     * definitely not a valid identifier.         */
    if (!*ss)
	return 1;
    if (s == ss)
	return 0;
    if (*ss != '[')
	return 0;

    /* Require balanced [ ] pairs with something between */
    if (!(ss = parse_subscript(++ss, 1, ']')))
	return 0;
    untokenize(s);
    return !ss[1];
}

/*
 * Parse a single argument to a parameter subscript.
 * The subscripts starts at *str; *str is updated (input/output)
 *
 * *inv is set to indicate if the subscript is reversed (output)
 * v is the Value for the parameter being accessed (input; note
 *  v->isarr may be modified, and if v is a hash the parameter will
 *  be updated to the element of the hash)
 * a2 is 1 if this is the second subscript of a range (input)
 * *w is only set if we need to find the end of a word (input; should
 *  be set to 0 by the caller).
 *
 * The final two arguments are to support multibyte characters.
 * If supplied they are set to the length of the character before
 * the index position and the one at the index position.  If
 * multibyte characters are not in use they are set to 1 for
 * consistency.  Note they aren't fully handled if a2 is non-zero,
 * since they aren't needed.
 *
 * Returns a raw offset into the value from the start or end (i.e.
 * after the arithmetic for Meta and possible multibyte characters has
 * been taken into account).  This actually gives the offset *after*
 * the character in question; subtract *prevcharlen if necessary.
 */

/**/
static zlong
getarg(char **str, int *inv, Value v, int a2, zlong *w,
       int *prevcharlen, int *nextcharlen, int flags)
{
    int hasbeg = 0, word = 0, rev = 0, ind = 0, down = 0, l, i, ishash;
    int keymatch = 0, needtok = 0, arglen, len, inpar = 0;
    char *s = *str, *sep = NULL, *t, sav, *d, **ta, **p, *tt, c;
    zlong num = 1, beg = 0, r = 0, quote_arg = 0;
    Patprog pprog = NULL;

    /*
     * If in NO_EXEC mode, the parameters won't be set up properly,
     * so just pretend everything is a hash for subscript parsing
     */

    ishash = (unset(EXECOPT) ||
	      (v->pm && PM_TYPE(v->pm->node.flags) == PM_HASHED));
    if (prevcharlen)
	*prevcharlen = 1;
    if (nextcharlen)
	*nextcharlen = 1;

    /* first parse any subscription flags */
    if (v->pm && (*s == '(' || *s == Inpar)) {
	int escapes = 0;
	int waste;
	for (s++; *s != ')' && *s != Outpar && s != *str; s++) {
	    switch (*s) {
	    case 'r':
		rev = 1;
		keymatch = down = ind = 0;
		break;
	    case 'R':
		rev = down = 1;
		keymatch = ind = 0;
		break;
	    case 'k':
		keymatch = ishash;
		rev = 1;
		down = ind = 0;
		break;
	    case 'K':
		keymatch = ishash;
		rev = down = 1;
		ind = 0;
		break;
	    case 'i':
		rev = ind = 1;
		down = keymatch = 0;
		break;
	    case 'I':
		rev = ind = down = 1;
		keymatch = 0;
		break;
	    case 'w':
		/* If the parameter is a scalar, then make subscription *
		 * work on a per-word basis instead of characters.      */
		word = 1;
		break;
	    case 'f':
		word = 1;
		sep = "\n";
		break;
	    case 'e':
		quote_arg = 1;
		break;
	    case 'n':
		t = get_strarg(++s, &arglen);
		if (!*t)
		    goto flagerr;
		sav = *t;
		*t = '\0';
		num = mathevalarg(s + arglen, &d);
		if (!num)
		    num = 1;
		*t = sav;
		s = t + arglen - 1;
		break;
	    case 'b':
		hasbeg = 1;
		t = get_strarg(++s, &arglen);
		if (!*t)
		    goto flagerr;
		sav = *t;
		*t = '\0';
		if ((beg = mathevalarg(s + arglen, &d)) > 0)
		    beg--;
		*t = sav;
		s = t + arglen - 1;
		break;
	    case 'p':
		escapes = 1;
		break;
	    case 's':
		/* This gives the string that separates words *
		 * (for use with the `w' flag).               */
		t = get_strarg(++s, &arglen);
		if (!*t)
		    goto flagerr;
		sav = *t;
		*t = '\0';
		s += arglen;
		sep = escapes ? getkeystring(s, &waste, GETKEYS_SEP, NULL)
		    : dupstring(s);
		*t = sav;
		s = t + arglen - 1;
		break;
	    default:
	      flagerr:
		num = 1;
		word = rev = ind = down = keymatch = 0;
		sep = NULL;
		s = *str - 1;
	    }
	}
	if (s != *str)
	    s++;
    }
    if (num < 0) {
	down = !down;
	num = -num;
    }
    if (v->isarr & SCANPM_WANTKEYS)
	*inv = (ind || !(v->isarr & SCANPM_WANTVALS));
    else if (v->isarr & SCANPM_WANTVALS)
	*inv = 0;
    else {
	if (v->isarr) {
	    if (ind) {
		v->isarr |= SCANPM_WANTKEYS;
		v->isarr &= ~SCANPM_WANTVALS;
	    } else if (rev)
		v->isarr |= SCANPM_WANTVALS;
	    /*
	     * This catches the case where we are using "k" (rather
	     * than "K") on a hash.
	     */
	    if (!down && keymatch && ishash)
		v->isarr &= ~SCANPM_MATCHMANY;
	}
	*inv = ind;
    }

    for (t = s, i = 0;
	 (c = *t) &&
	     ((c != Outbrack && (ishash || c != ',')) || i || inpar);
	 t++) {
	/* Untokenize inull() except before brackets and double-quotes */
	if (inull(c)) {
	    c = t[1];
	    if (c == '[' || c == ']' ||
		c == '(' || c == ')' ||
		c == '{' || c == '}') {
		/* This test handles nested subscripts in hash keys */
		if (ishash && i)
		    *t = ztokens[*t - Pound];
		needtok = 1;
		++t;
	    } else if (c != '"')
		*t = ztokens[*t - Pound];
	    continue;
	}
	/* Inbrack and Outbrack are probably never found here ... */
	if (c == '[' || c == Inbrack)
	    i++;
	else if (c == ']' || c == Outbrack)
	    i--;
	if (c == '(' || c == Inpar)
	    inpar++;
	else if (c == ')' || c == Outpar)
	    inpar--;
	if (ispecial(c))
	    needtok = 1;
    }
    if (!c)
	return 0;
    *str = tt = t;

    /*
     * If in NO_EXEC mode, the parameters won't be set up properly,
     * so there's no additional sanity checking we can do.
     * Just return 0 now.
     */
    if (unset(EXECOPT))
	return 0;

    s = dupstrpfx(s, t - s);

    /* If we're NOT reverse subscripting, strip the inull()s so brackets *
     * are not backslashed after parsestr().  Otherwise leave them alone *
     * so that the brackets will be escaped when we patcompile() or when *
     * subscript arithmetic is performed (for nested subscripts).        */
    if (ishash && (keymatch || !rev))
	remnulargs(s);
    if (needtok) {
	s = dupstring(s);
	if (parsestr(&s))
	    return 0;
	singsub(&s);
    } else if (rev)
	remnulargs(s);	/* This is probably always a no-op, but ... */
    if (!rev) {
	if (ishash) {
	    HashTable ht = v->pm->gsu.h->getfn(v->pm);
	    if (!ht) {
		if (flags & SCANPM_CHECKING)
		    return 0;
		ht = newparamtable(17, v->pm->node.nam);
		v->pm->gsu.h->setfn(v->pm, ht);
	    }
	    untokenize(s);
	    if (!(v->pm = (Param) ht->getnode(ht, s))) {
		HashTable tht = paramtab;
		paramtab = ht;
		v->pm = createparam(s, PM_SCALAR|PM_UNSET);
		paramtab = tht;
	    }
	    v->isarr = (*inv ? SCANPM_WANTINDEX : 0);
	    v->start = 0;
	    *inv = 0;	/* We've already obtained the "index" (key) */
	    *w = v->end = -1;
	    r = isset(KSHARRAYS) ? 1 : 0;
	} else {
	    r = mathevalarg(s, &s);
	    if (isset(KSHARRAYS) && r >= 0)
		r++;
	}
	if (word && !v->isarr) {
	    s = t = getstrvalue(v);
	    i = wordcount(s, sep, 0);
	    if (r < 0)
		r += i + 1;
	    if (r < 1)
		r = 1;
	    if (r > i)
		r = i;
	    if (!s || !*s)
		return 0;
	    while ((d = findword(&s, sep)) && --r);
	    if (!d)
		return 0;

	    if (!a2 && *tt != ',')
		*w = (zlong)(s - t);

	    return (a2 ? s : d + 1) - t;
	} else if (!v->isarr && !word) {
	    int lastcharlen = 1;
	    s = getstrvalue(v);
	    /*
	     * Note for the confused (= pws):  the index r we
	     * have so far is that specified by the user.  The value
	     * passed back is an offset from the start or end of
	     * the string.  Hence it needs correcting at least
	     * for Meta characters and maybe for multibyte characters.
	     */
	    if (r > 0) {
		zlong nchars = r;

		MB_METACHARINIT();
		for (t = s; nchars && *t; nchars--)
		    t += (lastcharlen = MB_METACHARLEN(t));
		/* for consistency, keep any remainder off the end */
		r = (zlong)(t - s) + nchars;
		if (prevcharlen && !nchars /* ignore if off the end */)
		    *prevcharlen = lastcharlen;
		if (nextcharlen && *t)
		    *nextcharlen = MB_METACHARLEN(t);
	    } else if (r == 0) {
		if (prevcharlen)
		    *prevcharlen = 0;
		if (nextcharlen && *s) {
		    MB_METACHARINIT();
		    *nextcharlen = MB_METACHARLEN(s);
		}
	    } else {
		zlong nchars = (zlong)MB_METASTRLEN(s) + r;

		if (nchars < 0) {
		    /* make sure this isn't valid as a raw pointer */
		    r -= (zlong)strlen(s);
		} else {
		    MB_METACHARINIT();
		    for (t = s; nchars && *t; nchars--)
			t += (lastcharlen = MB_METACHARLEN(t));
		    r = - (zlong)strlen(t); /* keep negative */
		    if (prevcharlen)
			*prevcharlen = lastcharlen;
		    if (nextcharlen && *t)
			*nextcharlen = MB_METACHARLEN(t);
		}
	    }
	}
    } else {
	if (!v->isarr && !word && !quote_arg) {
	    l = strlen(s);
	    if (a2) {
		if (!l || *s != '*') {
		    d = (char *) hcalloc(l + 2);
		    *d = '*';
		    strcpy(d + 1, s);
		    s = d;
		}
	    } else {
		if (!l || s[l - 1] != '*' || (l > 1 && s[l - 2] == '\\')) {
		    d = (char *) hcalloc(l + 2);
		    strcpy(d, s);
		    strcat(d, "*");
		    s = d;
		}
	    }
	}
	if (!keymatch) {
	    if (quote_arg) {
		untokenize(s);
		/* Scalar (e) needs implicit asterisk tokens */
		if (!v->isarr && !word) {
		    l = strlen(s);
		    d = (char *) hcalloc(l + 2);
		    if (a2) {
			*d = Star;
			strcpy(d + 1, s);
		    } else {
			strcpy(d, s);
			d[l] = Star;
			d[l + 1] = '\0';
		    }
		    s = d;
		}
	    } else
		tokenize(s);
	    remnulargs(s);
	    pprog = patcompile(s, 0, NULL);
	} else
	    pprog = NULL;

	if (v->isarr) {
	    if (ishash) {
		scanprog = pprog;
		scanstr = s;
		if (keymatch)
		    v->isarr |= SCANPM_KEYMATCH;
		else {
		    if (!pprog)
			return 1;
		    if (ind)
			v->isarr |= SCANPM_MATCHKEY;
		    else
			v->isarr |= SCANPM_MATCHVAL;
		}
		if (down)
		    v->isarr |= SCANPM_MATCHMANY;
		if ((ta = getvaluearr(v)) &&
		    (*ta || ((v->isarr & SCANPM_MATCHMANY) &&
			     (v->isarr & (SCANPM_MATCHKEY | SCANPM_MATCHVAL |
					  SCANPM_KEYMATCH))))) {
		    *inv = (v->flags & VALFLAG_INV) ? 1 : 0;
		    *w = v->end;
		    scanprog = NULL;
		    return 1;
		}
		scanprog = NULL;
	    } else
		ta = getarrvalue(v);
	    if (!ta || !*ta)
		return !down;
	    len = arrlen(ta);
	    if (beg < 0)
		beg += len;
	    if (down) {
		if (beg < 0)
		    return 0;
	    } else if (beg >= len)
		return len + 1;
	    if (beg >= 0 && beg < len) {
		if (down) {
		    if (!hasbeg)
			beg = len - 1;
		    for (r = 1 + beg, p = ta + beg; p >= ta; r--, p--) {
			if (pprog && pattry(pprog, *p) && !--num)
			    return r;
		    }
		} else
		    for (r = 1 + beg, p = ta + beg; *p; r++, p++)
			if (pprog && pattry(pprog, *p) && !--num)
			    return r;
	    }
	} else if (word) {
	    ta = sepsplit(d = s = getstrvalue(v), sep, 1, 1);
	    len = arrlen(ta);
	    if (beg < 0)
		beg += len;
	    if (down) {
		if (beg < 0)
		    return 0;
	    } else if (beg >= len)
		return len + 1;
	    if (beg >= 0 && beg < len) {
		if (down) {
		    if (!hasbeg)
			beg = len - 1;
		    for (r = 1 + beg, p = ta + beg; p >= ta; p--, r--)
			if (pprog && pattry(pprog, *p) && !--num)
			    break;
		    if (p < ta)
			return 0;
		} else {
		    for (r = 1 + beg, p = ta + beg; *p; r++, p++)
			if (pprog && pattry(pprog, *p) && !--num)
			    break;
		    if (!*p)
			return 0;
		}
	    }
	    if (a2)
		r++;
	    for (i = 0; (t = findword(&d, sep)) && *t; i++)
		if (!--r) {
		    r = (zlong)(t - s + (a2 ? -1 : 1));
		    if (!a2 && *tt != ',')
			*w = r + strlen(ta[i]) - 1;
		    return r;
		}
	    return a2 ? -1 : 0;
	} else {
	    /* Searching characters */
	    int slen;
	    d = getstrvalue(v);
	    if (!d || !*d)
		return 0;
	    /*
	     * beg and len are character counts, not raw offsets.
	     * Remember we need to return a raw offset.
	     */
	    len = MB_METASTRLEN(d);
	    slen = strlen(d);
	    if (beg < 0)
		beg += len;
	    MB_METACHARINIT();
	    if (beg >= 0 && beg < len) {
		char *de = d + slen;

		if (a2) {
		    /*
		     * Second argument: we don't need to
		     * handle prevcharlen or nextcharlen, but
		     * we do need to handle characters appropriately.
		     */
		    if (down) {
			int nmatches = 0;
			char *lastpos = NULL;

			if (!hasbeg)
			    beg = len;

			/*
			 * See below: we have to move forward,
			 * but need to count from the end.
			 */
			for (t = d, r = 0; r <= beg; r++) {
			    sav = *t;
			    *t = '\0';
			    if (pprog && pattry(pprog, d)) {
				nmatches++;
				lastpos = t;
			    }
			    *t = sav;
			    if (t == de)
				break;
			    t += MB_METACHARLEN(t);
			}

			if (nmatches >= num) {
			    if (num > 1) {
				nmatches -= num;
				MB_METACHARINIT();
				for (t = d, r = 0; ; r++) {
				    sav = *t;
				    *t = '\0';
				    if (pprog && pattry(pprog, d) &&
					nmatches-- == 0) {
					lastpos = t;
					*t = sav;
					break;
				    }
				    *t = sav;
				    t += MB_METACHARLEN(t);
				}
			    }
			    /* else lastpos is already OK */

			    return lastpos - d;
			}
		    } else {
			/*
			 * This handling of the b flag
			 * gives odd results, but this is the
			 * way it's always worked.
			 */
			for (t = d; beg && t <= de; beg--)
			    t += MB_METACHARLEN(t);
			for (;;) {
			    sav = *t;
			    *t = '\0';
			    if (pprog && pattry(pprog, d) && !--num) {
				*t = sav;
				/*
				 * This time, don't increment
				 * pointer, since it's already
				 * after everything we matched.
				 */
				return t - d;
			    }
			    *t = sav;
			    if (t == de)
				break;
			    t += MB_METACHARLEN(t);
			}
		    }
		} else {
		    /*
		     * First argument: this is the only case
		     * where we need prevcharlen and nextcharlen.
		     */
		    int lastcharlen;

		    if (down) {
			int nmatches = 0;
			char *lastpos = NULL;

			if (!hasbeg)
			    beg = len;

			/*
			 * We can only move forward through
			 * multibyte strings, so record the
			 * matches.
			 * Unfortunately the count num works
			 * from the end, so it's easy to get the
			 * last one but we need to repeat if
			 * we want another one.
			 */
			for (t = d, r = 0; r <= beg; r++) {
			    if (pprog && pattry(pprog, t)) {
				nmatches++;
				lastpos = t;
			    }
			    if (t == de)
				break;
			    t += MB_METACHARLEN(t);
			}

			if (nmatches >= num) {
			    if (num > 1) {
				/*
				 * Need to start again and repeat
				 * to get the right match.
				 */
				nmatches -= num;
				MB_METACHARINIT();
				for (t = d, r = 0; ; r++) {
				    if (pprog && pattry(pprog, t) &&
					nmatches-- == 0) {
					lastpos = t;
					break;
				    }
				    t += MB_METACHARLEN(t);
				}
			    }
			    /* else lastpos is already OK */

			    /* return pointer after matched char */
			    lastpos +=
				(lastcharlen = MB_METACHARLEN(lastpos));
			    if (prevcharlen)
				*prevcharlen = lastcharlen;
			    if (nextcharlen)
				*nextcharlen = MB_METACHARLEN(lastpos);
			    return lastpos - d;
			}

			for (r = beg + 1, t = d + beg; t >= d; r--, t--) {
			    if (pprog && pattry(pprog, t) &&
				!--num)
				return r;
			}
		    } else {
			for (t = d; beg && t <= de; beg--)
			    t += MB_METACHARLEN(t);
			for (;;) {
			    if (pprog && pattry(pprog, t) && !--num) {
				/* return pointer after matched char */
				t += (lastcharlen = MB_METACHARLEN(t));
				if (prevcharlen)
				    *prevcharlen = lastcharlen;
				if (nextcharlen)
				    *nextcharlen = MB_METACHARLEN(t);
				return t - d;
			    }
			    if (t == de)
				break;
			    t += MB_METACHARLEN(t);
			}
		    }
		}
	    }
	    return down ? 0 : slen + 1;
	}
    }
    return r;
}

/*
 * Parse a subscript.
 *
 * pptr: In/Out parameter.  On entry, *ptr points to a "[foo]" string.  On exit
 * it will point one past the closing bracket.
 *
 * v: In/Out parameter.  Its .start and .end members (at least) will be updated
 * with the parsed indices.
 *
 * flags: can be either SCANPM_DQUOTED or zero.  Other bits are not used.
 */

/**/
int
getindex(char **pptr, Value v, int flags)
{
    int start, end, inv = 0;
    char *s = *pptr, *tbrack;

    *s++ = '[';
    /* Error handled after untokenizing */
    s = parse_subscript(s, flags & SCANPM_DQUOTED, ']');
    /* Now we untokenize everything except inull() markers so we can check *
     * for the '*' and '@' special subscripts.  The inull()s are removed  *
     * in getarg() after we know whether we're doing reverse indexing.    */
    for (tbrack = *pptr + 1; *tbrack && tbrack != s; tbrack++) {
	if (inull(*tbrack) && !*++tbrack)
	    break;
	if (itok(*tbrack))	/* Need to check for Nularg here? */
	    *tbrack = ztokens[*tbrack - Pound];
    }
    /* If we reached the end of the string (s == NULL) we have an error */
    if (*tbrack)
	*tbrack = Outbrack;
    else {
	zerr("invalid subscript");
	*pptr = tbrack;
	return 1;
    }
    s = *pptr + 1;
    if ((s[0] == '*' || s[0] == '@') && s + 1 == tbrack) {
	if ((v->isarr || IS_UNSET_VALUE(v)) && s[0] == '@')
	    v->isarr |= SCANPM_ISVAR_AT;
	v->start = 0;
	v->end = -1;
	s += 2;
    } else {
	zlong we = 0, dummy;
	int startprevlen, startnextlen;

	start = getarg(&s, &inv, v, 0, &we, &startprevlen, &startnextlen,
		       flags);

	if (inv) {
	    if (!v->isarr && start != 0) {
		char *t, *p;
		t = getstrvalue(v);
		/*
		 * Note for the confused (= pws): this is an inverse
		 * offset so at this stage we need to convert from
		 * the immediate offset into the value that we have
		 * into a logical character position.
		 */
		if (start > 0) {
		    int nstart = 0;
		    char *target = t + start - startprevlen;

		    p = t;
		    MB_METACHARINIT();
		    while (*p) {
			/*
			 * move up characters, counting how many we
			 * found
			 */
			p += MB_METACHARLEN(p);
			if (p < target)
			    nstart++;
			else {
			    if (p == target)
				nstart++;
			    else
				p = target; /* pretend we hit exactly */
			    break;
			}
		    }
		    /* if start was too big, keep the difference */
		    start = nstart + (target - p) + 1;
		} else {
		    zlong startoff = start + strlen(t);
#ifdef DEBUG
		    dputs("BUG: can't have negative inverse offsets???");
#endif
		    if (startoff < 0) {
			/* invalid: keep index but don't dereference */
			start = startoff;
		    } else {
			/* find start in full characters */
			MB_METACHARINIT();
			for (p = t; p < t + startoff;)
			    p += MB_METACHARLEN(p);
			start = - MB_METASTRLEN(p);
		    }
		}
	    }
	    if (start > 0 && (isset(KSHARRAYS) || (v->pm->node.flags & PM_HASHED)))
		start--;
	    if (v->isarr != SCANPM_WANTINDEX) {
		v->flags |= VALFLAG_INV;
		v->isarr = 0;
		v->start = start;
		v->end = start + 1;
	    }
	    if (*s == ',') {
		zerr("invalid subscript");
		*tbrack = ']';
		*pptr = tbrack+1;
		return 1;
	    }
	    if (s == tbrack)
		s++;
	} else {
	    int com;

	    if ((com = (*s == ','))) {
		s++;
		end = getarg(&s, &inv, v, 1, &dummy, NULL, NULL, flags);
	    } else {
		end = we ? we : start;
	    }
	    if (start != end)
		com = 1;
	    /*
	     * Somehow the logic sometimes forces us to use the previous
	     * or next character to what we would expect, which is
	     * why we had to calculate them in getarg().
	     */
	    if (start > 0)
		start -= startprevlen;
	    else if (start == 0 && end == 0)
	    {
		/*
		 * Strictly, this range is entirely off the
		 * start of the available index range.
		 * This can't happen with KSH_ARRAYS; we already
		 * altered the start index in getarg().
		 * Are we being strict?
		 */
		if (isset(KSHZEROSUBSCRIPT)) {
		    /*
		     * We're not.
		     * Treat this as accessing the first element of the
		     * array.
		     */
		    end = startnextlen;
		} else {
		    /*
		     * We are.  Flag that this range is invalid
		     * for setting elements.  Set the indexes
		     * to a range that returns empty for other accesses.
		     */
		    v->flags |= VALFLAG_EMPTY;
		    start = -1;
		    com = 1;
		}
	    }
	    if (s == tbrack) {
		s++;
		if (v->isarr && !com &&
		    (!(v->isarr & SCANPM_MATCHMANY) ||
		     !(v->isarr & (SCANPM_MATCHKEY | SCANPM_MATCHVAL |
				   SCANPM_KEYMATCH))))
		    v->isarr = 0;
		v->start = start;
		v->end = end;
	    } else
		s = *pptr;
	}
    }
    *tbrack = ']';
    *pptr = s;
    return 0;
}


/**/
mod_export Value
getvalue(Value v, char **pptr, int bracks)
{
  return fetchvalue(v, pptr, bracks, 0);
}

/**/
mod_export Value
fetchvalue(Value v, char **pptr, int bracks, int flags)
{
    char *s, *t, *ie;
    char sav, c;
    int ppar = 0;

    s = t = *pptr;

    if (idigit(c = *s)) {
	if (bracks >= 0)
	    ppar = zstrtol(s, &s, 10);
	else
	    ppar = *s++ - '0';
    }
    else if ((ie = itype_end(s, IIDENT, 0)) != s)
	s = ie;
    else if (c == Quest)
	*s++ = '?';
    else if (c == Pound)
	*s++ = '#';
    else if (c == String)
	*s++ = '$';
    else if (c == Qstring)
	*s++ = '$';
    else if (c == Star)
	*s++ = '*';
    else if (IS_DASH(c))
	*s++ = '-';
    else if (c == '#' || c == '?' || c == '$' ||
	     c == '!' || c == '@' || c == '*')
	s++;
    else
	return NULL;

    if ((sav = *s))
	*s = '\0';
    if (ppar) {
	if (v)
	    memset(v, 0, sizeof(*v));
	else
	    v = (Value) hcalloc(sizeof *v);
	v->pm = argvparam;
	v->flags = 0;
	v->start = ppar - 1;
	v->end = ppar;
	if (sav)
	    *s = sav;
    } else {
	Param pm;
	int isvarat;

        isvarat = (t[0] == '@' && !t[1]);
	pm = (Param) paramtab->getnode(paramtab, *t == '0' ? "0" : t);
	if (sav)
	    *s = sav;
	*pptr = s;
	if (!pm || ((pm->node.flags & PM_UNSET) &&
		    !(pm->node.flags & PM_DECLARED)))
	    return NULL;
	if (v)
	    memset(v, 0, sizeof(*v));
	else
	    v = (Value) hcalloc(sizeof *v);
	if (PM_TYPE(pm->node.flags) & (PM_ARRAY|PM_HASHED)) {
	    /* Overload v->isarr as the flag bits for hashed arrays. */
	    v->isarr = flags | (isvarat ? SCANPM_ISVAR_AT : 0);
	    /* If no flags were passed, we need something to represent *
	     * `true' yet differ from an explicit WANTVALS.  Use a     *
	     * special flag for this case.                             */
	    if (!v->isarr)
		v->isarr = SCANPM_ARRONLY;
	}
	v->pm = pm;
	v->flags = 0;
	v->start = 0;
	v->end = -1;
	if (bracks > 0 && (*s == '[' || *s == Inbrack)) {
	    if (getindex(&s, v, flags)) {
		*pptr = s;
		return v;
	    }
	} else if (!(flags & SCANPM_ASSIGNING) && v->isarr &&
		   itype_end(t, IIDENT, 1) != t && isset(KSHARRAYS))
	    v->end = 1, v->isarr = 0;
    }
    if (!bracks && *s)
	return NULL;
    *pptr = s;
#if 0
    /*
     * Check for large subscripts that might be erroneous.
     * This code is too gross in several ways:
     * - the limit is completely arbitrary
     * - the test vetoes operations on existing arrays
     * - it's not at all clear a general test on large arrays of
     *   this kind is any use.
     *
     * Until someone comes up with workable replacement code it's
     * therefore commented out.
     */
    if (v->start > MAX_ARRLEN) {
	zerr("subscript too %s: %d", "big", v->start + !isset(KSHARRAYS));
	return NULL;
    }
    if (v->start < -MAX_ARRLEN) {
	zerr("subscript too %s: %d", "small", v->start);
	return NULL;
    }
    if (v->end > MAX_ARRLEN+1) {
	zerr("subscript too %s: %d", "big", v->end - !!isset(KSHARRAYS));
	return NULL;
    }
    if (v->end < -MAX_ARRLEN) {
	zerr("subscript too %s: %d", "small", v->end);
	return NULL;
    }
#endif
    return v;
}

/**/
mod_export char *
getstrvalue(Value v)
{
    char *s, **ss;
    char buf[BDIGBUFSIZE];
    int len;

    if (!v)
	return hcalloc(1);

    if ((v->flags & VALFLAG_INV) && !(v->pm->node.flags & PM_HASHED)) {
	sprintf(buf, "%d", v->start);
	s = dupstring(buf);
	return s;
    }

    switch(PM_TYPE(v->pm->node.flags)) {
    case PM_HASHED:
	/* (!v->isarr) should be impossible unless emulating ksh */
	if (!v->isarr && EMULATION(EMULATE_KSH)) {
	    s = dupstring("[0]");
	    if (getindex(&s, v, 0) == 0)
		s = getstrvalue(v);
	    return s;
	} /* else fall through */
    case PM_ARRAY:
	ss = getvaluearr(v);
	if (v->isarr)
	    s = sepjoin(ss, NULL, 1);
	else {
	    if (v->start < 0)
		v->start += arrlen(ss);
	    s = (arrlen_le(ss, v->start) || v->start < 0) ?
		(char *) hcalloc(1) : ss[v->start];
	}
	return s;
    case PM_INTEGER:
	convbase(buf, v->pm->gsu.i->getfn(v->pm), v->pm->base);
	s = dupstring(buf);
	break;
    case PM_EFLOAT:
    case PM_FFLOAT:
	s = convfloat(v->pm->gsu.f->getfn(v->pm),
		      v->pm->base, v->pm->node.flags, NULL);
	break;
    case PM_SCALAR:
	s = v->pm->gsu.s->getfn(v->pm);
	break;
    default:
	s = "";
	DPUTS(1, "BUG: param node without valid type");
	break;
    }

    if (v->flags & VALFLAG_SUBST) {
	if (v->pm->node.flags & (PM_LEFT|PM_RIGHT_B|PM_RIGHT_Z)) {
	    size_t fwidth = v->pm->width ? (unsigned int)v->pm->width : MB_METASTRLEN(s);
	    switch (v->pm->node.flags & (PM_LEFT | PM_RIGHT_B | PM_RIGHT_Z)) {
		char *t, *tend;
		size_t t0;

	    case PM_LEFT:
	    case PM_LEFT | PM_RIGHT_Z:
		t = s;
		if (v->pm->node.flags & PM_RIGHT_Z)
		    while (*t == '0')
			t++;
		else
		    while (iblank(*t))
			t++;
		MB_METACHARINIT();
		for (tend = t, t0 = 0; t0 < fwidth && *tend; t0++)
		    tend += MB_METACHARLEN(tend);
		/*
		 * t0 is the number of characters from t used,
		 * hence (fwidth - t0) is the number of padding
		 * characters.  fwidth is a misnomer: we use
		 * character counts, not character widths.
		 *
		 * (tend - t) is the number of bytes we need
		 * to get fwidth characters or the entire string;
		 * the characters may be multiple bytes.
		 */
		fwidth -= t0; /* padding chars remaining */
		t0 = tend - t; /* bytes to copy from string */
		s = (char *) hcalloc(t0 + fwidth + 1);
		memcpy(s, t, t0);
		if (fwidth)
		    memset(s + t0, ' ', fwidth);
		s[t0 + fwidth] = '\0';
		break;
	    case PM_RIGHT_B:
	    case PM_RIGHT_Z:
	    case PM_RIGHT_Z | PM_RIGHT_B:
		{
		    int zero = 1;
		    /* Calculate length in possibly multibyte chars */
		    unsigned int charlen = MB_METASTRLEN(s);

		    if (charlen < fwidth) {
			char *valprefend = s;
			int preflen;
			if (v->pm->node.flags & PM_RIGHT_Z) {
			    /*
			     * This is a documented feature: when deciding
			     * whether to pad with zeroes, ignore
			     * leading blanks already in the value;
			     * only look for numbers after that.
			     * Not sure how useful this really is.
			     * It's certainly confusing to code around.
			     */
			    for (t = s; iblank(*t); t++)
				;
			    /*
			     * Allow padding after initial minus
			     * for numeric variables.
			     */
			    if ((v->pm->node.flags &
				 (PM_INTEGER|PM_EFLOAT|PM_FFLOAT)) &&
				*t == '-')
				t++;
			    /*
			     * Allow padding after initial 0x or
			     * base# for integer variables.
			     */
			    if (v->pm->node.flags & PM_INTEGER) {
				if (isset(CBASES) &&
				    t[0] == '0' && t[1] == 'x')
				    t += 2;
				else if ((valprefend = strchr(t, '#')))
				    t = valprefend + 1;
			    }
			    valprefend = t;
			    if (!*t)
				zero = 0;
			    else if (v->pm->node.flags &
				     (PM_INTEGER|PM_EFLOAT|PM_FFLOAT)) {
				/* zero always OK */
			    } else if (!idigit(*t))
				zero = 0;
			}
			/* number of characters needed for padding */
			fwidth -= charlen;
			/* bytes from original string */
			t0 = strlen(s);
			t = (char *) hcalloc(fwidth + t0 + 1);
			/* prefix guaranteed to be single byte chars */
			preflen = valprefend - s;
			memset(t + preflen, 
			       (((v->pm->node.flags & PM_RIGHT_B)
				 || !zero) ?       ' ' : '0'), fwidth);
			/*
			 * Copy - or 0x or base# before any padding
			 * zeroes.
			 */
			if (preflen)
			    memcpy(t, s, preflen);
			memcpy(t + preflen + fwidth,
			       valprefend, t0 - preflen);
			t[fwidth + t0] = '\0';
			s = t;
		    } else {
			/* Need to skip (charlen - fwidth) chars */
			for (t0 = charlen - fwidth; t0; t0--)
			    s += MB_METACHARLEN(s);
		    }
		}
		break;
	    }
	}
	switch (v->pm->node.flags & (PM_LOWER | PM_UPPER)) {
	case PM_LOWER:
	    s = casemodify(s, CASMOD_LOWER);
	    break;
	case PM_UPPER:
	    s = casemodify(s, CASMOD_UPPER);
	    break;
	}
    }
    if (v->start == 0 && v->end == -1)
	return s;

    len = strlen(s);
    if (v->start < 0) {
	v->start += len;
	if (v->start < 0)
	    v->start = 0;
    }
    if (v->end < 0) {
	v->end += len;
	if (v->end >= 0) {
	    char *eptr = s + v->end;
	    if (*eptr)
		v->end += MB_METACHARLEN(eptr);
	}
    }

    s = (v->start > len) ? dupstring("") :
	dupstring_wlen(s + v->start, len - v->start);

    if (v->end <= v->start)
	s[0] = '\0';
    else if (v->end - v->start <= len - v->start)
	s[v->end - v->start] = '\0';

    return s;
}

static char *nular[] = {"", NULL};

/**/
mod_export char **
getarrvalue(Value v)
{
    char **s;

    if (!v)
	return arrdup(nular);
    else if (IS_UNSET_VALUE(v))
	return arrdup(&nular[1]);
    if (v->flags & VALFLAG_INV) {
	char buf[DIGBUFSIZE];

	s = arrdup(nular);
	sprintf(buf, "%d", v->start);
	s[0] = dupstring(buf);
	return s;
    }
    s = getvaluearr(v);
    if (v->start == 0 && v->end == -1)
	return s;
    if (v->start < 0)
	v->start += arrlen(s);
    if (v->end < 0)
	v->end += arrlen(s) + 1;

    /* Null if 1) array too short, 2) index still negative */
    if (v->end <= v->start) {
	s = arrdup_max(nular, 0);
    }
    else if (v->start < 0) {
	s = arrdup_max(nular, 1);
    }
    else if (arrlen_le(s, v->start)) {
	/* Handle $ary[i,i] consistently for any $i > $#ary
	 * and $ary[i,j] consistently for any $j > $i > $#ary
	 */
	s = arrdup_max(nular, v->end - (v->start + 1));
    }
    else {
        /* Copy to a point before the end of the source array:
         * arrdup_max will copy at most v->end - v->start elements,
         * starting from v->start element. Original code said:
	 *  s[v->end - v->start] = NULL
         * which means that there are exactly the same number of
         * elements as the value of the above *0-based* index.
         */
	s = arrdup_max(s + v->start, v->end - v->start);
    }

    return s;
}

/**/
mod_export zlong
getintvalue(Value v)
{
    if (!v)
	return 0;
    if (v->flags & VALFLAG_INV)
	return v->start;
    if (v->isarr) {
	char **arr = getarrvalue(v);
	if (arr) {
	    char *scal = sepjoin(arr, NULL, 1);
	    return mathevali(scal);
	} else
	    return 0;
    }
    if (PM_TYPE(v->pm->node.flags) == PM_INTEGER)
	return v->pm->gsu.i->getfn(v->pm);
    if (v->pm->node.flags & (PM_EFLOAT|PM_FFLOAT))
	return (zlong)v->pm->gsu.f->getfn(v->pm);
    return mathevali(getstrvalue(v));
}

/**/
mnumber
getnumvalue(Value v)
{
    mnumber mn;
    mn.type = MN_INTEGER;


    if (!v) {
	mn.u.l = 0;
    } else if (v->flags & VALFLAG_INV) {
	mn.u.l = v->start;
    } else if (v->isarr) {
	char **arr = getarrvalue(v);
	if (arr) {
	    char *scal = sepjoin(arr, NULL, 1);
	    return matheval(scal);
	} else
	    mn.u.l = 0;
    } else if (PM_TYPE(v->pm->node.flags) == PM_INTEGER) {
	mn.u.l = v->pm->gsu.i->getfn(v->pm);
    } else if (v->pm->node.flags & (PM_EFLOAT|PM_FFLOAT)) {
	mn.type = MN_FLOAT;
	mn.u.d = v->pm->gsu.f->getfn(v->pm);
    } else
	return matheval(getstrvalue(v));
    return mn;
}

/**/
void
export_param(Param pm)
{
    char buf[BDIGBUFSIZE], *val;

    if (PM_TYPE(pm->node.flags) & (PM_ARRAY|PM_HASHED)) {
#if 0	/* Requires changes elsewhere in params.c and builtin.c */
	if (EMULATION(EMULATE_KSH) /* isset(KSHARRAYS) */) {
	    struct value v;
	    v.isarr = 1;
	    v.flags = 0;
	    v.start = 0;
	    v.end = -1;
	    val = getstrvalue(&v);
	} else
#endif
	    return;
    } else if (PM_TYPE(pm->node.flags) == PM_INTEGER)
	convbase(val = buf, pm->gsu.i->getfn(pm), pm->base);
    else if (pm->node.flags & (PM_EFLOAT|PM_FFLOAT))
	val = convfloat(pm->gsu.f->getfn(pm), pm->base,
			pm->node.flags, NULL);
    else
	val = pm->gsu.s->getfn(pm);

    addenv(pm, val);
}

/**/
mod_export void
setstrvalue(Value v, char *val)
{
    assignstrvalue(v, val, 0);
}

/**/
mod_export void
assignstrvalue(Value v, char *val, int flags)
{
    if (unset(EXECOPT))
	return;
    if (v->pm->node.flags & PM_READONLY) {
	zerr("read-only variable: %s", v->pm->node.nam);
	zsfree(val);
	return;
    }
    if ((v->pm->node.flags & PM_RESTRICTED) && isset(RESTRICTED)) {
	zerr("%s: restricted", v->pm->node.nam);
	zsfree(val);
	return;
    }
    if ((v->pm->node.flags & PM_HASHED) &&
	(v->isarr & (SCANPM_MATCHMANY|SCANPM_ARRONLY))) {
	zerr("%s: attempt to set slice of associative array", v->pm->node.nam);
	zsfree(val);
	return;
    }
    if (v->flags & VALFLAG_EMPTY) {
	zerr("%s: assignment to invalid subscript range", v->pm->node.nam);
	zsfree(val);
	return;
    }
    v->pm->node.flags &= ~PM_UNSET;
    switch (PM_TYPE(v->pm->node.flags)) {
    case PM_SCALAR:
	if (v->start == 0 && v->end == -1) {
	    v->pm->gsu.s->setfn(v->pm, val);
	    if ((v->pm->node.flags & (PM_LEFT | PM_RIGHT_B | PM_RIGHT_Z)) &&
		!v->pm->width)
		v->pm->width = strlen(val);
	} else {
	    char *z, *x;
            int zlen, vlen, newsize;

            z = v->pm->gsu.s->getfn(v->pm);
            zlen = strlen(z);

	    if ((v->flags & VALFLAG_INV) && unset(KSHARRAYS))
		v->start--, v->end--;
	    if (v->start < 0) {
		v->start += zlen;
		if (v->start < 0)
		    v->start = 0;
	    }
	    if (v->start > zlen)
		v->start = zlen;
	    if (v->end < 0) {
		v->end += zlen;
		if (v->end < 0) {
		    v->end = 0;
		} else if (v->end >= zlen) {
		    v->end = zlen;
		} else {
#ifdef MULTIBYTE_SUPPORT
		    if (isset(MULTIBYTE)) {
			v->end += MB_METACHARLEN(z + v->end);
		    } else {
			v->end++;
		    }
#else
		    v->end++;
#endif
		}
	    }
	    else if (v->end > zlen)
		v->end = zlen;

            vlen = strlen(val);
            /* Characters preceding start index +
               characters of what is assigned +
               characters following end index */
            newsize = v->start + vlen + (zlen - v->end);

            /* Does new size differ? */
            if (newsize != zlen || v->pm->gsu.s->setfn != strsetfn) {
                x = (char *) zalloc(newsize + 1);
                strncpy(x, z, v->start);
                strcpy(x + v->start, val);
                strcat(x + v->start, z + v->end);
                v->pm->gsu.s->setfn(v->pm, x);
            } else {
		Param pm = v->pm;
                /* Size doesn't change, can limit actions to only
                 * overwriting bytes in already allocated string */
		memcpy(z + v->start, val, vlen);
		/* Implement remainder of strsetfn */
		if (!(pm->node.flags & PM_HASHELEM) &&
		    ((pm->node.flags & PM_NAMEDDIR) ||
		     isset(AUTONAMEDIRS))) {
		    pm->node.flags |= PM_NAMEDDIR;
		    adduserdir(pm->node.nam, z, 0, 0);
		}
            }
            zsfree(val);
	}
	break;
    case PM_INTEGER:
	if (val) {
	    zlong ival;
	    if (flags & ASSPM_ENV_IMPORT) {
		char *ptr;
		ival = zstrtol_underscore(val, &ptr, 0, 1);
	    } else
		ival = mathevali(val);
	    v->pm->gsu.i->setfn(v->pm, ival);
	    if ((v->pm->node.flags & (PM_LEFT | PM_RIGHT_B | PM_RIGHT_Z)) &&
		!v->pm->width)
		v->pm->width = strlen(val);
	    zsfree(val);
	}
	if (!v->pm->base && lastbase != -1)
	    v->pm->base = lastbase;
	break;
    case PM_EFLOAT:
    case PM_FFLOAT:
	if (val) {
	    mnumber mn;
	    if (flags & ASSPM_ENV_IMPORT) {
		char *ptr;
		mn.type = MN_FLOAT;
		mn.u.d = strtod(val, &ptr);
	    } else
		mn = matheval(val);
	    v->pm->gsu.f->setfn(v->pm, (mn.type & MN_FLOAT) ? mn.u.d :
			       (double)mn.u.l);
	    if ((v->pm->node.flags & (PM_LEFT | PM_RIGHT_B | PM_RIGHT_Z)) &&
		!v->pm->width)
		v->pm->width = strlen(val);
	    zsfree(val);
	}
	break;
    case PM_ARRAY:
	{
	    char **ss = (char **) zalloc(2 * sizeof(char *));

	    ss[0] = val;
	    ss[1] = NULL;
	    setarrvalue(v, ss);
	}
	break;
    case PM_HASHED:
        {
	    if (foundparam == NULL)
	    {
		zerr("%s: attempt to set associative array to scalar",
		     v->pm->node.nam);
		zsfree(val);
		return;
	    }
	    else
		foundparam->gsu.s->setfn(foundparam, val);
        }
	break;
    }
    if ((!v->pm->env && !(v->pm->node.flags & PM_EXPORTED) &&
	 !(isset(ALLEXPORT) && !(v->pm->node.flags & PM_HASHELEM))) ||
	(v->pm->node.flags & PM_ARRAY) || v->pm->ename)
	return;
    export_param(v->pm);
}

/**/
void
setnumvalue(Value v, mnumber val)
{
    char buf[BDIGBUFSIZE], *p;

    if (unset(EXECOPT))
	return;
    if (v->pm->node.flags & PM_READONLY) {
	zerr("read-only variable: %s", v->pm->node.nam);
	return;
    }
    if ((v->pm->node.flags & PM_RESTRICTED) && isset(RESTRICTED)) {
	zerr("%s: restricted", v->pm->node.nam);
	return;
    }
    switch (PM_TYPE(v->pm->node.flags)) {
    case PM_SCALAR:
    case PM_ARRAY:
	if ((val.type & MN_INTEGER) || outputradix) {
	    if (!(val.type & MN_INTEGER))
		val.u.l = (zlong) val.u.d;
	    p = convbase_underscore(buf, val.u.l, outputradix,
				    outputunderscore);
	} else
	    p = convfloat_underscore(val.u.d, outputunderscore);
	setstrvalue(v, ztrdup(p));
	break;
    case PM_INTEGER:
	v->pm->gsu.i->setfn(v->pm, (val.type & MN_INTEGER) ? val.u.l :
			    (zlong) val.u.d);
	setstrvalue(v, NULL);
	break;
    case PM_EFLOAT:
    case PM_FFLOAT:
	v->pm->gsu.f->setfn(v->pm, (val.type & MN_INTEGER) ?
			    (double)val.u.l : val.u.d);
	setstrvalue(v, NULL);
	break;
    }
}

/**/
mod_export void
setarrvalue(Value v, char **val)
{
    if (unset(EXECOPT))
	return;
    if (v->pm->node.flags & PM_READONLY) {
	zerr("read-only variable: %s", v->pm->node.nam);
	freearray(val);
	return;
    }
    if ((v->pm->node.flags & PM_RESTRICTED) && isset(RESTRICTED)) {
	zerr("%s: restricted", v->pm->node.nam);
	freearray(val);
	return;
    }
    if (!(PM_TYPE(v->pm->node.flags) & (PM_ARRAY|PM_HASHED))) {
	freearray(val);
	zerr("%s: attempt to assign array value to non-array",
	     v->pm->node.nam);
	return;
    }
    if (v->flags & VALFLAG_EMPTY) {
	zerr("%s: assignment to invalid subscript range", v->pm->node.nam);
	freearray(val);
	return;
    }

    if (v->start == 0 && v->end == -1) {
	if (PM_TYPE(v->pm->node.flags) == PM_HASHED)
	    arrhashsetfn(v->pm, val, 0);
	else
	    v->pm->gsu.a->setfn(v->pm, val);
    } else if (v->start == -1 && v->end == 0 &&
    	    PM_TYPE(v->pm->node.flags) == PM_HASHED) {
    	arrhashsetfn(v->pm, val, ASSPM_AUGMENT);
    } else if ((PM_TYPE(v->pm->node.flags) == PM_HASHED)) {
	freearray(val);
	zerr("%s: attempt to set slice of associative array",
	     v->pm->node.nam);
	return;
    } else {
	char **const old = v->pm->gsu.a->getfn(v->pm);
	char **new;
	char **p, **q, **r; /* index variables */
	const int pre_assignment_length = arrlen(old);
	int post_assignment_length;
	int i;

	q = old;

	if ((v->flags & VALFLAG_INV) && unset(KSHARRAYS)) {
	    if (v->start > 0)
		v->start--;
	    v->end--;
	}
	if (v->start < 0) {
	    v->start += pre_assignment_length;
	    if (v->start < 0)
		v->start = 0;
	}
	if (v->end < 0) {
	    v->end += pre_assignment_length + 1;
	    if (v->end < 0)
		v->end = 0;
	}
	if (v->end < v->start)
	    v->end = v->start;

	post_assignment_length = v->start + arrlen(val);
	if (v->end < pre_assignment_length) {
	    /* 
	     * Allocate room for array elements between the end of the slice `v'
	     * and the original array's end.
	     */
	    post_assignment_length += pre_assignment_length - v->end;
	}

	if (pre_assignment_length == post_assignment_length
	    && v->pm->gsu.a->setfn == arrsetfn
	    /* ... and isn't something that arrsetfn() treats specially */
	    && 0 == (v->pm->node.flags & (PM_SPECIAL|PM_UNIQUE))
	    && NULL == v->pm->ename)
	{
	    /* v->start is 0-based */
	    p = old + v->start;
	    for (r = val; *r;) {
		/* Free previous string */
		zsfree(*p);
		/* Give away ownership of the string */
		*p++ = *r++;
	    }
	} else {
            /* arr+=( ... )
             * arr[${#arr}+x,...]=( ... ) */
            if (post_assignment_length > pre_assignment_length &&
                    pre_assignment_length <= v->start &&
                    pre_assignment_length > 0 &&
                    v->pm->gsu.a->setfn == arrsetfn)
            {
                p = new = (char **) zrealloc(old, sizeof(char *)
                                           * (post_assignment_length + 1));

                p += pre_assignment_length; /* after old elements */

                /* Consider 1 < 0, case for a=( 1 ); a[1,..] =
                 *          1 < 1, case for a=( 1 ); a[2,..] = */
                if (pre_assignment_length < v->start) {
                    for (i = pre_assignment_length; i < v->start; i++) {
                        *p++ = ztrdup("");
                    }
                }

                for (r = val; *r;) {
                    /* Give away ownership of the string */
                    *p++ = *r++;
                }

                /* v->end doesn't matter:
                 * a=( 1 2 ); a[4,100]=( a b ); echo "${(q@)a}"
                 * 1 2 '' a b */
                *p = NULL;

                v->pm->u.arr = NULL;
                v->pm->gsu.a->setfn(v->pm, new);
            } else {
                p = new = (char **) zalloc(sizeof(char *)
                                           * (post_assignment_length + 1));
                for (i = 0; i < v->start; i++)
                    *p++ = i < pre_assignment_length ? ztrdup(*q++) : ztrdup("");
                for (r = val; *r;) {
                    /* Give away ownership of the string */
                    *p++ = *r++;
                }
                if (v->end < pre_assignment_length)
                    for (q = old + v->end; *q;)
                        *p++ = ztrdup(*q++);
                *p = NULL;

                v->pm->gsu.a->setfn(v->pm, new);
            }

	    DPUTS2(p - new != post_assignment_length, "setarrvalue: wrong allocation: %d 1= %lu",
		   post_assignment_length, (unsigned long)(p - new));
	}

        /* Ownership of all strings has been
         * given away, can plainly free */
	free(val);
    }
}

/* Retrieve an integer parameter */

/**/
mod_export zlong
getiparam(char *s)
{
    struct value vbuf;
    Value v;

    if (!(v = getvalue(&vbuf, &s, 1)))
	return 0;
    return getintvalue(v);
}

/* Retrieve a numerical parameter, either integer or floating */

/**/
mnumber
getnparam(char *s)
{
    struct value vbuf;
    Value v;

    if (!(v = getvalue(&vbuf, &s, 1))) {
	mnumber mn;
	mn.type = MN_INTEGER;
	mn.u.l = 0;
	return mn;
    }
    return getnumvalue(v);
}

/* Retrieve a scalar (string) parameter */

/**/
mod_export char *
getsparam(char *s)
{
    struct value vbuf;
    Value v;

    if (!(v = getvalue(&vbuf, &s, 0)))
	return NULL;
    return getstrvalue(v);
}

/**/
mod_export char *
getsparam_u(char *s)
{
    if ((s = getsparam(s)))
	return unmetafy(s, NULL);
    return s;
}

/* Retrieve an array parameter */

/**/
mod_export char **
getaparam(char *s)
{
    struct value vbuf;
    Value v;

    if (!idigit(*s) && (v = getvalue(&vbuf, &s, 0)) &&
	PM_TYPE(v->pm->node.flags) == PM_ARRAY)
	return v->pm->gsu.a->getfn(v->pm);
    return NULL;
}

/* Retrieve an assoc array parameter as an array */

/**/
mod_export char **
gethparam(char *s)
{
    struct value vbuf;
    Value v;

    if (!idigit(*s) && (v = getvalue(&vbuf, &s, 0)) &&
	PM_TYPE(v->pm->node.flags) == PM_HASHED)
	return paramvalarr(v->pm->gsu.h->getfn(v->pm), SCANPM_WANTVALS);
    return NULL;
}

/* Retrieve the keys of an assoc array parameter as an array */

/**/
mod_export char **
gethkparam(char *s)
{
    struct value vbuf;
    Value v;

    if (!idigit(*s) && (v = getvalue(&vbuf, &s, 0)) &&
	PM_TYPE(v->pm->node.flags) == PM_HASHED)
	return paramvalarr(v->pm->gsu.h->getfn(v->pm), SCANPM_WANTKEYS);
    return NULL;
}

/*
 * Function behind WARNCREATEGLOBAL and WARNNESTEDVAR option.
 *
 * For WARNNESTEDVAR:
 * Called when the variable is created.
 * Apply heuristics to see if this variable was just created
 * globally but in a local context.
 *
 * For WARNNESTEDVAR:
 * Called when the variable already exists and is set.
 * Apply heuristics to see if this variable is setting
 * a variable that was created in a less nested function
 * or globally.
 */

/**/
static void
check_warn_pm(Param pm, const char *pmtype, int created,
	      int may_warn_about_nested_vars)
{
    Funcstack i;

    if (!may_warn_about_nested_vars && !created)
	return;

    if (created && isset(WARNCREATEGLOBAL)) {
	if (locallevel <= forklevel || pm->level != 0)
	    return;
    } else if (!created && isset(WARNNESTEDVAR)) {
	if (pm->level >= locallevel)
	    return;
    } else
	return;

    if (pm->node.flags & PM_SPECIAL)
	return;

    for (i = funcstack; i; i = i->prev) {
	if (i->tp == FS_FUNC) {
	    char *msg;
	    DPUTS(!i->name, "funcstack entry with no name");
	    msg = created ?
		"%s parameter %s created globally in function %s" :
		"%s parameter %s set in enclosing scope in function %s";
	    zwarn(msg, pmtype, pm->node.nam, i->name);
	    break;
	}
    }
}

/**/
mod_export Param
assignsparam(char *s, char *val, int flags)
{
    struct value vbuf;
    Value v;
    char *t = s;
    char *ss, *copy, *var;
    size_t lvar;
    mnumber lhs, rhs;
    int sstart, created = 0;

    if (!isident(s)) {
	zerr("not an identifier: %s", s);
	zsfree(val);
	errflag |= ERRFLAG_ERROR;
	return NULL;
    }
    queue_signals();
    if ((ss = strchr(s, '['))) {
	*ss = '\0';
	if (!(v = getvalue(&vbuf, &s, 1))) {
	    createparam(t, PM_ARRAY);
	    created = 1;
	} else {
	    if (v->pm->node.flags & PM_READONLY) {
		zerr("read-only variable: %s", v->pm->node.nam);
		*ss = '[';
		zsfree(val);
		unqueue_signals();
		return NULL;
	    }
	    /*
	     * Parameter defined here is a temporary bogus one.
	     * Don't warn about anything.
	     */
	    flags &= ~ASSPM_WARN;
	    v->pm->node.flags &= ~PM_DEFAULTED;
	}
	*ss = '[';
	v = NULL;
    } else {
	if (!(v = getvalue(&vbuf, &s, 1))) {
	    createparam(t, PM_SCALAR);
	    created = 1;
	} else if ((((v->pm->node.flags & PM_ARRAY) && !(flags & ASSPM_AUGMENT)) ||
	    	 (v->pm->node.flags & PM_HASHED)) &&
		 !(v->pm->node.flags & (PM_SPECIAL|PM_TIED)) && 
		 unset(KSHARRAYS)) {
	    unsetparam(t);
	    createparam(t, PM_SCALAR);
	    /* not regarded as a new creation */
	    v = NULL;
	}
    }
    if (!v && !(v = getvalue(&vbuf, &t, 1))) {
	unqueue_signals();
	zsfree(val);
	/* errflag |= ERRFLAG_ERROR; */
	return NULL;
    }
    if (flags & ASSPM_WARN)
	check_warn_pm(v->pm, "scalar", created, 1);
    v->pm->node.flags &= ~PM_DEFAULTED;
    if (flags & ASSPM_AUGMENT) {
	if (v->start == 0 && v->end == -1) {
	    switch (PM_TYPE(v->pm->node.flags)) {
	    case PM_SCALAR:
		v->start = INT_MAX;  /* just append to scalar value */
		break;
	    case PM_INTEGER:
	    case PM_EFLOAT:
	    case PM_FFLOAT:
		rhs = matheval(val);
		lhs = getnumvalue(v);
		if (lhs.type == MN_FLOAT) {
		    if ((rhs.type) == MN_FLOAT)
        		lhs.u.d = lhs.u.d + rhs.u.d;
		    else
			lhs.u.d = lhs.u.d + (double)rhs.u.l;
		} else {
        	    if ((rhs.type) == MN_INTEGER)
			lhs.u.l = lhs.u.l + rhs.u.l;
		    else
			lhs.u.l = lhs.u.l + (zlong)rhs.u.d;
		}
		setnumvalue(v, lhs);
    	    	unqueue_signals();
		zsfree(val);
		return v->pm; /* avoid later setstrvalue() call */
	    case PM_ARRAY:
	    	if (unset(KSHARRAYS)) {
		    v->start = arrlen(v->pm->gsu.a->getfn(v->pm));
		    v->end = v->start + 1;
		} else {
		    /* ksh appends scalar to first element */
		    v->end = 1;
		    goto kshappend;
		}
		break;
	    }
	} else {
	    switch (PM_TYPE(v->pm->node.flags)) {
	    case PM_SCALAR:
    		if (v->end > 0)
		    v->start = v->end;
		else
		    v->start = v->end = strlen(v->pm->gsu.s->getfn(v->pm)) +
			v->end + 1;
	    	break;
	    case PM_INTEGER:
	    case PM_EFLOAT:
	    case PM_FFLOAT:
		unqueue_signals();
		zerr("attempt to add to slice of a numeric variable");
		zsfree(val);
		return NULL;
	    case PM_ARRAY:
	      kshappend:
		/* treat slice as the end element */
		v->start = sstart = v->end > 0 ? v->end - 1 : v->end;
		v->isarr = 0;
		var = getstrvalue(v);
		v->start = sstart;
		copy = val;
		lvar = strlen(var);
		val = (char *)zalloc(lvar + strlen(val) + 1);
		strcpy(val, var);
		strcpy(val + lvar, copy);
		zsfree(copy);
		break;
	    }
	}
    }

    assignstrvalue(v, val, flags);
    unqueue_signals();
    return v->pm;
}

/**/
mod_export Param
setsparam(char *s, char *val)
{
    return assignsparam(s, val, ASSPM_WARN);
}

/**/
mod_export Param
assignaparam(char *s, char **val, int flags)
{
    struct value vbuf;
    Value v;
    char *t = s;
    char *ss;
    int created = 0;
    int may_warn_about_nested_vars = 1;

    if (!isident(s)) {
	zerr("not an identifier: %s", s);
	freearray(val);
	errflag |= ERRFLAG_ERROR;
	return NULL;
    }
    queue_signals();
    if ((ss = strchr(s, '['))) {
	*ss = '\0';
	if (!(v = getvalue(&vbuf, &s, 1))) {
	    createparam(t, PM_ARRAY);
	    created = 1;
	} else {
	    may_warn_about_nested_vars = 0;
	}
	*ss = '[';
	if (v && PM_TYPE(v->pm->node.flags) == PM_HASHED) {
	    unqueue_signals();
	    zerr("%s: attempt to set slice of associative array",
		 v->pm->node.nam);
	    freearray(val);
	    errflag |= ERRFLAG_ERROR;
	    return NULL;
	}
	v = NULL;
    } else {
	if (!(v = fetchvalue(&vbuf, &s, 1, SCANPM_ASSIGNING))) {
	    createparam(t, PM_ARRAY);
	    created = 1;
	} else if (!(PM_TYPE(v->pm->node.flags) & (PM_ARRAY|PM_HASHED)) &&
		 !(v->pm->node.flags & (PM_SPECIAL|PM_TIED))) {
	    int uniq = v->pm->node.flags & PM_UNIQUE;
	    if (flags & ASSPM_AUGMENT) {
	    	/* insert old value at the beginning of the val array */
		char **new;
		int lv = arrlen(val);

		new = (char **) zalloc(sizeof(char *) * (lv + 2));
		*new = ztrdup(getstrvalue(v));
		memcpy(new+1, val, sizeof(char *) * (lv + 1));
		free(val);
		val = new;
	    }
	    unsetparam(t);
	    createparam(t, PM_ARRAY | uniq);
	    v = NULL;
	}
    }
    if (!v)
	if (!(v = fetchvalue(&vbuf, &t, 1, SCANPM_ASSIGNING))) {
	    unqueue_signals();
	    freearray(val);
	    /* errflag |= ERRFLAG_ERROR; */
	    return NULL;
	}

    if (flags & ASSPM_WARN)
	check_warn_pm(v->pm, "array", created, may_warn_about_nested_vars);
    v->pm->node.flags &= ~PM_DEFAULTED;

    /*
     * At this point, we may have array entries consisting of
     * - a Marker element --- normally allocated array entry but
     *   with just Marker char and null
     * - an array index element --- as normal for associative array,
     *   but non-standard for normal array which we handle now.
     * - a value for the indexed element.
     * This only applies if the flag ASSPM_KEY_VALUE is passed in,
     * indicating prefork() detected this syntax.
     *
     * For associative arrays we just junk the Marker elements.
     */
    if (flags & ASSPM_KEY_VALUE) {
	char **aptr;
	if (PM_TYPE(v->pm->node.flags) & PM_ARRAY) {
	    /*
	     * This is an ordinary array with key / value pairs.
	     */
	    int maxlen, origlen, nextind;
	    char **fullval, **origptr;
	    zlong *subscripts = (zlong *)zhalloc(arrlen(val) * sizeof(zlong));
	    zlong *iptr = subscripts;
	    if (flags & ASSPM_AUGMENT) {
		origptr = v->pm->gsu.a->getfn(v->pm);
		maxlen = origlen = arrlen(origptr);
	    } else {
		maxlen = origlen = 0;
		origptr = NULL;
	    }
	    nextind = 0;
	    for (aptr = val; *aptr; ) {
		if (**aptr == Marker) {
		    *iptr = mathevali(*++aptr);
		    if (*iptr < 0 ||
			(!isset(KSHARRAYS) && *iptr == 0)) {
			unqueue_signals();
			zerr("bad subscript for direct array assignment: %s", *aptr);
			freearray(val);
			return NULL;
		    }
		    if (!isset(KSHARRAYS))
			--*iptr;
		    nextind = *iptr + 1;
		    ++iptr;
		    aptr += 2;
		} else {
		    ++nextind;
		    ++aptr;
		}
		if (nextind > maxlen)
		    maxlen = nextind;
	    }
	    fullval = zshcalloc((maxlen+1) * sizeof(char *));
	    if (!fullval) {
		zerr("array too large");
		freearray(val);
		return NULL;
	    }
	    fullval[maxlen] = NULL;
	    if (flags & ASSPM_AUGMENT) {
		char **srcptr = origptr;
		for (aptr = fullval; aptr <= fullval + origlen; aptr++) {
		    *aptr = ztrdup(*srcptr);
		    srcptr++;
		}
	    }
	    iptr = subscripts;
	    nextind = 0;
	    for (aptr = val; *aptr; ++aptr) {
		char *old;
		if (**aptr == Marker) {
		    int augment = ((*aptr)[1] == '+');
		    zsfree(*aptr);
		    zsfree(*++aptr); /* Index, no longer needed */
		    old = fullval[*iptr];
		    if (augment && old) {
			fullval[*iptr] = bicat(old, *++aptr);
			zsfree(*aptr);
		    } else {
			fullval[*iptr] = *++aptr;
		    }
		    nextind = *iptr + 1;
		    ++iptr;
		} else {
		    old = fullval[nextind];
		    fullval[nextind] = *aptr;
		    ++nextind;
		}
		if (old)
		    zsfree(old);
		/* aptr now on value in both cases */
	    }
	    if (*aptr) {		/* Shouldn't be possible */
		DPUTS(1, "Extra element in key / value array");
		zsfree(*aptr);
	    }
	    free(val);
	    for (aptr = fullval; aptr < fullval + maxlen; aptr++) {
		/*
		 * Remember we don't have sparse arrays but and they're null
		 * terminated --- so any value we don't set has to be an
		 * empty string.
		 */
		if (!*aptr)
		    *aptr = ztrdup("");
	    }
	    setarrvalue(v, fullval);
	    unqueue_signals();
	    return v->pm;
	} else if (PM_TYPE(v->pm->node.flags & PM_HASHED)) {
	    /*
	     * We strictly enforce [key]=value syntax for associative
	     * arrays.  Marker can only indicate a Marker / key / value
	     * triad; it cannot be there by accident.
	     *
	     * It's too inefficient to strip Markers here, and they
	     * can't be there in the other form --- so just ignore
	     * them willy nilly lower down.
	     */
	    for (aptr = val; *aptr; aptr += 3) {
		if (**aptr != Marker) {
		    unqueue_signals();
		    freearray(val);
		    zerr("bad [key]=value syntax for associative array");
		    return NULL;
		}
	    }
	} else {
	    unqueue_signals();
	    freearray(val);
	    zerr("invalid use of [key]=value assignment syntax");
	    return NULL;
	}
    }

    if (flags & ASSPM_AUGMENT) {
    	if (v->start == 0 && v->end == -1) {
	    if (PM_TYPE(v->pm->node.flags) & PM_ARRAY) {
	    	v->start = arrlen(v->pm->gsu.a->getfn(v->pm));
	    	v->end = v->start + 1;
	    } else if (PM_TYPE(v->pm->node.flags) & PM_HASHED)
	    	v->start = -1, v->end = 0;
	} else {
	    if (v->end > 0)
		v->start = v->end--;
	    else if (PM_TYPE(v->pm->node.flags) & PM_ARRAY) {
		v->end = arrlen(v->pm->gsu.a->getfn(v->pm)) + v->end;
		v->start = v->end + 1;
	    }
	}
    }

    setarrvalue(v, val);
    unqueue_signals();
    return v->pm;
}


/**/
mod_export Param
setaparam(char *s, char **aval)
{
    return assignaparam(s, aval, ASSPM_WARN);
}

/**/
mod_export Param
sethparam(char *s, char **val)
{
    struct value vbuf;
    Value v;
    char *t = s;
    int checkcreate = 0;

    if (!isident(s)) {
	zerr("not an identifier: %s", s);
	freearray(val);
	errflag |= ERRFLAG_ERROR;
	return NULL;
    }
    if (strchr(s, '[')) {
	freearray(val);
	zerr("nested associative arrays not yet supported");
	errflag |= ERRFLAG_ERROR;
	return NULL;
    }
    if (unset(EXECOPT))
	return NULL;
    queue_signals();
    if (!(v = fetchvalue(&vbuf, &s, 1, SCANPM_ASSIGNING))) {
	createparam(t, PM_HASHED);
	checkcreate = 1;
    } else if (!(PM_TYPE(v->pm->node.flags) & PM_HASHED)) {
	if (!(v->pm->node.flags & PM_SPECIAL)) {
	    unsetparam(t);
	    /* no WARNCREATEGLOBAL check here as parameter already existed */
	    createparam(t, PM_HASHED);
	    v = NULL;
	} else {
	    zerr("%s: can't change type of a special parameter", t);
	    unqueue_signals();
	    return NULL;
	}
    }
    if (!v)
	if (!(v = fetchvalue(&vbuf, &t, 1, SCANPM_ASSIGNING))) {
	    unqueue_signals();
	    /* errflag |= ERRFLAG_ERROR; */
	    return NULL;
	}
    check_warn_pm(v->pm, "associative array", checkcreate, 1);
    v->pm->node.flags &= ~PM_DEFAULTED;
    setarrvalue(v, val);
    unqueue_signals();
    return v->pm;
}


/*
 * Set a generic shell number, floating point or integer.
 * Option to warn on setting.
 */

/**/
mod_export Param
assignnparam(char *s, mnumber val, int flags)
{
    struct value vbuf;
    Value v;
    char *t = s, *ss;
    Param pm;
    int was_unset = 0;

    if (!isident(s)) {
	zerr("not an identifier: %s", s);
	errflag |= ERRFLAG_ERROR;
	return NULL;
    }
    if (unset(EXECOPT))
	return NULL;
    queue_signals();
    ss = strchr(s, '[');
    v = getvalue(&vbuf, &s, 1);
    if (v && (v->pm->node.flags & (PM_ARRAY|PM_HASHED)) &&
	!(v->pm->node.flags & (PM_SPECIAL|PM_TIED)) &&
	/*
	 * not sure what KSHARRAYS has got to do with this...
	 * copied this from assignsparam().
	 */
	unset(KSHARRAYS) && !ss) {
	unsetparam_pm(v->pm, 0, 1);
	was_unset = 1;
	s = t;
	v = NULL;
    }
    if (!v) {
	/* s has been updated by getvalue, so check again */
	ss = strchr(s, '[');
	if (ss)
	    *ss = '\0';
	pm = createparam(t, ss ? PM_ARRAY :
			 isset(POSIXIDENTIFIERS) ? PM_SCALAR :
			 (val.type & MN_INTEGER) ? PM_INTEGER : PM_FFLOAT);
	if (!pm)
	    pm = (Param) paramtab->getnode(paramtab, t);
	DPUTS(!pm, "BUG: parameter not created");
	if (ss) {
	    *ss = '[';
	} else if (val.type & MN_INTEGER) {
	    pm->base = outputradix;
	}
	if (!(v = getvalue(&vbuf, &t, 1))) {
	    DPUTS(!v, "BUG: value not found for new parameter");
	    /* errflag |= ERRFLAG_ERROR; */
	    unqueue_signals();
	    return NULL;
	}
	if (flags & ASSPM_WARN)
	    check_warn_pm(v->pm, "numeric", !was_unset, 1);
    } else {
	if (flags & ASSPM_WARN)
	    check_warn_pm(v->pm, "numeric", 0, 1);
    }
    v->pm->node.flags &= ~PM_DEFAULTED;
    setnumvalue(v, val);
    unqueue_signals();
    return v->pm;
}

/*
 * Set a generic shell number, floating point or integer.
 * Warn on setting based on option.
 */

/**/
mod_export Param
setnparam(char *s, mnumber val)
{
    return assignnparam(s, val, ASSPM_WARN);
}

/* Simplified interface to assignnparam */

/**/
mod_export Param
assigniparam(char *s, zlong val, int flags)
{
    mnumber mnval;
    mnval.type = MN_INTEGER;
    mnval.u.l = val;
    return assignnparam(s, mnval, flags);
}

/* Simplified interface to setnparam */

/**/
mod_export Param
setiparam(char *s, zlong val)
{
    mnumber mnval;
    mnval.type = MN_INTEGER;
    mnval.u.l = val;
    return assignnparam(s, mnval, ASSPM_WARN);
}

/*
 * Set an integer parameter without forcing creation of an integer type.
 * This is useful if the integer is going to be set to a parameter which
 * would usually be scalar but may not exist.
 */

/**/
mod_export Param
setiparam_no_convert(char *s, zlong val)
{
    /*
     * If the target is already an integer, thisgets converted
     * back.  Low technology rules.
     */
    char buf[BDIGBUFSIZE];
    convbase(buf, val, 10);
    return assignsparam(s, ztrdup(buf), ASSPM_WARN);
}

/* Unset a parameter */

/**/
mod_export void
unsetparam(char *s)
{
    Param pm;

    queue_signals();
    if ((pm = (Param) (paramtab == realparamtab ?
		       /* getnode2() to avoid autoloading */
		       paramtab->getnode2(paramtab, s) :
		       paramtab->getnode(paramtab, s))))
	unsetparam_pm(pm, 0, 1);
    unqueue_signals();
}

/* Unset a parameter
 *
 * altflag: if true, don't remove pm->ename from the environment
 * exp: See stdunsetfn()
 */

/**/
mod_export int
unsetparam_pm(Param pm, int altflag, int exp)
{
    Param oldpm, altpm;
    char *altremove;

    if ((pm->node.flags & PM_READONLY) && pm->level <= locallevel) {
	zerr("read-only variable: %s", pm->node.nam);
	return 1;
    }
    if ((pm->node.flags & PM_RESTRICTED) && isset(RESTRICTED)) {
	zerr("%s: restricted", pm->node.nam);
	return 1;
    }

    if (pm->ename && !altflag)
	altremove = ztrdup(pm->ename);
    else
	altremove = NULL;

    pm->node.flags &= ~PM_DECLARED;	/* like ksh, not like bash */
    if (!(pm->node.flags & PM_UNSET))
	pm->gsu.s->unsetfn(pm, exp);
    if (pm->env)
	delenv(pm);

    /* remove it under its alternate name if necessary */
    if (altremove) {
	altpm = (Param) paramtab->getnode(paramtab, altremove);
	/* tied parameters are at the same local level as each other */
	oldpm = NULL;
	/*
	 * Look for param under alternate name hidden by a local.
	 * If this parameter is special, however, the visible
	 * parameter is the special and the hidden one is keeping
	 * an old value --- we just mark the visible one as unset.
	 */
	if (altpm && !(altpm->node.flags & PM_SPECIAL))
	{
	    while (altpm && altpm->level > pm->level) {
		oldpm = altpm;
		altpm = altpm->old;
	    }
	}
	if (altpm) {
	    if (oldpm && !altpm->level) {
		oldpm->old = NULL;
		/* fudge things so removenode isn't called */
		altpm->level = 1;
	    }
	    unsetparam_pm(altpm, 1, exp);
	}

	zsfree(altremove);
	if (!(pm->node.flags & PM_SPECIAL))
	    pm->gsu.s = &stdscalar_gsu;
    }

    /*
     * If this was a local variable, we need to keep the old
     * struct so that it is resurrected at the right level.
     * This is partly because when an array/scalar value is set
     * and the parameter used to be the other sort, unsetparam()
     * is called.  Beyond that, there is an ambiguity:  should
     * foo() { local bar; unset bar; } make the global bar
     * available or not?  The following makes the answer "no".
     *
     * Some specials, such as those used in zle, still need removing
     * from the parameter table; they have the PM_REMOVABLE flag.
     */
    if ((pm->level && locallevel >= pm->level) ||
	(pm->node.flags & (PM_SPECIAL|PM_REMOVABLE)) == PM_SPECIAL)
	return 0;

    /* remove parameter node from table */
    paramtab->removenode(paramtab, pm->node.nam);

    if (pm->old) {
	oldpm = pm->old;
	paramtab->addnode(paramtab, oldpm->node.nam, oldpm);
	if ((PM_TYPE(oldpm->node.flags) == PM_SCALAR) &&
	    !(pm->node.flags & PM_HASHELEM) &&
	    (oldpm->node.flags & PM_NAMEDDIR) &&
	    oldpm->gsu.s == &stdscalar_gsu)
	    adduserdir(oldpm->node.nam, oldpm->u.str, 0, 0);
	if (oldpm->node.flags & PM_EXPORTED) {
	    /*
	     * Re-export the old value which we removed in typeset_single().
	     * I don't think we need to test for ALL_EXPORT here, since if
	     * it was used to export the parameter originally the parameter
	     * should still have the PM_EXPORTED flag.
	     */
	    export_param(oldpm);
	}
    }

    paramtab->freenode(&pm->node); /* free parameter node */

    return 0;
}

/* Standard function to unset a parameter.  This is mostly delegated to *
 * the specific set function.
 *
 * This could usefully be made type-specific, but then we need
 * to be more careful when calling the unset method directly.
 *
 * The "exp"licit parameter should be nonzero for assignments and the
 * unset command, and zero for implicit unset (e.g., end of scope).
 * Currently this is used only by some modules.
 */

/**/
mod_export void
stdunsetfn(Param pm, UNUSED(int exp))
{
    switch (PM_TYPE(pm->node.flags)) {
	case PM_SCALAR:
	    if (pm->gsu.s->setfn)
		pm->gsu.s->setfn(pm, NULL);
	    break;

	case PM_ARRAY:
	    if (pm->gsu.a->setfn)
		pm->gsu.a->setfn(pm, NULL);
	    break;

	case PM_HASHED:
	    if (pm->gsu.h->setfn)
		pm->gsu.h->setfn(pm, NULL);
	    break;

	default:
	    if (!(pm->node.flags & PM_SPECIAL))
	    	pm->u.str = NULL;
	    break;
    }
    if ((pm->node.flags & (PM_SPECIAL|PM_TIED)) == PM_TIED) {
	if (pm->ename) {
	    zsfree(pm->ename);
	    pm->ename = NULL;
	}
	pm->node.flags &= ~PM_TIED;
    }
    pm->node.flags |= PM_UNSET;
}

/* Function to get value of an integer parameter */

/**/
mod_export zlong
intgetfn(Param pm)
{
    return pm->u.val;
}

/* Function to set value of an integer parameter */

/**/
static void
intsetfn(Param pm, zlong x)
{
    pm->u.val = x;
}

/* Function to get value of a floating point parameter */

/**/
static double
floatgetfn(Param pm)
{
    return pm->u.dval;
}

/* Function to set value of an integer parameter */

/**/
static void
floatsetfn(Param pm, double x)
{
    pm->u.dval = x;
}

/* Function to get value of a scalar (string) parameter */

/**/
mod_export char *
strgetfn(Param pm)
{
    return pm->u.str ? pm->u.str : (char *) hcalloc(1);
}

/* Function to set value of a scalar (string) parameter */

/**/
mod_export void
strsetfn(Param pm, char *x)
{
    zsfree(pm->u.str);
    pm->u.str = x;
    if (!(pm->node.flags & PM_HASHELEM) &&
	((pm->node.flags & PM_NAMEDDIR) || isset(AUTONAMEDIRS))) {
	pm->node.flags |= PM_NAMEDDIR;
	adduserdir(pm->node.nam, x, 0, 0);
    }
    /* If you update this function, you may need to update the
     * `Implement remainder of strsetfn' block in assignstrvalue(). */
}

/* Function to get value of an array parameter */

static char *nullarray = NULL;

/**/
char **
arrgetfn(Param pm)
{
    return pm->u.arr ? pm->u.arr : &nullarray;
}

/* Function to set value of an array parameter */

/**/
mod_export void
arrsetfn(Param pm, char **x)
{
    if (pm->u.arr && pm->u.arr != x)
	freearray(pm->u.arr);
    if (pm->node.flags & PM_UNIQUE)
	uniqarray(x);
    pm->u.arr = x;
    /* Arrays tied to colon-arrays may need to fix the environment */
    if (pm->ename && x)
	arrfixenv(pm->ename, x);
    /* If you extend this function, update the list of conditions in
     * setarrvalue(). */
}

/* Function to get value of an association parameter */

/**/
mod_export HashTable
hashgetfn(Param pm)
{
    return pm->u.hash;
}

/* Function to set value of an association parameter */

/**/
mod_export void
hashsetfn(Param pm, HashTable x)
{
    if (pm->u.hash && pm->u.hash != x)
	deleteparamtable(pm->u.hash);
    pm->u.hash = x;
}

/* Function to dispose of setting of an unsettable hash */

/**/
mod_export void
nullsethashfn(UNUSED(Param pm), HashTable x)
{
    deleteparamtable(x);
}

/* Function to set value of an association parameter using key/value pairs */

/**/
static void
arrhashsetfn(Param pm, char **val, int flags)
{
    /* Best not to shortcut this by using the existing hash table,   *
     * since that could cause trouble for special hashes.  This way, *
     * it's up to pm->gsu.h->setfn() what to do.                     */
    int alen = 0;
    HashTable opmtab = paramtab, ht = 0;
    char **aptr;
    Value v = (Value) hcalloc(sizeof *v);
    v->end = -1;

    for (aptr = val; *aptr; ++aptr) {
	if (**aptr != Marker)
	    ++alen;
    }

    if (alen % 2) {
	freearray(val);
	zerr("bad set of key/value pairs for associative array");
	return;
    }
    if (flags & ASSPM_AUGMENT) {
	ht = paramtab = pm->gsu.h->getfn(pm);
    }
    if (alen && (!(flags & ASSPM_AUGMENT) || !paramtab)) {
	ht = paramtab = newparamtable(17, pm->node.nam);
    }
    for (aptr = val; *aptr; ) {
	int eltflags = 0;
	if (**aptr == Marker) {
	    /* Either all elements have Marker or none. Checked in caller. */
	    if ((*aptr)[1] == '+') {
		/* Actually, assignstrvalue currently doesn't handle this... */
		eltflags = ASSPM_AUGMENT;
		/* ...so we'll use the trick from setsparam(). */
		v->start = INT_MAX;
	    } else {
		v->start = 0;
	    }
	    v->end = -1;
	    zsfree(*aptr++);
	}
	/* The parameter name is ztrdup'd... */
	v->pm = createparam(*aptr, PM_SCALAR|PM_UNSET);
	/*
	 * createparam() doesn't return anything if the parameter
	 * already existed.
	 */
	if (!v->pm)
	    v->pm = (Param) paramtab->getnode(paramtab, *aptr);
	zsfree(*aptr++);
	/* ...but we can use the value without copying. */
	assignstrvalue(v, *aptr++, eltflags);
    }
    paramtab = opmtab;
    pm->gsu.h->setfn(pm, ht);
    free(val);		/* not freearray() */
}

/*
 * These functions are used as the set function for special parameters that
 * cannot be set by the user.  The set is incomplete as the only such
 * parameters are scalar and integer.
 */

/**/
mod_export void
nullstrsetfn(UNUSED(Param pm), char *x)
{
    zsfree(x);
}

/**/
mod_export void
nullintsetfn(UNUSED(Param pm), UNUSED(zlong x))
{}

/**/
mod_export void
nullunsetfn(UNUSED(Param pm), UNUSED(int exp))
{}


/* Function to get value of generic special integer *
 * parameter.  data is pointer to global variable   *
 * containing the integer value.                    */

/**/
mod_export zlong
intvargetfn(Param pm)
{
    return *pm->u.valptr;
}

/* Function to set value of generic special integer *
 * parameter.  data is pointer to global variable   *
 * where the value is to be stored.                 */

/**/
mod_export void
intvarsetfn(Param pm, zlong x)
{
    *pm->u.valptr = x;
}

/* Function to set value of any ZLE-related integer *
 * parameter.  data is pointer to global variable   *
 * where the value is to be stored.                 */

/**/
void
zlevarsetfn(Param pm, zlong x)
{
    zlong *p = pm->u.valptr;

    *p = x;
    if (p == &zterm_lines || p == &zterm_columns)
	adjustwinsize(2 + (p == &zterm_columns));
}


/* Implements gsu_integer.unsetfn for ZLE_RPROMPT_INDENT; see stdunsetfn() */

static void
rprompt_indent_unsetfn(Param pm, int exp)
{
    stdunsetfn(pm, exp);
    rprompt_indent = 1; /* Keep this in sync with init_term() */
}

/* Function to set value of generic special scalar    *
 * parameter.  data is pointer to a character pointer *
 * representing the scalar (string).                  */

/**/
mod_export void
strvarsetfn(Param pm, char *x)
{
    char **q = ((char **)pm->u.data);

    zsfree(*q);
    *q = x;
}

/* Function to get value of generic special scalar    *
 * parameter.  data is pointer to a character pointer *
 * representing the scalar (string).                  */

/**/
mod_export char *
strvargetfn(Param pm)
{
    char *s = *((char **)pm->u.data);

    if (!s)
	return hcalloc(1);
    return s;
}

/* Function to get value of generic special array  *
 * parameter.  data is a pointer to the pointer to *
 * a pointer (a pointer to a variable length array *
 * of pointers).                                   */

/**/
mod_export char **
arrvargetfn(Param pm)
{
    char **arrptr = *((char ***)pm->u.data);

    return arrptr ? arrptr : &nullarray;
}

/* Function to set value of generic special array parameter.    *
 * data is pointer to a variable length array of pointers which *
 * represents this array of scalars (strings).  If pm->ename is *
 * non NULL, then it is a colon separated environment variable  *
 * version of this array which will need to be updated.         */

/**/
mod_export void
arrvarsetfn(Param pm, char **x)
{
    char ***dptr = (char ***)pm->u.data;

    if (*dptr != x)
	freearray(*dptr);
    if (pm->node.flags & PM_UNIQUE)
	uniqarray(x);
    /*
     * Special tied arrays point to variables accessible in other
     * ways which need to be set to NULL.  We can't do this
     * with user tied variables since we can leak memory.
     */
    if ((pm->node.flags & PM_SPECIAL) && !x)
	*dptr = mkarray(NULL);
    else
	*dptr = x;
    if (pm->ename) {
	if (x)
	    arrfixenv(pm->ename, x);
	else if (*dptr == path)
	    pathchecked = path;
    }
}

/**/
mod_export char *
colonarrgetfn(Param pm)
{
    char ***dptr = (char ***)pm->u.data;
    return *dptr ? zjoin(*dptr, ':', 1) : "";
}

/**/
mod_export void
colonarrsetfn(Param pm, char *x)
{
    char ***dptr = (char ***)pm->u.data;
    /*
     * We have to make sure this is never NULL, since that
     * can cause problems.
     */
    if (*dptr)
	freearray(*dptr);
    if (x)
	*dptr = colonsplit(x, pm->node.flags & PM_UNIQUE);
    else
	*dptr = mkarray(NULL);
    arrfixenv(pm->node.nam, *dptr);
    zsfree(x);
}

/**/
char *
tiedarrgetfn(Param pm)
{
    struct tieddata *dptr = (struct tieddata *)pm->u.data;
    return *dptr->arrptr ? zjoin(*dptr->arrptr, STOUC(dptr->joinchar), 1) : "";
}

/**/
void
tiedarrsetfn(Param pm, char *x)
{
    struct tieddata *dptr = (struct tieddata *)pm->u.data;

    if (*dptr->arrptr)
	freearray(*dptr->arrptr);
    else if (pm->ename) {
	Param altpm = (Param) paramtab->getnode(paramtab, pm->ename);
	if (altpm)
	    altpm->node.flags &= ~PM_DEFAULTED;
    }
    if (x) {
	char sepbuf[3];
	if (imeta(dptr->joinchar))
	{
	    sepbuf[0] = Meta;
	    sepbuf[1] = dptr->joinchar ^ 32;
	    sepbuf[2] = '\0';
	}
	else
	{
	    sepbuf[0] = dptr->joinchar;
	    sepbuf[1] = '\0';
	}
	*dptr->arrptr = sepsplit(x, sepbuf, 0, 0);
	if (pm->node.flags & PM_UNIQUE)
	    uniqarray(*dptr->arrptr);
	zsfree(x);
    } else
	*dptr->arrptr = NULL;
    if (pm->ename)
	arrfixenv(pm->node.nam, *dptr->arrptr);
}

/**/
void
tiedarrunsetfn(Param pm, UNUSED(int exp))
{
    /*
     * Special unset function because we allocated a struct tieddata
     * in typeset_single to hold the special data which we now
     * need to delete.
     */
    pm->gsu.s->setfn(pm, NULL);
    zfree(pm->u.data, sizeof(struct tieddata));
    /* paranoia -- shouldn't need these, but in case we reuse the struct... */
    pm->u.data = NULL;
    zsfree(pm->ename);
    pm->ename = NULL;
    pm->node.flags &= ~PM_TIED;
    pm->node.flags |= PM_UNSET;
}

/**/
static void
simple_arrayuniq(char **x, int freeok)
{
    char **t, **p = x;
    char *hole = "";

    /* Find duplicates and replace them with holes */
    while (*++p)
	for (t = x; t < p; t++)
	    if (*t != hole && !strcmp(*p, *t)) {
		if (freeok)
		    zsfree(*p);
		*p = hole;
		break;
	    }
    /* Swap non-holes into holes in optimal jumps */
    for (p = t = x; *t != NULL; t++) {
	if (*t == hole) {
	    while (*p == hole)
		++p;
	    if ((*t = *p) != NULL)
		*p++ = hole;
	} else if (p == t)
	    p++;
    }
    /* Erase all the remaining holes, just in case */
    while (++t < p)
	*t = NULL;
}

/**/
static void
arrayuniq_freenode(HashNode hn)
{
    (void)hn;
}

/**/
HashTable
newuniqtable(zlong size)
{
    HashTable ht = newhashtable((int)size, "arrayuniq", NULL);
    /* ??? error checking */

    ht->hash        = hasher;
    ht->emptytable  = emptyhashtable;
    ht->filltable   = NULL;
    ht->cmpnodes    = strcmp;
    ht->addnode     = addhashnode;
    ht->getnode     = gethashnode2;
    ht->getnode2    = gethashnode2;
    ht->removenode  = removehashnode;
    ht->disablenode = disablehashnode;
    ht->enablenode  = enablehashnode;
    ht->freenode    = arrayuniq_freenode;
    ht->printnode   = NULL;

    return ht;
}

/**/
static void
arrayuniq(char **x, int freeok)
{
    char **it, **write_it;
    zlong array_size = arrlen(x);
    HashTable ht;

    if (array_size == 0)
	return;
    if (array_size < 10 || !(ht = newuniqtable(array_size + 1))) {
	/* fallback to simpler routine */
	simple_arrayuniq(x, freeok);
	return;
    }

    for (it = x, write_it = x; *it;) {
	if (! gethashnode2(ht, *it)) {
	    HashNode new_node = zhalloc(sizeof(struct hashnode));
	    if (!new_node) {
		/* Oops, out of heap memory, no way to recover */
		zerr("out of memory in arrayuniq");
		break;
	    }
	    (void) addhashnode2(ht, *it, new_node);
	    *write_it = *it;
	    if (it != write_it)
		*it = NULL;
	    ++write_it;
	}
	else {
	    if (freeok)
		zsfree(*it);
	    *it = NULL;
	}
	++it;
    }
    
    deletehashtable(ht);
}

/**/
void
uniqarray(char **x)
{
    if (!x || !*x)
	return;
    arrayuniq(x, !zheapptr(*x));
}

/**/
void
zhuniqarray(char **x)
{
    if (!x || !*x)
	return;
    arrayuniq(x, 0);
}

/* Function to get value of special parameter `#' and `ARGC' */

/**/
zlong
poundgetfn(UNUSED(Param pm))
{
    return arrlen(pparams);
}

/* Function to get value for special parameter `RANDOM' */

/**/
zlong
randomgetfn(UNUSED(Param pm))
{
    return rand() & 0x7fff;
}

/* Function to set value of special parameter `RANDOM' */

/**/
void
randomsetfn(UNUSED(Param pm), zlong v)
{
    srand((unsigned int)v);
}

/* Function to get value for special parameter `SECONDS' */

/**/
zlong
intsecondsgetfn(UNUSED(Param pm))
{
    struct timeval now;
    struct timezone dummy_tz;

    gettimeofday(&now, &dummy_tz);

    return (zlong)(now.tv_sec - shtimer.tv_sec -
		  (now.tv_usec < shtimer.tv_usec ? 1 : 0));
}

/* Function to set value of special parameter `SECONDS' */

/**/
void
intsecondssetfn(UNUSED(Param pm), zlong x)
{
    struct timeval now;
    struct timezone dummy_tz;
    zlong diff;

    gettimeofday(&now, &dummy_tz);
    diff = (zlong)now.tv_sec - x;
    shtimer.tv_sec = diff;
    if ((zlong)shtimer.tv_sec != diff)
	zwarn("SECONDS truncated on assignment");
    shtimer.tv_usec = now.tv_usec;
}

/**/
double
floatsecondsgetfn(UNUSED(Param pm))
{
    struct timeval now;
    struct timezone dummy_tz;

    gettimeofday(&now, &dummy_tz);

    return (double)(now.tv_sec - shtimer.tv_sec) +
	(double)(now.tv_usec - shtimer.tv_usec) / 1000000.0;
}

/**/
void
floatsecondssetfn(UNUSED(Param pm), double x)
{
    struct timeval now;
    struct timezone dummy_tz;

    gettimeofday(&now, &dummy_tz);
    shtimer.tv_sec = now.tv_sec - (zlong)x;
    shtimer.tv_usec = now.tv_usec - (zlong)((x - (zlong)x) * 1000000.0);
}

/**/
double
getrawseconds(void)
{
    return (double)shtimer.tv_sec + (double)shtimer.tv_usec / 1000000.0;
}

/**/
void
setrawseconds(double x)
{
    shtimer.tv_sec = (zlong)x;
    shtimer.tv_usec = (zlong)((x - (zlong)x) * 1000000.0);
}

/**/
int
setsecondstype(Param pm, int on, int off)
{
    int newflags = (pm->node.flags | on) & ~off;
    int tp = PM_TYPE(newflags);
    /* Only one of the numeric types is allowed. */
    if (tp == PM_EFLOAT || tp == PM_FFLOAT)
    {
	pm->gsu.f = &floatseconds_gsu;
    }
    else if (tp == PM_INTEGER)
    {
	pm->gsu.i = &intseconds_gsu;
    }
    else
	return 1;
    pm->node.flags = newflags;
    return 0;
}

/* Function to get value for special parameter `USERNAME' */

/**/
char *
usernamegetfn(UNUSED(Param pm))
{
    return get_username();
}

/* Function to set value of special parameter `USERNAME' */

/**/
void
usernamesetfn(UNUSED(Param pm), char *x)
{
#if defined(HAVE_SETUID) && defined(USE_GETPWNAM)
    struct passwd *pswd;

    if (x && (pswd = getpwnam(x)) && (pswd->pw_uid != cached_uid)) {
# ifdef USE_INITGROUPS
	initgroups(x, pswd->pw_gid);
# endif
	if (setgid(pswd->pw_gid))
	    zwarn("failed to change group ID: %e", errno);
	else if (setuid(pswd->pw_uid))
	    zwarn("failed to change user ID: %e", errno);
	else {
	    zsfree(cached_username);
	    cached_username = ztrdup(pswd->pw_name);
	    cached_uid = pswd->pw_uid;
	}
    }
#endif /* HAVE_SETUID && USE_GETPWNAM */
    zsfree(x);
}

/* Function to get value for special parameter `UID' */

/**/
zlong
uidgetfn(UNUSED(Param pm))
{
    return getuid();
}

/* Function to set value of special parameter `UID' */

/**/
void
uidsetfn(UNUSED(Param pm), zlong x)
{
#ifdef HAVE_SETUID
    if (setuid((uid_t)x))
	zerr("failed to change user ID: %e", errno);
#endif
}

/* Function to get value for special parameter `EUID' */

/**/
zlong
euidgetfn(UNUSED(Param pm))
{
    return geteuid();
}

/* Function to set value of special parameter `EUID' */

/**/
void
euidsetfn(UNUSED(Param pm), zlong x)
{
#ifdef HAVE_SETEUID
    if (seteuid((uid_t)x))
	zerr("failed to change effective user ID: %e", errno);
#endif
}

/* Function to get value for special parameter `GID' */

/**/
zlong
gidgetfn(UNUSED(Param pm))
{
    return getgid();
}

/* Function to set value of special parameter `GID' */

/**/
void
gidsetfn(UNUSED(Param pm), zlong x)
{
#ifdef HAVE_SETUID
    if (setgid((gid_t)x))
	zerr("failed to change group ID: %e", errno);
#endif
}

/* Function to get value for special parameter `EGID' */

/**/
zlong
egidgetfn(UNUSED(Param pm))
{
    return getegid();
}

/* Function to set value of special parameter `EGID' */

/**/
void
egidsetfn(UNUSED(Param pm), zlong x)
{
#ifdef HAVE_SETEUID
    if (setegid((gid_t)x))
	zerr("failed to change effective group ID: %e", errno);
#endif
}

/**/
zlong
ttyidlegetfn(UNUSED(Param pm))
{
    struct stat ttystat;

    if (SHTTY == -1 || fstat(SHTTY, &ttystat))
	return -1;
    return time(NULL) - ttystat.st_atime;
}

/* Function to get value for special parameter `IFS' */

/**/
char *
ifsgetfn(UNUSED(Param pm))
{
    return ifs;
}

/* Function to set value of special parameter `IFS' */

/**/
void
ifssetfn(UNUSED(Param pm), char *x)
{
    zsfree(ifs);
    ifs = x;
    inittyptab();
}

/* Functions to set value of special parameters `LANG' and `LC_*' */

#ifdef USE_LOCALE
static struct localename {
    char *name;
    int category;
} lc_names[] = {
#ifdef LC_COLLATE
    {"LC_COLLATE", LC_COLLATE},
#endif
#ifdef LC_CTYPE
    {"LC_CTYPE", LC_CTYPE},
#endif
#ifdef LC_MESSAGES
    {"LC_MESSAGES", LC_MESSAGES},
#endif
#ifdef LC_NUMERIC
    {"LC_NUMERIC", LC_NUMERIC},
#endif
#ifdef LC_TIME
    {"LC_TIME", LC_TIME},
#endif
    {NULL, 0}
};

/* On some systems (at least on NetBSD-9), when LC_CTYPE changes,
 * global variables (type mbstate_t) used by mbrtowc() etc. need be
 * reset by clear_mbstate() */

/**/
static void
clear_mbstate(void) {
#ifdef MULTIBYTE_SUPPORT
    mb_charinit();	/* utils.c */
    clear_shiftstate();	/* pattern.c */
#endif
}

/**/
static void
setlang(char *x)
{
    struct localename *ln;
    char *x2;

    if ((x2 = getsparam_u("LC_ALL")) && *x2)
	return;

    /*
     * Set the global locale to the value passed, but override
     * this with any non-empty definitions for specific
     * categories.
     *
     * We only use non-empty definitions because empty values aren't
     * valid as locales; when passed to setlocale() they mean "use the
     * environment variable", but if that's what we're setting the value
     * from this is meaningless.  So just all $LANG to show through in
     * that case.
     */
    setlocale(LC_ALL, x ? unmeta(x) : "");
    clear_mbstate();
    queue_signals();
    for (ln = lc_names; ln->name; ln++)
	if ((x = getsparam_u(ln->name)) && *x)
	    setlocale(ln->category, x);
    unqueue_signals();
}

/**/
void
lc_allsetfn(Param pm, char *x)
{
    strsetfn(pm, x);
    /*
     * Treat an empty LC_ALL the same as an unset one,
     * namely by using LANG as the default locale but overriding
     * that with any LC_* that are set.
     */
    if (!x || !*x) {
	x = getsparam_u("LANG");
	if (x && *x) {
	    queue_signals();
	    setlang(x);
	    unqueue_signals();
	}
    }
    else {
	setlocale(LC_ALL, unmeta(x));
	clear_mbstate();
    }
}

/**/
void
langsetfn(Param pm, char *x)
{
    strsetfn(pm, x);
    setlang(unmeta(x));
}

/**/
void
lcsetfn(Param pm, char *x)
{
    char *x2;
    struct localename *ln;

    strsetfn(pm, x);
    if ((x2 = getsparam("LC_ALL")) && *x2)
	return;
    queue_signals();
    /* Treat empty LC_* the same as unset. */
    if (!x || !*x)
	x = getsparam("LANG");

    /*
     * If we've got no non-empty string at this
     * point (after checking $LANG, too),
     * we shouldn't bother setting anything.
     */
    if (x && *x) {
	for (ln = lc_names; ln->name; ln++)
	    if (!strcmp(ln->name, pm->node.nam))
		setlocale(ln->category, unmeta(x));
    }
    unqueue_signals();
    clear_mbstate();	/* LC_CTYPE may have changed */
}
#endif /* USE_LOCALE */

/* Function to set value for special parameter `0' */

/**/
static void
argzerosetfn(UNUSED(Param pm), char *x)
{
    if (x) {
	if (isset(POSIXARGZERO))
	    zerr("read-only variable: 0");
	else {
	    zsfree(argzero);
	    argzero = ztrdup(x);
	}
	zsfree(x);
    }
}

/* Function to get value for special parameter `0' */

/**/
static char *
argzerogetfn(UNUSED(Param pm))
{
    if (isset(POSIXARGZERO))
	return posixzero;
    return argzero;
}

/* Function to get value for special parameter `HISTSIZE' */

/**/
zlong
histsizegetfn(UNUSED(Param pm))
{
    return histsiz;
}

/* Function to set value of special parameter `HISTSIZE' */

/**/
void
histsizesetfn(UNUSED(Param pm), zlong v)
{
    if ((histsiz = v) < 1)
	histsiz = 1;
    resizehistents();
}

/* Function to get value for special parameter `SAVEHIST' */

/**/
zlong
savehistsizegetfn(UNUSED(Param pm))
{
    return savehistsiz;
}

/* Function to set value of special parameter `SAVEHIST' */

/**/
void
savehistsizesetfn(UNUSED(Param pm), zlong v)
{
    if ((savehistsiz = v) < 0)
	savehistsiz = 0;
}

/* Function to set value for special parameter `ERRNO' */

/**/
void
errnosetfn(UNUSED(Param pm), zlong x)
{
    errno = (int)x;
    if ((zlong)errno != x)
	zwarn("errno truncated on assignment");
}

/* Function to get value for special parameter `ERRNO' */

/**/
zlong
errnogetfn(UNUSED(Param pm))
{
    return errno;
}

/* Function to get value for special parameter `KEYBOARD_HACK' */

/**/
char *
keyboardhackgetfn(UNUSED(Param pm))
{
    static char buf[2];

    buf[0] = keyboardhackchar;
    buf[1] = '\0';
    return buf;
}


/* Function to set value of special parameter `KEYBOARD_HACK' */

/**/
void
keyboardhacksetfn(UNUSED(Param pm), char *x)
{
    if (x) {
	int len, i;

	unmetafy(x, &len);
	if (len > 1) {
	    len = 1;
	    zwarn("Only one KEYBOARD_HACK character can be defined");  /* could be changed if needed */
	}
	for (i = 0; i < len; i++) {
	    if (!isascii(STOUC(x[i]))) {
		zwarn("KEYBOARD_HACK can only contain ASCII characters");
		return;
	    }
	}
	keyboardhackchar = len ? STOUC(x[0]) : '\0';
	free(x);
    } else
	keyboardhackchar = '\0';
}

/* Function to get value for special parameter `histchar' */

/**/
char *
histcharsgetfn(UNUSED(Param pm))
{
    static char buf[4];

    buf[0] = bangchar;
    buf[1] = hatchar;
    buf[2] = hashchar;
    buf[3] = '\0';
    return buf;
}

/* Function to set value of special parameter `histchar' */

/**/
void
histcharssetfn(UNUSED(Param pm), char *x)
{
    if (x) {
	int len, i;

	unmetafy(x, &len);
	if (len > 3)
	    len = 3;
	for (i = 0; i < len; i++) {
	    if (!isascii(STOUC(x[i]))) {
		zwarn("HISTCHARS can only contain ASCII characters");
		return;
	    }
	}
	bangchar = len ? STOUC(x[0]) : '\0';
	hatchar =  len > 1 ? STOUC(x[1]) : '\0';
	hashchar = len > 2 ? STOUC(x[2]) : '\0';
	free(x);
    } else {
	bangchar = '!';
	hashchar = '#';
	hatchar = '^';
    }
    inittyptab();
}

/* Function to get value for special parameter `HOME' */

/**/
char *
homegetfn(UNUSED(Param pm))
{
    return home;
}

/* Function to set value of special parameter `HOME' */

/**/
void
homesetfn(UNUSED(Param pm), char *x)
{
    zsfree(home);
    if (x && isset(CHASELINKS) && (home = xsymlink(x, 0)))
	zsfree(x);
    else
	home = x ? x : ztrdup("");
    finddir(NULL);
}

/* Function to get value for special parameter `WORDCHARS' */

/**/
char *
wordcharsgetfn(UNUSED(Param pm))
{
    return wordchars;
}

/* Function to set value of special parameter `WORDCHARS' */

/**/
void
wordcharssetfn(UNUSED(Param pm), char *x)
{
    zsfree(wordchars);
    wordchars = x;
    inittyptab();
}

/* Function to get value for special parameter `_' */

/**/
char *
underscoregetfn(UNUSED(Param pm))
{
    char *u = dupstring(zunderscore);

    untokenize(u);
    return u;
}

/* Function used when we need to reinitialise the terminal */

static void
term_reinit_from_pm(void)
{
    /* If non-interactive, delay setting up term till we need it. */
    if (unset(INTERACTIVE) || !*term)
	termflags |= TERM_UNKNOWN;
    else
	init_term();
}

/* Function to get value for special parameter `TERM' */

/**/
char *
termgetfn(UNUSED(Param pm))
{
    return term;
}

/* Function to set value of special parameter `TERM' */

/**/
void
termsetfn(UNUSED(Param pm), char *x)
{
    zsfree(term);
    term = x ? x : ztrdup("");
    term_reinit_from_pm();
}

/* Function to get value of special parameter `TERMINFO' */

/**/
char *
terminfogetfn(UNUSED(Param pm))
{
    return zsh_terminfo ? zsh_terminfo : dupstring("");
}

/* Function to set value of special parameter `TERMINFO' */

/**/
void
terminfosetfn(Param pm, char *x)
{
    zsfree(zsh_terminfo);
    zsh_terminfo = x;

    /*
     * terminfo relies on the value being exported before
     * we reinitialise the terminal.  This is a bit inefficient.
     */
    if ((pm->node.flags & PM_EXPORTED) && x)
	addenv(pm, x);

    term_reinit_from_pm();
}

/* Function to get value of special parameter `TERMINFO_DIRS' */

/**/
char *
terminfodirsgetfn(UNUSED(Param pm))
{
    return zsh_terminfodirs ? zsh_terminfodirs : dupstring("");
}

/* Function to set value of special parameter `TERMINFO_DIRS' */

/**/
void
terminfodirssetfn(Param pm, char *x)
{
    zsfree(zsh_terminfodirs);
    zsh_terminfodirs = x;

    /*
     * terminfo relies on the value being exported before
     * we reinitialise the terminal.  This is a bit inefficient.
     */
    if ((pm->node.flags & PM_EXPORTED) && x)
	addenv(pm, x);

    term_reinit_from_pm();
}
/* Function to get value for special parameter `pipestatus' */

/**/
static char **
pipestatgetfn(UNUSED(Param pm))
{
    char **x = (char **) zhalloc((numpipestats + 1) * sizeof(char *));
    char buf[DIGBUFSIZE], **p;
    int *q, i;

    for (p = x, q = pipestats, i = numpipestats; i--; p++, q++) {
	sprintf(buf, "%d", *q);
	*p = dupstring(buf);
    }
    *p = NULL;

    return x;
}

/* Function to get value for special parameter `pipestatus' */

/**/
static void
pipestatsetfn(UNUSED(Param pm), char **x)
{
    if (x) {
        int i;

        for (i = 0; *x && i < MAX_PIPESTATS; i++, x++)
            pipestats[i] = atoi(*x);
        numpipestats = i;
    }
    else
        numpipestats = 0;
}

/**/
void
arrfixenv(char *s, char **t)
{
    Param pm;
    int joinchar;

    if (t == path)
	cmdnamtab->emptytable(cmdnamtab);

    pm = (Param) paramtab->getnode(paramtab, s);
    
    /*
     * Only one level of a parameter can be exported.  Unless
     * ALLEXPORT is set, this must be global.
     */

    if (pm->node.flags & PM_HASHELEM)
	return;

    if (isset(ALLEXPORT))
	pm->node.flags |= PM_EXPORTED;
    pm->node.flags &= ~PM_DEFAULTED;

    /*
     * Do not "fix" parameters that were not exported
     */

    if (!(pm->node.flags & PM_EXPORTED))
	return;

    if (pm->node.flags & PM_SPECIAL)
	joinchar = ':';
    else
	joinchar = STOUC(((struct tieddata *)pm->u.data)->joinchar);

    addenv(pm, t ? zjoin(t, joinchar, 1) : "");
}


/**/
int
zputenv(char *str)
{
    DPUTS(!str, "Attempt to put null string into environment.");
#ifdef USE_SET_UNSET_ENV
    /*
     * If we are using unsetenv() to remove values from the
     * environment, which is the safe thing to do, we
     * need to use setenv() to put them there in the first place.
     * Unfortunately this is a slightly different interface
     * from what zputenv() assumes.
     */
    char *ptr;
    int ret;

    for (ptr = str; *ptr && STOUC(*ptr) < 128 && *ptr != '='; ptr++)
	;
    if (STOUC(*ptr) >= 128) {
	/*
	 * Environment variables not in the portable character
	 * set are non-standard and we don't really know of
	 * a use for them.
	 *
	 * We'll disable until someone complains.
	 */
	return 1;
    } else if (*ptr) {
	*ptr = '\0';
	ret = setenv(str, ptr+1, 1);
	*ptr = '=';
    } else {
	/* safety first */
	DPUTS(1, "bad environment string");
	ret = setenv(str, ptr, 1);
    }
    return ret;
#else
#ifdef HAVE_PUTENV
    return putenv(str);
#else
    char **ep;
    int num_env;


    /* First check if there is already an environment *
     * variable matching string `name'.               */
    if (findenv(str, &num_env)) {
	environ[num_env] = str;
    } else {
    /* Else we have to make room and add it */
	num_env = arrlen(environ);
	environ = (char **) zrealloc(environ, (sizeof(char *)) * (num_env + 2));

	/* Now add it at the end */
	ep = environ + num_env;
	*ep = str;
	*(ep + 1) = NULL;
    }
    return 0;
#endif
#endif
}

/**/
#ifndef USE_SET_UNSET_ENV
/**/
static int
findenv(char *name, int *pos)
{
    char **ep, *eq;
    int  nlen;


    eq = strchr(name, '=');
    nlen = eq ? eq - name : (int)strlen(name);
    for (ep = environ; *ep; ep++) 
	if (!strncmp (*ep, name, nlen) && *((*ep)+nlen) == '=') {
	    if (pos)
		*pos = ep - environ;
	    return 1;
	}
    
    return 0;
}
/**/
#endif

/* Given *name = "foo", it searches the environment for string *
 * "foo=bar", and returns a pointer to the beginning of "bar"  */

/**/
mod_export char *
zgetenv(char *name)
{
#ifdef HAVE_GETENV
    return getenv(name);
#else
    char **ep, *s, *t;
 
    for (ep = environ; *ep; ep++) {
       for (s = *ep, t = name; *s && *s == *t; s++, t++);
       if (*s == '=' && !*t)
           return s + 1;
    }
    return NULL;
#endif
}

/**/
static void
copyenvstr(char *s, char *value, int flags)
{
    while (*s++) {
	if ((*s = *value++) == Meta)
	    *s = *value++ ^ 32;
	if (flags & PM_LOWER)
	    *s = tulower(*s);
	else if (flags & PM_UPPER)
	    *s = tuupper(*s);
    }
}

/**/
void
addenv(Param pm, char *value)
{
    char *newenv = 0;
#ifndef USE_SET_UNSET_ENV
    char *oldenv = 0, *env = 0;
    int pos;

    /*
     * First check if there is already an environment
     * variable matching string `name'.
     */
    if (findenv(pm->node.nam, &pos))
	oldenv = environ[pos];
#endif

     newenv = mkenvstr(pm->node.nam, value, pm->node.flags);
     if (zputenv(newenv)) {
        zsfree(newenv);
	pm->env = NULL;
	return;
    }
#ifdef USE_SET_UNSET_ENV
     /*
      * If we are using setenv/unsetenv to manage the environment,
      * we simply store the string we created in pm->env since
      * memory management of the environment is handled entirely
      * by the system.
      *
      * TODO: is this good enough to fix problem cases from
      * the other branch?  If so, we don't actually need to
      * store pm->env at all, just a flag that the value was set.
      */
     if (pm->env)
         zsfree(pm->env);
     pm->env = newenv;
     pm->node.flags |= PM_EXPORTED;
#else
    /*
     * Under Cygwin we must use putenv() to maintain consistency.
     * Unfortunately, current version (1.1.2) copies argument and may
     * silently reuse existing environment string. This tries to
     * check for both cases
     */
    if (findenv(pm->node.nam, &pos)) {
	env = environ[pos];
	if (env != oldenv)
	    zsfree(oldenv);
	if (env != newenv)
	    zsfree(newenv);
	pm->node.flags |= PM_EXPORTED;
	pm->env = env;
	return;
    }

    DPUTS(1, "addenv should never reach the end");
    pm->env = NULL;
#endif
}


/* Given strings *name = "foo", *value = "bar", *
 * return a new string *str = "foo=bar".        */

/**/
static char *
mkenvstr(char *name, char *value, int flags)
{
    char *str, *s = value;
    int len_name, len_value = 0;

    len_name = strlen(name);
    if (s)
	while (*s && (*s++ != Meta || *s++ != 32))
	    len_value++;
    s = str = (char *) zalloc(len_name + len_value + 2);
    strcpy(s, name);
    s += len_name;
    *s = '=';
    if (value)
	copyenvstr(s, value, flags);
    else
	*++s = '\0';
    return str;
}

/* Given *name = "foo", *value = "bar", add the    *
 * string "foo=bar" to the environment.  Return a  *
 * pointer to the location of this new environment *
 * string.                                         */


#ifndef USE_SET_UNSET_ENV
/**/
void
delenvvalue(char *x)
{
    char **ep;

    for (ep = environ; *ep; ep++) {
	if (*ep == x)
	    break;
    }
    if (*ep) {
	for (; (ep[0] = ep[1]); ep++);
    }
    zsfree(x);
}
#endif


/* Delete a pointer from the list of pointers to environment *
 * variables by shifting all the other pointers up one slot. */

/**/
void
delenv(Param pm)
{
#ifdef USE_SET_UNSET_ENV
    unsetenv(pm->node.nam);
    zsfree(pm->env);
#else
    delenvvalue(pm->env);
#endif
    pm->env = NULL;
    /*
     * Note we don't remove PM_EXPORT from the flags.  This
     * may be asking for trouble but we need to know later
     * if we restore this parameter to its old value.
     */
}

/*
 * Guts of convbase: this version can return the number of digits
 * sans any base discriminator.
 */

/**/
void
convbase_ptr(char *s, zlong v, int base, int *ndigits)
{
    int digs = 0;
    zulong x;

    if (v < 0)
	*s++ = '-', v = -v;
    if (base >= -1 && base <= 1)
	base = -10;

    if (base > 0) {
	if (isset(CBASES) && base == 16)
	    sprintf(s, "0x");
	else if (isset(CBASES) && base == 8 && isset(OCTALZEROES))
	    sprintf(s, "0");
	else if (base != 10)
	    sprintf(s, "%d#", base);
	else
	    *s = 0;
	s += strlen(s);
    } else
	base = -base;
    for (x = v; x; digs++)
	x /= base;
    if (!digs)
	digs = 1;
    if (ndigits)
	*ndigits = digs;
    s[digs--] = '\0';
    x = v;
    while (digs >= 0) {
	int dig = x % base;

	s[digs--] = (dig < 10) ? '0' + dig : dig - 10 + 'A';
	x /= base;
    }
}

/*
 * Basic conversion of integer to a string given a base.
 * If 0 base is 10.
 * If negative no base discriminator is output.
 */

/**/
mod_export void
convbase(char *s, zlong v, int base)
{
    convbase_ptr(s, v, base, NULL);
}

/*
 * Add underscores to converted integer for readability with given spacing.
 * s is as for convbase: at least BDIGBUFSIZE.
 * If underscores were added, returned value with underscores comes from
 * heap, else the returned value is s.
 */

/**/
char *
convbase_underscore(char *s, zlong v, int base, int underscore)
{
    char *retptr, *sptr, *dptr;
    int ndigits, nunderscore, mod, len;

    convbase_ptr(s, v, base, &ndigits);

    if (underscore <= 0)
	return s;

    nunderscore = (ndigits - 1) / underscore;
    if (!nunderscore)
	return s;
    len = strlen(s);
    retptr = zhalloc(len + nunderscore + 1);
    mod = 0;
    memcpy(retptr, s, len - ndigits);
    sptr = s + len;
    dptr = retptr + len + nunderscore;
    /* copy the null */
    *dptr-- = *sptr--;
    for (;;) {
	*dptr = *sptr;
	if (!--ndigits)
	    break;
	dptr--;
	sptr--;
	if (++mod == underscore) {
	    mod = 0;
	    *dptr-- = '_';
	}
    }

    return retptr;
}

/*
 * Convert a floating point value for output.
 * Unlike convbase(), this has its own internal storage and returns
 * a value from the heap.
 */

/**/
char *
convfloat(double dval, int digits, int flags, FILE *fout)
{
    char fmt[] = "%.*e";
    char *prev_locale, *ret;

    /*
     * The difficulty with the buffer size is that a %f conversion
     * prints all digits before the decimal point: with 64 bit doubles,
     * that's around 310.  We can't check without doing some quite
     * serious floating point operations we'd like to avoid.
     * Then we are liable to get all the digits
     * we asked for after the decimal point, or we should at least
     * bargain for it.  So we just allocate 512 + digits.  This
     * should work until somebody decides on 128-bit doubles.
     */
    if (!(flags & (PM_EFLOAT|PM_FFLOAT))) {
	/*
	 * Conversion from a floating point expression without using
	 * a variable.  The best bet in this case just seems to be
	 * to use the general %g format with something like the maximum
	 * double precision.
	 */
	fmt[3] = 'g';
	if (!digits)
	    digits = 17;
    } else {
	if (flags & PM_FFLOAT)
	    fmt[3] = 'f';
	if (digits <= 0)
	    digits = 10;
	if (flags & PM_EFLOAT) {
	    /*
	     * Here, we are given the number of significant figures, but
	     * %e wants the number of decimal places (unlike %g)
	     */
	    digits--;
	}
    }
#ifdef USE_LOCALE
    prev_locale = dupstring(setlocale(LC_NUMERIC, NULL));
    setlocale(LC_NUMERIC, "POSIX");
#endif
    if (fout) {
	fprintf(fout, fmt, digits, dval);
	ret = NULL;
    } else {
	VARARR(char, buf, 512 + digits);
	if (isinf(dval))
	    ret = dupstring((dval < 0.0) ? "-Inf" : "Inf");
	else if (isnan(dval))
	    ret = dupstring("NaN");
	else {
	    sprintf(buf, fmt, digits, dval);
	    if (!strchr(buf, 'e') && !strchr(buf, '.'))
		strcat(buf, ".");
	    ret = dupstring(buf);
	}
    }
#ifdef USE_LOCALE
    if (prev_locale) setlocale(LC_NUMERIC, prev_locale);
#endif
    return ret;
}

/*
 * convert float to string with basic options but inserting underscores
 * for readability.
 */

/**/
char *convfloat_underscore(double dval, int underscore)
{
    int ndigits_int = 0, ndigits_frac = 0, nunderscore, len;
    char *s, *retptr, *sptr, *dptr;

    s = convfloat(dval, 0, 0, NULL);
    if (underscore <= 0)
	return s;

    /*
     * Count the number of digits before and after the decimal point, if any.
     */
    sptr = s;
    if (*sptr == '-')
	sptr++;
    while (idigit(*sptr)) {
	ndigits_int++;
	sptr++;
    }
    if (*sptr == '.') {
	sptr++;
	while (idigit(*sptr)) {
	    ndigits_frac++;
	    sptr++;
	}
    }

    /*
     * Work out how many underscores to insert --- remember we
     * put them in integer and fractional parts separately.
     */
    nunderscore = (ndigits_int-1) / underscore + (ndigits_frac-1) / underscore;
    if (!nunderscore)
	return s;
    len = strlen(s);
    dptr = retptr = zhalloc(len + nunderscore + 1);

    /*
     * Insert underscores in integer part.
     * Grouping starts from the point in both directions.
     */
    sptr = s;
    if (*sptr == '-')
	*dptr++ = *sptr++;
    while (ndigits_int) {
	*dptr++ = *sptr++;
	if (--ndigits_int && !(ndigits_int % underscore))
	    *dptr++ = '_';
    }
    if (ndigits_frac) {
	/*
	 * Insert underscores in the fractional part.
	 */
	int mod = 0;
	/* decimal point, we already checked */
	*dptr++ = *sptr++;
	while (ndigits_frac) {
	    *dptr++ = *sptr++;
	    mod++;
	    if (--ndigits_frac && mod == underscore) {
		*dptr++ = '_';
		mod = 0;
	    }
	}
    }
    /* Copy exponent and anything else up to null */
    while ((*dptr++ = *sptr++))
	;
    return retptr;
}

/* Start a parameter scope */

/**/
mod_export void
startparamscope(void)
{
    locallevel++;
}

#ifdef USE_LOCALE
/*
 * Flag that one of the special LC_ functions or LANG changed on scope
 * end
 */
static int lc_update_needed;
#endif /* USE_LOCALE */

/* End a parameter scope: delete the parameters local to the scope. */

/**/
mod_export void
endparamscope(void)
{
    queue_signals();
    locallevel--;
    /* This pops anything from a higher locallevel */
    saveandpophiststack(0, HFILE_USE_OPTIONS);
#ifdef USE_LOCALE
    lc_update_needed = 0;
#endif
    scanhashtable(paramtab, 0, 0, 0, scanendscope, 0);
#ifdef USE_LOCALE
    if (lc_update_needed)
    {
	/* Locale changed --- ensure it is restored. */
	char *val;
	if ((val = getsparam_u("LC_ALL")) && *val) {
	    setlocale(LC_ALL, val);
	} else {
	    struct localename *ln;
	    if ((val = getsparam_u("LANG")) && *val)
		setlang(val);
	    for (ln = lc_names; ln->name; ln++) {
		if ((val = getsparam_u(ln->name)) && *val)
		    setlocale(ln->category, val);
	    }
	}
	clear_mbstate();    /* LC_CTYPE may have changed */
    }
#endif /* USE_LOCALE */
    unqueue_signals();
}

/**/
static void
scanendscope(HashNode hn, UNUSED(int flags))
{
    Param pm = (Param)hn;
    if (pm->level > locallevel) {
	if ((pm->node.flags & (PM_SPECIAL|PM_REMOVABLE)) == PM_SPECIAL) {
	    /*
	     * Removable specials are normal in that they can be removed
	     * to reveal an ordinary parameter beneath.  Here we handle
	     * non-removable specials, which were made local by stealth
	     * (see newspecial code in typeset_single()).  In fact the
	     * visible pm is always the same struct; the pm->old is
	     * just a place holder for old data and flags.
	     */
	    Param tpm = pm->old;

#ifdef USE_LOCALE
	    if (!strncmp(pm->node.nam, "LC_", 3) ||
		!strcmp(pm->node.nam, "LANG"))
		lc_update_needed = 1;
#endif
	    if (!strcmp(pm->node.nam, "SECONDS"))
	    {
		setsecondstype(pm, PM_TYPE(tpm->node.flags), PM_TYPE(pm->node.flags));
		/*
		 * We restore SECONDS by restoring its raw internal value
		 * that we cached off into tpm->u.dval.
		 */
		setrawseconds(tpm->u.dval);
		tpm->node.flags |= PM_NORESTORE;
	    }
	    DPUTS(!tpm || PM_TYPE(pm->node.flags) != PM_TYPE(tpm->node.flags) ||
		  !(tpm->node.flags & PM_SPECIAL),
		  "BUG: in restoring scope of special parameter");
	    pm->old = tpm->old;
	    pm->node.flags = (tpm->node.flags & ~PM_NORESTORE);
	    pm->level = tpm->level;
	    pm->base = tpm->base;
	    pm->width = tpm->width;
	    if (pm->env)
		delenv(pm);

	    if (!(tpm->node.flags & (PM_NORESTORE|PM_READONLY)))
		switch (PM_TYPE(pm->node.flags)) {
		case PM_SCALAR:
		    pm->gsu.s->setfn(pm, tpm->u.str);
		    break;
		case PM_INTEGER:
		    pm->gsu.i->setfn(pm, tpm->u.val);
		    break;
		case PM_EFLOAT:
		case PM_FFLOAT:
		    pm->gsu.f->setfn(pm, tpm->u.dval);
		    break;
		case PM_ARRAY:
		    pm->gsu.a->setfn(pm, tpm->u.arr);
		    break;
		case PM_HASHED:
		    pm->gsu.h->setfn(pm, tpm->u.hash);
		    break;
		}
	    zfree(tpm, sizeof(*tpm));

	    if (pm->node.flags & PM_EXPORTED)
		export_param(pm);
	} else
	    unsetparam_pm(pm, 0, 0);
    }
}


/**********************************/
/* Parameter Hash Table Functions */
/**********************************/

/**/
void
freeparamnode(HashNode hn)
{
    Param pm = (Param) hn;
 
    /* The second argument of unsetfn() is used by modules to
     * differentiate "exp"licit unset from implicit unset, as when
     * a parameter is going out of scope.  It's not clear which
     * of these applies here, but passing 1 has always worked.
     */
    if (delunset)
	pm->gsu.s->unsetfn(pm, 1);
    zsfree(pm->node.nam);
    /* If this variable was tied by the user, ename was ztrdup'd */
    if (!(pm->node.flags & PM_SPECIAL))
	zsfree(pm->ename);
    zfree(pm, sizeof(struct param));
}

/* Print a parameter */

enum paramtypes_flags {
    PMTF_USE_BASE	= (1<<0),
    PMTF_USE_WIDTH	= (1<<1),
    PMTF_TEST_LEVEL	= (1<<2)
};

struct paramtypes {
    int binflag;	/* The relevant PM_FLAG(S) */
    const char *string;	/* String for verbose output */
    int typeflag;	/* Flag for typeset -? */
    int flags;		/* The enum above */
};

static const struct paramtypes pmtypes[] = {
    { PM_AUTOLOAD, "undefined", 0, 0},
    { PM_INTEGER, "integer", 'i', PMTF_USE_BASE},
    { PM_EFLOAT, "float", 'E', 0},
    { PM_FFLOAT, "float", 'F', 0},
    { PM_ARRAY, "array", 'a', 0},
    { PM_HASHED, "association", 'A', 0},
    { 0, "local", 0, PMTF_TEST_LEVEL},
    { PM_LEFT, "left justified", 'L', PMTF_USE_WIDTH},
    { PM_RIGHT_B, "right justified", 'R', PMTF_USE_WIDTH},
    { PM_RIGHT_Z, "zero filled", 'Z', PMTF_USE_WIDTH},
    { PM_LOWER, "lowercase", 'l', 0},
    { PM_UPPER, "uppercase", 'u', 0},
    { PM_READONLY, "readonly", 'r', 0},
    { PM_TAGGED, "tagged", 't', 0},
    { PM_EXPORTED, "exported", 'x', 0},
    { PM_UNIQUE, "unique", 'U', 0},
    { PM_TIED, "tied", 'T', 0}
};

#define PMTYPES_SIZE ((int)(sizeof(pmtypes)/sizeof(struct paramtypes)))

static void
printparamvalue(Param p, int printflags)
{
    char *t, **u;

    if (!(printflags & PRINT_KV_PAIR))
	putchar('=');

    /* How the value is displayed depends *
     * on the type of the parameter       */
    switch (PM_TYPE(p->node.flags)) {
    case PM_SCALAR:
	/* string: simple output */
	if (p->gsu.s->getfn && (t = p->gsu.s->getfn(p)))
	    quotedzputs(t, stdout);
	break;
    case PM_INTEGER:
	/* integer */
#ifdef ZSH_64_BIT_TYPE
	fputs(output64(p->gsu.i->getfn(p)), stdout);
#else
	printf("%ld", p->gsu.i->getfn(p));
#endif
	break;
    case PM_EFLOAT:
    case PM_FFLOAT:
	/* float */
	convfloat(p->gsu.f->getfn(p), p->base, p->node.flags, stdout);
	break;
    case PM_ARRAY:
	/* array */
	if (!(printflags & PRINT_KV_PAIR)) {
	    putchar('(');
	    if (!(printflags & PRINT_LINE))
		putchar(' ');
	}
	u = p->gsu.a->getfn(p);
	if(*u) {
	    if (printflags & PRINT_LINE) {
		if (printflags & PRINT_KV_PAIR)
		    printf("  ");
		else
		    printf("\n  ");
	    }
	    quotedzputs(*u++, stdout);
	    while (*u) {
		if (printflags & PRINT_LINE)
		    printf("\n  ");
		else
		    putchar(' ');
		quotedzputs(*u++, stdout);
	    }
	    if ((printflags & (PRINT_LINE|PRINT_KV_PAIR)) == PRINT_LINE)
		putchar('\n');
	}
	if (!(printflags & PRINT_KV_PAIR)) {
	    if (!(printflags & PRINT_LINE))
		putchar(' ');
	    putchar(')');
	}
	break;
    case PM_HASHED:
	/* association */
	{
	    HashTable ht;
	    int found = 0;
	    if (!(printflags & PRINT_KV_PAIR)) {
		putchar('(');
		if (!(printflags & PRINT_LINE))
		    putchar(' ');
	    }
            ht = p->gsu.h->getfn(p);
            if (ht)
		found = scanhashtable(ht, 1, 0, PM_UNSET,
				      ht->printnode, PRINT_KV_PAIR |
				      (printflags & PRINT_LINE));
	    if (!(printflags & PRINT_KV_PAIR)) {
		if (found && (printflags & PRINT_LINE))
		    putchar('\n');
		putchar(')');
	    }
	}
	break;
    }
}

/**/
mod_export void
printparamnode(HashNode hn, int printflags)
{
    Param p = (Param) hn;
    Param peer = NULL;

    if (p->node.flags & PM_UNSET) {
	if ((printflags & (PRINT_POSIX_READONLY|PRINT_POSIX_EXPORT) &&
	     p->node.flags & (PM_READONLY|PM_EXPORTED)) ||
	    (p->node.flags & PM_DEFAULTED) == PM_DEFAULTED) {
	    /*
	     * Special POSIX rules: show the parameter as readonly/exported
	     * even though it's unset, but with no value.
	     */
	    printflags |= PRINT_NAMEONLY;
	}
	else
	    return;
    }
    if (p->node.flags & PM_AUTOLOAD)
	printflags |= PRINT_NAMEONLY;

    if (printflags & (PRINT_TYPESET|PRINT_POSIX_READONLY|PRINT_POSIX_EXPORT)) {
	if (p->node.flags & (PM_RO_BY_DESIGN|PM_AUTOLOAD)) {
	    /*
	     * It's not possible to restore the state of
	     * these, so don't output.
	     */
	    return;
	}
	/*
	 * The zsh variants of export -p/readonly -p also report other
	 * flags to indicate other attributes or scope. The POSIX variants
	 * don't.
	 */
	if (printflags & PRINT_POSIX_EXPORT) {
	    if (!(p->node.flags & PM_EXPORTED))
		return;
	    printf("export ");
	} else if (printflags & PRINT_POSIX_READONLY) {
	    if (!(p->node.flags & PM_READONLY))
		return;
	    printf("readonly ");
	} else if (locallevel && p->level >= locallevel) {
	    printf("typeset ");	    /* printf("local "); */
	} else if ((p->node.flags & PM_EXPORTED) &&
		   !(p->node.flags & (PM_ARRAY|PM_HASHED))) {
	    printf("export ");
	} else if (locallevel) {
	    printf("typeset -g ");
	} else
	    printf("typeset ");
    }

    /* Print the attributes of the parameter */
    if (printflags & (PRINT_TYPE|PRINT_TYPESET)) {
	int doneminus = 0, i;
	const struct paramtypes *pmptr;

	for (pmptr = pmtypes, i = 0; i < PMTYPES_SIZE; i++, pmptr++) {
	    int doprint = 0;
	    if (pmptr->flags & PMTF_TEST_LEVEL) {
		if (p->level)
		    doprint = 1;
	    } else if ((pmptr->binflag != PM_EXPORTED || p->level ||
			(p->node.flags & (PM_LOCAL|PM_ARRAY|PM_HASHED))) &&
		       (p->node.flags & pmptr->binflag))
		doprint = 1;

	    if (doprint) {
		if (printflags & PRINT_TYPESET) {
		    if (pmptr->typeflag) {
			if (!doneminus) {
			    putchar('-');
			    doneminus = 1;
			}
			putchar(pmptr->typeflag);
		    }
		} else
		    printf("%s ", pmptr->string);
		if ((pmptr->flags & PMTF_USE_BASE) && p->base) {
		    printf("%d ", p->base);
		    doneminus = 0;
		}
		if ((pmptr->flags & PMTF_USE_WIDTH) && p->width) {
		    printf("%u ", p->width);
		    doneminus = 0;
		}
	    }
	}
	if (doneminus)
	    putchar(' ');

	if (p->node.flags & PM_TIED) {
	    /*
	     * For scalars tied to arrays,s
	     *   * typeset +m outputs
	     *      array tied SCALAR array
	     *      tied array SCALAR
	     *   * typeset -p outputs:
	     *      typeset -T SCALAR array  (for hidden values)
	     *      typeset -T SCALAR array=(values)
	     *      for both scalar and array (flags may be different)
	     *
	     * We choose to print the value for the array instead of the scalar
	     * as scalars can't disambiguate between
	     * typeset -T SCALAR array=()
	     * and
	     * typeset -T SCALAR array=('')
	     * (same for (a b:c)...)
	     */
	    Param tmp = (Param) paramtab->getnode(paramtab, p->ename);

	    /*
	     * Swap param and tied peer for typeset -p output
	     */
	    if (!(printflags & PRINT_TYPESET) || (p->node.flags & PM_ARRAY))
		peer = tmp;
	    else {
		peer = p;
		p = tmp;
	    }

	    quotedzputs(peer->node.nam, stdout);
	    putchar(' ');
	}
    }

    if ((printflags & PRINT_NAMEONLY) ||
	((p->node.flags & PM_HIDEVAL) && !(printflags & PRINT_INCLUDEVALUE)))
	quotedzputs(p->node.nam, stdout);
    else {
	if (printflags & PRINT_KV_PAIR) {
	    if (printflags & PRINT_LINE)
		printf("\n  ");
	    putchar('[');
	}
	quotedzputs(p->node.nam, stdout);
	if (printflags & PRINT_KV_PAIR)
	    printf("]=");

	printparamvalue(p, printflags);
    }
    if (peer && (printflags & PRINT_TYPESET) && !(p->node.flags & PM_SPECIAL)) {
	/*
	 * append the join char for tied parameters if different from colon
	 * for typeset -p output.
	 */
	unsigned char joinchar = STOUC(((struct tieddata *)peer->u.data)->joinchar);
	if (joinchar != ':') {
	    char buf[2];
	    buf[0] = joinchar;
	    buf[1] = '\0';
	    putchar(' ');
	    quotedzputs(buf, stdout);
	}
    }
    if ((printflags & (PRINT_KV_PAIR|PRINT_LINE)) == PRINT_KV_PAIR)
	putchar(' ');
    else if (!(printflags & PRINT_KV_PAIR))
	putchar('\n');
}

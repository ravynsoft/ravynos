/*
 * fontconfig/src/fcxml.c
 *
 * Copyright © 2002 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "fcint.h"
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef ENABLE_LIBXML2

#include <libxml/parser.h>

#define XML_Char			xmlChar
#define XML_Parser			xmlParserCtxtPtr
#define XML_ParserFree			xmlFreeParserCtxt
#define XML_GetCurrentLineNumber	xmlSAX2GetLineNumber
#define XML_GetErrorCode		xmlCtxtGetLastError
#define XML_ErrorString(Error)		(Error)->message

#else /* ENABLE_LIBXML2 */

#ifndef HAVE_XMLPARSE_H
#define HAVE_XMLPARSE_H 0
#endif

#if HAVE_XMLPARSE_H
#include <xmlparse.h>
#else
#include <expat.h>
#endif

#endif /* ENABLE_LIBXML2 */

#ifdef _WIN32
#include <mbstring.h>
extern FcChar8 fontconfig_instprefix[];
pfnGetSystemWindowsDirectory pGetSystemWindowsDirectory = NULL;
pfnSHGetFolderPathA pSHGetFolderPathA = NULL;
static void
_ensureWin32GettersReady();
#endif

static FcChar8  *__fc_userdir = NULL;
static FcChar8  *__fc_userconf = NULL;

static void
FcExprDestroy (FcExpr *e);
static FcBool
_FcConfigParse (FcConfig	*config,
		const FcChar8	*name,
		FcBool		complain,
		FcBool		load);

void
FcTestDestroy (FcTest *test)
{
    FcExprDestroy (test->expr);
    free (test);
}

void
FcRuleDestroy (FcRule *rule)
{
    FcRule *n = rule->next;

    switch (rule->type) {
    case FcRuleTest:
	FcTestDestroy (rule->u.test);
	break;
    case FcRuleEdit:
	FcEditDestroy (rule->u.edit);
	break;
    case FcRuleUnknown:
    default:
	break;
    }
    free (rule);
    if (n)
	FcRuleDestroy (n);
}

static FcExpr *
FcExprCreateInteger (FcConfig *config, int i)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpInteger;
	e->u.ival = i;
    }
    return e;
}

static FcExpr *
FcExprCreateDouble (FcConfig *config, double d)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpDouble;
	e->u.dval = d;
    }
    return e;
}

static FcExpr *
FcExprCreateString (FcConfig *config, const FcChar8 *s)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpString;
	e->u.sval = FcStrdup (s);
    }
    return e;
}

static FcExprMatrix *
FcExprMatrixCopyShallow (const FcExprMatrix *matrix)
{
  FcExprMatrix *m = malloc (sizeof (FcExprMatrix));
  if (m)
  {
    *m = *matrix;
  }
  return m;
}

static void
FcExprMatrixFreeShallow (FcExprMatrix *m)
{
  if (!m)
    return;

  free (m);
}

static void
FcExprMatrixFree (FcExprMatrix *m)
{
  if (!m)
    return;

  FcExprDestroy (m->xx);
  FcExprDestroy (m->xy);
  FcExprDestroy (m->yx);
  FcExprDestroy (m->yy);

  free (m);
}

static FcExpr *
FcExprCreateMatrix (FcConfig *config, const FcExprMatrix *matrix)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpMatrix;
	e->u.mexpr = FcExprMatrixCopyShallow (matrix);
    }
    return e;
}

static FcExpr *
FcExprCreateRange (FcConfig *config, FcRange *range)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpRange;
	e->u.rval = FcRangeCopy (range);
    }
    return e;
}

static FcExpr *
FcExprCreateBool (FcConfig *config, FcBool b)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpBool;
	e->u.bval = b;
    }
    return e;
}

static FcExpr *
FcExprCreateCharSet (FcConfig *config, FcCharSet *charset)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpCharSet;
	e->u.cval = FcCharSetCopy (charset);
    }
    return e;
}

static FcExpr *
FcExprCreateLangSet (FcConfig *config, FcLangSet *langset)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpLangSet;
	e->u.lval = FcLangSetCopy (langset);
    }
    return e;
}

static FcExpr *
FcExprCreateName (FcConfig *config, FcExprName name)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpField;
	e->u.name = name;
    }
    return e;
}

static FcExpr *
FcExprCreateConst (FcConfig *config, const FcChar8 *constant)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpConst;
	e->u.constant = FcStrdup (constant);
    }
    return e;
}

static FcExpr *
FcExprCreateOp (FcConfig *config, FcExpr *left, FcOp op, FcExpr *right)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = op;
	e->u.tree.left = left;
	e->u.tree.right = right;
    }
    return e;
}

static void
FcExprDestroy (FcExpr *e)
{
    if (!e)
	return;
    switch (FC_OP_GET_OP (e->op)) {
    case FcOpInteger:
	break;
    case FcOpDouble:
	break;
    case FcOpString:
	FcFree (e->u.sval);
	break;
    case FcOpMatrix:
	FcExprMatrixFree (e->u.mexpr);
	break;
    case FcOpRange:
	FcRangeDestroy (e->u.rval);
	break;
    case FcOpCharSet:
	FcCharSetDestroy (e->u.cval);
	break;
    case FcOpLangSet:
	FcLangSetDestroy (e->u.lval);
	break;
    case FcOpBool:
	break;
    case FcOpField:
	break;
    case FcOpConst:
	FcFree (e->u.constant);
	break;
    case FcOpAssign:
    case FcOpAssignReplace:
    case FcOpPrepend:
    case FcOpPrependFirst:
    case FcOpAppend:
    case FcOpAppendLast:
    case FcOpDelete:
    case FcOpDeleteAll:
	break;
    case FcOpOr:
    case FcOpAnd:
    case FcOpEqual:
    case FcOpNotEqual:
    case FcOpLess:
    case FcOpLessEqual:
    case FcOpMore:
    case FcOpMoreEqual:
    case FcOpContains:
    case FcOpListing:
    case FcOpNotContains:
    case FcOpPlus:
    case FcOpMinus:
    case FcOpTimes:
    case FcOpDivide:
    case FcOpQuest:
    case FcOpComma:
	FcExprDestroy (e->u.tree.right);
	/* fall through */
    case FcOpNot:
    case FcOpFloor:
    case FcOpCeil:
    case FcOpRound:
    case FcOpTrunc:
	FcExprDestroy (e->u.tree.left);
	break;
    case FcOpNil:
    case FcOpInvalid:
	break;
    }

    e->op = FcOpNil;
}

void
FcEditDestroy (FcEdit *e)
{
    if (e->expr)
	FcExprDestroy (e->expr);
    free (e);
}

typedef enum _FcElement {
    FcElementNone,
    FcElementFontconfig,
    FcElementDir,
    FcElementCacheDir,
    FcElementCache,
    FcElementInclude,
    FcElementConfig,
    FcElementMatch,
    FcElementAlias,
    FcElementDescription,
    FcElementRemapDir,
    FcElementResetDirs,

    FcElementRescan,

    FcElementPrefer,
    FcElementAccept,
    FcElementDefault,
    FcElementFamily,

    FcElementSelectfont,
    FcElementAcceptfont,
    FcElementRejectfont,
    FcElementGlob,
    FcElementPattern,
    FcElementPatelt,

    FcElementTest,
    FcElementEdit,
    FcElementInt,
    FcElementDouble,
    FcElementString,
    FcElementMatrix,
    FcElementRange,
    FcElementBool,
    FcElementCharSet,
    FcElementLangSet,
    FcElementName,
    FcElementConst,
    FcElementOr,
    FcElementAnd,
    FcElementEq,
    FcElementNotEq,
    FcElementLess,
    FcElementLessEq,
    FcElementMore,
    FcElementMoreEq,
    FcElementContains,
    FcElementNotContains,
    FcElementPlus,
    FcElementMinus,
    FcElementTimes,
    FcElementDivide,
    FcElementNot,
    FcElementIf,
    FcElementFloor,
    FcElementCeil,
    FcElementRound,
    FcElementTrunc,
    FcElementUnknown
} FcElement;

static const struct {
    const char  name[16];
    FcElement   element;
} fcElementMap[] = {
    { "fontconfig",	FcElementFontconfig },
    { "dir",		FcElementDir },
    { "cachedir",	FcElementCacheDir },
    { "cache",		FcElementCache },
    { "include",	FcElementInclude },
    { "config",		FcElementConfig },
    { "match",		FcElementMatch },
    { "alias",		FcElementAlias },
    { "description",	FcElementDescription },
    { "remap-dir",	FcElementRemapDir },
    { "reset-dirs",	FcElementResetDirs },

    { "rescan",		FcElementRescan },

    { "prefer",		FcElementPrefer },
    { "accept",		FcElementAccept },
    { "default",	FcElementDefault },
    { "family",		FcElementFamily },

    { "selectfont",	FcElementSelectfont },
    { "acceptfont",	FcElementAcceptfont },
    { "rejectfont",	FcElementRejectfont },
    { "glob",		FcElementGlob },
    { "pattern",	FcElementPattern },
    { "patelt",		FcElementPatelt },

    { "test",		FcElementTest },
    { "edit",		FcElementEdit },
    { "int",		FcElementInt },
    { "double",		FcElementDouble },
    { "string",		FcElementString },
    { "matrix",		FcElementMatrix },
    { "range",		FcElementRange },
    { "bool",		FcElementBool },
    { "charset",	FcElementCharSet },
    { "langset",	FcElementLangSet },
    { "name",		FcElementName },
    { "const",		FcElementConst },
    { "or",		FcElementOr },
    { "and",		FcElementAnd },
    { "eq",		FcElementEq },
    { "not_eq",		FcElementNotEq },
    { "less",		FcElementLess },
    { "less_eq",	FcElementLessEq },
    { "more",		FcElementMore },
    { "more_eq",	FcElementMoreEq },
    { "contains",	FcElementContains },
    { "not_contains",	FcElementNotContains },
    { "plus",		FcElementPlus },
    { "minus",		FcElementMinus },
    { "times",		FcElementTimes },
    { "divide",		FcElementDivide },
    { "not",		FcElementNot },
    { "if",		FcElementIf },
    { "floor",		FcElementFloor },
    { "ceil",		FcElementCeil },
    { "round",		FcElementRound },
    { "trunc",		FcElementTrunc },
};
#define NUM_ELEMENT_MAPS (int) (sizeof fcElementMap / sizeof fcElementMap[0])

static const char *fcElementIgnoreName[16] = {
    "its:",
    NULL
};

static FcElement
FcElementMap (const XML_Char *name)
{

    int	    i;
    for (i = 0; i < NUM_ELEMENT_MAPS; i++)
	if (!strcmp ((char *) name, fcElementMap[i].name))
	    return fcElementMap[i].element;
    for (i = 0; fcElementIgnoreName[i] != NULL; i++)
	if (!strncmp ((char *) name, fcElementIgnoreName[i], strlen (fcElementIgnoreName[i])))
	    return FcElementNone;
    return FcElementUnknown;
}

static const char *
FcElementReverseMap (FcElement e)
{
    int i;

    for (i = 0; i < NUM_ELEMENT_MAPS; i++)
	if (fcElementMap[i].element == e)
	    return fcElementMap[i].name;

    return NULL;
}


typedef struct _FcPStack {
    struct _FcPStack   *prev;
    FcElement		element;
    FcChar8		**attr;
    FcStrBuf		str;
    FcChar8            *attr_buf_static[16];
} FcPStack;

typedef enum _FcVStackTag {
    FcVStackNone,

    FcVStackString,
    FcVStackFamily,
    FcVStackConstant,
    FcVStackGlob,
    FcVStackName,
    FcVStackPattern,

    FcVStackPrefer,
    FcVStackAccept,
    FcVStackDefault,

    FcVStackInteger,
    FcVStackDouble,
    FcVStackMatrix,
    FcVStackRange,
    FcVStackBool,
    FcVStackCharSet,
    FcVStackLangSet,

    FcVStackTest,
    FcVStackExpr,
    FcVStackEdit
} FcVStackTag;

typedef struct _FcVStack {
    struct _FcVStack	*prev;
    FcPStack		*pstack;	/* related parse element */
    FcVStackTag		tag;
    union {
	FcChar8		*string;

	int		integer;
	double		_double;
	FcExprMatrix	*matrix;
	FcRange		*range;
	FcBool		bool_;
	FcCharSet	*charset;
	FcLangSet	*langset;
	FcExprName	name;

	FcTest		*test;
	FcQual		qual;
	FcOp		op;
	FcExpr		*expr;
	FcEdit		*edit;

	FcPattern	*pattern;
    } u;
} FcVStack;

typedef struct _FcConfigParse {
    FcPStack	    *pstack;
    FcVStack	    *vstack;
    FcBool	    error;
    const FcChar8   *name;
    FcConfig	    *config;
    FcRuleSet	    *ruleset;
    XML_Parser	    parser;
    unsigned int    pstack_static_used;
    FcPStack        pstack_static[8];
    unsigned int    vstack_static_used;
    FcVStack        vstack_static[64];
    FcBool          scanOnly;
} FcConfigParse;

typedef enum _FcConfigSeverity {
    FcSevereInfo, FcSevereWarning, FcSevereError
} FcConfigSeverity;

static void
FcConfigMessage (FcConfigParse *parse, FcConfigSeverity severe, const char *fmt, ...)
{
    const char	*s = "unknown";
    va_list	args;

    va_start (args, fmt);

    switch (severe) {
    case FcSevereInfo: s = "info"; break;
    case FcSevereWarning: s = "warning"; break;
    case FcSevereError: s = "error"; break;
    }
    if (parse)
    {
	if (parse->name)
	    fprintf (stderr, "Fontconfig %s: \"%s\", line %d: ", s,
		     parse->name, (int)XML_GetCurrentLineNumber (parse->parser));
	else
	    fprintf (stderr, "Fontconfig %s: line %d: ", s,
		     (int)XML_GetCurrentLineNumber (parse->parser));
	if (severe >= FcSevereError)
	    parse->error = FcTrue;
    }
    else
	fprintf (stderr, "Fontconfig %s: ", s);
    vfprintf (stderr, fmt, args);
    fprintf (stderr, "\n");
    va_end (args);
}


static FcExpr *
FcPopExpr (FcConfigParse *parse);


static const char *
FcTypeName (FcType type)
{
    switch (type) {
    case FcTypeVoid:
	return "void";
    case FcTypeInteger:
    case FcTypeDouble:
	return "number";
    case FcTypeString:
	return "string";
    case FcTypeBool:
	return "bool";
    case FcTypeMatrix:
	return "matrix";
    case FcTypeCharSet:
	return "charset";
    case FcTypeFTFace:
	return "FT_Face";
    case FcTypeLangSet:
	return "langset";
    case FcTypeRange:
	return "range";
    case FcTypeUnknown:
    default:
	return "unknown";
    }
}

static void
FcTypecheckValue (FcConfigParse *parse, FcType value, FcType type)
{
    if (value == FcTypeInteger)
	value = FcTypeDouble;
    if (type == FcTypeInteger)
	type = FcTypeDouble;
    if (value != type)
    {
	if ((value == FcTypeLangSet && type == FcTypeString) ||
	    (value == FcTypeString && type == FcTypeLangSet) ||
	    (value == FcTypeDouble && type == FcTypeRange))
	    return;
	if (type ==  FcTypeUnknown)
	    return;
	/* It's perfectly fine to use user-define elements in expressions,
	 * so don't warn in that case. */
	if (value == FcTypeUnknown)
	    return;
	FcConfigMessage (parse, FcSevereWarning, "saw %s, expected %s",
			 FcTypeName (value), FcTypeName (type));
    }
}

static void
FcTypecheckExpr (FcConfigParse *parse, FcExpr *expr, FcType type)
{
    const FcObjectType	*o;
    const FcConstant	*c;

    /* If parsing the expression failed, some nodes may be NULL */
    if (!expr)
	return;

    switch (FC_OP_GET_OP (expr->op)) {
    case FcOpInteger:
    case FcOpDouble:
	FcTypecheckValue (parse, FcTypeDouble, type);
	break;
    case FcOpString:
	FcTypecheckValue (parse, FcTypeString, type);
	break;
    case FcOpMatrix:
	FcTypecheckValue (parse, FcTypeMatrix, type);
	break;
    case FcOpBool:
	FcTypecheckValue (parse, FcTypeBool, type);
	break;
    case FcOpCharSet:
	FcTypecheckValue (parse, FcTypeCharSet, type);
	break;
    case FcOpLangSet:
	FcTypecheckValue (parse, FcTypeLangSet, type);
	break;
    case FcOpRange:
	FcTypecheckValue (parse, FcTypeRange, type);
	break;
    case FcOpNil:
	break;
    case FcOpField:
	o = FcNameGetObjectType (FcObjectName (expr->u.name.object));
	if (o)
	    FcTypecheckValue (parse, o->type, type);
	break;
    case FcOpConst:
	c = FcNameGetConstant (expr->u.constant);
	if (c)
	{
	    o = FcNameGetObjectType (c->object);
	    if (o)
		FcTypecheckValue (parse, o->type, type);
	}
	else
            FcConfigMessage (parse, FcSevereWarning,
                             "invalid constant used : %s",
                             expr->u.constant);
	break;
    case FcOpQuest:
	FcTypecheckExpr (parse, expr->u.tree.left, FcTypeBool);
	FcTypecheckExpr (parse, expr->u.tree.right->u.tree.left, type);
	FcTypecheckExpr (parse, expr->u.tree.right->u.tree.right, type);
	break;
    case FcOpAssign:
    case FcOpAssignReplace:
	break;
    case FcOpEqual:
    case FcOpNotEqual:
    case FcOpLess:
    case FcOpLessEqual:
    case FcOpMore:
    case FcOpMoreEqual:
    case FcOpContains:
    case FcOpNotContains:
    case FcOpListing:
	FcTypecheckValue (parse, FcTypeBool, type);
	break;
    case FcOpComma:
    case FcOpOr:
    case FcOpAnd:
    case FcOpPlus:
    case FcOpMinus:
    case FcOpTimes:
    case FcOpDivide:
	FcTypecheckExpr (parse, expr->u.tree.left, type);
	FcTypecheckExpr (parse, expr->u.tree.right, type);
	break;
    case FcOpNot:
	FcTypecheckValue (parse, FcTypeBool, type);
	FcTypecheckExpr (parse, expr->u.tree.left, FcTypeBool);
	break;
    case FcOpFloor:
    case FcOpCeil:
    case FcOpRound:
    case FcOpTrunc:
	FcTypecheckValue (parse, FcTypeDouble, type);
	FcTypecheckExpr (parse, expr->u.tree.left, FcTypeDouble);
	break;
    default:
	break;
    }
}

static FcTest *
FcTestCreate (FcConfigParse *parse,
	      FcMatchKind   kind,
	      FcQual	    qual,
	      const FcChar8 *field,
	      unsigned int  compare,
	      FcExpr	    *expr)
{
    FcTest	*test = (FcTest *) malloc (sizeof (FcTest));

    if (test)
    {
	const FcObjectType	*o;

	test->kind = kind;
	test->qual = qual;
	test->object = FcObjectFromName ((const char *) field);
	test->op = compare;
	test->expr = expr;
	o = FcNameGetObjectType (FcObjectName (test->object));
	if (o)
	    FcTypecheckExpr (parse, expr, o->type);
    }
    return test;
}

static FcEdit *
FcEditCreate (FcConfigParse	*parse,
	      FcObject		object,
	      FcOp		op,
	      FcExpr		*expr,
	      FcValueBinding	binding)
{
    FcEdit *e = (FcEdit *) malloc (sizeof (FcEdit));

    if (e)
    {
	const FcObjectType	*o;

	e->object = object;
	e->op = op;
	e->expr = expr;
	e->binding = binding;
	o = FcNameGetObjectType (FcObjectName (e->object));
	if (o)
	    FcTypecheckExpr (parse, expr, o->type);
    }
    return e;
}

static FcRule *
FcRuleCreate (FcRuleType type,
	      void       *p)
{
    FcRule *r = (FcRule *) malloc (sizeof (FcRule));

    if (!r)
	return NULL;

    r->next = NULL;
    r->type = type;
    switch (type)
    {
    case FcRuleTest:
	r->u.test = (FcTest *) p;
	break;
    case FcRuleEdit:
	r->u.edit = (FcEdit *) p;
	break;
    case FcRuleUnknown:
    default:
	free (r);
	r = NULL;
	break;
    }

    return r;
}

static FcVStack *
FcVStackCreateAndPush (FcConfigParse *parse)
{
    FcVStack    *new;

    if (parse->vstack_static_used < sizeof (parse->vstack_static) / sizeof (parse->vstack_static[0]))
	new = &parse->vstack_static[parse->vstack_static_used++];
    else
    {
	new = malloc (sizeof (FcVStack));
	if (!new)
	    return 0;
    }
    new->tag = FcVStackNone;
    new->prev = 0;

    new->prev = parse->vstack;
    new->pstack = parse->pstack ? parse->pstack->prev : 0;
    parse->vstack = new;

    return new;
}

static FcBool
FcVStackPushString (FcConfigParse *parse, FcVStackTag tag, FcChar8 *string)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.string = string;
    vstack->tag = tag;
    return FcTrue;
}

static FcBool
FcVStackPushInteger (FcConfigParse *parse, int integer)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.integer = integer;
    vstack->tag = FcVStackInteger;
    return FcTrue;
}

static FcBool
FcVStackPushDouble (FcConfigParse *parse, double _double)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u._double = _double;
    vstack->tag = FcVStackDouble;
    return FcTrue;
}

static FcBool
FcVStackPushMatrix (FcConfigParse *parse, FcExprMatrix *matrix)
{
    FcVStack    *vstack;
    vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.matrix = FcExprMatrixCopyShallow (matrix);
    vstack->tag = FcVStackMatrix;
    return FcTrue;
}

static FcBool
FcVStackPushRange (FcConfigParse *parse, FcRange *range)
{
    FcVStack 	*vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.range = range;
    vstack->tag = FcVStackRange;
    return FcTrue;
}

static FcBool
FcVStackPushBool (FcConfigParse *parse, FcBool bool_)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.bool_ = bool_;
    vstack->tag = FcVStackBool;
    return FcTrue;
}

static FcBool
FcVStackPushCharSet (FcConfigParse *parse, FcCharSet *charset)
{
    FcVStack	*vstack;
    if (!charset)
	return FcFalse;
    vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.charset = charset;
    vstack->tag = FcVStackCharSet;
    return FcTrue;
}

static FcBool
FcVStackPushLangSet (FcConfigParse *parse, FcLangSet *langset)
{
    FcVStack	*vstack;
    if (!langset)
	return FcFalse;
    vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.langset = langset;
    vstack->tag = FcVStackLangSet;
    return FcTrue;
}

static FcBool
FcVStackPushName (FcConfigParse *parse, FcMatchKind kind, FcObject object)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.name.object = object;
    vstack->u.name.kind = kind;
    vstack->tag = FcVStackName;
    return FcTrue;
}

static FcBool
FcVStackPushTest (FcConfigParse *parse, FcTest *test)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.test = test;
    vstack->tag = FcVStackTest;
    return FcTrue;
}

static FcBool
FcVStackPushExpr (FcConfigParse *parse, FcVStackTag tag, FcExpr *expr)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.expr = expr;
    vstack->tag = tag;
    return FcTrue;
}

static FcBool
FcVStackPushEdit (FcConfigParse *parse, FcEdit *edit)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.edit = edit;
    vstack->tag = FcVStackEdit;
    return FcTrue;
}

static FcBool
FcVStackPushPattern (FcConfigParse *parse, FcPattern *pattern)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.pattern = pattern;
    vstack->tag = FcVStackPattern;
    return FcTrue;
}

static FcVStack *
FcVStackFetch (FcConfigParse *parse, int off)
{
    FcVStack    *vstack;

    for (vstack = parse->vstack; vstack && off-- > 0; vstack = vstack->prev);
    return vstack;
}

static FcVStack *
FcVStackPeek (FcConfigParse *parse)
{
    FcVStack	*vstack = parse->vstack;

    return vstack && vstack->pstack == parse->pstack ? vstack : 0;
}

static void
FcVStackPopAndDestroy (FcConfigParse *parse)
{
    FcVStack	*vstack = parse->vstack;

    if (!vstack || vstack->pstack != parse->pstack)
	return;

    parse->vstack = vstack->prev;

    switch (vstack->tag) {
    case FcVStackNone:
	break;
    case FcVStackName:
	break;
    case FcVStackFamily:
	break;
    case FcVStackString:
    case FcVStackConstant:
    case FcVStackGlob:
	FcStrFree (vstack->u.string);
	break;
    case FcVStackPattern:
	FcPatternDestroy (vstack->u.pattern);
	break;
    case FcVStackInteger:
    case FcVStackDouble:
	break;
    case FcVStackMatrix:
	FcExprMatrixFreeShallow (vstack->u.matrix);
	break;
    case FcVStackBool:
	break;
    case FcVStackRange:
	FcRangeDestroy (vstack->u.range);
	break;
    case FcVStackCharSet:
	FcCharSetDestroy (vstack->u.charset);
	break;
    case FcVStackLangSet:
	FcLangSetDestroy (vstack->u.langset);
	break;
    case FcVStackTest:
	FcTestDestroy (vstack->u.test);
	break;
    case FcVStackExpr:
    case FcVStackPrefer:
    case FcVStackAccept:
    case FcVStackDefault:
	FcExprDestroy (vstack->u.expr);
	break;
    case FcVStackEdit:
	FcEditDestroy (vstack->u.edit);
	break;
    }

    if (vstack == &parse->vstack_static[parse->vstack_static_used - 1])
	parse->vstack_static_used--;
    else
	free (vstack);
}

static void
FcVStackClear (FcConfigParse *parse)
{
    while (FcVStackPeek (parse))
	FcVStackPopAndDestroy (parse);
}

static int
FcVStackElements (FcConfigParse *parse)
{
    int		h = 0;
    FcVStack	*vstack = parse->vstack;
    while (vstack && vstack->pstack == parse->pstack)
    {
	h++;
	vstack = vstack->prev;
    }
    return h;
}

static FcChar8 **
FcConfigSaveAttr (const XML_Char **attr, FcChar8 **buf, int size_bytes)
{
    int		slen;
    int		i;
    FcChar8	**new;
    FcChar8	*s;

    if (!attr)
	return 0;
    slen = 0;
    for (i = 0; attr[i]; i++)
	slen += strlen ((char *) attr[i]) + 1;
    if (i == 0)
	return 0;
    slen += (i + 1) * sizeof (FcChar8 *);
    if (slen <= size_bytes)
	new = buf;
    else
    {
	new = malloc (slen);
	if (!new)
	{
	    FcConfigMessage (0, FcSevereError, "out of memory");
	    return 0;
	}
    }
    s = (FcChar8 *) (new + (i + 1));
    for (i = 0; attr[i]; i++)
    {
	new[i] = s;
	strcpy ((char *) s, (char *) attr[i]);
	s += strlen ((char *) s) + 1;
    }
    new[i] = 0;
    return new;
}

static FcBool
FcPStackPush (FcConfigParse *parse, FcElement element, const XML_Char **attr)
{
    FcPStack   *new;

    if (parse->pstack_static_used < sizeof (parse->pstack_static) / sizeof (parse->pstack_static[0]))
	new = &parse->pstack_static[parse->pstack_static_used++];
    else
    {
	new = malloc (sizeof (FcPStack));
	if (!new)
	    return FcFalse;
    }

    new->prev = parse->pstack;
    new->element = element;
    new->attr = FcConfigSaveAttr (attr, new->attr_buf_static, sizeof (new->attr_buf_static));
    FcStrBufInit (&new->str, 0, 0);
    parse->pstack = new;
    return FcTrue;
}

static FcBool
FcPStackPop (FcConfigParse *parse)
{
    FcPStack   *old;

    if (!parse->pstack)
    {
	FcConfigMessage (parse, FcSevereError, "mismatching element");
	return FcFalse;
    }

    /* Don't check the attributes for FcElementNone */
    if (parse->pstack->element != FcElementNone &&
	parse->pstack->attr)
    {
	/* Warn about unused attrs. */
	FcChar8 **attrs = parse->pstack->attr;
	while (*attrs)
	{
	    if (attrs[0][0])
	    {
		FcConfigMessage (parse, FcSevereWarning, "invalid attribute '%s'", attrs[0]);
	    }
	    attrs += 2;
	}
    }

    FcVStackClear (parse);
    old = parse->pstack;
    parse->pstack = old->prev;
    FcStrBufDestroy (&old->str);

    if (old->attr && old->attr != old->attr_buf_static)
	free (old->attr);

    if (old == &parse->pstack_static[parse->pstack_static_used - 1])
	parse->pstack_static_used--;
    else
	free (old);
    return FcTrue;
}

static FcBool
FcConfigParseInit (FcConfigParse	*parse,
		   const FcChar8	*name,
		   FcConfig		*config,
		   XML_Parser		parser,
		   FcBool		enabled)
{
    parse->pstack = 0;
    parse->pstack_static_used = 0;
    parse->vstack = 0;
    parse->vstack_static_used = 0;
    parse->error = FcFalse;
    parse->name = name;
    parse->config = config;
    parse->ruleset = FcRuleSetCreate (name);
    parse->parser = parser;
    parse->scanOnly = !enabled;
    FcRuleSetEnable (parse->ruleset, enabled);

    return FcTrue;
}

static void
FcConfigCleanup (FcConfigParse	*parse)
{
    while (parse->pstack)
	FcPStackPop (parse);
    FcRuleSetDestroy (parse->ruleset);
    parse->ruleset = NULL;
}

static const FcChar8 *
FcConfigGetAttribute (FcConfigParse *parse, const char *attr)
{
    FcChar8 **attrs;
    if (!parse->pstack)
	return 0;

    attrs = parse->pstack->attr;
    if (!attrs)
        return 0;

    while (*attrs)
    {
	if (!strcmp ((char *) *attrs, attr))
	{
	    attrs[0][0] = '\0'; /* Mark as used. */
	    return attrs[1];
	}
	attrs += 2;
    }
    return 0;
}

static FcStrSet *
_get_real_paths_from_prefix(FcConfigParse *parse, const FcChar8 *path, const FcChar8 *prefix)
{
#ifdef _WIN32
    FcChar8 buffer[1000] = { 0 };
#endif
    FcChar8 *parent = NULL, *retval = NULL;
    FcStrSet *e = NULL;

    if (prefix)
    {
	if (FcStrCmp (prefix, (const FcChar8 *) "xdg") == 0)
	{
	    parent = FcConfigXdgDataHome ();
	    if (!parent)
	    {
		/* Home directory might be disabled */
		return NULL;
	    }
	    e = FcConfigXdgDataDirs ();
	    if (!e)
	    {
		FcStrFree (parent);
		return NULL;
	    }
	}
	else if (FcStrCmp (prefix, (const FcChar8 *) "default") == 0 ||
		 FcStrCmp (prefix, (const FcChar8 *) "cwd") == 0)
	{
	    /* Nothing to do */
	}
	else if (FcStrCmp (prefix, (const FcChar8 *) "relative") == 0)
	{
	    FcChar8 *p = FcStrRealPath (parse->name);

	    if (!p)
		return NULL;
	    parent = FcStrDirname (p);
	    if (!parent)
	    {
		free (p);
		return NULL;
	    }
	}
    }
#ifndef _WIN32
    /* For Win32, check this later for dealing with special cases */
    else
    {
	if (!FcStrIsAbsoluteFilename (path) && path[0] != '~')
	    FcConfigMessage (parse, FcSevereWarning, "Use of ambiguous path in <%s> element. please add prefix=\"cwd\" if current behavior is desired.", FcElementReverseMap (parse->pstack->element));
    }
#else
    if (strcmp ((const char *) path, "CUSTOMFONTDIR") == 0)
    {
	FcChar8 *p;
	path = buffer;
	if (!GetModuleFileName (NULL, (LPCH) buffer, sizeof (buffer) - 20))
	{
	    FcConfigMessage (parse, FcSevereError, "GetModuleFileName failed");
	    return NULL;
	}
	/*
	 * Must use the multi-byte aware function to search
	 * for backslash because East Asian double-byte code
	 * pages have characters with backslash as the second
	 * byte.
	 */
	p = _mbsrchr (path, '\\');
	if (p) *p = '\0';
	strcat ((char *) path, "\\fonts");
    }
    else if (strcmp ((const char *) path, "APPSHAREFONTDIR") == 0)
    {
	FcChar8 *p;
	path = buffer;
	if (!GetModuleFileName (NULL, (LPCH) buffer, sizeof (buffer) - 20))
	{
	    FcConfigMessage (parse, FcSevereError, "GetModuleFileName failed");
	    return NULL;
	}
	p = _mbsrchr (path, '\\');
	if (p) *p = '\0';
	strcat ((char *) path, "\\..\\share\\fonts");
    }
    else if (strcmp ((const char *) path, "WINDOWSUSERFONTDIR") == 0)
    {
        path = buffer;
        if (!(pSHGetFolderPathA && SUCCEEDED(pSHGetFolderPathA(NULL, /* CSIDL_LOCAL_APPDATA */ 28, NULL, 0, (char *) buffer))))
        {
            FcConfigMessage(parse, FcSevereError, "SHGetFolderPathA failed");
            return NULL;
        }
        strcat((char *) path, "\\Microsoft\\Windows\\Fonts");
    }
    else if (strcmp ((const char *) path, "WINDOWSFONTDIR") == 0)
    {
	int rc;
	path = buffer;
	_ensureWin32GettersReady();
	rc = pGetSystemWindowsDirectory ((LPSTR) buffer, sizeof (buffer) - 20);
	if (rc == 0 || rc > sizeof (buffer) - 20)
	{
	    FcConfigMessage (parse, FcSevereError, "GetSystemWindowsDirectory failed");
	    return NULL;
	}
	if (path [strlen ((const char *) path) - 1] != '\\')
	    strcat ((char *) path, "\\");
	strcat ((char *) path, "fonts");
    }
    else
    {
	if (!prefix)
	{
	    if (!FcStrIsAbsoluteFilename (path) && path[0] != '~')
		FcConfigMessage (parse, FcSevereWarning, "Use of ambiguous path in <%s> element. please add prefix=\"cwd\" if current behavior is desired.", FcElementReverseMap (parse->pstack->element));
	}
    }
#endif
    if (parent)
    {
	retval = FcStrBuildFilename (parent, path, NULL);
	FcStrFree (parent);
    }
    else
    {
	retval = FcStrdup (path);
    }
    if (!e)
	e = FcStrSetCreate ();
    else
    {
	FcChar8 *s;
	int i;

	for (i = 0; i < e->num; i++)
	{
	    s = FcStrBuildFilename (e->strs[i], path, NULL);
	    FcStrFree (e->strs[i]);
	    e->strs[i] = s;
	}
    }
    if (!FcStrSetInsert (e, retval, 0))
    {
	FcStrSetDestroy (e);
	e = NULL;
    }
    FcStrFree (retval);

    return e;
}

static void
FcStartElement(void *userData, const XML_Char *name, const XML_Char **attr)
{
    FcConfigParse   *parse = userData;
    FcElement	    element;

    element = FcElementMap (name);
    if (element == FcElementUnknown)
	FcConfigMessage (parse, FcSevereWarning, "unknown element \"%s\"", name);

    if (!FcPStackPush (parse, element, attr))
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    return;
}

static void
FcParseRescan (FcConfigParse *parse)
{
    int	    n = FcVStackElements (parse);
    while (n-- > 0)
    {
	FcVStack    *v = FcVStackFetch (parse, n);
	if (v->tag != FcVStackInteger)
	    FcConfigMessage (parse, FcSevereWarning, "non-integer rescan");
	else
	    parse->config->rescanInterval = v->u.integer;
    }
}

static void
FcParseInt (FcConfigParse *parse)
{
    FcChar8 *s, *end;
    int	    l;

    if (!parse->pstack)
	return;
    s = FcStrBufDoneStatic (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    end = 0;
    l = (int) strtol ((char *) s, (char **)&end, 0);
    if (end != s + strlen ((char *) s))
	FcConfigMessage (parse, FcSevereError, "\"%s\": not a valid integer", s);
    else
	FcVStackPushInteger (parse, l);
    FcStrBufDestroy (&parse->pstack->str);
}

/*
 * idea copied from glib g_ascii_strtod with
 * permission of the author (Alexander Larsson)
 */

#include <locale.h>

static double
FcStrtod (char *s, char **end)
{
#ifndef __BIONIC__
    struct lconv    *locale_data;
#endif
    const char	    *decimal_point;
    int		    dlen;
    char	    *dot;
    double	    v;

    /*
     * Have to swap the decimal point to match the current locale
     * if that locale doesn't use 0x2e
     */
#ifndef __BIONIC__
    locale_data = localeconv ();
    decimal_point = locale_data->decimal_point;
    dlen = strlen (decimal_point);
#else
    decimal_point = ".";
    dlen = 1;
#endif

    if ((dot = strchr (s, 0x2e)) &&
	(decimal_point[0] != 0x2e ||
	 decimal_point[1] != 0))
    {
	char	buf[128];
	int	slen = strlen (s);

	if (slen + dlen > (int) sizeof (buf))
	{
	    if (end)
		*end = s;
	    v = 0;
	}
	else
	{
	    char	*buf_end;
	    /* mantissa */
	    strncpy (buf, s, dot - s);
	    /* decimal point */
	    strcpy (buf + (dot - s), decimal_point);
	    /* rest of number */
	    strcpy (buf + (dot - s) + dlen, dot + 1);
	    buf_end = 0;
	    v = strtod (buf, &buf_end);
	    if (buf_end) {
		buf_end = s + (buf_end - buf);
		if (buf_end > dot)
		    buf_end -= dlen - 1;
	    }
	    if (end)
		*end = buf_end;
	}
    }
    else
	v = strtod (s, end);
    return v;
}

static void
FcParseDouble (FcConfigParse *parse)
{
    FcChar8 *s, *end;
    double  d;

    if (!parse->pstack)
	return;
    s = FcStrBufDoneStatic (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    end = 0;
    d = FcStrtod ((char *) s, (char **)&end);
    if (end != s + strlen ((char *) s))
	FcConfigMessage (parse, FcSevereError, "\"%s\": not a valid double", s);
    else
	FcVStackPushDouble (parse, d);
    FcStrBufDestroy (&parse->pstack->str);
}

static void
FcParseString (FcConfigParse *parse, FcVStackTag tag)
{
    FcChar8 *s;

    if (!parse->pstack)
	return;
    s = FcStrBufDone (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    if (!FcVStackPushString (parse, tag, s))
	FcStrFree (s);
}

static void
FcParseName (FcConfigParse *parse)
{
    const FcChar8   *kind_string;
    FcMatchKind	    kind;
    FcChar8 *s;
    FcObject object;

    kind_string = FcConfigGetAttribute (parse, "target");
    if (!kind_string)
	kind = FcMatchDefault;
    else
    {
	if (!strcmp ((char *) kind_string, "pattern"))
	    kind = FcMatchPattern;
	else if (!strcmp ((char *) kind_string, "font"))
	    kind = FcMatchFont;
	else if (!strcmp ((char *) kind_string, "default"))
	    kind = FcMatchDefault;
	else
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid name target \"%s\"", kind_string);
	    return;
	}
    }

    if (!parse->pstack)
	return;
    s = FcStrBufDone (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    object = FcObjectFromName ((const char *) s);

    FcVStackPushName (parse, kind, object);

    FcStrFree (s);
}

static void
FcParseMatrix (FcConfigParse *parse)
{
    FcExprMatrix m;

    m.yy = FcPopExpr (parse);
    m.yx = FcPopExpr (parse);
    m.xy = FcPopExpr (parse);
    m.xx = FcPopExpr (parse);

    if (!m.yy || !m.yx || !m.xy || !m.xx)
    {
	FcConfigMessage (parse, FcSevereWarning, "Missing values in matrix element");
	return;
    }
    if (FcPopExpr (parse))
      FcConfigMessage (parse, FcSevereError, "wrong number of matrix elements");
    else
      FcVStackPushMatrix (parse, &m);
}

static void
FcParseRange (FcConfigParse *parse)
{
    FcVStack	*vstack;
    FcRange	*r;
    FcChar32	n[2] = {0, 0};
    int		count = 1;
    double	d[2] = {0.0L, 0.0L};
    FcBool	dflag = FcFalse;

    while ((vstack = FcVStackPeek (parse)))
    {
	if (count < 0)
	{
	    FcConfigMessage (parse, FcSevereError, "too many elements in range");
	    return;
	}
	switch ((int) vstack->tag) {
	case FcVStackInteger:
	    if (dflag)
		d[count] = (double)vstack->u.integer;
	    else
		n[count] = vstack->u.integer;
	    break;
	case FcVStackDouble:
	    if (count == 0 && !dflag)
		d[1] = (double)n[1];
	    d[count] = vstack->u._double;
	    dflag = FcTrue;
	    break;
	default:
	    FcConfigMessage (parse, FcSevereError, "invalid element in range");
	    if (dflag)
		d[count] = 0.0L;
	    else
		n[count] = 0;
	    break;
	}
	count--;
	FcVStackPopAndDestroy (parse);
    }
    if (count >= 0)
    {
	FcConfigMessage (parse, FcSevereError, "invalid range");
	return;
    }
    if (dflag)
    {
	if (d[0] > d[1])
	{
	    FcConfigMessage (parse, FcSevereError, "invalid range");
	    return;
	}
	r = FcRangeCreateDouble (d[0], d[1]);
    }
    else
    {
	if (n[0] > n[1])
	{
	    FcConfigMessage (parse, FcSevereError, "invalid range");
	    return;
	}
	r = FcRangeCreateInteger (n[0], n[1]);
    }
    FcVStackPushRange (parse, r);
}

static FcBool
FcConfigLexBool (FcConfigParse *parse, const FcChar8 *bool_)
{
    FcBool  result = FcFalse;

    if (!FcNameBool (bool_, &result))
	FcConfigMessage (parse, FcSevereWarning, "\"%s\" is not known boolean",
			 bool_);
    return result;
}

static void
FcParseBool (FcConfigParse *parse)
{
    FcChar8 *s;

    if (!parse->pstack)
	return;
    s = FcStrBufDoneStatic (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    FcVStackPushBool (parse, FcConfigLexBool (parse, s));
    FcStrBufDestroy (&parse->pstack->str);
}

static void
FcParseCharSet (FcConfigParse *parse)
{
    FcVStack	*vstack;
    FcCharSet	*charset = FcCharSetCreate ();
    FcChar32	i, begin, end;
    int n = 0;

    while ((vstack = FcVStackPeek (parse)))
    {
	switch ((int) vstack->tag) {
	case FcVStackInteger:
	    if (!FcCharSetAddChar (charset, vstack->u.integer))
	    {
		FcConfigMessage (parse, FcSevereWarning, "invalid character: 0x%04x", vstack->u.integer);
	    }
	    else
		n++;
	    break;
	case FcVStackRange:
	    begin = (FcChar32) vstack->u.range->begin;
	    end = (FcChar32) vstack->u.range->end;

	    if (begin <= end)
	    {
	      for (i = begin; i <= end; i++)
	      {
		  if (!FcCharSetAddChar (charset, i))
		  {
		      FcConfigMessage (parse, FcSevereWarning, "invalid character: 0x%04x", i);
		  }
		  else
		      n++;
	      }
	    }
	    break;
	default:
		FcConfigMessage (parse, FcSevereError, "invalid element in charset");
		break;
	}
	FcVStackPopAndDestroy (parse);
    }
    if (n > 0)
	    FcVStackPushCharSet (parse, charset);
    else
	    FcCharSetDestroy (charset);
}

static void
FcParseLangSet (FcConfigParse *parse)
{
    FcVStack	*vstack;
    FcLangSet	*langset = FcLangSetCreate ();
    int n = 0;

    while ((vstack = FcVStackPeek (parse)))
    {
	switch ((int) vstack->tag) {
	case FcVStackString:
	    if (!FcLangSetAdd (langset, vstack->u.string))
	    {
		FcConfigMessage (parse, FcSevereWarning, "invalid langset: %s", vstack->u.string);
	    }
	    else
		n++;
	    break;
	default:
		FcConfigMessage (parse, FcSevereError, "invalid element in langset");
		break;
	}
	FcVStackPopAndDestroy (parse);
    }
    if (n > 0)
	    FcVStackPushLangSet (parse, langset);
    else
	    FcLangSetDestroy (langset);
}

static FcBool
FcConfigLexBinding (FcConfigParse   *parse,
		    const FcChar8   *binding_string,
		    FcValueBinding  *binding_ret)
{
    FcValueBinding binding;

    if (!binding_string)
	binding = FcValueBindingWeak;
    else
    {
	if (!strcmp ((char *) binding_string, "weak"))
	    binding = FcValueBindingWeak;
	else if (!strcmp ((char *) binding_string, "strong"))
	    binding = FcValueBindingStrong;
	else if (!strcmp ((char *) binding_string, "same"))
	    binding = FcValueBindingSame;
	else
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid binding \"%s\"", binding_string);
	    return FcFalse;
	}
    }
    *binding_ret = binding;
    return FcTrue;
}

static void
FcParseFamilies (FcConfigParse *parse, FcVStackTag tag)
{
    FcVStack	*vstack;
    FcExpr	*left, *expr = 0, *new;

    while ((vstack = FcVStackPeek (parse)))
    {
	if (vstack->tag != FcVStackFamily)
	{
	    FcConfigMessage (parse, FcSevereWarning, "non-family");
	    FcVStackPopAndDestroy (parse);
	    continue;
	}
	left = vstack->u.expr;
	vstack->tag = FcVStackNone;
	FcVStackPopAndDestroy (parse);
	if (expr)
	{
	    new = FcExprCreateOp (parse->config, left, FcOpComma, expr);
	    if (!new)
	    {
		FcConfigMessage (parse, FcSevereError, "out of memory");
		FcExprDestroy (left);
		FcExprDestroy (expr);
		break;
	    }
	    expr = new;
	}
	else
	    expr = left;
    }
    if (expr)
    {
	if (!FcVStackPushExpr (parse, tag, expr))
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
            FcExprDestroy (expr);
	}
    }
}

static void
FcParseFamily (FcConfigParse *parse)
{
    FcChar8 *s;
    FcExpr  *expr;

    if (!parse->pstack)
	return;
    s = FcStrBufDoneStatic (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    expr = FcExprCreateString (parse->config, s);
    FcStrBufDestroy (&parse->pstack->str);
    if (expr)
	FcVStackPushExpr (parse, FcVStackFamily, expr);
}

static void
FcParseAlias (FcConfigParse *parse)
{
    FcExpr	*family = 0, *accept = 0, *prefer = 0, *def = 0, *new = 0;
    FcEdit	*edit = 0;
    FcVStack	*vstack;
    FcRule	*rule = NULL, *r;
    FcValueBinding  binding;
    int		    n;

    if (!FcConfigLexBinding (parse, FcConfigGetAttribute (parse, "binding"), &binding))
	return;
    while ((vstack = FcVStackPeek (parse)))
    {
	switch ((int) vstack->tag) {
	case FcVStackFamily:
	    if (family)
	    {
		FcConfigMessage (parse, FcSevereWarning, "Having multiple <family> in <alias> isn't supported and may not work as expected");
		new = FcExprCreateOp (parse->config, vstack->u.expr, FcOpComma, family);
		if (!new)
		    FcConfigMessage (parse, FcSevereError, "out of memory");
		else
		    family = new;
	    }
	    else
		new = vstack->u.expr;
	    if (new)
	    {
		family = new;
		vstack->tag = FcVStackNone;
	    }
	    break;
	case FcVStackPrefer:
	    if (prefer)
		FcExprDestroy (prefer);
	    prefer = vstack->u.expr;
	    vstack->tag = FcVStackNone;
	    break;
	case FcVStackAccept:
	    if (accept)
		FcExprDestroy (accept);
	    accept = vstack->u.expr;
	    vstack->tag = FcVStackNone;
	    break;
	case FcVStackDefault:
	    if (def)
		FcExprDestroy (def);
	    def = vstack->u.expr;
	    vstack->tag = FcVStackNone;
	    break;
	case FcVStackTest:
	    if (rule)
	    {
		r = FcRuleCreate (FcRuleTest, vstack->u.test);
		r->next = rule;
		rule = r;
	    }
	    else
		rule = FcRuleCreate (FcRuleTest, vstack->u.test);
	    vstack->tag = FcVStackNone;
	    break;
	default:
	    FcConfigMessage (parse, FcSevereWarning, "bad alias");
	    break;
	}
	FcVStackPopAndDestroy (parse);
    }
    if (!family)
    {
	FcConfigMessage (parse, FcSevereError, "missing family in alias");
	if (prefer)
	    FcExprDestroy (prefer);
	if (accept)
	    FcExprDestroy (accept);
	if (def)
	    FcExprDestroy (def);
	if (rule)
	    FcRuleDestroy (rule);
	return;
    }
    if (!prefer &&
	!accept &&
	!def)
    {
	FcExprDestroy (family);
	if (rule)
	    FcRuleDestroy (rule);
	return;
    }
    else
    {
	FcTest *t = FcTestCreate (parse, FcMatchPattern,
				  FcQualAny,
				  (FcChar8 *) FC_FAMILY,
				  FC_OP (FcOpEqual, FcOpFlagIgnoreBlanks),
				  family);
	if (rule)
	{
	    for (r = rule; r->next; r = r->next);
	    r->next = FcRuleCreate (FcRuleTest, t);
	    r = r->next;
	}
	else
	{
	    r = rule = FcRuleCreate (FcRuleTest, t);
	}
    }
    if (prefer)
    {
	edit = FcEditCreate (parse,
			     FC_FAMILY_OBJECT,
			     FcOpPrepend,
			     prefer,
			     binding);
	if (!edit)
	    FcExprDestroy (prefer);
	else
	{
	    r->next = FcRuleCreate (FcRuleEdit, edit);
	    r = r->next;
	}
    }
    if (accept)
    {
	edit = FcEditCreate (parse,
			     FC_FAMILY_OBJECT,
			     FcOpAppend,
			     accept,
			     binding);
	if (!edit)
	    FcExprDestroy (accept);
	else
	{
	    r->next = FcRuleCreate (FcRuleEdit, edit);
	    r = r->next;
	}
    }
    if (def)
    {
	edit = FcEditCreate (parse,
			     FC_FAMILY_OBJECT,
			     FcOpAppendLast,
			     def,
			     binding);
	if (!edit)
	    FcExprDestroy (def);
	else
	{
	    r->next = FcRuleCreate (FcRuleEdit, edit);
	    r = r->next;
	}
    }
    if ((n = FcRuleSetAdd (parse->ruleset, rule, FcMatchPattern)) == -1)
	FcRuleDestroy (rule);
    else
	if (parse->config->maxObjects < n)
	    parse->config->maxObjects = n;
}

static void
FcParseDescription (FcConfigParse *parse)
{
    const FcChar8 *domain;
    FcChar8 *desc;

    domain = FcConfigGetAttribute (parse, "domain");
    desc = FcStrBufDone (&parse->pstack->str);
    if (!desc)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    FcRuleSetAddDescription (parse->ruleset, domain, desc);

    FcStrFree (desc);
}

static void
FcParseRemapDir (FcConfigParse *parse)
{
    const FcChar8 *path, *attr, *data, *salt;
    FcStrSet *prefix_dirs = NULL;

    data = FcStrBufDoneStatic (&parse->pstack->str);
    if (!data)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    if (data[0] == 0)
    {
	FcConfigMessage (parse, FcSevereWarning, "empty font directory name for remap ignored");
	return;
    }
    path = FcConfigGetAttribute (parse, "as-path");
    if (!path)
    {
	FcConfigMessage (parse, FcSevereWarning, "Missing as-path in remap-dir");
	return;
    }
    attr = FcConfigGetAttribute (parse, "prefix");
    salt = FcConfigGetAttribute (parse, "salt");
    prefix_dirs = _get_real_paths_from_prefix (parse, data, attr);
    if (prefix_dirs)
    {
	FcStrList *l = FcStrListCreate (prefix_dirs);
	FcChar8 *prefix;

	FcStrSetDestroy (prefix_dirs);
	while ((prefix = FcStrListNext (l)))
	{
	    if (!prefix || prefix[0] == 0)
	    {
		/* nop */
	    }
	    else if (!parse->scanOnly && (!FcStrUsesHome (prefix) || FcConfigHome ()))
	    {
		if (!FcConfigAddFontDir (parse->config, prefix, path, salt))
		    FcConfigMessage (parse, FcSevereError, "out of memory; cannot create remap data for %s as %s", prefix, path);
	    }
	    FcStrBufDestroy (&parse->pstack->str);
	}
	FcStrListDone (l);
    }
}

static void
FcParseResetDirs (FcConfigParse *parse)
{
    if (!parse->scanOnly)
    {
	if (!FcConfigResetFontDirs (parse->config))
	    FcConfigMessage (parse, FcSevereError, "Unable to reset fonts dirs");
    }
}

static FcExpr *
FcPopExpr (FcConfigParse *parse)
{
    FcVStack	*vstack = FcVStackPeek (parse);
    FcExpr	*expr = 0;
    if (!vstack)
	return 0;
    switch ((int) vstack->tag) {
    case FcVStackNone:
	break;
    case FcVStackString:
    case FcVStackFamily:
	expr = FcExprCreateString (parse->config, vstack->u.string);
	break;
    case FcVStackName:
	expr = FcExprCreateName (parse->config, vstack->u.name);
	break;
    case FcVStackConstant:
	expr = FcExprCreateConst (parse->config, vstack->u.string);
	break;
    case FcVStackGlob:
	/* XXX: What's the correct action here? (CDW) */
	break;
    case FcVStackPrefer:
    case FcVStackAccept:
    case FcVStackDefault:
	expr = vstack->u.expr;
	vstack->tag = FcVStackNone;
	break;
    case FcVStackInteger:
	expr = FcExprCreateInteger (parse->config, vstack->u.integer);
	break;
    case FcVStackDouble:
	expr = FcExprCreateDouble (parse->config, vstack->u._double);
	break;
    case FcVStackMatrix:
	expr = FcExprCreateMatrix (parse->config, vstack->u.matrix);
	break;
    case FcVStackRange:
	expr = FcExprCreateRange (parse->config, vstack->u.range);
	break;
    case FcVStackBool:
	expr = FcExprCreateBool (parse->config, vstack->u.bool_);
	break;
    case FcVStackCharSet:
	expr = FcExprCreateCharSet (parse->config, vstack->u.charset);
	break;
    case FcVStackLangSet:
	expr = FcExprCreateLangSet (parse->config, vstack->u.langset);
	break;
    case FcVStackTest:
	break;
    case FcVStackExpr:
	expr = vstack->u.expr;
	vstack->tag = FcVStackNone;
	break;
    case FcVStackEdit:
	break;
    default:
	break;
    }
    FcVStackPopAndDestroy (parse);
    return expr;
}

/*
 * This builds a tree of binary operations.  Note
 * that every operator is defined so that if only
 * a single operand is contained, the value of the
 * whole expression is the value of the operand.
 *
 * This code reduces in that case to returning that
 * operand.
 */
static FcExpr *
FcPopBinary (FcConfigParse *parse, FcOp op)
{
    FcExpr  *left, *expr = 0, *new;

    while ((left = FcPopExpr (parse)))
    {
	if (expr)
	{
	    new = FcExprCreateOp (parse->config, left, op, expr);
	    if (!new)
	    {
		FcConfigMessage (parse, FcSevereError, "out of memory");
		FcExprDestroy (left);
		FcExprDestroy (expr);
		return 0;
	    }
	    expr = new;
	}
	else
	    expr = left;
    }
    return expr;
}

static void
FcParseBinary (FcConfigParse *parse, FcOp op)
{
    FcExpr  *expr = FcPopBinary (parse, op);
    if (expr)
	FcVStackPushExpr (parse, FcVStackExpr, expr);
}

/*
 * This builds a a unary operator, it consumes only
 * a single operand
 */

static FcExpr *
FcPopUnary (FcConfigParse *parse, FcOp op)
{
    FcExpr  *operand, *new = 0;

    if ((operand = FcPopExpr (parse)))
    {
	new = FcExprCreateOp (parse->config, operand, op, 0);
	if (!new)
	{
	    FcExprDestroy (operand);
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	}
    }
    return new;
}

static void
FcParseUnary (FcConfigParse *parse, FcOp op)
{
    FcExpr  *expr = FcPopUnary (parse, op);
    if (expr)
	FcVStackPushExpr (parse, FcVStackExpr, expr);
}

static void
FcParseDir (FcConfigParse *parse)
{
    const FcChar8 *attr, *data, *salt;
    FcStrSet *prefix_dirs = NULL;

    data = FcStrBufDoneStatic (&parse->pstack->str);
    if (!data)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    if (data[0] == 0)
    {
	FcConfigMessage (parse, FcSevereWarning, "empty font directory name ignored");
	return;
    }
    attr = FcConfigGetAttribute (parse, "prefix");
    salt = FcConfigGetAttribute (parse, "salt");
    prefix_dirs = _get_real_paths_from_prefix (parse, data, attr);
    if (prefix_dirs)
    {
	FcStrList *l = FcStrListCreate (prefix_dirs);
	FcChar8 *prefix;

	FcStrSetDestroy (prefix_dirs);
	while ((prefix = FcStrListNext (l)))
	{
	    if (!prefix || prefix[0] == 0)
	    {
		/* nop */
	    }
	    else if (!parse->scanOnly && (!FcStrUsesHome (prefix) || FcConfigHome ()))
	    {
		if (!FcConfigAddFontDir (parse->config, prefix, NULL, salt))
		    FcConfigMessage (parse, FcSevereError, "out of memory; cannot add directory %s", prefix);
	    }
	    FcStrBufDestroy (&parse->pstack->str);
	}
	FcStrListDone (l);
    }
}

static void
FcParseCacheDir (FcConfigParse *parse)
{
    const FcChar8 *attr;
    FcChar8 *prefix = NULL, *p, *data = NULL;

    attr = FcConfigGetAttribute (parse, "prefix");
    if (attr && FcStrCmp (attr, (const FcChar8 *)"xdg") == 0)
    {
	prefix = FcConfigXdgCacheHome ();
	/* home directory might be disabled.
	 * simply ignore this element.
	 */
	if (!prefix)
	    goto bail;
    }
    data = FcStrBufDone (&parse->pstack->str);
    if (!data)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	data = prefix;
	goto bail;
    }
    if (data[0] == 0)
    {
	FcConfigMessage (parse, FcSevereWarning, "empty cache directory name ignored");
	FcStrFree (data);
	data = prefix;
	goto bail;
    }
    if (prefix)
    {
	size_t plen = strlen ((const char *)prefix);
	size_t dlen = strlen ((const char *)data);

	p = realloc (prefix, plen + 1 + dlen + 1);
	if (!p)
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	    FcStrFree (prefix);
	    goto bail;
	}
	prefix = p;
	prefix[plen] = FC_DIR_SEPARATOR;
	memcpy (&prefix[plen + 1], data, dlen);
	prefix[plen + 1 + dlen] = 0;
	FcStrFree (data);
	data = prefix;
    }
#ifdef _WIN32
    else if (data[0] == '/' && fontconfig_instprefix[0] != '\0')
    {
	size_t plen = strlen ((const char *)fontconfig_instprefix);
	size_t dlen = strlen ((const char *)data);

	prefix = malloc (plen + 1 + dlen + 1);
	if (!prefix)
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	    goto bail;
	}
	strcpy ((char *) prefix, (char *) fontconfig_instprefix);
	prefix[plen] = FC_DIR_SEPARATOR;
	memcpy (&prefix[plen + 1], data, dlen);
	prefix[plen + 1 + dlen] = 0;
	FcStrFree (data);
	data = prefix;
    }
    else if (strcmp ((const char *) data, "WINDOWSTEMPDIR_FONTCONFIG_CACHE") == 0)
    {
	int rc;

	FcStrFree (data);
	data = malloc (1000);
	if (!data)
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	    goto bail;
	}
	rc = GetTempPath (800, (LPSTR) data);
	if (rc == 0 || rc > 800)
	{
	    FcConfigMessage (parse, FcSevereError, "GetTempPath failed");
	    goto bail;
	}
	if (data [strlen ((const char *) data) - 1] != '\\')
	    strcat ((char *) data, "\\");
	strcat ((char *) data, "fontconfig\\cache");
    }
    else if (strcmp ((const char *) data, "LOCAL_APPDATA_FONTCONFIG_CACHE") == 0)
    {
	char szFPath[MAX_PATH + 1];
	size_t len;

	if (!(pSHGetFolderPathA && SUCCEEDED(pSHGetFolderPathA(NULL, /* CSIDL_LOCAL_APPDATA */ 28, NULL, 0, szFPath))))
	{
	    FcConfigMessage (parse, FcSevereError, "SHGetFolderPathA failed");
	    goto bail;
	}
	strncat(szFPath, "\\fontconfig\\cache", MAX_PATH - 1 - strlen(szFPath));
	len = strlen(szFPath) + 1;
	FcStrFree (data);
	data = malloc(len);
	if (!data)
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	    goto bail;
	}
	strncpy((char *) data, szFPath, len);
    }
#endif
    if (strlen ((char *) data) == 0)
	FcConfigMessage (parse, FcSevereWarning, "empty cache directory name ignored");
    else if (!parse->scanOnly && (!FcStrUsesHome (data) || FcConfigHome ()))
    {
	if (!FcConfigAddCacheDir (parse->config, data))
	    FcConfigMessage (parse, FcSevereError, "out of memory; cannot add cache directory %s", data);
    }
    FcStrBufDestroy (&parse->pstack->str);

  bail:
    if (data)
	FcStrFree (data);
}

void
FcConfigPathFini (void)
{
    FcChar8 *s;

retry_dir:
    s = fc_atomic_ptr_get (&__fc_userdir);
    if (!fc_atomic_ptr_cmpexch (&__fc_userdir, s, NULL))
	goto retry_dir;
    free (s);

retry_conf:
    s = fc_atomic_ptr_get (&__fc_userconf);
    if (!fc_atomic_ptr_cmpexch (&__fc_userconf, s, NULL))
	goto retry_conf;
    free (s);
}

static void
FcParseInclude (FcConfigParse *parse)
{
    FcChar8	    *s;
    const FcChar8   *attr;
    FcBool	    ignore_missing = FcFalse;
#ifndef _WIN32
    FcBool	    deprecated = FcFalse;
#endif
    FcChar8	    *prefix = NULL, *p;
    FcChar8	    *userdir = NULL, *userconf = NULL;
    FcRuleSet	    *ruleset;
    FcMatchKind	    k;

    s = FcStrBufDoneStatic (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	goto bail;
    }
    attr = FcConfigGetAttribute (parse, "ignore_missing");
    if (attr && FcConfigLexBool (parse, (FcChar8 *) attr) == FcTrue)
	ignore_missing = FcTrue;
    attr = FcConfigGetAttribute (parse, "deprecated");
#ifndef _WIN32
    if (attr && FcConfigLexBool (parse, (FcChar8 *) attr) == FcTrue)
        deprecated = FcTrue;
#endif
    attr = FcConfigGetAttribute (parse, "prefix");
    if (attr && FcStrCmp (attr, (const FcChar8 *)"xdg") == 0)
    {
	prefix = FcConfigXdgConfigHome ();
	/* home directory might be disabled.
	 * simply ignore this element.
	 */
	if (!prefix)
	    goto bail;
    }
    if (prefix)
    {
	size_t plen = strlen ((const char *)prefix);
	size_t dlen = strlen ((const char *)s);
	FcChar8 *u;

	p = realloc (prefix, plen + 1 + dlen + 1);
	if (!p)
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	    goto bail;
	}
	prefix = p;
	prefix[plen] = FC_DIR_SEPARATOR;
	memcpy (&prefix[plen + 1], s, dlen);
	prefix[plen + 1 + dlen] = 0;
	s = prefix;
	if (FcFileIsDir (s))
	{
	userdir:
	    userdir = fc_atomic_ptr_get (&__fc_userdir);
	    if (!userdir)
	    {
		u = FcStrdup (s);
		if (!fc_atomic_ptr_cmpexch (&__fc_userdir, userdir, u))
		{
		    free (u);
		    goto userdir;
		}
		userdir = u;
	    }
	}
	else if (FcFileIsFile (s))
	{
	userconf:
	    userconf = fc_atomic_ptr_get (&__fc_userconf);
	    if (!userconf)
	    {
		u = FcStrdup (s);
		if (!fc_atomic_ptr_cmpexch (&__fc_userconf, userconf, u))
		{
		    free (u);
		    goto userconf;
		}
		userconf = u;
	    }
	}
	else
	{
	    /* No config dir nor file on the XDG directory spec compliant place
	     * so need to guess what it is supposed to be.
	     */
	    if (FcStrStr (s, (const FcChar8 *)"conf.d") != NULL)
		goto userdir;
	    else
		goto userconf;
	}
    }
    /* flush the ruleset into the queue */
    ruleset = parse->ruleset;
    parse->ruleset = FcRuleSetCreate (ruleset->name);
    FcRuleSetEnable (parse->ruleset, ruleset->enabled);
    FcRuleSetAddDescription (parse->ruleset, ruleset->domain, ruleset->description);
    for (k = FcMatchKindBegin; k < FcMatchKindEnd; k++)
    {
	FcPtrListIter iter;

	FcPtrListIterInit (ruleset->subst[k], &iter);
	if (FcPtrListIterIsValid (ruleset->subst[k], &iter))
	{
	    FcPtrListIterInitAtLast (parse->config->subst[k], &iter);
	    FcRuleSetReference (ruleset);
	    FcPtrListIterAdd (parse->config->subst[k], &iter, ruleset);
	}
    }
    FcRuleSetDestroy (ruleset);
    if (!_FcConfigParse (parse->config, s, !ignore_missing, !parse->scanOnly))
	parse->error = FcTrue;
#ifndef _WIN32
    else
    {
        FcChar8 *filename;
	static FcBool warn_conf = FcFalse, warn_confd = FcFalse;

        filename = FcConfigGetFilename(parse->config, s);
	if (deprecated == FcTrue &&
	    filename != NULL &&
	    userdir != NULL &&
	    !FcFileIsLink (filename))
	{
	    if (FcFileIsDir (filename))
	    {
		FcChar8 *parent = FcStrDirname (userdir);

		if (!FcFileIsDir (parent))
		    FcMakeDirectory (parent);
		FcStrFree (parent);
		if (FcFileIsDir (userdir) ||
		    rename ((const char *)filename, (const char *)userdir) != 0 ||
		    symlink ((const char *)userdir, (const char *)filename) != 0)
		{
		    if (!warn_confd)
		    {
			FcConfigMessage (parse, FcSevereWarning, "reading configurations from %s is deprecated. please move it to %s manually", s, userdir);
			warn_confd = FcTrue;
		    }
		}
	    }
	    else
	    {
		FcChar8 *parent = FcStrDirname (userconf);

		if (!FcFileIsDir (parent))
		    FcMakeDirectory (parent);
		FcStrFree (parent);
		if (FcFileIsFile (userconf) ||
		    rename ((const char *)filename, (const char *)userconf) != 0 ||
		    symlink ((const char *)userconf, (const char *)filename) != 0)
		{
		    if (!warn_conf)
		    {
			FcConfigMessage (parse, FcSevereWarning, "reading configurations from %s is deprecated. please move it to %s manually", s, userconf);
			warn_conf = FcTrue;
		    }
		}
	    }
        }
        if(filename)
            FcStrFree(filename);
    }
#endif
    FcStrBufDestroy (&parse->pstack->str);

  bail:
    if (prefix)
	FcStrFree (prefix);
}

typedef struct _FcOpMap {
    char    name[16];
    FcOp    op;
} FcOpMap;

static FcOp
FcConfigLexOp (const FcChar8 *op, const FcOpMap	*map, int nmap)
{
    int	i;

    for (i = 0; i < nmap; i++)
	if (!strcmp ((char *) op, map[i].name))
	    return map[i].op;
    return FcOpInvalid;
}

static const FcOpMap fcCompareOps[] = {
    { "eq",		FcOpEqual	    },
    { "not_eq",		FcOpNotEqual	    },
    { "less",		FcOpLess	    },
    { "less_eq",	FcOpLessEqual	    },
    { "more",		FcOpMore	    },
    { "more_eq",	FcOpMoreEqual	    },
    { "contains",	FcOpContains	    },
    { "not_contains",	FcOpNotContains	    }
};

#define NUM_COMPARE_OPS	(int) (sizeof fcCompareOps / sizeof fcCompareOps[0])

static FcOp
FcConfigLexCompare (const FcChar8 *compare)
{
    return FcConfigLexOp (compare, fcCompareOps, NUM_COMPARE_OPS);
}

static void
FcParseTest (FcConfigParse *parse)
{
    const FcChar8   *kind_string;
    FcMatchKind	    kind;
    const FcChar8   *qual_string;
    FcQual	    qual;
    const FcChar8   *name;
    const FcChar8   *compare_string;
    FcOp	    compare;
    FcExpr	    *expr;
    FcTest	    *test;
    const FcChar8   *iblanks_string;
    int              flags = 0;

    kind_string = FcConfigGetAttribute (parse, "target");
    if (!kind_string)
	kind = FcMatchDefault;
    else
    {
	if (!strcmp ((char *) kind_string, "pattern"))
	    kind = FcMatchPattern;
	else if (!strcmp ((char *) kind_string, "font"))
	    kind = FcMatchFont;
	else if (!strcmp ((char *) kind_string, "scan"))
	    kind = FcMatchScan;
	else if (!strcmp ((char *) kind_string, "default"))
	    kind = FcMatchDefault;
	else
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid test target \"%s\"", kind_string);
	    return;
	}
    }
    qual_string = FcConfigGetAttribute (parse, "qual");
    if (!qual_string)
	qual = FcQualAny;
    else
    {
	if (!strcmp ((char *) qual_string, "any"))
	    qual = FcQualAny;
	else if (!strcmp ((char *) qual_string, "all"))
	    qual = FcQualAll;
	else if (!strcmp ((char *) qual_string, "first"))
	    qual = FcQualFirst;
	else if (!strcmp ((char *) qual_string, "not_first"))
	    qual = FcQualNotFirst;
	else
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid test qual \"%s\"", qual_string);
	    return;
	}
    }
    name = FcConfigGetAttribute (parse, "name");
    if (!name)
    {
	FcConfigMessage (parse, FcSevereWarning, "missing test name");
	return;
    }
    compare_string = FcConfigGetAttribute (parse, "compare");
    if (!compare_string)
	compare = FcOpEqual;
    else
    {
	compare = FcConfigLexCompare (compare_string);
	if (compare == FcOpInvalid)
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid test compare \"%s\"", compare_string);
	    return;
	}
    }
    iblanks_string = FcConfigGetAttribute (parse, "ignore-blanks");
    if (iblanks_string)
    {
	FcBool f = FcFalse;

	if (!FcNameBool (iblanks_string, &f))
	{
	    FcConfigMessage (parse,
			     FcSevereWarning,
			     "invalid test ignore-blanks \"%s\"", iblanks_string);
	}
	if (f)
	    flags |= FcOpFlagIgnoreBlanks;
    }
    expr = FcPopBinary (parse, FcOpComma);
    if (!expr)
    {
	FcConfigMessage (parse, FcSevereWarning, "missing test expression");
	return;
    }
    if (expr->op == FcOpComma)
    {
	FcConfigMessage (parse, FcSevereWarning, "Having multiple values in <test> isn't supported and may not work as expected");
    }
    test = FcTestCreate (parse, kind, qual, name, FC_OP (compare, flags), expr);
    if (!test)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    FcVStackPushTest (parse, test);
}

static const FcOpMap fcModeOps[] = {
    { "assign",		FcOpAssign	    },
    { "assign_replace",	FcOpAssignReplace   },
    { "prepend",	FcOpPrepend	    },
    { "prepend_first",	FcOpPrependFirst    },
    { "append",		FcOpAppend	    },
    { "append_last",	FcOpAppendLast	    },
    { "delete",		FcOpDelete	    },
    { "delete_all",	FcOpDeleteAll	    },
};

#define NUM_MODE_OPS (int) (sizeof fcModeOps / sizeof fcModeOps[0])

static FcOp
FcConfigLexMode (const FcChar8 *mode)
{
    return FcConfigLexOp (mode, fcModeOps, NUM_MODE_OPS);
}

static void
FcParseEdit (FcConfigParse *parse)
{
    const FcChar8   *name;
    const FcChar8   *mode_string;
    FcOp	    mode;
    FcValueBinding  binding;
    FcExpr	    *expr;
    FcEdit	    *edit;

    name = FcConfigGetAttribute (parse, "name");
    if (!name)
    {
	FcConfigMessage (parse, FcSevereWarning, "missing edit name");
	return;
    }
    mode_string = FcConfigGetAttribute (parse, "mode");
    if (!mode_string)
	mode = FcOpAssign;
    else
    {
	mode = FcConfigLexMode (mode_string);
	if (mode == FcOpInvalid)
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid edit mode \"%s\"", mode_string);
	    return;
	}
    }
    if (!FcConfigLexBinding (parse, FcConfigGetAttribute (parse, "binding"), &binding))
	return;

    expr = FcPopBinary (parse, FcOpComma);
    if ((mode == FcOpDelete || mode == FcOpDeleteAll) &&
	expr != NULL)
    {
	FcConfigMessage (parse, FcSevereWarning, "Expression doesn't take any effects for delete and delete_all");
	FcExprDestroy (expr);
	expr = NULL;
    }
    edit = FcEditCreate (parse, FcObjectFromName ((char *) name),
			 mode, expr, binding);
    if (!edit)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	FcExprDestroy (expr);
	return;
    }
    if (!FcVStackPushEdit (parse, edit))
	FcEditDestroy (edit);
}

static void
FcParseMatch (FcConfigParse *parse)
{
    const FcChar8   *kind_name;
    FcMatchKind	    kind;
    FcVStack	    *vstack;
    FcRule	    *rule = NULL, *r;
    int		    n;

    kind_name = FcConfigGetAttribute (parse, "target");
    if (!kind_name)
	kind = FcMatchPattern;
    else
    {
	if (!strcmp ((char *) kind_name, "pattern"))
	    kind = FcMatchPattern;
	else if (!strcmp ((char *) kind_name, "font"))
	    kind = FcMatchFont;
	else if (!strcmp ((char *) kind_name, "scan"))
	    kind = FcMatchScan;
	else
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid match target \"%s\"", kind_name);
	    return;
	}
    }
    while ((vstack = FcVStackPeek (parse)))
    {
	switch ((int) vstack->tag) {
	case FcVStackTest:
	    r = FcRuleCreate (FcRuleTest, vstack->u.test);
	    if (rule)
		r->next = rule;
	    rule = r;
	    vstack->tag = FcVStackNone;
	    break;
	case FcVStackEdit:
	    if (kind == FcMatchScan && vstack->u.edit->object > FC_MAX_BASE_OBJECT)
	    {
		FcConfigMessage (parse, FcSevereError,
				 "<match target=\"scan\"> cannot edit user-defined object \"%s\"",
				 FcObjectName(vstack->u.edit->object));
		if (rule)
		    FcRuleDestroy (rule);
		return;
	    }
	    r = FcRuleCreate (FcRuleEdit, vstack->u.edit);
	    if (rule)
		r->next = rule;
	    rule = r;
	    vstack->tag = FcVStackNone;
	    break;
	default:
	    FcConfigMessage (parse, FcSevereWarning, "invalid match element");
	    break;
	}
	FcVStackPopAndDestroy (parse);
    }
    if (!rule)
    {
	FcConfigMessage (parse, FcSevereWarning, "No <test> nor <edit> elements in <match>");
	return;
    }
    if ((n = FcRuleSetAdd (parse->ruleset, rule, kind)) == -1)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	FcRuleDestroy (rule);
    }
    else
	if (parse->config->maxObjects < n)
	    parse->config->maxObjects = n;
}

static void
FcParseAcceptRejectFont (FcConfigParse *parse, FcElement element)
{
    FcVStack	*vstack;

    while ((vstack = FcVStackPeek (parse)))
    {
	switch ((int) vstack->tag) {
	case FcVStackGlob:
	    if (!parse->scanOnly && !FcConfigGlobAdd (parse->config,
						      vstack->u.string,
						      element == FcElementAcceptfont))
	    {
			if (FcStrUsesHome(vstack->u.string) && FcConfigHome() == NULL)
				FcConfigMessage (parse, FcSevereWarning, "Home is disabled");
			else
				FcConfigMessage (parse, FcSevereError, "out of memory");
	    }
	    else
	    {
		if (parse->scanOnly && vstack->u.string)
		{
		    FcStrFree (vstack->u.string);
		    vstack->tag = FcVStackNone;
		}
	    }
	    break;
	case FcVStackPattern:
	    if (!parse->scanOnly && !FcConfigPatternsAdd (parse->config,
							  vstack->u.pattern,
							  element == FcElementAcceptfont))
	    {
		FcConfigMessage (parse, FcSevereError, "out of memory");
	    }
	    else
	    {
		if (parse->scanOnly && vstack->u.pattern)
		    FcPatternDestroy (vstack->u.pattern);
		vstack->tag = FcVStackNone;
	    }
	    break;
	default:
	    FcConfigMessage (parse, FcSevereWarning, "bad font selector");
	    break;
	}
	FcVStackPopAndDestroy (parse);
    }
}


static FcValue
FcPopValue (FcConfigParse *parse)
{
    FcVStack	*vstack = FcVStackPeek (parse);
    FcValue	value;

    value.type = FcTypeVoid;

    if (!vstack)
	return value;

    switch ((int) vstack->tag) {
    case FcVStackString:
	value.u.s = FcStrdup (vstack->u.string);
	if (value.u.s)
	    value.type = FcTypeString;
	break;
    case FcVStackConstant:
	if (FcNameConstant (vstack->u.string, &value.u.i))
	    value.type = FcTypeInteger;
	break;
    case FcVStackInteger:
	value.u.i = vstack->u.integer;
	value.type = FcTypeInteger;
	break;
    case FcVStackDouble:
	value.u.d = vstack->u._double;
	value.type = FcTypeDouble;
	break;
    case FcVStackBool:
	value.u.b = vstack->u.bool_;
	value.type = FcTypeBool;
	break;
    case FcVStackCharSet:
	value.u.c = FcCharSetCopy (vstack->u.charset);
	if (value.u.c)
	    value.type = FcTypeCharSet;
	break;
    case FcVStackLangSet:
	value.u.l = FcLangSetCopy (vstack->u.langset);
	if (value.u.l)
	    value.type = FcTypeLangSet;
	break;
    case FcVStackRange:
	value.u.r = FcRangeCopy (vstack->u.range);
	if (value.u.r)
	    value.type = FcTypeRange;
	break;
    default:
	FcConfigMessage (parse, FcSevereWarning, "unknown pattern element %d",
			 vstack->tag);
	break;
    }
    FcVStackPopAndDestroy (parse);

    return value;
}

static void
FcParsePatelt (FcConfigParse *parse)
{
    FcValue	value;
    FcPattern	*pattern = FcPatternCreate ();
    const char	*name;

    if (!pattern)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }

    name = (char *) FcConfigGetAttribute (parse, "name");
    if (!name)
    {
	FcConfigMessage (parse, FcSevereWarning, "missing pattern element name");
	FcPatternDestroy (pattern);
	return;
    }

    for (;;)
    {
	value = FcPopValue (parse);
	if (value.type == FcTypeVoid)
	    break;
	if (!FcPatternAdd (pattern, name, value, FcTrue))
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
            FcValueDestroy(value);
	    break;
	}
        FcValueDestroy(value);
    }

    FcVStackPushPattern (parse, pattern);
}

static void
FcParsePattern (FcConfigParse *parse)
{
    FcVStack	*vstack;
    FcPattern	*pattern = FcPatternCreate ();

    if (!pattern)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }

    while ((vstack = FcVStackPeek (parse)))
    {
	switch ((int) vstack->tag) {
	case FcVStackPattern:
	    if (!FcPatternAppend (pattern, vstack->u.pattern))
	    {
		FcConfigMessage (parse, FcSevereError, "out of memory");
		FcPatternDestroy (pattern);
		return;
	    }
	    break;
	default:
	    FcConfigMessage (parse, FcSevereWarning, "unknown pattern element");
	    break;
	}
	FcVStackPopAndDestroy (parse);
    }

    FcVStackPushPattern (parse, pattern);
}

static void
FcEndElement(void *userData, const XML_Char *name FC_UNUSED)
{
    FcConfigParse   *parse = userData;
    FcChar8	    *data;

    if (!parse->pstack)
	return;
    switch (parse->pstack->element) {
    case FcElementNone:
	break;
    case FcElementFontconfig:
	break;
    case FcElementDir:
	FcParseDir (parse);
	break;
    case FcElementCacheDir:
	FcParseCacheDir (parse);
	break;
    case FcElementCache:
	data = FcStrBufDoneStatic (&parse->pstack->str);
	if (!data)
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	    break;
	}
	/* discard this data; no longer used */
	FcStrBufDestroy (&parse->pstack->str);
	break;
    case FcElementInclude:
	FcParseInclude (parse);
	break;
    case FcElementConfig:
	break;
    case FcElementMatch:
	FcParseMatch (parse);
	break;
    case FcElementAlias:
	FcParseAlias (parse);
	break;
    case FcElementDescription:
	FcParseDescription (parse);
	break;
    case FcElementRemapDir:
	FcParseRemapDir (parse);
	break;
    case FcElementResetDirs:
	FcParseResetDirs (parse);
	break;

    case FcElementRescan:
	FcParseRescan (parse);
	break;

    case FcElementPrefer:
	FcParseFamilies (parse, FcVStackPrefer);
	break;
    case FcElementAccept:
	FcParseFamilies (parse, FcVStackAccept);
	break;
    case FcElementDefault:
	FcParseFamilies (parse, FcVStackDefault);
	break;
    case FcElementFamily:
	FcParseFamily (parse);
	break;

    case FcElementTest:
	FcParseTest (parse);
	break;
    case FcElementEdit:
	FcParseEdit (parse);
	break;

    case FcElementInt:
	FcParseInt (parse);
	break;
    case FcElementDouble:
	FcParseDouble (parse);
	break;
    case FcElementString:
	FcParseString (parse, FcVStackString);
	break;
    case FcElementMatrix:
	FcParseMatrix (parse);
	break;
    case FcElementRange:
	FcParseRange (parse);
	break;
    case FcElementBool:
	FcParseBool (parse);
	break;
    case FcElementCharSet:
	FcParseCharSet (parse);
	break;
    case FcElementLangSet:
	FcParseLangSet (parse);
	break;
    case FcElementSelectfont:
	break;
    case FcElementAcceptfont:
    case FcElementRejectfont:
	FcParseAcceptRejectFont (parse, parse->pstack->element);
	break;
    case FcElementGlob:
	FcParseString (parse, FcVStackGlob);
	break;
    case FcElementPattern:
	FcParsePattern (parse);
	break;
    case FcElementPatelt:
	FcParsePatelt (parse);
	break;
    case FcElementName:
	FcParseName (parse);
	break;
    case FcElementConst:
	FcParseString (parse, FcVStackConstant);
	break;
    case FcElementOr:
	FcParseBinary (parse, FcOpOr);
	break;
    case FcElementAnd:
	FcParseBinary (parse, FcOpAnd);
	break;
    case FcElementEq:
	FcParseBinary (parse, FcOpEqual);
	break;
    case FcElementNotEq:
	FcParseBinary (parse, FcOpNotEqual);
	break;
    case FcElementLess:
	FcParseBinary (parse, FcOpLess);
	break;
    case FcElementLessEq:
	FcParseBinary (parse, FcOpLessEqual);
	break;
    case FcElementMore:
	FcParseBinary (parse, FcOpMore);
	break;
    case FcElementMoreEq:
	FcParseBinary (parse, FcOpMoreEqual);
	break;
    case FcElementContains:
	FcParseBinary (parse, FcOpContains);
	break;
    case FcElementNotContains:
	FcParseBinary (parse, FcOpNotContains);
	break;
    case FcElementPlus:
	FcParseBinary (parse, FcOpPlus);
	break;
    case FcElementMinus:
	FcParseBinary (parse, FcOpMinus);
	break;
    case FcElementTimes:
	FcParseBinary (parse, FcOpTimes);
	break;
    case FcElementDivide:
	FcParseBinary (parse, FcOpDivide);
	break;
    case FcElementNot:
	FcParseUnary (parse, FcOpNot);
	break;
    case FcElementIf:
	FcParseBinary (parse, FcOpQuest);
	break;
    case FcElementFloor:
	FcParseUnary (parse, FcOpFloor);
	break;
    case FcElementCeil:
	FcParseUnary (parse, FcOpCeil);
	break;
    case FcElementRound:
	FcParseUnary (parse, FcOpRound);
	break;
    case FcElementTrunc:
	FcParseUnary (parse, FcOpTrunc);
	break;
    case FcElementUnknown:
	break;
    }
    (void) FcPStackPop (parse);
}

static void
FcCharacterData (void *userData, const XML_Char *s, int len)
{
    FcConfigParse   *parse = userData;

    if (!parse->pstack)
	return;
    if (!FcStrBufData (&parse->pstack->str, (FcChar8 *) s, len))
	FcConfigMessage (parse, FcSevereError, "out of memory");
}

static void
FcStartDoctypeDecl (void	    *userData,
		    const XML_Char  *doctypeName,
		    const XML_Char  *sysid FC_UNUSED,
		    const XML_Char  *pubid FC_UNUSED,
		    int		    has_internal_subset FC_UNUSED)
{
    FcConfigParse   *parse = userData;

    if (strcmp ((char *) doctypeName, "fontconfig") != 0)
	FcConfigMessage (parse, FcSevereError, "invalid doctype \"%s\"", doctypeName);
}

#ifdef ENABLE_LIBXML2

static void
FcInternalSubsetDecl (void            *userData,
		      const XML_Char  *doctypeName,
		      const XML_Char  *sysid,
		      const XML_Char  *pubid)
{
    FcStartDoctypeDecl (userData, doctypeName, sysid, pubid, 1);
}

static void
FcExternalSubsetDecl (void            *userData,
		      const XML_Char  *doctypeName,
		      const XML_Char  *sysid,
		      const XML_Char  *pubid)
{
    FcStartDoctypeDecl (userData, doctypeName, sysid, pubid, 0);
}

#else /* ENABLE_LIBXML2 */

static void
FcEndDoctypeDecl (void *userData FC_UNUSED)
{
}

#endif /* ENABLE_LIBXML2 */

static int
FcSortCmpStr (const void *a, const void *b)
{
    const FcChar8    *as = *((FcChar8 **) a);
    const FcChar8    *bs = *((FcChar8 **) b);
    return FcStrCmp (as, bs);
}

static FcBool
FcConfigParseAndLoadDir (FcConfig	*config,
			 const FcChar8	*name,
			 const FcChar8	*dir,
			 FcBool		complain,
			 FcBool		load)
{
    DIR		    *d;
    struct dirent   *e;
    FcBool	    ret = FcTrue;
    FcChar8	    *file;
    FcChar8	    *base;
    FcStrSet	    *files;

    d = opendir ((char *) dir);
    if (!d)
    {
	if (complain)
	    FcConfigMessage (0, FcSevereError, "Cannot open config dir \"%s\"",
			     name);
	ret = FcFalse;
	goto bail0;
    }
    /* freed below */
    file = (FcChar8 *) malloc (strlen ((char *) dir) + 1 + FC_MAX_FILE_LEN + 1);
    if (!file)
    {
	ret = FcFalse;
	goto bail1;
    }

    strcpy ((char *) file, (char *) dir);
    strcat ((char *) file, "/");
    base = file + strlen ((char *) file);

    files = FcStrSetCreateEx (FCSS_GROW_BY_64);
    if (!files)
    {
	ret = FcFalse;
	goto bail2;
    }

    if (FcDebug () & FC_DBG_CONFIG)
	printf ("\tScanning config dir %s\n", dir);

    if (load)
	FcConfigAddConfigDir (config, dir);

    while (ret && (e = readdir (d)))
    {
	int d_len;
#define TAIL	    ".conf"
#define TAIL_LEN    5
	/*
	 * Add all files of the form [0-9]*.conf
	 */
	d_len = strlen (e->d_name);
	if ('0' <= e->d_name[0] && e->d_name[0] <= '9' &&
	    d_len > TAIL_LEN &&
	    strcmp (e->d_name + d_len - TAIL_LEN, TAIL) == 0)
	{
	    strcpy ((char *) base, (char *) e->d_name);
	    if (!FcStrSetAdd (files, file))
	    {
		ret = FcFalse;
		goto bail3;
	    }
	}
    }
    if (ret)
    {
	int i;
	qsort (files->strs, files->num, sizeof (FcChar8 *),
	       (int (*)(const void *, const void *)) FcSortCmpStr);
	for (i = 0; ret && i < files->num; i++)
	    ret = _FcConfigParse (config, files->strs[i], complain, load);
    }
bail3:
    FcStrSetDestroy (files);
bail2:
    free (file);
bail1:
    closedir (d);
bail0:
    return ret || !complain;
}

static FcBool
FcConfigParseAndLoadFromMemoryInternal (FcConfig       *config,
					const FcChar8  *filename,
					const FcChar8  *buffer,
					FcBool         complain,
					FcBool         load)
{

    XML_Parser	    p;
    size_t	    len;
    FcConfigParse   parse;
    FcBool	    error = FcTrue;
    FcMatchKind	    k;
    FcPtrListIter   liter;

#ifdef ENABLE_LIBXML2
    xmlSAXHandler   sax;
#else
    void            *buf;
    const FcChar8   *s;
    size_t	    buflen;
#endif

    if (!buffer)
	return FcFalse;
    len = strlen ((const char *) buffer);
    if (FcDebug () & FC_DBG_CONFIG)
	printf ("\t%s config file from %s\n", load ? "Loading" : "Scanning", filename);

#ifdef ENABLE_LIBXML2
    memset(&sax, 0, sizeof(sax));

    sax.internalSubset = FcInternalSubsetDecl;
    sax.externalSubset = FcExternalSubsetDecl;
    sax.startElement = FcStartElement;
    sax.endElement = FcEndElement;
    sax.characters = FcCharacterData;

    p = xmlCreatePushParserCtxt (&sax, &parse, NULL, 0, (const char *) filename);
#else
    p = XML_ParserCreate ("UTF-8");
#endif

    if (!p)
	goto bail1;

    if (!FcConfigParseInit (&parse, filename, config, p, load))
	goto bail2;

#ifndef ENABLE_LIBXML2

    XML_SetUserData (p, &parse);

    XML_SetDoctypeDeclHandler (p, FcStartDoctypeDecl, FcEndDoctypeDecl);
    XML_SetElementHandler (p, FcStartElement, FcEndElement);
    XML_SetCharacterDataHandler (p, FcCharacterData);

#endif /* ENABLE_LIBXML2 */

#ifndef ENABLE_LIBXML2
    s = buffer;
    do {
	buf = XML_GetBuffer (p, BUFSIZ);
	if (!buf)
	{
	    FcConfigMessage (&parse, FcSevereError, "cannot get parse buffer");
	    goto bail3;
	}
	if (len > BUFSIZ)
	{
	    buflen = BUFSIZ;
	    len -= BUFSIZ;
	}
	else
	{
	    buflen = len;
	    len = 0;
	}
	memcpy (buf, s, buflen);
	s = s + buflen;
#endif

#ifdef ENABLE_LIBXML2
	if (xmlParseChunk (p, (const char *)buffer, len, len == 0))
#else
	if (!XML_ParseBuffer (p, buflen, buflen == 0))
#endif
	{
	    FcConfigMessage (&parse, FcSevereError, "%s",
			   XML_ErrorString (XML_GetErrorCode (p)));
	    goto bail3;
	}
#ifndef ENABLE_LIBXML2
    } while (buflen != 0);
#endif
    error = parse.error;
    if (load)
    {
	for (k = FcMatchKindBegin; k < FcMatchKindEnd; k++)
	{
	    FcPtrListIter iter;

	    FcPtrListIterInit (parse.ruleset->subst[k], &iter);
	    if (FcPtrListIterIsValid (parse.ruleset->subst[k], &iter))
	    {
		FcPtrListIterInitAtLast (parse.config->subst[k], &iter);
		FcRuleSetReference (parse.ruleset);
		FcPtrListIterAdd (parse.config->subst[k], &iter, parse.ruleset);
	    }
	}
    }
    FcPtrListIterInitAtLast (parse.config->rulesetList, &liter);
    FcRuleSetReference (parse.ruleset);
    FcPtrListIterAdd (parse.config->rulesetList, &liter, parse.ruleset);
bail3:
    FcConfigCleanup (&parse);
bail2:
    XML_ParserFree (p);
bail1:
    if (error && complain)
    {
	FcConfigMessage (0, FcSevereError, "Cannot %s config file from %s", load ? "load" : "scan", filename);
	return FcFalse;
    }
    if (FcDebug () & FC_DBG_CONFIG)
	printf ("\t%s config file from %s done\n", load ? "Loading" : "Scanning", filename);
    return FcTrue;
}

static FcBool
_FcConfigParse (FcConfig	*config,
		const FcChar8	*name,
		FcBool		complain,
		FcBool		load)
{
    FcChar8	    *filename = NULL, *realfilename = NULL;
    int		    fd;
    int		    len;
    FcStrBuf	    sbuf;
    char            buf[BUFSIZ];
    FcBool	    ret = FcFalse, complain_again = complain;
    FcStrBuf	    reason;

    FcStrBufInit (&reason, NULL, 0);
#ifdef _WIN32
    _ensureWin32GettersReady();
#endif

    filename = FcConfigGetFilename (config, name);
    if (!filename)
    {
	FcStrBufString (&reason, (FcChar8 *)"No such file: ");
	FcStrBufString (&reason, name ? name : (FcChar8 *)"(null)");
	goto bail0;
    }
    realfilename = FcConfigRealFilename (config, name);
    if (!realfilename)
    {
	FcStrBufString (&reason, (FcChar8 *)"No such realfile: ");
	FcStrBufString (&reason, name ? name : (FcChar8 *)"(null)");
	goto bail0;
    }
    if (FcStrSetMember (config->availConfigFiles, realfilename))
    {
        FcStrFree (filename);
	FcStrFree (realfilename);
        return FcTrue;
    }

    if (load)
    {
	if (!FcStrSetAdd (config->configFiles, filename))
	    goto bail0;
    }
    if (!FcStrSetAdd (config->availConfigFiles, realfilename))
	goto bail0;

    if (FcFileIsDir (realfilename))
    {
	ret = FcConfigParseAndLoadDir (config, name, realfilename, complain, load);
	FcStrFree (filename);
	FcStrFree (realfilename);
	return ret;
    }

    FcStrBufInit (&sbuf, NULL, 0);

    fd = FcOpen ((char *) realfilename, O_RDONLY);
    if (fd == -1)
    {
	FcStrBufString (&reason, (FcChar8 *)"Unable to open ");
	FcStrBufString (&reason, realfilename);
	goto bail1;
    }

    do {
	len = read (fd, buf, BUFSIZ);
	if (len < 0)
	{
	    int errno_ = errno;
	    char ebuf[BUFSIZ+1];

#if HAVE_STRERROR_R
	    strerror_r (errno_, ebuf, BUFSIZ);
#elif HAVE_STRERROR
	    char *tmp = strerror (errno_);
	    size_t len = strlen (tmp);
	    memcpy (ebuf, tmp, FC_MIN (BUFSIZ, len));
	    ebuf[FC_MIN (BUFSIZ, len)] = 0;
#else
	    ebuf[0] = 0;
#endif
	    FcConfigMessage (0, FcSevereError, "failed reading config file: %s: %s (errno %d)", realfilename, ebuf, errno_);
	    close (fd);
	    goto bail1;
	}
	FcStrBufData (&sbuf, (const FcChar8 *)buf, len);
    } while (len != 0);
    close (fd);

    ret = FcConfigParseAndLoadFromMemoryInternal (config, filename, FcStrBufDoneStatic (&sbuf), complain, load);
    complain_again = FcFalse; /* no need to reclaim here */
bail1:
    FcStrBufDestroy (&sbuf);
bail0:
    if (filename)
	FcStrFree (filename);
    if (realfilename)
	FcStrFree (realfilename);
    if (!complain)
	return FcTrue;
    if (!ret && complain_again)
    {
	if (name)
	    FcConfigMessage (0, FcSevereError, "Cannot %s config file \"%s\": %s", load ? "load" : "scan", name, FcStrBufDoneStatic (&reason));
	else
	    FcConfigMessage (0, FcSevereError, "Cannot %s default config file: %s", load ? "load" : "scan", FcStrBufDoneStatic (&reason));
	FcStrBufDestroy (&reason);
	return FcFalse;
    }
    FcStrBufDestroy (&reason);
    return ret;
}

FcBool
FcConfigParseOnly (FcConfig		*config,
		   const FcChar8	*name,
		   FcBool		complain)
{
    return _FcConfigParse (config, name, complain, FcFalse);
}

FcBool
FcConfigParseAndLoad (FcConfig	    *config,
		      const FcChar8 *name,
		      FcBool	    complain)
{
    return _FcConfigParse (config, name, complain, FcTrue);
}

FcBool
FcConfigParseAndLoadFromMemory (FcConfig       *config,
				const FcChar8  *buffer,
				FcBool         complain)
{
    return FcConfigParseAndLoadFromMemoryInternal (config, (const FcChar8 *)"memory", buffer, complain, FcTrue);
}

#ifdef _WIN32
static void
_ensureWin32GettersReady()
{
    if (!pGetSystemWindowsDirectory)
    {
        HMODULE hk32 = GetModuleHandleA("kernel32.dll");
        if (!(pGetSystemWindowsDirectory = (pfnGetSystemWindowsDirectory)GetProcAddress(hk32, "GetSystemWindowsDirectoryA")))
            pGetSystemWindowsDirectory = (pfnGetSystemWindowsDirectory)GetWindowsDirectory;
    }
    if (!pSHGetFolderPathA)
    {
        HMODULE hSh = LoadLibraryA("shfolder.dll");
        /* the check is done later, because there is no provided fallback */
        if (hSh)
            pSHGetFolderPathA = (pfnSHGetFolderPathA)GetProcAddress(hSh, "SHGetFolderPathA");
    }
}
#endif // _WIN32

#define __fcxml__
#include "fcaliastail.h"
#undef __fcxml__

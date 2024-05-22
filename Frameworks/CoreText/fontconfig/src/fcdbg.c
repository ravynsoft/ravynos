/*
 * fontconfig/src/fcdbg.c
 *
 * Copyright © 2000 Keith Packard
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
#include <stdio.h>
#include <stdlib.h>

static void
_FcValuePrintFile (FILE *f, const FcValue v)
{
    switch (v.type) {
    case FcTypeUnknown:
	fprintf (f, "<unknown>");
	break;
    case FcTypeVoid:
	fprintf (f, "<void>");
	break;
    case FcTypeInteger:
	fprintf (f, "%d(i)", v.u.i);
	break;
    case FcTypeDouble:
	fprintf (f, "%g(f)", v.u.d);
	break;
    case FcTypeString:
	fprintf (f, "\"%s\"", v.u.s);
	break;
    case FcTypeBool:
	fprintf (f,
		 v.u.b == FcTrue  ? "True" :
		 v.u.b == FcFalse ? "False" :
				    "DontCare");
	break;
    case FcTypeMatrix:
	fprintf (f, "[%g %g; %g %g]", v.u.m->xx, v.u.m->xy, v.u.m->yx, v.u.m->yy);
	break;
    case FcTypeCharSet:	/* XXX */
	if (f == stdout)
	    FcCharSetPrint (v.u.c);
	break;
    case FcTypeLangSet:
	FcLangSetPrint (v.u.l);
	break;
    case FcTypeFTFace:
	fprintf (f, "face");
	break;
    case FcTypeRange:
	fprintf (f, "[%g %g]", v.u.r->begin, v.u.r->end);
	break;
    }
}

void
FcValuePrintFile (FILE *f, const FcValue v)
{
    fprintf (f, " ");
    _FcValuePrintFile (f, v);
}

void
FcValuePrint (const FcValue v)
{
    printf (" ");
    _FcValuePrintFile (stdout, v);
}

void
FcValuePrintWithPosition (const FcValue v, FcBool show_pos_mark)
{
    if (show_pos_mark)
	printf (" [marker] ");
    else
	printf (" ");
    _FcValuePrintFile (stdout, v);
}

static void
FcValueBindingPrint (const FcValueListPtr l)
{
    switch (l->binding) {
    case FcValueBindingWeak:
	printf ("(w)");
	break;
    case FcValueBindingStrong:
	printf ("(s)");
	break;
    case FcValueBindingSame:
	printf ("(=)");
	break;
    default:
	/* shouldn't be reached */
	printf ("(?)");
	break;
    }
}

void
FcValueListPrintWithPosition (FcValueListPtr l, const FcValueListPtr pos)
{
    for (; l != NULL; l = FcValueListNext(l))
    {
	FcValuePrintWithPosition (FcValueCanonicalize (&l->value), pos != NULL && l == pos);
	FcValueBindingPrint (l);
    }
    if (!pos)
	printf (" [marker]");
}

void
FcValueListPrint (FcValueListPtr l)
{
    for (; l != NULL; l = FcValueListNext(l))
    {
	FcValuePrint (FcValueCanonicalize (&l->value));
	FcValueBindingPrint (l);
    }
}

void
FcLangSetPrint (const FcLangSet *ls)
{
    FcStrBuf	buf;
    FcChar8	init_buf[1024];

    FcStrBufInit (&buf, init_buf, sizeof (init_buf));
    if (FcNameUnparseLangSet (&buf, ls) && FcStrBufChar (&buf,'\0'))
       printf ("%s", buf.buf);
    else
       printf ("langset (alloc error)");
    FcStrBufDestroy (&buf);
}

void
FcCharSetPrint (const FcCharSet *c)
{
    int	i, j;
    intptr_t	*leaves = FcCharSetLeaves (c);
    FcChar16	*numbers = FcCharSetNumbers (c);

#if 0
    printf ("CharSet  0x%x\n", (intptr_t) c);
    printf ("Leaves:  +%d = 0x%x\n", c->leaves_offset, (intptr_t) leaves);
    printf ("Numbers: +%d = 0x%x\n", c->numbers_offset, (intptr_t) numbers);

    for (i = 0; i < c->num; i++)
    {
	printf ("Page %d: %04x +%d = 0x%x\n",
		i, numbers[i], leaves[i],
		(intptr_t) FcOffsetToPtr (leaves, leaves[i], FcCharLeaf));
    }
#endif
		
    printf ("\n");
    for (i = 0; i < c->num; i++)
    {
	intptr_t	leaf_offset = leaves[i];
	FcCharLeaf	*leaf = FcOffsetToPtr (leaves, leaf_offset, FcCharLeaf);
	
	printf ("\t");
	printf ("%04x:", numbers[i]);
	for (j = 0; j < 256/32; j++)
	    printf (" %08x", leaf->map[j]);
	printf ("\n");
    }
}

void
FcPatternPrint (const FcPattern *p)
{
    FcPatternIter iter;

    if (!p)
    {
	printf ("Null pattern\n");
	return;
    }
    printf ("Pattern has %d elts (size %d)\n", FcPatternObjectCount (p), p->size);
    FcPatternIterStart (p, &iter);
    do
    {
	printf ("\t%s:", FcPatternIterGetObject (p, &iter));
	FcValueListPrint (FcPatternIterGetValues (p, &iter));
	printf ("\n");
    } while (FcPatternIterNext (p, &iter));
    printf ("\n");
}

#define FcOpFlagsPrint(_o_)		\
    {					\
	int f = FC_OP_GET_FLAGS (_o_);	\
	if (f & FcOpFlagIgnoreBlanks)	\
	    printf ("(ignore blanks)");	\
    }

void
FcPatternPrint2 (FcPattern         *pp1,
		 FcPattern         *pp2,
		 const FcObjectSet *os)
{
    int i, j, k, pos;
    FcPatternElt *e1, *e2;
    FcPattern *p1, *p2;

    if (os)
    {
	p1 = FcPatternFilter (pp1, os);
	p2 = FcPatternFilter (pp2, os);
    }
    else
    {
	p1 = pp1;
	p2 = pp2;
    }
    printf ("Pattern has %d elts (size %d), %d elts (size %d)\n",
	    p1->num, p1->size, p2->num, p2->size);
    for (i = 0, j = 0; i < p1->num; i++)
    {
	e1 = &FcPatternElts(p1)[i];
	e2 = &FcPatternElts(p2)[j];
	if (!e2 || e1->object != e2->object)
	{
	    pos = FcPatternPosition (p2, FcObjectName (e1->object));
	    if (pos >= 0)
	    {
		for (k = j; k < pos; k++)
		{
		    e2 = &FcPatternElts(p2)[k];
		    printf ("\t%s: (None) -> ", FcObjectName (e2->object));
		    FcValueListPrint (FcPatternEltValues (e2));
		    printf ("\n");
		}
		j = pos;
		goto cont;
	    }
	    else
	    {
		printf ("\t%s:", FcObjectName (e1->object));
		FcValueListPrint (FcPatternEltValues (e1));
		printf (" -> (None)\n");
	    }
	}
	else
	{
	cont:
	    printf ("\t%s:", FcObjectName (e1->object));
	    FcValueListPrint (FcPatternEltValues (e1));
	    printf (" -> ");
	    e2 = &FcPatternElts(p2)[j];
	    FcValueListPrint (FcPatternEltValues (e2));
	    printf ("\n");
	    j++;
	}
    }
    if (j < p2->num)
    {
	for (k = j; k < p2->num; k++)
	{
	    e2 = &FcPatternElts(p2)[k];
	    if (FcObjectName (e2->object))
	    {
		printf ("\t%s: (None) -> ", FcObjectName (e2->object));
		FcValueListPrint (FcPatternEltValues (e2));
		printf ("\n");
	    }
	}
    }
    if (p1 != pp1)
	FcPatternDestroy (p1);
    if (p2 != pp2)
	FcPatternDestroy (p2);
}

void
FcOpPrint (FcOp op_)
{
    FcOp op = FC_OP_GET_OP (op_);

    switch (op) {
    case FcOpInteger: printf ("Integer"); break;
    case FcOpDouble: printf ("Double"); break;
    case FcOpString: printf ("String"); break;
    case FcOpMatrix: printf ("Matrix"); break;
    case FcOpRange: printf ("Range"); break;
    case FcOpBool: printf ("Bool"); break;
    case FcOpCharSet: printf ("CharSet"); break;
    case FcOpLangSet: printf ("LangSet"); break;
    case FcOpField: printf ("Field"); break;
    case FcOpConst: printf ("Const"); break;
    case FcOpAssign: printf ("Assign"); break;
    case FcOpAssignReplace: printf ("AssignReplace"); break;
    case FcOpPrepend: printf ("Prepend"); break;
    case FcOpPrependFirst: printf ("PrependFirst"); break;
    case FcOpAppend: printf ("Append"); break;
    case FcOpAppendLast: printf ("AppendLast"); break;
    case FcOpDelete: printf ("Delete"); break;
    case FcOpDeleteAll: printf ("DeleteAll"); break;
    case FcOpQuest: printf ("Quest"); break;
    case FcOpOr: printf ("Or"); break;
    case FcOpAnd: printf ("And"); break;
    case FcOpEqual: printf ("Equal"); FcOpFlagsPrint (op_); break;
    case FcOpNotEqual: printf ("NotEqual"); FcOpFlagsPrint (op_); break;
    case FcOpLess: printf ("Less"); break;
    case FcOpLessEqual: printf ("LessEqual"); break;
    case FcOpMore: printf ("More"); break;
    case FcOpMoreEqual: printf ("MoreEqual"); break;
    case FcOpContains: printf ("Contains"); break;
    case FcOpNotContains: printf ("NotContains"); break;
    case FcOpPlus: printf ("Plus"); break;
    case FcOpMinus: printf ("Minus"); break;
    case FcOpTimes: printf ("Times"); break;
    case FcOpDivide: printf ("Divide"); break;
    case FcOpNot: printf ("Not"); break;
    case FcOpNil: printf ("Nil"); break;
    case FcOpComma: printf ("Comma"); break;
    case FcOpFloor: printf ("Floor"); break;
    case FcOpCeil: printf ("Ceil"); break;
    case FcOpRound: printf ("Round"); break;
    case FcOpTrunc: printf ("Trunc"); break;
    case FcOpListing: printf ("Listing"); FcOpFlagsPrint (op_); break;
    case FcOpInvalid: printf ("Invalid"); break;
    }
}

void
FcExprPrint (const FcExpr *expr)
{
    if (!expr) printf ("none");
    else switch (FC_OP_GET_OP (expr->op)) {
    case FcOpInteger: printf ("%d", expr->u.ival); break;
    case FcOpDouble: printf ("%g", expr->u.dval); break;
    case FcOpString: printf ("\"%s\"", expr->u.sval); break;
    case FcOpMatrix:
	printf ("[");
	FcExprPrint (expr->u.mexpr->xx);
	printf (" ");
	FcExprPrint (expr->u.mexpr->xy);
	printf ("; ");
	FcExprPrint (expr->u.mexpr->yx);
	printf (" ");
	FcExprPrint (expr->u.mexpr->yy);
	printf ("]");
	break;
    case FcOpRange:
	printf ("(%g, %g)", expr->u.rval->begin, expr->u.rval->end);
	break;
    case FcOpBool: printf ("%s", expr->u.bval ? "true" : "false"); break;
    case FcOpCharSet: printf ("charset\n"); break;
    case FcOpLangSet:
	printf ("langset:");
	FcLangSetPrint(expr->u.lval);
	printf ("\n");
	break;
    case FcOpNil: printf ("nil\n"); break;
    case FcOpField: printf ("%s ", FcObjectName(expr->u.name.object));
      switch ((int) expr->u.name.kind) {
      case FcMatchPattern:
	  printf ("(pattern) ");
	  break;
      case FcMatchFont:
	  printf ("(font) ");
	  break;
      }
      break;
    case FcOpConst: printf ("%s", expr->u.constant); break;
    case FcOpQuest:
	FcExprPrint (expr->u.tree.left);
	printf (" quest ");
	FcExprPrint (expr->u.tree.right->u.tree.left);
	printf (" colon ");
	FcExprPrint (expr->u.tree.right->u.tree.right);
	break;
    case FcOpAssign:
    case FcOpAssignReplace:
    case FcOpPrependFirst:
    case FcOpPrepend:
    case FcOpAppend:
    case FcOpAppendLast:
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
    case FcOpComma:
	FcExprPrint (expr->u.tree.left);
	printf (" ");
	switch (FC_OP_GET_OP (expr->op)) {
	case FcOpAssign: printf ("Assign"); break;
	case FcOpAssignReplace: printf ("AssignReplace"); break;
	case FcOpPrependFirst: printf ("PrependFirst"); break;
	case FcOpPrepend: printf ("Prepend"); break;
	case FcOpAppend: printf ("Append"); break;
	case FcOpAppendLast: printf ("AppendLast"); break;
	case FcOpOr: printf ("Or"); break;
	case FcOpAnd: printf ("And"); break;
	case FcOpEqual: printf ("Equal"); FcOpFlagsPrint (expr->op); break;
	case FcOpNotEqual: printf ("NotEqual"); FcOpFlagsPrint (expr->op); break;
	case FcOpLess: printf ("Less"); break;
	case FcOpLessEqual: printf ("LessEqual"); break;
	case FcOpMore: printf ("More"); break;
	case FcOpMoreEqual: printf ("MoreEqual"); break;
	case FcOpContains: printf ("Contains"); break;
	case FcOpListing: printf ("Listing"); FcOpFlagsPrint (expr->op); break;
	case FcOpNotContains: printf ("NotContains"); break;
	case FcOpPlus: printf ("Plus"); break;
	case FcOpMinus: printf ("Minus"); break;
	case FcOpTimes: printf ("Times"); break;
	case FcOpDivide: printf ("Divide"); break;
	case FcOpComma: printf ("Comma"); break;
	default: break;
	}
	printf (" ");
	FcExprPrint (expr->u.tree.right);
	break;
    case FcOpNot:
	printf ("Not ");
	FcExprPrint (expr->u.tree.left);
	break;
    case FcOpFloor:
	printf ("Floor ");
	FcExprPrint (expr->u.tree.left);
	break;
    case FcOpCeil:
	printf ("Ceil ");
	FcExprPrint (expr->u.tree.left);
	break;
    case FcOpRound:
	printf ("Round ");
	FcExprPrint (expr->u.tree.left);
	break;
    case FcOpTrunc:
	printf ("Trunc ");
	FcExprPrint (expr->u.tree.left);
	break;
    case FcOpInvalid: printf ("Invalid"); break;
    }
}

void
FcTestPrint (const FcTest *test)
{
    switch (test->kind) {
    case FcMatchPattern:
	printf ("pattern ");
	break;
    case FcMatchFont:
	printf ("font ");
	break;
    case FcMatchScan:
	printf ("scan ");
	break;
    case FcMatchKindEnd:
	/* shouldn't be reached */
	return;
    }
    switch (test->qual) {
    case FcQualAny:
	printf ("any ");
	break;
    case FcQualAll:
	printf ("all ");
	break;
    case FcQualFirst:
	printf ("first ");
	break;
    case FcQualNotFirst:
	printf ("not_first ");
	break;
    }
    printf ("%s ", FcObjectName (test->object));
    FcOpPrint (test->op);
    printf (" ");
    FcExprPrint (test->expr);
    printf ("\n");
}

void
FcEditPrint (const FcEdit *edit)
{
    printf ("Edit %s ", FcObjectName (edit->object));
    FcOpPrint (edit->op);
    printf (" ");
    FcExprPrint (edit->expr);
}

void
FcRulePrint (const FcRule *rule)
{
    FcRuleType last_type = FcRuleUnknown;
    const FcRule *r;

    for (r = rule; r; r = r->next)
    {
	if (last_type != r->type)
	{
	    switch (r->type) {
	    case FcRuleTest:
		printf ("[test]\n");
		break;
	    case FcRuleEdit:
		printf ("[edit]\n");
		break;
	    default:
		break;
	    }
	    last_type = r->type;
	}
	printf ("\t");
	switch (r->type) {
	case FcRuleTest:
	    FcTestPrint (r->u.test);
	    break;
	case FcRuleEdit:
	    FcEditPrint (r->u.edit);
	    printf (";\n");
	    break;
	default:
	    break;
	}
    }
    printf ("\n");
}

void
FcFontSetPrint (const FcFontSet *s)
{
    int	    i;

    printf ("FontSet %d of %d\n", s->nfont, s->sfont);
    for (i = 0; i < s->nfont; i++)
    {
	printf ("Font %d ", i);
	FcPatternPrint (s->fonts[i]);
    }
}

int FcDebugVal;

void
FcInitDebug (void)
{
    if (!FcDebugVal) {
	char    *e;

	e = getenv ("FC_DEBUG");
	if (e)
	{
	    printf ("FC_DEBUG=%s\n", e);
	    FcDebugVal = atoi (e);
	    if (FcDebugVal < 0)
		FcDebugVal = 0;
	}
    }
}
#define __fcdbg__
#include "fcaliastail.h"
#undef __fcdbg__

/*    perly.c
 *
 *    Copyright (c) 2004, 2005, 2006, 2007, 2008,
 *    2009, 2010, 2011 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 * 
 *    Note that this file was originally generated as an output from
 *    GNU bison version 1.875, but now the code is statically maintained
 *    and edited; the bits that are dependent on perly.y are now
 *    #included from the files perly.tab and perly.act.
 *
 *    Here is an important copyright statement from the original, generated
 *    file:
 *
 *	As a special exception, when this file is copied by Bison into a
 *	Bison output file, you may use that output file without
 *	restriction.  This special exception was added by the Free
 *	Software Foundation in version 1.24 of Bison.
 *
 */

#include "EXTERN.h"
#define PERL_IN_PERLY_C
#include "perl.h"
#include "feature.h"
#include "keywords.h"

typedef unsigned char yytype_uint8;
typedef signed char yytype_int8;
typedef unsigned short int yytype_uint16;
typedef short int yytype_int16;
typedef signed char yysigned_char;

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#define YYINITDEPTH 200

#ifdef YYDEBUG
#  undef YYDEBUG
#endif
#ifdef DEBUGGING
#  define YYDEBUG 1
#else
#  define YYDEBUG 0
#endif

#ifndef YY_NULL
# define YY_NULL 0
#endif

#ifndef YY_NULLPTR
# define YY_NULLPTR NULL
#endif

#ifndef YY_CAST
# ifdef __cplusplus
#  define YY_CAST(Type, Val) static_cast<Type> (Val)
#  define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
# else
#  define YY_CAST(Type, Val) ((Type) (Val))
#  define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* contains all the parser state tables; auto-generated from perly.y */
#include "perly.tab"

# define YYSIZE_T size_t

#define YYEOF		0
#define YYTERROR	1

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Enable debugging if requested.  */
#ifdef DEBUGGING

#  define yydebug (DEBUG_p_TEST)

#  define YYFPRINTF PerlIO_printf

#  define YYDPRINTF(Args)			\
do {						\
    if (yydebug)				\
        YYFPRINTF Args;				\
} while (0)

#  define YYDSYMPRINTF(Title, Token, Value)			\
do {								\
    if (yydebug) {						\
        YYFPRINTF (Perl_debug_log, "%s ", Title);		\
        yysymprint (aTHX_ Perl_debug_log,  Token, Value);	\
        YYFPRINTF (Perl_debug_log, "\n");			\
    }								\
} while (0)

/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yysymprint(pTHX_ PerlIO * const yyoutput, int yytype, const YYSTYPE * const yyvaluep)
{
    PERL_UNUSED_CONTEXT;
    if (yytype < YYNTOKENS) {
        YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
#   ifdef YYPRINT
        YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
#   else
        YYFPRINTF (yyoutput, "0x%" UVxf, (UV)yyvaluep->ival);
#   endif
    }
    else
        YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

    YYFPRINTF (yyoutput, ")");
}


/*  yy_stack_print()
 *  print the top 8 items on the parse stack.
 */

static void
yy_stack_print (pTHX_ const yy_parser *parser)
{
    const yy_stack_frame *ps, *min;

    min = parser->ps - 8 + 1;
    if (min <= parser->stack)
        min = parser->stack + 1;

    PerlIO_printf(Perl_debug_log, "\nindex:");
    for (ps = min; ps <= parser->ps; ps++)
        PerlIO_printf(Perl_debug_log, " %8d", (int)(ps - parser->stack));

    PerlIO_printf(Perl_debug_log, "\nstate:");
    for (ps = min; ps <= parser->ps; ps++)
        PerlIO_printf(Perl_debug_log, " %8d", ps->state);

    PerlIO_printf(Perl_debug_log, "\ntoken:");
    for (ps = min; ps <= parser->ps; ps++)
        PerlIO_printf(Perl_debug_log, " %8.8s", ps->name);

    PerlIO_printf(Perl_debug_log, "\nvalue:");
    for (ps = min; ps <= parser->ps; ps++) {
        switch (yy_type_tab[yystos[ps->state]]) {
        case toketype_opval:
            PerlIO_printf(Perl_debug_log, " %8.8s",
                  ps->val.opval
                    ? PL_op_name[ps->val.opval->op_type]
                    : "(Nullop)"
            );
            break;
        case toketype_ival:
            PerlIO_printf(Perl_debug_log, " %8" IVdf, (IV)ps->val.ival);
            break;
        default:
            PerlIO_printf(Perl_debug_log, " %8" UVxf, (UV)ps->val.ival);
        }
    }
    PerlIO_printf(Perl_debug_log, "\n\n");
}

#  define YY_STACK_PRINT(parser)	\
do {					\
    if (yydebug && DEBUG_v_TEST)	\
        yy_stack_print (aTHX_ parser);	\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (pTHX_ int yyrule)
{
    int yyi;
    const unsigned int yylineno = yyrline[yyrule];
    YYFPRINTF (Perl_debug_log, "Reducing stack by rule %d (line %u), ",
                          yyrule - 1, yylineno);
    /* Print the symbols being reduced, and their result.  */
#if PERL_BISON_VERSION >= 30000 /* 3.0+ */
    for (yyi = 0; yyi < yyr2[yyrule]; yyi++)
        YYFPRINTF (Perl_debug_log, "%s ",
            yytname [yystos[(PL_parser->ps)[yyi + 1 - yyr2[yyrule]].state]]);
#else
    for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
        YYFPRINTF (Perl_debug_log, "%s ", yytname [yyrhs[yyi]]);
#endif
    YYFPRINTF (Perl_debug_log, "-> %s\n", yytname [yyr1[yyrule]]);
}

#  define YY_REDUCE_PRINT(Rule)		\
do {					\
    if (yydebug)			\
        yy_reduce_print (aTHX_ Rule);		\
} while (0)

#else /* !DEBUGGING */
#  define YYDPRINTF(Args)
#  define YYDSYMPRINTF(Title, Token, Value)
#  define YY_STACK_PRINT(parser)
#  define YY_REDUCE_PRINT(Rule)
#endif /* !DEBUGGING */

/* called during cleanup (via SAVEDESTRUCTOR_X) to free any items on the
 * parse stack, thus avoiding leaks if we die  */

static void
S_clear_yystack(pTHX_  const yy_parser *parser)
{
    yy_stack_frame *ps     = parser->ps;
    int i = 0;

    if (!parser->stack)
        return;

    YYDPRINTF ((Perl_debug_log, "clearing the parse stack\n"));

    for (i=0; i< parser->yylen; i++) {
        SvREFCNT_dec(ps[-i].compcv);
    }
    ps -= parser->yylen;

    /* now free whole the stack, including the just-reduced ops */

    while (ps > parser->stack) {
        LEAVE_SCOPE(ps->savestack_ix);
        if (yy_type_tab[yystos[ps->state]] == toketype_opval
            && ps->val.opval)
        {
            if (ps->compcv && (ps->compcv != PL_compcv)) {
                PL_compcv = ps->compcv;
                PAD_SET_CUR_NOSAVE(CvPADLIST(PL_compcv), 1);
                PL_comppad_name = PadlistNAMES(CvPADLIST(PL_compcv));
            }
            YYDPRINTF ((Perl_debug_log, "(freeing op)\n"));
            op_free(ps->val.opval);
        }
        SvREFCNT_dec(ps->compcv);
        ps--;
    }

    Safefree(parser->stack);
}


/*----------.
| yyparse.  |
`----------*/

int
Perl_yyparse (pTHX_ int gramtype)
{
    int yystate;
    int yyn;
    int yyresult;

    /* Lookahead token as an internal (translated) token number.  */
    int yytoken = 0;

    yy_parser *parser;	    /* the parser object */
    yy_stack_frame  *ps;   /* current parser stack frame */

#define YYPOPSTACK   parser->ps = --ps
#define YYPUSHSTACK  parser->ps = ++ps

    /* The variable used to return semantic value and location from the
          action routines: ie $$.  */
    YYSTYPE yyval;

    YYDPRINTF ((Perl_debug_log, "Starting parse\n"));

    parser = PL_parser;

    ENTER;  /* force parser state cleanup/restoration before we return */
    SAVEPPTR(parser->yylval.pval);
    SAVEINT(parser->yychar);
    SAVEINT(parser->yyerrstatus);
    SAVEINT(parser->yylen);
    SAVEVPTR(parser->stack);
    SAVEVPTR(parser->stack_max1);
    SAVEVPTR(parser->ps);

    /* initialise state for this parse */
    parser->yychar = gramtype;
    yytoken = YYTRANSLATE((int)NATIVE_TO_UNI(parser->yychar));

    parser->yyerrstatus = 0;
    parser->yylen = 0;
    Newx(parser->stack, YYINITDEPTH, yy_stack_frame);
    parser->stack_max1 = parser->stack + YYINITDEPTH - 1;
    ps = parser->ps = parser->stack;
    ps->state = 0;
    SAVEDESTRUCTOR_X(S_clear_yystack, parser);

    while (1) {
        /* main loop: shift some tokens, then reduce when possible */

        while (1) {
            /* shift a token, or quit when it's possible to reduce */

            yystate = ps->state;

            YYDPRINTF ((Perl_debug_log, "Entering state %d\n", yystate));

            parser->yylen = 0;

            /* Grow the stack? We always leave 1 spare slot, in case of a
             * '' -> 'foo' reduction.
             * Note that stack_max1 points to the (top-1)th allocated stack
             * element to make this check faster */

            if (ps >= parser->stack_max1) {
                Size_t pos = ps - parser->stack;
                Size_t newsize = 2 * (parser->stack_max1 + 2 - parser->stack);
                /* this will croak on insufficient memory */
                Renew(parser->stack, newsize, yy_stack_frame);
                ps = parser->ps = parser->stack + pos;
                parser->stack_max1 = parser->stack + newsize - 1;

                YYDPRINTF((Perl_debug_log,
                                "parser stack size increased to %lu frames\n",
                                (unsigned long int)newsize));
            }

            /* Do appropriate processing given the current state. Read a
             * lookahead token if we need one and don't already have one.
             * */

            /* First try to decide what to do without reference to
             * lookahead token. */

            yyn = yypact[yystate];
            if (yyn == YYPACT_NINF)
                goto yydefault;

            /* Not known => get a lookahead token if don't already have
             * one.  YYCHAR is either YYEMPTY or YYEOF or a valid
             * lookahead symbol. */

            if (parser->yychar == YYEMPTY) {
                YYDPRINTF ((Perl_debug_log, "Reading a token:\n"));
                parser->yychar = yylex();
                assert(parser->yychar >= 0);
                if (parser->yychar == YYEOF) {
                    YYDPRINTF ((Perl_debug_log, "Now at end of input.\n"));
                }
                /* perly.tab is shipped based on an ASCII system, so need
                 * to index it with characters translated to ASCII.
                 * Although it's not designed for this purpose, we can use
                 * NATIVE_TO_UNI here.  It returns its argument on ASCII
                 * platforms, and on EBCDIC translates native to ascii in
                 * the 0-255 range, leaving every other possible input
                 * unchanged.  This jibes with yylex() returning some bare
                 * characters in that range, but all tokens it returns are
                 * either 0, or above 255.  There could be a problem if NULs
                 * weren't 0, or were ever returned as raw chars by yylex() */
                yytoken = YYTRANSLATE((int)NATIVE_TO_UNI(parser->yychar));
            }

            /* make sure no-one's changed yychar since the last call to yylex */
            assert(yytoken == YYTRANSLATE((int)NATIVE_TO_UNI(parser->yychar)));
            YYDSYMPRINTF("lookahead token is", yytoken, &parser->yylval);


            /* If the proper action on seeing token YYTOKEN is to reduce or to
             * detect an error, take that action.
             * Casting yyn to unsigned allows a >=0 test to be included as
             * part of the  <=YYLAST test for speed */
            yyn += yytoken;
            if ((unsigned int)yyn > YYLAST || yycheck[yyn] != yytoken) {
              yydefault:
                /* do the default action for the current state. */
                yyn = yydefact[yystate];
                if (yyn == 0)
                    goto yyerrlab;
                break; /* time to reduce */
            }

            yyn = yytable[yyn];
            if (yyn <= 0) {
                if (yyn == 0 || yyn == YYTABLE_NINF)
                    goto yyerrlab;
                yyn = -yyn;
                break; /* time to reduce */
            }

            if (yyn == YYFINAL)
                YYACCEPT;

            /* Shift the lookahead token.  */
            YYDPRINTF ((Perl_debug_log, "Shifting token %s, ", yytname[yytoken]));

            /* Discard the token being shifted unless it is eof.  */
            if (parser->yychar != YYEOF)
                parser->yychar = YYEMPTY;

            YYPUSHSTACK;
            ps->state   = yyn;
            ps->val     = parser->yylval;
            ps->compcv  = (CV*)SvREFCNT_inc(PL_compcv);
            ps->savestack_ix = PL_savestack_ix;
#ifdef DEBUGGING
            ps->name    = (const char *)(yytname[yytoken]);
#endif

            /* Count tokens shifted since error; after three, turn off error
                  status.  */
            if (parser->yyerrstatus)
                parser->yyerrstatus--;

        }

        /* Do a reduction */

        /* yyn is the number of a rule to reduce with.  */
        parser->yylen = yyr2[yyn];

        /* If YYLEN is nonzero, implement the default value of the action:
          "$$ = $1".

          Otherwise, the following line sets YYVAL to garbage.
          This behavior is undocumented and Bison
          users should not rely upon it.  Assigning to YYVAL
          unconditionally makes the parser a bit smaller, and it avoids a
          GCC warning that YYVAL may be used uninitialized.  */
        yyval = ps[1-parser->yylen].val;

        YY_STACK_PRINT(parser);
        YY_REDUCE_PRINT (yyn);

        switch (yyn) {

    /* contains all the rule actions; auto-generated from perly.y */
#include "perly.act"

        }

        {
            int i;
            for (i=0; i< parser->yylen; i++) {
                SvREFCNT_dec(ps[-i].compcv);
            }
        }

        parser->ps = ps -= (parser->yylen-1);

        /* Now shift the result of the reduction.  Determine what state
              that goes to, based on the state we popped back to and the rule
              number reduced by.  */

        ps->val     = yyval;
        ps->compcv  = (CV*)SvREFCNT_inc(PL_compcv);
        ps->savestack_ix = PL_savestack_ix;
#ifdef DEBUGGING
        ps->name    = (const char *)(yytname [yyr1[yyn]]);
#endif

        yyn = yyr1[yyn];

        yystate = yypgoto[yyn - YYNTOKENS] + ps[-1].state;
        if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == ps[-1].state)
            yystate = yytable[yystate];
        else
            yystate = yydefgoto[yyn - YYNTOKENS];
        ps->state = yystate;

        continue;


      /*------------------------------------.
      | yyerrlab -- here on detecting error |
      `------------------------------------*/
      yyerrlab:
        /* If not already recovering from an error, report this error.  */
        if (!parser->yyerrstatus) {
            yyerror("syntax error");
            yyquit();
        }


        if (parser->yyerrstatus == 3) {
            /* If just tried and failed to reuse lookahead token after an
                  error, discard it.  */

            /* Return failure if at end of input.  */
            if (parser->yychar == YYEOF) {
                /* Pop the error token.  */
                SvREFCNT_dec(ps->compcv);
                YYPOPSTACK;
                /* Pop the rest of the stack.  */
                while (ps > parser->stack) {
                    YYDSYMPRINTF ("Error: popping", yystos[ps->state], &ps->val);
                    LEAVE_SCOPE(ps->savestack_ix);
                    if (yy_type_tab[yystos[ps->state]] == toketype_opval
                            && ps->val.opval)
                    {
                        YYDPRINTF ((Perl_debug_log, "(freeing op)\n"));
                        if (ps->compcv != PL_compcv) {
                            PL_compcv = ps->compcv;
                            PAD_SET_CUR_NOSAVE(CvPADLIST(PL_compcv), 1);
                        }
                        op_free(ps->val.opval);
                    }
                    SvREFCNT_dec(ps->compcv);
                    YYPOPSTACK;
                }
                YYABORT;
            }

            YYDSYMPRINTF ("Error: discarding", yytoken, &parser->yylval);
            parser->yychar = YYEMPTY;

        }

        /* Else will try to reuse lookahead token after shifting the error
              token.  */
        goto yyerrlab1;


      /*----------------------------------------------------.
      | yyerrlab1 -- error raised explicitly by an action.  |
      `----------------------------------------------------*/
      yyerrlab1:
        parser->yyerrstatus = 3;	/* Each real token shifted decrements this.  */

        for (;;) {
            yyn = yypact[yystate];
            if (yyn != YYPACT_NINF) {
                yyn += YYTERROR;
                if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR) {
                    yyn = yytable[yyn];
                    if (0 < yyn)
                        break;
                }
            }

            /* Pop the current state because it cannot handle the error token.  */
            if (ps == parser->stack)
                YYABORT;

            YYDSYMPRINTF ("Error: popping", yystos[ps->state], &ps->val);
            LEAVE_SCOPE(ps->savestack_ix);
            if (yy_type_tab[yystos[ps->state]] == toketype_opval && ps->val.opval) {
                YYDPRINTF ((Perl_debug_log, "(freeing op)\n"));
                if (ps->compcv != PL_compcv) {
                    PL_compcv = ps->compcv;
                    PAD_SET_CUR_NOSAVE(CvPADLIST(PL_compcv), 1);
                }
                op_free(ps->val.opval);
            }
            SvREFCNT_dec(ps->compcv);
            YYPOPSTACK;
            yystate = ps->state;

            YY_STACK_PRINT(parser);
        }

        if (yyn == YYFINAL)
            YYACCEPT;

        YYDPRINTF ((Perl_debug_log, "Shifting error token, "));

        YYPUSHSTACK;
        ps->state   = yyn;
        ps->val     = parser->yylval;
        ps->compcv  = (CV*)SvREFCNT_inc(PL_compcv);
        ps->savestack_ix = PL_savestack_ix;
#ifdef DEBUGGING
        ps->name    ="<err>";
#endif

    } /* main loop */


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    for (ps=parser->ps; ps > parser->stack; ps--) {
        SvREFCNT_dec(ps->compcv);
    }
    parser->ps = parser->stack; /* disable cleanup */
    goto yyreturn;

  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    LEAVE;	/* force parser stack cleanup before we return */
    return yyresult;
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */

/*    perly.y
 *
 *    Copyright (c) 1991-2002, 2003, 2004, 2005, 2006 Larry Wall
 *    Copyright (c) 2007, 2008, 2009, 2010, 2011 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'I see,' laughed Strider.  'I look foul and feel fair.  Is that it?
 *  All that is gold does not glitter, not all those who wander are lost.'
 *
 *     [p.171 of _The Lord of the Rings_, I/x: "Strider"]
 */

/*
 * This file holds the grammar for the Perl language. If edited, you need
 * to run regen_perly.pl, which re-creates the files perly.h, perly.tab
 * and perly.act which are derived from this.
 *
 * The main job of this grammar is to call the various newFOO()
 * functions in op.c to build a syntax tree of OP structs.
 * It relies on the lexer in toke.c to do the tokenizing.
 *
 * Note: due to the way that the cleanup code works WRT to freeing ops on
 * the parse stack, it is dangerous to assign to the $n variables within
 * an action.
 */

/*  Make the parser re-entrant. */

%define api.pure

%start grammar

%union {
    I32	ival; /* __DEFAULT__ (marker for regen_perly.pl;
				must always be 1st union member) */
    void *pval;
    OP *opval;
    GV *gvval;
}

%token <ival> GRAMPROG GRAMEXPR GRAMBLOCK GRAMBARESTMT GRAMFULLSTMT GRAMSTMTSEQ GRAMSUBSIGNATURE

/* Tokens emitted by toke.c for simple punctiation characters - &, {, }, etc... */
%token <ival> PERLY_AMPERSAND
%token <ival> PERLY_BRACE_OPEN
%token <ival> PERLY_BRACE_CLOSE
%token <ival> PERLY_BRACKET_OPEN
%token <ival> PERLY_BRACKET_CLOSE
%token <ival> PERLY_COMMA
%token <ival> PERLY_DOLLAR
%token <ival> PERLY_DOT
%token <ival> PERLY_EQUAL_SIGN
%token <ival> PERLY_MINUS
%token <ival> PERLY_PERCENT_SIGN
%token <ival> PERLY_PLUS
%token <ival> PERLY_SEMICOLON
%token <ival> PERLY_SLASH
%token <ival> PERLY_SNAIL
%token <ival> PERLY_STAR

/* Tokens emitted by toke.c on simple keywords */
%token <ival> KW_FORMAT KW_PACKAGE KW_CLASS
%token <ival> KW_LOCAL KW_MY KW_FIELD
%token <ival> KW_IF KW_ELSE KW_ELSIF KW_UNLESS
%token <ival> KW_FOR KW_UNTIL KW_WHILE KW_CONTINUE
%token <ival> KW_GIVEN KW_WHEN KW_DEFAULT
%token <ival> KW_TRY KW_CATCH KW_FINALLY KW_DEFER
%token <ival> KW_REQUIRE KW_DO

/* The 'use' and 'no' keywords both emit this */
%token <ival> KW_USE_or_NO

/* The 'sub' keyword is a bit special; four different tokens depending on
 *   named-vs-anon, and whether signatures are in effect */
%token <ival> KW_SUB_named KW_SUB_named_sig KW_SUB_anon KW_SUB_anon_sig
%token <ival> KW_METHOD_named KW_METHOD_anon

/* Tokens emitted in other situations */
%token <opval> BAREWORD METHCALL0 METHCALL THING PMFUNC PRIVATEREF QWLIST
%token <opval> FUNC0OP FUNC0SUB UNIOPSUB LSTOPSUB
%token <opval> PLUGEXPR PLUGSTMT
%token <opval> LABEL
%token <ival> LOOPEX DOTDOT YADAYADA
%token <ival> FUNC0 FUNC1 FUNC UNIOP LSTOP
%token <ival> POWOP MULOP ADDOP
%token <ival> DOLSHARP HASHBRACK NOAMP
%token <ival> COLONATTR FORMLBRACK FORMRBRACK
%token <ival> SUBLEXSTART SUBLEXEND
%token <ival> PHASER

%type <ival> grammar remember mremember
%type <ival>  startsub startanonsub startanonmethod startformsub

%type <ival> mintro

%type <ival>  sigsub_or_method_named
%type <opval> stmtseq fullstmt labfullstmt barestmt block mblock else finally
%type <opval> expr term subscripted scalar ary hsh arylen star amper sideff
%type <opval> condition
%type <opval> catch_paren
%type <opval> empty
%type <opval> sliceme kvslice gelem
%type <opval> listexpr nexpr texpr iexpr mexpr mnexpr
%type <opval> optlistexpr optexpr optrepl indirob listop methodname
%type <opval> formname subname proto cont my_scalar my_var
%type <opval> list_of_scalars my_list_of_scalars refgen_topic formblock
%type <opval> subattrlist myattrlist myattrterm myterm
%type <pval>  fieldvar /* pval is PADNAME */
%type <opval> optfieldattrlist fielddecl
%type <opval> termbinop termunop anonymous termdo
%type <opval> termrelop relopchain termeqop eqopchain
%type <ival>  sigslurpsigil
%type <opval> sigvarname sigdefault sigscalarelem sigslurpelem
%type <opval> sigelem siglist optsiglist subsigguts subsignature optsubsignature
%type <opval> subbody optsubbody sigsubbody optsigsubbody
%type <opval> formstmtseq formline formarg

%nonassoc <ival> PREC_LOW
%nonassoc LOOPEX

%nonassoc <pval> PLUGIN_LOW_OP
%left <ival> OROP <pval> PLUGIN_LOGICAL_OR_LOW_OP
%left <ival> ANDOP <pval> PLUGIN_LOGICAL_AND_LOW_OP
%right <ival> NOTOP
%nonassoc LSTOP LSTOPSUB
%left PERLY_COMMA
%right <ival> ASSIGNOP <pval> PLUGIN_ASSIGN_OP
%right <ival> PERLY_QUESTION_MARK PERLY_COLON
%nonassoc DOTDOT
%left <ival> OROR DORDOR <pval> PLUGIN_LOGICAL_OR_OP
%left <ival> ANDAND <pval> PLUGIN_LOGICAL_AND_OP
%left <ival> BITOROP
%left <ival> BITANDOP
%left <ival> CHEQOP NCEQOP
%left <ival> CHRELOP NCRELOP
%nonassoc <pval> PLUGIN_REL_OP
%nonassoc UNIOP UNIOPSUB
%nonassoc KW_REQUIRE
%left <ival> SHIFTOP
%left ADDOP <pval> PLUGIN_ADD_OP
%left MULOP <pval> PLUGIN_MUL_OP
%left <ival> MATCHOP
%right <ival> PERLY_EXCLAMATION_MARK PERLY_TILDE UMINUS REFGEN
%right POWOP <pval> PLUGIN_POW_OP
%nonassoc <ival> PREINC PREDEC POSTINC POSTDEC POSTJOIN
%nonassoc <pval> PLUGIN_HIGH_OP
%left <ival> ARROW
%nonassoc <ival> PERLY_PAREN_CLOSE
%left <ival> PERLY_PAREN_OPEN
%left PERLY_BRACKET_OPEN PERLY_BRACE_OPEN

%% /* RULES */

/* Top-level choice of what kind of thing yyparse was called to parse */
grammar	:	GRAMPROG
			{
			  parser->expect = XSTATE;
                          $<ival>$ = 0;
			}
		remember stmtseq
			{
			  newPROG(block_end($remember,$stmtseq));
			  PL_compiling.cop_seq = 0;
			  $$ = 0;
			}
	|	GRAMEXPR
			{
			  parser->expect = XTERM;
                          $<ival>$ = 0;
			}
		optexpr
			{
			  PL_eval_root = $optexpr;
			  $$ = 0;
			}
	|	GRAMBLOCK
			{
			  parser->expect = XBLOCK;
                          $<ival>$ = 0;
			}
		block
			{
			  PL_pad_reset_pending = TRUE;
			  PL_eval_root = $block;
			  $$ = 0;
			  yyunlex();
			  parser->yychar = yytoken = YYEOF;
			}
	|	GRAMBARESTMT
			{
			  parser->expect = XSTATE;
                          $<ival>$ = 0;
			}
		barestmt
			{
			  PL_pad_reset_pending = TRUE;
			  PL_eval_root = $barestmt;
			  $$ = 0;
			  yyunlex();
			  parser->yychar = yytoken = YYEOF;
			}
	|	GRAMFULLSTMT
			{
			  parser->expect = XSTATE;
                          $<ival>$ = 0;
			}
		fullstmt
			{
			  PL_pad_reset_pending = TRUE;
			  PL_eval_root = $fullstmt;
			  $$ = 0;
			  yyunlex();
			  parser->yychar = yytoken = YYEOF;
			}
	|	GRAMSTMTSEQ
			{
			  parser->expect = XSTATE;
                          $<ival>$ = 0;
			}
		stmtseq
			{
			  PL_eval_root = $stmtseq;
			  $$ = 0;
			}
	|	GRAMSUBSIGNATURE
			{
			  parser->expect = XSTATE;
			  $<ival>$ = 0;
			}
		subsigguts
			{
			  PL_eval_root = $subsigguts;
			  $$ = 0;
			}
	;

/* Either a signatured 'sub' or 'method' keyword */
sigsub_or_method_named
	:	KW_SUB_named_sig
			{ $$ = KW_SUB_named_sig; }
	|	KW_METHOD_named
			{ $$ = KW_METHOD_named; }
	;

/* An ordinary block */
block	:	PERLY_BRACE_OPEN remember stmtseq PERLY_BRACE_CLOSE
			{ if (parser->copline > (line_t)$PERLY_BRACE_OPEN)
			      parser->copline = (line_t)$PERLY_BRACE_OPEN;
			  $$ = block_end($remember, $stmtseq);
			}
	;

empty
	:	%empty          { $$ = NULL; }
	;

/* format body */
formblock:	PERLY_EQUAL_SIGN remember PERLY_SEMICOLON FORMRBRACK formstmtseq PERLY_SEMICOLON PERLY_DOT
			{ if (parser->copline > (line_t)$PERLY_EQUAL_SIGN)
			      parser->copline = (line_t)$PERLY_EQUAL_SIGN;
			  $$ = block_end($remember, $formstmtseq);
			}
	;

remember:	%empty	/* start a full lexical scope */
			{ $$ = block_start(TRUE);
			  parser->parsed_sub = 0; }
	;

mblock	:	PERLY_BRACE_OPEN mremember stmtseq PERLY_BRACE_CLOSE
			{ if (parser->copline > (line_t)$PERLY_BRACE_OPEN)
			      parser->copline = (line_t)$PERLY_BRACE_OPEN;
			  $$ = block_end($mremember, $stmtseq);
			}
	;

mremember:	%empty	/* start a partial lexical scope */
			{ $$ = block_start(FALSE);
			  parser->parsed_sub = 0; }
	;

/* The parenthesized variable of a catch block */
catch_paren:	empty
			/* not really valid grammar but we detect it in the
			 * action block to throw a nicer error message */
	|	PERLY_PAREN_OPEN
			{ parser->in_my = 1; }
		scalar
			{ parser->in_my = 0; intro_my(); }
		PERLY_PAREN_CLOSE
			{ $$ = $scalar; }
	;

/* A sequence of statements in the program */
stmtseq
	:	empty
	|	stmtseq[list] fullstmt
			{   $$ = op_append_list(OP_LINESEQ, $list, $fullstmt);
			    PL_pad_reset_pending = TRUE;
			    if ($list && $fullstmt)
				PL_hints |= HINT_BLOCK_SCOPE;
			}
	;

/* A sequence of format lines */
formstmtseq
	:	empty
	|	formstmtseq[list] formline
			{   $$ = op_append_list(OP_LINESEQ, $list, $formline);
			    PL_pad_reset_pending = TRUE;
			    if ($list && $formline)
				PL_hints |= HINT_BLOCK_SCOPE;
			}
	;

/* A statement in the program, including optional labels */
fullstmt:	barestmt
			{
			  $$ = $barestmt ? newSTATEOP(0, NULL, $barestmt) : NULL;
			}
	|	labfullstmt
			{ $$ = $labfullstmt; }
	;

labfullstmt:	LABEL barestmt
			{
                          SV *label = cSVOPx_sv($LABEL);
			  $$ = newSTATEOP(SvFLAGS(label) & SVf_UTF8,
                                            savepv(SvPVX_const(label)), $barestmt);
                          op_free($LABEL);
			}
	|	LABEL labfullstmt[list]
			{
                          SV *label = cSVOPx_sv($LABEL);
			  $$ = newSTATEOP(SvFLAGS(label) & SVf_UTF8,
                                            savepv(SvPVX_const(label)), $list);
                          op_free($LABEL);
			}
	;

/* A bare statement, lacking label and other aspects of state op */
barestmt:	PLUGSTMT
			{ $$ = $PLUGSTMT; }
	|	KW_FORMAT startformsub formname formblock
			{
			  CV *fmtcv = PL_compcv;
			  newFORM($startformsub, $formname, $formblock);
			  $$ = NULL;
			  if (CvOUTSIDE(fmtcv) && !CvEVAL(CvOUTSIDE(fmtcv))) {
			      pad_add_weakref(fmtcv);
			  }
			  parser->parsed_sub = 1;
			}
	|	KW_SUB_named subname startsub
                    /* sub declaration or definition not within scope
                       of 'use feature "signatures"'*/
			{
                          init_named_cv(PL_compcv, $subname);
			  parser->in_my = 0;
			  parser->in_my_stash = NULL;
			}
                    proto subattrlist optsubbody
			{
			  SvREFCNT_inc_simple_void(PL_compcv);
			  $subname->op_type == OP_CONST
			      ? newATTRSUB($startsub, $subname, $proto, $subattrlist, $optsubbody)
			      : newMYSUB($startsub, $subname, $proto, $subattrlist, $optsubbody)
			  ;
			  $$ = NULL;
			  intro_my();
			  parser->parsed_sub = 1;
			}
	|	sigsub_or_method_named subname startsub
                    /* sub declaration or definition under 'use feature
                     * "signatures"'. (Note that a signature isn't
                     * allowed in a declaration)
                     */
			{
                          init_named_cv(PL_compcv, $subname);
			  if($sigsub_or_method_named == KW_METHOD_named) {
			      croak_kw_unless_class("method");
			      class_prepare_method_parse(PL_compcv);
			  }
			  parser->in_my = 0;
			  parser->in_my_stash = NULL;
			}
                    subattrlist optsigsubbody
			{
			  OP *body = $optsigsubbody;

			  SvREFCNT_inc_simple_void(PL_compcv);
			  $subname->op_type == OP_CONST
			      ? newATTRSUB($startsub, $subname, NULL, $subattrlist, body)
			      : newMYSUB(  $startsub, $subname, NULL, $subattrlist, body)
			  ;
			  $$ = NULL;
			  intro_my();
			  parser->parsed_sub = 1;
			}
	|	PHASER startsub
			{
			  switch($PHASER) {
			      case KEY_ADJUST:
			         croak_kw_unless_class("ADJUST");
			         class_prepare_method_parse(PL_compcv);
			         break;
			      default:
			         NOT_REACHED;
			  }
			}
		    optsubbody
			{
			  OP *body = $optsubbody;
			  SvREFCNT_inc_simple_void(PL_compcv);

			  CV *cv;

			  switch($PHASER) {
			      case KEY_ADJUST:
			          cv = newATTRSUB($startsub, NULL, NULL, NULL, body);
			          class_add_ADJUST(PL_curstash, cv);
			          break;
			  }
			  $$ = NULL;
			}
	|	KW_PACKAGE BAREWORD[version] BAREWORD[package] PERLY_SEMICOLON
		    /* version and package appear in the reverse order to what may be
		     * expected, because toke.c has already pushed both of them to a stack
		     * by calling force_next() from within force_version().
		     * When the parser pops them back out again they appear swapped */
			{
			  package($package);
			  if ($version)
			      package_version($version);
			  $$ = NULL;
			}
	|	KW_CLASS BAREWORD[version] BAREWORD[package] subattrlist PERLY_SEMICOLON
			{
			  package($package);
			  if ($version)
			      package_version($version);
			  $$ = NULL;
			  class_setup_stash(PL_curstash);
			  if ($subattrlist) {
			      class_apply_attributes(PL_curstash, $subattrlist);
			  }
			}
	|	KW_USE_or_NO startsub
			{ CvSPECIAL_on(PL_compcv); /* It's a BEGIN {} */ }
		BAREWORD[version] BAREWORD[module] optlistexpr PERLY_SEMICOLON
		    /* version and package appear in reverse order for the same reason as
		     * KW_PACKAGE; see comment above */
			{
			  SvREFCNT_inc_simple_void(PL_compcv);
			  utilize($KW_USE_or_NO, $startsub, $version, $module, $optlistexpr);
			  parser->parsed_sub = 1;
			  $$ = NULL;
			}
	|	KW_IF PERLY_PAREN_OPEN remember mexpr PERLY_PAREN_CLOSE mblock else
			{
			  $$ = block_end($remember,
			      newCONDOP(0, $mexpr, op_scope($mblock), $else));
			  parser->copline = (line_t)$KW_IF;
			}
	|	KW_UNLESS PERLY_PAREN_OPEN remember mexpr PERLY_PAREN_CLOSE mblock else
			{
			  $$ = block_end($remember,
                              newCONDOP(0, $mexpr, $else, op_scope($mblock)));
			  parser->copline = (line_t)$KW_UNLESS;
			}
	|	KW_GIVEN PERLY_PAREN_OPEN remember mexpr PERLY_PAREN_CLOSE mblock
			{
			  $$ = block_end($remember, newGIVENOP($mexpr, op_scope($mblock), 0));
			  parser->copline = (line_t)$KW_GIVEN;
			}
	|	KW_WHEN PERLY_PAREN_OPEN remember mexpr PERLY_PAREN_CLOSE mblock
			{ $$ = block_end($remember, newWHENOP($mexpr, op_scope($mblock))); }
	|	KW_DEFAULT block
			{ $$ = newWHENOP(0, op_scope($block)); }
	|	KW_WHILE PERLY_PAREN_OPEN remember texpr PERLY_PAREN_CLOSE mintro mblock cont
			{
			  $$ = block_end($remember,
				  newWHILEOP(0, 1, NULL,
				      $texpr, $mblock, $cont, $mintro));
			  parser->copline = (line_t)$KW_WHILE;
			}
	|	KW_UNTIL PERLY_PAREN_OPEN remember iexpr PERLY_PAREN_CLOSE mintro mblock cont
			{
			  $$ = block_end($remember,
				  newWHILEOP(0, 1, NULL,
				      $iexpr, $mblock, $cont, $mintro));
			  parser->copline = (line_t)$KW_UNTIL;
			}
	|	KW_FOR PERLY_PAREN_OPEN remember mnexpr[init_mnexpr] PERLY_SEMICOLON
			{ parser->expect = XTERM; }
		texpr PERLY_SEMICOLON
			{ parser->expect = XTERM; }
		mintro mnexpr[iterate_mnexpr] PERLY_PAREN_CLOSE
		mblock
			{
			  OP *initop = $init_mnexpr;
			  OP *forop = newWHILEOP(0, 1, NULL,
				      scalar($texpr), $mblock, $iterate_mnexpr, $mintro);
			  if (initop) {
			      forop = op_prepend_elem(OP_LINESEQ, initop,
				  op_append_elem(OP_LINESEQ,
				      newOP(OP_UNSTACK, OPf_SPECIAL),
				      forop));
			  }
			  PL_hints |= HINT_BLOCK_SCOPE;
			  $$ = block_end($remember, forop);
			  parser->copline = (line_t)$KW_FOR;
			}
	|	KW_FOR KW_MY remember my_scalar PERLY_PAREN_OPEN mexpr PERLY_PAREN_CLOSE mblock cont
			{
			  $$ = block_end($remember, newFOROP(0, $my_scalar, $mexpr, $mblock, $cont));
			  parser->copline = (line_t)$KW_FOR;
			}
	|	KW_FOR KW_MY remember PERLY_PAREN_OPEN my_list_of_scalars PERLY_PAREN_CLOSE PERLY_PAREN_OPEN mexpr PERLY_PAREN_CLOSE mblock cont
			{
                          if ($my_list_of_scalars->op_type == OP_PADSV)
                            /* degenerate case of 1 var: for my ($x) ....
                               Flag it so it can be special-cased in newFOROP */
                                $my_list_of_scalars->op_flags |= OPf_PARENS;
			  $$ = block_end($remember, newFOROP(0, $my_list_of_scalars, $mexpr, $mblock, $cont));
			  parser->copline = (line_t)$KW_FOR;
			}
	|	KW_FOR scalar PERLY_PAREN_OPEN remember mexpr PERLY_PAREN_CLOSE mblock cont
			{
			  $$ = block_end($remember, newFOROP(0,
				      op_lvalue($scalar, OP_ENTERLOOP), $mexpr, $mblock, $cont));
			  parser->copline = (line_t)$KW_FOR;
			}
	|	KW_FOR my_refgen remember my_var
			{ parser->in_my = 0; $<opval>$ = my($my_var); }[variable]
		PERLY_PAREN_OPEN mexpr PERLY_PAREN_CLOSE mblock cont
			{
			  $$ = block_end(
				$remember,
				newFOROP(0,
					 op_lvalue(
					    newUNOP(OP_REFGEN, 0,
						    $<opval>variable),
					    OP_ENTERLOOP),
					 $mexpr, $mblock, $cont)
			  );
			  parser->copline = (line_t)$KW_FOR;
			}
	|	KW_FOR REFGEN refgen_topic PERLY_PAREN_OPEN remember mexpr PERLY_PAREN_CLOSE mblock cont
			{
			  $$ = block_end($remember, newFOROP(
				0, op_lvalue(newUNOP(OP_REFGEN, 0,
						     $refgen_topic),
					     OP_ENTERLOOP), $mexpr, $mblock, $cont));
			  parser->copline = (line_t)$KW_FOR;
			}
	|	KW_FOR PERLY_PAREN_OPEN remember mexpr PERLY_PAREN_CLOSE mblock cont
			{
			  $$ = block_end($remember,
				  newFOROP(0, NULL, $mexpr, $mblock, $cont));
			  parser->copline = (line_t)$KW_FOR;
			}
	|       KW_TRY mblock[try] KW_CATCH remember catch_paren[scalar]
			{
			  if(!$scalar) {
			      yyerror("catch block requires a (VAR)");
			      YYERROR;
			  }
			}
		mblock[catch] finally
			{
			  $$ = newTRYCATCHOP(0,
				  $try, $scalar, block_end($remember, op_scope($catch)));
			  if($finally)
			      $$ = op_wrap_finally($$, $finally);
			  parser->copline = (line_t)$KW_TRY;
			}
	|	block cont
			{
			  /* a block is a loop that happens once */
			  $$ = newWHILEOP(0, 1, NULL,
				  NULL, $block, $cont, 0);
			}
	|	KW_PACKAGE BAREWORD[version] BAREWORD[package] PERLY_BRACE_OPEN remember
			{
			  package($package);
			  if ($version) {
			      package_version($version);
			  }
			}
		stmtseq PERLY_BRACE_CLOSE
			{
			  /* a block is a loop that happens once */
			  $$ = newWHILEOP(0, 1, NULL,
				  NULL, block_end($remember, $stmtseq), NULL, 0);
			  if (parser->copline > (line_t)$PERLY_BRACE_OPEN)
			      parser->copline = (line_t)$PERLY_BRACE_OPEN;
			}
	|	KW_CLASS BAREWORD[version] BAREWORD[package] subattrlist PERLY_BRACE_OPEN remember
			{
			  package($package);

			  if ($version) {
			      package_version($version);
			  }
			  class_setup_stash(PL_curstash);
			  if ($subattrlist) {
			      class_apply_attributes(PL_curstash, $subattrlist);
			  }
			}
		stmtseq PERLY_BRACE_CLOSE
			{
			  /* a block is a loop that happens once */
			  $$ = newWHILEOP(0, 1, NULL,
				  NULL, block_end($remember, $stmtseq), NULL, 0);
			  if (parser->copline > (line_t)$PERLY_BRACE_OPEN)
			      parser->copline = (line_t)$PERLY_BRACE_OPEN;
			}
	|	fielddecl PERLY_SEMICOLON
			{
			  $$ = $fielddecl;
			}
	|	sideff PERLY_SEMICOLON
			{
			  $$ = $sideff;
			}
	|	KW_DEFER mblock
			{
			  $$ = newDEFEROP(0, op_scope($2));
			}
	|	YADAYADA PERLY_SEMICOLON
			{
                          /* diag_listed_as: Unimplemented */
			  $$ = newLISTOP(OP_DIE, 0, newOP(OP_PUSHMARK, 0),
				newSVOP(OP_CONST, 0, newSVpvs("Unimplemented")));
			}
	|	PERLY_SEMICOLON
			{
			  $$ = NULL;
			  parser->copline = NOLINE;
			}
	;

/* Format line */
formline:	THING formarg
			{ OP *list;
			  if ($formarg) {
			      OP *term = $formarg;
			      list = op_append_elem(OP_LIST, $THING, term);
			  }
			  else {
			      list = $THING;
			  }
			  if (parser->copline == NOLINE)
			       parser->copline = CopLINE(PL_curcop)-1;
			  else parser->copline--;
			  $$ = newSTATEOP(0, NULL,
					  op_convert_list(OP_FORMLINE, 0, list));
			}
	;

formarg
	:	empty
	|	FORMLBRACK stmtseq FORMRBRACK
			{ $$ = op_unscope($stmtseq); }
	;

condition: expr
;

/* An expression which may have a side-effect */
sideff	:	error
			{ $$ = NULL; }
	|	expr[body]
			{ $$ = $body; }
	|	expr[body] KW_IF condition
			{ $$ = newLOGOP(OP_AND, 0, $condition, $body); }
	|	expr[body] KW_UNLESS condition
			{ $$ = newLOGOP(OP_OR, 0, $condition, $body); }
	|	expr[body] KW_WHILE condition
			{ $$ = newLOOPOP(OPf_PARENS, 1, scalar($condition), $body); }
	|	expr[body] KW_UNTIL iexpr
			{ $$ = newLOOPOP(OPf_PARENS, 1, $iexpr, $body); }
	|	expr[body] KW_FOR condition
			{ $$ = newFOROP(0, NULL, $condition, $body, NULL);
			  parser->copline = (line_t)$KW_FOR; }
	|	expr[body] KW_WHEN condition
			{ $$ = newWHENOP($condition, op_scope($body)); }
	;

/* else and elsif blocks */
else
	:	empty
	|	KW_ELSE mblock
			{
			  ($mblock)->op_flags |= OPf_PARENS;
			  $$ = op_scope($mblock);
			}
	|	KW_ELSIF PERLY_PAREN_OPEN mexpr PERLY_PAREN_CLOSE mblock else[else.recurse]
			{ parser->copline = (line_t)$KW_ELSIF;
			    $$ = newCONDOP(0,
				newSTATEOP(OPf_SPECIAL,NULL,$mexpr),
				op_scope($mblock), $[else.recurse]);
			  PL_hints |= HINT_BLOCK_SCOPE;
			}
	;

/* Continue blocks */
cont
	:	empty
	|	KW_CONTINUE block
			{ $$ = op_scope($block); }
	;

/* Finally blocks */
finally	:	%empty
			{ $$ = NULL; }
	|	KW_FINALLY block
			{ $$ = op_scope($block); }
	;

/* determine whether there are any new my declarations */
mintro	:	%empty
			{ $$ = (PL_min_intro_pending &&
			    PL_max_intro_pending >=  PL_min_intro_pending);
			  intro_my(); }

/* Normal expression */
nexpr
	:	empty
	|	sideff
	;

/* Boolean expression */
texpr	:	%empty /* NULL means true */
			{ YYSTYPE tmplval;
			  (void)scan_num("1", &tmplval);
			  $$ = tmplval.opval; }
	|	expr
	;

/* Inverted boolean expression */
iexpr	:	expr
			{ $$ = invert(scalar($expr)); }
	;

/* Expression with its own lexical scope */
mexpr	:	expr
			{ $$ = $expr; intro_my(); }
	;

mnexpr	:	nexpr
			{ $$ = $nexpr; intro_my(); }
	;

formname:	BAREWORD	{ $$ = $BAREWORD; }
	|	empty
	;

startsub:	%empty	/* start a regular subroutine scope */
			{ $$ = start_subparse(FALSE, 0);
			    SAVEFREESV(PL_compcv); }

	;

startanonsub:	%empty	/* start an anonymous subroutine scope */
			{ $$ = start_subparse(FALSE, CVf_ANON);
			    SAVEFREESV(PL_compcv); }
	;

startanonmethod:	%empty	/* start an anonymous method scope */
			{ $$ = start_subparse(FALSE, CVf_ANON|CVf_IsMETHOD);
			    SAVEFREESV(PL_compcv); }
	;

startformsub:	%empty	/* start a format subroutine scope */
			{ $$ = start_subparse(TRUE, 0);
			    SAVEFREESV(PL_compcv); }
	;

/* Name of a subroutine - must be a bareword, could be special */
subname	:	BAREWORD
	|	PRIVATEREF
	;

/* Subroutine prototype */
proto
	:	empty
	|	THING
	;

/* Optional list of subroutine attributes */
subattrlist
	:	empty
	|	COLONATTR THING
			{
			  OP *attrlist = $THING;
			  if(attrlist && !PL_parser->sig_seen)
			      attrlist = apply_builtin_cv_attributes(PL_compcv, attrlist);
			  $$ = attrlist;
			}
	|	COLONATTR
			{ $$ = NULL; }
	;

/* List of attributes for a "my" variable declaration */
myattrlist:	COLONATTR THING
			{ $$ = $THING; }
	|	COLONATTR
			{ $$ = NULL; }
	;



/* --------------------------------------
 * subroutine signature parsing
 */

/* the '' or 'foo' part of a '$' or '@foo' etc signature variable  */
sigvarname:     %empty
			{ parser->in_my = 0; $$ = NULL; }
        |       PRIVATEREF
                        { parser->in_my = 0; $$ = $PRIVATEREF; }
	;

sigslurpsigil:
                PERLY_SNAIL
                        { $$ = '@'; }
        |       PERLY_PERCENT_SIGN
                        { $$ = '%'; }

/* @, %, @foo, %foo */
sigslurpelem: sigslurpsigil sigvarname sigdefault/* def only to catch errors */ 
                        {
                            I32 sigil = $sigslurpsigil;
                            OP *var   = $sigvarname;
                            OP *defop = $sigdefault;

                            if (parser->sig_slurpy)
                                yyerror("Multiple slurpy parameters not allowed");
                            parser->sig_slurpy = (char)sigil;

                            if (defop)
                                yyerror("A slurpy parameter may not have "
                                        "a default value");

                            $$ = var ? newSTATEOP(0, NULL, var) : NULL;
                        }
	;

/* default part of sub signature scalar element: i.e. '= default_expr' */
sigdefault
	:	empty
        |       ASSIGNOP
                        { $$ = newARGDEFELEMOP(0, newOP(OP_NULL, 0), parser->sig_elems); }
        |       ASSIGNOP term
                        {
                            I32 flags = 0;
                            if ($ASSIGNOP == OP_DORASSIGN)
                                flags |= OPpARG_IF_UNDEF << 8;
                            if ($ASSIGNOP == OP_ORASSIGN)
                                flags |= OPpARG_IF_FALSE << 8;
                            $$ = newARGDEFELEMOP(flags, $term, parser->sig_elems);
                        }


/* subroutine signature scalar element: e.g. '$x', '$=', '$x = $default' */
sigscalarelem:
                PERLY_DOLLAR sigvarname sigdefault
                        {
                            OP *var   = $sigvarname;
                            OP *defop = $sigdefault;

                            if (parser->sig_slurpy)
                                yyerror("Slurpy parameter not last");

                            parser->sig_elems++;

                            if (defop) {
                                parser->sig_optelems++;

                                OP *defexpr = cLOGOPx(defop)->op_first;

                                if (   defexpr->op_type == OP_NULL
                                    && !(defexpr->op_flags & OPf_KIDS))
                                {
                                    /* handle '$=' special case */
                                    if (var)
                                        yyerror("Optional parameter "
                                                    "lacks default expression");
                                    op_free(defop);
                                }
                                else { 
                                    /* a normal '=default' expression */ 
                                    if (var) {
                                        var->op_flags |= OPf_STACKED;
                                        (void)op_sibling_splice(var,
                                                        NULL, 0, defop);
                                        scalar(defop);
                                    }
                                    else
                                        var = newUNOP(OP_NULL, 0, defop);

                                    LINKLIST(var);
                                    /* NB: normally the first child of a
                                     * logop is executed before the logop,
                                     * and it pushes a boolean result
                                     * ready for the logop. For ARGDEFELEM,
                                     * the op itself does the boolean
                                     * calculation, so set the first op to
                                     * it instead.
                                     */
                                    var->op_next = defop;
                                    defexpr->op_next = var;
                                }
                            }
                            else {
                                if (parser->sig_optelems)
                                    yyerror("Mandatory parameter "
                                            "follows optional parameter");
                            }

                            $$ = var ? newSTATEOP(0, NULL, var) : NULL;
                        }
	;


/* subroutine signature element: e.g. '$x = $default' or '%h' */
sigelem:        sigscalarelem
                        { parser->in_my = KEY_sigvar; $$ = $sigscalarelem; }
        |       sigslurpelem
                        { parser->in_my = KEY_sigvar; $$ = $sigslurpelem; }
	;

/* list of subroutine signature elements */
siglist:
	 	siglist[list] PERLY_COMMA
			{ $$ = $list; }
	|	siglist[list] PERLY_COMMA sigelem[element]
			{
			  $$ = op_append_list(OP_LINESEQ, $list, $element);
			}
        |	sigelem[element]  %prec PREC_LOW
			{ $$ = $element; }
	;

/* () or (....) */
optsiglist
	:	empty
	|	siglist
	;

/* optional subroutine signature */
optsubsignature
	:	empty
	|	subsignature
	;

/* Subroutine signature */
subsignature:	PERLY_PAREN_OPEN subsigguts PERLY_PAREN_CLOSE
			{ $$ = $subsigguts; }

subsigguts:
                        {
                            ENTER;
                            SAVEIV(parser->sig_elems);
                            SAVEIV(parser->sig_optelems);
                            SAVEI8(parser->sig_slurpy);
                            parser->sig_elems    = 0;
                            parser->sig_optelems = 0;
                            parser->sig_slurpy   = 0;
                            parser->in_my        = KEY_sigvar;
                        }
                optsiglist
			{
                            OP            *sigops = $optsiglist;
                            struct op_argcheck_aux *aux;
                            OP            *check;

			    if (!FEATURE_SIGNATURES_IS_ENABLED && !CvIsMETHOD(PL_compcv))
			        Perl_croak(aTHX_ "Experimental "
                                    "subroutine signatures not enabled");

                            /* We shouldn't get here otherwise */
                            aux = (struct op_argcheck_aux*)
                                    PerlMemShared_malloc(
                                        sizeof(struct op_argcheck_aux));
                            aux->params     = parser->sig_elems;
                            aux->opt_params = parser->sig_optelems;
                            aux->slurpy     = parser->sig_slurpy;
                            check = newUNOP_AUX(OP_ARGCHECK, 0, NULL,
                                            (UNOP_AUX_item *)aux);
                            sigops = op_prepend_elem(OP_LINESEQ, check, sigops);
                            sigops = op_prepend_elem(OP_LINESEQ,
                                                newSTATEOP(0, NULL, NULL),
                                                sigops);
                            /* a nextstate at the end handles context
                             * correctly for an empty sub body */
                            sigops = op_append_elem(OP_LINESEQ,
                                                sigops,
                                                newSTATEOP(0, NULL, NULL));
                            /* wrap the list of arg ops in a NULL aux op.
                              This serves two purposes. First, it makes
                              the arg list a separate subtree from the
                              body of the sub, and secondly the null op
                              may in future be upgraded to an OP_SIGNATURE
                              when implemented. For now leave it as
                              ex-argcheck */
                            $$ = newUNOP_AUX(OP_ARGCHECK, 0, sigops, NULL);
                            op_null($$);

			    CvSIGNATURE_on(PL_compcv);

                            parser->in_my = 0;
                            /* tell the toker that attrributes can follow
                             * this sig, but only so that the toker
                             * can skip through any (illegal) trailing
                             * attribute text then give a useful error
                             * message about "attributes before sig",
                             * rather than falling over ina mess at
                             * unrecognised syntax.
                             */
                            parser->expect = XATTRBLOCK;
                            parser->sig_seen = TRUE;
                            LEAVE;
			}
	;

/* Optional subroutine body (for named subroutine declaration) */
optsubbody
	:	subbody
	|	PERLY_SEMICOLON	{ $$ = NULL; }
	;


/* Subroutine body (without signature) */
subbody:	remember  PERLY_BRACE_OPEN stmtseq PERLY_BRACE_CLOSE
			{
			  if (parser->copline > (line_t)$PERLY_BRACE_OPEN)
			      parser->copline = (line_t)$PERLY_BRACE_OPEN;
			  $$ = block_end($remember, $stmtseq);
			}
	;


/* optional [ Subroutine body with optional signature ] (for named
 * subroutine declaration) */
optsigsubbody
	:	sigsubbody
	|	PERLY_SEMICOLON	   { $$ = NULL; }
	;

/* Subroutine body with optional signature */
sigsubbody:	remember optsubsignature PERLY_BRACE_OPEN stmtseq PERLY_BRACE_CLOSE
			{
			  if (parser->copline > (line_t)$PERLY_BRACE_OPEN)
			      parser->copline = (line_t)$PERLY_BRACE_OPEN;
			  $$ = block_end($remember,
				op_append_list(OP_LINESEQ, $optsubsignature, $stmtseq));
 			}
 	;


/* Ordinary expressions; logical combinations */
expr	:	expr[lhs] ANDOP expr[rhs]
			{ $$ = newLOGOP(OP_AND, 0, $lhs, $rhs); }
	|	expr[lhs] PLUGIN_LOGICAL_AND_LOW_OP[op] expr[rhs]
			{ $$ = build_infix_plugin($lhs, $rhs, $op); }
	|	expr[lhs] OROP[operator] expr[rhs]
			{ $$ = newLOGOP($operator, 0, $lhs, $rhs); }
	|	expr[lhs] PLUGIN_LOGICAL_OR_LOW_OP[op] expr[rhs]
			{ $$ = build_infix_plugin($lhs, $rhs, $op); }
	|	listexpr %prec PREC_LOW
	;

/* Expressions are a list of terms joined by commas */
listexpr:	listexpr[list] PERLY_COMMA
			{ $$ = $list; }
	|	listexpr[list] PERLY_COMMA term
			{
			  OP* term = $term;
			  $$ = op_append_elem(OP_LIST, $list, term);
			}
	|	term %prec PREC_LOW
	;

/* List operators */
listop	:	LSTOP indirob listexpr /* map {...} @args or print $fh @args */
			{ $$ = op_convert_list($LSTOP, OPf_STACKED,
				op_prepend_elem(OP_LIST, newGVREF($LSTOP,$indirob), $listexpr) );
			}
	|	FUNC PERLY_PAREN_OPEN indirob expr PERLY_PAREN_CLOSE      /* print ($fh @args */
			{ $$ = op_convert_list($FUNC, OPf_STACKED,
				op_prepend_elem(OP_LIST, newGVREF($FUNC,$indirob), $expr) );
			}
	|	term ARROW methodname PERLY_PAREN_OPEN optexpr PERLY_PAREN_CLOSE /* $foo->bar(list) */
			{ $$ = op_convert_list(OP_ENTERSUB, OPf_STACKED,
				op_append_elem(OP_LIST,
				    op_prepend_elem(OP_LIST, scalar($term), $optexpr),
				    newMETHOP(OP_METHOD, 0, $methodname)));
			}
	|	term ARROW methodname                     /* $foo->bar */
			{ $$ = op_convert_list(OP_ENTERSUB, OPf_STACKED,
				op_append_elem(OP_LIST, scalar($term),
				    newMETHOP(OP_METHOD, 0, $methodname)));
			}
	|	METHCALL0 indirob optlistexpr           /* new Class @args */
			{ $$ = op_convert_list(OP_ENTERSUB, OPf_STACKED,
				op_append_elem(OP_LIST,
				    op_prepend_elem(OP_LIST, $indirob, $optlistexpr),
				    newMETHOP(OP_METHOD, 0, $METHCALL0)));
			}
	|	METHCALL indirob PERLY_PAREN_OPEN optexpr PERLY_PAREN_CLOSE    /* method $object (@args) */
			{ $$ = op_convert_list(OP_ENTERSUB, OPf_STACKED,
				op_append_elem(OP_LIST,
				    op_prepend_elem(OP_LIST, $indirob, $optexpr),
				    newMETHOP(OP_METHOD, 0, $METHCALL)));
			}
	|	LSTOP optlistexpr                    /* print @args */
			{ $$ = op_convert_list($LSTOP, 0, $optlistexpr); }
	|	FUNC PERLY_PAREN_OPEN optexpr PERLY_PAREN_CLOSE                 /* print (@args) */
			{ $$ = op_convert_list($FUNC, 0, $optexpr); }
	|	FUNC SUBLEXSTART optexpr SUBLEXEND          /* uc($arg) from "\U..." */
			{ $$ = op_convert_list($FUNC, 0, $optexpr); }
	|	LSTOPSUB startanonsub block /* sub f(&@);   f { foo } ... */
			{ SvREFCNT_inc_simple_void(PL_compcv);
			  $<opval>$ = newANONATTRSUB($startanonsub, 0, NULL, $block); }[anonattrsub]
		    optlistexpr		%prec LSTOP  /* ... @bar */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				 op_append_elem(OP_LIST,
				   op_prepend_elem(OP_LIST, $<opval>anonattrsub, $optlistexpr), $LSTOPSUB));
			}
	;

/* Names of methods. May use $object->$methodname */
methodname:	METHCALL0
	|	scalar
	;

/* Some kind of subscripted expression */
subscripted:    gelem PERLY_BRACE_OPEN expr PERLY_SEMICOLON PERLY_BRACE_CLOSE        /* *main::{something} */
                        /* In this and all the hash accessors, PERLY_SEMICOLON is
                         * provided by the tokeniser */
			{ $$ = newBINOP(OP_GELEM, 0, $gelem, scalar($expr)); }
	|	scalar[array] PERLY_BRACKET_OPEN expr PERLY_BRACKET_CLOSE          /* $array[$element] */
			{ $$ = newBINOP(OP_AELEM, 0, oopsAV($array), scalar($expr));
			}
	|	term[array_reference] ARROW PERLY_BRACKET_OPEN expr PERLY_BRACKET_CLOSE      /* somearef->[$element] */
			{ $$ = newBINOP(OP_AELEM, 0,
					ref(newAVREF($array_reference),OP_RV2AV),
					scalar($expr));
			}
	|	subscripted[array_reference] PERLY_BRACKET_OPEN expr PERLY_BRACKET_CLOSE    /* $foo->[$bar]->[$baz] */
			{ $$ = newBINOP(OP_AELEM, 0,
					ref(newAVREF($array_reference),OP_RV2AV),
					scalar($expr));
			}
	|	scalar[hash] PERLY_BRACE_OPEN expr PERLY_SEMICOLON PERLY_BRACE_CLOSE    /* $foo{bar();} */
			{ $$ = newBINOP(OP_HELEM, 0, oopsHV($hash), jmaybe($expr));
			}
	|	term[hash_reference] ARROW PERLY_BRACE_OPEN expr PERLY_SEMICOLON PERLY_BRACE_CLOSE /* somehref->{bar();} */
			{ $$ = newBINOP(OP_HELEM, 0,
					ref(newHVREF($hash_reference),OP_RV2HV),
					jmaybe($expr)); }
	|	subscripted[hash_reference] PERLY_BRACE_OPEN expr PERLY_SEMICOLON PERLY_BRACE_CLOSE /* $foo->[bar]->{baz;} */
			{ $$ = newBINOP(OP_HELEM, 0,
					ref(newHVREF($hash_reference),OP_RV2HV),
					jmaybe($expr)); }
	|	term[code_reference] ARROW PERLY_PAREN_OPEN PERLY_PAREN_CLOSE          /* $subref->() */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				   newCVREF(0, scalar($code_reference)));
			  if (parser->expect == XBLOCK)
			      parser->expect = XOPERATOR;
			}
	|	term[code_reference] ARROW PERLY_PAREN_OPEN expr PERLY_PAREN_CLOSE     /* $subref->(@args) */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				   op_append_elem(OP_LIST, $expr,
				       newCVREF(0, scalar($code_reference))));
			  if (parser->expect == XBLOCK)
			      parser->expect = XOPERATOR;
			}

	|	subscripted[code_reference] PERLY_PAREN_OPEN expr PERLY_PAREN_CLOSE   /* $foo->{bar}->(@args) */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				   op_append_elem(OP_LIST, $expr,
					       newCVREF(0, scalar($code_reference))));
			  if (parser->expect == XBLOCK)
			      parser->expect = XOPERATOR;
			}
	|	subscripted[code_reference] PERLY_PAREN_OPEN PERLY_PAREN_CLOSE        /* $foo->{bar}->() */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				   newCVREF(0, scalar($code_reference)));
			  if (parser->expect == XBLOCK)
			      parser->expect = XOPERATOR;
			}
	|	PERLY_PAREN_OPEN expr[list] PERLY_PAREN_CLOSE PERLY_BRACKET_OPEN expr[slice] PERLY_BRACKET_CLOSE            /* list slice */
			{ $$ = newSLICEOP(0, $slice, $list); }
	|	QWLIST PERLY_BRACKET_OPEN expr PERLY_BRACKET_CLOSE            /* list literal slice */
			{ $$ = newSLICEOP(0, $expr, $QWLIST); }
	|	PERLY_PAREN_OPEN PERLY_PAREN_CLOSE PERLY_BRACKET_OPEN expr PERLY_BRACKET_CLOSE                 /* empty list slice! */
			{ $$ = newSLICEOP(0, $expr, NULL); }
    ;

/* Binary operators between terms */
termbinop:	term[lhs] PLUGIN_HIGH_OP[op] term[rhs]
			{ $$ = build_infix_plugin($lhs, $rhs, $op); }
	|	term[lhs] ASSIGNOP term[rhs]                     /* $x = $y, $x += $y */
			{ $$ = newASSIGNOP(OPf_STACKED, $lhs, $ASSIGNOP, $rhs); }
	|	term[lhs] PLUGIN_ASSIGN_OP[op] term[rhs]
			{ $$ = build_infix_plugin($lhs, $rhs, $op); }
	|	term[lhs] POWOP term[rhs]                        /* $x ** $y */
			{ $$ = newBINOP($POWOP, 0, scalar($lhs), scalar($rhs)); }
	|	term[lhs] PLUGIN_POW_OP[op] term[rhs]
			{ $$ = build_infix_plugin($lhs, $rhs, $op); }
	|	term[lhs] MULOP term[rhs]                        /* $x * $y, $x x $y */
			{   if ($MULOP != OP_REPEAT)
				scalar($lhs);
			    $$ = newBINOP($MULOP, 0, $lhs, scalar($rhs));
			}
	|	term[lhs] PLUGIN_MUL_OP[op] term[rhs]
			{ $$ = build_infix_plugin($lhs, $rhs, $op); }
	|	term[lhs] ADDOP term[rhs]                        /* $x + $y */
			{ $$ = newBINOP($ADDOP, 0, scalar($lhs), scalar($rhs)); }
	|	term[lhs] PLUGIN_ADD_OP[op] term[rhs]
			{ $$ = build_infix_plugin($lhs, $rhs, $op); }
	|	term[lhs] SHIFTOP term[rhs]                      /* $x >> $y, $x << $y */
			{ $$ = newBINOP($SHIFTOP, 0, scalar($lhs), scalar($rhs)); }
	|	termrelop %prec PREC_LOW               /* $x > $y, etc. */
			{ $$ = $termrelop; }
	|	termeqop %prec PREC_LOW                /* $x == $y, $x cmp $y */
			{ $$ = $termeqop; }
	|	term[lhs] BITANDOP term[rhs]                     /* $x & $y */
			{ $$ = newBINOP($BITANDOP, 0, scalar($lhs), scalar($rhs)); }
	|	term[lhs] BITOROP term[rhs]                      /* $x | $y */
			{ $$ = newBINOP($BITOROP, 0, scalar($lhs), scalar($rhs)); }
	|	term[lhs] DOTDOT term[rhs]                       /* $x..$y, $x...$y */
			{ $$ = newRANGE($DOTDOT, scalar($lhs), scalar($rhs)); }
	|	term[lhs] ANDAND term[rhs]                       /* $x && $y */
			{ $$ = newLOGOP(OP_AND, 0, $lhs, $rhs); }
	|	term[lhs] PLUGIN_LOGICAL_AND_OP[op] term[rhs]
			{ $$ = build_infix_plugin($lhs, $rhs, $op); }
	|	term[lhs] OROR term[rhs]                         /* $x || $y */
			{ $$ = newLOGOP(OP_OR, 0, $lhs, $rhs); }
	|	term[lhs] PLUGIN_LOGICAL_OR_OP[op] term[rhs]
			{ $$ = build_infix_plugin($lhs, $rhs, $op); }
	|	term[lhs] DORDOR term[rhs]                       /* $x // $y */
			{ $$ = newLOGOP(OP_DOR, 0, $lhs, $rhs); }
	|	term[lhs] MATCHOP term[rhs]                      /* $x =~ /$y/ */
			{ $$ = bind_match($MATCHOP, $lhs, $rhs); }
	|	term[lhs] PLUGIN_LOW_OP[op] term[rhs]
			{ $$ = build_infix_plugin($lhs, $rhs, $op); }
    ;

termrelop:	relopchain %prec PREC_LOW
			{ $$ = cmpchain_finish($relopchain); }
	|	term[lhs] NCRELOP term[rhs]
			{ $$ = newBINOP($NCRELOP, 0, scalar($lhs), scalar($rhs)); }
	|	termrelop NCRELOP
			{ yyerror("syntax error"); YYERROR; }
	|	termrelop CHRELOP
			{ yyerror("syntax error"); YYERROR; }
	|	term[lhs] PLUGIN_REL_OP[op] term[rhs]
			{ $$ = build_infix_plugin($lhs, $rhs, $op); }
	;

relopchain:	term[lhs] CHRELOP term[rhs]
			{ $$ = cmpchain_start($CHRELOP, $lhs, $rhs); }
	|	relopchain[lhs] CHRELOP term[rhs]
			{ $$ = cmpchain_extend($CHRELOP, $lhs, $rhs); }
	;

termeqop:	eqopchain %prec PREC_LOW
			{ $$ = cmpchain_finish($eqopchain); }
	|	term[lhs] NCEQOP term[rhs]
			{ $$ = newBINOP($NCEQOP, 0, scalar($lhs), scalar($rhs)); }
	|	termeqop NCEQOP
			{ yyerror("syntax error"); YYERROR; }
	|	termeqop CHEQOP
			{ yyerror("syntax error"); YYERROR; }
	;

eqopchain:	term[lhs] CHEQOP term[rhs]
			{ $$ = cmpchain_start($CHEQOP, $lhs, $rhs); }
	|	eqopchain[lhs] CHEQOP term[rhs]
			{ $$ = cmpchain_extend($CHEQOP, $lhs, $rhs); }
	;

/* Unary operators and terms */
termunop : PERLY_MINUS term %prec UMINUS                       /* -$x */
			{ $$ = newUNOP(OP_NEGATE, 0, scalar($term)); }
	|	PERLY_PLUS term %prec UMINUS                  /* +$x */
			{ $$ = $term; }

	|	PERLY_EXCLAMATION_MARK term                               /* !$x */
			{ $$ = newUNOP(OP_NOT, 0, scalar($term)); }
	|	PERLY_TILDE term                               /* ~$x */
			{ $$ = newUNOP($PERLY_TILDE, 0, scalar($term)); }
	|	term POSTINC                           /* $x++ */
			{ $$ = newUNOP(OP_POSTINC, 0,
					op_lvalue(scalar($term), OP_POSTINC)); }
	|	term POSTDEC                           /* $x-- */
			{ $$ = newUNOP(OP_POSTDEC, 0,
					op_lvalue(scalar($term), OP_POSTDEC));}
	|	term POSTJOIN    /* implicit join after interpolated ->@ */
			{ $$ = op_convert_list(OP_JOIN, 0,
				       op_append_elem(
					OP_LIST,
					newSVREF(scalar(
					    newSVOP(OP_CONST,0,
						    newSVpvs("\""))
					)),
					$term
				       ));
			}
	|	PREINC term                            /* ++$x */
			{ $$ = newUNOP(OP_PREINC, 0,
					op_lvalue(scalar($term), OP_PREINC)); }
	|	PREDEC term                            /* --$x */
			{ $$ = newUNOP(OP_PREDEC, 0,
					op_lvalue(scalar($term), OP_PREDEC)); }

    ;

/* Constructors for anonymous data */
anonymous
	:	PERLY_BRACKET_OPEN optexpr PERLY_BRACKET_CLOSE
			{ $$ = newANONLIST($optexpr); }
	|	HASHBRACK optexpr PERLY_SEMICOLON PERLY_BRACE_CLOSE	%prec PERLY_PAREN_OPEN /* { foo => "Bar" } */
			{ $$ = newANONHASH($optexpr); }
	|	KW_SUB_anon     startanonsub proto subattrlist subbody    %prec PERLY_PAREN_OPEN
			{ SvREFCNT_inc_simple_void(PL_compcv);
			  $$ = newANONATTRSUB($startanonsub, $proto, $subattrlist, $subbody); }
	|	KW_SUB_anon_sig startanonsub subattrlist sigsubbody %prec PERLY_PAREN_OPEN
			{ SvREFCNT_inc_simple_void(PL_compcv);
			  $$ = newANONATTRSUB($startanonsub, NULL, $subattrlist, $sigsubbody); }
	|	KW_METHOD_anon startanonmethod subattrlist sigsubbody %prec PERLY_PAREN_OPEN
			{
			  SvREFCNT_inc_simple_void(PL_compcv);
			  $$ = newANONATTRSUB($startanonmethod, NULL, $subattrlist, $sigsubbody);
			}
    ;

/* Things called with "do" */
termdo	:       KW_DO term	%prec UNIOP                     /* do $filename */
			{ $$ = dofile($term, $KW_DO);}
	|	KW_DO block	%prec PERLY_PAREN_OPEN               /* do { code */
			{ $$ = newUNOP(OP_NULL, OPf_SPECIAL, op_scope($block));}
        ;

term[product]	:	termbinop
	|	termunop
	|	anonymous
	|	termdo
	|	term[condition] PERLY_QUESTION_MARK term[then] PERLY_COLON term[else]
			{ $$ = newCONDOP(0, $condition, $then, $else); }
	|	REFGEN term[operand]                          /* \$x, \@y, \%z */
			{ $$ = newUNOP(OP_REFGEN, 0, $operand); }
	|	myattrterm	%prec UNIOP
			{ $$ = $myattrterm; }
	|	KW_LOCAL term[operand]	%prec UNIOP
			{ $$ = localize($operand,0); }
	|	PERLY_PAREN_OPEN expr PERLY_PAREN_CLOSE
			{ $$ = sawparens($expr); }
	|	QWLIST
			{ $$ = $QWLIST; }
	|	PERLY_PAREN_OPEN PERLY_PAREN_CLOSE
			{ $$ = sawparens(newNULLLIST()); }
	|	scalar	%prec PERLY_PAREN_OPEN
			{ $$ = $scalar; }
	|	star	%prec PERLY_PAREN_OPEN
			{ $$ = $star; }
	|	hsh 	%prec PERLY_PAREN_OPEN
			{ $$ = $hsh; }
	|	ary 	%prec PERLY_PAREN_OPEN
			{ $$ = $ary; }
	|	arylen 	%prec PERLY_PAREN_OPEN                    /* $#x, $#{ something } */
			{ $$ = newUNOP(OP_AV2ARYLEN, 0, ref($arylen, OP_AV2ARYLEN));}
	|       subscripted
			{ $$ = $subscripted; }
	|	sliceme PERLY_BRACKET_OPEN expr PERLY_BRACKET_CLOSE                     /* array slice */
			{ $$ = op_prepend_elem(OP_ASLICE,
				newOP(OP_PUSHMARK, 0),
				    newLISTOP(OP_ASLICE, 0,
					list($expr),
					ref($sliceme, OP_ASLICE)));
			  if ($$ && $sliceme)
			      $$->op_private |=
				  $sliceme->op_private & OPpSLICEWARNING;
			}
	|	kvslice PERLY_BRACKET_OPEN expr PERLY_BRACKET_CLOSE                 /* array key/value slice */
			{ $$ = op_prepend_elem(OP_KVASLICE,
				newOP(OP_PUSHMARK, 0),
				    newLISTOP(OP_KVASLICE, 0,
					list($expr),
					ref(oopsAV($kvslice), OP_KVASLICE)));
			  if ($$ && $kvslice)
			      $$->op_private |=
				  $kvslice->op_private & OPpSLICEWARNING;
			}
	|	sliceme PERLY_BRACE_OPEN expr PERLY_SEMICOLON PERLY_BRACE_CLOSE                 /* @hash{@keys} */
			{ $$ = op_prepend_elem(OP_HSLICE,
				newOP(OP_PUSHMARK, 0),
				    newLISTOP(OP_HSLICE, 0,
					list($expr),
					ref(oopsHV($sliceme), OP_HSLICE)));
			  if ($$ && $sliceme)
			      $$->op_private |=
				  $sliceme->op_private & OPpSLICEWARNING;
			}
	|	kvslice PERLY_BRACE_OPEN expr PERLY_SEMICOLON PERLY_BRACE_CLOSE                 /* %hash{@keys} */
			{ $$ = op_prepend_elem(OP_KVHSLICE,
				newOP(OP_PUSHMARK, 0),
				    newLISTOP(OP_KVHSLICE, 0,
					list($expr),
					ref($kvslice, OP_KVHSLICE)));
			  if ($$ && $kvslice)
			      $$->op_private |=
				  $kvslice->op_private & OPpSLICEWARNING;
			}
	|	THING	%prec PERLY_PAREN_OPEN
			{ $$ = $THING; }
	|	amper                                /* &foo; */
			{ $$ = newUNOP(OP_ENTERSUB, 0, scalar($amper)); }
	|	amper PERLY_PAREN_OPEN PERLY_PAREN_CLOSE                 /* &foo() or foo() */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED, scalar($amper));
			}
	|	amper PERLY_PAREN_OPEN expr PERLY_PAREN_CLOSE          /* &foo(@args) or foo(@args) */
			{
			  $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				op_append_elem(OP_LIST, $expr, scalar($amper)));
			}
	|	NOAMP subname optlistexpr       /* foo @args (no parens) */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
			    op_append_elem(OP_LIST, $optlistexpr, scalar($subname)));
			}
	|	term[operand] ARROW PERLY_DOLLAR PERLY_STAR
			{ $$ = newSVREF($operand); }
	|	term[operand] ARROW PERLY_SNAIL PERLY_STAR
			{ $$ = newAVREF($operand); }
	|	term[operand] ARROW PERLY_PERCENT_SIGN PERLY_STAR
			{ $$ = newHVREF($operand); }
	|	term[operand] ARROW PERLY_AMPERSAND PERLY_STAR
			{ $$ = newUNOP(OP_ENTERSUB, 0,
				       scalar(newCVREF($PERLY_AMPERSAND,$operand))); }
	|	term[operand] ARROW PERLY_STAR PERLY_STAR	%prec PERLY_PAREN_OPEN
			{ $$ = newGVREF(0,$operand); }
	|	LOOPEX  /* loop exiting command (goto, last, dump, etc) */
			{ $$ = newOP($LOOPEX, OPf_SPECIAL);
			    PL_hints |= HINT_BLOCK_SCOPE; }
	|	LOOPEX term[operand]
			{ $$ = newLOOPEX($LOOPEX,$operand); }
	|	NOTOP listexpr                       /* not $foo */
			{ $$ = newUNOP(OP_NOT, 0, scalar($listexpr)); }
	|	UNIOP                                /* Unary op, $_ implied */
			{ $$ = newOP($UNIOP, 0); }
	|	UNIOP block                          /* eval { foo }* */
			{ $$ = newUNOP($UNIOP, 0, $block); }
	|	UNIOP term[operand]                           /* Unary op */
			{ $$ = newUNOP($UNIOP, 0, $operand); }
	|	KW_REQUIRE                              /* require, $_ implied */
			{ $$ = newOP(OP_REQUIRE, $KW_REQUIRE ? OPf_SPECIAL : 0); }
	|	KW_REQUIRE term[operand]                         /* require Foo */
			{ $$ = newUNOP(OP_REQUIRE, $KW_REQUIRE ? OPf_SPECIAL : 0, $operand); }
	|	UNIOPSUB
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED, scalar($UNIOPSUB)); }
	|	UNIOPSUB term[operand]                        /* Sub treated as unop */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
			    op_append_elem(OP_LIST, $operand, scalar($UNIOPSUB))); }
	|	FUNC0                                /* Nullary operator */
			{ $$ = newOP($FUNC0, 0); }
	|	FUNC0 PERLY_PAREN_OPEN PERLY_PAREN_CLOSE
			{ $$ = newOP($FUNC0, 0);}
	|	FUNC0OP       /* Same as above, but op created in toke.c */
			{ $$ = $FUNC0OP; }
	|	FUNC0OP PERLY_PAREN_OPEN PERLY_PAREN_CLOSE
			{ $$ = $FUNC0OP; }
	|	FUNC0SUB                             /* Sub treated as nullop */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED, scalar($FUNC0SUB)); }
	|	FUNC1 PERLY_PAREN_OPEN PERLY_PAREN_CLOSE                        /* not () */
			{ $$ = ($FUNC1 == OP_NOT)
                          ? newUNOP($FUNC1, 0, newSVOP(OP_CONST, 0, newSViv(0)))
                          : newOP($FUNC1, OPf_SPECIAL); }
	|	FUNC1 PERLY_PAREN_OPEN expr PERLY_PAREN_CLOSE                   /* not($foo) */
			{ $$ = newUNOP($FUNC1, 0, $expr); }
	|	PMFUNC /* m//, s///, qr//, tr/// */
			{
			    if (   $PMFUNC->op_type != OP_TRANS
			        && $PMFUNC->op_type != OP_TRANSR
				&& (((PMOP*)$PMFUNC)->op_pmflags & PMf_HAS_CV))
			    {
				$<ival>$ = start_subparse(FALSE, CVf_ANON);
				SAVEFREESV(PL_compcv);
			    } else
				$<ival>$ = 0;
			}
		    SUBLEXSTART listexpr optrepl SUBLEXEND
			{ $$ = pmruntime($PMFUNC, $listexpr, $optrepl, 1, $<ival>2); }
	|	BAREWORD
	|	listop
	|	PLUGEXPR
	;

/* "my" declarations, with optional attributes */
myattrterm
	:	KW_MY myterm myattrlist
			{ $$ = my_attrs($myterm,$myattrlist); }
	|	KW_MY myterm
			{ $$ = localize($myterm,1); }
	|	KW_MY REFGEN myterm myattrlist
			{ $$ = newUNOP(OP_REFGEN, 0, my_attrs($myterm,$myattrlist)); }
	|	KW_MY REFGEN term[operand]
			{ $$ = newUNOP(OP_REFGEN, 0, localize($operand,1)); }
	;

/* Things that can be "my"'d */
myterm	:	PERLY_PAREN_OPEN expr PERLY_PAREN_CLOSE
			{ $$ = sawparens($expr); }
	|	PERLY_PAREN_OPEN PERLY_PAREN_CLOSE
			{ $$ = sawparens(newNULLLIST()); }

	|	scalar	%prec PERLY_PAREN_OPEN
			{ $$ = $scalar; }
	|	hsh 	%prec PERLY_PAREN_OPEN
			{ $$ = $hsh; }
	|	ary 	%prec PERLY_PAREN_OPEN
			{ $$ = $ary; }
	;

/* "field" declarations */
fieldvar:	scalar	%prec PERLY_PAREN_OPEN
			{
			  $$ = PadnamelistARRAY(PL_comppad_name)[$scalar->op_targ];
			  op_free($scalar);
			}
	|	hsh 	%prec PERLY_PAREN_OPEN
			{
			  $$ = PadnamelistARRAY(PL_comppad_name)[$hsh->op_targ];
			  op_free($hsh);
			}
	|	ary 	%prec PERLY_PAREN_OPEN
			{
			  $$ = PadnamelistARRAY(PL_comppad_name)[$ary->op_targ];
			  op_free($ary);
			}
	;

optfieldattrlist:
		COLONATTR THING
			{ $$ = $THING; }
	|	COLONATTR
			{ $$ = NULL; }
	|	empty
	;

fielddecl
	:	KW_FIELD fieldvar optfieldattrlist
			{
			  parser->in_my = 0;
			  if($optfieldattrlist)
			    class_apply_field_attributes((PADNAME *)$fieldvar, $optfieldattrlist);
			  $$ = newOP(OP_NULL, 0);
			}
	|	KW_FIELD fieldvar optfieldattrlist ASSIGNOP
			{
			  parser->in_my = 0;
			  if($optfieldattrlist)
			    class_apply_field_attributes((PADNAME *)$fieldvar, $optfieldattrlist);
			  ENTER;
			  class_prepare_initfield_parse();
			}
		term
			{
			  class_set_field_defop((PADNAME *)$fieldvar, $ASSIGNOP, $term);
			  LEAVE;
			  $$ = newOP(OP_NULL, 0);
			}
	;

/* Basic list expressions */
optlistexpr
	:	empty                   %prec PREC_LOW
	|	listexpr                %prec PREC_LOW
	;

optexpr
	:	empty
	|	expr
	;

optrepl
	:	empty
	|	PERLY_SLASH expr        { $$ = $expr; }
	;

/* A little bit of trickery to make "for my $foo (@bar)" actually be
   lexical */
my_scalar:	scalar
			{ parser->in_my = 0; $$ = my($scalar); }
	;

/* A list of scalars for "for my ($foo, $bar) (@baz)"  */
list_of_scalars:	list_of_scalars[list] PERLY_COMMA
			{ $$ = $list; }
	|		list_of_scalars[list] PERLY_COMMA scalar
			{
			  $$ = op_append_elem(OP_LIST, $list, $scalar);
			}
	|		scalar %prec PREC_LOW
	;

my_list_of_scalars:	list_of_scalars
			{ parser->in_my = 0; $$ = $list_of_scalars; }
	;

my_var	:	scalar
	|	ary
	|	hsh
	;

refgen_topic:	my_var
	|	amper
	;

my_refgen:	KW_MY REFGEN
	|	REFGEN KW_MY
	;

amper	:	PERLY_AMPERSAND indirob
			{ $$ = newCVREF($PERLY_AMPERSAND,$indirob); }
	;

scalar	:	PERLY_DOLLAR indirob
			{ $$ = newSVREF($indirob); }
	;

ary	:	PERLY_SNAIL indirob
			{ $$ = newAVREF($indirob);
			  if ($$) $$->op_private |= $PERLY_SNAIL;
			}
	;

hsh	:	PERLY_PERCENT_SIGN indirob
			{ $$ = newHVREF($indirob);
			  if ($$) $$->op_private |= $PERLY_PERCENT_SIGN;
			}
	;

arylen	:	DOLSHARP indirob
			{ $$ = newAVREF($indirob); }
	|	term ARROW DOLSHARP PERLY_STAR
			{ $$ = newAVREF($term); }
	;

star	:	PERLY_STAR indirob
			{ $$ = newGVREF(0,$indirob); }
	;

sliceme	:	ary
	|	term ARROW PERLY_SNAIL
			{ $$ = newAVREF($term); }
	;

kvslice	:	hsh
	|	term ARROW PERLY_PERCENT_SIGN
			{ $$ = newHVREF($term); }
	;

gelem	:	star
	|	term ARROW PERLY_STAR
			{ $$ = newGVREF(0,$term); }
	;

/* Indirect objects */
indirob	:	BAREWORD
			{ $$ = scalar($BAREWORD); }
	|	scalar %prec PREC_LOW
			{ $$ = scalar($scalar); }
	|	block
			{ $$ = op_scope($block); }

	|	PRIVATEREF
			{ $$ = $PRIVATEREF; }
	;

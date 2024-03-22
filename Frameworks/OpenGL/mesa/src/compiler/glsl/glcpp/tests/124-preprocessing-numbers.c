#define e THIS_SHOULD_NOT_BE_EXPANDED
#define E NOR_THIS
#define p NOT_THIS_EITHER
#define P AND_SURELY_NOT_THIS
#define OK CRAZY_BUT_TRUE_THIS_NEITHER

/* This one is actually meant to be expanded */
#define MUST_EXPAND GO

/* The following are "preprocessing numbers" and should not trigger macro
 * expansion. */
1e
1OK

/* These are also "preprocessing numbers", so no expansion */
123e+OK
.23E+OK
1.3e-OK
12.E-OK
123p+OK
.23P+OK
1.3p-OK
12.P-OK
123..OK
.23.OK.OK

/* Importantly, just before the MUST_EXPAND in each of these, the preceding
 * "preprocessing number" ends and we have an actual expression. So the
 * MUST_EXPAND macro must be expanded (who would have though?) in each case. */
123ef+MUST_EXPAND
.23E3-MUST_EXPAND
1.3e--MUST_EXPAND
12.E-&MUST_EXPAND
123p+OK+MUST_EXPAND
.23P+OK;MUST_EXPAND
1.3p-OK-MUST_EXPAND
12.P-OK&MUST_EXPAND

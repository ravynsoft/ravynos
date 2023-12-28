/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _COMP_COM_H
#define _COMP_COM_H

#include <sys/types.h>
#include <nl_types.h>

/*
 * This file describes format for the compiler-commentary
 * section to be added to .o's and propagated to the a.out.  It reflects
 * information the compiler can expose to the user about his or her
 * program.  The section should be generated for all compiles where
 * the user has specified -g on the compile line.
 *
 * In the analyzer, display of the messages will be governed by a user UI
 * that sets a vis_bits bitmap, and matches it against a show_bits
 * bitmap table, which is maintained separately from the producer
 * code.  For any message, if (vis_bits&show_bits) is  non-zero, the
 * message is shown.  If zero, the message is not shown.  A similar
 * mechanism would be used for a stand-alone source or disassembly browser.
 *
 *
 * The .compcom Section
 * --------------------
 * The section will be named ".compcom"; it is generated for each
 * .o, and aggregated into a single section in the a.out.  In that
 * section, each .o's data is separate, and the tools will loop
 * over the data for each .o in order to find the subsection for
 * the particular .o being annotated.
 *
 *
 * Since the header is fixed-length, and the total size of the section
 * can be easily determined as:
 *
 *     sizeof(stuct compcomhdr)
 * 	+ msgcount * sizeof(struct compmsg)
 * 	+ paramcount * sizeof(int32_t)
 * 	+ stringlen
 *
 * there is no need to have the size in the header.
 */

typedef struct
{ /* Header describing the section */
  int32_t srcname;          /* index into strings of source file path */
  int32_t version;          /* a version number for the .compcom format */
  int32_t msgcount;         /* count of messages in the section */
  int32_t paramcount;       /* count of parameters in the section */
  int32_t stringcount;      /* count of strings in the section */
  int32_t stringlen;        /* count of total bytes in strings */
} compcomhdr;

/*
 * The data for the .o after the header as:
 *
 *    compmsg	msgs[msgcount];		the array of messages
 *    int32_t	param[paramcount];	the parameters used in the messages
 *					parameters are either integers or
 *					string-indices
 *    char	msgstrings[stringlen];	the strings used in the messages
 */

/*
 * Message Classes and Visualization Bits
 * --------------------------------------
 * Each of the messages above may belong to zero or more visualization
 * classes, governed by a table using zero or more of the following symbolic
 * names for the classes:
 */
typedef enum {
CCMV_WANT   = 0x000,		/* High-priority RFE -- used only for human */
				/*   reading of message list */
CCMV_UNIMPL = 0x000,		/* Unimplemented -- used only for human */
				/*   reading of message list */
CCMV_OBS    = 0x000,		/* Obsolete -- to be replaced by a different */
				/*   message with different parameters -- */
				/*   used only for human reading of message */
				/*   list */
CCMV_VER    = 0x001,		/* Versioning messages */
CCMV_WARN   = 0x002,		/* Warning messages */
CCMV_PAR    = 0x004,		/* Parallelization messages */
CCMV_QUERY  = 0x008,		/* Compiler queries */
CCMV_LOOP   = 0x010,		/* Loop detail messages */
CCMV_PIPE   = 0x020,		/* Pipelining messages */
CCMV_INLINE = 0x040,		/* Inlining information */
CCMV_MEMOPS = 0x080,		/* Messages concerning memory operations */
CCMV_FE     = 0x100,		/* Front-end messages (all compilers) */
CCMV_CG     = 0x200,		/* Code-generator messages (all compilers) */
CCMV_BASIC  = 0x400,		/* for default messages */
CCMV_ALL    = 0x7FFFFFFF	/* for all messages */
} COMPCLASS_ID;

typedef enum ccm_msgid
{
  /*	Group: Versioning Messages */
  /*	All of these are global to the .o, and will */
  /*	have lineno = pcoffset = 0 */

CCM_MODDATE=0x00100,   	/* Source file <s1>, last modified on date <s2> */
CCM_COMPVER,           	/* Component <s1>, version <s2> */
			/* [Emitted for each component of the compiler.] */
CCM_COMPDATE,          	/* Compilation date <s1> */
			/* [<s1> is an I18n string with the date and time] */
CCM_COMPOPT,           	/* Compilation options <s1> */
			/* [As specified by the user] */
CCM_ACOMPOPT,          	/* Actual Compilation options <s1> */
			/* [As expanded by the driver] */

  /* Group: Warning Messages */
CCM_VAR_ALIAS=0x00200, 	/* Variable <v1> aliased to <v2> */
CCM_FBIRDIFF,          	/* Profile feedback data inconsistent with */
			/* intermediate representation file; check compiler */
			/* version, flags and source file */
CCM_OPTRED_SWAP,       	/* Optimization level for <p1> reduced from <i2> to */
			/* <i3> due to insufficient swap space */
CCM_OPTRED_CPLX,       	/* Optimization level for <p1> reduced from <i2> to */
			/* <i3> due to program complexity */
CCM_UNKNOWN,           	/* Unexpected compiler comment <i1> */

  /* Group: Parallelization Messages */
CCM_UNPAR_CALL=0x00400,	/* Loop below not parallelized because it contains a */
			/* call to <p1> */

  /* CCMV_WANT: Don't generate CCM_PAR_SER; always use CCM_PAR_SER_VER */
CCM_PAR_SER,           	/* Both serial and parallel versions generated for */
			/* loop below */
CCM_PAR_SER_VER,       	/* Both serial and parallel versions generated for */
			/* loop below; with parallel version used if <s1>, */
			/* serial otherwise */
CCM_PAR_DRECTV,        	/* Loop below parallelized by explicit user */
			/* directive */
CCM_APAR,              	/* Loop below autoparallelized */
CCM_AUTOPAR,           	/* Loop below autoparallelized; equivalent */
			/* explict directive is <s1> */
CCM_UNPAR_DD,          	/* Loop below could not be parallelized because of a */
			/* data dependency on <v1>, <v2>, ... */
			/* [The number of parameters will determine how many */
			/* names appear, and the formatter will get the */
			/* commas right.] */
CCM_UNPAR_DDA,         	/* Loop below could not be parallelized because of a */
			/* data dependency or aliasing of <v1>, <v2>, ... */
CCM_UNPAR_ANONDD,      	/* Loop below could not be parallelized because of */
			/* an anonymous data dependency */
CCM_UNPAR_ANONDDA,     	/* Loop below could not be parallelized because of */
			/* an anonymous data dependency or aliasing */
CCM_PAR_WORK,          	/* Loop below parallelized, but might not contain */
			/* enough work to be efficiently run in parallel */
CCM_UNPAR_EXIT,        	/* Loop below not parallelized because it contains */
			/* multiple exit points */
CCM_UNPAR_STRNG,       	/* Loop below not parallelized because it contains a */
			/* strange flow of control */
CCM_UNPAR_IO,          	/* Loop below not parallelized because it contains */
			/* I/O or other MT-unsafe calls */
CCM_PAR_BODY_NAME,     	/* Parallel loop-body code is in function <p1> */
CCM_UNPAR_NLOOPIDX,    	/* Loop below not parallelized because loop index */
			/* not found */
CCM_UNPAR_DRECTV,      	/* Loop below not parallelized because of explicit */
			/* user directive */
CCM_UNPAR_NOTPROFIT,   	/* Loop below not parallelized because it was not */
			/* profitable to do so */
CCM_UNPAR_NEST,        	/* Loop below not parallelized because it was */
			/* nested in a parallel loop */
CCM_UNPAR,             	/* Loop below not parallelized */
CCM_UNPAR_NOAUTO,      	/* Loop below not parallelized because */
			/* autoparallelization is not enabled */
CCM_PR_L_VAR,          	/* Private variables in loop below: */
			/* <v1>, <v2>, ... */
			/* [The number of parameters will determine how many */
			/* names appear, and the formatter will get the */
			/* commas right.] */
CCM_SH_L_VAR,          	/* Shared variables in loop below: */
			/* <v1>, <v2>, ... */
CCM_TP_L_VAR,          	/* Threadprivate variables in loop below: */
			/* <v1>, <v2>, ... */
CCM_RV_L_VAR,          	/* Reduction variables in loop below: */
			/* <v1>, <v2>, ... */
CCM_IM_L_VAR,          	/* Implicit variables in loop below: */
			/* <v1>, <v2>, ... */
CCM_PR_O_VAR,          	/* Private variables in OpenMP construct below: */
			/* <v1>, <v2>, ... */
CCM_SH_O_VAR,          	/* Shared variables in OpenMP construct below: */
			/* <v1>, <v2>, ... */
CCM_TP_O_VAR,          	/* Threadprivate variables in OpenMP construct */
			/* below: <v1>, <v2>, ... */
CCM_RV_O_VAR,          	/* Reduction variables in OpenMP construct below: */
			/* <v1>, <v2>, ... */
CCM_IM_O_VAR,          	/* Implicit variables in OpenMP construct below: */
			/* <v1>, <v2>, ... */
CCM_UNPAR_IN_OMP,      	/* Loop below not parallelized because it is inside */
			/* an OpenMP region */
CCM_FP_O_VAR,          	/* Firstprivate variables in OpenMP construct below: */
			/* <v1>, <v2>, ... */
CCM_LP_O_VAR,          	/* Lastprivate variables in OpenMP construct below: */
			/* <v1>, <v2>, ... */
CCM_CP_O_VAR,          	/* Copyprivate variables in OpenMP construct below: */
			/* <v1>, <v2>, ... */
CCM_PR_OAS_VAR,        	/* Variables autoscoped as PRIVATE in OpenMP */
			/* construct below: <v1>, <v2>, ... */
CCM_SH_OAS_VAR,        	/* Variables autoscoped as SHARED in OpenMP */
			/* construct below: <v1>, <v2>, ... */
CCM_FP_OAS_VAR,        	/* Variables autoscoped as FIRSTPRIVATE in OpenMP */
			/* construct below: <v1>, <v2>, ... */
CCM_LP_OAS_VAR,        	/* Variables autoscoped as LASTPRIVATE in OpenMP */
			/* construct below: <v1>, <v2>, ... */
CCM_RV_OAS_VAR,        	/* Variables autoscoped as REDUCTION in OpenMP */
			/* construct below: <v1>, <v2>, ... */
CCM_FAIL_OAS_VAR,      	/* Variables cannot be autoscoped in OpenMP */
			/* construct below: <v1>, <v2>, ... */
CCM_SERIALIZE_OAS,     	/* OpenMP parallel region below is serialized */
			/* because autoscoping has failed */
CCM_UNPAR_CALL_2,      	/* <l1> not parallelized because it contains calls */
			/* to: <p2>, <p3>, ... */
CCM_PAR_DRECTV_2,      	/* <l1> parallelized by explicit user directive */
CCM_APAR_2,            	/* <l1> autoparallelized */
CCM_AUTOPAR_2,         	/* <l1> autoparallelized; equivalent */
			/* explict directive is <s2> */
CCM_UNPAR_DD_2,        	/* <l1> could not be parallelized because of */
			/* data dependences on: <v2>, <v3>, ... */
			/* [The number of parameters will determine how many */
			/* names appear, and the formatter will get the */
			/* commas right.] */
CCM_UNPAR_DDA_2,       	/* <l1> could not be parallelized because of a */
			/* data dependence or aliasing of: <v2>, <v3>, ... */
CCM_UNPAR_ANONDD_2,    	/* <l1> could not be parallelized because of an */
			/* anonymous data dependence */
CCM_UNPAR_ANONDDA_2,   	/* <l1> could not be parallelized because of an */
			/* anonymous data dependence or aliasing */
CCM_PAR_WORK_2,        	/* <l1> parallelized, but might not contain */
			/* enough work to run efficiently in parallel */
CCM_UNPAR_EXIT_2,      	/* <l1> not parallelized because it contains */
			/* multiple exit points */
CCM_UNPAR_STRANGE_2,   	/* <l1> not parallelized because it contains a */
			/* strange flow of control */
CCM_UNPAR_IO_2,        	/* <l1> not parallelized because it contains */
			/* I/O or other MT-unsafe calls */
CCM_PAR_BODY_NAME_2,   	/* <l1> parallel loop-body code placed in */
			/* function <p2> along with <i3> inner loops */
CCM_UNPAR_NLOOPIDX_2,  	/* <l1> not parallelized because loop index not */
			/* found */
CCM_UNPAR_DRECTV_2,    	/* <l1> not parallelized because of explicit */
			/* user directive */
CCM_UNPAR_NOTPROFIT_2, 	/* <l1> not parallelized because it was not */
			/* profitable to do so */
CCM_UNPAR_NEST_2,      	/* <l1> not parallelized because it was */
			/* nested within a parallel loop */
CCM_UNPAR_2,           	/* <l1> not parallelized */
CCM_UNPAR_NOAUTO_2,    	/* <l1> not parallelized because */
			/* autoparallelization is not enabled */
CCM_PR_L_VAR_2,        	/* Private variables in <l1>: */
			/* <v2>, <v3>, ... */
			/* [The number of parameters will determine how many */
			/* names appear, and the formatter will get the */
			/* commas right.] */
CCM_SH_L_VAR_2,        	/* Shared variables in <l1>: */
			/* <v2>, <v3>, ... */
CCM_TP_L_VAR_2,        	/* Threadprivate variables in <l1>: */
			/* <v2>, <v3>, ... */
CCM_RV_L_VAR_2,        	/* Reduction variables of operator <s1> in <l2>: */
			/* <v3>, <v4>, ... */
CCM_IM_L_VAR_2,        	/* Implicit variables in <l1>: */
			/* <v2>, <v3>, ... */
CCM_PR_O_VAR_2,        	/* Private variables in <r1>: */
			/* <v2>, <v3>, ... */
CCM_SH_O_VAR_2,        	/* Shared variables in <r1>: */
			/* <v2>, <v3>, ... */
CCM_TP_O_VAR_2,        	/* Threadprivate variables in <r1>: */
			/* <v2>, <v3>, ... */
CCM_RV_O_VAR_2,        	/* Reduction variables of operator <s1> in <r2>: */
			/* <v3>, <v4>, ... */
CCM_IM_O_VAR_2,        	/* Implicit variables in <r1>: */
			/* <v2>, <v3>, ... */
CCM_UNPAR_IN_OMP_2,    	/* <l1> not parallelized because it is inside */
			/* OpenMP region <r2> */
CCM_FP_O_VAR_2,        	/* Firstprivate variables in <r1>: */
			/* <v2>, <v3>, ... */
CCM_LP_O_VAR_2,        	/* Lastprivate variables in <r1>: */
			/* <v2>, <v3>, ... */
CCM_CP_O_VAR_2,        	/* Copyprivate variables in <r1>: */
			/* <v2>, <v3>, ... */
CCM_PR_OAS_VAR_2,      	/* Variables autoscoped as PRIVATE in <r1>: */
			/* <v2>, <v3>, ... */
CCM_SH_OAS_VAR_2,      	/* Variables autoscoped as SHARED in <r1>: */
			/* <v2>, <v3>, ... */
CCM_FP_OAS_VAR_2,      	/* Variables autoscoped as FIRSTPRIVATE in <r1>: */
			/* <v2>, <v3>, ... */
CCM_LP_OAS_VAR_2,      	/* Variables autoscoped as LASTPRIVATE in <r1>: */
			/* <v2>, <v3>, ... */
CCM_RV_OAS_VAR_2,      	/* Variables autoscoped as REDUCTION of operator */
			/* <s1> in <r2>: <v3>, <v4>, ... */
CCM_FAIL_OAS_VAR_2,    	/* Variables treated as shared because they cannot */
			/* be autoscoped in <r1>: <v2>, <v3>, ... */
CCM_SERIALIZE_OAS_2,   	/* <r1> will be executed by a single thread because */
			/* autoscoping for some variables was not successful */

  /* Group: Parallelization Questions asked of the user */
  /*	How will the user answer these questions? */
CCM_QPERMVEC=0x00800,  	/* Is <v1> a permutation vector during execution of */
			/* <l2>? */
CCM_QEXPR,             	/* Is expression <s1> true for <l2>? */
CCM_QSAFECALL,         	/* Is subroutine <p1> MP-safe as used in <l2>? */

  /* Group: Loop Optimization Messages */
CCM_LCOST=0x01000,     	/* Loop below estimated to cost <i1> cycles per */
			/* iteration */
CCM_UNROLL,            	/* Loop below unrolled <i1> times */
  /* CCMV_WANT: the next one should be replaced by CCM_IMIX2 */
CCM_IMIX,              	/* Loop below has <i1> loads, <i2> stores, */
			/* <i3> prefetches, <i4> FPadds, <i5> FPmuls, and */
			/* <i6> FPdivs per iteration */
CCM_SPILLS,            	/* Loop below required <i1> integer register spills, */
			/* <i2> FP register spills, and used */
			/* <i3> integer registers and <i4> FP registers */
CCM_LFISSION,          	/* Loop below fissioned into <i1> loops */
CCM_LPEEL,             	/* Loop below had iterations peeled off for better */
			/* unrolling and/or parallelization */
CCM_LBLOCKED,          	/* Loop below blocked by <i1> for improved cache */
			/* performance */
CCM_LTILED,            	/* Loop below tiled for better performance */
CCM_LUNRJAM,           	/* Loop below unrolled and jammed */
CCM_LWHILE2DO,         	/* Bounds test for loop below moved to top of loop */
CCM_L2CALL,            	/* Loop below replaced by a call to <p1> */
CCM_LDEAD,             	/* Loop below deleted as dead code */
CCM_LINTRCHNG,         	/* Loop below interchanged with loop on line <i1> */
CCM_FUSEDTO,           	/* Loop below fused with loop on line <i1> */
CCM_FUSEDFROM,         	/* Loop from line <i1> fused with loop below */
CCM_VECINTRNSC,        	/* Loop below transformed to use calls to vector */
			/* intrinsic <p1>, <p2>, ... */
			/* [The number of parameters will determine how many */
			/* names appear, and the formatter will get the */
			/* commas right.] */
CCM_LSTRIPMINE,        	/* Loop below strip-mined */
CCM_LNEST2LOOPS,       	/* Loop below collapsed with loop on line <i1> */
CCM_LREVERSE,          	/* Loop below has had its iteration direction */
			/* reversed */
CCM_IMIX2,             	/* Loop below has <i1> loads, <i2> stores, */
			/* <i3> prefetches, <i4> FPadds, <i5> FPmuls, */
			/* <i6> FPdivs, <i7> FPsubs, and <i8> FPsqrts per */
			/* iteration */
CCM_LUNRFULL,          	/* Loop below fully unrolled */
CCM_ELIM_NOAMORTINST,  	/* Loop below was eliminated as it contains no */
			/* non-amortizable instructions */
CCM_COMP_DALIGN,       	/* Performance of loop below could be improved */
			/* by compiling with -dalign */
CCM_INTIMIX,           	/* Loop below has <i1> int-loads, <i2> int-stores, */
			/* <i3> alu-ops, <i4> muls, <i5> int-divs and */
			/* <i6> shifts per iteration */
CCM_LMULTI_VERSION,    	/* <l1> multi-versioned.  Specialized version */
			/* is <l2> */
CCM_LCOST_2,           	/* <l1> estimated to cost <i2> cycles per iteration */
CCM_UNROLL_2,          	/* <l1> unrolled <i2> times */

  /* CCMV_WANT: the next one should be replaced by CCM_IMIX2_B or CCM_IMIX3_B */
CCM_IMIX_B,            	/* <l1> has <i2> loads, <i3> stores, */
			/* <i4> prefetches, <i5> FPadds, <i6> FPmuls, and */
			/* <i7> FPdivs per iteration */
CCM_SPILLS_2,          	/* <l1> required <i2> integer register spills, */
			/* <i3> FP register spills, and used */
			/* <i4> integer registers and <i5> FP registers */
CCM_LFISSION_2,        	/* <l1> fissioned into <i2> loops, generating: */
			/* <l3>, <l4>, ... */
			/* [The number of parameters will determine how many */
			/* names appear, and the formatter will get the */
			/* commas right.] */
CCM_LFISSION_FRAG,     	/* <l1> contains code from lines: <i2>, <i3>, ... */
CCM_LPEEL_2,           	/* <l1> had iterations peeled off for better */
			/* unrolling and/or parallelization */
CCM_LBLOCKED_2,        	/* <l1> blocked by <i2> for improved memory */
			/* hierarchy performance, new inner loop <l3> */
CCM_LOUTER_UNROLL,     	/* <l1> is outer-unrolled <i2> times as part */
			/* of unroll and jam */
CCM_LJAMMED,           	/* All <i1> copies of <l2> are fused together */
			/* as part of unroll and jam */
CCM_LWHILE2DO_2,       	/* Bounds test for <l1> moved to top of loop */
CCM_L2CALL_2,          	/* <l1> replaced by a call to <p2> */
CCM_LDEAD_2,           	/* <l1> deleted as dead code */
CCM_LINTRCHNG_2,       	/* <l1> interchanged with <l2> */
CCM_LINTRCHNG_ORDER,   	/* For loop nest below, the final order of loops */
			/* after interchanging and subsequent */
			/* transformations is: <l1>, <l2>, ... */
			/* [The number of parameters will determine how many */
			/* names appear, and the formatter will get the */
			/* commas right.] */
CCM_FUSED_2,           	/* <l1> fused with <l2>, new loop <l3> */
CCM_VECINTRNSC_2,      	/* <l1> transformed to use calls to vector */
			/* intrinsics: <p2>, <p3>, ... */
CCM_LSTRIPMINE_2,      	/* <l1> strip-mined by <i2>, new inner loop <l3> */
CCM_LNEST2LOOPS_2,     	/* <l1> collapsed with <l2>, new loop <l3> */
CCM_LREVERSE_2,        	/* <l1> has had its iteration direction reversed */
CCM_IMIX2_B,           	/* <l1> has <i2> loads, <i3> stores, */
			/* <i4> prefetches, <i5> FPadds, <i6> FPmuls, */
			/* <i7> FPdivs, <i8> FPsubs, and <i9> FPsqrts per */
			/* iteration */
CCM_LUNRFULL_2,        	/* <l1> fully unrolled */
CCM_ELIM_NOAMORTINST_2,	/* <l1> was eliminated as it contains no */
			/* non-amortizable instructions */
CCM_COMP_DALIGN_2,     	/* Performance of <l1> could be improved by */
			/* compiling with -dalign */
CCM_INTIMIX_2,         	/* <l1> has <i2> int-loads, <i3> int-stores, */
			/* <i4> alu-ops, <i5> muls, <i6> int-divs and */
			/* <i7> shifts per iteration */
CCM_OMP_REGION,        	/* Source OpenMP region below has tag <r1> */
CCM_LMICROVECTORIZE,   	/* <l1> is micro-vectorized */
CCM_LMULTI_VERSION_2,  	/* <l1> multi-versioned for <s2>. */
			/* Specialized version is <l3> */
CCM_LCLONED,           	/* <l1> cloned for <s2>.  Clone is <l3> */
CCM_LUNSWITCHED,       	/* <l1> is unswitched.  New loops */
			/* are <l2> and <l3> */
CCM_LRESWITCHED,       	/* Loops <l1> and <l2> and their surrounding */
			/* conditional code have been merged to */
			/* form loop <l3> */
CCM_LSKEWBLOCKED,      	/* <l1> skew-blocked by <i2> with slope */
			/* <i3> for improved memory hierarchy */
			/* performance, new inner loop <l4> */
CCM_IVSUB,             	/* Induction variable substitution performed on <l1> */
CCM_ONEITER_REPLACED,  	/* <l1> determined to have a trip count of 1; */
			/* converted to straight-line code */
CCM_IMIX3_B,           	/* <l1> has <i2> loads, <i3> stores, */
			/* <i4> prefetches, <i5> FPadds, <i6> FPmuls, */
			/* <i7> FPmuladds, <i8> FPdivs, and <i9> FPsqrts per */
			/* iteration */

  /* Group: Pipelining Messages */
CCM_PIPELINE=0x02000,  	/* Loop below pipelined */
CCM_PIPESTATS,         	/* Loop below scheduled with steady-state cycle */
			/* count = <i1> */
CCM_NOPIPE_CALL,       	/* Loop could not be pipelined because it contains */
			/* calls */
CCM_NOPIPE_INTCC,      	/* Loop could not be pipelined because it sets */
			/* multiple integer condition codes. */
CCM_NOPIPE_MBAR,       	/* Loop could not be pipelined because it contains a */
			/* memory barrier instruction */
CCM_NOPIPE_MNMX,       	/* Loop could not be pipelined because it contains */
			/* a minimum or a maximum operation */
CCM_NOPIPE_U2FLT,      	/* Loop could not be pipelined because it contains */
			/* an unsigned to float conversion */
CCM_NOPIPE_GOT,        	/* Loop could not be pipelined because it sets the */
			/* Global Offset Table pointer */
CCM_NOPIPE_IDIV,       	/* Loop could not be pipelined because it contains */
			/* an integer divide */
CCM_NOPIPE_PRFTCH,     	/* Loop could not be pipelined because it contains */
			/* a prefetch operation */
CCM_NOPIPE_EXIT,       	/* Loop could not be pipelined because it contains */
			/* an exit operation */
CCM_NOPIPE_REG,        	/* Loop could not be pipelined because it contains */
			/* instructions that set the %gsr or %fsr register */
CCM_NOPIPE_UNS,        	/* Loop could not be pipelined because it has an */
			/* unsigned loop counter */
CCM_NOPIPE_UNSUIT,     	/* Loop was unsuitable for pipelining */
CCM_NOPIPE_INTRINSIC,  	/* Loop could not be pipelined because it has an */
			/* intrinsic call to <p1> */
CCM_NOPIPE_BIG,        	/* Loop could not be pipelined as it is too big */
CCM_NOPIPE_INVINTPR,   	/* Loop could not be pipelined as it contains too */
			/* many loop invariant integers = <i1> */
CCM_NOPIPE_INVFLTPR,   	/* Loop could not be pipelined as it contains too */
			/* many loop invariant floats = <i1> */
CCM_NOPIPE_INVDBLPR,   	/* Loop could not be pipelined as it contains too */
			/* many loop invariant doubles = <i1> */
CCM_PIPE_SCHEDAFIPR,   	/* Loop below was adversely affected by high */
			/* integer register pressure = <i1> */
CCM_PIPE_SCHEDAFDPR,   	/* Loop below was adversely affected by high */
			/* double register pressure = <i1> */
CCM_PIPE_SCHEDAFFPR,   	/* Loop below was adversely affected by high */
			/* float register pressure = <i1> */
CCM_NOPIPE_INTPR,      	/* Loop could not be pipelined due to high */
			/* integer register pressure = <i1> */
CCM_NOPIPE_DBLPR,      	/* Loop could not be pipelined due to high */
			/* double register pressure = <i1> */
CCM_NOPIPE_FLTPR,      	/* Loop could not be pipelined due to high */
			/* float register pressure = <i1> */
CCM_PIPELINE_2,        	/* <l1> pipelined */
CCM_PIPESTATS_2,       	/* <l1> scheduled with steady-state cycle */
			/* count = <i2> */
CCM_NOPIPE_CALL_2,     	/* <l1> could not be pipelined because it contains */
			/* calls */
CCM_NOPIPE_INTCC_2,    	/* <l1> could not be pipelined because it sets */
			/* multiple integer condition codes. */
CCM_NOPIPE_MBAR_2,     	/* <l1> could not be pipelined because it contains */
			/* a memory barrier instruction */
CCM_NOPIPE_MNMX_2,     	/* <l1> could not be pipelined because it contains */
			/* a minimum or a maximum operation */
CCM_NOPIPE_U2FLT_2,    	/* <l1> could not be pipelined because it contains */
			/* an unsigned to float conversion */
CCM_NOPIPE_GOT_2,      	/* <l1> could not be pipelined because it sets the */
			/* Global Offset Table pointer */
CCM_NOPIPE_IDIV_2,     	/* <l1> could not be pipelined because it contains */
			/* an integer divide */
CCM_NOPIPE_PRFTCH_2,   	/* <l1> could not be pipelined because it contains */
			/* a prefetch operation */
CCM_NOPIPE_EXIT_2,     	/* <l1> could not be pipelined because it contains */
			/* an exit operation */
CCM_NOPIPE_REG_2,      	/* <l1> could not be pipelined because it contains */
			/* instructions that set the %gsr or %fsr register */
CCM_NOPIPE_UNS_2,      	/* <l1> could not be pipelined because it has an */
			/* unsigned loop counter */
CCM_NOPIPE_UNSUIT_2,   	/* <l1> is unsuitable for pipelining */
CCM_NOPIPE_INTRINSIC_2,	/* <l1> could not be pipelined because it contains */
			/* a call to intrinsic <p2> */
CCM_NOPIPE_BIG_2,      	/* <l1> could not be pipelined as it is too big */
CCM_NOPIPE_INVINTPR_2, 	/* <l1> could not be pipelined as it contains too */
			/* many loop invariant integers = <i2> */
CCM_NOPIPE_INVFLTPR_2, 	/* <l1> could not be pipelined as it contains too */
			/* many loop invariant floats = <i2> */
CCM_NOPIPE_INVDBLPR_2, 	/* <l1> could not be pipelined as it contains too */
			/* many loop invariant doubles = <i2> */
CCM_PIPE_SCHEDAFIPR_2, 	/* <l1> was adversely affected by high */
			/* integer register pressure = <i2> */
CCM_PIPE_SCHEDAFDPR_2, 	/* <l1> was adversely affected by high */
			/* double register pressure = <i2> */
CCM_PIPE_SCHEDAFFPR_2, 	/* <l1> was adversely affected by high */
			/* float register pressure = <i2> */
CCM_NOPIPE_INTPR_2,    	/* <l1> could not be pipelined due to high */
			/* integer register pressure = <i2> */
CCM_NOPIPE_DBLPR_2,    	/* <l1> could not be pipelined due to high */
			/* double register pressure = <i2> */
CCM_NOPIPE_FLTPR_2,    	/* <l1> could not be pipelined due to high */
			/* float register pressure = <i2> */

  /* Group: Inlining Messages */
CCM_INLINE=0x04000,    	/* Function <p1> inlined from source file <s2> into */
			/* the code for the following line */
CCM_INLINE2,           	/* Function <p1> inlined from source file <s2> into */
			/* inline copy of function <p3> */
CCM_INLINE_TMPLT,      	/* Function <p1> inlined from template file <s2> */
			/* into the code for the following line */
CCM_INLINE_TMPLT2,     	/* Function <p1> inlined from template file <s2> */
			/* into inline copy of function <p3> */
CCM_INLINE_OUT_COPY,   	/* Out-of-line copy of inlined function <p1> from */
			/* source file <s2> generated */
CCM_NINLINE_REC,       	/* Recursive function <p1> inlined only up to */
			/* depth <i2> */
CCM_NINLINE_NEST,      	/* Function <p1> not inlined because inlining is */
			/* already nested too deeply */
CCM_NINLINE_CMPLX,     	/* Function <p1> not inlined because it contains */
			/* too many operations */
CCM_NINLINE_FB,        	/* Function <p1> not inlined because the */
			/* profile-feedback execution count is too low */
CCM_NINLINE_PAR,       	/* Function <p1> not inlined because it contains */
			/* explicit parallel pragmas */
CCM_NINLINE_OPT,       	/* Function <p1> not inlined because it is */
			/* compiled with optimization level <= 2 */
CCM_NINLINE_USR,       	/* Function <p1> not inlined because either command */
			/* line option or source code pragma prohibited it, */
			/* or it's not safe to inline it */
CCM_NINLINE_AUTO,      	/* Function <p1> not inlined because doing so */
			/* would make automatic storage for <p2> too large */
CCM_NINLINE_CALLS,     	/* Function <p1> not inlined because it contains */
			/* too many calls */
CCM_NINLINE_ACTUAL,    	/* Function <p1> not inlined because it has more */
			/* actual parameters than formal parameters */
CCM_NINLINE_FORMAL,    	/* Function <p1> not inlined because it has more */
			/* formal parameters than actual parameters */
CCM_NINLINE_TYPE,      	/* Function <p1> not inlined because formal */
			/* argument type does not match actual type */
CCM_NINLINE_ATYPE,     	/* Function <p1> not inlined because array formal */
			/* argument does not match reshaped array actual */
			/* argument type */
CCM_NINLINE_RETTYPE,   	/* Function <p1> not inlined because return type */
			/* does not match */
CCM_NINLINE_EXCPT,     	/* Function <p1> not inlined because it */
			/* guarded by an exception handler */
CCM_NINLINE_UNSAFE,    	/* Function <p1> not inlined because it might be */
			/* unsafe (call alloca(), etc) */
CCM_NINLINE_ALIAS,     	/* Function <p1> not inlined because inlining it */
			/* will make the alias analysis in the calling */
			/* function more conservative */
CCM_NINLINE_FEMARK,    	/* Function <p1> not inlined because it contains */
			/* setjmp/longjmp, or indirect goto, etc */
CCM_NINLINE_RAREX,     	/* Function <p1> not inlined because it is known */
			/* to be rarely executed */
CCM_CLONING,           	/* Function <p1> from source file <s2> cloned, */
			/* creating cloned function <p3>; constant */
			/* parameters propagated to clone */
CCM_INLINE_B,          	/* Function <p1> inlined from source file <s2> into */
			/* the code for the following line.  <i3> loops */
			/* inlined */
CCM_INLINE2_B,         	/* Function <p1> inlined from source file <s2> into */
			/* inline copy of function <p3>.  <i4> loops inlined */
CCM_INLINE_LOOP,       	/* Loop in function <p1>, line <i2> has */
			/* tag <l3> */
CCM_NINLINE_MULTIENTRY,	/* Function <p1> not inlined because it */
			/* contains an ENTRY statement */
CCM_NINLINE_VARARGS,   	/* Function <p1> not inlined because variable */
			/* argument routines cannot be inlined */
CCM_NINLINE_UNSEEN_BODY,	/* Function <p1> not inlined because the compiler */
			/* has not seen the body of the function.  Use */
			/* -xcrossfile or -xipo in order to inline it */
CCM_NINLINE_UPLEVEL,   	/* Function <p1> not inlined because it is a */
			/* nested routine containing references to */
			/* variables defined in an outer function */
CCM_NINLINE_CMDLINE,   	/* Function <p1> not inlined because either */
			/* -xinline or source code pragma prohibited it */
CCM_NINLINE_CALL_CMPLX,	/* Call to <p1> not inlined because of the */
			/* complexity of the calling routine */
CCM_NINLINE_LANG_MISMATCH,	/* Call to <p1> not inlined because it is in */
			/* a different language */
CCM_NINLINE_RTN_WEAK,  	/* Function <p1> not inlined because it */
			/* is marked weak */
CCM_NINLINE_CALL_WEAKFILE,	/* Call to <p1> not inlined because it is */
			/* in a different file and it contains a */
			/* call to a weak routine */
CCM_NINLINE_CALL_TRYCATCH,	/* Call to <p1> not inlined because it is */
			/* in a different file and contains an */
			/* explicit try/catch */
CCM_NINLINE_CALL_REGP, 	/* Call to <p1> not inlined because it would */
			/* cause excessive register pressure */
CCM_NINLINE_RTN_REGP,  	/* Function <p1> not inlined because it would */
			/* cause excessive register pressure */
CCM_NINLINE_CALL_XPENSV,	/* Call to <p1> not inlined because analysis */
			/* exceeds the compilation time limit */
CCM_NINLINE_READONLYIR,	/* Function <p1> not inlined because it is in a file */
			/* specified as read-only by -xipo_archive=readonly */
			/* and it contains calls to static functions */
CCM_NINLINE_CALL_THUNK,	/* Call to <p1> not inlined because it is in a */
			/* compiler-generated function that does not */
			/* permit inlining */
CCM_NINLINE_CALL_XTARGETS,	/* Indirect callsite has too many targets; */
			/* callsite marked do not inline */
CCM_NINLINE_SELFTAIL_RECURSIVE,	/* Function <p1> not inlined because */
			/* of a recursive tail-call to itself */
CCM_NINLINE_PRAGMA,    	/* Function <p1> not inlined because it contains */
			/* explicit parallel or alias pragmas */
CCM_NINLINE_CMPLX2,    	/* Function <p1> not inlined because it contains too */
			/* many operations.  Increase max_inst_hard in order */
			/* to inline it: -xinline_param=max_inst_hard:n */
CCM_NINLINE_RARE,      	/* Function <p1> not inlined because the call */
			/* is rarely executed */
CCM_NINLINE_PAR2,      	/* Function <p1> not inlined because it is called */
			/* within a region guarded by an explicit */
			/* parallel pragmas */
CCM_NINLINE_G_LIMIT,   	/* Function <p1> not inlined because it would exceed */
			/* the permitted global code size growth limit.  Try */
			/* to increase max_growth in order to inline it: */
			/* -xinline_param=max_growth:n */
CCM_NINLINE_L_LIMIT,   	/* Function <p1> not inlined because it would exceed */
			/* the maximum function size growth limit.  Increase */
			/* max_function_inst in order to inline it: */
			/* -xinline_param=max_function_inst:n */
CCM_NINLINE_REC2,      	/* Recursive function <p1> is inlined only up to */
			/* <i2> levels and up to <i3> size.  Increase */
			/* max_recursive_deptha or max_recursive_inst in */
			/* order to inline it: */
			/* -xinline_param=max_recursive_depth:n, */
			/* -xinline_param=max_recursive_inst:n */
CCM_NINLINE_FB2,       	/* Function <p1> not inlined because the */
			/* profile-feedback execution count is too */
			/* low.  Decrease min_counter in order to inline it: */
			/* -xinline_param:min_counter:n */
CCM_NINLINE_CS_CMPLX,  	/* Function <p1> not inlined because called */
			/* function's size is too big.  Increase */
			/* max_inst_soft in order to inline it: */
			/* -xinline_param=max_inst_soft:n */
CCM_NINLINE_R_EXCPT,   	/* Function <p1> not inlined because it contains */
			/* an exception handler */
CCM_NINLINE_ASM,       	/* Function <p1> not inlined because */
			/* it contains asm statements */
CCM_NINLINE_R_READONLYIR,	/* Function <p1> not inlined because it is in a file */
			/* specified as read-only by -xipo_archive=readonly */
			/* and it is a static function */
CCM_NINLINE_C_READONLYIR,	/* Call to <p1> not inlined because the calling */
			/* function is in a file specified as read-only */
			/* by -xipo_archive=readonly */
CCM_NINLINE_NEVERRETURN,	/* Function <p1> not inlined because it */
			/* never returns */

  /* Group: Messages Concerning Memory Operations */
  /*	Notes: */
  /*	a.  In all of these, <s1> is a string that is something like */
  /*	"A(i+5*k)" or "structure.field", giving the high-level */
  /*	construct that is being loaded or stored. */
  /*	 */
  /*	b.  In all of these, <x2> refers to an instruction offset, */
  /*	expressed as a 32-bit signed integer.  It is assumed */
  /*	that any prefetches will be within this range of the */
  /*	load/store they are prefetching for. */
CCM_MPREFETCH=0x08000, 	/* Prefetch of <s1> inserted */
			/* [This message has a lineno for the source, */
			/* but no instaddr for the disassembly.] */
CCM_MPREFETCH_LD,      	/* Prefetch of <s1> inserted for load at <x2> */
			/* [This message has lineno = -1, */
			/* and is for disassembly only] */
CCM_MPREFETCH_ST,      	/* Prefetch of <s1> inserted for store at <x2> */
			/* [This message has lineno = -1, */
			/* and is for disassembly only] */
CCM_MPREFETCH_FB,      	/* Prefetch of <s1> inserted based on feedback data */
			/* [This message has a lineno for the source, */
			/* but no instaddr for the disassembly.] */
CCM_MPREFETCH_FB_LD,   	/* Prefetch of <s1> inserted for load at <x2> based */
			/* on feedback data */
			/* [This message has lineno = -1, */
			/* and is for disassembly only] */
CCM_MPREFETCH_FB_ST,   	/* Prefetch of <s1> inserted for store at <x2> based */
			/* on feedback data */
			/* [This message has lineno = -1, */
			/* and is for disassembly only] */
CCM_MLOAD,             	/* Load below refers to <s1> */
			/* [This message has lineno = -1, */
			/* and is for disassembly only] */
CCM_MSTORE,            	/* Store below refers to <s1> */
			/* [This message has lineno = -1, */
			/* and is for disassembly only] */
CCM_MLOAD_P,           	/* Load below refers to <s1>, and was prefetched */
			/* at <x2> */
			/* [This message has lineno = -1, */
			/* and is for disassembly only] */
CCM_MSTORE_P,          	/* Store below refers to <s1>, and was prefetched */
			/* at <x2> */
			/* [This message has lineno = -1, */
			/* and is for disassembly only] */

  /* Group: Front-end messages [all compilers] */
  /* Group: F95 Front-end Messages */
CCM_COPYIN=0x10000,    	/* Parameter <i1> caused a copyin in the following */
			/* call */
CCM_COPYOUT,           	/* Parameter <i1> caused a copyout in the following */
			/* call */
CCM_COPYINOUT,         	/* Parameter <i1> caused both a copyin and copyout */
			/* in the following call */
CCM_PADDING,           	/* Padding of <i1> bytes inserted before */
			/* array <v2> */
CCM_PADCOMMON,         	/* Padding of <i1> bytes inserted before */
			/* array <v2> in common block <v3> */
CCM_ALIGN_EQ,          	/* Variable/array <v1> can not be double-aligned, */
			/* because it is equivalenced */
CCM_ALIGN_PERF,        	/* Alignment of variables in common block may cause */
			/* performance degradation */
CCM_ALIGN_STRUCT,      	/* Alignment of component <s1> in numeric sequence */
			/* structure <s2> may cause performance degradation */
CCM_TMP_COPY,          	/* Argument <v1> copied to a temporary */
CCM_TMP_COPYM,         	/* Argument <v1> might be copied to a temporary; */
			/* runtime decision made */
CCM_PROC_MISMATCH,     	/* Argument <i1> to subprogram <p2> differs from */
			/* reference on line <i3> */
CCM_PROC_MISMATCH2,    	/* Scalar argument <i1> to subprogram <p2> is */
			/* referred to as an array on line <i3> */
CCM_PROC_MISMATCH3,    	/* Return type/rank from subprogram <p1> differs */
			/* from return on line <i2> */
CCM_DO_EXPR,           	/* DO statement bounds lead to no executions of the */
			/* loop */
CCM_AUTO_BND,          	/* The bounds for automatic variable <v1> are not */
			/* available at all entry points; zero-length */
			/* variable might be allocated */
CCM_LIT_PAD,           	/* The character string literal <s1> padded */
			/* to the length specified for the dummy argument */
CCM_ARRAY_LOOP,        	/* Array statement below generated a loop */
CCM_ARRAY_LOOPNEST,    	/* Array statement below generated <i1> nested loops */
CCM_ALIGN_PERF2,       	/* Alignment of variable <v1> in common block <v2> */
			/* may cause a performance degradation */
CCM_ALIGN_PERF3,       	/* Alignment of variable <v1> in blank common may */
			/* cause a performance degradation */
CCM_IO_LOOP_ARRAY,     	/* I/O implied do item below generated an array */
			/* section */

  /* Group: C++ Front-end Messages */
CCM_TMPCONST,          	/* Implicit invocation of class <s1> constructor for */
			/* temporary */
CCM_TMPDEST,           	/* Implicit invocation of class <s1> destructor for */
			/* temporary */
CCM_DBL_CONST,         	/* Double constant <s1> used in float expression */
CCM_MINLINE,           	/* Function <p1> inlined from source file <s2> by */
			/* front-end */
			/* [This refers to front-end inlining, */
			/* not the backend inlining above.] */
CCM_MINLINE2,          	/* Function <p1> from source file <s2> inlined into */
			/* inline copy of method <p3> by front-end */
			/* [This refers to front-end inlining, */
			/* not the backend inlining above.] */
CCM_MINLINE3,          	/* Function <p1> not inlined because it uses keyword */
			/* <s2> */
CCM_MINLINE4,          	/* Function <p1> not inlined because it is too */
			/* complex */
CCM_TMP_COPYOUT,       	/* Argument <v1> copied from a temporary */
CCM_TMP_COPYOUTM,      	/* Argument <v1> might be copied from a temporary; */
			/* runtime decision made */
CCM_TMP_COPYINOUT,     	/* Argument <v1> copied in and out of a temporary */
CCM_TMP_COPYINOUTM,    	/* Argument <v1> might be copied in and out of */
			/* a temporary; runtime decision made */

  /* Group: C Front-end Messages */
  /* Group: NJC Front-end Messages */
  /* Group: Updated F95 Front-end Messages */
CCM_ARRAY_LOOP_2,      	/* Array statement below generated loop <l1> */
CCM_ARRAY_LOOPNEST_2,  	/* Array statement below generated <i1> nested */
			/* loops: <l2>, <l3>, ... */
			/* [The number of parameters will determine how many */
			/* names appear, and the formatter will get the */
			/* commas right.] */
CCM_IO_LOOP_ARRAY_2,   	/* I/O implied do item below generated an array */
			/* section: <l1> */
CCM_USER_LOOP,         	/* Source loop below has tag <l1> */
CCM_FOUND_LOOP,        	/* Discovered loop below has tag <l1> */
CCM_MFUNCTION_LOOP,    	/* Copy in M-function of loop below has tag <l1> */

  /* Group: Code-generator Messages */
CCM_FSIMPLE=0x20000,   	/* Transformations for fsimple=<i1> applied */
CCM_STACK,             	/* Function <p1> requires <i2> Mbytes of stack */
			/* storage */
CCM_TAILRECUR,         	/* Recursive tail call in <p1> optimized to jump to */
			/* entry point */
CCM_TAILCALL,          	/* Call to function <p1> was tail-call optimized */
CCM_NI_EXIT_OR_PSEUDO, 	/* Template could not be early inlined because it */
			/* contains the pseudo instruction <s1> */
CCM_NI_BAD_UNARY_OPC,  	/* Template could not be early inlined because it */
			/* contains the instruction opcode <s1> */
CCM_NI_INT_LDD_ON_V9,  	/* Template could not be early inlined because it */
			/* contains integer ldd instructions, which are */
			/* deprecated in the v9 architecture */
CCM_NI_LATE_INL_OPC,   	/* Template could not be early inlined because it */
			/* contains the instruction opcode <s1> */
CCM_NI_BAD_IMM_OP,     	/* Template could not be early inlined because the */
			/* relocation or immediate operand <s1> is not well */
			/* understood by the optimizer */
CCM_NI_BAD_STATELEAF,  	/* Template could not be early inlined because it */
			/* references the state register <s1> */
CCM_NI_BAD_ASR_19,     	/* Template could not be early inlined because */
			/* %asr19 is not supported in pre v8plus code */
CCM_NI_BAD_FSR_USE,    	/* Template could not be early inlined because */
			/* references to %fsr can only be optimized when the */
			/* -iaopts flag is used */
CCM_NI_BAD_REGISTER,   	/* Template could not be early inlined because it */
			/* references the register <s1> */
CCM_NI_NO_RET_VAL,     	/* Template could not be early inlined because it */
			/* does not return the value declared */
CCM_NI_DELAY,          	/* Template could not be early inlined because it */
			/* contains a non nop delay slot */
CCM_NI_SCALL,          	/* Template could not be early inlined because it */
			/* calls a function which returns a structure */
CCM_CASE_POSITION,     	/* Case block below was placed at position <i1> */
			/* based on execution frequency */
CCM_CALL_WITH_CODE,    	/* Call to <p1> replaced with inline code.  <i2> */
			/* loops created: <l3>, <l4>, ... */
CCM_NI_BAD_SP_ADDR,    	/* Template could not be early inlined because it */
			/* contains a %sp+reg address */
CCM_NI_BAD_SP_USAGE,   	/* Template could not be early inlined because it */
			/* uses/defines the stack pointer in a non-load/store instruction */
CCM_NI_MIXED_REG_TYPES,	/* Template could not be early inlined because it */
			/* contains register <s1> used as both x-register and register pair */
CCM_LAST
} COMPMSG_ID;
/*
 * The Message Structure
 * Each message is a fixed-length structure as follows:
 */
typedef struct
{
  int64_t instaddr;     /* the PC offset, relative to the .o .text section */
  int32_t lineno;       /* the source line to which it refers */
  COMPMSG_ID msg_type;  /* the specific message index */
  int32_t nparam;       /* number of parameters to this message */
  int32_t param_index;  /* the index of the first parameter */
} compmsg;

#if defined(__cplusplus)
extern "C"
{
#endif
  /*
   * Initializes the data structures, converts the source name to a string,
   * and fills in srcname and version in the header
   */
  void compcom_p_open (char *srcname, int32_t version);

  /*
   * Finds or enters the string s into the string table, and returns the index
   * of the string
   */
  int32_t compcom_p_string (char *s);

  /*
   * Enter the single message.  Any string parameters should have been converted
   * to int32's by calling compcom_p_string()
   */
  void compcom_p_putmsg (int32_t show_bits, int64_t pcoffset, int32_t lineno,
			 COMPMSG_ID m, int32_t nparams);

  /*
   * Whatever is needed to close the section and write it out to the .o
   */
  void compcom_p_finalize ();

#if defined(__cplusplus)
}
#endif

#endif /* _COMP_COM_H */

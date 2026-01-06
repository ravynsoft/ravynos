/* md.h -machine dependent- */

/* Copyright (C) 1987 Free Software Foundation, Inc.

This file is part of Gas, the GNU Assembler.

The GNU assembler is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU Assembler General
Public License for full details.

Everyone is granted permission to copy, modify and redistribute
the GNU Assembler, but only under the conditions described in the
GNU Assembler General Public License.  A copy of this license is
supposed to have been given to you along with the GNU Assembler
so you can know your rights and responsibilities.  It should be
in a file named COPYING.  Among other things, the copyright
notice and this notice must be preserved on all copies.  */

#import <mach/machine.h>
#import "stuff/bytesex.h"
#import "frags.h"
#import "relax.h"
#import "struc-symbol.h"
#import "fixes.h"
#import "read.h"

/* These are the default cputype and cpusubtype for this target MACHINE */
extern const cpu_type_t md_cputype;
extern cpu_subtype_t md_cpusubtype;

/* This is the byte sex for this target MACHINE */
extern const enum byte_sex md_target_byte_sex;

/* These characters start a comment anywhere on the line */
extern const char md_comment_chars[];

/* These characters only start a comment at the beginning of a line */
extern const char md_line_comment_chars[];

/*
 * These characters can be used to separate mantissa decimal digits from 
 * exponent decimal digits in floating point numbers.
 */
extern const char md_EXP_CHARS[];

/*
 * The characters after a leading 0 that means this number is a floating point
 * constant as in 0f123.456 or 0d1.234E-12 (the characters 'f' and 'd' in these
 * case).
 */
extern const char md_FLT_CHARS[];

/*
 * This is the machine dependent pseudo opcode table for this target MACHINE.
 */
extern const pseudo_typeS md_pseudo_table[];

/*
 * This is the machine dependent table that is used to drive the span dependent
 * branch algorithm in relax_section() in layout.c.  See the comments in relax.h
 * on how this table is used.  For machines with all instructions of the same
 * size (RISC machines) this this table is just a zero filled element and not
 * used.
 */
extern const relax_typeS md_relax_table[];

/*
 * md_parse_option() is called from main() in as.c to parse target machine
 * dependent command line options.  This routine returns 0 if it is passed an
 * option that is not recognized non-zero otherwise.
 */
extern int md_parse_option(
    char **argP,
    int *cntP,
    char ***vecP);

/*
 * md_begin() is called from main() in as.c before assembly begins.  It is used
 * to allow target machine dependent initialization.
 */
extern void md_begin(
    void);

/*
 * md_end() is called from main() in as.c after assembly ends.  It is used
 * to allow target machine dependent clean up.
 */
extern void md_end(
    void);

/*
 * md_assemble() is passed a pointer to a string that should be a assembly
 * statement for the target machine.  This routine assembles the string into
 * a machine instruction.
 */
extern void md_assemble(
    char *str);

/*
 * md_atof() turns a string pointed to by input_line_pointer into a floating
 * point constant of type type, and store the appropriate bytes in *litP.
 * The number of LITTLENUMS emitted is stored indirectly through *sizeP.
 * An error message is returned, or a string containg only a '\0' for OK.
 */
extern char *md_atof(
    int type,
    char *litP,
    int *sizeP);

/*
 * md_number_to_chars() is the target machine dependent routine that puts out
 * a binary value of size 8, 4, 2, or 1 bytes into the specified buffer.  This
 * is done in the target machine's byte sex.
 */
extern void md_number_to_chars(
    char *buf,
    signed_expr_t val,
    int n);
/* FROM tc.h line 55 */
void   md_apply_fix3 (fixS *, valueT *, segT);

/*
 * md_number_to_imm() is the target machine dependent routine that puts out
 * a binary value of size 4, 2, or 1 bytes into the specified buffer with
 * reguard to a possible relocation entry (the fixP->fx_r_type field in the fixS
 * structure pointed to by fixP) for the section with the ordinal nsect.  This
 * is done in the target machine's byte sex using it's relocation types.
 */
extern void md_number_to_imm(
    unsigned char *buf,
    signed_expr_t val,
    int n,
    fixS *fixP,
    int nsect);

/*
 * md_estimate_size_before_relax() is called as part of the algorithm in
 * relax_section() in layout.c that drives the span dependent branch algorithm.
 * It is called once for each machine dependent frag to allow things like
 * braches to undefined symbols to be "relaxed" to their maximum size.
 * For machines with all instructions of the same size (RISC machines) this
 * won't ever be called.
 */
extern int md_estimate_size_before_relax(
    fragS *fragP,
    int nsect);

/*
 * md_convert_frag() is called on each machine dependent frag after the span
 * dependent branch algorithm has been run to determine the sizes and addresses
 * of all the fragments.  This routine is to put the bytes inside the fragment
 * and make it conform to the "relaxed" final size.  For machines with all
 * instructions of the same size (RISC machines) this won't ever be called.
 */
extern void md_convert_frag(
    fragS *fragP);

/*
 * md_pcrel_from() returns the PC-relative offset from the given fixup.
 * This is not implemented or used for most targets.
 */
extern int32_t md_pcrel_from(
    const fixS *fixP);

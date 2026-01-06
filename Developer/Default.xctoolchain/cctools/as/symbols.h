/* symbols.h -
   Copyright (C) 1987 Free Software Foundation, Inc.

This file is part of GAS, the GNU Assembler.

GAS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GAS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GAS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#import "struc-symbol.h"
#import "hash.h"

extern struct hash_control *sy_hash;
extern struct obstack notes;
extern symbolS *symbol_rootP;
extern symbolS *symbol_lastP;
extern symbolS abs_symbol;

extern void symbol_begin(
    void);
extern char *fb_label_name(
    int32_t n,
    int32_t augend);
extern void
    local_colon(
    int n);
extern symbolS *symbol_new(
    char *name,
    unsigned char type,
    char other,
    short desc,
    valueT value,
    struct frag *frag);
/* FROM line 58 */
extern symbolS *symbol_create (const char *name,
	       segT segment,
	       valueT valu,
	       fragS *frag);
extern void symbol_assign_index(
    struct symbol *symbolP);
extern void colon(
    char *sym_name,
    int local_colon);
extern void symbol_table_insert(
    struct symbol *symbolP);
extern symbolS *symbol_find_or_make(
    char *name);
extern symbolS *
    symbol_find(
    char *name);
extern symbolS *
    symbol_table_lookup(
    char *name);
extern isymbolS *indirect_symbol_new(
    char *name,
    struct frag *frag,
    uint32_t offset);

/* FROM line 98 */
extern int S_IS_DEFINED (symbolS *);
extern int S_FORCE_RELOC (symbolS *, int);
extern int S_IS_DEBUG (symbolS *);
extern int S_IS_LOCAL (symbolS *);
extern int S_IS_EXTERN (symbolS *);
extern int S_IS_STABD (symbolS *);
extern const char *S_GET_NAME (symbolS *);
extern segT S_GET_SEGMENT (symbolS *);
extern void S_SET_SEGMENT (symbolS *, segT);
extern void S_SET_EXTERNAL (symbolS *);
extern void S_SET_NAME (symbolS *, const char *);
extern void S_CLEAR_EXTERNAL (symbolS *);
extern void S_SET_WEAK (symbolS *);
extern void S_SET_THREAD_LOCAL (symbolS *);
extern int S_IS_LOCAL (symbolS *s);
extern fragS * symbol_get_frag (symbolS *s);

/* FROM line tc-arm.h 104 */
#define TC_SYMFIELD_TYPE 	unsigned int

/* FROM line 210 */
#ifdef TC_SYMFIELD_TYPE
TC_SYMFIELD_TYPE *symbol_get_tc (symbolS *);
void symbol_set_tc (symbolS *, TC_SYMFIELD_TYPE *);
#endif

#ifdef ARM
#define tc_frob_label arm_frob_label
extern void arm_frob_label (symbolS *);
#endif

extern symbolS * symbol_temp_new(
    segT nsect,
    valueT value,
    struct frag *frag);
extern symbolS * symbol_temp_new_now(
    void);
extern symbolS * symbol_temp_make(
    void);
extern void symbol_set_value_now(
    symbolS *sym);

/*
   Copyright (C) 2021-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3.  If not,
   see <http://www.gnu.org/licenses/>.  */


struct yy_buffer_state;


struct yy_buffer_state *yy_scan_string (const char *);
void yy_delete_buffer (struct yy_buffer_state *b);
void get_internal_label (expressionS *label_expr,
			unsigned long label,
			int augend);
int
loongarch_parse_expr (const char *expr,
		      struct reloc_info *reloc_stack_top,
		      size_t max_reloc_num,
		      size_t *reloc_num,
		      offsetT *imm);
bfd_reloc_code_real_type
loongarch_larch_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				   const char *l_r_name);

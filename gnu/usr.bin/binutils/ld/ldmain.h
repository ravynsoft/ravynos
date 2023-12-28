/* ldmain.h -
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

   This file is part of the GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef LDMAIN_H
#define LDMAIN_H

extern char *program_name;
extern const char *ld_sysroot;
extern char *ld_canon_sysroot;
extern int ld_canon_sysroot_len;
extern FILE *saved_script_handle;
extern FILE *previous_script_handle;
extern bool force_make_executable;
extern char *default_target;
extern unsigned int trace_files;
extern bool verbose;
extern bool version_printed;
extern bool demangling;
extern int g_switch_value;
extern const char *output_filename;
extern struct bfd_link_info link_info;
extern int overflow_cutoff_limit;
#if SUPPORT_ERROR_HANDLING_SCRIPT
extern char *error_handling_script;
#endif

#define RELAXATION_DISABLED_BY_DEFAULT	\
  (link_info.disable_target_specific_optimizations < 0)
#define RELAXATION_DISABLED_BY_USER	\
  (link_info.disable_target_specific_optimizations > 1)
#define RELAXATION_ENABLED		\
  (link_info.disable_target_specific_optimizations == 0 \
   || link_info.disable_target_specific_optimizations == 1)
#define RELAXATION_ENABLED_BY_USER		\
  (link_info.disable_target_specific_optimizations == 0)
#define TARGET_ENABLE_RELAXATION		\
  do { link_info.disable_target_specific_optimizations = 1; } while (0)
#define DISABLE_RELAXATION		\
  do { link_info.disable_target_specific_optimizations = 2; } while (0)
#define ENABLE_RELAXATION		\
  do { link_info.disable_target_specific_optimizations = 0; } while (0)

extern void add_ysym (const char *);
extern void add_wrap (const char *);
extern void add_ignoresym (struct bfd_link_info *, const char *);
extern void add_keepsyms_file (const char *);
extern void track_dependency_files (const char *);

#endif

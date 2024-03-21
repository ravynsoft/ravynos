/* bucomm.h -- binutils common include file.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

   This file is part of GNU Binutils.

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

#ifndef _BUCOMM_H
#define _BUCOMM_H

/* In bucomm.c.  */

/* Return the filename in a static buffer.  */
const char *bfd_get_archive_filename (const bfd *);

void bfd_nonfatal (const char *);

void bfd_nonfatal_message (const char *, const bfd *, const asection *,
			   const char *, ...);

void bfd_fatal (const char *) ATTRIBUTE_NORETURN;

void report (const char *, va_list) ATTRIBUTE_PRINTF(1,0);

void fatal (const char *, ...) ATTRIBUTE_PRINTF_1 ATTRIBUTE_NORETURN;

void non_fatal (const char *, ...) ATTRIBUTE_PRINTF_1;

void *bfd_xalloc (bfd *, size_t);

void set_default_bfd_target (void);

void list_matching_formats (char **);

void list_supported_targets (const char *, FILE *);

void list_supported_architectures (const char *, FILE *);

int display_info (void);

void print_arelt_descr (FILE *, bfd *, bool, bool);

char *make_tempname (const char *, int *);
char *make_tempdir (const char *);

bfd_vma parse_vma (const char *, const char *);

off_t get_file_size (const char *);

bool is_valid_archive_path (char const *);

extern char *program_name;

/* In filemode.c.  */
void mode_string (unsigned long, char *);

/* In version.c.  */
extern void print_version (const char *);

/* In rename.c.  */
extern void set_times (const char *, const struct stat *);

extern int smart_rename (const char *, const char *, int,
			 struct stat *, bool);


/* In libiberty.  */
void *xmalloc (size_t);

void *xrealloc (void *, size_t);

#if __GNUC__ >= 7
#define _mul_overflow(a, b, res) __builtin_mul_overflow (a, b, res)
#else
/* Assumes unsigned values.  Careful!  Args evaluated multiple times.  */
#define _mul_overflow(a, b, res) \
  ((*res) = (a), (*res) *= (b), (b) != 0 && (*res) / (b) != (a))
#endif

#endif /* _BUCOMM_H */

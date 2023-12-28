/* Binutils emulation layer.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   Written by Tom Rix, Red Hat Inc.

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

#include "binemul.h"

extern bin_emulation_xfer_type bin_dummy_emulation;

void
ar_emul_usage (FILE *fp)
{
  if (bin_dummy_emulation.ar_usage)
    bin_dummy_emulation.ar_usage (fp);
}

void
ar_emul_default_usage (FILE *fp)
{
  AR_EMUL_USAGE_PRINT_OPTION_HEADER (fp);
  /* xgettext:c-format */
  fprintf (fp, _("  No emulation specific options\n"));
}

bool
ar_emul_append (bfd **after_bfd, char *file_name, const char *target,
		bool verbose, bool flatten)
{
  bfd *new_bfd;

  new_bfd = bfd_openr (file_name, target);
  AR_EMUL_ELEMENT_CHECK (new_bfd, file_name);
  if (bin_dummy_emulation.ar_append)
    return bin_dummy_emulation.ar_append (after_bfd, new_bfd,
					  verbose, flatten);

  return false;
}

bool
ar_emul_append_bfd (bfd **after_bfd, bfd *new_bfd,
		bool verbose, bool flatten)
{
  if (bin_dummy_emulation.ar_append)
    return bin_dummy_emulation.ar_append (after_bfd, new_bfd,
					  verbose, flatten);

  return false;
}

static bool
any_ok (bfd *new_bfd ATTRIBUTE_UNUSED)
{
  return true;
}

bool
do_ar_emul_append (bfd **after_bfd, bfd *new_bfd,
		   bool verbose, bool flatten,
		   bool (*check) (bfd *))
{
  /* When flattening, add the members of an archive instead of the
     archive itself.  */
  if (flatten && bfd_check_format (new_bfd, bfd_archive))
    {
      bfd *elt;
      bool added = false;

      for (elt = bfd_openr_next_archived_file (new_bfd, NULL);
           elt;
           elt = bfd_openr_next_archived_file (new_bfd, elt))
        {
          if (do_ar_emul_append (after_bfd, elt, verbose, true, check))
            {
              added = true;
              after_bfd = &((*after_bfd)->archive_next);
            }
        }

      return added;
    }

  if (!check (new_bfd))
    return false;

  AR_EMUL_APPEND_PRINT_VERBOSE (verbose, bfd_get_filename (new_bfd));

  new_bfd->archive_next = *after_bfd;
  *after_bfd = new_bfd;

  return true;
}

bool
ar_emul_default_append (bfd **after_bfd, bfd *new_bfd,
			bool verbose, bool flatten)
{
  return do_ar_emul_append (after_bfd, new_bfd, verbose, flatten, any_ok);
}

bool
ar_emul_replace (bfd **after_bfd, char *file_name, const char *target,
		 bool verbose)
{
  bfd *new_bfd;

  new_bfd = bfd_openr (file_name, target);
  AR_EMUL_ELEMENT_CHECK (new_bfd, file_name);

  if (bin_dummy_emulation.ar_replace)
    return bin_dummy_emulation.ar_replace (after_bfd, new_bfd,
					   verbose);

  return false;
}

bool
ar_emul_replace_bfd (bfd **after_bfd, bfd *new_bfd,
		 bool verbose)
{
  if (bin_dummy_emulation.ar_replace)
    return bin_dummy_emulation.ar_replace (after_bfd, new_bfd,
					   verbose);

  return false;
}

bool
ar_emul_default_replace (bfd **after_bfd, bfd *new_bfd,
			 bool verbose)
{
  AR_EMUL_REPLACE_PRINT_VERBOSE (verbose, bfd_get_filename (new_bfd));

  new_bfd->archive_next = *after_bfd;
  *after_bfd = new_bfd;

  return true;
}

bool
ar_emul_parse_arg (char *arg)
{
  if (bin_dummy_emulation.ar_parse_arg)
    return bin_dummy_emulation.ar_parse_arg (arg);

  return false;
}

bool
ar_emul_default_parse_arg (char *arg ATTRIBUTE_UNUSED)
{
  return false;
}

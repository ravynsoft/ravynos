/* arsup.c - Archive support for MRI compatibility
   Copyright (C) 1992-2023 Free Software Foundation, Inc.

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


/* Contributed by Steve Chamberlain
   sac@cygnus.com

   This file looks after requests from arparse.y, to provide the MRI
   style librarian command syntax + 1 word LIST.  */

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "filenames.h"
#include "bucomm.h"
#include "arsup.h"

static void map_over_list
  (bfd *, void (*function) (bfd *, bfd *), struct list *);
static void ar_directory_doer (bfd *, bfd *);
static void ar_addlib_doer (bfd *, bfd *);

extern int verbose;
extern int deterministic;

static bfd *obfd;
static char *real_name;
static char *temp_name;
static int temp_fd;
static FILE *outfile;

static void
map_over_list (bfd *arch, void (*function) (bfd *, bfd *), struct list *list)
{
  bfd *head;

  if (list == NULL)
    {
      bfd *next;

      head = arch->archive_next;
      while (head != NULL)
	{
	  next = head->archive_next;
	  function (head, (bfd *) NULL);
	  head = next;
	}
    }
  else
    {
      struct list *ptr;

      /* This may appear to be a baroque way of accomplishing what we
	 want.  however we have to iterate over the filenames in order
	 to notice where a filename is requested but does not exist in
	 the archive.  Ditto mapping over each file each time -- we
	 want to hack multiple references.  */
      for (ptr = list; ptr; ptr = ptr->next)
	{
	  bool found = false;
	  bfd *prev = arch;

	  for (head = arch->archive_next; head; head = head->archive_next)
	    {
	      if (bfd_get_filename (head) != NULL
		  && FILENAME_CMP (ptr->name, bfd_get_filename (head)) == 0)
		{
		  found = true;
		  function (head, prev);
		}
	      prev = head;
	    }
	  if (! found)
	    fprintf (stderr, _("No entry %s in archive.\n"), ptr->name);
	}
    }
}



static void
ar_directory_doer (bfd *abfd, bfd *ignore ATTRIBUTE_UNUSED)
{
  print_arelt_descr(outfile, abfd, verbose, false);
}

void
ar_directory (char *ar_name, struct list *list, char *output)
{
  bfd *arch;

  arch = open_inarch (ar_name, (char *) NULL);
  if (output)
    {
      outfile = fopen(output,"w");
      if (outfile == 0)
	{
	  outfile = stdout;
	  fprintf (stderr,_("Can't open file %s\n"), output);
	  output = 0;
	}
    }
  else
    outfile = stdout;

  map_over_list (arch, ar_directory_doer, list);

  bfd_close (arch);

  if (output)
   fclose (outfile);
}

void
prompt (void)
{
  extern int interactive;

  if (interactive)
    {
      printf ("AR >");
      fflush (stdout);
    }
}

void
maybequit (void)
{
  if (! interactive)
    xexit (9);
}


void
ar_open (char *name, int t)
{
  real_name = xstrdup (name);
  temp_name = make_tempname (real_name, &temp_fd);

  if (temp_name == NULL)
    {
      fprintf (stderr, _("%s: Can't open temporary file (%s)\n"),
	       program_name, strerror(errno));
      maybequit ();
      return;
    }

  obfd = bfd_fdopenw (temp_name, NULL, temp_fd);

  if (!obfd)
    {
      fprintf (stderr,
	       _("%s: Can't open output archive %s\n"),
	       program_name, temp_name);

      maybequit ();
    }
  else
    {
      if (!t)
	{
	  bfd **ptr;
	  bfd *element;
	  bfd *ibfd;

#if BFD_SUPPORTS_PLUGINS	  
	  ibfd = bfd_openr (name, "plugin");
#else
	  ibfd = bfd_openr (name, NULL);
#endif

	  if (!ibfd)
	    {
	      fprintf (stderr,_("%s: Can't open input archive %s\n"),
		       program_name, name);
	      maybequit ();
	      return;
	    }

	  if (!bfd_check_format(ibfd, bfd_archive))
	    {
	      fprintf (stderr,
		       _("%s: file %s is not an archive\n"),
		       program_name, name);
	      maybequit ();
	      return;
	    }

	  ptr = &(obfd->archive_head);
	  element = bfd_openr_next_archived_file (ibfd, NULL);

	  while (element)
	    {
	      *ptr = element;
	      ptr = &element->archive_next;
	      element = bfd_openr_next_archived_file (ibfd, element);
	    }
	}

      bfd_set_format (obfd, bfd_archive);

      obfd->has_armap = 1;
      obfd->is_thin_archive = 0;
    }
}

static void
ar_addlib_doer (bfd *abfd, bfd *prev)
{
  /* Add this module to the output bfd.  */
  if (prev != NULL)
    prev->archive_next = abfd->archive_next;

  abfd->archive_next = obfd->archive_head;
  obfd->archive_head = abfd;
}

void
ar_addlib (char *name, struct list *list)
{
  if (obfd == NULL)
    {
      fprintf (stderr, _("%s: no output archive specified yet\n"), program_name);
      maybequit ();
    }
  else
    {
      bfd *arch;

      arch = open_inarch (name, (char *) NULL);
      if (arch != NULL)
	map_over_list (arch, ar_addlib_doer, list);

      /* Don't close the bfd, since it will make the elements disappear.  */
    }
}

void
ar_addmod (struct list *list)
{
  if (!obfd)
    {
      fprintf (stderr, _("%s: no open output archive\n"), program_name);
      maybequit ();
    }
  else
    {
      while (list)
	{
	  bfd *abfd;

#if BFD_SUPPORTS_PLUGINS	  
	  abfd = bfd_openr (list->name, "plugin");
#else
	  abfd = bfd_openr (list->name, NULL);
#endif
	  if (!abfd)
	    {
	      fprintf (stderr, _("%s: can't open file %s\n"),
		       program_name, list->name);
	      maybequit ();
	    }
	  else
	    {
	      abfd->archive_next = obfd->archive_head;
	      obfd->archive_head = abfd;
	    }
	  list = list->next;
	}
    }
}


void
ar_clear (void)
{
  if (obfd)
    obfd->archive_head = 0;
}

void
ar_delete (struct list *list)
{
  if (!obfd)
    {
      fprintf (stderr, _("%s: no open output archive\n"), program_name);
      maybequit ();
    }
  else
    {
      while (list)
	{
	  /* Find this name in the archive.  */
	  bfd *member = obfd->archive_head;
	  bfd **prev = &(obfd->archive_head);
	  int found = 0;

	  while (member)
	    {
	      if (FILENAME_CMP (bfd_get_filename (member), list->name) == 0)
		{
		  *prev = member->archive_next;
		  found = 1;
		}
	      else
		prev = &(member->archive_next);

	      member = member->archive_next;
	    }

	  if (!found)
	    {
	      fprintf (stderr, _("%s: can't find module file %s\n"),
		       program_name, list->name);
	      maybequit ();
	    }

	  list = list->next;
	}
    }
}

void
ar_save (void)
{
  if (!obfd)
    {
      fprintf (stderr, _("%s: no open output archive\n"), program_name);
      maybequit ();
    }
  else
    {
      struct stat target_stat;

      if (deterministic > 0)
        obfd->flags |= BFD_DETERMINISTIC_OUTPUT;

      temp_fd = dup (temp_fd);
      bfd_close (obfd);

      if (stat (real_name, &target_stat) != 0)
	{
	  /* The temp file created in ar_open has mode 0600 as per mkstemp.
	     Create the real empty output file here so smart_rename will
	     update the mode according to the process umask.  */
	  obfd = bfd_openw (real_name, NULL);
	  if (obfd != NULL)
	    {
	      bfd_set_format (obfd, bfd_archive);
	      bfd_close (obfd);
	    }
	}

      smart_rename (temp_name, real_name, temp_fd, NULL, false);
      obfd = 0;
      free (temp_name);
      free (real_name);
    }
}

void
ar_replace (struct list *list)
{
  if (!obfd)
    {
      fprintf (stderr, _("%s: no open output archive\n"), program_name);
      maybequit ();
    }
  else
    {
      while (list)
	{
	  /* Find this name in the archive.  */
	  bfd *member = obfd->archive_head;
	  bfd **prev = &(obfd->archive_head);
	  int found = 0;

	  while (member)
	    {
	      if (FILENAME_CMP (bfd_get_filename (member), list->name) == 0)
		{
		  /* Found the one to replace.  */
		  bfd *abfd = bfd_openr (list->name, NULL);

		  if (!abfd)
		    {
		      fprintf (stderr, _("%s: can't open file %s\n"),
			       program_name, list->name);
		      maybequit ();
		    }
		  else
		    {
		      *prev = abfd;
		      abfd->archive_next = member->archive_next;
		      found = 1;
		    }
		}
	      else
		{
		  prev = &(member->archive_next);
		}
	      member = member->archive_next;
	    }

	  if (!found)
	    {
	      bfd *abfd = bfd_openr (list->name, NULL);

	      fprintf (stderr,_("%s: can't find module file %s\n"),
		       program_name, list->name);
	      if (!abfd)
		{
		  fprintf (stderr, _("%s: can't open file %s\n"),
			   program_name, list->name);
		  maybequit ();
		}
	      else
		*prev = abfd;
	    }

	  list = list->next;
	}
    }
}

/* And I added this one.  */
void
ar_list (void)
{
  if (!obfd)
    {
      fprintf (stderr, _("%s: no open output archive\n"), program_name);
      maybequit ();
    }
  else
    {
      bfd *abfd;

      outfile = stdout;
      verbose =1 ;
      printf (_("Current open archive is %s\n"), bfd_get_filename (obfd));

      for (abfd = obfd->archive_head;
	   abfd != (bfd *)NULL;
	   abfd = abfd->archive_next)
	ar_directory_doer (abfd, (bfd *) NULL);
    }
}

void
ar_end (void)
{
  if (obfd)
    {
      const char *filename = bfd_get_filename (obfd);
      bfd_close_all_done (obfd);
      unlink (filename);
    }
}

void
ar_extract (struct list *list)
{
  if (!obfd)
    {
      fprintf (stderr, _("%s: no open archive\n"), program_name);
      maybequit ();
    }
  else
    {
      while (list)
	{
	  /* Find this name in the archive.  */
	  bfd *member = obfd->archive_head;
	  int found = 0;

	  while (member && !found)
	    {
	      if (FILENAME_CMP (bfd_get_filename (member), list->name) == 0)
		{
		  extract_file (member);
		  found = 1;
		}

	      member = member->archive_next;
	    }

	  if (!found)
	    {
	      bfd_openr (list->name, NULL);
	      fprintf (stderr, _("%s: can't find module file %s\n"),
		       program_name, list->name);
	    }

	  list = list->next;
	}
    }
}

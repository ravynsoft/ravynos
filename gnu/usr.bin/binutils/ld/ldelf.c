/* ELF emulation code for targets using elf.em.
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

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "filenames.h"
#include "safe-ctype.h"
#include "bfdlink.h"
#include "ctf-api.h"
#include "ld.h"
#include "ldmain.h"
#include "ldmisc.h"
#include "ldexp.h"
#include "ldlang.h"
#include "ldfile.h"
#include "ldemul.h"
#include "ldbuildid.h"
#include <ldgram.h>
#include "elf-bfd.h"
#ifdef HAVE_GLOB
#include <glob.h>
#endif
#include "ldelf.h"
#ifdef HAVE_JANSSON
#include <jansson.h>
#endif

struct dt_needed
{
  bfd *by;
  const char *name;
};

/* Style of .note.gnu.build-id section.  */
const char *ldelf_emit_note_gnu_build_id;

/* Content of .note.package section.  */
const char *ldelf_emit_note_fdo_package_metadata;

/* These variables are required to pass information back and forth
   between after_open and check_needed and stat_needed and vercheck.  */

static struct bfd_link_needed_list *global_needed;
static lang_input_statement_type *global_found;
static struct stat global_stat;
static struct bfd_link_needed_list *global_vercheck_needed;
static bool global_vercheck_failed;

void
ldelf_after_parse (void)
{
  if (bfd_link_pie (&link_info))
    link_info.flags_1 |= (bfd_vma) DF_1_PIE;

  if (bfd_link_executable (&link_info)
      && link_info.nointerp)
    {
      if (link_info.dynamic_undefined_weak > 0)
	einfo (_("%P: warning: -z dynamic-undefined-weak ignored\n"));
      link_info.dynamic_undefined_weak = 0;
    }

  /* Disable DT_RELR if not building PIE nor shared library.  */
  if (!bfd_link_pic (&link_info))
    link_info.enable_dt_relr = 0;

  /* Add 3 spare tags for DT_RELR, DT_RELRSZ and DT_RELRENT.  */
  if (link_info.enable_dt_relr)
    link_info.spare_dynamic_tags += 3;

  after_parse_default ();
  if (link_info.commonpagesize > link_info.maxpagesize)
    {
      if (!link_info.commonpagesize_is_set)
	link_info.commonpagesize = link_info.maxpagesize;
      else if (!link_info.maxpagesize_is_set)
	link_info.maxpagesize = link_info.commonpagesize;
      else
	einfo (_("%F%P: common page size (0x%v) > maximum page size (0x%v)\n"),
	       link_info.commonpagesize, link_info.maxpagesize);
    }
}

/* Handle the generation of DT_NEEDED tags.  */

bool
ldelf_load_symbols (lang_input_statement_type *entry)
{
  int link_class = 0;

  /* Tell the ELF linker that we don't want the output file to have a
     DT_NEEDED entry for this file, unless it is used to resolve
     references in a regular object.  */
  if (entry->flags.add_DT_NEEDED_for_regular)
    link_class = DYN_AS_NEEDED;

  /* Tell the ELF linker that we don't want the output file to have a
     DT_NEEDED entry for any dynamic library in DT_NEEDED tags from
     this file at all.  */
  if (!entry->flags.add_DT_NEEDED_for_dynamic)
    link_class |= DYN_NO_ADD_NEEDED;

  if (entry->flags.just_syms
      && (bfd_get_file_flags (entry->the_bfd) & DYNAMIC) != 0)
    einfo (_("%F%P: %pB: --just-symbols may not be used on DSO\n"),
	   entry->the_bfd);

  if (link_class == 0
      || (bfd_get_file_flags (entry->the_bfd) & DYNAMIC) == 0)
    return false;

  bfd_elf_set_dyn_lib_class (entry->the_bfd,
			     (enum dynamic_lib_link_class) link_class);

  /* Continue on with normal load_symbols processing.  */
  return false;
}

/* On Linux, it's possible to have different versions of the same
   shared library linked against different versions of libc.  The
   dynamic linker somehow tags which libc version to use in
   /etc/ld.so.cache, and, based on the libc that it sees in the
   executable, chooses which version of the shared library to use.

   We try to do a similar check here by checking whether this shared
   library needs any other shared libraries which may conflict with
   libraries we have already included in the link.  If it does, we
   skip it, and try to find another shared library farther on down the
   link path.

   This is called via lang_for_each_input_file.
   GLOBAL_VERCHECK_NEEDED is the list of objects needed by the object
   which we are checking.  This sets GLOBAL_VERCHECK_FAILED if we find
   a conflicting version.  */

static void
ldelf_vercheck (lang_input_statement_type *s)
{
  const char *soname;
  struct bfd_link_needed_list *l;

  if (global_vercheck_failed)
    return;
  if (s->the_bfd == NULL
      || (bfd_get_file_flags (s->the_bfd) & DYNAMIC) == 0)
    return;

  soname = bfd_elf_get_dt_soname (s->the_bfd);
  if (soname == NULL)
    soname = lbasename (bfd_get_filename (s->the_bfd));

  for (l = global_vercheck_needed; l != NULL; l = l->next)
    {
      const char *suffix;

      if (filename_cmp (soname, l->name) == 0)
	{
	  /* Probably can't happen, but it's an easy check.  */
	  continue;
	}

      if (strchr (l->name, '/') != NULL)
	continue;

      suffix = strstr (l->name, ".so.");
      if (suffix == NULL)
	continue;

      suffix += sizeof ".so." - 1;

      if (filename_ncmp (soname, l->name, suffix - l->name) == 0)
	{
	  /* Here we know that S is a dynamic object FOO.SO.VER1, and
	     the object we are considering needs a dynamic object
	     FOO.SO.VER2, and VER1 and VER2 are different.  This
	     appears to be a version mismatch, so we tell the caller
	     to try a different version of this library.  */
	  global_vercheck_failed = true;
	  return;
	}
    }
}


/* See if an input file matches a DT_NEEDED entry by running stat on
   the file.  */

static void
ldelf_stat_needed (lang_input_statement_type *s)
{
  struct stat st;
  const char *suffix;
  const char *soname;

  if (global_found != NULL)
    return;
  if (s->the_bfd == NULL)
    return;

  /* If this input file was an as-needed entry, and wasn't found to be
     needed at the stage it was linked, then don't say we have loaded it.  */
  if ((bfd_elf_get_dyn_lib_class (s->the_bfd) & DYN_AS_NEEDED) != 0)
    return;

  if (bfd_stat (s->the_bfd, &st) != 0)
    {
      einfo (_("%P: %pB: bfd_stat failed: %E\n"), s->the_bfd);
      return;
    }

  /* Some operating systems, e.g. Windows, do not provide a meaningful
     st_ino; they always set it to zero.  (Windows does provide a
     meaningful st_dev.)  Do not indicate a duplicate library in that
     case.  While there is no guarantee that a system that provides
     meaningful inode numbers will never set st_ino to zero, this is
     merely an optimization, so we do not need to worry about false
     negatives.  */
  if (st.st_dev == global_stat.st_dev
      && st.st_ino == global_stat.st_ino
      && st.st_ino != 0)
    {
      global_found = s;
      return;
    }

  /* We issue a warning if it looks like we are including two
     different versions of the same shared library.  For example,
     there may be a problem if -lc picks up libc.so.6 but some other
     shared library has a DT_NEEDED entry of libc.so.5.  This is a
     heuristic test, and it will only work if the name looks like
     NAME.so.VERSION.  FIXME: Depending on file names is error-prone.
     If we really want to issue warnings about mixing version numbers
     of shared libraries, we need to find a better way.  */

  if (strchr (global_needed->name, '/') != NULL)
    return;
  suffix = strstr (global_needed->name, ".so.");
  if (suffix == NULL)
    return;
  suffix += sizeof ".so." - 1;

  soname = bfd_elf_get_dt_soname (s->the_bfd);
  if (soname == NULL)
    soname = lbasename (s->filename);

  if (filename_ncmp (soname, global_needed->name,
		     suffix - global_needed->name) == 0)
    einfo (_("%P: warning: %s, needed by %pB, may conflict with %s\n"),
	   global_needed->name, global_needed->by, soname);
}

/* This function is called for each possible name for a dynamic object
   named by a DT_NEEDED entry.  The FORCE parameter indicates whether
   to skip the check for a conflicting version.  */

static bool
ldelf_try_needed (struct dt_needed *needed, int force, int is_linux)
{
  bfd *abfd;
  const char *name = needed->name;
  const char *soname;
  int link_class;

  abfd = bfd_openr (name, bfd_get_target (link_info.output_bfd));
  if (abfd == NULL)
    {
      if (verbose)
	info_msg (_("attempt to open %s failed\n"), name);
      return false;
    }

  track_dependency_files (name);

  /* Linker needs to decompress sections.  */
  abfd->flags |= BFD_DECOMPRESS;

  if (! bfd_check_format (abfd, bfd_object))
    {
      bfd_close (abfd);
      return false;
    }
  if ((bfd_get_file_flags (abfd) & DYNAMIC) == 0)
    {
      bfd_close (abfd);
      return false;
    }

  /* For DT_NEEDED, they have to match.  */
  if (abfd->xvec != link_info.output_bfd->xvec)
    {
      bfd_close (abfd);
      return false;
    }

  /* Check whether this object would include any conflicting library
     versions.  If FORCE is set, then we skip this check; we use this
     the second time around, if we couldn't find any compatible
     instance of the shared library.  */

  if (!force)
    {
      struct bfd_link_needed_list *needs;

      if (! bfd_elf_get_bfd_needed_list (abfd, &needs))
	einfo (_("%F%P: %pB: bfd_elf_get_bfd_needed_list failed: %E\n"), abfd);

      if (needs != NULL)
	{
	  global_vercheck_needed = needs;
	  global_vercheck_failed = false;
	  lang_for_each_input_file (ldelf_vercheck);
	  if (global_vercheck_failed)
	    {
	      bfd_close (abfd);
	      /* Return FALSE to force the caller to move on to try
		 another file on the search path.  */
	      return false;
	    }

	  /* But wait!  It gets much worse.  On Linux, if a shared
	     library does not use libc at all, we are supposed to skip
	     it the first time around in case we encounter a shared
	     library later on with the same name which does use the
	     version of libc that we want.  This is much too horrible
	     to use on any system other than Linux.  */
	  if (is_linux)
	    {
	      struct bfd_link_needed_list *l;

	      for (l = needs; l != NULL; l = l->next)
		if (startswith (l->name, "libc.so"))
		  break;
	      if (l == NULL)
		{
		  bfd_close (abfd);
		  return false;
		}
	    }
	}
    }

  /* We've found a dynamic object matching the DT_NEEDED entry.  */

  /* We have already checked that there is no other input file of the
     same name.  We must now check again that we are not including the
     same file twice.  We need to do this because on many systems
     libc.so is a symlink to, e.g., libc.so.1.  The SONAME entry will
     reference libc.so.1.  If we have already included libc.so, we
     don't want to include libc.so.1 if they are the same file, and we
     can only check that using stat.  */

  if (bfd_stat (abfd, &global_stat) != 0)
    einfo (_("%F%P: %pB: bfd_stat failed: %E\n"), abfd);

  /* First strip off everything before the last '/'.  */
  soname = lbasename (bfd_get_filename (abfd));

  if (verbose)
    info_msg (_("found %s at %s\n"), soname, name);

  global_found = NULL;
  lang_for_each_input_file (ldelf_stat_needed);
  if (global_found != NULL)
    {
      /* Return TRUE to indicate that we found the file, even though
	 we aren't going to do anything with it.  */
      return true;
    }

  /* Specify the soname to use.  */
  bfd_elf_set_dt_needed_name (abfd, soname);

  /* Tell the ELF linker that we don't want the output file to have a
     DT_NEEDED entry for this file, unless it is used to resolve
     references in a regular object.  */
  link_class = DYN_DT_NEEDED;

  /* Tell the ELF linker that we don't want the output file to have a
     DT_NEEDED entry for this file at all if the entry is from a file
     with DYN_NO_ADD_NEEDED.  */
  if (needed->by != NULL
      && (bfd_elf_get_dyn_lib_class (needed->by) & DYN_NO_ADD_NEEDED) != 0)
    link_class |= DYN_NO_NEEDED | DYN_NO_ADD_NEEDED;

  bfd_elf_set_dyn_lib_class (abfd, (enum dynamic_lib_link_class) link_class);

  *link_info.input_bfds_tail = abfd;
  link_info.input_bfds_tail = &abfd->link.next;

  /* Add this file into the symbol table.  */
  if (! bfd_link_add_symbols (abfd, &link_info))
    einfo (_("%F%P: %pB: error adding symbols: %E\n"), abfd);

  return true;
}

/* Search for a needed file in a path.  */

static bool
ldelf_search_needed (const char *path, struct dt_needed *n, int force,
		     int is_linux, int elfsize)
{
  const char *s;
  const char *name = n->name;
  size_t len;
  struct dt_needed needed;

  if (name[0] == '/')
    return ldelf_try_needed (n, force, is_linux);

  if (path == NULL || *path == '\0')
    return false;

  needed.by = n->by;
  needed.name = n->name;

  len = strlen (name);
  while (1)
    {
      unsigned offset = 0;
      char * var;
      char *filename, *sset;

      s = strchr (path, config.rpath_separator);
      if (s == NULL)
	s = path + strlen (path);

#if HAVE_DOS_BASED_FILE_SYSTEM
      /* Assume a match on the second char is part of drive specifier.  */
      else if (config.rpath_separator == ':'
	       && s == path + 1
	       && ISALPHA (*path))
	{
	  s = strchr (s + 1, config.rpath_separator);
	  if (s == NULL)
	    s = path + strlen (path);
	}
#endif
      filename = (char *) xmalloc (s - path + len + 2);
      if (s == path)
	sset = filename;
      else
	{
	  memcpy (filename, path, s - path);
	  filename[s - path] = '/';
	  sset = filename + (s - path) + 1;
	}
      strcpy (sset, name);

      /* PR 20535: Support the same pseudo-environment variables that
	 are supported by ld.so.  Namely, $ORIGIN, $LIB and $PLATFORM.
	 Since there can be more than one occurrence of these tokens in
	 the path we loop until no more are found.  Since we might not
	 be able to substitute some of the tokens we maintain an offset
	 into the filename for where we should begin our scan.  */
      while ((var = strchr (filename + offset, '$')) != NULL)
	{
	  /* The ld.so manual page does not say, but I am going to assume that
	     these tokens are terminated by a directory separator character
	     (/) or the end of the string.  There is also an implication that
	     $ORIGIN should only be used at the start of a path, but that is
	     not enforced here.

	     The ld.so manual page also states that it allows ${ORIGIN},
	     ${LIB} and ${PLATFORM}, so these are supported as well.

	     FIXME: The code could be a lot cleverer about allocating space
	     for the processed string.  */
	  char *    end = strchr (var, '/');
	  const char *replacement = NULL;
	  char *    v = var + 1;
	  char *    freeme = NULL;
	  unsigned  flen = strlen (filename);

	  if (end != NULL)
	    /* Temporarily terminate the filename at the end of the token.  */
	    * end = 0;

	  if (*v == '{')
	    ++ v;
	  switch (*v++)
	    {
	    case 'O':
	      if (strcmp (v, "RIGIN") == 0 || strcmp (v, "RIGIN}") == 0)
		{
		  /* ORIGIN - replace with the full path to the directory
		     containing the program or shared object.  */
		  if (needed.by == NULL)
		    {
		      if (link_info.output_bfd == NULL)
			{
			  break;
			}
		      else
			replacement = bfd_get_filename (link_info.output_bfd);
		    }
		  else
		    replacement = bfd_get_filename (needed.by);

		  if (replacement)
		    {
		      char * slash;

		      if (replacement[0] == '/')
			freeme = xstrdup (replacement);
		      else
			{
			  char * current_dir = getpwd ();

			  freeme = xmalloc (strlen (replacement)
					    + strlen (current_dir) + 2);
			  sprintf (freeme, "%s/%s", current_dir, replacement);
			}

		      replacement = freeme;
		      if ((slash = strrchr (replacement, '/')) != NULL)
			* slash = 0;
		    }
		}
	      break;

	    case 'L':
	      if (strcmp (v, "IB") == 0 || strcmp (v, "IB}") == 0)
		{
		  /* LIB - replace with "lib" in 32-bit environments
		     and "lib64" in 64-bit environments.  */

		  switch (elfsize)
		    {
		    case 32: replacement = "lib"; break;
		    case 64: replacement = "lib64"; break;
		    default:
		      abort ();
		    }
		}
	      break;

	    case 'P':
	      /* Supporting $PLATFORM in a cross-hosted environment is not
		 possible.  Supporting it in a native environment involves
		 loading the <sys/auxv.h> header file which loads the
		 system <elf.h> header file, which conflicts with the
		 "include/elf/mips.h" header file.  */
	      /* Fall through.  */
	    default:
	      break;
	    }

	  if (replacement)
	    {
	      char * filename2 = xmalloc (flen + strlen (replacement));

	      if (end)
		{
		  sprintf (filename2, "%.*s%s/%s",
			   (int)(var - filename), filename,
			   replacement, end + 1);
		  offset = (var - filename) + 1 + strlen (replacement);
		}
	      else
		{
		  sprintf (filename2, "%.*s%s",
			   (int)(var - filename), filename,
			   replacement);
		  offset = var - filename + strlen (replacement);
		}

	      free (filename);
	      filename = filename2;
	      /* There is no need to restore the path separator (when
		 end != NULL) as we have replaced the entire string.  */
	    }
	  else
	    {
	      if (verbose)
		/* We only issue an "unrecognised" message in verbose mode
		   as the $<foo> token might be a legitimate component of
		   a path name in the target's file system.  */
		info_msg (_("unrecognised or unsupported token "
			    "'%s' in search path\n"), var);
	      if (end)
		/* Restore the path separator.  */
		* end = '/';

	      /* PR 20784: Make sure that we resume the scan *after*
		 the token that we could not replace.  */
	      offset = (var + 1) - filename;
	    }

	  free (freeme);
	}

      needed.name = filename;

      if (ldelf_try_needed (&needed, force, is_linux))
	return true;

      free (filename);

      if (*s == '\0')
	break;
      path = s + 1;
    }

  return false;
}

/* Prefix the sysroot to absolute paths in PATH, a string containing
   paths separated by config.rpath_separator.  If running on a DOS
   file system, paths containing a drive spec won't have the sysroot
   prefix added, unless the sysroot also specifies the same drive.  */

static const char *
ldelf_add_sysroot (const char *path)
{
  size_t len, extra;
  const char *p;
  char *ret, *q;
  int dos_drive_sysroot = HAS_DRIVE_SPEC (ld_sysroot);

  len = strlen (ld_sysroot);
  for (extra = 0, p = path; ; )
    {
      int dos_drive = HAS_DRIVE_SPEC (p);

      if (dos_drive)
	p += 2;
      if (IS_DIR_SEPARATOR (*p)
	  && (!dos_drive
	      || (dos_drive_sysroot
		  && ld_sysroot[0] == p[-2])))
	{
	  if (dos_drive && dos_drive_sysroot)
	    extra += len - 2;
	  else
	    extra += len;
	}
      p = strchr (p, config.rpath_separator);
      if (!p)
	break;
      ++p;
    }

  ret = xmalloc (strlen (path) + extra + 1);

  for (q = ret, p = path; ; )
    {
      const char *end;
      int dos_drive = HAS_DRIVE_SPEC (p);

      if (dos_drive)
	{
	  *q++ = *p++;
	  *q++ = *p++;
	}
      if (IS_DIR_SEPARATOR (*p)
	  && (!dos_drive
	      || (dos_drive_sysroot
		  && ld_sysroot[0] == p[-2])))
	{
	  if (dos_drive && dos_drive_sysroot)
	    {
	      strcpy (q, ld_sysroot + 2);
	      q += len - 2;
	    }
	  else
	    {
	      strcpy (q, ld_sysroot);
	      q += len;
	    }
	}
      end = strchr (p, config.rpath_separator);
      if (end)
	{
	  size_t n = end - p + 1;
	  strncpy (q, p, n);
	  q += n;
	  p += n;
	}
      else
	{
	  strcpy (q, p);
	  break;
	}
    }

  return ret;
}

/* Read the system search path the FreeBSD way rather than the Linux way.  */
#ifdef HAVE_ELF_HINTS_H
#include <elf-hints.h>
#else
#include "elf-hints-local.h"
#endif

static bool
ldelf_check_ld_elf_hints (const struct bfd_link_needed_list *l, int force,
			  int elfsize)
{
  static bool initialized;
  static const char *ld_elf_hints;
  struct dt_needed needed;

  if (!initialized)
    {
      FILE *f;
      char *tmppath;

      tmppath = concat (ld_sysroot, _PATH_ELF_HINTS, (const char *) NULL);
      f = fopen (tmppath, FOPEN_RB);
      free (tmppath);
      if (f != NULL)
	{
	  struct elfhints_hdr hdr;

	  if (fread (&hdr, 1, sizeof (hdr), f) == sizeof (hdr)
	      && hdr.magic == ELFHINTS_MAGIC
	      && hdr.version == 1)
	    {
	      if (fseek (f, hdr.strtab + hdr.dirlist, SEEK_SET) != -1)
		{
		  char *b;

		  b = xmalloc (hdr.dirlistlen + 1);
		  if (fread (b, 1, hdr.dirlistlen + 1, f) ==
		      hdr.dirlistlen + 1)
		    ld_elf_hints = ldelf_add_sysroot (b);

		  free (b);
		}
	    }
	  fclose (f);
	}

      initialized = true;
    }

  if (ld_elf_hints == NULL)
    return false;

  needed.by = l->by;
  needed.name = l->name;
  return ldelf_search_needed (ld_elf_hints, &needed, force, false, elfsize);
}

/* For a native linker, check the file /etc/ld.so.conf for directories
   in which we may find shared libraries.  /etc/ld.so.conf is really
   only meaningful on Linux.  */

struct ldelf_ld_so_conf
{
  char *path;
  size_t len, alloc;
};

static bool
ldelf_parse_ld_so_conf (struct ldelf_ld_so_conf *, const char *);

static void
ldelf_parse_ld_so_conf_include (struct ldelf_ld_so_conf *info,
				const char *filename,
				const char *pattern)
{
  char *newp = NULL;
#ifdef HAVE_GLOB
  glob_t gl;
#endif

  if (pattern[0] != '/')
    {
      char *p = strrchr (filename, '/');
      size_t patlen = strlen (pattern) + 1;

      newp = xmalloc (p - filename + 1 + patlen);
      memcpy (newp, filename, p - filename + 1);
      memcpy (newp + (p - filename + 1), pattern, patlen);
      pattern = newp;
    }

#ifdef HAVE_GLOB
  if (glob (pattern, 0, NULL, &gl) == 0)
    {
      size_t i;

      for (i = 0; i < gl.gl_pathc; ++i)
	ldelf_parse_ld_so_conf (info, gl.gl_pathv[i]);
      globfree (&gl);
    }
#else
  /* If we do not have glob, treat the pattern as a literal filename.  */
  ldelf_parse_ld_so_conf (info, pattern);
#endif

  free (newp);
}

static bool
ldelf_parse_ld_so_conf (struct ldelf_ld_so_conf *info, const char *filename)
{
  FILE *f = fopen (filename, FOPEN_RT);
  char *line;
  size_t linelen;

  if (f == NULL)
    return false;

  linelen = 256;
  line = xmalloc (linelen);
  do
    {
      char *p = line, *q;

      /* Normally this would use getline(3), but we need to be portable.  */
      while ((q = fgets (p, linelen - (p - line), f)) != NULL
	     && strlen (q) == linelen - (p - line) - 1
	     && line[linelen - 2] != '\n')
	{
	  line = xrealloc (line, 2 * linelen);
	  p = line + linelen - 1;
	  linelen += linelen;
	}

      if (q == NULL && p == line)
	break;

      p = strchr (line, '\n');
      if (p)
	*p = '\0';

      /* Because the file format does not know any form of quoting we
	 can search forward for the next '#' character and if found
	 make it terminating the line.  */
      p = strchr (line, '#');
      if (p)
	*p = '\0';

      /* Remove leading whitespace.  NUL is no whitespace character.  */
      p = line;
      while (*p == ' ' || *p == '\f' || *p == '\r' || *p == '\t' || *p == '\v')
	++p;

      /* If the line is blank it is ignored.  */
      if (p[0] == '\0')
	continue;

      if (startswith (p, "include") && (p[7] == ' ' || p[7] == '\t'))
	{
	  char *dir, c;
	  p += 8;
	  do
	    {
	      while (*p == ' ' || *p == '\t')
		++p;

	      if (*p == '\0')
		break;

	      dir = p;

	      while (*p != ' ' && *p != '\t' && *p)
		++p;

	      c = *p;
	      *p++ = '\0';
	      if (dir[0] != '\0')
		ldelf_parse_ld_so_conf_include (info, filename, dir);
	    }
	  while (c != '\0');
	}
      else
	{
	  char *dir = p;
	  while (*p && *p != '=' && *p != ' ' && *p != '\t' && *p != '\f'
		 && *p != '\r' && *p != '\v')
	    ++p;

	  while (p != dir && p[-1] == '/')
	    --p;
	  if (info->path == NULL)
	    {
	      info->alloc = p - dir + 1 + 256;
	      info->path = xmalloc (info->alloc);
	      info->len = 0;
	    }
	  else
	    {
	      if (info->len + 1 + (p - dir) >= info->alloc)
		{
		  info->alloc += p - dir + 256;
		  info->path = xrealloc (info->path, info->alloc);
		}
	      info->path[info->len++] = config.rpath_separator;
	    }
	  memcpy (info->path + info->len, dir, p - dir);
	  info->len += p - dir;
	  info->path[info->len] = '\0';
	}
    }
  while (! feof (f));
  free (line);
  fclose (f);
  return true;
}

static bool
ldelf_check_ld_so_conf (const struct bfd_link_needed_list *l, int force,
			int elfsize, const char *prefix)
{
  static bool initialized;
  static const char *ld_so_conf;
  struct dt_needed needed;

  if (! initialized)
    {
      char *tmppath;
      struct ldelf_ld_so_conf info;

      info.path = NULL;
      info.len = info.alloc = 0;
      tmppath = concat (ld_sysroot, prefix, "/etc/ld.so.conf",
			(const char *) NULL);
      if (!ldelf_parse_ld_so_conf (&info, tmppath))
	{
	  free (tmppath);
	  tmppath = concat (ld_sysroot, "/etc/ld.so.conf",
			    (const char *) NULL);
	  ldelf_parse_ld_so_conf (&info, tmppath);
	}
      free (tmppath);

      if (info.path)
	{
	  ld_so_conf = ldelf_add_sysroot (info.path);
	  free (info.path);
	}
      initialized = true;
    }

  if (ld_so_conf == NULL)
    return false;


  needed.by = l->by;
  needed.name = l->name;
  return ldelf_search_needed (ld_so_conf, &needed, force, true, elfsize);
}

/* See if an input file matches a DT_NEEDED entry by name.  */

static void
ldelf_check_needed (lang_input_statement_type *s)
{
  const char *soname;

  /* Stop looking if we've found a loaded lib.  */
  if (global_found != NULL
      && (bfd_elf_get_dyn_lib_class (global_found->the_bfd)
	  & DYN_AS_NEEDED) == 0)
    return;

  if (s->filename == NULL || s->the_bfd == NULL)
    return;

  /* Don't look for a second non-loaded as-needed lib.  */
  if (global_found != NULL
      && (bfd_elf_get_dyn_lib_class (s->the_bfd) & DYN_AS_NEEDED) != 0)
    return;

  if (filename_cmp (s->filename, global_needed->name) == 0)
    {
      global_found = s;
      return;
    }

  if (s->flags.search_dirs)
    {
      const char *f = strrchr (s->filename, '/');
      if (f != NULL
	  && filename_cmp (f + 1, global_needed->name) == 0)
	{
	  global_found = s;
	  return;
	}
    }

  soname = bfd_elf_get_dt_soname (s->the_bfd);
  if (soname != NULL
      && filename_cmp (soname, global_needed->name) == 0)
    {
      global_found = s;
      return;
    }
}

static void
ldelf_handle_dt_needed (struct elf_link_hash_table *htab,
			int use_libpath, int native, int is_linux,
			int is_freebsd, int elfsize, const char *prefix)
{
  struct bfd_link_needed_list *needed, *l;
  bfd *abfd;
  bfd **save_input_bfd_tail;

  /* Get the list of files which appear in DT_NEEDED entries in
     dynamic objects included in the link (often there will be none).
     For each such file, we want to track down the corresponding
     library, and include the symbol table in the link.  This is what
     the runtime dynamic linker will do.  Tracking the files down here
     permits one dynamic object to include another without requiring
     special action by the person doing the link.  Note that the
     needed list can actually grow while we are stepping through this
     loop.  */
  save_input_bfd_tail = link_info.input_bfds_tail;
  needed = bfd_elf_get_needed_list (link_info.output_bfd, &link_info);
  for (l = needed; l != NULL; l = l->next)
    {
      struct bfd_link_needed_list *ll;
      struct dt_needed n, nn;
      int force;

      /* If the lib that needs this one was --as-needed and wasn't
	 found to be needed, then this lib isn't needed either.  */
      if (l->by != NULL
	  && (bfd_elf_get_dyn_lib_class (l->by) & DYN_AS_NEEDED) != 0)
	continue;

      /* Skip the lib if --no-copy-dt-needed-entries and when we are
	 handling DT_NEEDED entries or --allow-shlib-undefined is in
	 effect.  */
      if (l->by != NULL
	  && (htab->handling_dt_needed
	      || link_info.unresolved_syms_in_shared_libs == RM_IGNORE)
	  && (bfd_elf_get_dyn_lib_class (l->by) & DYN_NO_ADD_NEEDED) != 0)
	continue;

      /* If we've already seen this file, skip it.  */
      for (ll = needed; ll != l; ll = ll->next)
	if ((ll->by == NULL
	     || (bfd_elf_get_dyn_lib_class (ll->by) & DYN_AS_NEEDED) == 0)
	    && strcmp (ll->name, l->name) == 0)
	  break;
      if (ll != l)
	continue;

      /* See if this file was included in the link explicitly.  */
      global_needed = l;
      global_found = NULL;
      lang_for_each_input_file (ldelf_check_needed);
      if (global_found != NULL
	  && (bfd_elf_get_dyn_lib_class (global_found->the_bfd)
	      & DYN_AS_NEEDED) == 0)
	continue;

      n.by = l->by;
      n.name = l->name;
      nn.by = l->by;
      if (verbose)
	info_msg (_("%s needed by %pB\n"), l->name, l->by);

      /* As-needed libs specified on the command line (or linker script)
	 take priority over libs found in search dirs.  */
      if (global_found != NULL)
	{
	  nn.name = global_found->filename;
	  if (ldelf_try_needed (&nn, true, is_linux))
	    continue;
	}

      /* We need to find this file and include the symbol table.  We
	 want to search for the file in the same way that the dynamic
	 linker will search.  That means that we want to use
	 rpath_link, rpath, then the environment variable
	 LD_LIBRARY_PATH (native only), then the DT_RPATH/DT_RUNPATH
	 entries (native only), then the linker script LIB_SEARCH_DIRS.
	 We do not search using the -L arguments.

	 We search twice.  The first time, we skip objects which may
	 introduce version mismatches.  The second time, we force
	 their use.  See ldelf_vercheck comment.  */
      for (force = 0; force < 2; force++)
	{
	  size_t len;
	  search_dirs_type *search;
	  const char *path;
	  struct bfd_link_needed_list *rp;
	  int found;

	  if (ldelf_search_needed (command_line.rpath_link, &n, force,
				   is_linux, elfsize))
	    break;

	  if (use_libpath)
	    {
	      path = command_line.rpath;
	      if (path)
		{
		  path = ldelf_add_sysroot (path);
		  found = ldelf_search_needed (path, &n, force,
					       is_linux, elfsize);
		  free ((char *) path);
		  if (found)
		    break;
		}
	    }
	  if (native)
	    {
	      if (command_line.rpath_link == NULL
		  && command_line.rpath == NULL)
		{
		  path = (const char *) getenv ("LD_RUN_PATH");
		  if (path
		      && ldelf_search_needed (path, &n, force,
					      is_linux, elfsize))
		    break;
		}
	      path = (const char *) getenv ("LD_LIBRARY_PATH");
	      if (path
		  && ldelf_search_needed (path, &n, force,
					  is_linux, elfsize))
		break;
	    }
	  if (use_libpath)
	    {
	      found = 0;
	      rp = bfd_elf_get_runpath_list (link_info.output_bfd, &link_info);
	      for (; !found && rp != NULL; rp = rp->next)
		{
		  path = ldelf_add_sysroot (rp->name);
		  found = (rp->by == l->by
			   && ldelf_search_needed (path, &n, force,
						   is_linux, elfsize));
		  free ((char *) path);
		}
	      if (found)
		break;

	      if (is_freebsd
		  && ldelf_check_ld_elf_hints (l, force, elfsize))
		break;

	      if (is_linux
		  && ldelf_check_ld_so_conf (l, force, elfsize, prefix))
		break;
	    }

	  len = strlen (l->name);
	  for (search = search_head; search != NULL; search = search->next)
	    {
	      char *filename;

	      if (search->cmdline)
		continue;
	      filename = (char *) xmalloc (strlen (search->name) + len + 2);
	      sprintf (filename, "%s/%s", search->name, l->name);
	      nn.name = filename;
	      if (ldelf_try_needed (&nn, force, is_linux))
		break;
	      free (filename);
	    }
	  if (search != NULL)
	    break;
	}

      if (force < 2)
	continue;

      einfo (_("%P: warning: %s, needed by %pB, not found "
	       "(try using -rpath or -rpath-link)\n"),
	     l->name, l->by);
    }

  /* Don't add DT_NEEDED when loading shared objects from DT_NEEDED for
     plugin symbol resolution while handling DT_NEEDED entries.  */
  if (!htab->handling_dt_needed)
    for (abfd = link_info.input_bfds; abfd; abfd = abfd->link.next)
      if (bfd_get_format (abfd) == bfd_object
	  && ((abfd->flags) & DYNAMIC) != 0
	  && bfd_get_flavour (abfd) == bfd_target_elf_flavour
	  && (elf_dyn_lib_class (abfd) & (DYN_AS_NEEDED | DYN_NO_NEEDED)) == 0
	  && elf_dt_name (abfd) != NULL)
	{
	  if (bfd_elf_add_dt_needed_tag (abfd, &link_info) < 0)
	    einfo (_("%F%P: failed to add DT_NEEDED dynamic tag\n"));
	}

  link_info.input_bfds_tail = save_input_bfd_tail;
  *save_input_bfd_tail = NULL;
}

/* This is called before calling plugin 'all symbols read' hook.  */

void
ldelf_before_plugin_all_symbols_read (int use_libpath, int native,
				      int is_linux, int is_freebsd,
				      int elfsize, const char *prefix)
{
  struct elf_link_hash_table *htab = elf_hash_table (&link_info);

  if (!link_info.lto_plugin_active
      || !is_elf_hash_table (&htab->root))
    return;

  htab->handling_dt_needed = true;
  ldelf_handle_dt_needed (htab, use_libpath, native, is_linux,
			  is_freebsd, elfsize, prefix);
  htab->handling_dt_needed = false;
}

/* This is called after all the input files have been opened and all
   symbols have been loaded.  */

void
ldelf_after_open (int use_libpath, int native, int is_linux, int is_freebsd,
		  int elfsize, const char *prefix)
{
  struct elf_link_hash_table *htab;
  asection *s;
  bfd *abfd;

  after_open_default ();

  htab = elf_hash_table (&link_info);
  if (!is_elf_hash_table (&htab->root))
    return;

  if (command_line.out_implib_filename)
    {
      unlink_if_ordinary (command_line.out_implib_filename);
      link_info.out_implib_bfd
	= bfd_openw (command_line.out_implib_filename,
		     bfd_get_target (link_info.output_bfd));

      if (link_info.out_implib_bfd == NULL)
	{
	  einfo (_("%F%P: %s: can't open for writing: %E\n"),
		 command_line.out_implib_filename);
	}
    }

  if (ldelf_emit_note_gnu_build_id != NULL
      || ldelf_emit_note_fdo_package_metadata != NULL)
    {
      /* Find an ELF input.  */
      for (abfd = link_info.input_bfds;
	   abfd != (bfd *) NULL; abfd = abfd->link.next)
	if (bfd_get_flavour (abfd) == bfd_target_elf_flavour
	    && bfd_count_sections (abfd) != 0
	    && !bfd_input_just_syms (abfd))
	  break;

      /* PR 10555: If there are no ELF input files do not try to
	 create a .note.gnu-build-id section.  */
      if (abfd == NULL
	  || (ldelf_emit_note_gnu_build_id != NULL
	      && !ldelf_setup_build_id (abfd)))
	{
	  free ((char *) ldelf_emit_note_gnu_build_id);
	  ldelf_emit_note_gnu_build_id = NULL;
	}

      if (abfd == NULL
	  || (ldelf_emit_note_fdo_package_metadata != NULL
	      && !ldelf_setup_package_metadata (abfd)))
	{
	  free ((char *) ldelf_emit_note_fdo_package_metadata);
	  ldelf_emit_note_fdo_package_metadata = NULL;
	}
    }

  get_elf_backend_data (link_info.output_bfd)->setup_gnu_properties (&link_info);

  /* Do not allow executable files to be used as inputs to the link.  */
  for (abfd = link_info.input_bfds; abfd; abfd = abfd->link.next)
    {
      /* Discard input .note.gnu.build-id sections.  */
      s = bfd_get_section_by_name (abfd, ".note.gnu.build-id");
      while (s != NULL)
	{
	  if (s != elf_tdata (link_info.output_bfd)->o->build_id.sec)
	    s->flags |= SEC_EXCLUDE;
	  s = bfd_get_next_section_by_name (NULL, s);
	}

      if (abfd->xvec->flavour == bfd_target_elf_flavour
	  && !bfd_input_just_syms (abfd)
	  && elf_tdata (abfd) != NULL
	  /* FIXME: Maybe check for other non-supportable types as well ?  */
	  && (elf_tdata (abfd)->elf_header->e_type == ET_EXEC
	      || (elf_tdata (abfd)->elf_header->e_type == ET_DYN
		  && elf_tdata (abfd)->is_pie)))
	einfo (_("%F%P: cannot use executable file '%pB' as input to a link\n"),
	       abfd);
    }

  if (bfd_link_relocatable (&link_info))
    {
      if (link_info.execstack == !link_info.noexecstack)
	{
	  /* PR ld/16744: If "-z [no]execstack" has been specified on the
	     command line and we are perfoming a relocatable link then no
	     PT_GNU_STACK segment will be created and so the
	     linkinfo.[no]execstack values set in _handle_option() will have no
	     effect.  Instead we create a .note.GNU-stack section in much the
	     same way as the assembler does with its --[no]execstack option.  */
	  flagword flags = SEC_READONLY | (link_info.execstack ? SEC_CODE : 0);
	  (void) bfd_make_section_with_flags (link_info.input_bfds,
					      ".note.GNU-stack", flags);
	}
      return;
    }

  if (!link_info.traditional_format)
    {
      bfd *elfbfd = NULL;
      bool warn_eh_frame = false;
      int seen_type = 0;

      for (abfd = link_info.input_bfds; abfd; abfd = abfd->link.next)
	{
	  int type = 0;

	  if (bfd_input_just_syms (abfd))
	    continue;

	  for (s = abfd->sections; s && type < COMPACT_EH_HDR; s = s->next)
	    {
	      const char *name = bfd_section_name (s);

	      if (bfd_is_abs_section (s->output_section))
		continue;
	      if (startswith (name, ".eh_frame_entry"))
		type = COMPACT_EH_HDR;
	      else if (strcmp (name, ".eh_frame") == 0 && s->size > 8)
		type = DWARF2_EH_HDR;
	    }

	  if (type != 0)
	    {
	      if (seen_type == 0)
		{
		  seen_type = type;
		}
	      else if (seen_type != type)
		{
		  einfo (_("%F%P: compact frame descriptions incompatible with"
			   " DWARF2 .eh_frame from %pB\n"),
			 type == DWARF2_EH_HDR ? abfd : elfbfd);
		  break;
		}

	      if (!elfbfd
		  && (type == COMPACT_EH_HDR
		      || link_info.eh_frame_hdr_type != 0))
		{
		  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour)
		    elfbfd = abfd;

		  warn_eh_frame = true;
		}
	    }

	  if (seen_type == COMPACT_EH_HDR)
	    link_info.eh_frame_hdr_type = COMPACT_EH_HDR;
	}
      if (elfbfd)
	{
	  const struct elf_backend_data *bed;

	  bed = get_elf_backend_data (elfbfd);
	  s = bfd_make_section_with_flags (elfbfd, ".eh_frame_hdr",
					   bed->dynamic_sec_flags
					   | SEC_READONLY);
	  if (s != NULL
	      && bfd_set_section_alignment (s, 2))
	    {
	      htab->eh_info.hdr_sec = s;
	      warn_eh_frame = false;
	    }
	}
      if (warn_eh_frame)
	einfo (_("%P: warning: cannot create .eh_frame_hdr section,"
		 " --eh-frame-hdr ignored\n"));
    }

  if (link_info.eh_frame_hdr_type == COMPACT_EH_HDR)
    if (!bfd_elf_parse_eh_frame_entries (NULL, &link_info))
      einfo (_("%F%P: failed to parse EH frame entries\n"));

  ldelf_handle_dt_needed (htab, use_libpath, native, is_linux,
			  is_freebsd, elfsize, prefix);
}

static bfd_size_type
id_note_section_size (bfd *abfd ATTRIBUTE_UNUSED)
{
  const char *style = ldelf_emit_note_gnu_build_id;
  bfd_size_type size;
  bfd_size_type build_id_size;

  size = offsetof (Elf_External_Note, name[sizeof "GNU"]);
  size = (size + 3) & -(bfd_size_type) 4;

  build_id_size = compute_build_id_size (style);
  if (build_id_size)
    size += build_id_size;
  else
    size = 0;

  return size;
}

static bool
write_build_id (bfd *abfd)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_obj_tdata *t = elf_tdata (abfd);
  const char *style;
  asection *asec;
  Elf_Internal_Shdr *i_shdr;
  unsigned char *contents, *id_bits;
  bfd_size_type size;
  file_ptr position;
  Elf_External_Note *e_note;

  style = t->o->build_id.style;
  asec = t->o->build_id.sec;
  if (bfd_is_abs_section (asec->output_section))
    {
      einfo (_("%P: warning: .note.gnu.build-id section discarded,"
	       " --build-id ignored\n"));
      return true;
    }
  i_shdr = &elf_section_data (asec->output_section)->this_hdr;

  if (i_shdr->contents == NULL)
    {
      if (asec->contents == NULL)
	asec->contents = (unsigned char *) xmalloc (asec->size);
      contents = asec->contents;
    }
  else
    contents = i_shdr->contents + asec->output_offset;

  e_note = (Elf_External_Note *) contents;
  size = offsetof (Elf_External_Note, name[sizeof "GNU"]);
  size = (size + 3) & -(bfd_size_type) 4;
  id_bits = contents + size;
  size = asec->size - size;

  /* Clear the build ID field.  */
  memset (id_bits, 0, size);

  bfd_h_put_32 (abfd, sizeof "GNU", &e_note->namesz);
  bfd_h_put_32 (abfd, size, &e_note->descsz);
  bfd_h_put_32 (abfd, NT_GNU_BUILD_ID, &e_note->type);
  memcpy (e_note->name, "GNU", sizeof "GNU");

  generate_build_id (abfd, style, bed->s->checksum_contents, id_bits, size);

  position = i_shdr->sh_offset + asec->output_offset;
  size = asec->size;
  return (bfd_seek (abfd, position, SEEK_SET) == 0
	  && bfd_bwrite (contents, size, abfd) == size);
}

/* Make .note.gnu.build-id section, and set up elf_tdata->build_id.  */

bool
ldelf_setup_build_id (bfd *ibfd)
{
  asection *s;
  bfd_size_type size;
  flagword flags;

  size = id_note_section_size (ibfd);
  if (size == 0)
    {
      einfo (_("%P: warning: unrecognized --build-id style ignored\n"));
      return false;
    }

  flags = (SEC_ALLOC | SEC_LOAD | SEC_IN_MEMORY
	   | SEC_LINKER_CREATED | SEC_READONLY | SEC_DATA);
  s = bfd_make_section_anyway_with_flags (ibfd, ".note.gnu.build-id",
					  flags);
  if (s != NULL && bfd_set_section_alignment (s, 2))
    {
      struct elf_obj_tdata *t = elf_tdata (link_info.output_bfd);
      t->o->build_id.after_write_object_contents = &write_build_id;
      t->o->build_id.style = ldelf_emit_note_gnu_build_id;
      t->o->build_id.sec = s;
      elf_section_type (s) = SHT_NOTE;
      s->size = size;
      return true;
    }

  einfo (_("%P: warning: cannot create .note.gnu.build-id section,"
	   " --build-id ignored\n"));
  return false;
}

static bool
write_package_metadata (bfd *abfd)
{
  struct elf_obj_tdata *t = elf_tdata (abfd);
  const char *json;
  asection *asec;
  Elf_Internal_Shdr *i_shdr;
  unsigned char *contents, *json_bits;
  bfd_size_type size;
  file_ptr position;
  Elf_External_Note *e_note;

  json = t->o->package_metadata.json;
  asec = t->o->package_metadata.sec;
  if (bfd_is_abs_section (asec->output_section))
    {
      einfo (_("%P: warning: .note.package section discarded,"
	       " --package-metadata ignored\n"));
      return true;
    }
  i_shdr = &elf_section_data (asec->output_section)->this_hdr;

  if (i_shdr->contents == NULL)
    {
      if (asec->contents == NULL)
	asec->contents = (unsigned char *) xmalloc (asec->size);
      contents = asec->contents;
    }
  else
    contents = i_shdr->contents + asec->output_offset;

  e_note = (Elf_External_Note *) contents;
  size = offsetof (Elf_External_Note, name[sizeof "FDO"]);
  size = (size + 3) & -(bfd_size_type) 4;
  json_bits = contents + size;
  size = asec->size - size;

  /* Clear the package metadata field.  */
  memset (json_bits, 0, size);

  bfd_h_put_32 (abfd, sizeof "FDO", &e_note->namesz);
  bfd_h_put_32 (abfd, size, &e_note->descsz);
  bfd_h_put_32 (abfd, FDO_PACKAGING_METADATA, &e_note->type);
  memcpy (e_note->name, "FDO", sizeof "FDO");
  memcpy (json_bits, json, strlen(json));

  position = i_shdr->sh_offset + asec->output_offset;
  size = asec->size;
  return (bfd_seek (abfd, position, SEEK_SET) == 0
	  && bfd_bwrite (contents, size, abfd) == size);
}

/* Make .note.package section.
   https://systemd.io/ELF_PACKAGE_METADATA/  */

bool
ldelf_setup_package_metadata (bfd *ibfd)
{
  asection *s;
  bfd_size_type size;
  size_t json_length;
  flagword flags;

  /* If the option wasn't specified, silently return. */
  if (!ldelf_emit_note_fdo_package_metadata)
    return false;

  /* The option was specified, but it's empty, log and return. */
  json_length = strlen (ldelf_emit_note_fdo_package_metadata);
  if (json_length == 0)
    {
      einfo (_("%P: warning: --package-metadata is empty, ignoring\n"));
      return false;
    }

#ifdef HAVE_JANSSON
  json_error_t json_error;
  json_t *json = json_loads (ldelf_emit_note_fdo_package_metadata,
			     0, &json_error);
  if (!json)
    {
      einfo (_("%P: warning: --package-metadata=%s does not contain valid "
	       "JSON, ignoring: %s\n"),
	     ldelf_emit_note_fdo_package_metadata, json_error.text);
      return false;
    }
  else
    json_decref (json);
#endif

  size = offsetof (Elf_External_Note, name[sizeof "FDO"]);
  size += json_length + 1;
  size = (size + 3) & -(bfd_size_type) 4;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_IN_MEMORY
	   | SEC_LINKER_CREATED | SEC_READONLY | SEC_DATA);
  s = bfd_make_section_anyway_with_flags (ibfd, ".note.package",
					  flags);
  if (s != NULL && bfd_set_section_alignment (s, 2))
    {
      struct elf_obj_tdata *t = elf_tdata (link_info.output_bfd);
      t->o->package_metadata.after_write_object_contents
	= &write_package_metadata;
      t->o->package_metadata.json = ldelf_emit_note_fdo_package_metadata;
      t->o->package_metadata.sec = s;
      elf_section_type (s) = SHT_NOTE;
      s->size = size;
      return true;
    }

  einfo (_("%P: warning: cannot create .note.package section,"
	   " --package-metadata ignored\n"));
  return false;
}

/* Look through an expression for an assignment statement.  */

static void
ldelf_find_exp_assignment (etree_type *exp)
{
  bool provide = false;

  switch (exp->type.node_class)
    {
    case etree_provide:
    case etree_provided:
      provide = true;
      /* Fallthru */
    case etree_assign:
      /* We call record_link_assignment even if the symbol is defined.
	 This is because if it is defined by a dynamic object, we
	 actually want to use the value defined by the linker script,
	 not the value from the dynamic object (because we are setting
	 symbols like etext).  If the symbol is defined by a regular
	 object, then, as it happens, calling record_link_assignment
	 will do no harm.  */
      if (strcmp (exp->assign.dst, ".") != 0)
	{
	  if (!bfd_elf_record_link_assignment (link_info.output_bfd,
					       &link_info,
					       exp->assign.dst, provide,
					       exp->assign.hidden))
	    einfo (_("%F%P: failed to record assignment to %s: %E\n"),
		   exp->assign.dst);
	}
      ldelf_find_exp_assignment (exp->assign.src);
      break;

    case etree_binary:
      ldelf_find_exp_assignment (exp->binary.lhs);
      ldelf_find_exp_assignment (exp->binary.rhs);
      break;

    case etree_trinary:
      ldelf_find_exp_assignment (exp->trinary.cond);
      ldelf_find_exp_assignment (exp->trinary.lhs);
      ldelf_find_exp_assignment (exp->trinary.rhs);
      break;

    case etree_unary:
      ldelf_find_exp_assignment (exp->unary.child);
      break;

    default:
      break;
    }
}

/* This is called by the before_allocation routine via
   lang_for_each_statement.  It locates any assignment statements, and
   tells the ELF backend about them, in case they are assignments to
   symbols which are referred to by dynamic objects.  */

static void
ldelf_find_statement_assignment (lang_statement_union_type *s)
{
  if (s->header.type == lang_assignment_statement_enum)
    ldelf_find_exp_assignment (s->assignment_statement.exp);
}

/* Used by before_allocation and handle_option. */

void
ldelf_append_to_separated_string (char **to, char *op_arg)
{
  if (*to == NULL)
    *to = xstrdup (op_arg);
  else
    {
      size_t to_len = strlen (*to);
      size_t op_arg_len = strlen (op_arg);
      char *buf;
      char *cp = *to;

      /* First see whether OPTARG is already in the path.  */
      do
	{
	  if (strncmp (op_arg, cp, op_arg_len) == 0
	      && (cp[op_arg_len] == 0
		  || cp[op_arg_len] == config.rpath_separator))
	    /* We found it.  */
	    break;

	  /* Not yet found.  */
	  cp = strchr (cp, config.rpath_separator);
	  if (cp != NULL)
	    ++cp;
	}
      while (cp != NULL);

      if (cp == NULL)
	{
	  buf = xmalloc (to_len + op_arg_len + 2);
	  sprintf (buf, "%s%c%s", *to,
		   config.rpath_separator, op_arg);
	  free (*to);
	  *to = buf;
	}
    }
}

/* This is called after the sections have been attached to output
   sections, but before any sizes or addresses have been set.  */

void
ldelf_before_allocation (char *audit, char *depaudit,
			 const char *default_interpreter_name)
{
  const char *rpath;
  asection *sinterp;
  bfd *abfd;
  struct bfd_link_hash_entry *ehdr_start = NULL;
  unsigned char ehdr_start_save_type = 0;
  char ehdr_start_save_u[sizeof ehdr_start->u
			 - sizeof ehdr_start->u.def.next] = "";

  if (is_elf_hash_table (link_info.hash))
    {
      _bfd_elf_tls_setup (link_info.output_bfd, &link_info);

      /* Make __ehdr_start hidden if it has been referenced, to
	 prevent the symbol from being dynamic.  */
      if (!bfd_link_relocatable (&link_info))
	{
	  struct elf_link_hash_table *htab = elf_hash_table (&link_info);
	  struct elf_link_hash_entry *h
	    = elf_link_hash_lookup (htab, "__ehdr_start", false, false, true);

	  /* Only adjust the export class if the symbol was referenced
	     and not defined, otherwise leave it alone.  */
	  if (h != NULL
	      && (h->root.type == bfd_link_hash_new
		  || h->root.type == bfd_link_hash_undefined
		  || h->root.type == bfd_link_hash_undefweak
		  || h->root.type == bfd_link_hash_common))
	    {
	      /* Don't leave the symbol undefined.  Undefined hidden
		 symbols typically won't have dynamic relocations, but
		 we most likely will need dynamic relocations for
		 __ehdr_start if we are building a PIE or shared
		 library.  */
	      ehdr_start = &h->root;
	      ehdr_start_save_type = ehdr_start->type;
	      memcpy (ehdr_start_save_u,
		      (char *) &ehdr_start->u + sizeof ehdr_start->u.def.next,
		      sizeof ehdr_start_save_u);
	      ehdr_start->type = bfd_link_hash_defined;
	      /* It will be converted to section-relative later.  */
	      ehdr_start->u.def.section = bfd_abs_section_ptr;
	      ehdr_start->u.def.value = 0;
	    }
	}

      /* If we are going to make any variable assignments, we need to
	 let the ELF backend know about them in case the variables are
	 referred to by dynamic objects.  */
      lang_for_each_statement (ldelf_find_statement_assignment);
    }

  /* Let the ELF backend work out the sizes of any sections required
     by dynamic linking.  */
  rpath = command_line.rpath;
  if (rpath == NULL)
    rpath = (const char *) getenv ("LD_RUN_PATH");

  for (abfd = link_info.input_bfds; abfd; abfd = abfd->link.next)
    if (bfd_get_flavour (abfd) == bfd_target_elf_flavour)
      {
	const char *audit_libs = elf_dt_audit (abfd);

	/* If the input bfd contains an audit entry, we need to add it as
	   a dep audit entry.  */
	if (audit_libs && *audit_libs != '\0')
	  {
	    char *cp = xstrdup (audit_libs);
	    do
	      {
		int more = 0;
		char *cp2 = strchr (cp, config.rpath_separator);

		if (cp2)
		  {
		    *cp2 = '\0';
		    more = 1;
		  }

		if (cp != NULL && *cp != '\0')
		  ldelf_append_to_separated_string (&depaudit, cp);

		cp = more ? ++cp2 : NULL;
	      }
	    while (cp != NULL);
	  }
      }

  if (! (bfd_elf_size_dynamic_sections
	 (link_info.output_bfd, command_line.soname, rpath,
	  command_line.filter_shlib, audit, depaudit,
	  (const char * const *) command_line.auxiliary_filters,
	  &link_info, &sinterp)))
    einfo (_("%F%P: failed to set dynamic section sizes: %E\n"));

  if (sinterp != NULL)
    {
      /* Let the user override the dynamic linker we are using.  */
      if (command_line.interpreter != NULL)
	default_interpreter_name = command_line.interpreter;
      if (default_interpreter_name != NULL)
	{
	  sinterp->contents = (bfd_byte *) default_interpreter_name;
	  sinterp->size = strlen ((char *) sinterp->contents) + 1;
	}
    }

  /* Look for any sections named .gnu.warning.  As a GNU extensions,
     we treat such sections as containing warning messages.  We print
     out the warning message, and then zero out the section size so
     that it does not get copied into the output file.  */

  {
    LANG_FOR_EACH_INPUT_STATEMENT (is)
      {
	asection *s;
	bfd_size_type sz;
	char *msg;

	if (is->flags.just_syms)
	  continue;

	s = bfd_get_section_by_name (is->the_bfd, ".gnu.warning");
	if (s == NULL)
	  continue;

	sz = s->size;
	msg = (char *) xmalloc ((size_t) (sz + 1));
	if (! bfd_get_section_contents (is->the_bfd, s,	msg,
					(file_ptr) 0, sz))
	  einfo (_("%F%P: %pB: can't read contents of section .gnu.warning: %E\n"),
		 is->the_bfd);
	msg[sz] = '\0';
	(*link_info.callbacks->warning) (&link_info, msg,
					 (const char *) NULL, is->the_bfd,
					 (asection *) NULL, (bfd_vma) 0);
	free (msg);

	/* Clobber the section size, so that we don't waste space
	   copying the warning into the output file.  If we've already
	   sized the output section, adjust its size.  The adjustment
	   is on rawsize because targets that size sections early will
	   have called lang_reset_memory_regions after sizing.  */
	if (s->output_section != NULL
	    && s->output_section->rawsize >= s->size)
	  s->output_section->rawsize -= s->size;

	s->size = 0;

	/* Also set SEC_EXCLUDE, so that local symbols defined in the
	   warning section don't get copied to the output.  */
	s->flags |= SEC_EXCLUDE | SEC_KEEP;
      }
  }

  before_allocation_default ();

  if (!bfd_elf_size_dynsym_hash_dynstr (link_info.output_bfd, &link_info))
    einfo (_("%F%P: failed to set dynamic section sizes: %E\n"));

  if (ehdr_start != NULL)
    {
      /* If we twiddled __ehdr_start to defined earlier, put it back
	 as it was.  */
      ehdr_start->type = ehdr_start_save_type;
      memcpy ((char *) &ehdr_start->u + sizeof ehdr_start->u.def.next,
	      ehdr_start_save_u,
	      sizeof ehdr_start_save_u);
    }
}
/* Try to open a dynamic archive.  This is where we know that ELF
   dynamic libraries have an extension of .so (or .sl on oddball systems
   like hpux).  */

bool
ldelf_open_dynamic_archive (const char *arch, search_dirs_type *search,
			    lang_input_statement_type *entry)
{
  const char *filename;
  char *string;
  size_t len;
  bool opened = false;

  if (! entry->flags.maybe_archive)
    return false;

  filename = entry->filename;
  len = strlen (search->name) + strlen (filename);
  if (entry->flags.full_name_provided)
    {
      len += sizeof "/";
      string = (char *) xmalloc (len);
      sprintf (string, "%s/%s", search->name, filename);
    }
  else
    {
      size_t xlen = 0;

      len += strlen (arch) + sizeof "/lib.so";
#ifdef EXTRA_SHLIB_EXTENSION
      xlen = (strlen (EXTRA_SHLIB_EXTENSION) > 3
	      ? strlen (EXTRA_SHLIB_EXTENSION) - 3
	      : 0);
#endif
      string = (char *) xmalloc (len + xlen);
      sprintf (string, "%s/lib%s%s.so", search->name, filename, arch);
#ifdef EXTRA_SHLIB_EXTENSION
      /* Try the .so extension first.  If that fails build a new filename
	 using EXTRA_SHLIB_EXTENSION.  */
      opened = ldfile_try_open_bfd (string, entry);
      if (!opened)
	strcpy (string + len - 4, EXTRA_SHLIB_EXTENSION);
#endif
    }

  if (!opened && !ldfile_try_open_bfd (string, entry))
    {
      free (string);
      return false;
    }

  entry->filename = string;

  /* We have found a dynamic object to include in the link.  The ELF
     backend linker will create a DT_NEEDED entry in the .dynamic
     section naming this file.  If this file includes a DT_SONAME
     entry, it will be used.  Otherwise, the ELF linker will just use
     the name of the file.  For an archive found by searching, like
     this one, the DT_NEEDED entry should consist of just the name of
     the file, without the path information used to find it.  Note
     that we only need to do this if we have a dynamic object; an
     archive will never be referenced by a DT_NEEDED entry.

     FIXME: This approach--using bfd_elf_set_dt_needed_name--is not
     very pretty.  I haven't been able to think of anything that is
     pretty, though.  */
  if (bfd_check_format (entry->the_bfd, bfd_object)
      && (entry->the_bfd->flags & DYNAMIC) != 0)
    {
      ASSERT (entry->flags.maybe_archive && entry->flags.search_dirs);

      /* Rather than duplicating the logic above.  Just use the
	 filename we recorded earlier.  */

      if (!entry->flags.full_name_provided)
	filename = lbasename (entry->filename);
      bfd_elf_set_dt_needed_name (entry->the_bfd, filename);
    }

  return true;
}

/* A variant of lang_output_section_find used by place_orphan.  */

static lang_output_section_statement_type *
output_rel_find (int isdyn, int rela)
{
  lang_output_section_statement_type *lookup;
  lang_output_section_statement_type *last = NULL;
  lang_output_section_statement_type *last_alloc = NULL;
  lang_output_section_statement_type *last_ro_alloc = NULL;
  lang_output_section_statement_type *last_rel = NULL;
  lang_output_section_statement_type *last_rel_alloc = NULL;

  for (lookup = (void *) lang_os_list.head;
       lookup != NULL;
       lookup = lookup->next)
    {
      if (lookup->constraint >= 0
	  && startswith (lookup->name, ".rel"))
	{
	  int lookrela = lookup->name[4] == 'a';

	  /* .rel.dyn must come before all other reloc sections, to suit
	     GNU ld.so.  */
	  if (isdyn)
	    break;

	  /* Don't place after .rel.plt as doing so results in wrong
	     dynamic tags.  */
	  if (strcmp (".plt", lookup->name + 4 + lookrela) == 0)
	    break;

	  if (rela == lookrela || last_rel == NULL)
	    last_rel = lookup;
	  if ((rela == lookrela || last_rel_alloc == NULL)
	      && lookup->bfd_section != NULL
	      && (lookup->bfd_section->flags & SEC_ALLOC) != 0)
	    last_rel_alloc = lookup;
	}

      last = lookup;
      if (lookup->bfd_section != NULL
	  && (lookup->bfd_section->flags & SEC_ALLOC) != 0)
	{
	  last_alloc = lookup;
	  if ((lookup->bfd_section->flags & SEC_READONLY) != 0)
	    last_ro_alloc = lookup;
	}
    }

  if (last_rel_alloc)
    return last_rel_alloc;

  if (last_rel)
    return last_rel;

  if (last_ro_alloc)
    return last_ro_alloc;

  if (last_alloc)
    return last_alloc;

  return last;
}

/* Return whether IN is suitable to be part of OUT.  */

static bool
elf_orphan_compatible (asection *in, asection *out)
{
  /* Non-zero sh_info implies a section with SHF_INFO_LINK with
     unknown semantics for the generic linker, or a SHT_REL/SHT_RELA
     section where sh_info specifies a symbol table.  (We won't see
     SHT_GROUP, SHT_SYMTAB or SHT_DYNSYM sections here.)  We clearly
     can't merge SHT_REL/SHT_RELA using differing symbol tables, and
     shouldn't merge sections with differing unknown semantics.  */
  if (elf_section_data (out)->this_hdr.sh_info
      != elf_section_data (in)->this_hdr.sh_info)
    return false;
  /* We can't merge with a member of an output section group or merge
     two sections with differing SHF_EXCLUDE or other processor and OS
     specific flags when doing a relocatable link.  */
  if (bfd_link_relocatable (&link_info)
      && (elf_next_in_group (out) != NULL
	  || ((elf_section_flags (out) ^ elf_section_flags (in))
	      & (SHF_MASKPROC | SHF_MASKOS)) != 0))
    return false;
  return _bfd_elf_match_sections_by_type (link_info.output_bfd, out,
					  in->owner, in);
}

/* Place an orphan section.  We use this to put random SHF_ALLOC
   sections in the right segment.  */

lang_output_section_statement_type *
ldelf_place_orphan (asection *s, const char *secname, int constraint)
{
  static struct orphan_save hold[] =
    {
      { ".text",
	SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE,
	0, 0, 0, 0 },
      { ".rodata",
	SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_DATA,
	0, 0, 0, 0 },
      { ".tdata",
	SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_DATA | SEC_THREAD_LOCAL,
	0, 0, 0, 0 },
      { ".data",
	SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_DATA,
	0, 0, 0, 0 },
      { ".bss",
	SEC_ALLOC,
	0, 0, 0, 0 },
      { 0,
	SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_DATA,
	0, 0, 0, 0 },
      { ".interp",
	SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_DATA,
	0, 0, 0, 0 },
      { ".sdata",
	SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_DATA | SEC_SMALL_DATA,
	0, 0, 0, 0 },
      { ".comment",
	SEC_HAS_CONTENTS,
	0, 0, 0, 0 },
    };
  enum orphan_save_index
    {
      orphan_text = 0,
      orphan_rodata,
      orphan_tdata,
      orphan_data,
      orphan_bss,
      orphan_rel,
      orphan_interp,
      orphan_sdata,
      orphan_nonalloc
    };
  static int orphan_init_done = 0;
  struct orphan_save *place;
  lang_output_section_statement_type *after;
  lang_output_section_statement_type *os;
  lang_output_section_statement_type *match_by_name = NULL;
  int isdyn = 0;
  int elfinput = s->owner->xvec->flavour == bfd_target_elf_flavour;
  int elfoutput = link_info.output_bfd->xvec->flavour == bfd_target_elf_flavour;
  unsigned int sh_type = elfinput ? elf_section_type (s) : SHT_NULL;
  flagword flags;
  asection *nexts;

  if (!bfd_link_relocatable (&link_info)
      && link_info.combreloc
      && (s->flags & SEC_ALLOC))
    {
      if (elfinput)
	switch (sh_type)
	  {
	  case SHT_RELA:
	    secname = ".rela.dyn";
	    isdyn = 1;
	    break;
	  case SHT_REL:
	    secname = ".rel.dyn";
	    isdyn = 1;
	    break;
	  default:
	    break;
	  }
      else if (startswith (secname, ".rel"))
	{
	  secname = secname[4] == 'a' ? ".rela.dyn" : ".rel.dyn";
	  isdyn = 1;
	}
    }

  if (!bfd_link_relocatable (&link_info)
      && elfinput
      && elfoutput
      && (s->flags & SEC_ALLOC) != 0
      && (elf_tdata (s->owner)->has_gnu_osabi & elf_gnu_osabi_mbind) != 0
      && (elf_section_flags (s) & SHF_GNU_MBIND) != 0)
    {
      /* Find the output mbind section with the same type, attributes
	 and sh_info field.  */
      for (os = (void *) lang_os_list.head;
	   os != NULL;
	   os = os->next)
	if (os->bfd_section != NULL
	    && !bfd_is_abs_section (os->bfd_section)
	    && (elf_section_flags (os->bfd_section) & SHF_GNU_MBIND) != 0
	    && ((s->flags & (SEC_ALLOC
			     | SEC_LOAD
			     | SEC_HAS_CONTENTS
			     | SEC_READONLY
			     | SEC_CODE))
		== (os->bfd_section->flags & (SEC_ALLOC
					      | SEC_LOAD
					      | SEC_HAS_CONTENTS
					      | SEC_READONLY
					      | SEC_CODE)))
	    && (elf_section_data (os->bfd_section)->this_hdr.sh_info
		== elf_section_data (s)->this_hdr.sh_info))
	    {
	      lang_add_section (&os->children, s, NULL, NULL, os);
	      return os;
	    }

      /* Create the output mbind section with the ".mbind." prefix
	 in section name.  */
      if ((s->flags & (SEC_LOAD | SEC_HAS_CONTENTS)) == 0)
	secname = ".mbind.bss";
      else if ((s->flags & SEC_READONLY) == 0)
	secname = ".mbind.data";
      else if ((s->flags & SEC_CODE) == 0)
	secname = ".mbind.rodata";
      else
	secname = ".mbind.text";
      elf_tdata (link_info.output_bfd)->has_gnu_osabi |= elf_gnu_osabi_mbind;
    }

  /* Look through the script to see where to place this section.  The
     script includes entries added by previous lang_insert_orphan
     calls, so this loop puts multiple compatible orphans of the same
     name into a single output section.  */
  if (constraint == 0)
    for (os = lang_output_section_find (secname);
	 os != NULL;
	 os = next_matching_output_section_statement (os, 0))
      {
	/* If we don't match an existing output section, tell
	   lang_insert_orphan to create a new output section.  */
	constraint = SPECIAL;

	/* Check to see if we already have an output section statement
	   with this name, and its bfd section has compatible flags.
	   If the section already exists but does not have any flags
	   set, then it has been created by the linker, possibly as a
	   result of a --section-start command line switch.  */
	if (os->bfd_section != NULL
	    && (os->bfd_section->flags == 0
		|| (((s->flags ^ os->bfd_section->flags)
		     & (SEC_LOAD | SEC_ALLOC)) == 0
		    && (!elfinput
			|| !elfoutput
			|| elf_orphan_compatible (s, os->bfd_section)))))
	  {
	    lang_add_section (&os->children, s, NULL, NULL, os);
	    return os;
	  }

	/* Save unused output sections in case we can match them
	   against orphans later.  */
	if (os->bfd_section == NULL)
	  match_by_name = os;
      }

  /* If we didn't match an active output section, see if we matched an
     unused one and use that.  */
  if (match_by_name)
    {
      lang_add_section (&match_by_name->children, s, NULL, NULL, match_by_name);
      return match_by_name;
    }

  if (!orphan_init_done)
    {
      struct orphan_save *ho;

      for (ho = hold; ho < hold + sizeof (hold) / sizeof (hold[0]); ++ho)
	if (ho->name != NULL)
	  {
	    ho->os = lang_output_section_find (ho->name);
	    if (ho->os != NULL && ho->os->flags == 0)
	      ho->os->flags = ho->flags;
	  }
      orphan_init_done = 1;
    }

  /* If this is a final link, then always put .gnu.warning.SYMBOL
     sections into the .text section to get them out of the way.  */
  if (bfd_link_executable (&link_info)
      && startswith (s->name, ".gnu.warning.")
      && hold[orphan_text].os != NULL)
    {
      os = hold[orphan_text].os;
      lang_add_section (&os->children, s, NULL, NULL, os);
      return os;
    }

  flags = s->flags;
  if (!bfd_link_relocatable (&link_info))
    {
      nexts = s;
      while ((nexts = bfd_get_next_section_by_name (nexts->owner, nexts))
	     != NULL)
	if (nexts->output_section == NULL
	    && (nexts->flags & SEC_EXCLUDE) == 0
	    && ((nexts->flags ^ flags) & (SEC_LOAD | SEC_ALLOC)) == 0
	    && (nexts->owner->flags & DYNAMIC) == 0
	    && !bfd_input_just_syms (nexts->owner)
	    && _bfd_elf_match_sections_by_type (nexts->owner, nexts,
						s->owner, s))
	  flags = (((flags ^ SEC_READONLY)
		    | (nexts->flags ^ SEC_READONLY))
		   ^ SEC_READONLY);
    }

  /* Decide which segment the section should go in based on the
     section name and section flags.  We put loadable .note sections
     right after the .interp section, so that the PT_NOTE segment is
     stored right after the program headers where the OS can read it
     in the first page.  */

  place = NULL;
  if ((flags & (SEC_ALLOC | SEC_DEBUGGING)) == 0)
    place = &hold[orphan_nonalloc];
  else if ((flags & SEC_ALLOC) == 0)
    ;
  else if ((flags & SEC_LOAD) != 0
	   && (elfinput
	       ? sh_type == SHT_NOTE
	       : startswith (secname, ".note")))
    place = &hold[orphan_interp];
  else if ((flags & (SEC_LOAD | SEC_HAS_CONTENTS | SEC_THREAD_LOCAL)) == 0)
    place = &hold[orphan_bss];
  else if ((flags & SEC_SMALL_DATA) != 0)
    place = &hold[orphan_sdata];
  else if ((flags & SEC_THREAD_LOCAL) != 0)
    place = &hold[orphan_tdata];
  else if ((flags & SEC_READONLY) == 0)
    place = &hold[orphan_data];
  else if ((flags & SEC_LOAD) != 0
	   && (elfinput
	       ? sh_type == SHT_RELA || sh_type == SHT_REL
	       : startswith (secname, ".rel")))
    place = &hold[orphan_rel];
  else if ((flags & SEC_CODE) == 0)
    place = &hold[orphan_rodata];
  else
    place = &hold[orphan_text];

  after = NULL;
  if (place != NULL)
    {
      if (place->os == NULL)
	{
	  if (place->name != NULL)
	    place->os = lang_output_section_find (place->name);
	  else
	    {
	      int rela = elfinput ? sh_type == SHT_RELA : secname[4] == 'a';
	      place->os = output_rel_find (isdyn, rela);
	    }
	}
      after = place->os;
      if (after == NULL)
	after
	  = lang_output_section_find_by_flags (s, flags, &place->os,
					       _bfd_elf_match_sections_by_type);
      if (after == NULL)
	/* *ABS* is always the first output section statement.  */
	after = (void *) lang_os_list.head;
    }

  return lang_insert_orphan (s, secname, constraint, after, place, NULL, NULL);
}

void
ldelf_before_place_orphans (void)
{
  bfd *abfd;

  for (abfd = link_info.input_bfds;
       abfd != (bfd *) NULL; abfd = abfd->link.next)
    if (bfd_get_flavour (abfd) == bfd_target_elf_flavour
	&& bfd_count_sections (abfd) != 0
	&& !bfd_input_just_syms (abfd))
      {
	asection *isec;
	for (isec = abfd->sections; isec != NULL; isec = isec->next)
	  {
	    /* Discard a section if any of its linked-to section has
	       been discarded.  */
	    asection *linked_to_sec;
	    for (linked_to_sec = elf_linked_to_section (isec);
		 linked_to_sec != NULL && !linked_to_sec->linker_mark;
		 linked_to_sec = elf_linked_to_section (linked_to_sec))
	      {
		if (discarded_section (linked_to_sec))
		  {
		    isec->output_section = bfd_abs_section_ptr;
		    isec->flags |= SEC_EXCLUDE;
		    break;
		  }
		linked_to_sec->linker_mark = 1;
	      }
	    for (linked_to_sec = elf_linked_to_section (isec);
		 linked_to_sec != NULL && linked_to_sec->linker_mark;
		 linked_to_sec = elf_linked_to_section (linked_to_sec))
	      linked_to_sec->linker_mark = 0;
	  }
      }
}

void
ldelf_set_output_arch (void)
{
  set_output_arch_default ();
  if (link_info.output_bfd->xvec->flavour == bfd_target_elf_flavour)
    elf_link_info (link_info.output_bfd) = &link_info;
}

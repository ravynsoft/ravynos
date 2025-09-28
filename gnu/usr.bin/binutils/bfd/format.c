/* Generic BFD support for file formats.
   Copyright (C) 1990-2023 Free Software Foundation, Inc.
   Written by Cygnus Support.

   This file is part of BFD, the Binary File Descriptor library.

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


/*
SECTION
	File formats

	A format is a BFD concept of high level file contents type. The
	formats supported by BFD are:

	o <<bfd_object>>

	The BFD may contain data, symbols, relocations and debug info.

	o <<bfd_archive>>

	The BFD contains other BFDs and an optional index.

	o <<bfd_core>>

	The BFD contains the result of an executable core dump.

SUBSECTION
	File format functions
*/

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

/* IMPORT from targets.c.  */
extern const size_t _bfd_target_vector_entries;

/*
FUNCTION
	bfd_check_format

SYNOPSIS
	bool bfd_check_format (bfd *abfd, bfd_format format);

DESCRIPTION
	Verify if the file attached to the BFD @var{abfd} is compatible
	with the format @var{format} (i.e., one of <<bfd_object>>,
	<<bfd_archive>> or <<bfd_core>>).

	If the BFD has been set to a specific target before the
	call, only the named target and format combination is
	checked. If the target has not been set, or has been set to
	<<default>>, then all the known target backends is
	interrogated to determine a match.  If the default target
	matches, it is used.  If not, exactly one target must recognize
	the file, or an error results.

	The function returns <<TRUE>> on success, otherwise <<FALSE>>
	with one of the following error codes:

	o <<bfd_error_invalid_operation>> -
	if <<format>> is not one of <<bfd_object>>, <<bfd_archive>> or
	<<bfd_core>>.

	o <<bfd_error_system_call>> -
	if an error occured during a read - even some file mismatches
	can cause bfd_error_system_calls.

	o <<file_not_recognised>> -
	none of the backends recognised the file format.

	o <<bfd_error_file_ambiguously_recognized>> -
	more than one backend recognised the file format.
*/

bool
bfd_check_format (bfd *abfd, bfd_format format)
{
  return bfd_check_format_matches (abfd, format, NULL);
}

struct bfd_preserve
{
  void *marker;
  void *tdata;
  flagword flags;
  const struct bfd_iovec *iovec;
  void *iostream;
  const struct bfd_arch_info *arch_info;
  const struct bfd_build_id *build_id;
  bfd_cleanup cleanup;
  struct bfd_section *sections;
  struct bfd_section *section_last;
  unsigned int section_count;
  unsigned int section_id;
  unsigned int symcount;
  bool read_only;
  bfd_vma start_address;
  struct bfd_hash_table section_htab;
};

/* When testing an object for compatibility with a particular target
   back-end, the back-end object_p function needs to set up certain
   fields in the bfd on successfully recognizing the object.  This
   typically happens in a piecemeal fashion, with failures possible at
   many points.  On failure, the bfd is supposed to be restored to its
   initial state, which is virtually impossible.  However, restoring a
   subset of the bfd state works in practice.  This function stores
   the subset.  */

static bool
bfd_preserve_save (bfd *abfd, struct bfd_preserve *preserve,
		   bfd_cleanup cleanup)
{
  preserve->tdata = abfd->tdata.any;
  preserve->arch_info = abfd->arch_info;
  preserve->flags = abfd->flags;
  preserve->iovec = abfd->iovec;
  preserve->iostream = abfd->iostream;
  preserve->sections = abfd->sections;
  preserve->section_last = abfd->section_last;
  preserve->section_count = abfd->section_count;
  preserve->section_id = _bfd_section_id;
  preserve->symcount = abfd->symcount;
  preserve->read_only = abfd->read_only;
  preserve->start_address = abfd->start_address;
  preserve->section_htab = abfd->section_htab;
  preserve->marker = bfd_alloc (abfd, 1);
  preserve->build_id = abfd->build_id;
  preserve->cleanup = cleanup;
  if (preserve->marker == NULL)
    return false;

  return bfd_hash_table_init (&abfd->section_htab, bfd_section_hash_newfunc,
			      sizeof (struct section_hash_entry));
}

/* A back-end object_p function may flip a bfd from file backed to
   in-memory, eg. pe_ILF_object_p.  In that case to restore the
   original IO state we need to reopen the file.  Conversely, if we
   are restoring a previously matched pe ILF format and have been
   checking further target matches using file IO then we need to close
   the file and detach the bfd from the cache lru list.  */

static void
io_reinit (bfd *abfd, struct bfd_preserve *preserve)
{
  if (abfd->iovec != preserve->iovec)
    {
      /* Handle file backed to in-memory transition.  bfd_cache_close
	 won't do anything unless abfd->iovec is the cache_iovec.  */
      bfd_cache_close (abfd);
      abfd->iovec = preserve->iovec;
      abfd->iostream = preserve->iostream;
      /* Handle in-memory to file backed transition.  */
      if ((abfd->flags & BFD_CLOSED_BY_CACHE) != 0
	  && (abfd->flags & BFD_IN_MEMORY) != 0
	  && (preserve->flags & BFD_CLOSED_BY_CACHE) == 0
	  && (preserve->flags & BFD_IN_MEMORY) == 0)
	bfd_open_file (abfd);
    }
  abfd->flags = preserve->flags;
}

/* Clear out a subset of BFD state.  */

static void
bfd_reinit (bfd *abfd, unsigned int section_id,
	    struct bfd_preserve *preserve, bfd_cleanup cleanup)
{
  _bfd_section_id = section_id;
  if (cleanup)
    cleanup (abfd);
  abfd->tdata.any = NULL;
  abfd->arch_info = &bfd_default_arch_struct;
  io_reinit (abfd, preserve);
  abfd->symcount = 0;
  abfd->read_only = 0;
  abfd->start_address = 0;
  abfd->build_id = NULL;
  bfd_section_list_clear (abfd);
}

/* Restores bfd state saved by bfd_preserve_save.  */

static bfd_cleanup
bfd_preserve_restore (bfd *abfd, struct bfd_preserve *preserve)
{
  bfd_hash_table_free (&abfd->section_htab);

  abfd->tdata.any = preserve->tdata;
  abfd->arch_info = preserve->arch_info;
  io_reinit (abfd, preserve);
  abfd->section_htab = preserve->section_htab;
  abfd->sections = preserve->sections;
  abfd->section_last = preserve->section_last;
  abfd->section_count = preserve->section_count;
  _bfd_section_id = preserve->section_id;
  abfd->symcount = preserve->symcount;
  abfd->read_only = preserve->read_only;
  abfd->start_address = preserve->start_address;
  abfd->build_id = preserve->build_id;

  /* bfd_release frees all memory more recently bfd_alloc'd than
     its arg, as well as its arg.  */
  bfd_release (abfd, preserve->marker);
  preserve->marker = NULL;
  return preserve->cleanup;
}

/* Called when the bfd state saved by bfd_preserve_save is no longer
   needed.  */

static void
bfd_preserve_finish (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_preserve *preserve)
{
  if (preserve->cleanup)
    {
      /* Run the cleanup, assuming that all it will need is the
	 tdata at the time the cleanup was returned.  */
      void *tdata = abfd->tdata.any;
      abfd->tdata.any = preserve->tdata;
      preserve->cleanup (abfd);
      abfd->tdata.any = tdata;
    }
  /* It would be nice to be able to free more memory here, eg. old
     tdata, but that's not possible since these blocks are sitting
     inside bfd_alloc'd memory.  The section hash is on a separate
     objalloc.  */
  bfd_hash_table_free (&preserve->section_htab);
  preserve->marker = NULL;
}

static void
print_warnmsg (struct per_xvec_message **list)
{
  fflush (stdout);
  fprintf (stderr, "%s: ", _bfd_get_error_program_name ());

  for (struct per_xvec_message *warn = *list; warn; warn = warn->next)
    {
      fputs (warn->message, stderr);
      fputc ('\n', stderr);
    }
  fflush (stderr);
}

static void
clear_warnmsg (struct per_xvec_message **list)
{
  struct per_xvec_message *warn = *list;
  while (warn)
    {
      struct per_xvec_message *next = warn->next;
      free (warn);
      warn = next;
    }
  *list = NULL;
}

static void
null_error_handler (const char *fmt ATTRIBUTE_UNUSED,
		    va_list ap ATTRIBUTE_UNUSED)
{
}

/*
FUNCTION
	bfd_check_format_matches

SYNOPSIS
	bool bfd_check_format_matches
	  (bfd *abfd, bfd_format format, char ***matching);

DESCRIPTION
	Like <<bfd_check_format>>, except when it returns FALSE with
	<<bfd_errno>> set to <<bfd_error_file_ambiguously_recognized>>.  In that
	case, if @var{matching} is not NULL, it will be filled in with
	a NULL-terminated list of the names of the formats that matched,
	allocated with <<malloc>>.
	Then the user may choose a format and try again.

	When done with the list that @var{matching} points to, the caller
	should free it.
*/

bool
bfd_check_format_matches (bfd *abfd, bfd_format format, char ***matching)
{
  extern const bfd_target binary_vec;
#if BFD_SUPPORTS_PLUGINS
  extern const bfd_target plugin_vec;
#endif
  const bfd_target * const *target;
  const bfd_target **matching_vector = NULL;
  const bfd_target *save_targ, *right_targ, *ar_right_targ, *match_targ;
  int match_count, best_count, best_match;
  int ar_match_index;
  unsigned int initial_section_id = _bfd_section_id;
  struct bfd_preserve preserve, preserve_match;
  bfd_cleanup cleanup = NULL;
  bfd_error_handler_type orig_error_handler;
  static int in_check_format;

  if (matching != NULL)
    *matching = NULL;

  if (!bfd_read_p (abfd)
      || (unsigned int) abfd->format >= (unsigned int) bfd_type_end)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  if (abfd->format != bfd_unknown)
    return abfd->format == format;

  if (matching != NULL || *bfd_associated_vector != NULL)
    {
      size_t amt;

      amt = sizeof (*matching_vector) * 2 * _bfd_target_vector_entries;
      matching_vector = (const bfd_target **) bfd_malloc (amt);
      if (!matching_vector)
	return false;
    }

  /* Presume the answer is yes.  */
  abfd->format = format;
  save_targ = abfd->xvec;

  /* Don't report errors on recursive calls checking the first element
     of an archive.  */
  if (in_check_format)
    orig_error_handler = bfd_set_error_handler (null_error_handler);
  else
    orig_error_handler = _bfd_set_error_handler_caching (abfd);
  ++in_check_format;

  preserve_match.marker = NULL;
  if (!bfd_preserve_save (abfd, &preserve, NULL))
    goto err_ret;

  /* If the target type was explicitly specified, just check that target.  */
  if (!abfd->target_defaulted)
    {
      if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)	/* rewind! */
	goto err_ret;

      cleanup = BFD_SEND_FMT (abfd, _bfd_check_format, (abfd));

      if (cleanup)
	goto ok_ret;

      /* For a long time the code has dropped through to check all
	 targets if the specified target was wrong.  I don't know why,
	 and I'm reluctant to change it.  However, in the case of an
	 archive, it can cause problems.  If the specified target does
	 not permit archives (e.g., the binary target), then we should
	 not allow some other target to recognize it as an archive, but
	 should instead allow the specified target to recognize it as an
	 object.  When I first made this change, it broke the PE target,
	 because the specified pei-i386 target did not recognize the
	 actual pe-i386 archive.  Since there may be other problems of
	 this sort, I changed this test to check only for the binary
	 target.  */
      if (format == bfd_archive && save_targ == &binary_vec)
	goto err_unrecog;
    }

  /* Since the target type was defaulted, check them all in the hope
     that one will be uniquely recognized.  */
  right_targ = NULL;
  ar_right_targ = NULL;
  match_targ = NULL;
  best_match = 256;
  best_count = 0;
  match_count = 0;
  ar_match_index = _bfd_target_vector_entries;

  for (target = bfd_target_vector; *target != NULL; target++)
    {
      void **high_water;

      /* The binary target matches anything, so don't return it when
	 searching.  Don't match the plugin target if we have another
	 alternative since we want to properly set the input format
	 before allowing a plugin to claim the file.  Also, don't
	 check the default target twice.  */
      if (*target == &binary_vec
#if BFD_SUPPORTS_PLUGINS
	  || (match_count != 0 && *target == &plugin_vec)
#endif
	  || (!abfd->target_defaulted && *target == save_targ))
	continue;

      /* If we already tried a match, the bfd is modified and may
	 have sections attached, which will confuse the next
	 _bfd_check_format call.  */
      bfd_reinit (abfd, initial_section_id, &preserve, cleanup);
      /* Free bfd_alloc memory too.  If we have matched and preserved
	 a target then the high water mark is that much higher.  */
      if (preserve_match.marker)
	high_water = &preserve_match.marker;
      else
	high_water = &preserve.marker;
      bfd_release (abfd, *high_water);
      *high_water = bfd_alloc (abfd, 1);

      /* Change BFD's target temporarily.  */
      abfd->xvec = *target;

      if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)
	goto err_ret;

      cleanup = BFD_SEND_FMT (abfd, _bfd_check_format, (abfd));
      if (cleanup)
	{
	  int match_priority = abfd->xvec->match_priority;
#if BFD_SUPPORTS_PLUGINS
	  /* If this object can be handled by a plugin, give that the
	     lowest priority; objects both handled by a plugin and
	     with an underlying object format will be claimed
	     separately by the plugin.  */
	  if (*target == &plugin_vec)
	    match_priority = (*target)->match_priority;
#endif

	  if (abfd->format != bfd_archive
	      || (bfd_has_map (abfd)
		  && bfd_get_error () != bfd_error_wrong_object_format))
	    {
	      /* If this is the default target, accept it, even if
		 other targets might match.  People who want those
		 other targets have to set the GNUTARGET variable.  */
	      if (abfd->xvec == bfd_default_vector[0])
		goto ok_ret;

	      if (matching_vector)
		matching_vector[match_count] = abfd->xvec;
	      match_count++;

	      if (match_priority < best_match)
		{
		  best_match = match_priority;
		  best_count = 0;
		}
	      if (match_priority <= best_match)
		{
		  /* This format checks out as ok!  */
		  right_targ = abfd->xvec;
		  best_count++;
		}
	    }
	  else
	    {
	      /* An archive with no armap or objects of the wrong
		 type.  We want this target to match if we get no
		 better matches.  */
	      if (ar_right_targ != bfd_default_vector[0])
		ar_right_targ = *target;
	      if (matching_vector)
		matching_vector[ar_match_index] = *target;
	      ar_match_index++;
	    }

	  if (preserve_match.marker == NULL)
	    {
	      match_targ = abfd->xvec;
	      if (!bfd_preserve_save (abfd, &preserve_match, cleanup))
		goto err_ret;
	      cleanup = NULL;
	    }
	}
    }

  if (best_count == 1)
    match_count = 1;

  if (match_count == 0)
    {
      /* Try partial matches.  */
      right_targ = ar_right_targ;

      if (right_targ == bfd_default_vector[0])
	{
	  match_count = 1;
	}
      else
	{
	  match_count = ar_match_index - _bfd_target_vector_entries;

	  if (matching_vector && match_count > 1)
	    memcpy (matching_vector,
		    matching_vector + _bfd_target_vector_entries,
		    sizeof (*matching_vector) * match_count);
	}
    }

  /* We have more than one equally good match.  If any of the best
     matches is a target in config.bfd targ_defvec or targ_selvecs,
     choose it.  */
  if (match_count > 1)
    {
      const bfd_target * const *assoc = bfd_associated_vector;

      while ((right_targ = *assoc++) != NULL)
	{
	  int i = match_count;

	  while (--i >= 0)
	    if (matching_vector[i] == right_targ
		&& right_targ->match_priority <= best_match)
	      break;

	  if (i >= 0)
	    {
	      match_count = 1;
	      break;
	    }
	}
    }

  /* We still have more than one equally good match, and at least some
     of the targets support match priority.  Choose the first of the
     best matches.  */
  if (matching_vector && match_count > 1 && best_count != match_count)
    {
      int i;

      for (i = 0; i < match_count; i++)
	{
	  right_targ = matching_vector[i];
	  if (right_targ->match_priority <= best_match)
	    break;
	}
      match_count = 1;
    }

  /* There is way too much undoing of half-known state here.  We
     really shouldn't iterate on live bfd's.  Note that saving the
     whole bfd and restoring it would be even worse; the first thing
     you notice is that the cached bfd file position gets out of sync.  */
  if (preserve_match.marker != NULL)
    cleanup = bfd_preserve_restore (abfd, &preserve_match);

  if (match_count == 1)
    {
      abfd->xvec = right_targ;
      /* If we come out of the loop knowing that the last target that
	 matched is the one we want, then ABFD should still be in a usable
	 state (except possibly for XVEC).  This is not just an
	 optimisation.  In the case of plugins a match against the
	 plugin target can result in the bfd being changed such that
	 it no longer matches the plugin target, nor will it match
	 RIGHT_TARG again.  */
      if (match_targ != right_targ)
	{
	  bfd_reinit (abfd, initial_section_id, &preserve, cleanup);
	  bfd_release (abfd, preserve.marker);
	  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)
	    goto err_ret;
	  cleanup = BFD_SEND_FMT (abfd, _bfd_check_format, (abfd));
	  BFD_ASSERT (cleanup != NULL);
	}

    ok_ret:
      /* If the file was opened for update, then `output_has_begun'
	 some time ago when the file was created.  Do not recompute
	 sections sizes or alignments in _bfd_set_section_contents.
	 We can not set this flag until after checking the format,
	 because it will interfere with creation of BFD sections.  */
      if (abfd->direction == both_direction)
	abfd->output_has_begun = true;

      free (matching_vector);
      if (preserve_match.marker != NULL)
	bfd_preserve_finish (abfd, &preserve_match);
      bfd_preserve_finish (abfd, &preserve);
      bfd_set_error_handler (orig_error_handler);

      struct per_xvec_message **list = _bfd_per_xvec_warn (abfd->xvec, 0);
      if (*list)
	print_warnmsg (list);
      list = _bfd_per_xvec_warn (NULL, 0);
      for (size_t i = 0; i < _bfd_target_vector_entries + 1; i++)
	clear_warnmsg (list++);
      --in_check_format;

      /* File position has moved, BTW.  */
      return true;
    }

  if (match_count == 0)
    {
    err_unrecog:
      bfd_set_error (bfd_error_file_not_recognized);
    err_ret:
      if (cleanup)
	cleanup (abfd);
      abfd->xvec = save_targ;
      abfd->format = bfd_unknown;
      free (matching_vector);
      goto out;
    }

  /* Restore original target type and format.  */
  abfd->xvec = save_targ;
  abfd->format = bfd_unknown;
  bfd_set_error (bfd_error_file_ambiguously_recognized);

  if (matching)
    {
      *matching = (char **) matching_vector;
      matching_vector[match_count] = NULL;
      /* Return target names.  This is a little nasty.  Maybe we
	 should do another bfd_malloc?  */
      while (--match_count >= 0)
	{
	  const char *name = matching_vector[match_count]->name;
	  *(const char **) &matching_vector[match_count] = name;
	}
    }
  else
    free (matching_vector);
  if (cleanup)
    cleanup (abfd);
 out:
  if (preserve_match.marker != NULL)
    bfd_preserve_finish (abfd, &preserve_match);
  bfd_preserve_restore (abfd, &preserve);
  bfd_set_error_handler (orig_error_handler);
  struct per_xvec_message **list = _bfd_per_xvec_warn (NULL, 0);
  struct per_xvec_message **one = NULL;
  for (size_t i = 0; i < _bfd_target_vector_entries + 1; i++)
    {
      if (list[i])
	{
	  if (!one)
	    one = list + i;
	  else
	    {
	      one = NULL;
	      break;
	    }
	}
    }
  if (one)
    print_warnmsg (one);
  for (size_t i = 0; i < _bfd_target_vector_entries + 1; i++)
    clear_warnmsg (list++);
  --in_check_format;
  return false;
}

/*
FUNCTION
	bfd_set_format

SYNOPSIS
	bool bfd_set_format (bfd *abfd, bfd_format format);

DESCRIPTION
	This function sets the file format of the BFD @var{abfd} to the
	format @var{format}. If the target set in the BFD does not
	support the format requested, the format is invalid, or the BFD
	is not open for writing, then an error occurs.
*/

bool
bfd_set_format (bfd *abfd, bfd_format format)
{
  if (bfd_read_p (abfd)
      || (unsigned int) abfd->format >= (unsigned int) bfd_type_end)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  if (abfd->format != bfd_unknown)
    return abfd->format == format;

  /* Presume the answer is yes.  */
  abfd->format = format;

  if (!BFD_SEND_FMT (abfd, _bfd_set_format, (abfd)))
    {
      abfd->format = bfd_unknown;
      return false;
    }

  return true;
}

/*
FUNCTION
	bfd_format_string

SYNOPSIS
	const char *bfd_format_string (bfd_format format);

DESCRIPTION
	Return a pointer to a const string
	<<invalid>>, <<object>>, <<archive>>, <<core>>, or <<unknown>>,
	depending upon the value of @var{format}.
*/

const char *
bfd_format_string (bfd_format format)
{
  if (((int) format < (int) bfd_unknown)
      || ((int) format >= (int) bfd_type_end))
    return "invalid";

  switch (format)
    {
    case bfd_object:
      return "object";		/* Linker/assembler/compiler output.  */
    case bfd_archive:
      return "archive";		/* Object archive file.  */
    case bfd_core:
      return "core";		/* Core dump.  */
    default:
      return "unknown";
    }
}

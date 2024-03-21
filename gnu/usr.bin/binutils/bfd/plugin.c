/* Plugin support for BFD.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

#include "sysdep.h"
#include "bfd.h"

#if BFD_SUPPORTS_PLUGINS

#include <assert.h>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#elif defined (HAVE_WINDOWS_H)
#include <windows.h>
#else
#error Unknown how to handle dynamic-load-libraries.
#endif
#include <stdarg.h>
#include "plugin-api.h"
#include "plugin.h"
#include "libbfd.h"
#include "libiberty.h"
#include <dirent.h>

#if !defined (HAVE_DLFCN_H) && defined (HAVE_WINDOWS_H)

#define RTLD_NOW 0      /* Dummy value.  */

static void *
dlopen (const char *file, int mode ATTRIBUTE_UNUSED)
{
  return LoadLibrary (file);
}

static void *
dlsym (void *handle, const char *name)
{
  return GetProcAddress (handle, name);
}

static int ATTRIBUTE_UNUSED
dlclose (void *handle)
{
  FreeLibrary (handle);
  return 0;
}

static const char *
dlerror (void)
{
  return "Unable to load DLL.";
}

#endif /* !defined (HAVE_DLFCN_H) && defined (HAVE_WINDOWS_H)  */

#define bfd_plugin_close_and_cleanup		      _bfd_generic_close_and_cleanup
#define bfd_plugin_bfd_free_cached_info		      _bfd_generic_bfd_free_cached_info
#define bfd_plugin_new_section_hook		      _bfd_generic_new_section_hook
#define bfd_plugin_get_section_contents		      _bfd_generic_get_section_contents
#define bfd_plugin_get_section_contents_in_window     _bfd_generic_get_section_contents_in_window
#define bfd_plugin_bfd_copy_private_header_data	      _bfd_generic_bfd_copy_private_header_data
#define bfd_plugin_bfd_merge_private_bfd_data	      _bfd_generic_bfd_merge_private_bfd_data
#define bfd_plugin_bfd_copy_private_header_data	      _bfd_generic_bfd_copy_private_header_data
#define bfd_plugin_bfd_set_private_flags	      _bfd_generic_bfd_set_private_flags
#define bfd_plugin_core_file_matches_executable_p     generic_core_file_matches_executable_p
#define bfd_plugin_bfd_is_local_label_name	      _bfd_nosymbols_bfd_is_local_label_name
#define bfd_plugin_bfd_is_target_special_symbol	      _bfd_bool_bfd_asymbol_false
#define bfd_plugin_get_lineno			      _bfd_nosymbols_get_lineno
#define bfd_plugin_find_nearest_line		      _bfd_nosymbols_find_nearest_line
#define bfd_plugin_find_nearest_line_with_alt	      _bfd_nosymbols_find_nearest_line_with_alt
#define bfd_plugin_find_line			      _bfd_nosymbols_find_line
#define bfd_plugin_find_inliner_info		      _bfd_nosymbols_find_inliner_info
#define bfd_plugin_get_symbol_version_string	      _bfd_nosymbols_get_symbol_version_string
#define bfd_plugin_bfd_make_debug_symbol	      _bfd_nosymbols_bfd_make_debug_symbol
#define bfd_plugin_read_minisymbols		      _bfd_generic_read_minisymbols
#define bfd_plugin_minisymbol_to_symbol		      _bfd_generic_minisymbol_to_symbol
#define bfd_plugin_set_arch_mach		      bfd_default_set_arch_mach
#define bfd_plugin_set_section_contents		      _bfd_generic_set_section_contents
#define bfd_plugin_bfd_get_relocated_section_contents bfd_generic_get_relocated_section_contents
#define bfd_plugin_bfd_relax_section		      bfd_generic_relax_section
#define bfd_plugin_bfd_link_hash_table_create	      _bfd_generic_link_hash_table_create
#define bfd_plugin_bfd_link_add_symbols		      _bfd_generic_link_add_symbols
#define bfd_plugin_bfd_link_just_syms		      _bfd_generic_link_just_syms
#define bfd_plugin_bfd_final_link		      _bfd_generic_final_link
#define bfd_plugin_bfd_link_split_section	      _bfd_generic_link_split_section
#define bfd_plugin_bfd_gc_sections		      bfd_generic_gc_sections
#define bfd_plugin_bfd_lookup_section_flags	      bfd_generic_lookup_section_flags
#define bfd_plugin_bfd_merge_sections		      bfd_generic_merge_sections
#define bfd_plugin_bfd_is_group_section		      bfd_generic_is_group_section
#define bfd_plugin_bfd_group_name		      bfd_generic_group_name
#define bfd_plugin_bfd_discard_group		      bfd_generic_discard_group
#define bfd_plugin_section_already_linked	      _bfd_generic_section_already_linked
#define bfd_plugin_bfd_define_common_symbol	      bfd_generic_define_common_symbol
#define bfd_plugin_bfd_link_hide_symbol		      _bfd_generic_link_hide_symbol
#define bfd_plugin_bfd_define_start_stop	      bfd_generic_define_start_stop
#define bfd_plugin_bfd_copy_link_hash_symbol_type     _bfd_generic_copy_link_hash_symbol_type
#define bfd_plugin_bfd_link_check_relocs	      _bfd_generic_link_check_relocs

static enum ld_plugin_status
message (int level ATTRIBUTE_UNUSED,
	 const char * format, ...)
{
  va_list args;
  va_start (args, format);
  printf ("bfd plugin: ");
  vprintf (format, args);
  putchar ('\n');
  va_end (args);
  return LDPS_OK;
}

struct plugin_list_entry
{
  /* These must be initialized for each IR object with LTO wrapper.  */
  ld_plugin_claim_file_handler claim_file;
  ld_plugin_claim_file_handler_v2 claim_file_v2;
  ld_plugin_all_symbols_read_handler all_symbols_read;
  ld_plugin_all_symbols_read_handler cleanup_handler;
  bool has_symbol_type;

  struct plugin_list_entry *next;

  /* These can be reused for all IR objects.  */
  const char *plugin_name;
};

static const char *plugin_program_name;

void
bfd_plugin_set_program_name (const char *program_name)
{
  plugin_program_name = program_name;
}

static struct plugin_list_entry *plugin_list = NULL;
static struct plugin_list_entry *current_plugin = NULL;

/* Register a claim-file handler. */

static enum ld_plugin_status
register_claim_file (ld_plugin_claim_file_handler handler)
{
  current_plugin->claim_file = handler;
  return LDPS_OK;
}


/* Register a claim-file handler, version 2. */

static enum ld_plugin_status
register_claim_file_v2 (ld_plugin_claim_file_handler_v2 handler)
{
  current_plugin->claim_file_v2 = handler;
  return LDPS_OK;
}

static enum ld_plugin_status
add_symbols (void * handle,
	     int nsyms,
	     const struct ld_plugin_symbol * syms)
{
  bfd *abfd = handle;
  struct plugin_data_struct *plugin_data =
    bfd_alloc (abfd, sizeof (plugin_data_struct));

  if (!plugin_data)
    return LDPS_ERR;

  plugin_data->nsyms = nsyms;
  plugin_data->syms = syms;

  if (nsyms != 0)
    abfd->flags |= HAS_SYMS;

  abfd->tdata.plugin_data = plugin_data;
  return LDPS_OK;
}

static enum ld_plugin_status
add_symbols_v2 (void *handle, int nsyms,
		const struct ld_plugin_symbol *syms)
{
  current_plugin->has_symbol_type = true;
  return add_symbols (handle, nsyms, syms);
}

int
bfd_plugin_open_input (bfd *ibfd, struct ld_plugin_input_file *file)
{
  bfd *iobfd;
  int fd;

  iobfd = ibfd;
  while (iobfd->my_archive
	 && !bfd_is_thin_archive (iobfd->my_archive))
    iobfd = iobfd->my_archive;
  file->name = bfd_get_filename (iobfd);

  if (!iobfd->iostream && !bfd_open_file (iobfd))
    return 0;

  /* Reuse the archive plugin file descriptor.  */
  if (iobfd != ibfd)
    fd = iobfd->archive_plugin_fd;
  else
    fd = -1;

  if (fd < 0)
    {
      /* The plugin API expects that the file descriptor won't be closed
	 and reused as done by the bfd file cache.  So open it again.
	 dup isn't good enough.  plugin IO uses lseek/read while BFD uses
	 fseek/fread.  It isn't wise to mix the unistd and stdio calls on
	 the same underlying file descriptor.  */
      fd = open (file->name, O_RDONLY | O_BINARY);
      if (fd < 0)
	{
#ifndef EMFILE
	  return 0;
#else
	  if (errno != EMFILE)
	    return 0;

#ifdef HAVE_GETRLIMIT
	  struct rlimit lim;

	  /* Complicated links involving lots of files and/or large
	     archives can exhaust the number of file descriptors
	     available to us.  If possible, try to allocate more
	     descriptors.  */
	  if (getrlimit (RLIMIT_NOFILE, & lim) == 0
	      && lim.rlim_cur < lim.rlim_max)
	    {
	      lim.rlim_cur = lim.rlim_max;
	      if (setrlimit (RLIMIT_NOFILE, &lim) == 0)
		fd = open (file->name, O_RDONLY | O_BINARY);
	    }

	  if (fd < 0)
#endif
	    {
	      _bfd_error_handler (_("plugin framework: out of file descriptors. Try using fewer objects/archives\n"));
	      return 0;
	    }
#endif
	}
    }

  if (iobfd == ibfd)
    {
      struct stat stat_buf;

      if (fstat (fd, &stat_buf))
	{
	  close (fd);
	  return 0;
	}

      file->offset = 0;
      file->filesize = stat_buf.st_size;
    }
  else
    {
      /* Cache the archive plugin file descriptor.  */
      iobfd->archive_plugin_fd = fd;
      iobfd->archive_plugin_fd_open_count++;

      file->offset = ibfd->origin;
      file->filesize = arelt_size (ibfd);
    }

  file->fd = fd;
  return 1;
}

/* Close the plugin file descriptor FD.  If ABFD isn't NULL, it is an
   archive member.   */

void
bfd_plugin_close_file_descriptor (bfd *abfd, int fd)
{
  if (abfd == NULL)
    close (fd);
  else
    {
      while (abfd->my_archive
	     && !bfd_is_thin_archive (abfd->my_archive))
	abfd = abfd->my_archive;

      /* Close the file descriptor if there is no archive plugin file
	 descriptor.  */
      if (abfd->archive_plugin_fd == -1)
	{
	  close (fd);
	  return;
	}

      abfd->archive_plugin_fd_open_count--;
      /* Dup the archive plugin file descriptor for later use, which
	 will be closed by _bfd_archive_close_and_cleanup.  */
      if (abfd->archive_plugin_fd_open_count == 0)
	{
	  abfd->archive_plugin_fd = dup (fd);
	  close (fd);
	}
    }
}

static int
try_claim (bfd *abfd)
{
  int claimed = 0;
  struct ld_plugin_input_file file;

  file.handle = abfd;
  if (bfd_plugin_open_input (abfd, &file)
      && current_plugin->claim_file)
    {
      current_plugin->claim_file (&file, &claimed);
      bfd_plugin_close_file_descriptor ((abfd->my_archive != NULL
					 ? abfd : NULL),
					file.fd);
    }

  return claimed;
}

static bool
try_load_plugin (const char *pname,
		 struct plugin_list_entry *plugin_list_iter,
		 bfd *abfd,
		 bool build_list_p)
{
  void *plugin_handle;
  struct ld_plugin_tv tv[6];
  int i;
  ld_plugin_onload onload;
  enum ld_plugin_status status;
  bool result = false;

  /* NB: Each object is independent.  Reuse the previous plugin from
     the last run will lead to wrong result.  */
  if (current_plugin)
    memset (current_plugin, 0,
	    offsetof (struct plugin_list_entry, next));

  if (plugin_list_iter)
    pname = plugin_list_iter->plugin_name;

  plugin_handle = dlopen (pname, RTLD_NOW);
  if (!plugin_handle)
    {
      /* If we are building a list of viable plugins, then
	 we do not bother the user with the details of any
	 plugins that cannot be loaded.  */
      if (! build_list_p)
	_bfd_error_handler ("Failed to load plugin '%s', reason: %s\n",
			    pname, dlerror ());
      return false;
    }

  if (plugin_list_iter == NULL)
    {
      size_t length_plugin_name = strlen (pname) + 1;
      char *plugin_name = bfd_malloc (length_plugin_name);

      if (plugin_name == NULL)
	goto short_circuit;
      plugin_list_iter = bfd_malloc (sizeof *plugin_list_iter);
      if (plugin_list_iter == NULL)
	{
	  free (plugin_name);
	  goto short_circuit;
	}
      /* Make a copy of PNAME since PNAME from load_plugin () will be
	 freed.  */
      memcpy (plugin_name, pname, length_plugin_name);
      memset (plugin_list_iter, 0, sizeof (*plugin_list_iter));
      plugin_list_iter->plugin_name = plugin_name;
      plugin_list_iter->next = plugin_list;
      plugin_list = plugin_list_iter;
    }

  current_plugin = plugin_list_iter;
  if (build_list_p)
    goto short_circuit;

  onload = dlsym (plugin_handle, "onload");
  if (!onload)
    goto short_circuit;

  i = 0;
  tv[i].tv_tag = LDPT_MESSAGE;
  tv[i].tv_u.tv_message = message;

  ++i;
  tv[i].tv_tag = LDPT_REGISTER_CLAIM_FILE_HOOK;
  tv[i].tv_u.tv_register_claim_file = register_claim_file;

  ++i;
  tv[i].tv_tag = LDPT_REGISTER_CLAIM_FILE_HOOK_V2;
  tv[i].tv_u.tv_register_claim_file_v2 = register_claim_file_v2;

  ++i;
  tv[i].tv_tag = LDPT_ADD_SYMBOLS;
  tv[i].tv_u.tv_add_symbols = add_symbols;

  ++i;
  tv[i].tv_tag = LDPT_ADD_SYMBOLS_V2;
  tv[i].tv_u.tv_add_symbols = add_symbols_v2;

  ++i;
  tv[i].tv_tag = LDPT_NULL;
  tv[i].tv_u.tv_val = 0;

  /* LTO plugin will call handler hooks to set up plugin handlers.  */
  status = (*onload)(tv);

  if (status != LDPS_OK)
    goto short_circuit;

  abfd->plugin_format = bfd_plugin_no;

  if (!current_plugin->claim_file)
    goto short_circuit;

  if (!try_claim (abfd))
    goto short_circuit;

  abfd->plugin_format = bfd_plugin_yes;
  result = true;

 short_circuit:
  dlclose (plugin_handle);
  return result;
}

/* There may be plugin libraries in lib/bfd-plugins.  */
static int has_plugin_list = -1;

static bfd_cleanup (*ld_plugin_object_p) (bfd *, bool);

static const char *plugin_name;

void
bfd_plugin_set_plugin (const char *p)
{
  plugin_name = p;
}

/* Return TRUE if a plugin library is used.  */

bool
bfd_plugin_specified_p (void)
{
  return plugin_list != NULL;
}

/* Return TRUE if ABFD can be claimed by linker LTO plugin.  */

bool
bfd_link_plugin_object_p (bfd *abfd)
{
  if (ld_plugin_object_p)
    return ld_plugin_object_p (abfd, false) != NULL;
  return false;
}

extern const bfd_target plugin_vec;

/* Return TRUE if TARGET is a pointer to plugin_vec.  */

bool
bfd_plugin_target_p (const bfd_target *target)
{
  return target == &plugin_vec;
}

/* Register OBJECT_P to be used by bfd_plugin_object_p.  */

void
register_ld_plugin_object_p (bfd_cleanup (*object_p) (bfd *, bool))
{
  ld_plugin_object_p = object_p;
}

static void
build_plugin_list (bfd *abfd)
{
  /* The intent was to search ${libdir}/bfd-plugins for plugins, but
     unfortunately the original implementation wasn't precisely that
     when configuring binutils using --libdir.  Search in the proper
     path first, then the old one for backwards compatibility.  */
  static const char *path[]
    = { LIBDIR "/bfd-plugins", BINDIR "/../lib/bfd-plugins" };
  struct stat last_st;
  unsigned int i;

  if (has_plugin_list >= 0)
    return;

  /* Try not to search the same dir twice, by looking at st_dev and
     st_ino for the dir.  If we are on a file system that always sets
     st_ino to zero or the actual st_ino is zero we might waste some
     time, but that doesn't matter too much.  */
  last_st.st_dev = 0;
  last_st.st_ino = 0;
  for (i = 0; i < sizeof (path) / sizeof (path[0]); i++)
    {
      char *plugin_dir = make_relative_prefix (plugin_program_name,
					       BINDIR,
					       path[i]);
      if (plugin_dir)
	{
	  struct stat st;
	  DIR *d;

	  if (stat (plugin_dir, &st) == 0
	      && S_ISDIR (st.st_mode)
	      && !(last_st.st_dev == st.st_dev
		   && last_st.st_ino == st.st_ino
		   && st.st_ino != 0)
	      && (d = opendir (plugin_dir)) != NULL)
	    {
	      struct dirent *ent;

	      last_st.st_dev = st.st_dev;
	      last_st.st_ino = st.st_ino;
	      while ((ent = readdir (d)) != NULL)
		{
		  char *full_name;

		  full_name = concat (plugin_dir, "/", ent->d_name, NULL);
		  if (stat (full_name, &st) == 0 && S_ISREG (st.st_mode))
		    (void) try_load_plugin (full_name, NULL, abfd, true);
		  free (full_name);
		}
	      closedir (d);
	    }
	  free (plugin_dir);
	}
    }

  has_plugin_list = plugin_list != NULL;
}

static bool
load_plugin (bfd *abfd)
{
  struct plugin_list_entry *plugin_list_iter;

  if (plugin_name)
    return try_load_plugin (plugin_name, plugin_list, abfd, false);

  if (plugin_program_name == NULL)
    return false;

  build_plugin_list (abfd);

  for (plugin_list_iter = plugin_list;
       plugin_list_iter;
       plugin_list_iter = plugin_list_iter->next)
    if (try_load_plugin (NULL, plugin_list_iter, abfd, false))
      return true;

  return false;
}


static bfd_cleanup
bfd_plugin_object_p (bfd *abfd)
{
  if (ld_plugin_object_p)
    return ld_plugin_object_p (abfd, false);

  if (abfd->plugin_format == bfd_plugin_unknown && !load_plugin (abfd))
    return NULL;

  return abfd->plugin_format == bfd_plugin_yes ? _bfd_no_cleanup : NULL;
}

/* Copy any private info we understand from the input bfd
   to the output bfd.  */

static bool
bfd_plugin_bfd_copy_private_bfd_data (bfd *ibfd ATTRIBUTE_UNUSED,
				      bfd *obfd ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return true;
}

/* Copy any private info we understand from the input section
   to the output section.  */

static bool
bfd_plugin_bfd_copy_private_section_data (bfd *ibfd ATTRIBUTE_UNUSED,
					  asection *isection ATTRIBUTE_UNUSED,
					  bfd *obfd ATTRIBUTE_UNUSED,
					  asection *osection ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return true;
}

/* Copy any private info we understand from the input symbol
   to the output symbol.  */

static bool
bfd_plugin_bfd_copy_private_symbol_data (bfd *ibfd ATTRIBUTE_UNUSED,
					 asymbol *isymbol ATTRIBUTE_UNUSED,
					 bfd *obfd ATTRIBUTE_UNUSED,
					 asymbol *osymbol ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return true;
}

static bool
bfd_plugin_bfd_print_private_bfd_data (bfd *abfd ATTRIBUTE_UNUSED, void *ptr ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return true;
}

static char *
bfd_plugin_core_file_failing_command (bfd *abfd ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return NULL;
}

static int
bfd_plugin_core_file_failing_signal (bfd *abfd ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return 0;
}

static int
bfd_plugin_core_file_pid (bfd *abfd ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return 0;
}

static long
bfd_plugin_get_symtab_upper_bound (bfd *abfd)
{
  struct plugin_data_struct *plugin_data = abfd->tdata.plugin_data;
  long nsyms = plugin_data->nsyms;

  BFD_ASSERT (nsyms >= 0);

  return ((nsyms + 1) * sizeof (asymbol *));
}

static flagword
convert_flags (const struct ld_plugin_symbol *sym)
{
 switch (sym->def)
   {
   case LDPK_DEF:
   case LDPK_COMMON:
   case LDPK_UNDEF:
     return BSF_GLOBAL;

   case LDPK_WEAKUNDEF:
   case LDPK_WEAKDEF:
     return BSF_GLOBAL | BSF_WEAK;

   default:
     BFD_ASSERT (0);
     return 0;
   }
}

static long
bfd_plugin_canonicalize_symtab (bfd *abfd,
				asymbol **alocation)
{
  struct plugin_data_struct *plugin_data = abfd->tdata.plugin_data;
  long nsyms = plugin_data->nsyms;
  const struct ld_plugin_symbol *syms = plugin_data->syms;
  static asection fake_text_section
    = BFD_FAKE_SECTION (fake_text_section, NULL, "plug", 0,
			SEC_ALLOC | SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS);
  static asection fake_data_section
    = BFD_FAKE_SECTION (fake_data_section, NULL, "plug", 0,
			SEC_ALLOC | SEC_LOAD | SEC_DATA | SEC_HAS_CONTENTS);
  static asection fake_bss_section
    = BFD_FAKE_SECTION (fake_bss_section, NULL, "plug", 0,
			SEC_ALLOC);
  static asection fake_common_section
    = BFD_FAKE_SECTION (fake_common_section, NULL, "plug", 0, SEC_IS_COMMON);
  int i;

  for (i = 0; i < nsyms; i++)
    {
      asymbol *s = bfd_alloc (abfd, sizeof (asymbol));

      BFD_ASSERT (s);
      alocation[i] = s;

      s->the_bfd = abfd;
      s->name = syms[i].name;
      s->value = 0;
      s->flags = convert_flags (&syms[i]);
      switch (syms[i].def)
	{
	case LDPK_COMMON:
	  s->section = &fake_common_section;
	  break;
	case LDPK_UNDEF:
	case LDPK_WEAKUNDEF:
	  s->section = bfd_und_section_ptr;
	  break;
	case LDPK_DEF:
	case LDPK_WEAKDEF:
	  if (current_plugin->has_symbol_type)
	    switch (syms[i].symbol_type)
	      {
	      default:
		/* FIXME: Should we issue an error here ?  */
	      case LDST_UNKNOWN:
		/* What is the best fake section for LDST_UNKNOWN?  */
	      case LDST_FUNCTION:
		s->section = &fake_text_section;
		break;
	      case LDST_VARIABLE:
		if (syms[i].section_kind == LDSSK_BSS)
		  s->section = &fake_bss_section;
		else
		  s->section = &fake_data_section;
		break;
	      }
	  else
	    s->section = &fake_text_section;
	  break;
	default:
	  BFD_ASSERT (0);
	}

      s->udata.p = (void *) &syms[i];
    }

  return nsyms;
}

static void
bfd_plugin_print_symbol (bfd *abfd ATTRIBUTE_UNUSED,
			 void *afile ATTRIBUTE_UNUSED,
			 asymbol *symbol ATTRIBUTE_UNUSED,
			 bfd_print_symbol_type how ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
}

static void
bfd_plugin_get_symbol_info (bfd *abfd ATTRIBUTE_UNUSED,
			    asymbol *symbol,
			    symbol_info *ret)
{
  bfd_symbol_info (symbol, ret);
}

/* Make an empty symbol. */

static asymbol *
bfd_plugin_make_empty_symbol (bfd *abfd)
{
  asymbol *new_symbol = bfd_zalloc (abfd, sizeof (asymbol));
  if (new_symbol == NULL)
    return new_symbol;
  new_symbol->the_bfd = abfd;
  return new_symbol;
}

static int
bfd_plugin_sizeof_headers (bfd *a ATTRIBUTE_UNUSED,
			   struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return 0;
}

const bfd_target plugin_vec =
{
  "plugin",			/* Name.  */
  bfd_target_unknown_flavour,
  BFD_ENDIAN_LITTLE,		/* Target byte order.  */
  BFD_ENDIAN_LITTLE,		/* Target headers byte order.  */
  (HAS_RELOC | EXEC_P |		/* Object flags.  */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | DYNAMIC | WP_TEXT | D_PAGED),
  (SEC_CODE | SEC_DATA | SEC_ROM | SEC_HAS_CONTENTS
   | SEC_ALLOC | SEC_LOAD | SEC_RELOC),	/* Section flags.  */
  0,				/* symbol_leading_char.  */
  '/',				/* ar_pad_char.  */
  15,				/* ar_max_namelen.  */
  255,				/* match priority.  */
  TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */

  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,	/* data */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,	/* hdrs */

  {				/* bfd_check_format.  */
    _bfd_dummy_target,
    bfd_plugin_object_p,
    bfd_generic_archive_p,
    _bfd_dummy_target
  },
  {				/* bfd_set_format.  */
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_generic_mkarchive,
    _bfd_bool_bfd_false_error,
  },
  {				/* bfd_write_contents.  */
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_write_archive_contents,
    _bfd_bool_bfd_false_error,
  },

  BFD_JUMP_TABLE_GENERIC (bfd_plugin),
  BFD_JUMP_TABLE_COPY (bfd_plugin),
  BFD_JUMP_TABLE_CORE (bfd_plugin),
#ifdef USE_64_BIT_ARCHIVE
  BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_64_bit),
#else
  BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_coff),
#endif
  BFD_JUMP_TABLE_SYMBOLS (bfd_plugin),
  BFD_JUMP_TABLE_RELOCS (_bfd_norelocs),
  BFD_JUMP_TABLE_WRITE (bfd_plugin),
  BFD_JUMP_TABLE_LINK (bfd_plugin),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  NULL,

  NULL				/* backend_data.  */
};
#endif /* BFD_SUPPORTS_PLUGINS */

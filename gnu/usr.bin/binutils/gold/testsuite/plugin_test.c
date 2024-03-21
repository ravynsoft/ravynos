/* test_plugin.c -- simple linker plugin test

   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Written by Cary Coutant <ccoutant@google.com>.

   This file is part of gold.

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plugin-api.h"

struct claimed_file
{
  const char* name;
  void* handle;
  int nsyms;
  struct ld_plugin_symbol* syms;
  struct claimed_file* next;
};

struct sym_info
{
  int size;
  char* type;
  char* bind;
  char* vis;
  char* sect;
  char* name;
  char* ver;
};

static struct claimed_file* first_claimed_file = NULL;
static struct claimed_file* last_claimed_file = NULL;

static ld_plugin_register_claim_file register_claim_file_hook = NULL;
static ld_plugin_register_all_symbols_read register_all_symbols_read_hook = NULL;
static ld_plugin_register_cleanup register_cleanup_hook = NULL;
static ld_plugin_add_symbols add_symbols = NULL;
static ld_plugin_get_symbols get_symbols = NULL;
static ld_plugin_get_symbols get_symbols_v2 = NULL;
static ld_plugin_get_symbols get_symbols_v3 = NULL;
static ld_plugin_add_input_file add_input_file = NULL;
static ld_plugin_message message = NULL;
static ld_plugin_get_input_file get_input_file = NULL;
static ld_plugin_release_input_file release_input_file = NULL;
static ld_plugin_get_input_section_count get_input_section_count = NULL;
static ld_plugin_get_input_section_type get_input_section_type = NULL;
static ld_plugin_get_input_section_name get_input_section_name = NULL;
static ld_plugin_get_input_section_contents get_input_section_contents = NULL;
static ld_plugin_update_section_order update_section_order = NULL;
static ld_plugin_allow_section_ordering allow_section_ordering = NULL;
static ld_plugin_get_wrap_symbols get_wrap_symbols = NULL;

#define MAXOPTS 10

static const char *opts[MAXOPTS];
static int nopts = 0;

enum ld_plugin_status onload(struct ld_plugin_tv *tv);
enum ld_plugin_status claim_file_hook(const struct ld_plugin_input_file *file,
                                      int *claimed);
enum ld_plugin_status all_symbols_read_hook(void);
enum ld_plugin_status cleanup_hook(void);

static void parse_readelf_line(char*, struct sym_info*);

enum ld_plugin_status
onload(struct ld_plugin_tv *tv)
{
  struct ld_plugin_tv *entry;
  int api_version = 0;
  int gold_version = 0;
  int i;

  for (entry = tv; entry->tv_tag != LDPT_NULL; ++entry)
    {
      switch (entry->tv_tag)
        {
        case LDPT_API_VERSION:
          api_version = entry->tv_u.tv_val;
          break;
        case LDPT_GOLD_VERSION:
          gold_version = entry->tv_u.tv_val;
          break;
        case LDPT_LINKER_OUTPUT:
          break;
        case LDPT_OPTION:
          if (nopts < MAXOPTS)
            opts[nopts++] = entry->tv_u.tv_string;
          break;
        case LDPT_REGISTER_CLAIM_FILE_HOOK:
          register_claim_file_hook = entry->tv_u.tv_register_claim_file;
          break;
        case LDPT_REGISTER_ALL_SYMBOLS_READ_HOOK:
          register_all_symbols_read_hook =
            entry->tv_u.tv_register_all_symbols_read;
          break;
        case LDPT_REGISTER_CLEANUP_HOOK:
          register_cleanup_hook = entry->tv_u.tv_register_cleanup;
          break;
        case LDPT_ADD_SYMBOLS:
          add_symbols = entry->tv_u.tv_add_symbols;
          break;
        case LDPT_GET_SYMBOLS:
          get_symbols = entry->tv_u.tv_get_symbols;
          break;
        case LDPT_GET_SYMBOLS_V2:
          get_symbols_v2 = entry->tv_u.tv_get_symbols;
          break;
        case LDPT_GET_SYMBOLS_V3:
          get_symbols_v3 = entry->tv_u.tv_get_symbols;
          break;
        case LDPT_ADD_INPUT_FILE:
          add_input_file = entry->tv_u.tv_add_input_file;
          break;
        case LDPT_MESSAGE:
          message = entry->tv_u.tv_message;
          break;
        case LDPT_GET_INPUT_FILE:
          get_input_file = entry->tv_u.tv_get_input_file;
          break;
        case LDPT_RELEASE_INPUT_FILE:
          release_input_file = entry->tv_u.tv_release_input_file;
          break;
        case LDPT_GET_INPUT_SECTION_COUNT:
          get_input_section_count = *entry->tv_u.tv_get_input_section_count;
          break;
        case LDPT_GET_INPUT_SECTION_TYPE:
          get_input_section_type = *entry->tv_u.tv_get_input_section_type;
          break;
        case LDPT_GET_INPUT_SECTION_NAME:
          get_input_section_name = *entry->tv_u.tv_get_input_section_name;
          break;
        case LDPT_GET_INPUT_SECTION_CONTENTS:
          get_input_section_contents = *entry->tv_u.tv_get_input_section_contents;
          break;
	case LDPT_UPDATE_SECTION_ORDER:
	  update_section_order = *entry->tv_u.tv_update_section_order;
	  break;
	case LDPT_ALLOW_SECTION_ORDERING:
	  allow_section_ordering = *entry->tv_u.tv_allow_section_ordering;
	  break;
	case LDPT_GET_WRAP_SYMBOLS:
	  get_wrap_symbols = *entry->tv_u.tv_get_wrap_symbols;
	  break;
        default:
          break;
        }
    }

  if (message == NULL)
    {
      fprintf(stderr, "tv_message interface missing\n");
      return LDPS_ERR;
    }

  if (register_claim_file_hook == NULL)
    {
      fprintf(stderr, "tv_register_claim_file_hook interface missing\n");
      return LDPS_ERR;
    }

  if (register_all_symbols_read_hook == NULL)
    {
      fprintf(stderr, "tv_register_all_symbols_read_hook interface missing\n");
      return LDPS_ERR;
    }

  if (register_cleanup_hook == NULL)
    {
      fprintf(stderr, "tv_register_cleanup_hook interface missing\n");
      return LDPS_ERR;
    }

  (*message)(LDPL_INFO, "API version:   %d", api_version);
  (*message)(LDPL_INFO, "gold version:  %d", gold_version);

  for (i = 0; i < nopts; ++i)
    (*message)(LDPL_INFO, "option: %s", opts[i]);

  if ((*register_claim_file_hook)(claim_file_hook) != LDPS_OK)
    {
      (*message)(LDPL_ERROR, "error registering claim file hook");
      return LDPS_ERR;
    }

  if ((*register_all_symbols_read_hook)(all_symbols_read_hook) != LDPS_OK)
    {
      (*message)(LDPL_ERROR, "error registering all symbols read hook");
      return LDPS_ERR;
    }

  if ((*register_cleanup_hook)(cleanup_hook) != LDPS_OK)
    {
      (*message)(LDPL_ERROR, "error registering cleanup hook");
      return LDPS_ERR;
    }

  if (get_input_section_count == NULL)
    {
      fprintf(stderr, "tv_get_input_section_count interface missing\n");
      return LDPS_ERR;
    }

  if (get_input_section_type == NULL)
    {
      fprintf(stderr, "tv_get_input_section_type interface missing\n");
      return LDPS_ERR;
    }

  if (get_input_section_name == NULL)
    {
      fprintf(stderr, "tv_get_input_section_name interface missing\n");
      return LDPS_ERR;
    }

  if (get_input_section_contents == NULL)
    {
      fprintf(stderr, "tv_get_input_section_contents interface missing\n");
      return LDPS_ERR;
    }

  if (update_section_order == NULL)
    {
      fprintf(stderr, "tv_update_section_order interface missing\n");
      return LDPS_ERR;
    }

  if (allow_section_ordering == NULL)
    {
      fprintf(stderr, "tv_allow_section_ordering interface missing\n");
      return LDPS_ERR;
    }

  if (get_wrap_symbols == NULL)
    {
      fprintf(stderr, "tv_get_wrap_symbols interface missing\n");
      return LDPS_ERR;
    }
  else
    {
      const char **wrap_symbols;
      uint64_t count = 0;
      if (get_wrap_symbols(&count, &wrap_symbols) == LDPS_OK)
	{
	  (*message)(LDPL_INFO, "Number of wrap symbols = %lu", count);
	  for (; count > 0; --count)
            (*message)(LDPL_INFO, "Wrap symbol %s", wrap_symbols[count - 1]);
	}
      else
	{
          fprintf(stderr, "tv_get_wrap_symbols interface call failed\n");
          return LDPS_ERR;
	}
    }

  return LDPS_OK;
}

enum ld_plugin_status
claim_file_hook (const struct ld_plugin_input_file* file, int* claimed)
{
  int len;
  off_t end_offset;
  char buf[160];
  struct claimed_file* claimed_file;
  struct ld_plugin_symbol* syms;
  int nsyms = 0;
  int maxsyms = 0;
  FILE* irfile;
  struct sym_info info;
  int weak;
  int def;
  int vis;
  int is_comdat;
  int i;
  int irfile_was_opened = 0;
  char syms_name[80];

  (*message)(LDPL_INFO,
             "%s: claim file hook called (offset = %ld, size = %ld)",
             file->name, (long)file->offset, (long)file->filesize);

  /* Look for matching syms file for an archive member.  */
  if (file->offset == 0)
    snprintf(syms_name, sizeof(syms_name), "%s.syms", file->name);
  else
    snprintf(syms_name, sizeof(syms_name), "%s-%d.syms",
	     file->name, (int)file->offset);
  irfile = fopen(syms_name, "r");
  if (irfile != NULL)
    {
      irfile_was_opened = 1;
      end_offset = 1 << 20;
    }

  /* Otherwise, see if the file itself is a syms file.  */
  if (!irfile_was_opened)
    {
      irfile = fdopen(file->fd, "r");
      (void)fseek(irfile, file->offset, SEEK_SET);
      end_offset = file->offset + file->filesize;
    }

  /* Look for the beginning of output from readelf -s.  */
  len = fread(buf, 1, 13, irfile);
  if (len < 13 || strncmp(buf, "\nSymbol table", 13) != 0)
    return LDPS_OK;

  /* Skip the two header lines.  */
  (void) fgets(buf, sizeof(buf), irfile);
  (void) fgets(buf, sizeof(buf), irfile);

  if (add_symbols == NULL)
    {
      fprintf(stderr, "tv_add_symbols interface missing\n");
      return LDPS_ERR;
    }

  /* Parse the output from readelf. The columns are:
     Index Value Size Type Binding Visibility Section Name.  */
  syms = (struct ld_plugin_symbol*)malloc(sizeof(struct ld_plugin_symbol) * 8);
  if (syms == NULL)
    return LDPS_ERR;
  maxsyms = 8;
  while (ftell(irfile) < end_offset
         && fgets(buf, sizeof(buf), irfile) != NULL)
    {
      parse_readelf_line(buf, &info);

      /* Ignore local symbols.  */
      if (strncmp(info.bind, "LOCAL", 5) == 0)
        continue;

      weak = strncmp(info.bind, "WEAK", 4) == 0;
      if (strncmp(info.sect, "UND", 3) == 0)
        def = weak ? LDPK_WEAKUNDEF : LDPK_UNDEF;
      else if (strncmp(info.sect, "COM", 3) == 0)
        def = LDPK_COMMON;
      else
        def = weak ? LDPK_WEAKDEF : LDPK_DEF;

      if (strncmp(info.vis, "INTERNAL", 8) == 0)
        vis = LDPV_INTERNAL;
      else if (strncmp(info.vis, "HIDDEN", 6) == 0)
        vis = LDPV_HIDDEN;
      else if (strncmp(info.vis, "PROTECTED", 9) == 0)
        vis = LDPV_PROTECTED;
      else
        vis = LDPV_DEFAULT;

      /* If the symbol is listed in the options list, special-case
         it as a comdat symbol.  */
      is_comdat = 0;
      for (i = 0; i < nopts; ++i)
        {
          if (info.name != NULL && strcmp(info.name, opts[i]) == 0)
            {
              is_comdat = 1;
              break;
            }
        }

      if (nsyms >= maxsyms)
        {
          syms = (struct ld_plugin_symbol*)
            realloc(syms, sizeof(struct ld_plugin_symbol) * maxsyms * 2);
          if (syms == NULL)
            return LDPS_ERR;
          maxsyms *= 2;
        }

      if (info.name == NULL)
        syms[nsyms].name = NULL;
      else
        {
          len = strlen(info.name);
          syms[nsyms].name = malloc(len + 1);
          strncpy(syms[nsyms].name, info.name, len + 1);
        }
      if (info.ver == NULL)
        syms[nsyms].version = NULL;
      else
        {
          len = strlen(info.ver);
          syms[nsyms].version = malloc(len + 1);
          strncpy(syms[nsyms].version, info.ver, len + 1);
        }
      syms[nsyms].def = def;
      syms[nsyms].visibility = vis;
      syms[nsyms].size = info.size;
      syms[nsyms].comdat_key = is_comdat ? syms[nsyms].name : NULL;
      syms[nsyms].resolution = LDPR_UNKNOWN;
      ++nsyms;
    }

  claimed_file = (struct claimed_file*) malloc(sizeof(struct claimed_file));
  if (claimed_file == NULL)
    return LDPS_ERR;

  claimed_file->name = file->name;
  claimed_file->handle = file->handle;
  claimed_file->nsyms = nsyms;
  claimed_file->syms = syms;
  claimed_file->next = NULL;
  if (last_claimed_file == NULL)
    first_claimed_file = claimed_file;
  else
    last_claimed_file->next = claimed_file;
  last_claimed_file = claimed_file;

  (*message)(LDPL_INFO, "%s: claiming file, adding %d symbols",
             file->name, nsyms);

  if (nsyms > 0)
    (*add_symbols)(file->handle, nsyms, syms);

  *claimed = 1;
  if (irfile_was_opened)
    fclose(irfile);
  return LDPS_OK;
}

enum ld_plugin_status
all_symbols_read_hook(void)
{
  int i;
  const char* res;
  struct claimed_file* claimed_file;
  struct ld_plugin_input_file file;
  FILE* irfile;
  off_t end_offset;
  struct sym_info info;
  int len;
  char buf[160];
  char* p;
  const char* filename;

  (*message)(LDPL_INFO, "all symbols read hook called");

  if (get_symbols_v3 == NULL)
    {
      fprintf(stderr, "tv_get_symbols (v3) interface missing\n");
      return LDPS_ERR;
    }

  for (claimed_file = first_claimed_file;
       claimed_file != NULL;
       claimed_file = claimed_file->next)
    {
      enum ld_plugin_status status = (*get_symbols_v3)(
          claimed_file->handle, claimed_file->nsyms, claimed_file->syms);
      if (status == LDPS_NO_SYMS)
        {
          (*message)(LDPL_INFO, "%s: no symbols", claimed_file->name);
          continue;
        }

      for (i = 0; i < claimed_file->nsyms; ++i)
        {
          switch (claimed_file->syms[i].resolution)
            {
            case LDPR_UNKNOWN:
              res = "UNKNOWN";
              break;
            case LDPR_UNDEF:
              res = "UNDEF";
              break;
            case LDPR_PREVAILING_DEF:
              res = "PREVAILING_DEF_REG";
              break;
            case LDPR_PREVAILING_DEF_IRONLY:
              res = "PREVAILING_DEF_IRONLY";
              break;
            case LDPR_PREVAILING_DEF_IRONLY_EXP:
              res = "PREVAILING_DEF_IRONLY_EXP";
              break;
            case LDPR_PREEMPTED_REG:
              res = "PREEMPTED_REG";
              break;
            case LDPR_PREEMPTED_IR:
              res = "PREEMPTED_IR";
              break;
            case LDPR_RESOLVED_IR:
              res = "RESOLVED_IR";
              break;
            case LDPR_RESOLVED_EXEC:
              res = "RESOLVED_EXEC";
              break;
            case LDPR_RESOLVED_DYN:
              res = "RESOLVED_DYN";
              break;
            default:
              res = "?";
              break;
            }
          (*message)(LDPL_INFO, "%s: %s: %s", claimed_file->name,
                     claimed_file->syms[i].name, res);
        }
    }

  if (add_input_file == NULL)
    {
      fprintf(stderr, "tv_add_input_file interface missing\n");
      return LDPS_ERR;
    }
  if (get_input_file == NULL)
    {
      fprintf(stderr, "tv_get_input_file interface missing\n");
      return LDPS_ERR;
    }
  if (release_input_file == NULL)
    {
      fprintf(stderr, "tv_release_input_file interface missing\n");
      return LDPS_ERR;
    }

  for (claimed_file = first_claimed_file;
       claimed_file != NULL;
       claimed_file = claimed_file->next)
    {
      int irfile_was_opened = 0;
      char syms_name[80];

      (*get_input_file) (claimed_file->handle, &file);

      if (file.offset == 0)
	snprintf(syms_name, sizeof(syms_name), "%s.syms", file.name);
      else
	snprintf(syms_name, sizeof(syms_name), "%s-%d.syms",
		 file.name, (int)file.offset);
      irfile = fopen(syms_name, "r");
      if (irfile != NULL)
	{
	  irfile_was_opened = 1;
	  end_offset = 1 << 20;
	}

      if (!irfile_was_opened)
	{
	  irfile = fdopen(file.fd, "r");
	  (void)fseek(irfile, file.offset, SEEK_SET);
	  end_offset = file.offset + file.filesize;
	}

      /* Look for the beginning of output from readelf -s.  */
      len = fread(buf, 1, 13, irfile);
      if (len < 13 || strncmp(buf, "\nSymbol table", 13) != 0)
        {
          fprintf(stderr, "%s: can't re-read original input file\n",
                  claimed_file->name);
          return LDPS_ERR;
        }

      /* Skip the two header lines.  */
      (void) fgets(buf, sizeof(buf), irfile);
      (void) fgets(buf, sizeof(buf), irfile);

      filename = NULL;
      while (ftell(irfile) < end_offset
             && fgets(buf, sizeof(buf), irfile) != NULL)
        {
          parse_readelf_line(buf, &info);

          /* Look for file name.  */
          if (strncmp(info.type, "FILE", 4) == 0)
            {
              len = strlen(info.name);
              p = malloc(len + 1);
              strncpy(p, info.name, len + 1);
              filename = p;
              break;
            }
        }

      if (irfile_was_opened)
	fclose(irfile);

      (*release_input_file) (claimed_file->handle);

      if (filename == NULL)
        filename = claimed_file->name;

      if (claimed_file->nsyms == 0)
        continue;

      if (strlen(filename) >= sizeof(buf))
        {
          (*message)(LDPL_FATAL, "%s: filename too long", filename);
          return LDPS_ERR;
        }
      strcpy(buf, filename);
      p = strrchr(buf, '.');
      if (p == NULL
          || (strcmp(p, ".syms") != 0
              && strcmp(p, ".c") != 0
              && strcmp(p, ".cc") != 0))
        {
          (*message)(LDPL_FATAL, "%s: filename has unknown suffix",
                     filename);
          return LDPS_ERR;
        }
      p[1] = 'o';
      p[2] = '\0';
      (*message)(LDPL_INFO, "%s: adding new input file", buf);
      (*add_input_file)(buf);
    }

  return LDPS_OK;
}

enum ld_plugin_status
cleanup_hook(void)
{
  (*message)(LDPL_INFO, "cleanup hook called");
  return LDPS_OK;
}

static void
parse_readelf_line(char* p, struct sym_info* info)
{
  int len;

  p += strspn(p, " ");

  /* Index field.  */
  p += strcspn(p, " ");
  p += strspn(p, " ");

  /* Value field.  */
  p += strcspn(p, " ");
  p += strspn(p, " ");

  /* Size field.  */
  info->size = atoi(p);
  p += strcspn(p, " ");
  p += strspn(p, " ");

  /* Type field.  */
  info->type = p;
  p += strcspn(p, " ");
  p += strspn(p, " ");

  /* Binding field.  */
  info->bind = p;
  p += strcspn(p, " ");
  p += strspn(p, " ");

  /* Visibility field.  */
  info->vis = p;
  p += strcspn(p, " ");
  p += strspn(p, " ");

  if (*p == '[')
    {
      /* Skip st_other.  */
      p += strcspn(p, "]");
      p += strspn(p, "] ");
    }

  /* Section field.  */
  info->sect = p;
  p += strcspn(p, " ");
  p += strspn(p, " ");

  /* Name field.  */
  len = strcspn(p, "@\n");
  if (len > 0 && p[len] == '@')
    {
      /* Get the symbol version.  */
      char* vp = p + len;
      int vlen;

      vp += strspn(vp, "@");
      vlen = strcspn(vp, "\n");
      vp[vlen] = '\0';
      if (vlen > 0)
	info->ver = vp;
      else
	info->ver = NULL;
    }
  else
    info->ver = NULL;
  p[len] = '\0';
  if (len > 0)
    info->name = p;
  else
    info->name = NULL;
}

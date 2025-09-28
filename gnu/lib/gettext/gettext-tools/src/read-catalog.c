/* Reading PO files.
   Copyright (C) 1995-1998, 2000-2003, 2005-2006, 2008-2009, 2014-2016, 2023 Free Software Foundation, Inc.
   This file was written by Peter Miller <millerp@canb.auug.org.au>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include "read-catalog.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "open-catalog.h"
#include "po-charset.h"
#include "po-xerror.h"
#include "xalloc.h"
#include "gettext.h"

#define _(str) gettext (str)


/* ========================================================================= */
/* Inline functions to invoke the methods.  */

static inline void
call_set_domain (struct default_catalog_reader_ty *this, char *name)
{
  default_catalog_reader_class_ty *methods =
    (default_catalog_reader_class_ty *) this->methods;

  if (methods->set_domain)
    methods->set_domain (this, name);
}

static inline void
call_add_message (struct default_catalog_reader_ty *this,
                  char *msgctxt,
                  char *msgid, lex_pos_ty *msgid_pos, char *msgid_plural,
                  char *msgstr, size_t msgstr_len, lex_pos_ty *msgstr_pos,
                  char *prev_msgctxt, char *prev_msgid, char *prev_msgid_plural,
                  bool force_fuzzy, bool obsolete)
{
  default_catalog_reader_class_ty *methods =
    (default_catalog_reader_class_ty *) this->methods;

  if (methods->add_message)
    methods->add_message (this, msgctxt,
                          msgid, msgid_pos, msgid_plural,
                          msgstr, msgstr_len, msgstr_pos,
                          prev_msgctxt, prev_msgid, prev_msgid_plural,
                          force_fuzzy, obsolete);
}

static inline void
call_frob_new_message (struct default_catalog_reader_ty *this, message_ty *mp,
                       const lex_pos_ty *msgid_pos,
                       const lex_pos_ty *msgstr_pos)
{
  default_catalog_reader_class_ty *methods =
    (default_catalog_reader_class_ty *) this->methods;

  if (methods->frob_new_message)
    methods->frob_new_message (this, mp, msgid_pos, msgstr_pos);
}


/* ========================================================================= */
/* Implementation of default_catalog_reader_ty's methods.  */


/* Implementation of methods declared in the superclass.  */


/* Prepare for first message.  */
void
default_constructor (abstract_catalog_reader_ty *that)
{
  default_catalog_reader_ty *this = (default_catalog_reader_ty *) that;
  size_t i;

  this->domain = MESSAGE_DOMAIN_DEFAULT;
  this->comment = NULL;
  this->comment_dot = NULL;
  this->filepos_count = 0;
  this->filepos = NULL;
  this->is_fuzzy = false;
  for (i = 0; i < NFORMATS; i++)
    this->is_format[i] = undecided;
  this->range.min = -1;
  this->range.max = -1;
  this->do_wrap = undecided;
  for (i = 0; i < NSYNTAXCHECKS; i++)
    this->do_syntax_check[i] = undecided;
}


void
default_destructor (abstract_catalog_reader_ty *that)
{
  default_catalog_reader_ty *this = (default_catalog_reader_ty *) that;
  size_t j;

  /* Do not free this->mdlp and this->mlp.  */
  if (this->handle_comments)
    {
      if (this->comment != NULL)
        string_list_free (this->comment);
      if (this->comment_dot != NULL)
        string_list_free (this->comment_dot);
    }

  for (j = 0; j < this->filepos_count; ++j)
    free ((char *) this->filepos[j].file_name);
  if (this->filepos != NULL)
    free (this->filepos);
}


void
default_parse_brief (abstract_catalog_reader_ty *that)
{
  /* We need to parse comments, because even if this->handle_comments
     is false, we need to know which messages are fuzzy.  */
  po_lex_pass_comments (true);
}


void
default_parse_debrief (abstract_catalog_reader_ty *that)
{
}


/* Add the accumulated comments to the message.  */
static void
default_copy_comment_state (default_catalog_reader_ty *this, message_ty *mp)
{
  size_t j, i;

  if (this->handle_comments)
    {
      if (this->comment != NULL)
        for (j = 0; j < this->comment->nitems; ++j)
          message_comment_append (mp, this->comment->item[j]);
      if (this->comment_dot != NULL)
        for (j = 0; j < this->comment_dot->nitems; ++j)
          message_comment_dot_append (mp, this->comment_dot->item[j]);
    }
  for (j = 0; j < this->filepos_count; ++j)
    {
      lex_pos_ty *pp;

      pp = &this->filepos[j];
      message_comment_filepos (mp, pp->file_name, pp->line_number);
    }
  mp->is_fuzzy = this->is_fuzzy;
  for (i = 0; i < NFORMATS; i++)
    mp->is_format[i] = this->is_format[i];
  mp->range = this->range;
  mp->do_wrap = this->do_wrap;
  for (i = 0; i < NSYNTAXCHECKS; i++)
    mp->do_syntax_check[i] = this->do_syntax_check[i];
}


static void
default_reset_comment_state (default_catalog_reader_ty *this)
{
  size_t j, i;

  if (this->handle_comments)
    {
      if (this->comment != NULL)
        {
          string_list_free (this->comment);
          this->comment = NULL;
        }
      if (this->comment_dot != NULL)
        {
          string_list_free (this->comment_dot);
          this->comment_dot = NULL;
        }
    }
  for (j = 0; j < this->filepos_count; ++j)
    free ((char *) this->filepos[j].file_name);
  if (this->filepos != NULL)
    free (this->filepos);
  this->filepos_count = 0;
  this->filepos = NULL;
  this->is_fuzzy = false;
  for (i = 0; i < NFORMATS; i++)
    this->is_format[i] = undecided;
  this->range.min = -1;
  this->range.max = -1;
  this->do_wrap = undecided;
  for (i = 0; i < NSYNTAXCHECKS; i++)
    this->do_syntax_check[i] = undecided;
}


/* Process 'domain' directive from .po file.  */
void
default_directive_domain (abstract_catalog_reader_ty *that, char *name)
{
  default_catalog_reader_ty *this = (default_catalog_reader_ty *) that;

  call_set_domain (this, name);

  /* If there are accumulated comments, throw them away, they are
     probably part of the file header, or about the domain directive,
     and will be unrelated to the next message.  */
  default_reset_comment_state (this);
}


/* Process ['msgctxt'/]'msgid'/'msgstr' pair from .po file.  */
void
default_directive_message (abstract_catalog_reader_ty *that,
                           char *msgctxt,
                           char *msgid,
                           lex_pos_ty *msgid_pos,
                           char *msgid_plural,
                           char *msgstr, size_t msgstr_len,
                           lex_pos_ty *msgstr_pos,
                           char *prev_msgctxt,
                           char *prev_msgid, char *prev_msgid_plural,
                           bool force_fuzzy, bool obsolete)
{
  default_catalog_reader_ty *this = (default_catalog_reader_ty *) that;

  call_add_message (this, msgctxt, msgid, msgid_pos, msgid_plural,
                    msgstr, msgstr_len, msgstr_pos,
                    prev_msgctxt, prev_msgid, prev_msgid_plural,
                    force_fuzzy, obsolete);

  /* Prepare for next message.  */
  default_reset_comment_state (this);
}


void
default_comment (abstract_catalog_reader_ty *that, const char *s)
{
  default_catalog_reader_ty *this = (default_catalog_reader_ty *) that;

  if (this->handle_comments)
    {
      if (this->comment == NULL)
        this->comment = string_list_alloc ();
      string_list_append (this->comment, s);
    }
}


void
default_comment_dot (abstract_catalog_reader_ty *that, const char *s)
{
  default_catalog_reader_ty *this = (default_catalog_reader_ty *) that;

  if (this->handle_comments)
    {
      if (this->comment_dot == NULL)
        this->comment_dot = string_list_alloc ();
      string_list_append (this->comment_dot, s);
    }
}


void
default_comment_filepos (abstract_catalog_reader_ty *that,
                         const char *name, size_t line)
{
  default_catalog_reader_ty *this = (default_catalog_reader_ty *) that;
  size_t nbytes;
  lex_pos_ty *pp;

  nbytes = (this->filepos_count + 1) * sizeof (this->filepos[0]);
  this->filepos = xrealloc (this->filepos, nbytes);
  pp = &this->filepos[this->filepos_count++];
  pp->file_name = xstrdup (name);
  pp->line_number = line;
}


/* Test for '#, fuzzy' comments and warn.  */
void
default_comment_special (abstract_catalog_reader_ty *that, const char *s)
{
  default_catalog_reader_ty *this = (default_catalog_reader_ty *) that;

  po_parse_comment_special (s, &this->is_fuzzy, this->is_format, &this->range,
                            &this->do_wrap, this->do_syntax_check);
}


/* Default implementation of methods not inherited from the superclass.  */


void
default_set_domain (default_catalog_reader_ty *this, char *name)
{
  if (this->allow_domain_directives)
    /* Override current domain name.  Don't free memory.  */
    this->domain = name;
  else
    {
      po_gram_error_at_line (&gram_pos,
                             _("this file may not contain domain directives"));

      /* NAME was allocated in po-gram-gen.y but is not used anywhere.  */
      free (name);
    }
}

void
default_add_message (default_catalog_reader_ty *this,
                     char *msgctxt,
                     char *msgid,
                     lex_pos_ty *msgid_pos,
                     char *msgid_plural,
                     char *msgstr, size_t msgstr_len,
                     lex_pos_ty *msgstr_pos,
                     char *prev_msgctxt,
                     char *prev_msgid,
                     char *prev_msgid_plural,
                     bool force_fuzzy, bool obsolete)
{
  message_ty *mp;

  if (this->mdlp != NULL)
    /* Select the appropriate sublist of this->mdlp.  */
    this->mlp = msgdomain_list_sublist (this->mdlp, this->domain, true);

  if (this->allow_duplicates && msgid[0] != '\0')
    /* Doesn't matter if this message ID has been seen before.  */
    mp = NULL;
  else
    /* See if this message ID has been seen before.  */
    mp = message_list_search (this->mlp, msgctxt, msgid);

  if (mp)
    {
      if (!(this->allow_duplicates_if_same_msgstr
            && msgstr_len == mp->msgstr_len
            && memcmp (msgstr, mp->msgstr, msgstr_len) == 0))
        {
          /* We give a fatal error about this, regardless whether the
             translations are equal or different.  This is for consistency
             with msgmerge, msgcat and others.  The user can use the
             msguniq program to get rid of duplicates.  */
          po_xerror2 (PO_SEVERITY_ERROR,
                      NULL, msgid_pos->file_name, msgid_pos->line_number,
                      (size_t)(-1), false, _("duplicate message definition"),
                      mp, NULL, 0, 0, false,
                      _("this is the location of the first definition"));
        }
      /* We don't need the just constructed entries' parameter string
         (allocated in po-gram-gen.y).  */
      free (msgid);
      if (msgid_plural != NULL)
        free (msgid_plural);
      free (msgstr);
      if (msgctxt != NULL)
        free (msgctxt);
      if (prev_msgctxt != NULL)
        free (prev_msgctxt);
      if (prev_msgid != NULL)
        free (prev_msgid);
      if (prev_msgid_plural != NULL)
        free (prev_msgid_plural);

      /* Add the accumulated comments to the message.  */
      default_copy_comment_state (this, mp);
    }
  else
    {
      /* Construct message to add to the list.
         Obsolete message go into the list at least for duplicate checking.
         It's the caller's responsibility to ignore obsolete messages when
         appropriate.  */
      mp = message_alloc (msgctxt, msgid, msgid_plural, msgstr, msgstr_len,
                          msgstr_pos);
      if (msgid_plural != NULL)
        free (msgid_plural);
      mp->prev_msgctxt = prev_msgctxt;
      mp->prev_msgid = prev_msgid;
      mp->prev_msgid_plural = prev_msgid_plural;
      mp->obsolete = obsolete;
      default_copy_comment_state (this, mp);
      if (force_fuzzy)
        mp->is_fuzzy = true;

      call_frob_new_message (this, mp, msgid_pos, msgstr_pos);

      message_list_append (this->mlp, mp);
    }
}


/* So that the one parser can be used for multiple programs, and also
   use good data hiding and encapsulation practices, an object
   oriented approach has been taken.  An object instance is allocated,
   and all actions resulting from the parse will be through
   invocations of method functions of that object.  */

static default_catalog_reader_class_ty default_methods =
{
  {
    sizeof (default_catalog_reader_ty),
    default_constructor,
    default_destructor,
    default_parse_brief,
    default_parse_debrief,
    default_directive_domain,
    default_directive_message,
    default_comment,
    default_comment_dot,
    default_comment_filepos,
    default_comment_special
  },
  default_set_domain, /* set_domain */
  default_add_message, /* add_message */
  NULL /* frob_new_message */
};


default_catalog_reader_ty *
default_catalog_reader_alloc (default_catalog_reader_class_ty *method_table)
{
  return
    (default_catalog_reader_ty *) catalog_reader_alloc (&method_table->super);
}


/* ========================================================================= */
/* Exported functions.  */


/* If false, duplicate msgids in the same domain and file generate an error.
   If true, such msgids are allowed; the caller should treat them
   appropriately.  Defaults to false.  */
bool allow_duplicates = false;


msgdomain_list_ty *
read_catalog_stream (FILE *fp, const char *real_filename,
                     const char *logical_filename,
                     catalog_input_format_ty input_syntax)
{
  default_catalog_reader_ty *pop;
  msgdomain_list_ty *mdlp;

  pop = default_catalog_reader_alloc (&default_methods);
  pop->handle_comments = true;
  pop->allow_domain_directives = true;
  pop->allow_duplicates = allow_duplicates;
  pop->allow_duplicates_if_same_msgstr = false;
  pop->file_name = real_filename;
  pop->mdlp = msgdomain_list_alloc (!pop->allow_duplicates);
  pop->mlp = msgdomain_list_sublist (pop->mdlp, pop->domain, true);
  if (input_syntax->produces_utf8)
    /* We know a priori that input_syntax->parse convert strings to UTF-8.  */
    pop->mdlp->encoding = po_charset_utf8;
  po_lex_pass_obsolete_entries (true);
  catalog_reader_parse ((abstract_catalog_reader_ty *) pop, fp, real_filename,
                        logical_filename, input_syntax);
  mdlp = pop->mdlp;
  catalog_reader_free ((abstract_catalog_reader_ty *) pop);
  return mdlp;
}


msgdomain_list_ty *
read_catalog_file (const char *filename, catalog_input_format_ty input_syntax)
{
  char *real_filename;
  FILE *fp = open_catalog_file (filename, &real_filename, true);
  msgdomain_list_ty *result;

  result = read_catalog_stream (fp, real_filename, filename, input_syntax);

  if (fp != stdin)
    fclose (fp);

  return result;
}

/* Resolving ambiguity of argument lists: Progressive parsing of an
   argument list, keeping track of all possibilities.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.

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
#include "xg-arglist-parser.h"

#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "error-progname.h"
#include "flexmember.h"
#include "xalloc.h"
#include "xsize.h"

#include "xgettext.h"
#include "xg-message.h"

#include "gettext.h"
#define _(str) gettext (str)


struct arglist_parser *
arglist_parser_alloc (message_list_ty *mlp, const struct callshapes *shapes)
{
  if (shapes == NULL || shapes->nshapes == 0)
    {
      struct arglist_parser *ap =
        (struct arglist_parser *)
        xmalloc (FLEXNSIZEOF (struct arglist_parser, alternative, 0));

      ap->mlp = mlp;
      ap->keyword = NULL;
      ap->keyword_len = 0;
      ap->next_is_msgctxt = false;
      ap->nalternatives = 0;

      return ap;
    }
  else
    {
      struct arglist_parser *ap =
        (struct arglist_parser *)
        xmalloc (FLEXNSIZEOF (struct arglist_parser, alternative,
                              shapes->nshapes));
      size_t i;

      ap->mlp = mlp;
      ap->keyword = shapes->keyword;
      ap->keyword_len = shapes->keyword_len;
      ap->next_is_msgctxt = false;
      ap->nalternatives = shapes->nshapes;
      for (i = 0; i < shapes->nshapes; i++)
        {
          ap->alternative[i].argnumc = shapes->shapes[i].argnumc;
          ap->alternative[i].argnum1 = shapes->shapes[i].argnum1;
          ap->alternative[i].argnum2 = shapes->shapes[i].argnum2;
          ap->alternative[i].argnum1_glib_context =
            shapes->shapes[i].argnum1_glib_context;
          ap->alternative[i].argnum2_glib_context =
            shapes->shapes[i].argnum2_glib_context;
          ap->alternative[i].argtotal = shapes->shapes[i].argtotal;
          ap->alternative[i].xcomments = shapes->shapes[i].xcomments;
          ap->alternative[i].msgctxt = NULL;
          ap->alternative[i].msgctxt_pos.file_name = NULL;
          ap->alternative[i].msgctxt_pos.line_number = (size_t)(-1);
          ap->alternative[i].msgid = NULL;
          ap->alternative[i].msgid_context = null_context;
          ap->alternative[i].msgid_pos.file_name = NULL;
          ap->alternative[i].msgid_pos.line_number = (size_t)(-1);
          ap->alternative[i].msgid_comment = NULL;
          ap->alternative[i].msgid_comment_is_utf8 = false;
          ap->alternative[i].msgid_plural = NULL;
          ap->alternative[i].msgid_plural_context = null_context;
          ap->alternative[i].msgid_plural_pos.file_name = NULL;
          ap->alternative[i].msgid_plural_pos.line_number = (size_t)(-1);
        }

      return ap;
    }
}


struct arglist_parser *
arglist_parser_clone (struct arglist_parser *ap)
{
  struct arglist_parser *copy =
    (struct arglist_parser *)
    xmalloc (FLEXNSIZEOF (struct arglist_parser, alternative,
                          ap->nalternatives));
  size_t i;

  copy->mlp = ap->mlp;
  copy->keyword = ap->keyword;
  copy->keyword_len = ap->keyword_len;
  copy->next_is_msgctxt = ap->next_is_msgctxt;
  copy->nalternatives = ap->nalternatives;
  for (i = 0; i < ap->nalternatives; i++)
    {
      const struct partial_call *cp = &ap->alternative[i];
      struct partial_call *ccp = &copy->alternative[i];

      ccp->argnumc = cp->argnumc;
      ccp->argnum1 = cp->argnum1;
      ccp->argnum2 = cp->argnum2;
      ccp->argnum1_glib_context = cp->argnum1_glib_context;
      ccp->argnum2_glib_context = cp->argnum2_glib_context;
      ccp->argtotal = cp->argtotal;
      ccp->xcomments = cp->xcomments;
      ccp->msgctxt =
        (cp->msgctxt != NULL ? mixed_string_clone (cp->msgctxt) : NULL);
      ccp->msgctxt_pos = cp->msgctxt_pos;
      ccp->msgid = (cp->msgid != NULL ? mixed_string_clone (cp->msgid) : NULL);
      ccp->msgid_context = cp->msgid_context;
      ccp->msgid_pos = cp->msgid_pos;
      ccp->msgid_comment = add_reference (cp->msgid_comment);
      ccp->msgid_comment_is_utf8 = cp->msgid_comment_is_utf8;
      ccp->msgid_plural =
        (cp->msgid_plural != NULL ? mixed_string_clone (cp->msgid_plural) : NULL);
      ccp->msgid_plural_context = cp->msgid_plural_context;
      ccp->msgid_plural_pos = cp->msgid_plural_pos;
    }

  return copy;
}


void
arglist_parser_remember (struct arglist_parser *ap,
                         int argnum, mixed_string_ty *string,
                         flag_context_ty context,
                         const char *file_name, size_t line_number,
                         refcounted_string_list_ty *comment,
                         bool comment_is_utf8)
{
  bool stored_string = false;
  size_t nalternatives = ap->nalternatives;
  size_t i;

  if (!(argnum > 0))
    abort ();
  for (i = 0; i < nalternatives; i++)
    {
      struct partial_call *cp = &ap->alternative[i];

      if (argnum == cp->argnumc)
        {
          cp->msgctxt = string;
          cp->msgctxt_pos.file_name = file_name;
          cp->msgctxt_pos.line_number = line_number;
          stored_string = true;
          /* Mark msgctxt as done.  */
          cp->argnumc = 0;
        }
      else
        {
          if (argnum == cp->argnum1)
            {
              cp->msgid = string;
              cp->msgid_context = context;
              cp->msgid_pos.file_name = file_name;
              cp->msgid_pos.line_number = line_number;
              cp->msgid_comment = add_reference (comment);
              cp->msgid_comment_is_utf8 = comment_is_utf8;
              stored_string = true;
              /* Mark msgid as done.  */
              cp->argnum1 = 0;
            }
          if (argnum == cp->argnum2)
            {
              cp->msgid_plural = string;
              cp->msgid_plural_context = context;
              cp->msgid_plural_pos.file_name = file_name;
              cp->msgid_plural_pos.line_number = line_number;
              stored_string = true;
              /* Mark msgid_plural as done.  */
              cp->argnum2 = 0;
            }
        }
    }
  /* Note: There is a memory leak here: When string was stored but is later
     not used by arglist_parser_done, we don't free it.  */
  if (!stored_string)
    mixed_string_free (string);
}


void
arglist_parser_remember_msgctxt (struct arglist_parser *ap,
                                 mixed_string_ty *string,
                                 flag_context_ty context,
                                 const char *file_name, size_t line_number)
{
  bool stored_string = false;
  size_t nalternatives = ap->nalternatives;
  size_t i;

  for (i = 0; i < nalternatives; i++)
    {
      struct partial_call *cp = &ap->alternative[i];

      cp->msgctxt = string;
      cp->msgctxt_pos.file_name = file_name;
      cp->msgctxt_pos.line_number = line_number;
      stored_string = true;
      /* Mark msgctxt as done.  */
      cp->argnumc = 0;
    }
  /* Note: There is a memory leak here: When string was stored but is later
     not used by arglist_parser_done, we don't free it.  */
  if (!stored_string)
    mixed_string_free (string);
}


bool
arglist_parser_decidedp (struct arglist_parser *ap, int argnum)
{
  size_t i;

  /* Test whether all alternatives are decided.
     Note: A decided alternative can be complete
       cp->argnumc == 0 && cp->argnum1 == 0 && cp->argnum2 == 0
       && cp->argtotal == 0
     or it can be failed if no literal strings were found at the specified
     argument positions:
       cp->argnumc <= argnum && cp->argnum1 <= argnum && cp->argnum2 <= argnum
     or it can be failed if the number of arguments is exceeded:
       cp->argtotal > 0 && cp->argtotal < argnum
   */
  for (i = 0; i < ap->nalternatives; i++)
    {
      struct partial_call *cp = &ap->alternative[i];

      if (!((cp->argnumc <= argnum
             && cp->argnum1 <= argnum
             && cp->argnum2 <= argnum)
            || (cp->argtotal > 0 && cp->argtotal < argnum)))
        /* cp is still undecided.  */
        return false;
    }
  return true;
}


void
arglist_parser_done (struct arglist_parser *ap, int argnum)
{
  size_t ncomplete;
  size_t i;

  /* Determine the number of complete calls.  */
  ncomplete = 0;
  for (i = 0; i < ap->nalternatives; i++)
    {
      struct partial_call *cp = &ap->alternative[i];

      if (cp->argnumc == 0 && cp->argnum1 == 0 && cp->argnum2 == 0
          && (cp->argtotal == 0 || cp->argtotal == argnum))
        ncomplete++;
    }

  if (ncomplete > 0)
    {
      struct partial_call *best_cp = NULL;
      bool ambiguous = false;

      /* Find complete calls where msgctxt, msgid, msgid_plural are all
         provided.  */
      for (i = 0; i < ap->nalternatives; i++)
        {
          struct partial_call *cp = &ap->alternative[i];

          if (cp->argnumc == 0 && cp->argnum1 == 0 && cp->argnum2 == 0
              && (cp->argtotal == 0 || cp->argtotal == argnum)
              && cp->msgctxt != NULL
              && cp->msgid != NULL
              && cp->msgid_plural != NULL)
            {
              if (best_cp != NULL)
                {
                  ambiguous = true;
                  break;
                }
              best_cp = cp;
            }
        }

      if (best_cp == NULL)
        {
          struct partial_call *best_cp1 = NULL;
          struct partial_call *best_cp2 = NULL;

          /* Find complete calls where msgctxt, msgid are provided.  */
          for (i = 0; i < ap->nalternatives; i++)
            {
              struct partial_call *cp = &ap->alternative[i];

              if (cp->argnumc == 0 && cp->argnum1 == 0 && cp->argnum2 == 0
                  && (cp->argtotal == 0 || cp->argtotal == argnum)
                  && cp->msgctxt != NULL
                  && cp->msgid != NULL)
                {
                  if (best_cp1 != NULL)
                    {
                      ambiguous = true;
                      break;
                    }
                  best_cp1 = cp;
                }
            }

          /* Find complete calls where msgid, msgid_plural are provided.  */
          for (i = 0; i < ap->nalternatives; i++)
            {
              struct partial_call *cp = &ap->alternative[i];

              if (cp->argnumc == 0 && cp->argnum1 == 0 && cp->argnum2 == 0
                  && (cp->argtotal == 0 || cp->argtotal == argnum)
                  && cp->msgid != NULL
                  && cp->msgid_plural != NULL)
                {
                  if (best_cp2 != NULL)
                    {
                      ambiguous = true;
                      break;
                    }
                  best_cp2 = cp;
                }
            }

          if (best_cp1 != NULL)
            best_cp = best_cp1;
          if (best_cp2 != NULL)
            {
              if (best_cp != NULL)
                ambiguous = true;
              else
                best_cp = best_cp2;
            }
        }

      if (best_cp == NULL)
        {
          /* Find complete calls where msgid is provided.  */
          for (i = 0; i < ap->nalternatives; i++)
            {
              struct partial_call *cp = &ap->alternative[i];

              if (cp->argnumc == 0 && cp->argnum1 == 0 && cp->argnum2 == 0
                  && (cp->argtotal == 0 || cp->argtotal == argnum)
                  && cp->msgid != NULL)
                {
                  if (best_cp != NULL)
                    {
                      ambiguous = true;
                      break;
                    }
                  best_cp = cp;
                }
            }
        }

      if (ambiguous)
        {
          error_with_progname = false;
          error_at_line (0, 0,
                         best_cp->msgid_pos.file_name,
                         best_cp->msgid_pos.line_number,
                         _("ambiguous argument specification for keyword '%.*s'"),
                         (int) ap->keyword_len, ap->keyword);
          error_with_progname = true;
        }

      if (best_cp != NULL)
        {
          /* best_cp indicates the best found complete call.
             Now call remember_a_message.  */
          flag_context_ty msgid_context;
          flag_context_ty msgid_plural_context;
          char *best_msgctxt;
          char *best_msgid;
          char *best_msgid_plural;
          message_ty *mp;

          msgid_context = best_cp->msgid_context;
          msgid_plural_context = best_cp->msgid_plural_context;

          /* Special support for the 3-argument tr operator in Qt:
             When --qt and --keyword=tr:1,1,2c,3t are specified, add to the
             context the information that the argument is expected to be a
             qt-plural-format.  */
          if (recognize_qt_formatstrings ()
              && best_cp->msgid_plural == best_cp->msgid)
            {
              msgid_context.is_format4 = yes_according_to_context;
              msgid_plural_context.is_format4 = yes_according_to_context;
            }

          best_msgctxt =
            (best_cp->msgctxt != NULL
             ? mixed_string_contents_free1 (best_cp->msgctxt)
             : NULL);
          best_msgid =
            (best_cp->msgid != NULL
             ? mixed_string_contents_free1 (best_cp->msgid)
             : NULL);
          best_msgid_plural =
            (best_cp->msgid_plural != NULL
             ? /* Special support for the 3-argument tr operator in Qt.  */
               (best_cp->msgid_plural == best_cp->msgid
                ? xstrdup (best_msgid)
                : mixed_string_contents_free1 (best_cp->msgid_plural))
             : NULL);

          /* Split strings in the GNOME glib syntax "msgctxt|msgid".  */
          if (best_cp->argnum1_glib_context || best_cp->argnum2_glib_context)
            /* split_keywordspec should not allow the context to be specified
               in two different ways.  */
            if (best_msgctxt != NULL)
              abort ();
          if (best_cp->argnum1_glib_context)
            {
              const char *separator = strchr (best_msgid, '|');

              if (separator == NULL)
                {
                  error_with_progname = false;
                  error_at_line (0, 0,
                                 best_cp->msgid_pos.file_name,
                                 best_cp->msgid_pos.line_number,
                                 _("warning: missing context for keyword '%.*s'"),
                                 (int) ap->keyword_len, ap->keyword);
                  error_with_progname = true;
                }
              else
                {
                  size_t ctxt_len = separator - best_msgid;
                  char *ctxt = XNMALLOC (ctxt_len + 1, char);

                  memcpy (ctxt, best_msgid, ctxt_len);
                  ctxt[ctxt_len] = '\0';
                  best_msgctxt = ctxt;
                  best_msgid = xstrdup (separator + 1);
                }
            }
          if (best_msgid_plural != NULL && best_cp->argnum2_glib_context)
            {
              const char *separator = strchr (best_msgid_plural, '|');

              if (separator == NULL)
                {
                  error_with_progname = false;
                  error_at_line (0, 0,
                                 best_cp->msgid_plural_pos.file_name,
                                 best_cp->msgid_plural_pos.line_number,
                                 _("warning: missing context for plural argument of keyword '%.*s'"),
                                 (int) ap->keyword_len, ap->keyword);
                  error_with_progname = true;
                }
              else
                {
                  size_t ctxt_len = separator - best_msgid_plural;
                  char *ctxt = XNMALLOC (ctxt_len + 1, char);

                  memcpy (ctxt, best_msgid_plural, ctxt_len);
                  ctxt[ctxt_len] = '\0';
                  if (best_msgctxt == NULL)
                    best_msgctxt = ctxt;
                  else
                    {
                      if (strcmp (ctxt, best_msgctxt) != 0)
                        {
                          error_with_progname = false;
                          error_at_line (0, 0,
                                         best_cp->msgid_plural_pos.file_name,
                                         best_cp->msgid_plural_pos.line_number,
                                         _("context mismatch between singular and plural form"));
                          error_with_progname = true;
                        }
                      free (ctxt);
                    }
                  best_msgid_plural = xstrdup (separator + 1);
                }
            }

          mp = remember_a_message (ap->mlp, best_msgctxt, best_msgid, true,
                                   best_msgid_plural != NULL,
                                   msgid_context,
                                   &best_cp->msgid_pos,
                                   NULL, best_cp->msgid_comment,
                                   best_cp->msgid_comment_is_utf8);
          if (mp != NULL && best_msgid_plural != NULL)
            remember_a_message_plural (mp, best_msgid_plural, true,
                                       msgid_plural_context,
                                       &best_cp->msgid_plural_pos,
                                       NULL, false);

          if (best_cp->xcomments.nitems > 0)
            {
              /* Add best_cp->xcomments to mp->comment_dot, unless already
                 present.  */
              size_t j;

              for (j = 0; j < best_cp->xcomments.nitems; j++)
                {
                  const char *xcomment = best_cp->xcomments.item[j];
                  bool found = false;

                  if (mp != NULL && mp->comment_dot != NULL)
                    {
                      size_t k;

                      for (k = 0; k < mp->comment_dot->nitems; k++)
                        if (strcmp (xcomment, mp->comment_dot->item[k]) == 0)
                          {
                            found = true;
                            break;
                          }
                    }
                  if (!found)
                    message_comment_dot_append (mp, xcomment);
                }
            }
        }
    }
  else
    {
      /* No complete call was parsed.  */
      /* Note: There is a memory leak here: When there is more than one
         alternative, the same string can be stored in multiple alternatives,
         and it's not easy to free all strings reliably.  */
      if (ap->nalternatives == 1)
        {
          if (ap->alternative[0].msgctxt != NULL)
            free (ap->alternative[0].msgctxt);
          if (ap->alternative[0].msgid != NULL)
            free (ap->alternative[0].msgid);
          if (ap->alternative[0].msgid_plural != NULL)
            free (ap->alternative[0].msgid_plural);
        }
    }

  for (i = 0; i < ap->nalternatives; i++)
    drop_reference (ap->alternative[i].msgid_comment);
  free (ap);
}

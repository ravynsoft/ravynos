/* GNU gettext - internationalization aids
   Copyright (C) 1995-1998, 2000-2010, 2012-2013, 2015-2016, 2019-2020, 2023 Free Software Foundation, Inc.

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
#include "message.h"

#include <stdlib.h>
#include <string.h>

#include "fstrcmp.h"
#include "mem-hash-map.h"
#include "xalloc.h"
#include "xmalloca.h"


const char *const format_language[NFORMATS] =
{
  /* format_c */                "c",
  /* format_objc */             "objc",
  /* format_cplusplus_brace */  "c++",
  /* format_python */           "python",
  /* format_python_brace */     "python-brace",
  /* format_java */             "java",
  /* format_java_printf */      "java-printf",
  /* format_csharp */           "csharp",
  /* format_javascript */       "javascript",
  /* format_scheme */           "scheme",
  /* format_lisp */             "lisp",
  /* format_elisp */            "elisp",
  /* format_librep */           "librep",
  /* format_ruby */             "ruby",
  /* format_sh */               "sh",
  /* format_awk */              "awk",
  /* format_lua */              "lua",
  /* format_pascal */           "object-pascal",
  /* format_smalltalk */        "smalltalk",
  /* format_qt */               "qt",
  /* format_qt_plursl */        "qt-plural",
  /* format_kde */              "kde",
  /* format_kde_kuit */         "kde-kuit",
  /* format_boost */            "boost",
  /* format_tcl */              "tcl",
  /* format_perl */             "perl",
  /* format_perl_brace */       "perl-brace",
  /* format_php */              "php",
  /* format_gcc_internal */     "gcc-internal",
  /* format_gfc_internal */     "gfc-internal",
  /* format_ycp */              "ycp"
};

const char *const format_language_pretty[NFORMATS] =
{
  /* format_c */                "C",
  /* format_objc */             "Objective C",
  /* format_cplusplus_brace */  "C++",
  /* format_python */           "Python",
  /* format_python_brace */     "Python brace",
  /* format_java */             "Java MessageFormat",
  /* format_java_printf */      "Java printf",
  /* format_csharp */           "C#",
  /* format_javascript */       "JavaScript",
  /* format_scheme */           "Scheme",
  /* format_lisp */             "Lisp",
  /* format_elisp */            "Emacs Lisp",
  /* format_librep */           "librep",
  /* format_ruby */             "Ruby",
  /* format_sh */               "Shell",
  /* format_awk */              "awk",
  /* format_lua */              "Lua",
  /* format_pascal */           "Object Pascal",
  /* format_smalltalk */        "Smalltalk",
  /* format_qt */               "Qt",
  /* format_qt_plural */        "Qt plural",
  /* format_kde */              "KDE",
  /* format_kde_kuit */         "KDE KUIT",
  /* format_boost */            "Boost",
  /* format_tcl */              "Tcl",
  /* format_perl */             "Perl",
  /* format_perl_brace */       "Perl brace",
  /* format_php */              "PHP",
  /* format_gcc_internal */     "GCC internal",
  /* format_gfc_internal */     "GFC internal",
  /* format_ycp */              "YCP"
};


bool
possible_format_p (enum is_format is_format)
{
  return is_format == possible
         || is_format == yes_according_to_context
         || is_format == yes;
}


const char *const syntax_check_name[NSYNTAXCHECKS] =
{
  /* sc_ellipsis_unicode */     "ellipsis-unicode",
  /* sc_space_ellipsis */       "space-ellipsis",
  /* sc_quote_unicode */        "quote-unicode",
  /* sc_bullet_unicode */       "bullet-unicode"
};


message_ty *
message_alloc (const char *msgctxt,
               const char *msgid, const char *msgid_plural,
               const char *msgstr, size_t msgstr_len,
               const lex_pos_ty *pp)
{
  message_ty *mp;
  size_t i;

  mp = XMALLOC (message_ty);
  mp->msgctxt = msgctxt;
  mp->msgid = msgid;
  mp->msgid_plural = (msgid_plural != NULL ? xstrdup (msgid_plural) : NULL);
  mp->msgstr = msgstr;
  mp->msgstr_len = msgstr_len;
  mp->pos = *pp;
  mp->comment = NULL;
  mp->comment_dot = NULL;
  mp->filepos_count = 0;
  mp->filepos = NULL;
  mp->is_fuzzy = false;
  for (i = 0; i < NFORMATS; i++)
    mp->is_format[i] = undecided;
  mp->range.min = -1;
  mp->range.max = -1;
  mp->do_wrap = undecided;
  for (i = 0; i < NSYNTAXCHECKS; i++)
    mp->do_syntax_check[i] = undecided;
  mp->prev_msgctxt = NULL;
  mp->prev_msgid = NULL;
  mp->prev_msgid_plural = NULL;
  mp->used = 0;
  mp->obsolete = false;
  return mp;
}


void
message_free (message_ty *mp)
{
  size_t j;

  free ((char *) mp->msgid);
  if (mp->msgid_plural != NULL)
    free ((char *) mp->msgid_plural);
  free ((char *) mp->msgstr);
  if (mp->comment != NULL)
    string_list_free (mp->comment);
  if (mp->comment_dot != NULL)
    string_list_free (mp->comment_dot);
  for (j = 0; j < mp->filepos_count; ++j)
    free ((char *) mp->filepos[j].file_name);
  if (mp->filepos != NULL)
    free (mp->filepos);
  if (mp->prev_msgctxt != NULL)
    free ((char *) mp->prev_msgctxt);
  if (mp->prev_msgid != NULL)
    free ((char *) mp->prev_msgid);
  if (mp->prev_msgid_plural != NULL)
    free ((char *) mp->prev_msgid_plural);
  free (mp);
}


void
message_comment_append (message_ty *mp, const char *s)
{
  if (mp->comment == NULL)
    mp->comment = string_list_alloc ();
  string_list_append (mp->comment, s);
}


void
message_comment_dot_append (message_ty *mp, const char *s)
{
  if (mp->comment_dot == NULL)
    mp->comment_dot = string_list_alloc ();
  string_list_append (mp->comment_dot, s);
}


void
message_comment_filepos (message_ty *mp, const char *name, size_t line)
{
  size_t j;
  size_t nbytes;
  lex_pos_ty *pp;

  /* See if we have this position already.  */
  for (j = 0; j < mp->filepos_count; j++)
    {
      pp = &mp->filepos[j];
      if (strcmp (pp->file_name, name) == 0 && pp->line_number == line)
        return;
    }

  /* Extend the list so that we can add a position to it.  */
  nbytes = (mp->filepos_count + 1) * sizeof (mp->filepos[0]);
  mp->filepos = xrealloc (mp->filepos, nbytes);

  /* Insert the position at the end.  Don't sort the file positions here.  */
  pp = &mp->filepos[mp->filepos_count++];
  pp->file_name = xstrdup (name);
  pp->line_number = line;
}


message_ty *
message_copy (message_ty *mp)
{
  message_ty *result;
  size_t j, i;

  result = message_alloc (mp->msgctxt != NULL ? xstrdup (mp->msgctxt) : NULL,
                          xstrdup (mp->msgid), mp->msgid_plural,
                          mp->msgstr, mp->msgstr_len, &mp->pos);

  if (mp->comment)
    {
      for (j = 0; j < mp->comment->nitems; ++j)
        message_comment_append (result, mp->comment->item[j]);
    }
  if (mp->comment_dot)
    {
      for (j = 0; j < mp->comment_dot->nitems; ++j)
        message_comment_dot_append (result, mp->comment_dot->item[j]);
    }
  result->is_fuzzy = mp->is_fuzzy;
  for (i = 0; i < NFORMATS; i++)
    result->is_format[i] = mp->is_format[i];
  result->range = mp->range;
  result->do_wrap = mp->do_wrap;
  for (i = 0; i < NSYNTAXCHECKS; i++)
    result->do_syntax_check[i] = mp->do_syntax_check[i];
  for (j = 0; j < mp->filepos_count; ++j)
    {
      lex_pos_ty *pp = &mp->filepos[j];
      message_comment_filepos (result, pp->file_name, pp->line_number);
    }
  result->prev_msgctxt =
    (mp->prev_msgctxt != NULL ? xstrdup (mp->prev_msgctxt) : NULL);
  result->prev_msgid =
    (mp->prev_msgid != NULL ? xstrdup (mp->prev_msgid) : NULL);
  result->prev_msgid_plural =
    (mp->prev_msgid_plural != NULL ? xstrdup (mp->prev_msgid_plural) : NULL);
  return result;
}


message_list_ty *
message_list_alloc (bool use_hashtable)
{
  message_list_ty *mlp;

  mlp = XMALLOC (message_list_ty);
  mlp->nitems = 0;
  mlp->nitems_max = 0;
  mlp->item = NULL;
  if ((mlp->use_hashtable = use_hashtable))
    hash_init (&mlp->htable, 10);
  return mlp;
}


void
message_list_free (message_list_ty *mlp, int keep_messages)
{
  size_t j;

  if (keep_messages == 0)
    for (j = 0; j < mlp->nitems; ++j)
      message_free (mlp->item[j]);
  if (mlp->item)
    free (mlp->item);
  if (mlp->use_hashtable)
    hash_destroy (&mlp->htable);
  free (mlp);
}


static int
message_list_hash_insert_entry (hash_table *htable, message_ty *mp)
{
  char *alloced_key;
  const char *key;
  size_t keylen;
  int found;

  if (mp->msgctxt != NULL)
    {
      /* Concatenate mp->msgctxt and mp->msgid, to form the hash table key.  */
      size_t msgctxt_len = strlen (mp->msgctxt);
      size_t msgid_len = strlen (mp->msgid);
      keylen = msgctxt_len + 1 + msgid_len + 1;
      alloced_key = (char *) xmalloca (keylen);
      memcpy (alloced_key, mp->msgctxt, msgctxt_len);
      alloced_key[msgctxt_len] = MSGCTXT_SEPARATOR;
      memcpy (alloced_key + msgctxt_len + 1, mp->msgid, msgid_len + 1);
      key = alloced_key;
    }
  else
    {
      alloced_key = NULL;
      key = mp->msgid;
      keylen = strlen (mp->msgid) + 1;
    }

  found = (hash_insert_entry (htable, key, keylen, mp) == NULL);

  if (mp->msgctxt != NULL)
    freea (alloced_key);

  return found;
}


void
message_list_append (message_list_ty *mlp, message_ty *mp)
{
  if (mlp->nitems >= mlp->nitems_max)
    {
      size_t nbytes;

      mlp->nitems_max = mlp->nitems_max * 2 + 4;
      nbytes = mlp->nitems_max * sizeof (message_ty *);
      mlp->item = xrealloc (mlp->item, nbytes);
    }
  mlp->item[mlp->nitems++] = mp;

  if (mlp->use_hashtable)
    if (message_list_hash_insert_entry (&mlp->htable, mp))
      /* A message list has duplicates, although it was allocated with the
         assertion that it wouldn't have duplicates.  It is a bug.  */
      abort ();
}


void
message_list_prepend (message_list_ty *mlp, message_ty *mp)
{
  size_t j;

  if (mlp->nitems >= mlp->nitems_max)
    {
      size_t nbytes;

      mlp->nitems_max = mlp->nitems_max * 2 + 4;
      nbytes = mlp->nitems_max * sizeof (message_ty *);
      mlp->item = xrealloc (mlp->item, nbytes);
    }
  for (j = mlp->nitems; j > 0; j--)
    mlp->item[j] = mlp->item[j - 1];
  mlp->item[0] = mp;
  mlp->nitems++;

  if (mlp->use_hashtable)
    if (message_list_hash_insert_entry (&mlp->htable, mp))
      /* A message list has duplicates, although it was allocated with the
         assertion that it wouldn't have duplicates.  It is a bug.  */
      abort ();
}


void
message_list_insert_at (message_list_ty *mlp, size_t n, message_ty *mp)
{
  size_t j;

  if (mlp->nitems >= mlp->nitems_max)
    {
      size_t nbytes;

      mlp->nitems_max = mlp->nitems_max * 2 + 4;
      nbytes = mlp->nitems_max * sizeof (message_ty *);
      mlp->item = xrealloc (mlp->item, nbytes);
    }
  for (j = mlp->nitems; j > n; j--)
    mlp->item[j] = mlp->item[j - 1];
  mlp->item[j] = mp;
  mlp->nitems++;

  if (mlp->use_hashtable)
    if (message_list_hash_insert_entry (&mlp->htable, mp))
      /* A message list has duplicates, although it was allocated with the
         assertion that it wouldn't have duplicates.  It is a bug.  */
      abort ();
}


#if 0 /* unused */
void
message_list_delete_nth (message_list_ty *mlp, size_t n)
{
  size_t j;

  if (n >= mlp->nitems)
    return;
  message_free (mlp->item[n]);
  for (j = n + 1; j < mlp->nitems; ++j)
    mlp->item[j - 1] = mlp->item[j];
  mlp->nitems--;

  if (mlp->use_hashtable)
    {
      /* Our simple-minded hash tables don't support removal.  */
      hash_destroy (&mlp->htable);
      mlp->use_hashtable = false;
    }
}
#endif


void
message_list_remove_if_not (message_list_ty *mlp,
                            message_predicate_ty *predicate)
{
  size_t i, j;

  for (j = 0, i = 0; j < mlp->nitems; j++)
    if (predicate (mlp->item[j]))
      mlp->item[i++] = mlp->item[j];
  if (mlp->use_hashtable && i < mlp->nitems)
    {
      /* Our simple-minded hash tables don't support removal.  */
      hash_destroy (&mlp->htable);
      mlp->use_hashtable = false;
    }
  mlp->nitems = i;
}


bool
message_list_msgids_changed (message_list_ty *mlp)
{
  if (mlp->use_hashtable)
    {
      unsigned long int size = mlp->htable.size;
      size_t j;

      hash_destroy (&mlp->htable);
      hash_init (&mlp->htable, size);

      for (j = 0; j < mlp->nitems; j++)
        {
          message_ty *mp = mlp->item[j];

          if (message_list_hash_insert_entry (&mlp->htable, mp))
            /* A message list has duplicates, although it was allocated with
               the assertion that it wouldn't have duplicates, and before the
               msgids changed it indeed didn't have duplicates.  */
            {
              hash_destroy (&mlp->htable);
              mlp->use_hashtable = false;
              return true;
            }
        }
    }
  return false;
}


message_list_ty *
message_list_copy (message_list_ty *mlp, int copy_level)
{
  message_list_ty *result;
  size_t j;

  result = message_list_alloc (mlp->use_hashtable);
  for (j = 0; j < mlp->nitems; j++)
    {
      message_ty *mp = mlp->item[j];

      message_list_append (result, copy_level ? mp : message_copy (mp));
    }

  return result;
}


message_ty *
message_list_search (message_list_ty *mlp,
                     const char *msgctxt, const char *msgid)
{
  if (mlp->use_hashtable)
    {
      char *alloced_key;
      const char *key;
      size_t keylen;

      if (msgctxt != NULL)
        {
          /* Concatenate the msgctxt and msgid, to form the hash table key.  */
          size_t msgctxt_len = strlen (msgctxt);
          size_t msgid_len = strlen (msgid);
          keylen = msgctxt_len + 1 + msgid_len + 1;
          alloced_key = (char *) xmalloca (keylen);
          memcpy (alloced_key, msgctxt, msgctxt_len);
          alloced_key[msgctxt_len] = MSGCTXT_SEPARATOR;
          memcpy (alloced_key + msgctxt_len + 1, msgid, msgid_len + 1);
          key = alloced_key;
        }
      else
        {
          alloced_key = NULL;
          key = msgid;
          keylen = strlen (msgid) + 1;
        }

      {
        void *htable_value;
        int found = !hash_find_entry (&mlp->htable, key, keylen, &htable_value);

        if (msgctxt != NULL)
          freea (alloced_key);

        if (found)
          return (message_ty *) htable_value;
        else
          return NULL;
      }
    }
  else
    {
      size_t j;

      for (j = 0; j < mlp->nitems; ++j)
        {
          message_ty *mp;

          mp = mlp->item[j];
          if ((msgctxt != NULL
               ? mp->msgctxt != NULL && strcmp (msgctxt, mp->msgctxt) == 0
               : mp->msgctxt == NULL)
              && strcmp (msgid, mp->msgid) == 0)
            return mp;
        }
      return NULL;
    }
}


double
fuzzy_search_goal_function (const message_ty *mp,
                            const char *msgctxt, const char *msgid,
                            double lower_bound)
{
  double bonus = 0.0;
  /* A translation for a context is a good proposal also for another.  But
     give mp a small advantage if mp is valid regardless of any context or
     has the same context as the one being looked up.  */
  if (mp->msgctxt == NULL
      || (msgctxt != NULL && strcmp (msgctxt, mp->msgctxt) == 0))
    {
      bonus = 0.00001;
      /* Since we will consider (weight + bonus) at the end, we are only
         interested in weights that are >= lower_bound - bonus.  Subtract
         a little more than the bonus, in order to avoid trouble due to
         rounding errors.  */
      lower_bound -= bonus * 1.01;
    }

  {
    /* The use of 'volatile' guarantees that excess precision bits are dropped
       before the addition and before the following comparison at the caller's
       site.  It is necessary on x86 systems where double-floats are not IEEE
       compliant by default, to avoid that msgmerge results become platform and
       compiler option dependent.  'volatile' is a portable alternative to
       gcc's -ffloat-store option.  */
    volatile double weight = fstrcmp_bounded (msgid, mp->msgid, lower_bound);

    weight += bonus;

    return weight;
  }
}


static message_ty *
message_list_search_fuzzy_inner (message_list_ty *mlp,
                                 const char *msgctxt, const char *msgid,
                                 double *best_weight_p)
{
  size_t j;
  message_ty *best_mp;

  best_mp = NULL;
  for (j = 0; j < mlp->nitems; ++j)
    {
      message_ty *mp;

      mp = mlp->item[j];

      if (mp->msgstr != NULL && mp->msgstr[0] != '\0')
        {
          double weight =
            fuzzy_search_goal_function (mp, msgctxt, msgid, *best_weight_p);
          if (weight > *best_weight_p)
            {
              *best_weight_p = weight;
              best_mp = mp;
            }
        }
    }
  return best_mp;
}


message_ty *
message_list_search_fuzzy (message_list_ty *mlp,
                           const char *msgctxt, const char *msgid)
{
  double best_weight;

  best_weight = FUZZY_THRESHOLD;
  return message_list_search_fuzzy_inner (mlp, msgctxt, msgid, &best_weight);
}


message_list_list_ty *
message_list_list_alloc ()
{
  message_list_list_ty *mllp;

  mllp = XMALLOC (message_list_list_ty);
  mllp->nitems = 0;
  mllp->nitems_max = 0;
  mllp->item = NULL;
  return mllp;
}


void
message_list_list_free (message_list_list_ty *mllp, int keep_level)
{
  size_t j;

  if (keep_level < 2)
    for (j = 0; j < mllp->nitems; ++j)
      message_list_free (mllp->item[j], keep_level);
  if (mllp->item)
    free (mllp->item);
  free (mllp);
}


void
message_list_list_append (message_list_list_ty *mllp, message_list_ty *mlp)
{
  if (mllp->nitems >= mllp->nitems_max)
    {
      size_t nbytes;

      mllp->nitems_max = mllp->nitems_max * 2 + 4;
      nbytes = mllp->nitems_max * sizeof (message_list_ty *);
      mllp->item = xrealloc (mllp->item, nbytes);
    }
  mllp->item[mllp->nitems++] = mlp;
}


void
message_list_list_append_list (message_list_list_ty *mllp,
                               message_list_list_ty *mllp2)
{
  size_t j;

  for (j = 0; j < mllp2->nitems; ++j)
    message_list_list_append (mllp, mllp2->item[j]);
}


message_ty *
message_list_list_search (message_list_list_ty *mllp,
                          const char *msgctxt, const char *msgid)
{
  message_ty *best_mp;
  int best_weight; /* 0: not found, 1: found without msgstr, 2: translated */
  size_t j;

  best_mp = NULL;
  best_weight = 0;
  for (j = 0; j < mllp->nitems; ++j)
    {
      message_list_ty *mlp;
      message_ty *mp;

      mlp = mllp->item[j];
      mp = message_list_search (mlp, msgctxt, msgid);
      if (mp)
        {
          int weight = (mp->msgstr_len == 1 && mp->msgstr[0] == '\0' ? 1 : 2);
          if (weight > best_weight)
            {
              best_mp = mp;
              best_weight = weight;
            }
        }
    }
  return best_mp;
}


#if 0 /* unused */
message_ty *
message_list_list_search_fuzzy (message_list_list_ty *mllp,
                                const char *msgctxt, const char *msgid)
{
  size_t j;
  double best_weight;
  message_ty *best_mp;

  best_weight = FUZZY_THRESHOLD;
  best_mp = NULL;
  for (j = 0; j < mllp->nitems; ++j)
    {
      message_list_ty *mlp;
      message_ty *mp;

      mlp = mllp->item[j];
      mp = message_list_search_fuzzy_inner (mlp, msgctxt, msgid, &best_weight);
      if (mp)
        best_mp = mp;
    }
  return best_mp;
}
#endif


msgdomain_ty*
msgdomain_alloc (const char *domain, bool use_hashtable)
{
  msgdomain_ty *mdp;

  mdp = XMALLOC (msgdomain_ty);
  mdp->domain = domain;
  mdp->messages = message_list_alloc (use_hashtable);
  return mdp;
}


void
msgdomain_free (msgdomain_ty *mdp)
{
  message_list_free (mdp->messages, 0);
  free (mdp);
}


msgdomain_list_ty *
msgdomain_list_alloc (bool use_hashtable)
{
  msgdomain_list_ty *mdlp;

  mdlp = XMALLOC (msgdomain_list_ty);
  /* Put the default domain first, so that when we output it,
     we can omit the 'domain' directive.  */
  mdlp->nitems = 1;
  mdlp->nitems_max = 1;
  mdlp->item = XNMALLOC (mdlp->nitems_max, msgdomain_ty *);
  mdlp->item[0] = msgdomain_alloc (MESSAGE_DOMAIN_DEFAULT, use_hashtable);
  mdlp->use_hashtable = use_hashtable;
  mdlp->encoding = NULL;
  return mdlp;
}


void
msgdomain_list_free (msgdomain_list_ty *mdlp)
{
  size_t j;

  for (j = 0; j < mdlp->nitems; ++j)
    msgdomain_free (mdlp->item[j]);
  if (mdlp->item)
    free (mdlp->item);
  free (mdlp);
}


void
msgdomain_list_append (msgdomain_list_ty *mdlp, msgdomain_ty *mdp)
{
  if (mdlp->nitems >= mdlp->nitems_max)
    {
      size_t nbytes;

      mdlp->nitems_max = mdlp->nitems_max * 2 + 4;
      nbytes = mdlp->nitems_max * sizeof (msgdomain_ty *);
      mdlp->item = xrealloc (mdlp->item, nbytes);
    }
  mdlp->item[mdlp->nitems++] = mdp;
}


#if 0 /* unused */
void
msgdomain_list_append_list (msgdomain_list_ty *mdlp, msgdomain_list_ty *mdlp2)
{
  size_t j;

  for (j = 0; j < mdlp2->nitems; ++j)
    msgdomain_list_append (mdlp, mdlp2->item[j]);
}
#endif


message_list_ty *
msgdomain_list_sublist (msgdomain_list_ty *mdlp, const char *domain,
                        bool create)
{
  size_t j;

  for (j = 0; j < mdlp->nitems; j++)
    if (strcmp (mdlp->item[j]->domain, domain) == 0)
      return mdlp->item[j]->messages;

  if (create)
    {
      msgdomain_ty *mdp = msgdomain_alloc (domain, mdlp->use_hashtable);
      msgdomain_list_append (mdlp, mdp);
      return mdp->messages;
    }
  else
    return NULL;
}


/* Copy a message domain list.
   If copy_level = 0, also copy the messages.  If copy_level = 1, share the
   messages but copy the domains.  If copy_level = 2, share the domains.  */
msgdomain_list_ty *
msgdomain_list_copy (msgdomain_list_ty *mdlp, int copy_level)
{
  msgdomain_list_ty *result;
  size_t j;

  result = XMALLOC (msgdomain_list_ty);
  result->nitems = 0;
  result->nitems_max = 0;
  result->item = NULL;
  result->use_hashtable = mdlp->use_hashtable;
  result->encoding = mdlp->encoding;

  for (j = 0; j < mdlp->nitems; j++)
    {
      msgdomain_ty *mdp = mdlp->item[j];

      if (copy_level < 2)
        {
          msgdomain_ty *result_mdp = XMALLOC (msgdomain_ty);

          result_mdp->domain = mdp->domain;
          result_mdp->messages = message_list_copy (mdp->messages, copy_level);

          msgdomain_list_append (result, result_mdp);
        }
      else
        msgdomain_list_append (result, mdp);
    }

  return result;
}


#if 0 /* unused */
message_ty *
msgdomain_list_search (msgdomain_list_ty *mdlp,
                       const char *msgctxt, const char *msgid)
{
  size_t j;

  for (j = 0; j < mdlp->nitems; ++j)
    {
      msgdomain_ty *mdp;
      message_ty *mp;

      mdp = mdlp->item[j];
      mp = message_list_search (mdp->messages, msgctxt, msgid);
      if (mp)
        return mp;
    }
  return NULL;
}
#endif


#if 0 /* unused */
message_ty *
msgdomain_list_search_fuzzy (msgdomain_list_ty *mdlp,
                             const char *msgctxt, const char *msgid)
{
  size_t j;
  double best_weight;
  message_ty *best_mp;

  best_weight = FUZZY_THRESHOLD;
  best_mp = NULL;
  for (j = 0; j < mdlp->nitems; ++j)
    {
      msgdomain_ty *mdp;
      message_ty *mp;

      mdp = mdlp->item[j];
      mp = message_list_search_fuzzy_inner (mdp->messages, msgctxt, msgid,
                                            &best_weight);
      if (mp)
        best_mp = mp;
    }
  return best_mp;
}
#endif

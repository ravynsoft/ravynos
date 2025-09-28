/* Message list charset and locale charset handling.
   Copyright (C) 2001-2003, 2005-2009, 2019-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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
# include "config.h"
#endif
#include <alloca.h>

/* Specification.  */
#include "msgl-iconv.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_ICONV
# include <iconv.h>
#endif

#include "noreturn.h"
#include "progname.h"
#include "basename-lgpl.h"
#include "string-desc.h"
#include "message.h"
#include "po-charset.h"
#include "xstriconv.h"
#include "xstriconveh.h"
#include "msgl-ascii.h"
#include "msgl-ofn.h"
#include "xalloc.h"
#include "xmalloca.h"
#include "c-strstr.h"
#include "xvasprintf.h"
#include "po-xerror.h"
#include "gettext.h"

#define _(str) gettext (str)


#if HAVE_ICONV

_GL_NORETURN_FUNC static void conversion_error (const struct conversion_context* context);
static void
conversion_error (const struct conversion_context* context)
{
  if (context->to_code == po_charset_utf8)
    /* If a conversion to UTF-8 fails, the problem lies in the input.  */
    po_xerror (PO_SEVERITY_FATAL_ERROR, context->message, NULL, 0, 0, false,
               xasprintf (_("%s: input is not valid in \"%s\" encoding"),
                          context->from_filename, context->from_code));
  else
    po_xerror (PO_SEVERITY_FATAL_ERROR, context->message, NULL, 0, 0, false,
               xasprintf (_("%s: error while converting from \"%s\" encoding to \"%s\" encoding"),
                          context->from_filename, context->from_code,
                          context->to_code));
  /* NOTREACHED */
  abort ();
}

char *
convert_string_directly (iconv_t cd, const char *string,
                         const struct conversion_context* context)
{
  size_t len = strlen (string) + 1;
  char *result = NULL;
  size_t resultlen = 0;

  if (xmem_cd_iconv (string, len, cd, &result, &resultlen) == 0)
    /* Verify the result has exactly one NUL byte, at the end.  */
    if (resultlen > 0 && result[resultlen - 1] == '\0'
        && strlen (result) == resultlen - 1)
      return result;

  conversion_error (context);
  /* NOTREACHED */
  return NULL;
}

string_desc_t
convert_string_desc_directly (iconv_t cd, string_desc_t string,
                              const struct conversion_context* context)
{
  char *result = NULL;
  size_t resultlen = 0;

  if (xmem_cd_iconv (string_desc_data (string), string_desc_length (string),
                     cd, &result, &resultlen) == 0)
    return string_desc_new_addr (resultlen, result);

  conversion_error (context);
  /* NOTREACHED */
  return string_desc_new_empty ();
}

static char *
convert_string (const iconveh_t *cd, const char *string,
                const struct conversion_context* context)
{
  size_t len = strlen (string) + 1;
  char *result = NULL;
  size_t resultlen = 0;

  if (xmem_cd_iconveh (string, len, cd, iconveh_error, NULL,
                       &result, &resultlen) == 0)
    /* Verify the result has exactly one NUL byte, at the end.  */
    if (resultlen > 0 && result[resultlen - 1] == '\0'
        && strlen (result) == resultlen - 1)
      return result;

  conversion_error (context);
  /* NOTREACHED */
  return NULL;
}

static void
convert_string_list (const iconveh_t *cd, string_list_ty *slp,
                     const struct conversion_context* context)
{
  size_t i;

  if (slp != NULL)
    for (i = 0; i < slp->nitems; i++)
      slp->item[i] = convert_string (cd, slp->item[i], context);
}

static void
convert_prev_msgid (const iconveh_t *cd, message_ty *mp,
                    const struct conversion_context* context)
{
  if (mp->prev_msgctxt != NULL)
    mp->prev_msgctxt = convert_string (cd, mp->prev_msgctxt, context);
  if (mp->prev_msgid != NULL)
    mp->prev_msgid = convert_string (cd, mp->prev_msgid, context);
  if (mp->prev_msgid_plural != NULL)
    mp->prev_msgid_plural = convert_string (cd, mp->prev_msgid_plural, context);
}

static void
convert_msgid (const iconveh_t *cd, message_ty *mp,
               const struct conversion_context* context)
{
  if (mp->msgctxt != NULL)
    mp->msgctxt = convert_string (cd, mp->msgctxt, context);
  mp->msgid = convert_string (cd, mp->msgid, context);
  if (mp->msgid_plural != NULL)
    mp->msgid_plural = convert_string (cd, mp->msgid_plural, context);
}

static void
convert_msgstr (const iconveh_t *cd, message_ty *mp,
                const struct conversion_context* context)
{
  char *result = NULL;
  size_t resultlen = 0;

  if (!(mp->msgstr_len > 0 && mp->msgstr[mp->msgstr_len - 1] == '\0'))
    abort ();

  if (xmem_cd_iconveh (mp->msgstr, mp->msgstr_len, cd, iconveh_error, NULL,
                       &result, &resultlen) == 0)
    /* Verify the result has a NUL byte at the end.  */
    if (resultlen > 0 && result[resultlen - 1] == '\0')
      /* Verify the result has the same number of NUL bytes.  */
      {
        const char *p;
        const char *pend;
        int nulcount1;
        int nulcount2;

        for (p = mp->msgstr, pend = p + mp->msgstr_len, nulcount1 = 0;
             p < pend;
             p += strlen (p) + 1, nulcount1++);
        for (p = result, pend = p + resultlen, nulcount2 = 0;
             p < pend;
             p += strlen (p) + 1, nulcount2++);

        if (nulcount1 == nulcount2)
          {
            mp->msgstr = result;
            mp->msgstr_len = resultlen;
            return;
          }
      }

  conversion_error (context);
}

#endif


static bool
iconv_message_list_internal (message_list_ty *mlp,
                             const char *canon_from_code,
                             const char *canon_to_code,
                             bool update_header,
                             const char *from_filename)
{
  bool canon_from_code_overridden = (canon_from_code != NULL);
  bool msgids_changed;
  size_t j;

  /* If the list is empty, nothing to do.  */
  if (mlp->nitems == 0)
    return false;

  /* Search the header entry, and extract and replace the charset name.  */
  for (j = 0; j < mlp->nitems; j++)
    if (is_header (mlp->item[j]) && !mlp->item[j]->obsolete)
      {
        const char *header = mlp->item[j]->msgstr;

        if (header != NULL)
          {
            const char *charsetstr = c_strstr (header, "charset=");

            if (charsetstr != NULL)
              {
                size_t len;
                char *charset;
                const char *canon_charset;

                charsetstr += strlen ("charset=");
                len = strcspn (charsetstr, " \t\n");
                charset = (char *) xmalloca (len + 1);
                memcpy (charset, charsetstr, len);
                charset[len] = '\0';

                canon_charset = po_charset_canonicalize (charset);
                if (canon_charset == NULL)
                  {
                    if (!canon_from_code_overridden)
                      {
                        /* Don't give an error for POT files, because
                           POT files usually contain only ASCII msgids.
                           Also don't give an error for disguised POT
                           files that actually contain only ASCII msgids.  */
                        const char *filename = from_filename;
                        size_t filenamelen;

                        if (strcmp (charset, "CHARSET") == 0
                            && ((filename != NULL
                                 && (filenamelen = strlen (filename)) >= 4
                                 && memcmp (filename + filenamelen - 4, ".pot", 4)
                                    == 0)
                                || is_ascii_message_list (mlp)))
                          canon_charset = po_charset_ascii;
                        else
                          po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0,
                                     false,
                                     xasprintf (_("present charset \"%s\" is not a portable encoding name"),
                                                charset));
                      }
                  }
                else
                  {
                    if (canon_from_code == NULL)
                      canon_from_code = canon_charset;
                    else if (canon_from_code != canon_charset)
                      po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0,  0,
                                 false,
                                 xasprintf (_("two different charsets \"%s\" and \"%s\" in input file"),
                                            canon_from_code, canon_charset));
                  }
                freea (charset);

                if (update_header)
                  {
                    size_t len1, len2, len3;
                    char *new_header;

                    len1 = charsetstr - header;
                    len2 = strlen (canon_to_code);
                    len3 = (header + strlen (header)) - (charsetstr + len);
                    new_header = XNMALLOC (len1 + len2 + len3 + 1, char);
                    memcpy (new_header, header, len1);
                    memcpy (new_header + len1, canon_to_code, len2);
                    memcpy (new_header + len1 + len2, charsetstr + len,
                            len3 + 1);
                    mlp->item[j]->msgstr = new_header;
                    mlp->item[j]->msgstr_len = len1 + len2 + len3 + 1;
                  }
              }
          }
      }
  if (canon_from_code == NULL)
    {
      if (is_ascii_message_list (mlp))
        canon_from_code = po_charset_ascii;
      else
        po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                   _("input file doesn't contain a header entry with a charset specification"));
    }

  msgids_changed = false;

  /* If the two encodings are the same, nothing to do.  */
  if (canon_from_code != canon_to_code)
    {
#if HAVE_ICONV
      iconveh_t cd;
      struct conversion_context context;

      if (iconveh_open (canon_to_code, canon_from_code, &cd) < 0)
        po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                   xasprintf (_("Cannot convert from \"%s\" to \"%s\". %s relies on iconv(), and iconv() does not support this conversion."),
                              canon_from_code, canon_to_code,
                              last_component (program_name)));

      context.from_code = canon_from_code;
      context.to_code = canon_to_code;
      context.from_filename = from_filename;

      for (j = 0; j < mlp->nitems; j++)
        {
          message_ty *mp = mlp->item[j];

          if ((mp->msgctxt != NULL && !is_ascii_string (mp->msgctxt))
              || !is_ascii_string (mp->msgid))
            msgids_changed = true;
          context.message = mp;
          convert_string_list (&cd, mp->comment, &context);
          convert_string_list (&cd, mp->comment_dot, &context);
          convert_prev_msgid (&cd, mp, &context);
          convert_msgid (&cd, mp, &context);
          convert_msgstr (&cd, mp, &context);
        }

      iconveh_close (&cd);

      if (msgids_changed)
        if (message_list_msgids_changed (mlp))
          po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                     xasprintf (_("Conversion from \"%s\" to \"%s\" introduces duplicates: some different msgids become equal."),
                                canon_from_code, canon_to_code));
#else
          po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                     xasprintf (_("Cannot convert from \"%s\" to \"%s\". %s relies on iconv(). This version was built without iconv()."),
                                canon_from_code, canon_to_code,
                                last_component (program_name)));
#endif
    }

  return msgids_changed;
}

bool
iconv_message_list (message_list_ty *mlp,
                    const char *canon_from_code, const char *canon_to_code,
                    const char *from_filename)
{
  return iconv_message_list_internal (mlp,
                                      canon_from_code, canon_to_code, true,
                                      from_filename);
}

msgdomain_list_ty *
iconv_msgdomain_list (msgdomain_list_ty *mdlp,
                      const char *to_code,
                      bool update_header,
                      const char *from_filename)
{
  const char *canon_to_code;
  size_t k;

  /* Canonicalize target encoding.  */
  canon_to_code = po_charset_canonicalize (to_code);
  if (canon_to_code == NULL)
    po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
               xasprintf (_("target charset \"%s\" is not a portable encoding name."),
                          to_code));

  /* Test whether the control characters required for escaping file names with
     spaces are present in the target encoding.  */
  if (msgdomain_list_has_filenames_with_spaces (mdlp)
      && !(canon_to_code == po_charset_utf8
           || strcmp (canon_to_code, "GB18030") == 0))
    po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
               xasprintf (_("Cannot write the control characters that protect file names with spaces in the %s encoding"),
                          canon_to_code));

  for (k = 0; k < mdlp->nitems; k++)
    iconv_message_list_internal (mdlp->item[k]->messages,
                                 mdlp->encoding, canon_to_code, update_header,
                                 from_filename);

  mdlp->encoding = canon_to_code;
  return mdlp;
}

#if HAVE_ICONV

static bool
iconvable_string (const iconveh_t *cd, const char *string)
{
  size_t len = strlen (string) + 1;
  char *result = NULL;
  size_t resultlen = 0;

  if (xmem_cd_iconveh (string, len, cd, iconveh_error, NULL,
                       &result, &resultlen) == 0)
    {
      /* Test if the result has exactly one NUL byte, at the end.  */
      bool ok = (resultlen > 0 && result[resultlen - 1] == '\0'
                 && strlen (result) == resultlen - 1);
      free (result);
      return ok;
    }
  return false;
}

static bool
iconvable_string_list (const iconveh_t *cd, string_list_ty *slp)
{
  size_t i;

  if (slp != NULL)
    for (i = 0; i < slp->nitems; i++)
      if (!iconvable_string (cd, slp->item[i]))
        return false;
  return true;
}

static bool
iconvable_prev_msgid (const iconveh_t *cd, message_ty *mp)
{
  if (mp->prev_msgctxt != NULL)
    if (!iconvable_string (cd, mp->prev_msgctxt))
      return false;
  if (mp->prev_msgid != NULL)
    if (!iconvable_string (cd, mp->prev_msgid))
      return false;
  if (mp->prev_msgid_plural != NULL)
    if (!iconvable_string (cd, mp->prev_msgid_plural))
      return false;
  return true;
}

static bool
iconvable_msgid (const iconveh_t *cd, message_ty *mp)
{
  if (mp->msgctxt != NULL)
    if (!iconvable_string (cd, mp->msgctxt))
      return false;
  if (!iconvable_string (cd, mp->msgid))
    return false;
  if (mp->msgid_plural != NULL)
    if (!iconvable_string (cd, mp->msgid_plural))
      return false;
  return true;
}

static bool
iconvable_msgstr (const iconveh_t *cd, message_ty *mp)
{
  char *result = NULL;
  size_t resultlen = 0;

  if (!(mp->msgstr_len > 0 && mp->msgstr[mp->msgstr_len - 1] == '\0'))
    abort ();

  if (xmem_cd_iconveh (mp->msgstr, mp->msgstr_len, cd, iconveh_error, NULL,
                       &result, &resultlen) == 0)
    {
      bool ok = false;

      /* Test if the result has a NUL byte at the end.  */
      if (resultlen > 0 && result[resultlen - 1] == '\0')
        /* Test if the result has the same number of NUL bytes.  */
        {
          const char *p;
          const char *pend;
          int nulcount1;
          int nulcount2;

          for (p = mp->msgstr, pend = p + mp->msgstr_len, nulcount1 = 0;
               p < pend;
               p += strlen (p) + 1, nulcount1++);
          for (p = result, pend = p + resultlen, nulcount2 = 0;
               p < pend;
               p += strlen (p) + 1, nulcount2++);

          if (nulcount1 == nulcount2)
            ok = true;
        }

      free (result);
      return ok;
    }
  return false;
}

#endif

bool
is_message_list_iconvable (message_list_ty *mlp,
                           const char *canon_from_code,
                           const char *canon_to_code)
{
  bool canon_from_code_overridden = (canon_from_code != NULL);
  size_t j;

  /* If the list is empty, nothing to check.  */
  if (mlp->nitems == 0)
    return true;

  /* Search the header entry, and extract the charset name.  */
  for (j = 0; j < mlp->nitems; j++)
    if (is_header (mlp->item[j]) && !mlp->item[j]->obsolete)
      {
        const char *header = mlp->item[j]->msgstr;

        if (header != NULL)
          {
            const char *charsetstr = c_strstr (header, "charset=");

            if (charsetstr != NULL)
              {
                size_t len;
                char *charset;
                const char *canon_charset;

                charsetstr += strlen ("charset=");
                len = strcspn (charsetstr, " \t\n");
                charset = (char *) xmalloca (len + 1);
                memcpy (charset, charsetstr, len);
                charset[len] = '\0';

                canon_charset = po_charset_canonicalize (charset);
                if (canon_charset == NULL)
                  {
                    if (!canon_from_code_overridden)
                      {
                        /* Don't give an error for POT files, because POT
                           files usually contain only ASCII msgids.  */
                        if (strcmp (charset, "CHARSET") == 0)
                          canon_charset = po_charset_ascii;
                        else
                          {
                            /* charset is not a portable encoding name.  */
                            freea (charset);
                            return false;
                          }
                      }
                  }
                else
                  {
                    if (canon_from_code == NULL)
                      canon_from_code = canon_charset;
                    else if (canon_from_code != canon_charset)
                      {
                        /* Two different charsets in input file.  */
                        freea (charset);
                        return false;
                      }
                  }
                freea (charset);
              }
          }
      }
  if (canon_from_code == NULL)
    {
      if (is_ascii_message_list (mlp))
        canon_from_code = po_charset_ascii;
      else
        /* Input file lacks a header entry with a charset specification.  */
        return false;
    }

  /* If the two encodings are the same, nothing to check.  */
  if (canon_from_code != canon_to_code)
    {
#if HAVE_ICONV
      iconveh_t cd;

      if (iconveh_open (canon_to_code, canon_from_code, &cd) < 0)
        /* iconv() doesn't support this conversion.  */
        return false;

      for (j = 0; j < mlp->nitems; j++)
        {
          message_ty *mp = mlp->item[j];

          if (!(iconvable_string_list (&cd, mp->comment)
                && iconvable_string_list (&cd, mp->comment_dot)
                && iconvable_prev_msgid (&cd, mp)
                && iconvable_msgid (&cd, mp)
                && iconvable_msgstr (&cd, mp)))
            return false;
        }

      iconveh_close (&cd);
#else
      /* This version was built without iconv().  */
      return false;
#endif
    }

  return true;
}

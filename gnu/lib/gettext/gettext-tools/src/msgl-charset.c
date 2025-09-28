/* Message list charset and locale charset handling.
   Copyright (C) 2001-2003, 2005-2007, 2009, 2019-2020 Free Software Foundation, Inc.
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
#include "msgl-charset.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "po-charset.h"
#include "localcharset.h"
#include "error.h"
#include "progname.h"
#include "basename-lgpl.h"
#include "xmalloca.h"
#include "xerror.h"
#include "xvasprintf.h"
#include "message.h"
#include "c-strstr.h"
#include "gettext.h"

#define _(str) gettext (str)

void
compare_po_locale_charsets (const msgdomain_list_ty *mdlp)
{
  const char *locale_code;
  const char *canon_locale_code;
  bool warned;
  size_t j, k;

  /* Check whether the locale encoding and the PO file's encoding are the
     same.  Otherwise emit a warning.  */
  locale_code = locale_charset ();
  canon_locale_code = po_charset_canonicalize (locale_code);
  warned = false;
  for (k = 0; k < mdlp->nitems; k++)
    {
      const message_list_ty *mlp = mdlp->item[k]->messages;

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
                      error (EXIT_FAILURE, 0,
                             _("present charset \"%s\" is not a portable encoding name"),
                             charset);
                    freea (charset);
                    if (canon_locale_code != canon_charset)
                      {
                        multiline_warning (xasprintf (_("warning: ")),
                                           xasprintf (_("\
Locale charset \"%s\" is different from\n\
input file charset \"%s\".\n\
Output of '%s' might be incorrect.\n\
Possible workarounds are:\n\
"), locale_code, canon_charset, last_component (program_name)));
                        multiline_warning (NULL,
                                           xasprintf (_("\
- Set LC_ALL to a locale with encoding %s.\n\
"), canon_charset));
                        if (canon_locale_code != NULL)
                          multiline_warning (NULL,
                                             xasprintf (_("\
- Convert the translation catalog to %s using 'msgconv',\n\
  then apply '%s',\n\
  then convert back to %s using 'msgconv'.\n\
"), canon_locale_code, last_component (program_name), canon_charset));
                        if (strcmp (canon_charset, "UTF-8") != 0
                            && (canon_locale_code == NULL
                                || strcmp (canon_locale_code, "UTF-8") != 0))
                          multiline_warning (NULL,
                                             xasprintf (_("\
- Set LC_ALL to a locale with encoding %s,\n\
  convert the translation catalog to %s using 'msgconv',\n\
  then apply '%s',\n\
  then convert back to %s using 'msgconv'.\n\
"), "UTF-8", "UTF-8", last_component (program_name), canon_charset));
                        warned = true;
                      }
                  }
              }
          }
      }
  if (canon_locale_code == NULL && !warned)
    multiline_warning (xasprintf (_("warning: ")),
                       xasprintf (_("\
Locale charset \"%s\" is not a portable encoding name.\n\
Output of '%s' might be incorrect.\n\
A possible workaround is to set LC_ALL=C.\n\
"), locale_code, last_component (program_name)));
}

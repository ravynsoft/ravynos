/* xgettext C/C++/ObjectiveC backend.
   Copyright (C) 1995-1998, 2000-2009, 2012-2015, 2018-2023 Free Software Foundation, Inc.

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
# include "config.h"
#endif

/* Specification.  */
#include "x-c.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attribute.h"
#include "message.h"
#include "rc-str-list.h"
#include "xgettext.h"
#include "xg-pos.h"
#include "xg-encoding.h"
#include "xg-mixed-string.h"
#include "xg-arglist-context.h"
#include "xg-arglist-callshape.h"
#include "xg-arglist-parser.h"
#include "xg-message.h"
#include "error.h"
#include "error-progname.h"
#include "xalloc.h"
#include "xvasprintf.h"
#include "mem-hash-map.h"
#include "po-charset.h"
#include "gettext.h"

#define _(s) gettext(s)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* The ANSI C standard defines several phases of translation:

   1. Terminate line by \n, regardless of the external representation
      of a text line.  Stdio does this for us.

   2. Convert trigraphs to their single character equivalents.

   3. Concatenate each line ending in backslash (\) with the following
      line.

   4. Replace each comment with a space character.

   5. Parse each resulting logical line as preprocessing tokens a
      white space.

   6. Recognize and carry out directives (it also expands macros on
      non-directive lines, which we do not do here).

   7. Replaces escape sequences within character strings with their
      single character equivalents (we do this in step 5, because we
      don't have to worry about the #include argument).

   8. Concatenates adjacent string literals to form single string
      literals (because we don't expand macros, there are a few things
      we will miss).

   9. Converts the remaining preprocessing tokens to C tokens and
      discards any white space from the translation unit.

   This lexer implements the above, and presents the scanner (in
   xgettext.c) with a stream of C tokens.  The comments are
   accumulated in a buffer, and given to xgettext when asked for.  */


/* ========================= Lexer customization.  ========================= */

static bool trigraphs = false;

void
x_c_trigraphs ()
{
  trigraphs = true;
}


/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table c_keywords;
static hash_table objc_keywords;
static bool default_keywords = true;


void
x_c_extract_all ()
{
  extract_all = true;
}


static void
add_keyword (const char *name, hash_table *keywords)
{
  if (name == NULL)
    default_keywords = false;
  else
    {
      const char *end;
      struct callshape shape;
      const char *colon;

      if (keywords->table == NULL)
        hash_init (keywords, 100);

      split_keywordspec (name, &end, &shape);

      /* The characters between name and end should form a valid C identifier.
         A colon means an invalid parse in split_keywordspec().  */
      colon = strchr (name, ':');
      if (colon == NULL || colon >= end)
        insert_keyword_callshape (keywords, name, end - name, &shape);
    }
}

void
x_c_keyword (const char *name)
{
  add_keyword (name, &c_keywords);
}

void
x_objc_keyword (const char *name)
{
  add_keyword (name, &objc_keywords);
}

static bool additional_keywords_kde;

void
activate_additional_keywords_kde ()
{
  additional_keywords_kde = true;
}

/* Finish initializing the keywords hash tables.
   Called after argument processing, before each file is processed.  */
static void
init_keywords ()
{
  if (default_keywords)
    {
      /* When adding new keywords here, also update the documentation in
         xgettext.texi!  */
      x_c_keyword ("gettext");
      x_c_keyword ("dgettext:2");
      x_c_keyword ("dcgettext:2");
      x_c_keyword ("ngettext:1,2");
      x_c_keyword ("dngettext:2,3");
      x_c_keyword ("dcngettext:2,3");
      x_c_keyword ("gettext_noop");
      x_c_keyword ("pgettext:1c,2");
      x_c_keyword ("dpgettext:2c,3");
      x_c_keyword ("dcpgettext:2c,3");
      x_c_keyword ("npgettext:1c,2,3");
      x_c_keyword ("dnpgettext:2c,3,4");
      x_c_keyword ("dcnpgettext:2c,3,4");

      if (additional_keywords_kde)
        {
          x_c_keyword ("i18n:1");
          x_c_keyword ("i18nc:1c,2");
          x_c_keyword ("i18np:1,2");
          x_c_keyword ("i18ncp:1c,2,3");
          x_c_keyword ("i18nd:2");
          x_c_keyword ("i18ndc:2c,3");
          x_c_keyword ("i18ndp:2,3");
          x_c_keyword ("i18ndcp:2c,3,4");
          x_c_keyword ("ki18n:1");
          x_c_keyword ("ki18nc:1c,2");
          x_c_keyword ("ki18np:1,2");
          x_c_keyword ("ki18ncp:1c,2,3");
          x_c_keyword ("ki18nd:2");
          x_c_keyword ("ki18ndc:2c,3");
          x_c_keyword ("ki18ndp:2,3");
          x_c_keyword ("ki18ndcp:2c,3,4");
          x_c_keyword ("I18N_NOOP:1");
          x_c_keyword ("I18NC_NOOP:1c,2");
          x_c_keyword ("I18N_NOOP2:1c,2");
          x_c_keyword ("I18N_NOOP2_NOSTRIP:1c,2");
          x_c_keyword ("xi18n:1");
          x_c_keyword ("xi18nc:1c,2");
          x_c_keyword ("xi18np:1,2");
          x_c_keyword ("xi18ncp:1c,2,3");
          x_c_keyword ("xi18nd:2");
          x_c_keyword ("xi18ndc:2c,3");
          x_c_keyword ("xi18ndp:2,3");
          x_c_keyword ("xi18ndcp:2c,3,4");
          x_c_keyword ("kxi18n:1");
          x_c_keyword ("kxi18nc:1c,2");
          x_c_keyword ("kxi18np:1,2");
          x_c_keyword ("kxi18ncp:1c,2,3");
          x_c_keyword ("kxi18nd:2");
          x_c_keyword ("kxi18ndc:2c,3");
          x_c_keyword ("kxi18ndp:2,3");
          x_c_keyword ("kxi18ndcp:2c,3,4");
          x_c_keyword ("XI18N_NOOP:1");
          x_c_keyword ("XI18NC_NOOP:1c,2");
          x_c_keyword ("XI18N_NOOP2:1c,2");
          x_c_keyword ("XI18N_NOOP2_NOSTRIP:1c,2");
        }

      x_objc_keyword ("gettext");
      x_objc_keyword ("dgettext:2");
      x_objc_keyword ("dcgettext:2");
      x_objc_keyword ("ngettext:1,2");
      x_objc_keyword ("dngettext:2,3");
      x_objc_keyword ("dcngettext:2,3");
      x_objc_keyword ("gettext_noop");
      x_objc_keyword ("pgettext:1c,2");
      x_objc_keyword ("dpgettext:2c,3");
      x_objc_keyword ("dcpgettext:2c,3");
      x_objc_keyword ("npgettext:1c,2,3");
      x_objc_keyword ("dnpgettext:2c,3,4");
      x_objc_keyword ("dcnpgettext:2c,3,4");
      x_objc_keyword ("NSLocalizedString");       /* similar to gettext */
      x_objc_keyword ("_");                       /* similar to gettext */
      x_objc_keyword ("NSLocalizedStaticString"); /* similar to gettext_noop */
      x_objc_keyword ("__");                      /* similar to gettext_noop */

      default_keywords = false;
    }
}

void
init_flag_table_c ()
{
  xgettext_record_flag ("gettext:1:pass-c-format");
  xgettext_record_flag ("dgettext:2:pass-c-format");
  xgettext_record_flag ("dcgettext:2:pass-c-format");
  xgettext_record_flag ("ngettext:1:pass-c-format");
  xgettext_record_flag ("ngettext:2:pass-c-format");
  xgettext_record_flag ("dngettext:2:pass-c-format");
  xgettext_record_flag ("dngettext:3:pass-c-format");
  xgettext_record_flag ("dcngettext:2:pass-c-format");
  xgettext_record_flag ("dcngettext:3:pass-c-format");
  xgettext_record_flag ("gettext_noop:1:pass-c-format");
  xgettext_record_flag ("pgettext:2:pass-c-format");
  xgettext_record_flag ("dpgettext:3:pass-c-format");
  xgettext_record_flag ("dcpgettext:3:pass-c-format");
  xgettext_record_flag ("npgettext:2:pass-c-format");
  xgettext_record_flag ("npgettext:3:pass-c-format");
  xgettext_record_flag ("dnpgettext:3:pass-c-format");
  xgettext_record_flag ("dnpgettext:4:pass-c-format");
  xgettext_record_flag ("dcnpgettext:3:pass-c-format");
  xgettext_record_flag ("dcnpgettext:4:pass-c-format");

  /* <stdio.h> */
  xgettext_record_flag ("fprintf:2:c-format");
  xgettext_record_flag ("vfprintf:2:c-format");
  xgettext_record_flag ("printf:1:c-format");
  xgettext_record_flag ("vprintf:1:c-format");
  xgettext_record_flag ("sprintf:2:c-format");
  xgettext_record_flag ("vsprintf:2:c-format");
  xgettext_record_flag ("snprintf:3:c-format");
  xgettext_record_flag ("vsnprintf:3:c-format");
#if 0 /* These functions are not standard.  */
  /* <stdio.h> */
  xgettext_record_flag ("asprintf:2:c-format");
  xgettext_record_flag ("vasprintf:2:c-format");
  xgettext_record_flag ("dprintf:2:c-format");
  xgettext_record_flag ("vdprintf:2:c-format");
  xgettext_record_flag ("obstack_printf:2:c-format");
  xgettext_record_flag ("obstack_vprintf:2:c-format");
  /* <error.h> */
  xgettext_record_flag ("error:3:c-format");
  xgettext_record_flag ("error_at_line:5:c-format");
  /* <argp.h> */
  xgettext_record_flag ("argp_error:2:c-format");
  xgettext_record_flag ("argp_failure:2:c-format");
#endif

  xgettext_record_flag ("gettext:1:pass-c++-format");
  xgettext_record_flag ("dgettext:2:pass-c++-format");
  xgettext_record_flag ("dcgettext:2:pass-c++-format");
  xgettext_record_flag ("ngettext:1:pass-c++-format");
  xgettext_record_flag ("ngettext:2:pass-c++-format");
  xgettext_record_flag ("dngettext:2:pass-c++-format");
  xgettext_record_flag ("dngettext:3:pass-c++-format");
  xgettext_record_flag ("dcngettext:2:pass-c++-format");
  xgettext_record_flag ("dcngettext:3:pass-c++-format");
  xgettext_record_flag ("gettext_noop:1:pass-c++-format");
  xgettext_record_flag ("pgettext:2:pass-c++-format");
  xgettext_record_flag ("dpgettext:3:pass-c++-format");
  xgettext_record_flag ("dcpgettext:3:pass-c++-format");
  xgettext_record_flag ("npgettext:2:pass-c++-format");
  xgettext_record_flag ("npgettext:3:pass-c++-format");
  xgettext_record_flag ("dnpgettext:3:pass-c++-format");
  xgettext_record_flag ("dnpgettext:4:pass-c++-format");
  xgettext_record_flag ("dcnpgettext:3:pass-c++-format");
  xgettext_record_flag ("dcnpgettext:4:pass-c++-format");

  /* C++ <format> */
  xgettext_record_flag ("vformat:1:c++-format");
  xgettext_record_flag ("vformat_to:2:c++-format");

  xgettext_record_flag ("gettext:1:pass-qt-format");
  xgettext_record_flag ("dgettext:2:pass-qt-format");
  xgettext_record_flag ("dcgettext:2:pass-qt-format");
  xgettext_record_flag ("ngettext:1:pass-qt-format");
  xgettext_record_flag ("ngettext:2:pass-qt-format");
  xgettext_record_flag ("dngettext:2:pass-qt-format");
  xgettext_record_flag ("dngettext:3:pass-qt-format");
  xgettext_record_flag ("dcngettext:2:pass-qt-format");
  xgettext_record_flag ("dcngettext:3:pass-qt-format");
  xgettext_record_flag ("gettext_noop:1:pass-qt-format");
  xgettext_record_flag ("pgettext:2:pass-qt-format");
  xgettext_record_flag ("dpgettext:3:pass-qt-format");
  xgettext_record_flag ("dcpgettext:3:pass-qt-format");
  xgettext_record_flag ("npgettext:2:pass-qt-format");
  xgettext_record_flag ("npgettext:3:pass-qt-format");
  xgettext_record_flag ("dnpgettext:3:pass-qt-format");
  xgettext_record_flag ("dnpgettext:4:pass-qt-format");
  xgettext_record_flag ("dcnpgettext:3:pass-qt-format");
  xgettext_record_flag ("dcnpgettext:4:pass-qt-format");

  xgettext_record_flag ("gettext:1:pass-kde-format");
  xgettext_record_flag ("dgettext:2:pass-kde-format");
  xgettext_record_flag ("dcgettext:2:pass-kde-format");
  xgettext_record_flag ("ngettext:1:pass-kde-format");
  xgettext_record_flag ("ngettext:2:pass-kde-format");
  xgettext_record_flag ("dngettext:2:pass-kde-format");
  xgettext_record_flag ("dngettext:3:pass-kde-format");
  xgettext_record_flag ("dcngettext:2:pass-kde-format");
  xgettext_record_flag ("dcngettext:3:pass-kde-format");
  xgettext_record_flag ("gettext_noop:1:pass-kde-format");
  xgettext_record_flag ("pgettext:2:pass-kde-format");
  xgettext_record_flag ("dpgettext:3:pass-kde-format");
  xgettext_record_flag ("dcpgettext:3:pass-kde-format");
  xgettext_record_flag ("npgettext:2:pass-kde-format");
  xgettext_record_flag ("npgettext:3:pass-kde-format");
  xgettext_record_flag ("dnpgettext:3:pass-kde-format");
  xgettext_record_flag ("dnpgettext:4:pass-kde-format");
  xgettext_record_flag ("dcnpgettext:3:pass-kde-format");
  xgettext_record_flag ("dcnpgettext:4:pass-kde-format");

  xgettext_record_flag ("gettext:1:pass-boost-format");
  xgettext_record_flag ("dgettext:2:pass-boost-format");
  xgettext_record_flag ("dcgettext:2:pass-boost-format");
  xgettext_record_flag ("ngettext:1:pass-boost-format");
  xgettext_record_flag ("ngettext:2:pass-boost-format");
  xgettext_record_flag ("dngettext:2:pass-boost-format");
  xgettext_record_flag ("dngettext:3:pass-boost-format");
  xgettext_record_flag ("dcngettext:2:pass-boost-format");
  xgettext_record_flag ("dcngettext:3:pass-boost-format");
  xgettext_record_flag ("gettext_noop:1:pass-boost-format");
  xgettext_record_flag ("pgettext:2:pass-boost-format");
  xgettext_record_flag ("dpgettext:3:pass-boost-format");
  xgettext_record_flag ("dcpgettext:3:pass-boost-format");
  xgettext_record_flag ("npgettext:2:pass-boost-format");
  xgettext_record_flag ("npgettext:3:pass-boost-format");
  xgettext_record_flag ("dnpgettext:3:pass-boost-format");
  xgettext_record_flag ("dnpgettext:4:pass-boost-format");
  xgettext_record_flag ("dcnpgettext:3:pass-boost-format");
  xgettext_record_flag ("dcnpgettext:4:pass-boost-format");

  /* <boost/format.hpp> */
  xgettext_record_flag ("format:1:boost-format");
}

void
init_flag_table_objc ()
{
  /* Since the settings done in init_flag_table_c() also have an effect for
     the ObjectiveC parser, we don't have to repeat them here.  */
  xgettext_record_flag ("gettext:1:pass-objc-format");
  xgettext_record_flag ("dgettext:2:pass-objc-format");
  xgettext_record_flag ("dcgettext:2:pass-objc-format");
  xgettext_record_flag ("ngettext:1:pass-objc-format");
  xgettext_record_flag ("ngettext:2:pass-objc-format");
  xgettext_record_flag ("dngettext:2:pass-objc-format");
  xgettext_record_flag ("dngettext:3:pass-objc-format");
  xgettext_record_flag ("dcngettext:2:pass-objc-format");
  xgettext_record_flag ("dcngettext:3:pass-objc-format");
  xgettext_record_flag ("gettext_noop:1:pass-objc-format");
  xgettext_record_flag ("pgettext:2:pass-objc-format");
  xgettext_record_flag ("dpgettext:3:pass-objc-format");
  xgettext_record_flag ("dcpgettext:3:pass-objc-format");
  xgettext_record_flag ("npgettext:2:pass-objc-format");
  xgettext_record_flag ("npgettext:3:pass-objc-format");
  xgettext_record_flag ("dnpgettext:3:pass-objc-format");
  xgettext_record_flag ("dnpgettext:4:pass-objc-format");
  xgettext_record_flag ("dcnpgettext:3:pass-objc-format");
  xgettext_record_flag ("dcnpgettext:4:pass-objc-format");
  xgettext_record_flag ("NSLocalizedString:1:pass-c-format");
  xgettext_record_flag ("NSLocalizedString:1:pass-objc-format");
  xgettext_record_flag ("_:1:pass-c-format");
  xgettext_record_flag ("_:1:pass-objc-format");
  xgettext_record_flag ("stringWithFormat::1:objc-format");
  xgettext_record_flag ("initWithFormat::1:objc-format");
  xgettext_record_flag ("stringByAppendingFormat::1:objc-format");
  xgettext_record_flag ("localizedStringWithFormat::1:objc-format");
  xgettext_record_flag ("appendFormat::1:objc-format");
}

void
init_flag_table_gcc_internal ()
{
  xgettext_record_flag ("gettext:1:pass-gcc-internal-format");
  xgettext_record_flag ("dgettext:2:pass-gcc-internal-format");
  xgettext_record_flag ("dcgettext:2:pass-gcc-internal-format");
  xgettext_record_flag ("ngettext:1:pass-gcc-internal-format");
  xgettext_record_flag ("ngettext:2:pass-gcc-internal-format");
  xgettext_record_flag ("dngettext:2:pass-gcc-internal-format");
  xgettext_record_flag ("dngettext:3:pass-gcc-internal-format");
  xgettext_record_flag ("dcngettext:2:pass-gcc-internal-format");
  xgettext_record_flag ("dcngettext:3:pass-gcc-internal-format");
  xgettext_record_flag ("gettext_noop:1:pass-gcc-internal-format");
  xgettext_record_flag ("pgettext:2:pass-gcc-internal-format");
  xgettext_record_flag ("dpgettext:3:pass-gcc-internal-format");
  xgettext_record_flag ("dcpgettext:3:pass-gcc-internal-format");
  xgettext_record_flag ("npgettext:2:pass-gcc-internal-format");
  xgettext_record_flag ("npgettext:3:pass-gcc-internal-format");
  xgettext_record_flag ("dnpgettext:3:pass-gcc-internal-format");
  xgettext_record_flag ("dnpgettext:4:pass-gcc-internal-format");
  xgettext_record_flag ("dcnpgettext:3:pass-gcc-internal-format");
  xgettext_record_flag ("dcnpgettext:4:pass-gcc-internal-format");
#if 0 /* This should better be done inside GCC.  */
  /* grepping for ATTRIBUTE_PRINTF in gcc-3.3/gcc/?*.h */
  /* c-format.c */
  xgettext_record_flag ("status_warning:2:gcc-internal-format");
  /* c-tree.h */
  xgettext_record_flag ("pedwarn_c99:1:pass-gcc-internal-format");
  /* collect2.h */
  //xgettext_record_flag ("error:1:c-format"); // 3 different versions
  xgettext_record_flag ("notice:1:c-format");
  //xgettext_record_flag ("fatal:1:c-format"); // 2 different versions
  xgettext_record_flag ("fatal_perror:1:c-format");
  /* cpplib.h */
  xgettext_record_flag ("cpp_error:3:c-format");
  xgettext_record_flag ("cpp_error_with_line:5:c-format");
  /* diagnostic.h */
  xgettext_record_flag ("diagnostic_set_info:2:pass-gcc-internal-format");
  xgettext_record_flag ("output_printf:2:gcc-internal-format");
  xgettext_record_flag ("output_verbatim:2:pass-gcc-internal-format");
  xgettext_record_flag ("verbatim:1:gcc-internal-format");
  xgettext_record_flag ("inform:1:pass-gcc-internal-format");
  /* gcc.h */
  //xgettext_record_flag ("fatal:1:c-format"); // 2 different versions
  //xgettext_record_flag ("error:1:c-format"); // 3 different versions
  /* genattrtab.h */
  xgettext_record_flag ("attr_printf:2:pass-c-format");
  /* gengtype.h */
  xgettext_record_flag ("error_at_line:2:pass-c-format");
  xgettext_record_flag ("xvasprintf:2:pass-c-format");
  xgettext_record_flag ("xasprintf:1:pass-c-format");
  xgettext_record_flag ("oprintf:2:pass-c-format");
  /* gensupport.h */
  xgettext_record_flag ("message_with_line:2:pass-c-format");
  /* output.h */
  xgettext_record_flag ("output_operand_lossage:1:c-format");
  /* ra.h */
   xgettext_record_flag ("ra_debug_msg:2:pass-c-format");
  /* toplev.h */
  xgettext_record_flag ("fnotice:2:c-format");
  xgettext_record_flag ("fatal_io_error:2:gcc-internal-format");
  xgettext_record_flag ("error_for_asm:2:pass-gcc-internal-format");
  xgettext_record_flag ("warning_for_asm:2:pass-gcc-internal-format");
  xgettext_record_flag ("error_with_file_and_line:3:pass-gcc-internal-format");
  xgettext_record_flag ("error_with_decl:2:pass-gcc-internal-format");
  xgettext_record_flag ("pedwarn:1:gcc-internal-format");
  xgettext_record_flag ("pedwarn_with_file_and_line:3:gcc-internal-format");
  xgettext_record_flag ("pedwarn_with_decl:2:gcc-internal-format");
  xgettext_record_flag ("sorry:1:gcc-internal-format");
  xgettext_record_flag ("error:1:pass-gcc-internal-format");
  xgettext_record_flag ("fatal_error:1:pass-gcc-internal-format");
  xgettext_record_flag ("internal_error:1:pass-gcc-internal-format");
  xgettext_record_flag ("warning:1:pass-gcc-internal-format");
  xgettext_record_flag ("warning_with_file_and_line:3:pass-gcc-internal-format");
  xgettext_record_flag ("warning_with_decl:2:pass-gcc-internal-format");
  /* f/com.h */
  xgettext_record_flag ("ffecom_get_invented_identifier:1:pass-c-format");
  /* f/sts.h */
  xgettext_record_flag ("ffests_printf:2:pass-c-format");
  /* java/java-tree.h */
  xgettext_record_flag ("parse_error_context:2:pass-c-format");
#endif

  xgettext_record_flag ("gettext:1:pass-gfc-internal-format");
  xgettext_record_flag ("dgettext:2:pass-gfc-internal-format");
  xgettext_record_flag ("dcgettext:2:pass-gfc-internal-format");
  xgettext_record_flag ("ngettext:1:pass-gfc-internal-format");
  xgettext_record_flag ("ngettext:2:pass-gfc-internal-format");
  xgettext_record_flag ("dngettext:2:pass-gfc-internal-format");
  xgettext_record_flag ("dngettext:3:pass-gfc-internal-format");
  xgettext_record_flag ("dcngettext:2:pass-gfc-internal-format");
  xgettext_record_flag ("dcngettext:3:pass-gfc-internal-format");
  xgettext_record_flag ("gettext_noop:1:pass-gfc-internal-format");
  xgettext_record_flag ("pgettext:2:pass-gfc-internal-format");
  xgettext_record_flag ("dpgettext:3:pass-gfc-internal-format");
  xgettext_record_flag ("dcpgettext:3:pass-gfc-internal-format");
  xgettext_record_flag ("npgettext:2:pass-gfc-internal-format");
  xgettext_record_flag ("npgettext:3:pass-gfc-internal-format");
  xgettext_record_flag ("dnpgettext:3:pass-gfc-internal-format");
  xgettext_record_flag ("dnpgettext:4:pass-gfc-internal-format");
  xgettext_record_flag ("dcnpgettext:3:pass-gfc-internal-format");
  xgettext_record_flag ("dcnpgettext:4:pass-gfc-internal-format");
#if 0 /* This should better be done inside GCC.  */
  /* fortran/error.c */
  xgettext_record_flag ("gfc_error:1:gfc-internal-format");
  xgettext_record_flag ("gfc_error_now:1:gfc-internal-format");
  xgettext_record_flag ("gfc_fatal_error:1:gfc-internal-format");
  xgettext_record_flag ("gfc_internal_error:1:gfc-internal-format");
  xgettext_record_flag ("gfc_notify_std:2:gfc-internal-format");
  xgettext_record_flag ("gfc_warning:1:gfc-internal-format");
  xgettext_record_flag ("gfc_warning_now:1:gfc-internal-format");
#endif
}

void
init_flag_table_kde ()
{
  xgettext_record_flag ("i18n:1:kde-format");
  xgettext_record_flag ("i18nc:2:kde-format");
  xgettext_record_flag ("i18np:1:kde-format");
  xgettext_record_flag ("i18ncp:2:kde-format");
  xgettext_record_flag ("i18nd:2:kde-format");
  xgettext_record_flag ("i18ndc:3:kde-format");
  xgettext_record_flag ("i18ndp:2:kde-format");
  xgettext_record_flag ("i18ndcp:3:kde-format");
  xgettext_record_flag ("ki18n:1:kde-format");
  xgettext_record_flag ("ki18nc:2:kde-format");
  xgettext_record_flag ("ki18np:1:kde-format");
  xgettext_record_flag ("ki18ncp:2:kde-format");
  xgettext_record_flag ("ki18nd:2:kde-format");
  xgettext_record_flag ("ki18ndc:3:kde-format");
  xgettext_record_flag ("ki18ndp:2:kde-format");
  xgettext_record_flag ("ki18ndcp:3:kde-format");
  xgettext_record_flag ("I18N_NOOP:1:kde-format");
  xgettext_record_flag ("I18NC_NOOP:2:kde-format");
  xgettext_record_flag ("I18N_NOOP2:2:kde-format");
  xgettext_record_flag ("I18N_NOOP2_NOSTRIP:2:kde-format");
  xgettext_record_flag ("xi18n:1:kde-kuit-format");
  xgettext_record_flag ("xi18nc:2:kde-kuit-format");
  xgettext_record_flag ("xi18np:1:kde-kuit-format");
  xgettext_record_flag ("xi18ncp:2:kde-kuit-format");
  xgettext_record_flag ("xi18nd:2:kde-kuit-format");
  xgettext_record_flag ("xi18ndc:3:kde-kuit-format");
  xgettext_record_flag ("xi18ndp:2:kde-kuit-format");
  xgettext_record_flag ("xi18ndcp:3:kde-kuit-format");
  xgettext_record_flag ("kxi18n:1:kde-kuit-format");
  xgettext_record_flag ("kxi18nc:2:kde-kuit-format");
  xgettext_record_flag ("kxi18np:1:kde-kuit-format");
  xgettext_record_flag ("kxi18ncp:2:kde-kuit-format");
  xgettext_record_flag ("kxi18nd:2:kde-kuit-format");
  xgettext_record_flag ("kxi18ndc:3:kde-kuit-format");
  xgettext_record_flag ("kxi18ndp:2:kde-kuit-format");
  xgettext_record_flag ("kxi18ndcp:3:kde-kuit-format");
  xgettext_record_flag ("XI18N_NOOP:1:kde-kuit-format");
  xgettext_record_flag ("XI18NC_NOOP:2:kde-kuit-format");
  xgettext_record_flag ("XI18N_NOOP2:2:kde-kuit-format");
  xgettext_record_flag ("XI18N_NOOP2_NOSTRIP:2:kde-kuit-format");
}

/* ======================== Reading of characters.  ======================== */

/* The input file stream.  */
static FILE *fp;


/* 0. Terminate line by \n, regardless whether the external representation of
   a line terminator is LF (Unix), CR (Mac) or CR/LF (DOS/Windows).
   It is debatable whether supporting CR/LF line terminators in C sources
   on Unix is ISO C or POSIX compliant, but since GCC 3.3 now supports it
   unconditionally, it must be OK.
   The so-called "text mode" in stdio on DOS/Windows translates CR/LF to \n
   automatically, but here we also need this conversion on Unix.  As a side
   effect, on DOS/Windows we also parse CR/CR/LF into a single \n, but this
   is not a problem.  */


static int
phase0_getc ()
{
  int c;

  c = getc (fp);
  if (c == EOF)
    {
      if (ferror (fp))
        error (EXIT_FAILURE, errno, _("error while reading \"%s\""),
               real_file_name);
      return EOF;
    }

  if (c == '\r')
    {
      int c1 = getc (fp);

      if (c1 != EOF && c1 != '\n')
        ungetc (c1, fp);

      /* Seen line terminator CR or CR/LF.  */
      return '\n';
    }

  return c;
}


/* Supports only one pushback character, and not '\n'.  */
static inline void
phase0_ungetc (int c)
{
  if (c != EOF)
    ungetc (c, fp);
}


/* 1. line_number handling.  Combine backslash-newline to nothing.  */

static unsigned char phase1_pushback[2];
static int phase1_pushback_length;


static int
phase1_getc ()
{
  int c;

  if (phase1_pushback_length)
    {
      c = phase1_pushback[--phase1_pushback_length];
      if (c == '\n')
        ++line_number;
      return c;
    }
  for (;;)
    {
      c = phase0_getc ();
      switch (c)
        {
        case '\n':
          ++line_number;
          return '\n';

        case '\\':
          c = phase0_getc ();
          if (c != '\n')
            {
              phase0_ungetc (c);
              return '\\';
            }
          ++line_number;
          break;

        default:
          return c;
        }
    }
}


/* Supports 2 characters of pushback.  */
static void
phase1_ungetc (int c)
{
  switch (c)
    {
    case EOF:
      break;

    case '\n':
      --line_number;
      FALLTHROUGH;

    default:
      if (phase1_pushback_length == SIZEOF (phase1_pushback))
        abort ();
      phase1_pushback[phase1_pushback_length++] = c;
      break;
    }
}


/* 2. Convert trigraphs to their single character equivalents.  Most
   sane human beings vomit copiously at the mention of trigraphs, which
   is why they are an option.  */

static unsigned char phase2_pushback[1];
static int phase2_pushback_length;


static int
phase2_getc ()
{
  int c;

  if (phase2_pushback_length)
    return phase2_pushback[--phase2_pushback_length];
  if (!trigraphs)
    return phase1_getc ();

  c = phase1_getc ();
  if (c != '?')
    return c;
  c = phase1_getc ();
  if (c != '?')
    {
      phase1_ungetc (c);
      return '?';
    }
  c = phase1_getc ();
  switch (c)
    {
    case '(':
      return '[';
    case '/':
      return '\\';
    case ')':
      return ']';
    case '\'':
      return '^';
    case '<':
      return '{';
    case '!':
      return '|';
    case '>':
      return '}';
    case '-':
      return '~';
    case '#':
      return '=';
    }
  phase1_ungetc (c);
  phase1_ungetc ('?');
  return '?';
}


/* Supports only one pushback character.  */
static void
phase2_ungetc (int c)
{
  if (c != EOF)
    {
      if (phase2_pushback_length == SIZEOF (phase2_pushback))
        abort ();
      phase2_pushback[phase2_pushback_length++] = c;
    }
}


/* 3. Concatenate each line ending in backslash (\) with the following
   line.  Basically, all you need to do is elide "\\\n" sequences from
   the input.  */

static unsigned char phase3_pushback[9];
static int phase3_pushback_length;


static int
phase3_getc ()
{
  if (phase3_pushback_length)
    return phase3_pushback[--phase3_pushback_length];
  for (;;)
    {
      int c = phase2_getc ();
      if (c != '\\')
        return c;
      c = phase2_getc ();
      if (c != '\n')
        {
          phase2_ungetc (c);
          return '\\';
        }
    }
}


/* Supports 9 characters of pushback.  */
static void
phase3_ungetc (int c)
{
  if (c != EOF)
    {
      if (phase3_pushback_length == SIZEOF (phase3_pushback))
        abort ();
      phase3_pushback[phase3_pushback_length++] = c;
    }
}


/* Accumulating comments.  */

static char *buffer;
static size_t bufmax;
static size_t buflen;

static inline void
comment_start ()
{
  buflen = 0;
}

static inline void
comment_add (int c)
{
  if (buflen >= bufmax)
    {
      bufmax = 2 * bufmax + 10;
      buffer = xrealloc (buffer, bufmax);
    }
  buffer[buflen++] = c;
}

static inline void
comment_line_end (size_t chars_to_remove)
{
  buflen -= chars_to_remove;
  while (buflen >= 1
         && (buffer[buflen - 1] == ' ' || buffer[buflen - 1] == '\t'))
    --buflen;
  if (chars_to_remove == 0 && buflen >= bufmax)
    {
      bufmax = 2 * bufmax + 10;
      buffer = xrealloc (buffer, bufmax);
    }
  buffer[buflen] = '\0';
  savable_comment_add (buffer);
}


/* These are for tracking whether comments count as immediately before
   keyword.  */
static int last_comment_line;
static int last_non_comment_line;
static int newline_count;


/* 4. Replace each comment that is not inside a character constant or
   string literal with a space character.  We need to remember the
   comment for later, because it may be attached to a keyword string.
   We also optionally understand C++ comments.  */

static int
phase4_getc ()
{
  int c;
  bool last_was_star;

  c = phase3_getc ();
  if (c != '/')
    return c;
  c = phase3_getc ();
  switch (c)
    {
    default:
      phase3_ungetc (c);
      return '/';

    case '*':
      /* C comment.  */
      comment_start ();
      last_was_star = false;
      for (;;)
        {
          c = phase3_getc ();
          if (c == EOF)
            break;
          /* We skip all leading white space, but not EOLs.  */
          if (!(buflen == 0 && (c == ' ' || c == '\t')))
            comment_add (c);
          switch (c)
            {
            case '\n':
              comment_line_end (1);
              comment_start ();
              last_was_star = false;
              continue;

            case '*':
              last_was_star = true;
              continue;

            case '/':
              if (last_was_star)
                {
                  comment_line_end (2);
                  break;
                }
              FALLTHROUGH;

            default:
              last_was_star = false;
              continue;
            }
          break;
        }
      last_comment_line = newline_count;
      return ' ';

    case '/':
      /* C++ or ISO C 99 comment.  */
      comment_start ();
      for (;;)
        {
          c = phase3_getc ();
          if (c == '\n' || c == EOF)
            break;
          /* We skip all leading white space, but not EOLs.  */
          if (!(buflen == 0 && (c == ' ' || c == '\t')))
            comment_add (c);
        }
      comment_line_end (0);
      last_comment_line = newline_count;
      return '\n';
    }
}


/* Supports only one pushback character.  */
static void
phase4_ungetc (int c)
{
  phase3_ungetc (c);
}


/* ========================== Reading of tokens.  ========================== */


/* True if ObjectiveC extensions are recognized.  */
static bool objc_extensions;

/* True if C++ extensions are recognized.  */
static bool cxx_extensions;

enum token_type_ty
{
  token_type_character_constant,        /* 'x' */
  token_type_eof,
  token_type_eoln,
  token_type_hash,                      /* # */
  token_type_lparen,                    /* ( */
  token_type_rparen,                    /* ) */
  token_type_comma,                     /* , */
  token_type_colon,                     /* : */
  token_type_name,                      /* abc */
  token_type_number,                    /* 2.7 */
  token_type_string_literal,            /* "abc" */
  token_type_symbol,                    /* < > = etc. */
  token_type_objc_special,              /* @ */
  token_type_white_space
};
typedef enum token_type_ty token_type_ty;

typedef struct token_ty token_ty;
struct token_ty
{
  token_type_ty type;
  char *string;                         /* for token_type_name */
  mixed_string_ty *mixed_string;        /* for token_type_string_literal */
  refcounted_string_list_ty *comment;   /* for token_type_string_literal,
                                           token_type_objc_special */
  long number;
  int line_number;
};


/* 7. Replace escape sequences within character strings with their
   single character equivalents.  This is called from phase 5, because
   we don't have to worry about the #include argument.  There are
   pathological cases which could bite us (like the DOS directory
   separator), but just pretend it can't happen.  */

/* Return value of phase7_getc when EOF is reached.  */
#define P7_EOF (-1)

/* Replace escape sequences within character strings with their single
   character equivalents.  */
#define P7_QUOTES (-3)
#define P7_QUOTE (-4)
#define P7_NEWLINE (-5)

/* Convert an UTF-16 or UTF-32 code point to a return value that can be
   distinguished from a single-byte return value.  */
#define UNICODE(code) (0x100 + (code))

/* Test a return value of phase7_getuc whether it designates an UTF-16 or
   UTF-32 code point.  */
#define IS_UNICODE(p7_result) ((p7_result) >= 0x100)

/* Extract the UTF-16 or UTF-32 code of a return value that satisfies
   IS_UNICODE.  */
#define UNICODE_VALUE(p7_result) ((p7_result) - 0x100)


static int
phase7_getc ()
{
  int c, j;

  /* Use phase 3, because phase 4 elides comments.  */
  c = phase3_getc ();

  if (c == EOF)
    return P7_EOF;

  /* Return a magic newline indicator, so that we can distinguish
     between the user requesting a newline in the string (e.g. using
     "\n" or "\012") from the user failing to terminate the string or
     character constant.  The ANSI C standard says: 3.1.3.4 Character
     Constants contain "any character except single quote, backslash or
     newline; or an escape sequence" and 3.1.4 String Literals contain
     "any character except double quote, backslash or newline; or an
     escape sequence".

     Most compilers give a fatal error in this case, however gcc is
     stupidly silent, even though this is a very common typo.  OK, so
     "gcc --pedantic" will tell me, but that gripes about too much other
     stuff.  Could I have a "gcc -Wnewline-in-string" option, or
     better yet a "gcc -fno-newline-in-string" option, please?  Gcc is
     also inconsistent between string literals and character constants:
     you may not embed newlines in character constants; try it, you get
     a useful diagnostic.  --PMiller  */
  if (c == '\n')
    return P7_NEWLINE;

  if (c == '"')
    return P7_QUOTES;
  if (c == '\'')
    return P7_QUOTE;
  if (c != '\\')
    return c;
  c = phase3_getc ();
  switch (c)
    {
    default:
      /* Invalid escape sequences generate a GCC warning, and GCC transforms
         \c to the character c.  So let's do the same.  */
    case '"':
    case '\'':
    case '?':
    case '\\':
      return c;

    case 'a':
      return '\a';
    case 'b':
      return '\b';

      /* The \e escape is peculiar to gcc, and assumes an ASCII
         character set (or superset).  We don't provide support for it
         here.  */

    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    case 'v':
      return '\v';

    case 'x':
      c = phase3_getc ();
      switch (c)
        {
        default:
          phase3_ungetc (c);
          phase3_ungetc ('x');
          return '\\';

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
          break;
        }
      {
        int n;
        bool overflow;

        n = 0;
        overflow = false;

        for (;;)
          {
            switch (c)
              {
              default:
                phase3_ungetc (c);
                if (overflow)
                  {
                    error_with_progname = false;
                    error (0, 0, _("%s:%d: warning: hexadecimal escape sequence out of range"),
                           logical_file_name, line_number);
                    error_with_progname = true;
                  }
                return n;

              case '0': case '1': case '2': case '3': case '4':
              case '5': case '6': case '7': case '8': case '9':
                if (n < 0x100 / 16)
                  n = n * 16 + c - '0';
                else
                  overflow = true;
                break;

              case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                if (n < 0x100 / 16)
                  n = n * 16 + 10 + c - 'A';
                else
                  overflow = true;
                break;

              case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                if (n < 0x100 / 16)
                  n = n * 16 + 10 + c - 'a';
                else
                  overflow = true;
                break;
              }
            c = phase3_getc ();
          }
      }

    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
      {
        int n;

        n = 0;
        for (j = 0; j < 3; ++j)
          {
            n = n * 8 + c - '0';
            c = phase3_getc ();
            switch (c)
              {
              default:
                break;

              case '0': case '1': case '2': case '3':
              case '4': case '5': case '6': case '7':
                continue;
              }
            break;
          }
        phase3_ungetc (c);
        return n;
      }

    case 'U': case 'u':
      {
        unsigned char buf[8];
        int n;

        n = 0;
        for (j = 0; j < (c == 'u' ? 4 : 8); j++)
          {
            int c1 = phase3_getc ();

            if (c1 >= '0' && c1 <= '9')
              n = (n << 4) + (c1 - '0');
            else if (c1 >= 'A' && c1 <= 'F')
              n = (n << 4) + (c1 - 'A' + 10);
            else if (c1 >= 'a' && c1 <= 'f')
              n = (n << 4) + (c1 - 'a' + 10);
            else
              {
                phase3_ungetc (c1);
                while (--j >= 0)
                  phase3_ungetc (buf[j]);
                phase3_ungetc (c);
                return '\\';
              }

            buf[j] = c1;
          }

        if (n < 0x110000)
          return UNICODE (n);

        error_with_progname = false;
        error (0, 0, _("%s:%d: warning: invalid Unicode character"),
               logical_file_name, line_number);
        error_with_progname = true;

        while (--j >= 0)
          phase3_ungetc (buf[j]);
        phase3_ungetc (c);
        return '\\';
      }
    }
}


static void
phase7_ungetc (int c)
{
  phase3_ungetc (c);
}


/* Free the memory pointed to by a 'struct token_ty'.  */
static inline void
free_token (token_ty *tp)
{
  if (tp->type == token_type_name)
    free (tp->string);
  if (tp->type == token_type_string_literal)
    mixed_string_free (tp->mixed_string);
  if (tp->type == token_type_string_literal
      || tp->type == token_type_objc_special)
    drop_reference (tp->comment);
}


/* 5. Parse each resulting logical line as preprocessing tokens and
   white space.  Preprocessing tokens and C tokens don't always match.  */

static token_ty phase5_pushback[1];
static int phase5_pushback_length;


static void
phase5_get (token_ty *tp)
{
  static char *buffer;
  static int bufmax;
  int bufpos;
  int c;

  if (phase5_pushback_length)
    {
      *tp = phase5_pushback[--phase5_pushback_length];
      return;
    }
  tp->string = NULL;
  tp->number = 0;
  tp->line_number = line_number;
  c = phase4_getc ();
  switch (c)
    {
    case EOF:
      tp->type = token_type_eof;
      return;

    case '\n':
      tp->type = token_type_eoln;
      return;

    case ' ':
    case '\f':
    case '\t':
      for (;;)
        {
          c = phase4_getc ();
          switch (c)
            {
            case ' ':
            case '\f':
            case '\t':
              continue;

            default:
              phase4_ungetc (c);
              break;
            }
          break;
        }
      tp->type = token_type_white_space;
      return;

    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '_':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
      bufpos = 0;
      for (;;)
        {
          if (bufpos >= bufmax)
            {
              bufmax = 2 * bufmax + 10;
              buffer = xrealloc (buffer, bufmax);
            }
          buffer[bufpos++] = c;
          c = phase4_getc ();
          switch (c)
            {
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
            case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
            case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
            case 'Y': case 'Z':
            case '_':
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
            case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
            case 's': case 't': case 'u': case 'v': case 'w': case 'x':
            case 'y': case 'z':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
              continue;

            case '"':
              /* Recognize C11 / C++11 string literals.
                 See (for C) ISO 9899:2011 section 6.4.5
                 and (for C++) ISO C++ 11 section 2.14.5 [lex.string].
                 Note: The programmer who passes an UTF-8 encoded string to
                 gettext() or similar API functions will have to have called
                 bind_textdomain_codeset (DOMAIN, "UTF-8") first.  */
              if ((bufpos == 1
                   && (buffer[0] == 'u' || buffer[0] == 'U'
                       || buffer[0] == 'L'))
                  || (bufpos == 2 && buffer[0] == 'u' && buffer[1] == '8'))
                goto string_literal;
              /* Recognize C++11 raw string literals.
                 See ISO C++ 11 section 2.14.5 [lex.string].
                 Here it is important to properly parse all cases according to
                 the standard, otherwise our parser could get confused by
                 double-quotes inside the raw string.
                 Note: The programmer who passes an UTF-8 encoded string to
                 gettext() or similar API functions will have to have called
                 bind_textdomain_codeset (DOMAIN, "UTF-8") first.  */
              if (cxx_extensions
                  && (bufpos == 1
                      || (bufpos == 2
                          && (buffer[0] == 'u' || buffer[0] == 'U'
                              || buffer[0] == 'L'))
                      || (bufpos == 3 && buffer[0] == 'u' && buffer[1] == '8'))
                  && buffer[bufpos - 1] == 'R')
                {
                  /* Only R and u8R raw strings can be used as gettext()
                     arguments, for type reasons.  But the programmer may have
                     defined
                       - a c16gettext function that takes a 'const char16_t *'
                         argument, or
                       - a c32gettext function that takes a 'const char32_t *'
                         argument, or
                       - a wgettext function that takes a 'const wchar_t *'
                         argument.  */
                  int starting_line_number = line_number;
                  bufpos = 0;
                  /* Start the buffer with a closing parenthesis.  This makes the
                     parsing code below simpler.  */
                  buffer[bufpos++] = ')';
                  /* Parse the initial delimiter.  */
                  for (;;)
                    {
                      bool valid_delimiter_char;

                      c = phase3_getc ();
                      switch (c)
                        {
                        case 'A': case 'B': case 'C': case 'D': case 'E':
                        case 'F': case 'G': case 'H': case 'I': case 'J':
                        case 'K': case 'L': case 'M': case 'N': case 'O':
                        case 'P': case 'Q': case 'R': case 'S': case 'T':
                        case 'U': case 'V': case 'W': case 'X': case 'Y':
                        case 'Z':
                        case 'a': case 'b': case 'c': case 'd': case 'e':
                        case 'f': case 'g': case 'h': case 'i': case 'j':
                        case 'k': case 'l': case 'm': case 'n': case 'o':
                        case 'p': case 'q': case 'r': case 's': case 't':
                        case 'u': case 'v': case 'w': case 'x': case 'y':
                        case 'z':
                        case '0': case '1': case '2': case '3': case '4':
                        case '5': case '6': case '7': case '8': case '9':
                        case '_': case '{': case '}': case '[': case ']':
                        case '#': case '<': case '>': case '%': case ':':
                        case ';': case '.': case '?': case '*': case '+':
                        case '-': case '/': case '^': case '&': case '|':
                        case '~': case '!': case '=': case ',': case '\'':
                          valid_delimiter_char = true;
                          break;
                        case '"':
                          /* A double-quote within the delimiter! This is too
                             weird.  We don't support this.  */
                          error_with_progname = false;
                          error (0, 0, _("%s:%d: warning: a double-quote in the delimiter of a raw string literal is unsupported"),
                                 logical_file_name, starting_line_number);
                          error_with_progname = true;
                          FALLTHROUGH;
                        default:
                          valid_delimiter_char = false;
                          break;
                        }
                      if (!valid_delimiter_char)
                        break;

                      if (bufpos >= bufmax)
                        {
                          bufmax = 2 * bufmax + 10;
                          buffer = xrealloc (buffer, bufmax);
                        }
                      buffer[bufpos++] = c;
                    }
                  if (c == '(')
                    {
                      struct mixed_string_buffer msb;
                      /* The state is either 0 or
                         N, after a ')' and N-1 bytes of the delimiter have been
                         encountered.  */
                      int state;

                      /* Start accumulating the string.  */
                      mixed_string_buffer_init (&msb, lc_string,
                                                logical_file_name, line_number);
                      state = 0;

                      for (;;)
                        {
                          c = phase3_getc ();

                          /* Keep line_number in sync.  */
                          msb.line_number = line_number;

                          if (c == EOF)
                            break;

                          /* Update the state.  */
                          if (c == (state < bufpos ? buffer[state] : '"'))
                            {
                              if (state < bufpos)
                                state++;
                              else /* state == bufpos && c == '"' */
                                {
                                  /* Finished parsing the string.  */
                                  tp->type = token_type_string_literal;
                                  tp->mixed_string = mixed_string_buffer_result (&msb);
                                  tp->comment = add_reference (savable_comment);
                                  return;
                                }
                            }
                          else
                            {
                              int i;

                              /* None of the bytes buffer[0]...buffer[state-1]
                                 can be ')'.  */
                              for (i = 0; i < state; i++)
                                mixed_string_buffer_append_char (&msb, buffer[i]);

                              /* But c may be ')'.  */
                              if (c == ')')
                                state = 1;
                              else
                                {
                                  mixed_string_buffer_append_char (&msb, c);
                                  state = 0;
                                }
                            }
                        }
                    }
                  if (c == EOF)
                    {
                      error_with_progname = false;
                      error (0, 0, _("%s:%d: warning: unterminated raw string literal"),
                             logical_file_name, starting_line_number);
                      error_with_progname = true;
                      tp->type = token_type_eof;
                      return;
                    }
                  /* The error message for c == '"' was already emitted above.  */
                  if (c != '"')
                    {
                      error_with_progname = false;
                      error (0, 0, _("%s:%d: warning: invalid raw string literal syntax"),
                             logical_file_name, starting_line_number);
                      error_with_progname = true;
                    }
                  /* To get into a sane state, read up until the next double-quote,
                     newline, or EOF.  */
                  while (!(c == EOF || c == '"' || c == '\n'))
                    c = phase3_getc ();
                  tp->type = token_type_symbol;
                  return;
                }
              FALLTHROUGH;

            default:
              phase4_ungetc (c);
              break;
            }
          break;
        }
      if (bufpos >= bufmax)
        {
          bufmax = 2 * bufmax + 10;
          buffer = xrealloc (buffer, bufmax);
        }
      buffer[bufpos] = 0;
      tp->string = xstrdup (buffer);
      tp->type = token_type_name;
      return;

    case '.':
      c = phase4_getc ();
      phase4_ungetc (c);
      switch (c)
        {
        default:
          tp->type = token_type_symbol;
          return;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          c = '.';
          break;
        }
      FALLTHROUGH;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      /* The preprocessing number token is more "generous" than the C
         number tokens.  This is mostly due to token pasting (another
         thing we can ignore here).  */
      bufpos = 0;
      for (;;)
        {
          if (bufpos >= bufmax)
            {
              bufmax = 2 * bufmax + 10;
              buffer = xrealloc (buffer, bufmax);
            }
          buffer[bufpos++] = c;
          c = phase4_getc ();
          switch (c)
            {
            case 'p':
            case 'P':
              /* In C99 and C++17, 'p' and 'P' can be used as an exponent
                 marker.  */
              FALLTHROUGH;
            case 'e':
            case 'E':
              if (bufpos >= bufmax)
                {
                  bufmax = 2 * bufmax + 10;
                  buffer = xrealloc (buffer, bufmax);
                }
              buffer[bufpos++] = c;
              c = phase4_getc ();
              if (c != '+' && c != '-')
                {
                  phase4_ungetc (c);
                  break;
                }
              continue;

            case 'A': case 'B': case 'C': case 'D':           case 'F':
            case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
            case 'M': case 'N': case 'O':           case 'Q': case 'R':
            case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
            case 'Y': case 'Z':
            case 'a': case 'b': case 'c': case 'd':           case 'f':
            case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
            case 'm': case 'n': case 'o':           case 'q': case 'r':
            case 's': case 't': case 'u': case 'v': case 'w': case 'x':
            case 'y': case 'z':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            case '.':
              continue;

            case '_':
              if (cxx_extensions)
                /* In C++, an underscore can be part of a preprocessing number
                   token.  */
                continue;
              else
                {
                  phase4_ungetc (c);
                  break;
                }

            case '\'':
              if (cxx_extensions)
                {
                  /* In C++14, a single-quote followed by a digit, ASCII letter,
                     or underscore can be part of a preprocessing number token.  */
                  int c1 = phase4_getc ();
                  switch (c1)
                    {
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
                    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
                    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
                    case 'Y': case 'Z':
                    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
                    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
                    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                    case 'y': case 'z':
                    case '_':
                      if (bufpos >= bufmax)
                        {
                          bufmax = 2 * bufmax + 10;
                          buffer = xrealloc (buffer, bufmax);
                        }
                      buffer[bufpos++] = c;
                      c = c1;
                      continue;
                    default:
                      /* The two phase4_getc() calls that returned c and c1 did
                         nothing more than to call phase3_getc(), without any
                         lookahead.  Therefore 2 pushback characters are
                         supported in this case.  */
                      phase4_ungetc (c1);
                      break;
                    }
                }
              else
                {
                  /* In C23, a single-quote between two hexadecimal digits
                     can be part of a number token.  It's called a "digit
                     separator".  See ISO C 23 § 6.4.4.1 and § 6.4.4.2.  */
                  if (bufpos > 0)
                    {
                      char prev = buffer[bufpos - 1];
                      if ((prev >= '0' && prev <= '9')
                          || (prev >= 'A' && prev <= 'F')
                          || (prev >= 'a' && prev <= 'f'))
                        {
                          int c1 = phase4_getc ();
                          if ((c1 >= '0' && c1 <= '9')
                              || (c1 >= 'A' && c1 <= 'F')
                              || (c1 >= 'a' && c1 <= 'f'))
                            {
                              if (bufpos >= bufmax)
                                {
                                  bufmax = 2 * bufmax + 10;
                                  buffer = xrealloc (buffer, bufmax);
                                }
                              buffer[bufpos++] = c;
                              c = c1;
                              continue;
                            }
                          /* The two phase4_getc() calls that returned c and c1
                             did nothing more than to call phase3_getc(),
                             without any lookahead.  Therefore 2 pushback
                             characters are supported in this case.  */
                          phase4_ungetc (c1);
                        }
                    }
                }
              FALLTHROUGH;
            default:
              phase4_ungetc (c);
              break;
            }
          break;
        }
      if (bufpos >= bufmax)
        {
          bufmax = 2 * bufmax + 10;
          buffer = xrealloc (buffer, bufmax);
        }
      buffer[bufpos] = 0;
      tp->type = token_type_number;
      tp->number = atol (buffer);
      return;

    case '\'':
      /* We could worry about the 'L' before wide character constants,
         but ignoring it has no effect unless one of the keywords is
         "L".  Just pretend it won't happen.  Also, we don't need to
         remember the character constant.  */
      for (;;)
        {
          c = phase7_getc ();
          if (c == P7_NEWLINE)
            {
              error_with_progname = false;
              error (0, 0, _("%s:%d: warning: unterminated character constant"),
                     logical_file_name, line_number - 1);
              error_with_progname = true;
              phase7_ungetc ('\n');
              break;
            }
          if (c == P7_EOF || c == P7_QUOTE)
            break;
        }
      tp->type = token_type_character_constant;
      return;

    case '"':
    string_literal:
      /* We could worry about the 'L' or 'u' or 'U' before wide string
         constants, but since gettext's argument is a 'const char *', not
         a 'const wchar_t *' (for 'L') nor a 'const char16_t *' (for 'u')
         nor a 'const char32_t *' (for 'U'), the compiler would complain
         about the argument not matching the prototype.  Just pretend it
         won't happen.  */
      {
        struct mixed_string_buffer msb;

        /* Start accumulating the string.  */
        mixed_string_buffer_init (&msb, lc_string,
                                  logical_file_name, line_number);

        for (;;)
          {
            c = phase7_getc ();

            /* Keep line_number in sync.  */
            msb.line_number = line_number;

            if (c == P7_NEWLINE)
              {
                error_with_progname = false;
                error (0, 0, _("%s:%d: warning: unterminated string literal"),
                       logical_file_name, line_number - 1);
                error_with_progname = true;
                phase7_ungetc ('\n');
                break;
              }
            if (c == P7_EOF || c == P7_QUOTES)
              break;
            if (c == P7_QUOTE)
              c = '\'';
            if (IS_UNICODE (c))
              {
                assert (UNICODE_VALUE (c) >= 0
                        && UNICODE_VALUE (c) < 0x110000);
                mixed_string_buffer_append_unicode (&msb, UNICODE_VALUE (c));
              }
            else
              mixed_string_buffer_append_char (&msb, c);
          }
        tp->type = token_type_string_literal;
        tp->mixed_string = mixed_string_buffer_result (&msb);
        tp->comment = add_reference (savable_comment);
        return;
      }

    case '(':
      tp->type = token_type_lparen;
      return;

    case ')':
      tp->type = token_type_rparen;
      return;

    case ',':
      tp->type = token_type_comma;
      return;

    case '#':
      tp->type = token_type_hash;
      return;

    case ':':
      tp->type = token_type_colon;
      return;

    case '@':
      if (objc_extensions)
        {
          tp->type = token_type_objc_special;
          tp->comment = add_reference (savable_comment);
          return;
        }
      FALLTHROUGH;

    default:
      /* We could carefully recognize each of the 2 and 3 character
         operators, but it is not necessary, as we only need to recognize
         gettext invocations.  Don't bother.  */
      tp->type = token_type_symbol;
      return;
    }
}


/* Supports only one pushback token.  */
static void
phase5_unget (token_ty *tp)
{
  if (tp->type != token_type_eof)
    {
      if (phase5_pushback_length == SIZEOF (phase5_pushback))
        abort ();
      phase5_pushback[phase5_pushback_length++] = *tp;
    }
}


/* X. Recognize a leading # symbol.  Leave leading hash as a hash, but
   turn hash in the middle of a line into a plain symbol token.  This
   makes the phase 6 easier.  */

static void
phaseX_get (token_ty *tp)
{
  static bool middle;   /* false at the beginning of a line, true otherwise.  */

  phase5_get (tp);

  if (tp->type == token_type_eoln || tp->type == token_type_eof)
    middle = false;
  else
    {
      if (middle)
        {
          /* Turn hash in the middle of a line into a plain symbol token.  */
          if (tp->type == token_type_hash)
            tp->type = token_type_symbol;
        }
      else
        {
          /* When we see leading whitespace followed by a hash sign,
             discard the leading white space token.  The hash is all
             phase 6 is interested in.  */
          if (tp->type == token_type_white_space)
            {
              token_ty next;

              phase5_get (&next);
              if (next.type == token_type_hash)
                *tp = next;
              else
                phase5_unget (&next);
            }
          middle = true;
        }
    }
}


/* 6. Recognize and carry out directives (it also expands macros on
   non-directive lines, which we do not do here).  The only directive
   we care about are the #line and #define directive.  We throw all the
   others away.  */

static token_ty phase6_pushback[2];
static int phase6_pushback_length;


static void
phase6_get (token_ty *tp)
{
  static token_ty *buf;
  static int bufmax;
  int bufpos;
  int j;

  if (phase6_pushback_length)
    {
      *tp = phase6_pushback[--phase6_pushback_length];
      return;
    }
  for (;;)
    {
      /* Get the next token.  If it is not a '#' at the beginning of a
         line (ignoring whitespace), return immediately.  */
      phaseX_get (tp);
      if (tp->type != token_type_hash)
        return;

      /* Accumulate the rest of the directive in a buffer, until the
         "define" keyword is seen or until end of line.  */
      bufpos = 0;
      for (;;)
        {
          phaseX_get (tp);
          if (tp->type == token_type_eoln || tp->type == token_type_eof)
            break;

          /* Before the "define" keyword and inside other directives
             white space is irrelevant.  So just throw it away.  */
          if (tp->type != token_type_white_space)
            {
              /* If it is a #define directive, return immediately,
                 thus treating the body of the #define directive like
                 normal input.  */
              if (bufpos == 0
                  && tp->type == token_type_name
                  && strcmp (tp->string, "define") == 0)
                return;

              /* Accumulate.  */
              if (bufpos >= bufmax)
                {
                  bufmax = 2 * bufmax + 10;
                  buf = xrealloc (buf, bufmax * sizeof (buf[0]));
                }
              buf[bufpos++] = *tp;
            }
        }

      /* If it is a #line directive, with no macros to expand, act on
         it.  Ignore all other directives.  */
      if (bufpos >= 3 && buf[0].type == token_type_name
          && strcmp (buf[0].string, "line") == 0
          && buf[1].type == token_type_number
          && buf[2].type == token_type_string_literal)
        {
          logical_file_name = mixed_string_contents (buf[2].mixed_string);
          line_number = buf[1].number;
        }
      if (bufpos >= 2 && buf[0].type == token_type_number
          && buf[1].type == token_type_string_literal)
        {
          logical_file_name = mixed_string_contents (buf[1].mixed_string);
          line_number = buf[0].number;
        }

      /* Release the storage held by the directive.  */
      for (j = 0; j < bufpos; ++j)
        free_token (&buf[j]);

      /* We must reset the selected comments.  */
      savable_comment_reset ();
    }
}


/* Supports 2 tokens of pushback.  */
static void
phase6_unget (token_ty *tp)
{
  if (tp->type != token_type_eof)
    {
      if (phase6_pushback_length == SIZEOF (phase6_pushback))
        abort ();
      phase6_pushback[phase6_pushback_length++] = *tp;
    }
}


/* 8a. Convert ISO C 99 section 7.8.1 format string directives to string
   literal placeholders.  */

/* Test for an ISO C 99 section 7.8.1 format string directive.  */
static bool
is_inttypes_macro (const char *name)
{
  /* Syntax:
     P R I { d | i | o | u | x | X }
     { { | LEAST | FAST } { 8 | 16 | 32 | 64 } | MAX | PTR }  */
  if (name[0] == 'P' && name[1] == 'R' && name[2] == 'I')
    {
      name += 3;
      if (name[0] == 'd' || name[0] == 'i' || name[0] == 'o' || name[0] == 'u'
          || name[0] == 'x' || name[0] == 'X')
        {
          name += 1;
          if (name[0] == 'M' && name[1] == 'A' && name[2] == 'X'
              && name[3] == '\0')
            return true;
          if (name[0] == 'P' && name[1] == 'T' && name[2] == 'R'
              && name[3] == '\0')
            return true;
          if (name[0] == 'L' && name[1] == 'E' && name[2] == 'A'
              && name[3] == 'S' && name[4] == 'T')
            name += 5;
          else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'S'
                   && name[3] == 'T')
            name += 4;
          if (name[0] == '8' && name[1] == '\0')
            return true;
          if (name[0] == '1' && name[1] == '6' && name[2] == '\0')
            return true;
          if (name[0] == '3' && name[1] == '2' && name[2] == '\0')
            return true;
          if (name[0] == '6' && name[1] == '4' && name[2] == '\0')
            return true;
        }
    }
  return false;
}

static void
phase8a_get (token_ty *tp)
{
  phase6_get (tp);
  if (tp->type == token_type_name && is_inttypes_macro (tp->string))
    {
      /* Turn PRIdXXX into "<PRIdXXX>".  */
      char *new_string = xasprintf ("<%s>", tp->string);
      free (tp->string);
      tp->mixed_string =
        mixed_string_alloc_utf8 (new_string, lc_string,
                                 logical_file_name, line_number);
      tp->comment = add_reference (savable_comment);
      tp->type = token_type_string_literal;
    }
}

/* Supports 2 tokens of pushback.  */
static inline void
phase8a_unget (token_ty *tp)
{
  phase6_unget (tp);
}


/* 8b. Drop whitespace.  */
static void
phase8b_get (token_ty *tp)
{
  for (;;)
    {
      phase8a_get (tp);

      if (tp->type == token_type_white_space)
        continue;
      if (tp->type == token_type_eoln)
        {
          /* We have to track the last occurrence of a string.  One
             mode of xgettext allows to group an extracted message
             with a comment for documentation.  The rule which states
             which comment is assumed to be grouped with the message
             says it should immediately precede it.  Our
             interpretation: between the last line of the comment and
             the line in which the keyword is found must be no line
             with non-white space tokens.  */
          ++newline_count;
          if (last_non_comment_line > last_comment_line)
            savable_comment_reset ();
          continue;
        }
      break;
    }
}

/* Supports 2 tokens of pushback.  */
static inline void
phase8b_unget (token_ty *tp)
{
  phase8a_unget (tp);
}


/* 8c. In ObjectiveC mode, drop '@' before a literal string.  We need to
   do this before performing concatenation of adjacent string literals.  */
static void
phase8c_get (token_ty *tp)
{
  token_ty tmp;

  phase8b_get (tp);
  if (tp->type != token_type_objc_special)
    return;
  phase8b_get (&tmp);
  if (tmp.type != token_type_string_literal)
    {
      phase8b_unget (&tmp);
      return;
    }
  /* Drop the '@' token and return immediately the following string.  */
  drop_reference (tmp.comment);
  tmp.comment = tp->comment;
  *tp = tmp;
}

/* Supports only one pushback token.  */
static inline void
phase8c_unget (token_ty *tp)
{
  phase8b_unget (tp);
}


/* 8. Concatenate adjacent string literals to form single string
   literals (because we don't expand macros, there are a few things we
   will miss).  */

static void
phase8_get (token_ty *tp)
{
  phase8c_get (tp);
  if (tp->type != token_type_string_literal)
    return;
  for (;;)
    {
      token_ty tmp;

      phase8c_get (&tmp);
      if (tmp.type != token_type_string_literal)
        {
          phase8c_unget (&tmp);
          return;
        }
      tp->mixed_string =
        mixed_string_concat_free1 (tp->mixed_string, tmp.mixed_string);
      free_token (&tmp);
    }
}


/* ===================== Reading of high-level tokens.  ==================== */


enum xgettext_token_type_ty
{
  xgettext_token_type_eof,
  xgettext_token_type_keyword,
  xgettext_token_type_symbol,
  xgettext_token_type_lparen,
  xgettext_token_type_rparen,
  xgettext_token_type_comma,
  xgettext_token_type_colon,
  xgettext_token_type_string_literal,
  xgettext_token_type_other
};
typedef enum xgettext_token_type_ty xgettext_token_type_ty;

typedef struct xgettext_token_ty xgettext_token_ty;
struct xgettext_token_ty
{
  xgettext_token_type_ty type;

  /* This field is used only for xgettext_token_type_keyword.  */
  const struct callshapes *shapes;

  /* This field is used only for xgettext_token_type_keyword,
     xgettext_token_type_symbol.  */
  char *string;

  /* This field is used only for xgettext_token_type_string_literal.  */
  mixed_string_ty *mixed_string;

  /* This field is used only for xgettext_token_type_string_literal.  */
  refcounted_string_list_ty *comment;

  /* This field is used only for xgettext_token_type_keyword,
     xgettext_token_type_string_literal.  */
  lex_pos_ty pos;
};


/* 9. Convert the remaining preprocessing tokens to C tokens and
   discards any white space from the translation unit.  */

static void
x_c_lex (xgettext_token_ty *tp)
{
  for (;;)
    {
      token_ty token;
      void *keyword_value;

      phase8_get (&token);
      switch (token.type)
        {
        case token_type_eof:
          tp->type = xgettext_token_type_eof;
          return;

        case token_type_name:
          last_non_comment_line = newline_count;

          if (hash_find_entry (objc_extensions ? &objc_keywords : &c_keywords,
                               token.string, strlen (token.string),
                               &keyword_value)
              == 0)
            {
              tp->type = xgettext_token_type_keyword;
              tp->shapes = (const struct callshapes *) keyword_value;
              tp->pos.file_name = logical_file_name;
              tp->pos.line_number = token.line_number;
            }
          else
            tp->type = xgettext_token_type_symbol;
          tp->string = token.string;
          return;

        case token_type_lparen:
          last_non_comment_line = newline_count;

          tp->type = xgettext_token_type_lparen;
          return;

        case token_type_rparen:
          last_non_comment_line = newline_count;

          tp->type = xgettext_token_type_rparen;
          return;

        case token_type_comma:
          last_non_comment_line = newline_count;

          tp->type = xgettext_token_type_comma;
          return;

        case token_type_colon:
          last_non_comment_line = newline_count;

          tp->type = xgettext_token_type_colon;
          return;

        case token_type_string_literal:
          last_non_comment_line = newline_count;

          tp->type = xgettext_token_type_string_literal;
          tp->mixed_string = token.mixed_string;
          tp->comment = token.comment;
          tp->pos.file_name = logical_file_name;
          tp->pos.line_number = token.line_number;
          return;

        case token_type_objc_special:
          drop_reference (token.comment);
          FALLTHROUGH;

        default:
          last_non_comment_line = newline_count;

          tp->type = xgettext_token_type_other;
          return;
        }
    }
}


/* ========================= Extracting strings.  ========================== */


/* Context lookup table.  */
static flag_context_list_table_ty *flag_context_list_table;


/* Maximum supported nesting depth.
   ISO C 23 § 5.2.4.1.(1) requires 63 "nesting levels of parenthesized
   expressions within a full expression"; then 1000 is more than enough.  */
#define MAX_NESTING_DEPTH 1000

/* Current nesting depth.  */
static int nesting_depth;


/* The file is broken into tokens.  Scan the token stream, looking for
   a keyword, followed by a left paren, followed by a string.  When we
   see this sequence, we have something to remember.  We assume we are
   looking at a valid C or C++ program, and leave the complaints about
   the grammar to the compiler.

     Normal handling: Look for
       keyword ( ... msgid ... )
     Plural handling: Look for
       keyword ( ... msgid ... msgid_plural ... )

   We use recursion because the arguments before msgid or between msgid
   and msgid_plural can contain subexpressions of the same form.  */


/* Extract messages until the next balanced closing parenthesis.
   Extracted messages are added to MLP.
   Return true upon eof, false upon closing parenthesis.  */
static bool
extract_parenthesized (message_list_ty *mlp,
                       flag_context_ty outer_context,
                       flag_context_list_iterator_ty context_iter,
                       struct arglist_parser *argparser)
{
  /* Current argument number.  */
  int arg = 1;
  /* 0 when no keyword has been seen.  1 right after a keyword is seen.  */
  int state;
  /* Parameters of the keyword just seen.  Defined only in state 1.  */
  const struct callshapes *next_shapes = NULL;
  /* Context iterator that will be used if the next token is a '('.  */
  flag_context_list_iterator_ty next_context_iter =
    passthrough_context_list_iterator;
  /* Context iterator that will be used if the next token is a ':'.
     (Objective C selector syntax.)  */
  flag_context_list_iterator_ty selectorcall_context_iter =
    passthrough_context_list_iterator;
  /* Current context.  */
  flag_context_ty inner_context =
    inherited_context (outer_context,
                       flag_context_list_iterator_advance (&context_iter));

  /* Start state is 0.  */
  state = 0;

  for (;;)
    {
      xgettext_token_ty token;

      x_c_lex (&token);
      switch (token.type)
        {
        case xgettext_token_type_keyword:
          next_shapes = token.shapes;
          state = 1;
          goto keyword_or_symbol;

        case xgettext_token_type_symbol:
          state = 0;
        keyword_or_symbol:
          next_context_iter =
            flag_context_list_iterator (
              flag_context_list_table_lookup (
                flag_context_list_table,
                token.string, strlen (token.string)));
          if (objc_extensions)
            {
              size_t token_string_len = strlen (token.string);
              token.string = xrealloc (token.string, token_string_len + 2);
              token.string[token_string_len] = ':';
              token.string[token_string_len + 1] = '\0';
              selectorcall_context_iter =
                flag_context_list_iterator (
                  flag_context_list_table_lookup (
                    flag_context_list_table,
                    token.string, token_string_len + 1));
            }
          free (token.string);
          continue;

        case xgettext_token_type_lparen:
          if (++nesting_depth > MAX_NESTING_DEPTH)
            {
              error_with_progname = false;
              error (EXIT_FAILURE, 0, _("%s:%d: error: too many open parentheses"),
                     logical_file_name, line_number);
            }
          if (extract_parenthesized (mlp, inner_context, next_context_iter,
                                     arglist_parser_alloc (mlp,
                                                           state ? next_shapes : NULL)))
            {
              arglist_parser_done (argparser, arg);
              return true;
            }
          nesting_depth--;
          next_context_iter = null_context_list_iterator;
          selectorcall_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case xgettext_token_type_rparen:
          arglist_parser_done (argparser, arg);
          return false;

        case xgettext_token_type_comma:
          arg++;
          inner_context =
            inherited_context (outer_context,
                               flag_context_list_iterator_advance (
                                 &context_iter));
          next_context_iter = passthrough_context_list_iterator;
          selectorcall_context_iter = passthrough_context_list_iterator;
          state = 0;
          continue;

        case xgettext_token_type_colon:
          if (objc_extensions)
            {
              context_iter = selectorcall_context_iter;
              inner_context =
                inherited_context (inner_context,
                                   flag_context_list_iterator_advance (
                                     &context_iter));
              next_context_iter = passthrough_context_list_iterator;
              selectorcall_context_iter = passthrough_context_list_iterator;
            }
          else
            {
              next_context_iter = null_context_list_iterator;
              selectorcall_context_iter = null_context_list_iterator;
            }
          state = 0;
          continue;

        case xgettext_token_type_string_literal:
          {
            if (extract_all)
              {
                char *string = mixed_string_contents (token.mixed_string);
                mixed_string_free (token.mixed_string);
                remember_a_message (mlp, NULL, string, true, false,
                                    inner_context, &token.pos,
                                    NULL, token.comment, false);
              }
            else
              arglist_parser_remember (argparser, arg, token.mixed_string,
                                       inner_context,
                                       token.pos.file_name,
                                       token.pos.line_number,
                                       token.comment, false);
            drop_reference (token.comment);
          }
          next_context_iter = null_context_list_iterator;
          selectorcall_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case xgettext_token_type_other:
          next_context_iter = null_context_list_iterator;
          selectorcall_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case xgettext_token_type_eof:
          arglist_parser_done (argparser, arg);
          return true;

        default:
          abort ();
        }
    }
}


static void
extract_whole_file (FILE *f,
                    const char *real_filename, const char *logical_filename,
                    flag_context_list_table_ty *flag_table,
                    msgdomain_list_ty *mdlp)
{
  message_list_ty *mlp = mdlp->item[0]->messages;

  fp = f;
  real_file_name = real_filename;
  logical_file_name = xstrdup (logical_filename);
  line_number = 1;

  phase1_pushback_length = 0;
  phase2_pushback_length = 0;
  phase3_pushback_length = 0;

  last_comment_line = -1;
  last_non_comment_line = -1;
  newline_count = 0;

  phase5_pushback_length = 0;
  phase6_pushback_length = 0;

  flag_context_list_table = flag_table;
  nesting_depth = 0;

  init_keywords ();

  /* Eat tokens until eof is seen.  When extract_parenthesized returns
     due to an unbalanced closing parenthesis, just restart it.  */
  while (!extract_parenthesized (mlp, null_context, null_context_list_iterator,
                                 arglist_parser_alloc (mlp, NULL)))
    ;

  /* Close scanner.  */
  fp = NULL;
  real_file_name = NULL;
  logical_file_name = NULL;
  line_number = 0;
}


void
extract_c (FILE *f,
           const char *real_filename, const char *logical_filename,
           flag_context_list_table_ty *flag_table,
           msgdomain_list_ty *mdlp)
{
  objc_extensions = false;
  cxx_extensions = false;
  extract_whole_file (f, real_filename, logical_filename, flag_table, mdlp);
}

void
extract_cxx (FILE *f,
             const char *real_filename, const char *logical_filename,
             flag_context_list_table_ty *flag_table,
             msgdomain_list_ty *mdlp)
{
  objc_extensions = false;
  cxx_extensions = true;
  extract_whole_file (f, real_filename, logical_filename, flag_table, mdlp);
}

void
extract_objc (FILE *f,
              const char *real_filename, const char *logical_filename,
              flag_context_list_table_ty *flag_table,
              msgdomain_list_ty *mdlp)
{
  objc_extensions = true;
  cxx_extensions = false;
  extract_whole_file (f, real_filename, logical_filename, flag_table, mdlp);
}

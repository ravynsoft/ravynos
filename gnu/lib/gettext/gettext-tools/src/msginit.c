/* Initializes a new PO file.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#if HAVE_PWD_H
# include <pwd.h>
#endif

#include <unistd.h>

#if HAVE_DIRENT_H
# include <dirent.h>
#endif

#if HAVE_DIRENT_H
# define HAVE_DIR 1
#else
# define HAVE_DIR 0
#endif

#include <textstyle.h>

/* Get BINDIR.  */
#include "configmake.h"

#include "noreturn.h"
#include "closeout.h"
#include "error.h"
#include "error-progname.h"
#include "progname.h"
#include "relocatable.h"
#include "basename-lgpl.h"
#include "c-strstr.h"
#include "c-strcase.h"
#include "message.h"
#include "read-catalog.h"
#include "read-po.h"
#include "read-properties.h"
#include "read-stringtable.h"
#include "write-catalog.h"
#include "write-po.h"
#include "write-properties.h"
#include "write-stringtable.h"
#include "po-charset.h"
#include "localcharset.h"
#include "localename.h"
#include "po-time.h"
#include "plural-table.h"
#include "lang-table.h"
#include "xalloc.h"
#include "xmalloca.h"
#include "concat-filename.h"
#include "xerror.h"
#include "xvasprintf.h"
#include "msgl-english.h"
#include "plural-count.h"
#include "spawn-pipe.h"
#include "wait-process.h"
#include "xsetenv.h"
#include "str-list.h"
#include "propername.h"
#include "gettext.h"

#define _(str) gettext (str)
#define N_(str) (str)

/* Get F_OK.  It is lacking from <fcntl.h> on Woe32.  */
#ifndef F_OK
# define F_OK 0
#endif

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))

extern const char * _nl_expand_alias (const char *name);

/* Locale name.  */
static const char *locale;

/* Language (ISO-639 code) and optional territory (ISO-3166 code).  */
static const char *catalogname;

/* Language (ISO-639 code).  */
static const char *language;

/* If true, the user is not considered to be the translator.  */
static bool no_translator;

/* Long options.  */
static const struct option long_options[] =
{
  { "color", optional_argument, NULL, CHAR_MAX + 5 },
  { "help", no_argument, NULL, 'h' },
  { "input", required_argument, NULL, 'i' },
  { "locale", required_argument, NULL, 'l' },
  { "no-translator", no_argument, NULL, CHAR_MAX + 1 },
  { "no-wrap", no_argument, NULL, CHAR_MAX + 2 },
  { "output-file", required_argument, NULL, 'o' },
  { "properties-input", no_argument, NULL, 'P' },
  { "properties-output", no_argument, NULL, 'p' },
  { "stringtable-input", no_argument, NULL, CHAR_MAX + 3 },
  { "stringtable-output", no_argument, NULL, CHAR_MAX + 4 },
  { "style", required_argument, NULL, CHAR_MAX + 6 },
  { "version", no_argument, NULL, 'V' },
  { "width", required_argument, NULL, 'w' },
  { NULL, 0, NULL, 0 }
};

/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void usage (int status);
static const char *find_pot (void);
static const char *catalogname_for_locale (const char *locale);
static const char *language_of_locale (const char *locale);
static char *get_field (const char *header, const char *field);
static msgdomain_list_ty *fill_header (msgdomain_list_ty *mdlp);
static msgdomain_list_ty *update_msgstr_plurals (msgdomain_list_ty *mdlp);


int
main (int argc, char **argv)
{
  int opt;
  bool do_help;
  bool do_version;
  char *output_file;
  const char *input_file;
  msgdomain_list_ty *result;
  catalog_input_format_ty input_syntax = &input_format_po;
  catalog_output_format_ty output_syntax = &output_format_po;

  /* Set program name for messages.  */
  set_program_name (argv[0]);
  error_print_progname = maybe_print_progname;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, relocate (LOCALEDIR));
  bindtextdomain ("bison-runtime", relocate (BISON_LOCALEDIR));
  textdomain (PACKAGE);

  /* Ensure that write errors on stdout are detected.  */
  atexit (close_stdout);

  /* Set default values for variables.  */
  do_help = false;
  do_version = false;
  output_file = NULL;
  input_file = NULL;
  locale = NULL;

  while ((opt = getopt_long (argc, argv, "hi:l:o:pPVw:", long_options, NULL))
         != EOF)
    switch (opt)
      {
      case '\0':                /* Long option.  */
        break;

      case 'h':
        do_help = true;
        break;

      case 'i':
        if (input_file != NULL)
          {
            error (EXIT_SUCCESS, 0, _("at most one input file allowed"));
            usage (EXIT_FAILURE);
          }
        input_file = optarg;
        break;

      case 'l':
        locale = optarg;
        break;

      case 'o':
        output_file = optarg;
        break;

      case 'p':
        output_syntax = &output_format_properties;
        break;

      case 'P':
        input_syntax = &input_format_properties;
        break;

      case 'V':
        do_version = true;
        break;

      case 'w':
        {
          int value;
          char *endp;
          value = strtol (optarg, &endp, 10);
          if (endp != optarg)
            message_page_width_set (value);
        }
        break;

      case CHAR_MAX + 1:
        no_translator = true;
        break;

      case CHAR_MAX + 2: /* --no-wrap */
        message_page_width_ignore ();
        break;

      case CHAR_MAX + 3: /* --stringtable-input */
        input_syntax = &input_format_stringtable;
        break;

      case CHAR_MAX + 4: /* --stringtable-output */
        output_syntax = &output_format_stringtable;
        break;

      case CHAR_MAX + 5: /* --color */
        if (handle_color_option (optarg) || color_test_mode)
          usage (EXIT_FAILURE);
        break;

      case CHAR_MAX + 6: /* --style */
        handle_style_option (optarg);
        break;

      default:
        usage (EXIT_FAILURE);
        break;
      }

  /* Version information is requested.  */
  if (do_version)
    {
      printf ("%s (GNU %s) %s\n", last_component (program_name),
              PACKAGE, VERSION);
      /* xgettext: no-wrap */
      printf (_("Copyright (C) %s Free Software Foundation, Inc.\n\
License GPLv3+: GNU GPL version 3 or later <%s>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
"),
              "2001-2023", "https://gnu.org/licenses/gpl.html");
      printf (_("Written by %s.\n"), proper_name ("Bruno Haible"));
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* Test for extraneous arguments.  */
  if (optind != argc)
    error (EXIT_FAILURE, 0, _("too many arguments"));

  /* Search for the input file.  */
  if (input_file == NULL)
    input_file = find_pot ();

  /* Determine target locale.  */
  if (locale == NULL)
    {
      locale = gl_locale_name (LC_MESSAGES, "LC_MESSAGES");
      if (strcmp (locale, "C") == 0)
        {
          const char *doc_url =
            "https://www.gnu.org/software/gettext/manual/html_node/Setting-the-POSIX-Locale.html";
          multiline_error (xstrdup (""),
                           xasprintf (_("\
You are in a language indifferent environment.  Please set\n\
your LANG environment variable, as described in\n\
<%s>.\n\
This is necessary so you can test your translations.\n"),
                                      doc_url));
          exit (EXIT_FAILURE);
        }
    }
  {
    const char *alias = _nl_expand_alias (locale);
    if (alias != NULL)
      locale = alias;
  }
  catalogname = catalogname_for_locale (locale);
  language = language_of_locale (locale);

  /* Default output file name is CATALOGNAME.po.  */
  if (output_file == NULL)
    {
      output_file = xasprintf ("%s.po", catalogname);

      /* But don't overwrite existing PO files.  */
      if (access (output_file, F_OK) == 0)
        {
          multiline_error (xstrdup (""),
                           xasprintf (_("\
Output file %s already exists.\n\
Please specify the locale through the --locale option or\n\
the output .po file through the --output-file option.\n"),
                                      output_file));
          exit (EXIT_FAILURE);
        }
    }

  /* Read input file.  */
  result = read_catalog_file (input_file, input_syntax);

#if defined _WIN32 || defined __CYGWIN__
  /* The function fill_header invokes, directly or indirectly, some programs
     that are installed in ${libdir}/gettext:
       - hostname, invoked indirectly through 'user-email'.
       - urlget, invoked indirectly through 'team-address'.
       - cldr-plurals, invoked directly.
     These programs depend on libintl.  In installations with shared libraries,
     we need to guarantee that the programs find the DLL, which is installed
     in ${bindir}, not in ${libdir}/gettext.  The preferred way to do so is to
     extend $PATH, so that it contains ${bindir}.  */
  {
    const char *orig_path;
    size_t orig_path_len;
    char separator;
    const char *bindir;
    size_t bindir_len;
    char *augmented_path;

    orig_path = getenv ("PATH");
    if (orig_path == NULL)
      orig_path = "";
    orig_path_len = strlen (orig_path);

    #if defined __CYGWIN__
    separator = ':';
    #else /* native Windows */
    separator = ';';
    #endif

    bindir = BINDIR;
    bindir_len = strlen (bindir);

    /* Concatenate bindir, separator, orig_path.  */
    augmented_path = XNMALLOC (bindir_len + 1 + orig_path_len + 1, char);
    memcpy (augmented_path, bindir, bindir_len);
    augmented_path[bindir_len] = separator;
    memcpy (augmented_path + bindir_len + 1, orig_path, orig_path_len + 1);

    xsetenv ("PATH", augmented_path, 1);
  }
#endif

  /* Fill the header entry.  */
  result = fill_header (result);

  /* Initialize translations.  */
  if (strcmp (language, "en") == 0)
    result = msgdomain_list_english (result);
  else
    result = update_msgstr_plurals (result);

  /* Write the modified message list out.  */
  msgdomain_list_print (result, output_file, output_syntax, true, false);

  if (!no_translator)
    fprintf (stderr, "\n");
  fprintf (stderr, _("Created %s.\n"), output_file);

  exit (EXIT_SUCCESS);
}


/* Display usage information and exit.  */
static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try '%s --help' for more information.\n"),
             program_name);
  else
    {
      printf (_("\
Usage: %s [OPTION]\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Creates a new PO file, initializing the meta information with values from the\n\
user's environment.\n\
"));
      printf ("\n");
      printf (_("\
Mandatory arguments to long options are mandatory for short options too.\n"));
      printf ("\n");
      printf (_("\
Input file location:\n"));
      printf (_("\
  -i, --input=INPUTFILE       input POT file\n"));
      printf (_("\
If no input file is given, the current directory is searched for the POT file.\n\
If it is -, standard input is read.\n"));
      printf ("\n");
      printf (_("\
Output file location:\n"));
      printf (_("\
  -o, --output-file=FILE      write output to specified PO file\n"));
      printf (_("\
If no output file is given, it depends on the --locale option or the user's\n\
locale setting.  If it is -, the results are written to standard output.\n"));
      printf ("\n");
      printf (_("\
Input file syntax:\n"));
      printf (_("\
  -P, --properties-input      input file is in Java .properties syntax\n"));
      printf (_("\
      --stringtable-input     input file is in NeXTstep/GNUstep .strings syntax\n"));
      printf ("\n");
      printf (_("\
Output details:\n"));
      printf (_("\
  -l, --locale=LL_CC[.ENCODING]  set target locale\n"));
      printf (_("\
      --no-translator         assume the PO file is automatically generated\n"));
      printf (_("\
      --color                 use colors and other text attributes always\n\
      --color=WHEN            use colors and other text attributes if WHEN.\n\
                              WHEN may be 'always', 'never', 'auto', or 'html'.\n"));
      printf (_("\
      --style=STYLEFILE       specify CSS style rule file for --color\n"));
      printf (_("\
  -p, --properties-output     write out a Java .properties file\n"));
      printf (_("\
      --stringtable-output    write out a NeXTstep/GNUstep .strings file\n"));
      printf (_("\
  -w, --width=NUMBER          set output page width\n"));
      printf (_("\
      --no-wrap               do not break long message lines, longer than\n\
                              the output page width, into several lines\n"));
      printf ("\n");
      printf (_("\
Informative output:\n"));
      printf (_("\
  -h, --help                  display this help and exit\n"));
      printf (_("\
  -V, --version               output version information and exit\n"));
      printf ("\n");
      /* TRANSLATORS: The first placeholder is the web address of the Savannah
         project of this package.  The second placeholder is the bug-reporting
         email address for this package.  Please add _another line_ saying
         "Report translation bugs to <...>\n" with the address for translation
         bugs (typically your translation team's web or email address).  */
      printf(_("\
Report bugs in the bug tracker at <%s>\n\
or by email to <%s>.\n"),
             "https://savannah.gnu.org/projects/gettext",
             "bug-gettext@gnu.org");
    }

  exit (status);
}


/* Search for the POT file and return its name.  */
static const char *
find_pot ()
{
#if HAVE_DIR
  DIR *dirp;
  char *found = NULL;

  dirp = opendir (".");
  if (dirp != NULL)
    {
      for (;;)
        {
          struct dirent *dp;

          errno = 0;
          dp = readdir (dirp);
          if (dp != NULL)
            {
              const char *name = dp->d_name;
              size_t namlen = strlen (name);

              if (namlen > 4 && memcmp (name + namlen - 4, ".pot", 4) == 0)
                {
                  if (found == NULL)
                    found = xstrdup (name);
                  else
                    {
                      multiline_error (xstrdup (""),
                                       xstrdup (_("\
Found more than one .pot file.\n\
Please specify the input .pot file through the --input option.\n")));
                      usage (EXIT_FAILURE);
                    }
                }
            }
          else if (errno != 0)
            error (EXIT_FAILURE, errno, _("error reading current directory"));
          else
            break;
        }
      if (closedir (dirp))
        error (EXIT_FAILURE, errno, _("error reading current directory"));

      if (found != NULL)
        return found;
    }
#endif

  multiline_error (xstrdup (""),
                   xstrdup (_("\
Found no .pot file in the current directory.\n\
Please specify the input .pot file through the --input option.\n")));
  usage (EXIT_FAILURE);
  /* NOTREACHED */
  return NULL;
}


/* Return the gettext catalog name corresponding to a locale.  If the locale
   consists of a language and a territory, and the language is mainly spoken
   in that territory, the territory is removed from the locale name.
   For example, "de_DE" or "de_DE.ISO-8859-1" are simplified to "de",
   because the resulting catalog can be used as a default for all "de_XX",
   such as "de_AT".  */
static const char *
catalogname_for_locale (const char *locale)
{
  static const char *locales_with_principal_territory[] = {
                /* Language     Main territory */
    "ace_ID",   /* Achinese     Indonesia */
    "af_ZA",    /* Afrikaans    South Africa */
    "ak_GH",    /* Akan         Ghana */
    "am_ET",    /* Amharic      Ethiopia */
    "an_ES",    /* Aragonese    Spain */
    "ang_GB",   /* Old English  Britain */
    "arn_CL",   /* Mapudungun   Chile */
    "as_IN",    /* Assamese     India */
    "ast_ES",   /* Asturian     Spain */
    "av_RU",    /* Avaric       Russia */
    "awa_IN",   /* Awadhi       India */
    "az_AZ",    /* Azerbaijani  Azerbaijan */
    "ban_ID",   /* Balinese     Indonesia */
    "be_BY",    /* Belarusian   Belarus */
    "bej_SD",   /* Beja         Sudan */
    "bem_ZM",   /* Bemba        Zambia */
    "bg_BG",    /* Bulgarian    Bulgaria */
    "bho_IN",   /* Bhojpuri     India */
    "bi_VU",    /* Bislama      Vanuatu */
    "bik_PH",   /* Bikol        Philippines */
    "bin_NG",   /* Bini         Nigeria */
    "bm_ML",    /* Bambara      Mali */
    "bn_IN",    /* Bengali      India */
    "bo_CN",    /* Tibetan      China */
    "br_FR",    /* Breton       France */
    "bs_BA",    /* Bosnian      Bosnia */
    "bug_ID",   /* Buginese     Indonesia */
    "ca_ES",    /* Catalan      Spain */
    "ce_RU",    /* Chechen      Russia */
    "ceb_PH",   /* Cebuano      Philippines */
    "co_FR",    /* Corsican     France */
    "cr_CA",    /* Cree         Canada */
    /* Don't put "crh_UZ" or "crh_UA" here.  That would be asking for fruitless
       political discussion.  */
    "cs_CZ",    /* Czech        Czech Republic */
    "csb_PL",   /* Kashubian    Poland */
    "cy_GB",    /* Welsh        Britain */
    "da_DK",    /* Danish       Denmark */
    "de_DE",    /* German       Germany */
    "din_SD",   /* Dinka        Sudan */
    "doi_IN",   /* Dogri        India */
    "dsb_DE",   /* Lower Sorbian        Germany */
    "dv_MV",    /* Divehi       Maldives */
    "dz_BT",    /* Dzongkha     Bhutan */
    "ee_GH",    /* Éwé          Ghana */
    "el_GR",    /* Greek        Greece */
    /* Don't put "en_GB" or "en_US" here.  That would be asking for fruitless
       political discussion.  */
    "es_ES",    /* Spanish      Spain */
    "et_EE",    /* Estonian     Estonia */
    "fa_IR",    /* Persian      Iran */
    "fi_FI",    /* Finnish      Finland */
    "fil_PH",   /* Filipino     Philippines */
    "fj_FJ",    /* Fijian       Fiji */
    "fo_FO",    /* Faroese      Faeroe Islands */
    "fon_BJ",   /* Fon          Benin */
    "fr_FR",    /* French       France */
    "fur_IT",   /* Friulian     Italy */
    "fy_NL",    /* Western Frisian      Netherlands */
    "ga_IE",    /* Irish        Ireland */
    "gd_GB",    /* Scottish Gaelic      Britain */
    "gon_IN",   /* Gondi        India */
    "gsw_CH",   /* Swiss German Switzerland */
    "gu_IN",    /* Gujarati     India */
    "he_IL",    /* Hebrew       Israel */
    "hi_IN",    /* Hindi        India */
    "hil_PH",   /* Hiligaynon   Philippines */
    "hr_HR",    /* Croatian     Croatia */
    "hsb_DE",   /* Upper Sorbian        Germany */
    "ht_HT",    /* Haitian      Haiti */
    "hu_HU",    /* Hungarian    Hungary */
    "hy_AM",    /* Armenian     Armenia */
    "id_ID",    /* Indonesian   Indonesia */
    "ig_NG",    /* Igbo         Nigeria */
    "ii_CN",    /* Sichuan Yi   China */
    "ilo_PH",   /* Iloko        Philippines */
    "is_IS",    /* Icelandic    Iceland */
    "it_IT",    /* Italian      Italy */
    "ja_JP",    /* Japanese     Japan */
    "jab_NG",   /* Hyam         Nigeria */
    "jv_ID",    /* Javanese     Indonesia */
    "ka_GE",    /* Georgian     Georgia */
    "kab_DZ",   /* Kabyle       Algeria */
    "kaj_NG",   /* Jju          Nigeria */
    "kam_KE",   /* Kamba        Kenya */
    "kmb_AO",   /* Kimbundu     Angola */
    "kcg_NG",   /* Tyap         Nigeria */
    "kdm_NG",   /* Kagoma       Nigeria */
    "kg_CD",    /* Kongo        Democratic Republic of Congo */
    "kk_KZ",    /* Kazakh       Kazakhstan */
    "kl_GL",    /* Kalaallisut  Greenland */
    "km_KH",    /* Central Khmer        Cambodia */
    "kn_IN",    /* Kannada      India */
    "ko_KR",    /* Korean       Korea (South) */
    "kok_IN",   /* Konkani      India */
    "kr_NG",    /* Kanuri       Nigeria */
    "kru_IN",   /* Kurukh       India */
    "ky_KG",    /* Kyrgyz       Kyrgyzstan */
    "lg_UG",    /* Ganda        Uganda */
    "li_BE",    /* Limburgish   Belgium */
    "lo_LA",    /* Laotian      Laos */
    "lt_LT",    /* Lithuanian   Lithuania */
    "lu_CD",    /* Luba-Katanga Democratic Republic of Congo */
    "lua_CD",   /* Luba-Lulua   Democratic Republic of Congo */
    "luo_KE",   /* Luo          Kenya */
    "lv_LV",    /* Latvian      Latvia */
    "mad_ID",   /* Madurese     Indonesia */
    "mag_IN",   /* Magahi       India */
    "mai_IN",   /* Maithili     India */
    "mak_ID",   /* Makasar      Indonesia */
    "man_ML",   /* Mandingo     Mali */
    "men_SL",   /* Mende        Sierra Leone */
    "mfe_MU",   /* Mauritian Creole     Mauritius */
    "mg_MG",    /* Malagasy     Madagascar */
    "mi_NZ",    /* Maori        New Zealand */
    "min_ID",   /* Minangkabau  Indonesia */
    "mk_MK",    /* Macedonian   North Macedonia */
    "ml_IN",    /* Malayalam    India */
    "mn_MN",    /* Mongolian    Mongolia */
    "mni_IN",   /* Manipuri     India */
    "mos_BF",   /* Mossi        Burkina Faso */
    "mr_IN",    /* Marathi      India */
    "ms_MY",    /* Malay        Malaysia */
    "mt_MT",    /* Maltese      Malta */
    "mwr_IN",   /* Marwari      India */
    "my_MM",    /* Burmese      Myanmar */
    "na_NR",    /* Nauru        Nauru */
    "nah_MX",   /* Nahuatl      Mexico */
    "nap_IT",   /* Neapolitan   Italy */
    "nb_NO",    /* Norwegian Bokmål    Norway */
    "nds_DE",   /* Low Saxon    Germany */
    "ne_NP",    /* Nepali       Nepal */
    "nl_NL",    /* Dutch        Netherlands */
    "nn_NO",    /* Norwegian Nynorsk    Norway */
    "no_NO",    /* Norwegian    Norway */
    "nr_ZA",    /* South Ndebele        South Africa */
    "nso_ZA",   /* Northern Sotho       South Africa */
    "ny_MW",    /* Chichewa     Malawi */
    "nym_TZ",   /* Nyamwezi     Tanzania */
    "nyn_UG",   /* Nyankole     Uganda */
    "oc_FR",    /* Occitan      France */
    "oj_CA",    /* Ojibwa       Canada */
    "or_IN",    /* Oriya        India */
    "pa_IN",    /* Punjabi      India */
    "pag_PH",   /* Pangasinan   Philippines */
    "pam_PH",   /* Pampanga     Philippines */
    "pap_AN",   /* Papiamento   Netherlands Antilles - this line can be removed in 2018 */
    "pbb_CO",   /* Páez                Colombia */
    "pl_PL",    /* Polish       Poland */
    "ps_AF",    /* Pashto       Afghanistan */
    "pt_PT",    /* Portuguese   Portugal */
    "raj_IN",   /* Rajasthani   India */
    "rm_CH",    /* Romansh      Switzerland */
    "rn_BI",    /* Kirundi      Burundi */
    "ro_RO",    /* Romanian     Romania */
    "ru_RU",    /* Russian      Russia */
    "rw_RW",    /* Kinyarwanda  Rwanda */
    "sa_IN",    /* Sanskrit     India */
    "sah_RU",   /* Yakut        Russia */
    "sas_ID",   /* Sasak        Indonesia */
    "sat_IN",   /* Santali      India */
    "sc_IT",    /* Sardinian    Italy */
    "scn_IT",   /* Sicilian     Italy */
    "sg_CF",    /* Sango        Central African Republic */
    "shn_MM",   /* Shan         Myanmar */
    "si_LK",    /* Sinhala      Sri Lanka */
    "sid_ET",   /* Sidamo       Ethiopia */
    "sk_SK",    /* Slovak       Slovakia */
    "sl_SI",    /* Slovenian    Slovenia */
    "smn_FI",   /* Inari Sami   Finland */
    "sms_FI",   /* Skolt Sami   Finland */
    "so_SO",    /* Somali       Somalia */
    "sq_AL",    /* Albanian     Albania */
    "sr_RS",    /* Serbian      Serbia */
    "srr_SN",   /* Serer        Senegal */
    "suk_TZ",   /* Sukuma       Tanzania */
    "sus_GN",   /* Susu         Guinea */
    "sv_SE",    /* Swedish      Sweden */
    "te_IN",    /* Telugu       India */
    "tem_SL",   /* Timne        Sierra Leone */
    "tet_ID",   /* Tetum        Indonesia */
    "tg_TJ",    /* Tajik        Tajikistan */
    "th_TH",    /* Thai         Thailand */
    "tiv_NG",   /* Tiv          Nigeria */
    "tk_TM",    /* Turkmen      Turkmenistan */
    "tl_PH",    /* Tagalog      Philippines */
    "to_TO",    /* Tonga        Tonga */
    "tpi_PG",   /* Tok Pisin    Papua New Guinea */
    "tr_TR",    /* Turkish      Türkiye */
    "tum_MW",   /* Tumbuka      Malawi */
    "ug_CN",    /* Uighur       China */
    "uk_UA",    /* Ukrainian    Ukraine */
    "umb_AO",   /* Umbundu      Angola */
    "ur_PK",    /* Urdu         Pakistan */
    "uz_UZ",    /* Uzbek        Uzbekistan */
    "ve_ZA",    /* Venda        South Africa */
    "vi_VN",    /* Vietnamese   Vietnam */
    "wa_BE",    /* Walloon      Belgium */
    "wal_ET",   /* Walamo       Ethiopia */
    "war_PH",   /* Waray        Philippines */
    "wen_DE",   /* Sorbian      Germany */
    "yao_MW",   /* Yao          Malawi */
    "zap_MX"    /* Zapotec      Mexico */
  };
  const char *dot;
  size_t i;

  /* Remove the ".codeset" part from the locale.  */
  dot = strchr (locale, '.');
  if (dot != NULL)
    {
      const char *codeset_end;
      char *shorter_locale;

      codeset_end = strpbrk (dot + 1, "_@");
      if (codeset_end == NULL)
        codeset_end = dot + strlen (dot);

      shorter_locale = XNMALLOC (strlen (locale), char);
      memcpy (shorter_locale, locale, dot - locale);
      strcpy (shorter_locale + (dot - locale), codeset_end);
      locale = shorter_locale;
    }

  /* If the territory is the language's principal territory, drop it.  */
  for (i = 0; i < SIZEOF (locales_with_principal_territory); i++)
    if (strcmp (locale, locales_with_principal_territory[i]) == 0)
      {
        const char *language_end;
        size_t len;
        char *shorter_locale;

        language_end = strchr (locale, '_');
        if (language_end == NULL)
          abort ();

        len = language_end - locale;
        shorter_locale = XNMALLOC (len + 1, char);
        memcpy (shorter_locale, locale, len);
        shorter_locale[len] = '\0';
        locale = shorter_locale;
        break;
      }

  return locale;
}


/* Return the language of a locale.  */
static const char *
language_of_locale (const char *locale)
{
  const char *language_end;

  language_end = strpbrk (locale, "_.@");
  if (language_end != NULL)
    {
      size_t len;
      char *result;

      len = language_end - locale;
      result = XNMALLOC (len + 1, char);
      memcpy (result, locale, len);
      result[len] = '\0';

      return result;
    }
  else
    return locale;
}


/* Return the most likely desired charset for the PO file, as a portable
   charset name.  */
static const char *
canonical_locale_charset ()
{
  const char *tmp;
  char *old_LC_ALL;
  const char *charset;

  /* Save LC_ALL environment variable.  */

  tmp = getenv ("LC_ALL");
  old_LC_ALL = (tmp != NULL ? xstrdup (tmp) : NULL);

  xsetenv ("LC_ALL", locale, 1);

  if (setlocale (LC_ALL, "") == NULL)
    /* Nonexistent locale.  Use anything.  */
    charset = "";
  else
    /* Get the locale's charset.  */
    charset = locale_charset ();

  /* Restore LC_ALL environment variable.  */

  if (old_LC_ALL != NULL)
    xsetenv ("LC_ALL", old_LC_ALL, 1), free (old_LC_ALL);
  else
    unsetenv ("LC_ALL");

  setlocale (LC_ALL, "");

  /* Canonicalize it.  */
  charset = po_charset_canonicalize (charset);
  if (charset == NULL)
    charset = po_charset_ascii;

  return charset;
}


/* Return the English name of the language.  */
static const char *
englishname_of_language ()
{
  size_t i;

  for (i = 0; i < language_table_size; i++)
    if (strcmp (language_table[i].code, language) == 0)
      return language_table[i].english;

  return xasprintf ("Language %s", language);
}


/* Construct the value for the PACKAGE name.  */
static const char *
project_id (const char *header)
{
  const char *old_field;

  /* Return the first part of the Project-Id-Version field if present, assuming
     it was already filled in by xgettext.  */
  old_field = get_field (header, "Project-Id-Version");
  if (old_field != NULL && strcmp (old_field, "PACKAGE VERSION") != 0)
    {
      /* Remove the last word from old_field.  */
      const char *last_space;

      last_space = strrchr (old_field, ' ');
      if (last_space != NULL)
        {
          while (last_space > old_field && last_space[-1] == ' ')
            last_space--;
          if (last_space > old_field)
            {
              size_t package_len = last_space - old_field;
              char *package = XNMALLOC (package_len + 1, char);
              memcpy (package, old_field, package_len);
              package[package_len] = '\0';

              return package;
            }
        }
      /* It contains no version, just a package name.  */
      return old_field;
    }

  /* On native Windows, a Bourne shell is generally not available.
     Avoid error messages such as
     "msginit.exe: subprocess ... failed: No such file or directory"  */
#if !(defined _WIN32 && ! defined __CYGWIN__)
  {
    const char *gettextlibdir;
    char *prog;
    const char *argv[3];
    pid_t child;
    int fd[1];
    FILE *fp;
    char *line;
    size_t linesize;
    size_t linelen;
    int exitstatus;

    gettextlibdir = getenv ("GETTEXTLIBDIR_SRCDIR");
    if (gettextlibdir == NULL || gettextlibdir[0] == '\0')
      gettextlibdir = relocate (LIBDIR "/gettext");

    prog = xconcatenated_filename (gettextlibdir, "project-id", NULL);

    /* Call the project-id shell script.  */
    argv[0] = BOURNE_SHELL;
    argv[1] = prog;
    argv[2] = NULL;
    child = create_pipe_in (prog, BOURNE_SHELL, argv, NULL,
                            DEV_NULL, false, true, false, fd);
    if (child == -1)
      goto failed;

    /* Retrieve its result.  */
    fp = fdopen (fd[0], "r");
    if (fp == NULL)
      {
        error (0, errno, _("fdopen() failed"));
        goto failed;
      }

    line = NULL; linesize = 0;
    linelen = getline (&line, &linesize, fp);
    if (linelen == (size_t)(-1))
      {
        error (0, 0, _("%s subprocess I/O error"), prog);
        fclose (fp);
        goto failed;
      }
    if (linelen > 0 && line[linelen - 1] == '\n')
      line[linelen - 1] = '\0';

    fclose (fp);

    /* Remove zombie process from process list, and retrieve exit status.  */
    exitstatus = wait_subprocess (child, prog, false, false, true, false, NULL);
    if (exitstatus != 0)
      {
        error (0, 0, _("%s subprocess failed with exit code %d"),
               prog, exitstatus);
        goto failed;
      }

    return line;
  }

failed:
#endif
  return "PACKAGE";
}


/* Construct the value for the Project-Id-Version field.  */
static const char *
project_id_version (const char *header)
{
  const char *old_field;

  /* Return the old value if present, assuming it was already filled in by
     xgettext.  */
  old_field = get_field (header, "Project-Id-Version");
  if (old_field != NULL && strcmp (old_field, "PACKAGE VERSION") != 0)
    return old_field;

  /* On native Windows, a Bourne shell is generally not available.
     Avoid error messages such as
     "msginit.exe: subprocess ... failed: No such file or directory"  */
#if !(defined _WIN32 && ! defined __CYGWIN__)
  {
    const char *gettextlibdir;
    char *prog;
    const char *argv[4];
    pid_t child;
    int fd[1];
    FILE *fp;
    char *line;
    size_t linesize;
    size_t linelen;
    int exitstatus;

    gettextlibdir = getenv ("GETTEXTLIBDIR_SRCDIR");
    if (gettextlibdir == NULL || gettextlibdir[0] == '\0')
      gettextlibdir = relocate (LIBDIR "/gettext");

    prog = xconcatenated_filename (gettextlibdir, "project-id", NULL);

    /* Call the project-id shell script.  */
    argv[0] = BOURNE_SHELL;
    argv[1] = prog;
    argv[2] = "yes";
    argv[3] = NULL;
    child = create_pipe_in (prog, BOURNE_SHELL, argv, NULL,
                            DEV_NULL, false, true, false, fd);
    if (child == -1)
      goto failed;

    /* Retrieve its result.  */
    fp = fdopen (fd[0], "r");
    if (fp == NULL)
      {
        error (0, errno, _("fdopen() failed"));
        goto failed;
      }

    line = NULL; linesize = 0;
    linelen = getline (&line, &linesize, fp);
    if (linelen == (size_t)(-1))
      {
        error (0, 0, _("%s subprocess I/O error"), prog);
        fclose (fp);
        goto failed;
      }
    if (linelen > 0 && line[linelen - 1] == '\n')
      line[linelen - 1] = '\0';

    fclose (fp);

    /* Remove zombie process from process list, and retrieve exit status.  */
    exitstatus = wait_subprocess (child, prog, false, false, true, false, NULL);
    if (exitstatus != 0)
      {
        error (0, 0, _("%s subprocess failed with exit code %d"),
               prog, exitstatus);
        goto failed;
      }

    return line;
  }

failed:
#endif
  return "PACKAGE VERSION";
}


/* Construct the value for the PO-Revision-Date field.  */
static const char *
po_revision_date (const char *header)
{
  if (no_translator)
    /* Because the PO file is automatically generated, we use the
       POT-Creation-Date, not the current time.  */
    return get_field (header, "POT-Creation-Date");
  else
    {
      /* Assume the translator will modify the PO file now.  */
      time_t now;

      time (&now);
      return po_strftime (&now);
    }
}


#if HAVE_PWD_H  /* Only Unix, not native Windows.  */

/* Returns the struct passwd entry for the current user.  */
static struct passwd *
get_user_pwd ()
{
  const char *username;
  struct passwd *userpasswd;

  /* 1. attempt: getpwnam(getenv("USER"))  */
  username = getenv ("USER");
  if (username != NULL)
    {
      errno = 0;
      userpasswd = getpwnam (username);
      if (userpasswd != NULL)
        return userpasswd;
      if (errno != 0)
        error (EXIT_FAILURE, errno, "getpwnam(\"%s\")", username);
    }

  /* 2. attempt: getpwnam(getlogin())  */
  username = getlogin ();
  if (username != NULL)
    {
      errno = 0;
      userpasswd = getpwnam (username);
      if (userpasswd != NULL)
        return userpasswd;
      if (errno != 0)
        error (EXIT_FAILURE, errno, "getpwnam(\"%s\")", username);
    }

  /* 3. attempt: getpwuid(getuid())  */
  errno = 0;
  userpasswd = getpwuid (getuid ());
  if (userpasswd != NULL)
    return userpasswd;
  if (errno != 0)
    error (EXIT_FAILURE, errno, "getpwuid(%ju)", (uintmax_t) getuid ());

  return NULL;
}

#endif


/* Return the user's full name.  */
static const char *
get_user_fullname ()
{
#if HAVE_PWD_H
  struct passwd *pwd;

  pwd = get_user_pwd ();
  if (pwd != NULL)
    {
      const char *fullname;
      const char *fullname_end;
      char *result;

      /* Return the pw_gecos field, up to the first comma (if any).  */
      fullname = pwd->pw_gecos;
      fullname_end = strchr (fullname, ',');
      if (fullname_end == NULL)
        fullname_end = fullname + strlen (fullname);

      result = XNMALLOC (fullname_end - fullname + 1, char);
      memcpy (result, fullname, fullname_end - fullname);
      result[fullname_end - fullname] = '\0';

      return result;
    }
#endif

  return NULL;
}


/* Return the user's email address.  */
static const char *
get_user_email ()
{
  /* On native Windows, a Bourne shell is generally not available.
     Avoid error messages such as
     "msginit.exe: subprocess ... failed: No such file or directory"  */
#if !(defined _WIN32 && ! defined __CYGWIN__)
  {
    const char *prog = relocate (LIBDIR "/gettext/user-email");
    const char *argv[4];
    pid_t child;
    int fd[1];
    FILE *fp;
    char *line;
    size_t linesize;
    size_t linelen;
    int exitstatus;

    /* Ask the user for his email address.  */
    argv[0] = BOURNE_SHELL;
    argv[1] = prog;
    argv[2] = _("\
The new message catalog should contain your email address, so that users can\n\
give you feedback about the translations, and so that maintainers can contact\n\
you in case of unexpected technical problems.\n");
    argv[3] = NULL;
    child = create_pipe_in (prog, BOURNE_SHELL, argv, NULL,
                            DEV_NULL, false, true, false, fd);
    if (child == -1)
      goto failed;

    /* Retrieve his answer.  */
    fp = fdopen (fd[0], "r");
    if (fp == NULL)
      {
        error (0, errno, _("fdopen() failed"));
        goto failed;
      }

    line = NULL; linesize = 0;
    linelen = getline (&line, &linesize, fp);
    if (linelen == (size_t)(-1))
      {
        error (0, 0, _("%s subprocess I/O error"), prog);
        fclose (fp);
        goto failed;
      }
    if (linelen > 0 && line[linelen - 1] == '\n')
      line[linelen - 1] = '\0';

    fclose (fp);

    /* Remove zombie process from process list, and retrieve exit status.  */
    exitstatus = wait_subprocess (child, prog, false, false, true, false, NULL);
    if (exitstatus != 0)
      {
        error (0, 0, _("%s subprocess failed with exit code %d"),
               prog, exitstatus);
        goto failed;
      }

    return line;
  }

failed:
#endif
  return "EMAIL@ADDRESS";
}


/* Construct the value for the Last-Translator field.  */
static const char *
last_translator ()
{
  if (no_translator)
    return "Automatically generated";
  else
    {
      const char *fullname = get_user_fullname ();
      const char *email = get_user_email ();

      if (fullname != NULL)
        return xasprintf ("%s <%s>", fullname, email);
      else
        return xasprintf ("<%s>", email);
    }
}


/* Return the name of the language used by the language team, in English.  */
static const char *
language_team_englishname ()
{
  size_t i;

  /* Search for a name depending on the catalogname.  */
  for (i = 0; i < language_variant_table_size; i++)
    if (strcmp (language_variant_table[i].code, catalogname) == 0)
      return language_variant_table[i].english;

  /* Search for a name depending on the language only.  */
  return englishname_of_language ();
}


/* Return the language team's mailing list address or homepage URL.  */
static const char *
language_team_address ()
{
  /* On native Windows, a Bourne shell is generally not available.
     Avoid error messages such as
     "msginit.exe: subprocess ... failed: No such file or directory"  */
#if !(defined _WIN32 && ! defined __CYGWIN__)
  {
    const char *prog = relocate (PROJECTSDIR "/team-address");
    const char *argv[7];
    pid_t child;
    int fd[1];
    FILE *fp;
    char *line;
    size_t linesize;
    size_t linelen;
    const char *result;
    int exitstatus;

    /* Call the team-address shell script.  */
    argv[0] = BOURNE_SHELL;
    argv[1] = prog;
    argv[2] = relocate (PROJECTSDIR);
    argv[3] = relocate (LIBDIR "/gettext");
    argv[4] = catalogname;
    argv[5] = language;
    argv[6] = NULL;
    child = create_pipe_in (prog, BOURNE_SHELL, argv, NULL,
                            DEV_NULL, false, true, false, fd);
    if (child == -1)
      goto failed;

    /* Retrieve its result.  */
    fp = fdopen (fd[0], "r");
    if (fp == NULL)
      {
        error (0, errno, _("fdopen() failed"));
        goto failed;
      }

    line = NULL; linesize = 0;
    linelen = getline (&line, &linesize, fp);
    if (linelen == (size_t)(-1))
      result = "";
    else
      {
        if (linelen > 0 && line[linelen - 1] == '\n')
          line[linelen - 1] = '\0';
        result = line;
      }

    fclose (fp);

    /* Remove zombie process from process list, and retrieve exit status.  */
    exitstatus = wait_subprocess (child, prog, false, false, true, false, NULL);
    if (exitstatus != 0)
      {
        error (0, 0, _("%s subprocess failed with exit code %d"),
               prog, exitstatus);
        goto failed;
      }

    return result;
  }

failed:
#endif
  return "";
}


/* Construct the value for the Language-Team field.  */
static const char *
language_team ()
{
  if (no_translator)
    return "none";
  else
    {
      const char *englishname = language_team_englishname ();
      const char *address = language_team_address ();

      if (address != NULL && address[0] != '\0')
        return xasprintf ("%s %s", englishname, address);
      else
        return englishname;
    }
}


/* Construct the value for the Language field.  */
static const char *
language_value ()
{
  return catalogname;
}


/* Construct the value for the MIME-Version field.  */
static const char *
mime_version ()
{
  return "1.0";
}


/* Construct the value for the Content-Type field.  */
static const char *
content_type (const char *header)
{
  bool was_utf8;
  const char *old_field;

  /* If the POT file contains charset=UTF-8, it means that the POT file
     contains non-ASCII characters, and we keep the UTF-8 encoding.
     Otherwise, when the POT file is plain ASCII, we use the locale's
     encoding.  */
  was_utf8 = false;
  old_field = get_field (header, "Content-Type");
  if (old_field != NULL)
    {
      const char *charsetstr = c_strstr (old_field, "charset=");

      if (charsetstr != NULL)
        {
          charsetstr += strlen ("charset=");
          was_utf8 = (c_strcasecmp (charsetstr, "UTF-8") == 0);
        }
    }
  return xasprintf ("text/plain; charset=%s",
                    was_utf8 ? "UTF-8" : canonical_locale_charset ());
}


/* Construct the value for the Content-Transfer-Encoding field.  */
static const char *
content_transfer_encoding ()
{
  return "8bit";
}


/* Construct the value for the Plural-Forms field.  */
static const char *
plural_forms ()
{
  const char *gettextcldrdir;
  char *prog = NULL;
  size_t i;

  /* Search for a formula depending on the catalogname.  */
  for (i = 0; i < plural_table_size; i++)
    if (strcmp (plural_table[i].lang, catalogname) == 0)
      return plural_table[i].value;

  /* Search for a formula depending on the language only.  */
  for (i = 0; i < plural_table_size; i++)
    if (strcmp (plural_table[i].lang, language) == 0)
      return plural_table[i].value;

  gettextcldrdir = getenv ("GETTEXTCLDRDIR");
  if (gettextcldrdir != NULL && gettextcldrdir[0] != '\0')
    {
      const char *gettextlibdir;
      const char *dirs[3];
      char *last_dir;
      const char *argv[4];
      pid_t child;
      int fd[1];
      FILE *fp;
      char *line;
      size_t linesize;
      size_t linelen;
      int exitstatus;

      gettextlibdir = getenv ("GETTEXTLIBDIR_BUILDDIR");
      if (gettextlibdir == NULL || gettextlibdir[0] == '\0')
        gettextlibdir = relocate (LIBDIR "/gettext");

      prog = xconcatenated_filename (gettextlibdir, "cldr-plurals", EXEEXT);

      last_dir = xstrdup (gettextcldrdir);
      dirs[0] = "common";
      dirs[1] = "supplemental";
      dirs[2] = "plurals.xml";
      for (i = 0; i < SIZEOF (dirs); i++)
        {
          char *dir = xconcatenated_filename (last_dir, dirs[i], NULL);
          free (last_dir);
          last_dir = dir;
        }

      /* Call the cldr-plurals command.
         argv[0] must be prog, not just the base name "cldr-plurals",
         because on Cygwin in a build with --enable-shared, the libtool
         wrapper of cldr-plurals.exe apparently needs this.  */
      argv[0] = prog;
      argv[1] = language;
      argv[2] = last_dir;
      argv[3] = NULL;
      child = create_pipe_in (prog, prog, argv, NULL,
                              DEV_NULL, false, true, false, fd);
      free (last_dir);
      if (child == -1)
        goto failed;

      /* Retrieve its result.  */
      fp = fdopen (fd[0], "r");
      if (fp == NULL)
        {
          error (0, errno, _("fdopen() failed"));
          goto failed;
        }

      line = NULL; linesize = 0;
      linelen = getline (&line, &linesize, fp);
      if (linelen == (size_t)(-1))
        {
          error (0, 0, _("%s subprocess I/O error"), prog);
          fclose (fp);
          goto failed;
        }
      if (linelen > 0 && line[linelen - 1] == '\n')
        {
          line[linelen - 1] = '\0';
#if defined _WIN32 && ! defined __CYGWIN__
          if (linelen > 1 && line[linelen - 2] == '\r')
            line[linelen - 2] = '\0';
#endif
        }

      fclose (fp);

      /* Remove zombie process from process list, and retrieve exit status.  */
      exitstatus = wait_subprocess (child, prog, false, false, true, false,
                                    NULL);
      if (exitstatus != 0)
        {
          error (0, 0, _("%s subprocess failed with exit code %d"),
                 prog, exitstatus);
          goto failed;
        }

      return line;
    }

 failed:
  free (prog);
  return NULL;
}


static struct
{
  const char *name;
  const char * (*getter0) (void);
  const char * (*getter1) (const char *header);
}
fields[] =
  {
    { "Project-Id-Version", NULL, project_id_version },
    { "PO-Revision-Date", NULL, po_revision_date },
    { "Last-Translator", last_translator, NULL },
    { "Language-Team", language_team, NULL },
    { "Language", language_value, NULL },
    { "MIME-Version", mime_version, NULL },
    { "Content-Type", NULL, content_type },
    { "Content-Transfer-Encoding", content_transfer_encoding, NULL },
    { "Plural-Forms", plural_forms, NULL }
  };

#define NFIELDS SIZEOF (fields)
#define FIELD_LAST_TRANSLATOR 2


/* Retrieve a freshly allocated copy of a field's value.  */
static char *
get_field (const char *header, const char *field)
{
  size_t len = strlen (field);
  const char *line;

  for (line = header;;)
    {
      if (strncmp (line, field, len) == 0 && line[len] == ':')
        {
          const char *value_start;
          const char *value_end;
          char *value;

          value_start = line + len + 1;
          if (*value_start == ' ')
            value_start++;
          value_end = strchr (value_start, '\n');
          if (value_end == NULL)
            value_end = value_start + strlen (value_start);

          value = XNMALLOC (value_end - value_start + 1, char);
          memcpy (value, value_start, value_end - value_start);
          value[value_end - value_start] = '\0';

          return value;
        }

      line = strchr (line, '\n');
      if (line != NULL)
        line++;
      else
        break;
    }

  return NULL;
}

/* Add a field with value to a header, and return the new header.  */
static char *
put_field (const char *old_header, const char *field, const char *value)
{
  size_t len = strlen (field);
  const char *line;
  char *new_header;
  char *p;

  for (line = old_header;;)
    {
      if (strncmp (line, field, len) == 0 && line[len] == ':')
        {
          const char *value_start;
          const char *value_end;

          value_start = line + len + 1;
          if (*value_start == ' ')
            value_start++;
          value_end = strchr (value_start, '\n');
          if (value_end == NULL)
            value_end = value_start + strlen (value_start);

          new_header = XNMALLOC (strlen (old_header)
                                 - (value_end - value_start)
                                 + strlen (value)
                                 + (*value_end != '\n' ? 1 : 0)
                                 + 1,
                                 char);
          p = new_header;
          memcpy (p, old_header, value_start - old_header);
          p += value_start - old_header;
          memcpy (p, value, strlen (value));
          p += strlen (value);
          if (*value_end != '\n')
            *p++ = '\n';
          strcpy (p, value_end);

          return new_header;
        }

      line = strchr (line, '\n');
      if (line != NULL)
        line++;
      else
        break;
    }

  new_header = XNMALLOC (strlen (old_header) + 1
                         + len + 2 + strlen (value) + 1
                         + 1,
                         char);
  p = new_header;
  memcpy (p, old_header, strlen (old_header));
  p += strlen (old_header);
  if (p > new_header && p[-1] != '\n')
    *p++ = '\n';
  memcpy (p, field, len);
  p += len;
  *p++ = ':';
  *p++ = ' ';
  memcpy (p, value, strlen (value));
  p += strlen (value);
  *p++ = '\n';
  *p = '\0';

  return new_header;
}


/* Return the title format string.  */
static const char *
get_title ()
{
  /* This is tricky.  We want the translation in the given locale specified by
     the command line, not the current locale.  But we want it in the encoding
     that we put into the header entry, not the encoding of that locale.
     We could avoid the use of OUTPUT_CHARSET by using a separate message
     catalog and bind_textdomain_codeset(), but that doesn't seem worth the
     trouble for one single message.  */
  const char *encoding;
  const char *tmp;
  char *old_LC_ALL;
  char *old_LANGUAGE;
  char *old_OUTPUT_CHARSET;
  const char *msgid;
  const char *english;
  const char *result;

  encoding = canonical_locale_charset ();

  /* First, the English title.  */
  english = xasprintf ("%s translations for %%s package",
                       englishname_of_language ());

  /* Save LC_ALL, LANGUAGE, OUTPUT_CHARSET environment variables.  */

  tmp = getenv ("LC_ALL");
  old_LC_ALL = (tmp != NULL ? xstrdup (tmp) : NULL);

  tmp = getenv ("LANGUAGE");
  old_LANGUAGE = (tmp != NULL ? xstrdup (tmp) : NULL);

  tmp = getenv ("OUTPUT_CHARSET");
  old_OUTPUT_CHARSET = (tmp != NULL ? xstrdup (tmp) : NULL);

  xsetenv ("LC_ALL", locale, 1);
  unsetenv ("LANGUAGE");
  xsetenv ("OUTPUT_CHARSET", encoding, 1);

  if (setlocale (LC_ALL, "") == NULL)
    /* Nonexistent locale.  Use the English title.  */
    result = english;
  else
    {
      /* Fetch the translation.  */
      /* TRANSLATORS: "English" needs to be replaced by your language.
         For example in it.po write "Traduzioni italiani ...",
         *not* "Traduzioni inglesi ...".  */
      msgid = N_("English translations for %s package");
      result = gettext (msgid);
      if (result != msgid && strcmp (result, msgid) != 0)
        /* Use the English and the foreign title.  */
        result = xasprintf ("%s\n%s", english, result);
      else
        /* No translation found.  Use the English title.  */
        result = english;
    }

  /* Restore LC_ALL, LANGUAGE, OUTPUT_CHARSET environment variables.  */

  if (old_LC_ALL != NULL)
    xsetenv ("LC_ALL", old_LC_ALL, 1), free (old_LC_ALL);
  else
    unsetenv ("LC_ALL");

  if (old_LANGUAGE != NULL)
    xsetenv ("LANGUAGE", old_LANGUAGE, 1), free (old_LANGUAGE);
  else
    unsetenv ("LANGUAGE");

  if (old_OUTPUT_CHARSET != NULL)
    xsetenv ("OUTPUT_CHARSET", old_OUTPUT_CHARSET, 1), free (old_OUTPUT_CHARSET);
  else
    unsetenv ("OUTPUT_CHARSET");

  setlocale (LC_ALL, "");

  return result;
}


/* Perform a set of substitutions in a string and return the resulting
   string.  When subst[j][0] found, it is replaced with subst[j][1].
   subst[j][0] must not be the empty string.  */
static const char *
subst_string (const char *str,
              unsigned int nsubst, const char *(*subst)[2])
{
  if (nsubst > 0)
    {
      char *malloced = NULL;
      size_t *substlen;
      size_t i;
      unsigned int j;

      substlen = (size_t *) xmalloca (nsubst * sizeof (size_t));
      for (j = 0; j < nsubst; j++)
        {
          substlen[j] = strlen (subst[j][0]);
          if (substlen[j] == 0)
            abort ();
        }

      for (i = 0;;)
        {
          if (str[i] == '\0')
            break;
          for (j = 0; j < nsubst; j++)
            if (*(str + i) == *subst[j][0]
                && strncmp (str + i, subst[j][0], substlen[j]) == 0)
              {
                size_t replacement_len = strlen (subst[j][1]);
                size_t new_len = strlen (str) - substlen[j] + replacement_len;
                char *new_str = XNMALLOC (new_len + 1, char);
                memcpy (new_str, str, i);
                memcpy (new_str + i, subst[j][1], replacement_len);
                strcpy (new_str + i + replacement_len, str + i + substlen[j]);
                if (malloced != NULL)
                  free (malloced);
                str = new_str;
                malloced = new_str;
                i += replacement_len;
                break;
              }
          if (j == nsubst)
            i++;
        }

      freea (substlen);
    }

  return str;
}

/* Perform a set of substitutions on each string of a string list.
   When subst[j][0] found, it is replaced with subst[j][1].  subst[j][0]
   must not be the empty string.  */
static void
subst_string_list (string_list_ty *slp,
                   unsigned int nsubst, const char *(*subst)[2])
{
  size_t j;

  for (j = 0; j < slp->nitems; j++)
    slp->item[j] = subst_string (slp->item[j], nsubst, subst);
}


/* Fill the templates in all fields of the header entry.  */
static msgdomain_list_ty *
fill_header (msgdomain_list_ty *mdlp)
{
  /* Cache the strings filled in, for use when there are multiple domains
     and a header entry for each domain.  */
  const char *field_value[NFIELDS];
  size_t k, j, i;

  for (i = 0; i < NFIELDS; i++)
    field_value[i] = NULL;

  for (k = 0; k < mdlp->nitems; k++)
    {
      message_list_ty *mlp = mdlp->item[k]->messages;

      if (mlp->nitems > 0)
        {
          message_ty *header_mp = NULL;
          char *header;

          /* Search the header entry.  */
          for (j = 0; j < mlp->nitems; j++)
            if (is_header (mlp->item[j]) && !mlp->item[j]->obsolete)
              {
                header_mp = mlp->item[j];
                break;
              }

          /* If it wasn't found, provide one.  */
          if (header_mp == NULL)
            {
              static lex_pos_ty pos = { __FILE__, __LINE__ };

              header_mp = message_alloc (NULL, "", NULL, "", 1, &pos);
              message_list_prepend (mlp, header_mp);
            }

          header = xstrdup (header_mp->msgstr);

          /* Fill in the fields.  */
          for (i = 0; i < NFIELDS; i++)
            {
              if (field_value[i] == NULL)
                field_value[i] =
                  (fields[i].getter1 != NULL
                   ? fields[i].getter1 (header)
                   : fields[i].getter0 ());

              if (field_value[i] != NULL)
                {
                  char *old_header = header;
                  header = put_field (header, fields[i].name, field_value[i]);
                  free (old_header);
                }
            }

          /* Replace the old translation in the header entry.  */
          header_mp->msgstr = header;
          header_mp->msgstr_len = strlen (header) + 1;

          /* Update the comments in the header entry.  */
          if (header_mp->comment != NULL)
            {
              const char *subst[4][2];
              const char *id;
              time_t now;

              id = project_id (header);
              subst[0][0] = "SOME DESCRIPTIVE TITLE";
              subst[0][1] = xasprintf (get_title (), id, id);
              subst[1][0] = "PACKAGE";
              subst[1][1] = id;
              subst[2][0] = "FIRST AUTHOR <EMAIL@ADDRESS>";
              subst[2][1] = field_value[FIELD_LAST_TRANSLATOR];
              subst[3][0] = "YEAR";
              subst[3][1] =
                xasprintf ("%d",
                           (time (&now), (localtime (&now))->tm_year + 1900));
              subst_string_list (header_mp->comment, SIZEOF (subst), subst);
            }

          /* Finally remove the fuzzy attribute.  */
          header_mp->is_fuzzy = false;
        }
    }

  return mdlp;
}


/* Update the msgstr plural entries according to the nplurals count.  */
static msgdomain_list_ty *
update_msgstr_plurals (msgdomain_list_ty *mdlp)
{
  size_t k;

  for (k = 0; k < mdlp->nitems; k++)
    {
      message_list_ty *mlp = mdlp->item[k]->messages;
      message_ty *header_entry;
      unsigned long int nplurals;
      char *untranslated_plural_msgstr;
      size_t j;

      header_entry = message_list_search (mlp, NULL, "");
      nplurals = get_plural_count (header_entry ? header_entry->msgstr : NULL);
      untranslated_plural_msgstr = XNMALLOC (nplurals, char);
      memset (untranslated_plural_msgstr, '\0', nplurals);

      for (j = 0; j < mlp->nitems; j++)
        {
          message_ty *mp = mlp->item[j];
          bool is_untranslated;
          const char *p;
          const char *pend;

          if (mp->msgid_plural != NULL)
            {
              /* Test if mp is untranslated.  (It most likely is.)  */
              is_untranslated = true;
              for (p = mp->msgstr, pend = p + mp->msgstr_len; p < pend; p++)
                if (*p != '\0')
                  {
                    is_untranslated = false;
                    break;
                  }
              if (is_untranslated)
                {
                  /* Change mp->msgstr_len consecutive empty strings into
                     nplurals consecutive empty strings.  */
                  if (nplurals > mp->msgstr_len)
                    mp->msgstr = untranslated_plural_msgstr;
                  mp->msgstr_len = nplurals;
                }
            }
        }
    }
  return mdlp;
}

/* setlocale() function that respects the locale chosen by the user.
   Copyright (C) 2009, 2011, 2013, 2018-2019, 2022-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2009.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Override setlocale() and newlocale() so that when the default locale is
   requested (locale = "") and no relevant environment variable is set, the
   locale chosen by the user is used.
   This matters on MacOS X 10 and Windows.
   See the comments in localename.c, function gl_locale_name_default.  */

/* Specification.  */
#include <locale.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* When building a DLL, we must export some functions.  Note that because
   the functions are only defined for binary backward compatibility, we
   don't need to use __declspec(dllimport) in any case.  */
#if HAVE_VISIBILITY && BUILDING_DLL
# define DLL_EXPORTED __attribute__((__visibility__("default")))
#elif defined _MSC_VER && BUILDING_DLL
/* When building with MSVC, exporting a symbol means that the object file
   contains a "linker directive" of the form /EXPORT:symbol.  This can be
   inspected through the "objdump -s --section=.drectve FILE" or
   "dumpbin /directives FILE" commands.
   The symbols from this file should be exported if and only if the object
   file gets included in a DLL.  Libtool, on Windows platforms, defines
   the C macro DLL_EXPORT (together with PIC) when compiling for a DLL
   and does not define it when compiling an object file meant to be linked
   statically into some executable.  */
# if defined DLL_EXPORT
#  define DLL_EXPORTED __declspec(dllexport)
# else
#  define DLL_EXPORTED
# endif
#else
# define DLL_EXPORTED
#endif

#include "localename.h"

#if HAVE_CFLOCALECOPYPREFERREDLANGUAGES || HAVE_CFPREFERENCESCOPYAPPVALUE
# if HAVE_CFLOCALECOPYPREFERREDLANGUAGES
#  include <CoreFoundation/CFLocale.h>
# elif HAVE_CFPREFERENCESCOPYAPPVALUE
#  include <CoreFoundation/CFPreferences.h>
# endif
# include <CoreFoundation/CFPropertyList.h>
# include <CoreFoundation/CFArray.h>
# include <CoreFoundation/CFString.h>
extern void gl_locale_name_canonicalize (char *name);
#endif

/* Get _nl_msg_cat_cntr declaration.  */
#include "gettextP.h"

#if (defined __APPLE__ && defined __MACH__) || defined _WIN32 || defined __CYGWIN__

# undef setlocale
# undef newlocale

/* Return string representation of locale category CATEGORY.  */
static const char *
category_to_name (int category)
{
  const char *retval;

  switch (category)
  {
  case LC_COLLATE:
    retval = "LC_COLLATE";
    break;
  case LC_CTYPE:
    retval = "LC_CTYPE";
    break;
  case LC_MONETARY:
    retval = "LC_MONETARY";
    break;
  case LC_NUMERIC:
    retval = "LC_NUMERIC";
    break;
  case LC_TIME:
    retval = "LC_TIME";
    break;
  case LC_MESSAGES:
    retval = "LC_MESSAGES";
    break;
  default:
    /* If you have a better idea for a default value let me know.  */
    retval = "LC_XXX";
  }

  return retval;
}

# if defined _WIN32 && ! defined __CYGWIN__

/* The native Windows setlocale() function expects locale names of the form
   "German" or "German_Germany" or "DEU", but not "de" or "de_DE".  We need
   to convert the names from the form with ISO 639 language code and ISO 3166
   country code to the form with English names or with three-letter identifier.
   The three-letter identifiers known by a Windows XP SP2 or SP3 are:
     AFK  Afrikaans_South Africa.1252
     ARA  Arabic_Saudi Arabia.1256
     ARB  Arabic_Lebanon.1256
     ARE  Arabic_Egypt.1256
     ARG  Arabic_Algeria.1256
     ARH  Arabic_Bahrain.1256
     ARI  Arabic_Iraq.1256
     ARJ  Arabic_Jordan.1256
     ARK  Arabic_Kuwait.1256
     ARL  Arabic_Libya.1256
     ARM  Arabic_Morocco.1256
     ARO  Arabic_Oman.1256
     ARQ  Arabic_Qatar.1256
     ARS  Arabic_Syria.1256
     ART  Arabic_Tunisia.1256
     ARU  Arabic_U.A.E..1256
     ARY  Arabic_Yemen.1256
     AZE  Azeri (Latin)_Azerbaijan.1254
     BEL  Belarusian_Belarus.1251
     BGR  Bulgarian_Bulgaria.1251
     BSB  Bosnian_Bosnia and Herzegovina.1250
     BSC  Bosnian (Cyrillic)_Bosnia and Herzegovina.1250  (wrong encoding!)
     CAT  Catalan_Spain.1252
     CHH  Chinese_Hong Kong S.A.R..950
     CHI  Chinese_Singapore.936
     CHS  Chinese_People's Republic of China.936
     CHT  Chinese_Taiwan.950
     CSY  Czech_Czech Republic.1250
     CYM  Welsh_United Kingdom.1252
     DAN  Danish_Denmark.1252
     DEA  German_Austria.1252
     DEC  German_Liechtenstein.1252
     DEL  German_Luxembourg.1252
     DES  German_Switzerland.1252
     DEU  German_Germany.1252
     ELL  Greek_Greece.1253
     ENA  English_Australia.1252
     ENB  English_Caribbean.1252
     ENC  English_Canada.1252
     ENG  English_United Kingdom.1252
     ENI  English_Ireland.1252
     ENJ  English_Jamaica.1252
     ENL  English_Belize.1252
     ENP  English_Republic of the Philippines.1252
     ENS  English_South Africa.1252
     ENT  English_Trinidad and Tobago.1252
     ENU  English_United States.1252
     ENW  English_Zimbabwe.1252
     ENZ  English_New Zealand.1252
     ESA  Spanish_Panama.1252
     ESB  Spanish_Bolivia.1252
     ESC  Spanish_Costa Rica.1252
     ESD  Spanish_Dominican Republic.1252
     ESE  Spanish_El Salvador.1252
     ESF  Spanish_Ecuador.1252
     ESG  Spanish_Guatemala.1252
     ESH  Spanish_Honduras.1252
     ESI  Spanish_Nicaragua.1252
     ESL  Spanish_Chile.1252
     ESM  Spanish_Mexico.1252
     ESN  Spanish_Spain.1252
     ESO  Spanish_Colombia.1252
     ESP  Spanish_Spain.1252
     ESR  Spanish_Peru.1252
     ESS  Spanish_Argentina.1252
     ESU  Spanish_Puerto Rico.1252
     ESV  Spanish_Venezuela.1252
     ESY  Spanish_Uruguay.1252
     ESZ  Spanish_Paraguay.1252
     ETI  Estonian_Estonia.1257
     EUQ  Basque_Spain.1252
     FAR  Farsi_Iran.1256
     FIN  Finnish_Finland.1252
     FOS  Faroese_Faroe Islands.1252
     FPO  Filipino_Philippines.1252
     FRA  French_France.1252
     FRB  French_Belgium.1252
     FRC  French_Canada.1252
     FRL  French_Luxembourg.1252
     FRM  French_Principality of Monaco.1252
     FRS  French_Switzerland.1252
     FYN  Frisian_Netherlands.1252
     GLC  Galician_Spain.1252
     HEB  Hebrew_Israel.1255
     HRB  Croatian_Bosnia and Herzegovina.1250
     HRV  Croatian_Croatia.1250
     HUN  Hungarian_Hungary.1250
     IND  Indonesian_Indonesia.1252
     IRE  Irish_Ireland.1252
     ISL  Icelandic_Iceland.1252
     ITA  Italian_Italy.1252
     ITS  Italian_Switzerland.1252
     IUK  Inuktitut (Latin)_Canada.1252
     JPN  Japanese_Japan.932
     KKZ  Kazakh_Kazakhstan.1251
     KOR  Korean_Korea.949
     KYR  Kyrgyz_Kyrgyzstan.1251
     LBX  Luxembourgish_Luxembourg.1252
     LTH  Lithuanian_Lithuania.1257
     LVI  Latvian_Latvia.1257
     MKI  FYRO Macedonian_Former Yugoslav Republic of Macedonia.1251
     MON  Mongolian_Mongolia.1251
     MPD  Mapudungun_Chile.1252
     MSB  Malay_Brunei Darussalam.1252
     MSL  Malay_Malaysia.1252
     MWK  Mohawk_Canada.1252
     NLB  Dutch_Belgium.1252
     NLD  Dutch_Netherlands.1252
     NON  Norwegian-Nynorsk_Norway.1252
     NOR  Norwegian (Bokmål)_Norway.1252
     NSO  Northern Sotho_South Africa.1252
     PLK  Polish_Poland.1250
     PTB  Portuguese_Brazil.1252
     PTG  Portuguese_Portugal.1252
     QUB  Quechua_Bolivia.1252
     QUE  Quechua_Ecuador.1252
     QUP  Quechua_Peru.1252
     RMC  Romansh_Switzerland.1252
     ROM  Romanian_Romania.1250
     RUS  Russian_Russia.1251
     SKY  Slovak_Slovakia.1250
     SLV  Slovenian_Slovenia.1250
     SMA  Sami (Southern)_Norway.1252
     SMB  Sami (Southern)_Sweden.1252
     SME  Sami (Northern)_Norway.1252
     SMF  Sami (Northern)_Sweden.1252
     SMG  Sami (Northern)_Finland.1252
     SMJ  Sami (Lule)_Norway.1252
     SMK  Sami (Lule)_Sweden.1252
     SMN  Sami (Inari)_Finland.1252
     SMS  Sami (Skolt)_Finland.1252
     SQI  Albanian_Albania.1250
     SRB  Serbian (Cyrillic)_Serbia and Montenegro.1251
     SRL  Serbian (Latin)_Serbia and Montenegro.1250
     SRN  Serbian (Cyrillic)_Bosnia and Herzegovina.1251
     SRS  Serbian (Latin)_Bosnia and Herzegovina.1250
     SVE  Swedish_Sweden.1252
     SVF  Swedish_Finland.1252
     SWK  Swahili_Kenya.1252
     THA  Thai_Thailand.874
     TRK  Turkish_Turkey.1254
     TSN  Tswana_South Africa.1252
     TTT  Tatar_Russia.1251
     UKR  Ukrainian_Ukraine.1251
     URD  Urdu_Islamic Republic of Pakistan.1256
     USA  English_United States.1252
     UZB  Uzbek (Latin)_Uzbekistan.1254
     VIT  Vietnamese_Viet Nam.1258
     XHO  Xhosa_South Africa.1252
     ZHH  Chinese_Hong Kong S.A.R..950
     ZHI  Chinese_Singapore.936
     ZHM  Chinese_Macau S.A.R..950
     ZUL  Zulu_South Africa.1252
 */

/* Table from ISO 639 language code, optionally with country or script suffix,
   to English name.
   Keep in sync with the gl_locale_name_from_win32_LANGID function in
   localename.c!  */
struct table_entry
{
  const char *code;
  const char *english;
};
static const struct table_entry language_table[] =
  {
    { "af", "Afrikaans" },
    { "am", "Amharic" },
    { "ar", "Arabic" },
    { "arn", "Mapudungun" },
    { "as", "Assamese" },
    { "az@cyrillic", "Azeri (Cyrillic)" },
    { "az@latin", "Azeri (Latin)" },
    { "ba", "Bashkir" },
    { "be", "Belarusian" },
    { "ber", "Tamazight" },
    { "ber@arabic", "Tamazight (Arabic)" },
    { "ber@latin", "Tamazight (Latin)" },
    { "bg", "Bulgarian" },
    { "bin", "Edo" },
    { "bn", "Bengali" },
    { "bn_BD", "Bengali (Bangladesh)" },
    { "bn_IN", "Bengali (India)" },
    { "bnt", "Sutu" },
    { "bo", "Tibetan" },
    { "br", "Breton" },
    { "bs", "BSB" }, /* "Bosnian (Latin)" */
    { "bs@cyrillic", "BSC" }, /* Bosnian (Cyrillic) */
    { "ca", "Catalan" },
    { "chr", "Cherokee" },
    { "co", "Corsican" },
    { "cpe", "Hawaiian" },
    { "cs", "Czech" },
    { "cy", "Welsh" },
    { "da", "Danish" },
    { "de", "German" },
    { "dsb", "Lower Sorbian" },
    { "dv", "Divehi" },
    { "el", "Greek" },
    { "en", "English" },
    { "es", "Spanish" },
    { "et", "Estonian" },
    { "eu", "Basque" },
    { "fa", "Farsi" },
    { "ff", "Fulfulde" },
    { "fi", "Finnish" },
    { "fo", "Faroese" }, /* "Faeroese" does not work */
    { "fr", "French" },
    { "fy", "Frisian" },
    { "ga", "IRE" }, /* Gaelic (Ireland) */
    { "gd", "Gaelic (Scotland)" },
    { "gd", "Scottish Gaelic" },
    { "gl", "Galician" },
    { "gn", "Guarani" },
    { "gsw", "Alsatian" },
    { "gu", "Gujarati" },
    { "ha", "Hausa" },
    { "he", "Hebrew" },
    { "hi", "Hindi" },
    { "hr", "Croatian" },
    { "hsb", "Upper Sorbian" },
    { "hu", "Hungarian" },
    { "hy", "Armenian" },
    { "id", "Indonesian" },
    { "ig", "Igbo" },
    { "ii", "Yi" },
    { "is", "Icelandic" },
    { "it", "Italian" },
    { "iu", "IUK" }, /* Inuktitut */
    { "ja", "Japanese" },
    { "ka", "Georgian" },
    { "kk", "Kazakh" },
    { "kl", "Greenlandic" },
    { "km", "Cambodian" },
    { "km", "Khmer" },
    { "kn", "Kannada" },
    { "ko", "Korean" },
    { "kok", "Konkani" },
    { "kr", "Kanuri" },
    { "ks", "Kashmiri" },
    { "ks_IN", "Kashmiri_India" },
    { "ks_PK", "Kashmiri (Arabic)_Pakistan" },
    { "ky", "Kyrgyz" },
    { "la", "Latin" },
    { "lb", "Luxembourgish" },
    { "lo", "Lao" },
    { "lt", "Lithuanian" },
    { "lv", "Latvian" },
    { "mi", "Maori" },
    { "mk", "FYRO Macedonian" },
    { "mk", "Macedonian" },
    { "ml", "Malayalam" },
    { "mn", "Mongolian" },
    { "mni", "Manipuri" },
    { "moh", "Mohawk" },
    { "mr", "Marathi" },
    { "ms", "Malay" },
    { "mt", "Maltese" },
    { "my", "Burmese" },
    { "nb", "NOR" }, /* Norwegian Bokmål */
    { "ne", "Nepali" },
    { "nic", "Ibibio" },
    { "nl", "Dutch" },
    { "nn", "NON" }, /* Norwegian Nynorsk */
    { "no", "Norwegian" },
    { "nso", "Northern Sotho" },
    { "nso", "Sepedi" },
    { "oc", "Occitan" },
    { "om", "Oromo" },
    { "or", "Oriya" },
    { "pa", "Punjabi" },
    { "pap", "Papiamentu" },
    { "pl", "Polish" },
    { "prs", "Dari" },
    { "ps", "Pashto" },
    { "pt", "Portuguese" },
    { "qu", "Quechua" },
    { "qut", "K'iche'" },
    { "rm", "Romansh" },
    { "ro", "Romanian" },
    { "ru", "Russian" },
    { "rw", "Kinyarwanda" },
    { "sa", "Sanskrit" },
    { "sah", "Yakut" },
    { "sd", "Sindhi" },
    { "se", "Sami (Northern)" },
    { "se", "Northern Sami" },
    { "si", "Sinhalese" },
    { "sk", "Slovak" },
    { "sl", "Slovenian" },
    { "sma", "Sami (Southern)" },
    { "sma", "Southern Sami" },
    { "smj", "Sami (Lule)" },
    { "smj", "Lule Sami" },
    { "smn", "Sami (Inari)" },
    { "smn", "Inari Sami" },
    { "sms", "Sami (Skolt)" },
    { "sms", "Skolt Sami" },
    { "so", "Somali" },
    { "sq", "Albanian" },
    { "sr", "Serbian (Latin)" },
    { "sr@cyrillic", "SRB" }, /* Serbian (Cyrillic) */
    { "sv", "Swedish" },
    { "sw", "Swahili" },
    { "syr", "Syriac" },
    { "ta", "Tamil" },
    { "te", "Telugu" },
    { "tg", "Tajik" },
    { "th", "Thai" },
    { "ti", "Tigrinya" },
    { "tk", "Turkmen" },
    { "tl", "Filipino" },
    { "tn", "Tswana" },
    { "tr", "Turkish" },
    { "ts", "Tsonga" },
    { "tt", "Tatar" },
    { "ug", "Uighur" },
    { "uk", "Ukrainian" },
    { "ur", "Urdu" },
    { "uz", "Uzbek" },
    { "uz", "Uzbek (Latin)" },
    { "uz@cyrillic", "Uzbek (Cyrillic)" },
    { "ve", "Venda" },
    { "vi", "Vietnamese" },
    { "wen", "Sorbian" },
    { "wo", "Wolof" },
    { "xh", "Xhosa" },
    { "yi", "Yiddish" },
    { "yo", "Yoruba" },
    { "zh", "Chinese" },
    { "zu", "Zulu" }
  };

/* Table from ISO 3166 country code to English name.
   Keep in sync with the gl_locale_name_from_win32_LANGID function in
   localename.c!  */
static const struct table_entry country_table[] =
  {
    { "AE", "U.A.E." },
    { "AF", "Afghanistan" },
    { "AL", "Albania" },
    { "AM", "Armenia" },
    { "AN", "Netherlands Antilles" },
    { "AR", "Argentina" },
    { "AT", "Austria" },
    { "AU", "Australia" },
    { "AZ", "Azerbaijan" },
    { "BA", "Bosnia and Herzegovina" },
    { "BD", "Bangladesh" },
    { "BE", "Belgium" },
    { "BG", "Bulgaria" },
    { "BH", "Bahrain" },
    { "BN", "Brunei Darussalam" },
    { "BO", "Bolivia" },
    { "BR", "Brazil" },
    { "BT", "Bhutan" },
    { "BY", "Belarus" },
    { "BZ", "Belize" },
    { "CA", "Canada" },
    { "CG", "Congo" },
    { "CH", "Switzerland" },
    { "CI", "Cote d'Ivoire" },
    { "CL", "Chile" },
    { "CM", "Cameroon" },
    { "CN", "People's Republic of China" },
    { "CO", "Colombia" },
    { "CR", "Costa Rica" },
    { "CS", "Serbia and Montenegro" },
    { "CZ", "Czech Republic" },
    { "DE", "Germany" },
    { "DK", "Denmark" },
    { "DO", "Dominican Republic" },
    { "DZ", "Algeria" },
    { "EC", "Ecuador" },
    { "EE", "Estonia" },
    { "EG", "Egypt" },
    { "ER", "Eritrea" },
    { "ES", "Spain" },
    { "ET", "Ethiopia" },
    { "FI", "Finland" },
    { "FO", "Faroe Islands" },
    { "FR", "France" },
    { "GB", "United Kingdom" },
    { "GD", "Caribbean" },
    { "GE", "Georgia" },
    { "GL", "Greenland" },
    { "GR", "Greece" },
    { "GT", "Guatemala" },
    { "HK", "Hong Kong" },
    { "HK", "Hong Kong S.A.R." },
    { "HN", "Honduras" },
    { "HR", "Croatia" },
    { "HT", "Haiti" },
    { "HU", "Hungary" },
    { "ID", "Indonesia" },
    { "IE", "Ireland" },
    { "IL", "Israel" },
    { "IN", "India" },
    { "IQ", "Iraq" },
    { "IR", "Iran" },
    { "IS", "Iceland" },
    { "IT", "Italy" },
    { "JM", "Jamaica" },
    { "JO", "Jordan" },
    { "JP", "Japan" },
    { "KE", "Kenya" },
    { "KG", "Kyrgyzstan" },
    { "KH", "Cambodia" },
    { "KR", "South Korea" },
    { "KW", "Kuwait" },
    { "KZ", "Kazakhstan" },
    { "LA", "Laos" },
    { "LB", "Lebanon" },
    { "LI", "Liechtenstein" },
    { "LK", "Sri Lanka" },
    { "LT", "Lithuania" },
    { "LU", "Luxembourg" },
    { "LV", "Latvia" },
    { "LY", "Libya" },
    { "MA", "Morocco" },
    { "MC", "Principality of Monaco" },
    { "MD", "Moldava" },
    { "MD", "Moldova" },
    { "ME", "Montenegro" },
    { "MK", "Former Yugoslav Republic of Macedonia" },
    { "ML", "Mali" },
    { "MM", "Myanmar" },
    { "MN", "Mongolia" },
    { "MO", "Macau S.A.R." },
    { "MT", "Malta" },
    { "MV", "Maldives" },
    { "MX", "Mexico" },
    { "MY", "Malaysia" },
    { "NG", "Nigeria" },
    { "NI", "Nicaragua" },
    { "NL", "Netherlands" },
    { "NO", "Norway" },
    { "NP", "Nepal" },
    { "NZ", "New Zealand" },
    { "OM", "Oman" },
    { "PA", "Panama" },
    { "PE", "Peru" },
    { "PH", "Philippines" },
    { "PK", "Islamic Republic of Pakistan" },
    { "PL", "Poland" },
    { "PR", "Puerto Rico" },
    { "PT", "Portugal" },
    { "PY", "Paraguay" },
    { "QA", "Qatar" },
    { "RE", "Reunion" },
    { "RO", "Romania" },
    { "RS", "Serbia" },
    { "RU", "Russia" },
    { "RW", "Rwanda" },
    { "SA", "Saudi Arabia" },
    { "SE", "Sweden" },
    { "SG", "Singapore" },
    { "SI", "Slovenia" },
    { "SK", "Slovak" },
    { "SN", "Senegal" },
    { "SO", "Somalia" },
    { "SR", "Suriname" },
    { "SV", "El Salvador" },
    { "SY", "Syria" },
    { "TH", "Thailand" },
    { "TJ", "Tajikistan" },
    { "TM", "Turkmenistan" },
    { "TN", "Tunisia" },
    { "TR", "Turkey" },
    { "TT", "Trinidad and Tobago" },
    { "TW", "Taiwan" },
    { "TZ", "Tanzania" },
    { "UA", "Ukraine" },
    { "US", "United States" },
    { "UY", "Uruguay" },
    { "VA", "Vatican" },
    { "VE", "Venezuela" },
    { "VN", "Viet Nam" },
    { "YE", "Yemen" },
    { "ZA", "South Africa" },
    { "ZW", "Zimbabwe" }
  };

/* Given a string STRING, find the set of indices i such that TABLE[i].code is
   the given STRING.  It is a range [lo,hi-1].  */
typedef struct { size_t lo; size_t hi; } range_t;
static void
search (const struct table_entry *table, size_t table_size, const char *string,
        range_t *result)
{
  /* The table is sorted.  Perform a binary search.  */
  size_t hi = table_size;
  size_t lo = 0;
  while (lo < hi)
    {
      /* Invariant:
         for i < lo, strcmp (table[i].code, string) < 0,
         for i >= hi, strcmp (table[i].code, string) > 0.  */
      size_t mid = (hi + lo) >> 1; /* >= lo, < hi */
      int cmp = strcmp (table[mid].code, string);
      if (cmp < 0)
        lo = mid + 1;
      else if (cmp > 0)
        hi = mid;
      else
        {
          /* Found an i with
               strcmp (language_table[i].code, string) == 0.
             Find the entire interval of such i.  */
          {
            size_t i;

            for (i = mid; i > lo; )
              {
                i--;
                if (strcmp (table[i].code, string) < 0)
                  {
                    lo = i + 1;
                    break;
                  }
              }
          }
          {
            size_t i;

            for (i = mid + 1; i < hi; i++)
              {
                if (strcmp (table[i].code, string) > 0)
                  {
                    hi = i;
                    break;
                  }
              }
          }
          /* The set of i with
               strcmp (language_table[i].code, string) == 0
             is the interval [lo, hi-1].  */
          break;
        }
    }
  result->lo = lo;
  result->hi = hi;
}

/* Like setlocale, but accept also locale names in the form ll or ll_CC,
   where ll is an ISO 639 language code and CC is an ISO 3166 country code.  */
static char *
setlocale_unixlike (int category, const char *locale)
{
  char *result;
  char llCC_buf[64];
  char ll_buf[64];
  char CC_buf[64];

  /* The native Windows implementation of setlocale understands the special
     locale name "C", but not "POSIX".  Therefore map "POSIX" to "C".  */
  if (locale != NULL && strcmp (locale, "POSIX") == 0)
    locale = "C";

  /* First, try setlocale with the original argument unchanged.  */
  result = setlocale (category, locale);
  if (result != NULL)
    return result;

  /* Otherwise, assume the argument is in the form
       language[_territory][.codeset][@modifier]
     and try to map it using the tables.  */
  if (strlen (locale) < sizeof (llCC_buf))
    {
      /* Second try: Remove the codeset part.  */
      {
        const char *p = locale;
        char *q = llCC_buf;

        /* Copy the part before the dot.  */
        for (; *p != '\0' && *p != '.'; p++, q++)
          *q = *p;
        if (*p == '.')
          /* Skip the part up to the '@', if any.  */
          for (; *p != '\0' && *p != '@'; p++)
            ;
        /* Copy the part starting with '@', if any.  */
        for (; *p != '\0'; p++, q++)
          *q = *p;
        *q = '\0';
      }
      /* llCC_buf now contains
           language[_territory][@modifier]
       */
      if (strcmp (llCC_buf, locale) != 0)
        {
          result = setlocale (category, llCC_buf);
          if (result != NULL)
            return result;
        }
      /* Look it up in language_table.  */
      {
        range_t range;
        size_t i;

        search (language_table,
                sizeof (language_table) / sizeof (language_table[0]),
                llCC_buf,
                &range);

        for (i = range.lo; i < range.hi; i++)
          {
            /* Try the replacement in language_table[i].  */
            result = setlocale (category, language_table[i].english);
            if (result != NULL)
              return result;
          }
      }
      /* Split language[_territory][@modifier]
         into  ll_buf = language[@modifier]
         and   CC_buf = territory
       */
      {
        const char *underscore = strchr (llCC_buf, '_');
        if (underscore != NULL)
          {
            const char *territory_start = underscore + 1;
            const char *territory_end = strchr (territory_start, '@');
            if (territory_end == NULL)
              territory_end = territory_start + strlen (territory_start);

            memcpy (ll_buf, llCC_buf, underscore - llCC_buf);
            strcpy (ll_buf + (underscore - llCC_buf), territory_end);

            memcpy (CC_buf, territory_start, territory_end - territory_start);
            CC_buf[territory_end - territory_start] = '\0';

            {
              /* Look up ll_buf in language_table
                 and CC_buf in country_table.  */
              range_t language_range;

              search (language_table,
                      sizeof (language_table) / sizeof (language_table[0]),
                      ll_buf,
                      &language_range);
              if (language_range.lo < language_range.hi)
                {
                  range_t country_range;

                  search (country_table,
                          sizeof (country_table) / sizeof (country_table[0]),
                          CC_buf,
                          &country_range);
                  if (country_range.lo < country_range.hi)
                    {
                      size_t i;
                      size_t j;

                      for (i = language_range.lo; i < language_range.hi; i++)
                        for (j = country_range.lo; j < country_range.hi; j++)
                          {
                            /* Concatenate the replacements.  */
                            const char *part1 = language_table[i].english;
                            size_t part1_len = strlen (part1);
                            const char *part2 = country_table[j].english;
                            size_t part2_len = strlen (part2) + 1;
                            char buf[64+64];

                            if (!(part1_len + 1 + part2_len <= sizeof (buf)))
                              abort ();
                            memcpy (buf, part1, part1_len);
                            buf[part1_len] = '_';
                            memcpy (buf + part1_len + 1, part2, part2_len);

                            /* Try the concatenated replacements.  */
                            result = setlocale (category, buf);
                            if (result != NULL)
                              return result;
                          }
                    }

                  /* Try omitting the country entirely.  This may set a locale
                     corresponding to the wrong country, but is better than
                     failing entirely.  */
                  {
                    size_t i;

                    for (i = language_range.lo; i < language_range.hi; i++)
                      {
                        /* Try only the language replacement.  */
                        result =
                          setlocale (category, language_table[i].english);
                        if (result != NULL)
                          return result;
                      }
                  }
                }
            }
          }
      }
    }

  /* Failed.  */
  return NULL;
}

# elif defined __ANDROID__

/* Like setlocale, but accept also the locale names "C" and "POSIX".  */
static char *
setlocale_unixlike (int category, const char *locale)
{
  char *result = setlocale (category, locale);
  if (result == NULL)
    switch (category)
      {
      case LC_CTYPE:
      case LC_NUMERIC:
      case LC_TIME:
      case LC_COLLATE:
      case LC_MONETARY:
      case LC_MESSAGES:
      case LC_ALL:
      case LC_PAPER:
      case LC_NAME:
      case LC_ADDRESS:
      case LC_TELEPHONE:
      case LC_MEASUREMENT:
        if (locale == NULL
            || strcmp (locale, "C") == 0 || strcmp (locale, "POSIX") == 0)
          result = (char *) "C";
        break;
      default:
        break;
      }
  return result;
}
#  define setlocale setlocale_unixlike

# else
#  define setlocale_unixlike setlocale
# endif

# if LC_MESSAGES == 1729

/* The system does not store an LC_MESSAGES locale category.  Do it here.  */
static char lc_messages_name[64] = "C";

/* Like setlocale, but support also LC_MESSAGES.  */
static char *
setlocale_single (int category, const char *locale)
{
  if (category == LC_MESSAGES)
    {
      if (locale != NULL)
        {
          lc_messages_name[sizeof (lc_messages_name) - 1] = '\0';
          strncpy (lc_messages_name, locale, sizeof (lc_messages_name) - 1);
        }
      return lc_messages_name;
    }
  else
    return setlocale_unixlike (category, locale);
}

# else
#  define setlocale_single setlocale_unixlike
# endif

# if defined __APPLE__ && defined __MACH__

/* Mapping from language to main territory where that language is spoken.  */
static char const locales_with_principal_territory[][6 + 1] =
  {
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
    "pbb_CO",   /* Páez         Colombia */
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
    "sm_WS",    /* Samoan       Samoa */
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
    "ti_ER",    /* Tigrinya     Eritrea */
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

/* Compare just the language part of two locale names.  */
static int
langcmp (const char *locale1, const char *locale2)
{
  size_t locale1_len;
  size_t locale2_len;
  int cmp;

  {
    const char *locale1_end = strchr (locale1, '_');
    if (locale1_end != NULL)
      locale1_len = locale1_end - locale1;
    else
      locale1_len = strlen (locale1);
  }
  {
    const char *locale2_end = strchr (locale2, '_');
    if (locale2_end != NULL)
      locale2_len = locale2_end - locale2;
    else
      locale2_len = strlen (locale2);
  }

  if (locale1_len < locale2_len)
    {
      cmp = memcmp (locale1, locale2, locale1_len);
      if (cmp == 0)
        cmp = -1;
    }
  else
    {
      cmp = memcmp (locale1, locale2, locale2_len);
      if (locale1_len > locale2_len && cmp == 0)
        cmp = 1;
    }

  return cmp;
}

/* Given a locale name, return the main locale with the same language,
   or NULL if not found.
   For example: "fr_DE" -> "fr_FR".  */
static const char *
get_main_locale_with_same_language (const char *locale)
{
#  define table locales_with_principal_territory
  /* The table is sorted.  Perform a binary search.  */
  size_t hi = sizeof (table) / sizeof (table[0]);
  size_t lo = 0;
  while (lo < hi)
    {
      /* Invariant:
         for i < lo, langcmp (table[i], locale) < 0,
         for i >= hi, langcmp (table[i], locale) > 0.  */
      size_t mid = (hi + lo) >> 1; /* >= lo, < hi */
      int cmp = langcmp (table[mid], locale);
      if (cmp < 0)
        lo = mid + 1;
      else if (cmp > 0)
        hi = mid;
      else
        {
          /* Found an i with
               langcmp (language_table[i], locale) == 0.
             Verify that it is the only such i.  */
          if (mid > lo && langcmp (table[mid - 1], locale) >= 0)
            abort ();
          if (mid + 1 < hi && langcmp (table[mid + 1], locale) <= 0)
            abort ();
          return table[mid];
        }
    }
#  undef table
  return NULL;
}

/* Mapping from territory to main language that is spoken in that territory.  */
static char const locales_with_principal_language[][6 + 1] =
  {
    /* This is based on the set of existing locales in glibc, with duplicates
       removed, and on the Wikipedia pages named "Languages of <territory>".
       If in doubt, use the locale that exists in macOS.  For example, the only
       "*_IN" locale in macOS 10.13 is "hi_IN", so use that.  */
    /* A useful shell function for producing a line of this table is:
         func_line ()
         {
           # Usage: func_line ll_CC
           ll=`echo "$1" | sed -e 's|_.*||'`
           cc=`echo "$1" | sed -e 's|^.*_||'`
           llx=`sed -n -e "s|^${ll} ||p" < gettext-tools/doc/ISO_639`
           ccx=`expand gettext-tools/doc/ISO_3166 | sed -n -e "s|^${cc}  *||p"`
           echo "    \"$1\",    /$X* ${llx} ${ccx} *$X/"
         }
     */
              /* Main language  Territory */
    "ca_AD",    /* Catalan      Andorra */
    "ar_AE",    /* Arabic       United Arab Emirates */
    "ps_AF",    /* Pashto       Afghanistan */
    "en_AG",    /* English      Antigua and Barbuda */
    "sq_AL",    /* Albanian     Albania */
    "hy_AM",    /* Armenian     Armenia */
    "pap_AN",   /* Papiamento   Netherlands Antilles - this line can be removed in 2018 */
    "pt_AO",    /* Portuguese   Angola */
    "es_AR",    /* Spanish      Argentina */
    "de_AT",    /* German       Austria */
    "en_AU",    /* English      Australia */
    /* Aruba has two official languages: "nl_AW", "pap_AW".  */
    "az_AZ",    /* Azerbaijani  Azerbaijan */
    "bs_BA",    /* Bosnian      Bosnia */
    "bn_BD",    /* Bengali      Bangladesh */
    "nl_BE",    /* Dutch        Belgium */
    "fr_BF",    /* French       Burkina Faso */
    "bg_BG",    /* Bulgarian    Bulgaria */
    "ar_BH",    /* Arabic       Bahrain */
    "rn_BI",    /* Kirundi      Burundi */
    "fr_BJ",    /* French       Benin */
    "es_BO",    /* Spanish      Bolivia */
    "pt_BR",    /* Portuguese   Brazil */
    "dz_BT",    /* Dzongkha     Bhutan */
    "en_BW",    /* English      Botswana */
    "be_BY",    /* Belarusian   Belarus */
    "en_CA",    /* English      Canada */
    "fr_CD",    /* French       Democratic Republic of Congo */
    "sg_CF",    /* Sango        Central African Republic */
    "de_CH",    /* German       Switzerland */
    "es_CL",    /* Spanish      Chile */
    "zh_CN",    /* Chinese      China */
    "es_CO",    /* Spanish      Colombia */
    "es_CR",    /* Spanish      Costa Rica */
    "es_CU",    /* Spanish      Cuba */
    /* Curaçao has three official languages: "nl_CW", "pap_CW", "en_CW".  */
    "el_CY",    /* Greek        Cyprus */
    "cs_CZ",    /* Czech        Czech Republic */
    "de_DE",    /* German       Germany */
    /* Djibouti has two official languages: "ar_DJ" and "fr_DJ".  */
    "da_DK",    /* Danish       Denmark */
    "es_DO",    /* Spanish      Dominican Republic */
    "ar_DZ",    /* Arabic       Algeria */
    "es_EC",    /* Spanish      Ecuador */
    "et_EE",    /* Estonian     Estonia */
    "ar_EG",    /* Arabic       Egypt */
    "ti_ER",    /* Tigrinya     Eritrea */
    "es_ES",    /* Spanish      Spain */
    "am_ET",    /* Amharic      Ethiopia */
    "fi_FI",    /* Finnish      Finland */
    /* Fiji has three official languages: "en_FJ", "fj_FJ", "hif_FJ".  */
    "fo_FO",    /* Faroese      Faeroe Islands */
    "fr_FR",    /* French       France */
    "en_GB",    /* English      Britain */
    "ka_GE",    /* Georgian     Georgia */
    "en_GH",    /* English      Ghana */
    "kl_GL",    /* Kalaallisut  Greenland */
    "fr_GN",    /* French       Guinea */
    "el_GR",    /* Greek        Greece */
    "es_GT",    /* Spanish      Guatemala */
    "zh_HK",    /* Chinese      Hong Kong */
    "es_HN",    /* Spanish      Honduras */
    "hr_HR",    /* Croatian     Croatia */
    "ht_HT",    /* Haitian      Haiti */
    "hu_HU",    /* Hungarian    Hungary */
    "id_ID",    /* Indonesian   Indonesia */
    "en_IE",    /* English      Ireland */
    "he_IL",    /* Hebrew       Israel */
    "hi_IN",    /* Hindi        India */
    "ar_IQ",    /* Arabic       Iraq */
    "fa_IR",    /* Persian      Iran */
    "is_IS",    /* Icelandic    Iceland */
    "it_IT",    /* Italian      Italy */
    "ar_JO",    /* Arabic       Jordan */
    "ja_JP",    /* Japanese     Japan */
    "sw_KE",    /* Swahili      Kenya */
    "ky_KG",    /* Kyrgyz       Kyrgyzstan */
    "km_KH",    /* Central Khmer        Cambodia */
    "ko_KR",    /* Korean       Korea (South) */
    "ar_KW",    /* Arabic       Kuwait */
    "kk_KZ",    /* Kazakh       Kazakhstan */
    "lo_LA",    /* Laotian      Laos */
    "ar_LB",    /* Arabic       Lebanon */
    "de_LI",    /* German       Liechtenstein */
    "si_LK",    /* Sinhala      Sri Lanka */
    "lt_LT",    /* Lithuanian   Lithuania */
    /* Luxembourg has three official languages: "lb_LU", "fr_LU", "de_LU".  */
    "lv_LV",    /* Latvian      Latvia */
    "ar_LY",    /* Arabic       Libya */
    "ar_MA",    /* Arabic       Morocco */
    "sr_ME",    /* Serbian      Montenegro */
    "mg_MG",    /* Malagasy     Madagascar */
    "mk_MK",    /* Macedonian   North Macedonia */
    "fr_ML",    /* French       Mali */
    "my_MM",    /* Burmese      Myanmar */
    "mn_MN",    /* Mongolian    Mongolia */
    "mt_MT",    /* Maltese      Malta */
    "mfe_MU",   /* Mauritian Creole     Mauritius */
    "dv_MV",    /* Divehi       Maldives */
    "ny_MW",    /* Chichewa     Malawi */
    "es_MX",    /* Spanish      Mexico */
    "ms_MY",    /* Malay        Malaysia */
    "en_NG",    /* English      Nigeria */
    "es_NI",    /* Spanish      Nicaragua */
    "nl_NL",    /* Dutch        Netherlands */
    "no_NO",    /* Norwegian    Norway */
    "ne_NP",    /* Nepali       Nepal */
    "na_NR",    /* Nauru        Nauru */
    "niu_NU",   /* Niuean       Niue */
    "en_NZ",    /* English      New Zealand */
    "ar_OM",    /* Arabic       Oman */
    "es_PA",    /* Spanish      Panama */
    "es_PE",    /* Spanish      Peru */
    "tpi_PG",   /* Tok Pisin    Papua New Guinea */
    "fil_PH",   /* Filipino     Philippines */
    "pa_PK",    /* Punjabi      Pakistan */
    "pl_PL",    /* Polish       Poland */
    "es_PR",    /* Spanish      Puerto Rico */
    "pt_PT",    /* Portuguese   Portugal */
    "es_PY",    /* Spanish      Paraguay */
    "ar_QA",    /* Arabic       Qatar */
    "ro_RO",    /* Romanian     Romania */
    "sr_RS",    /* Serbian      Serbia */
    "ru_RU",    /* Russian      Russia */
    "rw_RW",    /* Kinyarwanda  Rwanda */
    "ar_SA",    /* Arabic       Saudi Arabia */
    "en_SC",    /* English      Seychelles */
    "ar_SD",    /* Arabic       Sudan */
    "sv_SE",    /* Swedish      Sweden */
    "en_SG",    /* English      Singapore */
    "sl_SI",    /* Slovenian    Slovenia */
    "sk_SK",    /* Slovak       Slovakia */
    "en_SL",    /* English      Sierra Leone */
    "fr_SN",    /* French       Senegal */
    "so_SO",    /* Somali       Somalia */
    "ar_SS",    /* Arabic       South Sudan */
    "es_SV",    /* Spanish      El Salvador */
    "ar_SY",    /* Arabic       Syria */
    "th_TH",    /* Thai         Thailand */
    "tg_TJ",    /* Tajik        Tajikistan */
    "tk_TM",    /* Turkmen      Turkmenistan */
    "ar_TN",    /* Arabic       Tunisia */
    "to_TO",    /* Tonga        Tonga */
    "tr_TR",    /* Turkish      Türkiye */
    "zh_TW",    /* Chinese      Taiwan */
    "sw_TZ",    /* Swahili      Tanzania */
    "uk_UA",    /* Ukrainian    Ukraine */
    "lg_UG",    /* Ganda        Uganda */
    "en_US",    /* English      United States of America */
    "es_UY",    /* Spanish      Uruguay */
    "uz_UZ",    /* Uzbek        Uzbekistan */
    "es_VE",    /* Spanish      Venezuela */
    "vi_VN",    /* Vietnamese   Vietnam */
    "bi_VU",    /* Bislama      Vanuatu */
    "sm_WS",    /* Samoan       Samoa */
    "ar_YE",    /* Arabic       Yemen */
    "en_ZA",    /* English      South Africa */
    "en_ZM",    /* English      Zambia */
    "en_ZW"     /* English      Zimbabwe */
  };

/* Compare just the territory part of two locale names.  */
static int
terrcmp (const char *locale1, const char *locale2)
{
  const char *territory1 = strrchr (locale1, '_') + 1;
  const char *territory2 = strrchr (locale2, '_') + 1;

  return strcmp (territory1, territory2);
}

/* Given a locale name, return the locale corresponding to the main language
   with the same territory, or NULL if not found.
   For example: "fr_DE" -> "de_DE".  */
static const char *
get_main_locale_with_same_territory (const char *locale)
{
  if (strrchr (locale, '_') != NULL)
    {
#  define table locales_with_principal_language
      /* The table is sorted.  Perform a binary search.  */
      size_t hi = sizeof (table) / sizeof (table[0]);
      size_t lo = 0;
      while (lo < hi)
        {
          /* Invariant:
             for i < lo, terrcmp (table[i], locale) < 0,
             for i >= hi, terrcmp (table[i], locale) > 0.  */
          size_t mid = (hi + lo) >> 1; /* >= lo, < hi */
          int cmp = terrcmp (table[mid], locale);
          if (cmp < 0)
            lo = mid + 1;
          else if (cmp > 0)
            hi = mid;
          else
            {
              /* Found an i with
                   terrcmp (language_table[i], locale) == 0.
                 Verify that it is the only such i.  */
              if (mid > lo && terrcmp (table[mid - 1], locale) >= 0)
                abort ();
              if (mid + 1 < hi && terrcmp (table[mid + 1], locale) <= 0)
                abort ();
              return table[mid];
            }
        }
#  undef table
    }
  return NULL;
}

# endif

DLL_EXPORTED
char *
libintl_setlocale (int category, const char *locale)
{
  if (locale != NULL && locale[0] == '\0')
    {
      /* A request to the set the current locale to the default locale.  */
      if (category == LC_ALL)
        {
          /* Set LC_CTYPE first.  Then the other categories.  */
          static int const categories[] =
            {
              LC_CTYPE,
              LC_NUMERIC,
              LC_TIME,
              LC_COLLATE,
              LC_MONETARY,
              LC_MESSAGES
            };
          char *saved_locale;
          const char *base_name;
          unsigned int i;

          /* Back up the old locale, in case one of the steps fails.  */
          saved_locale = setlocale (LC_ALL, NULL);
          if (saved_locale == NULL)
            return NULL;
          saved_locale = strdup (saved_locale);
          if (saved_locale == NULL)
            return NULL;

          /* Set LC_CTYPE category.  Set all other categories (except possibly
             LC_MESSAGES) to the same value in the same call; this is likely to
             save calls.  */
          base_name =
            gl_locale_name_environ (LC_CTYPE, category_to_name (LC_CTYPE));
          if (base_name == NULL)
            base_name = gl_locale_name_default ();

          if (setlocale_unixlike (LC_ALL, base_name) != NULL)
            {
              /* LC_CTYPE category already set.  */
              i = 1;
            }
          else
            {
              /* On Mac OS X, "UTF-8" is a valid locale name for LC_CTYPE but
                 not for LC_ALL.  Therefore this call may fail.  So, try
                 another base_name.  */
              base_name = "C";
              if (setlocale_unixlike (LC_ALL, base_name) == NULL)
                goto fail;
              i = 0;
            }
# if defined _WIN32 && ! defined __CYGWIN__
          /* On native Windows, setlocale(LC_ALL,...) may succeed but set the
             LC_CTYPE category to an invalid value ("C") when it does not
             support the specified encoding.  Report a failure instead.  */
          if (strchr (base_name, '.') != NULL
              && strcmp (setlocale (LC_CTYPE, NULL), "C") == 0)
            goto fail;
# endif

          for (; i < sizeof (categories) / sizeof (categories[0]); i++)
            {
              int cat = categories[i];
              const char *name;

              name = gl_locale_name_environ (cat, category_to_name (cat));
              if (name == NULL)
                name = gl_locale_name_default ();

              /* If name is the same as base_name, it has already been set
                 through the setlocale call before the loop.  */
              if (strcmp (name, base_name) != 0
# if LC_MESSAGES == 1729
                  || cat == LC_MESSAGES
# endif
                 )
                if (setlocale_single (cat, name) == NULL)
# if defined __APPLE__ && defined __MACH__
                  {
                    /* On Mac OS X 10.13, some locales can be set through
                       System Preferences > Language & Region, that are not
                       supported by libc.  The system's setlocale() falls
                       back to "C" for these locale categories.  We can do
                       better, by trying an existing locale with the same
                       language or an existing locale with the same territory.
                       If we can't, print a warning, to limit user
                       expectations.  */
                    int warn = 0;

                    if (cat == LC_CTYPE)
                      warn = (setlocale_single (cat, "UTF-8") == NULL);
                    else if (cat == LC_MESSAGES)
                      {
#  if HAVE_CFLOCALECOPYPREFERREDLANGUAGES || HAVE_CFPREFERENCESCOPYAPPVALUE /* MacOS X 10.4 or newer */
                        /* Take the primary language preference.  */
#   if HAVE_CFLOCALECOPYPREFERREDLANGUAGES /* MacOS X 10.5 or newer */
                        CFArrayRef prefArray = CFLocaleCopyPreferredLanguages ();
#   elif HAVE_CFPREFERENCESCOPYAPPVALUE /* MacOS X 10.4 or newer */
                        CFTypeRef preferences =
                          CFPreferencesCopyAppValue (CFSTR ("AppleLanguages"),
                                                     kCFPreferencesCurrentApplication);
                        if (preferences != NULL
                            && CFGetTypeID (preferences) == CFArrayGetTypeID ())
                          {
                            CFArrayRef prefArray = (CFArrayRef)preferences;
#   endif
                            int n = CFArrayGetCount (prefArray);
                            if (n > 0)
                              {
                                char buf[256];
                                CFTypeRef element = CFArrayGetValueAtIndex (prefArray, 0);
                                if (element != NULL
                                    && CFGetTypeID (element) == CFStringGetTypeID ()
                                    && CFStringGetCString ((CFStringRef)element,
                                                           buf, sizeof (buf),
                                                           kCFStringEncodingASCII))
                                  {
                                    /* Remove the country.
                                       E.g. "zh-Hans-DE" -> "zh-Hans".  */
                                    char *last_minus = strrchr (buf, '-');
                                    if (last_minus != NULL)
                                      *last_minus = '\0';

                                    /* Convert to Unix locale name.
                                       E.g. "zh-Hans" -> "zh_CN".  */
                                    gl_locale_name_canonicalize (buf);

                                    /* Try setlocale with this value.  */
                                    if (setlocale_single (cat, buf) == NULL)
                                      {
                                        const char *last_try =
                                          get_main_locale_with_same_language (buf);

                                        if (last_try == NULL
                                            || setlocale_single (cat, last_try) == NULL)
                                          warn = 1;
                                      }
                                  }
                              }
#   if HAVE_CFLOCALECOPYPREFERREDLANGUAGES /* MacOS X 10.5 or newer */
                        CFRelease (prefArray);
#   elif HAVE_CFPREFERENCESCOPYAPPVALUE /* MacOS X 10.4 or newer */
                          }
#   endif
#  else
                        const char *last_try =
                          get_main_locale_with_same_language (name);

                        if (last_try == NULL
                            || setlocale_single (cat, last_try) == NULL)
                          warn = 1;
#  endif
                      }
                    else
                      {
                        /* For LC_NUMERIC, the application should use the locale
                           properties kCFLocaleDecimalSeparator,
                           kCFLocaleGroupingSeparator.
                           For LC_TIME, the application should use the locale
                           property kCFLocaleCalendarIdentifier.
                           For LC_COLLATE, the application should use the locale
                           properties kCFLocaleCollationIdentifier,
                           kCFLocaleCollatorIdentifier.
                           For LC_MONETARY, the applicationshould use the locale
                           properties kCFLocaleCurrencySymbol,
                           kCFLocaleCurrencyCode.
                           But since most applications don't have macOS specific
                           code like this, try an existing locale with the same
                           territory.  */
                        const char *last_try =
                          get_main_locale_with_same_territory (name);

                        if (last_try == NULL
                            || setlocale_single (cat, last_try) == NULL)
                          warn = 1;
                      }

                    if (warn)
                      {
                        /* Warn only if the environment variable
                           SETLOCALE_VERBOSE is set.  Otherwise these warnings
                           are just annoyances, since normal users won't invoke
                           'localedef'.  */
                        const char *verbose = getenv ("SETLOCALE_VERBOSE");
                        if (verbose != NULL && verbose[0] != '\0')
                          fprintf (stderr,
                                   "Warning: Failed to set locale category %s to %s.\n",
                                   category_to_name (cat), name);
                      }
                  }
# else
                  goto fail;
# endif
            }

          /* All steps were successful.  */
          ++_nl_msg_cat_cntr;
          free (saved_locale);
          return setlocale (LC_ALL, NULL);

        fail:
          if (saved_locale[0] != '\0') /* don't risk an endless recursion */
            setlocale (LC_ALL, saved_locale);
          free (saved_locale);
          return NULL;
        }
      else
        {
          char *result;
          const char *name =
            gl_locale_name_environ (category, category_to_name (category));
          if (name == NULL)
            name = gl_locale_name_default ();

          result = setlocale_single (category, name);
          if (result != NULL)
            ++_nl_msg_cat_cntr;
          return result;
        }
    }
  else
    {
# if defined _WIN32 && ! defined __CYGWIN__
      if (category == LC_ALL && locale != NULL && strchr (locale, '.') != NULL)
        {
          char *saved_locale;

          /* Back up the old locale.  */
          saved_locale = setlocale (LC_ALL, NULL);
          if (saved_locale == NULL)
            return NULL;
          saved_locale = strdup (saved_locale);
          if (saved_locale == NULL)
            return NULL;

          if (setlocale_unixlike (LC_ALL, locale) == NULL)
            {
              free (saved_locale);
              return NULL;
            }

          /* On native Windows, setlocale(LC_ALL,...) may succeed but set the
             LC_CTYPE category to an invalid value ("C") when it does not
             support the specified encoding.  Report a failure instead.  */
          if (strcmp (setlocale (LC_CTYPE, NULL), "C") == 0)
            {
              if (saved_locale[0] != '\0') /* don't risk an endless recursion */
                setlocale (LC_ALL, saved_locale);
              free (saved_locale);
              return NULL;
            }

          /* It was really successful.  */
          ++_nl_msg_cat_cntr;
          free (saved_locale);
          return setlocale (LC_ALL, NULL);
        }
      else
# endif
        {
          char *result = setlocale_single (category, locale);
          if (result != NULL)
            ++_nl_msg_cat_cntr;
          return result;
        }
    }
}

# if HAVE_NEWLOCALE

DLL_EXPORTED
locale_t
libintl_newlocale (int category_mask, const char *locale, locale_t base)
{
  if (category_mask != 0 && locale != NULL && locale[0] == '\0')
    {
      /* A request to construct a locale_t object that refers to the default
         locale.  */

      /* Set LC_CTYPE first.  Then the other categories.  */
      static struct { int cat; int mask; } const categories[] =
        {
          { LC_CTYPE,    LC_CTYPE_MASK },
          { LC_NUMERIC,  LC_NUMERIC_MASK },
          { LC_TIME,     LC_TIME_MASK },
          { LC_COLLATE,  LC_COLLATE_MASK },
          { LC_MONETARY, LC_MONETARY_MASK },
          { LC_MESSAGES, LC_MESSAGES_MASK }
        };

      locale_t orig_base = base;

      if ((LC_ALL_MASK & ~category_mask) == 0)
        {
          const char *base_name;
          unsigned int i;

          /* Set LC_CTYPE category.  Set all other categories (except possibly
             LC_MESSAGES) to the same value in the same call; this is likely to
             save calls.  */
          base_name =
            gl_locale_name_environ (LC_CTYPE, category_to_name (LC_CTYPE));
          if (base_name == NULL)
            base_name = gl_locale_name_default ();

          base = newlocale (LC_ALL_MASK, base_name, base);
          if (base == NULL)
            return NULL;

          for (i = 1; i < sizeof (categories) / sizeof (categories[0]); i++)
            {
              int category = categories[i].cat;
              int category_mask = categories[i].mask;
              const char *name;

              name =
                gl_locale_name_environ (category, category_to_name (category));
              if (name == NULL)
                name = gl_locale_name_default ();

              /* If name is the same as base_name, it has already been set
                 through the setlocale call before the loop.  */
              if (strcmp (name, base_name) != 0)
                {
                  locale_t copy = newlocale (category_mask, name, base);
                  if (copy == NULL)
                    goto fail;
                  /* No need to call freelocale (base) if copy != base; the
                     newlocale function already takes care of doing it.  */
                  base = copy;
                }
            }
        }
      else
        {
          unsigned int i;

          for (i = 0; i < sizeof (categories) / sizeof (categories[0]); i++)
            {
              int cat_mask = categories[i].mask;

              if ((category_mask & cat_mask) != 0)
                {
                  int cat = categories[i].cat;
                  const char *name;
                  locale_t copy;

                  name = gl_locale_name_environ (cat, category_to_name (cat));
                  if (name == NULL)
                    name = gl_locale_name_default ();

                  copy = newlocale (cat_mask, name, base);
                  if (copy == NULL)
                    goto fail;
                  /* No need to call freelocale (base) if copy != base; the
                     newlocale function already takes care of doing it.  */
                  base = copy;
                }
            }
        }

      /* All steps were successful.  */
      return base;

    fail:
      if (base != NULL && orig_base == NULL)
        {
          int saved_errno = errno;
          freelocale (base);
          errno = saved_errno;
        }
      return NULL;
    }
  else
    return newlocale (category_mask, locale, base);
}

# endif

#endif

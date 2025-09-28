/* ANSI-C code produced by gperf version 3.2 */
/* Command-line: gperf -m 10 ./iconv_open-zos.gperf  */
/* Computed positions: -k'4,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 17 "./iconv_open-zos.gperf"
struct mapping { int standard_name; const char vendor_name[10 + 1]; };

#define TOTAL_KEYWORDS 49
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 11
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 64
/* maximum key range = 62, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
mapping_hash (register const char *str, register size_t len)
{
  static const unsigned char asso_values[] =
    {
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 16, 38,
      14,  1, 32, 22, 29,  3,  0,  7, 40,  2,
       5, 18, 23, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65,  0, 65,  0, 65, 65, 65,  0,
      43, 65,  1, 65, 65,  8, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[3]+6];
#if defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang_major__ && defined __clang_minor__ && __clang_major__ + (__clang_minor__ >= 9) > 3))
      [[fallthrough]];
#elif defined __GNUC__ && __GNUC__ >= 7
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 3:
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

struct stringpool_t
  {
    char stringpool_str3[sizeof("GBK")];
    char stringpool_str5[sizeof("ASCII")];
    char stringpool_str7[sizeof("CP1253")];
    char stringpool_str8[sizeof("EUC-KR")];
    char stringpool_str9[sizeof("CP1257")];
    char stringpool_str10[sizeof("CP857")];
    char stringpool_str11[sizeof("ISO-8859-8")];
    char stringpool_str12[sizeof("ISO-8859-3")];
    char stringpool_str13[sizeof("ISO-8859-13")];
    char stringpool_str14[sizeof("ISO-8859-7")];
    char stringpool_str15[sizeof("CP437")];
    char stringpool_str16[sizeof("CP1129")];
    char stringpool_str17[sizeof("CP869")];
    char stringpool_str18[sizeof("ISO-8859-9")];
    char stringpool_str19[sizeof("CP922")];
    char stringpool_str20[sizeof("CP1252")];
    char stringpool_str21[sizeof("CP852")];
    char stringpool_str22[sizeof("CP1250")];
    char stringpool_str23[sizeof("CP850")];
    char stringpool_str24[sizeof("CP862")];
    char stringpool_str25[sizeof("ISO-8859-2")];
    char stringpool_str26[sizeof("CP932")];
    char stringpool_str27[sizeof("GB2312")];
    char stringpool_str28[sizeof("CP1255")];
    char stringpool_str29[sizeof("CP855")];
    char stringpool_str30[sizeof("KOI8-R")];
    char stringpool_str31[sizeof("CP1125")];
    char stringpool_str32[sizeof("CP865")];
    char stringpool_str33[sizeof("ISO-8859-5")];
    char stringpool_str34[sizeof("ISO-8859-15")];
    char stringpool_str35[sizeof("CP1256")];
    char stringpool_str36[sizeof("CP856")];
    char stringpool_str37[sizeof("KOI8-U")];
    char stringpool_str38[sizeof("CP1254")];
    char stringpool_str39[sizeof("CP866")];
    char stringpool_str40[sizeof("ISO-8859-6")];
    char stringpool_str41[sizeof("CP1124")];
    char stringpool_str42[sizeof("CP864")];
    char stringpool_str43[sizeof("ISO-8859-4")];
    char stringpool_str44[sizeof("CP1251")];
    char stringpool_str45[sizeof("CP775")];
    char stringpool_str46[sizeof("CP943")];
    char stringpool_str47[sizeof("CP1131")];
    char stringpool_str48[sizeof("CP861")];
    char stringpool_str49[sizeof("ISO-8859-1")];
    char stringpool_str50[sizeof("EUC-JP")];
    char stringpool_str52[sizeof("CP949")];
    char stringpool_str55[sizeof("CP874")];
    char stringpool_str64[sizeof("CP1046")];
  };
static const struct stringpool_t stringpool_contents =
  {
    "GBK",
    "ASCII",
    "CP1253",
    "EUC-KR",
    "CP1257",
    "CP857",
    "ISO-8859-8",
    "ISO-8859-3",
    "ISO-8859-13",
    "ISO-8859-7",
    "CP437",
    "CP1129",
    "CP869",
    "ISO-8859-9",
    "CP922",
    "CP1252",
    "CP852",
    "CP1250",
    "CP850",
    "CP862",
    "ISO-8859-2",
    "CP932",
    "GB2312",
    "CP1255",
    "CP855",
    "KOI8-R",
    "CP1125",
    "CP865",
    "ISO-8859-5",
    "ISO-8859-15",
    "CP1256",
    "CP856",
    "KOI8-U",
    "CP1254",
    "CP866",
    "ISO-8859-6",
    "CP1124",
    "CP864",
    "ISO-8859-4",
    "CP1251",
    "CP775",
    "CP943",
    "CP1131",
    "CP861",
    "ISO-8859-1",
    "EUC-JP",
    "CP949",
    "CP874",
    "CP1046"
  };
#define stringpool ((const char *) &stringpool_contents)

static const struct mapping mappings[] =
  {
    {-1}, {-1}, {-1},
#line 76 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str3, "IBM-1386"},
    {-1},
#line 28 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str5, "00367"},
    {-1},
#line 68 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str7, "IBM-5349"},
#line 75 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str8, "IBM-eucKR"},
#line 72 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str9, "09449"},
#line 48 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str10, "00857"},
#line 36 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str11, "05012"},
#line 31 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str12, "00913"},
#line 38 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str13, "ISO8859-13"},
#line 35 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str14, "ISO8859-7"},
#line 42 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str15, "IBM-437"},
#line 63 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str16, "01129"},
#line 54 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str17, "IBM-869"},
#line 37 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str18, "ISO8859-9"},
#line 56 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str19, "IBM-922"},
#line 67 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str20, "IBM-5348"},
#line 45 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str21, "IBM-852"},
#line 65 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str22, "IBM-5346"},
#line 44 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str23, "09042"},
#line 50 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str24, "IBM-862"},
#line 30 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str25, "ISO8859-2"},
#line 57 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str26, "IBM-943"},
#line 73 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str27, "IBM-eucCN"},
#line 70 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str28, "09447"},
#line 46 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str29, "13143"},
#line 40 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str30, "00878"},
#line 62 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str31, "IBM-1125"},
#line 52 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str32, "00865"},
#line 33 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str33, "ISO8859-5"},
#line 39 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str34, "ISO8859-15"},
#line 71 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str35, "09448"},
#line 47 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str36, "IBM-856"},
#line 41 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str37, "01168"},
#line 69 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str38, "IBM-5350"},
#line 53 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str39, "04962"},
#line 34 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str40, "ISO8859-6"},
#line 61 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str41, "IBM-1124"},
#line 51 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str42, "IBM-864"},
#line 32 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str43, "ISO8859-4"},
#line 66 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str44, "IBM-5347"},
#line 43 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str45, "00775"},
#line 58 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str46, "IBM-943"},
#line 64 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str47, "01131"},
#line 49 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str48, "IBM-861"},
#line 29 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str49, "ISO8859-1"},
#line 74 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str50, "01350"},
    {-1},
#line 59 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str52, "IBM-1363"},
    {-1}, {-1},
#line 55 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str55, "TIS-620"},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 60 "./iconv_open-zos.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str64, "IBM-1046"}
  };

const struct mapping *
mapping_lookup (register const char *str, register size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = mapping_hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register int o = mappings[key].standard_name;
          if (o >= 0)
            {
              register const char *s = o + stringpool;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &mappings[key];
            }
        }
    }
  return 0;
}

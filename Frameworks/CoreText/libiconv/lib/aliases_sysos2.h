/* ANSI-C code produced by gperf version 3.3 */
/* Command-line: gperf -m 10 lib/aliases_sysos2.gperf  */
/* Computed positions: -k'1,3-11,$' */

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

#line 1 "lib/aliases_sysos2.gperf"
struct alias { int name; unsigned int encoding_index; };

#define TOTAL_KEYWORDS 396
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 45
#define MIN_HASH_VALUE 14
#define MAX_HASH_VALUE 1106
/* maximum key range = 1093, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
aliases_hash (register const char *str, register size_t len)
{
  static const unsigned short asso_values[] =
    {
      1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107,
      1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107,
      1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107,
      1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107,
      1107, 1107, 1107, 1107, 1107,   18,   43, 1107,   59,    1,
         8,    6,   50,   11,    3,  103,    5,   22,  121, 1107,
      1107, 1107, 1107, 1107, 1107,   27,  200,   92,    1,  136,
        76,  144,  104,    1,  279,    4,  193,    1,    4,    1,
        40, 1107,  146,  130,   97,  178,  125,  141,   89,    1,
         6, 1107, 1107, 1107, 1107,  184, 1107, 1107, 1107, 1107,
      1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107,
      1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107,
      1107, 1107, 1107, 1107, 1107, 1107, 1107, 1107
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[10]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 10:
        hval += asso_values[(unsigned char)str[9]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 9:
        hval += asso_values[(unsigned char)str[8]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 8:
        hval += asso_values[(unsigned char)str[7]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 7:
        hval += asso_values[(unsigned char)str[6]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[(unsigned char)str[5]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[(unsigned char)str[4]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 4:
        hval += asso_values[(unsigned char)str[3]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 3:
        hval += asso_values[(unsigned char)str[2]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

struct stringpool_t
  {
    char stringpool_str14[sizeof("866")];
    char stringpool_str22[sizeof("IBM866")];
    char stringpool_str24[sizeof("862")];
    char stringpool_str32[sizeof("IBM862")];
    char stringpool_str38[sizeof("IBM-1131")];
    char stringpool_str40[sizeof("MS936")];
    char stringpool_str41[sizeof("IBM-866")];
    char stringpool_str42[sizeof("IBM-1381")];
    char stringpool_str45[sizeof("IBM-813")];
    char stringpool_str46[sizeof("IBM-1386")];
    char stringpool_str50[sizeof("IBM-1251")];
    char stringpool_str51[sizeof("IBM-862")];
    char stringpool_str52[sizeof("IBM-1383")];
    char stringpool_str54[sizeof("IBM-1256")];
    char stringpool_str56[sizeof("IBM-916")];
    char stringpool_str58[sizeof("IBM819")];
    char stringpool_str59[sizeof("IBM-921")];
    char stringpool_str60[sizeof("IBM-1253")];
    char stringpool_str62[sizeof("IBM-913")];
    char stringpool_str64[sizeof("IBM-1252")];
    char stringpool_str66[sizeof("IBM-912")];
    char stringpool_str69[sizeof("IBM-923")];
    char stringpool_str70[sizeof("IBM-1255")];
    char stringpool_str71[sizeof("IBM-932")];
    char stringpool_str72[sizeof("IBM-915")];
    char stringpool_str74[sizeof("ISO8859-1")];
    char stringpool_str76[sizeof("ISO8859-11")];
    char stringpool_str77[sizeof("IBM-819")];
    char stringpool_str78[sizeof("ISO8859-6")];
    char stringpool_str80[sizeof("ISO8859-16")];
    char stringpool_str82[sizeof("ISO8859-8")];
    char stringpool_str84[sizeof("ISO8859-3")];
    char stringpool_str86[sizeof("ISO8859-13")];
    char stringpool_str88[sizeof("ISO8859-2")];
    char stringpool_str93[sizeof("ISO-8859-1")];
    char stringpool_str94[sizeof("ISO8859-5")];
    char stringpool_str95[sizeof("ISO-8859-11")];
    char stringpool_str96[sizeof("ISO8859-15")];
    char stringpool_str97[sizeof("ISO-8859-6")];
    char stringpool_str98[sizeof("CN")];
    char stringpool_str99[sizeof("ISO-8859-16")];
    char stringpool_str101[sizeof("ISO-8859-8")];
    char stringpool_str103[sizeof("ISO-8859-3")];
    char stringpool_str105[sizeof("ISO-8859-13")];
    char stringpool_str106[sizeof("IBM-1208")];
    char stringpool_str107[sizeof("ISO-8859-2")];
    char stringpool_str108[sizeof("CP1131")];
    char stringpool_str110[sizeof("CP1361")];
    char stringpool_str111[sizeof("CP866")];
    char stringpool_str112[sizeof("HZ")];
    char stringpool_str113[sizeof("ISO-8859-5")];
    char stringpool_str115[sizeof("ISO-8859-15")];
    char stringpool_str116[sizeof("ISO8859-9")];
    char stringpool_str118[sizeof("CP1133")];
    char stringpool_str119[sizeof("X0212")];
    char stringpool_str120[sizeof("CP1251")];
    char stringpool_str121[sizeof("CP862")];
    char stringpool_str124[sizeof("CP1256")];
    char stringpool_str126[sizeof("850")];
    char stringpool_str127[sizeof("ASCII")];
    char stringpool_str128[sizeof("CP1258")];
    char stringpool_str130[sizeof("CP1253")];
    char stringpool_str131[sizeof("CP936")];
    char stringpool_str134[sizeof("CP1252")];
    char stringpool_str135[sizeof("ISO-8859-9")];
    char stringpool_str137[sizeof("IBM-1089")];
    char stringpool_str139[sizeof("C99")];
    char stringpool_str140[sizeof("CP1255")];
    char stringpool_str141[sizeof("CP932")];
    char stringpool_str142[sizeof("IBM850")];
    char stringpool_str143[sizeof("IBM-949")];
    char stringpool_str145[sizeof("IBM-878")];
    char stringpool_str147[sizeof("CP819")];
    char stringpool_str148[sizeof("IBM-1254")];
    char stringpool_str149[sizeof("KZ-1048")];
    char stringpool_str150[sizeof("IBM-914")];
    char stringpool_str152[sizeof("IBM-964")];
    char stringpool_str153[sizeof("R8")];
    char stringpool_str155[sizeof("GBK")];
    char stringpool_str156[sizeof("IBM-4946")];
    char stringpool_str157[sizeof("PT154")];
    char stringpool_str159[sizeof("IBM-3372")];
    char stringpool_str160[sizeof("IBM-954")];
    char stringpool_str161[sizeof("IBM-850")];
    char stringpool_str163[sizeof("X0201")];
    char stringpool_str166[sizeof("IBM-1250")];
    char stringpool_str168[sizeof("IBM-33722")];
    char stringpool_str171[sizeof("X0208")];
    char stringpool_str172[sizeof("ISO8859-4")];
    char stringpool_str174[sizeof("ISO8859-14")];
    char stringpool_str175[sizeof("IBM-920")];
    char stringpool_str178[sizeof("IBM-950")];
    char stringpool_str181[sizeof("GB2312")];
    char stringpool_str182[sizeof("IBM-CP1133")];
    char stringpool_str185[sizeof("ISO646-CN")];
    char stringpool_str187[sizeof("CP50221")];
    char stringpool_str188[sizeof("MAC")];
    char stringpool_str189[sizeof("MS-ANSI")];
    char stringpool_str191[sizeof("ISO-8859-4")];
    char stringpool_str192[sizeof("ISO8859-10")];
    char stringpool_str193[sizeof("ISO-8859-14")];
    char stringpool_str194[sizeof("ROMAN8")];
    char stringpool_str196[sizeof("L1")];
    char stringpool_str198[sizeof("L6")];
    char stringpool_str199[sizeof("ISO-IR-6")];
    char stringpool_str200[sizeof("L8")];
    char stringpool_str201[sizeof("L3")];
    char stringpool_str202[sizeof("ECMA-118")];
    char stringpool_str203[sizeof("L2")];
    char stringpool_str205[sizeof("ISO-IR-166")];
    char stringpool_str206[sizeof("L5")];
    char stringpool_str209[sizeof("CP154")];
    char stringpool_str210[sizeof("ISO-IR-126")];
    char stringpool_str211[sizeof("ISO-8859-10")];
    char stringpool_str212[sizeof("ISO-IR-138")];
    char stringpool_str213[sizeof("CP949")];
    char stringpool_str215[sizeof("ISO-IR-58")];
    char stringpool_str217[sizeof("ISO-IR-226")];
    char stringpool_str218[sizeof("CP1254")];
    char stringpool_str221[sizeof("ISO-IR-165")];
    char stringpool_str223[sizeof("IBM367")];
    char stringpool_str227[sizeof("ASMO-708")];
    char stringpool_str228[sizeof("KOI8-T")];
    char stringpool_str230[sizeof("CSKZ1048")];
    char stringpool_str231[sizeof("CP850")];
    char stringpool_str232[sizeof("ISO-2022-CN")];
    char stringpool_str234[sizeof("TCVN")];
    char stringpool_str235[sizeof("IBM-874")];
    char stringpool_str236[sizeof("CP1250")];
    char stringpool_str242[sizeof("IBM-367")];
    char stringpool_str245[sizeof("L4")];
    char stringpool_str247[sizeof("IBM-1004")];
    char stringpool_str248[sizeof("CP950")];
    char stringpool_str251[sizeof("ISO-IR-159")];
    char stringpool_str254[sizeof("IBM-1257")];
    char stringpool_str256[sizeof("ISO-IR-148")];
    char stringpool_str257[sizeof("ISO-IR-101")];
    char stringpool_str259[sizeof("ISO_8859-1")];
    char stringpool_str261[sizeof("ISO_8859-11")];
    char stringpool_str262[sizeof("ISO-IR-199")];
    char stringpool_str263[sizeof("ISO_8859-6")];
    char stringpool_str265[sizeof("ISO_8859-16")];
    char stringpool_str267[sizeof("ISO_8859-8")];
    char stringpool_str268[sizeof("ISO_8859-16:2001")];
    char stringpool_str269[sizeof("ISO_8859-3")];
    char stringpool_str270[sizeof("IBM-970")];
    char stringpool_str271[sizeof("ISO_8859-13")];
    char stringpool_str272[sizeof("RK1048")];
    char stringpool_str273[sizeof("ISO_8859-2")];
    char stringpool_str274[sizeof("ISO-IR-203")];
    char stringpool_str278[sizeof("ISO8859-7")];
    char stringpool_str279[sizeof("ISO_8859-5")];
    char stringpool_str280[sizeof("ISO_8859-15:1998")];
    char stringpool_str281[sizeof("ISO_8859-15")];
    char stringpool_str284[sizeof("MACROMAN")];
    char stringpool_str285[sizeof("UTF-16")];
    char stringpool_str287[sizeof("UTF-8")];
    char stringpool_str289[sizeof("ARMSCII-8")];
    char stringpool_str290[sizeof("ISO-IR-149")];
    char stringpool_str291[sizeof("PTCP154")];
    char stringpool_str292[sizeof("ECMA-114")];
    char stringpool_str295[sizeof("ISO-IR-14")];
    char stringpool_str297[sizeof("ISO-8859-7")];
    char stringpool_str298[sizeof("L7")];
    char stringpool_str299[sizeof("ISO-IR-109")];
    char stringpool_str300[sizeof("UTF-32")];
    char stringpool_str301[sizeof("ISO_8859-9")];
    char stringpool_str303[sizeof("LATIN1")];
    char stringpool_str305[sizeof("CP874")];
    char stringpool_str307[sizeof("LATIN6")];
    char stringpool_str310[sizeof("US")];
    char stringpool_str311[sizeof("LATIN8")];
    char stringpool_str312[sizeof("CP367")];
    char stringpool_str313[sizeof("LATIN3")];
    char stringpool_str314[sizeof("L10")];
    char stringpool_str315[sizeof("ISO-IR-110")];
    char stringpool_str316[sizeof("CSIBM866")];
    char stringpool_str317[sizeof("LATIN2")];
    char stringpool_str319[sizeof("ISO_8859-14:1998")];
    char stringpool_str320[sizeof("HP-ROMAN8")];
    char stringpool_str321[sizeof("JP")];
    char stringpool_str323[sizeof("LATIN5")];
    char stringpool_str324[sizeof("CP1257")];
    char stringpool_str326[sizeof("KOI8-R")];
    char stringpool_str327[sizeof("KOREAN")];
    char stringpool_str329[sizeof("ISO-2022-CN-EXT")];
    char stringpool_str330[sizeof("MACTHAI")];
    char stringpool_str331[sizeof("ISO_8859-10:1992")];
    char stringpool_str333[sizeof("EUCCN")];
    char stringpool_str337[sizeof("MACROMANIA")];
    char stringpool_str340[sizeof("GB18030")];
    char stringpool_str343[sizeof("ISO-IR-179")];
    char stringpool_str346[sizeof("ISO-IR-144")];
    char stringpool_str347[sizeof("UCS-2")];
    char stringpool_str351[sizeof("CSASCII")];
    char stringpool_str352[sizeof("EUC-CN")];
    char stringpool_str355[sizeof("ISO-10646-UCS-2")];
    char stringpool_str356[sizeof("VISCII")];
    char stringpool_str357[sizeof("ISO_8859-4")];
    char stringpool_str359[sizeof("ISO_8859-14")];
    char stringpool_str362[sizeof("TIS620")];
    char stringpool_str363[sizeof("KSC_5601")];
    char stringpool_str364[sizeof("LATIN-9")];
    char stringpool_str365[sizeof("UHC")];
    char stringpool_str370[sizeof("BIG5")];
    char stringpool_str373[sizeof("ISO-IR-100")];
    char stringpool_str377[sizeof("ISO_8859-10")];
    char stringpool_str379[sizeof("TCVN5712-1")];
    char stringpool_str381[sizeof("TIS-620")];
    char stringpool_str384[sizeof("TCVN-5712")];
    char stringpool_str385[sizeof("TIS620.2533-1")];
    char stringpool_str389[sizeof("BIG-5")];
    char stringpool_str390[sizeof("KOI8-U")];
    char stringpool_str393[sizeof("ISO_8859-8:1988")];
    char stringpool_str394[sizeof("ISO_8859-3:1988")];
    char stringpool_str395[sizeof("SJIS")];
    char stringpool_str397[sizeof("ISO-10646-UCS-4")];
    char stringpool_str399[sizeof("ISO_8859-5:1988")];
    char stringpool_str401[sizeof("LATIN4")];
    char stringpool_str402[sizeof("CSKOI8R")];
    char stringpool_str403[sizeof("TIS620.2529-1")];
    char stringpool_str405[sizeof("ISO-IR-87")];
    char stringpool_str410[sizeof("ISO-IR-127")];
    char stringpool_str411[sizeof("ISO-IR-57")];
    char stringpool_str413[sizeof("ISO-IR-157")];
    char stringpool_str415[sizeof("CHAR")];
    char stringpool_str418[sizeof("CSISO2022CN")];
    char stringpool_str421[sizeof("LATIN10")];
    char stringpool_str425[sizeof("VISCII1.1-1")];
    char stringpool_str427[sizeof("ISO_8859-9:1989")];
    char stringpool_str428[sizeof("ISO-2022-KR")];
    char stringpool_str429[sizeof("GREEK")];
    char stringpool_str431[sizeof("UCS-4")];
    char stringpool_str432[sizeof("MS-EE")];
    char stringpool_str435[sizeof("CSHPROMAN8")];
    char stringpool_str436[sizeof("GREEK8")];
    char stringpool_str438[sizeof("ISO_8859-4:1988")];
    char stringpool_str441[sizeof("TIS620-0")];
    char stringpool_str443[sizeof("TIS620.2533-0")];
    char stringpool_str444[sizeof("ISO646-JP")];
    char stringpool_str445[sizeof("ARABIC")];
    char stringpool_str451[sizeof("CSVISCII")];
    char stringpool_str454[sizeof("ISO-2022-JP-1")];
    char stringpool_str455[sizeof("MS-TURK")];
    char stringpool_str456[sizeof("US-ASCII")];
    char stringpool_str459[sizeof("UNICODE-1-1")];
    char stringpool_str461[sizeof("ISO-2022-JP-2")];
    char stringpool_str462[sizeof("JAVA")];
    char stringpool_str463[sizeof("ISO_8859-7")];
    char stringpool_str465[sizeof("CSBIG5")];
    char stringpool_str466[sizeof("ELOT_928")];
    char stringpool_str469[sizeof("WINDOWS-1251")];
    char stringpool_str471[sizeof("WINDOWS-1256")];
    char stringpool_str473[sizeof("WINDOWS-1258")];
    char stringpool_str474[sizeof("WINDOWS-1253")];
    char stringpool_str475[sizeof("CSGB2312")];
    char stringpool_str476[sizeof("WINDOWS-1252")];
    char stringpool_str479[sizeof("WINDOWS-1255")];
    char stringpool_str481[sizeof("WINDOWS-936")];
    char stringpool_str482[sizeof("CSPTCP154")];
    char stringpool_str483[sizeof("UTF-7")];
    char stringpool_str484[sizeof("CN-BIG5")];
    char stringpool_str487[sizeof("ISO_8859-1:1987")];
    char stringpool_str489[sizeof("ISO_8859-6:1987")];
    char stringpool_str490[sizeof("GB18030:2022")];
    char stringpool_str491[sizeof("ISO-2022-JP")];
    char stringpool_str492[sizeof("ISO_8859-7:2003")];
    char stringpool_str494[sizeof("ISO_8859-2:1987")];
    char stringpool_str501[sizeof("STRK1048-2002")];
    char stringpool_str502[sizeof("GB_2312-80")];
    char stringpool_str503[sizeof("MACCROATIAN")];
    char stringpool_str507[sizeof("LATIN7")];
    char stringpool_str509[sizeof("MS_KANJI")];
    char stringpool_str510[sizeof("TCVN5712-1:1993")];
    char stringpool_str512[sizeof("GB_1988-80")];
    char stringpool_str518[sizeof("WINDOWS-1254")];
    char stringpool_str519[sizeof("CSUNICODE11")];
    char stringpool_str523[sizeof("ISO646-US")];
    char stringpool_str525[sizeof("HZ-GB-2312")];
    char stringpool_str527[sizeof("WINDOWS-1250")];
    char stringpool_str529[sizeof("EUCKR")];
    char stringpool_str531[sizeof("CSKSC56011987")];
    char stringpool_str535[sizeof("IBM-EUCCN")];
    char stringpool_str537[sizeof("KOI8-RU")];
    char stringpool_str543[sizeof("MACINTOSH")];
    char stringpool_str544[sizeof("GB18030:2005")];
    char stringpool_str548[sizeof("EUC-KR")];
    char stringpool_str551[sizeof("EUCTW-1993")];
    char stringpool_str552[sizeof("JIS0208")];
    char stringpool_str558[sizeof("MACICELAND")];
    char stringpool_str559[sizeof("CSISOLATIN1")];
    char stringpool_str563[sizeof("CSISOLATIN6")];
    char stringpool_str566[sizeof("ANSI_X3.4-1986")];
    char stringpool_str567[sizeof("UNICODE-1-1-UTF-7")];
    char stringpool_str568[sizeof("ANSI_X3.4-1968")];
    char stringpool_str569[sizeof("CSISOLATIN3")];
    char stringpool_str571[sizeof("WINDOWS-1257")];
    char stringpool_str573[sizeof("CSISOLATIN2")];
    char stringpool_str574[sizeof("ISO_646.IRV:1991")];
    char stringpool_str579[sizeof("CSISOLATIN5")];
    char stringpool_str580[sizeof("MULELAO-1")];
    char stringpool_str584[sizeof("ISO-2022-JP-MS")];
    char stringpool_str589[sizeof("ISO_8859-7:1987")];
    char stringpool_str592[sizeof("EUCJP")];
    char stringpool_str593[sizeof("KS_C_5601-1989")];
    char stringpool_str597[sizeof("MS-GREEK")];
    char stringpool_str598[sizeof("CSUCS4")];
    char stringpool_str611[sizeof("EUC-JP")];
    char stringpool_str612[sizeof("EUCTW")];
    char stringpool_str614[sizeof("CSISO2022KR")];
    char stringpool_str621[sizeof("GEORGIAN-ACADEMY")];
    char stringpool_str625[sizeof("CSUNICODE11UTF7")];
    char stringpool_str626[sizeof("MS-ARAB")];
    char stringpool_str631[sizeof("EUC-TW")];
    char stringpool_str641[sizeof("NEXTSTEP")];
    char stringpool_str642[sizeof("CHINESE")];
    char stringpool_str646[sizeof("CSISO2022JP2")];
    char stringpool_str650[sizeof("CSUNICODE")];
    char stringpool_str651[sizeof("MS-CYRL")];
    char stringpool_str655[sizeof("WINDOWS-874")];
    char stringpool_str657[sizeof("CSISOLATIN4")];
    char stringpool_str659[sizeof("CN-GB")];
    char stringpool_str664[sizeof("CSMACINTOSH")];
    char stringpool_str671[sizeof("MACGREEK")];
    char stringpool_str672[sizeof("CSISO58GB231280")];
    char stringpool_str674[sizeof("KS_C_5601-1987")];
    char stringpool_str677[sizeof("CSISO2022JP")];
    char stringpool_str681[sizeof("CSISOLATINARABIC")];
    char stringpool_str682[sizeof("JISX0201-1976")];
    char stringpool_str687[sizeof("MACARABIC")];
    char stringpool_str709[sizeof("CSISOLATINGREEK")];
    char stringpool_str717[sizeof("CSPC862LATINHEBREW")];
    char stringpool_str723[sizeof("CSISO57GB1988")];
    char stringpool_str731[sizeof("IBM-EUCKR")];
    char stringpool_str733[sizeof("ISO-CELTIC")];
    char stringpool_str735[sizeof("MACUKRAINE")];
    char stringpool_str744[sizeof("UCS-2-SWAPPED")];
    char stringpool_str746[sizeof("JIS_C6226-1983")];
    char stringpool_str747[sizeof("CSISO159JISX02121990")];
    char stringpool_str748[sizeof("CSISOLATINCYRILLIC")];
    char stringpool_str749[sizeof("UTF-16LE")];
    char stringpool_str756[sizeof("UTF-16BE")];
    char stringpool_str758[sizeof("MS-HEBR")];
    char stringpool_str759[sizeof("UTF-32LE")];
    char stringpool_str766[sizeof("UTF-32BE")];
    char stringpool_str775[sizeof("JIS_X0212")];
    char stringpool_str776[sizeof("CN-GB-ISOIR165")];
    char stringpool_str786[sizeof("UCS-4-SWAPPED")];
    char stringpool_str794[sizeof("IBM-EUCJP")];
    char stringpool_str795[sizeof("CSISO14JISC6220RO")];
    char stringpool_str796[sizeof("GEORGIAN-PS")];
    char stringpool_str800[sizeof("JIS_C6220-1969-RO")];
    char stringpool_str801[sizeof("CSEUCKR")];
    char stringpool_str803[sizeof("WCHAR_T")];
    char stringpool_str806[sizeof("UCS-2LE")];
    char stringpool_str807[sizeof("CSISOLATINHEBREW")];
    char stringpool_str813[sizeof("UCS-2BE")];
    char stringpool_str814[sizeof("IBM-EUCTW")];
    char stringpool_str815[sizeof("JOHAB")];
    char stringpool_str818[sizeof("CYRILLIC")];
    char stringpool_str819[sizeof("JIS_X0201")];
    char stringpool_str821[sizeof("WINBALTRIM")];
    char stringpool_str825[sizeof("BIGFIVE")];
    char stringpool_str827[sizeof("JIS_X0208")];
    char stringpool_str844[sizeof("BIG-FIVE")];
    char stringpool_str848[sizeof("UCS-4LE")];
    char stringpool_str850[sizeof("JIS_X0212-1990")];
    char stringpool_str851[sizeof("CSISO87JISX0208")];
    char stringpool_str852[sizeof("JIS_X0208-1983")];
    char stringpool_str855[sizeof("UCS-4BE")];
    char stringpool_str867[sizeof("MACTURKISH")];
    char stringpool_str871[sizeof("SHIFT-JIS")];
    char stringpool_str872[sizeof("CSEUCPKDFMTJAPANESE")];
    char stringpool_str874[sizeof("HEBREW")];
    char stringpool_str877[sizeof("JIS_X0212.1990-0")];
    char stringpool_str882[sizeof("CSHALFWIDTHKATAKANA")];
    char stringpool_str884[sizeof("CSEUCTW")];
    char stringpool_str905[sizeof("JIS_X0208-1990")];
    char stringpool_str908[sizeof("UNICODEBIG")];
    char stringpool_str911[sizeof("CYRILLIC-ASIAN")];
    char stringpool_str915[sizeof("MACCYRILLIC")];
    char stringpool_str943[sizeof("UCS-2-INTERNAL")];
    char stringpool_str946[sizeof("UNICODELITTLE")];
    char stringpool_str954[sizeof("BIG5HKSCS")];
    char stringpool_str970[sizeof("BIG5-HKSCS:2001")];
    char stringpool_str973[sizeof("BIG5-HKSCS")];
    char stringpool_str974[sizeof("BIG5-HKSCS:2008")];
    char stringpool_str980[sizeof("CSPC850MULTILINGUAL")];
    char stringpool_str985[sizeof("UCS-4-INTERNAL")];
    char stringpool_str991[sizeof("BIG5-HKSCS:1999")];
    char stringpool_str1019[sizeof("BIG5-HKSCS:2004")];
    char stringpool_str1037[sizeof("SHIFT_JIS")];
    char stringpool_str1050[sizeof("CSSHIFTJIS")];
    char stringpool_str1058[sizeof("EXTENDED_UNIX_CODE_PACKED_FORMAT_FOR_JAPANESE")];
    char stringpool_str1076[sizeof("MACCENTRALEUROPE")];
    char stringpool_str1106[sizeof("MACHEBREW")];
  };
static const struct stringpool_t stringpool_contents =
  {
    "866",
    "IBM866",
    "862",
    "IBM862",
    "IBM-1131",
    "MS936",
    "IBM-866",
    "IBM-1381",
    "IBM-813",
    "IBM-1386",
    "IBM-1251",
    "IBM-862",
    "IBM-1383",
    "IBM-1256",
    "IBM-916",
    "IBM819",
    "IBM-921",
    "IBM-1253",
    "IBM-913",
    "IBM-1252",
    "IBM-912",
    "IBM-923",
    "IBM-1255",
    "IBM-932",
    "IBM-915",
    "ISO8859-1",
    "ISO8859-11",
    "IBM-819",
    "ISO8859-6",
    "ISO8859-16",
    "ISO8859-8",
    "ISO8859-3",
    "ISO8859-13",
    "ISO8859-2",
    "ISO-8859-1",
    "ISO8859-5",
    "ISO-8859-11",
    "ISO8859-15",
    "ISO-8859-6",
    "CN",
    "ISO-8859-16",
    "ISO-8859-8",
    "ISO-8859-3",
    "ISO-8859-13",
    "IBM-1208",
    "ISO-8859-2",
    "CP1131",
    "CP1361",
    "CP866",
    "HZ",
    "ISO-8859-5",
    "ISO-8859-15",
    "ISO8859-9",
    "CP1133",
    "X0212",
    "CP1251",
    "CP862",
    "CP1256",
    "850",
    "ASCII",
    "CP1258",
    "CP1253",
    "CP936",
    "CP1252",
    "ISO-8859-9",
    "IBM-1089",
    "C99",
    "CP1255",
    "CP932",
    "IBM850",
    "IBM-949",
    "IBM-878",
    "CP819",
    "IBM-1254",
    "KZ-1048",
    "IBM-914",
    "IBM-964",
    "R8",
    "GBK",
    "IBM-4946",
    "PT154",
    "IBM-3372",
    "IBM-954",
    "IBM-850",
    "X0201",
    "IBM-1250",
    "IBM-33722",
    "X0208",
    "ISO8859-4",
    "ISO8859-14",
    "IBM-920",
    "IBM-950",
    "GB2312",
    "IBM-CP1133",
    "ISO646-CN",
    "CP50221",
    "MAC",
    "MS-ANSI",
    "ISO-8859-4",
    "ISO8859-10",
    "ISO-8859-14",
    "ROMAN8",
    "L1",
    "L6",
    "ISO-IR-6",
    "L8",
    "L3",
    "ECMA-118",
    "L2",
    "ISO-IR-166",
    "L5",
    "CP154",
    "ISO-IR-126",
    "ISO-8859-10",
    "ISO-IR-138",
    "CP949",
    "ISO-IR-58",
    "ISO-IR-226",
    "CP1254",
    "ISO-IR-165",
    "IBM367",
    "ASMO-708",
    "KOI8-T",
    "CSKZ1048",
    "CP850",
    "ISO-2022-CN",
    "TCVN",
    "IBM-874",
    "CP1250",
    "IBM-367",
    "L4",
    "IBM-1004",
    "CP950",
    "ISO-IR-159",
    "IBM-1257",
    "ISO-IR-148",
    "ISO-IR-101",
    "ISO_8859-1",
    "ISO_8859-11",
    "ISO-IR-199",
    "ISO_8859-6",
    "ISO_8859-16",
    "ISO_8859-8",
    "ISO_8859-16:2001",
    "ISO_8859-3",
    "IBM-970",
    "ISO_8859-13",
    "RK1048",
    "ISO_8859-2",
    "ISO-IR-203",
    "ISO8859-7",
    "ISO_8859-5",
    "ISO_8859-15:1998",
    "ISO_8859-15",
    "MACROMAN",
    "UTF-16",
    "UTF-8",
    "ARMSCII-8",
    "ISO-IR-149",
    "PTCP154",
    "ECMA-114",
    "ISO-IR-14",
    "ISO-8859-7",
    "L7",
    "ISO-IR-109",
    "UTF-32",
    "ISO_8859-9",
    "LATIN1",
    "CP874",
    "LATIN6",
    "US",
    "LATIN8",
    "CP367",
    "LATIN3",
    "L10",
    "ISO-IR-110",
    "CSIBM866",
    "LATIN2",
    "ISO_8859-14:1998",
    "HP-ROMAN8",
    "JP",
    "LATIN5",
    "CP1257",
    "KOI8-R",
    "KOREAN",
    "ISO-2022-CN-EXT",
    "MACTHAI",
    "ISO_8859-10:1992",
    "EUCCN",
    "MACROMANIA",
    "GB18030",
    "ISO-IR-179",
    "ISO-IR-144",
    "UCS-2",
    "CSASCII",
    "EUC-CN",
    "ISO-10646-UCS-2",
    "VISCII",
    "ISO_8859-4",
    "ISO_8859-14",
    "TIS620",
    "KSC_5601",
    "LATIN-9",
    "UHC",
    "BIG5",
    "ISO-IR-100",
    "ISO_8859-10",
    "TCVN5712-1",
    "TIS-620",
    "TCVN-5712",
    "TIS620.2533-1",
    "BIG-5",
    "KOI8-U",
    "ISO_8859-8:1988",
    "ISO_8859-3:1988",
    "SJIS",
    "ISO-10646-UCS-4",
    "ISO_8859-5:1988",
    "LATIN4",
    "CSKOI8R",
    "TIS620.2529-1",
    "ISO-IR-87",
    "ISO-IR-127",
    "ISO-IR-57",
    "ISO-IR-157",
    "CHAR",
    "CSISO2022CN",
    "LATIN10",
    "VISCII1.1-1",
    "ISO_8859-9:1989",
    "ISO-2022-KR",
    "GREEK",
    "UCS-4",
    "MS-EE",
    "CSHPROMAN8",
    "GREEK8",
    "ISO_8859-4:1988",
    "TIS620-0",
    "TIS620.2533-0",
    "ISO646-JP",
    "ARABIC",
    "CSVISCII",
    "ISO-2022-JP-1",
    "MS-TURK",
    "US-ASCII",
    "UNICODE-1-1",
    "ISO-2022-JP-2",
    "JAVA",
    "ISO_8859-7",
    "CSBIG5",
    "ELOT_928",
    "WINDOWS-1251",
    "WINDOWS-1256",
    "WINDOWS-1258",
    "WINDOWS-1253",
    "CSGB2312",
    "WINDOWS-1252",
    "WINDOWS-1255",
    "WINDOWS-936",
    "CSPTCP154",
    "UTF-7",
    "CN-BIG5",
    "ISO_8859-1:1987",
    "ISO_8859-6:1987",
    "GB18030:2022",
    "ISO-2022-JP",
    "ISO_8859-7:2003",
    "ISO_8859-2:1987",
    "STRK1048-2002",
    "GB_2312-80",
    "MACCROATIAN",
    "LATIN7",
    "MS_KANJI",
    "TCVN5712-1:1993",
    "GB_1988-80",
    "WINDOWS-1254",
    "CSUNICODE11",
    "ISO646-US",
    "HZ-GB-2312",
    "WINDOWS-1250",
    "EUCKR",
    "CSKSC56011987",
    "IBM-EUCCN",
    "KOI8-RU",
    "MACINTOSH",
    "GB18030:2005",
    "EUC-KR",
    "EUCTW-1993",
    "JIS0208",
    "MACICELAND",
    "CSISOLATIN1",
    "CSISOLATIN6",
    "ANSI_X3.4-1986",
    "UNICODE-1-1-UTF-7",
    "ANSI_X3.4-1968",
    "CSISOLATIN3",
    "WINDOWS-1257",
    "CSISOLATIN2",
    "ISO_646.IRV:1991",
    "CSISOLATIN5",
    "MULELAO-1",
    "ISO-2022-JP-MS",
    "ISO_8859-7:1987",
    "EUCJP",
    "KS_C_5601-1989",
    "MS-GREEK",
    "CSUCS4",
    "EUC-JP",
    "EUCTW",
    "CSISO2022KR",
    "GEORGIAN-ACADEMY",
    "CSUNICODE11UTF7",
    "MS-ARAB",
    "EUC-TW",
    "NEXTSTEP",
    "CHINESE",
    "CSISO2022JP2",
    "CSUNICODE",
    "MS-CYRL",
    "WINDOWS-874",
    "CSISOLATIN4",
    "CN-GB",
    "CSMACINTOSH",
    "MACGREEK",
    "CSISO58GB231280",
    "KS_C_5601-1987",
    "CSISO2022JP",
    "CSISOLATINARABIC",
    "JISX0201-1976",
    "MACARABIC",
    "CSISOLATINGREEK",
    "CSPC862LATINHEBREW",
    "CSISO57GB1988",
    "IBM-EUCKR",
    "ISO-CELTIC",
    "MACUKRAINE",
    "UCS-2-SWAPPED",
    "JIS_C6226-1983",
    "CSISO159JISX02121990",
    "CSISOLATINCYRILLIC",
    "UTF-16LE",
    "UTF-16BE",
    "MS-HEBR",
    "UTF-32LE",
    "UTF-32BE",
    "JIS_X0212",
    "CN-GB-ISOIR165",
    "UCS-4-SWAPPED",
    "IBM-EUCJP",
    "CSISO14JISC6220RO",
    "GEORGIAN-PS",
    "JIS_C6220-1969-RO",
    "CSEUCKR",
    "WCHAR_T",
    "UCS-2LE",
    "CSISOLATINHEBREW",
    "UCS-2BE",
    "IBM-EUCTW",
    "JOHAB",
    "CYRILLIC",
    "JIS_X0201",
    "WINBALTRIM",
    "BIGFIVE",
    "JIS_X0208",
    "BIG-FIVE",
    "UCS-4LE",
    "JIS_X0212-1990",
    "CSISO87JISX0208",
    "JIS_X0208-1983",
    "UCS-4BE",
    "MACTURKISH",
    "SHIFT-JIS",
    "CSEUCPKDFMTJAPANESE",
    "HEBREW",
    "JIS_X0212.1990-0",
    "CSHALFWIDTHKATAKANA",
    "CSEUCTW",
    "JIS_X0208-1990",
    "UNICODEBIG",
    "CYRILLIC-ASIAN",
    "MACCYRILLIC",
    "UCS-2-INTERNAL",
    "UNICODELITTLE",
    "BIG5HKSCS",
    "BIG5-HKSCS:2001",
    "BIG5-HKSCS",
    "BIG5-HKSCS:2008",
    "CSPC850MULTILINGUAL",
    "UCS-4-INTERNAL",
    "BIG5-HKSCS:1999",
    "BIG5-HKSCS:2004",
    "SHIFT_JIS",
    "CSSHIFTJIS",
    "EXTENDED_UNIX_CODE_PACKED_FORMAT_FOR_JAPANESE",
    "MACCENTRALEUROPE",
    "MACHEBREW"
  };
#define stringpool ((const char *) &stringpool_contents)

#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
static const struct alias aliases[] =
  {
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 233 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str14, ei_cp866},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 232 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str22, ei_cp866},
    {-1},
#line 228 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str24, ei_cp862},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 227 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str32, ei_cp862},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 237 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str38, ei_cp1131},
    {-1},
#line 364 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str40, ei_cp936},
#line 235 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str41, ei_cp866},
#line 359 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str42, ei_euc_cn},
    {-1}, {-1},
#line 122 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str45, ei_iso8859_7},
#line 362 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str46, ei_ces_gbk},
    {-1}, {-1}, {-1},
#line 192 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str50, ei_cp1251},
#line 230 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str51, ei_cp862},
#line 360 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str52, ei_euc_cn},
    {-1},
#line 213 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str54, ei_cp1256},
    {-1},
#line 130 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str56, ei_iso8859_8},
    {-1},
#line 60 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str58, ei_iso8859_1},
#line 157 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str59, ei_iso8859_13},
#line 201 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str60, ei_cp1253},
    {-1},
#line 83 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str62, ei_iso8859_3},
    {-1},
#line 196 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str64, ei_cp1252},
    {-1},
#line 74 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str66, ei_iso8859_2},
    {-1}, {-1},
#line 172 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str69, ei_iso8859_15},
#line 209 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str70, ei_cp1255},
#line 345 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str71, ei_cp932},
#line 100 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str72, ei_iso8859_5},
    {-1},
#line 64 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str74, ei_iso8859_1},
    {-1},
#line 150 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str76, ei_iso8859_11},
#line 65 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str77, ei_iso8859_1},
#line 109 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str78, ei_iso8859_6},
    {-1},
#line 179 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str80, ei_iso8859_16},
    {-1},
#line 129 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str82, ei_iso8859_8},
    {-1},
#line 82 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str84, ei_iso8859_3},
    {-1},
#line 156 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str86, ei_iso8859_13},
    {-1},
#line 73 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str88, ei_iso8859_2},
    {-1}, {-1}, {-1}, {-1},
#line 55 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str93, ei_iso8859_1},
#line 99 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str94, ei_iso8859_5},
#line 148 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str95, ei_iso8859_11},
#line 171 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str96, ei_iso8859_15},
#line 101 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str97, ei_iso8859_6},
#line 317 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str98, ei_iso646_cn},
#line 173 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str99, ei_iso8859_16},
    {-1},
#line 123 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str101, ei_iso8859_8},
    {-1},
#line 75 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str103, ei_iso8859_3},
    {-1},
#line 151 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str105, ei_iso8859_13},
#line 25 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str106, ei_utf8},
#line 66 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str107, ei_iso8859_2},
#line 236 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str108, ei_cp1131},
    {-1},
#line 403 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str110, ei_johab},
#line 231 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str111, ei_cp866},
#line 372 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str112, ei_hz},
#line 93 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str113, ei_iso8859_5},
    {-1},
#line 166 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str115, ei_iso8859_15},
#line 138 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str116, ei_iso8859_9},
    {-1},
#line 272 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str118, ei_cp1133},
#line 311 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str119, ei_jisx0212},
#line 189 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str120, ei_cp1251},
#line 226 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str121, ei_cp862},
    {-1}, {-1},
#line 210 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str124, ei_cp1256},
    {-1},
#line 222 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str126, ei_cp850},
#line 13 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str127, ei_ascii},
#line 218 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str128, ei_cp1258},
    {-1},
#line 198 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str130, ei_cp1253},
#line 363 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str131, ei_cp936},
    {-1}, {-1},
#line 193 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str134, ei_cp1252},
#line 131 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str135, ei_iso8859_9},
    {-1},
#line 110 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str137, ei_iso8859_6},
    {-1},
#line 53 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str139, ei_c99},
#line 206 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str140, ei_cp1255},
#line 344 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str141, ei_cp932},
#line 221 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str142, ei_cp850},
#line 401 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str143, ei_cp949},
    {-1},
#line 182 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str145, ei_koi8_r},
    {-1},
#line 59 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str147, ei_iso8859_1},
#line 205 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str148, ei_cp1254},
#line 269 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str149, ei_rk1048},
#line 92 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str150, ei_iso8859_4},
    {-1},
#line 379 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str152, ei_euc_tw},
#line 255 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str153, ei_hp_roman8},
    {-1},
#line 361 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str155, ei_ces_gbk},
#line 225 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str156, ei_cp850},
#line 262 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str157, ei_pt154},
    {-1},
#line 336 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str159, ei_euc_jp},
#line 338 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str160, ei_euc_jp},
#line 224 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str161, ei_cp850},
    {-1},
#line 298 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str163, ei_jisx0201},
    {-1}, {-1},
#line 188 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str166, ei_cp1250},
    {-1},
#line 337 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str168, ei_euc_jp},
    {-1}, {-1},
#line 304 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str171, ei_jisx0208},
#line 91 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str172, ei_iso8859_4},
    {-1},
#line 165 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str174, ei_iso8859_14},
#line 139 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str175, ei_iso8859_9},
    {-1}, {-1},
#line 387 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str178, ei_cp950},
    {-1}, {-1},
#line 355 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str181, ei_euc_cn},
#line 273 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str182, ei_cp1133},
    {-1}, {-1},
#line 315 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str185, ei_iso646_cn},
    {-1},
#line 352 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str187, ei_iso2022_jpms},
#line 240 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str188, ei_mac_roman},
#line 195 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str189, ei_cp1252},
    {-1},
#line 84 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str191, ei_iso8859_4},
#line 147 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str192, ei_iso8859_10},
#line 158 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str193, ei_iso8859_14},
#line 254 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str194, ei_hp_roman8},
    {-1},
#line 62 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str196, ei_iso8859_1},
    {-1},
#line 145 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str198, ei_iso8859_10},
#line 16 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str199, ei_ascii},
#line 163 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str200, ei_iso8859_14},
#line 80 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str201, ei_iso8859_3},
#line 116 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str202, ei_iso8859_7},
#line 71 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str203, ei_iso8859_2},
    {-1},
#line 280 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str205, ei_tis620},
#line 136 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str206, ei_iso8859_9},
    {-1}, {-1},
#line 264 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str209, ei_pt154},
#line 115 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str210, ei_iso8859_7},
#line 140 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str211, ei_iso8859_10},
#line 126 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str212, ei_iso8859_8},
#line 399 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str213, ei_cp949},
    {-1},
#line 320 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str215, ei_gb2312},
    {-1},
#line 176 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str217, ei_iso8859_16},
#line 202 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str218, ei_cp1254},
    {-1}, {-1},
#line 323 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str221, ei_isoir165},
    {-1},
#line 20 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str223, ei_ascii},
    {-1}, {-1}, {-1},
#line 106 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str227, ei_iso8859_6},
#line 261 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str228, ei_koi8_t},
    {-1},
#line 270 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str230, ei_rk1048},
#line 220 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str231, ei_cp850},
#line 369 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str232, ei_iso2022_cn},
    {-1},
#line 287 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str234, ei_tcvn},
#line 283 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str235, ei_cp874},
#line 185 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str236, ei_cp1250},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 23 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str242, ei_ascii},
    {-1}, {-1},
#line 89 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str245, ei_iso8859_4},
    {-1},
#line 197 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str247, ei_cp1252},
#line 386 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str248, ei_cp950},
    {-1}, {-1},
#line 312 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str251, ei_jisx0212},
    {-1}, {-1},
#line 217 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str254, ei_cp1257},
    {-1},
#line 134 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str256, ei_iso8859_9},
#line 69 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str257, ei_iso8859_2},
    {-1},
#line 56 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str259, ei_iso8859_1},
    {-1},
#line 149 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str261, ei_iso8859_11},
#line 161 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str262, ei_iso8859_14},
#line 102 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str263, ei_iso8859_6},
    {-1},
#line 174 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str265, ei_iso8859_16},
    {-1},
#line 124 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str267, ei_iso8859_8},
#line 175 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str268, ei_iso8859_16},
#line 76 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str269, ei_iso8859_3},
#line 398 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str270, ei_euc_kr},
#line 152 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str271, ei_iso8859_13},
#line 267 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str272, ei_rk1048},
#line 67 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str273, ei_iso8859_2},
#line 169 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str274, ei_iso8859_15},
    {-1}, {-1}, {-1},
#line 121 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str278, ei_iso8859_7},
#line 94 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str279, ei_iso8859_5},
#line 168 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str280, ei_iso8859_15},
#line 167 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str281, ei_iso8859_15},
    {-1}, {-1},
#line 238 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str284, ei_mac_roman},
#line 40 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str285, ei_utf16},
    {-1},
#line 24 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str287, ei_utf8},
    {-1},
#line 258 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str289, ei_armscii_8},
#line 328 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str290, ei_ksc5601},
#line 263 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str291, ei_pt154},
#line 105 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str292, ei_iso8859_6},
    {-1}, {-1},
#line 293 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str295, ei_iso646_jp},
    {-1},
#line 111 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str297, ei_iso8859_7},
#line 155 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str298, ei_iso8859_13},
#line 78 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str299, ei_iso8859_3},
#line 43 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str300, ei_utf32},
#line 132 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str301, ei_iso8859_9},
    {-1},
#line 61 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str303, ei_iso8859_1},
    {-1},
#line 281 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str305, ei_cp874},
    {-1},
#line 144 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str307, ei_iso8859_10},
    {-1}, {-1},
#line 21 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str310, ei_ascii},
#line 162 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str311, ei_iso8859_14},
#line 19 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str312, ei_ascii},
#line 79 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str313, ei_iso8859_3},
#line 178 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str314, ei_iso8859_16},
#line 87 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str315, ei_iso8859_4},
#line 234 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str316, ei_cp866},
#line 70 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str317, ei_iso8859_2},
    {-1},
#line 160 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str319, ei_iso8859_14},
#line 253 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str320, ei_hp_roman8},
#line 294 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str321, ei_iso646_jp},
    {-1},
#line 135 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str323, ei_iso8859_9},
#line 214 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str324, ei_cp1257},
    {-1},
#line 180 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str326, ei_koi8_r},
#line 330 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str327, ei_ksc5601},
    {-1},
#line 371 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str329, ei_iso2022_cn_ext},
#line 252 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str330, ei_mac_thai},
#line 142 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str331, ei_iso8859_10},
    {-1},
#line 354 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str333, ei_euc_cn},
    {-1}, {-1}, {-1},
#line 245 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str337, ei_mac_romania},
    {-1}, {-1},
#line 366 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str340, ei_gb18030_2005},
    {-1}, {-1},
#line 153 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str343, ei_iso8859_13},
    {-1}, {-1},
#line 96 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str346, ei_iso8859_5},
#line 26 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str347, ei_ucs2},
    {-1}, {-1}, {-1},
#line 22 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str351, ei_ascii},
#line 353 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str352, ei_euc_cn},
    {-1}, {-1},
#line 27 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str355, ei_ucs2},
#line 284 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str356, ei_viscii},
#line 85 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str357, ei_iso8859_4},
    {-1},
#line 159 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str359, ei_iso8859_14},
    {-1}, {-1},
#line 275 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str362, ei_tis620},
#line 325 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str363, ei_ksc5601},
#line 170 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str364, ei_iso8859_15},
#line 400 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str365, ei_cp949},
    {-1}, {-1}, {-1}, {-1},
#line 380 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str370, ei_ces_big5},
    {-1}, {-1},
#line 58 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str373, ei_iso8859_1},
    {-1}, {-1}, {-1},
#line 141 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str377, ei_iso8859_10},
    {-1},
#line 289 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str379, ei_tcvn},
    {-1},
#line 274 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str381, ei_tis620},
    {-1}, {-1},
#line 288 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str384, ei_tcvn},
#line 279 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str385, ei_tis620},
    {-1}, {-1}, {-1},
#line 381 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str389, ei_ces_big5},
#line 183 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str390, ei_koi8_u},
    {-1}, {-1},
#line 125 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str393, ei_iso8859_8},
#line 77 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str394, ei_iso8859_3},
#line 341 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str395, ei_sjis},
    {-1},
#line 36 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str397, ei_ucs4},
    {-1},
#line 95 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str399, ei_iso8859_5},
    {-1},
#line 88 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str401, ei_iso8859_4},
#line 181 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str402, ei_koi8_r},
#line 277 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str403, ei_tis620},
    {-1},
#line 305 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str405, ei_jisx0208},
    {-1}, {-1}, {-1}, {-1},
#line 104 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str410, ei_iso8859_6},
#line 316 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str411, ei_iso646_cn},
    {-1},
#line 143 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str413, ei_iso8859_10},
    {-1},
#line 406 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str415, ei_local_char},
    {-1}, {-1},
#line 370 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str418, ei_iso2022_cn},
    {-1}, {-1},
#line 177 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str421, ei_iso8859_16},
    {-1}, {-1}, {-1},
#line 285 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str425, ei_viscii},
    {-1},
#line 133 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str427, ei_iso8859_9},
#line 404 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str428, ei_iso2022_kr},
#line 119 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str429, ei_iso8859_7},
    {-1},
#line 35 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str431, ei_ucs4},
#line 187 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str432, ei_cp1250},
    {-1}, {-1},
#line 256 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str435, ei_hp_roman8},
#line 118 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str436, ei_iso8859_7},
    {-1},
#line 86 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str438, ei_iso8859_4},
    {-1}, {-1},
#line 276 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str441, ei_tis620},
    {-1},
#line 278 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str443, ei_tis620},
#line 292 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str444, ei_iso646_jp},
#line 107 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str445, ei_iso8859_6},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 286 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str451, ei_viscii},
    {-1}, {-1},
#line 348 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str454, ei_iso2022_jp1},
#line 204 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str455, ei_cp1254},
#line 12 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str456, ei_ascii},
    {-1}, {-1},
#line 31 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str459, ei_ucs2be},
    {-1},
#line 349 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str461, ei_iso2022_jp2},
#line 54 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str462, ei_java},
#line 112 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str463, ei_iso8859_7},
    {-1},
#line 385 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str465, ei_ces_big5},
#line 117 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str466, ei_iso8859_7},
    {-1}, {-1},
#line 190 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str469, ei_cp1251},
    {-1},
#line 211 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str471, ei_cp1256},
    {-1},
#line 219 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str473, ei_cp1258},
#line 199 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str474, ei_cp1253},
#line 357 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str475, ei_euc_cn},
#line 194 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str476, ei_cp1252},
    {-1}, {-1},
#line 207 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str479, ei_cp1255},
    {-1},
#line 365 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str481, ei_cp936},
#line 266 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str482, ei_pt154},
#line 46 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str483, ei_utf7},
#line 384 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str484, ei_ces_big5},
    {-1}, {-1},
#line 57 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str487, ei_iso8859_1},
    {-1},
#line 103 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str489, ei_iso8859_6},
#line 368 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str490, ei_gb18030_2022},
#line 346 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str491, ei_iso2022_jp},
#line 114 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str492, ei_iso8859_7},
    {-1},
#line 68 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str494, ei_iso8859_2},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 268 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str501, ei_rk1048},
#line 319 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str502, ei_gb2312},
#line 244 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str503, ei_mac_croatian},
    {-1}, {-1}, {-1},
#line 154 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str507, ei_iso8859_13},
    {-1},
#line 342 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str509, ei_sjis},
#line 290 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str510, ei_tcvn},
    {-1},
#line 314 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str512, ei_iso646_cn},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 203 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str518, ei_cp1254},
#line 32 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str519, ei_ucs2be},
    {-1}, {-1}, {-1},
#line 14 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str523, ei_ascii},
    {-1},
#line 373 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str525, ei_hz},
    {-1},
#line 186 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str527, ei_cp1250},
    {-1},
#line 395 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str529, ei_euc_kr},
    {-1},
#line 329 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str531, ei_ksc5601},
    {-1}, {-1}, {-1},
#line 358 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str535, ei_euc_cn},
    {-1},
#line 184 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str537, ei_koi8_ru},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 239 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str543, ei_mac_roman},
#line 367 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str544, ei_gb18030_2005},
    {-1}, {-1}, {-1},
#line 394 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str548, ei_euc_kr},
    {-1}, {-1},
#line 378 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str551, ei_euc_tw},
#line 303 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str552, ei_jisx0208},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 243 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str558, ei_mac_iceland},
#line 63 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str559, ei_iso8859_1},
    {-1}, {-1}, {-1},
#line 146 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str563, ei_iso8859_10},
    {-1}, {-1},
#line 18 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str566, ei_ascii},
#line 47 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str567, ei_utf7},
#line 17 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str568, ei_ascii},
#line 81 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str569, ei_iso8859_3},
    {-1},
#line 215 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str571, ei_cp1257},
    {-1},
#line 72 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str573, ei_iso8859_2},
#line 15 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str574, ei_ascii},
    {-1}, {-1}, {-1}, {-1},
#line 137 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str579, ei_iso8859_9},
#line 271 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str580, ei_mulelao},
    {-1}, {-1}, {-1},
#line 351 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str584, ei_iso2022_jpms},
    {-1}, {-1}, {-1}, {-1},
#line 113 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str589, ei_iso8859_7},
    {-1}, {-1},
#line 332 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str592, ei_euc_jp},
#line 327 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str593, ei_ksc5601},
    {-1}, {-1}, {-1},
#line 200 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str597, ei_cp1253},
#line 37 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str598, ei_ucs4},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1},
#line 331 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str611, ei_euc_jp},
#line 375 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str612, ei_euc_tw},
    {-1},
#line 405 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str614, ei_iso2022_kr},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 259 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str621, ei_georgian_academy},
    {-1}, {-1}, {-1},
#line 48 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str625, ei_utf7},
#line 212 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str626, ei_cp1256},
    {-1}, {-1}, {-1}, {-1},
#line 374 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str631, ei_euc_tw},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 257 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str641, ei_nextstep},
#line 322 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str642, ei_gb2312},
    {-1}, {-1}, {-1},
#line 350 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str646, ei_iso2022_jp2},
    {-1}, {-1}, {-1},
#line 28 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str650, ei_ucs2},
#line 191 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str651, ei_cp1251},
    {-1}, {-1}, {-1},
#line 282 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str655, ei_cp874},
    {-1},
#line 90 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str657, ei_iso8859_4},
    {-1},
#line 356 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str659, ei_euc_cn},
    {-1}, {-1}, {-1}, {-1},
#line 241 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str664, ei_mac_roman},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 248 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str671, ei_mac_greek},
#line 321 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str672, ei_gb2312},
    {-1},
#line 326 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str674, ei_ksc5601},
    {-1}, {-1},
#line 347 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str677, ei_iso2022_jp},
    {-1}, {-1}, {-1},
#line 108 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str681, ei_iso8859_6},
#line 297 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str682, ei_jisx0201},
    {-1}, {-1}, {-1}, {-1},
#line 251 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str687, ei_mac_arabic},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1},
#line 120 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str709, ei_iso8859_7},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 229 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str717, ei_cp862},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 318 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str723, ei_iso646_cn},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 397 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str731, ei_euc_kr},
    {-1},
#line 164 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str733, ei_iso8859_14},
    {-1},
#line 247 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str735, ei_mac_ukraine},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 50 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str744, ei_ucs2swapped},
    {-1},
#line 306 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str746, ei_jisx0208},
#line 313 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str747, ei_jisx0212},
#line 98 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str748, ei_iso8859_5},
#line 42 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str749, ei_utf16le},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 41 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str756, ei_utf16be},
    {-1},
#line 208 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str758, ei_cp1255},
#line 45 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str759, ei_utf32le},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 44 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str766, ei_utf32be},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 308 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str775, ei_jisx0212},
#line 324 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str776, ei_isoir165},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 52 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str786, ei_ucs4swapped},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 335 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str794, ei_euc_jp},
#line 295 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str795, ei_iso646_jp},
#line 260 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str796, ei_georgian_ps},
    {-1}, {-1}, {-1},
#line 291 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str800, ei_iso646_jp},
#line 396 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str801, ei_euc_kr},
    {-1},
#line 407 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str803, ei_local_wchar_t},
    {-1}, {-1},
#line 33 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str806, ei_ucs2le},
#line 128 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str807, ei_iso8859_8},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 29 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str813, ei_ucs2be},
#line 377 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str814, ei_euc_tw},
#line 402 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str815, ei_johab},
    {-1}, {-1},
#line 97 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str818, ei_iso8859_5},
#line 296 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str819, ei_jisx0201},
    {-1},
#line 216 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str821, ei_cp1257},
    {-1}, {-1}, {-1},
#line 383 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str825, ei_ces_big5},
    {-1},
#line 300 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str827, ei_jisx0208},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 382 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str844, ei_ces_big5},
    {-1}, {-1}, {-1},
#line 39 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str848, ei_ucs4le},
    {-1},
#line 310 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str850, ei_jisx0212},
#line 307 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str851, ei_jisx0208},
#line 301 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str852, ei_jisx0208},
    {-1}, {-1},
#line 38 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str855, ei_ucs4be},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1},
#line 249 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str867, ei_mac_turkish},
    {-1}, {-1}, {-1},
#line 340 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str871, ei_sjis},
#line 334 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str872, ei_euc_jp},
    {-1},
#line 127 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str874, ei_iso8859_8},
    {-1}, {-1},
#line 309 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str877, ei_jisx0212},
    {-1}, {-1}, {-1}, {-1},
#line 299 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str882, ei_jisx0201},
    {-1},
#line 376 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str884, ei_euc_tw},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1},
#line 302 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str905, ei_jisx0208},
    {-1}, {-1},
#line 30 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str908, ei_ucs2be},
    {-1}, {-1},
#line 265 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str911, ei_pt154},
    {-1}, {-1}, {-1},
#line 246 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str915, ei_mac_cyrillic},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 49 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str943, ei_ucs2internal},
    {-1}, {-1},
#line 34 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str946, ei_ucs2le},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 392 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str954, ei_big5hkscs2008},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 389 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str970, ei_big5hkscs2001},
    {-1}, {-1},
#line 391 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str973, ei_big5hkscs2008},
#line 393 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str974, ei_big5hkscs2008},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 223 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str980, ei_cp850},
    {-1}, {-1}, {-1}, {-1},
#line 51 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str985, ei_ucs4internal},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 388 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str991, ei_big5hkscs1999},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 390 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str1019, ei_big5hkscs2004},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 339 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str1037, ei_sjis},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1},
#line 343 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str1050, ei_sjis},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 333 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str1058, ei_euc_jp},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 242 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str1076, ei_mac_centraleurope},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1},
#line 250 "lib/aliases_sysos2.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str1106, ei_mac_hebrew}
  };
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic pop
#endif

static const struct alias *
aliases_lookup (register const char *str, register size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = aliases_hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register int o = aliases[key].name;
          if (o >= 0)
            {
              register const char *s = o + stringpool;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &aliases[key];
            }
        }
    }
  return (struct alias *) 0;
}

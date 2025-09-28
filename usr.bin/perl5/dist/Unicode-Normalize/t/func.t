
BEGIN {
    if ($ENV{PERL_CORE}) {
        chdir('t') if -d 't';
        @INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

#########################

use strict;
use warnings;
BEGIN { $| = 1; print "1..217\n"; }
my $count = 0;
sub ok { Unicode::Normalize::ok(\$count, @_) }

use Unicode::Normalize qw(:all);

ok(1);

sub _pack_U { Unicode::Normalize::dot_t_pack_U(@_) }
sub hexU { _pack_U map hex, split ' ', shift }

# This won't work on EBCDIC platforms prior to v5.8.0, which is when this
# translation function was defined
*to_native = (defined &utf8::unicode_to_native)
             ? \&utf8::unicode_to_native
             : sub { return shift };

#########################

ok(getCombinClass( to_native(0)),   0);
ok(getCombinClass(to_native(41)),   0);
ok(getCombinClass(to_native(65)),   0);
ok(getCombinClass( 768), 230);
ok(getCombinClass(1809),  36);

ok(getCanon(to_native(   0)), undef);
ok(getCanon(to_native(0x29)), undef);
ok(getCanon(to_native(0x41)), undef);
ok(getCanon(to_native(0x00C0)), _pack_U(0x0041, 0x0300));
ok(getCanon(to_native(0x00EF)), _pack_U(0x0069, 0x0308));
ok(getCanon(0x304C), _pack_U(0x304B, 0x3099));
ok(getCanon(0x1EA4), _pack_U(0x0041, 0x0302, 0x0301));
ok(getCanon(0x1F82), _pack_U(0x03B1, 0x0313, 0x0300, 0x0345));
ok(getCanon(0x1FAF), _pack_U(0x03A9, 0x0314, 0x0342, 0x0345));
ok(getCanon(0xAC00), _pack_U(0x1100, 0x1161));
ok(getCanon(0xAE00), _pack_U(0x1100, 0x1173, 0x11AF));
ok(getCanon(0x212C), undef);
ok(getCanon(0x3243), undef);
ok(getCanon(0xFA2D), _pack_U(0x9DB4));

# 20

ok(getCompat(to_native(   0)), undef);
ok(getCompat(to_native(0x29)), undef);
ok(getCompat(to_native(0x41)), undef);
ok(getCompat(to_native(0x00C0)), _pack_U(0x0041, 0x0300));
ok(getCompat(to_native(0x00EF)), _pack_U(0x0069, 0x0308));
ok(getCompat(0x304C), _pack_U(0x304B, 0x3099));
ok(getCompat(0x1EA4), _pack_U(0x0041, 0x0302, 0x0301));
ok(getCompat(0x1F82), _pack_U(0x03B1, 0x0313, 0x0300, 0x0345));
ok(getCompat(0x1FAF), _pack_U(0x03A9, 0x0314, 0x0342, 0x0345));
ok(getCompat(0x212C), _pack_U(0x0042));
ok(getCompat(0x3243), _pack_U(0x0028, 0x81F3, 0x0029));
ok(getCompat(0xAC00), _pack_U(0x1100, 0x1161));
ok(getCompat(0xAE00), _pack_U(0x1100, 0x1173, 0x11AF));
ok(getCompat(0xFA2D), _pack_U(0x9DB4));

# 34

ok(getComposite(to_native(   0), to_native(   0)), undef);
ok(getComposite(to_native(   0), to_native(0x29)), undef);
ok(getComposite(to_native(0x29), to_native(   0)), undef);
ok(getComposite(to_native(0x29), to_native(0x29)), undef);
ok(getComposite(to_native(   0), to_native(0x41)), undef);
ok(getComposite(to_native(0x41), to_native(   0)), undef);
ok(getComposite(to_native(0x41), to_native(0x41)), undef);
ok(getComposite(to_native(12), to_native(0x0300)), undef);
ok(getComposite(to_native(0x0055), 0xFF00), undef);
ok(getComposite(to_native(0x0041), 0x0300), to_native(0x00C0));
ok(getComposite(to_native(0x0055), 0x0300), to_native(0x00D9));
ok(getComposite(0x0112, 0x0300), 0x1E14);
ok(getComposite(0x1100, 0x1161), 0xAC00);
ok(getComposite(0x1100, 0x1173), 0xADF8);
ok(getComposite(0x1100, 0x11AF), undef);
ok(getComposite(0x1173, 0x11AF), undef);
ok(getComposite(0xAC00, 0x11A7), undef);
ok(getComposite(0xAC00, 0x11A8), 0xAC01);
ok(getComposite(0xADF8, 0x11AF), 0xAE00);

# 53

sub uprops {
  my $uv = shift;
  my $r = "";
     $r .= isExclusion($uv)   ? 'X' : 'x';
     $r .= isSingleton($uv)   ? 'S' : 's';
     $r .= isNonStDecomp($uv) ? 'N' : 'n'; # Non-Starter Decomposition
     $r .= isComp_Ex($uv)     ? 'F' : 'f'; # Full exclusion (X + S + N)
     $r .= isComp2nd($uv)     ? 'B' : 'b'; # B = M = Y
     $r .= isNFD_NO($uv)      ? 'D' : 'd';
     $r .= isNFC_MAYBE($uv)   ? 'M' : 'm'; # Maybe
     $r .= isNFC_NO($uv)      ? 'C' : 'c';
     $r .= isNFKD_NO($uv)     ? 'K' : 'k';
     $r .= isNFKC_MAYBE($uv)  ? 'Y' : 'y'; # maYbe
     $r .= isNFKC_NO($uv)     ? 'G' : 'g';
  return $r;
}

ok(uprops(to_native(0x0000)), 'xsnfbdmckyg'); # NULL
ok(uprops(to_native(0x0029)), 'xsnfbdmckyg'); # RIGHT PARENTHESIS
ok(uprops(to_native(0x0041)), 'xsnfbdmckyg'); # LATIN CAPITAL LETTER A
ok(uprops(to_native(0x00A0)), 'xsnfbdmcKyG'); # NO-BREAK SPACE
ok(uprops(to_native(0x00C0)), 'xsnfbDmcKyg'); # LATIN CAPITAL LETTER A WITH GRAVE
ok(uprops(0x0300), 'xsnfBdMckYg'); # COMBINING GRAVE ACCENT
ok(uprops(0x0344), 'xsNFbDmCKyG'); # COMBINING GREEK DIALYTIKA TONOS
ok(uprops(0x0387), 'xSnFbDmCKyG'); # GREEK ANO TELEIA
ok(uprops(0x0958), 'XsnFbDmCKyG'); # DEVANAGARI LETTER QA
ok(uprops(0x0F43), 'XsnFbDmCKyG'); # TIBETAN LETTER GHA
ok(uprops(0x1100), 'xsnfbdmckyg'); # HANGUL CHOSEONG KIYEOK
ok(uprops(0x1161), 'xsnfBdMckYg'); # HANGUL JUNGSEONG A
ok(uprops(0x11AF), 'xsnfBdMckYg'); # HANGUL JONGSEONG RIEUL
ok(uprops(0x212B), 'xSnFbDmCKyG'); # ANGSTROM SIGN
ok(uprops(0xAC00), 'xsnfbDmcKyg'); # HANGUL SYLLABLE GA
ok(uprops(0xF900), 'xSnFbDmCKyG'); # CJK COMPATIBILITY IDEOGRAPH-F900
ok(uprops(0xFB4E), 'XsnFbDmCKyG'); # HEBREW LETTER PE WITH RAFE
ok(uprops(0xFF71), 'xsnfbdmcKyG'); # HALFWIDTH KATAKANA LETTER A

# 71

ok(decompose(""), "");
ok(decompose("A"), "A");
ok(decompose("", 1), "");
ok(decompose("A", 1), "A");

ok(decompose(hexU("1E14 AC01")), hexU("0045 0304 0300 1100 1161 11A8"));
ok(decompose(hexU("AC00 AE00")), hexU("1100 1161 1100 1173 11AF"));
ok(decompose(hexU("304C FF76")), hexU("304B 3099 FF76"));

ok(decompose(hexU("1E14 AC01"), 1), hexU("0045 0304 0300 1100 1161 11A8"));
ok(decompose(hexU("AC00 AE00"), 1), hexU("1100 1161 1100 1173 11AF"));
ok(decompose(hexU("304C FF76"), 1), hexU("304B 3099 30AB"));

# don't modify the source
my $sDec = "\x{FA19}";
ok(decompose($sDec), "\x{795E}");
ok($sDec, "\x{FA19}");

# 83

ok(reorder(""), "");
ok(reorder("A"), "A");
ok(reorder(hexU("0041 0300 0315 0313 031b 0061")),
	   hexU("0041 031b 0300 0313 0315 0061"));
ok(reorder(hexU("00C1 0300 0315 0313 031b 0061 309A 3099")),
	   hexU("00C1 031b 0300 0313 0315 0061 309A 3099"));

# don't modify the source
my $sReord = "\x{3000}\x{300}\x{31b}";
ok(reorder($sReord), "\x{3000}\x{31b}\x{300}");
ok($sReord, "\x{3000}\x{300}\x{31b}");

# 89

ok(compose(""), "");
ok(compose("A"), "A");
ok(compose(hexU("0061 0300")),      hexU("00E0"));
ok(compose(hexU("0061 0300 031B")), hexU("00E0 031B"));
ok(compose(hexU("0061 0300 0315")), hexU("00E0 0315"));
ok(compose(hexU("0061 0300 0313")), hexU("00E0 0313"));
ok(compose(hexU("0061 031B 0300")), hexU("00E0 031B"));
ok(compose(hexU("0061 0315 0300")), hexU("0061 0315 0300"));
ok(compose(hexU("0061 0313 0300")), hexU("0061 0313 0300"));

# don't modify the source
my $sCom = "\x{304B}\x{3099}";
ok(compose($sCom), "\x{304C}");
ok($sCom, "\x{304B}\x{3099}");

# 100

ok(composeContiguous(""), "");
ok(composeContiguous("A"), "A");
ok(composeContiguous(hexU("0061 0300")),      hexU("00E0"));
ok(composeContiguous(hexU("0061 0300 031B")), hexU("00E0 031B"));
ok(composeContiguous(hexU("0061 0300 0315")), hexU("00E0 0315"));
ok(composeContiguous(hexU("0061 0300 0313")), hexU("00E0 0313"));
ok(composeContiguous(hexU("0061 031B 0300")), hexU("0061 031B 0300"));
ok(composeContiguous(hexU("0061 0315 0300")), hexU("0061 0315 0300"));
ok(composeContiguous(hexU("0061 0313 0300")), hexU("0061 0313 0300"));

# don't modify the source
my $sCtg = "\x{30DB}\x{309A}";
ok(composeContiguous($sCtg), "\x{30DD}");
ok($sCtg, "\x{30DB}\x{309A}");

# 111

sub answer { defined $_[0] ? $_[0] ? "YES" : "NO" : "MAYBE" }

ok(answer(checkNFD("")),  "YES");
ok(answer(checkNFC("")),  "YES");
ok(answer(checkNFKD("")), "YES");
ok(answer(checkNFKC("")), "YES");
ok(answer(check("NFD", "")), "YES");
ok(answer(check("NFC", "")), "YES");
ok(answer(check("NFKD","")), "YES");
ok(answer(check("NFKC","")), "YES");

# U+0000 to U+007F are prenormalized in all the normalization forms.
ok(answer(checkNFD("AZaz\t12!#`")),  "YES");
ok(answer(checkNFC("AZaz\t12!#`")),  "YES");
ok(answer(checkNFKD("AZaz\t12!#`")), "YES");
ok(answer(checkNFKC("AZaz\t12!#`")), "YES");
ok(answer(check("D", "AZaz\t12!#`")), "YES");
ok(answer(check("C", "AZaz\t12!#`")), "YES");
ok(answer(check("KD","AZaz\t12!#`")), "YES");
ok(answer(check("KC","AZaz\t12!#`")), "YES");

ok(answer(checkNFD(NFD(_pack_U(0xC1, 0x1100, 0x1173, 0x11AF)))), "YES");
ok(answer(checkNFD(hexU("20 C1 1100 1173 11AF"))), "NO");
ok(answer(checkNFC(hexU("20 C1 1173 11AF"))), "MAYBE");
ok(answer(checkNFC(hexU("20 C1 AE00 1100"))), "YES");
ok(answer(checkNFC(hexU("20 C1 AE00 1100 0300"))), "MAYBE");
ok(answer(checkNFC(hexU("212B 1100 0300"))), "NO");
ok(answer(checkNFC(hexU("1100 0300 212B"))), "NO");
ok(answer(checkNFC(hexU("0041 0327 030A"))), "MAYBE"); # A+cedilla+ring
ok(answer(checkNFC(hexU("0041 030A 0327"))), "NO");    # A+ring+cedilla
ok(answer(checkNFC(hexU("20 C1 FF71 2025"))),"YES");
ok(answer(check("NFC", hexU("20 C1 212B 300"))), "NO");
ok(answer(checkNFKD(hexU("20 C1 FF71 2025"))),   "NO");
ok(answer(checkNFKC(hexU("20 C1 AE00 2025"))), "NO");
ok(answer(checkNFKC(hexU("212B 1100 0300"))), "NO");
ok(answer(checkNFKC(hexU("1100 0300 212B"))), "NO");
ok(answer(checkNFKC(hexU("0041 0327 030A"))), "MAYBE"); # A+cedilla+ring
ok(answer(checkNFKC(hexU("0041 030A 0327"))), "NO");    # A+ring+cedilla
ok(answer(check("NFKC", hexU("20 C1 212B 300"))), "NO");

# 145

"012ABC" =~ /(\d+)(\w+)/;
ok("012" eq NFC $1 && "ABC" eq NFC $2);

ok(normalize('C', $1), "012");
ok(normalize('C', $2), "ABC");

ok(normalize('NFC', $1), "012");
ok(normalize('NFC', $2), "ABC");
 # s/^NF// in normalize() must not prevent using $1, $&, etc.

# 150

# a string with initial zero should be treated like a number

# LATIN CAPITAL LETTER A WITH GRAVE
ok(getCombinClass(sprintf("0%d", to_native(192))), 0);
ok(getCanon (sprintf("0%d", to_native(192))), _pack_U(0x41, 0x300));
ok(getCompat(sprintf("0%d", to_native(192))), _pack_U(0x41, 0x300));
my $lead_zero = sprintf "0%d", to_native(65);
ok(getComposite($lead_zero, "0768"), to_native(192));
ok(isNFD_NO (sprintf("0%d", to_native(192))));
ok(isNFKD_NO(sprintf("0%d", to_native(192))));

# DEVANAGARI LETTER QA
ok(isExclusion("02392"));
ok(isComp_Ex  ("02392"));
ok(isNFC_NO   ("02392"));
ok(isNFKC_NO  ("02392"));
ok(isNFD_NO   ("02392"));
ok(isNFKD_NO  ("02392"));

# ANGSTROM SIGN
ok(isSingleton("08491"));
ok(isComp_Ex  ("08491"));
ok(isNFC_NO   ("08491"));
ok(isNFKC_NO  ("08491"));
ok(isNFD_NO   ("08491"));
ok(isNFKD_NO  ("08491"));

# COMBINING GREEK DIALYTIKA TONOS
ok(isNonStDecomp("0836"));
ok(isComp_Ex    ("0836"));
ok(isNFC_NO     ("0836"));
ok(isNFKC_NO    ("0836"));
ok(isNFD_NO     ("0836"));
ok(isNFKD_NO    ("0836"));

# COMBINING GRAVE ACCENT
ok(getCombinClass("0768"), 230);
ok(isComp2nd   ("0768"));
ok(isNFC_MAYBE ("0768"));
ok(isNFKC_MAYBE("0768"));

# HANGUL SYLLABLE GA
ok(getCombinClass("044032"), 0);
ok(getCanon("044032"),  _pack_U(0x1100, 0x1161));
ok(getCompat("044032"), _pack_U(0x1100, 0x1161));
ok(getComposite("04352", "04449"), 0xAC00);

# 182

# string with 22 combining characters: (0x300..0x315)
my $str_cc22 = _pack_U(0x3041, 0x300..0x315, 0x3042);
ok(decompose($str_cc22), $str_cc22);
ok(reorder($str_cc22), $str_cc22);
ok(compose($str_cc22), $str_cc22);
ok(composeContiguous($str_cc22), $str_cc22);
ok(NFD($str_cc22), $str_cc22);
ok(NFC($str_cc22), $str_cc22);
ok(NFKD($str_cc22), $str_cc22);
ok(NFKC($str_cc22), $str_cc22);
ok(FCD($str_cc22), $str_cc22);
ok(FCC($str_cc22), $str_cc22);

# 192

# string with 40 combining characters of the same class: (0x300..0x313)x2
my $str_cc40 = _pack_U(0x3041, 0x300..0x313, 0x300..0x313, 0x3042);
ok(decompose($str_cc40), $str_cc40);
ok(reorder($str_cc40), $str_cc40);
ok(compose($str_cc40), $str_cc40);
ok(composeContiguous($str_cc40), $str_cc40);
ok(NFD($str_cc40), $str_cc40);
ok(NFC($str_cc40), $str_cc40);
ok(NFKD($str_cc40), $str_cc40);
ok(NFKC($str_cc40), $str_cc40);
ok(FCD($str_cc40), $str_cc40);
ok(FCC($str_cc40), $str_cc40);

# 202

my $precomp = hexU("304C 304E 3050 3052 3054");
my $combseq = hexU("304B 3099 304D 3099 304F 3099 3051 3099 3053 3099");
ok(decompose($precomp x 5),  $combseq x 5);
ok(decompose($precomp x 10), $combseq x 10);
ok(decompose($precomp x 20), $combseq x 20);

my $hangsyl = hexU("AC00 B098 B2E4 B77C B9C8");
my $jamoseq = hexU("1100 1161 1102 1161 1103 1161 1105 1161 1106 1161");
ok(decompose($hangsyl x 5), $jamoseq x 5);
ok(decompose($hangsyl x 10), $jamoseq x 10);
ok(decompose($hangsyl x 20), $jamoseq x 20);

my $notcomp = hexU("304B 304D 304F 3051 3053");
ok(decompose($precomp . $notcomp),     $combseq . $notcomp);
ok(decompose($precomp . $notcomp x 5), $combseq . $notcomp x 5);
ok(decompose($precomp . $notcomp x10), $combseq . $notcomp x10);

# 211

my $preUnicode3_1 = !defined getCanon(0x1D15E);
my $preUnicode3_2 = !defined getCanon(0x2ADC);

# HEBREW LETTER YOD WITH HIRIQ
ok($preUnicode3_1 xor isExclusion(0xFB1D));
ok($preUnicode3_1 xor isComp_Ex  (0xFB1D));

# MUSICAL SYMBOL HALF NOTE
ok($preUnicode3_1 xor isExclusion(0x1D15E));
ok($preUnicode3_1 xor isComp_Ex  (0x1D15E));

# FORKING
ok($preUnicode3_2 xor isExclusion(0x2ADC));
ok($preUnicode3_2 xor isComp_Ex  (0x2ADC));

# 217


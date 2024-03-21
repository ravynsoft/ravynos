# This is not a widget function, it is only a helper for insert-composed-char
# to cut down on resident memory use.

emulate -L zsh
setopt cbases

# The associative array zsh_accent_chars is indexed by the
# accent.  The values are sets of character / Unicode pairs for
# the character with the given accent.  The Unicode value is
# a hex index with no base discriminator; essentially a UCS-4 index
# with the leading zeroes suppressed.
typeset -gA zsh_accented_chars

# Save quite a lot of space by using short names internally.
local -A z
local a b

# grave
a=\!
z[$a]="\
A C0 E C8 I CC O D2 U D9 a E0 e E8 i EC o F2 u F9 N 1F8 n 1F9 \
"
# acute
a=\'
z[$a]="\
A C1 E C9 I CD O D3 U DA Y DD a E1 e E9 i ED o F3 u FA y FD C 106 c 107 \
L 139 l 13A N 143 n 144 R 154 r 155 S 15A s 15B Z 179 z 17A \
"
# circumflex
a=\>
z[$a]="\
A C2 E CA I CE O D4 U DB a E2 e EA i EE o F4 u FB C 108 c 109 G 11C g 11d \
H 124 h 125 J 134 j 135 S 15C s 15D W 174 w 175 Y 176 y 177 \
"
# tilde
a=\?
z[$a]="\
A C3 E 1EBC N D1 O D5 a E3 e 1EBD n F1 o F5 I 128 i 129 U 168 u 169 \
"
# macron (d-, D- give eth)
a=-
z[$a]="\
A 100 a 101 d F0 D D0 E 112 e 113 I 12a i 12b O 14C o 14D U 16A u 16B \
"
# breve
a=\(
z[$a]="\
A 102 a 103 E 114 e 115 G 11E g 11F I 12C i 12D O 14E o 14F U 16C u 16D "
# dot above, small i with no dot, or l with middle dot
a=.
z[$a]="\
\
C 10A c 10b E 116 e 117 G 120 g 121 I 130 i 131 L 13F l 140 Z 17B z 17C \
"
# diaeresis / Umlaut
a=:
z[$a]="\
A C4 E CB I CF O D6 U DC a E4 e EB i EF o F6 u FC y FF Y 178 \
"
# cedilla
a=,
z[$a]="\
C C7 c E7 G 122 g 123 K 136 k 137 L 13B l 13C N 145 n 146 R 156 r 157 \
S 15E s 15F T 162 t 163 \
"
# underline (_) would go here
# stroke through
a=/
z[$a]="\
O D8 o F8 D 110 d 111 H 126 h 127 L 141 l 142 T 166 t 167 b 180 \
"
# double acute
a=\"
z[$a]="\
O 150 o 151 U 170 u 171\
"
# ogonek
a=\;
z[$a]="\
A 104 a 105 E 118 e 119 I 12E i 12F U 172 u 173 \
"
# caron
a=\<
z[$a]="\
C 10C c 10D D 10E d 10F E 11A e 11B L 13D l 13E N 147 n 148 R 158 r 159 \
S 160 s 161 T 164 t 165 Z 17D z 17E \
"
# ring above
a=0
z[$a]="\
A C5 a E5 U 16E u 16F \
"
# hook above
a=2
z[$a]="\
A 1EA2 a 1EA3 E 1EBA e 1EBA \
"
# horn, also right quotation marks
a=9
z[$a]="\
O 1A0 o 1A1 U 1Af u 1b0 ' 2019 . 201A \" 201D : 201E \
"
# left quotation marks
a=6
z[$a]="\
' 2018 \" 201C \
"
# reversed quotation marks for convenience
a=\'
z[$a]+=" \
9 201B \
"
a=\"
z[$a]+=" \
9 201F \
"

# ligature with E
a=E
z[$a]="\
A C6 O 152 \
"
# ligature with e
a=e
z[$a]="\
a E6 o 153 \
"
# ligature with J
a=J
z[$a]="\
I 132 \
"
# ligature with j
a=j
z[$a]="\
i 133 \
"
# ligature with f
a=f
z[$a]="\
f FB00 \
"
# ligature with i
a=i
z[$a]="\
f FB01 \
"
# ligature with l
a=l
z[$a]="\
f FB02 \
"
# ligature with t
a=t
z[$a]="\
f FB05 s FB06 \
"
# eszett
a=s
z[$a]="\
s DF \
"
# upper case thorn
a=H
z[$a]="\
T DE \
"
# lower case thorn
a=h
z[$a]="\
t FE \
"

# Arabic characters
a=\+
z[$a]+=" \
, 60C ; 61B ? 61F a 627 b 628 t 62A g 62C x 62E d 62F r 631 z 632 s 633 \
c 635 e 639 i 63A + 640 f 641 q 642 k 643 l 644 m 645 n 646 h 647 w 648 \
j 649 y 64A : 64B \" 64C = 64D / 64E ' 64F 1 650 3 651 0 652 p 67E v 6A4 \
"
a=\'
z[$a]+=" H 621"
z[a]+=" \
0 6F0 1 6F1 2 6F2 3 6F3 4 6F4 5 6F5 6 6F6 7 6F7 8 6F8 9 6F9 \
"
z[d]+=" d 636"
z[f]+=" g 6AF"
z[H]+=" a 623 w 624 y 626 z 638"
z[h]+=" a 625"
z[j]+=" t 637"
z[k]+=" t 62B h 62D d 630"
z[M]+=" a 622"
z[m]+=" t 629"
z[n]+=" s 634"
z[S]+=" a 670"

# Cyrillic characters
a=\=
z[$a]+=" \
A 410 B 411 V 412 G 413 D 414 E 415 Z 417 I 418 J 419 K 41A L 41B \
M 41C N 41D O 41E P 41F R 420 S 421 T 422 U 423 F 424 H 425 C 426 \
Y 42B \
a 430 b 431 v 432 g 433 d 434 e 435 z 437 i 438 j 439 k 43A l 43B \
m 43C n 43D o 43E p 43F r 440 s 441 t 442 u 443 f 444 h 445 c 446 \
y 44B \
"
z[%]+=" \
D 402 G 403 J 408 V 40E Z 416 C 427 S 428 z 436 c 447 s 448 \
d 452 g 453 j 458 v 45E \
"
z[A]+=" J 42F"
z[a]+=" j 44F"
z[c]+=" S 429 s 449"
z[E]+=" I 404 J 42D"
z[e]+=" j 44D i 454"
z[I]+=" I 406 Y 407"
z[i]+=" i 456 y 457"
z[J]+=" L 409 N 40A K 40C"
z[j]+=" l 459 n 45A k 45C"
z[O]+=" I 401"
z[o]+=" i 451"
z[S]+=" D 405"
z[s]+=" T 40B d 455 t 45B"
z[U]+=" J 42E"
z[u]+=" j 44E"
z[Z]+=" D 40F"
z[z]+=" d 45F"
a=\"
z[$a]+=" = 42A % 42C"
a=\'
z[$a]+=" = 44A % 44C"
z[3]+=" \
Y 462 y 463 O 46A o 46B F 472 f 473 V 474 v 475 C 480 c 481 \
G 490 g 491 \
"

# Greek characters
a=%
z[$a]+=" \
A 386 E 388 Y 389 I 38A O 38C U 38E W 38F \
a 3Ac e 3Ad y 3Ae i 3AF \
o 3CC u 3CD w 3CE ' 3F4 \
"
a=\*
z[$a]+=" \
A 391 B 392 G 393 D 394 E 395 Z 396 Y 397 H 398 I 399 K 39A L 39B \
M 39C N 39D C 39E O 39F P 3A0 R 3A1 S 3A3 T 3A4 U 3A5 F 3A6 X 3A7 \
Q 3A8 W 3A9 J 3AA V 3Ab \
a 3B1 b 3B2 g 3B3 d 3B4 e 3B5 z 3B6 y 3b7 h 3B8 i 3B9 k 3Ba l 3BB \
m 3BC n 3BD c 3BE o 3BF p 3C0 r 3C1 s 3C3 t 3C4 u 3C5 f 3C6 x 3C7 \
q 3C8 w 3C9 j 3CA v 3CB \
"
a=3
z[$a]+=" \
i 390 u 3B0 T 3DA t 3DB M 3DC m 3DD K 3DE k 3DF P 3E0 p 3E1 j 3F5 \
"
z[s]+=" * 3C2"
z[G]+=" ' 3D8 , 3D9"

# Hebrew characters
a=+
z[$a]+=" \
A 5D0 B 5D1 G 5D2 D 5D3 H 5D4 W 5D5 Z 5D6 X 5D7 J 5D9 K 5DB L 5Dc M 5dE \
N 5E0 S 5E1 E 5E2 P 5E4 Q 5E7 R 5E8 T 5EA \
"
a=j
z[$a]+=" T 5D8 Z 5E5"
a=%
z[$a]+=" K 5DA M 5DD N 5DF P 5E3 "
a=J
z[$a]+=" Z 5e6"
a=h
z[$a]+=" S 5e9"

# Superscripts
a=S
z[$a]+=" \
0 2070 1 B9 2 B2 3 B3 4 2074 5 2075 6 2076 7 2077 8 2078 9 2079 \
+ 207a - 207b = 207C ( 207D ) 207E n 207f \
"
# Subscripts
a=s
z[$a]+=" \
0 2080 1 2081 2 2082 3 2083 4 2084 5 2085 6 2086 7 2087 8 2088 9 2089 \
+ 208a - 208b = 208C ( 208D ) 208E \
"

typeset -i 16 -Z 4 ia
typeset -i 16 -Z 6 iuni
# Extended width characters ^A, ^B, ... (not RFC1345)
for (( ia = 0x21; ia < 0x7f; ia++ )); do
  (( iuni = ia + 0xff00 - 0x20 ))
  eval a="\$'\\x${ia##0x}'"
  z[$a]+=" ^ ${iuni##0x}"
done

# Card suits: here first character is the interesting one
for a b in S 2660 H 2661 D 2662 C 2663; do
  z[$a]+=" c $b"
done

# Music: ditto
for a b in d 2669 8 266a 2 266b b 266d x 266e X 266f; do
  z[$a]+=" M $b"
done

# Remaining characters are handled as separate pairs.
# We need to remember that the assoc array is keyed by the second character.
# Left square bracket
a=\(
z[$a]+=" < 5B"
# Reverse solidus (backslash to you and me).
a=/
z[$a]+=" / 5C"
# Right square bracket, circumflex
a=\>
z[$a]+=" ) 5D ' 5E"
# Grave a
a=\!
z[$a]+=" ' 60"
# diglyphys for (usually) standard characters {, |, }, ~
a=\!
z[$a]+=" ( 7B"
z[$a]+=" ! 7C"
a=\)
z[$a]+=" ! 7D"
a=\?
z[$a]+=" ' 7E"
# non-breaking space
z[S]+=" N A0"
# inverted exclamation mark
z[I]+=" ! A1"
# cent
z[t]+=" C A2"
# pound sterling
z[d]+=" P A3"
# currency
z[u]+=" C A4"
# yen
z[e]+=" Y A5"
# broken bar
z[B]+=" B A6"
# section
z[E]+=" S A7"
# lonely diaeresis
z[:]+=" ' A8"
# copyright
z[o]+=" C A9"
# spanish feminine ordinal
z[a]+=" - AA"
# left guillemet
a=\<
z[$a]+=" < AB"
z[O]+=" N AC"
# soft hyphen
z[-]+=" - AD"
# registered
z[g]+=" R AE"
# lonely macron
z[m]+=" ' AF"
# degree
z[G]+=" D B0"
# degree centigrade
z[C]+=" o 2103"
# degree fahrenheit
z[F]+=" o 2109"
# numero
z[0]+=" N 2116"
# +/-
z[-]+=" + B1"
# lonely acute
a=\'
z[$a]+=" ' B4"
# micro
z[y]+=" M B5"
# pilcrow (paragraph)
z[I]+=" P B6"
# Middle dot
z[M]+=" . B7"
# Lonely cedilla
z[,]+=" ' B8"
# spanish masculine ordinal
z[o]+=" - BA"
# right guillemet
a=\>
z[$a]+=" > BB"
# fractions
z[4]+=" 1 BC 3 BE"
z[2]+=" 1 BD"
# inverted question mark
z[I]+=" ? BF"
# multiplication
z[X]+=" * D7"
# division
z[:]+=" - F7"
# kra
z[k]+=" k 138"
# apostrophe n
z[n]+=" ' 149"
# Lappish ng
z[G]+=" N 14A"
z[g]+=" n 14B"
# OI
z[I]+=" O 1A2"
z[i]+=" o 1A3"
# yr
z[r]+=" y 1A6"
# ezh
z[D]+=" E 1B7"
# euro (I invented this but it's logical)
z[u]+=" E 20AC"
# hyphen
z[1]+=" - 2010"
# en dash
z[N]+=" - 2013"
# em dash
z[M]+=" - 2014"
# horizontal bar, vertical and horizontal ellipsis
z[3]+=" - 2015 : 22EE . 22EF"
# double vertical line, double low line
z[2]+=" ! 2016 = 2017"
# dagger and double dagger
z[-]+=" / 2020"
z[=]+=" / 2021"
# per mille
z[0]+=" % 2030"
# For all, partial derivative, there exists, empty set
z[A]+=" F 2200"
z[P]+=" d 2202"
z[E]+=" T 2203"
z[0]+=" / 2205"
# Increment, del (nabla), element of, contains, product, sum
z[E]+=" D 2206"
z[B]+=" N 2207"
z[-]+=" ( 2208"
a=\)
z[$a]+=" - 220b"
z[P]+=" * 220F"
z[Z]+=" + 2211"
# Minus, minus or plus, asterisk, ring, bullet
z[2]+=" - 2212"
z[+]+=" - 2213"
z[-]+=" * 2217"
z[b]+=" O 2218 S 2219"
# square root, proportional to, infinity
z[T]+=" R 221A"
a=\(
z[$a]+=" 0 221D"
z[0]+=" 0 221E"
# Female and male
z[m]+=" F 2640"
z[l]+=" M 2642"
# Commercial AT
z[t]+=" A 40"
# Prime, double prime, triple prime
a=\'
z[$a]+=" 1 2032 2 2033 3 2034"
# Arrows
z[-]+=" < 2190"
a=\!
z[$a]+=" - 2191"
a=\>
z[$a]+=" - 2192 < 2194 = 21D2"
z[v]+=" - 2193"
z[D]+=" U 2195"
a=\=
z[$a]+=" < 21D0 = 21D4"

zsh_accented_chars=("${(kv)z[@]}")

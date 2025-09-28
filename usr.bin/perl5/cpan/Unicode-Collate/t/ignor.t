
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..41\n"; }
my $count = 0;
sub ok ($;$) {
    my $p = my $r = shift;
    if (@_) {
	my $x = shift;
	$p = !defined $x ? !defined $r : !defined $r ? 0 : $r eq $x;
    }
    print $p ? "ok" : "not ok", ' ', ++$count, "\n";
}

use Unicode::Collate;

ok(1);

sub _pack_U   { Unicode::Collate::pack_U(@_) }
sub _unpack_U { Unicode::Collate::unpack_U(@_) }

#########################

my $trad = Unicode::Collate->new(
  table => 'keys.txt',
  normalization => undef,
  ignoreName => qr/HANGUL|HIRAGANA|KATAKANA|BOPOMOFO/,
  level => 3,
  entry => << 'ENTRIES',
 0063 0068 ; [.0A3F.0020.0002.0063] % "ch" in traditional Spanish
 0043 0068 ; [.0A3F.0020.0007.0043] # "Ch" in traditional Spanish
 0043 0048 ; [.0A3F.0020.0008.0043] # "CH" in traditional Spanish
ENTRIES
);
# 0063  ; [.0A3D.0020.0002.0063] # LATIN SMALL LETTER C
# 0064  ; [.0A49.0020.0002.0064] # LATIN SMALL LETTER D

##### 2..3

ok(
  join(':', $trad->sort( qw/ acha aca ada acia acka / ) ),
  join(':',              qw/ aca acia acka acha ada / ),
);

ok(
  join(':', $trad->sort( qw/ ACHA ACA ADA ACIA ACKA / ) ),
  join(':',              qw/ ACA ACIA ACKA ACHA ADA / ),
);

##### 4..7

ok($trad->gt("ocho", "oc\cAho")); # UCA v14
ok($trad->gt("ocho", "oc\0\cA\0\cBho"));  # UCA v14
ok($trad->eq("-", ""));
ok($trad->gt("ocho", "oc-ho"));

##### 8..11

$trad->change(UCA_Version => 9);

ok($trad->eq("ocho", "oc\cAho")); # UCA v9
ok($trad->eq("ocho", "oc\0\cA\0\cBho")); # UCA v9
ok($trad->eq("-", ""));
ok($trad->gt("ocho", "oc-ho"));

##### 12..15

$trad->change(UCA_Version => 8);

ok($trad->gt("ocho", "oc\cAho"));
ok($trad->gt("ocho", "oc\0\cA\0\cBho"));
ok($trad->eq("-", ""));
ok($trad->gt("ocho", "oc-ho"));


##### 16..19

$trad->change(UCA_Version => 9);

my $hiragana = "\x{3042}\x{3044}";
my $katakana = "\x{30A2}\x{30A4}";

# HIRAGANA and KATAKANA are ignorable via ignoreName
ok($trad->eq($hiragana, ""));
ok($trad->eq("", $katakana));
ok($trad->eq($hiragana, $katakana));
ok($trad->eq($katakana, $hiragana));


##### 20..31

# According to Conformance Test (UCA_Version == 9 or 11),
# a L3-ignorable is treated as a completely ignorable.

my $L3ignorable = Unicode::Collate->new(
  alternate => 'Non-ignorable',
  level => 3,
  table => undef,
  normalization => undef,
  UCA_Version => 9,
  entry => <<'ENTRIES',
0000  ; [.0000.0000.0000.0000] # [0000] NULL (in 6429)
0001  ; [.0000.0000.0000.0000] # [0001] START OF HEADING (in 6429)
0591  ; [.0000.0000.0000.0591] # HEBREW ACCENT ETNAHTA
1D165 ; [.0000.0000.0000.1D165] # MUSICAL SYMBOL COMBINING STEM
0021  ; [*024B.0020.0002.0021] # EXCLAMATION MARK
09BE  ; [.114E.0020.0002.09BE] # BENGALI VOWEL SIGN AA
09C7  ; [.1157.0020.0002.09C7] # BENGALI VOWEL SIGN E
09CB  ; [.1159.0020.0002.09CB] # BENGALI VOWEL SIGN O
09C7 09BE ; [.1159.0020.0002.09CB] # BENGALI VOWEL SIGN O
1D1B9 ; [*098A.0020.0002.1D1B9] # MUSICAL SYMBOL SEMIBREVIS WHITE
1D1BA ; [*098B.0020.0002.1D1BA] # MUSICAL SYMBOL SEMIBREVIS BLACK
1D1BB ; [*098A.0020.0002.1D1B9][.0000.0000.0000.1D165] # M.S. MINIMA
1D1BC ; [*098B.0020.0002.1D1BA][.0000.0000.0000.1D165] # M.S. MINIMA BLACK
ENTRIES
);

ok($L3ignorable->lt("\cA", "!"));
ok($L3ignorable->lt("\x{591}", "!"));
ok($L3ignorable->eq("\cA", "\x{591}"));
ok($L3ignorable->eq("\x{9C7}\x{9BE}A", "\x{9C7}\cA\x{9BE}A"));
ok($L3ignorable->eq("\x{9C7}\x{9BE}A", "\x{9C7}\x{591}\x{9BE}A"));
ok($L3ignorable->eq("\x{9C7}\x{9BE}A", "\x{9C7}\x{1D165}\x{9BE}A"));
ok($L3ignorable->eq("\x{9C7}\x{9BE}A", "\x{9CB}A"));
ok($L3ignorable->lt("\x{1D1BB}", "\x{1D1BC}"));
ok($L3ignorable->eq("\x{1D1BB}", "\x{1D1B9}"));
ok($L3ignorable->eq("\x{1D1BC}", "\x{1D1BA}"));
ok($L3ignorable->eq("\x{1D1BB}", "\x{1D1B9}\x{1D165}"));
ok($L3ignorable->eq("\x{1D1BC}", "\x{1D1BA}\x{1D165}"));

##### 32..41

my $c = Unicode::Collate->new(
  table => 'keys.txt',
  normalization => undef,
  level => 1,
  UCA_Version => 14,
  entry => << 'ENTRIES',
034F  ; [.0000.0000.0000.034F] # COMBINING GRAPHEME JOINER
0063 0068 ; [.0A3F.0020.0002.0063] % "ch" in traditional Spanish
0043 0068 ; [.0A3F.0020.0007.0043] # "Ch" in traditional Spanish
0043 0048 ; [.0A3F.0020.0008.0043] # "CH" in traditional Spanish
ENTRIES
);
# 0063  ; [.0A3D.0020.0002.0063] # LATIN SMALL LETTER C
# 0064  ; [.0A49.0020.0002.0064] # LATIN SMALL LETTER D

ok($c->gt("ocho", "oc\x00\x00ho"));
ok($c->gt("ocho", "oc\cAho"));
ok($c->gt("ocho", "oc\x{34F}ho"));
ok($c->gt("ocio", "oc\x{34F}ho"));
ok($c->lt("ocgo", "oc\x{34F}ho"));
ok($c->lt("oceo", "oc\x{34F}ho"));

ok($c->viewSortKey("ocho"),         "[0B4B 0A3F 0B4B | | |]");
ok($c->viewSortKey("oc\x00\x00ho"), "[0B4B 0A3D 0AB9 0B4B | | |]");
ok($c->viewSortKey("oc\cAho"),      "[0B4B 0A3D 0AB9 0B4B | | |]");
ok($c->viewSortKey("oc\x{34F}ho"),  "[0B4B 0A3D 0AB9 0B4B | | |]");



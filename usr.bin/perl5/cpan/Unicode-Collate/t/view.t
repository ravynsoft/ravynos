
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..214\n"; } # 62 + 8 x @Versions
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

##### 1

my $Collator = Unicode::Collate->new(
  table => 'keys.txt',
  normalization => undef,
  UCA_Version => 24,
);

ok($Collator->viewSortKey(""),		'[| | |]');
ok($Collator->viewSortKey("\0"),	'[| | |]');
ok($Collator->viewSortKey("\x{200B}"),	'[| | |]');

ok($Collator->viewSortKey("A"), '[0A15 | 0020 | 0008 | FFFF]');
ok($Collator->viewSortKey('a'), '[0A15 | 0020 | 0002 | FFFF]');

ok($Collator->viewSortKey("ABC"),
    "[0A15 0A29 0A3D | 0020 0020 0020 | 0008 0008 0008 | FFFF FFFF FFFF]");

ok($Collator->viewSortKey("(12)"),
    "[0A0C 0A0D | 0020 0020 | 0002 0002 | 027A FFFF FFFF 027B]");

ok($Collator->viewSortKey("!\x{300}"), "[| | | 024B]");

ok($Collator->viewSortKey("\x{300}"), "[| 0035 | 0002 | FFFF]");

ok($Collator->viewSortKey("\x{304C}"),
    '[1926 | 0020 013D | 000E 0002 | FFFF FFFF]');

ok($Collator->viewSortKey("\x{4E00}"),
    '[FB40 CE00 | 0020 | 0002 | FFFF FFFF]');

ok($Collator->viewSortKey("\x{100000}"),
    '[FBE0 8000 | 0020 | 0002 | FFFF FFFF]');

$Collator->change(level => 3);
ok($Collator->viewSortKey("A"), "[0A15 | 0020 | 0008 |]");

$Collator->change(level => 2);
ok($Collator->viewSortKey("A"), "[0A15 | 0020 | |]");

$Collator->change(level => 1);
ok($Collator->viewSortKey("A"), "[0A15 | | |]");

##### 16

$Collator->change(level => 4, UCA_Version => 8);

ok($Collator->viewSortKey(""), "[|||]");

ok($Collator->viewSortKey("A"), "[0A15|0020|0008|FFFF]");

ok($Collator->viewSortKey("ABC"),
    "[0A15 0A29 0A3D|0020 0020 0020|0008 0008 0008|FFFF FFFF FFFF]");

ok($Collator->viewSortKey("(12)"),
    "[0A0C 0A0D|0020 0020|0002 0002|027A FFFF FFFF 027B]");

ok($Collator->viewSortKey("!\x{300}"), "[|0035|0002|024B FFFF]");

ok($Collator->viewSortKey("\x{300}"), "[|0035|0002|FFFF]");

ok($Collator->viewSortKey("\x{304C}"),
    '[1926|0020 013D|000E 0002|FFFF FFFF]');

ok($Collator->viewSortKey("\x{4E00}"),
    '[4E00|0020|0002|FFFF]');

ok($Collator->viewSortKey("\x{100000}"),
    '[FFA0 8000|0002|0001|FFFF FFFF]');

$Collator->change(level => 3);
ok($Collator->viewSortKey("A"), "[0A15|0020|0008|]");

$Collator->change(level => 2);
ok($Collator->viewSortKey("A"), "[0A15|0020||]");

$Collator->change(level => 1);
ok($Collator->viewSortKey("A"), "[0A15|||]");

##### 28

$Collator->change(level => 3, UCA_Version => 9);
ok($Collator->viewSortKey("A\x{300}z\x{301}"),
    "[0A15 0C13 | 0020 0035 0020 0032 | 0008 0002 0002 0002 |]");

$Collator->change(backwards => 1);
ok($Collator->viewSortKey("A\x{300}z\x{301}"),
    "[0C13 0A15 | 0020 0035 0020 0032 | 0008 0002 0002 0002 |]");

$Collator->change(backwards => 2);
ok($Collator->viewSortKey("A\x{300}z\x{301}"),
    "[0A15 0C13 | 0032 0020 0035 0020 | 0008 0002 0002 0002 |]");

$Collator->change(backwards => [1,3]);
ok($Collator->viewSortKey("A\x{300}z\x{301}"),
    "[0C13 0A15 | 0020 0035 0020 0032 | 0002 0002 0002 0008 |]");

$Collator->change(backwards => [2]);
ok($Collator->viewSortKey("\x{300}\x{301}\x{302}\x{303}"),
    "[| 004E 003C 0032 0035 | 0002 0002 0002 0002 |]");

$Collator->change(backwards => []);
ok($Collator->viewSortKey("A\x{300}z\x{301}"),
    "[0A15 0C13 | 0020 0035 0020 0032 | 0008 0002 0002 0002 |]");

##### 34

$Collator->change(level => 4);

# Variable

our %origVar = $Collator->change(variable => 'Blanked');
ok($Collator->viewSortKey("1+2"),
    '[0A0C 0A0D | 0020 0020 | 0002 0002 | 0031 002B 0032]');

ok($Collator->viewSortKey("?\x{300}!\x{301}\x{315}."),
    '[| | | 003F 0021 002E]');

ok($Collator->viewSortKey("?!."), '[| | | 003F 0021 002E]');

$Collator->change(variable => 'Non-ignorable');
ok($Collator->viewSortKey("1+2"),
    '[0A0C 039F 0A0D | 0020 0020 0020 | 0002 0002 0002 | 0031 002B 0032]');

ok($Collator->viewSortKey("?\x{300}!"),
    '[024E 024B | 0020 0035 0020 | 0002 0002 0002 | 003F 0300 0021]');

ok($Collator->viewSortKey("?!."),
    '[024E 024B 0255 | 0020 0020 0020 | 0002 0002 0002 | 003F 0021 002E]');

$Collator->change(variable => 'Shifted');
ok($Collator->viewSortKey("1+2"),
    '[0A0C 0A0D | 0020 0020 | 0002 0002 | FFFF 039F FFFF]');

ok($Collator->viewSortKey("?\x{300}!\x{301}\x{315}."),
    '[| | | 024E 024B 0255]');

ok($Collator->viewSortKey("?!."), '[| | | 024E 024B 0255]');

$Collator->change(variable => 'Shift-Trimmed');
ok($Collator->viewSortKey("1+2"),
    '[0A0C 0A0D | 0020 0020 | 0002 0002 | 039F]');

ok($Collator->viewSortKey("?\x{300}!\x{301}\x{315}."),
    '[| | | 024E 024B 0255]');

ok($Collator->viewSortKey("?!."), '[| | | 024E 024B 0255]');

$Collator->change(%origVar);

##### 46

# Level 3 weight

ok($Collator->viewSortKey("a\x{3042}"),
    '[0A15 1921 | 0020 0020 | 0002 000E | FFFF FFFF]');

ok($Collator->viewSortKey("A\x{30A2}"),
    '[0A15 1921 | 0020 0020 | 0008 0011 | FFFF FFFF]');

$Collator->change(upper_before_lower => 1);

ok($Collator->viewSortKey("a\x{3042}"),
    '[0A15 1921 | 0020 0020 | 0008 000E | FFFF FFFF]');

ok($Collator->viewSortKey("A\x{30A2}"),
    '[0A15 1921 | 0020 0020 | 0002 0011 | FFFF FFFF]');

$Collator->change(katakana_before_hiragana => 1);

ok($Collator->viewSortKey("a\x{3042}"),
    '[0A15 1921 | 0020 0020 | 0008 0013 | FFFF FFFF]');
ok($Collator->viewSortKey("A\x{30A2}"),
    '[0A15 1921 | 0020 0020 | 0002 000F | FFFF FFFF]');

$Collator->change(upper_before_lower => 0);

ok($Collator->viewSortKey("a\x{3042}"),
    '[0A15 1921 | 0020 0020 | 0002 0013 | FFFF FFFF]');

ok($Collator->viewSortKey("A\x{30A2}"),
    '[0A15 1921 | 0020 0020 | 0008 000F | FFFF FFFF]');

$Collator->change(katakana_before_hiragana => 0);

ok($Collator->viewSortKey("a\x{3042}"),
    '[0A15 1921 | 0020 0020 | 0002 000E | FFFF FFFF]');

ok($Collator->viewSortKey("A\x{30A2}"),
    '[0A15 1921 | 0020 0020 | 0008 0011 | FFFF FFFF]');

##### 56

our $el = Unicode::Collate->new(
  entry => <<'ENTRY',
006C ; [.0B03.0020.0002.006C] # LATIN SMALL LETTER L
FF4C ; [.0B03.0020.0003.FF4C] # FULLWIDTH LATIN SMALL LETTER L; QQK
217C ; [.0B03.0020.0004.217C] # SMALL ROMAN NUMERAL FIFTY; QQK
2113 ; [.0B03.0020.0005.2113] # SCRIPT SMALL L; QQK
24DB ; [.0B03.0020.0006.24DB] # CIRCLED LATIN SMALL LETTER L; QQK
004C ; [.0B03.0020.0008.004C] # LATIN CAPITAL LETTER L
FF2C ; [.0B03.0020.0009.FF2C] # FULLWIDTH LATIN CAPITAL LETTER L; QQK
216C ; [.0B03.0020.000A.216C] # ROMAN NUMERAL FIFTY; QQK
2112 ; [.0B03.0020.000B.2112] # SCRIPT CAPITAL L; QQK
24C1 ; [.0B03.0020.000C.24C1] # CIRCLED LATIN CAPITAL LETTER L; QQK
ENTRY
  table => undef,
  normalization => undef,
  UCA_Version => 24,
);

our $el12 = '0B03 0B03 0B03 0B03 0B03 | 0020 0020 0020 0020 0020';

ok($el->viewSortKey("l\x{FF4C}\x{217C}\x{2113}\x{24DB}"),
    "[$el12 | 0002 0003 0004 0005 0006 | FFFF FFFF FFFF FFFF FFFF]");

ok($el->viewSortKey("L\x{FF2C}\x{216C}\x{2112}\x{24C1}"),
    "[$el12 | 0008 0009 000A 000B 000C | FFFF FFFF FFFF FFFF FFFF]");

$el->change(upper_before_lower => 1);

ok($el->viewSortKey("l\x{FF4C}\x{217C}\x{2113}\x{24DB}"),
    "[$el12 | 0008 0009 000A 000B 000C | FFFF FFFF FFFF FFFF FFFF]");

ok($el->viewSortKey("L\x{FF2C}\x{216C}\x{2112}\x{24C1}"),
    "[$el12 | 0002 0003 0004 0005 0006 | FFFF FFFF FFFF FFFF FFFF]");

$el->change(upper_before_lower => 0);

ok($el->viewSortKey("l\x{FF4C}\x{217C}\x{2113}\x{24DB}"),
    "[$el12 | 0002 0003 0004 0005 0006 | FFFF FFFF FFFF FFFF FFFF]");

ok($el->viewSortKey("L\x{FF2C}\x{216C}\x{2112}\x{24C1}"),
    "[$el12 | 0008 0009 000A 000B 000C | FFFF FFFF FFFF FFFF FFFF]");

##### 62

my @Versions = ( 8,  9, 11, 14, 16, 18, 20, 22, 24, 26,
		28, 30, 32, 34, 36, 38, 40, 41, 43);

for my $v (@Versions) {
    $Collator->change(UCA_Version => $v);

    # primary weights
    my $pri1 = '0A0C 0A0D';
    my $pri2 = '0A0C 039F 0A0D';
    my $pri3 = $v >= 9 ? 'FB40 CE02' : '4E02';

    # secondary weights
    my $sec1 = '0020';
    my $sec2 = '0020 0020';
    my $sec3 = '0020 0020 0020';

    # tertiary weights
    my $ter1 = '0002';
    my $ter2 = '0002 0002';
    my $ter3 = '0002 0002 0002';

    # quaternary weights
    my $eququat = 'FFFF 039F FFFF';
    my $hanquat = $v >= 36 || $v == 8 ? 'FFFF' : 'FFFF FFFF';

    # separators
    my $sep1 = $v >= 9 ? ' |'  : '|';
    my $sep2 = $v >= 9 ? ' | ' : '|';

    my $app = $v >= 26 ? ' |]' : ']';

    $Collator->change(variable => 'Shifted', level => 4);
    ok($Collator->viewSortKey("1+2"),
	"[$pri1$sep2$sec2$sep2$ter2$sep2$eququat$app");
    ok($Collator->viewSortKey("\x{4E02}"),
	"[$pri3$sep2$sec1$sep2$ter1$sep2$hanquat$app");

    $Collator->change(variable => 'Shift-Trimmed');
    ok($Collator->viewSortKey("1+2"),
	"[$pri1$sep2$sec2$sep2$ter2$sep2"."039F$app");
    ok($Collator->viewSortKey("\x{4E02}"),
	"[$pri3$sep2$sec1$sep2$ter1$sep1$app");

    $Collator->change(variable => 'Non-ignorable', level => 3);
    ok($Collator->viewSortKey("1+2"),
	"[$pri2$sep2$sec3$sep2$ter3$sep1]");
    ok($Collator->viewSortKey("\x{4E02}"),
	"[$pri3$sep2$sec1$sep2$ter1$sep1]");

    $Collator->change(variable => 'Blanked');
    ok($Collator->viewSortKey("1+2"),
	"[$pri1$sep2$sec2$sep2$ter2$sep1]");
    ok($Collator->viewSortKey("\x{4E02}"),
	"[$pri3$sep2$sec1$sep2$ter1$sep1]");
}


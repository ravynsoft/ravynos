
BEGIN {
    unless (5.008 <= $]) {
	print "1..0 # skipped: Perl 5.8.0 or later needed for this test\n";
	print $@;
	exit;
    }
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..176\n"; } # 81 + 5 x @Versions
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

no warnings 'utf8';

# NULL is tailorable but illegal code points are not.
# illegal code points should be always ingored
# (cf. UCA, 7.1.1 Illegal code points).

my $entry = <<'ENTRIES';
0000  ; [.0020.0000.0000.0000] # [0000] NULL
0001  ; [.0021.0000.0000.0001] # [0001] START OF HEADING
FFFE  ; [.0022.0000.0000.FFFE] # <noncharacter-FFFE> (invalid)
FFFF  ; [.0023.0000.0000.FFFF] # <noncharacter-FFFF> (invalid)
D800  ; [.0024.0000.0000.D800] # <surrogate-D800> (invalid)
DFFF  ; [.0025.0000.0000.DFFF] # <surrogate-DFFF> (invalid)
FDD0  ; [.0026.0000.0000.FDD0] # <noncharacter-FDD0> (invalid)
FDEF  ; [.0027.0000.0000.FDEF] # <noncharacter-FDEF> (invalid)
0002  ; [.0030.0000.0000.0002] # [0002] START OF TEXT
10FFFF; [.0040.0000.0000.10FFFF] # <noncharacter-10FFFF> (invalid)
110000; [.0041.0000.0000.110000] # <out-of-range 110000> (invalid)
0041  ; [.1000.0020.0008.0041] # latin A
0041 0000 ; [.1100.0020.0008.0041] # latin A + NULL
0041 FFFF ; [.1200.0020.0008.0041] # latin A + FFFF (invalid)
ENTRIES

##################

my $illeg = Unicode::Collate->new(
  entry => $entry,
  level => 1,
  table => undef,
  normalization => undef,
  UCA_Version => 20,
);

# 2..12
ok($illeg->lt("", "\x00"));
ok($illeg->lt("", "\x01"));
ok($illeg->eq("", "\x{FFFE}"));
ok($illeg->eq("", "\x{FFFF}"));
ok($illeg->eq("", "\x{D800}"));
ok($illeg->eq("", "\x{DFFF}"));
ok($illeg->eq("", "\x{FDD0}"));
ok($illeg->eq("", "\x{FDEF}"));
ok($illeg->lt("", "\x02"));
ok($illeg->eq("", "\x{10FFFF}"));
ok($illeg->eq("", "\x{110000}"));

# 13..22
ok($illeg->lt("\x00", "\x01"));
ok($illeg->lt("\x01", "\x02"));
ok($illeg->ne("\0", "\x{D800}"));
ok($illeg->ne("\0", "\x{DFFF}"));
ok($illeg->ne("\0", "\x{FDD0}"));
ok($illeg->ne("\0", "\x{FDEF}"));
ok($illeg->ne("\0", "\x{FFFE}"));
ok($illeg->ne("\0", "\x{FFFF}"));
ok($illeg->ne("\0", "\x{10FFFF}"));
ok($illeg->ne("\0", "\x{110000}"));

# 23..26
ok($illeg->eq("A",   "A\x{FFFF}"));
ok($illeg->gt("A\0", "A\x{FFFF}"));
ok($illeg->lt("A",  "A\0"));
ok($illeg->lt("AA", "A\0"));

##################

my $nonch = Unicode::Collate->new(
  entry => $entry,
  level => 1,
  table => undef,
  normalization => undef,
  UCA_Version => 22,
);

# 27..37
ok($nonch->lt("", "\x00"));
ok($nonch->lt("", "\x01"));
ok($nonch->lt("", "\x{FFFE}"));
ok($nonch->lt("", "\x{FFFF}"));
ok($nonch->lt("", "\x{D800}"));
ok($nonch->lt("", "\x{DFFF}"));
ok($nonch->lt("", "\x{FDD0}"));
ok($nonch->lt("", "\x{FDEF}"));
ok($nonch->lt("", "\x02"));
ok($nonch->lt("", "\x{10FFFF}"));
ok($nonch->lt("", "\x{110000}"));

# 38..47
ok($nonch->lt("\x00",     "\x01"));
ok($nonch->lt("\x01",     "\x{FFFE}"));
ok($nonch->lt("\x{FFFE}", "\x{FFFF}"));
ok($nonch->lt("\x{FFFF}", "\x{D800}"));
ok($nonch->lt("\x{D800}", "\x{DFFF}"));
ok($nonch->lt("\x{DFFF}", "\x{FDD0}"));
ok($nonch->lt("\x{FDD0}", "\x{FDEF}"));
ok($nonch->lt("\x{FDEF}", "\x02"));
ok($nonch->lt("\x02",     "\x{10FFFF}"));
ok($nonch->lt("\x{10FFFF}", "\x{110000}"));

# 48..51
ok($nonch->lt("A",   "A\x{FFFF}"));
ok($nonch->lt("A\0", "A\x{FFFF}"));
ok($nonch->lt("A",  "A\0"));
ok($nonch->lt("AA", "A\0"));

##################

my $Collator = Unicode::Collate->new(
  table => 'keys.txt',
  level => 1,
  normalization => undef,
  UCA_Version => 8,
);

my @ret = (
    "Pe\x{300}\x{301}",
    "Pe\x{300}\0\0\x{301}",
    "Pe\x{DA00}\x{301}\x{DFFF}",
    "Pe\x{FFFF}\x{301}",
    "Pe\x{110000}\x{301}",
    "Pe\x{300}\x{d801}\x{301}",
    "Pe\x{300}\x{ffff}\x{301}",
    "Pe\x{300}\x{110000}\x{301}",
    "Pe\x{D9ab}\x{DFFF}",
    "Pe\x{FFFF}",
    "Pe\x{110000}",
    "Pe\x{300}\x{D800}\x{DFFF}",
    "Pe\x{300}\x{FFFF}",
    "Pe\x{300}\x{110000}",
);

# 52..65
for my $ret (@ret) {
    my $str = $ret."rl";
    my($match) = $Collator->match($str, "pe");
    ok($match eq $ret);
}

##################

my $out = Unicode::Collate->new(
    level => 1,
    table => undef,
    normalization => undef,
    overrideOut => sub { 0xFFFD },
);

my @Versions = ( 8,  9, 11, 14, 16, 18, 20, 22, 24, 26,
		28, 30, 32, 34, 36, 38, 40, 41, 43);

for my $v (@Versions) {
    $out->change(UCA_Version => $v);
    ok($out->cmp('',           "\x{10FFFF}") == ($v >= 22 ? -1 : 0));
    ok($out->cmp('',           "\x{110000}") == ($v >= 22 ? -1 : 0));
    ok($out->cmp('ABC',        "\x{110000}") == ($v >= 22 ? -1 : 1));
    ok($out->cmp("\x{10FFFD}", "\x{110000}") == ($v >= 22 ? -1 : 1));
    ok($out->cmp("\x{11FFFD}", "\x{110000}") == ($v >= 22 ?  0 : 0));
}

# x+66..x+77
ok($out->lt('ABC',      "\x{123456}"));
ok($out->lt("\x{FFFD}", "\x{123456}"));

$out->change(overrideOut => sub {()});

ok($out->eq('',         "\x{123456}"));
ok($out->gt('ABC',      "\x{123456}"));
ok($out->gt("\x{FFFD}", "\x{123456}"));

$out->change(overrideOut => undef);
ok($out->lt('',         "\x{123456}"));
ok($out->eq("\x{FFFD}", "\x{123456}"));

$out->change(overrideOut => sub { 0xFFFD });

ok($out->lt('',         "\x{123456}"));
ok($out->lt('ABC',      "\x{123456}"));
ok($out->lt("\x{FFFD}", "\x{123456}"));

$out->change(overrideOut => 0);
ok($out->lt('',         "\x{123456}"));
ok($out->eq("\x{FFFD}", "\x{123456}"));

$out->change(overrideOut => sub { undef });
ok($out->lt('',         "\x{123456}"));
ok($out->eq("\x{FFFD}", "\x{123456}"));
ok($out->eq("\x{FFFD}", "\x{21FFFFF}"));
ok($out->eq("\x{FFFD}", "\x{2200000}"));


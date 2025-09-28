
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

BEGIN {
    eval { require Unicode::Normalize; };
    if ($@) {
	print "1..0 # skipped: Unicode::Normalize needed for this test\n";
	print $@;
	exit;
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..101\n"; }
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

our $Aring = _pack_U(0xC5);
our $aring = _pack_U(0xE5);

our $entry = <<'ENTRIES';
030A; [.0000.030A.0002] # COMBINING RING ABOVE
212B; [.002B.0020.0008] # ANGSTROM SIGN
0061; [.0A41.0020.0002] # LATIN SMALL LETTER A
0041; [.0A41.0020.0008] # LATIN CAPITAL LETTER A
007A; [.0A5A.0020.0002] # LATIN SMALL LETTER Z
005A; [.0A5A.0020.0008] # LATIN CAPITAL LETTER Z
FF41; [.0A87.0020.0002] # LATIN SMALL LETTER A
FF21; [.0A87.0020.0008] # LATIN CAPITAL LETTER A
00E5; [.0AC5.0020.0002] # LATIN SMALL LETTER A WITH RING ABOVE
00C5; [.0AC5.0020.0008] # LATIN CAPITAL LETTER A WITH RING ABOVE
ENTRIES

# Aong < A+ring < Z < fullA+ring < A-ring

#########################

our $noN = Unicode::Collate->new(
    level => 1,
    table => undef,
    normalization => undef,
    entry => $entry,
);

our $nfc = Unicode::Collate->new(
  level => 1,
  table => undef,
  normalization => 'NFC',
  entry => $entry,
);

our $nfd = Unicode::Collate->new(
  level => 1,
  table => undef,
  normalization => 'NFD',
  entry => $entry,
);

our $nfkc = Unicode::Collate->new(
  level => 1,
  table => undef,
  normalization => 'NFKC',
  entry => $entry,
);

our $nfkd = Unicode::Collate->new(
  level => 1,
  table => undef,
  normalization => 'NFKD',
  entry => $entry,
);

ok($noN->lt("\x{212B}", "A"));
ok($noN->lt("\x{212B}", $Aring));
ok($noN->lt("A\x{30A}", $Aring));
ok($noN->lt("A",       "\x{FF21}"));
ok($noN->lt("Z",       "\x{FF21}"));
ok($noN->lt("Z",        $Aring));
ok($noN->lt("\x{212B}", $aring));
ok($noN->lt("A\x{30A}", $aring));
ok($noN->lt("Z",        $aring));
ok($noN->lt("a\x{30A}", "Z"));

ok($nfd->eq("\x{212B}", "A"));
ok($nfd->eq("\x{212B}", $Aring));
ok($nfd->eq("A\x{30A}", $Aring));
ok($nfd->lt("A",       "\x{FF21}"));
ok($nfd->lt("Z",       "\x{FF21}"));
ok($nfd->gt("Z",        $Aring));
ok($nfd->eq("\x{212B}", $aring));
ok($nfd->eq("A\x{30A}", $aring));
ok($nfd->gt("Z",        $aring));
ok($nfd->lt("a\x{30A}", "Z"));

ok($nfc->gt("\x{212B}", "A"));
ok($nfc->eq("\x{212B}", $Aring));
ok($nfc->eq("A\x{30A}", $Aring));
ok($nfc->lt("A",       "\x{FF21}"));
ok($nfc->lt("Z",       "\x{FF21}"));
ok($nfc->lt("Z",        $Aring));
ok($nfc->eq("\x{212B}", $aring));
ok($nfc->eq("A\x{30A}", $aring));
ok($nfc->lt("Z",        $aring));
ok($nfc->gt("a\x{30A}", "Z"));

ok($nfkd->eq("\x{212B}", "A"));
ok($nfkd->eq("\x{212B}", $Aring));
ok($nfkd->eq("A\x{30A}", $Aring));
ok($nfkd->eq("A",       "\x{FF21}"));
ok($nfkd->gt("Z",       "\x{FF21}"));
ok($nfkd->gt("Z",        $Aring));
ok($nfkd->eq("\x{212B}", $aring));
ok($nfkd->eq("A\x{30A}", $aring));
ok($nfkd->gt("Z",        $aring));
ok($nfkd->lt("a\x{30A}", "Z"));

ok($nfkc->gt("\x{212B}", "A"));
ok($nfkc->eq("\x{212B}", $Aring));
ok($nfkc->eq("A\x{30A}", $Aring));
ok($nfkc->eq("A",       "\x{FF21}"));
ok($nfkc->gt("Z",       "\x{FF21}"));
ok($nfkc->lt("Z",        $Aring));
ok($nfkc->eq("\x{212B}", $aring));
ok($nfkc->eq("A\x{30A}", $aring));
ok($nfkc->lt("Z",        $aring));
ok($nfkc->gt("a\x{30A}", "Z"));

$nfd->change(normalization => undef);

ok($nfd->lt("\x{212B}", "A"));
ok($nfd->lt("\x{212B}", $Aring));
ok($nfd->lt("A\x{30A}", $Aring));
ok($nfd->lt("A",       "\x{FF21}"));
ok($nfd->lt("Z",       "\x{FF21}"));
ok($nfd->lt("Z",        $Aring));
ok($nfd->lt("\x{212B}", $aring));
ok($nfd->lt("A\x{30A}", $aring));
ok($nfd->lt("Z",        $aring));
ok($nfd->lt("a\x{30A}", "Z"));

$nfd->change(normalization => 'C');

ok($nfd->gt("\x{212B}", "A"));
ok($nfd->eq("\x{212B}", $Aring));
ok($nfd->eq("A\x{30A}", $Aring));
ok($nfd->lt("A",       "\x{FF21}"));
ok($nfd->lt("Z",       "\x{FF21}"));
ok($nfd->lt("Z",        $Aring));
ok($nfd->eq("\x{212B}", $aring));
ok($nfd->eq("A\x{30A}", $aring));
ok($nfd->lt("Z",        $aring));
ok($nfd->gt("a\x{30A}", "Z"));

$nfd->change(normalization => 'D');

ok($nfd->eq("\x{212B}", "A"));
ok($nfd->eq("\x{212B}", $Aring));
ok($nfd->eq("A\x{30A}", $Aring));
ok($nfd->lt("A",       "\x{FF21}"));
ok($nfd->lt("Z",       "\x{FF21}"));
ok($nfd->gt("Z",        $Aring));
ok($nfd->eq("\x{212B}", $aring));
ok($nfd->eq("A\x{30A}", $aring));
ok($nfd->gt("Z",        $aring));
ok($nfd->lt("a\x{30A}", "Z"));

$nfd->change(normalization => 'KD');

ok($nfd->eq("\x{212B}", "A"));
ok($nfd->eq("\x{212B}", $Aring));
ok($nfd->eq("A\x{30A}", $Aring));
ok($nfd->eq("A",       "\x{FF21}"));
ok($nfd->gt("Z",       "\x{FF21}"));
ok($nfd->gt("Z",        $Aring));
ok($nfd->eq("\x{212B}", $aring));
ok($nfd->eq("A\x{30A}", $aring));
ok($nfd->gt("Z",        $aring));
ok($nfd->lt("a\x{30A}", "Z"));

$nfd->change(normalization => 'KC');

ok($nfd->gt("\x{212B}", "A"));
ok($nfd->eq("\x{212B}", $Aring));
ok($nfd->eq("A\x{30A}", $Aring));
ok($nfd->eq("A",       "\x{FF21}"));
ok($nfd->gt("Z",       "\x{FF21}"));
ok($nfd->lt("Z",        $Aring));
ok($nfd->eq("\x{212B}", $aring));
ok($nfd->eq("A\x{30A}", $aring));
ok($nfd->lt("Z",        $aring));
ok($nfd->gt("a\x{30A}", "Z"));



BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..100\n"; } # 5 + 5 x @Versions
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

my @Versions = ( 8,  9, 11, 14, 16, 18, 20, 22, 24, 26,
		28, 30, 32, 34, 36, 38, 40, 41, 43);

my $Collator = Unicode::Collate->new(
    table => 'keys.txt',
    normalization => undef,
    UCA_Version => 43,
);

ok($Collator->viewSortKey("\x{18B00}"),
   '[FB02 8000 | 0020 | 0002 | FFFF |]');
ok($Collator->viewSortKey("\x{18B01}"),
   '[FB02 8001 | 0020 | 0002 | FFFF |]');
ok($Collator->viewSortKey("\x{18C00}"),
   '[FB02 8100 | 0020 | 0002 | FFFF |]');
ok($Collator->viewSortKey("\x{18CD5}"),
   '[FB02 81D5 | 0020 | 0002 | FFFF |]');

# Khitan < CJK UI (4E00) < Unassigned.

# 18B00..18CD5 are Khitan Characters since UCA_Version 43 (Unicode 13.0).

for my $v (@Versions) {
    $Collator->change(UCA_Version => $v);

    ok($Collator->cmp("\x{18B00}", "\x{4E00}") == ($v >= 43 ? -1 : 1));
    ok($Collator->cmp("\x{18C00}", "\x{4E00}") == ($v >= 43 ? -1 : 1));
    ok($Collator->cmp("\x{18CD5}", "\x{4E00}") == ($v >= 43 ? -1 : 1));
    ok($Collator->cmp("\x{18CD6}", "\x{4E00}") == 1);
    ok($Collator->cmp("\x{18CDF}", "\x{4E00}") == 1);
}

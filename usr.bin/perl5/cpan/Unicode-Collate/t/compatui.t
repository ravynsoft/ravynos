
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..1331\n"; } # 1 + 70 x @Versions
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

# 12 compatibility ideographs are treated as unified ideographs:
# FA0E, FA0F, FA11, FA13, FA14, FA1F, FA21, FA23, FA24, FA27, FA28, FA29.

my $Collator = Unicode::Collate->new(
    table => 'keys.txt',
    normalization => undef,
);

for my $v (@Versions) {
    $Collator->change(UCA_Version => $v);
    ok($Collator->lt("\x{4E00}", "\x{1FFF}"));
    ok($Collator->lt("\x{9FA5}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA00}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA0D}", "\x{1FFF}"));
    ok($Collator->lt("\x{FA0E}", "\x{1FFF}"));
    ok($Collator->lt("\x{FA0F}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA10}", "\x{1FFF}"));
    ok($Collator->lt("\x{FA11}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA12}", "\x{1FFF}"));
    ok($Collator->lt("\x{FA13}", "\x{1FFF}"));
    ok($Collator->lt("\x{FA14}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA15}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA16}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA17}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA18}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA19}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA1A}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA1B}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA1C}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA1D}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA1E}", "\x{1FFF}"));
    ok($Collator->lt("\x{FA1F}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA20}", "\x{1FFF}"));
    ok($Collator->lt("\x{FA21}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA22}", "\x{1FFF}"));
    ok($Collator->lt("\x{FA23}", "\x{1FFF}"));
    ok($Collator->lt("\x{FA24}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA25}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA26}", "\x{1FFF}"));
    ok($Collator->lt("\x{FA27}", "\x{1FFF}"));
    ok($Collator->lt("\x{FA28}", "\x{1FFF}"));
    ok($Collator->lt("\x{FA29}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA2A}", "\x{1FFF}"));
    ok($Collator->gt("\x{FA30}", "\x{1FFF}"));
    ok($Collator->gt("\x{FAFF}", "\x{1FFF}"));
}

my $IgnoreCJK = Unicode::Collate->new(
    table => 'keys.txt',
    normalization => undef,
    overrideCJK => sub {()},
);

for my $v (@Versions) {
    $IgnoreCJK->change(UCA_Version => $v);
    ok($IgnoreCJK->eq("\x{4E00}", ""));
    ok($IgnoreCJK->eq("\x{9FA5}", ""));
    ok($IgnoreCJK->gt("\x{FA00}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FA0D}", "\x{1FFF}"));
    ok($IgnoreCJK->eq("\x{FA0E}", ""));
    ok($IgnoreCJK->eq("\x{FA0F}", ""));
    ok($IgnoreCJK->gt("\x{FA10}", "\x{1FFF}"));
    ok($IgnoreCJK->eq("\x{FA11}", ""));
    ok($IgnoreCJK->gt("\x{FA12}", "\x{1FFF}"));
    ok($IgnoreCJK->eq("\x{FA13}", ""));
    ok($IgnoreCJK->eq("\x{FA14}", ""));
    ok($IgnoreCJK->gt("\x{FA15}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FA16}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FA17}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FA18}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FA19}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FA1A}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FA1B}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FA1C}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FA1D}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FA1E}", "\x{1FFF}"));
    ok($IgnoreCJK->eq("\x{FA1F}", ""));
    ok($IgnoreCJK->gt("\x{FA20}", "\x{1FFF}"));
    ok($IgnoreCJK->eq("\x{FA21}", ""));
    ok($IgnoreCJK->gt("\x{FA22}", "\x{1FFF}"));
    ok($IgnoreCJK->eq("\x{FA23}", ""));
    ok($IgnoreCJK->eq("\x{FA24}", ""));
    ok($IgnoreCJK->gt("\x{FA25}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FA26}", "\x{1FFF}"));
    ok($IgnoreCJK->eq("\x{FA27}", ""));
    ok($IgnoreCJK->eq("\x{FA28}", ""));
    ok($IgnoreCJK->eq("\x{FA29}", ""));
    ok($IgnoreCJK->gt("\x{FA2A}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FA30}", "\x{1FFF}"));
    ok($IgnoreCJK->gt("\x{FAFF}", "\x{1FFF}"));
}


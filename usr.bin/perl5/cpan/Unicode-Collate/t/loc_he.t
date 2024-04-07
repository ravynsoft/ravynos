
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..16\n"; }
my $count = 0;
sub ok ($;$) {
    my $p = my $r = shift;
    if (@_) {
	my $x = shift;
	$p = !defined $x ? !defined $r : !defined $r ? 0 : $r eq $x;
    }
    print $p ? "ok" : "not ok", ' ', ++$count, "\n";
}

use Unicode::Collate::Locale;

ok(1);

sub _pack_U   { Unicode::Collate::pack_U(@_) }
sub _unpack_U { Unicode::Collate::unpack_U(@_) }

#########################

my $objHe = Unicode::Collate::Locale->
    new(locale => 'HE', normalization => undef);

ok($objHe->getlocale, 'he');

$objHe->change(level => 1);

ok($objHe->eq("\x{5F3}", "\x{5F4}"));
ok($objHe->eq("\x{5F3}", ""));
ok($objHe->eq("\x{5F4}", ""));
ok($objHe->eq("\x{5F3}", q<'>));
ok($objHe->eq("\x{5F4}", q<">));

# 7

$objHe->change(variable => 'non-ignorable');

ok($objHe->ne("\x{5F3}", "\x{5F4}"));
ok($objHe->eq("\x{5F3}", q<'>));
ok($objHe->eq("\x{5F4}", q<">));
ok($objHe->eq("\x{5F3}z",q<'z>));
ok($objHe->eq("\x{5F4}z",q<"z>));

# 12

$objHe->change(level => 2);

ok($objHe->lt("\x{5F3}", q<'>));
ok($objHe->lt("\x{5F4}", q<">));
ok($objHe->lt("\x{5F3}z",q<'z>));
ok($objHe->lt("\x{5F4}z",q<"z>));

# 16

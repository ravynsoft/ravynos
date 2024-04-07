
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..8\n"; }
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

my $objAf = Unicode::Collate::Locale->
    new(locale => 'AF', normalization => undef);

ok($objAf->getlocale, 'af');

$objAf->change(level => 1);

ok($objAf->eq("n", "N"));
ok($objAf->eq("N", "\x{149}"));

# 4

$objAf->change(level => 2);

ok($objAf->eq("n", "N"));
ok($objAf->eq("N", "\x{149}"));

# 6

$objAf->change(level => 3);

ok($objAf->lt("n", "N"));
ok($objAf->lt("N", "\x{149}"));

# 8

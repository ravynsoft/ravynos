
BEGIN {
    if ($ENV{PERL_CORE}) {
        chdir('t') if -d 't';
        @INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

#########################

BEGIN {
    use Unicode::Normalize qw(:all);

    unless (exists &Unicode::Normalize::bootstrap or 5.008 <= $]) {
	print "1..0 # skipped: XSUB, or Perl 5.8.0 or later".
		" needed for this test\n";
	print $@;
	exit;
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..17\n"; }
my $count = 0;
sub ok { Unicode::Normalize::ok(\$count, @_) }

ok(1);

package tiescalar;
sub TIESCALAR {
    my ($class, $instance) = @_;
    return bless \$instance => $class;
}
sub FETCH   { return ${$_[0]}++ }
sub STORE   { return ${$_[0]} = $_[1] }
sub DESTROY { undef ${$_[0]} }

#########################

package main;

tie my $tie1, 'tiescalar', "123";
ok(NFD($tie1),  123);
ok(NFC($tie1),  124);
ok(NFKD($tie1), 125);
ok(NFKC($tie1), 126);
ok(FCD($tie1),  127);
ok(FCC($tie1),  128);

tie my $tie2, 'tiescalar', "256";
ok(normalize('NFD',  $tie2), 256);
ok(normalize('NFC',  $tie2), 257);
ok(normalize('NFKD', $tie2), 258);
ok(normalize('NFKC', $tie2), 259);
ok(normalize('FCD',  $tie2), 260);
ok(normalize('FCC',  $tie2), 261);

tie my $tie3, 'tiescalar', "315";
ok(decompose($tie3),         315);
ok(reorder($tie3),           316);
ok(compose($tie3),           317);
ok(composeContiguous($tie3), 318);


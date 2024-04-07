#!./perl -w

chdir 't' if -d 't';
require './test.pl';
use strict;

{
    package End;
    sub DESTROY { $_[0]->() }
    sub main::end(&) {
	my($cleanup) = @_;
	return bless(sub { $cleanup->() }, "End");
    }
}

my($val, $err);

$@ = "t0\n";
$val = eval {
	$@ = "t1\n";
	1;
}; $err = $@;
is($val, 1, "true return value from successful eval block");
is($err, "", "no exception after successful eval block");

$@ = "t0\n";
$val = eval {
	$@ = "t1\n";
	do {
		die "t3\n";
	};
	1;
}; $err = $@;
is($val, undef, "undefined return value from eval block with 'die'");
is($err, "t3\n", "exception after eval block with 'die'");

$@ = "t0\n";
$val = eval {
	$@ = "t1\n";
	local $@ = "t2\n";
	1;
}; $err = $@;
is($val, 1, "true return value from successful eval block with localized \$@");
is($err, "", "no exception after successful eval block with localized \$@");

$@ = "t0\n";
$val = eval {
	$@ = "t1\n";
	local $@ = "t2\n";
	do {
		die "t3\n";
	};
	1;
}; $err = $@;
is($val, undef,
    "undefined return value from eval block with 'die' and localized \$@");
is($err, "t3\n",
    "exception after eval block with 'die' and localized \$@");

$@ = "t0\n";
$val = eval {
	$@ = "t1\n";
	my $c = end { $@ = "t2\n"; };
	1;
}; $err = $@;
is($val, 1, "true return value from eval block with 'end'");
is($err, "", "no exception after eval block with 'end'");

$@ = "t0\n";
$val = eval {
	$@ = "t1\n";
	my $c = end { $@ = "t2\n"; };
	do {
		die "t3\n";
	};
	1;
}; $err = $@;
is($val, undef, "undefined return value from eval block with 'end' and 'die'");
is($err, "t3\n", "exception after eval block with 'end' and 'die'");

done_testing();

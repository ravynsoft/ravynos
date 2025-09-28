#!./perl -w

chdir 't' if -d 't';
require './test.pl';
use strict;

#
# This test checks for $@ being set early during an exceptional
# unwinding, and that this early setting does not affect the late
# setting used to emit the exception from eval{}.  The early setting is
# a backward-compatibility hack to satisfy modules that were relying on
# the historical early setting in order to detect exceptional unwinding.
# This hack should be removed when a proper way to detect exceptional
# unwinding has been developed.
#

{
    package End;
    sub DESTROY { $_[0]->() }
    sub main::end(&) {
	my($cleanup) = @_;
	return bless(sub { $cleanup->() }, "End");
    }
}

my($uerr, $val, $err);

$@ = "";
$val = eval {
	my $c = end { $uerr = $@; $@ = "t2\n"; };
	1;
}; $err = $@;
is($uerr, "", "\$@ false at start of 'end' block inside 'eval' block");
is($val, 1, "successful return from 'eval' block");
is($err, "", "\$@ still false after 'end' block inside 'eval' block");

$@ = "t0\n";
$val = eval {
	$@ = "t1\n";
	my $c = end { $uerr = $@; $@ = "t2\n"; };
	1;
}; $err = $@;
is($uerr, "t1\n", "true value assigned to \$@ before 'end' block inside 'eval' block");
is($val, 1, "successful return from 'eval' block");
is($err, "", "\$@ still false after 'end' block inside 'eval' block");

$@ = "";
$val = eval {
	my $c = end { $uerr = $@; $@ = "t2\n"; };
	do {
		die "t3\n";
	};
	1;
}; $err = $@;
is($uerr, "t3\n");
is($val, undef, "undefined return value from 'eval' block with 'die'");
is($err, "t3\n");

$@ = "t0\n";
$val = eval {
	$@ = "t1\n";
	my $c = end { $uerr = $@; $@ = "t2\n"; };
	do {
		die "t3\n";
	};
	1;
}; $err = $@;
is($uerr, "t3\n");
is($val, undef, "undefined return value from 'eval' block with 'die'");
is($err, "t3\n");

fresh_perl_like(<<'EOS', qr/Custom Message During Global Destruction/, { switches => ['-w'], stderr => 1 } );
package Foo; sub DESTROY { die "Custom Message During Global Destruction" }; package main; our $wut = bless [], "Foo"
EOS

done_testing();

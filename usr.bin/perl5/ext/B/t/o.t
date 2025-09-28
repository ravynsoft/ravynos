#!./perl -w

BEGIN {
	unshift @INC, 't';
	require Config;
	if (($Config::Config{'extensions'} !~ /\bB\b/) ){
		print "1..0 # Skip -- Perl configured without B module\n";
		exit 0;
	}
	require 'test.pl';
}

use strict;

plan( 9 ); # And someone's responsible.

# use() makes it difficult to avoid O::import()
require_ok( 'O' );

my @lines = get_lines( '-MO=success,foo,bar' );

is( $lines[0], 'Compiling!', 'Output should not be saved without -q switch' );
is( $lines[1], '(foo) <bar>', 'O.pm should call backend compile() method' );
is( $lines[2], '[]', 'Nothing should be in $O::BEGIN_output without -q' );
is( $lines[3], '-e syntax OK', 'O.pm should not munge perl output without -qq');

@lines = get_lines( '-MO=-q,success,foo,bar' );
isnt( $lines[1], 'Compiling!', 'Output should not be printed with -q switch' );

is( $lines[1], "[Compiling!", '... but should be in $O::BEGIN_output' );

@lines = get_lines( '-MO=-qq,success,foo,bar' );
is( scalar @lines, 3, '-qq should suppress even the syntax OK message' );

@lines = get_lines( '-MO=success,fail' );
like( $lines[1], qr/fail at .eval/,
	'O.pm should die if backend compile() does not return a subref' );

sub get_lines {
    my $compile = shift;
    split(/[\r\n]+/, runperl( switches => [ '-Ilib', '-It', $compile ],
                              prog => 1, stderr => 1 ));
}

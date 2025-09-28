#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    skip_all_without_perlio();
}

use strict;

# 6 == @char; paired tests inside 3 nested loops,
# plus extra pair of tests in a loop, plus extra pair of tests.
plan tests => 6 ** 3 * 2 + 6 * 2 + 2;

my @char = (pack('U*', 0x40), "\x{4E00}", "\x{4E9C}", "\x{4E02}",
           "\x{FF69}", "\x{304B}");

for my $rs (@char) {
	local $/ = $rs;
	for my $start (@char) {
	    for my $end (@char) {
		my $string = $start.$end;
		my ($expect, $return);
		if ($end eq $rs) {
		    $expect = $start;
		    # The answer will always be a length in utf8, even if the
		    # scalar was encoded with a different length
		    $return = length ($end . "\x{100}") - 1;
		} else {
		    $expect = $string;
		    $return = 0;
		}
		is (chomp ($string), $return);
		is ($string, $expect); # "$enc \$/=$rs $start $end"
	    }
	}
	# chomp should not stringify references unless it decides to modify
	# them
	$_ = [];
	my $got = chomp();
	is ($got, 0);
	is (ref($_), "ARRAY", "chomp ref (no modify)");
}

$/ = ")";  # the last char of something like "ARRAY(0x80ff6e4)"
my $got = chomp();
is ($got, 1);
ok (!ref($_), "chomp ref (modify)");


#!./perl
# vim: ts=4 sts=4 sw=4:

# $! may not be set if EOF was reached without any error.
# https://github.com/Perl/perl5/issues/8431

use strict;
use Config;

chdir 't' if -d 't';
require './test.pl';

plan( tests => 16 );

my $test_prog = 'undef $!;while(<>){print}; print $!';
my $saved_perlio;

BEGIN {
    $saved_perlio = $ENV{PERLIO};
}
END {
    delete $ENV{PERLIO};
    $ENV{PERLIO} = $saved_perlio if defined $saved_perlio;
}

for my $perlio ('perlio', 'stdio') {
    $ENV{PERLIO} = $perlio;
SKIP:
    for my $test_in ("test\n", "test") {
		skip("Guaranteed newline at EOF on VMS", 4) if $^O eq 'VMS' && $test_in eq 'test';
                # perl #71504 added skip in openbsd+threads+stdio;
                # then commit 23705063 made -lpthread the default.
                skip("[perl #71504] OpenBSD test failures in errno.t with ithreads and perlio]; [perl #126306: openbsd t/io/errno.t tests fail randomly]", 8)
                    if $^O eq 'openbsd' && $perlio eq 'stdio';
		my $test_in_esc = $test_in;
		$test_in_esc =~ s/\n/\\n/g;
		for my $rs_code ('', '$/=undef', '$/=\2', '$/=\1024') {
		    TODO:
		    {
			is( runperl( prog => "$rs_code; $test_prog",
						 stdin => $test_in, stderr => 1),
				$test_in,
				"Wrong errno, PERLIO=$ENV{PERLIO} stdin='$test_in_esc', $rs_code");
		    }
		}
    }
}

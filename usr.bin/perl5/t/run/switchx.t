#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}
use strict;

require './test.pl';

# Test '-x'
print runperl( switches => ['-x'],
               progfile => 'run/switchx.aux' );

# Test '-xdir'
print runperl( switches => ['-x./run'],
               progfile => 'run/switchx2.aux',
               args     => [ 4 ] );

curr_test(6);

# Test the error message for not found
like(runperl(switches => ['-x'], progfile => 'run/switchx3.aux', stderr => 1),
     qr/^No Perl script found in input\r?\n\z/,
     "Test the error message when -x can't find a #!perl line");

SKIP: {
    skip("These tests embed newlines in command line arguments, which isn't portable to $^O", 2)
	if $^O eq 'MSWin32' or $^O eq 'VMS';
    my @progs = ("die;\n", "#!perl\n", "warn;\n");
    is(runperl(progs => \@progs, stderr => 1, non_portable => 1),
       "Died at -e line 1.\n", 'Test program dies');
    is(runperl(progs => \@progs, stderr => 1, non_portable => 1,
	       switches => ['-x']),
       "No Perl script found in input\n", '-x and -e gives expected error');
}

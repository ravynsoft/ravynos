#!/usr/bin/perl -w

package main;

use strict;
use Test::More;

# We may see failures with package filehandles if Fatal/autodie
# incorrectly pulls out a cached subroutine from a different package.

# We're using Fatal because package filehandles are likely to
# see more use with Fatal than autodie.

use Fatal qw(open);

eval {
    open(FILE, '<', $0);
};


if ($@) {
    # Holy smokes!  We couldn't even open our own file, bail out...

    plan skip_all => q{Can't open $0 for filehandle tests}
}

plan tests => 4;

my $line = <FILE>;

like($line, qr{perl}, 'Looks like we opened $0 correctly');

close(FILE);

package autodie::test;
use Test::More;

use Fatal qw(open);

eval {
    open(FILE2, '<', $0);
};

is($@,"",'Opened $0 in autodie::test');

my $line2 = <FILE2>;

like($line2, qr{perl}, '...and we can read from $0 fine');

close(FILE2);

package main;

# This shouldn't read anything, because FILE2 should be inside
# autodie::test

no warnings;    # Otherwise we see problems with FILE2
my $wrong_line = <FILE2>;

ok(! defined($wrong_line),q{Filehandles shouldn't leak between packages});
